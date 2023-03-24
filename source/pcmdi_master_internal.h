#ifndef PCMDI_MASTER_INTERNAL_H
#define PCMDI_MASTER_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>

extern void     pcmdi_master_start_transmission(struct PCMDIMaster *master);
extern void     pcmdi_master_end_transmission(struct PCMDIMaster *master);
extern void     pcmdi_master_transmit_header(struct PCMDIMaster *master_device, uint16_t *header);
extern uint8_t  pcmdi_master_read_byte(struct PCMDIMaster *master);
extern void     pcmdi_master_write_byte(struct PCMDIMaster *master, uint8_t data);
extern void     pcmdi_master_sync(struct PCMDIMaster *master);

#ifdef __cplusplus
}
#endif 

#endif // PCMDI_MASTER_INTERNAL_H
