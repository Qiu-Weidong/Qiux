%ifndef LIB16_INC_
%define LIB16_INC_

; 字符串比较函数，ds:si和es:di分别指向要比较的字符串
strcmp:
    lodsb
    sub al, byte [es:di]
    jnz L6
    inc di
    loop strcmp
L6: ret

get_inode:
    push bp
    mov bp, sp
    
    xor eax, eax
    mov ax, [bp+4]
    dec ax

    shl eax, 5              ; eax是ino在inode array中的偏移
    mov ebx, eax

    and ax, 0x3ff

    shr ebx, 10             ; inode所在的zone号
    add bx, 2
    add bx, word [imap_blk]
    add bx, word [zmap_blk] ; bx为inode所在的zone号
    shl bx, 1               ; 得到扇区号

    mov byte [trans_sector_nr], 0x2
    mov dword [lba_low], ebx
    mov word [dest_addr_offset], OffsetOfInput
    mov word [dest_addr_seg], BaseOfInput
    call readsector

    mov si, OffsetOfInput
    add si, ax

    mov di, inode_addr
    xor ecx, ecx
    mov cx, inode_size
    call memcpy

    pop bp
    ret 
load_file:
    push bp
    mov bp, sp

    mov byte [trans_sector_nr], 0x2
    
    
    mov si, [bp+4] 
    mov eax, dword [si+4] 
    add eax, ZoneSize-1
    shr eax, 10 
    
    cmp ax, 7 
    jle L1 ;不需要加载中间块
    
    ; 加载中间块
    xor ebx, ebx
    mov bx, word [si+28] ; bx -> zone[7] 
    shl ebx, 1 ; 乘2得到扇区号
    mov dword [lba_low], ebx
    mov word [dest_addr_offset], OffsetOfSecondBlock
    mov word [dest_addr_seg], BaseOfSecondBlock
    call readsector

L1:
    mov bx, word [bp+8]
    mov word [dest_addr_seg], bx
    xor cx, cx
    add si, 14 ; si ->zone[0]
    mov dx, [bp+6] ; dest
    mov di, OffsetOfSecondBlock
L2: 
    cmp cx, 7
    jge L3 

    xor ebx, ebx
    mov bx, word [si]
    add si, 2
    jmp L4

L3: ; 间接块中加载
    xor ebx, ebx
    mov bx, word [di]   
    add di, 2
L4:
    shl ebx, 1
    mov word [dest_addr_offset], dx
    mov dword [lba_low], ebx
    call readsector
    
    add dx, ZoneSize
    jnc l7
    add word [dest_addr_seg], 0x1000 
l7:
    inc cx
    cmp cx, ax
    jl L2

    pop bp
    ret 
readsector:
    push ax
    push dx
    push si

    mov ah, 0x42
    mov dl, 0x80
    mov si, disk_addr_packet
    int 0x13

    pop si
    pop dx
    pop ax
    ret 
memcpy:
    cld
.1: lodsb
    stosb
    loop .1
    ret 

clean_screen:
    mov ax, 0x0600      
    mov bx, 0x0700                  
    mov cx, 0x0000         
    mov dx, 0x184f
    int 0x10 
    ret 
%endif ; LIB16_INC_
