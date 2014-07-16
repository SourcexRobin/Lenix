/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: machine.c
//  创建时间: 2011-02-09        创建者: 罗斌
//  修改时间: 2014-03-11        修改者: 罗斌
//  主要功能: 提供对目标机器的接口。
//  说    明: 1、 Lenix默认提供16个中断接口，并预定义了几个硬件中断的编号，也就
//            是假设目标机器具备这些硬件
//            2、 提供了关闭机器中断控制器接口
//            3、 提供了修改系统时钟频率接口
//            也就是目标机器必须具备中断控制器和时钟
//
//  变更记录:
//  版 本 号    |    时   间    |   作  者      |   主要变化记录
//=============================================================================
//              |   2014-03-11  |   罗  斌      | 增加获取物理内存数量
//  00.00.000   |   2011-02-09  |   罗  斌      | 建立文件
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

/*  Lenix默认提供16个中断入口   */
#define IVT_MAX                     16
#define IRQ_SRC_MAX					16
/*
 *  Lenix系统预定义的中断号，建议不要修改。
 *  为了方便往后的扩展，Lenix用户中断应使用ISR_USER及以后的中断号
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
 *  中断嵌套计数器，Lenix要求在编写中断驱动程序时，要使用该计数器统计中断嵌套层数，在中断
 *  嵌套层数超过INT_NEST_MAX 之后，不在处理中断请求，各个中断程序直接返回。保证之前的中断
 *  得到及时处理。
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