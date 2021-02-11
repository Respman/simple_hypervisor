
.intel_syntax noprefix
.globl   vmexit_wrapper
.globl   get_vmexit_addr
.align   4
 
get_vmexit_addr:
    call    m
m:
    pop     rax
    add     rax, 6
    ret  


vmexit_wrapper:

    push    0x0
    push    0x0
    push    0x0
    push    rax
    push    rcx
    push    rdx
    push    rbx
    push    0x0
    push    rbp
    push    rdi
    push    rsi

    mov     rax, rsp

    call    VMEXIT_handler

    pop     rsi
    pop     rdi
    pop     rbp
    pop     rbx
    pop     rbx
    pop     rdx
    pop     rcx
    pop     rax
    add     rsp, 24

    vmresume

    call    VMEnter_error

  
