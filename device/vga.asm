; vga相关控制函数

; 端口定义
CRTC_ADDR_REG   equ	0x3D4	;/* CRT Controller Registers - Addr Register */
CRTC_DATA_REG	equ 0x3D5	;/* CRT Controller Registers - Data Register */
START_ADDR_H	equ 0xC	    ;/* reg index of video mem start addr (MSB) */
START_ADDR_L	equ 0xD	    ;/* reg index of video mem start addr (LSB) */
CURSOR_H	    equ 0xE	    ;/* reg index of cursor position (MSB) */
CURSOR_L	    equ 0xF	    ;/* reg index of cursor position (LSB) */
V_MEM_BASE      equ 0xB8000
[global set_cursor ]
[global set_video_start_addr ]
[global vga_flush  ]

set_cursor:
    pushf

    ; outb(CRTC_ADDR_REG, CURSOR_H);
    mov al, CURSOR_H
    mov dx, CRTC_ADDR_REG
    out dx, al

    ; outb(CRTC_DATA_REG, (position >> 8) & 0xff);
    mov eax, [esp + 8]
    shr eax, 8
    mov dx, CRTC_DATA_REG
    out dx, al

    ; outb(CRTC_ADDR_REG, CURSOR_L);
    mov al, CURSOR_L
    mov dx, CRTC_ADDR_REG
    out dx, al

    ; outb(CRTC_DATA_REG, position & 0xff);
    mov eax, [esp + 8]
    mov dx, CRTC_DATA_REG
    out dx, al
    
    popf
    ret 

set_video_start_addr:
    pushf

    ; outb(CRTC_ADDR_REG, START_ADDR_H);
    mov al, START_ADDR_H
    mov dx, CRTC_ADDR_REG
    out dx, al

    ; outb(CRTC_DATA_REG, ((V_MEM_BASE + addr) >> 8) & 0xff);
    mov eax, [esp + 8]
    add eax, V_MEM_BASE
    mov ecx, eax
    shr eax, 8
    mov dx, CRTC_DATA_REG
    out dx, al

    ; outb(CRTC_ADDR_REG, START_ADDR_L);
    mov al, START_ADDR_L
    mov dx, CRTC_ADDR_REG
    out dx, al

    ; outb(CRTC_DATA_REG, (V_MEM_BASE + addr) & 0xff);
    mov eax, ecx
    mov dx, CRTC_DATA_REG
    out dx, al

    popf
    ret 

vga_flush:
    pushf

    mov al, CURSOR_L
    mov dx, CRTC_ADDR_REG
    out dx, al

    mov eax, [esp + 8] ; position
    mov dx, CRTC_DATA_REG
    out dx, al
    
    mov al, CURSOR_H
    mov dx, CRTC_ADDR_REG
    out dx, al

    mov eax, [esp + 8]
    shr eax, 8
    mov dx, CRTC_DATA_REG
    out dx, al

    mov al, START_ADDR_L
    mov dx, CRTC_ADDR_REG
    out dx, al

    mov eax, [esp + 12]
    add eax, V_MEM_BASE
    mov dx, CRTC_DATA_REG
    out dx, al

    mov al, START_ADDR_H
    mov dx, CRTC_ADDR_REG
    out dx, al

    mov eax, [esp + 12]
    add eax, V_MEM_BASE
    shr eax, 8
    mov dx, CRTC_DATA_REG
    out dx, al

    popf
    ret

