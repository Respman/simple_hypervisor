
BUS_numb	equ	0x00
DEV_numb	equ	31
FUNC_numb	equ	2
PORT_numb	equ	0x03

HYPER_SIZE 	equ 512*35

include 'memory_map.inc'
include 'defines.inc'
include 'msr_regs.inc'

use16
org LOADER_addr
real_mode:

	;jmp $
	cli
	xor		ax, ax
	mov		ds, ax
	mov		ss, ax
	mov		es, ax
	mov		sp, 0x7C00


load_OS_mbr:

	push	0000		; hight  of start sector
	push	0000		; hight  of start sector
	push	0000		; low addr of start sector
	push	0000 ; low addr of start sector			   
	push	0000   ; segment
	push	0x7c00  ; offset
	push	1		; number sectors to read
	push	0010h		; packet size
	mov		si, sp
	mov 	dx, 0x81
	mov		ah, 42h
	int		13h
	add		sp, 10h


;try to get RAM type map:

	mov 	ebx, 0
	mov 	di, MemChkBuf
.loop:

	mov 	eax, 0x0e820
	mov 	edx, 0x0534D4150
	mov 	ecx, 20
	int 	0x15
	jc 		LABEL_MEM_CHK_FAIL
	add	 	di, cx
	mov 	dword [BufLen], ecx
	inc	 	dword [_dwMCRNumber] ;Memory check result

	cmp	 	ebx, 0
	jne	 	.loop
	jmp	 	LABEL_MEM_CHK_OK

LABEL_MEM_CHK_FAIL:
	mov	 	dword [_dwMCRNumber], 0

	mov	 	si, err_msg_0
	call	 print_str
	jmp 	$


LABEL_MEM_CHK_OK:

;---------------------------

	mov	 	si, dbg_msg_1
	call 	print_str
;	jmp 	$

	; enable A20 gate
	in		al, 92h
	test	al, 02h
	jnz		.no92
	or		al, 02h
	out		92h, al
	


.no92:

	mov 	si, dbg_msg_2
	call 	print_str

	; disable PIC
	mov		al, 0FFh
	out		0A1h, al
	out		21h, al

	mov 	si, dbg_msg_3
	call 	print_str
;	jmp $

	; imcr access
	mov		al, 0x70
	out		0x22, al
	mov		al, 0x01			; set bit 1 for SMP mode
	out		0x23, al

	lgdt	fword ptr GDTR

	mov		eax, CR0
	or		al, 1
	mov		CR0, eax			; set bit 0 PE


	jmp		0008:main

include 'vga_text.inc'

err_msg_0 db 'RAM err', 0x0a, 0x0d, 0
dbg_msg_1 db 'enbl A20', 0x0a, 0x0d, 0
dbg_msg_2 db 'disbl PIC', 0x0a, 0x0d, 0
dbg_msg_3 db 'imcr access', 0x0a, 0x0d, 0
MemChkBuf: times 255 db 0

_dwMCRNumber dd 0 
MAX_PHY_ADDR dq 0
BufLen 	dq 0
HyperSpaceUsed dq 0x1000000

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

   CS64_SEGMENT 	db	00h, 00h		; segment limit 15:0
						db	00h, 00h		; segment base addr 15:0
						db	00h			; base 23:16
						db	98h
						db	20h
						db	00h			; base 31:24


   DS64_SEGMENT 	db	00h, 00h		; segment limit 15:0
						db	00h, 00h		; segment base addr 15:0
						db	00h			; base 23:16
						db	92h
						db	20h
						db	00h			; base 31:24

GDT_size			equ $-GDT
GDTR			dw  GDT_size-1
						dd  GDT

;----------------------------------------------------------

include 'serialport_log32.inc'

msg_pm_active		db	 'PM have been activated' ,0xa, 0xd, 0
msg_rd_ok			db	 'read...ok' ,0xa, 0xd, 0
msg_err 		db	 'read...err' ,0xa, 0xd, 0
msg_start_coping 		db	 'start coping' ,0xa, 0xd, 0
msg_end_coping 		db	 'end coping' ,0xa, 0xd, 0
msg_no_lm		db	 'IA32e is not supported',0xa, 0xd, 0
msg_lm_activated	db	 'Long Mode has been activated',0xa, 0xd, 0
msg_ram_addr 		db	 'max detected ram:' ,0xa, 0xd, 0
endl	db  0xa, 0xd, 0
;----------------------------------------------------------

main:
	mov		ax, 10h
	mov		ds, ax
	mov		es, ax
	mov		ss, ax
	mov		fs, ax
	mov		gs, ax
	mov		esp, STACK_addr

	call	Init_Com_Port32

	push	msg_pm_active
	call	SerialPrintStr32

