/*
///////////////////////////////////////////////////////////////////////////////
//                              Lenixʵʱ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: cpu.h
//  ����ʱ��: 2011-02-09       ������  : �ޱ�
//  �޸�ʱ��: 2014-02-01       �޸���  : �ޱ�
//  ��Ҫ����: �ṩ����CPU�Ļ�������.
//  ˵    ��: ���ļ������ĺ���������ֲʱ��Ҫʵ�ֵĻ������ܡ�
//
//  �����¼:
//  �� �� ��    |     ʱ��      |  ��  ��       |  ��Ҫ�仯��¼
//=============================================================================
//              |  2014-02-01   |  ��  ��       |  ������ֲ��32λX86������
//              |  2012-07-02   |  ��  ��       |  ��Cpu_tas_s�Ƴ����ļ�
//  00.00.000   |  2012-06-26   |  ��  ��       |  �޸�Cpu_tas�����ķ���ֵ
//  00.00.000   |  2011-02-09   |  ��  ��       |  ��һ��
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
                                              {MSD_KERNEL_CODE}, /*0�������*/
                                              {MSD_KERNEL_DATA}, /*0�����ݶ�*/
                                              {MSD_USER_CODE  }, /*3�������*/
                                              {MSD_USER_DATA  }};/*3�����ݶ�*/
                                              
static gate_t               idt[IDT_MAX];

static tss_t                tss = {
    0 ,                 /* backlink ��16λΪ0   */
    0 ,16,              /* esp0,ss0 ��16λΪ0   */
    0 ,0 ,              /* esp1,ss1 ��16λΪ0   */
    0 ,0 ,              /* esp2,ss2 ��16λΪ0   */
    0 ,                 /* cr3                  */
    0 ,0,               /* eip,eflags           */
    0 ,0 ,0 ,0 ,        /* eax,ecx,edx,ebx  */
    0 ,0 ,0 ,0 ,        /* esp,ebp,esi,edi  */
    16,8 ,16,16,16,16,  /* es,cs,ss,ds,fs,gs ��16λΪ0 */
    0 ,                 /* ldt ��16λΪ0       */
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
    *--sp   = 0;                                /*  �����˳�����             */
    *--sp   = (uint_t)param;                    /*  ������ڲ���             */
    *--sp   = (uint_t)Proc_exit;                /*  �����˳��������         */
    *--sp   = USER_DEFAULT_EFLAGS;              /*  EFLAGS �½���Ĭ�������ж�*/
    *--sp   = Seg_get_cs();                     /*  CS  */
    *--sp   = (uint_t)entry;                    /*  EIP  �������            */
                                                /*  ����Ϊ�Ĵ�������         */
    *--sp   = 0;                                /*  EAX                      */
    *--sp   = 0;                                /*  ECX                      */
    *--sp   = 0;                                /*  EDX                      */
    *--sp   = 0;                                /*  EBX                      */ 
    *--sp   = 0;                                /*  ESP                      */
    *--sp   = 0;                                /*  EBP                      */
    *--sp   = 0;                                /*  ESI                      */
    *--sp   = 0;                                /*  EDI                      */
                                                /*  ���л������ý���         */
    return sp;
}


