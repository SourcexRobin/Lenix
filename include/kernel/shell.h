/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenixʵʱ����ϵͳ
//                              2011 - 2012 @ Դ���빤����
//                                     �������а�Ȩ
//                          ***----------------------------***
//       ��  �� : tty.c 
//     ����ʱ�� : 2011-07-12       ������  : �ޱ�
//     �޸�ʱ�� : 2012-11-24       �޸���  : �ޱ�
//
//     ��Ҫ���� : �ṩ���ַ��ն˵�����������ܣ���ΪLenixĬ��֧�ֵ�Ӳ���豸
//
//      ˵  ��  : 
//
//  �汾�仯��¼:
//  �汾��      |    ʱ  ��     |    ��  ��     | ��Ҫ�仯��¼
//========================================================================================
//              |  2012-11-25   |    ��  ��     | ������������������ʶ�𣬺��ĸ��򵥵�
//                                              | �����ʾ��
//  00.00.000   |  2011-07-26   |    ��  ��     | ��һ��
//////////////////////////////////////////////////////////////////////////////////////////
*/


#ifndef _SHELL_H_
#define _SHELL_H_

#include <type.h>
#include <result.h>

#define SHELL_CMD_NAME_SIZE         8
#define SHELL_ARG_MAX               16

typedef int                 (* sc_entry_t)(int argc,char ** argv);

typedef struct _sc_map_t
{
    char                    scm_name[SHELL_CMD_NAME_SIZE];
    sc_entry_t              scm_entry;    
}sc_map_t;

void        Shell_initial(void);
char *      Sc_get_param(int argc,char ** argv,const char param);
result_t    Shell_cmd_add(const char * cmdname,sc_entry_t cmdentry);

#endif /*   _SHELL_H_   */

