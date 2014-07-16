/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    嵌入式操作系统
//                             2011 , 罗  斌，源代码工作室
//                                     保留所有版权
//                          ***---------------------------***
//
//  文件名称    : pc_vga.h
//  文件创建者  : 罗  斌
//  文件创建时间:
//  最后修改者  : 罗  斌
//  最后修改时间:
//
//  主要功能    : 提供基本的内存操作函数。
//
//  说明        : 
//
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//========================================================================================
//              |   2012-06-29  |  罗斌         |  第一版。框架已经测试，尚未实现具体功能
//////////////////////////////////////////////////////////////////////////////////////////
*/


#ifndef _PC_VGA_H_
#define _PC_VGA_H_

#include <config.h>
#include <type.h>

#ifdef _SLASH_
    #include <kernel/device.h>
#else
    #include <kernel\device.h>
#endif  /*  _SLASH_ */

typedef struct _vga_t
{
    void FAR            *   vga_buffer;
    byte_t                  vga_mode;           /*  显示模式    */
    byte_t                  vga_attr;
    uint16_t                vga_scale_x;        /*  横轴分辨率  */
    uint16_t                vga_scale_y;        /*  纵轴分辨率  */
}vga_t;

#define VGA_CMD_UNDEF               0
#define VGA_CMD_SET_ATTR            1

result_t    Vga_entry(device_t * device,int flag,void * param);

#endif  /*  _PC_VGA_H_  */