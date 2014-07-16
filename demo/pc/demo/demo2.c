
/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      Lenix
//                                                  嵌入式操作系统
//                                           2011 , 罗  斌，源代码工作室
//                                                   保留所有版权
//
//  文件名称    : demo1.c
//  最后修改者  : 罗  斌
//  最后修改时间:
//
//  主要功能    : 演示了Lenix系统多任务并行，以及相同优先级分配cpu时间的效果，在
//
//  说明        : 在使用时，将文件名改为userapp.c
//
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//======================================================================================================================
//  00.00.000   |   2011-12-31  |  罗斌         |  xxxxxx
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/

#include <lenix.h>

#define USER_APP_STACK              1024

byte_t                      app_stack1[USER_APP_STACK];
byte_t                      app_stack2[USER_APP_STACK];
byte_t                      app_stack3[USER_APP_STACK];
byte_t                      app_stack4[USER_APP_STACK];
byte_t                      app_stack5[USER_APP_STACK];
byte_t                      app_stack6[USER_APP_STACK];
byte_t                      app_stack7[USER_APP_STACK];

void        Con_print_char(byte_t c);
void        Clk_msg(void);

void        app1(void * param)
{
    char                    msg[80]     = {0};

    param = param; /*  避免编译器发出变量未使用警告  */


    for(;;)
    {
        Con_write_string(0,5," name          pid  pr  pn  run time  use  test");
        Con_write_string(0,6," ------------  ---  --  --  --------  ---  --------");
        _sprintf(msg,"system run time: %8ld",Clk_get_ticks());
        Con_write_string(0,24,msg);
        Proc_delay(1000);
    }
}

void        app2(void * param)
{
    long                    i           = 0;
    char                    msg[80]     = {0};
    char                    name[12]    = {0};
    int                     pid         = 0;

    Proc_delay(50);

    pid = Proc_get_pid();

    for(;;)
    {
        _sprintf(msg," %-12s  %3d  %2d  %2d  %8ld  %2d%%  %8ld",Proc_get_name(name),
            pid,Proc_get_priority(pid),Proc_get_prio_num(pid),
            Proc_get_run_time(),(int)(100*Proc_get_run_time()/Clk_get_ticks()),i++);
        Con_write_string(0,7 + (int)param,msg);
    }
}

void        User_initial(void)
{
    Tty_echo_hook_set(TTY_MAJOR,Con_print_char);

#ifdef _CFG_DEBUG_

    Clk_ticks_hook_set(Clk_msg);

#endif  /*  _CFG_DEBUG_    */

    Proc_create("app1",60,5,app1,0,
        MAKE_STACK(app_stack1,USER_APP_STACK),
        STACK_SIZE(app_stack1,USER_APP_STACK));
    Proc_create("app2",60,1,app2,0,
        MAKE_STACK(app_stack2,USER_APP_STACK),
        STACK_SIZE(app_stack2,USER_APP_STACK));
    Proc_create("app3",60,2,app2,(void *)1,
        MAKE_STACK(app_stack3,USER_APP_STACK),
        STACK_SIZE(app_stack3,USER_APP_STACK));
    Proc_create("app4",60,3,app2,(void *)2,
        MAKE_STACK(app_stack4,USER_APP_STACK),
        STACK_SIZE(app_stack5,USER_APP_STACK));
    Proc_create("app5",60,4,app2,(void *)3,
        MAKE_STACK(app_stack5,USER_APP_STACK),
        STACK_SIZE(app_stack5,USER_APP_STACK));
    Proc_create("app6",60,5,app2,(void *)4,
        MAKE_STACK(app_stack6,USER_APP_STACK),
        STACK_SIZE(app_stack6,USER_APP_STACK));
    Proc_create("app7",60,5,app2,(void *)5,
        MAKE_STACK(app_stack7,USER_APP_STACK),
        STACK_SIZE(app_stack7,USER_APP_STACK));
}