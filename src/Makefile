KERNEL         := kernel.elf
GEN_SYM_SCRIPT := gen_symtab.sh
SYMTAB_FILE        := symtab/symtab.c
SYMTAB_OBJ         := symtab/symtab.o
SYMTAB_PLACEHOLDER := symtab/symtab-placeholder

CC := x86_64-elf-gcc
AS := nasm
LD := x86_64-elf-ld

CFLAGS  := -Wall -Wextra -pedantic -O2 -pipe -g
ASFLAGS := -f elf64 -g -F dwarf
LDFLAGS := -g

INTERNALLDFLAGS :=         \
	-Tlinker.ld            \
	-nostdlib              \
	-zmax-page-size=0x1000 \
	-static

INTERNALCFLAGS :=        \
	-I.                  \
	-std=c11             \
	-ffreestanding       \
	-fno-stack-protector \
	-fno-stack-check     \
	-fno-pic             \
	-mabi=sysv           \
	-mno-80387           \
	-mno-mmx             \
	-mno-3dnow           \
	-mno-sse             \
	-mno-sse2            \
	-mno-red-zone        \
	-mcmodel=kernel      \
	-MMD

C_SRC       := $(shell find . -type f -name '*.c')
ASM_SRC     := $(shell find . -type f -name '*.asm')
OBJ         := $(C_SRC:.c=.o)
OBJ         += $(ASM_SRC:.asm=_asm.o)
HEADER_DEPS := $(C_SRC:.c=.d)

.PHONY: all
all:
	$(MAKE) clean
	$(MAKE) $(KERNEL)

# Build kernel with hacky workaround for symbol table
$(KERNEL): $(OBJ)
	if ! [ -f "$(SYMTAB_FILE)" ]; \
	then \
		cp $(SYMTAB_PLACEHOLDER) $(SYMTAB_FILE); \
		$(MAKE) compile-symtab; \
		$(LD) $(OBJ) $(SYMTAB_OBJ) $(LDFLAGS) $(INTERNALLDFLAGS) -o $@; \
	fi;
	./$(GEN_SYM_SCRIPT) $(KERNEL) $(SYMTAB_FILE)
	$(MAKE) compile-symtab
	$(LD) $(OBJ) $(SYMTAB_OBJ) $(LDFLAGS) $(INTERNALLDFLAGS) -o $@

.PHONY: compile-symtab
compile-symtab:
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $(SYMTAB_FILE) -o $(SYMTAB_OBJ)

-include $(HEADER_DEPS)
%.o: %.c
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $< -o $@

%_asm.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

.PHONY: limine
limine:
	curl https://raw.githubusercontent.com/limine-bootloader/limine/v3.0-branch/limine.h -o limine.h

.PHONY: clean
clean:
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS) $(SYMTAB_FILE)

.PHONY: distclean
distclean: clean
	rm -f limine.h
