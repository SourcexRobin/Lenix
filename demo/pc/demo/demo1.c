
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

#define USER_APP_STACK              1024

byte_t                      app_stack1[USER_APP_STACK];
byte_t                      app_stack2[USER_APP_STACK];
byte_t                      app_stack3[USER_APP_STACK];
byte_t                      app_stack4[USER_APP_STACK];
byte_t                      app_stack5[USER_APP_STACK];
byte_t                      app_stack6[USER_APP_STACK];
byte_t                      app_stack7[USER_APP_STACK];

void        app1(void * param)
{
    char                    msg[80]     = {0};

    param = param; /*  �����������������δʹ�þ��棬��ͬ  */


    for(;;)
    {
        Con_write_string(0,5," name          pid  pr  pn  run time  use  test");
        Con_write_string(0,6," ------------  ---  --  --  --------  ---  --------");
        _sprintf(msg,"system run time: %8ld",Clk_get_ticks());
        Con_write_string(0,24,msg);
        Proc_delay(1000);
    }
}

void        app2(void * param)
{
    long                    i           = 0;
    char                    msg[80]     = {0};
    char                    name[12]    = {0};
    int                     pid         = 0;
    param = param;
    Proc_delay(50);

    pid = Proc_get_pid();

    for(;;)
    {
        _sprintf(msg," %-12s  %3d  %2d  %2d  %8ld  %2d%%  %8ld",Proc_get_name(name),
            pid,Proc_get_priority(pid),Proc_get_prio_num(pid),
            Proc_get_run_time(),(int)(100*Proc_get_run_time()/Clk_get_ticks()),i++);
        Con_write_string(0,7,msg);
    }
}

void        app3(void * param)
{
    long                    i           = 0;
    char                    msg[80]     = {0};
    char                    name[12]    = {0};
    int                     pid         = 0;
    param = param;
    Proc_delay(50);

    pid = Proc_get_pid();

    for(;;)
    {
        _sprintf(msg," %-12s  %3d  %2d  %2d  %8ld  %2d%%  %8ld",Proc_get_name(name),
            pid,Proc_get_priority(pid),Proc_get_prio_num(pid),
            Proc_get_run_time(),(int)(100*Proc_get_run_time()/Clk_get_ticks()),i++);
        Con_write_string(0,8,msg);
    }
}
void        app4(void * param)
{
    long                    i           = 0;
    char                    msg[80]     = {0};
    char                    name[12]    = {0};
    int                     pid         = 0;
    param = param;
    Proc_delay(50);

    pid = Proc_get_pid();

    for(;;)
    {
        _sprintf(msg," %-12s  %3d  %2d  %2d  %8ld  %2d%%  %8ld",Proc_get_name(name),
            pid,Proc_get_priority(pid),Proc_get_prio_num(pid),
            Proc_get_run_time(),(int)(100*Proc_get_run_time()/Clk_get_ticks()),i++);
        Con_write_string(0,9,msg);
    }
}
void        app5(void * param)
{
    long                    i           = 0;
    char                    msg[80]     = {0};
    char                    name[12]    = {0};
    int                     pid         = 0;
    param = param;
    Proc_delay(50);

    pid = Proc_get_pid();

    for(;;)
    {
        _sprintf(msg," %-12s  %3d  %2d  %2d  %8ld  %2d%%  %8ld",Proc_get_name(name),
            pid,Proc_get_priority(pid),Proc_get_prio_num(pid),
            Proc_get_run_time(),(int)(100*Proc_get_run_time()/Clk_get_ticks()),i++);
        Con_write_string(0,10,msg);
    }
}

void        app6(void * param)
{
    long                    i           = 0;
    char                    msg[80]     = {0};
    char                    name[12]    = {0};
    int                     pid         = 0;
    param = param;
    Proc_delay(50);

    pid = Proc_get_pid();

    for(;;)
    {
        _sprintf(msg," %-12s  %3d  %2d  %2d  %8ld  %2d%%  %8ld",Proc_get_name(name),
            pid,Proc_get_priority(pid),Proc_get_prio_num(pid),
            Proc_get_run_time(),(int)(100*Proc_get_run_time()/Clk_get_ticks()),i++);
        Con_write_string(0,11,msg);
    }
}
void        app7(void * param)
{
    long                    i           = 0;
    char                    msg[80]     = {0};
    char                    name[12]    = {0};
    int                     pid         = 0;
    param = param;
    Proc_delay(50);

    pid = Proc_get_pid();

    for(;;)
    {
        _sprintf(msg," %-12s  %3d  %2d  %2d  %8ld  %2d%%  %8ld",Proc_get_name(name),
            pid,Proc_get_priority(pid),Proc_get_prio_num(pid),
            Proc_get_run_time(),(int)(100*Proc_get_run_time()/Clk_get_ticks()),i++);
        Con_write_string(0,12,msg);
    }
}

void        User_initial(void)
{
    Proc_create("app1",60,5,app1,0,
        MAKE_STACK(app_stack1,USER_APP_STACK),
        STACK_SIZE(app_stack1,USER_APP_STACK));
    Proc_create("app2",60,1,app2,0,
        MAKE_STACK(app_stack2,USER_APP_STACK),
        STACK_SIZE(app_stack2,USER_APP_STACK));
    Proc_create("app3",60,2,app2,0,
        MAKE_STACK(app_stack3,USER_APP_STACK),
        STACK_SIZE(app_stack3,USER_APP_STACK));
    Proc_create("app4",60,3,app2,0,
        MAKE_STACK(app_stack4,USER_APP_STACK),
        STACK_SIZE(app_stack5,USER_APP_STACK));
    Proc_create("app5",60,4,app2,0,
        MAKE_STACK(app_stack5,USER_APP_STACK),
        STACK_SIZE(app_stack5,USER_APP_STACK));
    Proc_create("app6",60,5,app2,0,
        MAKE_STACK(app_stack6,USER_APP_STACK),
        STACK_SIZE(app_stack6,USER_APP_STACK));
    Proc_create("app7",60,5,app2,0,
        MAKE_STACK(app_stack7,USER_APP_STACK),
        STACK_SIZE(app_stack7,USER_APP_STACK));
}