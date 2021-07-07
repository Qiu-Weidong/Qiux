; 硬盘启动扇区

; 常量定义
%include "const.inc"

    org OffsetOfBoot

    mov ax, cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov sp,TopOfStack

    mov byte [packet_size], disk_addr_packet_size   ; 只需要执行一次

    mov byte [trans_sector_nr], 0x1         ; 传输一个扇区
    mov word [dest_addr_offset], 0x7e00     ; 读入0x7e00的位置
    mov word [dest_addr_seg], 0             ; 读入的段地址
    mov dword [lba_low], 0x1                ; 读入的扇区号
    mov dword [lba_high], 0x0               ; 
    call readsector

    call clean_screen 

    mov ax, 0x1301                              
    mov bx, 0x000c
    mov dx, 0x0100
    mov bp, bootMsg                      
    mov cx, bootMsgLen                   
    int 0x10
    
    ; 读取super
    mov word [dest_addr_offset], OffsetOfInput 
    mov dword [lba_low], 0x2
    call readsector
    mov si, OffsetOfInput
    mov di, super_addr
    xor ecx, ecx
    mov cx, super_size
    call memcpy

    ; 读取inode 1
    push 0x1 
    call get_inode 
    add esp, 2

    ; 加载根目录
    push BaseOfKernel
    push OffsetOfKernel
    push inode_addr
    call load_file
    add esp, 6

    ; 目录项大小16
    mov edx, dword [isize]
    shr edx, 4                  ; eax 中保存目录项的条数
    mov ax, BaseOfKernel
    mov ds, ax
    mov si, OffsetOfKernel      ; ds:si指向根目录第一个目录项名称位置
    xor ebx, ebx
    xor ecx, ecx
L5:
    push si

    add si, 2
    mov cx, kernel_bin_len
    mov di, kernel_bin
    call strcmp
    pop si

    test al, al
    jz found 
    add si, 16
    inc bx

    cmp bx, dx
    jl L5
    
    ; 没有kernel
    mov ax, 0x1301
    mov bx, 0x008c
    mov dx, 0x0100
    mov bp, kernelNotFound
    mov cx, kernelNotFoundLen
    int 0x10

    jmp $
found:
    push word [ds:si]

    mov ax, 0
    mov ds, ax
    call get_inode
    add esp, 2

    ; 此时kernel.bin的inode已经在inode中
    push BaseOfKernel
    push OffsetOfKernel
    push inode_addr
    call load_file          ; 7cde
    add esp, 6

    jmp _start

%include "lib.inc"


times 510 - ($-$$) db 0
dw 0xaa55


%include "string.inc"
%include "gdt.inc"

_start:
    xor ebx, ebx
    mov di, MemChkBuf
.memchkloop:
    mov eax, 0xe820
    mov ecx, 0x20
    mov edx, 0x534d4150
    int 0x15
    jc mem_chk_fail
    add di, 20
    inc dword [dwMCRNumber]
    test ebx, ebx
    jne .memchkloop
    jmp mem_chk_ok
mem_chk_fail:
    mov dword [dwMCRNumber], 0
mem_chk_ok:

    ; 加载gdtr
    lgdt [GdtPtr]
    ; 关中断
    cli

    ; 打开A20地址线
    in al, 0x92
    or al, 00000010b
    out 92h, al

    ; 将cr0最低位置为1
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; 进入保护模式
    jmp dword SelectorFlatC:(PM_START)



[SECTION .s32]
align 32
[BITS 32]
PM_START:
    mov ax, SelectorVideo
    mov gs, ax
    mov ax, SelectorFlatRW
    mov es, ax
    mov ss, ax
    mov ds, ax
    mov fs, ax
    mov esp, TopOfStack

    ; 将0x10000到0x60000的内存全部清零
    mov ecx, 0x14000
    mov eax, 0x10000
clear_loop:
    mov dword [eax], 0
    add eax, 4
    loop clear_loop

    mov edx, BaseOfKernel                       ; '. 
    shl edx, 4                                  ;  | 计算kernel存放的地址
    add edx, OffsetOfKernel                     ;  /

    mov edi, edx                                ; edi -> kernel
    mov esi, edx                                ; esi -> kernel

    movzx ebx, word [edi+42]                    ; ebx -> e_phentsize
    movzx ecx, word [edi+44]                    ; ecx -> e_phnum

    add edi, dword [edi+28]                     ; edi -> program header table
Go_on_copy:
    mov eax, dword [edi+4]                      ; eax -> p_offset
    add eax, edx                                ; eax 为当前段在内存中的起始地址

    push dword [edi+16]                         ; push p_filesz
    push eax                                    ; push 当前段在内存中的起始地址
    push dword [edi+8]                          ; push p_vaddr
    call memcpy32                               ; memcpy(p_vaddr, 当前段在内存中的起始地址，p_filesz);
    add esp, 12
    add edi, ebx

    loop Go_on_copy
    
    jmp dword [esi+24]

memcpy32:
    push ebp
    mov ebp, esp
    push ecx
    push edi
    push esi

    mov ecx, [ebp+16]
    test ecx, ecx
    jz L32

    mov esi, [ebp+12]
    mov edi, [ebp+8]
    cld
mem_loop:
    lodsb
    stosb
    loop mem_loop
L32:
    pop esi
    pop edi
    pop ecx
    pop ebp
    ret