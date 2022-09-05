#pragma once

#include <stdbool.h>

bool hpet_available(void);
void hpet_init(void);
void hpet_msleep(uint64_t ms);
void hpet_usleep(uint64_t us);
void hpet_nsleep(uint64_t ns);
