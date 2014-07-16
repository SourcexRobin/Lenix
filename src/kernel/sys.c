/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenixʵʱ����ϵͳ
//                              2011 - 2012 @ Դ���빤����
//                                     �������а�Ȩ
//                          ***----------------------------***
//       ��  �� : sys.c
//     ����ʱ�� :                  ������  : �ޱ�
//     �޸�ʱ�� : 2012-11-29       �޸���  : �ޱ�
//
//  ��Ҫ����    : �ṩϵͳ���ù���
//
//  ˵��        :
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
//========================================================================================
//  00.00.000   |               |  �ޱ�         |  ��һ�治�ǵ�ʱ����
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
#include <config.h>
#include <lio.h>

#include <sys.h>
#include <proc.h>
#include <clock.h>

#ifdef _SLASH_


#else
    #include <machine\machine.h>
#endif  /*  _SLASH_ */

void *   syscall[SYSCALL_MAX] = {
    0
};


/*  ��ʾ��ʾ��Ϣ������  */
void        Sys_halt(const char * msg)
{
    PROC_INC_SEIZE();

#ifdef _CFG_TTY_ENABLE_
    _printf("Lenix halt. hit: %s",msg);
#else
    msg = msg;
#endif

    Disable_interrupt();

    for(;;) Cpu_hlt();    
}

void        Syscall_exit(int type,int refresh)
{
    /*
     *  ϵͳ���ú�Ҫ�����źŴ���
     */
#ifdef _CFG_SIGNAL_ENABLE_
    Signal_handle();
#endif  /*  _CFG_SIGNAL_ENABLE_ */

    /*
     *  �����˳�����ִ�в�ͬ�Ĳ���
     */
    if( SCEXIT_TYPE_IRQ == type )
    {
        /*
         *  �ж��˳���ִ�е���
         *  ��Ҫ���ȣ�������ռ�������жϴ����ڣ����ܵ���
         */
        if( proc_need_sched ) 
            Proc_sched(refresh);
    }
}

void        Syscall_initial(void)
{
}