/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: clock.c
//  ����ʱ��: 2011-12-30        ������: �ޱ�
//  �޸�ʱ��:                   �޸���: 
//  ��Ҫ����: �ṩ�������ַ���������
//
//  ˵    ��: ���ļ��ṩ��ϵͳʱ�ӡ���ʱ�����ܡ�
//            ϵͳʱ���ṩ�˻�ȡ����ʱ�䡢��ʱ������ʱ���жϻص������Ĺ��ܡ�
//            ��ʱ���ṩ�˴��������ٹ��ܡ�
//
//  �仯��¼:
//  �� �� ��    |   ʱ  ��    |   ��  ��    |  ��Ҫ�仯��¼
//=============================================================================
//              | 2011-12-30  |   ��  ��    |  
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

volatile uint32_t           ticks;                 /*  ʱ�ӽ��ļ�����       */
static ticks_hook_t         clk_ticks_hook;        /*  ʱ���жϻص�����     */
static timer_t              timer_pool[TIMER_MAX]; /*  ϵͳ��ʱ����         */

#ifdef _CFG_SMP_
static spin_lock_t          ticks_lock;         /*  ʱ�ӽ��ļ��������������*/
static spin_lock_t          timer_lock;         /*  ϵͳ��ʱ���ػ��������  */
#endif  /*  _CFG_SMP_   */


