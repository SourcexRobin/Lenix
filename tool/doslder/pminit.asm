
.386p
.model tiny

Desc            STRUC
LimitL          DW      0 ;�ν���(BIT0-15)
BaseL           DW      0 ;�λ���ַ(BIT0-15)
BaseM           DB      0 ;�λ���ַ(BIT16-23)
Attributes      DB      0 ;������
LimitH          DB      0 ;�ν���(BIT16-19)(�������Եĸ�4λ)
BaseH           DB      0 ;�λ���ַ(BIT24-31)
Desc            ENDS

PDesc           STRUC
Limit           DW      0 ;16λ����
Base            DD      0 ;32λ����ַ
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

    mov ax,cs                                    ;  ���쵱ǰ���л����Ķ�ѡ����
    mov bx,16                                    ;
    mul bx                                       ;  ���㵱ǰ�εĶε�ַ���������ݶκʹ����
    mov word ptr temp_code.BaseL,ax              ;  ��������������У�����κ����ݶ���ͬ
    mov byte ptr temp_code.BaseM,dl              ;  ����ʹ����ͬ�λ�ַ
    mov word ptr temp_data.BaseL,ax
    mov byte ptr temp_data.BaseM,dl
    mov bx,offset dummy                          ;  ����GDT��ַ
    add ax,bx                                    ;
    adc dx,0                                     ;
    mov word ptr p_desc.base,ax                  ;
    mov word ptr p_desc.base + 2,dx              ;


   ;initial pm ����ģʽ��ʼ��
    lgdt p_desc                                  ;  װ��GDT
    cli
    mov eax,cr0                                  ;  �򿪱���ģʽ�����ǽ�CR0�Ĵ�����0λ��1
    or eax,1                                     ;  �ڿ�������ģʽ�󣬵�ַ��λ��ʽ�Ѿ��仯
    mov cr0,eax                                  ;  ��Ҫ�����µĶ�ѡ������ȡָ�

    db  0eah                                     ;  �����תָ�������ָ��Ԥȡ����
    dw  offset init                              ;  ����CPU��ָ��Ԥȡ���ܣ��������ָ�����
    dw  temp_code_sel                            ;  �򿪱���ģʽǰȡ��
init:
    
	;
	;  ���ʱ�������öμĴ������������ö�ѡ���
	;
    mov ax,data_sel
    mov ds,ax
    mov es,ax
    mov ss,ax
    
	;
	;  ����32λ��תָ�ת��32λ�ں���������
	;
    db  66h
    db  0eah
    dd  021000h
    dw  code_sel

_Pm_initial ENDP

align QWORD:

gdt label byte
    dummy DESC <>   ;��������
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
    
