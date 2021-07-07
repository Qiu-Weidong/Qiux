%ifndef STRING_INC_
%define STRING_INC_

; 字符串
bootMsg:            db "booting"
bootMsgLen          equ $ - bootMsg
kernel_bin:         db "kernel.bin" 
kernel_bin_len      equ $ - kernel_bin 
kernelNotFound      db "No Kernel"
kernelNotFoundLen   equ $ - kernelNotFound
loadMsg             db "loading"
loadMsgLen          equ $ - loadMsg

%endif ; STRING_INC_
