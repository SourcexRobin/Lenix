/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenixʵʱ����ϵͳ
//                              2011 - 2012 @ Դ���빤����
//                                     �������а�Ȩ
//                          ***----------------------------***
//       ��  �� : democ.c 
//     ����ʱ�� : 2013-01-01       ������  : �ޱ�
//     �޸�ʱ�� : 2013-01-01       �޸���  : �ޱ�
//
//     ��Ҫ���� : �ṩ��Ϣ������ʾ
//
//      ˵  ��  : 
//
//  �汾�仯��¼:
//  �汾��      |    ʱ  ��     |    ��  ��     | ��Ҫ�仯��¼
//========================================================================================
//  00.00.000   |               |    ��  ��     | ��һ���ʱ���Ѿ����ǵã�����������ļ�
//////////////////////////////////////////////////////////////////////////////////////////
*/
#include <lenix.h>

void        Clk_msg(void);

void        Timer1(void * param)
{
    static long             i           = 0;
    char                    msg[40]     = {0};

    param = param;
    _sprintf(msg,"timer 1 : %ld",i++);

    Con_write_string(40,14,msg,(int)i);
}

void        Timer2(void * param)
{
    static long             i           = 0;
    char                    msg[40]     = {0};

    param = param;
    _sprintf(msg,"timer 2 : %ld",i++);

    Con_write_string(40,12,msg,(int)i);
}


void        User_initial(void)
{
    Clk_ticks_hook_set(Clk_msg);

    Timer_create(300,-1,Timer1,NULL);
    Timer_create(500,-1,Timer2,NULL);
}

void        main(void)
{
    Lenix_initial();
    User_initial();
    Lenix_start();
}

