/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: bb_test.c  
//  创建时间: 2014-03-09        创建者: 罗斌
//  修改时间:                   修改者: 
//  主要功能: 块缓存测试
//  说    明: 
//
//  变更记录:
//  版 本 号    |     时间      |  作  者       |  主要变化记录
//=============================================================================
//  00.00.000   |  2014-03-09   |  罗  斌       |  建立文件
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

