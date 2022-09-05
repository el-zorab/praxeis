#include "acpi/madt.h"
#include "framebuffer/framebuffer.h"
#include "lib/printf/printf.h"
#include "timer/hpet/hpet.h"
#include "x86/apic/lapic.h"
#include "x86/smp/smp.h"

#define LAPIC_REG_ID 0x20
#define LAPIC_REG_EOI 0xb0
#define LAPIC_REG_SPURIOUS 0xf0
#define LAPIC_REG_ICR_LOW 0x300
#define LAPIC_REG_ICR_HIGH 0x310
#define LAPIC_REG_TIMER_LVT 0x320
#define LAPIC_REG_TIMER_INIT_COUNT 0x380
#define LAPIC_REG_TIMER_CUR_COUNT 0x390
#define LAPIC_REG_TIMER_DIV 0x3e0

#define LAPIC_SPURIOUS_INTERRUPT_VEC 0xff

static uint64_t lapic_base;

extern uint64_t hhdm_offset;

static inline uint32_t lapic_read(uint32_t reg) {
    return *((volatile uint32_t*) (lapic_base + reg + hhdm_offset));
}

static inline void lapic_write(uint32_t reg, uint32_t value) {
    *((volatile uint32_t*) (lapic_base + reg + hhdm_offset)) = value;
}

static void lapic_timer_stop() {
    lapic_write(LAPIC_REG_TIMER_INIT_COUNT, 0);
    lapic_write(LAPIC_REG_TIMER_LVT, 1 << 16);
}

void lapic_eoi(void) {
    lapic_write(LAPIC_REG_EOI, 0);
}

void lapic_send_ipi(uint8_t lapic_id, uint8_t vector, uint32_t args) {
    // write the high doubleword first because the IPI will be sent
    // when the low doubleword is written
    lapic_write(LAPIC_REG_ICR_HIGH, (uint32_t) lapic_id << 24);
    lapic_write(LAPIC_REG_ICR_LOW, (uint32_t) vector | args);
}

void lapic_timer_calibrate(void) {
    uint32_t initial_count = UINT32_MAX;

    lapic_timer_stop();
    lapic_write(LAPIC_REG_TIMER_LVT, (1 << 16) | LAPIC_SPURIOUS_INTERRUPT_VEC);
    lapic_write(LAPIC_REG_TIMER_DIV, 0);
    lapic_write(LAPIC_REG_TIMER_INIT_COUNT, initial_count);

    hpet_usleep(1000);
    uint32_t final_count = lapic_read(LAPIC_REG_TIMER_CUR_COUNT);

    lapic_timer_stop();

    get_cpu_local()->lapic_freq = (initial_count - final_count) * 1000;
}

void lapic_timer_oneshot(uint64_t us, uint8_t vector) {
    lapic_timer_stop();
    lapic_write(LAPIC_REG_TIMER_LVT, vector);
    lapic_write(LAPIC_REG_TIMER_DIV, 0);
    lapic_write(LAPIC_REG_TIMER_INIT_COUNT, us * (get_cpu_local()->lapic_freq / 1000000));
}

void lapic_init(void) {
    lapic_base = madt_get_lapic_base();
    if (lapic_base != 0xfee00000) {
        fb_prepare_color(FRAMEBUFFER_COLOR_WARN);
        printf("LAPIC: Base does not equal 0xfee00000 (0x%llx) on CPU %u\n", lapic_base, get_cpu_local()->id);
        fb_reset_color();
    }

    lapic_write(LAPIC_REG_SPURIOUS, (1 << 8) | LAPIC_SPURIOUS_INTERRUPT_VEC);
}
