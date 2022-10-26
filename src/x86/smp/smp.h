#pragma once

#include <limine.h>
#include <stdint.h>

#define CPU_STACK_SIZE 0x10000
#define SMP_MAX_CORES UINT16_MAX

void smp_init(struct limine_smp_response *smp);
bool smp_is_init(void);
