/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenix实时操作系统
//                              2011 - 2012 @ 源代码工作室
//                                     保留所有版权
//                          ***----------------------------***
//       名  称 : sys.c
//     创建时间 :                  创建者  : 罗斌
//     修改时间 : 2012-11-29       修改者  : 罗斌
//
//  主要功能    : 提供系统调用功能
//
//  说明        :
//
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//========================================================================================
//  00.00.000   |               |  罗斌         |  第一版不记得时间了
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
#include <config.h>
#include <lio.h>

#include <sys.h>
#include <proc.h>
#include <clock.h>

#ifdef _SLASH_


#else
    #include <machine\machine.h>
#endif  /*  _SLASH_ */

void *   syscall[SYSCALL_MAX] = {
    0
};


/*  显示提示信息后死机  */
void        Sys_halt(const char * msg)
{
    PROC_INC_SEIZE();

#ifdef _CFG_TTY_ENABLE_
    _printf("Lenix halt. hit: %s",msg);
#else
    msg = msg;
#endif

    Disable_interrupt();

    for(;;) Cpu_hlt();    
}

void        Syscall_exit(int type,int refresh)
{
    /*
     *  系统调用后都要进行信号处理
     */
#ifdef _CFG_SIGNAL_ENABLE_
    Signal_handle();
#endif  /*  _CFG_SIGNAL_ENABLE_ */

    /*
     *  根据退出类型执行不同的操作
     */
    if( SCEXIT_TYPE_IRQ == type )
    {
        /*
         *  中断退出才执行调度
         *  需要调度，可以抢占，不再中断处理内，才能调度
         */
        if( proc_need_sched ) 
            Proc_sched(refresh);
    }
}

void        Syscall_initial(void)
{
}