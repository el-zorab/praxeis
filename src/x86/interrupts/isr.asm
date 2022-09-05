extern isr_exception_handler

%macro isr_error_stub 1
isr_stub_%+%1:
    push %1
    jmp cpu_state_assembler
%endmacro

%macro isr_no_error_stub 1
isr_stub_%+%1:
    push 0
    push %1
    jmp cpu_state_assembler
%endmacro

cpu_state_assembler:
    push rbp
    mov rbp, rsp

    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi

    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rax, cr0
    push rax
    mov rax, cr2
    push rax
    mov rax, cr3
    push rax
    mov rax, cr4
    push rax

    mov rdi, rsp ; pass cpu_state
    call isr_exception_handler

    pop rax
    mov cr4, rax
    pop rax
    mov cr3, rax
    pop rax
    mov cr2, rax
    pop rax
    mov cr0, rax

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8

    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    mov rsp, rbp
    pop rbp

    add rsp, 0x10

    iretq

isr_no_error_stub 0
isr_no_error_stub 1
isr_no_error_stub 2
isr_no_error_stub 3
isr_no_error_stub 4
isr_no_error_stub 5
isr_no_error_stub 6
isr_no_error_stub 7
isr_error_stub    8
isr_no_error_stub 9
isr_error_stub    10
isr_error_stub    11
isr_error_stub    12
isr_error_stub    13
isr_error_stub    14
isr_no_error_stub 15
isr_no_error_stub 16
isr_error_stub    17
isr_no_error_stub 18
isr_no_error_stub 19
isr_no_error_stub 20
isr_error_stub    21
isr_no_error_stub 22
isr_no_error_stub 23
isr_no_error_stub 24
isr_no_error_stub 25
isr_no_error_stub 26
isr_no_error_stub 27
isr_no_error_stub 28
isr_error_stub    29
isr_error_stub    30
isr_no_error_stub 31

global isr_table
isr_table:
%assign i 0
%rep    32
    dq isr_stub_%+i
%assign i i+1
%endrep

global halt_handler
halt_handler:
    cli
    hlt
    jmp halt_handler
