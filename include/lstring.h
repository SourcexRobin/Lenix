/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    Ƕ��ʽ����ϵͳ
//                             2011 , ��  ��Դ���빤����
//                                     �������а�Ȩ
//                          ***---------------------------***
//
//  �ļ�����    : lstring.h
//  �ļ�������  : ��  ��
//  �ļ�����ʱ��:
//  ����޸���  : ��  ��
//  ����޸�ʱ��:
//
//  ��Ҫ����    : �ṩ�������ַ�������������
//
//  ˵��        : 
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
//========================================================================================
//              |   2012-07-02  |  �ޱ�         |  ��type.h�з���
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