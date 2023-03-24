#include "pcmdi_header.h"

void pcmdi_header_set_address(uint16_t *header, uint8_t address)
{
    *header = (*header & 0xff) | (address << 8); 
}

void pcmdi_header_set_transmission_mode(uint16_t *header, enum PCMDITransmissionMode mode)
{
    *header = (*header & 0xff7f) | (mode << 7);
}

void pcmdi_header_set_ird(uint16_t *header, enum PCMDIInternalRegisterDescriptor ird)
{
    *header = (*header & 0xff9f) | (ird << 5);
}

void pcmdi_header_set_rid(uint16_t *header, uint8_t rid)
{
    *header = (*header & 0xffe0) | (rid & 0x1f);
}

uint8_t pcmdi_header_get_address(uint16_t *header)
{
    return (*header >> 8);
}

enum PCMDITransmissionMode pcmdi_header_get_transmission_mode(uint16_t *header)
{
    return (enum PCMDITransmissionMode) (*header >> 7) & 1;
}

enum PCMDIInternalRegisterDescriptor pcmdi_header_get_ird(uint16_t *header)
{
    return (enum PCMDIInternalRegisterDescriptor) (*header >> 5) & 3;
}

uint8_t pcmdi_header_get_rid(uint16_t *header)
{
    return *header & 0x1f;
}

