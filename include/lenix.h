/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: lenix.h
//  创建时间: 2011-12-02        创建者: 罗斌
//  修改时间: 2012-11-29        修改者: 罗斌
//  主要功能: 提供所有Lenix的头文件。
//  说    明: 
//
//  变更记录:
//  版 本 号    |     时间      |  作  者       | 主要变化记录
//=============================================================================
//              |   2014-02-26  |  罗  斌       | 增加koum.h
//              |   2012-07-31  |  罗  斌       | 增加非windows路径支持
//              |   2011-12-02  |  罗  斌       | 第一版
///////////////////////////////////////////////////////////////////////////////
*/
#ifndef _LENIX_H_
#define _LENIX_H_

#include <const.h>
#include <config.h>
#include <type.h>
#include <cpu.h>
#include <result.h>
#include <assert.h>

#include <lmemory.h>
#include <lchar.h>
#include <lstring.h>
#include <larg.h>
#include <lio.h>
#include <slist.h>
#include <ltime.h>      /*  2014-03-16 添加     */
#include <lmath.h>      /*  2014-03-16 添加     */

#include <koum.h>
#include <proc.h>
#include <ipc.h>
#include <clock.h>
#include <mm.h>
#include <sys.h>
#include <device.h>
#include <shell.h>
#include <tty.h>


void        Lenix_initial(void);
void        Lenix_start(void);
void        Lenix_start_hook(void);

#ifdef _SLASH_

    #include <gui/graph.h>
    #include <gui/window.h>
    #include <machine/machine.h>

    #include <driver/pc_ata.h>
    #include <driver/pc_vga.h>

#else

    #include <gui\graph.h>
    #include <gui\window.h>
    #include <machine\machine.h>

    #include <driver\pc_ata.h>
    #include <driver\pc_vga.h>

#endif  /*  _SLASH_     */
#endif  /*  _LENIX_H_   */