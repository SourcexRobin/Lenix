/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenixʵʱ����ϵͳ
//                              2011 - 2012 @ Դ���빤����
//                                     �������а�Ȩ
//                          ***----------------------------***
//       ��  �� : tty.c 
//     ����ʱ�� : 2011-07-11       ������  : �ޱ�
//     �޸�ʱ�� : 2012-11-24       �޸���  : �ޱ�
//
//     ��Ҫ���� : �ṩ���ַ��ն˵�����������ܣ���ΪLenixĬ��֧�ֵ�Ӳ���豸
//
//      ˵  ��  : 
//
//  �汾�仯��¼:
//  �汾��      |    ʱ  ��     |    ��  ��     | ��Ҫ�仯��¼
//========================================================================================
//              |   2011-07-12  |    ��  ��     |  ��һ�棬Ӧ��ʹ����������ʽʵ�֡�
//////////////////////////////////////////////////////////////////////////////////////////
*/

#ifndef _TTY_H_
#define _TTY_H_

#include <type.h>
#include <result.h>

#include <proc.h>
#include <ipc.h>

/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  �ն˶���
*/
/*
 *  �򻯵��ն˶�����
 */
typedef struct _termios_t
{
    int                     temo_type;          /*  �ն�����    */
    byte_t                  temo_iflags;        /*  �����־    */
    byte_t                  temo_oflags;        /*  �����־    */
    byte_t                  temo_sflags;        /*  ���ڿ��Ʊ�־    */
    byte_t                  temo_lflags;        /*  �߱�־      */
}termios_t;

#define TERMIOS_TYPE_UNDEF          0
#define TERMIOS_TYPE_SERIAL         1
#define TERMIOS_TYPE_TTY            2

#define TERMIOS_IFLAG_SPACE_FILL    0x01
#define TERMIOS_IFLAG_NEED_COOK     0x02
#define TERMIOS_IFLAG_CR_UNIX       0x04

#define TERMIOS_OFLAG_ECHO          0x01
#define TERMIOS_OFLAG_PSW           0x02
/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  TTY����
*/
#define TTY_BUF_SIZE                128

typedef struct _tty_queue_t
{
    byte_t                  tq_buf[TTY_BUF_SIZE];
    uint8_t                 tq_head;
    uint8_t                 tq_tail;
}tty_queue_t;

/*  ���л����ж�    */
#define TQ_IS_FULL(tq)              ( (((tq).tq_head + 1 ) & ( TTY_BUF_SIZE - 1)) == \
                                      (tq).tq_tail  )
#define TQ_IS_EMPTY(tq)             ( (((tq).tq_tail + 1 ) & ( TTY_BUF_SIZE - 1)) == \
                                      (tq).tq_head  )
#define TQ_LEFT(tq)                 ( ((tq).tq_tail - (tq).tq_head-1)&(TTY_BUF_SIZE-1))

/*  ���л�������    */
#define TQ_INIT(tq)                 do{ _memzero(&tq,sizeof(tty_queue_t)); \
                                        (tq).tq_tail = TTY_BUF_SIZE - 1;\
                                      }while(FALSE)
#define TQ_EMPTY(tq)                do{ (tq).tq_head = 0;(tq).tq_tail =TTY_BUF_SIZE - 1;\
                                      } while(FALSE)
#define TQ_INC(a)                   do{ (a) = ((a) + 1) & ( TTY_BUF_SIZE - 1); \
                                      } while(FALSE)
#define TQ_DEC(a)                   do{ (a) = ((a) - 1) & ( TTY_BUF_SIZE - 1); \
                                      } while(FALSE)
#define TQ_PUT_CHAR(tq,c)           do{ \
                                        (tq).tq_buf[(tq).tq_head]=(c);\
                                        TQ_INC((tq).tq_head);\
                                      }while(FALSE)
#define TQ_GET_CHAR(tq,c)           do{ \
                                        TQ_INC((tq).tq_tail);\
                                        (c)=(tq).tq_buf[(tq).tq_tail];\
                                      }while(FALSE)

/*
 *  ���п�
 */
typedef void *              uart_t;

