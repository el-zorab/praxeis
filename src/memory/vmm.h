#pragma once

#include <limine.h>
#include <stdint.h>

void vmm_init(struct limine_memmap_response *memmap, struct limine_kernel_address_response *kernel_address, struct limine_5_level_paging_response *five_level_paging);
void vmm_load_kernel_pagemap(void);
