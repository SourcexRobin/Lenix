
#include <stdio.h>
#include <lenix.h>

#define STACK_SIZE      1024

byte_t  stack1[STACK_SIZE];
byte_t  stack2[STACK_SIZE];
byte_t  stack3[STACK_SIZE];


msg_slot_t                  ms_pool[16];
message_t               *   msg;


int     send1(void)
{
    int    i;
    
    message_t   *   msg;
    qword_t         p64;
    msg = Msg_get("msg");
    
    if( NULL == msg)
    {
        printf("can not found message box\n");
        return 1;
    }
    
    i = 0;
    
    while( i < 16 )
    {
        Clk_delay(20);
        if( RESULT_SUCCEED != Msg_send(msg,i*i,i,p64) )
            break;
        printf("send1 send msg\n");
        i++;
    }
    return 0;
}


int     send2(void)
{
    int    i;
    
    message_t   *   msg;
    qword_t         p64;

    msg = Msg_get("msg");
    
    if( NULL == msg)
    {
        printf("can not found message box\n");
        return 1;
    }
    
    i = 0;
    
    while( i < 16 )
    {
        Clk_delay(30);
        if( RESULT_SUCCEED != Msg_send(msg,i*i,i,p64) )    
            break;
        printf("send2 send msg\n");
        i++;
    }
    
    return 0;
}

int     resv(void)
{
    int i;
    msg_slot_t      ms;
    msg = Msg_create("msg",ms_pool,4);
    
    if( NULL == msg )
    {
        printf("can not create message\n");
        return -1;
    }
    
    Proc_create("send1",send1,0,MAKE_STACK(stack2,STACK_SIZE));
    Proc_create("send2",send2,0,MAKE_STACK(stack3,STACK_SIZE));

    i = 0;
    while( i < 8 )  
    {
        Msg_resv(msg,&ms);
        
        printf("resv data.from pid: %ld msg type: %8ld param32: %8ld\n",
            ms.ms_pid,ms.ms_type,ms.ms_param32);
        
        if( 0 == (i % 3) )
            Clk_delay(100);
        
        i++;
    }
    
    Msg_destroy(msg);
    printf("resv process end\n");
    return 0;
}
