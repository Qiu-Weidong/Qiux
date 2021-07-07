#include "type.h"
#include "__asm__.h"
#include "keyboard.h"
#include "debug.h"
#include "../kernel/proc.h"
#include "pic.h"

#define KEYBOARD_BUFFER_SIZE 16
#define KB_DATA 0x60
#define KB_CMD 0x64
#define LED_CODE 0xED
#define KB_ACK 0xFA
#define NR_SCAN_CODES 0x80
#define MAP_ROWS 3
// 私有类型定义
typedef struct s_keyboard_buffer
{
    uint32_t head;
    uint32_t tail;
    uint8_t buffer[KEYBOARD_BUFFER_SIZE];
} keyboard_buffer;

// 私有变量
private keyboard_buffer kbd_buf;
private bool_t shift_l, shift_r;
private bool_t alt_l, alt_r;
private bool_t ctrl_l, ctrl_r;
private bool_t caps_lock;
private bool_t num_lock;
private bool_t scroll_lock;
private uint16_t keymap[MAP_ROWS][NR_SCAN_CODES] = {
    {0, ESC, '1', '2', '3', '4', '5', '6',                                           /*0x0 ~ 0x7*/
     '7', '8', '9', '0', '-', '=', '\b', '\t',                                   /*0x8 ~ 0xf*/
     'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',                                         /*0x10 ~ 0x17*/
     'o', 'p', '[', ']', '\n', CTRL_L, 'a', 's',                                   /*0x18 ~ 0x1f*/
     'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',                                         /*0x20 ~ 0x27*/
     '\'', '`', SHIFT_L, '\\', 'z', 'x', 'c', 'v',                                   /* 0x28 ~ 0x2f */
     'b', 'n', 'm', ',', '.', '/', SHIFT_R, '*',                                     /* 0x30 ~ 0x37 */
     ALT_L, ' ', CAPS_LOCK, F1, F2, F3, F4, F5,                                      /*0x38 ~ 0x3f*/
     F6, F7, F8, F9, F10, NUM_LOCK, SCROLL_LOCK, PAD_HOME,                           /*0x40 ~ 0x47*/
     PAD_UP, PAD_PAGEUP, PAD_MINUS, PAD_LEFT, PAD_MID, PAD_RIGHT, PAD_PLUS, PAD_END, /*0x48~0x4f*/
     PAD_DOWN, PAD_PAGEDOWN, PAD_INS, PAD_DOT, 0, 0, 0, F11,                         /*0x50~0x57*/
     F12},
    {0, ESC, '!', '@', '#', '$', '%', '^',            /*0x0 ~ 0x7*/
     '&', '*', '(', ')', '_', '+', '\b', '\t',    /*0x8 ~ 0xf*/
     'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',          /*0x10 ~ 0x17*/
     'O', 'P', '{', '}', '\n', SHIFT_R, 'A', 'S',    /*0x18 ~ 0x1f*/
     'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',          /*0x20 ~ 0x27*/
     '\"', '~', SHIFT_L, '|', 'Z', 'X', 'C', 'V',     /* 0x28 ~ 0x2f */
     'B', 'N', 'M', '<', '>', '?', SHIFT_R, '*',      /* 0x30 ~ 0x37 */
     ALT_L, ' ', CAPS_LOCK, F1, F2, F3, F4, F5,       /*0x38 ~ 0x3f*/
     F6, F7, F8, F9, F10, NUM_LOCK, SCROLL_LOCK, '7', /*0x40 ~ 0x47*/
     '8', '9', '-', '4', '5', '6', '+', '1',          /*0x48 ~ 0x4f*/
     '2', '3', '0', '.', 0, 0, 0, F11,                /*0x50~0x57*/
     F12},
    {
        0, 0, 0, 0, 0, 0, 0, 0,                     /*0x0~0x7*/
        0, 0, 0, 0, 0, 0, 0, 0,                     /*0x8~0xf*/
        0, 0, 0, 0, 0, 0, 0, 0,                     /*0x10~0x17*/
        0, 0, 0, 0, PAD_ENTER, 0, 0, 0,             /*0x18~0x1f*/
        0, 0, 0, 0, 0, 0, 0, 0,                     /*0x20~0x27*/
        0, 0, 0, 0, 0, 0, 0, 0,                     /*0x28~0x2f*/
        0, 0, 0, 0, 0, PAD_SLASH, 0, 0,             /*0x30~0x37*/
        0, 0, 0, 0, 0, 0, 0, 0,                     /*0x38~0x3f*/
        0, 0, 0, 0, 0, 0, 0, HOME,                  /*0x40~0x47*/
        UP, PAGEUP, 0, LEFT, 0, RIGHT, 0, END,      /*0x48~0x4f*/
        DOWN, PAGEDOWN, INSERT, DELETE, 0, 0, 0, 0, /*0x50~0x57*/
        0, 0, 0, WIN, GUI_R, APPS                   /*0x58~0x5f*/
    }

};

