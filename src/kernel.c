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
#include "x86/cpu/cpu-wrappers.h"
#include "x86/gdt/gdt.h"
#include "x86/interrupts/idt.h"
#include "x86/smp/smp.h"

__attribute__((section(".limine_reqs"))) 
uintptr_t requests[] = {
    (uintptr_t) &((struct limine_5_level_paging_request)  { .id = LIMINE_5_LEVEL_PAGING_REQUEST,  .revision = 0 }),
    (uintptr_t) &((struct limine_bootloader_info_request) { .id = LIMINE_BOOTLOADER_INFO_REQUEST, .revision = 0 }),
    (uintptr_t) &((struct limine_framebuffer_request)     { .id = LIMINE_FRAMEBUFFER_REQUEST,     .revision = 0 }),
    (uintptr_t) &((struct limine_hhdm_request)            { .id = LIMINE_HHDM_REQUEST,            .revision = 0 }),
    (uintptr_t) &((struct limine_kernel_address_request)  { .id = LIMINE_KERNEL_ADDRESS_REQUEST,  .revision = 0 }),
    (uintptr_t) &((struct limine_memmap_request)          { .id = LIMINE_MEMMAP_REQUEST,          .revision = 0 }),
    (uintptr_t) &((struct limine_rsdp_request)            { .id = LIMINE_RSDP_REQUEST,            .revision = 0 }),
    (uintptr_t) &((struct limine_smp_request)             { .id = LIMINE_SMP_REQUEST,             .revision = 0 }),
    (uintptr_t) NULL
};

uint64_t hhdm_offset;

void kmain(void) {
    struct limine_5_level_paging_response  *five_level_paging_response = ((struct limine_5_level_paging_request*)  requests[0])->response;
    struct limine_bootloader_info_response *bootloader_info_response   = ((struct limine_bootloader_info_request*) requests[1])->response;
    struct limine_framebuffer_response     *framebuffer_response       = ((struct limine_framebuffer_request*)     requests[2])->response;
    struct limine_hhdm_response            *hhdm_response              = ((struct limine_hhdm_request*)            requests[3])->response;
    struct limine_kernel_address_response  *kernel_address_response    = ((struct limine_kernel_address_request*)  requests[4])->response;
    struct limine_memmap_response          *memmap_response            = ((struct limine_memmap_request*)          requests[5])->response;
    struct limine_rsdp_response            *rsdp_response              = ((struct limine_rsdp_request*)            requests[6])->response;
    struct limine_smp_response             *smp_response               = ((struct limine_smp_request*)             requests[7])->response;

    hhdm_offset = hhdm_response->offset;

    if (framebuffer_response == NULL || framebuffer_response->framebuffer_count < 1) panic("No framebuffer available", false);
    if (framebuffer_response->framebuffers[0]->bpp != 32) panic("Framebuffer bpp does not equal 32", false);

    fb_init(framebuffer_response->framebuffers[0], 32, 32, 32, 32);
    fb_draw_image(fb_get_width() - IMAGE_WIDTH, 0);
    
    printf("\nPraxeis booted by %s v%s\n\n", bootloader_info_response->name, bootloader_info_response->version);

    uint32_t eax, ebx, ecx, edx;
    cpuid(0x80000001, 0, &eax, &ebx, &ecx, &edx);
    if (!(edx & (1 << 20))) panic("NX bit not available", false);

    cpuid(0, 0, &eax, &ebx, &ecx, &edx);
    printf("CPU Vendor String = %.4s%.4s%.4s\n", (const char*) &ebx, (const char*) &edx, (const char*) &ecx);

    gdt_init();
    idt_init();
    pmm_init(memmap_response);
    vmm_init(memmap_response, kernel_address_response, five_level_paging_response);
    acpi_init(rsdp_response);
    madt_init();
    hpet_init();
    smp_init(smp_response);
    pmm_reclaim_bootloader_memory(memmap_response);

    panic("Reached end of kernel entry point", true);
}
