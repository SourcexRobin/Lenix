/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    Ƕ��ʽ����ϵͳ
//                             2011 , ��  ��Դ���빤����
//                                     �������а�Ȩ
//                          ***---------------------------***
//
//  �ļ�����    : cpu.c
//  �ļ�������  : ��  ��
//  �ļ�����ʱ��: 2012-02-29
//  ����޸���  : 
//  ����޸�ʱ��:
//
//  ��Ҫ����    : �ṩ������CPU��������
//
//  ˵��        :
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ��  ��      | ��Ҫ�仯��¼
//========================================================================================
//  00.00.000   |   2012-06-29  |  ��  ��      | ��һ�棬��ԭ���ֲ��������ļ��Ĺ��ܼ���
//
//////////////////////////////////////////////////////////////////////////////////////////
*/

//  2012/6/29

#include <cpu.h>
#include <assert.h>

#ifndef _CFG_SMP_ 
#if _CFG_CRITICAL_METHOD_ == 1
#endif
#endif 

/*
 *  �ٽ��Ƕ�ײ�����������255�㣬����Ϊ����Ƕ��ʽϵͳ��˵���Ѿ��㹻��
 *  ����ʵ�ֲ�֧��PSWֱ�Ӳ����жϵ�CPU���ٽ�Ρ�
 */
byte_t critical_nest = 0;


int         Io_delay()
{
    volatile int i = 0;

    return ++i;
}


#ifndef _CFG_SMP_ 
/*
 *  ģ�ⷽʽʵ�ֲ��Բ���λ(test and set)���ܣ�ֻ�����ڵ�CPU������
 */
int         Cpu_tas_s(int * lck,int test,int set)
{
#if _CFG_CRITICAL_METHOD_ == 1
    
    Cpu_disable_interrupt();

    ++critical_nest;

    ASSERT(critical_nest < 255);

    if( test == *lck )
        *lck  = set;
    else
        test = *lck;

    if( --critical_nest == 0 )
        Cpu_enable_interrupt();
#else
    psw_t psw;

    psw = Cpu_disable_interrupt();

    if( test == *lck )
        *lck  = set;
    else
        test = *lck;

    Cpu_psw_set(psw);

#endif

    return test;
}

#endif  /*  _CFG_SMP_   */


#ifndef _CFG_IO_SPACE_  
/*
 *  IO��ַ�ռ���ڴ��ַ���
 *  ��������CPU��IO����ʹ��ͨ�����ڴ淽λָ����в�����ʵ����Ҳ����C���Ե�ָ�����
 *  
 *  �����ṩ����һ�ַ��ʷ�ʽ������в�ͬ�ķ��ʷ�ʽ������ֲ��ʱ��Ӧ�ý�����Ӧ���޸�
 *
 *  ����ĳЩ��������֧��C����ʹ��inline�ؼ���(Borland C/C++ 3.1)����˽��ⲿ�ֵ�������
 *  Դ�ļ��У�ԭ���ƻ�����ͷ�ļ���
 */
byte_t      Io_inb(void * port) 
{ 
    return *(byte_t  *)port;  
}

word_t      Io_inw(void * port) 
{ 
    return *(word_t  *)port;  
}
dword_t     Io_ind(void * port) 
{ 
    return *(dword_t *)port;  
}
void        Io_outb (void * port,byte_t  dat)   
{ 
    *(byte_t  *)port = dat;   
}
void        Io_outw (void * port,word_t  dat)   
{ 
    *(word_t  *)port = dat;   
}
void        Io_outd (void * port,dword_t dat)   
{ 
    *(dword_t *)port = dat;   
}

/*
 *  ���������������
 */
void *      Io_inb_buffer(void * port,void * buffer,size_t size)
{
    byte_t              *   buf = buffer;
    while(size--)
        *buf++ = *(byte_t *)port;
    return  buffer;
}
void *      Io_inw_buffer(void * port,void * buffer,size_t size)
{
    word_t              *   buf = buffer;
    while(size--)
        *buf++ = *(word_t *)port;
    return  buffer;
}
void *      Io_ind_buffer(void * port,void * buffer,size_t size)
{
    dword_t             *   buf = buffer;
    while(size--)
        *buf++ = *(dword_t *)port;
    return  buffer;

}

void *      Io_outb_buffer(void * port,void * buffer,size_t size)
{
    byte_t              *   buf = buffer;
    while(size--)
        *(byte_t *)port = *buf++;
    return  buffer;

}
void *      Io_outw_buffer(void * port,void * buffer,size_t size)
{
    word_t              *   buf = buffer;
    while(size--)
        *(word_t *)port = *buf++;
    return  buffer;
}
void *      Io_outd_buffer(void * port,void * buffer,size_t size)
{
    dword_t             *   buf = buffer;
    while(size--)
        *(dword_t *)port = *buf++;
    return  buffer;
}

#endif  /*  _CFG_IO_SPACE_*/
