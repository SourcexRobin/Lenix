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
//  ˵    ��:   ֱ��ʹ��ָ����ڽϴ�İ�ȫ������ͨ������ķ�ʽ������Ч��߳���
//            �İ�ȫ�ԡ�
//
//  �����¼:
//  �� �� ��    |   ʱ  ��   |   ��  ��     | ��Ҫ�仯��¼
//=============================================================================
//  00.00.000   | 2014-02-04 |   ��  ��     | 
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

/*  ö���ں˶������� */
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
 *  �ں˶���ͷ���ṩ���ں˶�������Ҫ�Ĺ������ԡ�
 */
typedef struct _kernel_object_header_t
{
    kot_t                   koh_type;               /*  ��������            */
    uint_t                  koh_idx;                /*  �ں˶��������      */
}koh_t;

#define OBJECT_TYPE(obj)            (((koh_t *)(obj))->koh_type)
#define OBJECT_IDX(obj)             (((koh_t *)(obj))->koh_idx )

#define Handle_add                  Koum_add
#define Handle_release              Koum_release

/*
 *  �ں˶���������Զ������ָ��ͬһ���ں˶���ʵ����ʵ�����ں˶���Ĺ���
 */
typedef struct _kernel_object_item_h
{
    void                *   koi_object;             /*  ����ָ��    */
    result_t           (*   koi_release)(void *);   /*  �ͷź���    */
    kot_t                   koi_type;               /*  ��������    */
    int                     koi_ref_cnt;            /*  ���ü���    */
}koi_t;

extern koi_t                koum_table[KOUM_MAX];

handle_t    Koum_add(void * object,result_t (* release)(void *),
                     kot_t kot,byte_t attr);
result_t    Koum_release(handle_t handle);
void *      Koum_handle_object(handle_t handle);
void *      Koum_handle_release(handle_t handle);
kot_t       Koum_handle_type(handle_t handle);

#endif  /*  _KOUM_H_    */
