#pragma once

#include <stdint.h>

#define GDT_OFFSET_KERNEL_CODE 0x08
#define GDT_OFFSET_KERNEL_DATA 0x10
#define GDT_OFFSET_USER_CODE   0x18
#define GDT_OFFSET_USER_DATA   0x20
#define GDT_OFFSET_TSS         0x28

typedef struct {
    uint32_t reserved0;
    uint64_t rsp[3];
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb;
} __attribute__((packed)) tss_t;

void gdt_init(void);
void gdt_load_tss(tss_t *tss);
void gdt_reload(void);
