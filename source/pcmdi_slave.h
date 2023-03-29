#ifndef PCMDI_SLAVE_H
#define PCMDI_SLAVE_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>
#include "pcmdi_register.h"
#include "pcmdi_hardware.h"

enum PCMDISlaveInternalMode
{
    PCMDI_SLAVE_INTERNAL_MODE_IDLE,
    PCMDI_SLAVE_INTERNAL_MODE_CLEANUP,
    PCMDI_SLAVE_INTERNAL_MODE_WAIT_FOR_HEADER,
    PCMDI_SLAVE_INTERNAL_MODE_HANDLE_REQUEST,
};

struct PCMDISlave
{
    uint8_t device_address;
    uint16_t received_header;
    enum PCMDISlaveInternalMode internal_mode;
    struct PCMDIRegisterBank *bank;
    struct PCMDIHardware *hardware;
};

// Public Functions

extern void pcmdi_slave_begin(struct PCMDISlave *device, struct PCMDIHardware *hardware, struct PCMDIRegisterBank *bank, uint8_t device_address);
extern void pcmdi_slave_listen(struct PCMDISlave *device);

// Private Functions

extern void    pcmdi_slave_update_internal_mode(struct PCMDISlave *device);
extern uint8_t pcmdi_slave_read_byte(struct PCMDISlave *device);
extern void    pcmdi_slave_write_byte(struct PCMDISlave *device, uint8_t data);
extern void    pcmdi_slave_read_header(struct PCMDISlave *device);
extern void    pcmdi_slave_handle_request(struct PCMDISlave *device);
extern void    pcmdi_slave_respond_to_discover_request(struct PCMDISlave *device);
extern void    pcmdi_slave_respond_to_bank_size_request(struct PCMDISlave *device);
extern void    pcmdi_slave_respond_to_register_size_request(struct PCMDISlave *device);
extern void    pcmdi_slave_respond_to_register_data_request(struct PCMDISlave *device);


#ifdef __cplusplus
}
#endif 

#endif // PCMDI_SLAVE_H
