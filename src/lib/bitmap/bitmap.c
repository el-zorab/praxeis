#include "lib/bitmap/bitmap.h"

uint8_t bitmap_get_bit(bitmap_t *bitmap, uint64_t bit) {
    return (bitmap->map[bit / 8] >> (bit & 7)) & 1;
}

void bitmap_set_bit(bitmap_t *bitmap, uint64_t bit) {
    bitmap->map[bit / 8] |= 1 << (bit & 7);
}

void bitmap_unset_bit(bitmap_t *bitmap, uint64_t bit) {
    bitmap->map[bit / 8] &= ~(1 << (bit & 7));
}
