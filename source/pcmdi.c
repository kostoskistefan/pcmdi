#include "pcmdi.h"
#include "pcmdi_internal.h"

// A counter used to save the current bit
// position of the receive buffer when reading
static uint16_t bit_counter = 0;

void pcmdi_begin(struct PCMDI *pcmdi, uint8_t clk, uint8_t tse, uint8_t dat)
{
    pcmdi->device_mode = PCMDI_DEVICE_MODE_IDLE;
    pcmdi->device_address = 0x00;

    pcmdi->hardware.pins.clk = clk;
    pcmdi->hardware.pins.tse = tse;
    pcmdi->hardware.pins.dat = dat;

    pcmdi->flags.received_address_matches_this_device = 0;

    pcmdi_clear_receive_flags(pcmdi);
    pcmdi_clear_receive_buffer(pcmdi);
}

void pcmdi_start_transmission(struct PCMDI *pcmdi, uint8_t address)
{
    pcmdi->hardware.pin_writer(pcmdi->hardware.pins.dat, 0);
    pcmdi->hardware.pin_writer(pcmdi->hardware.pins.tse, 1);
    pcmdi->hardware.delay_us(PCMDI_PIN_DELAY_TIME);

    pcmdi_write_byte(pcmdi, address);
}

void pcmdi_end_transmission(struct PCMDI *pcmdi)
{
    pcmdi->hardware.pin_writer(pcmdi->hardware.pins.dat, 0);
    pcmdi->hardware.pin_writer(pcmdi->hardware.pins.tse, 0);
    pcmdi->hardware.delay_us(PCMDI_PIN_DELAY_TIME);
}

uint8_t pcmdi_data_available(struct PCMDI *pcmdi)
{
    return pcmdi->flags.receive_buffer_data_ready;
}

void pcmdi_set_device_address(struct PCMDI *pcmdi, uint8_t device_address)
{
    pcmdi->device_address = device_address;
}

void pcmdi_read_request(struct PCMDI *pcmdi, uint8_t *out_data, uint8_t data_length)
{
    if (data_length > PCMDI_BUFFER_SIZE)
        return;

    for (uint8_t i = 0; i < data_length; i++)
        out_data[i] = pcmdi->buffers.receive_buffer[i];

    pcmdi_clear_receive_flags(pcmdi);
    pcmdi_clear_receive_buffer(pcmdi);
}

void pcmdi_write(struct PCMDI *pcmdi, uint8_t *data, uint8_t data_length)
{
    for (uint8_t i = 0; i < data_length; i++)
        pcmdi_write_byte(pcmdi, data[i]);
}

void pcmdi_write_byte(struct PCMDI *pcmdi, uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        pcmdi->hardware.pin_writer(pcmdi->hardware.pins.dat, (data >> i) & 0x01);
        pcmdi->hardware.delay_us(PCMDI_PIN_DELAY_TIME);
        pcmdi_sync(pcmdi);
    }
}

void pcmdi_run(struct PCMDI *pcmdi)
{
    pcmdi_update_device_mode(pcmdi);

    if (pcmdi->device_mode == PCMDI_DEVICE_MODE_IDLE)
        return;

    if (pcmdi_get_clk_pin_state(pcmdi) != PCMDI_PIN_STATE_RISING)
        return;

    pcmdi_read_incoming_bit_into_buffer(pcmdi);
    
    if (bit_counter >= PCMDI_ADDRESS_BUFFER_LENGTH)
        pcmdi->flags.receive_buffer_was_written = 1;

    bit_counter = (bit_counter + 1) % (PCMDI_BUFFER_SIZE * 8);
}

void pcmdi_set_bit(uint8_t *data, uint8_t bit_position, uint8_t value)
{
    uint8_t mask = (1 << bit_position);
    *data = (*data & ~mask) | (value << bit_position);
}

void pcmdi_read_incoming_bit_into_buffer(struct PCMDI *pcmdi)
{
    pcmdi_set_bit(
        &pcmdi->buffers.receive_buffer[bit_counter / 8], 
        bit_counter % 8, 
        pcmdi->hardware.pin_reader(pcmdi->hardware.pins.dat));
}

enum PCMDIPinState pcmdi_get_pin_state(struct PCMDI *pcmdi, uint8_t pin, uint8_t *previous_state)
{
    uint8_t current_state = pcmdi->hardware.pin_reader(pin);

