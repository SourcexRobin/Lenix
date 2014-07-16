


#include "stdio.h"

/*
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
*/

typedef struct _msd_t
{
    unsigned short      LimitL;
    unsigned short      BaseL;
    unsigned char       BaseM;
    unsigned char       Attributes;
    unsigned char       LimitH;
    unsigned char       BaseH;
}msd_t;

typedef struct _pd_t
{
    unsigned short      Limit;
    unsigned long       Base;
}pd_t;

/*
    dummy DESC <>   ;空描述符
    temp_code DESC <0ffffh,     ,   ,9ah,  0h,0h> ; 8
    temp_data DESC <0ffffh,     ,   ,92h,  0h,0h> ; 16
    code_desc DESC <0ffffh,  00h, 0h,9ah,0cfh,0h> ; 24
    data_desc DESC <0ffffh,  00h, 0h,92h,0cfh,0h> ; 32
    vbuf_desc DESC <0ffffh,8000h,0bh,92h, 40h,0h> ; 40
*/
msd_t       gdt[8] = {{0,0,0,0,0,0},
{0xFFFF,0,0,0x9A,0,0},
{0xFFFF,0,0,0x92,0,0},
{0xFFFF,0,0,0x9A,0xCF,0},
{0xFFFF,0,0,0x92,0xCF,0},
{0xFFFF,0x8000,0x0B,0x92,0x40,0}
};

pd_t        pd;
int         Load_lenix(FILE * lenix)
{
    lenix = lenix;
    return -1;
}
int         Get_cs(void)
{
    int retval;
    asm{
        mov ax,cs;
        mov retval,ax
    }
    return retval;
}
int         Get_ds(void)
{
    int retval;
    asm{
        mov ax,ds;
        mov retval,ax
    }
    return retval;
}

#define LGDT(pd)  asm lea ax,pd asm lgdt ax
//  换算出gdt在cs寄存器下的地址
void        Pm_initial(void)
{
    unsigned delta, cs,ds;

    cs = Get_cs();
    ds = Get_ds();

    delta = ds - cs;

    //  构造GDT
    gdt[1].BaseL = (unsigned)(cs * 16 );
    gdt[1].BaseM = (unsigned char)(cs >> 12);
    gdt[2].BaseL = (unsigned)(cs * 16 );
    gdt[2].BaseM = (unsigned char)(cs >> 12);

    pd.Limit    = sizeof(gdt) - 1;
    pd.Base     = (unsigned long)gdt + (unsigned long)cs * 16 + (unsigned long)delat;
    printf("%04X,%04X\n",cs,ds);
pm_init:
    asm
    {
        mov
    }
pm32:
    asm
    {
        mov ax,24 ;
    }
}
int main(int argc,char ** argv)
{
    FILE * lenix = (void *)0;
    char * name = "lenix";

    if( argc > 1 )
        name = argv[1];
    if( NULL == (lenix = fopen(name,"rb") ) )
    {
        printf("can not found file: %s\n",name);
        //return 1;
    }
    Load_lenix(lenix);

    Pm_initial();

    return 0;
}