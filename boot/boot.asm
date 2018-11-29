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

mov dl, [BOOT_DRIVER]    ; pass this value to kernel_entry

call KERNEL_OFFSET
jmp $

%include 'boot/print.asm'
%include 'boot/disk.asm'

load_kernel:
	mov al, 54
	mov ch, 0
	mov cl, 2
	mov bx, KERNEL_OFFSET  ; target address
	mov dh, 0
	mov dl, [BOOT_DRIVER]  ; boot driver number
	call load_disk
	ret

BOOT_DRIVER db 0
MSG_REAL db 'Start in 16bit real mode', 0

times 510-($-$$) db 0
dw 0xaa55