#define UART_RESV_SEND(uart)        uart
#define UART_IRQ_ENABLE(uart)       ((void *)((byte_t *)(uart) + 1))
#define UART_IRQ_FLAG(uart)         ((void *)((byte_t *)(uart) + 2))
#define UART_LINE_CTRL(uart)        ((void *)((byte_t *)(uart) + 3))
#define UART_MODEM_CTRL(uart)       ((void *)((byte_t *)(uart) + 4))
#define UART_LINE_STATUS(uart)      ((void *)((byte_t *)(uart) + 5))
#define UART_MODEM_STATUS(uart)     ((void *)((byte_t *)(uart) + 6))
#define UART_TEMP_BUFFER(uart)      ((void *)((byte_t *)(uart) + 7))


#define UART_IE_RESV_RDY            1
#define UART_IE_SEND_REG_EMPTY      2
#define UART_IE_RESV_LINE_STATUS    4
#define UART_IE_MODE_STATUS         8

#define UART_IF_MODEM_STATUS_CHG    0
#define UART_IF_NO_IRQ_HANGUP       1
#define UART_IF_SEND_REG_EMPTY      2
#define UART_IF_RESV_DATA_RDY       4
#define UART_IF_RESV_LINE_STATUS    6

#define UART_LC_DATA_LEN_MASK       0x03
#define UART_LC_DATA_LEN5           0x00
#define UART_LC_DATA_LEN6           0x01
#define UART_LC_DATA_LEN7           0x02
#define UART_LC_DATA_LEN8           0x03
#define UART_LC_STOP_BIT            0x04
#define UART_LC_OE_MASK             0x18
#define UART_LC_OE_1                0x00
#define UART_LC_OE_2                0x00
#define UART_LC_OE_3                0x00
#define UART_LC_OE_4                0x00
#define UART_LC_STOP                0x40
#define UART_LC_DLAB                0xC0

#define UART_LS_DATA_RDY            0x01
#define UART_LS_OVERLOAD_ERR        0x02
#define UART_LS_OE_ERR              0x04
#define UART_LS_
#define UART_LS_PAUSE_INT           0x10
#define UART_LS_SEND_EMPTY          0x20
#define UART_LS_SEND_SHIFT_EMPTY    0x40

typedef struct _tty_t
{
    termios_t               tty_termios;            /*  �ն˶���        */
    dword_t                 tty_mode;               /*  tty����ģʽ     */
    proc_list_t             tty_wait;               /*  �ȴ������б�    */
#ifdef _CFG_SMP_
    spin_lock_t             tty_lock;
#endif  /*  _CFG_SMP_ */
    void                  (*tty_echo_hook)(byte_t c);
	lock_t					tty_lock;           /*  tty�о����Ŀ��ܣ���Ҫ�䱸�� */
    tty_queue_t             tty_read_queue;     /*  �����У�tty���豸������ */
    tty_queue_t             tty_write_queue;    /*  д���У�tty���豸д���� */
    tty_queue_t             tty_second_queue;   /*  �������У���ʽ��ԭʼ����*/
}tty_t;

#define TTY_WAKEUP(tty)             do{ \
                                        CRITICAL_DECLARE(tty->tty_lock);\
                                        CRITICAL_BEGIN();\
                                        Proc_resume_on(&(tty->tty_wait));\
                                        CRITICAL_END();\
                                       }while(0)

/* 2012.12.06*/
#define TTY_ECHO(tty)               do{ byte_t  _c = 0; \
                                        while( !TQ_IS_EMPTY(tty->tty_write_queue) ) {\
                                            TQ_GET_CHAR(tty->tty_write_queue,_c);\
                                            tty->tty_echo_hook(_c);\
                                        }\
                                      }while(0)

#define TTY_LOCK(tty)				Lck_lock(&tty->tty_lock)
#define TTY_FREE(tty)				Lck_free(&tty->tty_lock)

void        Tty_initial       (void);
result_t    Tty_put_char      (int ttyid,byte_t c);
void *      Tty_echo_hook_set (int ttyid,void (* echo)(byte_t));
int         Tty_read          (int ttyid,void * buffer,size_t size);
int         Tty_write         (int ttyid,const void * buffer,size_t size);

#endif /*   _TTY_H_ */