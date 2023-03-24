#ifndef PCMDI_H
#define PCMDI_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>
#include "pcmdi_header.h"
#include "pcmdi_hardware.h"

struct PCMDIMaster
{
    uint8_t maximum_number_of_slave_devices;
    struct PCMDIHardware hardware;
};

extern void     pcmdi_master_begin(struct PCMDIMaster *master_device, struct PCMDIHardware *hardware);
extern uint8_t  pcmdi_master_discover(struct PCMDIMaster *master_device, uint8_t *out_available_device_container);
extern uint8_t  pcmdi_master_request_bank_size(struct PCMDIMaster *master_device, uint8_t address);
extern uint16_t pcmdi_master_request_register_size(struct PCMDIMaster *master_device, uint8_t address, uint8_t rid);
extern void     pcmdi_master_read_register(struct PCMDIMaster *master_device, uint8_t address, uint8_t rid, uint16_t size, uint8_t *out_data);
extern void     pcmdi_master_write_register(struct PCMDIMaster *master_device, uint8_t address, uint8_t rid, uint16_t size, uint8_t *data);

extern void     pcmdi_master_set_maximum_number_of_slave_devices(struct PCMDIMaster *master_device, uint8_t maximum_number_of_slave_devices); 

#ifdef __cplusplus
}
#endif 

#endif // PCMDI_H
