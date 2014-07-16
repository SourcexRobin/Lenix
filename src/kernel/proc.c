/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: proc.c
//  创建时间: 2011-07-02        创建者: 罗斌
//  修改时间: 2012-11-29        修改者: 罗斌
//  主要功能: 提供进程管理功能。
//  说    明: 
//
//  变更记录:
//  版 本 号    |     时间      |  作  者       |  主要变化记录
//=============================================================================
//  00.00.000   |  2011-07-02   |  罗  斌       |  第一版
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>
#include <const.h>
#include <koum.h>
#include <result.h>
#include <assert.h>

#include <lstring.h>
#include <lmemory.h>
#include <lio.h>

#include <proc.h>

#ifdef _SLASH_
/*  为非windows平台准备 */
    #include <machine/machine.h>
#else
    #include <machine\machine.h>
#endif

proc_t                  *   proc_current;           /*  当前进程，单CPU成立 */
volatile uint_t             proc_need_sched;        /*  需要调度标志        */

#ifdef _CFG_SMP_
static spin_lock_t          proc_rspl_lock;         /*  用以保护优先级列表  */
static spin_lock_t          proc_pool_lock;         /*  用以保护进程池      */
static spin_lock_t          proc_delay_lock;        /*  延时列表锁          */

#define proc_prio_lock              proc_rspl_lock
#define proc_lock                   proc_pool_lock
#endif

/*
 *  进程优先数
 */
const static byte_t         proc_priority_number[6] = {3,3,7,15,31,63};

/*
 *  运行态进程列表
 *  Running Statu Process List，简写为RSPL
 */
static proc_t           *   proc_rspl[PROC_PRIORITY_MAX + 1];   

/*
 *  2013.11.26
 *  优先级位图，
 */
static uint16_t				proc_prio_map[4];
/*
 *  进程池，Lenix 从进程池中分配进程对象  
 */
static proc_t               proc_pool[PROC_MAX];     

/*  
 *  空闲进程，永远处于运行状态
 *  用以保证系统中没有可运行的进程时，调度程序仍然可以找到可运行的进程
 */
static proc_t           *   proc_idle;

/*  
 *  Lenix 进程
 */
/*static proc_t           *   proc_lenix;*/

/*  延迟等待列表*/
static proc_list_t          proc_delay;
int                         proc_cpu_time;

#define FIRST_PROC          (proc_pool + 1)

extern void main(void);
extern void Proc_sched_msg(void *,void *);

#ifdef _CFG_DEBUG_

proc_t  *   Proc_pool(void )
{
    return proc_pool;
}
#endif  /*  _CFG_DEBUG_ */

/*
//////////////////////////////////////////////////////////////////////////////
///////////////////
//  进程管理内部使用的函数，
*/
/*  2012.11.17  */
static
byte_t      Proc_prio_num(byte_t prionum)
{
    int                     i       = 1;

    for( ; i < 6 ; i++)
        if( prionum == proc_priority_number[i] )
            return i;

    return PROC_INVALID_PRIONUM;
}
/*  2012.11.17 由宏调整为函数  */
static
void        Proc_list_add(proc_list_t * pl,proc_t * proc)
{
    if( NULL == pl->pl_list )
    { 
        proc->proc_sched_prev           = NULL;
        proc->proc_sched_next           = NULL;
        pl->pl_list                     = proc ;
    }
    else
    { 
        proc->proc_sched_prev           = NULL; 
        proc->proc_sched_next           = pl->pl_list;
        pl->pl_list->proc_sched_prev    = proc;
        pl->pl_list                     = proc;
    }
    proc->proc_wait     = pl ;
}

