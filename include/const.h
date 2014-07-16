/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: const.h
//  ����ʱ��: 2012-12-20        ������: �ޱ�
//  �޸�ʱ��:                   �޸���: 
//  ��Ҫ����: �ṩ������ĳ������壬�Լ�LenixԤ����ĺꡣ
//
//  ˵    ��: 
//
//  �仯��¼:
//  �� �� ��    |   ʱ  ��    |   ��  ��    |  ��Ҫ�仯��¼
//=============================================================================
//              | 2014-02-26  |   ��  ��    |  ����LenixԤ�����
//              | 2012-12-20  |   ��  ��    |  �����������
///////////////////////////////////////////////////////////////////////////////
*/

#ifndef _CONST_H_
#define _CONST_H_

#define K                           ((uint_t)1024)
#define M                           (K*K)
#define G                           (((uint32_t)*K) * ((uint32_t)M))

#define NULL                        ((void *)0)
#define TRUE                        1
#define FALSE                       0
#define INVALID_HANDLE              0

/*
 *  ϵͳ���ȼ�������Lenix�趨��64�����ȼ�
 */
#define PROC_PRIORITY_MAX           64
/*
 *  PROC_INVALID_PRIORITY   :  ��Ч���ȼ�
 *  PROC_INVALID_PRIONUM    :  ��Ч������
 */
#define PROC_INVALID_PRIORITY       255
#define PROC_INVALID_PRIONUM        255

#define MIN_HEAP_SIZE               8192

/*
 *  ������״̬
 */
#define LOCK_STATUS_FREE            0
#define LOCK_STATUS_LOCKED          1


#define INT8_MIN                    0x80
#define INT8_MAX                    0x7F
#define UINT8_MAX                   0xFF

#define INT16_MIN                   0x8000
#define INT16_MAX                   0x7FFF
#define UINT16_MAX                  0xFFFF

#define INT32_MIN                   0x80000000
#define INT32_MAX                   0x7FFFFFFF
#define UINT32_MAX                  0xFFFFFFFF

#define INT64_MIN                   0x8000000000000000
#define INT64_MAX                   0x7FFFFFFFFFFFFFFF
#define UINT64_MAX                  0xFFFFFFFFFFFFFFFF

#define OBJ_NAME_LEN                12

/*
 *  �黺��ɢ�б��Ͱ������
 *    ѡ�����������û�о����ϸ����ѧ֤�������Ǹо�����Ϊ�����������������ݿ�
 *  �ķֲ���Ϊƽ�������Բ��õ�����������137��257��311������Ŷ����Ҳû�о����ϸ�
 *  ��֤��������ѡ����311����Ӧ��16������Ϊ0x137����Ӧ��2������Ϊ100110111B����
 *  ������ѧ�ϵ����С�
 */
#define BBUF_BUCKETS                311

/*
 *  �黺���С��
 *    ������һ��ֵ������һ��Ӧ���в����������Ĵ�С���Լ�һ������չ�ԡ���Ȼ����
 *  �����˷�ҳ״̬�£�ʵ�������ڴ�����н��������ı����ԡ�
 *    ��ƽ����״̬�£�311��Ͱ��ÿ��Ͱ����8�����ݿ飬��һ��311*8*4K = 9952K��Լ
 *  Ϊ10M�Ļ���ռ䡣��������淶Χ�ڣ������ɴﵽ΢�뼶�����ܡ�
 */
#define BBUF_SIZE                   4096

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  LenixԤ������ĺ�
*/

#define ABS(n)                      ((n)>0?(n):-(n))
#define MAX(a,b)                    ((a)>(b)?(a):(b))
#define MIN(a,b)                    ((a)<(b)?(a):(b))

/*
 *  ȡ�ýṹ�����ϣ������г�Ա��ƫ������
 *  t: ��������(type)
 *  m: ��Ա����(member)
 */
#define OFFSET_OF(t,m)              ((uint_t)(&(((t*)(0))->(m))))
/*
 *  �ӽṹ�����ϣ����͵ĳ�Աָ�뷵�ؽṹ�����ϣ����͵�ָ��
 *  t: ��������(type)
 *  m: ��Ա����(member)
 *  p: �ṹ�����ϣ����͵ĳ�Աָ��(pointer)
 */
#define TYPE_OF(t,m,p)              ((t*)((uint_t)(p) - OFFSET_OF(t,m)))

/*
 *  ����
 */
#define SWAP(type,a,b)              do{type temp = a; a = b ; b = temp;\
                                      }while(FALSE)

#endif  /*  _COSNT_H_   */

