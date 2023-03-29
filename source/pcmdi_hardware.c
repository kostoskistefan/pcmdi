#include "pcmdi_hardware.h"

enum PCMDIPinState pcmdi_hardware_get_pin_state(struct PCMDIHardware *hardware, uint8_t pin, uint8_t *previous_state)
{
    enum PCMDIPinState state = (enum PCMDIPinState) ((*previous_state & 1) << 1) | (hardware->pin_reader(pin) & 1);

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

