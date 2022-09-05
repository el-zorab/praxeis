#pragma once

#include <stdint.h>

typedef struct {
    uint64_t size;
    uint8_t *map;
} __attribute__((packed)) bitmap_t;

uint8_t bitmap_get_bit(bitmap_t *bitmap, uint64_t bit);
void bitmap_set_bit(bitmap_t *bitmap, uint64_t bit);
void bitmap_unset_bit(bitmap_t *bitmap, uint64_t bit);
