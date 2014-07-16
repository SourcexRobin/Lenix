/*
////////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2012 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: proc.h
//  ����ʱ��: 2011-07-02        ������: �ޱ�
//  �޸�ʱ��: 2012-12-17        �޸���: �ޱ�
//  ��Ҫ����: �ṩ���̹�����
//
//  ˵    ��:
//
//  �����¼:
//  �� �� ��    |   ʱ  ��   |  ��  ��      | ��Ҫ�仯��¼
//==============================================================================
//              | 2014-02-05 |  ��  ��      | ʹ��KOUM�޸Ľ��̹���
//              | 2014-02-02 |  ��  ��      | �����µı���淶������ʽ
//              | 2011-07-02 |  ��  ��      | ��һ��
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
    koh_t                   koh;                    /*  �ں˶���ͷͳһ����      */
    struct _proc_t      *   proc_sched_prev,
                        *   proc_sched_next;
    struct _proc_list_t *   proc_wait;              /*  �ȴ��б�                */
    char                    proc_name[PROC_NAME_LEN];
    void                *   proc_stack_bottom;      /*  ����ջ��ָ��            */
    uint_t                  proc_stack_size;        /*  ջ����                  */
    void                *   proc_sp;                /*  ջָ��                  */
    proc_entry_t            proc_entry;
    int                     proc_pid;
    uint32_t                proc_run_time;          /*  ��������ʱ��            */
    uint_t                  proc_last_err;          /*  ���������            */
    sig_map_t               proc_signal_map;        /*  �ź�λͼ                */
#ifdef _CFG_SIGNAL_ENABLE_
    sig_handle_t            proc_signal_handle[16]; /*  �źŴ�����            */
#endif  /*  _CFG_SIGNAL_ENABLE_ */
    uint32_t                proc_alarm;             /*  ���̶�ʱ��              */
    uint32_t                proc_sched_factor;      /*  ��������                */
    volatile  uint_t        proc_seize;             /*  ��ռ��־                */
    byte_t                  proc_prio_num;          /*  ������������            */
    byte_t                  proc_priority;          /*  �������ȼ�              */
    byte_t                  proc_stat;              /*  ����״̬                */
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
 *  ��ϵͳ������������
 */
#define PROC_NEED_SCHED()           do{ proc_need_sched = 1; } while(0)
/*
 *  ���Ե���
 *  refresh : 0��ʾ��ˢ�£���0��ʾˢ��
 */
#define SCHED(refresh)              do{ if( proc_need_sched )  \
                                        {Proc_sched(refresh);} \
                                      } while(0)

#define PROC_REFRESH_SCHED_FACTOR   1
/*
 *  Lenix�ٽ�ν������ 
 *  CRITICAL_DECLARE(sl):  �����ٽ����Ҫʵ�õ����ݣ�����Ϊspin_lock_t���͵ı�
 *  ����������ȫ�ֱ�����ͨ���ٽ�ζ���һЩ���ʹ�����Դ�Ĵ���Σ����Ӧ��Ϊ����
 *  �Ĺ�����Դ���ṩһ�������������Ե�������ͨ����
 *
 *  ʹ�÷�����
 *      �����ڱ��������ĩβ����ע�⣬һ��Ҫ��ĩβ�����CRITICAL_DECLARE�꣬
 *      ������ٽ�ο�ʼ�����CRITICAL_BEGIN()
 *      ������ٽ��ĩβ���CRITICAL_END()
 *      ֻ�ܶԵ�һ����ʹ�ã����һ�������������������漰�ٽ�Σ�����Ҫ�ֹ�����
 *  �ر���SMP�����
 *
 *  ע��: ԭ���ϱ����ٽ��Ƕ�ס�����fun1()�к����ٽ�Σ���fun2()�е��ٽ�ε�����
 *  fun1()����SMP���п��ܻᵼ�¶�ͬһ���������ظ����ԣ����������
 *      ��Ȼ����ȷ��û�����������£�����Ƕ�ס�
 */
#ifdef _CFG_SMP_

#if _CFG_CRITICAL_METHOD_ == 2
/*
 *  ������ý�ֹ��ռ�ķ�ʽ����Ҫȷ������Ҫ���ȵĵط�ʹ��SCHED�꣬�ر������жϴ�
 *  ������С��ô��ǲ��ᶪʧ�ж�
 */
#define CRITICAL_DECLARE(sl)        
#define CRITICAL_BEGIN()            PROC_INC_SEIZE()
#define CRITICAL_END()              PROC_DEC_SEIZE()

#else
/*
 * SMPĬ�ϲ����������������
 */
#define CRITICAL_DECLARE(sl)        spin_lock_t * __spin_lock = &(sl)
#define CRITICAL_BEGIN()            Spin_lock(__spin_lock)
#define CRITICAL_END()              Spin_free(__spin_lock)

#endif 

#else   /*  _CFG_SMP_   */

/*
 *  ��CPU���Բ��ù��жϵķ�ʽ�Լ���ֹ��ռ�ķ�ʽ   
 */
#if _CFG_CRITICAL_METHOD_ == 2
/*
 *  ������ý�ֹ��ռ�ķ�ʽ����Ҫȷ������Ҫ���ȵĵط�ʹ��SCHED�꣬�ر������ж�
 *  ��������С��ô��ǲ��ᶪʧ�ж�
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
 *  ���ù��жϵķ�ʽ����̼�Ҳ��԰�ȫ��������Ҫȷ���ٽ�δ���ǳ��̣���������
 *  ��ʧ�ж��ź�
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
 *  ջ��͵�ַ�����Ĵ�����֤ջ����CPU�ֳ�����
 */
#define STACK_MAKE(sp,size)         ((void *)(((uint_t)(sp) + (size)) & \
                                        (~(sizeof(int) - 1))))
#define STACK_SIZE(sp,size)         ((uint_t)STACK_MAKE(sp,size) - (uint_t)(sp))
#define STACK_CHECK(proc)           ((uint_t)((proc)->proc_sp) > \
                                        (uint_t)((proc)->proc_stack_bottom))
#define STACK_BOTTOM(sp,size)       ((void*)((uint_t)(sp) - (size)))

#endif  /*  STACK_DIRECT    */

/*
 *  ջ����ϵ�к�
 *  n:  ����
 *  l:  ����
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
 *  X86ϵ��CPUʹ�õĽ����л�����
 */
void        Proc_switch_to(uint_t cs,uint_t flag,proc_t * next);

/*
//////////////////////////////////////////////////////////////////////////////
///////////////////
//  �ں˻�����API��ֱ��ʹ�ý��̶����ָ��
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
//  �û�API����Ҫʹ�ö�����
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