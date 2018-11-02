get_drive_infos:
    ;   Parameters:
    ;	AH = 08
    ;	DL = drive number (0=A:, 1=2nd floppy, 80h=drive 0, 81h=drive 1)
    ;
    ;	Returns:
    ;	AH = status  (see INT 13,STATUS)
    ;	BL = CMOS drive type
    ;	     01 - 5¬  360K	     03 - 3«  720K
    ;	     02 - 5¬  1.2Mb	     04 - 3« 1.44Mb
    ;	CH = cylinders (0-1023 dec. see below)
    ;	CL = sectors per track	(see below)
    ;	DH = number of sides (0 based)
    ;	DL = number of drives attached
    ;	ES:DI = pointer to 11 byte Disk Base Table (DBT)
    ;	CF = 0 if successful
    ;	   = 1 if error
    mov ah, 0x08            ;INT 0x13, 0x08
    ; mov dl, [BOOT_DRIVER] ; I don't know why, but this line causes error
    int 0x13
    mov ax, bx
    pop bx
    jc .error
    mov [bx], byte 0x01
    mov [bx+1], al
    mov al, ch
    mov ah, cl
    and ah, 0b11000000
    ror ah, 6
    mov [bx+2], ax
    and cl, 0b00111111
    mov [bx+4], cl
    mov [bx+5], dh
    mov [bx+6], dl
    ret

.error:
    cld
    mov di, ax
    mov ax, 0
    mov cx, 7
    rep stosb
    ret

