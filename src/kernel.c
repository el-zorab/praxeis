#include <limine.h>
#include <stddef.h>
#include <stdint.h>
#include "acpi/acpi.h"
#include "acpi/madt.h"
#include "framebuffer/framebuffer.h"
#include "framebuffer/image.h"
#include "lib/panic/panic.h"
#include "lib/printf/printf.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "timer/hpet/hpet.h"
#include "x86/apic/lapic.h"
#include "x86/cpu.h"
#include "x86/gdt/gdt.h"
#include "x86/interrupts/idt.h"
#include "x86/smp/smp.h"

static volatile struct limine_5_level_paging_request five_level_paging_request = {
    .id = LIMINE_5_LEVEL_PAGING_REQUEST,
    .revision = 0
};

static volatile struct limine_bootloader_info_request bootloader_info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST,
    .revision = 0
};

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};

uint64_t hhdm_offset;

void kmain(void) {
    struct limine_bootloader_info_response *bootloader_info = bootloader_info_request.response;
    struct limine_framebuffer_response *framebuffer_response = framebuffer_request.response;

    hhdm_offset = hhdm_request.response->offset;

    if (framebuffer_response == NULL || framebuffer_response->framebuffer_count < 1) panic("No framebuffer available", false);
    if (framebuffer_response->framebuffers[0]->bpp != 32) panic("Framebuffer bpp does not equal 32", false);

    fb_init(framebuffer_response->framebuffers[0], 32, 32, 32, 32);
    fb_draw_image(fb_get_width() - IMAGE_WIDTH, 0);
    
    printf("\nPraxeis booted by %s v%s\n\n", bootloader_info->name, bootloader_info->version);

    uint32_t eax, ebx, ecx, edx;
    cpuid(0, 0, &eax, &ebx, &ecx, &edx);

    printf("CPU Vendor String = %.4s%.4s%.4s\n", (const char*) &ebx, (const char*) &edx, (const char*) &ecx);

    gdt_init();
    idt_init();
    pmm_init(memmap_request.response);
    vmm_init(memmap_request.response, kernel_address_request.response, five_level_paging_request.response);
    acpi_init(rsdp_request.response);
    madt_init();
    hpet_init();
    smp_init(smp_request.response);

    panic("Reached end of kernel entry point", true);
}
