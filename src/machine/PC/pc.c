/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: pc.c
//  创建时间:                   创建者: 罗斌
//  修改时间: 2012-11-29        修改者: 罗斌
//  主要功能: 提供PC所需的功能，以及计算机模型的具体实现。
//  说    明: 
//
//  变更记录:
//  版 本 号    |     时  间    |   作  者      |   主要变化记录
//=============================================================================
//              |   2014-03-11  |   罗  斌      |   增加获取物理内存数量，实现
//              |               |               | 时间的获取
//  00.00.000   |               |   罗  斌      | 建立时间已经忘记
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
/*  201-03-11  读CMOS数据  */
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
 *  函数名称:init_8259A

 *  说明：初始化8259中断控制器将硬件中断映射到0x20 -- 0x2F范围
 */
void        Pic_initial(void)
{
    // 初始化主8259A
    _printk("PC pic initial...    ");
    Io_outb((void *)0x20,0x11       );Io_delay();Io_delay();
    Io_outb((void *)0x21,IRQ_BASE   );Io_delay();Io_delay(); // 设置中断向量
    Io_outb((void *)0x21,0x04       );Io_delay();Io_delay();
    Io_outb((void *)0x21,0x01       );Io_delay();Io_delay();    

    // 初始化从8259A    
    Io_outb((void *)0xa0,0x11       );Io_delay();Io_delay();
    Io_outb((void *)0xa1,IRQ_BASE+8 );Io_delay();Io_delay();
    Io_outb((void *)0xa1,0x02       );Io_delay();Io_delay();
    Io_outb((void *)0xa1,0x01       );Io_delay();Io_delay();

    Io_outb((void *)0x21,0xFB); // 屏蔽所有中断
    Io_outb((void *)0xa1,0xFF); // 屏蔽所有中断    
    _printk("OK!\n");
}

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  中断处理
*/
//    开放时钟中断，也就是打开主8259A的时钟引脚
void        Pc_enable_clock(void)
{
    Io_outb((void *)0x21,(byte_t)(Io_inb((void *)0x21) & 0xFE));
}
// 关闭时钟中断
void        Pc_disable_clock(void)
{
    Io_outb((void *)0x21,(byte_t)(Io_inb((void *)0x21) | 0x01));
}
//    开放键盘中断
void        Pc_enable_keyboard(void)
{
    Io_outb((void *)0x21,(byte_t)(Io_inb((void *)0x21) & 0xFD));
}
// 关闭键盘中断
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
    /*  32位情况 */
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

