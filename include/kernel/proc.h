/*
////////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2012 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: proc.h
//  创建时间: 2011-07-02        创建者: 罗斌
//  修改时间: 2012-12-17        修改者: 罗斌
//  主要功能: 提供进程管理功能
//
//  说    明:
//
//  变更记录:
//  版 本 号    |   时  间   |  作  者      | 主要变化记录
//==============================================================================
//              | 2014-02-05 |  罗  斌      | 使用KOUM修改进程管理
//              | 2014-02-02 |  罗  斌      | 根据新的编码规范调整格式
//              | 2011-07-02 |  罗  斌      | 第一版
////////////////////////////////////////////////////////////////////////////////
*/

#ifndef _PROC_H_
#define _PROC_H_

#include <config.h>
#include <type.h>
#include <koum.h>
#include <slist.h>

#ifdef  _SLASH_
    #include <machine/machine.h>
#else
    #include <machine\machine.h>
#endif  /*  _SLASH_ */

void        Spin_lock(spin_lock_t * sl);
void        Spin_free(spin_lock_t * sl);

typedef void                (* proc_entry_t )(void *);
typedef void                (* sig_handle_t )(void );

#if _CFG_WORD_ > 8 
typedef uint_t              sig_map_t;
#else
typedef uint_t              sig_map_t;
#endif

struct _proc_list_t;

typedef struct _user_ext_t
{
    uint_t                  ue_data[16];
}user_ext_t;

#define PROC_NAME_LEN               12
typedef struct _proc_t
{
    koh_t                   koh;                    /*  内核对象头统一定义      */
    struct _proc_t      *   proc_sched_prev,
                        *   proc_sched_next;
    struct _proc_list_t *   proc_wait;              /*  等待列表                */
    char                    proc_name[PROC_NAME_LEN];
    void                *   proc_stack_bottom;      /*  进程栈底指针            */
    uint_t                  proc_stack_size;        /*  栈容量                  */
    void                *   proc_sp;                /*  栈指针                  */
    proc_entry_t            proc_entry;
    int                     proc_pid;
    uint32_t                proc_run_time;          /*  进程运行时间            */
    uint_t                  proc_last_err;          /*  最后错误代码            */
    sig_map_t               proc_signal_map;        /*  信号位图                */
#ifdef _CFG_SIGNAL_ENABLE_
    sig_handle_t            proc_signal_handle[16]; /*  信号处理函数            */
#endif  /*  _CFG_SIGNAL_ENABLE_ */
    uint32_t                proc_alarm;             /*  进程定时器              */
    uint32_t                proc_sched_factor;      /*  调度因子                */
    volatile  uint_t        proc_seize;             /*  抢占标志                */
    byte_t                  proc_prio_num;          /*  进程优先数，            */
    byte_t                  proc_priority;          /*  进程优先级              */
    byte_t                  proc_stat;              /*  进程状态                */
    byte_t                  proc_pad;
    int                     proc_cpu_time;
#ifdef _CFG_PROC_USER_EXT_
    user_ext_t              proc_user_ext;
#endif  /*  */
}proc_t;

typedef struct _proc_list_t
{
    proc_t              *   pl_list;
}proc_list_t;

#define PROC_STAT_UNDEF             0
#define PROC_STAT_RUN               1
#define PROC_STAT_WAIT              2
#define PROC_STAT_SLEEP             3

#define PROC_STAT_FREE              PROC_STAT_UNDEF

#define PROC_MAKE_SIGNAL(id)        ((uint_t)1 << (id))
#define PROC_SIGNAL_WAKEUP          0
#define PROC_SIGNAL_KILL            1
#define PROC_SIGNAL_NET
#define PROC_SIGNAL_IO_OK
#define PROC_SIGNAL_USER            8

#define PROC_SAFE_PRIORITY(prio)    ( (prio) & (PROC_PRIORITY_MAX - 1))
#define PROC_SAFE_PRIONUM(pn)       ( proc_priority_number[(pn) % 6] )

