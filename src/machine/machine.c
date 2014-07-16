/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: machine.c
//  ����ʱ��: 2011-02-09        ������: �ޱ�
//  �޸�ʱ��: 2014-03-11        �޸���: �ޱ�
//  ��Ҫ����: �ṩ�˾���Ӳ���ĳ���
//  ˵    ��:   ���ļ��漰�����Ӳ��ƽ̨���������ֲ���ص㡣����Ӳ���߱��жϿ�
//            ������ʱ�ӡ�VGA������ʾ����4�����п�
//
//  �����¼:
//  �� �� ��    |    ʱ   ��    |   ��  ��      |   ��Ҫ�仯��¼
//=============================================================================
//              |   2014-03-11  |   ��  ��      | ���ӻ�ȡ�����ڴ�����
//  00.00.000   |   2011-02-09  |   ��  ��      | �����ļ�
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>
#include <assert.h>

#ifdef _SLASH_
    #include <machine/machine.h>
#else
    #include <machine\machine.h>
#endif  /*  _SLASH_ */

/*  �����ڴ���������KBΪ��λ���ھ����ƽ̨��ʼ��ʱ����д�������*/
uint32_t                    physical_memory;

/*  ʱ��Ƶ��                */
static uint16_t             clk_frequency;
/*  ��ֹӲ���ж�Ƕ�ײ���    */
static uint8_t              disable_interrupt_nest;
static int                  pic_lock;
/*  �ж϶�ʧ������          */
volatile static uint_t      interrupt_mis_count;
/*
 *  �ж�Ƕ�ײ���
 *    Lenix Ҫ����ά��Ӳ��Ƕ�ײ��������Ƕ�ײ������࣬˵���жϴ����������ʱ��
 *  ���������׵����ж���Ӧ����ʱ������ڳ������Ƕ�ײ���ʱ��Lenix������Ӧ�жϣ�
 *  �����жϴ������ֱ�ӷ���
 *    Lenix �����жϴ������һ����������д
 *      1.����Ĵ���
 *      2.���Ƕ�ײ���
 *      3.�����ж�
 *      4.��Ӧ�ж�
 *      5.���Ƕ�ײ�����
 */
uint8_t                     interrupt_nest;                 /*                      */
isp_t                       machine_ivt[IRQ_SRC_MAX];       /*  �ж�������           */
imr_t                       machine_imr;

#define ISR_MAX             IVT_MAX

#define machine_isr         machine_ivt

extern int                  proc_cpu_time;

void        Machine_initial_hook(void);
uint16_t    Machine_clock_frequency_set_hook(uint16_t frequency);


uint32_t    Machine_physical_memory(void)
{
    return physical_memory;
}
/*  LenixĬ�ϵ��жϴ������ʲô������ */
int         Machine_isp_default(int nest,int param)
{
    nest  = nest;   /*  ����ĳЩ��������������δʹ�þ���    */
    param = param;
    return 0;
}

/*  */
void        Machine_interrupt_mis(void)
{
    ++interrupt_mis_count;
    //ASSERT(interrupt_mis_count == 0);
}

uint_t      Machine_interrup_mis_get(void)
{
    return interrupt_mis_count;
}

#ifdef _CFG_MACHINE_INITIAL_HOOK_DEFAULT_

void        Machine_initial_hook(void)
{
}

#endif

