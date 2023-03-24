#include "pcmdi_master.h"
#include "pcmdi_master_internal.h"

#define PCMDI_MAXIMUM_DEVICE_LIMIT 255

void pcmdi_master_begin(struct PCMDIMaster *master_device, struct PCMDIHardware *hardware)
{
    master_device->maximum_number_of_slave_devices = PCMDI_MAXIMUM_DEVICE_LIMIT;

    master_device->hardware = *hardware;

    master_device->hardware.pin_direction_writer(master_device->hardware.pins.clk, PCMDI_PIN_DIRECTION_OUTPUT);
    master_device->hardware.pin_direction_writer(master_device->hardware.pins.tse, PCMDI_PIN_DIRECTION_OUTPUT);
    master_device->hardware.pin_direction_writer(master_device->hardware.pins.dat, PCMDI_PIN_DIRECTION_OUTPUT);

    master_device->hardware.pin_writer(master_device->hardware.pins.clk, PCMDI_PIN_LEVEL_LOW);
    master_device->hardware.pin_writer(master_device->hardware.pins.tse, PCMDI_PIN_LEVEL_LOW);
    master_device->hardware.pin_writer(master_device->hardware.pins.dat, PCMDI_PIN_LEVEL_LOW);
}

void pcmdi_master_set_maximum_number_of_slave_devices(struct PCMDIMaster *master_device, uint8_t maximum_number_of_slave_devices)
{
    master_device->maximum_number_of_slave_devices = maximum_number_of_slave_devices;
}

void pcmdi_master_start_transmission(struct PCMDIMaster *master_device)
{
    master_device->hardware.pin_direction_writer(master_device->hardware.pins.dat, PCMDI_PIN_DIRECTION_OUTPUT);
    master_device->hardware.pin_writer(master_device->hardware.pins.dat, 0);
    master_device->hardware.pin_writer(master_device->hardware.pins.tse, 1);
    master_device->hardware.delay_us(PCMDI_PIN_DELAY_TIME);
}

void pcmdi_master_end_transmission(struct PCMDIMaster *master_device)
{
    master_device->hardware.pin_direction_writer(master_device->hardware.pins.dat, PCMDI_PIN_DIRECTION_OUTPUT);
    master_device->hardware.pin_writer(master_device->hardware.pins.dat, 0);
    master_device->hardware.pin_writer(master_device->hardware.pins.tse, 0);
    master_device->hardware.delay_us(PCMDI_PIN_DELAY_TIME);
}

void pcmdi_master_transmit_header(struct PCMDIMaster *master_device, uint16_t *header)
{
    pcmdi_master_write_byte(master_device, *header >> 8);
    pcmdi_master_write_byte(master_device, *header & 0xFF);
}

uint8_t pcmdi_master_read_byte(struct PCMDIMaster *master_device)
{
    uint8_t data = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        if (master_device->hardware.pin_reader(master_device->hardware.pins.dat))
            data |= (1 << i);

        pcmdi_master_sync(master_device);
    }

    return data;
}

void pcmdi_master_write_byte(struct PCMDIMaster *master_device, uint8_t data)
{
    for (int8_t i = 7; i >= 0; i--)
    {
        master_device->hardware.pin_writer(master_device->hardware.pins.dat, (data >> i) & 0x01);
        master_device->hardware.delay_us(PCMDI_PIN_DELAY_TIME);
        pcmdi_master_sync(master_device);
    }
}

void pcmdi_master_read(struct PCMDIMaster *master_device, uint8_t size, uint8_t *out_data)
{
    for (uint8_t i = 0; i < size; i++)
        out_data[i] = pcmdi_master_read_byte(master_device);
}

void pcmdi_master_write(struct PCMDIMaster *master_device, uint8_t size, uint8_t *data)
{
    for (uint8_t i = 0; i < size; i++)
        pcmdi_master_write_byte(master_device, data[i]);
}

void pcmdi_master_sync(struct PCMDIMaster *master_device)
{
    master_device->hardware.pin_writer(master_device->hardware.pins.clk, PCMDI_PIN_LEVEL_HIGH);
    master_device->hardware.delay_us(PCMDI_PIN_DELAY_TIME);
    master_device->hardware.pin_writer(master_device->hardware.pins.clk, PCMDI_PIN_LEVEL_LOW);
    master_device->hardware.delay_us(PCMDI_PIN_DELAY_TIME);
}

