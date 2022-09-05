#include <stddef.h>
#include <stdint.h>
#include "framebuffer/framebuffer.h"
#include "lib/panic/panic.h"
#include "lib/printf/printf.h"
#include "lib/spinlock/spinlock.h"
#include "lib/stacktrace/stacktrace.h"
#include "timer/hpet/hpet.h"
#include "x86/interrupts/idt.h"
#include "x86/apic/lapic.h"
#include "x86/smp/smp.h"

extern uint8_t halt_vector;

void panic(const char *message, bool do_stacktrace, ...) {
    __asm__ volatile("cli");

    lapic_send_ipi(0, halt_vector, LAPIC_IPI_DEST_EXCL_SELF);
    hpet_msleep(10);

    fb_prepare_color(FRAMEBUFFER_COLOR_PANIC);
    if (get_cpu_local() == NULL) {
        printf("\nBSP CPU PANIC: ");
    } else {
        printf("\nCPU %llu PANIC: ", get_cpu_local()->id);
    }
    fb_reset_color();

    va_list args;
    va_start(args, do_stacktrace);
    vprintf(message, args);
    va_end(args);

    if (do_stacktrace) {
        printf("\nStacktrace:\n");
        print_stacktrace();
    }

    printf("\nAll CPUs halted. You can now shutdown your PC.");

    while (1) {
        __asm__ volatile("hlt");
    }
}
