use16
org 0x7c00+512
real_mode:
	cli
	xor	ax, ax
	mov	ds, ax
	mov	ss, ax
	mov	es, ax
	mov	sp, 0x7C00

	; enable A20 gate
	in	al, 92h
	test	al, 02h
	jnz	.no92
	or	al, 02h
	out	92h, al

.no92:

	lgdt	fword ptr GDTR

	mov	eax, CR0
	or	al, 1
	mov	CR0, eax			; set bit 0 PE

	jmp	0008:main

;----------------------------------------------------------

use32

GDT:
   NULL_descr		db	8 dup (0)

   CS_SEGMENT		db	0FFh, 0FFh	; segment limit 15:0
			db	00h, 00h	; segment base addr 15:0
						db	00h		; base 23:16
						db	10011011b	;
						db	11001111b	;
						db	00h			; base 31:24

   DATA_SEGMENT 	db	0FFh, 0FFh	; segment limit 15:0
						db	00h, 00h	; segment base addr 15:0
						db	00h		; base 23:16
						db	10010010b
						db	11001111b
						db	00h		; base 31:24

   CS_SEGMENT_REAL		db	0FFh, 0FFh	; segment limit 15:0
						db	00h, 00h	; segment base addr 15:0
						db	00h		; base 23:16
						db	10011011b	;
						db	00h			;
						db	00h			; base 31:24

   DATA_SEGMENT_REAL 	db	0FFh, 0FFh	; segment limit 15:0
						db	00h, 00h	; segment base addr 15:0
						db	00h		; base 23:16
						db	10010010b
						db	00h 	;
						db	00h		; base 31:24

GDT_size			equ $-GDT
GDTR			dw  GDT_size-1
						dd  GDT

;----------------------------------------------------------

include 'serialport_log32.inc'

msg1 	db 	"Cpu vendor:", 0xa, 0xd, 0
v_eax 	dd	0
v_ebx 	dd	0
v_ecx 	dd	0
v_edx 	dd	0
endl	db  0xa, 0xd, 0

;----------------------------------------------------------

main:
	mov	ax, 10h
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	fs, ax
	mov	gs, ax
	mov	esp, 0x7C00

	call	Init_Com_Port32

;-----cpuid------------------------------------------------

	push 	msg1
	call 	SerialPrintStr32

	mov eax, 0x80000002
	cpuid
	mov dword [v_eax], eax
	mov dword [v_ebx], ebx
	mov dword [v_ecx], ecx
	mov dword [v_edx], edx
	push v_eax
	call 	SerialPrintStr32

;----------------------------------------------------------

;Переход в реальный режим (снова регистр CR0)

	jmp 0x3*8:next_step


use16
next_step:	

	mov eax, cr0
	and eax, not 1
	mov cr0, eax

	xor	ax, ax
	mov	ds, ax
	mov	ss, ax
	mov	es, ax
	mov	sp, 0x7C00


; disable A20 gate
	in	al, 92h
	test	al, 02h
	jnz	.no92_2
	xor	al, 02h
	out	92h, al

.no92_2:

	jmp 0:real_mode_again

real_mode_again:

; power off virtual machine 
	mov ax, 5307h
	xor bx, bx
	inc bx
	mov cx, 3

	int 15h

