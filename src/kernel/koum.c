/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: koum.h   内核对象统一管理.kernel object unified manage
//  创建时间: 2014-02-04        创建者: 罗斌
//  修改时间:                   修改者: 
//
//  主要功能: 提供内核对象统一管理功能的数据定义
//
//  说    明: 
//
//  变更记录:
//  版 本 号    |   时  间   |   作  者     | 主要变化记录
//=============================================================================
//  00.00.000   | 2014-02-04 |   罗  斌     | 
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
//  名  称: Koum_add
//  作  用: 向系统添加内核对象。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      void *      |   object      |   内核对象指针
//result_t(*)(void*)|   release     |   内核对象释放函数，不能为空。因使用如下
//                                  | 的函数原型。
//                                  |   result release(void * objectptr);
//      kot_t       |    type       |   内核对象类型
//      byte_t      |    attr       |   句柄属性
//              HANDLE_ATTR_READ    |   句柄可读
//              HANDLE_ATTR_WRITE   |   句柄可写
//              HANDLE_ATTR_RDWR    |   句柄可读写
//              HANDLE_ATTR_SYSTEM  |   系统句柄
//  返回值: 成功返回对象句柄，失败返回INVALID_HANDLE
//  注  意:   
//
//  变更记录:
//  时  间      |   作  者      |  说明
//=============================================================================
//  2014-02-05  |   罗  斌      |  第一版
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
    /*  跳过第一个内核对象项 */
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
//  名  称: Koum_release
//  作  用: 释放内核对象。
//  参  数: 
//      handle      : handle_t      +   内核对象句柄
//  返回值: 无
//  注  意:   句柄类型与内核对象类型一致才能释放。
//
//  变更记录:
//  时  间      |   作  者      |  说明
//=============================================================================
//  2014-02-05  |   罗  斌      |  第一版
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
     *  检查类型。防止对象句柄变化，造成错误释放
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
         *    将查找索引设置为释放的条目，可以让下一次查找时，立即获得可用条目,
         *  有助于提高性能。
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
//  名  称: Koum_handle_object
//  作  用: 获得句柄对应的对象指针。
//  参  数: 
//      handle      : handle_t      +   内核对象句柄
//  返回值: 返回void *类型的指针
//  注  意:   
//
//  变更记录:
//  时  间      |   作  者      |  说明
//=============================================================================
//  2014-02-24  |   罗  斌      |  修正原对象释放后仍返回有效指针
//  2014-02-05  |   罗  斌      |  第一版
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
//  名  称: Koum_handle_release
//  作  用: 获得句柄对应的释放函数。
//  参  数: 
//      handle      : handle_t      +   内核对象句柄
//  返回值: 返回void *类型的指针
//  注  意:   
//
//  变更记录:
//  时  间      |   作  者      |  说明
//=============================================================================
//  2014-02-05  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void *      Koum_handle_release(handle_t handle)
{
    return  HANDLE_TO_RELEASE(handle);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Koum_handle_type
//  作  用: 获得句柄对应的类型。
//  参  数: 
//      handle      : handle_t      +   内核对象句柄
//  返回值: 无
//  注  意:   
//
//  变更记录:
//  时  间      |   作  者      |  说明
//=============================================================================
//  2014-02-05  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
kot_t       Koum_handle_type(handle_t handle)
{
    return HANDLE_TO_TYPE(handle);
}

