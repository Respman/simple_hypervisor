;--------------------------------SETTINGS------------------------------------------
DEF_P32_BSECTOR   EQU	     1

DEF_P32_SEGM	  EQU	     0000    ; segment to read

DEF_P32_OFFSET	  EQU	     0x7C00+512   ; offset to read
DEF_N		  EQU	     2	      ; number of sectors to read

;----------------------------------------------------------------------------------

use16
org 7C00h
boot:
    cli
    cld
    xor    ax, ax
    mov    ss, ax
    mov    ds, ax
    mov    sp, 7C00h

    call    set_text_mode

load_p32:

    push    0000	    ; hight  of start sector
    push    0000	    ; hight  of start sector
    push    0000	    ; low addr of start sector
    push    DEF_P32_BSECTOR ; low addr of start sector		       
    push    DEF_P32_SEGM    ; segment
    push    DEF_P32_OFFSET  ; offset
    push    DEF_N	    ; number sectors to read
    push    0010h	    ; packet size
    mov    si, sp
    mov    ah, 42h
    int    13h
    add    sp, 10h
 
 	mov    si, dbg_disk
 	call    print_str
    jnc    DEF_P32_OFFSET

    mov    si, dbg_disk_err
    call    print_str
    hlt
    jmp    $

include 'vga_text.inc'

;----------------------------------------------------------------------------------


dbg_disk 	db 'loading loader0', 0x0A, 0x0D, 0
dbg_disk_err	db 'disk error occured!' , 0

rb boot+512-2-$
dw 0AA55h