#define PROC_IS_FREE(proc)          ( NULL              == (proc)->proc_entry )
#define PROC_STAT_IS_RUN(proc)      ( PROC_STAT_RUN     == (proc)->proc_stat  )
#define PROC_STAT_IS_WAIT(proc)     ( PROC_STAT_WAIT    == (proc)->proc_stat  )
#define PROC_STAT_IS_SLEEP(proc)    ( PROC_STAT_SLEEP   == (proc)->proc_stat  )

#define PROC_STAT_SET_FREE(p)       do{(p)->proc_stat=PROC_STAT_FREE; }while(0)
#define PROC_STAT_SET_RUN(p)        do{(p)->proc_stat=PROC_STAT_RUN;  }while(0)
#define PROC_STAT_SET_WAIT(p)       do{(p)->proc_stat=PROC_STAT_WAIT; }while(0)
#define PROC_STAT_SET_SLEEP(p)      do{(p)->proc_stat=PROC_STAT_SLEEP;}while(0)

#define PROC_SET_INVALID(proc)      do{ proc = NULL; } while(0)
#define PROC_IS_VALID(proc)         ( proc )
#define PROC_IS_INVALID(proc)       ( NULL == ( proc) )
#define PROC_IS_ALARM(proc)         ( (proc)->proc_alarm )

#define PROC_CONTEXT(proc)          ( (context_t *)((proc)->proc_sp) )
#define PROC_CURRENT_CONTEXT()      PROC_CONTEXT(proc_current)

#define PROC_CAN_SEIZE()            ( 0 == (proc_current)->proc_seize )
#define PROC_INC_SEIZE()            do{ ++proc_current->proc_seize;} while(0)
#define PROC_DEC_SEIZE()            do{ --proc_current->proc_seize;} while(0)

#define PROC_SEIZE_DISABLE()        PROC_INC_SEIZE()
#define PROC_SEIZE_ENABLE()         PROC_DEC_SEIZE()

#define PROC_SET_LAST_ERR(e)        do{proc_current->proc_last_err=(e);}while(0)
#define PROC_SET_ERR(e)             do{proc_current->proc_last_err=(e);}while(0)
#define PROC_ERR()                  do{\
                                     proc_current->proc_last_err=RESULT_FAILED;\
                                    } while(0)
#define PROC_NO_ERR()               PROC_SET_ERR(0)

/*
 *  向系统发出调度请求
 */
#define PROC_NEED_SCHED()           do{ proc_need_sched = 1; } while(0)
/*
 *  尝试调度
 *  refresh : 0表示不刷新，非0表示刷新
 */
#define SCHED(refresh)              do{ if( proc_need_sched )  \
                                        {Proc_sched(refresh);} \
                                      } while(0)

#define PROC_REFRESH_SCHED_FACTOR   1
/*
 *  Lenix临界段解决方案 
 *  CRITICAL_DECLARE(sl):  定义临界段需要实用的数据，参数为spin_lock_t类型的变
 *  量，而且是全局变量。通常临界段都是一些访问共享资源的代码段，因此应该为所有
 *  的共享资源都提供一个自旋锁，用以到达程序的通用性
 *
 *  使用方法：
 *      首先在变量定义的末尾，请注意，一定要在末尾，添加CRITICAL_DECLARE宏，
 *      其次在临界段开始处添加CRITICAL_BEGIN()
 *      最后在临界段末尾添加CRITICAL_END()
 *      只能对单一对象使用，如果一个函数中有两个变量涉及临界段，则需要手工处理，
 *  特别是SMP情况。
 *
 *  注意: 原则上避免临界段嵌套。例如fun1()中含有临界段，在fun2()中的临界段调用了
 *  fun1()。在SMP中有可能会导致对同一个自旋锁重复测试，造成死锁。
 *      当然，能确保没有问题的情况下，可以嵌套。
 */
#ifdef _CFG_SMP_

#if _CFG_CRITICAL_METHOD_ == 2
/*
 *  如果采用禁止抢占的方式，需要确保在需要调度的地方使用SCHED宏，特别是在中断处
 *  理程序中。好处是不会丢失中断
 */
