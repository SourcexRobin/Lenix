/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      Lenix
//                                                  Ƕ��ʽ����ϵͳ
//                                           2012 , ��  ��Դ���빤����
//                                                   �������а�Ȩ
//
//  �ļ�����    : sema.c
//  ����޸���  : ��  ��
//  ����޸�ʱ��:
//
//  ��Ҫ����    : ���ڲ��Զ�ֵ�ź���
//
//  ˵��        : ������һ��4����ʹ��2����Դ�Ļ���
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
//======================================================================================================================
//  00.00.000   |   2012-01-09  |  �ޱ�         |  xxxxxx
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
#include <lenix.h>

#define USER_APP_STACK              2048

byte_t                      app_stack1[USER_APP_STACK];
byte_t                      app_stack2[USER_APP_STACK];
byte_t                      app_stack3[USER_APP_STACK];
byte_t                      app_stack4[USER_APP_STACK];

semaphore_t         sema;
int                 val;

#define N                           3
dword_t             time;

void        Con_print_char(byte_t c);
void        Clk_msg(void);
void        Shell_cmd_initial(void);

void        app1(void * param)
{
    int i;
    *(int *)param = (int)param;
    for( i = 0 ; i < N ; i++)
    {
        Sema_down(&sema);

        _printf("app1 write data! %d time : %ld\n",++val,Clk_get_ticks() - time);

        Proc_delay(1500);

        Sema_up(&sema);

        Proc_delay(200);
    }
    _printf("=app1 end \n");
}

void        app2(void * param)
{
    int i;
    *(int *)param = (int)param;
    for( i = 0 ; i < N ; i++)
    {
        Sema_down(&sema);

        _printf("\tapp2 write data! %d  time : %ld\n",++val,Clk_get_ticks() - time);

        Proc_delay(1000);

        Sema_up(&sema);

        Proc_delay(200);
    }
    _printf("==app2 end \n");
}

void        app3(void * param)
{
    int i;
    *(int *)param = (int)param;
    for( i = 0 ; i < N ; i++)
    {
        Sema_down(&sema);

        _printf("\t\tapp3 write data! %d  time : %ld\n",--val,Clk_get_ticks() - time);

        Proc_delay(800);

        Sema_up(&sema);
        
        Proc_delay(200);
    }
    _printf("===app3 end \n");
}

void        app4(void * param)
{
    int i;
    *(int *)param = (int)param;
    for( i = 0 ; i < N ; i++)
    {
        Sema_down(&sema);

        _printf("\t\t    app4 write data! %d  time : %ld\n",--val,Clk_get_ticks() - time);

        Proc_delay(200);

        Sema_up(&sema);

        Proc_delay(200);
    }
    _printf("====app4 end \n");
}

void        User_initial(void)
{
    Tty_echo_hook_set(TTY_MAJOR,Con_print_char);

#ifdef _CFG_DEBUG_

    Clk_ticks_hook_set(Clk_msg);

#endif  /*  _CFG_DEBUG_    */

    Shell_cmd_initial();

    SEMA_INITIAL(&sema,2);
    val = 0;
    time = Clk_get_ticks();
    Proc_create("app1",60,3,app1,0,
        MAKE_STACK(app_stack1,USER_APP_STACK),
        STACK_SIZE(app_stack1,USER_APP_STACK));
    Proc_create("app2",58,4,app2,0,
        MAKE_STACK(app_stack2,USER_APP_STACK),
        STACK_SIZE(app_stack2,USER_APP_STACK));
    Proc_create("app3",59,3,app3,0,
        MAKE_STACK(app_stack3,USER_APP_STACK),
        STACK_SIZE(app_stack3,USER_APP_STACK));
    Proc_create("app4",57,3,app4,0,
        MAKE_STACK(app_stack4,USER_APP_STACK),
        STACK_SIZE(app_stack4,USER_APP_STACK));
}


int         main(void)
{

    Lenix_initial();

    User_initial();

    Lenix_start();

    return 1;
}
