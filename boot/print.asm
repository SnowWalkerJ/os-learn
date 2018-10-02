print:
    pusha

.loop:
    mov ah, 0x0e
    mov al, [bx]
    cmp al, 0x00
    je .end
    int 0x10
    add bx, 1
    jmp .loop

.end:
    mov al, 0x0a
    int 0x10
    mov al, 0x0d
    int 0x10
    popa
    ret

print_hex:
    pusha
    mov ah, 0x0e
    mov al, '0'
    int 0x10
    mov al, 'x'
    int 0x10
    mov dl, 0

.loop:
    cmp dl, 4
    je .end
    rol bx, 4
    mov ax, bx
    and al, 0x0f
    add al, '0'
    cmp al, '9'
    jle .putchar
    add al, 7

.putchar:
    mov ah, 0x0e
    int 0x10
    add dl, 1
    jmp .loop

.end:
    mov al, 0x0a
    int 0x10
    mov al, 0x0d
    int 0x10
    popa
    ret
