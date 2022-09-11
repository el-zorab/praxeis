#pragma once

#include <stdint.h>

static inline uint64_t div_round_up(uint64_t a, uint64_t b) {
    return (a + (b - 1)) / b;
}

static inline uint64_t align_down(uint64_t value, uint64_t alignment) {
    return (value / alignment) * alignment;
}

static inline uint64_t align_up(uint64_t value, uint64_t alignment) {
    return div_round_up(value, alignment) * alignment;
}