    if (current_state == 0 && *previous_state == 1)
    {
        *previous_state = 0;
        return PCMDI_PIN_STATE_FALLING;
    }

    if (current_state == 1 && *previous_state == 0)
    {
        *previous_state = 1;
        return PCMDI_PIN_STATE_RISING;
    }

    if (current_state == 1 && *previous_state == 1)
        return PCMDI_PIN_STATE_HIGH;

    return PCMDI_PIN_STATE_LOW;
}

enum PCMDIPinState pcmdi_get_tse_pin_state(struct PCMDI *pcmdi)
{
    static uint8_t previous_state = 0;

    return pcmdi_get_pin_state(pcmdi, pcmdi->hardware.pins.tse, &previous_state);
}

enum PCMDIPinState pcmdi_get_clk_pin_state(struct PCMDI *pcmdi)
{
    static uint8_t previous_state = 0;

    return pcmdi_get_pin_state(pcmdi, pcmdi->hardware.pins.clk, &previous_state);
}

void pcmdi_check_received_address_match(struct PCMDI *pcmdi)
{
    pcmdi->flags.received_address_matches_this_device = 
        pcmdi->buffers.receive_buffer[PCMDI_ADDRESS_BUFFER_INDEX] == pcmdi->device_address;
}

void pcmdi_update_device_mode(struct PCMDI *pcmdi)
{
    enum PCMDIPinState tse_pin_state = pcmdi_get_tse_pin_state(pcmdi);

    switch (tse_pin_state)
    {
        case PCMDI_PIN_STATE_RISING:
            bit_counter = 0;
            pcmdi_clear_receive_flags(pcmdi);
            pcmdi_clear_receive_buffer(pcmdi);
            pcmdi->device_mode = PCMDI_DEVICE_MODE_READING_ADDRESS;
            break;

        case PCMDI_PIN_STATE_HIGH:
            if (bit_counter < PCMDI_ADDRESS_BUFFER_LENGTH)
                return;

            if (pcmdi->device_mode == PCMDI_DEVICE_MODE_READING_ADDRESS)
            {
                bit_counter = 0;
                pcmdi_check_received_address_match(pcmdi);
            }
    
            if (pcmdi->flags.received_address_matches_this_device)
                pcmdi->device_mode = PCMDI_DEVICE_MODE_READING_DATA;
                
            else 
                pcmdi->device_mode = PCMDI_DEVICE_MODE_IDLE;
            
            break;

        case PCMDI_PIN_STATE_FALLING:
            if (pcmdi->flags.received_address_matches_this_device && pcmdi->flags.receive_buffer_was_written)
                pcmdi->flags.receive_buffer_data_ready = 1;
                
            pcmdi->device_mode = PCMDI_DEVICE_MODE_IDLE;
            break;

        case PCMDI_PIN_STATE_LOW:
            pcmdi->device_mode = PCMDI_DEVICE_MODE_IDLE;
            break;
    }
}

void pcmdi_sync(struct PCMDI *pcmdi)
{
    pcmdi->hardware.pin_writer(pcmdi->hardware.pins.clk, 1);
    pcmdi->hardware.delay_us(PCMDI_PIN_DELAY_TIME);
    pcmdi->hardware.pin_writer(pcmdi->hardware.pins.clk, 0);
    pcmdi->hardware.delay_us(PCMDI_PIN_DELAY_TIME);
}

void pcmdi_clear_receive_buffer(struct PCMDI *pcmdi)
{
    for (uint8_t i = 0; i < sizeof(pcmdi->buffers.receive_buffer); i++)
        pcmdi->buffers.receive_buffer[i] = 0;
}

void pcmdi_clear_receive_flags(struct PCMDI *pcmdi)
{
    pcmdi->flags.receive_buffer_data_ready = 0;
    pcmdi->flags.receive_buffer_was_written = 0;
}

void pcmdi_register_delay_us(struct PCMDI *pcmdi, void (*delay_us)(unsigned int us))
{
    pcmdi->hardware.delay_us = delay_us;
}

void pcmdi_register_pin_reader(struct PCMDI *pcmdi, int (*pin_reader)(uint8_t pin))
{
    pcmdi->hardware.pin_reader = pin_reader;
}

void pcmdi_register_pin_writer(struct PCMDI *pcmdi, void (*pin_writer)(uint8_t pin, uint8_t value))
{
    pcmdi->hardware.pin_writer = pin_writer;
}
