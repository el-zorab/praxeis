#pragma once

#include <limine.h>
#include <stdint.h>

typedef struct {
    uint8_t signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) sdt_header_t;

sdt_header_t *acpi_find_sdt(const char *signature);
void acpi_init(struct limine_rsdp_response *rsdp_response);
