

INCDIR	= z:\include;z:\include\kernel
LIBDIR  = z:\lib
OBJDIR	= z:\obj

CC	= bcc.exe
CFLAGS	= -c -nobj -I$(INCDIR) -O2


KERLIB  = -I$(INCDIDR) -L$(LIBDIR) arch.lib asm.lib machine.lib libc.lib \
           kernel.lib gui.lib driver.lib $(OBJDIR)\kernel.obj

OBJS	= obj\demo.obj obj\gui.obj


target : $(OBJS)
        bcc @cfg.txt

obj\demo.obj : demo.c
    $(CC) $(CFLAGS) demo.c

obj\gui.obj : gui.c
    $(CC) $(CFLAGS) gui.c

clean:
    del obj\*.obj

	