;----------------------

	mov		eax, 0x80000000 		; Extended-function 8000000h.
	cpuid					; Is largest extended function
	cmp		eax, 0x80000000 		; any function > 80000000h?
	ja		.lm_supported
	mov		eax, 0x80000001 		; Extended-function 8000001h.
	cpuid					; Now EDX = extended-features flags.
	bt		edx, 29 			; Test if long mode is supported.
	jc		.lm_supported

	push	msg_no_lm
	call	SerialPrintStr32
	jmp		$

.lm_supported:
	; make PML4
	mov		edi, INIT32_PML4_addr
	mov		eax, (INIT32_PDPT_addr or 7)
	stosd
	xor		eax, eax
	stosd

	; make Page Directory Pointer Table
	mov		edi, INIT32_PDPT_addr
	mov		eax, 0x87				; 1 GB Page
	xor		edx, edx
	mov		ecx, PDPTE_CNT
.nxt_pdpte:
	stosd
	xchg 	eax, edx
	stosd
	xchg 	eax, edx
	add		eax, 0x40000000
	jnc		@f
	inc		edx
@@:
	dec		ecx
	jnz		.nxt_pdpte

	mov		eax, CR4
	or 		eax, 0xA0				; set PAE(5), PGE(7)
	mov		CR4, eax

	mov		eax, INIT32_PML4_addr
	mov		CR3, eax

	mov		ecx, IA32_EFER
	rdmsr
	or 		eax, 0x00000100			; set LME (8)
	wrmsr

	mov		eax, CR0
	or		eax, 0xE0000000			; set PG(31), CD(30), NE(29)
	mov		CR0, eax

	jmp		3*8:long_mode

use64
long_mode:

	mov		ax, 4 * 8
	mov		ds, ax
	mov		es, ax
	mov		ss, ax
	mov		fs, ax
	mov		gs, ax

;Calculating RAM size---------

	mov 	ecx, dword [_dwMCRNumber]

	mov 	rdx, MemChkBuf
loop_ram_size:

	xor 	rax, rax
	mov 	eax, dword [rdx + 16] ; type
	cmp 	rax, 1
	jne 	skip_addr_cmp

	mov 	rax, qword [rdx]      ; addr
	mov 	rbx, qword [rdx + 8]  ; len
	cmp 	rbx, qword [HyperSpaceUsed]
	jl 		skip_addr_cmp ;pomestitsya li 

	add 	rax, rbx
	cmp 	rax, qword [MAX_PHY_ADDR]
	jng 	skip_addr_cmp
	mov 	qword [MAX_PHY_ADDR], rax

skip_addr_cmp:
	add 	rdx, 20
	loop	loop_ram_size

;---------------------------------

;Calculating hypervisor addr------
	mov 	rax, qword [MAX_PHY_ADDR]

;qemu stub------------------------
	mov 	rax, 0x80000000
	;sub 	rax, 0x40000000
;---------------------------------

	sub 	rax, qword [HyperSpaceUsed]
	mov 	qword [BufLen], rax

;-----------------------------   

	push	msg_lm_activated
	call	SerialPrintStr64

	; make IDT
	mov	rdi, IDT64_addr
	mov	rdx, common_int
	mov	rax, 0x8E0100080000		; present and type = 1110 interrupt gate
	mov	ax, dx				; offset 15..0
	and	edx, 0xFFFF0000
	shl	rdx, 32
	or	rax, rdx			; offset 31..16
	mov	rdx, common_int
	shr	rdx, 32
	mov	rcx, 32
.nxt_idte:
	stosq
	xchg	rax, rdx
	stosq
	xchg	rax, rdx
	dec	rcx
	jnz	.nxt_idte

	xor	rax, rax
	push	rax
	mov	rax, IDT64_addr
	shl	rax, 16
	mov	ax, 0x1FF
	push	rax
	lidt	[rsp]
	add	rsp, 16
	

	push	msg_start_coping
	call	SerialPrintStr64

	mov 	rsi, 0x9000+512*5
	mov 	rax, qword [BufLen]
	mov 	rdi, rax
	mov 	ecx, HYPER_SIZE
	rep 	movsb

	push	msg_end_coping
	call	SerialPrintStr64


	mov 	rax, qword [BufLen]
	mov 	rbx, 0
	mov 	ebx, dword [_dwMCRNumber]
	mov 	rcx, MemChkBuf
	mov 	rdx, qword [HyperSpaceUsed]
	
	jmp		rax

;----------------------------------------------------------

common_int:
	mov	rax, 0xDEADC0DE
	push	rax
	call	SerialPrintDigit64
	jmp	$


include 'serialport_log64.inc'




