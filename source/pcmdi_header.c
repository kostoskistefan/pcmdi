#include "pcmdi_header.h"

void pcmdi_header_set_address(uint16_t *header, uint8_t address)
{
    *header = (*header & 0x00ff) | (uint16_t) (address << 0x08); 
}

void pcmdi_header_set_transmission_mode(uint16_t *header, enum PCMDITransmissionMode mode)
{
    *header = (*header & 0xff7f) | (mode << 0x07);
}

void pcmdi_header_set_ird(uint16_t *header, enum PCMDIInternalRegisterDescriptor ird)
{
    *header = (*header & 0xff9f) | (ird << 0x05);
}

void pcmdi_header_set_rid(uint16_t *header, uint8_t rid)
{
    *header = (*header & 0xffe0) | (rid & 0x1f);
}

uint8_t pcmdi_header_get_address(uint16_t *header)
{
    return (*header >> 0x08);
}

enum PCMDITransmissionMode pcmdi_header_get_transmission_mode(uint16_t *header)
{
    return (enum PCMDITransmissionMode) (*header >> 7) & 0x01;
}

enum PCMDIInternalRegisterDescriptor pcmdi_header_get_ird(uint16_t *header)
{
    return (enum PCMDIInternalRegisterDescriptor) (*header >> 5) & 0x03;
}

uint8_t pcmdi_header_get_rid(uint16_t *header)
{
    return *header & 0x001f;
}