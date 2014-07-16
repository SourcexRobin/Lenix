/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    嵌入式操作系统
//                             2011 , 罗  斌，源代码工作室
//                                     保留所有版权
//                          ***---------------------------***
//
//  文件名称    : cpu.c
//  文件创建者  : 罗  斌
//  文件创建时间: 2012-02-29
//  最后修改者  : 
//  最后修改时间:
//
//  主要功能    : 提供基本的CPU操作功能
//
//  说明        :
//
//  版本变化记录:
//  版本号      |     时间      |  作  者      | 主要变化记录
//========================================================================================
//  00.00.000   |   2012-06-29  |  罗  斌      | 第一版，将原来分布在其他文件的功能集中
//
//////////////////////////////////////////////////////////////////////////////////////////
*/

//  2012/6/29

#include <cpu.h>
#include <assert.h>

#ifndef _CFG_SMP_ 
#if _CFG_CRITICAL_METHOD_ == 1
#endif
#endif 

/*
 *  临界段嵌套层数，不超过255层，我认为对于嵌入式系统来说，已经足够。
 *  用于实现不支持PSW直接操作中断的CPU的临界段。
 */
byte_t critical_nest = 0;


int         Io_delay()
{
    volatile int i = 0;

    return ++i;
}


#ifndef _CFG_SMP_ 
/*
 *  模拟方式实现测试并置位(test and set)功能，只能用在单CPU单核心
 */
int         Cpu_tas_s(int * lck,int test,int set)
{
#if _CFG_CRITICAL_METHOD_ == 1
    
    Cpu_disable_interrupt();

    ++critical_nest;

    ASSERT(critical_nest < 255);

    if( test == *lck )
        *lck  = set;
    else
        test = *lck;

    if( --critical_nest == 0 )
        Cpu_enable_interrupt();
#else
    psw_t psw;

    psw = Cpu_disable_interrupt();

    if( test == *lck )
        *lck  = set;
    else
        test = *lck;

    Cpu_psw_set(psw);

#endif

    return test;
}

#endif  /*  _CFG_SMP_   */


#ifndef _CFG_IO_SPACE_  
/*
 *  IO地址空间和内存地址混合
 *  这种类型CPU的IO可以使用通常的内存方位指令进行操作，实际上也就是C语言的指针操作
 *  
 *  这里提供的是一种访问方式，如果有不同的访问方式，在移植的时候，应该进行相应的修改
 *
 *  由于某些编译器不支持C语言使用inline关键字(Borland C/C++ 3.1)，因此将这部分单独放在
 *  源文件中，原本计划放在头文件中
 */
byte_t      Io_inb(void * port) 
{ 
    return *(byte_t  *)port;  
}

word_t      Io_inw(void * port) 
{ 
    return *(word_t  *)port;  
}
dword_t     Io_ind(void * port) 
{ 
    return *(dword_t *)port;  
}
void        Io_outb (void * port,byte_t  dat)   
{ 
    *(byte_t  *)port = dat;   
}
void        Io_outw (void * port,word_t  dat)   
{ 
    *(word_t  *)port = dat;   
}
void        Io_outd (void * port,dword_t dat)   
{ 
    *(dword_t *)port = dat;   
}

/*
 *  批量输入输出数据
 */
void *      Io_inb_buffer(void * port,void * buffer,size_t size)
{
    byte_t              *   buf = buffer;
    while(size--)
        *buf++ = *(byte_t *)port;
    return  buffer;
}
void *      Io_inw_buffer(void * port,void * buffer,size_t size)
{
    word_t              *   buf = buffer;
    while(size--)
        *buf++ = *(word_t *)port;
    return  buffer;
}
void *      Io_ind_buffer(void * port,void * buffer,size_t size)
{
    dword_t             *   buf = buffer;
    while(size--)
        *buf++ = *(dword_t *)port;
    return  buffer;

}

void *      Io_outb_buffer(void * port,void * buffer,size_t size)
{
    byte_t              *   buf = buffer;
    while(size--)
        *(byte_t *)port = *buf++;
    return  buffer;

}
void *      Io_outw_buffer(void * port,void * buffer,size_t size)
{
    word_t              *   buf = buffer;
    while(size--)
        *(word_t *)port = *buf++;
    return  buffer;
}
void *      Io_outd_buffer(void * port,void * buffer,size_t size)
{
    dword_t             *   buf = buffer;
    while(size--)
        *(dword_t *)port = *buf++;
    return  buffer;
}

#endif  /*  _CFG_IO_SPACE_*/
