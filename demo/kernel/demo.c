/*
////////////////////////////////////////////////////////////////////////////////
//                              Lenix实时操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: demo.c 
//  创建时间:                   创建者: 罗斌
//  修改时间: 2012-12-10        修改者: 罗斌
//  主要功能: 测试内核
//  说    明: 
//
//  变更记录:
//  版 本 号:   |   时  间   |   作  者     | 主要变化记录
//==============================================================================
//              | 2012-12-10 |   罗  斌     | 
////////////////////////////////////////////////////////////////////////////////
*/
#include <lenix.h>

void        Con_print_char(byte_t c);
void        Clk_msg(void);
void        Shell_cmd_initial(void);

size_t      Pc_com_send(int com,const void * buffer,size_t size);
void        Cpu_initial(void)
{
}
void        User_initial(void)
{
    Tty_echo_hook_set(TTY_MAJOR,Con_print_char);

    Clk_ticks_hook_set(Clk_msg);

    Shell_cmd_initial();
}


void        main(void)
{
    Lenix_initial();

    User_initial();

    Lenix_start();

    for(;;) Cpu_hlt();
}
