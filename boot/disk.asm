load_disk:
	; arguments:
    ;     al: number of sectors to read
    ;     bx: target address in memory
    ;     cl: start number of sector
    ;     ch: cylinder
	;     dh: head
	;     dl: disk
    pusha
    push ax
    mov ah, 0x02   ; 'read' command
    int 0x13
    
    jc .disk_error
    
    pop dx
    cmp al, dl
    jne .sector_error
    
    popa
    ret

.disk_error:
    mov bx, DISK_ERROR_MSG
    call print
    jmp .end

.sector_error:
    mov bx, SECTOR_ERROR_MSG
    call print
    jmp .end

.end:
    jmp $

DISK_ERROR_MSG: db 'Load disk error', 0
SECTOR_ERROR_MSG: db 'Sector number not matched', 0
