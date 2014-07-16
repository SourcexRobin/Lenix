/*
///////////////////////////////////////////////////////////////////////////////
//                              Lenix实时操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: cpu.h
//  创建时间: 2011-02-09       创建者  : 罗斌
//  修改时间: 2014-02-01       修改者  : 罗斌
//  主要功能: 提供抽象CPU的基本操作.
//  说    明: 该文件声明的函数都是移植时需要实现的基本功能。
//
//  变更记录:
//  版 本 号    |     时间      |  作  者       |  主要变化记录
//=============================================================================
//              |  2014-02-01   |  罗  斌       |  增加移植到32位X86的内容
//              |  2012-07-02   |  罗  斌       |  将Cpu_tas_s移出该文件
//  00.00.000   |  2012-06-26   |  罗  斌       |  修改Cpu_tas函数的返回值
//  00.00.000   |  2011-02-09   |  罗  斌       |  第一版
///////////////////////////////////////////////////////////////////////////////
*/

#include <result.h>
#include <assert.h>
#include <lmemory.h>
#include <lio.h>
#include <arch\x86.h>

void        Proc_exit(int code);

#if _CPU_WORD_ == 32

#define GDT_MAX                     16
#define IDT_MAX                     256

static msd_t                gdt[GDT_MAX] =  { {MSD_DUMMY      },
                                              {MSD_KERNEL_CODE}, /*0级代码段*/
                                              {MSD_KERNEL_DATA}, /*0级数据段*/
                                              {MSD_USER_CODE  }, /*3级代码段*/
                                              {MSD_USER_DATA  }};/*3级数据段*/
                                              
static gate_t               idt[IDT_MAX];

static tss_t                tss = {
    0 ,                 /* backlink 高16位为0   */
    0 ,16,              /* esp0,ss0 高16位为0   */
    0 ,0 ,              /* esp1,ss1 高16位为0   */
    0 ,0 ,              /* esp2,ss2 高16位为0   */
    0 ,                 /* cr3                  */
    0 ,0,               /* eip,eflags           */
    0 ,0 ,0 ,0 ,        /* eax,ecx,edx,ebx  */
    0 ,0 ,0 ,0 ,        /* esp,ebp,esi,edi  */
    16,8 ,16,16,16,16,  /* es,cs,ss,ds,fs,gs 高16位为0 */
    0 ,                 /* ldt 高16位为0       */
    0                   /* trace bitmap         */
                        /* bits: trace 0, bitmap 16-31 */
                        /* I387    tss_i387     */
};

const char              *   i386_fault_name[16] = 
{
    "fault 0: div zero.",
    "fault 1: debug .",
    "fault 2: null .",
    "fault 3: single setup .",
    "fault 4: overflow .",
    "fault 5: bound .",
    "fault 6: invalide opcode .",
    "fault 7: device .",
    "fault 8: double .",
    "fault 9: fpu ob .",
    "fault A: tss .",
    "fault B: segment .",
    "fault C: stack .",
    "fault D: general .",
    "fault E: page .",
    "fault F: fpu ."
};
#endif /* _CPU_WORD_ == 32 */

uint_t *    Context_initial(void * entry,void * param,uint_t * sp)
{
    *--sp   = 0;                                /*  进程退出代码             */
    *--sp   = (uint_t)param;                    /*  进程入口参数             */
    *--sp   = (uint_t)Proc_exit;                /*  进程退出函数入口         */
    *--sp   = USER_DEFAULT_EFLAGS;              /*  EFLAGS 新进程默认允许中断*/
    *--sp   = Seg_get_cs();                     /*  CS  */
    *--sp   = (uint_t)entry;                    /*  EIP  进程入口            */
                                                /*  以下为寄存器环境         */
    *--sp   = 0;                                /*  EAX                      */
    *--sp   = 0;                                /*  ECX                      */
    *--sp   = 0;                                /*  EDX                      */
    *--sp   = 0;                                /*  EBX                      */ 
    *--sp   = 0;                                /*  ESP                      */
    *--sp   = 0;                                /*  EBP                      */
    *--sp   = 0;                                /*  ESI                      */
    *--sp   = 0;                                /*  EDI                      */
                                                /*  运行环境设置结束         */
    return sp;
}