#if _CPU_WORD_ == 32

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: I386_msd_set
//  ��  ��: ���ô洢����������
//  ��  ��: 
//      msd         : msd_t *       |   �洢��������ָ��
//                                  |   Memory Sgement Descriptor
//      base        : void *        |   ��ַָ��
//      limit       : dword_t       |   �ν��ޣ�����0xFFFFFʱ�����Զ�����Ϊ4K
//                                  | ���ȡ�
//      dpl         : uint_t        |   ���������ȼ� Descriptor Privilege Level
//      attr        : uint_t        |   ������
//                  MSD_ATTR_TYPE_A |   �ѷ���
//                 MSD_ATTR_TYPE_WR |   ���ݶ�ʱ��д�������ʱ�ɶ������ݶζ���
//                                  | �ɶ�������ζ���ִ�У�������д
//                 MSD_ATTR_TYPE_EC |   ���ݶ���ָ��չ���򣬴������ָһ�´���
//                                  | ��
//                     MSD_ATTR_AVL |   �������λ
//                       MSD_ATTR_D |   32λ��־������β���32λָ�������չ
//                                  | ���ݶ�ָ4G���ޣ���ջ��������esp�Ĵ���
//                MSD_ATTR_GRAIN_4K |   �ν��޲���4K����
//
//  ����ֵ: �ɹ�����RESULT_SUCCEED,ʧ�ܷ���RESULT_FAILED
//  ˵  ��: 
//
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
//=============================================================================
//  2014-01-25  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
result_t    I386_msd_set(msd_t * msd,void * base,dword_t limit,
                         uint_t dpl,uint_t attr)
{
    if( NULL == msd)
        return RESULT_FAILED;
    msd->msd_attr       = 0x90;             /*  ������������Ϊ�洢��    */
    msd->msd_attr      |= (dpl & 3) << 5 ;  /*  �������������ȼ�        */
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
    msd->msd_attr       = 0x80;                 /*  ������������Ϊϵͳ��    */
    msd->msd_attr      |= (dpl & 3) << 5 ;      /*  �������������ȼ�        */
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
//  ��  ��: I386_gate_set
//  ��  ��: ��������������
//  ��  ��: 
//      gate        : gate_t *      |   ��������ָ��,Gate Descriptor
//      selector    : word_t        |   ��ѡ���
//      offset      : void *        |   �������ƫ�ƣ�Ҳ���Ǵ�������ַ�����
//                                  | ѡ�������48λȫָ��
//      dpl         : uint_t        |   ���������ȼ� Descriptor Privilege Level
//      dcount      : uint_t        |   ˫�ֲ�������
//      type        : uint_t        |   ������������
//              GATE_TYPE_CALL_16   |   16λ������
//              GATE_TYPE_TASK      |   ������
//              GATE_TYPE_INT_16    |   16λ�ж���
//              GATE_TYPE_TRAP_16   |   16λ������
//              GATE_TYPE_CALL_32   |   32λ������
//              GATE_TYPE_INT_32    |   32λ�ж���
//              GATE_TYPE_TRAP_32   |   32λ������
//
//  ����ֵ: ��
//  ˵  ��: 
//
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
//=============================================================================
//  2014-01-31  |  �ޱ�         |  ��һ��
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
    gate->gate_attr         = 0x80;/*  ������������Ϊϵͳ�� */
    gate->gate_attr        |= (type & 0x0F) | ((dpl & 0x03)<< 5) ;
}

/*  2014.1.31
//  ���ر�ĺ�����ͨ�����ݲ����ķ�ʽ�������жϷ���ջ����
//  ����������Ȼ�γ���ջ��Ȼ����ó��򷵻ص�ַҲ��ջ����Ȼ�γ����жϷ��ػ���
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
//  ϵͳ���ϴ�����
*/

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: I386_reg_dump
//  ��  ��: �Ĵ���ת����
//  ��  ��: ���üĴ���ֵ
//  ����ֵ: ��
//
//  ˵  ��: 
//           �ڷ���ϵͳ����ʱ���ڹ��ϴ������ʼ����ʹ�øú�����ʾ�Ĵ������ݣ�
//         �Ա���ҹ��ϲ������ݺ͵��÷�ʽΪ
//             pushfd
//             push     cs
//             push     eip  ; �������ϣ�����ϵͳ�Զ�����
//                           ; ������Lenix�����˳��
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
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
//=============================================================================
//  2014-01-31  |  �ޱ�         |  ��һ��
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
//  ��  ��: I386_reg_dumpe
//  ��  ��: �Ĵ���ת����
//  ��  ��: ���üĴ���ֵ�ʹ������
//  ����ֵ: ��
//
//  ˵  ��: 
//           �ڷ���ϵͳ����ʱ���ڹ��ϴ������ʼ����ʹ�øú�����ʾ�Ĵ������ݣ�
//         �Ա���ҹ��ϲ������ݺ͵��÷�ʽΪ
//             pushfd
//             push     cs
//             push     eip 
//             push     err ; �������ϣ�����ϵͳ�Զ�����
//                          ;������Lenix�����˳��
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
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
//=============================================================================
//  2014-01-31  |  �ޱ�         |  ��һ��
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
//  ��  ��: Ivt_set
//  ��  ��: ��ʼ��ϵͳ���������š�
//  ��  ��: 
//      idx         : int       |   �洢��������ָ��,Memory Sgement Descriptor
//      offset      : dword_t   |   �ж���������i386�����ж����������Ĵ������
//                              | ƫ�ƣ���ַ��
//  ����ֵ: ԭ�������ƫ��
//  ˵  ��: ��������������Ϊ�ں˴����ѡ�������0���ɷ��ʣ��޲�����32λ�ж���
//
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
//=============================================================================
//  2014-01-31  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
dword_t     Ivt_set(int idx,dword_t offset)
{
    dword_t         po = 0; /* ԭ�������, prev offset   */

    po = GATE_OFFSET_GET(idt + (idx & 0xFF));
    I386_gate_set(idt + (idx & 0xFF),SELECTOR_KERNEL_CODE,(void *)offset,
                  0,0,GATE_TYPE_INT_32);

    return po;
}


