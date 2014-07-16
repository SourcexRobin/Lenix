/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: machine.c
//  创建时间: 2011-02-09        创建者: 罗斌
//  修改时间: 2014-03-11        修改者: 罗斌
//  主要功能: 提供了具体硬件的抽象。
//  说    明:   本文件涉及具体的硬件平台，因此是移植的重点。假设硬件具备中断控
//            制器、时钟、VGA兼容显示器、4个串行口
//
//  变更记录:
//  版 本 号    |    时   间    |   作  者      |   主要变化记录
//=============================================================================
//              |   2014-03-11  |   罗  斌      | 增加获取物理内存数量
//  00.00.000   |   2011-02-09  |   罗  斌      | 建立文件
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>
#include <assert.h>

#ifdef _SLASH_
    #include <machine/machine.h>
#else
    #include <machine\machine.h>
#endif  /*  _SLASH_ */

/*  物理内存数量，以KB为单位。在具体的平台初始化时，填写这个变量*/
uint32_t                    physical_memory;

/*  时钟频率                */
static uint16_t             clk_frequency;
/*  禁止硬件中断嵌套层数    */
static uint8_t              disable_interrupt_nest;
static int                  pic_lock;
/*  中断丢失计数器          */
volatile static uint_t      interrupt_mis_count;
/*
 *  中断嵌套层数
 *    Lenix 要求在维护硬件嵌套层数，如果嵌套层数过多，说明中断处理程序运行时间
 *  过长，容易导致中断响应不及时。因此在超过最大嵌套层数时，Lenix不在响应中断，
 *  各个中断处理程序直接返回
 *    Lenix 建议中断处理程序按一下流程来编写
 *      1.保存寄存器
 *      2.检测嵌套层数
 *      3.开放中断
 *      4.响应中断
 *      5.检测嵌套层数，
 */
