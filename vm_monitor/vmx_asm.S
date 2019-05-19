.section .text
.code64

.global vm_exit_handler

vm_exit_handler:

    cli
    hlt

