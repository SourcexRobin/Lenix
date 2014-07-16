/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenixʵʱ����ϵͳ
//                              2011 - 2012 @ Դ���빤����
//                                     �������а�Ȩ
//                          ***----------------------------***
//       ��  �� : lenix.c 
//     ����ʱ�� :                  ������  : �ޱ�
//     �޸�ʱ�� : 2012-11-27       �޸���  : �ޱ�
//
//     ��Ҫ���� : �ṩ���ַ��ն˵�����������ܣ���ΪLenixĬ��֧�ֵ�Ӳ���豸
//
//      ˵  ��  : 
//
//  �汾�仯��¼:
//  �汾��      |    ʱ  ��     |    ��  ��     | ��Ҫ�仯��¼
//========================================================================================
//              |  2012-11-27   |    ��  ��     | ���ӹ�������
//              |  2012-09-23   |    ��  ��     | ���ļ�����main.c��Ϊlenix.c
//  00.00.000   |               |    ��  ��     | ��һ���ʱ���Ѿ����ǵã�����������ļ�
//////////////////////////////////////////////////////////////////////////////////////////
*/


#include <lenix.h>

/*  2012.12.08  */
void        Lenix_initial(void)
{
    Disable_interrupt();
    ++critical_nest;
    
    Con_cls();

    Cpu_initial();

    //Mmu_initial();

    Machine_initial();

    Proc_initial();

    Clk_initial();

#ifdef _CFG_MUTEX_ENABLE_
    Mutex_initial();
#endif /*   _CFG_MUTEX_ENABLE_  */

#ifdef _CFG_MESSAGE_ENABLE_
    Msg_initial();
#endif  /*  _CFG_MESSAGE_ENABLE_*/

#ifdef _CFG_TTY_ENABLE_
    Tty_initial();
#endif  /*  _CFG_TTY_ENABLE_    */

#ifdef _CFG_DEVICE_ENABLE_
    Dev_initial();
#endif  /*  _CFG_DEVICE_ENABLE_ */

#ifdef _CFG_SHELL_ENABLE_
    Shell_initial();
#endif  /*  _CFG_SHELL_ENABLE_  */
    _printk("Lenix initial OK!\n");
}

/*  2012.12.08  */
void        Lenix_start(void)
{
    --critical_nest;
    ASSERT(critical_nest == 0);
    
    Lenix_start_hook();

    _printk("Lenix start...\n");

    Enable_interrupt();

    PROC_NEED_SCHED();
}

