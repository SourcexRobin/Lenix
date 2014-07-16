/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      Lenix
//                                                  Ƕ��ʽ����ϵͳ
//                                           2011 , ��  ��Դ���빤����
//                                                   �������а�Ȩ
//
//  �ļ�����    : device1.c
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
#include <lenix.h>

#define USER_APP_STACK              1024

result_t    Vga_entry(device_t * device,int flag,void * param);

byte_t                      app_stack1[USER_APP_STACK];
byte_t                      app_stack2[USER_APP_STACK];

void        Con_print_char(byte_t c);
void        Clk_msg(void);

void        app1(void * param)
{
    long                    i = 0;
    char                    msg[32];
    device_t            *   vga;
    param = param;

    if( NULL == (vga = Dev_open("vga",0)))
        _printf("can not open vga\n");
    else
    {
        Dev_write(vga,80,"aabb",4);
        Dev_close(vga);
    }

    Dev_unregiste("VGA",NULL);
}


void        User_initial(void)
{
    Tty_echo_hook_set(TTY_MAJOR,Con_print_char);

#ifdef _CFG_DEBUG_

    Clk_ticks_hook_set(Clk_msg);

#endif  /*  _CFG_DEBUG_    */

    if( RESULT_SUCCEED == Dev_registe("VGA",Vga_entry,NULL) )
    {
        _printf("vga registe OK\n");
        Proc_create("app1",60,3,app1,0,
            MAKE_STACK(app_stack1,USER_APP_STACK),
            STACK_SIZE(app_stack1,USER_APP_STACK));
    }
    else
        _printf("vga registe failed\n");
}