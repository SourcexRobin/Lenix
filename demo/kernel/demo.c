/*
////////////////////////////////////////////////////////////////////////////////
//                              Lenixʵʱ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: demo.c 
//  ����ʱ��:                   ������: �ޱ�
//  �޸�ʱ��: 2012-12-10        �޸���: �ޱ�
//  ��Ҫ����: �����ں�
//  ˵    ��: 
//
//  �����¼:
//  �� �� ��:   |   ʱ  ��   |   ��  ��     | ��Ҫ�仯��¼
//==============================================================================
//              | 2012-12-10 |   ��  ��     | 
////////////////////////////////////////////////////////////////////////////////
*/
#include <lenix.h>

void        Con_print_char(byte_t c);
void        Clk_msg(void);
void        Shell_cmd_initial(void);

size_t      Pc_com_send(int com,const void * buffer,size_t size);
void        Cpu_initial(void)
{
}
void        User_initial(void)
{
    Tty_echo_hook_set(TTY_MAJOR,Con_print_char);

    Clk_ticks_hook_set(Clk_msg);

    Shell_cmd_initial();
}


void        main(void)
{
    Lenix_initial();

    User_initial();

    Lenix_start();

    for(;;) Cpu_hlt();
}
