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
    // call the vmexit handler written in C
    movq %rsp, %rdi
    call vm_exit_handler_entry
    // Restore the RSP and guest non-vmcs processor state
    movq %rax, %rsp
    popq %rax
    movq %rax, %cr2
    popq %r8
    popq %r9
    popq %r10
    popq %r11
    popq %r12
    popq %r13
    popq %r14
    popq %r15
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    // resume the VM
    vmresume
    // FIXME: Error re-launching the vm, Let's go to some place to report to user
    cli
    hlt

