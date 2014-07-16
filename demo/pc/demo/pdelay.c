/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      Lenix
//                                                  嵌入式操作系统
//                                           2011 , 罗  斌，源代码工作室
//                                                   保留所有版权
//
//  文件名称    : pdelay.c
//  最后修改者  : 罗  斌
//  最后修改时间:
//
//  主要功能    : 测试延时功能
//
//  说明        : 给应用程序提供一个模板，使用时修改为将文件名修改为userapp.c即可
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
byte_t                      app_stack4[USER_APP_STACK];

void        app1(void * param)
{
    long i = 0;
    char    msg[32];
    *(int *)param = (int)param;
    for(;;)
    {
        sprintf(msg,"app1.%8ld \n",i++);
        Con_write_string(60,5,msg);

        if( 0 == (i % 10000) )
            Proc_delay(200);
    }
}

void        app2(void * param)
{
    long i = 0;
    char    msg[32];
    *(int *)param = (int)param;
    for(;;)
    {
        sprintf(msg,"app2.%8ld ",i++);
        Con_write_string(60,6,msg);

        if( 0 == (i % 10000) )
            Proc_delay(400);
    }
}

void        app3(void * param)
{
    long i = 0;
    char    msg[32];
    *(int *)param = (int)param;
    for(;;)
    {
        sprintf(msg,"app3.%8ld ",i++);
        Con_write_string(60,7,msg);

        if( 0 == (i % 10000) )
            Proc_delay(600);
    }
}

void        app4(void * param)
{
    long i = 0;
    char    msg[32];
    *(int *)param = (int)param;
    for(;;)
    {
        sprintf(msg,"app4.%8ld ",i++);
        Con_write_string(60,8,msg);

        if( 0 == (i % 10000) )
            Proc_delay(800);
    }
}

void        User_initial(void)
{
    Proc_create("app1",60,3,app1,0,
        MAKE_STACK(app_stack1,USER_APP_STACK),
        STACK_SIZE(app_stack1,USER_APP_STACK));
    Proc_create("app2",60,3,app2,0,
        MAKE_STACK(app_stack2,USER_APP_STACK),
        STACK_SIZE(app_stack2,USER_APP_STACK));
    Proc_create("app3",60,3,app3,0,
        MAKE_STACK(app_stack3,USER_APP_STACK),
        STACK_SIZE(app_stack3,USER_APP_STACK));
    Proc_create("app4",60,3,app4,0,
        MAKE_STACK(app_stack4,USER_APP_STACK),
        STACK_SIZE(app_stack4,USER_APP_STACK));
}