uint8_t                     interrupt_nest;                 /*                      */
isp_t                       machine_ivt[IRQ_SRC_MAX];       /*  中断向量表           */
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
/*  Lenix默认的中断处理程序，什么都不做 */
int         Machine_isp_default(int nest,int param)
{
    nest  = nest;   /*  避免某些编译器发出变量未使用警告    */
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
//  名  称: Machine_base_initial
//
//  作  用: 初始化Lenix假设的基本物理设备。
//
//  参  数: 无
//
//  返回值: 无
//
//  注  意: 本函数只是完成Lenix假设的目标机器的初始化，各个具体目标机器的初始化，要另外
//          实现Machine_initial。
//          具体设备的初始化工作由Machine_initial完成。Machine_initial在具体的设备初始化
//          文件中实现。Machine_initial才是main函数调用的函数。
//          例如PC，在pc.c文件中实现。
//
//  变更记录:
//  时间        |  作者         |  说明
//========================================================================================
//  xxxx-xx-xx  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
void        Machine_initial(void)
{
    int         i;
    
    /*  将中断处理程序设置为默认的中断处理程序*/
    for( i = 0 ; i < IRQ_SRC_MAX ; i++ )
        machine_ivt[i] = Machine_isp_default;

    clk_frequency           = DEFAULT_CLOCK_FREQUENCY;  /*  将始终频率设置为默认的频率 */
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
//  名  称: Machine_ivt_get
//
//  作  用: 获得相应编号的硬件中断处理程序。
//
//  参  数: 
//      ivtid               : uint_t
//                          : 硬件中断编号
//
//  返回值: 
//      类型: isp_t
//
//  注  意: 
//
//  变更记录:
//  时间        |  作者         |  说明
//========================================================================================
//  2012-08-05  |  罗斌         |  修改名称，由isr改为isp
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
//  名  称: Machine_ivt_set
//
//  作  用: 设置相应编号的硬件中断处理程序。
//
//  参  数: 
//      ivtid               : uint_t
//                          : 硬件中断编号
//
//      isp                 : isp_t
//                          : 新的中断处理程序
//
//  返回值: 
//      类型: isp_t
//      说明: 成功返回非NULL，失败返回NULL
//
//  注  意: 在ivtid超过Lenix设定的中断最大值或者传入的中断处理函数为NULL，将导致函数失败。
//          该最大值由ISR_MAX定义.
//
//  变更记录:
//  时间        |  作者         |  说明
//========================================================================================
//  2012-08-05  |  罗斌         |  修改名称，由isr改为isp
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
     *  修改硬件中断处理程序，考虑SMP情况，需要关闭硬件中断。
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

/*  将毫秒转换为时钟中断次数    */
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
//  名  称: Machine_clock_frequency_set
//
//  作  用: 设置机器的时钟频率。
//
//  参  数: 
//      frequency           : uint16_t
//                          : 时钟频率，以hz为单位.
//
//  返回值: 
//      类型: uint16_t
//      说明: 原时钟频率
//
//  注  意: 本程序会对传入的参数进行调整.超过允许的最大频率，Lenix将把频率设置为最大频率。
//          低于最小频率，Lenix则将频率，设置为最小频率，以保证系统时钟频率在一个可接受的
//          范围内.修改时钟频率后，进程以毫秒为单位的cpu时间不应该变化。
//          Lenix定义的频率如下:
//          MIN_CLOCK_FREQUENCY     : 定义了最小频率，默认为20hz
//          MAX_CLOCK_FREQUENCY     : 定义了最大频率，默认为1000hz
//
//  变更记录:
//  时间        |  作者         |  说明
//========================================================================================
//  xxxx-xx-xx  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
uint16_t    Machine_clock_frequency_set(uint16_t frequency)
{
    uint16_t    prev_freq = clk_frequency;

    /*  将频率限定在规定的范围内              */
    clk_frequency = MIN(frequency,MAX_CLOCK_FREQUENCY);
    clk_frequency = MAX(frequency,MIN_CLOCK_FREQUENCY);

    /*
     *  根据频率调整cpu时间片的长度
     *  计算方式为 (默认时间片*1000)/频率
     *  使用单位换算表示为: ( 毫秒 / 1000) / (次/秒)
     */
    proc_cpu_time = (DEFAULT_CPU_TIME * clk_frequency) / 1000;

    Machine_clock_frequency_set_hook(clk_frequency);

    return prev_freq;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  以下为具体设备的功能程序
*/

#ifdef _CFG_MACHINE_PC_

/*  本部分为Lenix相应功能的PC实现    */
#ifdef _SLASH_
    #include <machine/pc.h>
#else
    #include <machine\pc.h>
#endif  /*  _SLASH_ */

static byte_t                   pics;   /*  主中断控制器状态                    */
                                        /*  primary interrupt controlor status  */
static byte_t                   sics;   /* 从中断控制器状态                     */
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
//  名  称: Machine_imr_set
//
//  作  用: 对设备硬件包含的中断控制器进行设置
//
//  参  数: 
//      名称，imr              类型，imr_t
//                             说明：
//
//  返回值: 
//    类型: imr_t
//    说明: 
//
//  注  意: 
//
//  变更记录:
//  时间        |  作  者       |  说明
//========================================================================================
//  2012-07-20  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
imr_t       Machine_imr_set(imr_t imr)
{
    imr_t                   oimr    = 0; //  old mask

    /*
     *  这里应该是原子操作，不能被打断。
     *  但是，这个函数会在进程初始化之前调用，因此这里实现原子操作的方式有点特殊。
     *  只能使用tas机制
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

/*  直接将PC相应的源文件包含在内，这样可以编译出单独的一个obj文件   */
#ifndef _PC_DEBUG_
#include <..\src\machine\pc\pc.c>
#include <..\src\machine\pc\com.c>
#include <..\src\machine\pc\consol.c>
#include <..\src\machine\pc\keyboard.c>
#include <..\src\machine\pc\pc_debug.c>
#endif

#endif 




