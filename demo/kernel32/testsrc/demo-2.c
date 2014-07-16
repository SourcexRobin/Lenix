/*
////////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//     名    称 : demo.c
//     创建时间 : 2014-02-02       创建者  : 罗斌
//     修改时间 :                  修改者  : 
//
//     主要功能 : 演示多进程能力
//
//     说    明 : 
//
//  版本变化记录:
//  版本号      |     时间      |  作  者       |  主要变化记录
//==============================================================================
//              |  2014-02-02   |  罗  斌       |  从模板复制修改
////////////////////////////////////////////////////////////////////////////////
*/

#include <lenix.h>

#define USER_APP_STACK              2048

byte_t                      app_stack1[USER_APP_STACK];
byte_t                      app_stack2[USER_APP_STACK];

void        Clk_msg(void);
void        User_initial(void);

void        app1(void * param)
{
    long                    i = 0;
    char                    msg[32];

    param = param;
    for(;;)
    {
        _sprintf(msg,"|||  app1.%8d \n",i++);
        Con_write_string(60,5,msg,(byte_t)i);
    }
}

void        app2(void * param)
{
    long                    i = 0;
    char                    msg[32];

    param = param;
    for(;;)
    {
        _sprintf(msg,"===  app2.%8d ",i++);
        Con_write_string(60,6,msg,(byte_t)i);
    }
}


int         main( int argc, char *argv[ ], char *envp[ ] )
{
    Tty_echo_hook_set(TTY_MAJOR,Con_print_char);
    Clk_ticks_hook_set(Clk_msg);

    __asm
    {
        mov eax,0xD0000000
        mov [eax],0
    }
    Proc_create("app1",60,3,app1,0,
        MAKE_STACK(app_stack1,USER_APP_STACK),
        STACK_SIZE(app_stack1,USER_APP_STACK));
    Proc_create("app2",60,3,app2,0,
        MAKE_STACK(app_stack2,USER_APP_STACK),
        STACK_SIZE(app_stack2,USER_APP_STACK));
    return 0;
}