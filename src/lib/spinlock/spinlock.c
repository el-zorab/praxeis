#include "lib/spinlock/spinlock.h"

void spinlock_acquire(spinlock_t *lock) {
    uint32_t ticket = __atomic_fetch_add(&lock->next_ticket, 1, __ATOMIC_RELAXED);
    while (__atomic_load_n(&lock->serving_ticket, __ATOMIC_ACQUIRE) != ticket) {
        __asm__ volatile("pause");
    }
}

void spinlock_release(spinlock_t *lock) {
    uint32_t current = __atomic_load_n(&lock->serving_ticket, __ATOMIC_RELAXED);
    __atomic_store_n(&lock->serving_ticket, current + 1, __ATOMIC_RELEASE);
}