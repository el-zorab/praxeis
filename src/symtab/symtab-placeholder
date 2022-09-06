#include <stddef.h>
#include "symtab/symtab.h"

typedef struct {
    uint64_t address;
    const char* name;
} symbol_t;

__attribute__((section(".symbol_table")))
static const symbol_t symtab[] = {
    { .address = UINT64_MAX, .name = "" }
};

const char* get_symtab_function_name(uint64_t address) {
    for (uint64_t i = 0; symtab[i].address < UINT64_MAX; i++) {
        if (symtab[i].address < address && symtab[i + 1].address >= address) {
            return symtab[i].name;
        }
    }
    return NULL;
}
