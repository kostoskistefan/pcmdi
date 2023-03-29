#ifndef PCMDI_HEADER_H
#define PCMDI_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>

#define PCMDI_HEADER_DEFAULT_VALUE 0x0000

enum PCMDIInternalRegisterDescriptor
{
    PCMDI_IRD_DISCOVER_REQUEST = 0x00,
    PCMDI_IRD_BANK_SIZE_REQUEST = 0x01,
    PCMDI_IRD_REGISTER_SIZE_REQUEST = 0x02,
    PCMDI_IRD_REGISTER_DATA_REQUEST = 0x03
};

enum PCMDITransmissionMode
{
    PCMDI_TRANSMISSION_MODE_READ = 0x00,
    PCMDI_TRANSMISSION_MODE_WRITE = 0x01
};

extern void pcmdi_header_set_address(uint16_t *header, uint8_t address);
extern void pcmdi_header_set_transmission_mode(uint16_t *header, enum PCMDITransmissionMode mode);
extern void pcmdi_header_set_ird(uint16_t *header, enum PCMDIInternalRegisterDescriptor ird);
extern void pcmdi_header_set_rid(uint16_t *header, uint8_t rid);

extern uint8_t                              pcmdi_header_get_address(uint16_t *header);
extern enum PCMDITransmissionMode           pcmdi_header_get_transmission_mode(uint16_t *header);
extern enum PCMDIInternalRegisterDescriptor pcmdi_header_get_ird(uint16_t *header);
extern uint8_t                              pcmdi_header_get_rid(uint16_t *header);

#ifdef __cplusplus
}
#endif 

#endif // PCMDI_HEADER_H
