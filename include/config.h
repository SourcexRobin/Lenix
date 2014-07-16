/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: config.h
//  ����ʱ��: 2012-01-09        ������: �ޱ�
//  �޸�ʱ��:                   �޸���: 
//  ��Ҫ����: �ṩLenix�����������
//
//  ˵    ��: ���ļ��ṩ��ϵͳʱ�ӡ���ʱ�����ܡ�
//            ϵͳʱ���ṩ�˻�ȡ����ʱ�䡢��ʱ������ʱ���жϻص������Ĺ��ܡ�
//            ��ʱ���ṩ�˴��������ٹ��ܡ�
//
//  �仯��¼:
//  �� �� ��    |   ʱ  ��    |   ��  ��    |  ��Ҫ�仯��¼
//=============================================================================
//              | 2012-11-27  |   ��  ��    |  �����������
//              | 2012-01-09  |   ��  ��    |  
///////////////////////////////////////////////////////////////////////////////
*/


#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <const.h>

#define VER_MAJOR                   1
#define VER_MINOR                   65
/*
 *  ���ñ�����
 *  Windows��ʹ�á�\����Ϊ·���ָ�����Linux��Unix��ϵͳʹ�á�/����Ϊ·���ָ���
 *  ʹ�������������Ӧ��ͬ�ı��뻷���������˸ú���ʹ�á�/����Ϊ�ָ�������Ӧ��
 *  Unix��Linux�µı��롣û�ж���ú꣬��������Windows�±��롣���Ϊ
#ifdef _SLASH_

#else

#endif  /*  _SLASH_ * /
 *
#define _SLASH_
/*****************************************************************************/

/*
 *  ΢��˾�������Ŀ���
 */
#define _CFG_MSVC_
/*****************************************************************************/

#ifdef _CFG_MSVC_
#define asm                         __asm
#define NAKED                       __declspec( naked )
#else
#define NAKED
#endif
/*
 *  ����CPU����
 */
#define _CFG_CPU_X86_               0
#define _CFG_CPU_ARM_               1
#define _CFG_CPU_PPC_               2

#define _CFG_CPU_                   _CFG_CPU_X86_
/*
 *  �������Ƿ�֧��Ƕ��ʽ���
 */
#define _CFG_ASM_

/*
 *  ������IO��ַ�ռ�
 */
#define _CFG_IO_SPACE_
/*****************************************************************************/

/*
 *  ����CPU�ֳ����Լ������޶�
 *  Ĭ���ֳ�Ϊ32λ
 */
#define _CPU_WORD_                  32

#if _CPU_WORD_ == 8

#define CPU_WORD                    8
#define INT_MIN                     0x80
#define INT_MAX                     0x7F
#define UINT_MAX                    0xFF

#elif _CPU_WORD_ == 16

#define CPU_WORD                    16
#define INT_MIN                     0x8000
#define INT_MAX                     0x7FFF
#define UINT_MAX                    0xFFFF

#elif _CPU_WORD_ == 64

#define CPU_WORD                    64
#define INT_MIN                     0x8000000000000000
#define INT_MAX                     0x7FFFFFFFFFFFFFFF
#define UINT_MAX                    0xFFFFFFFFFFFFFFFF

#else

#define CPU_WORD                    32
#define INT_MIN                     0x80000000
#define INT_MAX                     0x7FFFFFFF
#define UINT_MAX                    0xFFFFFFFF

#endif


/*
 *  ����Ŀ�����
 */
#define _CFG_MACHINE_PC_

/*
 *  ���Կ���
 */
#define _CFG_DEBUG_
/*****************************************************************************/

/*
 *  ����CPU�ܹ���Ĭ��Ϊ��CPU
 *
#define _CFG_SMP_ 
/*****************************************************************************/

/*
 *  �ٽ��ʵ�ַ���
 *  ��CPU
 *  0 : Ƕ�׹��жϣ�ʹ��CPU�ṩ�ķ�������֤Ƕ�׺��Կ���ά��ԭ��״̬
 *  1 : �򵥹��жϣ�ֱ��ʹ��CPU�Ĺ��ж�ָ�
 *  2 : ��ֹ��ռ
 *  SMP
 *  0 : ����������
 *  1 : δ����
 *  2 : ��ֹ��ռ����δ���в���
 *
 *  Ĭ��Ϊ����0��
 *    ��ע�⣬ԭ����Ҫ�����ٽ��Ƕ�ס����纯��A�е��ٽ�ε����˺���B�� ������B
 *  ��Ҳ�����ٽ�Ρ���Ȼ�ⲻ�Ǿ��Խ�ֹ.
 */

#define _CFG_CRITICAL_METHOD_       0
/*****************************************************************************/


#define INLINE                      inline

/*
 *
 
#define _CFG_MACHINE_INITIAL_HOOK_DEFAULT_
#define _CFG_MACHINE_CLOCK_FREQUENCY_HOOK_DEFAULT_
*/

/*
 *  ջ���򿪹�
 *
 #define STACK_DIRECT_HIGH
/*****************************************************************************/

