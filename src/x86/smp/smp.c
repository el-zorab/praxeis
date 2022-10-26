#include "framebuffer/framebuffer.h"
#include "lib/misc/math.h"
#include "lib/misc/memutil.h"
#include "lib/panic/panic.h"
#include "lib/printf/printf.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "scheduler/thread.h"
#include "x86/apic/lapic.h"
#include "x86/cpu/cpu-local.h"
#include "x86/cpu/cpu-wrappers.h"
#include "x86/interrupts/idt.h"

static bool is_smp_init = false;
static uint16_t started_cpus_counter;

extern uint64_t hhdm_offset;

static void cpu_init(struct limine_smp_info *cpu) {
    struct cpu_local *cpu_local = (struct cpu_local*) (uintptr_t) cpu->extra_argument;

    gdt_reload();
    idt_reload();

    gdt_load_tss(&cpu_local->tss);

    vmm_load_kernel_pagemap();

    uint64_t *stack = (uint64_t*) ((uintptr_t) pmm_alloc(align_up(CPU_STACK_SIZE, PAGE_SIZE) / PAGE_SIZE, true) + CPU_STACK_SIZE + hhdm_offset);
    cpu_local->tss.rsp[0] = (uint64_t) stack;

    struct thread *idle_thread = (struct thread*) ((uintptr_t) pmm_alloc(align_up(sizeof(struct thread), PAGE_SIZE) / PAGE_SIZE, true) + hhdm_offset);
    cpu_local->idle_thread = idle_thread;
    idle_thread->cpu_local = cpu_local;
    
    set_gs_base((uint64_t) idle_thread);

    lapic_init();
    __asm__ volatile("sti");
    lapic_timer_calibrate();

    printf("SMP: Initialized CPU %u\n", cpu_local->id);

    started_cpus_counter++;

    if (!cpu_local->is_bsp) {
        while (1) {
            __asm__ volatile("hlt");
        }
    }
}

void smp_init(struct limine_smp_response *smp) {
    for (uint64_t i = 0; i < smp->cpu_count; i++) {
        struct limine_smp_info *current_cpu = smp->cpus[i];

        struct cpu_local *cpu_local = (struct cpu_local*) ((uintptr_t) pmm_alloc(align_up(sizeof(struct cpu_local), PAGE_SIZE) / PAGE_SIZE, true) + hhdm_offset);
        current_cpu->extra_argument = (uint64_t) cpu_local;
        cpu_local->id = i;
        cpu_local->lapic_id = current_cpu->lapic_id;

        if (current_cpu->lapic_id == smp->bsp_lapic_id) {
            cpu_local->is_bsp = true;
            cpu_init(current_cpu);
        } else {
            current_cpu->goto_address = cpu_init;
        }
    }

    while (started_cpus_counter != smp->cpu_count) {
        __asm__ volatile("pause");
    }

    is_smp_init = true;
}

bool smp_is_init(void) {
    return is_smp_init;
}
