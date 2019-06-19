.section .text
.code64

.global vm_exit_handler
.extern vm_exit_handler_entry

vm_exit_handler:

    // Save non-vmcs processor states
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %r11
    pushq %r10
    pushq %r9
    pushq %r8
    movq %cr2, %rax
    pushq %rax

    movq %rsp, %rdi
    call vm_exit_handler_entry

    cli
    hlt

