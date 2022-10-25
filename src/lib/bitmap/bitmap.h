#pragma once

#include <stdint.h>

struct __attribute__((packed)) bitmap {
    uint64_t size;
    uint8_t *map;
};

uint8_t bitmap_get_bit(struct bitmap *bitmap, uint64_t bit);
void bitmap_set_bit(struct bitmap *bitmap, uint64_t bit);
void bitmap_unset_bit(struct bitmap *bitmap, uint64_t bit);
