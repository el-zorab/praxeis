#pragma once

#include <stdint.h>

#define SPINLOCK_INIT {0, 0}

struct spinlock {
    uint32_t serving_ticket;
    uint32_t next_ticket;
};

void spinlock_acquire(struct spinlock *lock);
void spinlock_release(struct spinlock *lock);