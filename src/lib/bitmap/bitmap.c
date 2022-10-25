#include "lib/bitmap/bitmap.h"

uint8_t bitmap_get_bit(struct bitmap *bitmap, uint64_t bit) {
    return (bitmap->map[bit / 8] >> (bit & 7)) & 1;
}

void bitmap_set_bit(struct bitmap *bitmap, uint64_t bit) {
    bitmap->map[bit / 8] |= 1 << (bit & 7);
}

void bitmap_unset_bit(struct bitmap *bitmap, uint64_t bit) {
    bitmap->map[bit / 8] &= ~(1 << (bit & 7));
}
