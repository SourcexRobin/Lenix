#/*
#/////////////////////////////////////////////////////////////////////////////////////////
#//                                    Lenix 操作系统
#//                              2011 - 2014 @ 源代码工作室
#//                                     保留所有版权
#//                          ***----------------------------***
#//     名    称 : Makefile 运行库自动化编译脚本
#//     创建时间 : 2014-01-31       创建者  : 罗斌
#//     修改时间 :                  修改者  : 
#//
#//     主要功能 : 用于编译和链接调试用的引导程序
#//
#//     说    明 : 在VS2005下正常使用。如果需要使用其他编译工具，需要另行修改。
#//
#//  版本变化记录:
#//  版本号      |    时  间     |    作  者     | 主要变化记录
#//=======================================================================================
#//              |  2014-01-31   |    罗  斌     | 年初一写的
#/////////////////////////////////////////////////////////////////////////////////////////
#*/

# 编译路径，使用相对路径
OBJDIR  = ..\..\obj
LIBDIR  = ..\lib

# 依赖文件
OBJS	=clock.obj device.obj ipc.obj mm.obj koum.obj \
         proc.obj shell.obj sys.obj tty.obj blkbuf.obj

#
# VS2005
#    $(LIB) /OUT:$(LIBDIR)\kernel.lib $(OBJS)
#
# BC31
#    del  $(LIBDIR)\kernel.lib
#    $(LIB) $(LIBDIR)\kernel.lib +clock.obj +device.obj +ipc.obj +mm.obj \
#                                +proc.obj +shell.obj +sys.obj +tty.obj  \
#                                +koum.obj +blkbuf.obj , \
#           $(LIBDIR)\kernel.txt

$(LIBDIR)\kernel.lib : $(OBJS)
    cd $(OBJDIR)
    $(LIB) /OUT:$(LIBDIR)\kernel.lib $(OBJS)
    cd ..\src\kernel

# {$. }
.c.obj:
    $(CC) $(CFLAGS) $<


	