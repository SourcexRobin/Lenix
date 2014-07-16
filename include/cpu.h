/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    Ƕ��ʽ����ϵͳ
//                             2011 , ��  ��Դ���빤����
//                                     �������а�Ȩ
//                          ***---------------------------***
//
//  �ļ�����    : cpu.h
//  �ļ�������  :
//  �ļ�����ʱ��:
//  ����޸���  : ��  ��
//  ����޸�ʱ��:
//
//  ��Ҫ����    : �ṩ����CPU�Ļ�������.
//
//  ˵��        : ���ļ������ĺ���������ֲʱ��Ҫʵ�ֵĻ������ܡ�
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
//========================================================================================
//              |   2012-07-03  |  �ޱ�         |  ����IO�ռ�֧��
//              |   2012-06-30  |  �ޱ�         |  �޸�Cpu_tas
//                  2012-06-29  |  �ޱ�         |  �����������룬
//  00.00.000   |   2011-02-09  |  �ޱ�         |  xxxxxx
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
#ifndef _CPU_H_
#define _CPU_H_

#include <config.h>
#include <type.h>


//2012/07/12
/*
typedef struct _cpu_t
{
    psw_t                 (*cpu_get_psw)(void);
    psw_t                 (*cpu_set_psw)(psw_t psw);

    psw_t                 (*cpu_disable_interrupt)(void);
    psw_t                 (*cpu_enable_interrupt)(void);

    byte_t                  cpu_inb (void *);
    word_t                  cpu_inw (void *);
    dword_t                 cpu_ind (void *);
    void                    cpu_outb(void *,byte_t);
    void                    cpu_outw(void *,word_t);
    void                    cpu_outd(void *,dword_t);
    void                *   cpu_inb_buffer  (void *,void *,size_t);
    void                *   cpu_inw_buffer  (void *,void *,size_t);
    void                *   cpu_ind_buffer  (void *,void *,size_t);
    void                *   cpu_outb_buffer (void *,void *,size_t);
    void                *   cpu_outw_buffer (void *,void *,size_t);
    void                *   cpu_outd_buffer (void *,void *,size_t);
    void                    cpu_io_delay(void);

    int                     cpu_tas(int *,int,int);

    void                    cpu_halt(void);
}cpu_t;

extern cpu_t                cpu;
*/
/*
 *  CPU֧��tas��ָ���ǣ�����ú꣬�����ṩ��Ӧ��ʵ�ֺ���
 */
#define _CPU_TAS_

/*
 *  �����ר��Ϊ16λX86�����
 */
#if _CFG_CPU_ == _CFG_CPU_X86_ && _CPU_WORD_ == 16
    #define FAR                     far
#else
    #define FAR
#endif
/*
 *  ���������֧��Ƕ��ʽ��࣬��Lenix�ṩֱ��ʹ��CPU�����жϵ�ָ��
 *  ��ֲʱ����Ҫע���޸��ⲿ�֣������޸���ӦCPU�Ļ��ָ��
 */
#ifdef _CFG_ASM_

    #if _CFG_CPU_ == _CFG_CPU_X86_

        #define CPU_DISABLE_INTERRUPT() asm{ cli }
        #define CPU_ENABLE_INTERRUPT()  asm{ sti }

    #elif _CFG_CPU_ == _CFG_CPU_ARM_

        #define CPU_DISABLE_INTERRUPT() asm{ cli }
        #define CPU_ENABLE_INTERRUPT()  asm{ sti }

    #endif 

#else

    #define CPU_DISABLE_INTERRUPT() Cpu_disable_interrupt()
    #define CPU_ENABLE_INTERRUPT()  Cpu_enable_interrupt()

#endif /*   _CC_ASM_    */

psw_t       Cpu_psw_get(void);                      /*  ���CPU״̬��   */
psw_t       Cpu_psw_set(psw_t psw);                 /*  ����CPU״̬��   */

psw_t       Cpu_disable_interrupt(void);            /*  ��ֹCPU��Ӧ�ж� */
psw_t       Cpu_enable_interrupt(void);             /*  ����CPU��Ӧ�ն� */

/*
 *  IO��ַ�ռ���������͵�CPU��x86ϵ��
 *  ��������CPU��IO������һ����Ҫ������IOָ�C����ͨ����ֱ��֧��������ָ������Ҫͨ��
 *  ʹ�û�����Ա�д�ĳ������ṩ��Ӧ��֧��
 */
byte_t      Io_inb  (void * port);                  /*  ���� 8λ����    */
word_t      Io_inw  (void * port);                  /*  ����16λ����    */
dword_t     Io_ind  (void * port);                  /*  ����32λ����    */

void        Io_outb (void * port,byte_t  dat);      /*  ��� 8λ����    */
void        Io_outw (void * port,word_t  dat);      /*  ���16λ����    */
void        Io_outd (void * port,dword_t dat);      /*  ���32λ����    */

/*
 *  ���������������
 */
void    *   Io_inb_buffer   (void * port,void * buffer,size_t size);
void    *   Io_inw_buffer   (void * port,void * buffer,size_t size);
void    *   Io_ind_buffer   (void * port,void * buffer,size_t size);

void    *   Io_outb_buffer  (void * port,void * buffer,size_t size);
void    *   Io_outw_buffer  (void * port,void * buffer,size_t size);
void    *   Io_outd_buffer  (void * port,void * buffer,size_t size);

int         Io_delay(void);

/*
 *  ��  ��: Cpu_tas_i   : instructer
 *          Cpu_tas_s   : simulate
 *
 *  ��  ��: 
 *
 *  ����ֵ: lck == test������test
 *          lck != test������lck
 *
 *  ˵  ��: ʵ����������ζ��Ƿ���ԭʼ��lck
 *          ��ʹ��ʱ����CPU����ʹ��ģ��ʵ�֡�����Ƕ�CPU������ʹ��CPU�ṩ��Ӳ��ָ��
 */
int         Cpu_tas_i(int * lck,int test,int set);          /*  CPU������Ӧ��ָ��   */
int         Cpu_tas_s(int * lck,int test,int set);          /*  ���ģ��            */

#ifdef _CPU_TAS_
    #define Cpu_tas                 Cpu_tas_i
#else
    #define Cpu_tas                 Cpu_tas_s
#endif

void        Cpu_hlt(void);                          /*  CPUͣ��         */

uint_t *    Context_initial(void * entry,void * param,uint_t * sp); /*  ������ʼ�����  */

void        Cpu_initial(void);

#endif  /*  _CPU_H_ */