/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      Lenix
//                                                  嵌入式操作系统
//                                           2011 , 罗  斌，源代码工作室
//                                                   保留所有版权
//
//  文件名称    : mutex.c
//  文件创建者  : 
//  最后修改者  : 罗  斌
//  最后修改时间:
//
//  主要功能    :
//
//  说明        : 给应用程序提供一个模板，使用时修改为将文件名修改为userapp.c即可
//              : 演示用互斥对象保护公共变量
//
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//======================================================================================================================
//  00.00.000   |   2011-02-09  |  罗斌         |  xxxxxx
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
#include <stdio.h>
#include <lenix.h>

#define USER_APP_STACK              1024

byte_t                      app_stack1[USER_APP_STACK];
byte_t                      app_stack2[USER_APP_STACK];
byte_t                      app_stack3[USER_APP_STACK];

mutex_t                 *   mutex = NULL;
char                        msg[32];            /*   */

void        app1(void * param)
{
    long i = 0;
    param = param;
    for(;;)
    {
        Mutex_get(mutex);

        sprintf(msg,"app1.%8ld \n",i++);
        Con_write_string(60,5,msg);

        Clk_delay(1000);

        Mutex_put(mutex);

        Clk_delay(500);
    }
}

void        app2(void * param)
{
    long                    i = 0;
    char                    msg[32];
    param = param;
    while(1)
    {
        sprintf(msg,"app2 run: %8ld \n",i++);
        Con_write_string(60,6,msg);
        Proc_delay(20);
    }
}

void        app3(void * param)
{
    long                    i = 0;

    param = param;
    for(;;)
    {
        Mutex_get(mutex);

        sprintf(msg,"app3.%8ld ",i++);
        Con_write_string(60,7,msg);

        Mutex_put(mutex);
        
        Proc_delay(20);
    }
}

void        User_initial(void)
{
    if( NULL == ( mutex = Mutex_create() ) )
        Con_print_string("mutex create failed\n");

    Proc_create("app1",60,3,app1,0,
        MAKE_STACK(app_stack1,USER_APP_STACK),
        STACK_SIZE(app_stack1,USER_APP_STACK));

    Proc_create("app2",59,3,app2,0,
        MAKE_STACK(app_stack2,USER_APP_STACK),
        STACK_SIZE(app_stack2,USER_APP_STACK));

    Proc_create("app3",58,3,app3,0,
        MAKE_STACK(app_stack3,USER_APP_STACK),
        STACK_SIZE(app_stack3,USER_APP_STACK));
}