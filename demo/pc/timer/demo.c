/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenix实时操作系统
//                              2011 - 2012 @ 源代码工作室
//                                     保留所有版权
//                          ***----------------------------***
//       名  称 : democ.c 
//     创建时间 : 2013-01-01       创建者  : 罗斌
//     修改时间 : 2013-01-01       修改者  : 罗斌
//
//     主要功能 : 提供消息机制演示
//
//      说  明  : 
//
//  版本变化记录:
//  版本号      |    时  间     |    作  者     | 主要变化记录
//========================================================================================
//  00.00.000   |               |    罗  斌     | 第一版的时间已经不记得，这是最早的文件
//////////////////////////////////////////////////////////////////////////////////////////
*/
#include <lenix.h>

void        Clk_msg(void);

void        Timer1(void * param)
{
    static long             i           = 0;
    char                    msg[40]     = {0};

    param = param;
    _sprintf(msg,"timer 1 : %ld",i++);

    Con_write_string(40,14,msg,(int)i);
}

void        Timer2(void * param)
{
    static long             i           = 0;
    char                    msg[40]     = {0};

    param = param;
    _sprintf(msg,"timer 2 : %ld",i++);

    Con_write_string(40,12,msg,(int)i);
}


void        User_initial(void)
{
    Clk_ticks_hook_set(Clk_msg);

    Timer_create(300,-1,Timer1,NULL);
    Timer_create(500,-1,Timer2,NULL);
}

void        main(void)
{
    Lenix_initial();
    User_initial();
    Lenix_start();
}