/*
 *  ���ý��̵��û���չ����
 *
#define _CFG_PROC_USER_EXT_
/*****************************************************************************/

/*
 *  ���ý����л�����
 *
#define _CFG_SWITCH_PREPARE_HOOK_
/*****************************************************************************/


/*
 *  ջ��鿪��
 */
#define CHECK_STACK
#define _CFG_CHECK_STACK_
/*****************************************************************************/

/*
 *  ������鿪��
 */
#define CHECK_PARAMETER    
#define _CFG_CHECK_PARAMETER_

/*  ����ʱ��鿪��  */
#define _CFG_RUN_TIME_CHECK_
/*****************************************************************************/


/*
 *  ϵͳĬ��ʱ��Ƶ�� ��/��
 */
#define DEFAULT_CLOCK_FREQUENCY     1000

/*
 *  Ĭ��ʱ��Ƭ50����
 */
#define DEFAULT_CPU_TIME            50



#define IDLE_DEFAULT_STACK_BOTTOM   ((void *)(0xFC00))
#define IDLE_DEFAULT_STACK_SIZE     1020

#define SEMA_LIMIT                  30000
#define MSG_USER_MASK               0x8000
#define MSG_TAKE_TIMEOUT            100
#define MSG_TAKE_RETRY_TIMES        1
/*
 *  ��TTY�豸��ϵͳĬ�ϵ���������豸
 */
#define TTY_MAJOR                   0
/*
 *  Զ�̵���TTY,Remote Debug
 */
#define TTY_RDB                     1
/*
 *  ��ǽ��̵����ȼ�����������������Ҫ�޸�����������
 */
#define SHELL_PROCESS_PRIORITY      32
#define SHELL_PROCESS_PRIONUM       3
/*
 *  ϵͳ��߼�ʱ���ȣ�ÿ��10000���жϣ���10000Hz��
 */
#define TIMING_ACCURACY             10000
#define MAX_CLOCK_FREQUENCY         TIMING_ACCURACY
#define MIN_CLOCK_FREQUENCY         20
/*
///////////////////////////////////////////////////////////////////////////////
///////////////////
//  ϵͳ�ռ������
#define EVENT_MAX                   64
#define USER_EVENT                  16
*/

#if _CPU_WORD_ == 16
#define KOUM_MAX                    128
#else
#define KOUM_MAX                    (1*K)
#endif  /*  _CPU_WORD_  */

/*
 *  ϵͳĬ�ϵĽ���ջ�ռ�
 */
#define STACK_DEFAULT_SIZE          2048

/*
 *  ϵͳ����������
 *    Ĭ����32���������Ҫ����Ľ��̣��޸�������ּ��ɣ��������ơ���������漰
 *  ���̳صĿռ䣬�������Ҫ���á���ע�⣬Ĭ�ϵĽ�������ԶԶ�������ȼ���������
 */
#define PROC_MAX                    32

/*
 *  ϵͳ���ʱ������
 *    Ĭ����8����ʱ����ϵͳ�еĶ�ʱ�����˹��࣬���Ĭ�ϵ�������������
 */
#define TIMER_MAX                   8

#define MUTEX_MAX                   32
#define SEMA_MAX                    32
#define MB_MAX                      24
#define SHELL_CMD_MAX               32

/*
 *  LenixĬ��֧��16���豸�������Ҫ֧�ָ�����豸�������޸��������
 */
#define DEVICE_MAX                  16

#define TTY_MAX                     2

#define BLKBUF_SIZE                 (32*BBUF_SIZE)
 /*
  *  ������Ҫʹ�õĹ���ģ��
  *  ���ý����źŴ�����
  * /
#define _CFG_SIGNAL_ENABLE_
/*****************************************************************************/

/*
 *  ���ö�ʱ���������
 */
#define _CFG_TIMER_ENABLE_
/*****************************************************************************/

/*
 *  ���û������������
 */
#define _CFG_MUTEX_ENABLE_
/*****************************************************************************/

/*
 *  �����ź����������
 */
#define _CFG_SEMAPHORE_ENABLE_
/*****************************************************************************/

/*
 *  ������Ϣ�������
 *  ��CPU�ֳ�����8ʱ��������������Ϣ���
 */
#if _CPU_WORD_ > 8
#define _CFG_MESSAGE_ENABLE_
#endif 
/*****************************************************************************/

/*
 *  �����¼�����δ��ɣ���������
 *
#define _CFG_EVENT_ENABLE_
/*****************************************************************************/

/*
 *  ����TTY���
 */
#define _CFG_TTY_ENABLE_
/*****************************************************************************/

/*
 *  �����豸���������
 */
#define _CFG_DEVICE_ENABLE_
/*****************************************************************************/

/*
 *  ������ǽ���
 *  ������tty���
 */
#ifdef _CFG_TTY_ENABLE_
#define _CFG_SHELL_ENABLE_
#endif  /*  _CFG_TTY_ENABLE_    */
/*****************************************************************************/

#endif  /*  _CONFIG_H_  */