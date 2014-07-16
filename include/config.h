/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: config.h
//  创建时间: 2012-01-09        创建者: 罗斌
//  修改时间:                   修改者: 
//  主要功能: 提供Lenix最基本的配置
//
//  说    明: 本文件提供了系统时钟、定时器功能。
//            系统时钟提供了获取开机时间、延时、设置时钟中断回调函数的功能。
//            定时器提供了创建、销毁功能。
//
//  变化记录:
//  版 本 号    |   时  间    |   作  者    |  主要变化记录
//=============================================================================
//              | 2012-11-27  |   罗  斌    |  增加组件配置
//              | 2012-01-09  |   罗  斌    |  
///////////////////////////////////////////////////////////////////////////////
*/


#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <const.h>

#define VER_MAJOR                   1
#define VER_MINOR                   65
/*
 *  配置编译器
 *  Windows下使用‘\’作为路径分隔符，Linux、Unix等系统使用‘/’作为路径分隔符
 *  使用这个开关来适应不同的编译环境。定义了该宏则使用‘/’作为分隔符，适应于
 *  Unix、Linux下的编译。没有定义该宏，则适用于Windows下编译。框架为
#ifdef _SLASH_

#else

#endif  /*  _SLASH_ * /
 *
#define _SLASH_
/*****************************************************************************/

/*
 *  微软公司编译器的开关
 */
#define _CFG_MSVC_
/*****************************************************************************/

#ifdef _CFG_MSVC_
#define asm                         __asm
#define NAKED                       __declspec( naked )
#else
#define NAKED
#endif
/*
 *  定义CPU类型
 */
#define _CFG_CPU_X86_               0
#define _CFG_CPU_ARM_               1
#define _CFG_CPU_PPC_               2

#define _CFG_CPU_                   _CFG_CPU_X86_
/*
 *  编译器是否支持嵌入式汇编
 */
#define _CFG_ASM_

/*
 *  独立的IO地址空间
 */
#define _CFG_IO_SPACE_
/*****************************************************************************/

/*
 *  配置CPU字长，以及整型限度
 *  默认字长为32位
 */
#define _CPU_WORD_                  32

#if _CPU_WORD_ == 8

#define CPU_WORD                    8
#define INT_MIN                     0x80
#define INT_MAX                     0x7F
#define UINT_MAX                    0xFF

#elif _CPU_WORD_ == 16

#define CPU_WORD                    16
#define INT_MIN                     0x8000
#define INT_MAX                     0x7FFF
#define UINT_MAX                    0xFFFF

#elif _CPU_WORD_ == 64

#define CPU_WORD                    64
#define INT_MIN                     0x8000000000000000
#define INT_MAX                     0x7FFFFFFFFFFFFFFF
#define UINT_MAX                    0xFFFFFFFFFFFFFFFF

#else

#define CPU_WORD                    32
#define INT_MIN                     0x80000000
#define INT_MAX                     0x7FFFFFFF
#define UINT_MAX                    0xFFFFFFFF

#endif


/*
 *  配置目标机器
 */
#define _CFG_MACHINE_PC_

/*
 *  调试开关
 */
#define _CFG_DEBUG_
/*****************************************************************************/

/*
 *  配置CPU架构，默认为单CPU
 *
#define _CFG_SMP_ 
/*****************************************************************************/

/*
 *  临界段实现方法
 *  单CPU
 *  0 : 嵌套关中断，使用CPU提供的方法，保证嵌套后，仍可以维持原有状态
 *  1 : 简单关中断，直接使用CPU的关中断指令。
 *  2 : 禁止抢占
 *  SMP
 *  0 : 采用自旋锁
 *  1 : 未定义
 *  2 : 禁止抢占。尚未进行测试
 *
 *  默认为方法0。
 *    请注意，原则上要避免临界段嵌套。例如函数A中的临界段调用了函数B， 而函数B
 *  中也含有临界段。当然这不是绝对禁止.
 */

#define _CFG_CRITICAL_METHOD_       0
/*****************************************************************************/


#define INLINE                      inline

/*
 *
 
#define _CFG_MACHINE_INITIAL_HOOK_DEFAULT_
#define _CFG_MACHINE_CLOCK_FREQUENCY_HOOK_DEFAULT_
*/

/*
 *  栈方向开关
 *
 #define STACK_DIRECT_HIGH
/*****************************************************************************/

/*
 *  启用进程的用户扩展功能
 *
#define _CFG_PROC_USER_EXT_
/*****************************************************************************/

