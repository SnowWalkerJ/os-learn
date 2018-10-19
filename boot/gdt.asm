gdt_start:

gdt_null:
	; 8 bytes of null
	dd 0
	dd 0

gdt_code:
	dw 0xffff    ; limit 0-15
	dw 0         ; base 0-15
	db 0         ; base 16-23
	db 10011010b
	db 11001111b
	db 0         ; base 24-31

gdt_data:
	dw 0xffff
	dw 0
	db 0
	db 10010010b
	db 11001111b
	db 0

gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
