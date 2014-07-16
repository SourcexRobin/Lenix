#/*
#/////////////////////////////////////////////////////////////////////////////////////////
#//                                    Lenix ����ϵͳ
#//                              2011 - 2014 @ Դ���빤����
#//                                     �������а�Ȩ
#//                          ***----------------------------***
#//     ��    �� : Makefile
#//     ����ʱ�� : 2014-01-31       ������  : �ޱ�
#//     �޸�ʱ�� :                  �޸���  : 
#//
#//     ��Ҫ���� : ���ڱ�������ӵ����õ���������
#//
#//     ˵    �� : ��VS2005������ʹ�á������Ҫʹ���������빤�ߣ���Ҫ�����޸ġ�
#//
#//  �汾�仯��¼:
#//  �汾��      |    ʱ  ��     |    ��  ��     | ��Ҫ�仯��¼
#//=======================================================================================
#//              |  2014-01-31   |    ��  ��     | ���һд��
#/////////////////////////////////////////////////////////////////////////////////////////
#*/

#
# ������Ҫʹ�õ�·��������·��
#
MPATH   = e:\sourcexstudio\lenix\lenix
INC1    = $(MPATH)\include
INC2    = $(MPATH)\include\kernel
INCDIR  = $(INC1);$(INC2)
LIBDIR  = $(MPATH)\lib
OBJDIR  = $(MPATH)\obj

#
# ������Ҫ�������ͬ�Ŀ������ߣ����Ӧ�������������ͬ
#
CC      = CL.EXE
ASM     = ML.EXE
LIB     = LIB.EXE
LINK    = LINK.EXE
MAKE    = NMAKE.EXE

#
#  ����ĳЩmake���򣬴��ݺ��ʱ����Ҫʹ���ر�Ĳ�����ʶ��������BC31����Ҫʹ��-D
#  �����ʹ�ã����������Ϊ�ռ���
#
MACRO   = 
#
# �������
# Borland C/C++ 3.1 �ı������
#
#CFLAGS =-c -n$(OBJDIR) -I$(INCDIR) -O2

#
# VS2005�������
#
CFLAGS  = /c /Zp1 /O2 /GS- /W3 /I"$(INC1)" /I"$(INC2)" /Fo$(OBJDIR)\ /Fa$(OBJDIR)\  /FAcs


#��Ŀ¼


# �ں������ļ�
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

# ���������е��������obj��libĿ¼��
clean:
	del obj\*.obj
        del obj\*.cod
        del obj\*.asm
        del lib\*.lib
        del lib\*.txt
	