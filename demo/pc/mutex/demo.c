#include <lenix.h>

#define USER_APP_STACK              2048

byte_t                      app_stack1[USER_APP_STACK];
byte_t                      app_stack2[USER_APP_STACK];
byte_t                      app_stack3[USER_APP_STACK];

mutex_t                 *   mutex   = NULL;
int                         flag    = 0;

void        Con_print_char(byte_t c);
void        Clk_msg(void);

void        app1(void * param)
{
    long                    i = 0;
    char                    msg[32];

    while(1)
    {
        /*
        Mutex_get(mutex);
        */

        flag++;
        ASSERT(flag == 1);
        _sprintf(msg,"app1. flag = %d %8ld \n",flag,i++);
        Con_write_string(30,(int)param,msg);
        flag--;

        /*
        Mutex_put(mutex);
        */
    }
}

void        app2(void * param)
{
    long                    i       = 0;
    char                    msg[32] = {0};

    while(1)
    {
        /*
        Mutex_get(mutex);
        */

        flag++;
        ASSERT(flag == 1);
        _sprintf(msg," app2. flag = %d %8ld \n",flag,i++);
        Con_write_string(30,(int)param,msg);
        flag--;

        /*
        Mutex_put(mutex);
        */
    }
}

void        app3(void * param)
{
    long                    i       = 0;
    char                    msg[32] = {0};

    while(1)
    {
        /*
        Mutex_get(mutex);
        */

        flag++;
        ASSERT(flag == 1);
        _sprintf(msg,"  app3. flag = %d %8ld \n",flag,i++);
        Con_write_string(30,(int)param,msg);
        flag--;

        /*
        Mutex_put(mutex);
        */
    }
}

void        User_initial(void)
{
    if( NULL == ( mutex = Mutex_create() ) )
        return ;

    Tty_echo_hook_set(TTY_MAJOR,Con_print_char);

    Clk_ticks_hook_set(Clk_msg);

    Proc_create("app1",60,3,app1,(void *)5,
        MAKE_STACK(app_stack1,USER_APP_STACK),
        STACK_SIZE(app_stack1,USER_APP_STACK));

    Proc_create("app2",60,3,app2,(void *)6,
        MAKE_STACK(app_stack2,USER_APP_STACK),
        STACK_SIZE(app_stack2,USER_APP_STACK));

    Proc_create("app3",60,3,app3,(void *)7,
        MAKE_STACK(app_stack3,USER_APP_STACK),
        STACK_SIZE(app_stack3,USER_APP_STACK));
}

void        main(void)
{
    Lenix_initial();
    User_initial();
    Lenix_start();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenixʵʱ����ϵͳ
//                              2011 - 2012 @ Դ���빤����
//                                     �������а�Ȩ
//                          ***----------------------------***
//       ��  �� : democ.c 
//     ����ʱ�� :                  ������  : �ޱ�
//     �޸�ʱ�� : 2012-12-26       �޸���  : �ޱ�
//
//     ��Ҫ���� : �ṩ���������ʾ
//
//      ˵  ��  : 
//
//  �汾�仯��¼:
//  �汾��      |    ʱ  ��     |    ��  ��     | ��Ҫ�仯��¼
//========================================================================================
//  00.00.000   |               |    ��  ��     | 
//////////////////////////////////////////////////////////////////////////////////////////
*/
