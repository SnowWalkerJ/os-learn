[extern main]
[extern drive_infos]
[bits 16]
global _start
_start:
mov bx, MSG_SETUP
call print

mov [BOOT_DRIVER], dl

call detect_drive_infos


call switch_to_pm        ; switch to 32-bit protected mode
jmp $

detect_drive_infos:
	; detect 1st floppy info
	push drive_infos
	mov dl, 0x00
	call get_drive_infos
	; detect 1st hdd info
	push drive_infos+7
	mov dl, 0x80
	call get_drive_infos
	; detect 2nd hdd info
	push drive_infos+14
	mov dl, 0x81
	call get_drive_infos

[bits 32]
BEGIN_PM:                  ; this is where the program jumps to after entering protected mode
	mov ebx, MSG_PM
	call print_pm
	call main
	jmp $


[bits 16]
MSG_SETUP db 'Enter SETUP program', 0
MSG_PM db 'Land in 32bit protected mode', 0
global BOOT_DRIVER
BOOT_DRIVER db 0


%include 'boot/print.asm'
%include 'boot/disk.asm'
%include 'boot/gdt.asm'
%include 'boot/switch_protected_mode.asm'
%include 'boot/print_pm.asm'
%include 'boot/bios.asm'