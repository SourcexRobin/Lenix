/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    嵌入式操作系统
//                             2011 , 罗  斌，源代码工作室
//                                     保留所有版权
//                          ***---------------------------***
//
//  文件名称    : assert.h
//  文件创建者  : 罗  斌
//  文件创建时间:
//  最后修改者  : 罗  斌
//  最后修改时间: 
//
//  主要功能    : 提供断言功能，用于内核调试
//
//  说明        : 该宏需要在定义_CFG_DEBUG_后，方可使用。如果没有定义_CFG_DEBUG_，ASSERT宏
//                将被替换为空行。
//
//  版本变化记录:
//  版本号      |     时间      | 作  者       | 主要变化记录
//========================================================================================
//              |   2012-07-03  | 罗  斌       | 在BC31下使用字符串函数出现异常，修改为分
//                                               段输出信息
//  00.00.000   |   2011-12-01  | 罗  斌       | 第一版，具体时间已经不记得
//
//////////////////////////////////////////////////////////////////////////////////////////
*/

#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <config.h>
#include <lio.h>

void        Sys_halt(const char * msg);

#ifdef _CFG_DEBUG_

/*
 *  该宏判断参数所提供的表达式是否为真，如果为真，则不作处理。如果不为真，则输出对应的表达
 *  式、文件名、所在行号，然后关闭中断，进入死循环。也就是死机。
 */
#define ASSERT(ex)                  do{ if( !(ex) ){ \
                                        _printf("assert: %s \nfile: %s\nline : %d\n", \
                                                #ex,__FILE__,__LINE__); \
                                        Sys_halt("assert failed"); } \
                                      }while(0)

#else

#define ASSERT(ex)

#endif // _CFG_DEBUG_

//  2014.2.1
#define DEBUG_HLT(msg)              do{ _printk("%s\n",msg);for(;;) Cpu_hlt(); }while(0)

#endif  /*  _ASSERT_H_   */