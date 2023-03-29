#include "pcmdi_register.h"
#include <stdlib.h>
#include <string.h>

struct PCMDIRegister *pcmdi_register_bank_find_register(struct PCMDIRegisterBank *bank, uint8_t index_descriptor)
{
    for (uint8_t i = 0; i < bank->size; ++i)
    {
        if (bank->registers[i].index_descriptor != index_descriptor)
            continue;
    
        return &bank->registers[i];
    }

    return NULL;
}

void pcmdi_register_bank_get_register_value(struct PCMDIRegisterBank *bank, uint8_t index_descriptor, void *out_value)
{
    struct PCMDIRegister *found_register = pcmdi_register_bank_find_register(bank, index_descriptor);

    if (!found_register)
        return;

    memcpy(out_value, found_register->data_pointer, found_register->size);
}
