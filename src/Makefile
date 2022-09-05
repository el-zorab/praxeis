KERNEL := kernel.elf
GEN_SYM_SCRIPT := gen_symtab.sh
SYMBOL_TABLE := symtab/symtab.c

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
all: $(KERNEL)

$(KERNEL): $(OBJ)
	$(LD) $(OBJ) $(LDFLAGS) $(INTERNALLDFLAGS) -o $@
	$(MAKE) gen-sym

.PHONY: gen-sym
gen-sym:
	./$(GEN_SYM_SCRIPT) $(KERNEL) $(SYMBOL_TABLE)

%_asm.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

-include $(HEADER_DEPS)
%.o: %.c
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $< -o $@

.PHONY: limine
limine:
	curl https://raw.githubusercontent.com/limine-bootloader/limine/v3.0-branch/limine.h -o limine.h

.PHONY: clean
clean:
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS)

.PHONY: distclean
distclean:
	rm -f limine.h
