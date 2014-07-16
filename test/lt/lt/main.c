
#include "e:\sourcexstudio\lenix\lenix\demo\kernel32\koum.h"

typedef struct _t{
    koh_t       koh;
    int         i;
}t;
t i;
int re(t * pt)
{
    return 0;
}

int main()
{
    uint_t hnd;
    i.koh.koh_type = kot_proc;
    Koum_add(&i,re,kot_proc,HANDLE_ATTR_RDWR);
    hnd = Koum_add(&i,re,kot_proc,HANDLE_ATTR_RDWR);
    Koum_release(hnd);
    Koum_add(&i,re,kot_proc,HANDLE_ATTR_RDWR);
    Koum_add(&i,re,kot_proc,HANDLE_ATTR_RDWR);

   return ;
}