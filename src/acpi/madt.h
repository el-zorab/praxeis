#pragma once

#include <stdint.h>
#include "acpi/acpi.h"

typedef struct {
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) madt_record_header;

typedef struct {
    madt_record_header header;
    uint8_t cpu_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed)) madt_record_lapic;

typedef struct {
    madt_record_header header;
    uint32_t id;
    uint8_t reserved;
    uint32_t address;
    uint32_t gsi_base;
} __attribute__((packed)) madt_record_ioapic;

// typedef struct {
//     madt_record_header header;
//     uint8_t bus_source;
//     uint8_t irq_source;
//     uint32_t gsi;
//     uint16_t flags;
// } __attribute__((packed)) madt_record_iso;

// typedef struct {
//     madt_record_header header;
//     uint8_t cpu_id;
//     uint16_t flags;
//     uint8_t lint;
// } __attribute__((packed)) madt_record_nmi;

uint64_t madt_get_lapic_base(void);
void madt_init(void);
