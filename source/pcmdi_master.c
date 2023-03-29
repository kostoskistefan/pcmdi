#include "pcmdi_master.h"
#include "pcmdi_header.h"

#define PCMDI_MAXIMUM_DEVICE_LIMIT 255

void pcmdi_master_begin(struct PCMDIMaster *device, struct PCMDIHardware *hardware, void (*delay_us)(uint32_t us))
{
    device->maximum_number_of_slave_devices = PCMDI_MAXIMUM_DEVICE_LIMIT;

    device->hardware = hardware;
    device->delay_us = delay_us;

    device->hardware->pin_direction_writer(device->hardware->pins.clk, PCMDI_PIN_DIRECTION_OUTPUT);
    device->hardware->pin_direction_writer(device->hardware->pins.tse, PCMDI_PIN_DIRECTION_OUTPUT);
    device->hardware->pin_direction_writer(device->hardware->pins.dat, PCMDI_PIN_DIRECTION_OUTPUT);

    device->hardware->pin_writer(device->hardware->pins.clk, PCMDI_PIN_LEVEL_LOW);
    device->hardware->pin_writer(device->hardware->pins.tse, PCMDI_PIN_LEVEL_LOW);
    device->hardware->pin_writer(device->hardware->pins.dat, PCMDI_PIN_LEVEL_LOW);
}

void pcmdi_master_set_maximum_number_of_slave_devices(struct PCMDIMaster *device, uint8_t maximum_number_of_slave_devices)
{
    device->maximum_number_of_slave_devices = maximum_number_of_slave_devices;
}

void pcmdi_master_start_transmission(struct PCMDIMaster *device, uint16_t *header)
{
    device->hardware->pin_writer(device->hardware->pins.dat, PCMDI_PIN_LEVEL_LOW);
    device->hardware->pin_writer(device->hardware->pins.tse, PCMDI_PIN_LEVEL_HIGH);
    device->delay_us(PCMDI_PIN_MANIPULATION_DELAY_TIME);
    
    pcmdi_master_write_byte(device, *header >> 8);
    pcmdi_master_write_byte(device, *header & 0x00FF);
}

void pcmdi_master_end_transmission(struct PCMDIMaster *device)
{
    device->hardware->pin_writer(device->hardware->pins.dat, PCMDI_PIN_LEVEL_LOW);
    device->hardware->pin_writer(device->hardware->pins.tse, PCMDI_PIN_LEVEL_LOW);
    device->delay_us(PCMDI_PIN_MANIPULATION_DELAY_TIME);
}

uint8_t pcmdi_master_read_byte(struct PCMDIMaster *device)
{
    device->hardware->pin_direction_writer(device->hardware->pins.dat, PCMDI_PIN_DIRECTION_INPUT);
    device->delay_us(PCMDI_PIN_MANIPULATION_DELAY_TIME);

    uint8_t data = 0;

    for (uint8_t i = 0; i < 8; ++i)
    {
        if (device->hardware->pin_reader(device->hardware->pins.dat))
            data |= (1 << i);

        pcmdi_master_sync(device);
    }
    
    device->hardware->pin_direction_writer(device->hardware->pins.dat, PCMDI_PIN_DIRECTION_OUTPUT);
    device->delay_us(PCMDI_PIN_MANIPULATION_DELAY_TIME);

    return data;
}

void pcmdi_master_write_byte(struct PCMDIMaster *device, uint8_t data)
{
    for (uint8_t i = 0; i < 8; ++i)
    {
        device->hardware->pin_writer(device->hardware->pins.dat, (data >> i) & 0x01);
        device->delay_us(PCMDI_PIN_MANIPULATION_DELAY_TIME);
        pcmdi_master_sync(device);
    }
}

void pcmdi_master_read(struct PCMDIMaster *device, uint16_t size, uint8_t *out_data)
{
    for (uint16_t i = 0; i < size; ++i)
        out_data[i] = pcmdi_master_read_byte(device);
}

void pcmdi_master_write(struct PCMDIMaster *device, uint16_t size, uint8_t *data)
{
    for (uint16_t i = 0; i < size; ++i)
        pcmdi_master_write_byte(device, data[i]);
}

