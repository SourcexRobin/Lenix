/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    嵌入式操作系统
//                             2011 , 罗  斌，源代码工作室
//                                     保留所有版权
//                          ***---------------------------***
//
//  文件名称    : clock.h
//  文件创建者  : 罗  斌
//  文件创建时间:
//  最后修改者  : 罗  斌
//  最后修改时间: 
//
//  主要功能    : 提供定时器
//
//  说明        : 
//
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//========================================================================================
//  00.00.000   |   2011-12-30  |  罗斌         |  xxxxxx
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <type.h>
#include <result.h>

#define TIMER_REPEAT_MAX            30000

typedef void                (* ticks_hook_t)(void);
typedef void                (* tm_handle_t)(void *);

typedef struct _timer_t
{
    int                     tm_id;
    int                     tm_ticks;
    int                     tm_left;
#if CPU_WORD == 8
    short                   tm_repeat;
#else
    int                     tm_repeat;
#endif
    void                *   tm_param;
    tm_handle_t             tm_handle;
}timer_t;

#define TIMER_REPEAT_INFINITE       (-1)

#define TIMER_ZERO(tm)              _memzero(tm,sizeof(timer_t))

#define TIMER_IS_INFINITE(tm)       ( (tm)->tm_repeat < 0       )
#define TIMER_CAN_REPEAT(tm)        ( (tm)->tm_repeat           )
#define TIMER_IS_FREE(tm)           ( NULL == (tm)->tm_handle   )
#define TIMER_IS_VALID(tm)          ( (tm)->tm_handle           )

void        Clk_initial(void);
void        Clk_delay(uint32_t millisecond);
uint32_t    Clk_get_ticks(void);
void *      Clk_ticks_hook_get(void);
void *      Clk_ticks_hook_set(ticks_hook_t tickshook);

int         Timer_create(uint32_t millisecond,int repeat,void (*handle)(void *),void * param);
result_t    Timer_delete(int id);

#endif  /*  _CLOCK_H_   */