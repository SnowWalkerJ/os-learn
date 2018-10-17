[global read_cr0]
read_cr0:
	mov eax, cr0
	ret

[global read_cr1]
read_cr1:
	mov eax, cr1
	ret

[global read_cr2]
read_cr2:
	mov eax, cr2
	ret

[global read_cr3]
read_cr3:
	mov eax, cr3
	ret

[global write_cr0]
write_cr0:
	push ebp
	mov ebp, esp
	mov eax, [ebp+8]
	mov cr0, eax
	pop ebp
	ret

[global write_cr1]
write_cr1:
	push ebp
	mov ebp, esp
	mov eax, [ebp+8]
	mov cr1, eax
	pop ebp
	ret

[global write_cr2]
write_cr2:
	push ebp
	mov ebp, esp
	mov eax, [ebp+8]
	mov cr2, eax
	pop ebp
	ret

[global write_cr3]
write_cr3:
	push ebp
	mov ebp, esp
	mov eax, [ebp+8]
	mov cr3, eax
	pop ebp
	ret

