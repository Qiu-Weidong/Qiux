%ifndef GDT_INC_
%define GDT_INC_

;							G	D/B	L	AVL		P	DPL	S	XEWA	XCRA
DA_32		EQU	0x4000	;	0	1	0	0		0	00	0	0000			D/B位为1，表示32位段
DA_LIMIT_4K	EQU	0x8000	;	1	0	0	0		0	00	0	0000			G位为1，表示段界限单位为4K

; 以下4个常量分别定义了不同的特权级
DA_DPL0		EQU	  0x00	;	0	0	0	0		0	00	0	0000			
DA_DPL1		EQU	  0x20	;	0	0	0	0		0	01	0	0000
DA_DPL2		EQU	  0x40	;	0	0	0	0		0	10	0	0000
DA_DPL3		EQU	  0x60	;	0	0	0	0		0	11	0	0000

; 以下7个常量P和S都为1，表示存在的代码数据段，TYPE取值不同
DA_DR		EQU	  0x90	;	0	0	0	0		1	00	1	0000			TYPE都为0，只读数据段
DA_DRW		EQU	  0x92	;	0	0	0	0		1	00	1	0010			W为1，读写数据段
DA_DRWA		EQU	  0x93	;	0	0	0	0		1	00	1	0011			W为1，A为1，已访问读写数据段
DA_C		EQU	  0x98	;	0	0	0	0		1	00	1			1000	X为1，代码段
DA_CR		EQU	  0x9A	;	0	0	0	0		1	00	1			1010	X、R为1，可读代码段
DA_CCO		EQU	  0x9C	;	0	0	0	0		1	00	1			1100	X、C为1，一致代码段
DA_CCOR		EQU	  0x9E	;	0	0	0	0		1	00	1			1110	X、C、R为1，可读一致代码段

; 以下6个常量的P位为1，表示存在，S位为0，表示系统段
DA_LDT		EQU	  0x82	;	0	0	0	0		1	00	0	0010			TYPE=2，LDT
DA_TaskGate	EQU	  0x85	;	0	0	0	0		1	00	0	0101			TYPE=5，任务门
DA_386TSS	EQU	  0x89	;	0	0	0	0		1	00	0	1001			TYPE=9，386TSS
DA_386CGate	EQU	  0x8C	;	0	0	0	0		1	00	0	1100			TYPE=C，386调用门
DA_386IGate	EQU	  0x8E	;	0	0	0	0		1	00	0	1110			TYPE=E，386中断们
DA_386TGate	EQU	  0x8F	;	0	0	0	0		1	00	0	1111			TYPE=F，386陷阱门
;----------------------------------------------------------------------------

;							TI	RPL
SA_RPL0		EQU	   0x0	;	0	00
SA_RPL1		EQU	   0x1	;	0	01
SA_RPL2		EQU	   0x2	;	0	10
SA_RPL3		EQU	   0x3	;	0	11

SA_TIG		EQU	   0x0	;	0	00
SA_TIL		EQU	   0x4	;	1	00
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
; 分页机制使用的常量说明
;----------------------------------------------------------------------------
PG_P		EQU	   0x1	; 页存在属性位
PG_RWR		EQU	   0x0	; R/W 属性位值, 读/执行
PG_RWW		EQU	   0x2	; R/W 属性位值, 读/写/执行
PG_USS		EQU	   0x0	; U/S 属性位值, 系统级
PG_USU		EQU	   0x4	; U/S 属性位值, 用户级
;----------------------------------------------------------------------------

; 宏 ---------------------------------------------------------------------
;
; 描述符
; usage: Descriptor Base, Limit, Attr
;        Base:  dd
;        Limit: dd (low 20 bits available)
;        Attr:  dw (lower 4 bits of higher byte are always 0)
%macro Descriptor 3
	dw	(%2) & 0xFFFF				; 段界限 1				(2 字节)
	dw	(%1) & 0xFFFF				; 段基址 1				(2 字节)
	db	((%1) >> 16) & 0xFF			; 段基址 2				(1 字节)
	dw	(((%2) >> 8) & 0xF00) | ((%3) & 0F0FFh)	; 属性 1 + 段界限 2 + 属性 2		(2 字节)
	db	((%1) >> 24) & 0xFF			; 段基址 3				(1 字节)
%endmacro ; 共 8 字节
;
; 门
; usage: Gate Selector, Offset, DCount, Attr
;        Selector:  dw
;        Offset:    dd
;        DCount:    db
;        Attr:      db
%macro Gate 4
	dw	((%2) & 0xFFFF)							; 偏移 1				(2 字节)
	dw	(%1)									; 选择子				(2 字节)
	dw	((%3) & 0x1F) | (((%4) << 8) & 0xFF00)	; 属性					(2 字节)
	dw	(((%2) >> 16) & 0xFFFF)					; 偏移 2				(2 字节)
%endmacro ; 共 8 字节

; Gdt
GDT_ENTRY:      Descriptor          0,          0,  0
DESC_FLAT_C:    Descriptor          0,    0xfffff,  DA_CR|DA_32|DA_LIMIT_4K
DESC_FLAT_RW:   Descriptor          0,    0xfffff,  DA_DRW|DA_32|DA_LIMIT_4K
DESC_VIDEO:     Descriptor    0xb8000,     0xffff,  DA_DRW|DA_DPL3              

GdtLen                      equ $ - GDT_ENTRY
GdtPtr                      dw GdtLen - 1
                            dd BaseOfLoaderPhyAddr + GDT_ENTRY

; 选择子
SelectorFlatC               equ DESC_FLAT_C - GDT_ENTRY
SelectorFlatRW              equ DESC_FLAT_RW - GDT_ENTRY
SelectorVideo               equ DESC_VIDEO - GDT_ENTRY + SA_RPL3

%endif ; GDT_INC_