#if _CPU_WORD_ == 32

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: I386_msd_set
//  作  用: 设置存储段描述符。
//  参  数: 
//      msd         : msd_t *       |   存储段描述符指针
//                                  |   Memory Sgement Descriptor
//      base        : void *        |   基址指针
//      limit       : dword_t       |   段界限，大于0xFFFFF时，会自动调整为4K
//                                  | 粒度。
//      dpl         : uint_t        |   描述符优先级 Descriptor Privilege Level
//      attr        : uint_t        |   段属性
//                  MSD_ATTR_TYPE_A |   已访问
//                 MSD_ATTR_TYPE_WR |   数据段时可写，代码段时可读。数据段都是
//                                  | 可读，代码段都能执行，但不能写
//                 MSD_ATTR_TYPE_EC |   数据段是指扩展方向，代码段是指一致代码
//                                  | 段
//                     MSD_ATTR_AVL |   软件可用位
//                       MSD_ATTR_D |   32位标志。代码段采用32位指令，向下扩展
//                                  | 数据段指4G上限，堆栈操作采用esp寄存器
//                MSD_ATTR_GRAIN_4K |   段界限采用4K粒度
//
//  返回值: 成功返回RESULT_SUCCEED,失败返回RESULT_FAILED
//  说  明: 
//
//  变更记录:
//  时  间      |  作者         |  说明
//=============================================================================
//  2014-01-25  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
result_t    I386_msd_set(msd_t * msd,void * base,dword_t limit,
                         uint_t dpl,uint_t attr)
{
    if( NULL == msd)
        return RESULT_FAILED;
    msd->msd_attr       = 0x90;             /*  描述符存在且为存储段    */
    msd->msd_attr      |= (dpl & 3) << 5 ;  /*  设置描述符优先级        */
    msd->msd_attr      |= (byte_t)(attr & 0x0F );
    msd->msd_limit_h   |= (byte_t)(attr & 0xF0 );
    if( limit > 0xFFFFF )
    {
        limit           /= 0x1000;
        msd->msd_limit_h    |= MSD_ATTR_GRAIN_4K;
    }
    MSD_BASE_SET(msd,(uint32_t)base);
    MSD_LIMIT_SET(msd,limit);

    return RESULT_SUCCEED;
}

result_t    I386_ssd_set(msd_t * msd,void * base,dword_t limit,
                         uint_t dpl,uint_t type,uint_t attr)
{
    if( NULL == msd)
        return RESULT_FAILED;
    msd->msd_attr       = 0x80;                 /*  描述符存在且为系统段    */
    msd->msd_attr      |= (dpl & 3) << 5 ;      /*  设置描述符优先级        */
    msd->msd_attr      |= (byte_t)(type & 0x0F );
    msd->msd_limit_h   |= (byte_t)(attr & 0xF0 );
    if( limit > 0xFFFFF )
    {
        limit               /= 0x1000;
        msd->msd_limit_h    |= MSD_ATTR_GRAIN_4K;
    }
    MSD_BASE_SET(msd,(uint32_t)base);
    MSD_LIMIT_SET(msd,limit);

    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: I386_gate_set
//  作  用: 设置门描述符。
//  参  数: 
//      gate        : gate_t *      |   门描述符指针,Gate Descriptor
//      selector    : word_t        |   段选择符
//      offset      : void *        |   处理程序偏移，也就是处理程序地址。与段
//                                  | 选择符构成48位全指针
//      dpl         : uint_t        |   描述符优先级 Descriptor Privilege Level
//      dcount      : uint_t        |   双字参数计数
//      type        : uint_t        |   门描述符类型
//              GATE_TYPE_CALL_16   |   16位调用门
//              GATE_TYPE_TASK      |   任务门
//              GATE_TYPE_INT_16    |   16位中断门
//              GATE_TYPE_TRAP_16   |   16位陷阱门
//              GATE_TYPE_CALL_32   |   32位调用门
//              GATE_TYPE_INT_32    |   32位中断门
//              GATE_TYPE_TRAP_32   |   32位陷阱门
//
//  返回值: 无
//  说  明: 
//
//  变更记录:
//  时  间      |  作者         |  说明
//=============================================================================
//  2014-01-31  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        I386_gate_set(gate_t * gate,word_t selector,void * offset,
                          uint_t dpl,uint_t dcount,uint_t type)
{
    if( NULL == gate )
        return ;
    GATE_OFFSET_SET(gate,(uint32_t)offset);
    gate->gate_selector     = selector;
    gate->gate_dcount       = (byte_t)(dcount & 0x1F);
    gate->gate_attr         = 0x80;/*  描述符存在且为系统段 */
    gate->gate_attr        |= (type & 0x0F) | ((dpl & 0x03)<< 5) ;
}

