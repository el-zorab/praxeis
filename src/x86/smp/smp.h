#pragma once

#include <limine.h>
#include <stdbool.h>
#include <stdint.h>
#include "x86/gdt/gdt.h"

#define CPU_STACK_SIZE 0x10000
#define SMP_MAX_CORES UINT16_MAX

struct cpu_local {
    uint16_t id;
    bool is_bsp;
    uint32_t lapic_id;
    uint64_t lapic_freq;
    struct tss tss;
};

void smp_init(struct limine_smp_response *smp);
bool smp_is_init(void);
struct cpu_local *get_cpu_local(void);
