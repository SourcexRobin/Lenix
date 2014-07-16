/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: blkbuf.c  块缓存 block buffer
//  创建时间: 2014-02-28        创建者: 罗斌
//  修改时间:                   修改者: 
//  主要功能: 块缓存管理
//  说    明: 本模块引入了可用缓存块列表、已用缓存块列表、查找散列表。
//
//  变更记录:
//  版 本 号    |     时间      |  作  者       |  主要变化记录
//=============================================================================
//  00.00.000   |  2014-02-22   |  罗  斌       |  建立文件
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>
#include <result.h>
#include <assert.h>

#include <proc.h>
#include <device.h>

#include <blkbuf.h>

#define BBUF_STACK                  1024

/*  从设备地址转换为桶号    */
#define OFFSET_TO_BUCKET(offset)    ((((uint_t)(offset)) >> 3) % BBUF_BUCKETS)

/*  设备地址对应的桶        */
#define BBUF_BUCKET(offset)         bb_buckets[OFFSET_TO_BUCKET(offset)]

/*  系统中可用的块缓存列表，在需要块缓存时，从这个列表中分配*/
static blkbuf_t        *    bb_free;

/*  系统中已分配的块缓存列表，同时提供表头和表尾    */
static blkbuf_t        *    bb_used;
static blkbuf_t        *    bb_used_tail;

/*  系统块缓存数量  */
static uint32_t             bb_total;

/*  块缓存可用数    */
static uint32_t             bb_total_free;

/*
 *  Lenix对已经读出的数据块采用散列表的方式进行管理，目的是提高性能。
 *  散列表采用按地址左移3位后取模的方式来计算所在桶。
 *  桶中允许有重复的地址，因为对应的设备不同。
 */
static blkbuf_t         *   bb_buckets[BBUF_BUCKETS];

/*
 *  块缓存淘汰进程栈
 */
static byte_t               bb_stack[BBUF_STACK];

/*
 *  淘汰进程等待变量
 */
static proc_t           *   bb_eliminate_proc;

/*
 *  块缓存分配等待列表
 */
static proc_list_t          bb_wait_list;
/*
 *    从灵活性的角度来说，应该使用动态内存管理的方式来提供空间。但是，也应提供
 *  一种使用固定空间的方式。
 */
static byte_t               bb_buffer[BLKBUF_SIZE];

#ifdef _CFG_SMP_
static spin_lock_t          bb_lock;
#endif  /*  _CFG_SMP_   */

/*  2014-03-08 */
#define BBUF_WRITE(bb)              do{ Lck_lock(&bb->bb_lock); \
                                        Dev_write((bb)->bb_device,    \
                                                  (bb)->bb_address,   \
                                                  (bb)->bb_buffer,    \
                                                  BBUF_SIZE);         \
                                        (bb)->bb_flags &= ~BBUF_FLAGS_DIRTED;\
                                        Lck_free(&bb->bb_lock); }while(0)


/*
///////////////////////////////////////////////////////////////////////////////
/////////////////////
//  块缓存的分配、回收，在已用列表和散列表中插入和删除的基本操作。
*/

static
void        Bbuf_check(void)
{
    int             p       = 0,
                    n       = 0;
    blkbuf_t      * blkbuf  = NULL;

    for( blkbuf = bb_used ; blkbuf ; blkbuf = blkbuf->bb_next)
        p++;
    for( blkbuf = bb_used_tail ; blkbuf ; blkbuf = blkbuf->bb_prev)
        n++;
    _printk("p=%d,n=%d\n",p,n);
    ASSERT( n == p );
}

