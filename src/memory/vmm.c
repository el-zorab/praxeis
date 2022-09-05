#include <stddef.h>
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "lib/printf/printf.h"
#include "util/math.h"
#include "x86/cpu.h"

#define PTE_PRESENT  0x1
#define PTE_WRITABLE 0x2

static uint64_t *kernel_pagemap;
static uint8_t la57_enabled;

extern uint64_t hhdm_offset;

extern char elf_end_address[];

static uint64_t *vmm_get_next_pml(uintptr_t *pml, uint64_t pml_index, uint16_t flags) {
    if (!(pml[pml_index] & 0x1)) {
        pml[pml_index] = (uint64_t) ((uintptr_t) pmm_alloc(1, true)) | flags;
    }
    return (uint64_t*) ((pml[pml_index] & ~(0x1ff)) + hhdm_offset);
}

void vmm_map_page(uint64_t *pagemap, uintptr_t virtual_address, uintptr_t physical_address, uint16_t flags) {
    uint64_t *pml4;
    if (la57_enabled) {
        uint64_t pml5_index = (virtual_address & ((uint64_t) 0x1ff << 48)) >> 48;
        uint64_t *pml5 = pagemap;
        pml4 = vmm_get_next_pml(pml5, pml5_index, flags);
    } else {
        pml4 = pagemap;
    }

    uint64_t pml4_index = (virtual_address & ((uint64_t) 0x1ff << 39)) >> 39;
    uint64_t pml3_index = (virtual_address & ((uint64_t) 0x1ff << 30)) >> 30;
    uint64_t pml2_index = (virtual_address & ((uint64_t) 0x1ff << 21)) >> 21;
    uint64_t pml1_index = (virtual_address & ((uint64_t) 0x1ff << 12)) >> 12;

    uint64_t *pml3 = vmm_get_next_pml(pml4, pml4_index, flags);
    uint64_t *pml2 = vmm_get_next_pml(pml3, pml3_index, flags);
    uint64_t *pml1 = vmm_get_next_pml(pml2, pml2_index, flags);

    pml1[pml1_index] = physical_address | flags;

    invlpg((void*) virtual_address);
}

void vmm_load_kernel_pagemap(void) {
    __asm__ volatile("mov %0, %%cr3" : : "r" ((uintptr_t) kernel_pagemap - hhdm_offset) : "memory");
}

void vmm_init(struct limine_memmap_response *memmap, struct limine_kernel_address_response *kernel_address, struct limine_5_level_paging_response *five_level_paging) {
    la57_enabled = five_level_paging != 0;
    printf("VMM: 5-level paging %s\n", la57_enabled ? "supported" : "not supported");

    printf("VMM: Kernel physical address = 0x%016llx\n", kernel_address->physical_base);
    printf("VMM: Kernel virtual address  = 0x%016llx\n", kernel_address->virtual_base);

    kernel_pagemap = (uint64_t*) ((uintptr_t) pmm_alloc(1, true) + hhdm_offset);

    for (uint64_t i = 0x1000; i < 0x100000000; i += PAGE_SIZE) {
        vmm_map_page(kernel_pagemap, i + hhdm_offset, i, PTE_PRESENT | PTE_WRITABLE);
    }

    printf("VMM: Higher half mapping done\n");

    uint64_t elf_size = align_up((uint64_t) elf_end_address, PAGE_SIZE) - kernel_address->virtual_base;
    for (uint64_t i = 0; i < elf_size; i += PAGE_SIZE) {
        vmm_map_page(kernel_pagemap, kernel_address->virtual_base + i, kernel_address->physical_base + i, PTE_PRESENT | PTE_WRITABLE);
    }

    printf("VMM: Kernel mapping done\n");

    struct limine_memmap_entry *current_entry;
    uint64_t base, top;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        current_entry = memmap->entries[i];

        base = align_down(current_entry->base, PAGE_SIZE);
        top = align_up(current_entry->base + current_entry->length, PAGE_SIZE);

        if (top <= 0x100000000) {
            continue;
        }
        
        if (base < 0x100000000) {
            base = 0x100000000;
        }

        for (uint64_t j = base; j < top; j += PAGE_SIZE) {
            vmm_map_page(kernel_pagemap, j, j, PTE_PRESENT | PTE_WRITABLE);
            vmm_map_page(kernel_pagemap, j + hhdm_offset, j, PTE_PRESENT | PTE_WRITABLE);
        }
    }
    
    printf("VMM: Memory map mapping done\n");

    vmm_load_kernel_pagemap();

    printf("VMM initialized\n");
}
