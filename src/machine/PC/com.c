/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenix实时操作系统
//                              2011 - 2012 @ 源代码工作室
//                                     保留所有版权
//                          ***----------------------------***
//       名  称 : com.c 
//     创建时间 : 2013-12-05       创建者  : 罗斌
//     修改时间 :                  修改者  : 罗斌
//
//     主要功能 : 提供com口的操作API
//
//      说  明  : 
//
//  版本变化记录:
//  版本号      |    时  间     |    作  者     | 主要变化记录
//========================================================================================
//  00.00.000   |  2013-12-05   |    罗  斌     | 第一版
//////////////////////////////////////////////////////////////////////////////////////////
*/

#include <type.h>
#include <kernel\tty.h>

/*  pc的4个串行口地址  */
uart_t                      pc_com[4] = {PC_UART0,PC_UART1,PC_UART2,PC_UART3};

/*  远程调试使用的串口 */
static int                  rs_rdb;

#define PC_COM1                     (pc_com)
#define PC_COM2                     (pc_com + 1)


#define RS_IRQ_FLAG(uart)           Io_inb(UART_IRQ_FLAG(uart))
#define RS_STATUS(uart)             Io_inb(UART_LINE_STATUS(uart))
#define RS_RESV(uart)               Io_inb(uart);Io_delay()
#define RS_SEND(uart,data)          Io_outb(uart,data)
#define RS_CAN_RESV(uart)           (RS_STATUS(uart) & UART_LS_DATA_RDY)
#define RS_CAN_SEND(uart)           (RS_STATUS(uart) & UART_LS_SEND_EMPTY)

#define RS_DLAB_SET(uart)           (Io_outb(UART_LINE_CTRL(uart),\
                                     0x80 | Io_inb(UART_LINE_CTRL(uart))))
#define RS_DLAB_RESET(uart)         (Io_outb(UART_LINE_CTRL(uart),\
                                     0x7F & Io_inb(UART_LINE_CTRL(uart))))
/*  串口波特率设置 */
int         Rs_botrate(uart_t uart,int rate)
{
    uart = uart;
    rate = rate;
    return 0;
}


int         Pc_rdb_set(int com)
{
    int ret = rs_rdb;
    rs_rdb = com & 3;
    return ret;
}

int         Pc_com_setup(int com,int bps)
{
    uart_t uart = pc_com[com & 3];

    RS_DLAB_SET(uart);
    Io_outb(uart,(byte_t)bps);
    Io_outb(UART_IRQ_ENABLE(uart),(byte_t)(bps>>8));
    RS_DLAB_RESET(uart);

    return com & 3;
}
size_t      Pc_com_resv(int com,void * buffer,size_t size)
{
    uart_t                  uart    = pc_com[com & 3];
    byte_t              *   buf     = buffer;
    size_t                  cnt     = 0;
    int                     retry   = 0;

    while( cnt < size )
    {
        retry = 1000;
        while( --retry && !RS_CAN_RESV(uart) )
            Io_delay();
        if( retry == 0 )
            break;
        *buf++ = RS_RESV(uart);
        cnt++;
    }

    return size;
}

size_t      Pc_com_send(int com,const void * buffer,size_t size)
{
    uart_t                  uart    = pc_com[com & 3];
    const byte_t        *   buf     = buffer;
    size_t                  cnt     = 0;
    int                     retry   = 1000;

    while( cnt < size )
    {
        retry = 1000;
        /*  重复测试一定次数  */
        while( --retry && !RS_CAN_SEND(uart) )
            Io_delay();
        /*  重试次数到达以后，终止发送  */
        if( retry == 0 )
            break;
        RS_SEND(uart,*buf++);
        /*  等待数据发送完毕  */
        while(!(RS_STATUS(uart) & UART_LS_SEND_SHIFT_EMPTY))
            ;
        cnt++;
    }

    return cnt;
}

void        Pc_tty_hook_rdb(byte_t c)
{
    if( rs_rdb ) Pc_com_send(rs_rdb,&c,1);
}

int _printf(const char * fmt,...);

/*  2013-12-06  */
int         Com1_do_irq(int data,int flags)
{
    //Tty_put_char(TTY_RDB,0);

    _printf("com1 irq flags: %02X\n",flags & 0xFF);

    switch( flags & 0xFE )
    {
    case UART_IE_RESV_RDY:
        _printf("com resive data: %X\n",(byte_t)data);
        break;
    case UART_IE_SEND_REG_EMPTY:
        _printf("com wakeup sender\n");
        break;
    }

    return 0;
}

void        Rs_initial(void)
{
    uart_t uart = pc_com[0];
    Pc_com_setup(0,RS_BPS_9600);
    Io_outb(UART_LINE_CTRL(uart),0x03);
    Io_outb(UART_IRQ_ENABLE(uart),0x0F);

    Machine_ivt_set(ISP_COM0,Com1_do_irq);
}
