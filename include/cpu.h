/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    嵌入式操作系统
//                             2011 , 罗  斌，源代码工作室
//                                     保留所有版权
//                          ***---------------------------***
//
//  文件名称    : cpu.h
//  文件创建者  :
//  文件创建时间:
//  最后修改者  : 罗  斌
//  最后修改时间:
//
//  主要功能    : 提供抽象CPU的基本操作.
//
//  说明        : 该文件声明的函数都是移植时需要实现的基本功能。
//
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//========================================================================================
//              |   2012-07-03  |  罗斌         |  增加IO空间支持
//              |   2012-06-30  |  罗斌         |  修改Cpu_tas
//                  2012-06-29  |  罗斌         |  增加条件编译，
//  00.00.000   |   2011-02-09  |  罗斌         |  xxxxxx
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
#ifndef _CPU_H_
#define _CPU_H_

#include <config.h>
#include <type.h>


//2012/07/12
/*
typedef struct _cpu_t
{
    psw_t                 (*cpu_get_psw)(void);
    psw_t                 (*cpu_set_psw)(psw_t psw);

    psw_t                 (*cpu_disable_interrupt)(void);
    psw_t                 (*cpu_enable_interrupt)(void);

    byte_t                  cpu_inb (void *);
    word_t                  cpu_inw (void *);
    dword_t                 cpu_ind (void *);
    void                    cpu_outb(void *,byte_t);
    void                    cpu_outw(void *,word_t);
    void                    cpu_outd(void *,dword_t);
    void                *   cpu_inb_buffer  (void *,void *,size_t);
    void                *   cpu_inw_buffer  (void *,void *,size_t);
    void                *   cpu_ind_buffer  (void *,void *,size_t);
    void                *   cpu_outb_buffer (void *,void *,size_t);
    void                *   cpu_outw_buffer (void *,void *,size_t);
    void                *   cpu_outd_buffer (void *,void *,size_t);
    void                    cpu_io_delay(void);

    int                     cpu_tas(int *,int,int);

    void                    cpu_halt(void);
}cpu_t;

extern cpu_t                cpu;
*/
/*
 *  CPU支持tas类指令是，定义该宏，并且提供相应的实现函数
 */
#define _CPU_TAS_

/*
 *  这个是专门为16位X86定义的
 */
#if _CFG_CPU_ == _CFG_CPU_X86_ && _CPU_WORD_ == 16
    #define FAR                     far
#else
    #define FAR
#endif
/*
 *  如果编译器支持嵌入式汇编，则Lenix提供直接使用CPU开关中断的指令
 *  移植时，需要注意修改这部分，将其修改相应CPU的汇编指令
 */
#ifdef _CFG_ASM_

    #if _CFG_CPU_ == _CFG_CPU_X86_

        #define CPU_DISABLE_INTERRUPT() asm{ cli }
        #define CPU_ENABLE_INTERRUPT()  asm{ sti }

    #elif _CFG_CPU_ == _CFG_CPU_ARM_

        #define CPU_DISABLE_INTERRUPT() asm{ cli }
        #define CPU_ENABLE_INTERRUPT()  asm{ sti }

    #endif 

#else

    #define CPU_DISABLE_INTERRUPT() Cpu_disable_interrupt()
    #define CPU_ENABLE_INTERRUPT()  Cpu_enable_interrupt()

#endif /*   _CC_ASM_    */

psw_t       Cpu_psw_get(void);                      /*  获得CPU状态字   */
psw_t       Cpu_psw_set(psw_t psw);                 /*  设置CPU状态字   */

psw_t       Cpu_disable_interrupt(void);            /*  禁止CPU响应中断 */
psw_t       Cpu_enable_interrupt(void);             /*  允许CPU响应终端 */

/*
 *  IO地址空间独立，典型的CPU是x86系列
 *  这种类型CPU的IO操作，一般需要独立的IO指令，C语言通常不直接支持这样的指令，因此需要通过
 *  使用汇编语言编写的程序来提供相应的支持
 */
byte_t      Io_inb  (void * port);                  /*  输入 8位数据    */
word_t      Io_inw  (void * port);                  /*  输入16位数据    */
dword_t     Io_ind  (void * port);                  /*  输入32位数据    */

void        Io_outb (void * port,byte_t  dat);      /*  输出 8位数据    */
void        Io_outw (void * port,word_t  dat);      /*  输出16位数据    */
void        Io_outd (void * port,dword_t dat);      /*  输出32位数据    */

/*
 *  批量输入输出数据
 */
void    *   Io_inb_buffer   (void * port,void * buffer,size_t size);
void    *   Io_inw_buffer   (void * port,void * buffer,size_t size);
void    *   Io_ind_buffer   (void * port,void * buffer,size_t size);

void    *   Io_outb_buffer  (void * port,void * buffer,size_t size);
void    *   Io_outw_buffer  (void * port,void * buffer,size_t size);
void    *   Io_outd_buffer  (void * port,void * buffer,size_t size);

int         Io_delay(void);

/*
 *  名  称: Cpu_tas_i   : instructer
 *          Cpu_tas_s   : simulate
 *
 *  参  数: 
 *
 *  返回值: lck == test，返回test
 *          lck != test，返回lck
 *
 *  说  明: 实际上无论如何都是返回原始的lck
 *          在使用时，单CPU可以使用模拟实现。如果是多CPU，必须使用CPU提供的硬件指令
 */
int         Cpu_tas_i(int * lck,int test,int set);          /*  CPU具有相应的指令   */
int         Cpu_tas_s(int * lck,int test,int set);          /*  软件模拟            */

#ifdef _CPU_TAS_
    #define Cpu_tas                 Cpu_tas_i
#else
    #define Cpu_tas                 Cpu_tas_s
#endif

void        Cpu_hlt(void);                          /*  CPU停机         */

uint_t *    Context_initial(void * entry,void * param,uint_t * sp); /*  环境初始化入口  */

void        Cpu_initial(void);

#endif  /*  _CPU_H_ */