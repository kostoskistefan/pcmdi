#include "pcmdi_slave.h"
#include "pcmdi_slave_internal.h"

void pcmdi_slave_begin(struct PCMDISlave *slave_device, struct PCMDIHardware *hardware, uint8_t device_address)
{
    slave_device->device_address = device_address;
    slave_device->received_header = 0x0000;
    slave_device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_IDLE;
    slave_device->hardware = *hardware;

    slave_device->hardware.pin_direction_writer(slave_device->hardware.pins.clk, PCMDI_PIN_DIRECTION_INPUT);
    slave_device->hardware.pin_direction_writer(slave_device->hardware.pins.tse, PCMDI_PIN_DIRECTION_INPUT);
    slave_device->hardware.pin_direction_writer(slave_device->hardware.pins.dat, PCMDI_PIN_DIRECTION_INPUT);

    pcmdi_register_bank_create(&slave_device->bank);
}

void pcmdi_slave_listen(struct PCMDISlave *slave_device)
{
    pcmdi_slave_update_internal_mode(slave_device);

    switch (slave_device->internal_mode)
    {
        case PCMDI_SLAVE_INTERNAL_MODE_IDLE:
            return;

        case PCMDI_SLAVE_INTERNAL_MODE_WAIT_FOR_HEADER:
            pcmdi_slave_read_header(slave_device);
            return;

        case PCMDI_SLAVE_INTERNAL_MODE_HANDLE_REQUEST:
            pcmdi_slave_handle_request(slave_device);
            return;

        case PCMDI_SLAVE_INTERNAL_MODE_CLEANUP:
            slave_device->received_header = 0x0000;
            return;
    }
}

void pcmdi_slave_update_internal_mode(struct PCMDISlave *slave_device)
{
    enum PCMDIPinState tse_pin_state = pcmdi_hardware_get_tse_pin_state(&slave_device->hardware);

    switch (tse_pin_state)
    {
        case PCMDI_PIN_STATE_LEVEL_LOW:
            slave_device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_IDLE;
            return;

        case PCMDI_PIN_STATE_LEVEL_HIGH:
            return;

        case PCMDI_PIN_STATE_EDGE_FALLING:
            slave_device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_CLEANUP;
            return;

        case PCMDI_PIN_STATE_EDGE_RISING:
            slave_device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_WAIT_FOR_HEADER;
            return;
    }
}

void pcmdi_slave_read_header(struct PCMDISlave *slave_device)
{
    int8_t bit_counter = 15;

    while (bit_counter >= 0)
    {
        // TODO: Handle TSE errors (ex. TSE going low inside while loop)

        if (bit_counter == 7)
        {
            if (pcmdi_header_get_address(&slave_device->received_header) != slave_device->device_address)
            {
                slave_device->received_header = 0x0000;
                slave_device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_IDLE;
                return;
            }
        }

        enum PCMDIPinState clk_pin_state = pcmdi_hardware_get_clk_pin_state(&slave_device->hardware);

        if (clk_pin_state != PCMDI_PIN_STATE_EDGE_RISING)
            continue;

        slave_device->received_header = 
            (slave_device->received_header & ~(1 << bit_counter)) | 
            (slave_device->hardware.pin_reader(slave_device->hardware.pins.dat) << bit_counter);

        --bit_counter;
    }

    slave_device->internal_mode = PCMDI_SLAVE_INTERNAL_MODE_HANDLE_REQUEST;
}

void pcmdi_slave_handle_request(struct PCMDISlave *slave_device)
{
    enum PCMDIInternalRegisterDescriptor ird = pcmdi_header_get_ird(&slave_device->received_header);

    switch (ird)
    {
        case PCMDI_IRD_DISCOVER_REQUEST:
            pcmdi_slave_respond_to_discover_request(slave_device);
            break;

        default:
            break;
    }
}

void pcmdi_slave_respond_to_discover_request(struct PCMDISlave *slave_device)
{
    slave_device->hardware.pin_direction_writer(slave_device->hardware.pins.dat, PCMDI_PIN_DIRECTION_OUTPUT);
    slave_device->hardware.pin_writer(slave_device->hardware.pins.dat, PCMDI_PIN_LEVEL_HIGH);

    while (slave_device->hardware.pin_reader(slave_device->hardware.pins.tse));

    slave_device->hardware.pin_writer(slave_device->hardware.pins.dat, PCMDI_PIN_LEVEL_LOW);
    slave_device->hardware.pin_direction_writer(slave_device->hardware.pins.dat, PCMDI_PIN_DIRECTION_INPUT);
}
