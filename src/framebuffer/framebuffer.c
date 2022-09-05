#include <stdbool.h>
#include "lib/spinlock/spinlock.h"
#include "framebuffer/font.h"
#include "framebuffer/framebuffer.h"
#include "framebuffer/image.h"
#include "lib/panic/panic.h"
#include "lib/printf/printf.h"

static struct limine_framebuffer *fb;
static uint64_t fb_address;
static uint32_t fb_color = FRAMEBUFFER_COLOR_BASE;

static uint64_t x, x_min, x_max;
static uint64_t y, y_min, y_max;

static spinlock_t lock = SPINLOCK_INIT;

void fb_draw_image(uint64_t x, uint64_t y) {
    uint32_t *address = (uint32_t*) (uintptr_t) (fb_address + (x << 2) + y * fb->pitch);
    uint64_t newline = (fb->pitch >> 2) - IMAGE_HEIGHT;

    for (uint64_t j = 0; j < IMAGE_HEIGHT; j++) {
        for (uint64_t i = 0; i < IMAGE_WIDTH; i++) {
            *address++ = image[j * IMAGE_HEIGHT + i];
        }
        address += newline;
    }
}

uint16_t fb_get_height(void) {
    return fb->height;
}

uint16_t fb_get_width(void) {
    return fb->width;
}

void fb_init(struct limine_framebuffer *framebuffer, uint64_t left_margin, uint64_t right_margin, uint64_t top_margin, uint64_t bottom_margin) {
    static uint8_t FRAMEBUFFER_INIT = false;
    if (FRAMEBUFFER_INIT) {
        fb_prepare_color(FRAMEBUFFER_COLOR_WARN);
        printf("Framebuffer: fb_init() called multiple times\n");
        fb_reset_color();
        return;
    }
    
    fb = framebuffer;
    fb_address = (uint64_t) (uintptr_t) fb->address;

    x = x_min = left_margin;
    y = y_min = top_margin;
    x_max = fb->width - FONT_WIDTH - right_margin;
    y_max = fb->height - FONT_HEIGHT - bottom_margin;

    
    if (fb->red_mask_shift != 16 || fb->green_mask_shift != 8 || fb->blue_mask_shift != 0 || fb->red_mask_size != 8 || fb->green_mask_size != 8 || fb->blue_mask_size != 8) {
        panic("Unsupported framebuffer (RSH = %u, GSH = %u, BSH = %u, RMS = %u, GMS = %u, BMS = %u)", true,
            fb->red_mask_shift, fb->green_mask_shift, fb->blue_mask_shift, fb->red_mask_size, fb->green_mask_size, fb->blue_mask_size);
    }
    
    FRAMEBUFFER_INIT = true;

    printf("Framebuffer initialized (width = %llu, height = %llu, bpp = %llu, pitch = %llu)\n", fb->width, fb->height, fb->bpp, fb->pitch);
}

void fb_prepare_color(uint32_t color) {
    spinlock_acquire(&lock);
    fb_color = color;
}

void fb_reset_color(void) {
    fb_color = FRAMEBUFFER_COLOR_BASE;
    spinlock_release(&lock);
}

void _putchar(char character) {
    if (character == '\n') {
        x = x_min;
        y += FONT_HEIGHT;
    } else {
        uint32_t *address = (uint32_t*) (uintptr_t) (fb_address + (x << 2) + y * fb->pitch);
        uint64_t newline = (fb->pitch >> 2) - FONT_WIDTH;
        uint8_t char_row_byte, mask;

        for (uint16_t j = 0; j < FONT_HEIGHT; j++) {
            char_row_byte = font[character * FONT_HEIGHT + j];
            mask = 1 << (FONT_WIDTH - 1);
            for (uint16_t i = 0; i < FONT_WIDTH; i++) {
                if (char_row_byte & mask) *address = fb_color;
                address++;
                mask >>= 1;
            }
            address += newline;
        }

        x += FONT_WIDTH;

        if (x > x_max) {
            x = x_min;
            y += FONT_HEIGHT;
        }
    }
}
