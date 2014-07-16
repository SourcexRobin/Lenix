/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: clock.c
//  创建时间: 2011-12-30        创建者: 罗斌
//  修改时间:                   修改者: 
//  主要功能: 提供基本的字符操作函数
//
//  说    明: 本文件提供了系统时钟、定时器功能。
//            系统时钟提供了获取开机时间、延时、设置时钟中断回调函数的功能。
//            定时器提供了创建、销毁功能。
//
//  变化记录:
//  版 本 号    |   时  间    |   作  者    |  主要变化记录
//=============================================================================
//              | 2011-12-30  |   罗  斌    |  
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>
#include <assert.h>
#include <lmemory.h>
#include <lio.h>

#include <proc.h>
#include <clock.h>

#ifdef _SLASH_
    #include <machine/machine.h>
#else
    #include <machine\machine.h>
#endif

volatile uint32_t           ticks;                 /*  时钟节拍计数器       */
static ticks_hook_t         clk_ticks_hook;        /*  时钟中断回调函数     */
static timer_t              timer_pool[TIMER_MAX]; /*  系统定时器池         */

#ifdef _CFG_SMP_
static spin_lock_t          ticks_lock;         /*  时钟节拍计数器互斥访问锁*/
static spin_lock_t          timer_lock;         /*  系统定时器池互斥访问锁  */
#endif  /*  _CFG_SMP_   */