static
void    Bbuf_msg(blkbuf_t * blkbuf)
{
    _printk("device=%P address=%ld ref=%d\n",
        blkbuf->bb_device,blkbuf->bb_address,blkbuf->bb_refcnt);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Bbuf_get
//  作    用: 分配块缓存。
//  参    数: 无
//  返 回 值: 返回块缓存对象指针。成功返回非NULL，失败返回NULL
//  说    明: 按照设计是不会返回NULL的，也就是时必定会分配成功
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-01  |   罗  斌      |  建立函数
///////////////////////////////////////////////////////////////////////////////
*/
static
blkbuf_t *  Bbuf_get(void)
{
    blkbuf_t      * blkbuf = NULL;
    CRITICAL_DECLARE(bb_lock);

    /*
     *  无可用块缓存时，等待。
     *  此时淘汰进程必定已经启动，因此可以很快存在可用块缓存 
     */
blkbuf_get_begin:
    CRITICAL_BEGIN();
    if( 0 == bb_total )
    {
        Proc_wait_on(&bb_wait_list);
        CRITICAL_END();
        Proc_sched(0);
        goto blkbuf_get_begin;
    }
    ASSERT( NULL != bb_free );
    blkbuf  = bb_free;
    bb_free = bb_free->bb_next;
    if( bb_free )
        bb_free->bb_prev = NULL;
    --bb_total_free;
    CRITICAL_END();
    blkbuf->bb_next = NULL;

    return blkbuf;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Bbuf_put
//  作    用: 回收块缓存。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     blkbuf_t *   |   blkbuf      |   块缓存对象指针
//  返 回 值: 成功返回RESULT_SUCCEED，失败返回REUSLT_FAILED。
//  说    明: 
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-01  |   罗  斌      |  建立函数
///////////////////////////////////////////////////////////////////////////////
*/
static
result_t    Bbuf_put(blkbuf_t * blkbuf)
{
    result_t        result = RESULT_SUCCEED;

    ASSERT( NULL != blkbuf);
#ifdef _CFG_CHECK_PARAMETER_
    if( NULL == blkbuf )
    {
        result = RESULT_FAILED;
        goto bbuf_put_end;
    }
#endif  /*  _CFG_CHECK_PARAMETER_*/
    blkbuf->bb_prev         = NULL;
    blkbuf->bb_next         = NULL;
    blkbuf->bb_bucket_prev  = NULL;
    blkbuf->bb_bucket_next  = NULL;
    blkbuf->bb_device       = NULL;
    blkbuf->bb_address      = 0;
    blkbuf->bb_refcnt       = 0;
    if( NULL == bb_free )
        bb_free = blkbuf;
    else
    {
        /*  插入到表头  */
        blkbuf->bb_next     = bb_free;
        bb_free->bb_prev    = blkbuf;
        bb_free             = blkbuf;
    }
    ++bb_total_free;

bbuf_put_end:
    return result;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Bbuf_hash_add
//  作    用: 将块缓存添加入已用列表和散列表当中。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     blkbuf_t *   |   blkbuf      |   块缓存对象指针
//  返 回 值: 无。
//  说    明:   在已用列表和散列表中插入时，均在表头插入，保证最近读入的数据在
//            列表的最前面，便于以后的操作。相当于按读出时间排序。
//              为提高性能，在合适的时候，为散列表引入平衡树。
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-01  |   罗  斌      |  建立函数
///////////////////////////////////////////////////////////////////////////////
*/
static 
void        Bbuf_hash_add(blkbuf_t * blkbuf)
{
    blkbuf_t      * bucket  = NULL;
    CRITICAL_DECLARE(bb_lock);

    ASSERT( NULL != blkbuf );
    blkbuf->bb_prev         = NULL;
    blkbuf->bb_next         = NULL;
    blkbuf->bb_bucket_prev  = NULL;
    blkbuf->bb_bucket_next  = NULL;
    bucket = BBUF_BUCKET(blkbuf->bb_address);
    CRITICAL_BEGIN();
    /*  插入散列表  */
    if( bucket )
    {
        blkbuf->bb_bucket_next = bucket;
        bucket->bb_bucket_prev = blkbuf;
        bucket = blkbuf;
    }
    else
        BBUF_BUCKET(blkbuf->bb_address) = blkbuf;
    /*  加入已用列表*/
    if( NULL == bb_used )
    {
        bb_used         = blkbuf;
        bb_used_tail    = blkbuf;
    }
    else
    {
        blkbuf->bb_next     = bb_used;
        bb_used->bb_prev    = blkbuf;
        bb_used             = blkbuf;
    }
    CRITICAL_END();
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Bbuf_hash_del
//  作    用: 将块缓存从已用列表和散列表当中删除。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     blkbuf_t *   |   blkbuf      |   块缓存对象指针
//  返 回 值: 成功返回RESULT_SUCCEED，失败返回REUSLT_FAILED。
//  说    明: 传递参数时，需要确保参数在散列表内，如果传递错误，会导致错误。
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-01  |   罗  斌      |  建立函数
///////////////////////////////////////////////////////////////////////////////
*/
static
result_t    Bbuf_hash_del(blkbuf_t * blkbuf)
{
    blkbuf_t      * prev    = NULL,
                  * next    = NULL,
                  * bucket  = NULL;

    prev = blkbuf->bb_prev;
    next = blkbuf->bb_next;
    /*  从已用列表中删除 */
    if( blkbuf == bb_used)  /*  处理表头 */
    {
        bb_used = next;
        if( next )
            next->bb_prev = NULL;
        else
            bb_used_tail  = NULL;   /*  链表已经没有，链表尾也要处理*/
    }
    else
    {
        prev->bb_next = next;
        if( next )
            next->bb_prev = prev;
        else
            bb_used_tail  = prev;
    }
    /*  从散列表中删除  */
    bucket  = BBUF_BUCKET(blkbuf->bb_address);
    prev    = blkbuf->bb_bucket_prev;
    next    = blkbuf->bb_bucket_next;
    if( blkbuf == bucket )  /*  处理表头 */
    {
        BBUF_BUCKET(blkbuf->bb_address) = next;
        if( next )
            next->bb_bucket_prev = NULL;
    }
    else
    {
        prev->bb_bucket_next = next;
        if( next )
            next->bb_bucket_prev = prev;
    }
    /*  */
    blkbuf->bb_next         = NULL;
    blkbuf->bb_prev         = NULL;
    blkbuf->bb_bucket_next  = NULL;
    blkbuf->bb_bucket_prev  = NULL;
    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Bbuf_query
//  作    用: 查询块缓存。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   device      |   设备对象指针
//     offset_t     |   address     |   设备地址，由Bbuf_read函数进行调整。
//
//  返 回 值: 存在返回非NULL，失败返回NULL
//  说    明: 
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-01  |   罗  斌      |  建立函数
///////////////////////////////////////////////////////////////////////////////
*/
static
blkbuf_t *  Bbuf_query(device_t * device,offset_t address)
{
    blkbuf_t      * blkbuf  = BBUF_BUCKET(address);
    CRITICAL_DECLARE(bb_lock);

    CRITICAL_BEGIN();
    /*  采用遍历的方式查找*/
    for( blkbuf ; blkbuf ; blkbuf = blkbuf->bb_bucket_next )
        if( blkbuf->bb_device == device && blkbuf->bb_address == address )
            break;
    CRITICAL_END();
    return blkbuf;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Bbuf_eliminate
//  作    用: 块缓存淘汰进程。在系统块缓存不足时，淘汰一定的数量的块缓存。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     void *       |   param       |   进程参数
//
//  返 回 值: 无
//  说    明:   每次淘汰分两步执行。首先将部分修改过的块缓存写回磁盘，然后淘汰
//            一定数量的未修改块缓存。在淘汰得到一定比例的块缓存时，等待下一次
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-01  |   罗  斌      |  建立函数
///////////////////////////////////////////////////////////////////////////////
*/
static
void        Bbuf_eliminate(void * param)
{
    int             i       = 0;
    blkbuf_t      * blkbuf  = NULL,
                  * temp    = NULL;
    CRITICAL_DECLARE(bb_lock);

    param = param;
eliminate_begin:
    CRITICAL_BEGIN();
    /*
     *  将修改过的块缓存写回设备。
     *    从已使用链表尾开始遍历，遍历完毕或者已经同步了4个块缓存，写操作结束。
     *  这么做可以分散系统的写操作时间，同时可以保证有缓存块可淘汰。
     */
    blkbuf  = bb_used_tail; /*  从已使用列表尾开始  */
    i       = 0;            /*  这里用作写计数器    */
    for( ; i < 4 && blkbuf ; blkbuf = blkbuf->bb_prev )
    {
        if( blkbuf->bb_flags & BBUF_FLAGS_DIRTED )
        {
            Lck_lock(&blkbuf->bb_lock);
            i++;
        }
    }
    /*
     *  淘汰规则：
     *    1.从引用计数最少的块缓存开始淘汰。由于是从已使用列表尾开始，因此刚读
     *  出数据的块缓存一般不会被淘汰。
     *    2.仅淘汰未修改过的块缓存
     */
    blkbuf  = bb_used_tail; /*  从已使用列表尾开始  */ 
    i       = 0;            /*  这里用作淘汰计数器  */
    for(; i < 8 && blkbuf ; blkbuf = temp)
    {
        temp = blkbuf->bb_prev;
        if( 0 == blkbuf->bb_refcnt && !(blkbuf->bb_flags & BBUF_FLAGS_DIRTED))
        {
            Bbuf_hash_del(blkbuf);
            Bbuf_put(blkbuf);
            i++;
        }
    }
    Proc_resume_on(&bb_wait_list);
    CRITICAL_END();
    Proc_sched(0);
    /*  在可用块缓存超过总数的40%后，停止淘汰*/
    if( ((bb_total_free * 10 ) / bb_total) > 4 )
        Proc_wait(&bb_eliminate_proc);

    goto eliminate_begin;
}

/*
///////////////////////////////////////////////////////////////////////////////
/////////////////////
//  块缓存API
*/


/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Bbuf_read
//  作    用: 从块设备中读数据到块缓存中。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   device      |   设备对象指针
//     offset_t     |   address     |   设备地址，传入的地址会自动向前调整为8的
//                                  |  整数倍。例如，传入7，则会调整为0
//  返 回 值: 成功返回非NULL，失败返回NULL。
//  说    明:   函数首先查找该块是否已经存在，如果已存在，则不需要在次读设备。
//            如果不存在，才会执行具体的读操作。
//              在可用块缓存不足总数的20%时，唤醒块缓存淘汰进程。
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-01  |   罗  斌      |  建立函数
///////////////////////////////////////////////////////////////////////////////
*/
blkbuf_t *  Bbuf_read(device_t * device,offset_t address)
{
    blkbuf_t      * blkbuf  = NULL;

    address &= ~7;
    /*  先检测是否已经存在 */
    if( NULL != ( blkbuf = Bbuf_query(device,address) ) )
    {
        blkbuf->bb_refcnt++;
        goto blkbuf_read_end;
    }
    /*  到达这里说明数据未读入，需要分配新的缓存块然后读数据    */
    if( NULL == ( blkbuf = Bbuf_get() ) )
        Sys_halt("Block buffer is NULL!\n");    /*  按设计是不会出现NULL的 */
    /*  数据读取失败，释放块缓存，返回失败  */
    if( Dev_read(device,address,blkbuf->bb_buffer,BBUF_SIZE) != BBUF_SIZE)
    {
        Bbuf_put(blkbuf);
        blkbuf = NULL;
        goto blkbuf_read_end;
    }
    blkbuf->bb_device   = device;
    blkbuf->bb_address  = address;
    blkbuf->bb_refcnt   = 1;
    Bbuf_hash_add(blkbuf);
    /*  在可用缓存小于系统总数的20%的时候，唤醒淘汰进程 */
    if( bb_eliminate_proc && (((bb_total_free * 10 )) / bb_total) < 2 )
        Proc_resume(&bb_eliminate_proc);

blkbuf_read_end:
    return blkbuf;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Bbuf_release
//  作    用: 释放块缓存，让系统可以尽快将其淘汰。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   device      |   设备对象指针
//  返 回 值: 成功返回RESULT_SUCCEED，失败返回REUSLT_FAILED。
//  说    明: 
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-01  |   罗  斌      |  建立函数
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Bbuf_release(blkbuf_t * blkbuf)
{
    //Bbuf_msg(blkbuf);
    ASSERT( blkbuf->bb_refcnt > 0 );
    if( --blkbuf->bb_refcnt < 0 )
        return RESULT_FAILED;
    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Bbuf_sync
//  作    用: 将设备与块缓存同步，也就是将修改过的块缓存写回磁盘。
//  参    数: 无
//  返 回 值: 无
//  说    明: 
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-08  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Bbuf_sync(void)
{
    blkbuf_t      * blkbuf = bb_used;
    CRITICAL_DECLARE(bb_lock);

    CRITICAL_BEGIN();
    while( blkbuf )
    {
        if( blkbuf->bb_flags & BBUF_FLAGS_DIRTED )
            BBUF_WRITE(blkbuf);
        blkbuf = blkbuf->bb_next;
    }
    CRITICAL_END();

    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Bbuf_initial
//  作    用: 初始化块缓存。
//  参    数: 无
//  返 回 值: 无
//  说    明: 建立可用块缓存列表，创建块缓存淘汰进程。
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-01  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Bbuf_initial(void)
{
    int             i       = 0;
    byte_t        * bbuf    = NULL;    /*  块缓存地址，*/
    handle_t        handle  = INVALID_HANDLE;
    blkbuf_t      * blkbuf  = (blkbuf_t *)bb_buffer;

    bb_free         = NULL;
    bb_used         = NULL;
    bb_used_tail    = NULL;
    bb_total_free   = 0;
    for( i = 0 ; i < BBUF_BUCKETS ; i++)
        bb_buckets[i] = NULL;
    /*  计算出缓存区的尾部  */
    bbuf = (byte_t *)(((uint_t)bb_buffer + BLKBUF_SIZE - BBUF_SIZE) & ~7);
    /*  构造可用块缓存列表  */
    for( ; bbuf > (byte_t *)(blkbuf + 1) ; bbuf -= BBUF_SIZE,++blkbuf)
    {
        _memzero(blkbuf,sizeof(blkbuf_t));
        blkbuf->bb_buffer = bbuf;
        Bbuf_put(blkbuf);
    }
    bb_total = bb_total_free;
    /*  创建块缓存淘汰进程  */
    handle = Proc_create("bbelim",32,1,Bbuf_eliminate,NULL,
        STACK_MAKE(bb_stack,BBUF_STACK),STACK_SIZE(bb_stack,BBUF_STACK));
    ASSERT( INVALID_HANDLE != handle);
    Handle_release(handle);
#ifdef _CFG_SMP_
    bb_lock = 0;
#endif  /*  _CFG_SMP_   */
}