uint8_t pcmdi_master_discover(struct PCMDIMaster *master_device, uint8_t *out_available_device_container)
{
    uint16_t header = 0;

    pcmdi_header_set_transmission_mode(&header, PCMDI_TRANSMISSION_MODE_READ);
    pcmdi_header_set_ird(&header, PCMDI_IRD_DISCOVER_REQUEST);

    uint8_t number_of_discovered_master_devices = 0;

    for (uint8_t address = 0; address < master_device->maximum_number_of_slave_devices; address++)
    {
        pcmdi_header_set_address(&header, address);

        pcmdi_master_start_transmission(master_device);

        pcmdi_master_transmit_header(master_device, &header);

        master_device->hardware.pin_direction_writer(master_device->hardware.pins.dat, PCMDI_PIN_DIRECTION_INPUT);
        master_device->hardware.delay_us(PCMDI_PIN_DELAY_TIME);

        if (pcmdi_master_read_byte(master_device) != 0)
        {
            out_available_device_container[number_of_discovered_master_devices] = address;
            ++number_of_discovered_master_devices;
        }

        pcmdi_master_end_transmission(master_device);
    }

    return number_of_discovered_master_devices;
}

uint8_t pcmdi_master_request_bank_size(struct PCMDIMaster *master_device, uint8_t address)
{
    uint16_t header = 0;

    pcmdi_header_set_address(&header, address);
    pcmdi_header_set_transmission_mode(&header, PCMDI_TRANSMISSION_MODE_READ);
    pcmdi_header_set_ird(&header, PCMDI_IRD_BANK_SIZE_REQUEST);

    pcmdi_master_start_transmission(master_device);

    pcmdi_master_transmit_header(master_device, &header);

    master_device->hardware.pin_direction_writer(master_device->hardware.pins.dat, PCMDI_PIN_DIRECTION_INPUT);

    uint8_t bank_size = pcmdi_master_read_byte(master_device);

    pcmdi_master_end_transmission(master_device);

    return bank_size;
}

uint16_t pcmdi_master_request_register_size(struct PCMDIMaster *master_device, uint8_t address, uint8_t rid)
{
    uint16_t header = 0;

    pcmdi_header_set_address(&header, address);
    pcmdi_header_set_transmission_mode(&header, PCMDI_TRANSMISSION_MODE_READ);
    pcmdi_header_set_ird(&header, PCMDI_IRD_REGISTER_SIZE_REQUEST);
    pcmdi_header_set_rid(&header, rid);

    pcmdi_master_start_transmission(master_device);

    pcmdi_master_transmit_header(master_device, &header);

    master_device->hardware.pin_direction_writer(master_device->hardware.pins.dat, PCMDI_PIN_DIRECTION_INPUT);

    uint8_t register_size[2];
    pcmdi_master_read(master_device, 2, register_size);

    pcmdi_master_end_transmission(master_device);

    return (uint16_t) (register_size[0] << 8) | register_size[1];
}

void pcmdi_master_read_register(struct PCMDIMaster *master_device, uint8_t address, uint8_t rid, uint16_t size, uint8_t *out_data)
{
    uint16_t header = 0;

    pcmdi_header_set_address(&header, address);
    pcmdi_header_set_transmission_mode(&header, PCMDI_TRANSMISSION_MODE_READ);
    pcmdi_header_set_ird(&header, PCMDI_IRD_REGISTER_DATA_REQUEST);
    pcmdi_header_set_rid(&header, rid);

    pcmdi_master_start_transmission(master_device);

    pcmdi_master_transmit_header(master_device, &header);

    master_device->hardware.pin_direction_writer(master_device->hardware.pins.dat, PCMDI_PIN_DIRECTION_INPUT);

    pcmdi_master_read(master_device, size, out_data);

    pcmdi_master_end_transmission(master_device);
}

void pcmdi_master_write_register(struct PCMDIMaster *master_device, uint8_t address, uint8_t rid, uint16_t size, uint8_t *data)
{
    uint16_t header = 0;

    pcmdi_header_set_address(&header, address);
    pcmdi_header_set_transmission_mode(&header, PCMDI_TRANSMISSION_MODE_WRITE);
    pcmdi_header_set_ird(&header, PCMDI_IRD_REGISTER_DATA_REQUEST);
    pcmdi_header_set_rid(&header, rid);

    pcmdi_master_start_transmission(master_device);

    pcmdi_master_transmit_header(master_device, &header);

    pcmdi_master_write(master_device, size, data);

    pcmdi_master_end_transmission(master_device);
}
