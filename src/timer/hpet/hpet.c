#include <stddef.h>
#include <stdint.h>
#include "acpi/acpi.h"
#include "lib/panic/panic.h"
#include "lib/printf/printf.h"
#include "timer/hpet/hpet.h"

struct __attribute__((packed)) hpet {
    struct sdt_header header;
    uint8_t hardware_rev_id;
    uint8_t info;
    uint16_t pci_vendor_id;
    uint8_t address_space_id;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t address;
    uint8_t hpet_number;
    uint16_t minimum_tick;
    uint8_t page_protection;
};

struct __attribute__((packed)) hpet_contents {
    uint64_t general_capabilities;
    uint64_t reserved0;
    uint64_t general_configuration;
    uint64_t reserved1;
    uint64_t general_interrupt_status;
    uint64_t reserved2[25];
    volatile uint64_t main_counter_value;
    uint64_t reserved3;
};

static volatile struct hpet *hpet;
static volatile struct hpet_contents *hpet_contents;
static uint64_t hpet_period;
static bool is_hpet_available = false;

extern uint64_t hhdm_offset;

bool hpet_available(void) {
    return is_hpet_available;
}

void hpet_init(void) {
    hpet = (struct hpet*) acpi_find_sdt("HPET");
    if (hpet == NULL) {
        panic("No HPET found", true);
    }

    is_hpet_available = true;

    hpet_contents = (struct hpet_contents*) (uintptr_t) (hpet->address + hhdm_offset);
    hpet_contents->main_counter_value = 0;
    hpet_contents->general_configuration = 1;

    hpet_period = hpet_contents->general_capabilities >> 32;

    printf("HPET initialized\n");
}

void hpet_msleep(uint64_t ms) {
    hpet_nsleep(ms * 1000000);
}

void hpet_usleep(uint64_t us) {
    hpet_nsleep(us * 1000);
}

void hpet_nsleep(uint64_t ns) {
    uint64_t counter_target = hpet_contents->main_counter_value + (ns * 1000000) / hpet_period;
    while (hpet_contents->main_counter_value < counter_target);
}
