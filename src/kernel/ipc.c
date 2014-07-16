/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: ipc.c
//  ����ʱ��: 2011-07-12        ������: �ޱ�
//  �޸�ʱ��: 2014-02-16        �޸���: �ޱ�
//  ��Ҫ����: �ṩsemaphore��mutex��message��
//  ˵    ��: 
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ��  ��       |  ��Ҫ�仯��¼
//=============================================================================
//              |   2014-02-16  |  ��  ��       | ����KOUM�󣬴���޸�
//              |   2012-11-13  |  ��  ��       | �ع�����spin_lock��lock��ʵ��
//                                              | ���������ļ����Ѷ���ĳЩ�ֶ�
//                                              | �������������ȼ���ת�����⡣
//  00.00.000   |   2011-07-02  |  ��  ��       | ��һ��ľ���ʱ���Ѿ����ǵ���
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>
#include <assert.h>

#include <proc.h>
#include <ipc.h>

#ifdef _SLASH_
    #include <machine/machine.h>
#else
    #include <machine\machine.h>
#endif  /*  _SLASH_ */

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Lck_lock
//  ��  ��: ��������˯�ߣ��ȴ�����������ٽ���Դ��
//  ��  ��: ��
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      lock_t *    |   lck         | ������ָ��
//  ����ֵ: �ޡ�
//  ע  ��: ����ֱ��ʹ��ָ�룬���ڵ���ʱ�ṩ������⡣
//            ������Lck_free�ɶ�ʹ�á��ڻ�����Դ��Ҫ�ȴ��ϳ�ʱ�������£�Ӧʹ
//          �øú����ԣ�������ʹ��Spin_lock�����ԡ�
//            ��ռ�����Ժ󣬲�Ӧ�ó�����������CPU�������
//  �����¼:
//  ʱ  ��      |   ��  ��      |  ˵  ��
//=============================================================================
//  2012-11-17  |   ��  ��      |  �������ȼ���ת���
//  2012-01-01  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Lck_lock(lock_t * lck)
{
    CRITICAL_DECLARE(lck->lck_lock);

    ASSERT(NULL != lck);
    while( Cpu_tas((int *)&lck->lck_status,0,1) )
    {        
        /*
         *  ���ܴ��ڶ������ͬʱ���ܻ����
         */
        CRITICAL_BEGIN();
        /*
         *    ������Ի�����Ľ������ȼ����������̵����ȼ�������ܻ�������ȼ�
         *  ��ת��Lenix����������ȼ��ķ�ʽ�������������
         */
        if( proc_current->proc_priority < lck->lck_priority )
        {
            /*
             *  ����������ȼ�����ߵ���Ҫռ�ö���Ľ�������ߵ����ȼ�
             */
            lck->lck_priority = proc_current->proc_priority;
            /*
             *  �������̵����ȼ���ߣ�������RSPL��
             */
            Sched_del(lck->lck_user);
            lck->lck_user->proc_priority = proc_current->proc_priority;
            Sched_add(lck->lck_user);
        }
        CRITICAL_END();
        Proc_sleep();
    }
    lck->lck_user         = proc_current;
    lck->lck_priority     = proc_current->proc_priority;
    lck->lck_user_prio    = proc_current->proc_priority;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Lck_free
//  ��  ��: �ͷ�����
//  ��  ��: ��
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      lock_t *    |   lck         | ������ָ��
//  ����ֵ: �ޡ�
//  ע  ��: ����ֱ��ʹ��ָ�룬���ڵ���ʱ�ṩ������⡣
//  �����¼:
//  ʱ  ��      |   ��  ��      |  ˵  ��
//=============================================================================
//  2012-11-17  |   ��  ��      |  �������ȼ���ת���
//  2012-01-01  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Lck_free(lock_t * lck)
{
    CRITICAL_DECLARE(lck->lck_lock);

    ASSERT(NULL != lck);
    CRITICAL_BEGIN();
    if( proc_current->proc_priority != lck->lck_user_prio )
    {
        Sched_del(proc_current);
        proc_current->proc_priority = lck->lck_user_prio;
        Sched_add(proc_current);
    }
    lck->lck_user         = NULL;
    lck->lck_priority     = 0;
    lck->lck_user_prio    = 0;
    lck->lck_status       = 0;
    CRITICAL_END();
    Proc_wakeup();        
}
/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  ����
*/
#ifdef _CFG_MUTEX_ENABLE_

static mutex_t              mutex_pool[MUTEX_MAX];
#ifdef _CFG_SMP_
static spin_lock_t          mutex_lock;
#endif  /*  _CFG_SMP_   */

#define MUTEX_FIRST                 (mutex_pool)
#define MUTEX_LAST                  (&mutex_pool[MUTEX_MAX - 1])

void        Mutex_initial(void)
{
    _printk("mutex initial...     ");
#ifdef _CFG_SMP_
    mutex_lock  = 0;
#endif  /*  _CFG_SMP_   */
    _memzero(mutex_pool,MUTEX_MAX * sizeof(mutex_t));
    _printk("OK!\n");
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Mutex_release
//  ��  ��: �ͷŻ������
//  ��  ��: ��
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     mutex_t *    |   mutex       | �������ָ��
//  ����ֵ: �ޡ�
//  ע  ��: ����Koum_release���á�
//  �����¼:
//  ʱ  ��      |   ��  ��      |  ˵  ��
//=============================================================================
//  2012-11-21  |   ��  ��      |  ��д
//  2012-01-01  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
static
result_t    Mutex_release(mutex_t * mutex)
{
    result_t                result      = RESULT_SUCCEED;
    CRITICAL_DECLARE(mutex_lock);

    ASSERT(mutex);
    /*
     *  ����ϵͳ����Ķ��󣬲���ɾ��
     */
    if( mutex < MUTEX_FIRST || mutex > MUTEX_LAST)
    {
        PROC_SET_ERR(ERR_MUTEX_OUT_OF_POOL);
        result = ERR_MUTEX_OUT_OF_POOL;
        goto mutex_destroy_end;
    }
    CRITICAL_BEGIN();
    /*
     *  �н����ڵȴ��ģ���������
     */
    if( mutex->mtx_wait.pl_list )
    {
        PROC_SET_ERR(ERR_MUTEX_BUSY);
        result = ERR_MUTEX_BUSY;
    }
    else
    {
        MUTEX_INITIAL(mutex);
        PROC_NO_ERR();
    }
mutex_destroy_end:
    CRITICAL_END();

    return result;
}
/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Mutex_create
//  ��  ��: �����������
//  ��  ��: ��
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
//  2012-11-21  |   ��  ��      |  ��д
//  2012-01-01  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
handle_t    Mutex_create(void)
{
    int             i       = 0;
    mutex_t       * mtx     = mutex_pool;
    handle_t        handle  = INVALID_HANDLE;
    CRITICAL_DECLARE(mutex_lock);

    /*
     *  ����ϵͳ�������أ����ҿ��õĻ������
     */
    CRITICAL_BEGIN();
    for( ; i < MUTEX_MAX ; i++,mtx++)
    {
        if( MTX_FLAG_USED & mtx->mtx_flag )
            continue;
        mtx->mtx_flag |= MTX_FLAG_USED;
        break;
    }
    CRITICAL_END();
    /*
     *  ����Ƿ���䵽�������������䵽����Ҫ�����ں˶����
     */
    if( i < MUTEX_MAX )
    {
        handle = Koum_add(mtx,Mutex_release,kot_mutex,HANDLE_ATTR_RDWR);
        /*
         *  ����ϵͳ�ں˶����ʧ�ܣ���Ҫ�ͷ��ѷ��䵽�Ļ������
         */
        if( INVALID_HANDLE == handle )
        {
            CRITICAL_BEGIN();
            _memzero(mtx,sizeof(mutex_t));
            CRITICAL_END();
            PROC_SET_ERR(ERR_MUTEX_KOUM_ADD);
        }
        else
            PROC_NO_ERR();
    }
    else
        PROC_SET_ERR(ERR_MUTEX_EXHAUST);

    return handle ;
}


/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Mutex_get
//  ��  ��: ��û������
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   ���������
//  ����ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���RESULT_FAILED��
//  ע  ��:   ��������������ȼ��Ľ��̵ȴ��������Lenix���������ռ�û����
//          ����̵����ȼ���
//            ���ø�API����Ҫ��ⷵ��ֵ��
//  �����¼:
//  ʱ��        |   ��  ��      | ˵��
//=============================================================================
//  2014-02-16  |   ��  ��      | ����KOUMҪ���޸�
//  2012-07-03  |   ��  ��      | ���ӷ�ֹ���ȼ���ת�Ĺ���
//  2012-01-09  |   ��  ��      | ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Mutex_get(handle_t handle)
{
    mutex_t       * mutex   = Koum_handle_object(handle);
    CRITICAL_DECLARE(mutex->mtx_lock);

    ASSERT(handle);

    /*
     *  �������Ƿ���Ч
     */
    if( INVALID_HANDLE == handle || NULL == mutex )
        return RESULT_FAILED;
    while( Cpu_tas((int *)&mutex->mtx_status,0,1) )
    {
        CRITICAL_BEGIN();
        /*
         *  �������ȼ���ת���⣬�и������ȼ��Ľ������󻥳���
         */
        if( proc_current->proc_priority < mutex->mtx_priority )
        {
            /*
             *    ��ռ�û������Ľ�����ߵ��ý��̣�Ҳ���ǽ��ý��̸����������ȼ�
             *  ���б���ע�⣬���ﲢ��Ҫ����������
             */
            Sched_del(mutex->mtx_user);
            mutex->mtx_user->proc_priority = proc_current->proc_priority;
            Sched_add(mutex->mtx_user);
            mutex->mtx_priority = proc_current->proc_priority;
        }
        Proc_wait_on(&mutex->mtx_wait);
        CRITICAL_END();
        Proc_sched(0);
    }
    mutex->mtx_user      = proc_current;
    mutex->mtx_user_prio = proc_current->proc_priority;
    mutex->mtx_priority  = proc_current->proc_priority;

    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Mutex_put
//  ��  ��: �ͷŻ������
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   ���������
//  ����ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���RESULT_FAILED��
//  ע  ��: ���ø�API����Ҫ��ⷵ��ֵ��
//  �����¼:
//  ʱ��        |   ��  ��      | ˵��
//=============================================================================
//  2014-02-16  |   ��  ��      | ����KOUMҪ���޸�
//  2012-07-03  |   ��  ��      | ���ӷ�ֹ���ȼ���ת�Ĺ���
//  2012-01-09  |   ��  ��      | ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Mutex_put(handle_t handle)
{
    mutex_t       * mutex   = Koum_handle_object(handle);
    CRITICAL_DECLARE(mutex->mtx_lock);

    ASSERT(handle);
    /*
     *  �������Ƿ���Ч
     */
    if( INVALID_HANDLE == handle || NULL == mutex )
        return RESULT_FAILED;
    CRITICAL_BEGIN();
    /*
     *    ���������ռ�ý��̵����ȼ���ԭʼ�����ȼ���ͬ����˵�����������ȼ���
     *  ������Ҫ���ԭ�������ȼ�
     */
    if( mutex->mtx_user_prio != proc_current->proc_priority )
    {
        /*  �����ȼ��Ļ�ԭʼ���ȼ�  */
        Sched_del(mutex->mtx_user);
        proc_current->proc_priority = mutex->mtx_user_prio;
        Sched_add(mutex->mtx_user);
    }
    mutex->mtx_user_prio    = 0;
    mutex->mtx_priority     = 0;
    mutex->mtx_status       = 0;
    mutex->mtx_user         = NULL;
    /*  �������еȴ��ĵĽ���    */
    Proc_resume_on(&mutex->mtx_wait);
    CRITICAL_END();
    SCHED(0);
    return RESULT_SUCCEED;
}

#endif  /*  _CFG_MUTEX_ENABLE_  */

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  �ź�������ֵ�źţ�
*/
#ifdef _CFG_SEMAPHORE_ENABLE_

static semaphore_t          sema_pool[SEMA_MAX];    /*  ϵͳ�ź��������    */

#ifdef _CFG_SMP_
static spin_lock_t          sema_lock;              /*  ������              */
#endif  /*  _CFG_SMP_   */

#define SEMA_FIRST                  (sema_pool)
#define SEMA_LAST                   (&sema_pool[SEMA_MAX - 1])

void        Sema_initial(void)
{
    _printk("Sema initial...   ");
    _memzero(sema_pool,SEMA_MAX * sizeof(semaphore_t));

#ifdef _CFG_SMP_
    sema_lock = 0;
#endif  /*  _CFG_SMP_   */
    _printk("OK!\n");
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Sema_release
//  ��  ��: �ͷ��ź�������
//  ��  ��: ��
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//   semaphore_t *  |    sema       | �ź�������ָ��
//  ����ֵ: �ޡ�
//  ע  ��: ����Koum_release���á�
//  �����¼:
//  ʱ  ��      |   ��  ��      |  ˵  ��
//=============================================================================
//  2012-11-21  |   ��  ��      |  ��д
//  2012-01-01  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
static
result_t    Sema_release(semaphore_t * sema)
{
    CRITICAL_DECLARE(sema_lock);

    ASSERT( NULL != sema );
#ifdef _CFG_CHECK_PARAMETER_
    if( NULL == sema)
    {
        PROC_SET_LAST_ERR(ERR_NULL_POINTER);
        return ERR_NULL_POINTER;
    }
#endif  /*  _CFG_CHECK_PARAMETER_  */
    /*
     *  �����������Ķ���
     */
    if( sema < SEMA_FIRST || sema > SEMA_LAST )
    {
        PROC_SET_LAST_ERR(ERR_SEMA_NOT_SYS_CREATE);
        return ERR_SEMA_NOT_SYS_CREATE;
    }
    CRITICAL_BEGIN();
    /*
     *  ���н����ڵȴ��ź�������������
     */
    if( sema->sema_count < sema->sema_max )
    {
        PROC_SET_LAST_ERR(ERR_SEMA_BUSY);
        CRITICAL_END();
        return ERR_SEMA_BUSY;
    }
    SEMA_ZERO(sema);
    CRITICAL_END();
    PROC_NO_ERR();
    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Sema_create
//  ��  ��: �����ź�������
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      long        |     max       |   �ź������ֵ
//  ����ֵ: ʧ�ܷ���INVALID_HANDLE���ɹ���������ֵ��
//  ע  ��: 
//  �����¼:
//  ʱ  ��      |   ��  ��      | ˵��
//=============================================================================
//  2014-02-17  |   ��  ��      | ����KOUMҪ���޸�
//  2012-01-09  |   ��  ��      | ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
handle_t    Sema_create(long max)
{
    int             i       = 0;
    handle_t        handle  = INVALID_HANDLE;
    semaphore_t   * sema    = SEMA_FIRST;
    CRITICAL_DECLARE(sema_lock);
    
    ASSERT( max > 0 && max < SEMA_LIMIT);
    /*
     *  ���ź�������������
     */
    if( max > SEMA_LIMIT)
    {
        PROC_SET_ERR(ERR_SEMA_MAX_OVERFLOW);
        return INVALID_HANDLE;
    }

    CRITICAL_BEGIN();
    /*
     *  �����źŶ���أ����ҿ��ö������ź����ֵΪ0��Ϊ���õ�����
     */
    for( i = 0 ; i < SEMA_MAX ; i++,sema++)
    {
        if( 0 == sema->sema_max )
        {
            SEMA_INITIAL(sema,max);
            break;
        }
    }
    CRITICAL_END();
    if( i >= SEMA_MAX )
    {
        PROC_SET_ERR(ERR_SEMA_EXHAUST);
        goto sema_create_end;
    }
    handle = Koum_add(sema,Sema_release,kot_sema,HANDLE_ATTR_RDWR);
    if( INVALID_HANDLE != handle )
        PROC_NO_ERR();
    else
    {
        CRITICAL_BEGIN();
        SEMA_ZERO(sema);
        CRITICAL_END();
        PROC_SET_ERR(ERR_SEMA_KOUM_ADD);
    }
sema_create_end:
    return handle;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Sema_down
//  ��  ��: �����ź�ֵ�����ź�ֵ�þ��󣬽��̽���ȴ�״̬��
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   �ź���������
//  ����ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���RESULT_FAILED��
//  ע  ��:   ���ø�API����Ҫ��ⷵ��ֵ��
//             1���źż�����Ϊ�Ǹ�ʱ��˵��û�н��̵ȴ���
//  �����¼:
//  ʱ  ��      |   ��  ��      | ˵  ��
//=============================================================================
//  2014-02-17  |   ��  ��      | ����KOUMҪ���޸�
//  2012-07-03  |   ��  ��      | ���ӷ�ֹ���ȼ���ת�Ĺ���
//  2012-01-09  |   ��  ��      | ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Sema_down(handle_t handle)
{
    semaphore_t   * sema    = Koum_handle_object(handle);
    CRITICAL_DECLARE(sema->sema_lock);

    ASSERT( INVALID_HANDLE != handle );
#ifdef _CFG_CHECK_PARAMETER_
    if( INVALID_HANDLE == handle || NULL == sema )
        return RESULT_FAILED;
#endif  /*  _CFG_CHECK_PARAMETER_   */
    CRITICAL_BEGIN();
#ifdef d_CFG_DEBUG_
    _printf("%s semaphore down count: %d \n",
        proc_current->proc_name,sema->sema_count - 1);
#endif
    if( --sema->sema_count < 0 )
    {
        /*  �źż�����С��0˵����Դ�Ѿ��ľ���������Ҫ�ȴ���Դ���á�*/
        Sched_del(proc_current);
        Proc_wait_on(&sema->sema_wait);
        CRITICAL_END();
        Proc_sched(0);
        /*  ��������˵�������Ѿ��ָ����У�Ҳ˵����Դ���ͷ���һ��������ʹ�á�*/
        return RESULT_SUCCEED;
    }
    CRITICAL_END();   

    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Sema_down
//  ��  ��: �����ź�ֵ�����ź�ֵ�þ��󣬽��̽���ȴ�״̬��
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   �ź���������
//  ����ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���RESULT_FAILED��
//  ע  ��:   ���ø�API����Ҫ��ⷵ��ֵ��
//             1���źż�����Ϊ�Ǹ�ʱ��˵��û�н��̵ȴ���
//            ���н��̵ȴ��ź�ʱ�����ѵȴ��źŽ��������ȼ���ߵĽ��̣��������
//          ����ͬ�����ѵ���������ߵĽ��̡�
//  �����¼:
//  ʱ  ��      |   ��  ��      | ˵  ��
//=============================================================================
//  2014-02-17  |   ��  ��      | ����KOUMҪ���޸ģ�ɾ�����ȼ���ת����
//  2012-07-03  |   ��  ��      | ���ӷ�ֹ���ȼ���ת�Ĺ���
//  2012-01-09  |   ��  ��      | ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Sema_up(handle_t handle)
{
    semaphore_t   * sema    = Koum_handle_object(handle);
    CRITICAL_DECLARE(sema->sema_lock);

    ASSERT( INVALID_HANDLE != handle );
#ifdef _CFG_CHECK_PARAMETER_
    if( INVALID_HANDLE == handle || NULL == sema )
        return RESULT_FAILED;
#endif  /*  _CFG_CHECK_PARAMETER_   */

    CRITICAL_BEGIN();
#ifdef d_CFG_DEBUG_
    _printf("%s semaphore up count: %d \n",
        proc_current->proc_name,sema->sema_count + 1);
#endif
    /*  �������ֵ���϶��д�����  */
    if( ++sema->sema_count > sema->sema_max )
        Sys_halt("semaphore large than max!");
    /*  С��1��˵���еȴ���Դ�Ľ��̣���Ҫ����   */
    if( sema->sema_count < 1 )
        Proc_resume_max_on(&sema->sema_wait);
    CRITICAL_END();
    SCHED(0);
}

#endif  /*  _CFG_SEMAPHORE_ENABLE_  */

/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  IPC����Ϣ����
*/

#ifdef _CFG_MESSAGE_ENABLE_

#ifdef _CFG_SMP_
static spin_lock_t          mb_lock;
#endif  /*  _CFG_SMP_*/

static message_box_t        mb_pool[MB_MAX];

#define mb_POOL_FIRST              (mb_pool)
#define mb_POOL_LAST               (mb_pool + MB_MAX - 1)

void        Msg_initial(void)
{
    _printk("message initial....    ");
#ifdef _CFG_SMP_
    spin_lock_t     = 0;
#endif  /*  _CFG_SMP_*/
    _memzero(mb_pool,sizeof(message_box_t)*MB_MAX);
    _printk("OK!\n");
}

/*  �������� **/
static
result_t    Msg_release(message_box_t * mb)
{
    /*
     *  ���ܴ��ڵ������1������������
     *  ��ƣ��Լ��κ�ʱ�򶼿������ٶ��󣬲����Ƿ������á�
     *  �����߲������ٶ���
     *  ��ȫ���������ͷź󣬲�����ȫ�ͷ�
     */
    CRITICAL_DECLARE(mb_lock);
    --mb->mb_ref;
    /*  ֻ�д���������������    */
    if( proc_current != mb->mb_owner )
        return RESULT_SUCCEED;
    CRITICAL_BEGIN();
    do
    {
        /*
         *  �п��������ٶ���ʱ�����н����ڵȴ����䣬��Ҫ������Щ���̣�ʹ�����
         *  ���к����Ĵ���
         *  Ƕ��ʹ��CSPF��
         */
        CRITICAL_DECLARE(mb->mb_lock);
        CRITICAL_BEGIN();
        Proc_resume_on(&mb->mb_send_wait);
        CRITICAL_END();
    }while(0);
    _memzero(mb,sizeof(message_box_t));
    CRITICAL_END();
    SCHED(0);
    PROC_NO_ERR();

    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Msg_create
//  ��  ��: ������Ϣ����
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//    const char *  |   name        |   ��Ϣ��������
//    message_t *   |   msgbuf      |   ��Ϣ������
//    int           |   count       |   ��Ϣ������
//  ����ֵ: ʧ�ܷ���INVALID_HANDLE���ɹ�����������
//  ע  ��:  �ڴ������ͷ���Ϣ��������������ͷŶ���ʱ�����ͷ�ʧ��
//  �����¼:
//  ʱ  ��      |   ��  ��      | ˵  ��
//=============================================================================
//  2014-02-24  |   ��  ��      | ����KOUMҪ���޸�
//  2012-01-01  |   ��  ��      | ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
handle_t    Msg_create(const char * name,message_t * msgbuf,int count)
{
    int             i       = 0;
    message_box_t * mb      = mb_pool;
    handle_t        handle  = INVALID_HANDLE;
    CRITICAL_DECLARE(mb_lock);

    ASSERT( NULL != name);
    ASSERT( NULL != msgbuf  );
    ASSERT( 0 != count  );
#ifdef _CFG_CHECK_PARAMETER_
    if( NULL == name || NULL == msgbuf || 0 == count )
        goto mb_create_end;
#endif  /*  _CFG_CHECK_PARAMETER_   */
    CRITICAL_BEGIN();
    /*  ���ҵ����ö�����Ϣ����Ļ�����û�з��仺�棬����Ϊ�ö������ */
    for( ; i < MB_MAX ; i++,mb++)
    {
        if( NULL == mb->mb_buffer )
            break;
    }
    if( i >= MB_MAX )
    {
        PROC_SET_ERR(ERR_MSG_NO_SPACE);
        CRITICAL_END();
        goto mb_create_end;
    }
    /*  ռ�ö��󣬲�����������Ϊ��    */
    mb->mb_buffer     = msgbuf;
    mb->mb_name[0]    = 0;
    /*  ����Ƿ�����    */
    for( i = 0 ; i < MB_MAX ; i++ )
    {
        /*  ����û��ʹ�õĶ��󣬴�ʱ����û�����ƣ����Կ϶������ҵ�����*/
        if( NULL == mb_pool[i].mb_buffer )
            continue;
        /*  ϵͳ�д�������, ����ʧ�ܡ��ͷ��Ѿ�����Ķ���     */
        if( _namecmp(mb_pool[i].mb_name,name) == 0 )
        {
            PROC_SET_ERR(ERR_MSG_EXIST);
            mb->mb_buffer = NULL;
            CRITICAL_END();
            goto mb_create_end;
        }
    }
    CRITICAL_END();
    /*  ��������˵�����ڿ��ö�����û�����������Դ�������    */
    _nstrcpy(mb->mb_name,name,OBJ_NAME_LEN);
    PROC_LIST_ZERO(&mb->mb_send_wait);
    PROC_LIST_ZERO(&mb->mb_resv_wait);
#ifdef _CFG_SMP_
    msg->mb_lock       = 0;
#endif  /*  _CFG_SMP_*/
    mb->mb_owner        = proc_current;
    mb->mb_count        = count;
    mb->mb_send         = 0;
    mb->mb_resv         = count - 1;
    mb->mb_ref          = 1;
    handle = Koum_add(mb,Msg_release,kot_message,HANDLE_ATTR_RDWR);
    if( INVALID_HANDLE == handle )
    {
        CRITICAL_BEGIN();
        mb->mb_buffer = NULL;
        CRITICAL_END();
        goto mb_create_end;
    }
    PROC_NO_ERR();

mb_create_end:
    return handle;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Msg_get
//  ��  ��: ��ȡ��Ϣ����
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//    const char *  |   name        |   ��Ϣ��������
//  ����ֵ: ʧ�ܷ���INVALID_HANDLE���ɹ�����������
//  ע  ��:   
//  �����¼:
//  ʱ  ��      |   ��  ��      | ˵  ��
//=============================================================================
//  2014-02-24  |   ��  ��      | ����KOUMҪ���޸�
//  2012-01-01  |   ��  ��      | ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
handle_t    Msg_get(const char * name)
{
    handle_t        handle  = INVALID_HANDLE;
    message_box_t * mb      = mb_pool;
    int             i       = 0;
    CRITICAL_DECLARE(mb_lock);
        
    CRITICAL_BEGIN();
    for( ; i < MB_MAX ; i++,mb++)
    {
        if( !MB_IS_VALID(mb))
            continue;
        if( _namecmp(name,mb->mb_name) == 0 )
            break;
    }
    CRITICAL_END();
    if( i < MB_MAX )
        handle = Koum_add(mb,Msg_release,4,HANDLE_ATTR_RDWR);
    if( INVALID_HANDLE != handle )
        ++mb->mb_ref;
    return handle;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Msg_send
//  ��  ��: ������Ϣ��
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   ��Ϣ������
//      uint32_t    |   type        |   ��Ϣ����
//      dword_t     |   param32     |   ��Ϣ��32λ����
//      qword_t     |   param64     |   ��Ϣ��64λ����
//  ����ֵ: ʧ�ܷ���RESULT_SUCCEED���ɹ�����������
//  ע  ��: 1.��������������ȴ���
//          2.��������������Ϣ
//  �����¼:
//  ʱ  ��      |   ��  ��      | ˵  ��
//=============================================================================
//  2014-02-24  |   ��  ��      | ����KOUMҪ���޸�
//  2012-11-20  |   ��  ��      | �����߼�
//  2012-01-01  |   ��  ��      | ��һ��ʱ���Ѿ����ǵ�
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Msg_send(handle_t   handle ,uint_t      type,
                     dword_t    param32,qword_t     param64)
{
    message_box_t * mb      = Koum_handle_object(handle);
    message_t     * msg     = NULL;
    CRITICAL_DECLARE(mb->mb_lock);

    ASSERT( INVALID_HANDLE != handle);
#ifdef _CFG_CHECK_PARAMETER_
    if( INVALID_HANDLE == handle )
        return RESULT_FAILED;
#endif  /*  _CFG_CHECK_PARAMMETER_  */
    if( NULL == mb)
    {
        PROC_SET_ERR(ERR_MSG_INVALID);
        return ERR_MSG_INVALID;
    }
    /*
     *  ���ܸ���������Ϣ
     *  ��������˻�����������������Ҳ�������Լ�������Ϣ���������˵ȴ�״̬��
     *  ��ô���ͽ��̺ͽ��ս��̶��ᴦ�ڵȴ�״̬���൱��������
     */
    if( proc_current == mb->mb_owner )
    {
        PROC_SET_ERR(ERR_MSG_ITSELF);
        return ERR_MSG_ITSELF;
    }
send_msg:
    CRITICAL_BEGIN();
    if( !MB_IS_VALID(mb))
    {
        PROC_SET_ERR(ERR_MSG_NOT_EXIST);
        CRITICAL_END();
        return ERR_MSG_NOT_EXIST;
    }
    if( MB_IS_FULL(mb) )
    {
        /*
         *  ������������Ҫ�ȴ�
         */
        Proc_wait_on(&mb->mb_send_wait);
        CRITICAL_END();
        Proc_sched(0);
        goto send_msg;
    }
    msg = mb->mb_buffer + mb->mb_send;
    msg->msg_pid        = proc_current->proc_pid;
    msg->msg_type       = type;
    msg->msg_param32    = param32;
    msg->msg_param64    = param64;
    MB_SEND_FORWARD(mb);
    Proc_resume_on(&mb->mb_resv_wait);
    CRITICAL_END();
    SCHED(0);
    PROC_NO_ERR();

    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Msg_post
//  ��  ��: Ͷ����Ϣ��
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   ��Ϣ������
//      uint32_t    |   type        |   ��Ϣ����
//      dword_t     |   param32     |   ��Ϣ��32λ����
//      qword_t     |   param64     |   ��Ϣ��64λ����
//  ����ֵ: ʧ�ܷ���RESULT_SUCCEED���ɹ�����������
//  ע  ��: 1.��������ֱ�ӷ��أ����ȴ���
//          2.��������������Ϣ
//  �����¼:
//  ʱ  ��      |   ��  ��      | ˵  ��
//=============================================================================
//  2014-02-24  |   ��  ��      | ����KOUMҪ���޸�
//  2012-11-20  |   ��  ��      | �����߼�
//  2012-01-01  |   ��  ��      | ��һ��ʱ���Ѿ����ǵ�
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Msg_post(handle_t   handle ,uint_t      type,
                     dword_t    param32,qword_t     param64)
{
    message_box_t * mb      = Koum_handle_object(handle);
    message_t     * msg     = NULL;
    CRITICAL_DECLARE(mb->mb_lock);

    if( NULL == mb)
    {
        PROC_SET_ERR(ERR_MSG_INVALID);
        return ERR_MSG_INVALID;
    }
    CRITICAL_BEGIN();
    if( MB_IS_FULL(mb) )          /*  ����������������Ϣ����  */
    {
        CRITICAL_END();
        PROC_SET_ERR(ERR_MSG_BUFFER_FULL);
        return ERR_MSG_BUFFER_FULL;        
    }
    msg = mb->mb_buffer + mb->mb_send;
    msg->msg_pid        = proc_current->proc_pid;
    msg->msg_type       = type;
    msg->msg_param32    = param32;
    msg->msg_param64    = param64;
    MB_SEND_FORWARD(mb);
    Proc_resume_on(&mb->mb_resv_wait);
    CRITICAL_END();
    SCHED(0);
    PROC_NO_ERR();

    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Msg_resv
//  ��  ��: ������Ϣ��
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     handle_t     |   handle      |   ��Ϣ�ж�����
//     message_t *  |   msg         |   ��Ϣָ��
//  ����ֵ: ʧ�ܷ���RESULT_SUCCEED���ɹ�����������
//  ע  ��: 1.ֻ����Ϣ����Ĵ����ߣ�Ҳ����ӵ���ߣ����ܽ�����Ϣ��
//          2.����Ϣ���󻺳���Ϊ��ʱ������˯�ߵȴ�����
//  �����¼:
//  ʱ  ��      |   ��  ��      | ˵  ��
//=============================================================================
//  2014-02-24  |   ��  ��      | ����KOUMҪ���޸�
//  2012-11-20  |   ��  ��      | �����߼�
//  2012-01-01  |   ��  ��      | ��һ��ʱ���Ѿ����ǵ�
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Msg_resv(handle_t handle ,message_t * msg)
{
    message_box_t * mb      = Koum_handle_object(handle);
    CRITICAL_DECLARE(mb->mb_lock);    /*  ��mbΪNULLʱ�����ܻ���ɴ��� */

    if( NULL == mb || NULL == msg )
    {
        PROC_SET_ERR(ERR_MSG_INVALID);
        return ERR_MSG_INVALID;
    }
    /*
     *  ӵ���߲��ܽ�����Ϣ
     */
    if( proc_current != mb->mb_owner )
    {
        PROC_SET_ERR(ERR_MSG_NOT_OWNER);
        return ERR_MSG_NOT_OWNER;
    }
resv_msg:
    CRITICAL_BEGIN();
    if( MB_IS_EMPTY(mb) )
    {
        /*
         *  �������գ���Ҫ�ȴ�����
         */
        Proc_wait_on(&mb->mb_resv_wait);
        CRITICAL_END();
        Proc_sched(0);
        goto resv_msg;
    }
    MB_RESV_FORWARD(mb);
    *msg = *(mb->mb_buffer + mb->mb_resv);
    Proc_resume_on(&mb->mb_send_wait);
    CRITICAL_END();
    SCHED(0);
    PROC_NO_ERR();

    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Msg_take
//  ��  ��: ȡ����Ϣ��
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   ��Ϣ�ж�����
//      message_t   |   msg         |   ��Ϣָ��
//  ����ֵ: ʧ�ܷ���RESULT_SUCCEED���ɹ�����������
//  ע  ��: 1.ֻ����Ϣ����Ĵ����ߣ�Ҳ����ӵ���ߣ����ܽ�����Ϣ��
//          2.����Ϣ���󻺳���Ϊ��ʱ�����ȴ�
//  �����¼:
//  ʱ  ��      |   ��  ��      | ˵  ��
//=============================================================================
//  2014-02-24  |   ��  ��      | ����KOUMҪ���޸�
//  2012-11-20  |   ��  ��      | �����߼�
//  2012-01-01  |   ��  ��      | ��һ��ʱ���Ѿ����ǵ�
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Msg_take(handle_t handle,message_t * msg)
{
    message_box_t * mb      = Koum_handle_object(handle);
    int             retry   = MSG_TAKE_RETRY_TIMES;
    CRITICAL_DECLARE(mb->mb_lock);    /*  ��mbΪNULLʱ�����ܻ���ɴ��� */

    if( NULL == mb || NULL == msg )
    {
        PROC_SET_ERR(ERR_MSG_INVALID);
        return ERR_MSG_INVALID;
    }
    /*
     *  ӵ���߲��ܽ�����Ϣ
     */
    if( proc_current != mb->mb_owner )
    {
        PROC_SET_ERR(ERR_MSG_NOT_OWNER);
        return ERR_MSG_NOT_OWNER;
    }
resv_msg:
    CRITICAL_BEGIN();
    /*
     *  �ڻ����������ݵ�����£��ȴ�һ���ĺ������������
     *  ����ʧ��
     */
    if( MB_IS_EMPTY(mb))
    {
        CRITICAL_END();
        if( --retry < 0 )
        {
            PROC_SET_ERR(ERR_MSG_BUFFER_EMPTY);
            return ERR_MSG_BUFFER_EMPTY;
        }
        Proc_delay(MSG_TAKE_TIMEOUT);
        goto resv_msg;
    }
    MB_RESV_FORWARD(mb);
    *msg = *(mb->mb_buffer + mb->mb_resv);
    Proc_resume_on(&mb->mb_send_wait);
    CRITICAL_END();
    SCHED(0);
    PROC_NO_ERR();

    return RESULT_SUCCEED;
}


#endif  /*  _CFG_MESSAGE_ENABLE_    */
/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  �¼�
*/

#ifdef _CFG_EVENT_ENABLE_

static event_t              evn_pool[EVENT_MAX];        /*  �¼������ */
static event_t          *   evn_free_list;              /*  �����¼������б�    */
#ifdef _CFG_SMP_
static spin_lock_t          evn_lock;
#endif  /*  _CFG_SMP_   */

void        Event_initial(void)
{
    static char initialed = 0;
    register event_t    *   evn;

    if( initialed ) return ;
    initialed = 1;

#ifdef _CFG_SMP_
    evn_lock      = 0;
#endif  /*  _CFG_SMP_   */

    evn_free_list = NULL;
    for( evn = evn_pool ; evn < &evn_pool[EVENT_MAX] ; evn++)
        EVENT_FREE(evn);
}

event_t *   Event_create(uint32_t map)
{
    register event_t    *   evn;
    CRITICAL_DECLARE(evn_lock);

    CRITICAL_BEGIN();

    if( NULL == evn_free_list )
    {
        CRITICAL_END();
        return NULL;
    }
    evn = evn_free_list;

    evn_free_list = evn_free_list->evn_res;

    CRITICAL_END();

    evn->evn_res = NULL;
    evn->evn_map = map;

    return evn;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  ��  ��: Event_destory
//
//  ��  ��: �����¼�����
//
//  ��  ��: 
//      evn                 : event_t *
//                          : �¼�����ָ��
//
//  ����ֵ: 
//      ����: result_t
//      ˵��: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���RESULT_FAILED
//
//  ע  ��: �����¼�����ǰ������ȷ�����¼��ϣ�û�еȴ��Ľ���.
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//========================================================================================
//  2012-01-09  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
result_t    Event_destory(event_t * evn)
{
    CRITICAL_DECLARE(evn->evn_lock);

    if( NULL == evn )
        return RESULT_FAILED;

    CRITICAL_BEGIN();

    /*  ���еȴ��ö���Ľ��̣���������        */
    if( evn->evn_wait )                                                     
    {
        CRITICAL_END();
        return RESULT_FAILED;   
    }
    
    EVENT_FREE(evn);

    CRITICAL_END();

    return RESULT_SUCCEED;
}

void        Event_reset(event_t * evn)
{
    CRITICAL_DECLARE(evn->evn_lock);

    CRITICAL_BEGIN();
    
    evn->evn_arrive = 0;

    CRITICAL_END();
}
/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  ��  ��: Event_set
//
//  ��  ��: �����ѷ������¼�������ѷ������¼��봴��ʱ���¼�
//
//  ��  ��: 
//      evn                 : event_t *
//                          : �¼�����
//
//      map                 : uint32_t
//                          : �¼�λͼ
//
//  ����ֵ: ��
//
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//========================================================================================
//  2012-01-09  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
void        Event_set(event_t * evn,uint32_t map)
{
    proc_t  * proc,*next;
    CRITICAL_DECLARE(evn->evn_lock);

    CRITICAL_BEGIN();
    /*  ���¼�������Ϣ                        */
    evn->evn_arrive |= map;                                                 

    /*  �¼�û�з���*/
    if( (evn->evn_arrive & evn->evn_map) != evn->evn_map )                  
    {
        CRITICAL_END();
        return ;
    }

    /*
     *  ��������˵���¼�����������������Ѿ��ﵽ���������еȴ��¼��Ľ���
     */
    proc = evn->evn_wait;

    do
    {
        next = proc->proc_sched_next;
        Sched_add(proc);
        proc = next;
    }while( proc );

    CRITICAL_END();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  ��  ��: Event_wait
//
//  ��  ��: �ȴ��¼���
//
//  ��  ��: 
//      evn                 : event_t *
//                          : �¼�����
//
//      millisecond         : uint32_t
//                          : ��ʱʱ�䣬�Ժ���Ϊ��λ��0��ʾ�����Ƶȴ���
//
//  ����ֵ: 
//      ����: BOOL
//      ˵��: �¼����ﷵ��TRUE��
//            �ȴ���ʱ����FALSE
//
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//========================================================================================
//  2012-01-09  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
BOOL        Event_wait(event_t * evn,uint32_t millisecond)
{
    CRITICAL_DECLARE(evn->evn_lock);


    if( millisecond )   /*  �����˳�ʱʱ�䣬*/
    {
        uint_t wait;

        while(millisecond)
        {
            CRITICAL_BEGIN();
            /*  �¼��Ѿ�����                          */
            if( (evn->evn_arrive & evn->evn_map) == evn->evn_map )          
            {
                CRITICAL_END();
                return TRUE;
            }

            wait = 10;
            if( millisecond < wait ) wait = millisecond;
            /*  ��ʱ�ȴ�ǰ���˳��ٽ���                */
            CRITICAL_END();                                                 

            Proc_delay(wait);
            /*  ���ٵȴ�ʱ��                          */
            millisecond -= wait;                                            
        }
        return FALSE;
    }

    CRITICAL_BEGIN();

    Sched_del(proc_current);
    proc_current->proc_stat         = PROC_STAT_WAIT;
    proc_current->proc_sched_next   = evn->evn_wait;
    evn->evn_wait                   = proc_current;

    CRITICAL_END();

    Proc_sched(0);

    return TRUE;
}

#endif  /*  _CFG_EVENT_ENABLE_  */