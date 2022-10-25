#include <limine.h>
#include <stddef.h>
#include "lib/bitmap/bitmap.h"
#include "lib/misc/math.h"
#include "lib/misc/memutil.h"
#include "lib/panic/panic.h"
#include "lib/printf/printf.h"
#include "memory/pmm.h"

static struct bitmap bitmap;
static uint64_t highest_usable_page_index;
static uint64_t free_pages;
static uint64_t cached_page_index;

extern uint64_t hhdm_offset;

void pmm_init(struct limine_memmap_response *memmap) {
    uint64_t highest_usable_page = 0, memmap_entry_top = 0;

    uint64_t memory_usages[3] = {0, 0, 0};
    uint64_t total_pages = 0;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *current_entry = memmap->entries[i];

        switch (current_entry->type) {
            case LIMINE_MEMMAP_USABLE:
                memmap_entry_top = current_entry->base + current_entry->length;
                if (highest_usable_page < memmap_entry_top) {
                    highest_usable_page = memmap_entry_top;
                }
                break;
            case LIMINE_MEMMAP_RESERVED:
                memory_usages[0] += current_entry->length / PAGE_SIZE;
                break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                memory_usages[1] += current_entry->length / PAGE_SIZE;
                break;
            case LIMINE_MEMMAP_KERNEL_AND_MODULES:
                memory_usages[2] += current_entry->length / PAGE_SIZE;
                break;
        }

        total_pages += current_entry->length / PAGE_SIZE;
    }

    highest_usable_page_index = highest_usable_page / PAGE_SIZE;
    bitmap.size = align_up(highest_usable_page_index / 8, PAGE_SIZE);
    
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *current_entry = memmap->entries[i];

        if (current_entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }
        
        if (current_entry->length >= bitmap.size) {
            bitmap.map = (uint8_t*) (uintptr_t) (current_entry->base + hhdm_offset);

            current_entry->base += bitmap.size;
            current_entry->length -= bitmap.size;
            break;
        }
    }

    for (uint64_t i = 0; i < bitmap.size; i++) bitmap.map[i] = 0xff;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *current_entry = memmap->entries[i];
        if (current_entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        pmm_free((void*) current_entry->base, current_entry->length / PAGE_SIZE);
    }

    printf("Approximate physical memory usages out of total %lluMiB:\n  %lluMiB Usable  %lluMiB Reserved\n  %lluMiB Bl. Recl.  %lluMiB Kernel/Modules\n",
        total_pages / 256, free_pages / 256, memory_usages[0] / 256,  memory_usages[1] / 256, memory_usages[2] / 256);
    printf("PMM initialized\n");
}

static void *pmm_search_free_pages(uint64_t pages_count, uint64_t page_index_start, uint64_t page_index_end) {
    uint64_t pages_counter = 0;
    uint64_t page_index = page_index_start;
    while (page_index < page_index_end) {
        if (bitmap_get_bit(&bitmap, page_index)) {
            pages_counter = 0;
            page_index++;
        } else {
            pages_counter++;
            page_index++;
            if (pages_counter == pages_count) {
                uint64_t return_page_index = page_index - pages_count;
                for (uint64_t bit = return_page_index; bit < page_index; bit++) {
                    bitmap_set_bit(&bitmap, bit);
                }
                cached_page_index = page_index;
                return (void*) (return_page_index * PAGE_SIZE);
            }
        }
    }

    return NULL;
}

void *pmm_alloc(uint64_t pages_count, bool sanitize) {
    void *return_page = pmm_search_free_pages(pages_count, cached_page_index, highest_usable_page_index);

    if (return_page == NULL) {
        return_page = pmm_search_free_pages(pages_count, 0, cached_page_index);
        if (return_page == NULL) panic("Out of memory", true);
    }

    free_pages -= pages_count;

    if (sanitize) {
        uint64_t *page_virtual_addr = (uint64_t*) ((uintptr_t) return_page + hhdm_offset);
        for (uint64_t i = 0; i < pages_count * PAGE_SIZE / 8; i++) {
            page_virtual_addr[i] = 0;
        }
    }

    return return_page;
}

void pmm_free(void *pointer, uint64_t pages_count) {
    uint64_t page_index_start = (uintptr_t) pointer / PAGE_SIZE;
    uint64_t page_index_end = page_index_start + pages_count;
    for (uint64_t bit = page_index_start; bit < page_index_end; bit++) {
        bitmap_unset_bit(&bitmap, bit);
    }
    free_pages += pages_count;
}

void pmm_reclaim_bootloader_memory(struct limine_memmap_response *memmap) {
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *current_entry = memmap->entries[i];
        if (current_entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            pmm_free((void*) current_entry->base, current_entry->length / PAGE_SIZE);
        }
    }
}
