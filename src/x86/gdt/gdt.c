#include <stdint.h>
#include "lib/printf/printf.h"
#include "lib/spinlock/spinlock.h"
#include "x86/gdt/gdt.h"

#define GDT_RW             0x02
#define GDT_EXECUTABLE     0x08
#define GDT_CODE_OR_DATA   0x10
#define GDT_DPL_USER       0x60
#define GDT_PRESENT        0x80

#define GDT_64_BIT_CODE 0x20
#define GDT_32_BIT      0x40
#define GDT_GRANULARITY 0x80

typedef struct {
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access;
    uint8_t flags_and_limit1;
    uint8_t base2;
} __attribute__((packed)) gdt_descriptor_t;

typedef struct {
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access;
    uint8_t flags_and_limit1;
    uint8_t base2;
    uint32_t base3;
    uint32_t reserved;
} __attribute__((packed)) tss_descriptor_t;

typedef struct {
    gdt_descriptor_t descriptors[5];
    tss_descriptor_t tss_descriptor;
} gdt_t;

typedef struct {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) gdtr_t;

static gdt_t gdt;
static gdtr_t gdtr;

static uint16_t gdt_counter = 0;

void gdt_add_descriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt.descriptors[gdt_counter].limit0 = limit & 0xffff;
    gdt.descriptors[gdt_counter].base0 = base & 0xffff;
    gdt.descriptors[gdt_counter].base1 = (base >> 16) & 0xff;
    gdt.descriptors[gdt_counter].access = access;
    gdt.descriptors[gdt_counter].flags_and_limit1 = flags | ((limit >> 16) & 0xf);
    gdt.descriptors[gdt_counter].base2 = (base >> 24) & 0xff;

    gdt_counter++;
}

void gdt_init(void) {
    gdtr.offset = (uint64_t) &gdt;
    gdtr.size = sizeof(gdt) - 1;

    // null (0x00)
    gdt_add_descriptor(0, 0, 0, 0);

    // 64-bit kernel code (0x8)
    gdt_add_descriptor(0, 0         , GDT_PRESENT | GDT_CODE_OR_DATA | GDT_EXECUTABLE | GDT_RW               , GDT_GRANULARITY | GDT_64_BIT_CODE);
    
    // 64-bit kernel data (0x10)
    gdt_add_descriptor(0, 0         , GDT_PRESENT | GDT_CODE_OR_DATA | GDT_RW                                , GDT_GRANULARITY);

    // 64-bit user code (0x18)
    gdt_add_descriptor(0, 0         , GDT_PRESENT | GDT_DPL_USER | GDT_CODE_OR_DATA | GDT_EXECUTABLE | GDT_RW, GDT_GRANULARITY | GDT_64_BIT_CODE);
    
    // 64-bit user data (0x20)
    gdt_add_descriptor(0, 0         , GDT_PRESENT | GDT_DPL_USER | GDT_CODE_OR_DATA | GDT_RW                 , GDT_GRANULARITY);

    // tss descriptor (0x28)
    gdt.tss_descriptor.limit0 = sizeof(tss_t);
    gdt.tss_descriptor.base0 = 0;
    gdt.tss_descriptor.base1 = 0;
    gdt.tss_descriptor.access = 0x89;
    gdt.tss_descriptor.flags_and_limit1 = 0;
    gdt.tss_descriptor.base2 = 0;
    gdt.tss_descriptor.base3 = 0;
    gdt.tss_descriptor.reserved = 0;

    gdt_reload();

    printf("GDT initialized\n");
}

void gdt_reload(void) {
    __asm__ volatile(
        "lgdt %0\n\t"
        "push $0x8\n\t"
        "lea 1f(%%rip), %%rax\n\t"
        "push %%rax\n\t"
        "lretq\n\t"
        "1:\n\t"
        "mov $0x10, %%eax\n\t"
        "mov %%eax, %%ds\n\t"
        "mov %%eax, %%es\n\t"
        "mov %%eax, %%fs\n\t"
        "mov %%eax, %%gs\n\t"
        "mov %%eax, %%ss\n\t"
        : : "m" (gdtr) : "rax", "memory"
    );
}

void gdt_load_tss(tss_t *tss) {
    uintptr_t address = (uintptr_t) tss;

    static spinlock_t lock = SPINLOCK_INIT;
    spinlock_acquire(&lock);

    gdt.tss_descriptor.base0 = (uint16_t) address;
    gdt.tss_descriptor.base1 = (uint8_t) (address >> 16);
    gdt.tss_descriptor.access = 0x89;
    gdt.tss_descriptor.flags_and_limit1 = 0;
    gdt.tss_descriptor.base2 = (uint8_t) (address >> 24);
    gdt.tss_descriptor.base3 = (uint32_t) (address >> 32);
    gdt.tss_descriptor.reserved = 0;
    
    __asm__ volatile("ltr %0" : : "rm" ((uint16_t) GDT_OFFSET_TSS) : "memory");

    spinlock_release(&lock);
}
