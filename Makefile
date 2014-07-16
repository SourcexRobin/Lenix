#/*
#/////////////////////////////////////////////////////////////////////////////////////////
#//                                    Lenix 操作系统
#//                              2011 - 2014 @ 源代码工作室
#//                                     保留所有版权
#//                          ***----------------------------***
#//     名    称 : Makefile
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

#
# 编译需要使用的路径，绝对路径
#
MPATH   = e:\sourcexstudio\lenix\lenix
INC1    = $(MPATH)\include
INC2    = $(MPATH)\include\kernel
INCDIR  = $(INC1);$(INC2)
LIBDIR  = $(MPATH)\lib
OBJDIR  = $(MPATH)\obj

#
# 编译需要的命令，不同的开发工具，其对应的命令会有所不同
#
CC      = CL.EXE
ASM     = ML.EXE
LIB     = LIB.EXE
LINK    = LINK.EXE
MAKE    = NMAKE.EXE

#
#  对于某些make程序，传递宏的时候，需要使用特别的参数标识符，比如BC31，需要使用-D
#  如果不使用，这个宏设置为空即可
#
MACRO   = 
#
# 编译参数
# Borland C/C++ 3.1 的编译参数
#
#CFLAGS =-c -n$(OBJDIR) -I$(INCDIR) -O2

#
# VS2005编译参数
#
CFLAGS  = /c /Zp1 /O2 /GS- /W3 /I"$(INC1)" /I"$(INC2)" /Fo$(OBJDIR)\ /Fa$(OBJDIR)\  /FAcs


#子目录


# 内核依赖文件
OBJS    = obj\startup.obj obj\lenix.obj 

LIBS    = $(LIBDIR)\llibc.lib    \
          $(LIBDIR)\arch.lib     \
          $(LIBDIR)\asm.lib      \
          $(LIBDIR)\machine.lib  \
          $(LIBDIR)\kernel.lib   \
          $(LIBDIR)\gui.lib  	 \
          $(LIBDIR)\driver.lib

DEP     = $(INC1)\config.h $(INC1)\type.h $(INC1)\lenix.h

lenix : $(OBJS) $(LIBS)
        echo make lenix 

$(LIBDIR)\llibc.lib : 
  cd src\libc
  $(MAKE) $(MACRO)CC=$(CC) $(MACRO)LIB=$(LIB) $(MACRO)CFLAGS="$(CFLAGS)"
  cd ..\..

$(LIBDIR)\arch.lib : 
  cd src\arch
  $(MAKE) $(MACRO)CC=$(CC) $(MACRO)LIB=$(LIB) $(MACRO)CFLAGS="$(CFLAGS)"
  cd ..\..

$(LIBDIR)\asm.lib : 
  cd src\asm
  $(MAKE) $(MACRO)ASM=$(CC) $(MACRO)LIB=$(LIB) $(MACRO)CFLAGS="$(CFLAGS)"
  cd ..\..

$(LIBDIR)\machine.lib : 
  cd src\machine
  $(MAKE) $(MACRO)CC=$(CC) $(MACRO)LIB=$(LIB) $(MACRO)CFLAGS="$(CFLAGS)"
  cd ..\..

$(LIBDIR)\kernel.lib : 
  cd src\kernel
  $(MAKE) $(MACRO)CC=$(CC) $(MACRO)LIB=$(LIB) $(MACRO)CFLAGS="$(CFLAGS)"
  cd ..\..

$(LIBDIR)\gui.lib : 
  cd src\gui
  $(MAKE) $(MACRO)CC=$(CC) $(MACRO)LIB=$(LIB) $(MACRO)CFLAGS="$(CFLAGS)"
  cd ..\..

$(LIBDIR)\driver.lib : 
  cd src\driver
  $(MAKE) $(MACRO)CC=$(CC) $(MACRO)LIB=$(LIB) $(MACRO)CFLAGS="$(CFLAGS)"
  cd ..\..

#
obj\lenix.obj: src\lenix.c $(DEP)
  $(CC) $(CFLAGS) src\lenix.c	

obj\startup.obj: src\startup.c $(DEP)
  $(CC) $(CFLAGS) src\startup.c	

# 编译中所有的输出仅在obj和lib目录中
clean:
	del obj\*.obj
        del obj\*.cod
        del obj\*.asm
        del lib\*.lib
        del lib\*.txt
	