/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  ��  ��: Machine_base_initial
//
//  ��  ��: ��ʼ��Lenix����Ļ��������豸��
//
//  ��  ��: ��
//
//  ����ֵ: ��
//
//  ע  ��: ������ֻ�����Lenix�����Ŀ������ĳ�ʼ������������Ŀ������ĳ�ʼ����Ҫ����
//          ʵ��Machine_initial��
//          �����豸�ĳ�ʼ��������Machine_initial��ɡ�Machine_initial�ھ�����豸��ʼ��
//          �ļ���ʵ�֡�Machine_initial����main�������õĺ�����
//          ����PC����pc.c�ļ���ʵ�֡�
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//========================================================================================
//  xxxx-xx-xx  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
void        Machine_initial(void)
{
    int         i;
    
    /*  ���жϴ����������ΪĬ�ϵ��жϴ������*/
    for( i = 0 ; i < IRQ_SRC_MAX ; i++ )
        machine_ivt[i] = Machine_isp_default;

    clk_frequency           = DEFAULT_CLOCK_FREQUENCY;  /*  ��ʼ��Ƶ������ΪĬ�ϵ�Ƶ�� */
    interrupt_nest          = 0;
    pic_lock                = 0;
    disable_interrupt_nest  = 0;
    interrupt_mis_count     = 0;
    machine_imr             = -1;

    Machine_clock_frequency_set(clk_frequency);

    Machine_initial_hook();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  ��  ��: Machine_ivt_get
//
//  ��  ��: �����Ӧ��ŵ�Ӳ���жϴ������
//
//  ��  ��: 
//      ivtid               : uint_t
//                          : Ӳ���жϱ��
//
//  ����ֵ: 
//      ����: isp_t
//
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//========================================================================================
//  2012-08-05  |  �ޱ�         |  �޸����ƣ���isr��Ϊisp
//  xxxx-xx-xx  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
isp_t       Machine_ivt_get(uint_t ivtid)
{
    return machine_ivt[ivtid & 0xF ];
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  ��  ��: Machine_ivt_set
//
//  ��  ��: ������Ӧ��ŵ�Ӳ���жϴ������
//
//  ��  ��: 
//      ivtid               : uint_t
//                          : Ӳ���жϱ��
//
//      isp                 : isp_t
//                          : �µ��жϴ������
//
//  ����ֵ: 
//      ����: isp_t
//      ˵��: �ɹ����ط�NULL��ʧ�ܷ���NULL
//
//  ע  ��: ��ivtid����Lenix�趨���ж����ֵ���ߴ�����жϴ�����ΪNULL�������º���ʧ�ܡ�
//          �����ֵ��ISR_MAX����.
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//========================================================================================
//  2012-08-05  |  �ޱ�         |  �޸����ƣ���isr��Ϊisp
//  xxxx-xx-xx  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
isp_t       Machine_ivt_set(uint_t ivtid,isp_t isp)
{
    isp_t                   pisp        = NULL;   /* prev isp */
    uint16_t                mask        = 0;

    if( NULL == isp  )
        return NULL;

    ivtid = ivtid & 0xF;
    /*
     *  �޸�Ӳ���жϴ�����򣬿���SMP�������Ҫ�ر�Ӳ���жϡ�
     */
    mask = Machine_imr_set(0); 

    pisp = machine_ivt[ivtid];
    machine_ivt[ivtid]  = isp;

    Machine_imr_set(mask);

    return pisp;
}

uint16_t    Machine_clock_frequency_get(void)
{
    return clk_frequency;
}

/*  ������ת��Ϊʱ���жϴ���    */
uint32_t    Millisecond_to_ticks(uint32_t millisecond)
{
    return (uint32_t)(MILIONSECOND_TO_TICKS(millisecond));
}

#ifdef _CFG_MACHINE_CLOCK_FREQUENCY_HOOK_DEFAULT_

uint16_t    Machine_clock_frequency_set_hook(uint16_t frequency)
{
    return 0;
}

#endif
/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  ��  ��: Machine_clock_frequency_set
//
//  ��  ��: ���û�����ʱ��Ƶ�ʡ�
//
//  ��  ��: 
//      frequency           : uint16_t
//                          : ʱ��Ƶ�ʣ���hzΪ��λ.
//
//  ����ֵ: 
//      ����: uint16_t
//      ˵��: ԭʱ��Ƶ��
//
//  ע  ��: �������Դ���Ĳ������е���.������������Ƶ�ʣ�Lenix����Ƶ������Ϊ���Ƶ�ʡ�
//          ������СƵ�ʣ�Lenix��Ƶ�ʣ�����Ϊ��СƵ�ʣ��Ա�֤ϵͳʱ��Ƶ����һ���ɽ��ܵ�
//          ��Χ��.�޸�ʱ��Ƶ�ʺ󣬽����Ժ���Ϊ��λ��cpuʱ�䲻Ӧ�ñ仯��
//          Lenix�����Ƶ������:
//          MIN_CLOCK_FREQUENCY     : ��������СƵ�ʣ�Ĭ��Ϊ20hz
//          MAX_CLOCK_FREQUENCY     : ���������Ƶ�ʣ�Ĭ��Ϊ1000hz
//
//  �����¼:
//  ʱ��        |  ����         |  ˵��
//========================================================================================
//  xxxx-xx-xx  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
uint16_t    Machine_clock_frequency_set(uint16_t frequency)
{
    uint16_t    prev_freq = clk_frequency;

    /*  ��Ƶ���޶��ڹ涨�ķ�Χ��              */
    clk_frequency = MIN(frequency,MAX_CLOCK_FREQUENCY);
    clk_frequency = MAX(frequency,MIN_CLOCK_FREQUENCY);

    /*
     *  ����Ƶ�ʵ���cpuʱ��Ƭ�ĳ���
     *  ���㷽ʽΪ (Ĭ��ʱ��Ƭ*1000)/Ƶ��
     *  ʹ�õ�λ�����ʾΪ: ( ���� / 1000) / (��/��)
     */
    proc_cpu_time = (DEFAULT_CPU_TIME * clk_frequency) / 1000;

    Machine_clock_frequency_set_hook(clk_frequency);

    return prev_freq;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  ����Ϊ�����豸�Ĺ��ܳ���
*/

#ifdef _CFG_MACHINE_PC_

/*  ������ΪLenix��Ӧ���ܵ�PCʵ��    */
#ifdef _SLASH_
    #include <machine/pc.h>
#else
    #include <machine\pc.h>
#endif  /*  _SLASH_ */

static byte_t                   pics;   /*  ���жϿ�����״̬                    */
                                        /*  primary interrupt controlor status  */
static byte_t                   sics;   /* ���жϿ�����״̬                     */
                                        /*  slavry interrupt controlor status   */


static
uint16_t    Pc_pic_mask_convert(uint16_t mask)
{
    return mask;
}

imr_t       Machine_imr_get(void)
{
    return Io_inb((void *)0xA1) * 0x100 + Io_inb((void *)0x21);
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  ��  ��: Machine_imr_set
//
//  ��  ��: ���豸Ӳ���������жϿ�������������
//
//  ��  ��: 
//      ���ƣ�imr              ���ͣ�imr_t
//                             ˵����
//
//  ����ֵ: 
//    ����: imr_t
//    ˵��: 
//
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
//========================================================================================
//  2012-07-20  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
imr_t       Machine_imr_set(imr_t imr)
{
    imr_t                   oimr    = 0; //  old mask

    /*
     *  ����Ӧ����ԭ�Ӳ��������ܱ���ϡ�
     *  ���ǣ�����������ڽ��̳�ʼ��֮ǰ���ã��������ʵ��ԭ�Ӳ����ķ�ʽ�е����⡣
     *  ֻ��ʹ��tas����
     */
    while(Cpu_tas(&pic_lock,0,1)) 
        Cpu_hlt();

    oimr    = Io_inb((void *)0xA1) * 0x100 + Io_inb((void *)0x21);

    imr     = Pc_pic_mask_convert(imr);

    Io_outb((void *)0x21,(byte_t)( imr       & 0xFF));
    Io_outb((void *)0xA1,(byte_t)((imr >> 8) & 0xFF));

    pic_lock = 0;

    return oimr;
}

/*  ֱ�ӽ�PC��Ӧ��Դ�ļ��������ڣ��������Ա����������һ��obj�ļ�   */
#ifndef _PC_DEBUG_
#include <..\src\machine\pc\pc.c>
#include <..\src\machine\pc\com.c>
#include <..\src\machine\pc\consol.c>
#include <..\src\machine\pc\keyboard.c>
#include <..\src\machine\pc\pc_debug.c>
#endif

#endif 




