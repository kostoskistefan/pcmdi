#ifndef PCMDI_REGISTER_H
#define PCMDI_REGISTER_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>

struct PCMDIRegister
{
    uint8_t index_descriptor;
    uint16_t size;
    uint8_t *data;
};

struct PCMDIRegisterBank
{
    uint8_t size;
    struct PCMDIRegister *registers;
};

extern void pcmdi_register_create(struct PCMDIRegister *pcmdi_register, uint8_t index_descriptor, uint16_t size, uint8_t *data);
extern void pcmdi_register_bank_create(struct PCMDIRegisterBank *bank);

extern void pcmdi_register_bank_get_register_value(struct PCMDIRegisterBank *bank, uint8_t index_descriptor, void *out_value);
extern void pcmdi_register_bank_insert_registers(struct PCMDIRegisterBank *bank, struct PCMDIRegister *reg, uint8_t number_of_registers);

#ifdef __cplusplus
}
#endif 

#endif // PCMDI_REGISTER_H
