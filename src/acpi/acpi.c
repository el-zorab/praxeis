#include <stddef.h>
#include "acpi/acpi.h"
#include "framebuffer/framebuffer.h"
#include "lib/misc/memutil.h"
#include "lib/printf/printf.h"

struct __attribute__((packed)) rsdp {
    uint8_t signature[8];
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
};

struct __attribute__((packed)) sdt {
    struct sdt_header header;
    uint8_t data[];
};

static struct sdt *rsdt;
static struct sdt *xsdt;

static uint8_t xsdt_available;

extern uint64_t hhdm_offset;

static uint8_t acpi_do_checksum(struct sdt_header *header) {
    uint16_t checksum = 0;
    uint8_t *pointer = (uint8_t*) header;
    for (uint32_t i = 0; i < header->length; i++) {
        checksum += pointer[i];
    }
    return (checksum & 0xff) == 0;
}

struct sdt_header *acpi_find_sdt(const char *signature) {
    uint64_t length = ((xsdt_available ? xsdt->header.length : rsdt->header.length) - sizeof(struct sdt)) / (xsdt_available ? 8 : 4);
    for (uint64_t i = 0; i < length; i++) {
        struct sdt_header *header;

        if (xsdt_available) {
            header = (struct sdt_header*) (((uint64_t*) (uintptr_t) xsdt->data)[i] + hhdm_offset);
        } else {
            header = (struct sdt_header*) (((uint32_t*) (uintptr_t) rsdt->data)[i] + hhdm_offset);
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
    struct rsdp *rsdp = (struct rsdp*) rsdp_response->address;

    xsdt_available = rsdp->revision > 1 && rsdp->xsdt_address != 0;

    printf("ACPI: XSDT is %s\n", xsdt_available ? "supported" : "not supported");

    if (xsdt_available) {
        xsdt = (struct sdt*) (uintptr_t) (rsdp->xsdt_address + hhdm_offset);
    } else {
        rsdt = (struct sdt*) (uintptr_t) (rsdp->rsdt_address + hhdm_offset);
    }

    printf("ACPI initialized\n");
}
