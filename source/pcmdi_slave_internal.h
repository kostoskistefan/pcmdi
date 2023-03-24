#ifndef PCMDI_SLAVE_INTERNAL_H
#define PCMDI_SLAVE_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>

extern void pcmdi_slave_update_internal_mode(struct PCMDISlave *slave_device);
extern void pcmdi_slave_read_header(struct PCMDISlave *slave_device);
extern void pcmdi_slave_handle_request(struct PCMDISlave *slave_device);
extern void pcmdi_slave_respond_to_discover_request(struct PCMDISlave *slave_device);

#ifdef __cplusplus
}
#endif 

#endif // PCMDI_SLAVE_INTERNAL_H
