#pragma once

#include <stdint.h>
#include "x86/smp/smp.h"

#define LAPIC_IPI_DEST_EXCL_SELF (0x3 << 18)

void lapic_eoi(void);
void lapic_init(void);
void lapic_send_ipi(uint8_t lapic_id, uint8_t vector, uint32_t args);
void lapic_timer_calibrate(void);
void lapic_timer_oneshot(uint64_t us, uint8_t vector);
