#include "pcmdi_slave.h"
#include "pcmdi_hardware.h"
#include "pcmdi_header.h"

void pcmdi_slave_begin(struct PCMDISlave *device, struct PCMDIHardware *hardware, struct PCMDIRegisterBank *bank, uint8_t device_address)
{
    device->device_address = device_address;
    device->received_header = PCMDI_HEADER_DEFAULT_VALUE;
    device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_IDLE;
    device->hardware = hardware;
    device->bank = bank;

    device->hardware->pin_direction_writer(device->hardware->pins.clk, PCMDI_PIN_DIRECTION_INPUT);
    device->hardware->pin_direction_writer(device->hardware->pins.tse, PCMDI_PIN_DIRECTION_INPUT);
    device->hardware->pin_direction_writer(device->hardware->pins.dat, PCMDI_PIN_DIRECTION_INPUT);
}

uint8_t pcmdi_slave_read_byte(struct PCMDISlave *device)
{
    uint8_t data = 0;

    for (uint8_t i = 0; i < 8; ++i)
    {
        while (pcmdi_hardware_get_clk_pin_state(device->hardware) != PCMDI_PIN_STATE_EDGE_RISING);

        if (device->hardware->pin_reader(device->hardware->pins.dat))
            data |= (1 << i);
    }

    return data;
}

void pcmdi_slave_write_byte(struct PCMDISlave *device, uint8_t data)
{
    while (pcmdi_hardware_get_clk_pin_state(device->hardware) != PCMDI_PIN_STATE_EDGE_FALLING);

    device->hardware->pin_direction_writer(device->hardware->pins.dat, PCMDI_PIN_DIRECTION_OUTPUT);

    for (uint8_t i = 0; i < 8; ++i)
    {
        device->hardware->pin_writer(device->hardware->pins.dat, (data >> i) & 0x01);
        while (pcmdi_hardware_get_clk_pin_state(device->hardware) != PCMDI_PIN_STATE_EDGE_RISING);
    }
    
    device->hardware->pin_writer(device->hardware->pins.dat, PCMDI_PIN_LEVEL_LOW);
    device->hardware->pin_direction_writer(device->hardware->pins.dat, PCMDI_PIN_DIRECTION_INPUT);
}

void pcmdi_slave_update_internal_mode(struct PCMDISlave *device)
{
    enum PCMDIPinState tse_pin_state = pcmdi_hardware_get_tse_pin_state(device->hardware);

    switch (tse_pin_state)
    {
        case PCMDI_PIN_STATE_LEVEL_LOW:
            device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_IDLE;
            return;

        case PCMDI_PIN_STATE_LEVEL_HIGH:
            return;

        case PCMDI_PIN_STATE_EDGE_FALLING:
            device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_CLEANUP;
            return;

        case PCMDI_PIN_STATE_EDGE_RISING:
            device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_WAIT_FOR_HEADER;
            return;
    }
}

void pcmdi_slave_listen(struct PCMDISlave *device)
{
    pcmdi_slave_update_internal_mode(device);

    switch (device->internal_mode)
    {
        case PCMDI_SLAVE_INTERNAL_MODE_IDLE:
            return;

        case PCMDI_SLAVE_INTERNAL_MODE_WAIT_FOR_HEADER:
            pcmdi_slave_read_header(device);
            return;

        case PCMDI_SLAVE_INTERNAL_MODE_HANDLE_REQUEST:
            pcmdi_slave_handle_request(device);
            return;

        case PCMDI_SLAVE_INTERNAL_MODE_CLEANUP:
            device->received_header = PCMDI_HEADER_DEFAULT_VALUE;
            device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_IDLE;
            return;
    }
}

void pcmdi_slave_read_header(struct PCMDISlave *device)
{
    uint8_t high_byte_of_header = pcmdi_slave_read_byte(device);

    if (high_byte_of_header != device->device_address)
    {
        device->received_header = PCMDI_HEADER_DEFAULT_VALUE;
        device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_IDLE;
        return;
    }

    uint8_t low_byte_of_header = pcmdi_slave_read_byte(device); 

    device->received_header = ((uint16_t) (high_byte_of_header << 8) | low_byte_of_header); 

    device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_HANDLE_REQUEST;
}

void pcmdi_slave_handle_request(struct PCMDISlave *device)
{
    enum PCMDIInternalRegisterDescriptor ird = pcmdi_header_get_ird(&device->received_header);

    switch (ird)
    {
        case PCMDI_IRD_DISCOVER_REQUEST:
            pcmdi_slave_respond_to_discover_request(device);
            break;

        case PCMDI_IRD_BANK_SIZE_REQUEST:
            pcmdi_slave_respond_to_bank_size_request(device);
            break;

        case PCMDI_IRD_REGISTER_SIZE_REQUEST:
            pcmdi_slave_respond_to_register_size_request(device);
            break;

        case PCMDI_IRD_REGISTER_DATA_REQUEST:
            pcmdi_slave_respond_to_register_data_request(device);
            break;
    }

    device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_CLEANUP;
}

void pcmdi_slave_respond_to_discover_request(struct PCMDISlave *device)
{
    pcmdi_slave_write_byte(device, 0xff);
}

void pcmdi_slave_respond_to_bank_size_request(struct PCMDISlave *device)
{
    pcmdi_slave_write_byte(device, device->bank->size);
}

void pcmdi_slave_respond_to_register_size_request(struct PCMDISlave *device)
{
    uint8_t requested_rid = pcmdi_header_get_rid(&device->received_header);

    struct PCMDIRegister *found_register = pcmdi_register_bank_find_register(device->bank, requested_rid); 

    if (!found_register)
        return;

    pcmdi_slave_write_byte(device, found_register->size >> 8);
    pcmdi_slave_write_byte(device, found_register->size & 0x00FF);
}

void pcmdi_slave_respond_to_register_data_request(struct PCMDISlave *device)
{
    uint8_t requested_rid = pcmdi_header_get_rid(&device->received_header);
    struct PCMDIRegister *found_register = pcmdi_register_bank_find_register(device->bank, requested_rid); 

    if (!found_register)
        return;

    enum PCMDITransmissionMode transmission_mode = pcmdi_header_get_transmission_mode(&device->received_header);

    uint8_t *register_data_pointer = (uint8_t *) found_register->data_pointer;

    for (uint16_t i = 0; i < found_register->size; ++i)
    {
        // When master asks to read data, slave needs to write the data to the bus
        if (transmission_mode == PCMDI_TRANSMISSION_MODE_READ)
            pcmdi_slave_write_byte(device, register_data_pointer[i]);

        // When master asks to write data, slave needs to read the data from the bus
        else if (transmission_mode == PCMDI_TRANSMISSION_MODE_WRITE)
            register_data_pointer[i] = pcmdi_slave_read_byte(device);
    }
}
