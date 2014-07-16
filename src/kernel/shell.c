/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenixʵʱ����ϵͳ
//                              2011 - 2012 @ Դ���빤����
//                                     �������а�Ȩ
//                          ***----------------------------***
//       ��  �� : shell.c 
//     ����ʱ�� : 2011-07-12       ������  : �ޱ�
//     �޸�ʱ�� : 2012-11-24       �޸���  : �ޱ�
//
//     ��Ҫ���� : shell������Ϊ�����ṩ��
//
//      ˵  ��  : 
//
//  �汾�仯��¼:
//  �汾��      |    ʱ  ��     |    ��  ��     | ��Ҫ�仯��¼
//========================================================================================
//              |  2012-12-06   |    ��  ��     | �����������ķ�ʽ������Shell_cmd_add
//                                              | ������
//              |  2012-11-25   |    ��  ��     | ������������������ʶ�𣬺��ĸ��򵥵�
//                                              | �����ʾ��
//  00.00.000   |  2011-07-26   |    ��  ��     | ��һ��
//////////////////////////////////////////////////////////////////////////////////////////
*/
#include <config.h>

#ifdef _CFG_SHELL_ENABLE_

#include <lstring.h>
#include <lio.h>
#include <lmemory.h>
#include <result.h>
#include <assert.h>

#include <proc.h>
#include <shell.h>
#include <tty.h>

#ifdef _SLASH_
    #include <machine/machine.h>
#else
    #include <machine\machine.h>
#endif  /*  _SLASH_ */

#define SHELL_CMD_SIZE              180
#define SHELL_STACK_SIZE            2048

static byte_t               shell_stack[SHELL_STACK_SIZE];          /*  shell����ջ     */
static char                 cmdline[SHELL_CMD_SIZE];                /*  �����л�����    */
static sc_map_t             sc_map_table[SHELL_CMD_MAX] = {0};      /*  ϵͳ�����      */

#ifdef _CFG_SMP_
static spin_lock_t          sc_lock;
#endif  /*  _CFG_SMP_   */

static int  Sc_help         (int ,char **);
static int  Sc_ver          (int ,char **);

/*  2012.11.24 */
static int  Sc_decode_cmdline(char * cmdline,char ** argv)
{
    int                     argc        = 0;
    int                     status      = 0;
    char                    c           = 0;

    for( ; *cmdline && argc < SHELL_ARG_MAX; cmdline++)
    {
        c = *cmdline;
        switch( status )
        {
        case 0:
            if( c == ' ' || c == '\t' )
                break;
            else
            {
                status = 1;
                argv[argc++] = cmdline;
            }
            break;
        case 1:
            if( c == ' ' || c == '\t' )
            {
                *cmdline = 0;
                status = 0;
            }
            break;
        }

    }
    return argc;
}

/*  2012.11.24 */
sc_entry_t  Sc_name_to_cmd(const char * cmdname)
{
    const sc_map_t   *      scm = sc_map_table;
    int                     i   = 0;
    CRITICAL_DECLARE(sc_lock);

    CRITICAL_BEGIN();
    for( ; i < SHELL_CMD_MAX ; i++, scm++)
    {
        /*
        if( sc->sc_entry )
            _printf("%d %s\n",i,sc->sc_name);
         */

        if( scm->scm_entry && _namecmp(cmdname,scm->scm_name ) == 0 )
        {
            CRITICAL_END();
            return scm->scm_entry;
        }
    }
    CRITICAL_END();

    return NULL;
}


/*  2012.11.25 ���Ҳ��� */
char *      Sc_get_param(int argc,char ** argv,const char param)
{
    int                     i       = 0;

    for( ; i < argc ; i++)
    {
        if( argv[i][0] == '-' || '/' == argv[i][0] )
        {
            if( param == argv[i][1] )
                return argv[i] + 2;
        }
    }

    return NULL;
}


