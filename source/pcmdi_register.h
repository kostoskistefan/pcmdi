#ifndef PCMDI_REGISTER_H
#define PCMDI_REGISTER_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>

#define PCMDI_REGISTER_BANK_MAXIMUM_SIZE 32

struct PCMDIRegister
{
    uint8_t index_descriptor;
    uint16_t size;
    void *data_pointer;
};

struct PCMDIRegisterBank
{
    uint8_t size;
    struct PCMDIRegister *registers;
};

extern struct PCMDIRegister *pcmdi_register_bank_find_register(struct PCMDIRegisterBank *bank, uint8_t index_descriptor);
extern void pcmdi_register_bank_get_register_value(struct PCMDIRegisterBank *bank, uint8_t index_descriptor, void *out_value);

#ifdef __cplusplus
}
#endif 

#endif // PCMDI_REGISTER_H