// 私有函数定义
private void kb_wait()
{
    uint8_t stats;
    do
    {
        stats = inb(KB_CMD);
    } while (stats & 0x02);
}
private void kb_ack()
{
    uint8_t ack;

    do
    {
        ack = inb(KB_DATA);
    } while (ack != KB_ACK);
}
private void set_leds()
{
    uint8_t leds = (caps_lock << 2) | (num_lock << 1) | scroll_lock;
    kb_wait();
    outb(KB_DATA, LED_CODE);
    kb_ack();

    kb_wait();
    outb(KB_DATA, leds);
    kb_ack();
}

/*===========================================================================*
 *				kbuf_getc				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 从键盘缓冲区读出扫描码，如果没有则阻塞进程
 * 执行进程 TASK_TTY
 * 调用函数 keyboard_parse
 *****************************************************************************/
private uint8_t kbuf_getc()
{
    message_t msg;
    while(kbuf_empty()) receive(INTERRUPT, &msg);

    uint8_t scan_code = kbd_buf.buffer[kbd_buf.head];
    kbd_buf.head = (kbd_buf.head + 1) % KEYBOARD_BUFFER_SIZE;
    return scan_code;
}

/*===========================================================================*
 *				kbuf_putc				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 将扫面码放入键盘缓冲区
 * 内核执行
 * 调用函数 keyboard_handler
 *****************************************************************************/
private void kbuf_putc(uint8_t byte)
{
    if(kbuf_full()) return;
    kbd_buf.buffer[kbd_buf.tail] = byte;
    kbd_buf.tail = (kbd_buf.tail + 1) % KEYBOARD_BUFFER_SIZE;
}

/*===========================================================================*
 *				kbd_init				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 键盘初始化
 * 执行进程 TASK_TTY
 * 调用函数 tty_init
 *****************************************************************************/
public void kbd_init()
{
    kbd_buf.head = kbd_buf.tail = 0;

    shift_l = shift_r = 0;
    alt_l = alt_r = 0;
    ctrl_l = ctrl_r = 0;

    caps_lock = 0;
    num_lock = 1;
    scroll_lock = 0;

    set_leds();
    idt_register(INT_VECTOR_IRQ0+1, (intr_handler)keyboard_handler);
    enable_irq(1);
}

/*===========================================================================*
 *				kbuf_empty				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 判断键盘缓冲区是否为空
 * 执行进程 TASK_TTY
 * 调用函数 tty_dev_read、kbuf_getc
 *****************************************************************************/
public bool_t kbuf_empty()
{
    return kbd_buf.head == kbd_buf.tail;
}

/*===========================================================================*
 *				kbuf_full				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 判断键盘缓冲区是否已满
 * 内核执行
 * 调用函数 kbuf_putc
 *****************************************************************************/
public bool_t kbuf_full()
{
    return (kbd_buf.tail + 1) % KEYBOARD_BUFFER_SIZE == kbd_buf.head;
}

/*===========================================================================*
 *				keyboard_handler				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 键盘中断处理程序
 * 内核执行
 *****************************************************************************/
public void keyboard_handler(const intr_frame * frame UNUSED, process_t * p_proc UNUSED)
{
    uint8_t scan_code = inb(KB_DATA);
    kbuf_putc(scan_code);
    intr_send(TASK_TTY);// 告诉task_tty有输入了
}

/*===========================================================================*
 *				keyboard_parse				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 解析键盘输入
 * 执行进程 TASK_TTY
 * 调用函数 tty_dev_read
 * 返回值低8位为ANSI码(或者相应控制码)，
 * 高八位含义如下所示
 * 8   7    6      5      4     3      2       1
 * ext make ctrl_r ctrl_l alt_r alt_l  shift_r shift_l 
 *****************************************************************************/
public uint16_t keyboard_parse()
{

    uint8_t scan_code;
    uint16_t result = 0;

    scan_code = kbuf_getc();// 如果没有数据，这里会阻塞，直到有数据为止

    int caps = shift_r || shift_l; // 通过shift来判断是否大写
    int row;
    if (caps_lock && keymap[0][scan_code & 0x7f] >= 'a' && keymap[0][scan_code & 0x7f] <= 'z')
        row = !caps;
    else
        row = caps;
    assert(row == 0 || row == 1);

    if (scan_code == 0xe0)
    {
        assert(!kbuf_empty());
        scan_code = kbuf_getc();
        row = 2;
    }
    bool_t make = (scan_code & FLAG_BREAK ? false : true);

    result = keymap[row][scan_code & 0x7f];
    switch (result)
    {
    case SHIFT_L:
        shift_l = make;
        break;
    case SHIFT_R:
        shift_r = make;
        break;
    case CTRL_L:
        ctrl_l = make;
        break;
    case CTRL_R:
        ctrl_r = make;
        break;
    case ALT_L:
        alt_l = make;
        break;
    case ALT_R:
        alt_l = make;
        break;
    case CAPS_LOCK:
        if (make)
        {
            caps_lock = !caps_lock;
            set_leds();
        }
        break;
    case NUM_LOCK:
        if (make)
        {
            num_lock = !num_lock;
            set_leds();
        }
        break;
    case SCROLL_LOCK:
        if (make)
        {
            scroll_lock = !scroll_lock;
            set_leds();
        }
        break;
    default:
        break;
    }

    result |= ((shift_l << 8) + (shift_r << 9) + (alt_l << 10) + (alt_r << 11) +
               (ctrl_l << 12) + (ctrl_r << 13) + (make << 14));

    return result;
}

