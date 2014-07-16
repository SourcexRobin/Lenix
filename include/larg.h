/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenix实时操作系统
//                              2011 - 2012 @ 源代码工作室
//                                     保留所有版权
//                          ***----------------------------***
//       名  称 : larg.c
//     创建时间 : 2012-11-26       创建者  : 罗斌
//     修改时间 :                  修改者  : 
//
//     主要功能 : 
//
//      说  明  : 
//
//  版本变化记录:
//  版本号      |    时  间     |    作  者     | 主要变化记录
//========================================================================================
//  00.00.000   |  xxxx-xx-xx   |  xxxx         | xxxxxx
//////////////////////////////////////////////////////////////////////////////////////////
*/


#ifndef _LARG_H_
#define _LARG_H_

#include <type.h>

/*
 *  长格式类型定义
 */
#if _CPU_WORD_ == 8
    typedef int16_t         large_t;
    typedef uint16_t        ularge_t;
#elif _CPU_WORD_ == 16
    typedef int32_t         large_t;
    typedef uint32_t        ularge_t;
#elif _CPU_WORD_ == 32
    typedef int64_t         large_t;
    typedef uint64_t        ularge_t;
#else

#endif

#define _INTSIZEOF(n)               ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(al,fmt)            do{ al = (va_list)&(fmt) + _INTSIZEOF(fmt); }while(0)
#define va_arg(al,t)                *(t *)((al += _INTSIZEOF(t)) - _INTSIZEOF(t))
#define va_end(al)                  do{ al = (va_list)0; }while(0)


#endif  /*  _LARG_H_    */