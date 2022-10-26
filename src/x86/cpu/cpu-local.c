#include "scheduler/thread.h"
#include "x86/cpu/cpu-local.h"

struct cpu_local *get_cpu_local(void) {
    return sched_current_thread()->cpu_local;
}
