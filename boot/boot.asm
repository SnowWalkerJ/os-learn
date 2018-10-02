[org 0x7C00]
KERNEL_OFFSET equ 0x1000

mov [BOOT_DRIVER], dl
mov bp, 0x8000
mov sp, bp

mov bx, MSG_REAL
call print

call load_kernel
call switch_to_pm
jmp $

%include 'boot/print.asm'
%include 'boot/load_disk.asm'
%include 'boot/gdt.asm'
%include 'boot/switch_protected_mode.asm'
%include 'boot/print_pm.asm'

[bits 16]
load_kernel:
	mov bx, KERNEL_OFFSET
	mov dh, 9
	mov dl, [BOOT_DRIVER]
	call load_disk
	ret

[bits 32]
BEGIN_PM:
	mov ebx, MSG_PM
	call print_pm
	call KERNEL_OFFSET
	jmp $

BOOT_DRIVER db 0
MSG_REAL db 'Start in 16bit real mode', 0
MSG_PM db 'Land in 32bit protected mode', 0

times 510-($-$$) db 0
dw 0xaa55