#ifdef _CFG_TIMER_ENABLE_
/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Timer_create
//  ��  ��: ������ʱ����
//  ��  ��: 
//      millisecond : uint32_t      |   ��ʱ������Ժ���Ϊ��λ���ڵ��øú�����
//                                  | ��millisecond���룬���ö�ʱ���������
//      repeat      : int           |   �ظ�����������ܳ���TIMER_REPEAT_MAX
//                                  | �Σ�Lenix����TIMER_REPEAT_MAXΪ30000�Ρ�
//                                  | �����Ҫ�������޶�ʱ�����򽫸ò�������Ϊ
//                                  | TIMER_REPEAT_INFINITE��
//      handle      : void (*)(void *)
//                                  |   ��ʱ����������ڶ�ʱ��Ϻ���á�
//      param       : void *        |   ��ʱ������������
//  ����ֵ: 
//    ����: int
//    ˵��: �����ɹ��򷵻ض�ʱ����ţ�ʧ�ܷ���0��
//
//  ע  ��:   �޸�ϵͳʱ��Ƶ�ʺ󣬻ᵼ��ԭ��ʱ����ʱ�仯�����²���Ԥ�ϵ������
//          ������޸�ϵͳʱ��Ƶ���Ժ���Ҫ�ؽ���ʱ����
//
//            ��ʱ��������ʱ���ж�ʱ���ã�������ù���Ķ�ʱ�����ᵼ��ϵͳЧ��
//          ���͡����Ҵ������Ӧ�еȴ���˯�ߵ�ʱ�䲻��Ԥ��Ĳ�����Ҳ��Ӧ����
//          ʱ�������ڴ�������ڡ����仰˵����ʱ���������Ӧ�þ������١���С��
//
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
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
    if( ticks <= 0 || NULL == handle)   /*  ����У��    */
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
    if( tm )    /*  tm��Ϊ0����ʾϵͳ���Դ�����ʱ��     */
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
         *  �Զ�ʱ���������ã���ʱ�������޺��ظ�����������������
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
//  ��  ��: Timer_delete
//  ��  ��: 
//  ��  ��: 
//      id          : int           |   ��ʱ�����
//  ����ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���RESULT_FAILED��
//  ע  ��: id�����ڣ��Ϳ���ɾ��ʧ��
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
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
//  ��  ��: Timer_handle
//  ��  ��: ��ʱ���������
//  ��  ��: ��
//  ����ֵ: ��
//  ע  ��: 
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
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
     *  ����Ҫע��: 
     *  �ڴ���ʱ����ʱ�������ʱ���϶࣬��ִ��ʱ�䳤�����п��ܷ���Ƕ�ף�
     *  ���Ӧ���ü�����
     */
    if( Cpu_tas(&watch,0,1) == 0 )
        return ;
    for( i = 0 ; i < TIMER_MAX ; i++)
    {
        tm = timer_pool + i;
        /*  
         *  ������Ч�Ļ���δ��ʱ�Ķ�ʱ��
         */
        if( !TIMER_IS_VALID(tm) || --tm->tm_left > 0)       
            continue;
        tm->tm_handle(tm->tm_param);
        /*
         *  ����������ظ���ʱ�����������ö�ʱ���
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
//  ��  ��: Clk_do_clock
//  ��  ��: ʱ���жϴ������
//  ��  ��: ��
//  ����ֵ: ��
//  ע  ��:     ʱ���жϴ��������ÿ��ʱ���ж�ʱ���ᱻ���ã����������Ҫ�����
//          ����ʱ���ص���������ʱ����ˢ�½��̵ĵ������ӡ����ϵͳ���̽϶ࡢ��
//          ʱ���϶����Ҵ���ʱ��ϳ��������жϷ���ǰ��������ִ�е��ȣ����ᵼ��
//          �������ִ��ʱ�������
//
//          ϵͳЧ�ʷ�����
//              ʵʱϵͳҪ���ʱ��Ƶ�ʽϸߣ���˽�ʱ��Ƶ�ʼ���Ϊ200Hz��Ҳ����ÿ
//          ��200�Ρ�CPU����Ƶ�ʼ���Ϊ60M��ƽ��ÿ��ָ����Ҫ3��ʱ�����ڣ������
//          �����£�����ϵͳ�Ļ���������
//              ÿ��ʱ���ж϶���Ҫִ��5������
//                1.���������ʱ��
//                2.ʱ���жϻص�������
//                3.����ϵͳ��ʱ����
//              ��ʵ������У�1��2���������Ҫʱ���Ų���ִ�У�ͨ������£�ֻ
//          ��һ���򵥵ĺ������á���1��2���а���������Ȼ������CPUʱ�䣬������
//          ������Ҫ��������ϵͳ�Ķ�������
//              ��3�ϵͳ��ʱ������Ŀǰ���ñ����ķ�ʽ�������ڹ涨�˶�ʱ����
//          �����Ŀ�����������Ŀ�ǳ�С��Ĭ��Ϊ6��������һ��ֻ��Ҫִ�м�ʮ��ָ
//          ���ϵͳЧ��û��Ӱ�졣�������ϵͳ��ʱ�������Ѿ���ִ�й��ܣ���Ҫ
//          ����Ӧ��ʱ�䣬��˵�3���ϵͳ��Ӱ�첻��
//              Ԥ��ǰ3���ܹ�Ҫִ�д�Լ150��ָ���ʱ
//                ( 3 * 150 ) / 60M = 0.0000715 ��
//            
//  �����¼:
//  ʱ  ��      |  ��  ��       |  ˵��
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
     *  ���ûص�����
     */
    clk_ticks_hook();
#ifdef _CFG_TIMER_ENABLE_
    /*  
     *  ����ϵͳ��ʱ��
     */
    Timer_handle();
#endif  /*  _CFG_TIMER_ENABLE_   * /
    /*  
     *  ����ʱ������������¼���������Ӳ��ұ��Ϊ��Ҫ���ȣ���ÿ���жϻ���
     *  ϵͳ���ú󣬶�����е���
     */
    if( proc_current->proc_cpu_time < 1 )
    {                    
        /*  ����ĳЩ��������������     */
        notuse1 = notuse1;
        notuse2 = notuse2;
        proc_current->proc_cpu_time = 0;
        proc_need_sched = 1;
    } 
    
    return 0;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Clk_get_ticks
//  ��  ��: ���ʱ�ӽ��ļ���
//  ��  ��: ��
//  ����ֵ: 
//    ����: uint32_t
//    ˵��: ʱ�ӽ��ļ���
//  ע  ��: ��8λ��16λ����£���Ҫʹ���ٽ�δ���ʽ���ڲ��÷�ʽ2���ٽ�α���
//          ��ʽʱ��õ�ʱ����ܴ���ƫ�
//
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
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
//  ��  ��: Clk_delay
//  ��  ��: ��ʱ
//  ��  ��: 
//      millisecond         : uint32_t
//  ����ֵ: ��
//
//  ע  ��: æ�ȴ���ʱ������һֱ��������״̬���ڲ����������ȼ����̴�ϵ������
//          ���Դﵽ׼ȷ��ʱ��Ч����
//
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
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
//  ��  ��: Clk_ticks_hook_get
//  ��  ��: ���ʱ���жϻص�����
//  ��  ��: 
//      millisecond         : uint32_t
//  ����ֵ: ��
//  ע  ��: 
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
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
//  ��  ��: Clk_ticks_hook_set
//  ��  ��: ��ʱ
//  ��  ��: 
//      millisecond         : uint32_t
//  ����ֵ: ��
//  ע  ��: 
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
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