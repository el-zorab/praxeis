#pragma once

#include <stdint.h>

#define SPINLOCK_INIT {0, 0}

typedef struct {
    uint32_t serving_ticket;
    uint32_t next_ticket;
} spinlock_t;

void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);