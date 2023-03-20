#ifndef PCMDI_H
#define PCMDI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif 

#define PCMDI_BUFFER_SIZE 16

/**
 * @enum PCMDIDeviceMode
 * @brief Defines the modes a single device can be in
 */
enum PCMDIDeviceMode {
    PCMDI_DEVICE_MODE_IDLE,
    PCMDI_DEVICE_MODE_READING_ADDRESS,
    PCMDI_DEVICE_MODE_READING_DATA
};

/**
 * @struct PCMDI
 * @brief Physically Connected Multi Device Interface
 */
struct PCMDI
{
    /**
    * @struct PCMDIBuffers
    * @brief Contains buffers needed for communication
    */
    struct PCMDIBuffers
    {
        uint8_t receive_buffer[PCMDI_BUFFER_SIZE];
    } buffers;

    /**
    * @struct PCMDIFlags
    * @brief Contains flags which are set during communication
    */
    struct PCMDIFlags 
    {
        uint8_t receive_buffer_data_ready;
        uint8_t receive_buffer_was_written;
        uint8_t received_address_matches_this_device;
    } flags;
    
    /**
    * @struct PCMDIHardware
    * @brief Contains hardware specific definitions
    */
    struct PCMDIHardware
    {
        /**
        * @struct PCMDIPins
        * @brief Contains hardware specific pin definitions
        */
        struct PCMDIPins
        {
            /** @brief Clock signal pin */
            uint8_t clk;
            
            /** @brief Transmission start/end signal pin */
            uint8_t tse;
            
            /** @brief Data signal pin */
            uint8_t dat;
        } pins;

        /**
        * @brief A function that delays the code for a given amount of time
        * @param us Amount of time in microseconds
        */
        void (*delay_us)(unsigned int us);

        /**
        * @brief A function that reads the state of the clk/tse/dat pins
        * @param pin The pin to read
        * @return Value of the read pin
        */
        int (*pin_reader)(uint8_t pin);
        
        /**
        * @brief A function that writes a state to the clk/tse/dat pins
        * @param pin The pin to write to
        * @param value The value to write. 0 means off, 1 means on.
        */
        void (*pin_writer)(uint8_t pin, uint8_t value);
    } hardware;

    uint8_t device_address;
    enum PCMDIDeviceMode device_mode;
};

/**
* @brief Initializes a PCMDI structure
* @param pcmdi Pointer to a PCMDI structure
* @param clk The number of the clk pin
* @param tse The number of the tse pin
* @param dat The number of the dat pin
*/
extern void pcmdi_begin(struct PCMDI *pcmdi, uint8_t clk, uint8_t tse, uint8_t dat);

/**
* @brief Initiates the PCMDI start condition and sends the address to the bus
* @param pcmdi Pointer to a PCMDI structure
* @param address The address of a slave device to which we wish to send data
*/
extern void pcmdi_start_transmission(struct PCMDI *pcmdi, uint8_t address);

/**
* @brief Initiates the PCMDI end condition
* @param pcmdi Pointer to a PCMDI structure
*/
extern void pcmdi_end_transmission(struct PCMDI *pcmdi);

/**
* @brief Checks if data is successfully received and ready to be read
* @param pcmdi Pointer to a PCMDI structure
*/
extern uint8_t pcmdi_data_available(struct PCMDI *pcmdi);

/**
* @brief Reads data in the receive buffer sent by the master
* @param pcmdi Pointer to a PCMDI structure
* @param out_data An array in which the data will be read
* @param data_length The length of the data which needs to be read from the buffer
*/
extern void pcmdi_read_request(struct PCMDI *pcmdi, uint8_t *out_data, uint8_t data_length);

/**
* @brief Writes data to the bus
* @param pcmdi Pointer to a PCMDI structure
* @param data An array filled with the data which needs to be sent to the bus
* @param data_length The length of the data which needs to be sent to the bus
*/
extern void pcmdi_write(struct PCMDI *pcmdi, uint8_t *data, uint8_t data_length);

/**
* @brief Writes a single byte to the bus
* @param pcmdi Pointer to a PCMDI structure
* @param data A byte-sized data which needs to be sent to the bus
*/
extern void pcmdi_write_byte(struct PCMDI *pcmdi, uint8_t data);

/**
* @brief Checks multiple pin conditions and updates the buffers accordingly
* @param pcmdi Pointer to a PCMDI structure
* @note Call this function in the main loop as fast as possible
*/
extern void pcmdi_run(struct PCMDI *pcmdi);

/**
* @brief Sets the address of the current device. Default 0x00 (MASTER)
* @param pcmdi Pointer to a PCMDI structure
* @param device_address The address you wish to give to this device
*/
extern void pcmdi_set_device_address(struct PCMDI *pcmdi, uint8_t device_address);

/**
* @brief Registers a delay function which will be used while communication
* @param pcmdi Pointer to a PCMDI structure
* @param delay_us The function which generates a microsecond
*/
extern void pcmdi_register_delay_us(struct PCMDI *pcmdi, void (*delay_us)(unsigned int us));

/**
* @brief Registers a function that reads the state of the CLK/TSE/DAT pins
* @param pcmdi Pointer to a PCMDI structure
* @param pin_reader The function which reads a pin and returns its state as an integer value
*/
extern void pcmdi_register_pin_reader(struct PCMDI *pcmdi, int (*pin_reader)(uint8_t pin));

/**
* @brief Registers a function that writes the state of the CLK/TSE/DAT pins
* @param pcmdi Pointer to a PCMDI structure
* @param pin_reader The function which writes a value to a pin
*/
extern void pcmdi_register_pin_writer(struct PCMDI *pcmdi, void (*pin_writer)(uint8_t pin, uint8_t value));

#ifdef __cplusplus
}
#endif

#endif // PCMDI_H