static void Lenix_shell(void * param)
{
    int                     len         = 0;
    int                     argc        = 0;
    char                *   argv[16]    = { NULL };
    sc_entry_t              entry       = NULL;

    param = param;

    for(;;)
    {
        _printf("\nLenix: >");                                /*  ��ʾ������ʾ��        */
        len = Tty_read(TTY_MAJOR,cmdline,SHELL_CMD_SIZE - 1); /*  ����������            */
        cmdline[len] = 0;                                     /*  ȷ��������ĩβ��0��β */

#ifdef _CFG_DEBUG_
        /*
         *  �������鿴��������
         */
        _printf("\n");
        _printf(cmdline);
#endif  /*  _CFG_DEBUG_ */

        /*
         *  ����������������
         */
        argc = Sc_decode_cmdline(cmdline,argv);

        if( argc )      /*  ��������ų���ִ��   */
        {
            /*
             *  �����Ƿ��������
             */
            if( entry  = Sc_name_to_cmd(argv[0]) )
            {
                _printf("\n");
                entry(argc,argv);
            }
            else
                _printf("\nbad or unkown command\n");
        }
    }
}

result_t    Shell_cmd_add(const char * cmdname,sc_entry_t cmdentry)
{
    sc_map_t            *   scm     = sc_map_table,
                        *   scm_f   = NULL;         /*  �����������  */
    result_t                result  = RESULT_SUCCEED;
    int                     i       = 0;
    CRITICAL_DECLARE(sc_lock);

    CRITICAL_BEGIN();
    for( ; i < SHELL_CMD_MAX ; i++,scm++ )
    {
        if( NULL == scm_f && NULL == scm->scm_entry )
        {
            scm_f           = scm;
            scm->scm_entry  = cmdentry;
        }

        /*
         *  ��֤ϵͳ�����������
         */
        if( _namecmp(scm->scm_name,cmdname) == 0 )
        {
            scm_f->scm_entry    = NULL;
            scm_f               = NULL;
            result              = RESULT_FAILED;
            break;
        }
    }

    if( scm_f )
        _nstrcpy(scm_f->scm_name,cmdname,SHELL_CMD_NAME_SIZE);

    CRITICAL_END();

    return result;
}

void        Shell_initial(void)
{
    handle_t        handle  = INVALID_HANDLE;
    _printk("shell initial...    ");
    _memzero(sc_map_table,sizeof(sc_map_t)*SHELL_CMD_MAX);
#ifdef _CFG_SMP_
    sc_lock = 0;
#endif  /*  _CFG_SMP_   */

    Shell_cmd_add("help",Sc_help);
    Shell_cmd_add("ver" ,Sc_ver );

    handle = Proc_create("LeSH",
        SHELL_PROCESS_PRIORITY,SHELL_PROCESS_PRIONUM,
        Lenix_shell,0,
        STACK_MAKE(shell_stack,SHELL_STACK_SIZE),
        STACK_SIZE(shell_stack,SHELL_STACK_SIZE));

    if( INVALID_HANDLE == handle )
        _printk("FAILED!\n");
    else
    {
        Koum_release(handle);
        _printk("OK!\n");
    }
}


/*  2012.11.25 */
static int  Sc_help         (int argc,char ** argv)
{
    sc_map_t            *   scm     = sc_map_table;
    int                     i       = 0;
    CRITICAL_DECLARE(sc_lock);

    _printf("System command:\n");

    CRITICAL_BEGIN();

    for( ; i < SHELL_CMD_MAX ; i++,scm++ )
    {
        if( scm->scm_entry )
            _printf("    %S\n",scm->scm_name);
    }

    CRITICAL_END();

    argc = argc;
    argv = argv;

    return 0;
}

/*  2012.11.25 */
static int  Sc_ver          (int argc,char ** argv)
{
    argc = argc;
    argv = argv;

    _printf("Lenix ver: %d.%02d\n",VER_MAJOR,VER_MINOR);
    return 0;
}


#endif  /*  _CFG_SHELL_ENABLE_   */