#pragma once

#include <stdbool.h>
#include "x86/cpu.h"

void panic(const char *message, bool do_stacktrace, ...);
void panic_init(void);
