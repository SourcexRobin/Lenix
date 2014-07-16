/*
//////////////////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: demo.c 
//  创建时间:                   创建者: 罗斌
//  修改时间: 2012-12-10        修改者: 罗斌
//
//  主要功能: 提供SHELL的演示程序
//
//  说    明: 
//
//  变更记录:
//  版本号      |   时  间   |   作  者     | 主要变化记录
//========================================================================================
//              | 2012-12-10 |   罗  斌     | 
//////////////////////////////////////////////////////////////////////////////////////////
*/
#include <lenix.h>

#define APP_STACK_SIZE              1024

byte_t                      stack_1[APP_STACK_SIZE];
byte_t                      stack_2[APP_STACK_SIZE];
byte_t                      stack_3[APP_STACK_SIZE];
byte_t                      stack_4[APP_STACK_SIZE];
byte_t                      stack_5[APP_STACK_SIZE];

void        Con_print_char(byte_t c);
void        Clk_msg(void);
void        Shell_cmd_initial(void);

size_t      Pc_com_send(int com,const void * buffer,size_t size);

void        Pci_initial(void);

/*
 *  测试Proc_killAPI 
 */
void        Proc_kill_test(void * param)
{
    int             i       = 0;
    char            str[16] = "Kill test";
    while(1)
    {
        Con_write_string(5,8,str,++i);
    }
}
void        Proc_kill_test1(void * param)
{
    Proc_delay(2000);
    Proc_kill((handle_t)param);
}

int i = 0;
handle_t mutex;
/*
 *  测试互斥
 */
void        Test_mutex1(void * param)
{
    while(1)
    {
        if( Mutex_get(mutex) == RESULT_SUCCEED )
        {
            ASSERT( i == 0);
            ++i;
            _printk("mutex1 %d\n",i);
            --i;
            Mutex_put(mutex);
        }
        else
            break;
    }
    _printk("mutex1 test end!\n");
}

void        Test_mutex2(void * param)
{
    while(1)
    {
        if( Mutex_get(mutex) == RESULT_SUCCEED )
        {
            ASSERT( i == 0);
            ++i;
            _printk("\tmutexd2 %d\n",i);
            --i;
            Mutex_put(mutex);
        }
        else
            break;
    }
    _printk("\tmutex2 test end!\n");
}
void        Test_mutex3(void * param)
{
    while(1)
    {
        if( Mutex_get(mutex) == RESULT_SUCCEED )
        {
            ASSERT( i == 0);
            ++i;
            _printk("\t\tmutexd3 %d\n",i);
            --i;
            Mutex_put(mutex);
        }
        else
            break;
    }
    _printk("\tmutex3 test end!\n");
}

void        Test(void)
{
    handle_t        handle  = INVALID_HANDLE;
    Pci_initial();

    handle = Proc_create("kill 1",60,5,Proc_kill_test,NULL,
        STACK_MAKE(stack_1,APP_STACK_SIZE),
        STACK_SIZE(stack_1,APP_STACK_SIZE));

    Proc_create("kill 2",60,5,Proc_kill_test1,(void *)handle,
        STACK_MAKE(stack_2,APP_STACK_SIZE),
        STACK_SIZE(stack_2,APP_STACK_SIZE));

    mutex = Mutex_create();
    if( INVALID_HANDLE == mutex)
        _printk("mutex create failed!\n");
    else
    {
        _printk("mutex create OK!\n");
        handle = Proc_create("mtx1",60,5,Test_mutex1,NULL,
            STACK_MAKE(stack_3,APP_STACK_SIZE),
            STACK_SIZE(stack_3,APP_STACK_SIZE));
        Koum_release(handle);
        handle = Proc_create("mtx2",60,5,Test_mutex2,NULL,
            STACK_MAKE(stack_4,APP_STACK_SIZE),
            STACK_SIZE(stack_4,APP_STACK_SIZE));
        Koum_release(handle);
        handle = Proc_create("mtx3",60,5,Test_mutex3,NULL,
            STACK_MAKE(stack_5,APP_STACK_SIZE),
            STACK_SIZE(stack_5,APP_STACK_SIZE));
        Koum_release(handle);
    }
}

static byte_t  buf[512];
//2014.2.21  硬盘测试
void        Ata_test(void)
{
    device_t      * dev = NULL;
    int             id  = 0;
    
    _printk("ATA test\n");
    if( Dev_registe("harddisk0",Ata_entry,&id) != RESULT_SUCCEED )
    {
        _printk("ata registe failed!\n");
        return ;
    }
    else
        _printk("ata capacity: %d\n",(dword_t)dev->dev_capcity);
    if( NULL == ( dev = Dev_open("harddisk0",DEV_IO_RDWR) ) )
    {
        _printk("ata open failed!\n");
        return ;
    }

    if( Dev_read(dev,0,buf,512) < 1 )
    {
        _printk("ata read error!\n");
        return ;
    }
    _printk("sinature: %04X\n",*(word_t *)(buf + 510));
}
void        main(void)
{

    Tty_echo_hook_set(TTY_MAJOR,Con_print_char);

    Clk_ticks_hook_set(Clk_msg);

    Shell_cmd_initial();

    Ata_test();
}


