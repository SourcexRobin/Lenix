/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: pc.c
//  ����ʱ��:                   ������: �ޱ�
//  �޸�ʱ��: 2012-11-29        �޸���: �ޱ�
//  ��Ҫ����: �ṩPC����Ĺ��ܣ��Լ������ģ�͵ľ���ʵ�֡�
//  ˵    ��: 
//
//  �����¼:
//  �� �� ��    |     ʱ  ��    |   ��  ��      |   ��Ҫ�仯��¼
//=============================================================================
//              |   2014-03-11  |   ��  ��      |   ���ӻ�ȡ�����ڴ�������ʵ��
//              |               |               | ʱ��Ļ�ȡ
//  00.00.000   |               |   ��  ��      | ����ʱ���Ѿ�����
///////////////////////////////////////////////////////////////////////////////
*/

#include <result.h>
#include <lio.h>

#include <lmath.h>
#ifdef _SLASH_
    #include <machine/machine.h>
    #include <machine/pc.h>
#else
    #include <machine\machine.h>
    #include <machine\pc.h>
#endif  /*  _SLASH_ */

extern uint32_t             physical_memory;

#define CMOS_WAIT_TIME()            do{ while( Pc_read_cmos(0x0A) & 0x80) ;\
                                      }while(0)
/*  201-03-11  ��CMOS����  */
static
byte_t      Pc_read_cmos(byte_t reg)
{
    Io_outb((void *)0x70,reg);
    return Io_inb((void *)0x71);
}

void        Pc_clock_frequency(uint16_t frequency)
{
#define CLK0_INPUT_FREQUENCY 1193200
    frequency = CLK0_INPUT_FREQUENCY / frequency ;

    Io_outb((void *)0x43,0x36);
    Io_outb((void *)0x40,(byte_t)( frequency    &0xff));
    Io_outb((void *)0x40,(byte_t)((frequency>>8)&0xff));
}

uint16_t    Machine_clock_frequency_set_hook(uint16_t frequency)
{
    Pc_clock_frequency(frequency);

    return 0;
}

uint32_t    Pc_physical_memory(void)
{
    return (((uint32_t)Pc_read_cmos(0x36) * 0x100 + 
             (uint32_t)Pc_read_cmos(0x35)) + 1)*K ;
}

time_t  *   Machine_time_get(time_t * time)
{
    if( NULL == time )
        return NULL;
    CMOS_WAIT_TIME();
    time->time_week     = Bcd_to_bin(Pc_read_cmos(6));
    CMOS_WAIT_TIME();
    time->time_second   = Bcd_to_bin(Pc_read_cmos(0));
    CMOS_WAIT_TIME();
    time->time_minute   = Bcd_to_bin(Pc_read_cmos(2));
    CMOS_WAIT_TIME();
    time->time_hour     = Bcd_to_bin(Pc_read_cmos(4));

    return time;
}
result_t    Machine_time_set(const time_t * time)
{
    return RESULT_FAILED;
}
date_t  *   Machine_date_get(date_t * date)
{
    if( NULL == date )
        return NULL;
    CMOS_WAIT_TIME();
    date->date_day      = Bcd_to_bin(Pc_read_cmos(7));
    CMOS_WAIT_TIME();
    date->date_month    = Bcd_to_bin(Pc_read_cmos(8));
    CMOS_WAIT_TIME();
    date->date_year     = Bcd_to_bin(Pc_read_cmos(9)) + 2000;

    return date;
}
result_t    Machine_date_set(const date_t * date)
{
    return RESULT_FAILED;
}

/*
 *  ��������:init_8259A

 *  ˵������ʼ��8259�жϿ�������Ӳ���ж�ӳ�䵽0x20 -- 0x2F��Χ
 */
void        Pic_initial(void)
{
    // ��ʼ����8259A
    _printk("PC pic initial...    ");
    Io_outb((void *)0x20,0x11       );Io_delay();Io_delay();
    Io_outb((void *)0x21,IRQ_BASE   );Io_delay();Io_delay(); // �����ж�����
    Io_outb((void *)0x21,0x04       );Io_delay();Io_delay();
    Io_outb((void *)0x21,0x01       );Io_delay();Io_delay();    

    // ��ʼ����8259A    
    Io_outb((void *)0xa0,0x11       );Io_delay();Io_delay();
    Io_outb((void *)0xa1,IRQ_BASE+8 );Io_delay();Io_delay();
    Io_outb((void *)0xa1,0x02       );Io_delay();Io_delay();
    Io_outb((void *)0xa1,0x01       );Io_delay();Io_delay();

    Io_outb((void *)0x21,0xFB); // ���������ж�
    Io_outb((void *)0xa1,0xFF); // ���������ж�    
    _printk("OK!\n");
}

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  �жϴ���
*/
//    ����ʱ���жϣ�Ҳ���Ǵ���8259A��ʱ������
void        Pc_enable_clock(void)
{
    Io_outb((void *)0x21,(byte_t)(Io_inb((void *)0x21) & 0xFE));
}
// �ر�ʱ���ж�
void        Pc_disable_clock(void)
{
    Io_outb((void *)0x21,(byte_t)(Io_inb((void *)0x21) | 0x01));
}
//    ���ż����ж�
void        Pc_enable_keyboard(void)
{
    Io_outb((void *)0x21,(byte_t)(Io_inb((void *)0x21) & 0xFD));
}
// �رռ����ж�
void        Pc_disable_keyboard(void)
{
    Io_outb((void *)0x21,(byte_t)(Io_inb((void *)0x21) | 0x02));
}

/*
 *  2013-12-06
 */
void        Pc_enable_com1(void)
{
    Io_outb((void *)0x21,(byte_t)(Io_inb((void *)0x21) & 0xEF));
}
void        Pc_disable_com1(void)
{
    Io_outb((void *)0x21,(byte_t)(Io_inb((void *)0x21) | 0x10));
}
#ifdef _CFG_DEBUG_


#endif  /*  _CFG_DEBUG_    */

void        Machine_initial_hook(void)
{
#if _CPU_WORD_ == 16
    dword_t handle;
#endif
    Pic_initial();
    Con_initial();
    Kb_initial();
    //Rs_initial();

    physical_memory = Pc_physical_memory();
#if _CPU_WORD_ == 16
    handle = ((dword_t)Seg_get_cs()) * 0x10000 | (word_t)Sys_call_entry;
    Ivt_set(0x77,handle);

    handle = ((dword_t)Seg_get_cs()) * 0x10000 | (word_t)Irq_clock;
    Ivt_set(IRQ_CLOCK,handle);

    handle = ((dword_t)Seg_get_cs()) * 0x10000 | (word_t)Irq_keyboard;
    Ivt_set(IRQ_KEYBOARD,handle);

    handle = ((dword_t)Seg_get_cs()) * 0x10000 | (word_t)Irq_com1;
    /*Ivt_set(IRQ_COM1,handle);
    Ivt_set(IRQ_COM2,handle);*/
#else
    /*  32λ��� */
    Ivt_set(IRQ_CLOCK,(dword_t)Irq_clock);
    Ivt_set(IRQ_KEYBOARD,(dword_t)Irq_keyboard);
#endif
}

void        Lenix_start_hook(void)
{
    Pc_enable_clock();
    Pc_enable_keyboard();
    //Pc_enable_com1();
}

