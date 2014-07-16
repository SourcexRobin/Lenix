
.386p
.model tiny

Desc            STRUC
LimitL          DW      0 ;段界限(BIT0-15)
BaseL           DW      0 ;段基地址(BIT0-15)
BaseM           DB      0 ;段基地址(BIT16-23)
Attributes      DB      0 ;段属性
LimitH          DB      0 ;段界限(BIT16-19)(含段属性的高4位)
BaseH           DB      0 ;段基地址(BIT24-31)
Desc            ENDS

PDesc           STRUC
Limit           DW      0 ;16位界限
Base            DD      0 ;32位基地址
PDesc           ENDS

public _Pm_initial

_TEXT	segment use16
   ;	
   ;	int         Load_lenix(FILE * lenix)
   ;	
	assume	cs:_TEXT,ds:_TEXT
	org 0100h
start:

_Pm_initial PROC NEAR

    mov ax,cs                                    ;  构造当前运行环境的段选择子
    mov bx,16                                    ;
    mul bx                                       ;  计算当前段的段地址，包括数据段和代码段
    mov word ptr temp_code.BaseL,ax              ;  由于在这个环境中，代码段和数据段相同
    mov byte ptr temp_code.BaseM,dl              ;  可以使用相同段基址
    mov word ptr temp_data.BaseL,ax
    mov byte ptr temp_data.BaseM,dl
    mov bx,offset dummy                          ;  计算GDT地址
    add ax,bx                                    ;
    adc dx,0                                     ;
    mov word ptr p_desc.base,ax                  ;
    mov word ptr p_desc.base + 2,dx              ;


   ;initial pm 保护模式初始化
    lgdt p_desc                                  ;  装填GDT
    cli
    mov eax,cr0                                  ;  打开保护模式，就是将CR0寄存器的0位置1
    or eax,1                                     ;  在开启保护模式后，地址定位方式已经变化
    mov cr0,eax                                  ;  需要根据新的段选择子来取指令，

    db  0eah                                     ;  构造调转指令，完成清空指令预取队列
    dw  offset init                              ;  由于CPU有指令预取功能，因此这条指令会在
    dw  temp_code_sel                            ;  打开保护模式前取得
init:
    
	;
	;  这个时候不是设置段寄存器，而是设置段选择符
	;
    mov ax,data_sel
    mov ds,ax
    mov es,ax
    mov ss,ax
    
	;
	;  构造32位跳转指令，转向32位内核引导程序
	;
    db  66h
    db  0eah
    dd  021000h
    dw  code_sel

_Pm_initial ENDP

align QWORD:

gdt label byte
    dummy DESC <>   ;空描述符
    temp_code DESC <0ffffh,     ,   ,9ah,  0h,0h> ; 8
    temp_data DESC <0ffffh,     ,   ,92h,  0h,0h> ; 16
    code_desc DESC <0ffffh,  00h, 0h,9ah,0cfh,0h> ; 24
    data_desc DESC <0ffffh,  00h, 0h,92h,0cfh,0h> ; 32
    vbuf_desc DESC <0ffffh,8000h,0bh,92h, 40h,0h> ; 40

    gdt_len = $ - gdt
    temp_code_sel = temp_code - gdt
    temp_data_sel = temp_code - gdt
    code_sel = code_desc - gdt
    data_sel = data_desc - gdt
    vbuf_sel = vbuf_desc - gdt  

    p_desc PDesc <gdt_len -1,>
    data_len = $ - gdt
_TEXT	ends
    end start
    
