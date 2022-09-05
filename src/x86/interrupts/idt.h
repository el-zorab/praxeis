#pragma once

#include <stdint.h>

uint8_t idt_allocate_vector(void);
void idt_init(void);
void idt_reload(void);
void idt_set_handler(uint8_t vector, uintptr_t handler);
