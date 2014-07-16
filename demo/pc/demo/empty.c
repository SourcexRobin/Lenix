/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      Lenix
//                                                  嵌入式操作系统
//                                           2011 , 罗  斌，源代码工作室
//                                                   保留所有版权
//
//  文件名称    : empty.c
//  文件创建者  : 
//  最后修改者  : 罗  斌
//  最后修改时间:
//
//  主要功能    :
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

void        app1(void * param)
{
    long i = 0;
    char    msg[32];
    *(int *)param = (int)param;
    for(;;)
    {
        sprintf(msg,"app1.%8ld \n",i++);
        Con_write_string(60,5,msg);
        Proc_delay(20);
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
        Proc_delay(30);
    }
}

void        User_initial(void)
{
}