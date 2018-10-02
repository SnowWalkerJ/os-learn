load_disk:
	; arguments:
	;     dh: mumber of sectors to read
	;     dl: # of disk
	;     bx: target address
    pusha
    push dx
    mov ah, 0x02   ; 'read' command
    mov al, dh     ; number of sectors to read
    mov cl, 0x02   ; start # of sector
    mov ch, 0x00   ; cylinder
    mov dh, 0x00   ; head number
    int 0x13
    
    jc .disk_error
    
    pop dx
    cmp al, dh
    jne .sector_error
    
    popa
    ret

.disk_error:
    mov bx, DISK_ERROR_MSG
    call print
	mov bx, ax
	call print_hex
    jmp .end

.sector_error:
    mov bx, SECTOR_ERROR_MSG
    call print
    jmp .end

.end:
    jmp $

DISK_ERROR_MSG: db 'Load disk error', 0
SECTOR_ERROR_MSG: db 'Sector number not matched', 0
