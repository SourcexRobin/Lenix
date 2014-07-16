/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    嵌入式操作系统
//                             2011 , 罗  斌，源代码工作室
//                                     保留所有版权
//                          ***---------------------------***
//
//  文件名称    : lstring.h
//  文件创建者  : 罗  斌
//  文件创建时间:
//  最后修改者  : 罗  斌
//  最后修改时间:
//
//  主要功能    : 提供基本的字符串操作函数。
//
//  说明        : 
//
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//========================================================================================
//              |   2012-07-02  |  罗斌         |  从type.h中分离
//////////////////////////////////////////////////////////////////////////////////////////
*/

#ifndef _LENIX_STRING_H_
#define _LENIX_STRING_H_

#include <type.h>

int         _strlen     (const char * string);
int         _strcmp     (const char * str1,const char * str2);
char *      _strcat     (char * des,const char * src);
char *      _strcpy     (char * des,const char * src);
char *      _nstrcat    (char * des,const char * src,size_t size);
char *      _nstrcpy    (char * des,const char * src,size_t size);

int         _namecmp    (const char * name1,const char * name2);
int         _namecmpn   (const char * name1,const char * name2,int n);

int         _atoh       (const char * num);
int         _atoi       (const char * num);

int         _strmatch   (const char * str,const char * mode);

#endif  /*  _LENIX_STRING_H_    */