void pcmdi_master_sync(struct PCMDIMaster *device)
{
    device->hardware->pin_writer(device->hardware->pins.clk, PCMDI_PIN_LEVEL_HIGH);
    device->delay_us(PCMDI_PIN_MANIPULATION_DELAY_TIME);
    device->hardware->pin_writer(device->hardware->pins.clk, PCMDI_PIN_LEVEL_LOW);
    device->delay_us(PCMDI_PIN_MANIPULATION_DELAY_TIME);
}

uint8_t pcmdi_master_discover(struct PCMDIMaster *device, uint8_t *out_available_device_container)
{
    uint16_t header = 0;

    pcmdi_header_set_transmission_mode(&header, PCMDI_TRANSMISSION_MODE_READ);
    pcmdi_header_set_ird(&header, PCMDI_IRD_DISCOVER_REQUEST);

    uint8_t number_of_discovered_devices = 0;

    for (uint8_t address = 0; address < device->maximum_number_of_slave_devices; ++address)
    {
        pcmdi_header_set_address(&header, address);

        pcmdi_master_start_transmission(device, &header);

        if (pcmdi_master_read_byte(device) != 0)
        {
            out_available_device_container[number_of_discovered_devices] = address;
            ++number_of_discovered_devices;
        }

        pcmdi_master_end_transmission(device);
    }

    return number_of_discovered_devices;
}

uint8_t pcmdi_master_request_bank_size(struct PCMDIMaster *device, uint8_t address)
{
    uint16_t header = 0;

    pcmdi_header_set_address(&header, address);
    pcmdi_header_set_transmission_mode(&header, PCMDI_TRANSMISSION_MODE_READ);
    pcmdi_header_set_ird(&header, PCMDI_IRD_BANK_SIZE_REQUEST);

    pcmdi_master_start_transmission(device, &header);

    uint8_t bank_size = pcmdi_master_read_byte(device);

    pcmdi_master_end_transmission(device);

    return bank_size;
}

uint16_t pcmdi_master_request_register_size(struct PCMDIMaster *device, uint8_t address, uint8_t rid)
{
    uint16_t header = 0;

    pcmdi_header_set_address(&header, address);
    pcmdi_header_set_transmission_mode(&header, PCMDI_TRANSMISSION_MODE_READ);
    pcmdi_header_set_ird(&header, PCMDI_IRD_REGISTER_SIZE_REQUEST);
    pcmdi_header_set_rid(&header, rid);

    pcmdi_master_start_transmission(device, &header);

    uint8_t register_size[2];
    pcmdi_master_read(device, 2, register_size);

    pcmdi_master_end_transmission(device);

    return (uint16_t) (register_size[0] << 8) | register_size[1];
}

void pcmdi_master_read_register(struct PCMDIMaster *device, uint8_t address, uint8_t rid, uint16_t size, void *out_data)
{
    uint16_t header = 0;

    pcmdi_header_set_address(&header, address);
    pcmdi_header_set_transmission_mode(&header, PCMDI_TRANSMISSION_MODE_READ);
    pcmdi_header_set_ird(&header, PCMDI_IRD_REGISTER_DATA_REQUEST);
    pcmdi_header_set_rid(&header, rid);

    pcmdi_master_start_transmission(device, &header);

    pcmdi_master_read(device, size, (uint8_t *) out_data);

    pcmdi_master_end_transmission(device);
}

void pcmdi_master_write_register(struct PCMDIMaster *device, uint8_t address, uint8_t rid, uint16_t size, void *data)
{
    uint16_t header = 0;

    pcmdi_header_set_address(&header, address);
    pcmdi_header_set_transmission_mode(&header, PCMDI_TRANSMISSION_MODE_WRITE);
    pcmdi_header_set_ird(&header, PCMDI_IRD_REGISTER_DATA_REQUEST);
    pcmdi_header_set_rid(&header, rid);

    pcmdi_master_start_transmission(device, &header);

    pcmdi_master_write(device, size, (uint8_t *) data);

    pcmdi_master_end_transmission(device);
}
