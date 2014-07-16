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
//  说    明:   直接使用指针存在较大的安全隐患，通过句柄的方式可以有效提高程序
//            的安全性。
//
//  变更记录:
//  版 本 号    |   时  间   |   作  者     | 主要变化记录
//=============================================================================
//  00.00.000   | 2014-02-04 |   罗  斌     | 
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>
#include <type.h>

#ifndef _KOUM_H_
#define _KOUM_H_

#include <result.h>


#define HANDLE_TO_IDX(handle)       (((hnd_part_t*)(&(handle)))->hp_idx )
#define HANDLE_TO_ATTR(handle)      (((hnd_part_t*)(&(handle)))->hp_attr)

#define HANDLE_ATTR_READ            0x01
#define HANDLE_ATTR_WRITE           0x02
#define HANDLE_ATTR_SYSTEM          0x80
#define HANDLE_ATTR_RDWR            (HANDLE_ATTR_READ | HANDLE_ATTR_WRITE)

/*  枚举内核对象类型 */
typedef enum _kernel_object_type_t
{
    kot_unuse,
    kot_proc,
    kot_mutex,
    kot_sema,
    kot_message,
    kot_file
}kot_t;

/*
 *  内核对象头。提供了内核对象所需要的公共属性。
 */
typedef struct _kernel_object_header_t
{
    kot_t                   koh_type;               /*  对象类型            */
    uint_t                  koh_idx;                /*  内核对象表索引      */
}koh_t;

#define OBJECT_TYPE(obj)            (((koh_t *)(obj))->koh_type)
#define OBJECT_IDX(obj)             (((koh_t *)(obj))->koh_idx )

#define Handle_add                  Koum_add
#define Handle_release              Koum_release

/*
 *  内核对象表表项。可以多个表项指向同一个内核对象。实际上实现了内核对象的共享
 */
typedef struct _kernel_object_item_h
{
    void                *   koi_object;             /*  对象指针    */
    result_t           (*   koi_release)(void *);   /*  释放函数    */
    kot_t                   koi_type;               /*  对象类型    */
    int                     koi_ref_cnt;            /*  引用计数    */
}koi_t;

extern koi_t                koum_table[KOUM_MAX];

handle_t    Koum_add(void * object,result_t (* release)(void *),
                     kot_t kot,byte_t attr);
result_t    Koum_release(handle_t handle);
void *      Koum_handle_object(handle_t handle);
void *      Koum_handle_release(handle_t handle);
kot_t       Koum_handle_type(handle_t handle);

#endif  /*  _KOUM_H_    */
