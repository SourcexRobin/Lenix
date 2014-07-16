/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: type.h
//  创建时间: 2011-07-12        创建者: 罗斌
//  修改时间: 2012-11-29        修改者: 罗斌
//  主要功能: 提供Lenix的基本数据类型定义。
//  说    明: 
//
//  变更记录:
//  版 本 号    |     时间      |  作  者       | 主要变化记录
//=============================================================================
//  00.00.000   |  2011-07-12   |  罗  斌       | 第一版
///////////////////////////////////////////////////////////////////////////////
*/
#ifndef _TYPE_H_
#define _TYPE_H_

#include <config.h>

typedef unsigned int        uint_t;

typedef char                int8_t;
typedef short               int16_t;
typedef long                int32_t;

typedef unsigned char       uint8_t;        /*   8位无符号整数  */
typedef unsigned short      uint16_t;       /*  16位无符号整数  */
typedef unsigned long       uint32_t;       /*  32位无符号整数  */

typedef unsigned char       byte_t;         /*  字节            */
typedef unsigned short      word_t;         /*  字              */
typedef unsigned long       dword_t;        /*  双字            */

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
 *  返回值类型，必须确保16位以上
 */
#if _CPU_WORD_ > 16

    /*
     *  返回值类型，只能使用16位
     */
    typedef unsigned int    result_t;
    /*
     *  标志组类型，只能使用16位
     */
    typedef unsigned int    flag_t;

    typedef struct _handle_part_t
    {
        word_t              hp_idx;         /*  内核对象表索引  */
        byte_t              hp_pad;
        byte_t              hp_attr;        /*  对象属性        */
    }hnd_part_t;

#else

    typedef uint16_t        result_t;
    typedef uint16_t        flag_t;
    typedef uint16_t        handle_t;
    typedef struct _handle__part_t
    {
        byte_t              hp_idx:7;       /*  内核对象表索引  */
        byte_t              hp_pad:1;
        byte_t              hp_attr;        /*  对象属性        */
    }hnd_part_t;
#endif  /*  _CPU_WORD_  */

/*
 *  长标志组类型，可以使用32位。
 */
typedef dword_t             lflag_t;

typedef volatile int        spin_lock_t;

/*
 *  偏移类型，offset的缩写
 *  根据不同的类型，其定义并不一样
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

typedef unsigned int        psw_t;          /*  processor state word的首字母  */

typedef unsigned short      imr_t;

typedef struct _time_t
{
    uint8_t                 time_hour;      /*  时   */
    uint8_t                 time_minute;    /*  分   */
    uint8_t                 time_second;    /*  秒   */
    uint8_t                 time_week;
}time_t;

typedef struct _date_t
{
    uint16_t                date_year;      /*  年   */
    uint8_t                 date_month;     /*  月   */
    uint8_t                 date_day;       /*  日   */
}date_t;

/*  短格式的时间类型    */
typedef struct _short_date_t
{
    word_t                  sd_day:5;
    word_t                  sd_month:4;
    word_t                  sd_year:7;          /*  1980至2107年       */
}sdate_t;

typedef struct _short_time_t
{
    word_t                  st_second:5;        /*  精度为2秒           */
    word_t                  st_minute:6;
    word_t                  st_hour:5;
}stime_t;

/*  长格式的时间表示*/
typedef struct _long_time_t
{
    dword_t                 lt_second:5;        /*  精度为2秒           */
    dword_t                 lt_minute:6;
    dword_t                 lt_hour:5;
    dword_t                 lt_day:5;
    dword_t                 lt_month:4;
    dword_t                 lt_year:7;          /*  1980至2107年       */
}ltime_t;
/*
 *  在移植时，如果需要可以对该函数的参数进行修改，以满足不同的需求 
 *  中断服务程序，Interrupt Serveic Procdure，简称isp
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

/*  管理应用程序使用的内核对象，对外隔离内核 */
typedef struct _object_t
{
    void                *   obj_object;     /*  对象指针        */
    void                *   obj_release;    /*  对象释放函数    */
    int                     obj_type;       /*  对象类型        */
    int                     obj_refcnt;     /*  对象引用计数器  */
}object_t;

#endif /*   _TYPE_H_    */