#ifdef _CFG_TIMER_ENABLE_
/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Timer_create
//  作  用: 创建定时器。
//  参  数: 
//      millisecond : uint32_t      |   定时间隔，以毫秒为单位。在调用该函数后
//                                  | 的millisecond毫秒，调用定时器处理程序。
//      repeat      : int           |   重复次数。最大不能超过TIMER_REPEAT_MAX
//                                  | 次，Lenix定义TIMER_REPEAT_MAX为30000次。
//                                  | 如果需要创建无限定时器，则将该参数设置为
//                                  | TIMER_REPEAT_INFINITE。
//      handle      : void (*)(void *)
//                                  |   定时器处理程序。在定时完毕后调用。
//      param       : void *        |   定时器处理程序参数
//  返回值: 
//    类型: int
//    说明: 函数成功则返回定时器编号，失败返回0。
//
//  注  意:   修改系统时钟频率后，会导致原定时器定时变化，导致不可预料的情况，
//          因此在修改系统时钟频率以后，需要重建定时器。
//
//            定时器都是在时钟中断时调用，如果设置过多的定时器，会导致系统效率
//          降低。并且处理程序不应有等待、睡眠等时间不可预测的操作，也不应将耗
//          时操作放在处理程序内。换句话说，定时器处理程序应该尽量快速、短小。
//
//  变更记录:
//  时  间      |  作者         |  说明
//=============================================================================
// xxxx-xx-xx   |               |
///////////////////////////////////////////////////////////////////////////////
*/
int         Timer_create(uint32_t millisecond,int repeat,
                         void (*handle)(void *),void * param)
{
    static int              tmid    = 0;
    timer_t             *   tm      = NULL;
    int                     i       = 0;
    CRITICAL_DECLARE(timer_lock);

#ifdef CHECK_PARAMETER
    if( ticks <= 0 || NULL == handle)   /*  参数校验    */
        return 0;
#endif  /*  CHECK_PARAMETER */

    CRITICAL_BEGIN();

    for( i = 0 ; i < TIMER_MAX ; i++)
    {
        if( TIMER_IS_FREE(timer_pool + i) )
        {
            tm = timer_pool + i;
            tm->tm_handle = (tm_handle_t)(-1);
            break;
        }
    }
    if( tm )    /*  tm不为0，表示系统可以创建定时器     */
    {
create_timer_tmid:
        tmid++;
        if( tmid <= 0 ) tmid = 1;
        for( i = 0 ; i < TIMER_MAX ; i++)
        {
            if( !TIMER_IS_VALID(timer_pool + i) )
                continue;

            if( timer_pool[i].tm_id == tmid )
                goto create_timer_tmid;
        }

        /*
         *  对定时器进行设置，定时器的上限和重复次数都设置了上限
         */
        tm->tm_id       = tmid;
        tm->tm_ticks    = (int)MILIONSECOND_TO_TICKS(millisecond & 0x7FFFFFFF);
        tm->tm_left     = (int)tm->tm_ticks;
        tm->tm_repeat   = MIN(repeat,TIMER_REPEAT_MAX);
        tm->tm_param    = param;
        tm->tm_handle   = handle;
    }
    CRITICAL_END();

    return tm?tm->tm_id:(-1);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Timer_delete
//  作  用: 
//  参  数: 
//      id          : int           |   定时器编号
//  返回值: 成功返回RESULT_SUCCEED，失败返回RESULT_FAILED。
//  注  意: id不存在，就可能删除失败
//  变更记录:
//  时间        |  作  者       |  说明
//=============================================================================
//  xxxx-xx-xx  |               |
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Timer_delete(int id)
{
    result_t                ret     = RESULT_FAILED;
    int                     i       = 0;
    CRITICAL_DECLARE(timer_lock);

    CRITICAL_BEGIN();
    for( ; i < TIMER_MAX ; i++)
    {
        if( timer_pool[i].tm_id == id)
        {
            TIMER_ZERO(timer_pool + id);
            ret = RESULT_SUCCEED;
            break;
        }
    }
    CRITICAL_END();

    return ret;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Timer_handle
//  作  用: 定时器处理程序
//  参  数: 无
//  返回值: 无
//  注  意: 
//  变更记录:
//  时间        |  作  者       |  说明
//=============================================================================
//  xxxx-xx-xx  |               |
///////////////////////////////////////////////////////////////////////////////
*/
static 
void        Timer_handle(void)
{
    static int              watch   = 0;
    timer_t             *   tm      = NULL;
    int                     i       = 0;

    /*
     *  这里要注意: 
     *  在处理定时器的时候，如果定时器较多，且执行时间长，则有可能发生嵌套，
     *  因此应设置检测变量
     */
    if( Cpu_tas(&watch,0,1) == 0 )
        return ;
    for( i = 0 ; i < TIMER_MAX ; i++)
    {
        tm = timer_pool + i;
        /*  
         *  跳过无效的或者未到时的定时器
         */
        if( !TIMER_IS_VALID(tm) || --tm->tm_left > 0)       
            continue;
        tm->tm_handle(tm->tm_param);
        /*
         *  如果是无限重复定时器，重新设置定时间隔
         */
        if( TIMER_IS_INFINITE(tm) )                    
        {
            tm->tm_left = tm->tm_ticks;
            continue;
        }
        if( TIMER_CAN_REPEAT(tm) )
        {
            tm->tm_left = tm->tm_ticks;
            --tm->tm_repeat;
        }
        else
            TIMER_ZERO(tm);
    }
    watch = 0;
}

#endif  /*  _CFG_TIMER_ENABLE_   */

static 
void        Clk_default_ticks_hook(void)
{
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Clk_do_clock
//  作  用: 时钟中断处理程序
//  参  数: 无
//  返回值: 无
//  注  意:     时钟中断处理程序在每次时钟中断时都会被调用，而这个程序要处理进
//          程延时、回调函数、定时器、刷新进程的调度因子。如果系统进程较多、定
//          时器较而多且处理时间较长，且在中断返回前，还可能执行调度，将会导致
//          处理程序执行时间过长。
//
//          系统效率分析：
//              实时系统要求的时钟频率较高，因此将时钟频率假设为200Hz，也就是每
//          秒200次。CPU工作频率假设为60M，平均每条指令需要3个时钟周期，在这个
//          条件下，估算系统的基本开销。
//              每次时钟中断都需要执行5个任务。
//                1.处理进程延时。
//                2.时钟中断回调函数。
//                3.处理系统定时器。
//              在实际情况中，1、2项都是在有需要时，才产生执行，通常情况下，只
//          是一个简单的函数调用。在1、2项中安排任务，虽然是消耗CPU时间，但这是
//          功能需要，不算入系统的额外需求。
//              第3项，系统定时器处理。目前采用遍历的方式，但由于规定了定时器的
//          最大数目，而且这个数目非常小，默认为6个，遍历一次只需要执行几十条指
//          令，对系统效率没有影响。如果存在系统定时器，这已经是执行功能，是要
//          计入应用时间，因此第3项对系统的影响不大。
//              预计前3项总共要执行大约150条指令，耗时
//                ( 3 * 150 ) / 60M = 0.0000715 秒
//            
//  变更记录:
//  时  间      |  作  者       |  说明
//=============================================================================
//  xxxx-xx-xx  |               |
///////////////////////////////////////////////////////////////////////////////
*/
static
int         Clk_do_clock(int notuse1,int notuse2)
{
    --proc_current->proc_cpu_time;
    ++proc_current->proc_run_time;
    Proc_ticks();
    /*
     *  调用回调函数
     */
    clk_ticks_hook();
#ifdef _CFG_TIMER_ENABLE_
    /*  
     *  处理系统定时器
     */
    Timer_handle();
#endif  /*  _CFG_TIMER_ENABLE_   * /
    /*  
     *  进程时间消耗完后，重新计算调度算子并且标记为需要调度，在每次中断或者
     *  系统调用后，都会进行调度
     */
    if( proc_current->proc_cpu_time < 1 )
    {                    
        /*  避免某些编译器发出警告     */
        notuse1 = notuse1;
        notuse2 = notuse2;
        proc_current->proc_cpu_time = 0;
        proc_need_sched = 1;
    } 
    
    return 0;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Clk_get_ticks
//  作  用: 获得时钟节拍计数
//  参  数: 无
//  返回值: 
//    类型: uint32_t
//    说明: 时钟节拍计数
//  注  意: 在8位和16位情况下，需要使用临界段处理方式，在采用方式2的临界段保护
//          方式时获得的时间可能存在偏差。
//
//  变更记录:
//  时间        |  作  者       |  说明
//=============================================================================
//  xxxx-xx-xx  |               |
//
///////////////////////////////////////////////////////////////////////////////
*/
uint32_t    Clk_get_ticks(void)
{
#if _CFG_WORD_ == 8 || _CFG_WORD_ == 16
    uint32_t                ret     = 0;
    CRITICAL_DECLARE(NULL);

    CRITICAL_BEGIN();
    ret = ticks;
    CRITICAL_END();
    return ret;
#else
    return ticks;
#endif
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Clk_delay
//  作  用: 延时
//  参  数: 
//      millisecond         : uint32_t
//  返回值: 无
//
//  注  意: 忙等待延时，进程一直处于运行状态，在不被更高优先级进程打断的情况，
//          可以达到准确延时的效果。
//
//  变更记录:
//  时间        |  作  者       |  说明
//=============================================================================
//  xxxx-xx-xx  |               |
//
///////////////////////////////////////////////////////////////////////////////
*/
void        Clk_delay(uint32_t millisecond)
{
    uint32_t    start;
    
    millisecond = MILIONSECOND_TO_TICKS(millisecond);
    start   = Clk_get_ticks();
    while( Clk_get_ticks() - start < millisecond ) 
        ; 
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Clk_ticks_hook_get
//  作  用: 获得时钟中断回调函数
//  参  数: 
//      millisecond         : uint32_t
//  返回值: 无
//  注  意: 
//  变更记录:
//  时间        |  作  者       |  说明
//=============================================================================
//  xxxx-xx-xx  |               |
//
///////////////////////////////////////////////////////////////////////////////
*/
void *      Clk_ticks_hook_get(void)
{
    return clk_ticks_hook;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Clk_ticks_hook_set
//  作  用: 延时
//  参  数: 
//      millisecond         : uint32_t
//  返回值: 无
//  注  意: 
//  变更记录:
//  时间        |  作  者       |  说明
//=============================================================================
//  xxxx-xx-xx  |               |
//
///////////////////////////////////////////////////////////////////////////////
*/
void *      Clk_ticks_hook_set(ticks_hook_t  tickshook)
{
    void        *   ret    = NULL ;

    if( NULL == tickshook )
        return clk_ticks_hook;
    ret = clk_ticks_hook;
    clk_ticks_hook = tickshook;

    return ret;
}

void        Clk_initial(void)
{
    _printk("clock initial...    ");
#ifdef _CFG_SMP_
    timer_lock  = 0;
    ticks_lock  = 0;
#endif  /*  _CFG_SMP_   */
    ticks       = 0;
    _memzero(timer_pool,TIMER_MAX * sizeof( timer_t));
    clk_ticks_hook = Clk_default_ticks_hook;
    Machine_ivt_set(ISR_CLOCK,Clk_do_clock);
    _printk("OK!\n");
}