#ifndef PCMDI_SLAVE_H
#define PCMDI_SLAVE_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>
#include "pcmdi_header.h"
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
    struct PCMDIRegisterBank bank;
    struct PCMDIHardware hardware;
};

extern void pcmdi_slave_begin(struct PCMDISlave *slave_device, struct PCMDIHardware *hardware, uint8_t device_address);
extern void pcmdi_slave_listen(struct PCMDISlave *slave_device);
extern void pcmdi_slave_respond(struct PCMDISlave *slave_device);

#ifdef __cplusplus
}
#endif 

#endif // PCMDI_SLAVE_H
