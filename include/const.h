#ifndef QIUX_CONST_H_
#define QIUX_CONST_H_

// 显示颜色相关
// BL R G B I R G B
#define HIGHLIGHT   0x08
#define BLINK       0x80
#define BG_BLACK    0x00
#define BG_BLUE     0x10
#define BG_GREEN    0x20
#define BG_CYAN     0x30
#define BG_RED      0x40
#define BG_MAGENTA  0x50
#define BG_YELLOW   0x60
#define BG_WHITE    0x70
#define FG_BLACK    0x00
#define FG_BLUE     0x01
#define FG_GREEN    0x02
#define FG_CYAN     0x03
#define FG_RED      0x04
#define FG_MAGENTA  0x05
#define FG_YELLOW   0x06
#define FG_WHITE    0x07

// 保护模式相关
// 							    G	D/B	L	AVL		P	DPL	S	XEWA	XCRA
#define DA_32       0x4000  //	0	1	0	0		0	00	0	0000			D/B位为1，表示32位段
#define DA_LIMIT_4K 0x8000  //	1	0	0	0		0	00	0	0000			G位为1，表示段界限单位为4K

#define DA_DPL0     0x00    //	0	0	0	0		0	00	0	0000
#define DA_DPL1     0x20    //	0	0	0	0		0	01	0	0000
#define DA_DPL2     0x40    //	0	0	0	0		0	10	0	0000
#define DA_DPL3     0x60    //	0	0	0	0		0	11	0	0000

#define DA_DR       0x90    //	0	0	0	0		1	00	1	0000			TYPE都为0，只读数据段
#define DA_DRW      0x92    //	0	0	0	0		1	00	1	0010			W为1，读写数据段
#define DA_DRWA     0x93    //	0	0	0	0		1	00	1	0011			W为1，A为1，已访问读写数据段
#define DA_C        0x98    //	0	0	0	0		1	00	1			1000	X为1，代码段
#define DA_CR       0x9A    //	0	0	0	0		1	00	1			1010	X、R为1，可读代码段
#define DA_CCO      0x9C    //	0	0	0	0		1	00	1			1100	X、C为1，一致代码段
#define DA_CCOR     0x9E    //	0	0	0	0		1	00	1			1110	X、C、R为1，可读一致代码段

#define DA_LDT      0x82    //	0	0	0	0		1	00	0	0010			TYPE=2，LDT
#define DA_TaskGate 0x85    //	0	0	0	0		1	00	0	0101			TYPE=5，任务门
#define DA_386TSS   0x89    //	0	0	0	0		1	00	0	1001			TYPE=9，386TSS
#define DA_386CGate 0x8C    //	0	0	0	0		1	00	0	1100			TYPE=C，386调用门
#define DA_386IGate 0x8E    //	0	0	0	0		1	00	0	1110			TYPE=E，386中断们
#define DA_386TGate 0x8F    //	0	0	0	0		1	00	0	1111			TYPE=F，386陷阱门

#define SA_RPL0     0x0     //	0	00
#define SA_RPL1     0x1     //	0	01
#define SA_RPL2     0x2     //	0	10
#define SA_RPL3     0x3     //	0	11

#define SA_TIG      0x0     //	0	00
#define SA_TIL      0x4     //	1	00

// 中断向量号
#define	INT_VECTOR_DIVIDE_ERROR		            0x0
#define	INT_VECTOR_SINGLE_STEP_EXCEPTION		0x1
#define	INT_VECTOR_NMI			                0x2
#define	INT_VECTOR_BREAKPOINT_ECEPTION		    0x3
#define	INT_VECTOR_OVERFLOW		                0x4
#define	INT_VECTOR_BOUNDS_CHECK		            0x5
#define	INT_VECTOR_INVALID_OPCODE		        0x6
#define	INT_VECTOR_COPROC_NOT_AVAILABLE		    0x7
#define	INT_VECTOR_DOUBLE_FAULT		            0x8
#define	INT_VECTOR_COPROC_SEG_OVERRUN		    0x9
#define	INT_VECTOR_INVALID_TSS		            0xA
#define	INT_VECTOR_SEG_NOT_PRESENT		        0xB
#define	INT_VECTOR_STACK_EXCEPTION		        0xC
#define	INT_VECTOR_GENERAL_PROTECTION		    0xD
#define	INT_VECTOR_PAGE_FAULT		            0xE
#define	INT_VECTOR_COPROC_ERROR		            0x10
#define	INT_VECTOR_ALIGN_CHECK                  0x11
#define	INT_VECTOR_MACHINE_CHECK                0x12
#define	INT_VECTOR_SIMD_EXCEPTION               0x13

#define	INT_VECTOR_IRQ0			                0x20
#define	INT_VECTOR_IRQ8			                0x28

#define CLOCK_IRQ                               0x00
#define KEYBOARD_IRQ                            0x01
#define	CASCADE_IRQ	                            0x02

#define	AT_WINI_IRQ	                            0x0e	


#define INT_VECTOR_SEND         0x80
#define INT_VECTOR_RECEIVE      0x90

// eflags相关
#define MBS_FLAG                0x2     // 必须设置(must be set)
#define CARRY_FLAG              0x1     // 进位标志
#define PARITY_FLAG             0x4     // 奇偶标志
#define AUXILIARY_CARRY_FLAG    0x10    // 辅助进位标志
#define ZERO_FLAG               0x40    // 零标志
#define SIGN_FLAG               0x80    // 符号标志
#define TRAP_FLAG               0x100   // 陷阱标志(单步调试)
#define INTR_FLAG               0x200   // 中断许可标志
#define DIRECTION_FLAG          0x400   // 方向标志
#define OVERFLOW_FLAG           0x800   // 溢出标志
#define IOPL0_FLAG              0x0000  // IOPL0标志
#define IOPL1_FLAG              0x1000  // IOPL1标志
#define IOPL2_FLAG              0x2000  // IOPL2标志
#define IOPL3_FLAG              0x3000  // IOPL3标志
#define NESTED_FLAG             0x4000  // 嵌套标志
#define RESUME_FLAG             0x10000 // 恢复标志
#define VM_FLAG                 0x20000 // 虚拟8086标志
#define ALIGN_CHECK_FLAG        0x40000 // 对齐标志
#define VIF_FLAG                0x80000 // 虚拟中断标志
#define VIP_FLAG                0x100000 // 虚拟中断标志
#define ID_FLAG                 0x200000 // 判断CPU是否支持CPUID指令

#define MAG_CH_SMILE    '\001'
#define MAG_CH_SPADES	'\002'
#define MAG_CH_DIAMOND	'\003'
#define MAG_CH_CLUB     '\004'
#define MAG_CH_HEART    '\005'

// 标志输入、输出和错误文件描述符
#define	STDIN_FILENO	0
#define	STDOUT_FILENO	1
#define	STDERR_FILENO	2

// service的pid
#define TASK_SYS 0
#define TASK_IDE  1
#define TASK_FS  2
#define TASK_TTY 3
#define TASK_SHELL  4
#define TASK_IDLE 5

#define LDT_SIZE		        2
#define INDEX_LDT_C             0
#define INDEX_LDT_RW            1
#define ANY                     0xffffffff
#define INTERRUPT               0xfffffffe
#define NO_PROC                 0xfffffffd


#define SECTOR_SIZE		512
#define ZONE_SIZE 1024

#define FSEEK_CUR 0
#define FSEEK_SET 1
#define FSEEK_END 2

#endif // QIUX_CONST_H_
