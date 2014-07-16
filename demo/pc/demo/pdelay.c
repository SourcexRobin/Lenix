/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      Lenix
//                                                  Ƕ��ʽ����ϵͳ
//                                           2011 , ��  ��Դ���빤����
//                                                   �������а�Ȩ
//
//  �ļ�����    : pdelay.c
//  ����޸���  : ��  ��
//  ����޸�ʱ��:
//
//  ��Ҫ����    : ������ʱ����
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
byte_t                      app_stack3[USER_APP_STACK];
byte_t                      app_stack4[USER_APP_STACK];

void        app1(void * param)
{
    long i = 0;
    char    msg[32];
    *(int *)param = (int)param;
    for(;;)
    {
        sprintf(msg,"app1.%8ld \n",i++);
        Con_write_string(60,5,msg);

        if( 0 == (i % 10000) )
            Proc_delay(200);
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

        if( 0 == (i % 10000) )
            Proc_delay(400);
    }
}

void        app3(void * param)
{
    long i = 0;
    char    msg[32];
    *(int *)param = (int)param;
    for(;;)
    {
        sprintf(msg,"app3.%8ld ",i++);
        Con_write_string(60,7,msg);

        if( 0 == (i % 10000) )
            Proc_delay(600);
    }
}

void        app4(void * param)
{
    long i = 0;
    char    msg[32];
    *(int *)param = (int)param;
    for(;;)
    {
        sprintf(msg,"app4.%8ld ",i++);
        Con_write_string(60,8,msg);

        if( 0 == (i % 10000) )
            Proc_delay(800);
    }
}

void        User_initial(void)
{
    Proc_create("app1",60,3,app1,0,
        MAKE_STACK(app_stack1,USER_APP_STACK),
        STACK_SIZE(app_stack1,USER_APP_STACK));
    Proc_create("app2",60,3,app2,0,
        MAKE_STACK(app_stack2,USER_APP_STACK),
        STACK_SIZE(app_stack2,USER_APP_STACK));
    Proc_create("app3",60,3,app3,0,
        MAKE_STACK(app_stack3,USER_APP_STACK),
        STACK_SIZE(app_stack3,USER_APP_STACK));
    Proc_create("app4",60,3,app4,0,
        MAKE_STACK(app_stack4,USER_APP_STACK),
        STACK_SIZE(app_stack4,USER_APP_STACK));
}