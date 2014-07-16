/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    Ƕ��ʽ����ϵͳ
//                             2011 , ��  ��Դ���빤����
//                                     �������а�Ȩ
//                          ***---------------------------***
//
//  �ļ�����    : lmemory.h
//  �ļ�������  : ��  ��
//  �ļ�����ʱ��:
//  ����޸���  : ��  ��
//  ����޸�ʱ��:
//
//  ��Ҫ����    : �ṩ�������ڴ����������
//
//  ˵��        : 
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
//========================================================================================
//              |   2012-07-02  |  �ޱ�         |  ��type.h�з���
//////////////////////////////////////////////////////////////////////////////////////////
*/

#ifndef _LENIX_MEMORY_H_
#define _LENIX_MEMORY_H_

#include <type.h>

/*
#define MEM_ZERO(des,size)          do{ while((size_t)size)--) *((char *)(des))++ = 0; } while(0)
#define MEM_SET(des,v,size)         do{ while((size_t)size)--) *((char *)(des))++ = (char)(v); } while(0)
*/
void *      _memzero    (void * des,size_t size);
void *      _memset     (void * des,char v,size_t size);
int         _memcmp     (const void * des,const void * src,size_t size);
void *      _memcpy     (void * des,const void * src,size_t size);


#endif  /*  _LENIX_MEMORY_H_    */
