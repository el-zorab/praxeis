#include <stddef.h>
#include "acpi/acpi.h"
#include "acpi/madt.h"
#include "framebuffer/framebuffer.h"
#include "lib/panic/panic.h"
#include "lib/printf/printf.h"
#include "x86/smp/smp.h"

#define MADT_TYPE_LAPIC  0
#define MADT_TYPE_ISO    2
#define MADT_TYPE_NMI    4
#define MADT_TYPE_IOAPIC 16

#define IOAPIC_MAX_RECORDS 16

typedef struct {
    sdt_header_t header;
    uint32_t lapic_base;
    uint32_t flags;
    uint8_t records[];
} __attribute__((packed)) madt_t;

static madt_t *madt;

madt_record_lapic *lapic_records[SMP_MAX_CORES];
uint64_t lapic_counter;

madt_record_ioapic *ioapic_records[IOAPIC_MAX_RECORDS];
uint64_t ioapic_counter;

uint64_t madt_get_lapic_base(void) {
    return madt->lapic_base;
}

void madt_init(void) {
    madt = (madt_t*) acpi_find_sdt("APIC");

    if (madt == NULL) panic("No MADT found", true);

    uint64_t size = madt->header.length - sizeof(madt_t);
    madt_record_header *record_header;
    for (uint64_t i = 0; i < size; i += record_header->length) {
        record_header = (madt_record_header*) (uintptr_t) (madt->records + i);
        switch (record_header->type) {
            case MADT_TYPE_LAPIC:
                if (lapic_counter > SMP_MAX_CORES) {
                    panic("System has more LAPICs than the maximum (%llu)", true, SMP_MAX_CORES);
                }
                lapic_records[lapic_counter++] = (madt_record_lapic*) record_header;
                break;
            case MADT_TYPE_IOAPIC:
                if (ioapic_counter > IOAPIC_MAX_RECORDS) {
                    panic("System has more IOAPICs than the maximum (%llu)", true, IOAPIC_MAX_RECORDS);
                }
                ioapic_records[ioapic_counter++] = (madt_record_ioapic*) record_header;
                break;
        }
    }

    printf("MADT initialized\n");
}
