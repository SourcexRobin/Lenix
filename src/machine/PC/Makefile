
INC	= ..\..\..\include
INCDIR	= ..\..\..\include;..\..\..\include\kernel

CC	= bcc.exe
CFLAGS	= -c -n..\..\..\obj -I$(INCDIR) -O2

OBJS	=consol.obj keyboard.obj pc.obj pc_debug.obj com.obj

DEP     = $(INC)\config.h $(INC)\type.h

target : $(OBJS)  
	echo target machine: PC.

#    copy *.obj ..\..\obj


.c.obj:
    $(CC) $(CFLAGS) {$. }


clean:
    del *.obj

	