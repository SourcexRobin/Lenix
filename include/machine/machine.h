/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: machine.c
//  ����ʱ��: 2011-02-09        ������: �ޱ�
//  �޸�ʱ��: 2014-03-11        �޸���: �ޱ�
//  ��Ҫ����: �ṩ��Ŀ������Ľӿڡ�
//  ˵    ��: 1�� LenixĬ���ṩ16���жϽӿڣ���Ԥ�����˼���Ӳ���жϵı�ţ�Ҳ��
//            �Ǽ���Ŀ������߱���ЩӲ��
//            2�� �ṩ�˹رջ����жϿ������ӿ�
//            3�� �ṩ���޸�ϵͳʱ��Ƶ�ʽӿ�
//            Ҳ����Ŀ���������߱��жϿ�������ʱ��
//
//  �����¼:
//  �� �� ��    |    ʱ   ��    |   ��  ��      |   ��Ҫ�仯��¼
//=============================================================================
//              |   2014-03-11  |   ��  ��      | ���ӻ�ȡ�����ڴ�����
//  00.00.000   |   2011-02-09  |   ��  ��      | �����ļ�
///////////////////////////////////////////////////////////////////////////////
*/

#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <config.h>

#ifdef _CFG_MACHINE_PC_

#ifdef _SLASH_
    #include <machine/pc.h>
#else
    #include <machine\pc.h>
#endif  /*  _SLASH_ */

#endif  /*  _CFG_MACHINE_PC_    */

typedef isp_t               isr_t;

/*  LenixĬ���ṩ16���ж����   */
#define IVT_MAX                     16
#define IRQ_SRC_MAX					16
/*
 *  LenixϵͳԤ������жϺţ����鲻Ҫ�޸ġ�
 *  Ϊ�˷����������չ��Lenix�û��ж�Ӧʹ��ISR_USER���Ժ���жϺ�
 */
#define ISR_CLOCK                   0
#define ISR_KEYBOARD                1
#define ISP_COM0                    2
#define ISP_COM1                    3
#define ISP_NET                     4
#define ISP_DISK                    5

#define ISR_USER                    8

#define IRQ_MASK_CLOCK              0x0001
#define IRQ_MASK_KEYBORAD           0x0002
#define IRQ_MASK_COM0               0x0004
#define IRQ_MASK_COM1               0x0008
#define IRQ_MASK_NET                0x0010
#define IRQ_MASK_DISK               0x0020

#define MILIONSECOND_TO_TICKS(ms)   (((ms) * Machine_clock_frequency_get()) / 1000)

/*
 *  �ж�Ƕ�׼�������LenixҪ���ڱ�д�ж���������ʱ��Ҫʹ�øü�����ͳ���ж�Ƕ�ײ��������ж�
 *  Ƕ�ײ�������INT_NEST_MAX ֮�󣬲��ڴ����ж����󣬸����жϳ���ֱ�ӷ��ء���֤֮ǰ���ж�
 *  �õ���ʱ����
 */
#define IRQ_NEST_MAX                196

extern uint8_t                  interrupt_nest;

imr_t       Machine_imr_get(void);
imr_t       Machine_imr_set(imr_t imr);

isp_t       Machine_ivt_get(uint_t ivtid);
isp_t       Machine_ivt_set(uint_t ivtid,isp_t isp);

void        Machine_interrupt_mis(void);
uint_t      Machine_interrup_mis_get(void);

void        Machine_base_initial(void);
void        Machine_initial(void);

/*  2014.1.1  */
void        Machine_reboot(void);

uint16_t    Machine_clock_frequency_get(void);
uint16_t    Machine_clock_frequency_set(uint16_t clkfrequency);
time_t  *   Machine_time_get(time_t * time);
result_t    Machine_time_set(const time_t * time);
date_t  *   Machine_date_get(date_t * date);
result_t    Machine_date_set(const date_t * date);
uint32_t    Machine_physical_memory(void);
uint32_t    Millisecond_to_ticks(uint32_t millisecond);
uint32_t    Ticks_to_millisecond(uint32_t ticks);

#endif /*   _MACHINE_H_     */