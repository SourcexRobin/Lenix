
/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenixʵʱ����ϵͳ
//                              2011 - 2012 @ Դ���빤����
//                                     �������а�Ȩ
//                          ***----------------------------***
//       ��  �� : demo.c 
//     ����ʱ�� :                  ������  : �ޱ�
//     �޸�ʱ�� : 2012-11-27       �޸���  : �ޱ�
//
//     ��Ҫ���� : ��ʾ��ѧ���ò�����
//
//      ˵  ��  : 
//
//  �汾�仯��¼:
//  �汾��      |    ʱ  ��     |    ��  ��     | ��Ҫ�仯��¼
//========================================================================================
//              |  2012-12-15   |    ��  ��     | ���ӹ�������
//////////////////////////////////////////////////////////////////////////////////////////
*/

#include <lenix.h>

#define USER_APP_STACK              2048
#define N                           5
#define EAT_TIME                    2000
#define WAIT_TIME                   600
#define WAIT_FORK_TIME              500

byte_t                      app_stack1[USER_APP_STACK];
byte_t                      app_stack2[USER_APP_STACK];
byte_t                      app_stack3[USER_APP_STACK];
byte_t                      app_stack4[USER_APP_STACK];
byte_t                      app_stack5[USER_APP_STACK];

spin_lock_t                 fork_lock[N]    = {0,0,0,0,0};

void        Clk_msg(void);

int         Fork_get(int forkidx,int timeout)
{
    if( Cpu_tas(fork_lock + forkidx,0,1) == 0)
        return 1;
    Proc_delay(timeout);
    if( Cpu_tas(fork_lock + forkidx,0,1) == 0)
        return 1;
    return 0;
}

void        Fork_put(int forkidx)
{
    fork_lock[forkidx] = 0;
}

void        PhD(void * param)
{
    int                     i           = 0,    /*  �òʹ���          */
                            right       = 0,    /*  �ұ߲��ӵı��    */
                            left        = 0,    /*  ��߲��ӵı��    */
                            x           = 0,    /*  ��Ϣ��ʾx����     */
                            y           = 0,    /*  ��Ϣ��ʾy����     */
                            r_try       = 0,    /*  �ұ߳��Դ���      */
                            l_try       = 0;    /*  ��߳��Դ���      */

    char                    msg[80]     = {0};

    right   = (int)param;
    left    = (right + 1) % N;
    y       = right * 3 + 2;

    _sprintf(msg,"PhD.%d:",right);
    Con_write_string(x,y,msg);

    for(; i < 5 ; i++)
    {
        l_try = 0;
        r_try = 0;
        while(1)
        {
            /*  ����ȡ�ұߵĲ���  */
            if( Fork_get(right,WAIT_FORK_TIME) == 0 )
            {
                /*  δ�ܻ���ұߵĲ��ӣ��ȴ�һ����ʱ������³���  */
                Proc_delay(WAIT_TIME);
                r_try++;
                continue;
            }
            /*  ����ȡ��ߵĲ���  */
            if( Fork_get(left,WAIT_FORK_TIME) == 0 )
            {
                /*  δ�ܻ���ұߵĲ��ӣ��ȴ�һ����ʱ������³���  */
                Fork_put(right);
                Proc_delay(WAIT_TIME);
                l_try++;
                continue;
            }

            /*  �ò���Ҫʱ�䣬�ý�����ʱ����  */
            x  = i * 12 + 8;
            _sprintf(msg,"etms:%ld",Clk_get_ticks());
            Con_write_string(x,y,msg);
            _sprintf(msg,"r_try: %d",r_try);
            Con_write_string(x,y+1,msg);
            _sprintf(msg,"l_try: %d",l_try);
            Con_write_string(x,y+2,msg);

            Proc_delay(EAT_TIME);

            Fork_put(left);
            Fork_put(right);
            break;
        }

        Proc_delay(WAIT_TIME);
    }
    
    Con_write_string(68,y,"eat OK");
}

void        User_initial(void)
{
    Clk_ticks_hook_set(Clk_msg);

    Proc_create("PhD.0",60,5,PhD,0,
        MAKE_STACK(app_stack1,USER_APP_STACK),
        STACK_SIZE(app_stack1,USER_APP_STACK));

    Proc_create("PhD.1",60,5,PhD,(void *)1,
        MAKE_STACK(app_stack2,USER_APP_STACK),
        STACK_SIZE(app_stack2,USER_APP_STACK));

    Proc_create("PhD.2",60,5,PhD,(void *)2,
        MAKE_STACK(app_stack3,USER_APP_STACK),
        STACK_SIZE(app_stack3,USER_APP_STACK));

    Proc_create("PhD.3",60,5,PhD,(void *)3,
        MAKE_STACK(app_stack4,USER_APP_STACK),
        STACK_SIZE(app_stack4,USER_APP_STACK));

    Proc_create("PhD.4",60,5,PhD,(void *)4,
        MAKE_STACK(app_stack5,USER_APP_STACK),
        STACK_SIZE(app_stack5,USER_APP_STACK));
}

int         main(void)
{
    Lenix_initial();

    User_initial();

    Lenix_start();

    return 1;
}
