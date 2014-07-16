/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: x86.h
//  创建时间: 2012-07-02       创建者: 罗斌
//  修改时间: 2014-02-02       修改者: 罗斌 
//  主要功能: 提供x86CPU的一些基本功能，以及宏转换。
//  说    明: 
//
//  版本变化记录:
//  版本号      |     时间      |  作  者       |  主要变化记录
//=============================================================================
//  00.00.000   |  2012-07-02   |  罗  斌       |  第一版
///////////////////////////////////////////////////////////////////////////////
*/


#ifndef _X86_H_
#define _X86_H_

#include <config.h>
#include <type.h>
#include <cpu.h>

#if _CPU_WORD_ == 32

/*  x86系列CPU的32位保护模式所需的数据类型  */
typedef struct _i386_context_t
{
    //uint_t                ct_dr0;
    //uint_t                ct_dr1;/**/
    //uint_t                ct_dr2;
    //uint_t                ct_dr3;
    //uint_t                ct_dr6;
    //uint_t                ct_dr7;
    uint_t                  ct_edi;
    uint_t                  ct_esi;
    uint_t                  ct_ebp;
    uint_t                  ct_esp;
    uint_t                  ct_edx;
    uint_t                  ct_ebx;
    uint_t                  ct_ecx;
    uint_t                  ct_eax;
    uint_t                  ct_eflags;
}i386_context_t;


// 存储段描述符
typedef struct _memory_segment_descriptor_t
{
    word_t                  msd_limit_low;  /*  段界限低16位            */
    word_t                  msd_base_low;   /*  段基址低16位,0-15       */
    byte_t                  msd_base_mid;   /*  段基址中间8位,16-23     */
    byte_t                  msd_attr;       /*  段属性                  */
    byte_t                  msd_limit_h;    /*  段界限高4位和段属性     */
    byte_t                  msd_base_high;  /*  段基址高8位,24-31       */
}msd_t;


#define MSD_PRESENT                 1

#define MSD_DPL_RING0               0
#define MSD_DPL_RING1               1
#define MSD_DPL_RING2               2
#define MSD_DPL_RING3               3

#define MSD_ATTR_TYPE_A             0x01
#define MSD_ATTR_TYPE_WR            0x02
#define MSD_ATTR_TYPE_EC            0x04
#define MSD_ATTR_TYPE_CODE          0x08
#define MSD_ATTR_AVL                0x10
#define MSD_ATTR_D                  0x40
#define MSD_ATTR_GRAIN_4K           0x80

#define MSD_DUMMY                   0x0000,0x0000,0x00,0x00,0x00,0x00
#define MSD_KERNEL_CODE             0xFFFF,0x0000,0x00,0x9A,0xCF,0x00
#define MSD_KERNEL_DATA             0xFFFF,0x0000,0x00,0x92,0xCF,0x00
#define MSD_USER_CODE               0xFFFF,0x0000,0x00,0xFA,0xCC,0x00
#define MSD_USER_DATA               0xFFFF,0x0000,0x00,0xF2,0xCC,0x00

#define SELECTOR_DUMMY              0
#define SELECTOR_KERNEL_CODE        8
#define SELECTOR_KERNEL_DATA        16
#define SELECTOR_USER_CODE          24+3
#define SELECTOR_USER_DATA          32+3
#define SELECTOR_TSS                40

#define MSD_BASE_GET(msd)           ( ((uint32_t)((msd)->msd_base_high ) * \
                                      0x1000000) + \
                                      ((uint32_t)((msd)->msd_base_mid)   * \
                                      0x10000  ) + \
                                      ((uint32_t)((msd)->msd_limit_low) )
#define MSD_BASE_SET(msd,base)      do{ (msd)->msd_base_low = \
                                        (word_t)((base)& 0xFFFF); \
                                        (msd)->msd_base_mid = \
                                        (byte_t)((base)>>16); \
                                        (msd)->msd_base_high= \
                                        (byte_t)((base)>>24); \
                                      }while(0)
