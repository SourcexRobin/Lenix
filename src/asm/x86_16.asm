;/*
;/////////////////////////////////////////////////////////////////////////////////////////
;//                                       Lenix
;//                                   嵌入式操作系统
;//                            2011 , 罗  斌，源代码工作室
;//                                    保留所有版权
;//                         ***---------------------------***
;//
;//  文件名称    : x86_16.asm
;//  文件创建人  : 罗  斌
;//  最后修改者  : 罗  斌
;//  最后修改时间:
;//
;//  主要功能    : 提供无法使用C语言实现的基本功能
;//
;//  说明        : 
;//
;//  版本变化记录:
;//  版本号      |     时间      |  作者         |  主要变化记录
;//=======================================================================================
;//  00.00.000   |  2011-11-09   |  罗斌         |  第一版，其实时间已经不记得
;//
;/////////////////////////////////////////////////////////////////////////////////////////
;*/
; 
;


; CPU相关的功能
PUBLIC      _Seg_get_cs
PUBLIC      _Seg_get_ds
PUBLIC      _Seg_get_ss

PUBLIC      _Cpu_hlt
PUBLIC      _Cpu_tas_i

PUBLIC      _Cpu_disable_interrupt
PUBLIC      _Cpu_enable_interrupt

PUBLIC      _Cpu_psw_set
PUBLIC      _Cpu_psw_get

PUBLIC      _Io_inb
PUBLIC      _Io_outb

PUBLIC      _Io_inb_buffer
PUBLIC      _Io_outb_buffer

PUBLIC      _Io_inw
PUBLIC      _Io_outw

PUBLIC      _Io_inw_buffer
PUBLIC      _Io_outw_buffer

PUBLIC      _Io_ind
PUBLIC      _Io_outd

PUBLIC      _Ivt_set
PUBLIC      _Ivt_get

PUBLIC      _Proc_switch_to

EXTRN       _Proc_switch_prepare:PROC

_TEXT    segment byte public 'CODE'
_TEXT    ends

DGROUP    group    _DATA,_BSS
    assume    cs:_TEXT,ds:DGROUP
_DATA    segment word public 'DATA'
d@      label    byte
d@w     label    word
_DATA   ends


_BSS    segment word public 'BSS'
b@      label    byte
b@w     label    word
_BSS    ends
    
;    byte_t    Inb(word_t port)
_TEXT    segment byte public 'CODE'

api        PROC NEAR

_Seg_get_cs:
    mov     ax,cs
    ret
_Seg_get_ds:
    mov     ax,ds
    ret
_Seg_get_ss:
    mov     ax,ss
    ret
    
_Cpu_hlt:
    db      0f4h
    ret
    
;int         Cpu_tas_i(int * lck,int test,int set);          
_Cpu_tas_i:
    mov     bx,sp
    mov     cx,[bx + 2]
    mov     ax,[bx + 4]
    mov     dx,[bx + 6]
    mov     bx,cx
    ;lock cmpxchg [bx],dx
    db      0f0h
    dw      0b10fh
    db      017h
    ret
    dd      0h
    
;psw_t      Cpu_disable_interrupt();
_Cpu_disable_interrupt:
    pushf
    pop     ax
    cli
    ret
    
_Cpu_enable_interrupt:
    pushf
    pop     ax
    sti
    ret
        
;   uint_t        Cpu_psw_set(uint_t flag)
_Cpu_psw_set:
    mov     bx,sp
    push    word ptr [bx + 2]
    pushf
    pop     ax
    popf
    ret
    
_Cpu_psw_get:
    pushf
    pop     ax
    ret
        
;    byte_t    Io_inb(word_t port)
_Io_inb:
    mov     bx,sp
    xor     ax,ax;
    mov     dx,[bx + 2];
    in      al,dx
    ret
    
