#include "shutdown.h"
#include "__asm__.h"
#include "stdio.h"
#include "debug.h"
#include "../kernel/interrupt.h"

#define CONTROL_REG 0x64

private inline void ACPI_power_off()
{
    // ACPI power-off;这条指令执行后直接就关机了
    // 通过ACPI来关机，这是真实物理机器的关机方式
    outw(0xb004, 0x2000);
}
private inline void request_shutdown_port()
{
    // bochs和qemu的关机方式，但物理机不支持
    const char *s = "Shutdown";
    for (const char *p = s; *p != '\0'; p++)
        outb(0x8900, *p);
}

/*===========================================================================*
 *				shutdown_poweroff				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 关机
 * 执行进程 TASK_SYS
 *****************************************************************************/
public void shutdown_poweroff()
{
    printf("Powering Off ...\n");
    ACPI_power_off();
    request_shutdown_port();

    NOT_REACHED;
    cli();
    hlt();
}

/*===========================================================================*
 *				shutdown_reboot				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 通过键盘控制器来进行重启, 需要在内核执行
 * 执行进程 TASK_SYS
 *****************************************************************************/
public void shutdown_reboot()
{
    printf("Rebooting ...\n");

    for (;;)
    {
        for (int i = 0; i < 0x10000; i++)
        {
            if ((inb(CONTROL_REG) & 0x02) == 0)
                break;
            for(int i=0;i<20;i++);
        }
        outb(CONTROL_REG, 0xfe);
    }
}

public void shutdown_sleep()
{
    printf("Sleeping ...\n");
    cli();
    hlt();
}
