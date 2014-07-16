/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: blkbuf.c  �黺�� block buffer
//  ����ʱ��: 2014-02-28        ������: �ޱ�
//  �޸�ʱ��:                   �޸���: 
//  ��Ҫ����: �黺�����
//  ˵    ��: ��ģ�������˿��û�����б����û�����б�����ɢ�б�
//
//  �����¼:
//  �� �� ��    |     ʱ��      |  ��  ��       |  ��Ҫ�仯��¼
//=============================================================================
//  00.00.000   |  2014-02-22   |  ��  ��       |  �����ļ�
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>
#include <result.h>
#include <assert.h>

#include <proc.h>
#include <device.h>

#include <blkbuf.h>

#define BBUF_STACK                  1024

/*  ���豸��ַת��ΪͰ��    */
#define OFFSET_TO_BUCKET(offset)    ((((uint_t)(offset)) >> 3) % BBUF_BUCKETS)

/*  �豸��ַ��Ӧ��Ͱ        */
#define BBUF_BUCKET(offset)         bb_buckets[OFFSET_TO_BUCKET(offset)]

/*  ϵͳ�п��õĿ黺���б�����Ҫ�黺��ʱ��������б��з���*/
static blkbuf_t        *    bb_free;

/*  ϵͳ���ѷ���Ŀ黺���б�ͬʱ�ṩ��ͷ�ͱ�β    */
static blkbuf_t        *    bb_used;
static blkbuf_t        *    bb_used_tail;

/*  ϵͳ�黺������  */
static uint32_t             bb_total;

/*  �黺�������    */
static uint32_t             bb_total_free;

/*
 *  Lenix���Ѿ����������ݿ����ɢ�б�ķ�ʽ���й���Ŀ����������ܡ�
 *  ɢ�б���ð���ַ����3λ��ȡģ�ķ�ʽ����������Ͱ��
 *  Ͱ���������ظ��ĵ�ַ����Ϊ��Ӧ���豸��ͬ��
 */
static blkbuf_t         *   bb_buckets[BBUF_BUCKETS];

/*
 *  �黺����̭����ջ
 */
static byte_t               bb_stack[BBUF_STACK];

/*
 *  ��̭���̵ȴ�����
 */
static proc_t           *   bb_eliminate_proc;

/*
 *  �黺�����ȴ��б�
 */
