/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    嵌入式操作系统
//                             2011 , 罗  斌，源代码工作室
//                                     保留所有版权
//                          ***---------------------------***
//
//  文件名称    : lmemory.h
//  文件创建者  : 罗  斌
//  文件创建时间:
//  最后修改者  : 罗  斌
//  最后修改时间:
//
//  主要功能    : 提供基本的内存操作函数。
//
//  说明        : 
//
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//========================================================================================
//              |   2012-07-02  |  罗斌         |  从type.h中分离
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
