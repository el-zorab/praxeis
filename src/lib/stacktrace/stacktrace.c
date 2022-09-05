#include <stddef.h>
#include <stdint.h>
#include "lib/printf/printf.h"
#include "lib/stacktrace/stacktrace.h"
#include "symtab/symtab.h"

void print_stacktrace(void) {
    uint64_t *rbp = 0;
    __asm__ volatile("movq %%rbp, %0" : "=rm" (rbp));

    uint64_t i = 0;
    while (1) {
        uint64_t function_address = *(rbp + 1);
        const char *function_name = get_symtab_function_name(function_address);
        printf("    0x%016llx ", function_address);
        if (function_name == NULL) {
            printf("(unknown function)\n");
            break;
        }
        printf("%s\n", function_name);
        rbp = (uint64_t*) (uintptr_t) *rbp;
        i++;
    }
}
