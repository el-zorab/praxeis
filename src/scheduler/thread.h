#pragma once

#include <stdint.h>
#include "x86/cpu/cpu-local.h"

struct thread {
    uint16_t tid;
    struct cpu_local *cpu_local;
};
