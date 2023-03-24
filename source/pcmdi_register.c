#include "pcmdi_register.h"
#include <stdlib.h>
#include <string.h>

#define PCMDI_REGISTER_BANK_MAXIMUM_SIZE 32

void pcmdi_register_create(struct PCMDIRegister *pcmdi_register, uint8_t index_descriptor, uint16_t size, uint8_t *data)
{
    pcmdi_register->index_descriptor = index_descriptor;
    pcmdi_register->size = size;
    pcmdi_register->data = (uint8_t *) malloc(size * sizeof(uint8_t *));
    memcpy(pcmdi_register->data, data, size);
}

void pcmdi_register_bank_create(struct PCMDIRegisterBank *bank)
{
    bank->size = 0;
    bank->registers = (struct PCMDIRegister *) malloc(sizeof(struct PCMDIRegister *));
}

void pcmdi_register_bank_insert_registers(struct PCMDIRegisterBank *bank, struct PCMDIRegister *reg, uint8_t number_of_registers)
{
    uint8_t new_size = bank->size + number_of_registers;

    if (new_size >= PCMDI_REGISTER_BANK_MAXIMUM_SIZE)
        return;

    bank->registers = (struct PCMDIRegister *) realloc(bank->registers, new_size * sizeof(struct PCMDIRegister *));

    for (uint8_t i = 0; i < number_of_registers; i++)
        bank->registers[i + bank->size] = reg[i];

    bank->size = new_size;
}

void pcmdi_register_bank_get_register_value(struct PCMDIRegisterBank *bank, uint8_t index_descriptor, void *out_value)
{
    for (uint8_t i = 0; i < bank->size; i++)
    {
        if (bank->registers[i].index_descriptor != index_descriptor)
            continue;
    
        memcpy(out_value, bank->registers[i].data, bank->registers[i].size);
        return;
    }
}
