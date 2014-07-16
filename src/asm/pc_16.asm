;/*
;/////////////////////////////////////////////////////////////////////////////////////////
;//                                       Lenix
;//                                   Ƕ��ʽ����ϵͳ
;//                            2011 , ��  ��Դ���빤����
;//                                    �������а�Ȩ
;//                         ***---------------------------***
;//
;//  �ļ�����    : pc_16.asm
;//  �ļ�������  : ��  ��
;//  �ļ������¼�: 2011-08-13
;//  ����޸���  : ��  ��
;//  ����޸�ʱ��:
;//
;//  ��Ҫ����    : �ṩ�޷�ʹ��C����ʵ�ֵĻ�������
;//
;//  ˵��        : 
;//
;//  �汾�仯��¼:
;//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
;//=======================================================================================
;//              |  2013-12-06   |               |  ����com1�жϴ������
;//  00.00.000   |  2011-11-09   |  �ޱ�         |  ��һ�棬��ʵʱ���Ѿ����ǵ�
;//
;/////////////////////////////////////////////////////////////////////////////////////////
;*/
;

PUBLIC      _Irq_clock
PUBLIC      _Irq_keyboard
PUBLIC      _Irq_com1

PUBLIC      _Sys_call_entry

EXTRN       _Proc_fork:PROC
EXTRN       _Syscall_exit:PROC
EXTRN       _Machine_interrupt_mis:PROC

EXTRN       _e0e1
EXTRN       _syscall
EXTRN       _ticks
EXTRN       _machine_ivt
EXTRN       _interrupt_nest

_TEXT   segment byte public 'CODE'
_TEXT   ends

DGROUP  group    _DATA,_BSS
    assume  cs:_TEXT,ds:DGROUP
        
_DATA   segment word public 'DATA'
d@	    label    byte
d@w     label    word
_DATA   ends

_BSS    segment word public 'BSS'
b@     label    byte
b@w    label    word
_BSS    ends

_TEXT   segment byte public 'CODE'

api         PROC NEAR

;/////////////////////////////////////////////////////////////////////////////////////////
;  ʱ���жϴ������
_Irq_clock:
    db      060h ;pusha
    
    push    es
    push    ds
    pop     es
    
    add     word ptr [_ticks    ],1
    adc     word ptr [_ticks + 2],0;
    
    inc     byte ptr _interrupt_nest
    
    mov     al,020h 
    out     020h,al

	cmp     _interrupt_nest,196
	jg      CLOCK_IRQ_MIS

    push    0
    push    0    
    call    _machine_ivt
    add     sp,4
	jmp     CLOCK_IRQ_END

CLOCK_IRQ_MIS:
    call   _Machine_interrupt_mis

CLOCK_IRQ_END:
    dec     byte ptr _interrupt_nest
	push    1
    push    1
    call    _Syscall_exit
    add     sp,4
    
    pop     es
    db      061h ;popa
    iret


;/////////////////////////////////////////////////////////////////////////////////////////
;  �����жϴ������
_Irq_keyboard:
    db      060h ;pusha
    
    push    es
    
    push    ds
    pop     es
    
    inc     byte ptr _interrupt_nest

    xor     ax,ax
    mov     dx,060h
    in      al,dx

	mov     cl,al

    mov     al,020h 
    out     020h,al    

	mov     al,cl

	cmp     _interrupt_nest,196
	jg      KB_IRQ_MIS

    ; ��ü���ɨ����
    cmp     al,0E0h
    je      e0;
    cmp     al,0E1h
    je      e1;

    
    push    ax            ; ���жϴ�����򴫵ݲ���
    push    0
    call    [_machine_ivt + 2]
    add     sp,4
    
	jmp     KB_IRQ_END
    
e0:
    mov     ax,1
    or      _e0e1,ax;
    jmp     e0_e1_end;
e1:
    mov     ax,2
    or      _e0e1,ax
e0_e1_end:    
	jmp     KB_IRQ_END

KB_IRQ_MIS:
	call    _Machine_interrupt_mis;

KB_IRQ_END:
    dec     byte ptr _interrupt_nest
	push    0
    push    1
    call    _Syscall_exit
    add     sp,4

    pop     es

    db      061h ;popa
    iret

;/////////////////////////////////////////////////////////////////////////////////////////
;  com1�жϴ������
COM1_PORT EQU    03F8H

_Irq_com1:
    db      060h ;pusha
    
    push    es
    push    ds
    pop     es
    
    inc     byte ptr _interrupt_nest
    
    xor     bx,bx
    xor     cx,cx
    ; ���ж�״̬
    mov	    dx,COM1_PORT + 2
    in      al,dx
    mov     bl,al
    
    and     al,0Fh
    cmp     al,2
    je      COM1_SEND_EMPTY
    cmp     al,4
    je      COM1_RESV_READY
    
    jmp     COM1_DATA_END
COM1_RESV_READY:
    mov     dx,COM1_PORT
    in      al,dx
    mov     cl,al
    jmp     COM1_DATA_END
    
COM1_SEND_EMPTY:
    ; ���ѵȴ��������ݵĽ���
	xor     cx,cx
COM1_DATA_END:
    mov     al,020h 
    out     020h,al

	cmp     _interrupt_nest,196
	jg      COM1_IRQ_MIS

    push    bx
    push    cx
    call    [_machine_ivt + 4]
    add     sp,4
	jmp     COM1_IRQ_END

COM1_IRQ_MIS:
    call   _Machine_interrupt_mis

COM1_IRQ_END:
    dec     byte ptr _interrupt_nest
	push    1
    push    1
    call    _Syscall_exit
    add     sp,4

    pop     es
    
    db      061h ;popa
    iret


_Sys_call_entry:
    push    di
    push    si
    push    ax
    push    dx
    push    bx
    push    cx
    
    ; �����ӵ��ú�
    xor     dx,dx
    mov     dl,ah
    push    dx
    
    xor     ah,ah
    lea     dx,_syscall
    add     dx,ax
    call    dx
    
    push    ax
	push    0
    push    0
    call    _Syscall_exit;
    add     sp,4
    pop     ax
    add     sp,10
    pop     si
    pop     di  
    iret
    
_Sys_fork:
    ret;
    

api         ENDP

_TEXT       ends
    end