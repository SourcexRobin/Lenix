/*
////////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2012 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: proc.c
//  创建时间: 2011-07-02        创建者: 罗斌
//  修改时间: 2012-11-29        修改者: 罗斌
//  主要功能: 提供进程管理功能
//  说    明: 在16位与32位之间移植的时候，需要注意控制字符串的修改，主要是长整数
//            输出时，在%d与%ld选择。
//
//  变更记录:
//  版 本 号  |   时  间    |   作  者      | 主要变化记录
//==============================================================================
//  00.00.000 | 2011-07-02  |   罗  斌      | 第一版
////////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>

#ifdef _CFG_DEBUG_

#include <lio.h>

#ifdef _SLASH_
    #include <machine/machine.h>
    #include <machine/pc.h>
#else
    #include <machine\machine.h>
    #include <machine\pc.h>
    #include <kernel\clock.h>
    #include <kernel\proc.h>
#endif  /*  _SLASH_ */


/*
////////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_msg
//  作  用: 在控制台显示当前进程信息
//  参  数: 无
//  返回值: 无
//  注  意: 由于显示信息需要直接访问进程池，而进程池为定义为静态，所以将该函数放
//          置在本文件中，作为标准提供。
//
//  变更记录:
//  时  间      |  作者         |  说明
//==============================================================================
//  2012-01-01  |  罗斌         |  第一版
////////////////////////////////////////////////////////////////////////////////
*/
void        Proc_msg(void)
{
    char                    msg[64];    /*  显示字符串缓冲区                */
    proc_t              *   proc;
    int                     line;       /*  在屏幕上的行号                  */
    int                     i;
    
    line = 23;
    
    for( i = 0 , proc = Proc_pool(); i < PROC_MAX; i++,proc++)
    {
        if( proc->proc_entry )
        {
            _sprintf(msg,"pid:%3d prio:%2d sp:%P sf:%6d name: %s \n",
                proc->proc_pid,proc->proc_priority,proc->proc_sp,
                proc->proc_sched_factor,proc->proc_name);
            Con_write_string(25,line--,msg,TEXT_COLOR_GREEN);
        }
    }
}

void        Clk_msg(void)
{
    char                    msg[40];
    _sprintf(msg,"lenix ticks: %8d",Clk_get_ticks());
    Con_write_string(50,0,msg,0x07);
    Proc_msg();
}

/*  在Proc_sched中调用  */
void        Proc_sched_msg(void * p1,void *p2 )
{
    static uint32_t         cnt     = 0;
    char                    msg[40] = {0};

    p1 = p1;
    p2 = p2;
    _sprintf(msg,"sched times: %8d ",++cnt);

    Con_write_string(40,24,msg,TEXT_COLOR_RED|TEXT_COLOR_BLUE);
}



#endif  /*  _CFG_DEBUG_ */