/*  2014.1.31
//  很特别的函数，通过传递参数的方式来构造中断返回栈环境
//  两个参数自然形成入栈，然后调用程序返回地址也入栈，自然形成了中断返回环境
*/
static NAKED
void        I386_refresh_cs(int eflags,int selector)
{
    __asm iretd;
}

#define I386_REFRESH_CS(_flag,_cs)  do{ __asm push _flag __asm push _cs \
                                        __asm call I386_refresh_cs }while(0)

void        I386_load_gdt(uint_t limit,uint_t offset)
{
    fword_t         pd;

    pd.low_16   = (word_t)limit;
    pd.high_32  = offset;
    __asm lgdt fword ptr pd
}

void        I386_load_idt(uint_t limit,uint_t offset)
{
    fword_t     pd;

    pd.low_16   = (word_t)limit;
    pd.high_32  = offset;
    __asm lidt fword ptr pd
}

NAKED
void        I386_load_ldt(int selector)
{
    __asm
    {
        mov eax,[esp + 4];
        lldt ax
        ret
    }
}

NAKED
void        I386_load_tr(int selector)
{
    __asm
    {
        mov eax,[esp + 4];
        ltr ax
        ret
    }
}

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  系统故障处理部分
*/

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: I386_reg_dump
//  作  用: 寄存器转储。
//  参  数: 常用寄存器值
//  返回值: 无
//
//  说  明: 
//           在发生系统故障时，在故障处理程序开始部分使用该函数显示寄存器内容，
//         以便查找故障参数传递和调用方式为
//             pushfd
//             push     cs
//             push     eip  ; 这里以上，是由系统自动生成
//                           ; 以下是Lenix定义的顺序
//             pushad
//             push     gs
//             push     fs
//             push     es
//             push     ds
//             mov      eax,cr0
//             push     eax
//             mov      eax,cr2
//             push     eax
//             mov      eax,cr3
//             push     eax
//             call     I386_reg_dump
//
//  变更记录:
//  时  间      |  作者         |  说明
//=============================================================================
//  2014-01-31  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        I386_reg_dump(int cr3,int cr2,int cr0,
                          int ds ,int es ,int fs,int gs,
                          int edi,int esi,int ebp,int esp,
                          int ebx,int edx,int ecx,int eax,
                          int eip,int cs ,int eflags)
{
    _printk("\nCS:EIP=%04X:%08X EFLAGS=%08X\n",cs & 0xFFFF,eip,eflags);
    _printk("EAX=%08X ECX=%08X EDX=%08X EBX=%08X\n",eax,ecx,edx,ebx);
    _printk("ESP=%08X EBP=%08X ESI=%08X EDI=%08X\n",esp,ebp,esi,edi);
    _printk("CR0=%08X CR2=%08X CR3=%08X\n",cr0,cr2,cr3);
    _printk("DS=%04X ES=%04X FS=%04X GS=%04X\n",
        ds & 0xFFFF,es & 0xFFFF,fs & 0xFFFF,gs & 0xFFFF);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: I386_reg_dumpe
//  作  用: 寄存器转储。
//  参  数: 常用寄存器值和错误代码
//  返回值: 无
//
//  说  明: 
//           在发生系统故障时，在故障处理程序开始部分使用该函数显示寄存器内容，
//         以便查找故障参数传递和调用方式为
//             pushfd
//             push     cs
//             push     eip 
//             push     err ; 这里以上，是由系统自动生成
//                          ;以下是Lenix定义的顺序
//             pushad
//             push     gs
//             push     fs
//             push     es
//             push     ds
//             mov      eax,cr0
//             push     eax
//             mov      eax,cr2
//             push     eax
//             mov      eax,cr3
//             push     eax
//             call     I386_reg_dump
//
//  变更记录:
//  时  间      |  作者         |  说明
//=============================================================================
//  2014-01-31  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        I386_reg_dumpe(int cr3,int cr2,int cr0,
                           int ds ,int es ,int fs,int gs,
                           int edi,int esi,int ebp,int esp,
                           int ebx,int edx,int ecx,int eax,
                           int err,
                           int eip,int cs ,int eflags)
{
    _printk("\nCS:EIP=%04X:%08X EFLAGS=%08X\n",cs & 0xFFFF,eip,eflags);
    _printk("EAX=%08X ECX=%08X EDX=%08X EBX=%08X\n",eax,ecx,edx,ebx);
    _printk("ESP=%08X EBP=%08X ESI=%08X EDI=%08X\n",esp,ebp,esi,edi);
    _printk("CR0=%08X CR2=%08X CR3=%08X\n",cr0,cr2,cr3);
    _printk("DS=%04X ES=%04X FS=%04X GS=%04X\n",
        ds & 0xFFFF,es & 0xFFFF,fs & 0xFFFF,gs & 0xFFFF);
    _printk("Err code=%08X\n",err);
}
NAKED 
void        I386_fault_div_zero(void)
{
    I386_CLI;
    _printk("%s\n",i386_fault_name[0]);
    for(;;) Cpu_hlt();
}