/*  仅低20位有效，配合粒度位才能将界限扩展至4G*/
#define MSD_LIMIT_GET(msd)          ( (uint32_t)(((msd)->msd_limit_h & 0x0F) * \
                                      0x10000)+ (msd)->msd_limit_low )
#define MSD_LIMIT_SET(msd,limit)    do{(msd)->msd_limit_h &= 0xF0; \
                                       (msd)->msd_limit_h |= \
                                       ((limit) >> 16) & 0x0F ; \
                                       (msd)->msd_limit_low = \
                                       (word_t)((limit) & 0xFFFF);\
                                      }while(0)
#define MSD_PRIO_LEVEL_GET
#define MSD_PRIO_LEVEL_SET

// 门描述符
typedef struct _gate_descriptor_t
{
    word_t                  gate_offset_low;    /* 入口偏移低16位   */
    word_t                  gate_selector;      /* 选择子           */
    byte_t                  gate_dcount;        /* 双字计数，也就是参数个数 */
    byte_t                  gate_attr;          /* 属性             */
    word_t                  gate_offset_high;   /* 入口偏移高16位   */
}gate_t;

#define SSD_TYPE_GATE_UNUSED        0x0
#define SSD_TYPE_TSS_16             0x1
#define SSD_TYPE_TSS_LDT            0x2
#define SSD_TYPE_TSS_BUSY_16        0x3
#define GATE_TYPE_CALL_16           0x4
#define GATE_TYPE_TASK              0x5
#define GATE_TYPE_INT_16            0x6
#define GATE_TYPE_TRAP_16           0x7
#define SSD_TYPE_GATE_RSVD0         0x8
#define SSD_TYPE_TSS_32             0x9
#define SSD_TYPE_GATE_RSVD1         0xA
#define SSD_TYPE_TSS_BUSY_32        0xB
#define GATE_TYPE_CALL_32           0xC
#define SSD_TYPE_GATE_RSVD2         0xD
#define GATE_TYPE_INT_32            0xE
#define GATE_TYPE_TRAP_32           0xF

#define GATE_OFFSET_SET(gate,offset)  \
                                    do{(gate)->gate_offset_low=\
                                       (word_t)((offset)&0xFFFF);\
                                       (gate)->gate_offset_high=\
                                       (word_t)((offset)>>16);\
                                      }while(0)
#define GATE_OFFSET_GET(gate)       (((dword_t)((gate)->gate_offset_high)) * \
                                        0x10000 + (gate)->gate_offset_low)

typedef struct _tss_t{
    dword_t                 tss_back_link;  /* 高16位为0 */
    dword_t                 tss_esp0;
    dword_t                 tss_ss0;        /* 高16位为0 */
    dword_t                 tss_esp1;
    dword_t                 tss_ss1;        /* 高16位为0 */
    dword_t                 tss_esp2;
    dword_t                 tss_ss2;        /* 高16位为0 */
    dword_t                 tss_cr3;
    dword_t                 tss_eip;
    dword_t                 tss_eflags;
    dword_t                 tss_eax,tss_ecx,tss_edx,tss_ebx;
    dword_t                 tss_esp,tss_ebp,tss_esi,tss_edi;
    dword_t                 tss_es;         /* 高16位为0 */
    dword_t                 tss_cs;         /* 高16位为0 */
    dword_t                 tss_ss;         /* 高16位为0 */
    dword_t                 tss_ds;         /* 高16位为0 */
    dword_t                 tss_fs;         /* 高16位为0 */
    dword_t                 tss_gs;         /* 高16位为0 */
    dword_t                 tss_ldt;        /* 高16位为0 */
    dword_t                 tss_trace_bitmap; /* bits: trace 0, bitmap 16-31*/
    byte_t                  tss_iomap[256];
    //I387    tss_i387;
}tss_t;

