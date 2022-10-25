#pragma once

#include <stdint.h>
#include "acpi/acpi.h"

struct __attribute__((packed)) madt_record_header {
    uint8_t type;
    uint8_t length;
};

struct __attribute__((packed)) madt_record_lapic {
    struct madt_record_header header;
    uint8_t cpu_id;
    uint8_t apic_id;
    uint32_t flags;
};

struct __attribute__((packed)) madt_record_ioapic {
    struct madt_record_header header;
    uint32_t id;
    uint8_t reserved;
    uint32_t address;
    uint32_t gsi_base;
} ;

// struct __attribute__((packed)) madt_record_iso {
//     struct madt_record_header header;
//     uint8_t bus_source;
//     uint8_t irq_source;
//     uint32_t gsi;
//     uint16_t flags;
// };

// struct __attribute__((packed)) madt_record_nmi {
//     struct madt_record_header header;
//     uint8_t cpu_id;
//     uint16_t flags;
//     uint8_t lint;
// };

uint64_t madt_get_lapic_base(void);
void madt_init(void);
