
#include <lenix.h>

#define USER_STACK_SIZE             1024

byte_t                      stack1[USER_STACK_SIZE];
byte_t                      stack2[USER_STACK_SIZE];
byte_t                      stack3[USER_STACK_SIZE];


msg_slot_t                  ms_pool[16];
message_t               *   msg;

void        Con_print_char(byte_t c);
void        Clk_msg(void);

void        send1(void * param)
{
    int                     i       = 0;
    message_t           *   msg     = NULL;
    qword_t                 p64     = {0};

    param = param;
    msg = Msg_get("msg");
    
    if( NULL == msg)
    {
        _printf("can not found message box\n");
        return ;
    }
    
    while( i++ < 16 )
    {
        Clk_delay(20);
        if( RESULT_SUCCEED != Msg_send(msg,i*i,i,p64) )
            break;
        _printf("send1 send msg\n");
    }
    _printf("-----------send1 end\n");
}


void        send2(void * param)
{
    int                     i       = 0;
    message_t           *   msg     = NULL;
    qword_t                 p64     = {0};
    param = param;

    msg = Msg_get("msg");
    
    if( NULL == msg)
    {
        _printf("can not found message box\n");
        return;
    }
    
    while( i++ < 16 )
    {
        Clk_delay(30);
        if( RESULT_SUCCEED != Msg_send(msg,i*i,i,p64) )    
            break;
        _printf("send2 send msg\n");
    }
    _printf("----------send2 end\n");
}

void        resv(void * param)
{
    int                     i       = 0;
    msg_slot_t              ms      = {0};

    param = param;
    msg = Msg_create("msg",ms_pool,4);
    
    if( NULL == msg )
    {
        _printf("can not create message\n");
        return ;
    }
    
    Proc_create("send1",60,3,send1,0,
        MAKE_STACK(stack2,USER_STACK_SIZE),
        STACK_SIZE(stack2,USER_STACK_SIZE));
    Proc_create("send2",60,3,send2,0,
        MAKE_STACK(stack3,USER_STACK_SIZE),
        STACK_SIZE(stack3,USER_STACK_SIZE));

    i = 0;
    while( i++ < 8 )  
    {
        Msg_resv(msg,&ms);
        
        _printf("resv msg from pid: %ld msg type:%8ld param32: %8ld\n",
            ms.ms_pid,ms.ms_type,ms.ms_param32);
        
        if( 0 == (i % 3) )
            Clk_delay(1000);
    }
    
    Msg_destroy(msg);
    _printf("resv process end\n");
}

void        User_initial(void)
{
    Tty_echo_hook_set(TTY_MAJOR,Con_print_char);

#ifdef _CFG_DEBUG_

    Clk_ticks_hook_set(Clk_msg);

#endif  /*  _CFG_DEBUG_    */
    Proc_create("resv",59,3,resv,0,
        MAKE_STACK(stack1,USER_STACK_SIZE),
        STACK_SIZE(stack1,USER_STACK_SIZE));
}