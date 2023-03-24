#include "pcmdi_hardware.h"

void pcmdi_hardware_specify_pins(struct PCMDIHardware *hardware, uint8_t clk, uint8_t tse, uint8_t dat)
{
    hardware->pins.clk = clk;
    hardware->pins.tse = tse;
    hardware->pins.dat = dat;
}

void pcmdi_hardware_specify_delay_function(struct PCMDIHardware *hardware, void (*delay_us)(unsigned int us))
{
    hardware->delay_us = delay_us;
}

void pcmdi_hardware_specify_pin_reader_function(struct PCMDIHardware *hardware, uint8_t (*pin_reader)(uint8_t pin))
{
    hardware->pin_reader = pin_reader;
}

void pcmdi_hardware_specify_pin_writer_function(struct PCMDIHardware *hardware, void (*pin_writer)(uint8_t pin, uint8_t value))
{
    hardware->pin_writer = pin_writer;
}

void pcmdi_hardware_specify_pin_direction_writer_function(struct PCMDIHardware *hardware, void (*pin_direction_writer)(uint8_t pin, uint8_t direction))
{
    hardware->pin_direction_writer = pin_direction_writer;
}

enum PCMDIPinState pcmdi_hardware_get_pin_state(struct PCMDIHardware *hardware, uint8_t pin, uint8_t *previous_state)
{
    enum PCMDIPinState state = ((*previous_state & 1) << 1) | (hardware->pin_reader(pin) & 1);

    if (state == PCMDI_PIN_STATE_EDGE_RISING)
        *previous_state = PCMDI_PIN_LEVEL_HIGH;
    
    else if (state == PCMDI_PIN_STATE_EDGE_FALLING)
        *previous_state = PCMDI_PIN_LEVEL_LOW;
    
    return state;
}

enum PCMDIPinState pcmdi_hardware_get_clk_pin_state(struct PCMDIHardware *hardware)
{
    static uint8_t previous_state = PCMDI_PIN_LEVEL_LOW;

    return pcmdi_hardware_get_pin_state(hardware, hardware->pins.clk, &previous_state);
}

enum PCMDIPinState pcmdi_hardware_get_tse_pin_state(struct PCMDIHardware *hardware)
{
    static uint8_t previous_state = PCMDI_PIN_LEVEL_LOW;

    return pcmdi_hardware_get_pin_state(hardware, hardware->pins.tse, &previous_state);
}