typedef struct _dbg_reg7_t
{
    dword_t                 dr7_l0:1;
    dword_t                 dr7_g0:1;
    dword_t                 dr7_l1:1;
    dword_t                 dr7_g1:1;
    dword_t                 dr7_l2:1;
    dword_t                 dr7_g2:1;
    dword_t                 dr7_l3:1;
    dword_t                 dr7_g3:1;
    dword_t                 dr7_l4:1;
    dword_t                 dr7_g4:1;
    dword_t                 dr7_unused:6;
    dword_t                 dr7_len0:2;
    dword_t                 dr7_rwe0:2;
    dword_t                 dr7_len1:2;
    dword_t                 dr7_rwe1:2;
    dword_t                 dr7_len2:2;
    dword_t                 dr7_rwe2:2;
    dword_t                 dr7_len3:2;
    dword_t                 dr7_rwe3:2;
}dbg_reg7_t;

#endif /** _CPU_WORD_ == 32 */

#if _CPU_WORD_ == 16

typedef struct _context_t
{
    unsigned int            reg_di,reg_si,reg_bp,reg_sp;
    unsigned int            reg_dx,reg_bx,reg_cx,reg_ax;
    unsigned int            reg_ip,reg_cs,reg_flag;
}context_t;


/*  进程切换宏，非常关键    */
#define PROC_SWITCH_TO(next)        do{ asm{ push next  }; \
                                        asm{ pushf      }; \
                                        asm{ push cs    }; \
                                        asm{ lea ax,Proc_switch_to} ;\
                                        asm{ call ax    }; \
                                        asm{ add sp,2   }; }while(0)

#else

typedef struct _context_t
{
    dword_t                 reg_edi,reg_esi,reg_ebp,reg_esp;
    dword_t                 reg_edx,reg_ebx,reg_ecx,reg_eax;
    dword_t                 reg_eip,reg_es,reg_eflag;
}context_t;

/*  进程切换宏，非常关键    */
#define PROC_SWITCH_TO(next)        do{ asm{ push next  }; \
                                        asm{ pushfd     }; \
                                        asm{ push cs    }; \
                                        asm{ call Proc_switch_to} ;\
                                        asm{ add sp,4   }; }while(0)

#endif 

#define Disable_interrupt()         do{ asm{ cli } }while(0)
#define Enable_interrupt()          do{ asm{ sti } }while(0)

#define PM_ENABLE()                 do{ asm mov eax,cr0 \
                                        asm xor eax,0x80000000 \
                                        asm mov cr0,eax }while(0)
#define PM_DISABLE()                do{asm mov eax,cr0 \
                                        asm and eax,0x7FFFFFFF \
                                        asm mov cr0,eax }while(0)


#define DR7_LEN_BYTE                0
#define DR7_LEN_WORD                1
#define DR7_LEN_UNDEF               2
#define DR7_LEN_DWORD               4
    
#define DR7_RWE_EXEC                0
#define DR7_RWE_WRITEDAT            1    
#define DR7_RWE_IOPORT              2
#define DR7_RWE_RWDATA              3

#define I386_EFLAGS_NONE            0x00000000
#define I386_EFLAGS_CF              0x00000001
#define I386_EFLAGS_PF              0x00000004
#define I386_EFLAGS_AF              0x00000010
#define I386_EFLAGS_ZF              0x00000040
#define I386_EFLAGS_SF              0x00000080
#define I386_EFLAGS_TF              0x00000100
#define I386_EFLAGS_IF              0x00000200
#define I386_EFLAGS_DF              0x00000400
#define I386_EFLAGS_OF              0x00000800
#define I386_EFLAGS_IOPL            0x00002000
#define I386_EFLAGS_NT              0x00004000
#define I386_EFLAGS_RF              0x00010000
#define I386_EFLAGS_VM              0x00020000

