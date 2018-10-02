[bits 16]
switch_to_pm:
    ; clear interrupts
	cli

	; load global descriptor table
	lgdt [gdt_descriptor]

	; set the lowest first bit to 1 to switch to protected mode
	mov eax, cr0
	or eax, 0x1
	mov cr0, eax

	; perform a long jump to flush CPU pipelines
	jmp CODE_SEG:init_pm

[bits 32]
init_pm:
	; reset segments
	mov ax, DATA_SEG
	mov ds, eax
	mov ss, eax
	mov es, eax
	mov fs, eax
	mov gs, eax

	; reset stack position
	mov ebp, 0x9000
	mov esp, ebp
	call BEGIN_PM
