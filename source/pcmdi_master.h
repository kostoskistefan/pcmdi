#ifndef PCMDI_H
#define PCMDI_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>
#include "pcmdi_hardware.h"

struct PCMDIMaster
{
    uint8_t maximum_number_of_slave_devices;
    struct PCMDIHardware *hardware;
    void (*delay_us)(uint32_t us);
};

// Public Functions

extern void     pcmdi_master_begin(struct PCMDIMaster *device, struct PCMDIHardware *hardware, void (*delay_us)(uint32_t us));
extern uint8_t  pcmdi_master_discover(struct PCMDIMaster *device, uint8_t *out_available_master_container);
extern uint8_t  pcmdi_master_request_bank_size(struct PCMDIMaster *device, uint8_t address);
extern uint16_t pcmdi_master_request_register_size(struct PCMDIMaster *device, uint8_t address, uint8_t rid);
extern void     pcmdi_master_read_register(struct PCMDIMaster *device, uint8_t address, uint8_t rid, uint16_t size, void *out_data);
extern void     pcmdi_master_write_register(struct PCMDIMaster *device, uint8_t address, uint8_t rid, uint16_t size, void *data);
extern void     pcmdi_master_set_maximum_number_of_slave_devices(struct PCMDIMaster *device, uint8_t maximum_number_of_slave_devices); 

// Private Functions

extern void     pcmdi_master_start_transmission(struct PCMDIMaster *device, uint16_t *header);
extern void     pcmdi_master_end_transmission(struct PCMDIMaster *device);
extern uint8_t  pcmdi_master_read_byte(struct PCMDIMaster *device);
extern void     pcmdi_master_write_byte(struct PCMDIMaster *device, uint8_t data);
extern void     pcmdi_master_sync(struct PCMDIMaster *device);

#ifdef __cplusplus
}
#endif 

#endif // PCMDI_H
