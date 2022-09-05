#pragma once

#include <stdint.h>

static inline uint8_t cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    uint32_t cpuid_max;
    __asm__ volatile("cpuid"
                        : "=a" (cpuid_max)
                        : "a" (leaf & (1 << 31))
                        : "ebx", "ecx", "edx");

    if (leaf > cpuid_max) {
        return 0;
    }

    __asm__ volatile("cpuid"
                        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
                        : "a" (leaf), "c" (subleaf));
    
    return 1;
}

static inline void invlpg(void *virtual_address) {
    __asm__ volatile("invlpg (%0)" : : "r" (virtual_address));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t r;
    __asm__ volatile("inb %1, %0" : "=a" (r) : "Nd" (port));
    return r;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a" (value), "Nd" (port));
}

static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t eax = 0, edx = 0;
    __asm__ volatile("rdmsr" : "=a" (eax), "=d" (edx) : "c" (msr) : "memory");
    return ((uint64_t) edx << 32) | eax;
}

static inline void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t eax = (uint32_t) value;
    uint32_t edx = (uint32_t) (value >> 32);
    __asm__ volatile("wrmsr" : : "a" (eax), "c" (msr), "d" (edx) : "memory");
}

static inline void *get_gs_base(void) {
    return (void*) rdmsr(0xc0000101);
}

static inline void set_gs_base(uint64_t value) {
    wrmsr(0xc0000101, value);
}
