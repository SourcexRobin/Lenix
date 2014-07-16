/*  文件时间 2012.12.06   */

#include <lenix.h>
#include "bus_pci.h"

static int  Sc_mem          (int ,char **);
static int  Sc_cls          (int ,char **);
static int  Sc_com          (int ,char **);
static int  Sc_inb          (int ,char **);
static int  Sc_outb         (int ,char **);
static int  Sc_ind          (int ,char **);

void        Shell_cmd_initial(void)
{
    Shell_cmd_add("mem"     ,Sc_mem);
    Shell_cmd_add("cls"     ,Sc_cls);
    Shell_cmd_add("com"     ,Sc_com);
    Shell_cmd_add("inb"     ,Sc_inb);
    Shell_cmd_add("outb"    ,Sc_outb);
    Shell_cmd_add("ind"     ,Sc_ind);
}
/*
////////////////////////////////////////////////////////////////////////////////
////////////////////
//  以下为内部命令
*/



/*  2012.11.25 */
static int  Sc_mem          (int argc,char ** argv)
{
    const char          *   param   = NULL;
    void                *   m       = NULL;
    int                     size    = 256;

    param = Sc_get_param(argc,argv,'m');

    if( param )
        m = (void *)_atoh(param);

    param = Sc_get_param(argc,argv,'l');

    if( param )
    {
        size = _atoi(param);
        if( size > 256 )
            size = 256;
    }

    _mprintf(m,size);

    return 0;
}

/*  2012.11.25 */
static int  Sc_cls          (int argc,char ** argv)
{
    Con_cls();

    argc = argc;
    argv = argv;

    return 0;
}

/*2013.12.14  添加，用于测试VPC的com口 */
static int  Sc_com          (int argc,char ** argv)
{
    uart_t uart = PC_UART0;
    _printf("0: %02X\n",Io_inb(uart));
    _printf("1: %02X\n",Io_inb(UART_IRQ_ENABLE(uart)));
    _printf("2: %02X\n",Io_inb(UART_IRQ_FLAG(uart)));
    _printf("3: %02X\n",Io_inb(UART_LINE_CTRL(uart)));
    _printf("5: %02X\n",Io_inb(UART_LINE_STATUS(uart)));
    argc = argc;
    argv = argv;
    return 0;
}

/*2013.12.14  添加，向端口输入数据 */
static int  Sc_inb          (int argc,char ** argv)
{
    int                     ioaddr      = 0;
    byte_t                  data        = -1;
    const char          *   param       = NULL;
    
    if( argc > 1 )
    {
        param = Sc_get_param(argc,argv,'a');
        if( param )
            ioaddr = _atoh(param);
    }
    data = Io_inb((void *)ioaddr);
    _printf("port: %04X\ndata: %02X\n",ioaddr,data);

    return 0;
}

/*2013.12.14  添加，向端口输出数据 */
static int  Sc_outb         (int argc,char ** argv)
{
    int                     ioaddr      = 0;
    byte_t                  data        = -1;
    const char          *   param       = NULL;

    if( argc > 1 )
    {
        param = Sc_get_param(argc,argv,'a');
        if( param )
            ioaddr = _atoh(param);
        param = Sc_get_param(argc,argv,'d');
        if( param )
            data = (byte_t)_atoh(param);
        param = Sc_get_param(argc,argv,'c');
        if( param )
            data = (byte_t)(*param);
    }
    _printf("port: %04X\ndata: %02X\n",ioaddr,data);
    Io_outb((void *)ioaddr,data);

    asm int 024h
    return 0;
}

/*2013.12.17  添加，向端口输入数据 */
static int  Sc_ind          (int argc,char ** argv)
{
    int                     ioaddr      = 0;
    dword_t                 data        = -1;
    const char          *   param       = NULL;
    
    if( argc > 1 )
    {
        param = Sc_get_param(argc,argv,'a');
        if( param )
            ioaddr = _atoh(param);
    }
    data = Io_ind((void *)ioaddr);
    _printf("port: %04X\ndata: %08X\n",ioaddr,data);

    return 0;
}
