#include "lib/spinlock/spinlock.h"
#include "scheduler/thread.h"
#include "scheduler/scheduler.h"

static struct thread threads[MAX_THREADS];

static struct spinlock lock = SPINLOCK_INIT;