#define CRITICAL_DECLARE(sl)        
#define CRITICAL_BEGIN()            PROC_INC_SEIZE()
#define CRITICAL_END()              PROC_DEC_SEIZE()

#else
/*
 * SMP默认采用自旋锁解决方案
 */
#define CRITICAL_DECLARE(sl)        spin_lock_t * __spin_lock = &(sl)
#define CRITICAL_BEGIN()            Spin_lock(__spin_lock)
#define CRITICAL_END()              Spin_free(__spin_lock)

#endif 

#else   /*  _CFG_SMP_   */

/*
 *  单CPU可以采用关中断的方式以及禁止抢占的方式   
 */
#if _CFG_CRITICAL_METHOD_ == 2
/*
 *  如果采用禁止抢占的方式，需要确保在需要调度的地方使用SCHED宏，特别是在中断
 *  处理程序中。好处是不会丢失中断
 */
#define CRITICAL_DECLARE(sl)        
#define CRITICAL_BEGIN()            PROC_INC_SEIZE()
#define CRITICAL_END()              PROC_DEC_SEIZE()

#elif _CFG_CRITICAL_METHOD_ == 1

#include <assert.h>

#define CRITICAL_DECLARE(sl)        
#define CRITICAL_BEGIN()            do{ CPU_DISABLE_INTERRUPT(); ++critical_nest; \
                                        ASSERT(critical_nest < 255); } while(0)
#define CRITICAL_END()              do{ if( --critical_nest == 0 ) \
                                        { CPU_ENABLE_INTERRUPT() ;} } while(0)

#else
/*
 *  采用关中断的方式，编程简单也相对安全，但是需要确保临界段代码非常短，否则容易
 *  丢失中断信号
 */
#define CRITICAL_DECLARE(sl)        psw_t __psw
#define CRITICAL_BEGIN()            do{__psw = Cpu_disable_interrupt();}while(0)
#define CRITICAL_END()              Cpu_psw_set(__psw)

#endif  

#endif  /*  _CFG_SMP_ */

extern byte_t critical_nest;

#ifdef STACK_DIRECT_HIGH

/*
 *
 */
#define STACK_MAKE(sp,size)         ((void *)(((uint_t)(sp) + sizeof(int) - 1)& \
                                        ~(sizeof(int) - 1)))
#define STACK_SIZE(sp,size)         ((size) - ((uint_t)STACK_MAKE(sp,size) - \
                                        (uint_t)(sp)))
#define STACK_CHECK(proc)           ((uint_t)((proc)->proc_sp) < \
                                        (uint_t)((proc)->proc_stack_bottom))
#define STACK_BOTTOM(sp,size)       (void*)((uint_t)(sp) + (size))

#else

/*
 *  栈向低地址增长的处理，保证栈顶与CPU字长对齐
 */
#define STACK_MAKE(sp,size)         ((void *)(((uint_t)(sp) + (size)) & \
                                        (~(sizeof(int) - 1))))
#define STACK_SIZE(sp,size)         ((uint_t)STACK_MAKE(sp,size) - (uint_t)(sp))
#define STACK_CHECK(proc)           ((uint_t)((proc)->proc_sp) > \
                                        (uint_t)((proc)->proc_stack_bottom))
#define STACK_BOTTOM(sp,size)       ((void*)((uint_t)(sp) - (size)))

#endif  /*  STACK_DIRECT    */

/*
 *  栈定义系列宏
 *  n:  名称
 *  l:  长度
 */
#define STACK_DECLARE(name,len)     static byte_t name[len]
#define STACK_DEFINE(name)          STACK_DECLARE(__lenix_proc_stack_##name,\
                                        STACK_DEFAULT_SIZE)
#define STACK_MAKE_DEF(name)        STACK_MAKE(__lenix_proc_stack_##name,\
                                        STACK_DEFAULT_SIZE)

