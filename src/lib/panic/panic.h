#pragma once

#include <stdbool.h>

void panic(const char *message, bool do_stacktrace, ...);
void panic_init(void);
