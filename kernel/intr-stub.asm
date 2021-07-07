; 中断处理过程使用的栈顶
kernel_esp      equ 0x7e00

; 与8259A相关的一些常量
INT_M_CTL	    equ	0x20	
INT_M_CTLMASK	equ	0x21	
INT_S_CTL	    equ	0xA0	
INT_S_CTLMASK	equ	0xA1	
EOI		        equ	0x20
; 与中断相关的常量
intr_send       equ 0x80 ; 发送中断
intr_receive    equ 0x90 ; 接收中断


; 全局变量
[extern intr_handlers   ]
[extern current_proc    ]
[extern tss             ]
[extern k_reenter       ]

[global restart_current_process]

; 异常桩, 参数为错误码
%macro exception_stub 1
    call save
    sti
    call [intr_handlers+4*%1]
    add esp, 8                    
    cli
    ret
%endmacro

; 外中断都没有错误码
; intr_master_stub(int irq)
%macro intr_master_stub 1
    push %1 
    call save
    
    in al, INT_M_CTLMASK
    or al, (1<<%1)
    out INT_M_CTLMASK, al

    mov al, EOI
    out INT_M_CTL, al

    sti 
    call [intr_handlers+4*%1+32*4]
    add esp, 8
    cli 

    in al, INT_M_CTLMASK
    and al, ~(1<<%1)
    out INT_M_CTLMASK, al
    ret 
%endmacro

%macro intr_slave_stub 1
    push %1
    call save

    in al, INT_S_CTLMASK
    or al, ( 1 << (%1-8) )
    out INT_S_CTLMASK, al

    mov al, EOI
    out INT_M_CTL, al
    nop 
    out INT_S_CTL, al
    
    sti
    call [intr_handlers+4*%1+32*4]
    add esp, 8
    cli

    in al, INT_S_CTLMASK
    and al, ~(1 << (%1-8))
    out INT_S_CTLMASK, al
    ret
%endmacro

[section .data]
[global intr_stubs]
align 32
intr_stubs:
    dd divide_error_stub            ; 0
    dd single_step_exception_stub   ; 1
    dd nmi_stub                     ; 2
    dd breakpoint_exception_stub    ; 3
    dd overflow_stub                ; 4
    dd bounds_check_stub            ; 5
    dd invalid_opcode_stub          ; 6
    dd coproc_not_available_stub    ; 7
    dd double_fault_stub            ; 8
    dd coproc_seg_overrun_stub      ; 9
    dd invalid_tss_stub             ;10
    dd segment_not_present_stub     ;11
    dd stack_exception_stub         ;12
    dd general_protection_stub      ;13
    dd page_fault_stub              ;14
    dd 0                            ;15 intel保留未使用
    dd coproc_error_stub            ;16
    dd align_check_stub             ;17
    dd machine_check_stub           ;18
    dd simd_exception_stub          ;19

    ; 20~31 intel保留未使用
    times 31-20+1 dd 0

    ; 接下来是8259A外中断
    ; master中断
    dd clock_stub                   ;32 时钟中断 
    dd keyboard_stub                ;33 键盘中断
    dd cascade_stub
    dd serial2_stub
    dd serial1_stub
    dd lpt2_stub
    dd floppy_stub
    dd lpt1_stub

    ; slave中断
    dd realtime_clock_stub
    dd redirected_irp2_stub
    dd 0
    dd 0
    dd mouse_ps2_stub
    dd FPU_excp_stub
    dd AT_winchester_stub
    dd 0                            ;0x2f 

    
    times 0x80-0x2f-1 dd 0
    
    dd send_stub                    ;0x80 系统调用
    times 0x90-0x80-1 dd 0
    dd receive_stub                 ;0x90系统调用

    times 256 - (($-intr_stubs) >> 2) dd 0   ; 全部置为0

[section .text]

; 异常桩
divide_error_stub:          push 0
                            exception_stub 0
single_step_exception_stub: push 1
                            exception_stub 1
nmi_stub:                   push 2 
                            exception_stub 2
breakpoint_exception_stub:  push 3
                            exception_stub 3
overflow_stub:              push 4
                            exception_stub 4
bounds_check_stub:          push 5
                            exception_stub 5
invalid_opcode_stub:        push 6
                            exception_stub 6
coproc_not_available_stub:  push 7
                            exception_stub 7
double_fault_stub:          exception_stub 8
coproc_seg_overrun_stub:    push 9
                            exception_stub 9
invalid_tss_stub:           exception_stub 10
segment_not_present_stub:   exception_stub 11
stack_exception_stub:       exception_stub 12
general_protection_stub:    exception_stub 13
page_fault_stub:            exception_stub 14
coproc_error_stub:          push 16
                            exception_stub 16
align_check_stub:           exception_stub 17
machine_check_stub:         push 18
                            exception_stub 18
simd_exception_stub:        push 19
                            exception_stub 19

; 中断桩
; master中断0x20-0x27
clock_stub:                 intr_master_stub 0
keyboard_stub:              intr_master_stub 1
cascade_stub:               intr_master_stub 2
serial2_stub:               intr_master_stub 3
serial1_stub:               intr_master_stub 4
lpt2_stub:                  intr_master_stub 5
floppy_stub:                intr_master_stub 6
lpt1_stub:                  intr_master_stub 7
; slave中断, 0x28-0x2f
realtime_clock_stub:        intr_slave_stub 8
redirected_irp2_stub:       intr_slave_stub 9
                            ; 保留 10
                            ; 保留 11
mouse_ps2_stub:             intr_slave_stub 12
FPU_excp_stub:              intr_slave_stub 13
AT_winchester_stub:         intr_slave_stub 14
                            ; 保留 15

; 发送
send_stub:                  push intr_send
                            exception_stub intr_send
; 接收
receive_stub:               push intr_receive
                            exception_stub intr_receive


offset_of_retaddr   equ 48


save:
    pushad
    push ds 
    push es 
    push fs 
    push gs 

    mov ax, ss                   ; ss此时是ring0的ss
    mov ds, ax                   ; '.
    mov es, ax                   ;  | 将其他的段寄存器也设置为ring0
    mov fs, ax                   ;  /

    mov esi, esp                 ; 保存esp到esi，intr_frame的地址

    
    lock inc dword [k_reenter]   ; k_reenter的初始值为-1，如果没有发生重入，则inc后
                                 ; 为0，否则，inc后为一个大于0的数
    jnz set_restart1
    mov esp, kernel_esp
    ; 设置cr3指向内核页目录
    
    push restart_current_process ; 用于中断和异常桩中的ret指令
    push dword [current_proc]
    push esi                     ; 为intr_handler准备参数intr_frame*
                                 ; 内核栈中有两个数，一个intr_frame的地址，一个是返回地址
    xor ebp, ebp
    jmp [esi+offset_of_retaddr]
set_restart1:
    push re_enter                ; 这个地址用于中断和异常桩中的最后一条ret指令
    push 0                       ; 如果是重入，那么进程指针为nullptr
    push esi
    jmp [esi+offset_of_retaddr]

; void restart_current_process();
restart_current_process:
    mov esp, dword [current_proc]
    lldt [esp + 80]
    lea eax, [esp+76]
    mov dword [tss+4], eax
    ; 设置cr3为进程页表
re_enter:
    lock dec dword [k_reenter]
    pop gs 
    pop fs 
    pop es 
    pop ds 
    popad

    add esp, 8
    iretd
    