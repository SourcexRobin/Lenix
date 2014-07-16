

INCDIR	= z:\include;z:\include\kernel
LIBDIR  = z:\lib
OBJDIR	= z:\obj

CC	= bcc.exe
CFLAGS	= -c -nobj -I$(INCDIR) -O2


KERLIB  = -I$(INCDIDR) -L$(LIBDIR) arch.lib asm.lib machine.lib libc.lib \
           kernel.lib driver.lib $(OBJDIR)\kernel.obj

OBJS	= obj\demo.obj


target : $(OBJS)
        echo -nbin -nbin -edemo -Lz:\lib arch.lib asm.lib machine.lib llibc.lib kernel.lib driver.lib >>cfg.txt
        echo z:\obj\lenix.obj obj\demo.obj >>cfg.txt
        #bcc @cfg.txt
        #del cfg.txt


obj\demo.obj : demo.c
    $(CC) $(CFLAGS) demo.c

clean:
    del obj\*.obj

	