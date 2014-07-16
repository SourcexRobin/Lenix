/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenix实时操作系统
//                              2011 - 2012 @ 源代码工作室
//                                     保留所有版权
//                          ***----------------------------***
//       名  称 : tty.c 
//     创建时间 : 2011-07-12       创建者  : 罗斌
//     修改时间 : 2012-11-24       修改者  : 罗斌
//
//     主要功能 : 提供从字符终端的输入输出功能，作为Lenix默认支持的硬件设备
//
//      说  明  : 
//
//  版本变化记录:
//  版本号      |    时  间     |    作  者     | 主要变化记录
//========================================================================================
//              |  2012-11-25   |    罗  斌     | 增加了命令和命令参数识别，和四个简单的
//                                              | 命令，演示用
//  00.00.000   |  2011-07-26   |    罗  斌     | 第一版
//////////////////////////////////////////////////////////////////////////////////////////
*/


#ifndef _SHELL_H_
#define _SHELL_H_

#include <type.h>
#include <result.h>

#define SHELL_CMD_NAME_SIZE         8
#define SHELL_ARG_MAX               16

typedef int                 (* sc_entry_t)(int argc,char ** argv);

typedef struct _sc_map_t
{
    char                    scm_name[SHELL_CMD_NAME_SIZE];
    sc_entry_t              scm_entry;    
}sc_map_t;

void        Shell_initial(void);
char *      Sc_get_param(int argc,char ** argv,const char param);
result_t    Shell_cmd_add(const char * cmdname,sc_entry_t cmdentry);

#endif /*   _SHELL_H_   */

