
INT_M_CTL	    equ	0x20	
INT_M_CTLMASK	equ	0x21	
INT_S_CTL	    equ	0xA0	
INT_S_CTLMASK	equ	0xA1	
EOI		        equ	0x20

%macro delay 1
    times %1 nop
%endmacro

[section .text]
[global enable_irq ]
[global disable_irq]
[global pic_init   ]


; void pic_init();
pic_init:
    mov al, 0xff
    out INT_M_CTLMASK, al
    delay 2

    out INT_S_CTLMASK, al 
    delay 2

    mov al, 0x11
    out INT_M_CTL, al
    delay 2

    out INT_S_CTL, al
    delay 2

    mov al, 0x20
    out INT_M_CTLMASK, al 
    delay 2

    mov al, 0x28
    out INT_S_CTLMASK, al 
    delay 2

    mov al, 0x04
    out INT_M_CTLMASK, al 
    delay 2

    mov al, 0x02
    out INT_S_CTLMASK, al 
    delay 2

    mov al, 0x01
    out INT_M_CTLMASK, al 
    delay 2

    out INT_S_CTLMASK, al 
    delay 2

    ; 屏蔽所有中断
    mov al, 0xff 
    out INT_M_CTLMASK, al
    delay 2

    mov al, 0xff
    out INT_S_CTLMASK, al 
    delay 2

    ret

; void enable_irq(int irq)
enable_irq:
    mov ecx, [esp+4]
    push ebx
    pushf
    cli 
    mov ah, 0xfe
    rol ah, cl 
    mov dx, INT_M_CTLMASK
    mov bx, INT_S_CTLMASK
    cmp cl, 8
    cmovae dx, bx
    in al, dx
    and al, ah 
    out dx , al
    popf
    pop ebx
    ret 

; void disable_irq(int irq)
disable_irq:
    mov ecx, [esp+4]
    push ebx
    pushf
    cli
    mov ah, 0x1
    rol ah, cl
    mov dx, INT_M_CTLMASK
    mov bx, INT_S_CTLMASK
    cmp cl, 8
    cmovae dx, bx
    in al, dx
    or al, ah
    out dx, al
    popf
    pop ebx
    ret