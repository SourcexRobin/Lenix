

INCDIR	= z:\include;z:\include\kernel
LIBDIR  = z:\lib
OBJDIR	= z:\obj

CC	= bcc.exe
CFLAGS	= -c -nobj -I$(INCDIR)


KERLIB  = -I$(INCDIDR) -L$(LIBDIR) arch.lib asm.lib machine.lib libc.lib \
           kernel.lib driver.lib $(OBJDIR)\kernel.obj

OBJS	= obj\demo.obj


target : $(OBJS)
        bcc @cfg.txt


obj\demo.obj : demo.c
    $(CC) $(CFLAGS) demo.c

clean:
    del obj\*.obj

	