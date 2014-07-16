
/*
 *  2014.2.17
 */

#include <lenix.h>


#define STACK_DEFAULT_SIZE               2048

/*
 *  n:  Ãû³Æ
 *  l:  ³¤¶È
 */
#define STACK_DECLARE(n,l)              static byte_t n[l]
#define STACK_DEFINE(n)                 STACK_DECLARE(__lenix_proc_stack_##n,\
                                            STACK_DEFAULT_SIZE)
#define STACK_MAKE_DEF(n)               STACK_MAKE(__lenix_proc_stack_##n,\
                                            STACK_DEFAULT_SIZE)

/*
 * n: name
 * p: priority
 * pn: priority number
 * pe: process entry
 * pp: process param
 */
#define PROC_CREATE(n,p,pn,pe,pp)       Proc_create(#n,p,pn,pe,pp,\
                                            STACK_MAKE_DEF(n),STACK_DEFAULT_SIZE)

STACK_DEFINE(test1);
STACK_DEFINE(2);
STACK_DEFINE(3);
STACK_DEFINE(4);

void        Sema_test1(void *param)
{
}

void        Sema_test(void)
{
    handle_t    handle;
    handle = PROC_CREATE(test1,60,5,Sema_test1,NULL);
    Koum_release(handle);
}