#define USER_DEFAULT_EFLAGS         I386_EFLAGS_IF

#define I386_CR0_PE                 0x00000001
#define I386_CR0_MP                 0x00000002
#define I386_CR0_EM                 0x00000004
#define I386_CR0_TS                 0x00000008
#define I386_CR0_ET                 0x00000010
#define I386_CR0_PG                 0x80000000

#define I386_LOAD_DS(s)             do{__asm mov eax,s __asm mov ds,ax}while(0)
#define I386_LOAD_ES(s)             do{__asm mov eax,s __asm mov es,ax}while(0)
#define I386_LOAD_FS(s)             do{__asm mov eax,s __asm mov fs,ax}while(0)
#define I386_LOAD_GS(s)             do{__asm mov eax,s __asm mov gs,ax}while(0)
#define I386_LOAD_SS(s)             do{__asm mov eax,s __asm mov ss,ax}while(0)

#define I386_ENABLE_TRACE           do{__asm pushfd __asm pop eax  \
                                       __asm or eax,I386_EFLAGS_TF \
                                       __asm push eax __asm popfd }while(0)


#define RET                         { __asm ret         }
#define IRET                        { __asm iretd       }
#define I386_CLI                    { __asm cli         }
#define I386_STI                    { __asm sti         }

#define I386_ISP_ENTER()            do{ __asm pushad \
                                        __asm push ds __asm push es      \
                                        __asm push fs __asm mov eax,16   \
                                        __asm mov ds,ax __asm mov es,ax  \
                                        __asm mov fs,ax }while(0)
#define I386_ISP_LEAVE()            do{ __asm pop fs __asm pop es \
                                        __asm pop ds \
                                        __asm popad __asm iretd }while(0)

#define I386_REG_DUMP()             do{ __asm pushad \
                                        __asm push gs __asm push fs         \
                                        __asm push es __asm push ds         \
                                        __asm mov eax,cr0 __asm push eax    \
                                        __asm mov eax,cr2 __asm push eax    \
                                        __asm mov eax,cr3 __asm push eax    \
                                        __asm call I386_reg_dump \
                                        __asm add esp,60 }while(0)

#define I386_REG_DUMPE()            do{ __asm pushad    \
                                        __asm push gs __asm push fs\
                                        __asm push es __asm push ds         \
                                        __asm mov eax,cr0 __asm push eax    \
                                        __asm mov eax,cr2 __asm push eax    \
                                        __asm mov eax,cr3 __asm push eax    \
                                        __asm call I386_reg_dumpe \
                                        __asm add esp,60 }while(0)

int         Seg_get_cs(void);
int         Seg_get_ds(void);
int         Seg_get_ss(void);

result_t    I386_msd_set(msd_t * msd,void * base,dword_t limit,
                         uint_t dpl,uint_t attr);
result_t    I386_ssd_set(msd_t * msd,void * base,dword_t limit,
                         uint_t dpl,uint_t type,uint_t attr);
void        I386_gate_set(gate_t * gate,word_t selector,void * offset,
                          uint_t dpl,uint_t dcount,uint_t type);
void        I386_load_gdt(uint_t limit,uint_t offset);
void        I386_load_idt(uint_t limit,uint_t offset);
void        I386_load_ldt(int selector);
void        I386_load_tr(int selector);
void        I386_reg_dump (int cr3,int cr2,int cr0,
                           int ds ,int es ,int fs,int gs,
                           int edi,int esi,int ebp,int esp,
                           int ebx,int edx,int ecx,int eax,
                           int eip,int cs ,int eflags);
void        I386_reg_dumpe(int cr3,int cr2,int cr0,
                           int ds ,int es ,int fs,int gs,
                           int edi,int esi,int ebp,int esp,
                           int ebx,int edx,int ecx,int eax,
                           int err,
                           int eip,int cs ,int eflags);

#endif  /*  _X86_H_ */