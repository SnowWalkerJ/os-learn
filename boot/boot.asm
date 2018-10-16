[org 0x7C00]
KERNEL_OFFSET equ 0x1000 ; load the kernel code into this address

mov [BOOT_DRIVER], dl    ; save the boot driver number into memory

; set stack address
mov bp, 0x8000
mov sp, bp

; print infomation
mov bx, MSG_REAL
call print

call load_kernel         ; load the kernel code
call switch_to_pm        ; switch to 32-bit protected mode
jmp $

%include 'boot/print.asm'
%include 'boot/load_disk.asm'
%include 'boot/gdt.asm'
%include 'boot/switch_protected_mode.asm'
%include 'boot/print_pm.asm'

[bits 16]
load_kernel:
	mov bx, KERNEL_OFFSET  ; target address
	mov dh, 31             ; load nine sections
	mov dl, [BOOT_DRIVER]  ; boot driver number
	call load_disk
	ret

[bits 32]
BEGIN_PM:                  ; this is where the program jumps to after entering protected mode
	mov ebx, MSG_PM
	call print_pm
	call KERNEL_OFFSET     ; call the kernel
	jmp $

BOOT_DRIVER db 0
MSG_REAL db 'Start in 16bit real mode', 0
MSG_PM db 'Land in 32bit protected mode', 0

times 510-($-$$) db 0
dw 0xaa55
