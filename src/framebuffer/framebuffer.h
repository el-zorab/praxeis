#pragma once

#include <limine.h>
#include <stdarg.h>
#include <stdint.h>

#define FRAMEBUFFER_COLOR_BASE  0xadbeef
#define FRAMEBUFFER_COLOR_ALT   0x7b95e0
#define FRAMEBUFFER_COLOR_WARN  0xd9c936
#define FRAMEBUFFER_COLOR_PANIC 0xd41c1c

void fb_draw_image(uint64_t x, uint64_t y);
uint16_t fb_get_height(void);
uint16_t fb_get_width(void);
void fb_init(struct limine_framebuffer *framebuffer, uint64_t left_margin, uint64_t right_margin, uint64_t top_margin, uint64_t bottom_margin);
void fb_prepare_color(uint32_t color);
void fb_reset_color(void);
