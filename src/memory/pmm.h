#pragma once

#include <limine.h>
#include <stdbool.h>
#include <stdint.h>

#define PAGE_SIZE 4096

void pmm_init(struct limine_memmap_response *memmap);
void *pmm_alloc(uint64_t pages_count, bool sanitize);
void pmm_free(void *pointer, uint64_t pages_count);
