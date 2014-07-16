/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: proc.c
//  ����ʱ��: 2011-07-02        ������: �ޱ�
//  �޸�ʱ��: 2012-11-29        �޸���: �ޱ�
//  ��Ҫ����: �ṩ���̹����ܡ�
//  ˵    ��: 
//
//  �����¼:
//  �� �� ��    |     ʱ��      |  ��  ��       |  ��Ҫ�仯��¼
//=============================================================================
//  00.00.000   |  2011-07-02   |  ��  ��       |  ��һ��
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
/*  Ϊ��windowsƽ̨׼�� */
    #include <machine/machine.h>
#else
    #include <machine\machine.h>
#endif

proc_t                  *   proc_current;           /*  ��ǰ���̣���CPU���� */
volatile uint_t             proc_need_sched;        /*  ��Ҫ���ȱ�־        */

#ifdef _CFG_SMP_
static spin_lock_t          proc_rspl_lock;         /*  ���Ա������ȼ��б�  */
static spin_lock_t          proc_pool_lock;         /*  ���Ա������̳�      */
static spin_lock_t          proc_delay_lock;        /*  ��ʱ�б���          */

#define proc_prio_lock              proc_rspl_lock
#define proc_lock                   proc_pool_lock
#endif

/*
 *  ����������
 */
const static byte_t         proc_priority_number[6] = {3,3,7,15,31,63};

/*
 *  ����̬�����б�
 *  Running Statu Process List����дΪRSPL
 */
static proc_t           *   proc_rspl[PROC_PRIORITY_MAX + 1];   

/*
 *  2013.11.26
 *  ���ȼ�λͼ��
 */
static uint16_t				proc_prio_map[4];
/*
 *  ���̳أ�Lenix �ӽ��̳��з�����̶���  
 */
static proc_t               proc_pool[PROC_MAX];     

/*  
 *  ���н��̣���Զ��������״̬
 *  ���Ա�֤ϵͳ��û�п����еĽ���ʱ�����ȳ�����Ȼ�����ҵ������еĽ���
 */
static proc_t           *   proc_idle;

/*  
 *  Lenix ����
 */
/*static proc_t           *   proc_lenix;*/

/*  �ӳٵȴ��б�*/
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
//  ���̹����ڲ�ʹ�õĺ�����
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
/*  2012.11.17 �ɺ����Ϊ����  */
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

