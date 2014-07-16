/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: type.h
//  ����ʱ��: 2011-07-12        ������: �ޱ�
//  �޸�ʱ��: 2012-11-29        �޸���: �ޱ�
//  ��Ҫ����: �ṩLenix�Ļ����������Ͷ��塣
//  ˵    ��: 
//
//  �����¼:
//  �� �� ��    |     ʱ��      |  ��  ��       | ��Ҫ�仯��¼
//=============================================================================
//  00.00.000   |  2011-07-12   |  ��  ��       | ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
#ifndef _TYPE_H_
#define _TYPE_H_

#include <config.h>

typedef unsigned int        uint_t;

typedef char                int8_t;
typedef short               int16_t;
typedef long                int32_t;

typedef unsigned char       uint8_t;        /*   8λ�޷�������  */
typedef unsigned short      uint16_t;       /*  16λ�޷�������  */
typedef unsigned long       uint32_t;       /*  32λ�޷�������  */

typedef unsigned char       byte_t;         /*  �ֽ�            */
typedef unsigned short      word_t;         /*  ��              */
typedef unsigned long       dword_t;        /*  ˫��            */

#ifdef _CFG_MSVC_
#pragma pack(1)
#endif /* _CFG_MSVC_ */
typedef struct _fword_t
{
    word_t                  low_16;
    dword_t                 high_32;
}fword_t;
#ifdef _CFG_MSVC_
#pragma pack()
#endif /* _CFG_MSVC_ */

#ifdef _CFG_MSVC_

typedef __int64             int64_t;
typedef unsigned __int64    uint64_t;
typedef unsigned __int64    qword_t;

#elif _CFG_GCC_

typedef unsigned long long  qword_t;
typedef unsigned long long  uint64_t;

#else

typedef struct
{
    dword_t                 qw_low32;
    dword_t                 qw_high32;
}qword_t;
typedef qword_t             uint64_t;

#endif


typedef int                 BOOL;
typedef uint_t              handle_t;
/*
 *  ����ֵ���ͣ�����ȷ��16λ����
 */
#if _CPU_WORD_ > 16

    /*
     *  ����ֵ���ͣ�ֻ��ʹ��16λ
     */
    typedef unsigned int    result_t;
    /*
     *  ��־�����ͣ�ֻ��ʹ��16λ
     */
    typedef unsigned int    flag_t;

    typedef struct _handle_part_t
    {
        word_t              hp_idx;         /*  �ں˶��������  */
        byte_t              hp_pad;
        byte_t              hp_attr;        /*  ��������        */
    }hnd_part_t;

#else

    typedef uint16_t        result_t;
    typedef uint16_t        flag_t;
    typedef uint16_t        handle_t;
    typedef struct _handle__part_t
    {
        byte_t              hp_idx:7;       /*  �ں˶��������  */
        byte_t              hp_pad:1;
        byte_t              hp_attr;        /*  ��������        */
    }hnd_part_t;
#endif  /*  _CPU_WORD_  */

/*
 *  ����־�����ͣ�����ʹ��32λ��
 */
typedef dword_t             lflag_t;

typedef volatile int        spin_lock_t;

/*
 *  ƫ�����ͣ�offset����д
 *  ���ݲ�ͬ�����ͣ��䶨�岢��һ��
 */
#if _CPU_WORD_ == 64 || _CPU_WORD_ == 32
    typedef int64_t         offset_t;
#else
    typedef long            offset_t;
#endif


#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned            size_t;
#endif  /*  _SIZE_T     */

typedef char            *   va_list;

typedef unsigned int        psw_t;          /*  processor state word������ĸ  */

typedef unsigned short      imr_t;

typedef struct _time_t
{
    uint8_t                 time_hour;      /*  ʱ   */
    uint8_t                 time_minute;    /*  ��   */
    uint8_t                 time_second;    /*  ��   */
    uint8_t                 time_week;
}time_t;

typedef struct _date_t
{
    uint16_t                date_year;      /*  ��   */
    uint8_t                 date_month;     /*  ��   */
    uint8_t                 date_day;       /*  ��   */
}date_t;

/*  �̸�ʽ��ʱ������    */
typedef struct _short_date_t
{
    word_t                  sd_day:5;
    word_t                  sd_month:4;
    word_t                  sd_year:7;          /*  1980��2107��       */
}sdate_t;

typedef struct _short_time_t
{
    word_t                  st_second:5;        /*  ����Ϊ2��           */
    word_t                  st_minute:6;
    word_t                  st_hour:5;
}stime_t;

/*  ����ʽ��ʱ���ʾ*/
typedef struct _long_time_t
{
    dword_t                 lt_second:5;        /*  ����Ϊ2��           */
    dword_t                 lt_minute:6;
    dword_t                 lt_hour:5;
    dword_t                 lt_day:5;
    dword_t                 lt_month:4;
    dword_t                 lt_year:7;          /*  1980��2107��       */
}ltime_t;
/*
 *  ����ֲʱ�������Ҫ���ԶԸú����Ĳ��������޸ģ������㲻ͬ������ 
 *  �жϷ������Interrupt Serveic Procdure�����isp
 */
typedef int                 (* isp_t)(int param1,int param2);


typedef struct _list_node_t
{
    struct _list_node_t *   ln_prev;
    struct _list_node_t *   ln_next;
}list_node_t;

typedef struct _list_t
{
    list_node_t         *   ls_head;
    list_node_t         *   ls_tail;
}list_t;

typedef list_t              list_head_t;

/*  ����Ӧ�ó���ʹ�õ��ں˶��󣬶�������ں� */
typedef struct _object_t
{
    void                *   obj_object;     /*  ����ָ��        */
    void                *   obj_release;    /*  �����ͷź���    */
    int                     obj_type;       /*  ��������        */
    int                     obj_refcnt;     /*  �������ü�����  */
}object_t;

#endif /*   _TYPE_H_    */