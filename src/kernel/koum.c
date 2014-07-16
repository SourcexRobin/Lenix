/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: koum.h   �ں˶���ͳһ����.kernel object unified manage
//  ����ʱ��: 2014-02-04        ������: �ޱ�
//  �޸�ʱ��:                   �޸���: 
//
//  ��Ҫ����: �ṩ�ں˶���ͳһ�����ܵ����ݶ���
//
//  ˵    ��: 
//
//  �����¼:
//  �� �� ��    |   ʱ  ��   |   ��  ��     | ��Ҫ�仯��¼
//=============================================================================
//  00.00.000   | 2014-02-04 |   ��  ��     | 
///////////////////////////////////////////////////////////////////////////////
*/

#include <assert.h>
#include <proc.h>
#include "koum.h"


static koi_t                koum_table[KOUM_MAX];
static int                  koum_idx;
#define LAST_KOI            
#ifdef _CFG_SMP_
spin_lock_t                 koum_table_lock;
#endif  /*  _CFG_SMP_   */

#define HANDLE_TO_OBJECT(h)         (koum_table[HANDLE_TO_IDX(h)].koi_object)
#define HANDLE_TO_RELEASE(h)        (koum_table[HANDLE_TO_IDX(h)].koi_release)
#define HANDLE_TO_TYPE(h)           (koum_table[HANDLE_TO_IDX(h)].koi_type)

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Koum_add
//  ��  ��: ��ϵͳ����ں˶���
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      void *      |   object      |   �ں˶���ָ��
//result_t(*)(void*)|   release     |   �ں˶����ͷź���������Ϊ�ա���ʹ������
//                                  | �ĺ���ԭ�͡�
//                                  |   result release(void * objectptr);
//      kot_t       |    type       |   �ں˶�������
//      byte_t      |    attr       |   �������
//              HANDLE_ATTR_READ    |   ����ɶ�
//              HANDLE_ATTR_WRITE   |   �����д
//              HANDLE_ATTR_RDWR    |   ����ɶ�д
//              HANDLE_ATTR_SYSTEM  |   ϵͳ���
//  ����ֵ: �ɹ����ض�������ʧ�ܷ���INVALID_HANDLE
//  ע  ��:   
//
//  �����¼:
//  ʱ  ��      |   ��  ��      |  ˵��
//=============================================================================
//  2014-02-05  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
handle_t    Koum_add(void         * object,
                     result_t    (* release)(void *),
                     kot_t          type,
                     byte_t         attr)
{
    koi_t         * koi     = NULL;
    handle_t        handle  = INVALID_HANDLE;
    uint_t          idx     = koum_idx;
    CRITICAL_DECLARE(koum_table_lock);

    ASSERT( NULL != object);
    ASSERT( NULL != release);
    ASSERT( kot_unuse != type);
#ifdef _CFG_RUN_TIME_CHECK_
    if( NULL == object || NULL == release || kot_unuse == type )
        goto koum_add_end;
#endif  /*  _CFG_RUN_TIME_CHECK_ */
    /*  ������һ���ں˶����� */
    CRITICAL_BEGIN();
    do{
        if( NULL == koum_table[koum_idx].koi_object )
        {
            koi = &koum_table[koum_idx];
            koi->koi_object     = object;
            koi->koi_release    = release;
            koi->koi_type       = type;
            koi->koi_ref_cnt    = 1;
            ((koh_t *)object)->koh_type = type;
        }
        koum_idx = (koum_idx + 1) % KOUM_MAX;
    }while(NULL == koi && idx != koum_idx);
    CRITICAL_END();
    if( koi )
    {
        ((hnd_part_t *)(&handle))->hp_idx   = koi - koum_table;
        ((hnd_part_t *)(&handle))->hp_pad   = 0;
        ((hnd_part_t *)(&handle))->hp_attr  = attr;
    }
koum_add_end:
    return handle;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Koum_release
//  ��  ��: �ͷ��ں˶���
//  ��  ��: 
//      handle      : handle_t      +   �ں˶�����
//  ����ֵ: ��
//  ע  ��:   ����������ں˶�������һ�²����ͷš�
//
//  �����¼:
//  ʱ  ��      |   ��  ��      |  ˵��
//=============================================================================
//  2014-02-05  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Koum_release(handle_t handle)
{
    koi_t         * koi     = 0;
    CRITICAL_DECLARE(koum_table_lock);

    ASSERT( INVALID_HANDLE != handle );
    koi = &koum_table[HANDLE_TO_IDX(handle)];
    ASSERT( NULL != koi->koi_object );
    /*
     *  ������͡���ֹ�������仯����ɴ����ͷ�
     */
    if( koi->koi_type != OBJECT_TYPE(koi->koi_object) )
        goto koum_release_failed;
    koi->koi_ref_cnt--;
    ASSERT(koi->koi_ref_cnt >= 0 );
    if( koi->koi_ref_cnt == 0 )
    {
        if( RESULT_FAILED == koi->koi_release(HANDLE_TO_OBJECT(handle)))
            goto koum_release_failed;
        CRITICAL_BEGIN();
        koi->koi_object     = NULL;
        koi->koi_release    = NULL;
        koi->koi_type       = kot_unuse;
        /*
         *    ��������������Ϊ�ͷŵ���Ŀ����������һ�β���ʱ��������ÿ�����Ŀ,
         *  ������������ܡ�
         */
        koum_idx = koi - koum_table; 
        CRITICAL_END();
    }

    return RESULT_SUCCEED;
koum_release_failed:
    return RESULT_FAILED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Koum_handle_object
//  ��  ��: ��þ����Ӧ�Ķ���ָ�롣
//  ��  ��: 
//      handle      : handle_t      +   �ں˶�����
//  ����ֵ: ����void *���͵�ָ��
//  ע  ��:   
//
//  �����¼:
//  ʱ  ��      |   ��  ��      |  ˵��
//=============================================================================
//  2014-02-24  |   ��  ��      |  ����ԭ�����ͷź��Է�����Чָ��
//  2014-02-05  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void *      Koum_handle_object(handle_t handle)
{
    koi_t         * koi     = 0;

    ASSERT( INVALID_HANDLE != handle );
    koi = &koum_table[HANDLE_TO_IDX(handle)];
#ifdef _CFG_DEBUG_
    /*_printk("handle type: %d object type: %d\n",
        koi->koi_type,OBJECT_TYPE(koi->koi_object));
    /**/
#endif  /*  _CFG_DEBUG_ */
    if( koi->koi_type == OBJECT_TYPE(koi->koi_object) )
        return koi->koi_object;
    return NULL;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Koum_handle_release
//  ��  ��: ��þ����Ӧ���ͷź�����
//  ��  ��: 
//      handle      : handle_t      +   �ں˶�����
//  ����ֵ: ����void *���͵�ָ��
//  ע  ��:   
//
//  �����¼:
//  ʱ  ��      |   ��  ��      |  ˵��
//=============================================================================
//  2014-02-05  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void *      Koum_handle_release(handle_t handle)
{
    return  HANDLE_TO_RELEASE(handle);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Koum_handle_type
//  ��  ��: ��þ����Ӧ�����͡�
//  ��  ��: 
//      handle      : handle_t      +   �ں˶�����
//  ����ֵ: ��
//  ע  ��:   
//
//  �����¼:
//  ʱ  ��      |   ��  ��      |  ˵��
//=============================================================================
//  2014-02-05  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
kot_t       Koum_handle_type(handle_t handle)
{
    return HANDLE_TO_TYPE(handle);
}

