#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "scheduler/scheduler.h"
#include "x86/gdt/gdt.h"

struct thread;

struct cpu_local {
    uint16_t id;
    bool is_bsp;
    uint32_t lapic_id;
    uint64_t lapic_freq;
    struct tss tss;
    struct thread *idle_thread;
};

struct cpu_local *get_cpu_local(void);
