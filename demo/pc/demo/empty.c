/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      Lenix
//                                                  Ƕ��ʽ����ϵͳ
//                                           2011 , ��  ��Դ���빤����
//                                                   �������а�Ȩ
//
//  �ļ�����    : empty.c
//  �ļ�������  : 
//  ����޸���  : ��  ��
//  ����޸�ʱ��:
//
//  ��Ҫ����    :
//
//  ˵��        : ��Ӧ�ó����ṩһ��ģ�壬ʹ��ʱ�޸�Ϊ���ļ����޸�Ϊuserapp.c����
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
//======================================================================================================================
//  00.00.000   |   2011-02-09  |  �ޱ�         |  xxxxxx
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
#include <stdio.h>
#include <lenix.h>

#define USER_APP_STACK              1024

byte_t                      app_stack1[USER_APP_STACK];
byte_t                      app_stack2[USER_APP_STACK];

void        app1(void * param)
{
    long i = 0;
    char    msg[32];
    *(int *)param = (int)param;
    for(;;)
    {
        sprintf(msg,"app1.%8ld \n",i++);
        Con_write_string(60,5,msg);
        Proc_delay(20);
    }
}

void        app2(void * param)
{
    long i = 0;
    char    msg[32];
    *(int *)param = (int)param;
    for(;;)
    {
        sprintf(msg,"app2.%8ld ",i++);
        Con_write_string(60,6,msg);
        Proc_delay(30);
    }
}

void        User_initial(void)
{
}