/*  2012.11.17 �ɺ����Ϊ����  */
static
void        Proc_list_del(proc_list_t * pl,proc_t * proc)
{
    ASSERT(pl);
    
    /*
     *  ���������ͷ�ڵ㣬ɾ��ͷ�ڵ㼴��
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
         *  �ض�����ǰ���ڵ㡣
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
//  ��  ��: Proc_get
//  ��  ��: ͨ��pid��ý��̶���
//  ��  ��: 
//      pid         : int           |   ���̱��
//  ����ֵ: �����򷵻ط�0ֵ�����򷵻�NULL��
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
static 
proc_t  *   Proc_get(int pid)
{
    register proc_t     *   proc        = NULL;

    /*
     *  �������̳�
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
//  ��  ��: Proc_release
//  ��  ��: �ͷŽ��̶���
//  ��  ��: 
//      proc        : proc_t *  +   ���̶���ָ��
//  ����ֵ: ��Զ����RESULT_SUCCEED
//  ע  ��:   ���������˳�������ǿ���˳�ʱ,��ϵͳ��ɶ�����ͷš�KOUM�����ͷ�
//          ��ϵͳ�����Ŀռ䡣����ڴ�������̺����ȷ������Ҫʹ�þ������
//          �������ͷš�
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2014-02-05  |  �ޱ�         |  ����KOUMʱ���Ӹú���
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
//  �ںˡ�����API
*/

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Sched_add
//  ��  ��: �����̼���RSPL�У�����CPU������
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      proc_t *    |    proc       |   ���̶���ָ��
//  ����ֵ: �ޡ�
//  ע  ��: �ں�ʹ�õĺ�������������ֱ��ʹ���ں˶���ָ�롣Ӧ�ó������ʹ��
//
//  �����¼:
//  ʱ��        |   ��  ��      |  ˵��
//=============================================================================
//  2013-11-29  |   ��  ��      |  �������ȼ�λͼ
//  2012-01-02  |   ��  ��      |  ��һ��
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
     *    �������ȼ�λͼ�������̼���RSPL�󣬶�Ӧ�����ȼ���Ȼ���ڽ��̣���˿�ֱ
     *  �ӽ����ȼ�λͼ�Ķ�Ӧλ��1����ʾ��Ӧ���ȼ��Ѿ����ڽ��̡�
     */
    proc_prio_map[prio / 16] |= 1 << (prio & 0xF);

    CRITICAL_END();
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Sched_del
//  ��  ��: �����̴�RSPL��ɾ�������ٲ���CPU������
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      proc_t *    |    proc       |   ���̶���ָ��
//  ����ֵ: �ޡ�
//  ע  ��: �ں�ʹ�õĺ�������������ֱ��ʹ���ں˶���ָ�롣Ӧ�ó������ʹ��
//
//  �����¼:
//  ʱ��        |   ��  ��      |  ˵��
//=============================================================================
//  2013-11-29  |   ��  ��      |  �������ȼ�λͼ
//  2012-01-02  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Sched_del(proc_t * proc)
{
    int             prio        = proc->proc_priority;
    CRITICAL_DECLARE(proc_rspl_lock);
    
    CRITICAL_BEGIN();

    if( proc_rspl[prio]  )
    { 
        /*  ɾ�����̣�ά�������б�    */
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
     *    �����̴ӽ����б���ɾ��ʱ�����������б���û�н��̡�ֻ�������ȼ���û��
     *  ����ʱ������Ҫ�����ȼ�λͼ�ж�Ӧ��λ��0 ��
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
//  ��  ��: Proc_wakeup_proc
//  ��  ��: ����ָ�����̣������䵱ǰ״̬��Ρ�
//  ��  ��: 
//      proc        : proc_t        |   ��Ҫ���ѵĽ��̶���
//  ����ֵ: ��
//  ע  ��:   1.ʹ�øú���ʱ�����ȷ��������Ч��
//            2.�����ʹ�øú��������ڸú�����ֱ�ӻ��ѽ��̣����ܻ���ɲ���Ԥ��
//          �Ľ����
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_wakeup_proc(proc_t * proc)
{
    /*
     *  ���ܻ�����Ч���߿��ж���Ŀǰ��������
     */
    if( PROC_IS_INVALID(proc) || PROC_IS_FREE(proc) )                  
        Sys_halt("try to wakeup invalid process\n");
        
    if( proc->proc_stat == PROC_STAT_RUN )
        return ;

    proc->proc_stat = PROC_STAT_RUN;
    
    if( proc->proc_wait )
    {
        /*
         *  ��������
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
//  ��  ��: Proc_wait_on
//  ��  ��: ���б��ϵȴ���
//  ��  ��: 
//      proclist    : proc_list_t * |   �����б�
//  ����ֵ: ��
//  ע  ��:    1�������ں�ʹ�õĺ�����Ӧ�ó������ʹ�á�
//             2���ú���������ɽ������RSPl��ɾ�������뵽�����б��С���δִ��
//          ���ȡ���˵��øú����󣬱����ɵ����ߵ��õ��Ⱥ�����
//             3��������ȷ�Ļ��ѡ�
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_wait_on(proc_list_t * proclist)
{
    ASSERT(  proclist );
    /*
     * ϵͳ���̽���˯��״̬��˵���д�����
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
//  ��  ��: Proc_resume_on
//  ��  ��: �����б������еĽ��̡�
//  ��  ��: 
//      proclist    : proc_list_t * |   �����б�
//  ����ֵ: ��
//  ע  ��:    1���ú������ں��ú�����Ӧ�ó���Ӧʹ�á�
//             2���������ǽ������б��еĽ���ȫ�������RSPL����δ���ȡ���Ҫ����
//          ���ڵ��øú�������õ��Ⱥ���.
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_resume_on(proc_list_t * proclist)
{
    proc_t        * proc    = NULL,
                  * wakeup  = NULL;
    
    ASSERT(proclist );

#ifdef _CFG_RUN_TIME_CHECK_
    /*  ����ʱУ��  */
    if( NULL == proclist )
        return ;
#endif  /*  _CFG_RUN_TIME_CHECK_ */
    proc = proclist->pl_list;
    while( proc )                /*  �����б������еĽ���                */
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
//  ��  ��: Proc_resume_max_on
//  ��  ��: �����б������ȼ���ߡ������������Ľ��̡�
//  ��  ��: 
//      proclist    : proc_list_t * |   �����б�
//  ����ֵ: ��
//
//  ע  ��:    1���ú������ں��ú�����Ӧ�ó���Ӧʹ�á�
//             2���������ǽ������б������ȼ�����ҵ����������Ľ��̼���RSPL��
//          ��δ���ȡ���Ҫ�������ڵ��øú�������õ��Ⱥ���.
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_resume_max_on  (proc_list_t * proclist)
{
    proc_t       *  proc    = NULL,     /*  ��Ҫ���ѵĽ���      */
                 *  next    = NULL;     /*  ѭ������            */

    ASSERT(proclist);

    proc = proclist->pl_list;

#ifdef _CFG_RUN_TIME_CHECK_
    /*  ����ʱУ��  */
    if( NULL == proclist )
        return ;
#endif  /*  _CFG_RUN_TIME_CHECK_ */
    if( NULL == proc )
        return ;
    /*
     *  �����б������ȼ�����ҵ����������Ľ���
     */
    for( next = proc->proc_sched_next; next ; next = next->proc_sched_next)
    {
        /*
         *  �������ȼ��͵Ľ���
         */
        if( next->proc_priority > proc->proc_priority )
            continue;

        /*
         *  �������ȼ����ߵĽ���
         */
        if( next->proc_priority < proc->proc_priority )
        {
            proc = next;
            continue;
        }

        /*
         *  �������˵���������ȼ���ͬ������ѡ�������ӸߵĽ���
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
//  ��  ��: Proc_wait
//  ��  ��: ��ǰ�����ڵ�����ָ���ϵȴ���
//  ��  ��: 
//      proc        : proc_t **     |   ��Ҫ�ָ��Ľ���ָ���ָ��
//  ����ֵ: ��
//  ע  ��:    1���ú������ں��ú�����Ӧ�ó���Ӧʹ�á�
//             2�����øú����󣬽��̶����״̬�ǵȴ�̬������δ���κ��б��ϵȴ�
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_wait(proc_t ** proc)
{
    /*
     *  ����Ѿ��н����ڶ����ϵȴ���ϵͳ�߼����ڴ�������
     */
    if( NULL == proc || *proc )
        Sys_halt("parameter error or has process wait on!");
    *proc = proc_current;
    /*
     *  ��֤������������SMP�����²����ܱ�֤���⡣
     */
    PROC_SEIZE_DISABLE();
    Sched_del(proc_current);    /*  �Ѿ������ٽ�ο���      */
    proc_current->proc_stat = PROC_STAT_WAIT;
    PROC_SEIZE_ENABLE();
    Proc_sched(0);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Proc_resume
//  ��  ��: �ָ�����Proc_wait����ȴ��Ľ��̡�
//  ��  ��: 
//      proc        : proc_t **     |   ��Ҫ�ָ��Ľ���ָ���ָ��
//  ����ֵ: ��
//  ע  ��:   ֻ�ָܻ��ɵ���Proc_wait����ȴ��Ľ��̣����ָܻ���������ʽ����ȴ�
//          �Ľ��̡�
//
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_resume(proc_t ** proc)
{
    proc_t        * p   = NULL;

    if( NULL == proc || NULL == (p = *proc) )
        Sys_halt("process resume error!");
    /*
     *  ���ǵȴ�״̬���������������ϵȴ�      
     */
    if( PROC_STAT_WAIT != p->proc_stat || p->proc_wait  )
        return ;
    PROC_SEIZE_DISABLE();
    Sched_add(p);    /*  �Ѿ������ٽ�ο���      */
    p->proc_stat = PROC_STAT_RUN;
    PROC_SEIZE_ENABLE();
    if( p->proc_priority < proc_current->proc_priority )
        PROC_NEED_SCHED();
    *proc = NULL;
    SCHED(0);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Proc_ticks
//  ��  ��: ˢ�´�����ʱ״̬�Ľ��̵ļ�������
//  ��  ��: ��
//  ����ֵ: ��
//  ע  ��:    1���ú������ں��ú�������ʱ���жϴ��������ʹ�á�Ӧ�ó���Ӧʹ
//          �á�
//             2��Ӧȷ��ÿ��ʱ���ж϶����øú��������򽫻ᵼ�½�����ʱ��׼ȷ��
//             3���������źŻ�����ʱʱ�䵽�����ᱻ���ѡ�Ӧ�˲���һ������ʱ��Ԥ
//          ����ʱ�䡣
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
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
         *  �����յ��źŻ�����ʱʱ�䵽������������ 
         */
        if( proc->proc_signal_map || --proc->proc_alarm <= 0 )              
        {
            Proc_list_del(&proc_delay,proc);
            /*
             *    ��proc����RSPL��ԭproc���������ݾͻᱻ�����ݴ��档�����Ҫ
             *  ����һ��next����������ԭ�е�����ָ�롣
             */
            Sched_add(proc);
            /*
             * �н��̽�������̬�������ȼ����ڵ�ǰ���̣���Ҫ������������
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
//  ��  ��: Proc_switch_prepare
//  ��  ��: ��ɽ����л���
//  ��  ��: 
//      sp          : void *        +   ��ǰ����ջָ�롣��ָ����PROC_SWITCH_TO
//                                  | �����ṩ��PROC_SWITCH_TO��һ��CPU��ص�
//                                  | �꣬��˸ú�����ֲ��һ���ص�
//      next        : proc_t *      +   ��һ��Ҫ���еĽ���
//  ����ֵ: ������Ҫ���н��̵�ջָ�룬Ȼ��ӷ��ص�ָ���лָ����̵����л���
//  ע  ��: �ú�����PROC_SWITCH_TO���ڵ��á�
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void    *   Proc_switch_prepare(void * sp,proc_t * next)
{

#ifdef _CFG_SWITCH_PREPARE_HOOK_
    Proc_switch_prepare_hook(proc_current,next);
#endif  /*  _CFG_SWITCH_PREPARE_HOOK_   */

    /*
     *    �ڽ��н����л�ʱ����ǰ�����п����Ѿ���Ч���ڽ����˳��󣬾ͻ��������
     *  ����������ʱ��Ҫ����ָ�룬�����Ѿ��ͷŵĶ��������
     */
    if( proc_current->proc_entry)
        proc_current->proc_sp = sp;

#ifdef _CFG_CHECK_STACK_
    do
    {
        char    str[64];

        /*  ����Ƿ����ջԽ��                  */
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
//  ��  ��: Proc_refresh_sched_factor
//  ��  ��: ˢ�½��̵������ӡ�
//  ��  ��: ��
//  ����ֵ: ��
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//              |  �ޱ�         |  ʱ�䲻�ǵ���
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
//  ��  ��: Proc_initial
//  ��  ��: ��ʼ�����̹���ģ�顣
//  ��  ��: ��
//  ����ֵ: ��
//  ע  ��: ֻ����Lenix��ʼ����ʱ�����һ�Ρ�
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
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
    proc_idle->proc_priority        = PROC_PRIORITY_MAX;   /*�����ȼ���Ϊ���*/
    proc_rspl[PROC_PRIORITY_MAX]    = proc_idle;
    /*
     *  ϵͳ�е�һ���ں˶���
     */
    Koum_add(proc_idle,Proc_release,kot_proc,HANDLE_ATTR_RDWR);
    _printk("OK!\n");
}

/*  
 *    �ź���ÿ���жϡ�ϵͳ���ý���ǰ���д����ڱ��ʱ����Ҫע�����ж��Լ�ϵͳ
 *  ���ý���ǰ������Ӧ�Ĵ�����롣
 */
void        Signal_handle(void)
{
    int                     i           = 0;
    uint_t                  signal      = 0;

    /*
     *  û���źž�ֱ���˳�
     */
    if( 0 == proc_current->proc_signal_map )
        return ;

#ifdef _CFG_SIGNAL_ENABLE_
    for( i = 15 ; i >= 0 ; i-- )
    {
        signal = PROC_MAKE_SIGNAL(i);
        if( proc_current->proc_signal_map & signal )
        {
            proc_current->proc_signal_map &= ~signal;   /*  ���ź�λ */
            proc_current->proc_signal_handle[i]();
            return ;                                    /*ÿ��ֻ����һ���ź�*/
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
//  �������û�API
*/

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��:   Spin_lock
//  ��  ��:   ��������������˯�ߣ��ȴ�����������ٽ���Դ��
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//    spin_lock_t*  |   sl          |   ����������ָ��
//  ����ֵ:   ��
//  ˵  ��:   ������Spin_free�ɶ�ʹ�á��ں�����֮��Ĵ���Ӧ��֤��С�����٣�����
//          ��������������CPU�Ĳ�����Ҳ��ҪӦ���ڶ�ʱ������Դ�ķ����ϡ�
//            ���������������ȼ���ת���������ǰ���Ѿ�ȷ�����̲������CPU�����
//          ��Ȼ����ִ�ж��ȴ��Ľ���Ҳ�ڳ���ִ�У��ڵ�ǰ�����ͷ����Ժ󣬵ȴ���
//          ����Ȼ���Ի������
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2012-06-26  |   ��  ��      |  Cpu_tas ��������ֵ�仯
//  2012-01-01  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Spin_lock(spin_lock_t * sl)
{
    /*
     *  ��CPUʱ����֤����һֱ����ռ��CPU
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
//  ��  ��: Proc_create
//  ��  ��: ����һ�����̶���
//  ��  ��: 
//      name        : const char *  +   �����������ܳ���12���ַ������ַ�����β
//                                  | ��0��
//      priority    : byte_t        +   �������ȼ������ȼ����Ϊ0�����Ϊ63��
//                                  | Lenix���̽�Ϊ���⣬���ȼ�Ϊ64���ڴ�������
//                                  | ʱ���Ըò���ȡ64����������֤���ȼ���Ч��
//      prionum     : byte_t        +   ������������Lenix����������Ϊ5���ȼ���
//                                  | 1-5��Lenix���Զ��Ӳ������̴�����֤����
//                                  | ��ȷ��Χ�ڡ�
//      entry       : proc_entry_t  +   ������ڵ㡣��ԭ��Ϊ
//                                  | void Proc_entry(void * param);
//      param       : void *        +   ���̲�������ָ�뷽ʽ�ṩ��
//      stack       : void *        +   ����ջ��ָ�롣��ͨ��MAKE_STACK��õ���
//      stacksize   : int           +   ջ��������ͨ��STACK_SIZE���á�
//  ����ֵ: �ɹ����ط�NULL��ʧ�ܷ���INVALID_HANDLE��
//  ע  ��:   1.���жϴ����ڼ亯������ʧ��
//            2.�������̺���Ҫ��ʽ�ͷž��������ڳ��������ط�����Ҫʹ�þ�
//          ����Ӧ�ڴ������̺������ͷž��������ᵼ��ϵͳ���ں˶������Դй
//          ¶��
//          
//
//  �����¼:
//  ʱ  ��      |   ��  ��      |  ˵  ��
//=============================================================================
//  2014-02-16  |   ��  ��      |  ����KOUM�޸�
//  2014-02-05  |   ��  ��      |  �޸ķ���ֵ����
//  2012-01-02  |   ��  ��      |  ��һ��
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
    /*  �����ڼ����У��    */
    if( NULL == name || NULL == entry || NULL == stack || 0 == stacksize )  
        goto proc_create_end;
#endif  /*  _CFG_RUN_TIME_CHECK_   */
    if( interrupt_nest )    /*  �жϷ������ڼ䣬���ܴ�������     */
        goto proc_create_end;
    /*
     *    �����ٽ���漰���ý��̶�����ҺͿ��ý��̱�Ų��ң���ʱ���ܽϳ���
     *  ��˲�Ҫ��ʵʱ��Ҫ��ߵĳ����д������̡�
     */
    CRITICAL_BEGIN();
    /*  �������̳أ����ҿ��õĽ��̶��� */
    for( ; i < PROC_MAX ; i++,proc++)
    {   
        if( PROC_IS_FREE(proc) )
        {
            proc->proc_entry = entry;   /*  �����̶�����Ϊ��ռ��    */
            break;
        } 
    }
    if( i >= PROC_MAX )
    {
        /*
         *  ���̳����������ܴ�������
         */
        CRITICAL_END();
        goto proc_create_end;
    }
create_pid:
    /*
     *  ���½��̷���Ψһ��ʶ��
     */
    if( ++pid < 1 ) pid = 1;            /*  0��ϵͳ���̵�PID    */
    for( i = 1 ; i < PROC_MAX ; i++)    /*  ���PID�Ƿ�����ظ� */
    {
        /*
         *  �������еĽ��̶������������PID��Ϊ0����ϵͳ�������0�ı�ţ�
         *  ���Բ����������
         */
        if( PROC_IS_FREE(&proc_pool[i]) )
            continue;
        if( pid == proc_pool[i].proc_pid )  /*  �����ظ���pid������������*/
            goto create_pid;
    }
    proc->proc_pid = pid;
    CRITICAL_END();
    sp      = Context_initial(entry,param,stack);   /*  ��ʼ���������л���  */
    _nstrcpy(proc->proc_name,name,11);              /*  ���ƽ�������        */
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
     *  ����KOUM���ɹ�����Ҫ�ͷ��ѷ��䵽�Ľ��̶���
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
//  ��  ��: Proc_exit
//  ��  ��: ���������˳����С�
//  ��  ��: 
//      code        : int           |   �˳�����
//  ����ֵ: ��
//  ע  ��:   �ڵ��øú���ǰ��Ӧȷ�������Ѿ��ͷ������е���Դ��������ܻᵼ����
//          Դй¶��
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_exit(int code)
{
    CRITICAL_DECLARE(proc_pool_lock);

    CRITICAL_BEGIN();
    Sched_del(proc_current);    /*  �ӵ����б���ɾ������    */
    _memzero(proc_current,sizeof(proc_t));
    CRITICAL_END();
    Proc_sched(0);
    code = code;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Proc_kill
//  ��  ��: ǿ�ƽ����˳���
//  ��  ��: 
//      handle      : handle_t      |   ���̾��
//  ����ֵ: ��
//  ע  ��:   �ڵ��øú���ǰ��Ӧȷ�������Ѿ��ͷ������е���Դ��������ܻᵼ����
//          Դй¶��
//
//  �����¼:
//  ʱ��        |   ��  ��      | ˵  ��
//=============================================================================
//  2014-02-05  |   ��  ��      | �޸Ĳ���
//  2012-11-15  |   ��  ��      | ��һ��
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
     *  ������ͻ��߾��������Ч������ʧ��
     */
    if( kot_proc != Koum_handle_type(handle) || \
        NULL == ( proc = Koum_handle_object(handle)) )
    {
        result = RESULT_FAILED;
        goto proc_kill_end;
    }
    /*
     *  ɱ����������Ҫ����
     */
    if( proc == proc_current )
    {
        Proc_exit(1);
        /*
         *  ʵ�ʲ���ִ�е��������Ϊ��ʹ�������߼�����ȷ���������ⲿ�ִ���
         */
        goto proc_kill_end;
    }
    CRITICAL_BEGIN();
    /*
     *  �ҵ��Ľ��̻��в�ͬ��״̬����Ҫ���ദ��
     */
    switch( proc->proc_stat )
    {
    case PROC_STAT_RUN:
        Sched_del(proc);        /*  �ӵ����б���ɾ������      */
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
//  ��  ��: Proc_sched
//  ��  ��: ִ�н��̵��ȡ�
//  ��  ��: ��
//      refresh     : int           +   0��ʾ��ˢ�µ������ӣ������ʾˢ��
//  ����ֵ: ��
//  ע  ��:   Lenix�������������㷨����һ���������ȼ����ȡ��ڶ�������ͬһ������
//          ���У����յ������ӵ��ȡ�
//            1.��Ӳ���жϷ�����߽��̵Ľ�ֹ��ռ��־��λ���������е��ȡ�������
//          ϵͳ������������ķ�ʽ�����жϴ�����ϻ�������ռ���ڽ��е��ȡ����
//          ���ִ������ж�Ƕ�ף�����Ӱ��ʵʱ���ܡ����������ϵͳ�����˴�������
//          ��Ƕ�ף����Ŀ������жϴ��������Ƴ��˴����⣬Ҫ��������ˡ�
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2013-11-26  |               |  �޸ĵ����㷨���������ȼ�λͼ��
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_sched(int refresh)
{
    proc_t        * temp    = NULL,       /*  ���ȼ��б�ѭ������     */
                  * next    = NULL;       /*  ��һ����Ҫ���еĽ���   */
    int             prio    = 0;
    CRITICAL_DECLARE(proc_rspl_lock);

#ifdef _CFG_DEBUG_
    Proc_sched_msg(NULL,NULL);
#endif  /*  _CFG_DEBUG_ */

    /*
     *  �����ڴ����ж�ʱ���ȣ�
     *  ��ȷʵ��Ҫ���ȣ���˲�����ϵͳ������������ķ�ʽ������
     */
    if( interrupt_nest || proc_current->proc_seize )
    {
        PROC_NEED_SCHED();
        return ;
    }

    /*
     *    �����漰���̲��ң�����ǲ����˹��жϵ�CSPF����Ҫע������жϵ�ʱ��
     */
    CRITICAL_BEGIN();
    /*
     *  ����ǰ������ˢ�½��̵������ӣ��Ѿ�������һ�ν��̳�
     *  ��ʵʱ��Ҫ��ߵ�����£���Ӧ��ˢ�µ������ӡ���Ӧ��ֱ�ӽ�����ȣ�
     */
    if( PROC_REFRESH_SCHED_FACTOR == refresh )
        Proc_refresh_sched_factor();

    /*
     *  ע��: 
     *      �����۰���ҵķ��������Ҵ��ڽ��̵�������ȼ����������������ȼ�λ
     *  ͼ����ӦλΪ1���ͱ�ʾ������ȼ����ڽ��̡�
     *      ���ȱȽ�ǰ32�����ȼ��������ǶԱ�ʾǰ32�����ȼ�������16λλͼ���л�
     *  ���㣬��������Ϊ�㣬�ͱ�ʾǰ32�����ȼ��д��ڽ��̡�Ȼ������ǰ��16����
     *  �ȼ����м�⣬�����Ǽ����λͼ�Ƿ�Ϊ0�������Ϊ0�������ȼ���0��ʼ��
     *  �ҡ�����������ơ�
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
     *    �������˵��λͼ��Ϊ0����һ�����λͼ�ĵ�8λ�������8λΪ0����ʾ��8
     *  λ��Ϊ0�����Խ�һ����С���ȼ��Ĳ��ҷ�Χ
     */
    if( !(proc_prio_map[ prio / 16 ] & 0x00FF ) ) 
        prio += 8;

    /*
     *    ͨ��֮ǰ�Ĳ��ң����Խ�ɨ������������8�����ڡ�ͨ���鿴���ɵĻ����룬
     *  ���ҵ����ڽ������ȼ����ܹ���Լ��Ҫִ��90��ָ�
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
     *  ��ѡ������������ߵĽ���
     *    ���ڵ������ӿɱ䣬�������ֻ�ܲ���˳������ķ�ʽ�����ҡ����Ҫ��֤
     *  ʵʱ�ԣ���Ӧ����ÿ�����ȼ��еĽ���������������ò�Ҫ����8����
     *    �����ÿ�����ȼ���8�����̣����ҽ����ܹ���Ҫִ��90��ָ���ǰ�泬��
     *  ���ȼ���90��ָ����ӣ��ҵ���һ�����еĽ����ܹ���Ҫִ��180��ָ�����
     *  50MHz��CPU����ÿ��ָ����Ҫ3��ʼ�����ڼ��㣬����û����ˮ�߼��������ܹ�
     *  ��ʱ��
     *          180 * 3 / 50M = 0.0000108 (��) = 10.8 (΢��)
     *    ������ʵ����Ż����������Ҫ��������ȼ�������8���������ϣ�ÿ������
     *  ��ֻ����1-3�����̣������ܹ���Ҫִ�е�ָ���ԼΪ50�������ܹ���ʱ��
     *          50 * 3 / 50M = 0.000003 (��) = 3 (΢��)
     *    �Ѿ��㹻���ˣ����û�����о�������㷨��
     */
    while(temp)
    {
        if( temp->proc_sched_factor > next->proc_sched_factor )
            next = temp;
        temp = temp->proc_sched_next;
    }

    proc_need_sched             = 0;    /*  ÿ�ε��Ⱥ󣬶����ñ�־��0   */
    next->proc_sched_factor     = 0;    /*  ���CPU�󣬵�����������     */
    next->proc_cpu_time         = proc_cpu_time;

    CRITICAL_END();
        
    /*  
     *  ��ѡ���Ĳ��ǵ�ǰ���̣���ִ���л�    
     */
    if( next != proc_current )                      
        PROC_SWITCH_TO(next);   
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Proc_need_schedule
//  ��  ��: ��ʾϵͳ���е��ȣ�
//  ��  ��: ��
//  ����ֵ: ��
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-11-15  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_need_schedule(void)
{
    PROC_NEED_SCHED();
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Proc_sleep
//  ��  ��: ʹ���̽���˯��״̬��������CPU������
//  ��  ��: ��
//  ����ֵ: ��
//  ע  ��: ʹ�øú�������˯��״̬�󣬻������벻���ĵط������ѡ�
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_sleep(void)
{
    /*
     * ϵͳ���̽���˯��״̬��˵���д�����
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
//  ��  ��: Proc_wakeup
//  ��  ��: ����ϵͳ�����е���Proc_sleep��������˯��״̬�Ľ��̡�
//  ��  ��: ��
//  ����ֵ: ��
//  ע  ��:   ���ϵͳ�д��ڳ���2��������˯�ߵĽ��̣����øú�������Щ����ȫ��
//          ���ᱻ���ѡ����ʹ��Proc_sleep����ʱ��Ҫע�⵽��һ�㣬�������벻��
//          �ĵط������ѡ�
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
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
             *  �и������ȼ��Ľ��̣���֪ͨϵͳ��Ҫ���е���
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
//  ��  ��: Proc_delay
//  ��  ��: �ޡ�
//  ��  ��: 
//      millisecond : uint32_t      |   ��Ҫ�ӳٵ�ʱ�䣬�Ժ���Ϊ��λ
//  ����ֵ: ��
//
//  ע  ��:   ���øú����󣬽�����ͣһ��ʱ�䣬�����ʱ���ɲ���������������ͣ��
//          �䣬����CPU������ʱ��Ƶ����ʱ�����һ��ƫ��Լ����ָ̻����к�һ
//          ���ܹ��������CPU������ӳ�ʱ�䲻һ���ܹ���ȫ�������ȣ���������
//          ��Ȳ����ṩ��ʱ���Գ���
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
//
///////////////////////////////////////////////////////////////////////////////
*/
void        Proc_delay(uint32_t millisecond)
{
    CRITICAL_DECLARE(proc_delay_lock);
    /*
     *  ��ʱ��ת��Ϊʱ���жϴ���
     */
    proc_current->proc_alarm = MILIONSECOND_TO_TICKS(millisecond);          

    if( proc_current->proc_alarm )  /*  ȷʵ��Ҫ�ӳٵ�ʱ�򣬲ŷ���CPU   */
    {
        /*
         *  ֻ��ʹ�ý�ֹ��ռ�����ﲢ���Ǳ���ĳ����������������Ҫ��һ�����Ĳ���
         *  ���������������л����������̣����ǿ��Դ����жϡ�
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
//  ��  ��: Proc_signal_pid
//  ��  ��: ����̷����źš�
//  ��  ��: 
//      pid         : int           +   ���̱��
//      sigmap      : uint_t        +   �ź�λͼ
//  ����ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���RESULT_FAILED��
//
//  ע  ��:   ����ͬʱ���Ͷ���źţ�����ź��á��򡱲����������ӡ���������Ѿ�
//          �������Ӧ���źţ����ظ���¼������˯��״̬�Ľ��̻����������ѡ�
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
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

    /*  ���ź�λͼ                            */
    proc->proc_signal_map |= PROC_MAKE_SIGNAL(signal);

    /*
     *  ���̴���˯��״̬�����ѽ���
     */
    if( PROC_STAT_IS_SLEEP(proc) )
    {
        Sched_add(proc);
        proc->proc_stat = PROC_STAT_RUN;
        if( proc->proc_priority < proc_current->proc_priority )
            PROC_NEED_SCHED();    /*  �и����ȼ��Ľ��̽�������̬����Ҫ�л�  */
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
//  ��  ��: Proc_get_name
//  ��  ��: ��õ�ǰ���̵����ơ�
//  ��  ��: 
//      name        : char[PROC_NAME_LEN]+   ���ƻ�����
//  ����ֵ: ��ָ���ǲ��������ָ�롣
//
//  ע  ��:   Ӧȷ������Ļ������㹻����LenixĬ�ϵĽ�������Ϊ12���ֽڣ���˻���
//          ������Ϊ12���ֽڡ�
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
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
//  ��  ��: Proc_get_run_time
//  ��  ��: ��õ�ǰ�����Ѿ����е�ʱ�䡣
//  ��  ��: ��
//  ����ֵ: ��ǰ�����Ѿ����е�ʱ�䣬��ʱ���жϴ������㡣
//  ע  ��: ����������ڼ��޸���ϵͳ��ʱ��Ƶ�ʣ���ֵ����ʾ��ʱ�佫�������㡣
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
uint32_t    Proc_get_run_time(void)
{
    return proc_current->proc_run_time;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Proc_get_stack_size
//  ��  ��: ��õ�ǰ���̵�ջ������
//  ��  ��: ��
//  ����ֵ: ��ǰ���̵�ջ������
//  ע  ��: ��
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
uint_t      Proc_get_stack_size(void)
{
    return proc_current->proc_stack_size;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Proc_get_pid
//  ��  ��: ��õ�ǰ���̵�Ψһ��š�
//  ��  ��: ��
//  ����ֵ: ��ǰ���̵�Ψһ��š�
//  ע  ��: ��
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-01-02  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
uint_t      Proc_get_pid(void)
{
    return proc_current->proc_pid;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Proc_get_priority
//  ��  ��: ��ý��̵����ȼ���
//  ��  ��: ��
//  ����ֵ: ���̵����ȼ���
//  ע  ��: ��
//
//  �����¼:
//  ʱ��        |  ����         | ˵��
//=============================================================================
//  2012-11-17  |  �ޱ�         +   �Ļ�õ�ǰ���̵����ȼ�����Ϊ���ָ�����̵�
//                              | ���ȼ�
//  2012-01-02  |  �ޱ�         | ��һ��
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
//  ��  ��: Proc_set_priority
//  ��  ��: �ı���̵����ȼ���
//  ��  ��: 
//      pid         : int           |   ���̱��
//      priority    : byte_t        |   ��Ҫ�����������ȼ���
//  ����ֵ: ����ԭ���ȼ���
//  ע  ��:   ����Ӧȷ����[0 , 63]֮�䡣����������ڸ÷�Χ��,ϵͳ��Բ������е�
//          ������Ϊ�����ɿ��ơ�
//
//  �����¼:
//  ʱ��        |  ����         | ˵��
//=============================================================================
//  2012-11-17  |  �ޱ�         +   �Ļ�õ�ǰ���̵����ȼ�����Ϊ���ָ�����̵�
//                              | ���ȼ�
//  2012-01-02  |  �ޱ�         +  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
byte_t      Proc_set_priority(int pid, byte_t priority)
{
    byte_t          pprio       = PROC_INVALID_PRIORITY;    /*  ԭ���ȼ� */
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
     *  ���ȼ���ͬ�����õ���
     */
    if( pprio == priority)
        goto set_priority_end;

    /*
     *  �������ȼ��Ĳ����Ϊ3��
     *  1.��ԭ���ȼ��б���ɾ��
     *  2.�������ȼ�
     *  3.���������ȼ��б�
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
//  ��  ��: Proc_get_prio_num
//  ��  ��: ��ý��̵���������
//  ��  ��: 
//      pid         : int           |   ���̱��
//  ����ֵ: ��ǰ���̵���������
//  ע  ��: ���жϵ�ʱ����ܽϳ�����������ʵʱ��Ҫ��ߵĳ����С�
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-11-17  |  �ޱ�         |  �����Ե�ǰ�����޸�Ϊ��ָ������
//  2012-01-02  |  �ޱ�         |  ��һ��
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
//  ��  ��: Proc_set_prio_num
//  ��  ��: ���ý��̵���������
//  ��  ��: 
//      pid         : int           |   ���̱��
//      prionum     : byte_t        |   ��������
//  ����ֵ: ����ԭ������������
//  ע  ��: ���жϵ�ʱ����ܽϳ�����������ʵʱ��Ҫ��ߵĳ����С�
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//=============================================================================
//  2012-11-17  |  �ޱ�         |  �����Ե�ǰ�����޸�Ϊ��ָ������
//  2012-01-02  |  �ޱ�         |  ��һ��
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
