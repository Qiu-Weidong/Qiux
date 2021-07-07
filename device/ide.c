#include "../kernel/ipc.h"
#include "const.h"
#include "debug.h"
#include "__asm__.h"
#include "stdio.h"
#include "pic.h"
#include "ide.h"

#define REG_DATA 0x1F0
#define REG_FEATURES 0x1F1
#define REG_ERROR REG_FEATURES
#define REG_NSECTOR 0x1F2
#define REG_LBA_LOW 0x1F3
#define REG_LBA_MID 0x1F4
#define REG_LBA_HIGH 0x1F5
#define REG_DEVICE 0x1F6
#define REG_STATUS 0x1F7
#define REG_CMD REG_STATUS
#define REG_DEV_CTRL 0x3F6
#define REG_ALT_STATUS REG_DEV_CTRL
#define REG_DRV_ADDR 0x3F7
#define STATUS_BSY 0x80
#define STATUS_DRDY 0x40
#define STATUS_DFSE 0x20
#define STATUS_DSC 0x10
#define STATUS_DRQ 0x08
#define STATUS_CORR 0x04
#define STATUS_IDX 0x02
#define STATUS_ERR 0x01
#define ATA_IDENTIFY 0xEC
#define ATA_READ 0x20
#define ATA_WRITE 0x30
#define MAKE_DEVICE_REG(lba, drv, lba_highest) (((lba) << 6) | \
                                                ((drv) << 4) | \
                                                (lba_highest & 0xF) | 0xA0)
// 私有类型定义
typedef struct IDE_CMD
{
    uint8_t features;
    uint8_t count;
    uint8_t lba_low;
    uint8_t lba_mid;
    uint8_t lba_high;
    uint8_t device;
    uint8_t command;
} ide_cmd;

// 私有变量
private uint8_t ide_status;
private uint8_t hdbuf[SECTOR_SIZE * 2];

// 外部函数声明
public int intr_send(pid_t dest);

// 私有函数定义
private void ide_cmd_out(ide_cmd * cmd)
{
    // 向硬盘发送命令
    while(inb(REG_STATUS) & STATUS_BSY); // 如果硬盘处于bussy状态，则等待

	outb(REG_DEV_CTRL, 0);
	outb(REG_FEATURES, cmd->features);
	outb(REG_NSECTOR,  cmd->count);
	outb(REG_LBA_LOW,  cmd->lba_low);
	outb(REG_LBA_MID,  cmd->lba_mid);
	outb(REG_LBA_HIGH, cmd->lba_high);
	outb(REG_DEVICE,   cmd->device);
	outb(REG_CMD,     cmd->command);
}

private void print_identify_info(uint16_t *hdinfo)
{
    int i, k;
	char s[64];

	struct iden_info_ascii {
		int idx;
		int len;
		char * desc;
	} iinfo[] = {{10, 20, "HD SN"}, /* Serial number in ASCII */
		     {27, 40, "HD Model"} /* Model number in ASCII */ };

	for (k = 0; k < sizeof(iinfo)/sizeof(iinfo[0]); k++) {
		char * p = (char*)&hdinfo[iinfo[k].idx];
		for (i = 0; i < iinfo[k].len/2; i++) {
			s[i*2+1] = *p++;
			s[i*2] = *p++;
		}
		s[i*2] = 0;
		printf("{ IDE } %s: %s\n", iinfo[k].desc, s);
	}

	int capabilities = hdinfo[49];
	printf("{ IDE } LBA supported: %s\n",
	       (capabilities & 0x0200) ? "Yes" : "No");

	int cmd_set_supported = hdinfo[83];
	printf("{ IDE } LBA48 supported: %s\n",
	       (cmd_set_supported & 0x0400) ? "Yes" : "No");

	int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
	printf("{ IDE } HD size: %dMB\n", sectors * 512 / 1000000);
}


/*===========================================================================*
 *				ide_init				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 硬盘初始化
 * 执行进程 TASK_IDE
 * 调用函数 task_ide
 *****************************************************************************/
public void ide_init()
{
    uint8_t * pNrDrives = (uint8_t *)(0x475);
	printf("{ IDE } NrDrives:%d.\n", *pNrDrives);
	assert(*pNrDrives);
    idt_register(INT_VECTOR_IRQ0+AT_WINI_IRQ, (intr_handler)ide_handler);
	enable_irq(CASCADE_IRQ);
	enable_irq(AT_WINI_IRQ);
}

/*===========================================================================*
 *				ide_identify				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 打印硬盘参数
 * 执行进程 TASK_IDE
 * 调用函数 task_ide
 *****************************************************************************/
public void ide_identify(int drive)
{
    // drive表示驱动器号，0表示Master、1表示Slave
    ide_cmd cmd;
    cmd.command = ATA_IDENTIFY;
    cmd.device = MAKE_DEVICE_REG(0,drive,0);
    ide_cmd_out(&cmd);
    message_t msg;
    receive(INTERRUPT, &msg);

    // 从端口读入硬盘的执行结果,从0x01f0端口读取结果
    insw(REG_DATA, hdbuf, SECTOR_SIZE >> 1);

    print_identify_info((uint16_t *)hdbuf);
}

/*===========================================================================*
 *				ide_handler				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 硬盘中断处理程序
 * 内核执行
 *****************************************************************************/
public void ide_handler(const intr_frame * frame UNUSED, process_t * p_proc UNUSED)
{
    ide_status = inb(REG_STATUS);

    intr_send(TASK_IDE); 
}

/*===========================================================================*
 *				ide_read				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 读取硬盘
 * 执行进程 TASK_IDE
 * 调用函数 task_ide
 *****************************************************************************/
public void ide_read(int drive, int sector_nr, int n, void * buf)
{
    // 从drive盘当中，sector_nr扇区开始，读取n个扇区
    ide_cmd cmd;
    cmd.command = ATA_READ; // 读取
    cmd.features = 0;
    cmd.count = n;          // 读取n个扇区

    cmd.lba_low = sector_nr & 0xff;
    cmd.lba_mid = (sector_nr >> 8) & 0xff;
    cmd.lba_high = (sector_nr >> 16) & 0xff;
    cmd.device = MAKE_DEVICE_REG(1, drive, (sector_nr >> 24) & 0xf);

    ide_cmd_out(&cmd);
    
    message_t msg;
    while(n--)
    {
        receive(INTERRUPT, &msg);
        insw(REG_DATA, buf, SECTOR_SIZE / 2);
        buf += SECTOR_SIZE;
    }
    
}

/*===========================================================================*
 *				ide_write				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 向硬盘写入数据
 * 执行进程 TASK_IDE
 * 调用函数 task_ide
 *****************************************************************************/
public void ide_write(int drive, int sector_nr, int n, void * buf)
{
    ide_cmd cmd;
    cmd.command = ATA_WRITE; // 写如磁盘
    cmd.features = 0;
    cmd.count = n;          // 读取n个扇区

    cmd.lba_low = sector_nr & 0xff;
    cmd.lba_mid = (sector_nr >> 8) & 0xff;
    cmd.lba_high = (sector_nr >> 16) & 0xff;
    cmd.device = MAKE_DEVICE_REG(1, drive, (sector_nr >> 24) & 0xf);

    ide_cmd_out(&cmd);

    while(n--)
    {
        outsw(REG_DATA, buf, SECTOR_SIZE / 2);
        buf += SECTOR_SIZE;
        message_t msg;
        receive(INTERRUPT, &msg);
    }
    
}

