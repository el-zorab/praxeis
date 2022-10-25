#!/bin/bash

printf "#include <stddef.h>\n#include \"symtab/symtab.h\"\n\n" > $2
printf "struct symbol {\n    uint64_t address;\n    const char* name;\n};\n\n" >> $2
printf "__attribute__((section(\".symbol_table\")))\n" >> $2
printf "static const struct symbol symtab[] = {\n" >> $2
nm $1 | grep " T " | grep -v "text_start_address" | awk '{ print "    { .address = 0x"$1", .name = \""$3"\" }," }' | sort >> $2
printf "    { .address = UINT64_MAX, .name = \"\" }\n};\n\n" >> $2
printf "const char* get_symtab_function_name(uint64_t address) {\n" >> $2
printf "    for (uint64_t i = 0; symtab[i].address < UINT64_MAX; i++) {\n" >> $2
printf "        if (symtab[i].address < address && symtab[i + 1].address >= address) {\n" >> $2
printf "            return symtab[i].name;\n        }\n    }\n    return NULL;\n}\n" >> $2
