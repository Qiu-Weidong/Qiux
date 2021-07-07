%ifndef PM_INC_
%define PM_INC_

; 需要用到的常量

BaseOfBoot              equ 0x0 
OffsetOfBoot            equ 0x7c00 
BaseOfKernel            equ 0x6000                  ; kernel加载到的段地址
BaseOfLoaderPhyAddr     equ 0
OffsetOfKernel          equ 0x0                     ; kernel的偏移地址
TopOfStack              equ OffsetOfBoot

BaseOfInput             equ 0x0 
OffsetOfInput           equ 0x8000
BaseOfSecondBlock       equ 0x0  
OffsetOfSecondBlock     equ 0x8400 
ZoneSize                equ 1024 
; 0x8000到0x8400是输入缓存
; 0x8400到0x8800存放间接块

; disk_addr_packet地址在0x8800到0x8810
disk_addr_packet        equ 0x8800
packet_size             equ disk_addr_packet        ; xp /1bx 0x8800
trans_sector_nr         equ disk_addr_packet+2      ; xp /1bx 0x8802
dest_addr_offset        equ disk_addr_packet+4      ; xp /1hx 0x8804
dest_addr_seg           equ disk_addr_packet+6      ; xp /1hx 0x8806
lba_low                 equ disk_addr_packet+8      ; xp /1wx 0x8808
lba_high                equ disk_addr_packet+12     ; xp /1wx 0x880c
disk_addr_packet_size   equ 0x10

; super的存放位置 0x8810 0x8824
super_addr              equ 0x8810
ninodes                 equ super_addr
nzones                  equ ninodes+2
imap_blk                equ nzones+2
zmap_blk                equ imap_blk+2
fst_data_zone           equ zmap_blk+2
log_zone_size           equ fst_data_zone+2
max_size                equ log_zone_size+2
magic                   equ max_size+4
state                   equ magic+2
super_size              equ 20

; inode存放位置 0x8824 0x8844
inode_addr              equ 0x8824
mode                    equ inode_addr
uid                     equ mode+2
isize                   equ uid+2
mtime                   equ isize+4
gid                     equ mtime+4
nlinks                  equ gid+1
zone                    equ nlinks+1
inode_size              equ 0x20

; MemChkBuf 0x8900 0x8a00
; dwMCRNumber 0x88fc
dwMCRNumber             equ 0x88fc
MemChkBuf               equ 0x8900
%endif ; PM_INC_
