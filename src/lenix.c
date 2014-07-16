/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenix实时操作系统
//                              2011 - 2012 @ 源代码工作室
//                                     保留所有版权
//                          ***----------------------------***
//       名  称 : lenix.c 
//     创建时间 :                  创建者  : 罗斌
//     修改时间 : 2012-11-27       修改者  : 罗斌
//
//     主要功能 : 提供从字符终端的输入输出功能，作为Lenix默认支持的硬件设备
//
//      说  明  : 
//
//  版本变化记录:
//  版本号      |    时  间     |    作  者     | 主要变化记录
//========================================================================================
//              |  2012-11-27   |    罗  斌     | 增加功能配置
//              |  2012-09-23   |    罗  斌     | 将文件名从main.c改为lenix.c
//  00.00.000   |               |    罗  斌     | 第一版的时间已经不记得，这是最早的文件
//////////////////////////////////////////////////////////////////////////////////////////
*/


#include <lenix.h>

/*  2012.12.08  */
void        Lenix_initial(void)
{
    Disable_interrupt();
    ++critical_nest;
    
    Con_cls();

    Cpu_initial();

    //Mmu_initial();

    Machine_initial();

    Proc_initial();

    Clk_initial();

#ifdef _CFG_MUTEX_ENABLE_
    Mutex_initial();
#endif /*   _CFG_MUTEX_ENABLE_  */

#ifdef _CFG_MESSAGE_ENABLE_
    Msg_initial();
#endif  /*  _CFG_MESSAGE_ENABLE_*/

#ifdef _CFG_TTY_ENABLE_
    Tty_initial();
#endif  /*  _CFG_TTY_ENABLE_    */

#ifdef _CFG_DEVICE_ENABLE_
    Dev_initial();
#endif  /*  _CFG_DEVICE_ENABLE_ */

#ifdef _CFG_SHELL_ENABLE_
    Shell_initial();
#endif  /*  _CFG_SHELL_ENABLE_  */
    _printk("Lenix initial OK!\n");
}

/*  2012.12.08  */
void        Lenix_start(void)
{
    --critical_nest;
    ASSERT(critical_nest == 0);
    
    Lenix_start_hook();

    _printk("Lenix start...\n");

    Enable_interrupt();

    PROC_NEED_SCHED();
}