NAKED 
void        I386_fault_debug(void)
{
    I386_CLI;
    _printk("%s\n",i386_fault_name[1]);
    for(;;) Cpu_hlt();
}

NAKED 
void        I386_fault_single_step(void)
{
    __asm
    {
        pushad
        push gs
        push fs
        push es
        push ds
        mov eax,cr0
        push eax
        mov eax,cr3
        push eax
        call I386_reg_dump
    }
    I386_CLI;
    _printk("%s\n",i386_fault_name[3]);
    for(;;) Cpu_hlt();
}

NAKED 
void        I386_fault_overflow(void)
{
    I386_CLI;
    _printk("%s\n",i386_fault_name[4]);
    for(;;) Cpu_hlt();
}

NAKED 
void        I386_fault_bound(void)
{
    I386_CLI;
    _printk("%s\n",i386_fault_name[5]);
    for(;;) Cpu_hlt();
}

NAKED 
void        I386_fault_invalid_opcode(void)
{
    I386_CLI;
    _printk("%s\n",i386_fault_name[6]);
    for(;;) Cpu_hlt();
}

NAKED 
void        I386_fault_device()
{
    I386_CLI;
    _printk("%s\n",i386_fault_name[7]);
    for(;;) Cpu_hlt();
}

NAKED 
void        I386_fault_double(void)
{
    I386_CLI;
    _printk("%s\n",i386_fault_name[8]);
    for(;;) Cpu_hlt();
}

NAKED 
void        I386_fault_fpu_ob(void)
{
    I386_CLI;
    _printk("%s\n",i386_fault_name[9]);
    for(;;) Cpu_hlt();
}

NAKED 
void        I386_fault_tss(void)
{
    I386_CLI;
    _printk("%s\n",i386_fault_name[0xA]);
    for(;;) Cpu_hlt();
}

NAKED 
void        I386_fault_segment(void)
{
    I386_CLI;
    _printk("%s\n",i386_fault_name[0xB]);
    for(;;) Cpu_hlt();
}

NAKED 
void        I386_fault_stack(void)
{
    I386_CLI;
    _printk("%s\n",i386_fault_name[0xC]);
    for(;;) Cpu_hlt();
}

NAKED 
void        I386_fault_general(void)
{
    I386_CLI;
    _printk("%s\n",i386_fault_name[0xD]);
    for(;;) Cpu_hlt();
}

NAKED 
void        I386_fault_page(void)
{
    I386_CLI;
    I386_REG_DUMPE();
    DEBUG_HLT(i386_fault_name[0xE]);
}

NAKED 
void        I386_fault_fpu(void)
{
    I386_CLI;
    _printk("%s\n",i386_fault_name[0xF]);
    for(;;) Cpu_hlt();
}


/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Ivt_set
//  作  用: 初始化系统故障陷阱门。
//  参  数: 
//      idx         : int       |   存储段描述符指针,Memory Sgement Descriptor
//      offset      : dword_t   |   中断向量（在i386中是中断描述符）的处理程序
//                              | 偏移（地址）
//  返回值: 原处理程序偏移
//  说  明: 将门描述符设置为内核代码段选择符，环0级可访问，无参数，32位中断门
//
//  变更记录:
//  时  间      |  作者         |  说明
//=============================================================================
//  2014-01-31  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
dword_t     Ivt_set(int idx,dword_t offset)
{
    dword_t         po = 0; /* 原处理程序, prev offset   */

    po = GATE_OFFSET_GET(idt + (idx & 0xFF));
    I386_gate_set(idt + (idx & 0xFF),SELECTOR_KERNEL_CODE,(void *)offset,
                  0,0,GATE_TYPE_INT_32);

    return po;
}


