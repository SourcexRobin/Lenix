

//  2013.1.22

#include <config.h>
#include <type.h>
#include <proc.h>

//  2013.1.22
NAKED psw_t Cpu_psw_get(void)
{
    __asm
    {
        pushfd;
        pop eax;
        ret;
    }
}
//  2013.1.22
NAKED psw_t Cpu_psw_set(psw_t psw)
{
    __asm
    {
        push [esp + 4];
        pushfd;
        pop eax;
        popfd;
        ret
    }
}

//  2013.1.22
NAKED psw_t Cpu_disable_interrupt(void)
{
    __asm
    {
        pushfd
        cli
        pop eax
        ret
    }
}
//  2013.1.22
NAKED psw_t Cpu_enable_interrupt(void)
{
    __asm
    {
        pushfd
        sti
        pop eax
        ret
    }
}

/*
 *  IO地址空间独立，典型的CPU是x86系列
 *  这种类型CPU的IO操作，一般需要独立的IO指令，C语言通常不直接支持这样的指令，因此需要通过
 *  使用汇编语言编写的程序来提供相应的支持
 */
NAKED
byte_t      Io_inb  (void * port)
{
    __asm
    {
        xor eax,eax
        mov edx,[esp + 4];
        in al,dx
        ret
    }
}

NAKED
word_t      Io_inw  (void * port)
{
    __asm
    {
        xor eax,eax
        mov edx,[esp + 4];
        in ax,dx
        ret
    }
}

NAKED
dword_t     Io_ind  (void * port)
{
    __asm
    {
        mov edx,[esp + 4];
        in eax,dx
        ret
    }
}

NAKED
void        Io_outb (void * port,byte_t  dat)
{
    __asm
    {
        mov edx,[esp + 4]
        mov al,[esp + 8]
        out dx,al
        ret
    }
}

NAKED
void        Io_outw (void * port,word_t  dat)
{
    __asm
    {
        mov edx,[esp + 4]
        mov ax,[esp + 8]
        out dx,ax
        ret
    }
}

NAKED
void        Io_outd (void * port,dword_t dat)
{
    __asm
    {
        mov edx,[esp + 4]
        mov eax,[esp + 8]
        out dx,eax
        ret
    }
}

NAKED
void    *   Io_inw_buffer   (void * port,void * buffer,size_t size)
{
    __asm
    {
        cld
        mov edx,[esp +  4];
        mov edi,[esp +  8];
        mov ecx,[esp + 12]
        rep insw
        ret
    }
}

NAKED
void    *   Io_outw_buffer   (void * port,void * buffer,size_t size)
{
    __asm
    {
        cld
        mov edx,[esp +  4];
        mov esi,[esp +  8];
        mov ecx,[esp + 12]
        rep outsw
        ret
    }
}
NAKED
void        Cpu_hlt(void)
{
    __asm
    {
        hlt
        ret
    }
}

NAKED
int         Cpu_tas_i(int * lck,int test,int set)
{
    __asm
    {
        mov ecx,[esp + 4]
        mov eax,[esp + 8]
        mov edx,[esp + 12]
        lock cmpxchg [ecx],edx
        ret
    }
}

NAKED
int         Seg_get_cs(void)
{
    __asm
    {
        xor eax,eax
        mov ax,cs
        ret
    }
}

NAKED
int         Seg_get_ds(void)
{
    __asm
    {
        xor eax,eax
        mov ax,cs
        ret
    }
}
NAKED
int         Seg_get_ss(void)
{
    __asm
    {
        xor eax,eax
        mov ax,cs
        ret
    }
}

NAKED
void        Proc_switch_to(uint_t cs,uint_t eflags,proc_t * next)
{
    __asm
    {
        pushad
        mov ebx,esp
        mov eax,[esp + 44]
        push eax;
        push ebx
        call Proc_switch_prepare
        mov esp,eax
        popad
        iretd;
    }
}


dword_t    Ivt_get(int id)
{
    return 0;
}

void Sys_call_entry(void)
{
}