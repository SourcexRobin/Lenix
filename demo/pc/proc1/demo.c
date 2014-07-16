
/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      Lenix
//                                                  Ƕ��ʽ����ϵͳ
//                                           2011 , ��  ��Դ���빤����
//                                                   �������а�Ȩ
//
//  �ļ�����    : demo1.c
//  ����޸���  : ��  ��
//  ����޸�ʱ��:
//
//  ��Ҫ����    : ��ʾ��Lenixϵͳ�������У��Լ���ͬ���ȼ�����cpuʱ���Ч������
//
//  ˵��        : ��ʹ��ʱ�����ļ�����Ϊuserapp.c
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
//======================================================================================================================
//  00.00.000   |   2011-12-31  |  �ޱ�         |  xxxxxx
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/

#include <lenix.h>

#define USER_APP_STACK              1536

byte_t                      app_stack1[USER_APP_STACK];
byte_t                      app_stack2[USER_APP_STACK];
byte_t                      app_stack3[USER_APP_STACK];
byte_t                      app_stack4[USER_APP_STACK];
byte_t                      app_stack5[USER_APP_STACK];
byte_t                      app_stack6[USER_APP_STACK];
byte_t                      app_stack7[USER_APP_STACK];

void        Clk_msg(void);

void        Title(void * param)
{
    char                    msg[80]     = {0};

    param = param; /*  �����������������δʹ�þ���  */


    for(;;)
    {
        Con_write_string(0,5," name          pid  pr  pn  run time  use  test",0x7);
        Con_write_string(0,6," ------------  ---  --  --  --------  ---  --------",0x7);
        _sprintf(msg,"system run time: %8ld",Clk_get_ticks());
        Con_write_string(0,24,msg,0x7);
        Proc_delay(1000);
    }
}

void        Content(void * param)
{
    long                    i           = 0;
    char                    msg[80]     = {0};
    char                    name[12]    = {0};
    int                     pid         = 0;

    Proc_delay(50);

    pid = Proc_get_pid();

    for(;;)
    {
        _sprintf(msg," %-12s  %3d  %2d  %2d  %8ld  %2d%%  %8ld",Proc_get_name(name),
            pid,Proc_get_priority(pid),Proc_get_prio_num(pid),
            Proc_get_run_time(),(int)(100*Proc_get_run_time()/Clk_get_ticks()),i++);
        Con_write_string(0,7 + (int)param,msg,0x7);
    }
}

void        User_initial(void)
{
    Clk_ticks_hook_set(Clk_msg);

    Proc_create("app1",60,5,Title,0,
        MAKE_STACK(app_stack1,USER_APP_STACK),
        STACK_SIZE(app_stack1,USER_APP_STACK));
    Proc_create("app2",60,1,Content,0,
        MAKE_STACK(app_stack2,USER_APP_STACK),
        STACK_SIZE(app_stack2,USER_APP_STACK));
    Proc_create("app3",60,2,Content,(void *)1,
        MAKE_STACK(app_stack3,USER_APP_STACK),
        STACK_SIZE(app_stack3,USER_APP_STACK));
    Proc_create("app4",60,3,Content,(void *)2,
        MAKE_STACK(app_stack4,USER_APP_STACK),
        STACK_SIZE(app_stack5,USER_APP_STACK));
    Proc_create("app5",60,4,Content,(void *)3,
        MAKE_STACK(app_stack5,USER_APP_STACK),
        STACK_SIZE(app_stack5,USER_APP_STACK));
    Proc_create("app6",60,5,Content,(void *)4,
        MAKE_STACK(app_stack6,USER_APP_STACK),
        STACK_SIZE(app_stack6,USER_APP_STACK));
    Proc_create("app7",60,5,Content,(void *)5,
        MAKE_STACK(app_stack7,USER_APP_STACK),
        STACK_SIZE(app_stack7,USER_APP_STACK));
}

int         main(void)
{

    Lenix_initial();

    User_initial();

    Lenix_start();

    return 1;
}
