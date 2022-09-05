#include "lib/panic/panic.h"
#include "lib/printf/printf.h"
#include "lib/spinlock/spinlock.h"
#include "x86/gdt/gdt.h"
#include "x86/interrupts/idt.h"

typedef struct {
    uint16_t offset_0;
    uint16_t gdt_code_selector;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset_1;
    uint32_t offset_2;
    uint32_t reserved;
} __attribute__((packed)) idt_descriptor_t;

typedef struct {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) idtr_t;

uint8_t halt_vector;

__attribute__((aligned(0x10)))
static idt_descriptor_t idt[256];
static idtr_t idtr;

extern void halt_handler(void);
extern uint64_t isr_table[];

uint8_t idt_allocate_vector(void) {
    static spinlock_t lock = SPINLOCK_INIT;
    static uint8_t allocatable_vector = 32;

    spinlock_acquire(&lock);

    if (allocatable_vector == 0xf0) {
        panic("No more free IDT vectors", true);
    }

    uint8_t return_vector = allocatable_vector++;

    spinlock_release(&lock);

    return return_vector;
}

void idt_set_handler(uint8_t vector, uintptr_t handler) {
    idt[vector].offset_0 = (uint16_t) handler;
    idt[vector].gdt_code_selector = GDT_OFFSET_KERNEL_CODE;
    idt[vector].ist = 0;
    idt[vector].attributes = 0x8e;
    idt[vector].offset_1 = (uint16_t) (handler >> 16);
    idt[vector].offset_2 = (uint32_t) (handler >> 32);
    idt[vector].reserved = 0;
}

void idt_init(void) {
    idtr.offset = (uint64_t) &idt[0];
    idtr.size = sizeof(idt_descriptor_t) * 256 - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_handler(vector, isr_table[vector]);
    }

    idt_reload();

    halt_vector = idt_allocate_vector();
    idt_set_handler(halt_vector, (uintptr_t) halt_handler);

    printf("IDT initialized\n");
}

void idt_reload(void) {
    __asm__ volatile("lidt %0" : : "m" (idtr));
}