/*
 *  启用进程切换钩子
 *
#define _CFG_SWITCH_PREPARE_HOOK_
/*****************************************************************************/


/*
 *  栈检查开关
 */
#define CHECK_STACK
#define _CFG_CHECK_STACK_
/*****************************************************************************/

/*
 *  参数检查开关
 */
#define CHECK_PARAMETER    
#define _CFG_CHECK_PARAMETER_

/*  运行时检查开关  */
#define _CFG_RUN_TIME_CHECK_
/*****************************************************************************/


/*
 *  系统默认时钟频率 次/秒
 */
#define DEFAULT_CLOCK_FREQUENCY     1000

/*
 *  默认时间片50毫秒
 */
#define DEFAULT_CPU_TIME            50



#define IDLE_DEFAULT_STACK_BOTTOM   ((void *)(0xFC00))
#define IDLE_DEFAULT_STACK_SIZE     1020

#define SEMA_LIMIT                  30000
#define MSG_USER_MASK               0x8000
#define MSG_TAKE_TIMEOUT            100
#define MSG_TAKE_RETRY_TIMES        1
/*
 *  主TTY设备，系统默认的输入输出设备
 */
#define TTY_MAJOR                   0
/*
 *  远程调试TTY,Remote Debug
 */
#define TTY_RDB                     1
/*
 *  外壳进程的优先级和优先数，根据需要修改这两个参数
 */
#define SHELL_PROCESS_PRIORITY      32
#define SHELL_PROCESS_PRIONUM       3
/*
 *  系统最高计时精度，每秒10000次中断，即10000Hz。
 */
#define TIMING_ACCURACY             10000
#define MAX_CLOCK_FREQUENCY         TIMING_ACCURACY
#define MIN_CLOCK_FREQUENCY         20
/*
///////////////////////////////////////////////////////////////////////////////
///////////////////
//  系统空间的配置
#define EVENT_MAX                   64
#define USER_EVENT                  16
*/

#if _CPU_WORD_ == 16
#define KOUM_MAX                    128
#else
#define KOUM_MAX                    (1*K)
#endif  /*  _CPU_WORD_  */

/*
 *  系统默认的进程栈空间
 */
#define STACK_DEFAULT_SIZE          2048

/*
 *  系统最大进程数量
 *    默认是32个。如果需要更多的进程，修改这个数字即可，并不限制。这个数量涉及
 *  进程池的空间，请根据需要配置。请注意，默认的进程数量远远少于优先级的数量，
 */
#define PROC_MAX                    32

/*
 *  系统最大定时器数量
 *    默认是8个定时器。系统中的定时器不宜过多，因此默认的数量数量较少
 */
#define TIMER_MAX                   8

#define MUTEX_MAX                   32
#define SEMA_MAX                    32
#define MB_MAX                      24
#define SHELL_CMD_MAX               32

/*
 *  Lenix默认支持16个设备，如果需要支持更多地设备，可以修改这个常数
 */
#define DEVICE_MAX                  16

#define TTY_MAX                     2

#define BLKBUF_SIZE                 (32*BBUF_SIZE)
 /*
  *  配置需要使用的功能模块
  *  启用进程信号处理功能
  * /
#define _CFG_SIGNAL_ENABLE_
/*****************************************************************************/

/*
 *  启用定时器管理组件
 */
#define _CFG_TIMER_ENABLE_
/*****************************************************************************/

/*
 *  启用互斥对象管理组件
 */
#define _CFG_MUTEX_ENABLE_
/*****************************************************************************/

/*
 *  启用信号量管理组件
 */
#define _CFG_SEMAPHORE_ENABLE_
/*****************************************************************************/

/*
 *  启用消息管理组件
 *  在CPU字长大于8时，才允许启用消息组件
 */
#if _CPU_WORD_ > 8
#define _CFG_MESSAGE_ENABLE_
#endif 
/*****************************************************************************/

/*
 *  启用事件，尚未完成，请勿启用
 *
#define _CFG_EVENT_ENABLE_
/*****************************************************************************/

/*
 *  启用TTY组件
 */
#define _CFG_TTY_ENABLE_
/*****************************************************************************/

/*
 *  启用设备管理组件，
 */
#define _CFG_DEVICE_ENABLE_
/*****************************************************************************/

/*
 *  启用外壳进程
 *  依赖于tty组件
 */
#ifdef _CFG_TTY_ENABLE_
#define _CFG_SHELL_ENABLE_
#endif  /*  _CFG_TTY_ENABLE_    */
/*****************************************************************************/

#endif  /*  _CONFIG_H_  */