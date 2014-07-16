/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: bb_test.c  
//  ����ʱ��: 2014-03-09        ������: �ޱ�
//  �޸�ʱ��:                   �޸���: 
//  ��Ҫ����: �黺�����
//  ˵    ��: 
//
//  �����¼:
//  �� �� ��    |     ʱ��      |  ��  ��       |  ��Ҫ�仯��¼
//=============================================================================
//  00.00.000   |  2014-03-09   |  ��  ��       |  �����ļ�
///////////////////////////////////////////////////////////////////////////////
*/

#include <lenix.h>
#include <blkbuf.h>

STACK_DEFINE(btest1);
STACK_DEFINE(btest2);
STACK_DEFINE(btest3);

void        Con_print_char(byte_t c);
void        Clk_msg(void);
void        Shell_cmd_initial(void);

void        Bbuf_test(void * param)
{
    device_t * dev;
    blkbuf_t *  blkbuf;
    offset_t    addr = 0;

    if( NULL == ( dev = Dev_open("ata",DEV_IO_RDWR) ))
    {
        _printf("ata open failed!\n");
        return ;
    }

    while( addr < 5120)
    {
        if( NULL == ( blkbuf = Bbuf_read(dev,addr++) ) )
        {
            _printf("block buffer read failed!\n");
            return ;
        }
        BBUF_LOCK(blkbuf);
        _mprintf(blkbuf->bb_buffer,16);
        BBUF_FREE(blkbuf);
        Bbuf_release(blkbuf);
    }
}

void        User_initial(void)
{
    int         hd = 0;

    Tty_echo_hook_set(TTY_MAJOR,Con_print_char);

    Clk_ticks_hook_set(Clk_msg);

    Shell_cmd_initial();

    Bbuf_initial();

     if( Dev_registe("ata",Ata_entry,&hd) != RESULT_SUCCEED )
        _printf("ata register failed!\n");

    PROC_CREATE(btest1,60,3,Bbuf_test,NULL);
    PROC_CREATE(btest2,60,3,Bbuf_test,NULL);
    PROC_CREATE(btest3,60,3,Bbuf_test,NULL);

}

void        main(void)
{
    User_initial();
}

