/*  文件时间 2012.12.06   */

#include <lenix.h>

static int  Sc_mem          (int ,char **);
static int  Sc_cls          (int ,char **);

void        Shell_cmd_initial(void)
{
    Shell_cmd_add("mem",Sc_mem);
    Shell_cmd_add("cls",Sc_cls);
}
/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  以下为内部命令
*/



/*  2012.11.25 */
static int  Sc_mem          (int argc,char ** argv)
{
    const char          *   param   = NULL;

    param = Sc_get_param(argc,argv,'m');

    if( param )
    {
        _printf("address: ");
        _printf(param);
        _printf("\n");
    }

    param = Sc_get_param(argc,argv,'l');

    if( param )
    {
        _printf("length: ");
        _printf(param);
        _printf("\n");
    }

    return 0;
}

/*  2012.11.25 */
static int  Sc_cls          (int argc,char ** argv)
{
    Con_cls();

    argc = argc;
    argv = argv;

    return 0;
}

