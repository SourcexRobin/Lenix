/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: ipc.c
//  创建时间: 2011-07-12        创建者: 罗斌
//  修改时间: 2014-02-16        修改者: 罗斌
//  主要功能: 提供semaphore，mutex，message。
//  说    明: 
//
//  版本变化记录:
//  版本号      |     时间      |  作  者       |  主要变化记录
//=============================================================================
//              |   2014-02-16  |  罗  斌       | 引入KOUM后，大幅修改
//              |   2012-11-13  |  罗  斌       | 重构，将spin_lock，lock的实现
//                                              | 调整到该文件。把对象某些字段
//                                              | 改名，处理优先级反转的问题。
//  00.00.000   |   2011-07-02  |  罗  斌       | 第一版的具体时间已经不记得了
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
//  名  称: Lck_lock
//  作  用: 锁。用于睡眠（等待）互斥访问临界资源。
//  参  数: 无
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      lock_t *    |   lck         | 锁对象指针
//  返回值: 无。
//  注  意: 由于直接使用指针，仅在调试时提供参数检测。
//            必须与Lck_free成对使用。在互斥资源需要等待较长时间的情况下，应使
//          用该函数对，而不是使用Spin_lock函数对。
//            在占用锁以后，不应该出现主动放弃CPU的情况。
//  变更记录:
//  时  间      |   作  者      |  说  明
//=============================================================================
//  2012-11-17  |   罗  斌      |  处理优先级反转情况
//  2012-01-01  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Lck_lock(lock_t * lck)
{
    CRITICAL_DECLARE(lck->lck_lock);

    ASSERT(NULL != lck);
    while( Cpu_tas((int *)&lck->lck_status,0,1) )
    {        
        /*
         *  可能存在多个进程同时不能获得锁
         */
        CRITICAL_BEGIN();
        /*
         *    如果尝试获得锁的进程优先级高于锁进程的优先级，则可能会出现优先级
         *  反转。Lenix采用提高优先级的方式来避免这种情况
         */
        if( proc_current->proc_priority < lck->lck_priority )
        {
            /*
             *  提高锁的优先级，提高到需要占用对象的进程中最高的优先级
             */
            lck->lck_priority = proc_current->proc_priority;
            /*
             *  将锁进程的优先级提高，并调整RSPL。
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
//  名  称: Lck_free
//  作  用: 释放锁。
//  参  数: 无
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      lock_t *    |   lck         | 锁对象指针
//  返回值: 无。
//  注  意: 由于直接使用指针，仅在调试时提供参数检测。
//  变更记录:
//  时  间      |   作  者      |  说  明
//=============================================================================
//  2012-11-17  |   罗  斌      |  处理优先级反转情况
//  2012-01-01  |   罗  斌      |  第一版
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
//  互斥
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
//  名  称: Mutex_release
//  作  用: 释放互斥对象。
//  参  数: 无
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     mutex_t *    |   mutex       | 互斥对象指针
//  返回值: 无。
//  注  意: 仅由Koum_release调用。
//  变更记录:
//  时  间      |   作  者      |  说  明
//=============================================================================
//  2012-11-21  |   罗  斌      |  重写
//  2012-01-01  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
result_t    Mutex_release(mutex_t * mutex)
{
    result_t                result      = RESULT_SUCCEED;
    CRITICAL_DECLARE(mutex_lock);

    ASSERT(mutex);
    /*
     *  不是系统分配的对象，不能删除
     */
    if( mutex < MUTEX_FIRST || mutex > MUTEX_LAST)
    {
        PROC_SET_ERR(ERR_MUTEX_OUT_OF_POOL);
        result = ERR_MUTEX_OUT_OF_POOL;
        goto mutex_destroy_end;
    }
    CRITICAL_BEGIN();
    /*
     *  有进程在等待的，不能销毁
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
//  名  称: Mutex_create
//  作  用: 创建互斥对象。
//  参  数: 无
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
//  2012-11-21  |   罗  斌      |  重写
//  2012-01-01  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
handle_t    Mutex_create(void)
{
    int             i       = 0;
    mutex_t       * mtx     = mutex_pool;
    handle_t        handle  = INVALID_HANDLE;
    CRITICAL_DECLARE(mutex_lock);

    /*
     *  遍历系统互斥对象池，查找可用的互斥对象
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
     *  检测是否分配到互斥对象，如果分配到，则要加入内核对象表
     */
    if( i < MUTEX_MAX )
    {
        handle = Koum_add(mtx,Mutex_release,kot_mutex,HANDLE_ATTR_RDWR);
        /*
         *  加入系统内核对象表失败，需要释放已分配到的互斥对象
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
//  名  称: Mutex_get
//  作  用: 获得互斥对象。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   互斥对象句柄
//  返回值: 成功返回RESULT_SUCCEED，失败返回RESULT_FAILED。
//  注  意:   如果遇到更高优先级的进程等待互斥对象，Lenix将提高正在占用互斥对
//          象进程的优先级。
//            调用该API后，需要检测返回值。
//  变更记录:
//  时间        |   作  者      | 说明
//=============================================================================
//  2014-02-16  |   罗  斌      | 根据KOUM要求修改
//  2012-07-03  |   罗  斌      | 增加防止优先级反转的功能
//  2012-01-09  |   罗  斌      | 第一版
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Mutex_get(handle_t handle)
{
    mutex_t       * mutex   = Koum_handle_object(handle);
    CRITICAL_DECLARE(mutex->mtx_lock);

    ASSERT(handle);

    /*
     *  检测对象是否有效
     */
    if( INVALID_HANDLE == handle || NULL == mutex )
        return RESULT_FAILED;
    while( Cpu_tas((int *)&mutex->mtx_status,0,1) )
    {
        CRITICAL_BEGIN();
        /*
         *  处理优先级反转问题，有更高优先级的进程请求互斥量
         */
        if( proc_current->proc_priority < mutex->mtx_priority )
        {
            /*
             *    将占用互斥量的进程提高到该进程，也就是将该进程改至更高优先级
             *  的列表中注意，这里并不要求立即调度
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
//  名  称: Mutex_put
//  作  用: 释放互斥对象。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   互斥对象句柄
//  返回值: 成功返回RESULT_SUCCEED，失败返回RESULT_FAILED。
//  注  意: 调用该API后，需要检测返回值。
//  变更记录:
//  时间        |   作  者      | 说明
//=============================================================================
//  2014-02-16  |   罗  斌      | 根据KOUM要求修改
//  2012-07-03  |   罗  斌      | 增加防止优先级反转的功能
//  2012-01-09  |   罗  斌      | 第一版
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Mutex_put(handle_t handle)
{
    mutex_t       * mutex   = Koum_handle_object(handle);
    CRITICAL_DECLARE(mutex->mtx_lock);

    ASSERT(handle);
    /*
     *  检测对象是否有效
     */
    if( INVALID_HANDLE == handle || NULL == mutex )
        return RESULT_FAILED;
    CRITICAL_BEGIN();
    /*
     *    如果互斥量占用进程的优先级与原始的优先级不同，则说明发生了优先级变
     *  更，需要变回原来的优先级
     */
    if( mutex->mtx_user_prio != proc_current->proc_priority )
    {
        /*  将优先级改回原始优先级  */
        Sched_del(mutex->mtx_user);
        proc_current->proc_priority = mutex->mtx_user_prio;
        Sched_add(mutex->mtx_user);
    }
    mutex->mtx_user_prio    = 0;
    mutex->mtx_priority     = 0;
    mutex->mtx_status       = 0;
    mutex->mtx_user         = NULL;
    /*  唤醒所有等待的的进程    */
    Proc_resume_on(&mutex->mtx_wait);
    CRITICAL_END();
    SCHED(0);
    return RESULT_SUCCEED;
}

#endif  /*  _CFG_MUTEX_ENABLE_  */

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  信号量（多值信号）
*/
#ifdef _CFG_SEMAPHORE_ENABLE_

static semaphore_t          sema_pool[SEMA_MAX];    /*  系统信号量对象池    */

#ifdef _CFG_SMP_
static spin_lock_t          sema_lock;              /*  访问锁              */
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
//  名  称: Sema_release
//  作  用: 释放信号量对象。
//  参  数: 无
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//   semaphore_t *  |    sema       | 信号量对象指针
//  返回值: 无。
//  注  意: 仅由Koum_release调用。
//  变更记录:
//  时  间      |   作  者      |  说  明
//=============================================================================
//  2012-11-21  |   罗  斌      |  重写
//  2012-01-01  |   罗  斌      |  第一版
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
     *  不处理池以外的对象
     */
    if( sema < SEMA_FIRST || sema > SEMA_LAST )
    {
        PROC_SET_LAST_ERR(ERR_SEMA_NOT_SYS_CREATE);
        return ERR_SEMA_NOT_SYS_CREATE;
    }
    CRITICAL_BEGIN();
    /*
     *  还有进程在等待信号量，不能销毁
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
//  名  称: Sema_create
//  作  用: 创建信号量对象。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      long        |     max       |   信号量最大值
//  返回值: 失败返回INVALID_HANDLE，成功返回其他值。
//  注  意: 
//  变更记录:
//  时  间      |   作  者      | 说明
//=============================================================================
//  2014-02-17  |   罗  斌      | 根据KOUM要求修改
//  2012-01-09  |   罗  斌      | 第一版
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
     *  对信号数量进行限制
     */
    if( max > SEMA_LIMIT)
    {
        PROC_SET_ERR(ERR_SEMA_MAX_OVERFLOW);
        return INVALID_HANDLE;
    }

    CRITICAL_BEGIN();
    /*
     *  遍历信号对象池，查找可用对象，以信号最大值为0作为可用的依据
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
//  名  称: Sema_down
//  作  用: 减少信号值，当信号值用尽后，进程进入等待状态。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   信号量对象句柄
//  返回值: 成功返回RESULT_SUCCEED，失败返回RESULT_FAILED。
//  注  意:   调用该API后，需要检测返回值。
//             1、信号计数器为非负时，说明没有进程等待。
//  变更记录:
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-17  |   罗  斌      | 根据KOUM要求修改
//  2012-07-03  |   罗  斌      | 增加防止优先级反转的功能
//  2012-01-09  |   罗  斌      | 第一版
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
        /*  信号计数器小于0说明资源已经耗尽，进程需要等待资源可用。*/
        Sched_del(proc_current);
        Proc_wait_on(&sema->sema_wait);
        CRITICAL_END();
        Proc_sched(0);
        /*  到达这里说明进程已经恢复运行，也说明资源被释放了一个，可以使用。*/
        return RESULT_SUCCEED;
    }
    CRITICAL_END();   

    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Sema_down
//  作  用: 减少信号值，当信号值用尽后，进程进入等待状态。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   信号量对象句柄
//  返回值: 成功返回RESULT_SUCCEED，失败返回RESULT_FAILED。
//  注  意:   调用该API后，需要检测返回值。
//             1、信号计数器为非负时，说明没有进程等待。
//            当有进程等待信号时，唤醒等待信号进程中优先级最高的进程，如果优先
//          级相同，唤醒调度因子最高的进程。
//  变更记录:
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-17  |   罗  斌      | 根据KOUM要求修改，删除优先级反转处理
//  2012-07-03  |   罗  斌      | 增加防止优先级反转的功能
//  2012-01-09  |   罗  斌      | 第一版
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
    /*  超过最大值，肯定有错，死机  */
    if( ++sema->sema_count > sema->sema_max )
        Sys_halt("semaphore large than max!");
    /*  小于1，说明有等待资源的进程，需要唤醒   */
    if( sema->sema_count < 1 )
        Proc_resume_max_on(&sema->sema_wait);
    CRITICAL_END();
    SCHED(0);
}

#endif  /*  _CFG_SEMAPHORE_ENABLE_  */

/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  IPC。消息机制
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

/*  销毁邮箱 **/
static
result_t    Msg_release(message_box_t * mb)
{
    /*
     *  可能存在的情况。1、对象还有引用
     *  设计，自己任何时候都可以销毁对象，不管是否有引用。
     *  引用者不能销毁对象
     *  在全部引用者释放后，才能完全释放
     */
    CRITICAL_DECLARE(mb_lock);
    --mb->mb_ref;
    /*  只有创建者能销毁邮箱    */
    if( proc_current != mb->mb_owner )
        return RESULT_SUCCEED;
    CRITICAL_BEGIN();
    do
    {
        /*
         *  有可能在销毁对象时，还有进程在等待邮箱，需要唤醒这些进程，使其可以
         *  进行后续的处理。
         *  嵌套使用CSPF。
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
//  名  称: Msg_create
//  作  用: 创建消息对象。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//    const char *  |   name        |   消息对象名称
//    message_t *   |   msgbuf      |   消息缓冲区
//    int           |   count       |   消息槽数量
//  返回值: 失败返回INVALID_HANDLE，成功返回其他。
//  注  意:  在创建者释放消息对象后，引用者在释放对象时，会释放失败
//  变更记录:
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-24  |   罗  斌      | 根据KOUM要求修改
//  2012-01-01  |   罗  斌      | 第一版
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
    /*  先找到可用对象，消息对象的缓冲区没有分配缓存，则认为该对象可用 */
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
    /*  占用对象，并将名称设置为空    */
    mb->mb_buffer     = msgbuf;
    mb->mb_name[0]    = 0;
    /*  检测是否重名    */
    for( i = 0 ; i < MB_MAX ; i++ )
    {
        /*  跳过没有使用的对象，此时自身还没有名称，所以肯定不会找到自身。*/
        if( NULL == mb_pool[i].mb_buffer )
            continue;
        /*  系统中存在重名, 创建失败。释放已经分配的对象     */
        if( _namecmp(mb_pool[i].mb_name,name) == 0 )
        {
            PROC_SET_ERR(ERR_MSG_EXIST);
            mb->mb_buffer = NULL;
            CRITICAL_END();
            goto mb_create_end;
        }
    }
    CRITICAL_END();
    /*  到达这里说明存在可用对象，且没有重名，可以创建对象。    */
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
//  名  称: Msg_get
//  作  用: 获取消息对象。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//    const char *  |   name        |   消息对象名称
//  返回值: 失败返回INVALID_HANDLE，成功返回其他。
//  注  意:   
//  变更记录:
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-24  |   罗  斌      | 根据KOUM要求修改
//  2012-01-01  |   罗  斌      | 第一版
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
//  名  称: Msg_send
//  作  用: 发送消息。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   消息对象句柄
//      uint32_t    |   type        |   消息类型
//      dword_t     |   param32     |   消息的32位数据
//      qword_t     |   param64     |   消息的64位数据
//  返回值: 失败返回RESULT_SUCCEED，成功返回其他。
//  注  意: 1.如果缓冲区满，等待。
//          2.不能向自身发送消息
//  变更记录:
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-24  |   罗  斌      | 根据KOUM要求修改
//  2012-11-20  |   罗  斌      | 调整逻辑
//  2012-01-01  |   罗  斌      | 第一版时间已经不记得
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
     *  不能给自身发送消息
     *  如果出现了缓冲区已满，而自身也尝试向自己发送消息，而进入了等待状态，
     *  那么发送进程和接收进程都会处于等待状态，相当于死锁。
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
         *  缓冲区满，需要等待
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
//  名  称: Msg_post
//  作  用: 投递消息。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   消息对象句柄
//      uint32_t    |   type        |   消息类型
//      dword_t     |   param32     |   消息的32位数据
//      qword_t     |   param64     |   消息的64位数据
//  返回值: 失败返回RESULT_SUCCEED，成功返回其他。
//  注  意: 1.缓冲区满直接返回，不等待。
//          2.不能向自身发送消息
//  变更记录:
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-24  |   罗  斌      | 根据KOUM要求修改
//  2012-11-20  |   罗  斌      | 调整逻辑
//  2012-01-01  |   罗  斌      | 第一版时间已经不记得
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
    if( MB_IS_FULL(mb) )          /*  缓冲区满，丢弃消息返回  */
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
//  名  称: Msg_resv
//  作  用: 接收消息。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     handle_t     |   handle      |   消息盒对象句柄
//     message_t *  |   msg         |   消息指针
//  返回值: 失败返回RESULT_SUCCEED，成功返回其他。
//  注  意: 1.只能消息对象的创建者，也就是拥有者，才能接收消息。
//          2.在消息对象缓冲区为空时，进程睡眠等待数据
//  变更记录:
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-24  |   罗  斌      | 根据KOUM要求修改
//  2012-11-20  |   罗  斌      | 调整逻辑
//  2012-01-01  |   罗  斌      | 第一版时间已经不记得
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Msg_resv(handle_t handle ,message_t * msg)
{
    message_box_t * mb      = Koum_handle_object(handle);
    CRITICAL_DECLARE(mb->mb_lock);    /*  在mb为NULL时，可能会造成错误 */

    if( NULL == mb || NULL == msg )
    {
        PROC_SET_ERR(ERR_MSG_INVALID);
        return ERR_MSG_INVALID;
    }
    /*
     *  拥有者才能接受消息
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
         *  缓冲区空，需要等待数据
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
//  名  称: Msg_take
//  作  用: 取回消息。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      handle_t    |   handle      |   消息盒对象句柄
//      message_t   |   msg         |   消息指针
//  返回值: 失败返回RESULT_SUCCEED，成功返回其他。
//  注  意: 1.只能消息对象的创建者，也就是拥有者，才能接收消息。
//          2.在消息对象缓冲区为空时，不等待
//  变更记录:
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-24  |   罗  斌      | 根据KOUM要求修改
//  2012-11-20  |   罗  斌      | 调整逻辑
//  2012-01-01  |   罗  斌      | 第一版时间已经不记得
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Msg_take(handle_t handle,message_t * msg)
{
    message_box_t * mb      = Koum_handle_object(handle);
    int             retry   = MSG_TAKE_RETRY_TIMES;
    CRITICAL_DECLARE(mb->mb_lock);    /*  在mb为NULL时，可能会造成错误 */

    if( NULL == mb || NULL == msg )
    {
        PROC_SET_ERR(ERR_MSG_INVALID);
        return ERR_MSG_INVALID;
    }
    /*
     *  拥有者才能接受消息
     */
    if( proc_current != mb->mb_owner )
    {
        PROC_SET_ERR(ERR_MSG_NOT_OWNER);
        return ERR_MSG_NOT_OWNER;
    }
resv_msg:
    CRITICAL_BEGIN();
    /*
     *  在缓冲区无数据的情况下，等待一定的毫秒后，仍无数据
     *  返回失败
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
//  事件
*/

#ifdef _CFG_EVENT_ENABLE_

static event_t              evn_pool[EVENT_MAX];        /*  事件对象池 */
static event_t          *   evn_free_list;              /*  可用事件对象列表    */
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
//  名  称: Event_destory
//
//  作  用: 销毁事件对象
//
//  参  数: 
//      evn                 : event_t *
//                          : 事件对象指针
//
//  返回值: 
//      类型: result_t
//      说明: 成功返回RESULT_SUCCEED，失败返回RESULT_FAILED
//
//  注  意: 销毁事件对象前，必须确保在事件上，没有等待的进程.
//
//  变更记录:
//  时间        |  作者         |  说明
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

    /*  还有等待该对象的进程，不能销毁        */
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
//  名  称: Event_set
//
//  作  用: 设置已发生的事件。如果已发生的事件与创建时的事件
//
//  参  数: 
//      evn                 : event_t *
//                          : 事件对象
//
//      map                 : uint32_t
//                          : 事件位图
//
//  返回值: 无
//
//  注  意: 
//
//  变更记录:
//  时间        |  作者         |  说明
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
    /*  置事件到达信息                        */
    evn->evn_arrive |= map;                                                 

    /*  事件没有发生*/
    if( (evn->evn_arrive & evn->evn_map) != evn->evn_map )                  
    {
        CRITICAL_END();
        return ;
    }

    /*
     *  到达这里说明事件对象所代表的条件已经达到，唤醒所有等待事件的进程
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
//  名  称: Event_wait
//
//  作  用: 等待事件。
//
//  参  数: 
//      evn                 : event_t *
//                          : 事件对象
//
//      millisecond         : uint32_t
//                          : 超时时间，以毫秒为单位。0表示无限制等待。
//
//  返回值: 
//      类型: BOOL
//      说明: 事件到达返回TRUE。
//            等待超时返回FALSE
//
//  注  意: 
//
//  变更记录:
//  时间        |  作者         |  说明
//========================================================================================
//  2012-01-09  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
BOOL        Event_wait(event_t * evn,uint32_t millisecond)
{
    CRITICAL_DECLARE(evn->evn_lock);


    if( millisecond )   /*  设置了超时时间，*/
    {
        uint_t wait;

        while(millisecond)
        {
            CRITICAL_BEGIN();
            /*  事件已经到达                          */
            if( (evn->evn_arrive & evn->evn_map) == evn->evn_map )          
            {
                CRITICAL_END();
                return TRUE;
            }

            wait = 10;
            if( millisecond < wait ) wait = millisecond;
            /*  延时等待前，退出临界区                */
            CRITICAL_END();                                                 

            Proc_delay(wait);
            /*  减少等待时间                          */
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