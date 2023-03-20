#ifndef PCMDI_INTERNAL_H
#define PCMDI_INTERNAL_H

#include <stdint.h>
#include "pcmdi.h"

#ifdef __cplusplus
extern "C"{
#endif 

#define PCMDI_PIN_DELAY_TIME 150
#define PCMDI_ADDRESS_BUFFER_LENGTH 8
#define PCMDI_ADDRESS_BUFFER_INDEX 0

/**
 * @enum PCMDIPinState
 * @brief Defines the states a pin can be in
 */
enum PCMDIPinState {
    PCMDI_PIN_STATE_RISING,
    PCMDI_PIN_STATE_HIGH,
    PCMDI_PIN_STATE_FALLING,
    PCMDI_PIN_STATE_LOW
};

/**
* @brief Sets a bit at a given position to a particular value in a byte-sized variable 
* @param data Pointer the byte-sized variable
* @param bit_position The position of the bit we wish to modify
* @param value The value of the bit we wish to modify
*/
extern void pcmdi_set_bit(uint8_t *data, uint8_t bit_position, uint8_t value);

/**
* @brief Reads a bit from the DAT pin and writes it to the receive buffer 
* @param pcmdi Pointer to a PCMDI structure
*/
extern void pcmdi_read_incoming_bit_into_buffer(struct PCMDI *pcmdi);

/**
* @brief Gets the state of the CLK pin
* @param pcmdi Pointer to a PCMDI structure
* @return The state of the CLK pin
*/
extern enum PCMDIPinState pcmdi_get_clk_pin_state(struct PCMDI *pcmdi);

/**
* @brief Gets the state of the TSE pin
* @param pcmdi Pointer to a PCMDI structure
* @return The state of the TSE pin
*/
extern enum PCMDIPinState pcmdi_get_tse_pin_state(struct PCMDI *pcmdi);

/**
* @brief Gets the state of a specified pin
* @param pcmdi Pointer to a PCMDI structure
* @param pin The pin from which we want to get the state
* @param previous_state A pointer to a static variable holding the previous state of the pin
* @return The state of a specified pin
*/
extern enum PCMDIPinState pcmdi_get_pin_state(struct PCMDI *pcmdi, uint8_t pin, uint8_t *previous_state);

/**
* @brief Checks if the received address matches the address of the current device
* @param pcmdi Pointer to a PCMDI structure
*/
extern void pcmdi_check_received_address_match(struct PCMDI *pcmdi);

/**
* @brief Updates the device mode based on the TSE pin
* @param pcmdi Pointer to a PCMDI structure
*/
extern void pcmdi_update_device_mode(struct PCMDI *pcmdi);

/**
* @brief Creates a pulse on the CLK pin which synchronises all of the devices on the bus
* @param pcmdi Pointer to a PCMDI structure
*/
extern void pcmdi_sync(struct PCMDI *pcmdi);

/**
* @brief Clears the receive buffer
* @param pcmdi Pointer to a PCMDI structure
*/
extern void pcmdi_clear_receive_buffer(struct PCMDI *pcmdi);

/**
* @brief Clears all of the flags used for receiving data in a slave device
* @param pcmdi Pointer to a PCMDI structure
*/
extern void pcmdi_clear_receive_flags(struct PCMDI *pcmdi);

#ifdef __cplusplus
}
#endif

#endif // PCMDI_INTERNAL_H
