#include <stddef.h>
#include "acpi/acpi.h"
#include "framebuffer/framebuffer.h"
#include "lib/printf/printf.h"
#include "util/memutil.h"

typedef struct {
    uint8_t signature[8];
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed)) rsdp_t;

typedef struct {
    sdt_header_t header;
    uint8_t data[];
} __attribute__((packed)) sdt;

static sdt *rsdt;
static sdt *xsdt;

static uint8_t xsdt_available;

extern uint64_t hhdm_offset;

static uint8_t acpi_do_checksum(sdt_header_t *header) {
    uint16_t checksum = 0;
    uint8_t *pointer = (uint8_t*) header;
    for (uint32_t i = 0; i < header->length; i++) {
        checksum += pointer[i];
    }
    return (checksum & 0xff) == 0;
}

sdt_header_t *acpi_find_sdt(const char *signature) {
    uint64_t length = ((xsdt_available ? xsdt->header.length : rsdt->header.length) - sizeof(sdt)) / (xsdt_available ? 8 : 4);
    for (uint64_t i = 0; i < length; i++) {
        sdt_header_t *header;

        if (xsdt_available) {
            header = (sdt_header_t*) (((uint64_t*) (uintptr_t) xsdt->data)[i] + hhdm_offset);
        } else {
            header = (sdt_header_t*) (((uint32_t*) (uintptr_t) rsdt->data)[i] + hhdm_offset);
        }

        if (memcmp((const char*) header->signature, signature, 4) == 0) {
            if (acpi_do_checksum(header)) {
                printf("ACPI: Checksum verified for signature %s\n", signature);
                return header;
            } else {
                fb_prepare_color(FRAMEBUFFER_COLOR_WARN);
                printf("ACPI: Checksum is not valid for signature %s\n", signature);
                fb_reset_color();
                return NULL;
            }
        }
    }

    fb_prepare_color(FRAMEBUFFER_COLOR_WARN);
    printf("ACPI: No SDT matching signature %s was found\n", signature);
    fb_reset_color();
    return NULL;
}

void acpi_init(struct limine_rsdp_response *rsdp_response) {
    rsdp_t *rsdp = (rsdp_t*) rsdp_response->address;

    xsdt_available = rsdp->revision > 1 && rsdp->xsdt_address != 0;

    printf("ACPI: XSDT is %s\n", xsdt_available ? "supported" : "not supported");

    if (xsdt_available) {
        xsdt = (sdt*) (uintptr_t) (rsdp->xsdt_address + hhdm_offset);
    } else {
        rsdt = (sdt*) (uintptr_t) (rsdp->rsdt_address + hhdm_offset);
    }

    printf("ACPI initialized\n");
}
