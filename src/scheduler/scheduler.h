#pragma once

#include <stdint.h>
#include "x86/cpu/cpu-wrappers.h"

struct thread;

static inline struct thread *sched_current_thread(void) {
    return (struct thread*) (uintptr_t) get_gs_base();
}