/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: I386_sys_trap_initial
//  作  用: 初始化系统故障陷阱门。
//  参  数: 无
//  返回值: 无
//  说  明: 设置为内核代码段选择符，环3级可访问，无参数，32位陷阱门
//
//  变更记录:
//  时  间      |  作者         |  说明
//=============================================================================
//  2014-02-01  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
void        I386_sys_trap_initial(void)
{
    I386_gate_set(idt + 0,SELECTOR_KERNEL_CODE,I386_fault_div_zero,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt + 1,SELECTOR_KERNEL_CODE,I386_fault_debug,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt + 3,SELECTOR_KERNEL_CODE,I386_fault_single_step,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt + 4,SELECTOR_KERNEL_CODE,I386_fault_overflow,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt + 5,SELECTOR_KERNEL_CODE,I386_fault_bound,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt + 6,SELECTOR_KERNEL_CODE,I386_fault_invalid_opcode,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt + 7,SELECTOR_KERNEL_CODE,I386_fault_device,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt + 8,SELECTOR_KERNEL_CODE,I386_fault_double,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt + 9,SELECTOR_KERNEL_CODE,I386_fault_fpu_ob,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt +10,SELECTOR_KERNEL_CODE,I386_fault_tss,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt +11,SELECTOR_KERNEL_CODE,I386_fault_segment,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt +12,SELECTOR_KERNEL_CODE,I386_fault_stack,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt +12,SELECTOR_KERNEL_CODE,I386_fault_general,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt +14,SELECTOR_KERNEL_CODE,I386_fault_page,
        3,0,GATE_TYPE_TRAP_32);
    I386_gate_set(idt +15,SELECTOR_KERNEL_CODE,I386_fault_fpu,
        3,0,GATE_TYPE_TRAP_32);

}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Cpu_initial
//  作  用: 完成X86系列CPU的32位保护模式初始化。
//  参  数: 无
//  返回值: 无
//  说  明: 这里已经假设CPU运行在32位保护模式，并已启用分页。从开机到切换为
//          32位保护模式，再到启用分页，需要另外的引导程序完成。
//
//  变更记录:
//  时  间      |  作者         |  说明
//=============================================================================
//  2014-01-25  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Cpu_initial(void)
{
    /*
     *    重新初始化gdt寄存器。虽然引导程序已经初始化了gdt，但其所填写的地址是
     *  引导程序提供的地址。如果以后的程序需要使用到gdt，就可能造成错误，因此必
     *  须更换为内核的gdt，更换GDT后，需要刷新所有的段选择符
     */
    _printk("CPU pm32 initial...   ");
    I386_load_gdt(GDT_MAX * sizeof(msd_t) - 1,(uint_t)&gdt);
    I386_LOAD_DS(SELECTOR_KERNEL_DATA);
    I386_LOAD_ES(SELECTOR_KERNEL_DATA);
    I386_LOAD_FS(SELECTOR_KERNEL_DATA);
    I386_LOAD_GS(SELECTOR_KERNEL_DATA);
    I386_LOAD_SS(SELECTOR_KERNEL_DATA);
    /*  刷新代码段选择符和标志寄存器  */
    I386_REFRESH_CS(I386_EFLAGS_NONE,SELECTOR_KERNEL_CODE);
    /*  装载局部选择符，因为没有使用，所以实际上装在了一个无效的选择符    */
    I386_load_ldt(0);
    /*  装载TSS段描述符，涉及特权级变化时需要使用  */
    I386_ssd_set(gdt + 5,&tss,sizeof(tss_t),MSD_DPL_RING0,SSD_TYPE_TSS_32,0);
    I386_load_tr(SELECTOR_TSS);
    /*  装载IDT  */
    I386_load_idt(IDT_MAX * sizeof(gate_t) - 1,(uint_t)&idt);   
    /*  初始化系统陷阱  */
    I386_sys_trap_initial();
    _printk("OK\n");
}
#endif /* _CPU_WORD_ == 32 */