;   void    Io_outb(word_t port,byte_t dat)
_Io_outb:
    mov    bx,sp    
    mov    dx,[bx + 2]
    mov    al,[bx + 4];
    out    dx,al
    ret

_Io_ind:
    mov     bx,sp
    mov     dx,[bx + 2];
    ; in eax,dx
    db 066h
    db 0edh
    mov     bx,ax
    mov     cl,16
    ; shr eax,cl
    db 066h
    db 0d3h
    db 0e8h
    mov dx,ax
    mov ax,bx
    ret
    
_Io_outd:
    ret
    
;   void    *   Io_inb_buffer   (void * port,void * buffer,size_t size);
_Io_inb_buffer:
    mov     bx,sp
    cld
    push    di
    mov     dx,[bx + 2]
    mov     di,[bx + 4]
    mov     cx,[bx + 6]
    ;rep     insb ; bcc编译器不支持该指令，手动构造
    db      0F3h
    db      06Ch
    pop     di
    ret

;   void    *   Io_outb_buffer   (void * port,void * buffer,size_t size);
_Io_outb_buffer:
    mov     bx,sp
    cld
    push    si
    mov     dx,[bx + 2]
    mov     si,[bx + 4]
    mov     cx,[bx + 6]
    ;rep     outsb
    db      0F3h
    db      06Eh
    pop     si
    ret


;    byte_t    _Io_inw(word_t port)
_Io_inw:
    mov     bx,sp
    xor     ax,ax;
    mov     dx,[bx + 2];
    in      ax,dx
    ret
    
;   void    Io_outw(word_t port,word_t dat)
_Io_outw:
    mov    bx,sp    
    mov    dx,[bx + 2]
    mov    ax,[bx + 4];
    out    dx,ax
    ret

;   void    *   Io_inw_buffer   (void * port,void * buffer,size_t size);
_Io_inw_buffer:
    mov     bx,sp
    cld
    push    di
    mov     dx,[bx + 2]
    mov     di,[bx + 4]
    mov     cx,[bx + 6]
    ;rep     insw ; bcc编译器不支持该指令，手动构造
    db      0F3h
    db      06Dh
    pop     di
    ret

;   void    *   Io_outw_buffer   (void * port,void * buffer,size_t size);
_Io_outw_buffer:
    mov     bx,sp
    cld
    push    si
    mov     dx,[bx + 2]
    mov     si,[bx + 4]
    mov     cx,[bx + 6]
    ;rep     outsw
    db      0F3h
    db      06Fh
    pop     si
    ret

;    dword_t    Ivt_set(int id,dword_t    handle)
_Ivt_set:
    mov     bx, sp
    push    di
    mov     cx,[bx + 4];
    mov     di,[bx + 6];
    
    mov     ax,[bx + 2];
    shl     ax,2
    mov     bx,ax
    
    push    es
    xor     ax,ax
    mov     es,ax
    
    mov     ax,es:[bx    ]
    mov     dx,es:[bx + 2];
    cli
    mov     es:[bx  ],cx
    mov     es:[bx+2],di
    sti
    
    pop     es
    pop     di
    ret


;    dword_t    Ivt_get(int id)
_Ivt_get:
    mov     bx, sp
    
    mov     ax,[bx + 2];
    shl     ax,2
    mov     bx,ax
    
    push    es
    xor     ax,ax
    mov     es,ax
    
    cli
    mov     ax,es:[bx    ]
    mov     dx,es:[bx + 2];
    sti
    
    pop     es
    ret


; void    Proc_switch_to(cs,flag,proc_t * next)
_Proc_switch_to:
    db      060h ;pusha
    
    mov     bx,sp
    mov     ax,[bx + 22]
    push    ax                ;  在这里传递参数要注意，
    push    bx                ;  这里是要传递保存环境的栈指针，而不是当前栈指针
    call    _Proc_switch_prepare
    mov     sp,ax
    
    db      061h ;popa
    
    iret

api        ENDP

_TEXT    ends
    end

