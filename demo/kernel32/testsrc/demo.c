/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenix实时操作系统
//                              2011 - 2012 @ 源代码工作室
//                                     保留所有版权
//                          ***----------------------------***
//       名  称 : demo.c 
//     创建时间 :                  创建者  : 罗斌
//     修改时间 : 2012-12-26       修改者  : 罗斌
//
//     主要功能 : 提供消息机制演示
//
//      说  明  : 
//
//  版本变化记录:
//  版本号      |    时  间     |    作  者     | 主要变化记录
//========================================================================================
//  00.00.000   |               |    罗  斌     | 第一版的时间已经不记得，这是最早的文件
//////////////////////////////////////////////////////////////////////////////////////////
*/
#include <lenix.h>

#define USER_STACK_SIZE             2048
#define N                           5
#define TIMES                       10

byte_t                      stack1[USER_STACK_SIZE];
byte_t                      stack2[USER_STACK_SIZE];
byte_t                      stack3[USER_STACK_SIZE];


message_t                   ms_pool[N];
handle_t                    msg;

void        Con_print_char(byte_t c);
void        Clk_msg(void);

void        send1(void * param)
{
    int                     i       = 0;
    handle_t                msg     = INVALID_HANDLE;
    qword_t                 p64     = {0};

    param = param;
    msg = Msg_get("msg");
    
    if( INVALID_HANDLE == msg)
    {
        _printf("can not found message box\n");
        return ;
    }
    
    i = 0;
    
    _printf("+++ send1 begin \n");
    while( i < TIMES )
    {
        Clk_delay(20);
        if( RESULT_SUCCEED != Msg_send(msg,i*i,i,p64) )
        {
            _printf(" + send1 send message failed\n");
            break;
        }
        _printf("send1 send msg\n");
        i++;
    }
    _printf("+++ send1 end\n");
    Koum_release(msg);
}


void        send2(void * param)
{
    int                     i       = 0;
    handle_t                msg     = INVALID_HANDLE;
    qword_t                 p64     = {0};

    param = param;
    msg = Msg_get("msg");
    
    if( INVALID_HANDLE == msg)
    {
        _printf("can not found message box\n");
        return ;
    }
    
    i = 0;
    _printf("--- send2 begin\n");
    
    while( i < TIMES )
    {
        Clk_delay(30);
        if( RESULT_SUCCEED != Msg_send(msg,i*i,i,p64) )    
        {
            _printf(" - send1 send message failed\n");
            break;
        }
        _printf("send2 send msg\n");
        i++;
    }
    _printf("--- send2 end\n");
    Koum_release(msg);
}

void        resv(void * param)
{
    int                     i   = 0;
    message_t              ms  = {0};
    param = param;
    msg = Msg_create("msg",ms_pool,4);
    
    if( INVALID_HANDLE == msg )
    {
        _printf("can not create message\n");
        return ;
    }
    _printf("message demo:\n");
    
    Proc_create("send1",60,3,send1,0,
        STACK_MAKE(stack2,USER_STACK_SIZE),
        STACK_SIZE(stack2,USER_STACK_SIZE));
    Proc_create("send2",60,3,send2,0,
        STACK_MAKE(stack3,USER_STACK_SIZE),
        STACK_SIZE(stack3,USER_STACK_SIZE));

    i = 0;
    while( i < 6 )  
    {
        Msg_resv(msg,&ms);
        
        _printf("resv data.from pid: %d msg type: %d param32: %8d\n",
            ms.msg_pid,ms.msg_type,ms.msg_param32);
        
        if( 0 == (i % 3) )
            Clk_delay(100);
        
        i++;
    }
    
    Handle_release(msg);
    _printf("resv process end\n");
}

void        User_initial(void)
{
    Tty_echo_hook_set(TTY_MAJOR,Con_print_char);

    Clk_ticks_hook_set(Clk_msg);

    Proc_create("resv",60,3,resv,0,
        STACK_MAKE(stack1,USER_STACK_SIZE),
        STACK_SIZE(stack1,USER_STACK_SIZE));

}

void        main(void)
{
    User_initial();
}

