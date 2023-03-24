#ifndef PCMDI_COMMON_H
#define PCMDI_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>

#define PCMDI_PIN_DELAY_TIME 20

#define PCMDI_PIN_DIRECTION_INPUT 1
#define PCMDI_PIN_DIRECTION_OUTPUT 0

#define PCMDI_PIN_LEVEL_LOW 0
#define PCMDI_PIN_LEVEL_HIGH 1

enum PCMDIPinState
{
    PCMDI_PIN_STATE_LEVEL_LOW = 0b00,
    PCMDI_PIN_STATE_EDGE_RISING = 0b01,
    PCMDI_PIN_STATE_EDGE_FALLING = 0b10,
    PCMDI_PIN_STATE_LEVEL_HIGH = 0b11
};

struct PCMDIHardware
{
    struct PCMDIPins
    {
        uint8_t clk;
        uint8_t tse;
        uint8_t dat;
    } pins;

    void    (*delay_us)(unsigned int us);
    uint8_t (*pin_reader)(uint8_t pin);
    void    (*pin_writer)(uint8_t pin, uint8_t value);
    void    (*pin_direction_writer)(uint8_t pin, uint8_t direction);
};

extern void pcmdi_hardware_specify_pins(struct PCMDIHardware *hardware, uint8_t clk, uint8_t tse, uint8_t dat);
extern void pcmdi_hardware_specify_delay_function(struct PCMDIHardware *hardware, void (*delay_us)(unsigned int us));
extern void pcmdi_hardware_specify_pin_reader_function(struct PCMDIHardware *hardware, uint8_t (*pin_reader)(uint8_t pin));
extern void pcmdi_hardware_specify_pin_writer_function(struct PCMDIHardware *hardware, void (*pin_writer)(uint8_t pin, uint8_t value));
extern void pcmdi_hardware_specify_pin_direction_writer_function(struct PCMDIHardware *hardware, void (*pin_direction_writer)(uint8_t pin, uint8_t direction));

extern enum PCMDIPinState pcmdi_hardware_get_pin_state(struct PCMDIHardware *hardware, uint8_t pin, uint8_t *previous_state);
extern enum PCMDIPinState pcmdi_hardware_get_clk_pin_state(struct PCMDIHardware *hardware);
extern enum PCMDIPinState pcmdi_hardware_get_tse_pin_state(struct PCMDIHardware *hardware);

#ifdef __cplusplus
}
#endif 

#endif // PCMDI_COMMON_H
