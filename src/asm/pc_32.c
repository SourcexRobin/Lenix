/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: pc_32.c
//  ����ʱ��: 2014-01-31       ������  : �ޱ�
//  �޸�ʱ��:                  �޸���  : 
//  ��Ҫ����: �ṩ���ַ��ն˵�����������ܣ���ΪLenixĬ��֧�ֵ�Ӳ���豸
//  ˵    ��: 
//
//  �汾�仯��¼:
//  �汾��      |    ʱ  ��     |    ��  ��     | ��Ҫ�仯��¼
//=============================================================================
//              |  2014-01-31   |    ��  ��     | ���ӹ�������
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>
#include <type.h>
#include <assert.h>
#include <lio.h>
#include <machine\machine.h>
#include <sys.h>
#include <clock.h>
#include <arch\x86.h>

extern volatile uint32_t    ticks;
extern volatile uint_t      e0e1;   /*  ��չ�ֽڱ�־    */
extern byte_t               interrupt_nest;
extern isp_t                machine_ivt[IRQ_SRC_MAX];       /*  �ж�������  */

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Irq_clock
//  ��  ��: 32λX86��ʱ���жϴ�������
//  ��  ��: ��
//  ����ֵ: ��
//  ע  ��: �ж�Ƕ�׳���196���Ժ��жϻᶪʧ
//
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
//=============================================================================
//  2014-02-01  |  ��  ��       |  
///////////////////////////////////////////////////////////////////////////////
*/
NAKED
void        Irq_clock(void)
{
    I386_ISP_ENTER();
    ++ticks;
    ++interrupt_nest;                   /*  �����ж�Ƕ�׼���        */
    Io_outb((void *)0x20,0x20);         /*  ���¿�����8259A         */
    if( interrupt_nest >= IRQ_NEST_MAX) /*  �ж�Ƕ�׳���196����   */
        Machine_interrupt_mis();        /*  ����ж�                */
    else
        machine_ivt[0](0,0);            /*  �����ж�                */
    --interrupt_nest;                   /*  �ݼ��ж�Ƕ�׼���        */
    Syscall_exit(SCEXIT_TYPE_IRQ,1);    /*  �ж��˳�ǰ��Ҫ����      */
    I386_ISP_LEAVE();
}


/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: _Irq_keyboard
//  ��  ��: 32λX86�ļ����жϴ�������
//  ��  ��: ��
//  ����ֵ: ��
//  ע  ��: Ϊ��ʹ�þֲ����������������������__declspec( naked )�޶��������£�
//          ����ʹ�þֲ�������
//          �ж�Ƕ�׳���IRQ_NEST_MAX���Ժ��жϻᶪʧ
//
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
//=============================================================================
//  2014-02-02  |  ��  ��       |  
///////////////////////////////////////////////////////////////////////////////
*/
static
void        _Irq_keyboard(void)
{
    byte_t                  sc = 0;     /*  ϵͳɨ����,scan code    */
    
    sc = Io_inb((void*)0x60);
    Io_outb((void *)0x20,0x20);         /*  ���¿�����8259A         */
    if( interrupt_nest >= IRQ_NEST_MAX )
    {
        Machine_interrupt_mis();
        return ;
    }
    switch(sc)
    {
    case 0xE0:  e0e1 |= 1;  break;
    case 0xE1:  e0e1 |= 2;  break;
    default:    machine_ivt[1](0,sc);
    }
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Irq_keyboard
//  ��  ��: 32λX86�ļ����жϴ�������
//  ��  ��: ��
//  ����ֵ: ��
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
//=============================================================================
//  2014-02-02  |  ��  ��       |  
///////////////////////////////////////////////////////////////////////////////
*/
NAKED
void        Irq_keyboard(void)
{
    I386_ISP_ENTER();
    ++interrupt_nest;                   /*  �����ж�Ƕ�׼���        */
    _Irq_keyboard();
    --interrupt_nest;                   /*  �ݼ��ж�Ƕ�׼���        */
    Syscall_exit(SCEXIT_TYPE_IRQ,0);    /*  �ж��˳�ǰ��Ҫ����      */
    I386_ISP_LEAVE();
}

NAKED
void        Irq_com1(void)
{
    __asm iretd
}
