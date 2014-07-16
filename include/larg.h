/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenixʵʱ����ϵͳ
//                              2011 - 2012 @ Դ���빤����
//                                     �������а�Ȩ
//                          ***----------------------------***
//       ��  �� : larg.c
//     ����ʱ�� : 2012-11-26       ������  : �ޱ�
//     �޸�ʱ�� :                  �޸���  : 
//
//     ��Ҫ���� : 
//
//      ˵  ��  : 
//
//  �汾�仯��¼:
//  �汾��      |    ʱ  ��     |    ��  ��     | ��Ҫ�仯��¼
//========================================================================================
//  00.00.000   |  xxxx-xx-xx   |  xxxx         | xxxxxx
//////////////////////////////////////////////////////////////////////////////////////////
*/


#ifndef _LARG_H_
#define _LARG_H_

#include <type.h>

/*
 *  ����ʽ���Ͷ���
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