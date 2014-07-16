
// proc1.c
#include <stdio.h>
#include <lenix.h>

#define USER_APP_STACK      1024

byte_t  app_stack1[USER_APP_STACK];
byte_t  app_stack2[USER_APP_STACK];

void     app1(void * param)
{
    long i = 0;
    char    msg[32];
    param = param;
    Con_print_string("app1 test\n");
    for(;;)
    {
        sprintf(msg,"app1.%8ld \n",i++);
        Con_write_string(60,5,msg);

    }
}

void     app2(void * param)
{
    long i = 0;
    char    msg[32];
    param = param;
    for(;;)
    {
        sprintf(msg,"app2.%8ld ",i++);
        Con_write_string(60,6,msg);
    }
}

void        User_initial(void)
{
    Proc_create("app1",60,5,app1,0,
        MAKE_STACK(app_stack1,USER_APP_STACK),
        STACK_SIZE(app_stack1,USER_APP_STACK));
    Proc_create("app2",60,1,app2,0,
        MAKE_STACK(app_stack2,USER_APP_STACK),
        STACK_SIZE(app_stack2,USER_APP_STACK));
}