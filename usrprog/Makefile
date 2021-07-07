# Makefile for usrprog

ASM 		:= nasm
CC  		:= gcc
LD          := ld 

# 源文件
CSOURCE     := $(wildcard *.c )

# 目标文件(夹)
BUILD       := build
USRPROGS    := $(BUILD)/cat $(BUILD)/cp $(BUILD)/echo $(BUILD)/hexdump \
				$(BUILD)/ls $(BUILD)/mkdir $(BUILD)/pwd $(BUILD)/rm    \
				$(BUILD)/fcreate


CFLAGS		:= -m32 -c -fno-builtin -fno-stack-protector -nostdinc -g -O2 -I ../include/
ASMFLAGS	:= -f elf -g -F DWARF
LDFLAGS     := -Ttext 0x000 -m elf_i386

.PHONY: all clean

all: $(BUILD) $(USRPROGS)
	

clean:
	rm -rf $(BUILD)

$(BUILD):
	mkdir $@
$(BUILD)/start.o: start.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

$(BUILD)/cat.o: cat.c 
	$(CC) -o $@ $(CFLAGS) $<
$(BUILD)/cp.o: cp.c 
	$(CC) -o $@ $(CFLAGS) $<
$(BUILD)/echo.o: echo.c 
	$(CC) -o $@ $(CFLAGS) $<
$(BUILD)/hexdump.o: hexdump.c 
	$(CC) -o $@ $(CFLAGS) $<
$(BUILD)/ls.o: ls.c 
	$(CC) -o $@ $(CFLAGS) $<
$(BUILD)/mkdir.o: mkdir.c 
	$(CC) -o $@ $(CFLAGS) $<
$(BUILD)/pwd.o: pwd.c 
	$(CC) -o $@ $(CFLAGS) $<
$(BUILD)/rm.o: rm.c 
	$(CC) -o $@ $(CFLAGS) $<
$(BUILD)/fcreate.o: fcreate.c 
	$(CC) -o $@ $(CFLAGS) $<

$(BUILD)/cat: $(BUILD)/cat.o $(BUILD)/start.o ../build/qiux.a
	$(LD) -o $@ $(LDFLAGS) $^
$(BUILD)/cp: $(BUILD)/cp.o $(BUILD)/start.o ../build/qiux.a
	$(LD) -o $@ $(LDFLAGS) $^
$(BUILD)/echo: $(BUILD)/echo.o $(BUILD)/start.o ../build/qiux.a
	$(LD) -o $@ $(LDFLAGS) $^
$(BUILD)/hexdump: $(BUILD)/hexdump.o $(BUILD)/start.o ../build/qiux.a
	$(LD) -o $@ $(LDFLAGS) $^
$(BUILD)/ls: $(BUILD)/ls.o $(BUILD)/start.o ../build/qiux.a
	$(LD) -o $@ $(LDFLAGS) $^
$(BUILD)/mkdir: $(BUILD)/mkdir.o $(BUILD)/start.o ../build/qiux.a
	$(LD) -o $@ $(LDFLAGS) $^
$(BUILD)/pwd: $(BUILD)/pwd.o $(BUILD)/start.o ../build/qiux.a
	$(LD) -o $@ $(LDFLAGS) $^
$(BUILD)/rm: $(BUILD)/rm.o $(BUILD)/start.o ../build/qiux.a
	$(LD) -o $@ $(LDFLAGS) $^
$(BUILD)/fcreate: $(BUILD)/fcreate.o $(BUILD)/start.o ../build/qiux.a
	$(LD) -o $@ $(LDFLAGS) $^