static proc_list_t          bb_wait_list;
/*
 *    ������ԵĽǶ���˵��Ӧ��ʹ�ö�̬�ڴ����ķ�ʽ���ṩ�ռ䡣���ǣ�ҲӦ�ṩ
 *  һ��ʹ�ù̶��ռ�ķ�ʽ��
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
//  �黺��ķ��䡢���գ��������б��ɢ�б��в����ɾ���Ļ���������
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
//  ��    ��: Bbuf_get
//  ��    ��: ����黺�档
//  ��    ��: ��
//  �� �� ֵ: ���ؿ黺�����ָ�롣�ɹ����ط�NULL��ʧ�ܷ���NULL
//  ˵    ��: ��������ǲ��᷵��NULL�ģ�Ҳ����ʱ�ض������ɹ�
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-01  |   ��  ��      |  ��������
///////////////////////////////////////////////////////////////////////////////
*/
static
blkbuf_t *  Bbuf_get(void)
{
    blkbuf_t      * blkbuf = NULL;
    CRITICAL_DECLARE(bb_lock);

    /*
     *  �޿��ÿ黺��ʱ���ȴ���
     *  ��ʱ��̭���̱ض��Ѿ���������˿��Ժܿ���ڿ��ÿ黺�� 
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
//  ��    ��: Bbuf_put
//  ��    ��: ���տ黺�档
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     blkbuf_t *   |   blkbuf      |   �黺�����ָ��
//  �� �� ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���REUSLT_FAILED��
//  ˵    ��: 
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-01  |   ��  ��      |  ��������
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
        /*  ���뵽��ͷ  */
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
//  ��    ��: Bbuf_hash_add
//  ��    ��: ���黺������������б��ɢ�б��С�
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     blkbuf_t *   |   blkbuf      |   �黺�����ָ��
//  �� �� ֵ: �ޡ�
//  ˵    ��:   �������б��ɢ�б��в���ʱ�����ڱ�ͷ���룬��֤��������������
//            �б����ǰ�棬�����Ժ�Ĳ������൱�ڰ�����ʱ������
//              Ϊ������ܣ��ں��ʵ�ʱ��Ϊɢ�б�����ƽ������
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-01  |   ��  ��      |  ��������
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
    /*  ����ɢ�б�  */
    if( bucket )
    {
        blkbuf->bb_bucket_next = bucket;
        bucket->bb_bucket_prev = blkbuf;
        bucket = blkbuf;
    }
    else
        BBUF_BUCKET(blkbuf->bb_address) = blkbuf;
    /*  ���������б�*/
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
//  ��    ��: Bbuf_hash_del
//  ��    ��: ���黺��������б��ɢ�б���ɾ����
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     blkbuf_t *   |   blkbuf      |   �黺�����ָ��
//  �� �� ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���REUSLT_FAILED��
//  ˵    ��: ���ݲ���ʱ����Ҫȷ��������ɢ�б��ڣ�������ݴ��󣬻ᵼ�´���
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-01  |   ��  ��      |  ��������
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
    /*  �������б���ɾ�� */
    if( blkbuf == bb_used)  /*  �����ͷ */
    {
        bb_used = next;
        if( next )
            next->bb_prev = NULL;
        else
            bb_used_tail  = NULL;   /*  �����Ѿ�û�У�����βҲҪ����*/
    }
    else
    {
        prev->bb_next = next;
        if( next )
            next->bb_prev = prev;
        else
            bb_used_tail  = prev;
    }
    /*  ��ɢ�б���ɾ��  */
    bucket  = BBUF_BUCKET(blkbuf->bb_address);
    prev    = blkbuf->bb_bucket_prev;
    next    = blkbuf->bb_bucket_next;
    if( blkbuf == bucket )  /*  �����ͷ */
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
//  ��    ��: Bbuf_query
//  ��    ��: ��ѯ�黺�档
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   device      |   �豸����ָ��
//     offset_t     |   address     |   �豸��ַ����Bbuf_read�������е�����
//
//  �� �� ֵ: ���ڷ��ط�NULL��ʧ�ܷ���NULL
//  ˵    ��: 
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-01  |   ��  ��      |  ��������
///////////////////////////////////////////////////////////////////////////////
*/
static
blkbuf_t *  Bbuf_query(device_t * device,offset_t address)
{
    blkbuf_t      * blkbuf  = BBUF_BUCKET(address);
    CRITICAL_DECLARE(bb_lock);

    CRITICAL_BEGIN();
    /*  ���ñ����ķ�ʽ����*/
    for( blkbuf ; blkbuf ; blkbuf = blkbuf->bb_bucket_next )
        if( blkbuf->bb_device == device && blkbuf->bb_address == address )
            break;
    CRITICAL_END();
    return blkbuf;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Bbuf_eliminate
//  ��    ��: �黺����̭���̡���ϵͳ�黺�治��ʱ����̭һ���������Ŀ黺�档
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     void *       |   param       |   ���̲���
//
//  �� �� ֵ: ��
//  ˵    ��:   ÿ����̭������ִ�С����Ƚ������޸Ĺ��Ŀ黺��д�ش��̣�Ȼ����̭
//            һ��������δ�޸Ŀ黺�档����̭�õ�һ�������Ŀ黺��ʱ���ȴ���һ��
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-01  |   ��  ��      |  ��������
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
     *  ���޸Ĺ��Ŀ黺��д���豸��
     *    ����ʹ������β��ʼ������������ϻ����Ѿ�ͬ����4���黺�棬д����������
     *  ��ô�����Է�ɢϵͳ��д����ʱ�䣬ͬʱ���Ա�֤�л�������̭��
     */
    blkbuf  = bb_used_tail; /*  ����ʹ���б�β��ʼ  */
    i       = 0;            /*  ��������д������    */
    for( ; i < 4 && blkbuf ; blkbuf = blkbuf->bb_prev )
    {
        if( blkbuf->bb_flags & BBUF_FLAGS_DIRTED )
        {
            Lck_lock(&blkbuf->bb_lock);
            i++;
        }
    }
    /*
     *  ��̭����
     *    1.�����ü������ٵĿ黺�濪ʼ��̭�������Ǵ���ʹ���б�β��ʼ����˸ն�
     *  �����ݵĿ黺��һ�㲻�ᱻ��̭��
     *    2.����̭δ�޸Ĺ��Ŀ黺��
     */
    blkbuf  = bb_used_tail; /*  ����ʹ���б�β��ʼ  */ 
    i       = 0;            /*  ����������̭������  */
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
    /*  �ڿ��ÿ黺�泬��������40%��ֹͣ��̭*/
    if( ((bb_total_free * 10 ) / bb_total) > 4 )
        Proc_wait(&bb_eliminate_proc);

    goto eliminate_begin;
}

/*
///////////////////////////////////////////////////////////////////////////////
/////////////////////
//  �黺��API
*/


/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Bbuf_read
//  ��    ��: �ӿ��豸�ж����ݵ��黺���С�
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   device      |   �豸����ָ��
//     offset_t     |   address     |   �豸��ַ������ĵ�ַ���Զ���ǰ����Ϊ8��
//                                  |  �����������磬����7��������Ϊ0
//  �� �� ֵ: �ɹ����ط�NULL��ʧ�ܷ���NULL��
//  ˵    ��:   �������Ȳ��Ҹÿ��Ƿ��Ѿ����ڣ�����Ѵ��ڣ�����Ҫ�ڴζ��豸��
//            ��������ڣ��Ż�ִ�о���Ķ�������
//              �ڿ��ÿ黺�治��������20%ʱ�����ѿ黺����̭���̡�
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-01  |   ��  ��      |  ��������
///////////////////////////////////////////////////////////////////////////////
*/
blkbuf_t *  Bbuf_read(device_t * device,offset_t address)
{
    blkbuf_t      * blkbuf  = NULL;

    address &= ~7;
    /*  �ȼ���Ƿ��Ѿ����� */
    if( NULL != ( blkbuf = Bbuf_query(device,address) ) )
    {
        blkbuf->bb_refcnt++;
        goto blkbuf_read_end;
    }
    /*  ��������˵������δ���룬��Ҫ�����µĻ����Ȼ�������    */
    if( NULL == ( blkbuf = Bbuf_get() ) )
        Sys_halt("Block buffer is NULL!\n");    /*  ������ǲ������NULL�� */
    /*  ���ݶ�ȡʧ�ܣ��ͷſ黺�棬����ʧ��  */
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
    /*  �ڿ��û���С��ϵͳ������20%��ʱ�򣬻�����̭���� */
    if( bb_eliminate_proc && (((bb_total_free * 10 )) / bb_total) < 2 )
        Proc_resume(&bb_eliminate_proc);

blkbuf_read_end:
    return blkbuf;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Bbuf_release
//  ��    ��: �ͷſ黺�棬��ϵͳ���Ծ��콫����̭��
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   device      |   �豸����ָ��
//  �� �� ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���REUSLT_FAILED��
//  ˵    ��: 
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-01  |   ��  ��      |  ��������
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
//  ��    ��: Bbuf_sync
//  ��    ��: ���豸��黺��ͬ����Ҳ���ǽ��޸Ĺ��Ŀ黺��д�ش��̡�
//  ��    ��: ��
//  �� �� ֵ: ��
//  ˵    ��: 
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-08  |   ��  ��      |  ��һ��
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
//  ��    ��: Bbuf_initial
//  ��    ��: ��ʼ���黺�档
//  ��    ��: ��
//  �� �� ֵ: ��
//  ˵    ��: �������ÿ黺���б������黺����̭���̡�
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-01  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Bbuf_initial(void)
{
    int             i       = 0;
    byte_t        * bbuf    = NULL;    /*  �黺���ַ��*/
    handle_t        handle  = INVALID_HANDLE;
    blkbuf_t      * blkbuf  = (blkbuf_t *)bb_buffer;

    bb_free         = NULL;
    bb_used         = NULL;
    bb_used_tail    = NULL;
    bb_total_free   = 0;
    for( i = 0 ; i < BBUF_BUCKETS ; i++)
        bb_buckets[i] = NULL;
    /*  �������������β��  */
    bbuf = (byte_t *)(((uint_t)bb_buffer + BLKBUF_SIZE - BBUF_SIZE) & ~7);
    /*  ������ÿ黺���б�  */
    for( ; bbuf > (byte_t *)(blkbuf + 1) ; bbuf -= BBUF_SIZE,++blkbuf)
    {
        _memzero(blkbuf,sizeof(blkbuf_t));
        blkbuf->bb_buffer = bbuf;
        Bbuf_put(blkbuf);
    }
    bb_total = bb_total_free;
    /*  �����黺����̭����  */
    handle = Proc_create("bbelim",32,1,Bbuf_eliminate,NULL,
        STACK_MAKE(bb_stack,BBUF_STACK),STACK_SIZE(bb_stack,BBUF_STACK));
    ASSERT( INVALID_HANDLE != handle);
    Handle_release(handle);
#ifdef _CFG_SMP_
    bb_lock = 0;
#endif  /*  _CFG_SMP_   */
}