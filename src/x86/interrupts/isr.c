#include "lib/panic/panic.h"
#include "lib/printf/printf.h"

typedef struct {
    uint64_t cr4, cr3, cr2, cr0;
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t rbp;
    uint64_t vector, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
} cpu_state_t;

static char *exception_mnemonics[32] = {
    "DE", "DB", "XX", "BP", "OF", "BR", "UD", "NM",
    "DF", "XX", "TS", "NP", "SS", "GP", "PF", "XX",
    "MF", "AC", "MC", "XM", "VE", "CP", "XX", "XX",
    "XX", "XX", "XX", "XX", "HV", "VC", "SX", "XX"
};

static char *exception_names[32] = {
    "Divide-by-zero Error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",

    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

__attribute__((noreturn)) void isr_exception_handler(cpu_state_t *cpu_state) {
    printf("\nrax    = 0x%016llx, rbx = 0x%016llx, rcx = 0x%016llx, rdx = 0x%016llx\n", cpu_state->rax, cpu_state->rbx, cpu_state->rcx, cpu_state->rdx);
    printf("rsi    = 0x%016llx, rdi = 0x%016llx, rbp = 0x%016llx, rsp = 0x%016llx\n", cpu_state->rsi, cpu_state->rdi, cpu_state->rbp, cpu_state->rsp);
    printf("cr0    = 0x%016llx, cr2 = 0x%016llx, cr3 = 0x%016llx, cr3 = 0x%016llx\n", cpu_state->cr0, cpu_state->cr2, cpu_state->cr3, cpu_state->cr4);
    printf("r8     = 0x%016llx, r9  = 0x%016llx, r10 = 0x%016llx, r11 = 0x%016llx\n", cpu_state->r8, cpu_state->r9, cpu_state->r10, cpu_state->r11);
    printf("r12    = 0x%016llx, r13 = 0x%016llx, r14 = 0x%016llx, r15 = 0x%016llx\n", cpu_state->r12, cpu_state->r13, cpu_state->r14, cpu_state->r15);
    printf("rflags = 0x%016llx, rip = 0x%016llx, cs  = 0x%016llx, ss  = 0x%016llx\n", cpu_state->rflags, cpu_state->rip, cpu_state->cs, cpu_state->ss);
    
    panic("%s (#%s). Error Code = 0x%016llx", true, exception_names[cpu_state->vector], exception_mnemonics[cpu_state->vector], cpu_state->error_code);
    while (1) {
        __asm__ volatile("hlt"); // avoid warning: 'noreturn' function does return
    }
}
