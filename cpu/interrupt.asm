[bits 32]
[extern isr_handler]
[extern irq_handler]
[extern portByteOut]
[extern portByteIn]
[extern set_idt]
[extern set_idt_gate]
PIC1_CMD equ 0x20
PIC2_CMD equ 0xa0
PIC1_DATA equ 0x21
PIC2_DATA equ 0xa1
PIC_EOI equ 0x20

isr_common_stub:
	pusha
	mov ax, ds
	push eax
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	call isr_handler

	pop eax
	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	popa
	add esp, 8
	sti
	iret

irq_common_stub:
	pusha
	mov ax, ds
	push eax
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	call irq_handler

	pop ebx
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx
	popa
	add esp, 8
	sti
	iret

global isr_install
isr_install:
	ret
	pusha
	%macro isr_ 1
	cli
	push isr_%1
	push %1
	call set_idt_gate
	%endmacro
	
	%macro irq_ 2
	cli
	push irq_%1
	push %2
	call set_idt_gate
	%endmacro

	%macro pbo 2
	push byte %2
	push word %1
	call portByteOut
	%endmacro

	%assign i 0
	%rep 32
		isr_ i
		%assign i i+1
	%endrep

	push PIC1_DATA
	call portByteIn
	push eax
	push PIC2_DATA
	call portByteIn
	push eax
	pbo PIC1_CMD, 0x11
	pbo PIC2_CMD, 0x11
	pbo PIC1_DATA, 0x20
	pbo PIC2_DATA, 0x28
	pbo PIC1_DATA, 0x04
	pbo PIC2_DATA, 0x02
	pbo PIC1_DATA, 0x01
	pbo PIC2_DATA, 0x01
	pop eax
	pbo PIC1_DATA, eax
	pop eax
	pbo PIC2_DATA, eax

	%assign i 32
	%rep 16
		%assign j i-32
		irq_ j, i
		%assign i i+1
	%endrep
	call set_idt
	popa
	ret

%macro m_isr 1
global isr_%1
isr_%1:
	push byte 0
	push byte %1
	jmp isr_common_stub
%endmacro

%assign j 0
%rep 32
	m_isr j
	%assign j j+1
%endrep

%macro m_irq 1
global irq_%1
irq_%1:
	push byte 0
	%assign tmp %1+32
	push byte tmp
	jmp irq_common_stub
%endmacro

%assign k 0
%rep 16
	m_irq k
	%assign k k+1
%endrep