/*  2012.11.17 由宏调整为函数  */
static
void        Proc_list_del(proc_list_t * pl,proc_t * proc)
{
    ASSERT(pl);
    
    /*
     *  等于链表的头节点，删除头节点即可
     */
    if( proc == pl->pl_list )
    {
        pl->pl_list = proc->proc_sched_next;
        if( pl->pl_list )
            pl->pl_list->proc_sched_prev = NULL;
    }
    else
    {
        /*
         *  必定存在前导节点。
         */
        proc_t      *   prev    = proc->proc_sched_prev,
                    *   next    = proc->proc_sched_next;
        
        prev->proc_sched_next = next;
        if( next )
            next->proc_sched_prev = prev;
    }
    proc->proc_sched_prev   = NULL;
    proc->proc_sched_next   = NULL;
    proc->proc_wait         = NULL;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_get
//  作  用: 通过pid获得进程对象。
//  参  数: 
//      pid         : int           |   进程编号
//  返回值: 存在则返回非0值，否则返回NULL。
//  注  意: 
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static 
proc_t  *   Proc_get(int pid)
{
    register proc_t     *   proc        = NULL;

    /*
     *  遍历进程池
     */
    for( proc = proc_pool ; proc < &proc_pool[PROC_MAX] ; proc++)           
    {
        if( proc->proc_entry && pid == proc->proc_pid )                      
            return proc;
    }

    return NULL;
}

#ifdef _CFG_SIGNAL_ENABLE_

/*
 *  2012.12.01
 */
static void Proc_signal_initial(proc_t * proc)
{
    proc->proc_signal_handle[ 0]    = Signal_kill;
    proc->proc_signal_handle[ 1]    = Signal_default;
    proc->proc_signal_handle[ 2]    = Signal_default;
    proc->proc_signal_handle[ 3]    = Signal_default;
    proc->proc_signal_handle[ 4]    = Signal_default;
    proc->proc_signal_handle[ 5]    = Signal_default;
    proc->proc_signal_handle[ 6]    = Signal_default;
    proc->proc_signal_handle[ 7]    = Signal_default;
    proc->proc_signal_handle[ 8]    = Signal_default;
    proc->proc_signal_handle[ 9]    = Signal_default;
    proc->proc_signal_handle[10]    = Signal_default;
    proc->proc_signal_handle[11]    = Signal_default;
    proc->proc_signal_handle[12]    = Signal_default;
    proc->proc_signal_handle[13]    = Signal_default;
    proc->proc_signal_handle[14]    = Signal_default;
    proc->proc_signal_handle[15]    = Signal_default;
}
#endif  /*  _CFG_SIGNAL_ENABLE_ */

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_release
//  作  用: 释放进程对象。
//  参  数: 
//      proc        : proc_t *  +   进程对象指针
//  返回值: 永远返回RESULT_SUCCEED
//  注  意:   进程自行退出，或者强制退出时,由系统完成对象的释放。KOUM仅是释放
//          了系统对象表的空间。因此在创建完进程后，如果确定不需要使用句柄，可
//          以立即释放。
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2014-02-05  |  罗斌         |  引入KOUM时增加该函数
///////////////////////////////////////////////////////////////////////////////
*/
static 
result_t    Proc_release(proc_t * proc)
{
    return RESULT_SUCCEED;
}
/*
//////////////////////////////////////////////////////////////////////////////
///////////////////
//  内核、驱动API
*/

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Sched_add
//  作  用: 将进程加入RSPL中，参与CPU竞争。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      proc_t *    |    proc       |   进程对象指针
//  返回值: 无。
//  注  意: 内核使用的函数，参数可以直接使用内核对象指针。应用程序避免使用
//
//  变更记录:
//  时间        |   作  者      |  说明
//=============================================================================
//  2013-11-29  |   罗  斌      |  引入优先级位图
//  2012-01-02  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Sched_add(proc_t * proc)
{
    int             prio        = proc->proc_priority;
    CRITICAL_DECLARE(proc_rspl_lock);

    CRITICAL_BEGIN();
                                        
    if( NULL == proc_rspl[prio] ) 
    {   
        proc_rspl[prio]         = proc;
        proc->proc_sched_prev   = NULL;
        proc->proc_sched_next   = NULL;
    } 
    else
    { 
        proc->proc_sched_next   = proc_rspl[prio]; 
        proc->proc_sched_prev   = NULL; 
        proc_rspl[prio]->proc_sched_prev  = proc; 
        proc_rspl[prio]                   = proc;
    }
    proc->proc_wait = NULL ;
    proc->proc_stat = PROC_STAT_RUN;
    
    /*
     *    引入优先级位图。将进程加入RSPL后，对应的优先级必然存在进程，因此可直
     *  接将优先级位图的对应位置1，表示对应优先级已经存在进程。
     */
    proc_prio_map[prio / 16] |= 1 << (prio & 0xF);

    CRITICAL_END();
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Sched_del
//  作  用: 将进程从RSPL中删除，不再参与CPU竞争。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      proc_t *    |    proc       |   进程对象指针
//  返回值: 无。
//  注  意: 内核使用的函数，参数可以直接使用内核对象指针。应用程序避免使用
//
//  变更记录:
//  时间        |   作  者      |  说明
//=============================================================================
//  2013-11-29  |   罗  斌      |  引入优先级位图
//  2012-01-02  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Sched_del(proc_t * proc)
{
    int             prio        = proc->proc_priority;
    CRITICAL_DECLARE(proc_rspl_lock);
    
    CRITICAL_BEGIN();

    if( proc_rspl[prio]  )
    { 
        /*  删除进程，维护进程列表    */
        if( NULL == proc->proc_sched_prev ) 
        {
            proc_rspl[prio] = proc->proc_sched_next; 
            if( proc_rspl[prio] ) 
                proc_rspl[prio]->proc_sched_prev = NULL; 
        }
        else if( NULL == proc->proc_sched_next ) 
        {
            proc->proc_sched_prev->proc_sched_next = NULL; 
        }
        else 
        { 
            proc->proc_sched_prev->proc_sched_next = proc->proc_sched_next; 
            proc->proc_sched_next->proc_sched_prev = proc->proc_sched_prev; 
        }
    }

    /*
     *    将进程从进程列表中删除时，并不代表列表中没有进程。只有在优先级中没有
     *  进程时，才需要将优先级位图中对应的位清0 。
     */
    if( proc_rspl[prio] == NULL )
        proc_prio_map[prio / 16] &= !( 1 << ( prio & 0xF ));

    CRITICAL_END();

    proc->proc_sched_prev = NULL;
    proc->proc_sched_next = NULL;
    proc->proc_wait       = NULL;
    proc->proc_stat       = -1;
}


/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_wakeup_proc
//  作  用: 唤醒指定进程，无论其当前状态如何。
//  参  数: 
//      proc        : proc_t        |   需要唤醒的进程对象
//  返回值: 无
//  注  意:   1.使用该函数时，务必确保对象有效。
//            2.请谨慎使用该函数，由于该函数是直接唤醒进程，可能会造成不可预料
//          的结果。
//  变更记录:
//  时  间      |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_wakeup_proc(proc_t * proc)
{
    /*
     *  不能唤醒无效或者空闲对象，目前死机处理
     */
    if( PROC_IS_INVALID(proc) || PROC_IS_FREE(proc) )                  
        Sys_halt("try to wakeup invalid process\n");
        
    if( proc->proc_stat == PROC_STAT_RUN )
        return ;

    proc->proc_stat = PROC_STAT_RUN;
    
    if( proc->proc_wait )
    {
        /*
         *  存在隐患
         */
        Proc_list_del(proc->proc_wait,proc);
        
        proc->proc_wait = NULL;
    }

    Sched_add(proc);

    if( proc->proc_priority < proc_current->proc_priority )
        PROC_NEED_SCHED();
    
    SCHED(0);        
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_wait_on
//  作  用: 在列表上等待。
//  参  数: 
//      proclist    : proc_list_t * |   进程列表
//  返回值: 无
//  注  意:    1、这是内核使用的函数，应用程序避免使用。
//             2、该函数仅是完成将自身从RSPl中删除，加入到进程列表中。并未执行
//          调度。因此调用该函数后，必须由调用者调用调度函数。
//             3、必须明确的唤醒。
//  变更记录:
//  时  间      |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_wait_on(proc_list_t * proclist)
{
    ASSERT(  proclist );
    /*
     * 系统进程进入睡眠状态，说明有错，死机
     */
    if( proc_current == proc_idle )
        Sys_halt("Lenix try to sleep.");
    Sched_del(proc_current);
    proc_current->proc_wait = proclist;
    proc_current->proc_stat = PROC_STAT_WAIT;
    Proc_list_add(proclist,proc_current); 
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_resume_on
//  作  用: 唤醒列表中所有的进程。
//  参  数: 
//      proclist    : proc_list_t * |   进程列表
//  返回值: 无
//  注  意:    1、该函数是内核用函数，应用程序不应使用。
//             2、函数仅是将进程列表中的进程全部添加入RSPL，并未调度。需要调用
//          者在调用该函数后调用调度函数.
//  变更记录:
//  时  间      |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_resume_on(proc_list_t * proclist)
{
    proc_t        * proc    = NULL,
                  * wakeup  = NULL;
    
    ASSERT(proclist );

#ifdef _CFG_RUN_TIME_CHECK_
    /*  运行时校验  */
    if( NULL == proclist )
        return ;
#endif  /*  _CFG_RUN_TIME_CHECK_ */
    proc = proclist->pl_list;
    while( proc )                /*  唤醒列表中所有的进程                */
    {
        wakeup  = proc;
        proc    = proc->proc_sched_next;
        wakeup->proc_sched_next     = NULL;
        wakeup->proc_wait           = NULL;
        wakeup->proc_stat           = PROC_STAT_RUN;
        Sched_add(wakeup);
        if( wakeup->proc_priority <= proc_current->proc_priority )
            PROC_NEED_SCHED();
    }
    proclist->pl_list = NULL;
}


/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_resume_max_on
//  作  用: 唤醒列表中优先级最高、调度因子最大的进程。
//  参  数: 
//      proclist    : proc_list_t * |   进程列表
//  返回值: 无
//
//  注  意:    1、该函数是内核用函数，应用程序不应使用。
//             2、函数仅是将进程列表中优先级最高且调度因子最大的进程加入RSPL，
//          并未调度。需要调用者在调用该函数后调用调度函数.
//  变更记录:
//  时  间      |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_resume_max_on  (proc_list_t * proclist)
{
    proc_t       *  proc    = NULL,     /*  需要唤醒的进程      */
                 *  next    = NULL;     /*  循环算子            */

    ASSERT(proclist);

    proc = proclist->pl_list;

#ifdef _CFG_RUN_TIME_CHECK_
    /*  运行时校验  */
    if( NULL == proclist )
        return ;
#endif  /*  _CFG_RUN_TIME_CHECK_ */
    if( NULL == proc )
        return ;
    /*
     *  查找列表中优先级最高且调度因子最大的进程
     */
    for( next = proc->proc_sched_next; next ; next = next->proc_sched_next)
    {
        /*
         *  跳过优先级低的进程
         */
        if( next->proc_priority > proc->proc_priority )
            continue;

        /*
         *  发现优先级更高的进程
         */
        if( next->proc_priority < proc->proc_priority )
        {
            proc = next;
            continue;
        }

        /*
         *  到达这里，说明进程优先级相同，则挑选调度因子高的进程
         */
        if( next->proc_sched_factor > proc->proc_sched_factor)
            proc = next;
    }
    Proc_list_del(proclist,proc);
#ifdef d_CFG_DEBUG_
    _printk("resume proc name : %s \n",proc->proc_name);
#endif
    Sched_add(proc);
    if( proc->proc_priority < proc_current->proc_priority)
        PROC_NEED_SCHED();
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_wait
//  作  用: 当前进程在单独的指针上等待。
//  参  数: 
//      proc        : proc_t **     |   需要恢复的进程指针的指针
//  返回值: 无
//  注  意:    1、该函数是内核用函数，应用程序不应使用。
//             2、调用该函数后，进程对象的状态是等待态，但并未在任何列表上等待
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_wait(proc_t ** proc)
{
    /*
     *  如果已经有进程在对象上等待，系统逻辑存在错误，死机
     */
    if( NULL == proc || *proc )
        Sys_halt("parameter error or has process wait on!");
    *proc = proc_current;
    /*
     *  保证操作连续，在SMP条件下并不能保证互斥。
     */
    PROC_SEIZE_DISABLE();
    Sched_del(proc_current);    /*  已经包含临界段控制      */
    proc_current->proc_stat = PROC_STAT_WAIT;
    PROC_SEIZE_ENABLE();
    Proc_sched(0);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_resume
//  作  用: 恢复调用Proc_wait进入等待的进程。
//  参  数: 
//      proc        : proc_t **     |   需要恢复的进程指针的指针
//  返回值: 无
//  注  意:   只能恢复由调用Proc_wait进入等待的进程，不能恢复由其他方式进入等待
//          的进程。
//
//  变更记录:
//  时  间      |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_resume(proc_t ** proc)
{
    proc_t        * p   = NULL;

    if( NULL == proc || NULL == (p = *proc) )
        Sys_halt("process resume error!");
    /*
     *  不是等待状态或者在其他对象上等待      
     */
    if( PROC_STAT_WAIT != p->proc_stat || p->proc_wait  )
        return ;
    PROC_SEIZE_DISABLE();
    Sched_add(p);    /*  已经包含临界段控制      */
    p->proc_stat = PROC_STAT_RUN;
    PROC_SEIZE_ENABLE();
    if( p->proc_priority < proc_current->proc_priority )
        PROC_NEED_SCHED();
    *proc = NULL;
    SCHED(0);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_ticks
//  作  用: 刷新处于延时状态的进程的计数器。
//  参  数: 无
//  返回值: 无
//  注  意:    1、该函数是内核用函数，在时钟中断处理程序中使用。应用程序不应使
//          用。
//             2、应确保每次时钟中断都调用该函数，否则将会导致进程延时不准确。
//             3、进程有信号或者延时时间到，都会被唤醒。应此并不一定会延时到预
//          定的时间。
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_ticks(void)
{
    proc_t       * proc = NULL,
                 * next = NULL;
    CRITICAL_DECLARE(proc_delay_lock);

    proc = proc_delay.pl_list;

    CRITICAL_BEGIN();
    while(proc)
    {
        next = proc->proc_sched_next;
        /*
         *  进程收到信号或者延时时间到，都将被唤醒 
         */
        if( proc->proc_signal_map || --proc->proc_alarm <= 0 )              
        {
            Proc_list_del(&proc_delay,proc);
            /*
             *    将proc加入RSPL后，原proc的链表数据就会被新数据代替。因此需要
             *  增加一个next变量来保存原有的链表指针。
             */
            Sched_add(proc);
            /*
             * 有进程进入运行态，且优先级高于当前进程，需要发出调度请求
             */
            if( proc->proc_priority < proc_current->proc_priority )
                PROC_NEED_SCHED();              
        }
        proc = next;
    }
    CRITICAL_END();
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_switch_prepare
//  作  用: 完成进程切换。
//  参  数: 
//      sp          : void *        +   当前进程栈指针。该指针在PROC_SWITCH_TO
//                                  | 宏中提供，PROC_SWITCH_TO是一个CPU相关的
//                                  | 宏，因此该宏是移植的一个重点
//      next        : proc_t *      +   下一个要运行的进程
//  返回值: 返回需要运行进程的栈指针，然后从返回的指针中恢复进程的运行环境
//  注  意: 该函数在PROC_SWITCH_TO宏内调用。
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void    *   Proc_switch_prepare(void * sp,proc_t * next)
{

#ifdef _CFG_SWITCH_PREPARE_HOOK_
    Proc_switch_prepare_hook(proc_current,next);
#endif  /*  _CFG_SWITCH_PREPARE_HOOK_   */

    /*
     *    在进行进程切换时，当前进程有可能已经无效。在进程退出后，就会出现这样
     *  的情况如果此时还要保存指针，将会已经释放的对象的数据
     */
    if( proc_current->proc_entry)
        proc_current->proc_sp = sp;

#ifdef _CFG_CHECK_STACK_
    do
    {
        char    str[64];

        /*  检查是否存在栈越界                  */
        if( proc_current->proc_entry && !STACK_CHECK(proc_current) )
        {
            _sprintf(str,"proc:%s stack overflow! sp:%p bottom:%p\n",
                proc_current->proc_name,proc_current->proc_sp,
                proc_current->proc_stack_bottom);
            Sys_halt(str);
        }
    }while(0);
#endif  /*  _CFG_CHECK_STACK_   */

    proc_current = next;
    //printf("current proc; %s\n",proc_current->proc_name);
    return proc_current->proc_sp;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_refresh_sched_factor
//  作  用: 刷新进程调度因子。
//  参  数: 无
//  返回值: 无
//  注  意: 
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//              |  罗斌         |  时间不记得了
///////////////////////////////////////////////////////////////////////////////
*/
static
void        Proc_refresh_sched_factor(void)
{
    int                     i       = 1;
    proc_t              *   proc    = FIRST_PROC;

    for( i = 1 ; i < PROC_MAX ; i++,proc++)
    {
        if( proc->proc_entry && proc->proc_sched_factor < 0xFFFFFF00 )
            proc->proc_sched_factor += proc->proc_prio_num;
    }
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_initial
//  作  用: 初始化进程管理模块。
//  参  数: 无
//  返回值: 无
//  注  意: 只能在Lenix初始化的时候调用一次。
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_initial(void)
{
    _printk("process initial...   ");
    _memzero(proc_pool,PROC_MAX*sizeof(proc_t));
    _memzero(proc_rspl,sizeof(proc_t *) * (PROC_PRIORITY_MAX + 1));

#ifdef _CFG_SMP_
    proc_rspl_lock      = 0;
    proc_pool_lock      = 0;
    proc_delay_lock     = 0;
#endif      /*  _CFG_SMP_   */
    proc_need_sched     = 0;
    proc_current        = proc_pool;
    proc_idle           = proc_pool;
    _strcpy(proc_idle->proc_name,"idle");
    proc_idle->proc_entry           = (proc_entry_t)main;
    proc_idle->proc_stat            = PROC_STAT_RUN;
    proc_idle->proc_stack_bottom    = IDLE_DEFAULT_STACK_BOTTOM;
    proc_idle->proc_stack_size      = IDLE_DEFAULT_STACK_SIZE;
    proc_idle->proc_priority        = PROC_PRIORITY_MAX;   /*将优先级设为最低*/
    proc_rspl[PROC_PRIORITY_MAX]    = proc_idle;
    /*
     *  系统中第一个内核对象
     */
    Koum_add(proc_idle,Proc_release,kot_proc,HANDLE_ATTR_RDWR);
    _printk("OK!\n");
}

/*  
 *    信号在每次中断、系统调用结束前进行处理，在编程时，就要注意在中断以及系统
 *  调用结束前增加相应的处理代码。
 */
void        Signal_handle(void)
{
    int                     i           = 0;
    uint_t                  signal      = 0;

    /*
     *  没有信号就直接退出
     */
    if( 0 == proc_current->proc_signal_map )
        return ;

#ifdef _CFG_SIGNAL_ENABLE_
    for( i = 15 ; i >= 0 ; i-- )
    {
        signal = PROC_MAKE_SIGNAL(i);
        if( proc_current->proc_signal_map & signal )
        {
            proc_current->proc_signal_map &= ~signal;   /*  清信号位 */
            proc_current->proc_signal_handle[i]();
            return ;                                    /*每次只处理一个信号*/
        }
    }
#else
    i       = i;
    signal  = signal;
    proc_current->proc_signal_map = 0;
#endif
}


/*
//////////////////////////////////////////////////////////////////////////////
///////////////////
//  以下是用户API
*/

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称:   Spin_lock
//  作  用:   自旋锁。用于无睡眠（等待）互斥访问临界资源。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//    spin_lock_t*  |   sl          |   自旋锁对象指针
//  返回值:   无
//  说  明:   必须与Spin_free成对使用。在函数对之间的代码应保证短小、快速，并且
//          不能有主动放弃CPU的操作。也主要应用在短时互斥资源的访问上。
//            自旋锁不存在优先级反转情况，上锁前，已经确保进程不会放弃CPU，因此
//          必然持续执行而等待的进程也在持续执行，在当前进程释放锁以后，等待进
//          程自然可以获得锁。
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2012-06-26  |   罗  斌      |  Cpu_tas 函数返回值变化
//  2012-01-01  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Spin_lock(spin_lock_t * sl)
{
    /*
     *  多CPU时，保证进程一直可以占用CPU
     */
    PROC_SEIZE_DISABLE();

    while( Cpu_tas((int *)sl,LOCK_STATUS_FREE,LOCK_STATUS_LOCKED) ) 
        ;
}

void        Spin_free(spin_lock_t * sl)
{
    *sl = LOCK_STATUS_FREE;

    PROC_SEIZE_ENABLE();
}


/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_create
//  作  用: 创建一个进程对象。
//  参  数: 
//      name        : const char *  +   进程名，不能超过12个字符，含字符串结尾
//                                  | 的0。
//      priority    : byte_t        +   进程优先级，优先级最高为0，最低为63。
//                                  | Lenix进程较为特殊，优先级为64。在创建进程
//                                  | 时，对该参数取64的余数，保证优先级有效。
//      prionum     : byte_t        +   进程优先数。Lenix将优先数分为5个等级，
//                                  | 1-5。Lenix会自动加参数进程处理，保证其在
//                                  | 正确范围内。
//      entry       : proc_entry_t  +   进程入口点。其原型为
//                                  | void Proc_entry(void * param);
//      param       : void *        +   进程参数，以指针方式提供。
//      stack       : void *        +   进程栈顶指针。可通过MAKE_STACK宏得到。
//      stacksize   : int           +   栈容量。可通过STACK_SIZE宏获得。
//  返回值: 成功返回非NULL，失败返回INVALID_HANDLE。
//  注  意:   1.在中断处理期间函数返回失败
//            2.创建进程后，需要显式释放句柄。如果在程序其它地方不需要使用句
//          柄，应在创建进程后，立即释放句柄，否则会导致系统的内核对象表资源泄
//          露。
//          
//
//  变更记录:
//  时  间      |   作  者      |  说  明
//=============================================================================
//  2014-02-16  |   罗  斌      |  根据KOUM修改
//  2014-02-05  |   罗  斌      |  修改返回值类型
//  2012-01-02  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
handle_t    Proc_create(const char    * name,
                        byte_t          priority,
                        byte_t          prionum,
                        proc_entry_t    entry,
                        void          * param,
                        void          * stack,
                        int             stacksize)
{
    static int      pid     = 0;
    handle_t        hnd     = INVALID_HANDLE;
    proc_t        * proc    = FIRST_PROC;
    uint_t        * sp      = NULL;
    int             i       = 1;
    CRITICAL_DECLARE(proc_pool_lock);

    ASSERT( NULL != name    );
    ASSERT( NULL != entry   );
    ASSERT( NULL != stack   );
    ASSERT( 0 != stacksize  );

#ifdef _CFG_RUN_TIME_CHECK_
    /*  运行期间参数校验    */
    if( NULL == name || NULL == entry || NULL == stack || 0 == stacksize )  
        goto proc_create_end;
#endif  /*  _CFG_RUN_TIME_CHECK_   */
    if( interrupt_nest )    /*  中断服务处理期间，不能创建进程     */
        goto proc_create_end;
    /*
     *    以下临界段涉及可用进程对象查找和可用进程编号查找，耗时可能较长。
     *  因此不要在实时性要求高的程序中创建进程。
     */
    CRITICAL_BEGIN();
    /*  遍历进程池，查找可用的进程对象 */
    for( ; i < PROC_MAX ; i++,proc++)
    {   
        if( PROC_IS_FREE(proc) )
        {
            proc->proc_entry = entry;   /*  将进程对象置为已占用    */
            break;
        } 
    }
    if( i >= PROC_MAX )
    {
        /*
         *  进程池已满，不能创建进程
         */
        CRITICAL_END();
        goto proc_create_end;
    }
create_pid:
    /*
     *  给新进程分配唯一标识符
     */
    if( ++pid < 1 ) pid = 1;            /*  0是系统进程的PID    */
    for( i = 1 ; i < PROC_MAX ; i++)    /*  检测PID是否存在重复 */
    {
        /*
         *  跳过空闲的进程对象，由于自身的PID仍为0，而系统不会产生0的编号，
         *  所以不用理会自身
         */
        if( PROC_IS_FREE(&proc_pool[i]) )
            continue;
        if( pid == proc_pool[i].proc_pid )  /*  出现重复的pid，则重新生成*/
            goto create_pid;
    }
    proc->proc_pid = pid;
    CRITICAL_END();
    sp      = Context_initial(entry,param,stack);   /*  初始化进程运行环境  */
    _nstrcpy(proc->proc_name,name,11);              /*  复制进程名称        */
    proc->proc_sched_prev           = NULL;
    proc->proc_sched_next           = NULL;
    proc->proc_wait                 = NULL;
    proc->proc_sp                   = sp;
    proc->proc_stack_size           = stacksize;
    proc->proc_stack_bottom         = STACK_BOTTOM(stack,stacksize);
    proc->proc_stat                 = PROC_STAT_RUN;
    proc->proc_cpu_time             = proc_cpu_time;
    proc->proc_priority             = PROC_SAFE_PRIORITY(priority) ;
    proc->proc_prio_num             = PROC_SAFE_PRIONUM(prionum);
    proc->proc_sched_factor         = 0;
    proc->proc_signal_map           = 0;
#ifdef _CFG_SIGNAL_ENABLE_
    Proc_signal_initial(proc);
#endif  /*  _CFG_SIGNAL_ENABLE_ */
#ifdef _CFG_PROC_USER_EXT_
    Proc_create_ue_initial(&proc->proc_user_ext);
#endif  /* _CFG_PROC_USER_EXT_  */
    hnd = Koum_add(proc,Proc_release,kot_proc,HANDLE_ATTR_RDWR);
    /*
     *  加入KOUM不成功，则要释放已分配到的进程对象
     */
    if( INVALID_HANDLE == hnd )
    {
        CRITICAL_BEGIN();
        _memzero(proc,sizeof(proc_t));
        CRITICAL_END();
    }
    else
    {
        Sched_add(proc);
        PROC_NEED_SCHED();
    }
proc_create_end:
    return hnd;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_exit
//  作  用: 进程自身退出运行。
//  参  数: 
//      code        : int           |   退出代码
//  返回值: 无
//  注  意:   在调用该函数前，应确保进程已经释放了所有的资源，否则可能会导致资
//          源泄露。
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_exit(int code)
{
    CRITICAL_DECLARE(proc_pool_lock);

    CRITICAL_BEGIN();
    Sched_del(proc_current);    /*  从调度列表中删除进程    */
    _memzero(proc_current,sizeof(proc_t));
    CRITICAL_END();
    Proc_sched(0);
    code = code;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_kill
//  作  用: 强制进程退出。
//  参  数: 
//      handle      : handle_t      |   进程句柄
//  返回值: 无
//  注  意:   在调用该函数前，应确保进程已经释放了所有的资源，否则可能会导致资
//          源泄露。
//
//  变更记录:
//  时间        |   作  者      | 说  明
//=============================================================================
//  2014-02-05  |   罗  斌      | 修改参数
//  2012-11-15  |   罗  斌      | 第一版
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Proc_kill(handle_t handle)
{
    proc_t        * proc        = FIRST_PROC;
    result_t        result      = RESULT_SUCCEED;
    int             i           = 1;
    CRITICAL_DECLARE(proc_pool_lock);

    ASSERT( INVALID_HANDLE != handle);
    if( INVALID_HANDLE == handle )
        return result;
    /*
     *  句柄类型或者句柄对象无效，函数失败
     */
    if( kot_proc != Koum_handle_type(handle) || \
        NULL == ( proc = Koum_handle_object(handle)) )
    {
        result = RESULT_FAILED;
        goto proc_kill_end;
    }
    /*
     *  杀死自身，不需要返回
     */
    if( proc == proc_current )
    {
        Proc_exit(1);
        /*
         *  实际不会执行到这里，但是为了使函数在逻辑上正确，增加了这部分代码
         */
        goto proc_kill_end;
    }
    CRITICAL_BEGIN();
    /*
     *  找到的进程会有不同的状态，需要分类处理
     */
    switch( proc->proc_stat )
    {
    case PROC_STAT_RUN:
        Sched_del(proc);        /*  从调度列表中删除进程      */
        _memzero(proc,sizeof(proc_t));
        break;
    case PROC_STAT_WAIT:
        result = RESULT_FAILED;
        break;
    case PROC_STAT_SLEEP:
        _memzero(proc,sizeof(proc_t));
        break;
    }
    CRITICAL_END();
proc_kill_end:
    return result;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_sched
//  作  用: 执行进程调度。
//  参  数: 无
//      refresh     : int           +   0表示不刷新调度因子，非零表示刷新
//  返回值: 无
//  注  意:   Lenix采用两级调度算法。第一级，按优先级调度。第二级，在同一个优先
//          级中，按照调度因子调度。
//            1.在硬件中断服务或者进程的禁止抢占标志置位，都不进行调度。采用向
//          系统发出调度请求的方式，在中断处理完毕或允许抢占后在进行调度。如果
//          出现大量的中断嵌套，将会影响实时性能。不过，如果系统出现了大量的中
//          断嵌套，最大的可能是中断处理程序设计出了大问题，要重新设计了。
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2013-11-26  |               |  修改调度算法，引入优先级位图，
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_sched(int refresh)
{
    proc_t        * temp    = NULL,       /*  优先级列表循环算子     */
                  * next    = NULL;       /*  下一个需要运行的进程   */
    int             prio    = 0;
    CRITICAL_DECLARE(proc_rspl_lock);

#ifdef _CFG_DEBUG_
    Proc_sched_msg(NULL,NULL);
#endif  /*  _CFG_DEBUG_ */

    /*
     *  不能在处理中断时调度，
     *  但确实需要调度，因此采用想系统发出调度请求的方式来处理。
     */
    if( interrupt_nest || proc_current->proc_seize )
    {
        PROC_NEED_SCHED();
        return ;
    }

    /*
     *    这里涉及进程查找，如果是采用了关中断的CSPF，需要注意其关中断的时间
     */
    CRITICAL_BEGIN();
    /*
     *  调度前，首先刷新进程调度因子，已经遍历了一次进程池
     *  在实时性要求高的情况下，不应该刷新调度因子。而应该直接进入调度，
     */
    if( PROC_REFRESH_SCHED_FACTOR == refresh )
        Proc_refresh_sched_factor();

    /*
     *  注意: 
     *      采用折半查找的方法来查找存在进程的最高优先级。这里引入了优先级位
     *  图，对应位为1，就表示这个优先级存在进程。
     *      首先比较前32个优先级，方法是对表示前32个优先级的两个16位位图进行或
     *  运算，如果结果不为零，就表示前32个优先级中存在进程。然后再最前的16个优
     *  先级进行检测，方法是检测其位图是否不为0，如果不为0，则优先级从0开始查
     *  找。其余情况类似。
     */
	if( proc_prio_map[0] | proc_prio_map[1] )
    {
        if( proc_prio_map[0] )  prio =  0;
        else	                prio = 16;
    }
    else
    {
        if( proc_prio_map[2] )	prio = 32;
        else                    prio = 48;
    }

    /*
     *    到达这里，说明位图不为0。进一步检测位图的低8位，如果低8位为0，表示高8
     *  位不为0，可以进一步缩小优先级的查找范围
     */
    if( !(proc_prio_map[ prio / 16 ] & 0x00FF ) ) 
        prio += 8;

    /*
     *    通过之前的查找，可以将扫描数量控制在8个以内。通过查看生成的汇编代码，
     *  查找到存在进程优先级，总共大约需要执行90条指令。
     */
    while( prio <= 64 )
    {
        if( proc_rspl[prio] )
            break;
        prio++;
    }

    next = proc_rspl[prio];
    temp = next->proc_sched_next;

    /*
     *  挑选出调度因子最高的进程
     *    由于调度因子可变，因此这里只能采用顺序遍历的方式来查找。如果要保证
     *  实时性，那应控制每个优先级中的进程数量。建议最好不要超过8个。
     *    如果按每个优先级有8个进程，查找进程总共需要执行90条指令。与前面超找
     *  优先级的90条指令相加，找到下一个运行的进程总共需要执行180条指令。对于
     *  50MHz的CPU，按每条指令需要3个始终周期计算，假设没有流水线技术，则总共
     *  耗时：
     *          180 * 3 / 50M = 0.0000108 (秒) = 10.8 (微秒)
     *    如果做适当的优化，比如把重要任务的优先级安排在8的整数倍上，每个优先
     *  级只安排1-3个进程，这样总共需要执行的指令大约为50条，则总共耗时：
     *          50 * 3 / 50M = 0.000003 (秒) = 3 (微秒)
     *    已经足够快了，因此没有在研究更快的算法。
     */
    while(temp)
    {
        if( temp->proc_sched_factor > next->proc_sched_factor )
            next = temp;
        temp = temp->proc_sched_next;
    }

    proc_need_sched             = 0;    /*  每次调度后，都将该标志清0   */
    next->proc_sched_factor     = 0;    /*  获得CPU后，调度因子清零     */
    next->proc_cpu_time         = proc_cpu_time;

    CRITICAL_END();
        
    /*  
     *  挑选出的不是当前进程，则执行切换    
     */
    if( next != proc_current )                      
        PROC_SWITCH_TO(next);   
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_need_schedule
//  作  用: 提示系统进行调度，
//  参  数: 无
//  返回值: 无
//  注  意: 
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-11-15  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_need_schedule(void)
{
    PROC_NEED_SCHED();
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_sleep
//  作  用: 使进程进入睡眠状态，不参与CPU竞争。
//  参  数: 无
//  返回值: 无
//  注  意: 使用该函数进入睡眠状态后，会在意想不到的地方被唤醒。
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_sleep(void)
{
    /*
     * 系统进程进入睡眠状态，说明有错，死机
     */
    if( proc_current == proc_idle )     
        Sys_halt("idle try to sleep.");
    PROC_SEIZE_DISABLE();
    Sched_del(proc_current);
    proc_current->proc_stat = PROC_STAT_SLEEP;
    PROC_SEIZE_ENABLE();
    Proc_sched(0);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_wakeup
//  作  用: 唤醒系统中所有调用Proc_sleep函数进入睡眠状态的进程。
//  参  数: 无
//  返回值: 无
//  注  意:   如果系统中存在超过2个及以上睡眠的进程，调用该函数后，这些进程全部
//          都会被唤醒。因此使用Proc_sleep函数时，要注意到这一点，会在意想不到
//          的地方被唤醒。
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_wakeup(void)
{
    proc_t        * proc        = FIRST_PROC;
    int             i           = 1;
    CRITICAL_DECLARE(proc_pool_lock);

    CRITICAL_BEGIN();
    for( ; i < PROC_MAX ; i++,proc++)
    {
        if( proc->proc_entry && PROC_STAT_SLEEP == proc->proc_stat )
        {
            proc->proc_stat = PROC_STAT_RUN ;
            Sched_add(proc);
            /*
             *  有更高优先级的进程，则通知系统需要进行调度
             */
            if( proc->proc_priority < proc_current->proc_priority )
                PROC_NEED_SCHED();
        }
    }
    CRITICAL_END();
    SCHED(0);        
}


/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_delay
//  作  用: 无。
//  参  数: 
//      millisecond : uint32_t      |   需要延迟的时间，以毫秒为单位
//  返回值: 无
//
//  注  意:   调用该函数后，进程暂停一定时间，具体的时间由参数给出。进程暂停期
//          间，放弃CPU。由于时钟频率与时间存在一定偏差，以及进程恢复运行后不一
//          定能够立即获得CPU，因此延迟时间不一定能够完全与参数相等，大多数情况
//          会比参数提供的时间稍长。
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
//
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_delay(uint32_t millisecond)
{
    CRITICAL_DECLARE(proc_delay_lock);
    /*
     *  将时间转换为时钟中断次数
     */
    proc_current->proc_alarm = MILIONSECOND_TO_TICKS(millisecond);          

    if( proc_current->proc_alarm )  /*  确实需要延迟的时候，才放弃CPU   */
    {
        /*
         *  只能使用禁止抢占，这里并不是保护某个公共变量，而是要求一连串的操作
         *  必须连续，不能切换到其他进程，但是可以处理中断。
         */
        PROC_SEIZE_DISABLE();
        Sched_del(proc_current);
        CRITICAL_BEGIN();
        Proc_list_add(&proc_delay,proc_current);
        proc_current->proc_wait = &proc_delay;
        proc_current->proc_stat = PROC_STAT_WAIT;
        CRITICAL_END();
        PROC_SEIZE_ENABLE();
        Proc_sched(0);
    }
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_signal_pid
//  作  用: 向进程发送信号。
//  参  数: 
//      pid         : int           +   进程编号
//      sigmap      : uint_t        +   信号位图
//  返回值: 成功返回RESULT_SUCCEED，失败返回RESULT_FAILED。
//
//  注  意:   可以同时发送多个信号，多个信号用“或”操作进行连接。如果进程已经
//          获得了相应的信号，不重复记录。处于睡眠状态的进程会立即被唤醒。
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Proc_signal_pid(int pid,uint_t signal)
{
    register proc_t     *   proc        = NULL;
    CRITICAL_DECLARE(proc_pool_lock);

    CRITICAL_BEGIN();

    if( NULL == (proc = Proc_get(pid)))
    {
        CRITICAL_END();
        return RESULT_FAILED;
    }

    /*  置信号位图                            */
    proc->proc_signal_map |= PROC_MAKE_SIGNAL(signal);

    /*
     *  进程处于睡眠状态，则唤醒进程
     */
    if( PROC_STAT_IS_SLEEP(proc) )
    {
        Sched_add(proc);
        proc->proc_stat = PROC_STAT_RUN;
        if( proc->proc_priority < proc_current->proc_priority )
            PROC_NEED_SCHED();    /*  有高优先级的进程进入运行态，需要切换  */
    }

    CRITICAL_END();

    SCHED(0);

    return RESULT_SUCCEED;
}

#ifdef _CFG_SIGNAL_ENABLE_

void        Signal_kill(void)
{
    Proc_exit(1);
}
void        Signal_default(void)
{
}

#endif  /*  _CFG_SIGNAL_ENABLE_    */
/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_get_name
//  作  用: 获得当前进程的名称。
//  参  数: 
//      name        : char[PROC_NAME_LEN]+   名称缓冲区
//  返回值: 该指针是参数传入的指针。
//
//  注  意:   应确保传入的缓冲区足够长。Lenix默认的进程名称为12个字节，因此缓冲
//          区至少为12个字节。
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
char *      Proc_get_name(char name[PROC_NAME_LEN])
{
    if( NULL == name )
        return NULL;
    
    PROC_SEIZE_DISABLE();

    _strcpy(name,proc_current->proc_name);

    PROC_SEIZE_ENABLE();

    return name;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_get_run_time
//  作  用: 获得当前进程已经运行的时间。
//  参  数: 无
//  返回值: 当前进程已经运行的时间，以时钟中断次数计算。
//  注  意: 如果在运行期间修改了系统的时钟频率，该值所表示的时间将不可推算。
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
uint32_t    Proc_get_run_time(void)
{
    return proc_current->proc_run_time;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_get_stack_size
//  作  用: 获得当前进程的栈容量。
//  参  数: 无
//  返回值: 当前进程的栈容量。
//  注  意: 无
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
uint_t      Proc_get_stack_size(void)
{
    return proc_current->proc_stack_size;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_get_pid
//  作  用: 获得当前进程的唯一编号。
//  参  数: 无
//  返回值: 当前进程的唯一编号。
//  注  意: 无
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
uint_t      Proc_get_pid(void)
{
    return proc_current->proc_pid;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_get_priority
//  作  用: 获得进程的优先级。
//  参  数: 无
//  返回值: 进程的优先级。
//  注  意: 无
//
//  变更记录:
//  时间        |  作者         | 说明
//=============================================================================
//  2012-11-17  |  罗斌         +   改获得当前进程的优先级，变为获得指定进程的
//                              | 优先级
//  2012-01-02  |  罗斌         | 第一版
///////////////////////////////////////////////////////////////////////////////
*/
byte_t      Proc_get_priority(int pid)
{
    byte_t                  ret     = PROC_INVALID_PRIORITY;
    proc_t              *   proc    = FIRST_PROC;
    int                     i       = 1;
    CRITICAL_DECLARE(proc_pool_lock);

    CRITICAL_BEGIN();
    
    for( ; i < PROC_MAX ; i++,proc++)
    {
        if( proc->proc_entry && pid == proc->proc_pid )
        {
            ret = proc->proc_priority;
            break;
        }
    }
    
    CRITICAL_END();

    return ret;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_set_priority
//  作  用: 改变进程的优先级。
//  参  数: 
//      pid         : int           |   进程编号
//      priority    : byte_t        |   需要调整到的优先级。
//  返回值: 进程原优先级。
//  注  意:   参数应确保在[0 , 63]之间。如果参数不在该范围内,系统会对参数进行调
//          整，行为将不可控制。
//
//  变更记录:
//  时间        |  作者         | 说明
//=============================================================================
//  2012-11-17  |  罗斌         +   改获得当前进程的优先级，变为获得指定进程的
//                              | 优先级
//  2012-01-02  |  罗斌         +  第一版
///////////////////////////////////////////////////////////////////////////////
*/
byte_t      Proc_set_priority(int pid, byte_t priority)
{
    byte_t          pprio       = PROC_INVALID_PRIORITY;    /*  原优先级 */
    proc_t        * proc        = FIRST_PROC;
    int             i           = 1;
    CRITICAL_DECLARE(proc_pool_lock);

    CRITICAL_BEGIN();

    for( ; i < PROC_MAX ;i++,proc++)
    {
        if( proc->proc_entry && pid == proc->proc_pid )
            break;
    }

    if( i >= PROC_MAX )
        goto set_priority_end;

    pprio     = proc->proc_priority;

    /*
     *  优先级相同，不用调整
     */
    if( pprio == priority)
        goto set_priority_end;

    /*
     *  调整优先级的步骤分为3步
     *  1.从原优先级列表中删除
     *  2.调整优先级
     *  3.插入新优先级列表
     */
    switch( proc->proc_stat)
    {
    case PROC_STAT_RUN:
        Sched_del(proc);                   

        proc->proc_priority = PROC_SAFE_PRIORITY(priority);

        Sched_add(proc);

        if( proc->proc_priority < proc_current->proc_priority ) 
            PROC_NEED_SCHED();
        break;
    default:
        proc->proc_priority = PROC_SAFE_PRIORITY(priority);
    }
set_priority_end:

    CRITICAL_END();

    SCHED(0);

    return pprio;
}
/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_get_prio_num
//  作  用: 获得进程的优先数。
//  参  数: 
//      pid         : int           |   进程编号
//  返回值: 当前进程的优先数。
//  注  意: 关中断的时间可能较长，避免用在实时性要求高的程序中。
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-11-17  |  罗斌         |  将仅对当前进程修改为可指定进程
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
byte_t      Proc_get_prio_num(int pid)
{
    byte_t          ret     = PROC_INVALID_PRIONUM;
    proc_t        * proc    = FIRST_PROC;
    int             i       = 1;
    CRITICAL_DECLARE(proc_pool_lock);

    CRITICAL_BEGIN();
    for( ; i < PROC_MAX ; i++,proc++)
    {
        if( proc->proc_entry && pid == proc->proc_pid )
        {
            ret = Proc_prio_num(proc->proc_prio_num);
            break;
        }
    }
    CRITICAL_END();

    return ret;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Proc_set_prio_num
//  作  用: 设置进程的优先数。
//  参  数: 
//      pid         : int           |   进程编号
//      prionum     : byte_t        |   新优先数
//  返回值: 进程原来的优先数。
//  注  意: 关中断的时间可能较长，避免用在实时性要求高的程序中。
//
//  变更记录:
//  时间        |  作者         |  说明
//=============================================================================
//  2012-11-17  |  罗斌         |  将仅对当前进程修改为可指定进程
//  2012-01-02  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
byte_t      Proc_set_prio_num(int pid, byte_t prionum)
{
    byte_t                  ret     = PROC_INVALID_PRIONUM;
    proc_t              *   proc    = FIRST_PROC;
    int                     i       = 1;
    CRITICAL_DECLARE(proc_pool_lock);

    CRITICAL_BEGIN();
    for( ; i < PROC_MAX ; i++,proc++)
    {
        if( proc->proc_entry && pid == proc->proc_pid )
        {
            ret = Proc_prio_num(proc->proc_prio_num);
            proc->proc_prio_num = PROC_SAFE_PRIONUM(prionum);
            break;
        }
    }
    CRITICAL_END();

    return ret;
}

void        Proc_set_last_err(uint32_t err)
{
    proc_current->proc_last_err = err;
}

uint32_t    Proc_get_last_err(void)
{
    return proc_current->proc_last_err;
}