/*
 * n: name
 * p: priority
 * pn: priority number
 * pe: process entry
 * pp: process param
 */
#define PROC_CREATE(name,prio,pn,entry,param)   \
                                    Proc_create(#name,prio,pn,entry,param,\
                                        STACK_MAKE_DEF(name),\
                                        STACK_DEFAULT_SIZE)

#define PROC_LIST_ZERO(pl)          _memzero(pl,sizeof(proc_list_t))

#define PROC_CREATE_DEFAULT(name,entry,sp,size)  \
                                   Proc_create(name,32,3,entry,NULL,\
                                       MAKE_STACK(sp,size),STACK_SIZE(sp,size))

extern proc_t           *   proc_current;
extern volatile uint_t      proc_need_sched;

#define PROC_CURRENT_PID            ( proc_current->proc_pid        )
#define PROC_CURRENT_NAME           ( proc_current->proc_name       )
#define PROC_CURRENT_PRIORITY       ( proc_current->proc_priority   )
#define PROC_CURRENT_PRIONUM        ( proc_current->proc_prio_num   )

#ifdef _CFG_DEBUG_

proc_t  *   Proc_pool(void );

#endif  /*  _CFG_DEBUG_ */

void        Sys_halt(const char * msg);

/*
 *  X86系列CPU使用的进程切换函数
 */
void        Proc_switch_to(uint_t cs,uint_t flag,proc_t * next);

/*
//////////////////////////////////////////////////////////////////////////////
///////////////////
//  内核或驱动API，直接使用进程对象的指针
*/
void        Proc_initial(void);
void        Proc_ticks(void);
void    *   Proc_switch_prepare(void * sp,proc_t * next);
void        Proc_switch_prepare_hook(proc_t * current,proc_t * next);
void        Proc_refresh_sched_factor(void);
void        Proc_sched(int);
void        Sched_add(proc_t * proc);
void        Sched_del(proc_t * proc);

void        Proc_wakeup_proc    (proc_t * proc);
void        Proc_wait_on        (proc_list_t * proclist);
void        Proc_resume_on      (proc_list_t * proclist);
void        Proc_resume_max_on  (proc_list_t * proclist);
void        Proc_wait           (proc_t ** proc);
void        Proc_resume         (proc_t ** proc);

#ifdef _CFG_DEBUG_
void        Proc_msg(void);
#endif  /*  _CFG_DEBUG_    */

/*
//////////////////////////////////////////////////////////////////////////////
///////////////////
//  用户API，需要使用对象句柄
*/
handle_t    Proc_create(const char    * name,
                        byte_t          priority,
                        byte_t          prionum,
                        proc_entry_t    entry,
                        void          * param,
                        void          * stack,
                        int             stacksize);
void        Proc_create_ue_initial(user_ext_t * ue);
void        Proc_exit(int code);
result_t    Proc_kill(handle_t handle);
void        Proc_need_schedule(void);

void        Proc_delay  (uint32_t millisecond);
void        Proc_sleep  (void);
void        Proc_wakeup (void);

void        Proc_set_last_err   (uint32_t err);
uint32_t    Proc_get_last_err   (void);
char *      Proc_get_name       (char name[PROC_NAME_LEN]);
uint32_t    Proc_get_run_time   (void);
uint_t      Proc_get_stack_size (void);
uint_t      Proc_get_stack_use  (void);
uint_t      Proc_get_pid        (void);
byte_t      Proc_get_priority   (int pid);
byte_t      Proc_set_priority   (int pid, byte_t priority);
byte_t      Proc_get_prio_num   (int pid);
byte_t      Proc_set_prio_num   (int pid, byte_t prionum);

void        Proc_signal(proc_t * proc,uint_t signal);
result_t    Proc_signal_pid(int pid,uint_t sigmap);
void *      Signal_handle_set(byte_t signal,sig_handle_t handle);
void        Signal_handle(void);
void        Signal_kill(void);
void        Signal_wakeup(void);
void        Signal_default(void);


#endif  /*  _PROC_H_    */