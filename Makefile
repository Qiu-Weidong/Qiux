# Makefile for Qiux

# 设置调试选项, True 或 False
DEBUG           := True
# 设置优化级别, -O0, -O1, -O2, -O3等
OPTIMIZATION    := -O2

# 编译、运行和调试工具
GDB		        := gdb
BOCHS		    := bochs
BXIMG		    := bximage
LD              := ld 
ASM 		    := nasm
CC  		    := gcc
AR              := ar 

# 源文件
CSOURCE         := $(wildcard device/*.c filesys/*.c kernel/*.c lib/*.c service/*.c)
ASMSOURCE       := $(wildcard device/*.asm filesys/*.asm kernel/*.asm lib/*.asm service/*.asm)
HEADER          := $(wildcard device/*.h filesys/*.h kernel/*.h lib/*.h service/*.h include/*.h)
BOOTSOURCE      := boot/boot.asm
USRSOURCE       := $(wildcard usrprog/*.c)

# 目标文件
BUILD           := build
IMG             := Qiux.img 
BOOT            := $(BUILD)/boot.bin
KERNEL          := $(BUILD)/kernel.bin
OBJS            := $(addprefix $(BUILD)/, $(notdir $(CSOURCE:.c=.o) $(ASMSOURCE:.asm=.o)))
USRLIB          := $(BUILD)/qiux.a 
USRPROG         := usrprog/build/cat usrprog/build/cp usrprog/build/echo \
					usrprog/build/hexdump usrprog/build/ls usrprog/build/fcreate \
					usrprog/build/mkdir usrprog/build/pwd usrprog/build/rm   

# 配置文件
BOCHSRC 	    := bochsrc

# 参数
GDBFLAGS	    := -q -ex "target remote localhost:1234" 		\
						-ex "set disassembly-flavor intel" 	    \
						-ex "set disassemble-next-line on"  
LDFLAGS		    := -Ttext 0x10400 -e kernel_main -m elf_i386

ifeq ($(DEBUG), True)
	ASMFLAGS	:= -f elf -g -F DWARF
	CFLAGS		:= -I include/ -m32 -c -fno-builtin -fno-stack-protector -nostdinc -g -D DEBUG $(OPTIMIZATION)
else
	ASMFLAGS	:= -f elf
	CFLAGS		:= -I include/ -m32 -c -fno-builtin -fno-stack-protector -nostdinc $(OPTIMIZATION)
endif

.PHONY: all clean rebuild debug fsck copy


all:$(BUILD) $(IMG) 
	
clean:
	rm -f $(IMG)
	rm -rf $(BUILD)
rebuild: clean all

fsck:$(IMG)
	fsck.minix -fsl $<
copy:$(IMG)
	cp $(IMG) /mnt/c/Users/Qiu/Documents/qiux/

# 请确保bochsrc文件中最后一行gdbstub:enabled=1 ... 被取消注释
debug:$(IMG) $(KERNEL)
	$(BOCHS) -f $(BOCHSRC) -q 2>/dev/null >/dev/null  &
	$(GDB) $(GDBFLAGS) $(KERNEL)

$(IMG): $(BOOT) $(KERNEL) $(USRPROG)
	rm -f *.img
	$(BXIMG) -q $@ -hd=10M -mode=create
	mkfs.minix -1 -n14 $@
	dd if=$(BOOT) of=$@ bs=512 count=2 conv=notrunc
	tool/tool $(KERNEL) $(USRPROG) $@


$(BOOT): $(BOOTSOURCE) boot/include/*
	$(ASM) -o $@ $< -I boot/include/


$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^
$(BUILD)/%.o:*/%.c $(HEADER)
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD)/%.o:*/%.asm
	$(ASM) $(ASMFLAGS) -o $@ $<
$(BUILD):
	mkdir $@
$(USRLIB): $(BUILD)/list.o $(BUILD)/stdio.o $(BUILD)/stdlib.o \
					$(BUILD)/string.o $(BUILD)/syscall.o $(BUILD)/debug.o $(BUILD)/ipc.o 
	$(AR) rcs $@ $^
$(USRPROG): $(USRLIB) $(USRSOURCE)
	$(MAKE) -C usrprog