/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: I386_sys_trap_initial
//  ��  ��: ��ʼ��ϵͳ���������š�
//  ��  ��: ��
//  ����ֵ: ��
//  ˵  ��: ����Ϊ�ں˴����ѡ�������3���ɷ��ʣ��޲�����32λ������
//
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
//=============================================================================
//  2014-02-01  |  �ޱ�         |  ��һ��
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
//  ��  ��: Cpu_initial
//  ��  ��: ���X86ϵ��CPU��32λ����ģʽ��ʼ����
//  ��  ��: ��
//  ����ֵ: ��
//  ˵  ��: �����Ѿ�����CPU������32λ����ģʽ���������÷�ҳ���ӿ������л�Ϊ
//          32λ����ģʽ���ٵ����÷�ҳ����Ҫ���������������ɡ�
//
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
//=============================================================================
//  2014-01-25  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Cpu_initial(void)
{
    /*
     *    ���³�ʼ��gdt�Ĵ�������Ȼ���������Ѿ���ʼ����gdt����������д�ĵ�ַ��
     *  ���������ṩ�ĵ�ַ������Ժ�ĳ�����Ҫʹ�õ�gdt���Ϳ�����ɴ�����˱�
     *  �����Ϊ�ں˵�gdt������GDT����Ҫˢ�����еĶ�ѡ���
     */
    _printk("CPU pm32 initial...   ");
    I386_load_gdt(GDT_MAX * sizeof(msd_t) - 1,(uint_t)&gdt);
    I386_LOAD_DS(SELECTOR_KERNEL_DATA);
    I386_LOAD_ES(SELECTOR_KERNEL_DATA);
    I386_LOAD_FS(SELECTOR_KERNEL_DATA);
    I386_LOAD_GS(SELECTOR_KERNEL_DATA);
    I386_LOAD_SS(SELECTOR_KERNEL_DATA);
    /*  ˢ�´����ѡ����ͱ�־�Ĵ���  */
    I386_REFRESH_CS(I386_EFLAGS_NONE,SELECTOR_KERNEL_CODE);
    /*  װ�ؾֲ�ѡ�������Ϊû��ʹ�ã�����ʵ����װ����һ����Ч��ѡ���    */
    I386_load_ldt(0);
    /*  װ��TSS�����������漰��Ȩ���仯ʱ��Ҫʹ��  */
    I386_ssd_set(gdt + 5,&tss,sizeof(tss_t),MSD_DPL_RING0,SSD_TYPE_TSS_32,0);
    I386_load_tr(SELECTOR_TSS);
    /*  װ��IDT  */
    I386_load_idt(IDT_MAX * sizeof(gate_t) - 1,(uint_t)&idt);   
    /*  ��ʼ��ϵͳ����  */
    I386_sys_trap_initial();
    _printk("OK\n");
}
#endif /* _CPU_WORD_ == 32 */
