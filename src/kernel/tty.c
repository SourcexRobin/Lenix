/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenixʵʱ����ϵͳ
//                              2011 - 2012 @ Դ���빤����
//                                     �������а�Ȩ
//                          ***----------------------------***
//       ��  �� : tty.c 
//     ����ʱ�� : 2011-07-12       ������  : �ޱ�
//     �޸�ʱ�� : 2013-12-05       �޸���  : �ޱ�
//
//     ��Ҫ���� : �ṩ���ַ��ն˵�����������ܣ���ΪLenixĬ��֧�ֵ�Ӳ���豸
//
//      ˵  ��  : 
//
//  �汾�仯��¼:
//  �汾��      |    ʱ  ��     |    ��  ��     | ��Ҫ�仯��¼
//========================================================================================
//              |  2014-02-02   |    ��  ��     | ��ֲ��32λʱ�����˸����ɾ����ʾ����Ҫ��
//              |  2013-12-05   |
//  00.00.000   |  2011-07-12   |    ��  ��     | ��һ��
//////////////////////////////////////////////////////////////////////////////////////////
*/
#include <config.h>

#ifdef _CFG_TTY_ENABLE_

#include <assert.h>
#include <proc.h>
#include <device.h>
#include <tty.h>

#ifdef _SLASH_
    #include <machine/machine.h>
#else
    #include <machine\machine.h>
#endif  /*  _SLASH_ */


static tty_t                tty_pool[TTY_MAX];

/*
#ifdef _CFG_SMP_
static spin_lock_t          tty_lock;
#endif  /*  _CFG_SMP_   */

/* 2012.12.06*/
static void Tty_echo_hook_default(byte_t  c)
{
    c = c;
}


/*
 *  �ܷ����ʹ�õ��������ܵ�ʱ��ʵʱ�����Ѿ�����Ҫ
 *  2012.12.12  
 */
static void Tty_copy_to_cook(tty_t * tty)
{
    byte_t                  c       = 0;
    CRITICAL_DECLARE(tty->tty_lock);

    CRITICAL_BEGIN();

    /*  ���뻺���������ݲ��ܶ�����мӹ�  */
    while( !TQ_IS_EMPTY(tty->tty_read_queue)  )
    {
        TQ_GET_CHAR(tty->tty_read_queue,c);

        switch(c)
        {
        case CHAR_BACK:
            if( TQ_IS_EMPTY(tty->tty_second_queue) )
                continue;
            TQ_DEC(tty->tty_second_queue.tq_head);
            break;
        default:
            TQ_PUT_CHAR(tty->tty_second_queue,c);
            break;
        }
    }

    CRITICAL_END();
}

/* 2012.12.06*/
void *      Tty_echo_hook_set (int ttyid,void (* echo)(byte_t))
{
    tty_t               *   tty         = tty_pool + ttyid;
    void                *   handle      = NULL;
    CRITICAL_DECLARE(tty->tty_lock);

#ifdef _CFG_CHECK_PARAMETER_
    if( ttyid >= TTY_MAX )  return NULL;
    if( NULL == echo )      return NULL;
#endif  /*  _CFG_CHECK_PARAMETER_   */

    CRITICAL_BEGIN();
    handle = tty->tty_echo_hook;
    tty->tty_echo_hook = echo;
    CRITICAL_END();

    return handle;
}


/*
//////////////////////////////////////////////////////////////////////////////////////////
//  ��  �� : Tty_put_char
//
//  ��  �� : 
//
//  ��  �� : 
//      ttyid               : int
//      ˵��: tty���
//
//      c                   : byte_t
//      ˵��: ��Ҫ����tty������
//
//  ����ֵ : 
//      ���� :result_t 
//      ˵�� : 
//
//  ע  ��: ���ж��ڵ���
//
//  �����¼:
//  ʱ��        |    ��  ��     |  ˵  ��
//========================================================================================
//  2013-12-05
//  2011-11-11  |               |
//////////////////////////////////////////////////////////////////////////////////////////
*/
result_t    Tty_put_char(int ttyid,byte_t c)
{
    tty_t               *   tty     = tty_pool + ttyid;
    result_t                result  = RESULT_SUCCEED;
    CRITICAL_DECLARE(tty->tty_lock );

    if( ttyid >= TTY_MAX )
        return result;

    CRITICAL_BEGIN();
    /*
	 *  1����ͨtty���������������ݣ���Ϊ����һ���еȴ����ݵĽ���
	 *  2������ǽ����նˣ�����Ҫ���»س������˸���Ŀռ�  
	 */
	if( TQ_LEFT(tty->tty_second_queue) == 0 ||
		( tty->tty_termios.temo_type == TERMIOS_TYPE_TTY && 
		!( TQ_LEFT(tty->tty_second_queue) > 4 || CHAR_BACK == c || CHAR_CR == c ) ) )
	{
        CRITICAL_END();
		return RESULT_FAILED;
	}
    TQ_PUT_CHAR( tty->tty_read_queue,c);
    CRITICAL_END();
    /*  ���������ݼӹ���־����Ҫ�����ݽ��мӹ���  */
    if( tty->tty_termios.temo_iflags & TERMIOS_IFLAG_NEED_COOK )
        Tty_copy_to_cook(tty);
    else
    {
		TQ_INC((tty->tty_read_queue).tq_tail);
		TQ_PUT_CHAR(tty->tty_second_queue,c);
    }

    /*  ����ն˶����˻��ԣ�Ҳ����ʵʱ���������Ҫ���������õ�����  */
    if( tty->tty_termios.temo_oflags & TERMIOS_OFLAG_ECHO )
    {
        /*
         *  �����������ģʽ��һ����ַ���ʾ��Ϊ*��
         *  �����ַ����ı���ʾ
         */
        if( tty->tty_termios.temo_oflags & TERMIOS_OFLAG_PSW )
        {
            switch(c)
            {
            case CHAR_CR:
            case CHAR_BACK:
                break;
            default:
                c = '*';
                break;
            }
        }
        tty->tty_echo_hook(c);
    }

	switch(tty->tty_termios.temo_type)
	{
	case TERMIOS_TYPE_TTY:
		/*  �����ն˶��ڻس�����Ҫ���ѵȴ�����Ľ��� */
		if( CHAR_CR == c )
			TTY_WAKEUP(tty);
		break;
	default:
		/*  һ���tty�ڻ���������ʱ����Ҫ���ѵȴ��Ľ���  */
		if( TQ_LEFT(tty->tty_second_queue) == 0 )
			TTY_WAKEUP(tty);
		break;
	}

    return RESULT_SUCCEED;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
//  ��  �� : Tty_read
//
//  ��  �� : 
//
//  ��  �� : 
//      ttyid               : int
//      ˵��: tty���
//
//      c                   : byte_t
//      ˵��: ��Ҫ����tty������
//
//  ����ֵ : 
//      ���� :result_t 
//      ˵�� : 
//
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |    ��  ��     |  ˵  ��
//========================================================================================
//  2013-12-05
//  2011-11-11  |               |
//////////////////////////////////////////////////////////////////////////////////////////
*/
int         Tty_read(int ttyid,void * buffer,size_t  size)
{
    tty_t               *   tty     = tty_pool + ttyid;
    byte_t              *   buf     = buffer;
    byte_t                  c       = 0;
    CRITICAL_DECLARE(tty->tty_lock);

#ifdef _CFG_CHECK_PARAMETER_
    if( ttyid >= TTY_MAX )	return -1;
    if( NULL == buffer )    return -1;
#endif  /*  _CFG_CHECK_PARAMETER_   */

    /*  ��ֹ�����ǻ�������    */
    TTY_LOCK(tty);
    while( size )
    {
        CRITICAL_BEGIN();
        if( TQ_IS_EMPTY(tty->tty_second_queue) )
        {	/*  ����������Ҫ�ȴ�����  */
            Proc_wait_on(&(tty->tty_wait));
            TTY_FREE(tty);  /*  */
            CRITICAL_END();
            Proc_sched(0);
            TTY_LOCK(tty);
        }
        else
        {
            TQ_GET_CHAR(tty->tty_second_queue,c);
            CRITICAL_END();
            *buf++ = c;
            if( CHAR_CR == c && ( tty->tty_termios.temo_type & TERMIOS_TYPE_TTY ) )
			{	/*  ���TTY���նˣ������س�����Ҫ���ء����س����޸�Ϊ0  */
				*--buf = 0;
                break;
			}
            --size;
        }
    }
    TTY_FREE(tty);
    return buf - (byte_t *)buffer;
}

/*
  2013-12-05
  2012.11.27 
 */
int         Tty_write(int ttyid,const void * buffer,size_t size)
{
    tty_t               *   tty     = tty_pool + ttyid;
    const byte_t        *   buf     = buffer;
    //CRITICAL_DECLARE(tty->tty_lock);

#ifdef _CFG_CHECK_PARAMETER_
    if( ttyid >= TTY_MAX )	return -1;
    if( NULL == buffer )    return -1;
#endif  /*  _CFG_CHECK_PARAMETER_   */

	TTY_LOCK(tty);
    while( size-- > 0 )
        tty->tty_echo_hook(*buf++);        
	TTY_FREE(tty);

    return buf - (const byte_t *)buffer;
}

void        Tty_initial(void)
{
    tty_t               *   tty     = NULL;
    
    _printk("tty initial...    ");
    /*
     *  ��ʼ����TTY�豸��
     */
    tty = tty_pool;

    tty->tty_echo_hook              = Tty_echo_hook_default;
	tty->tty_termios.temo_type		= TERMIOS_TYPE_TTY;
    tty->tty_termios.temo_iflags    = TERMIOS_IFLAG_NEED_COOK ;
    tty->tty_termios.temo_oflags    = TERMIOS_OFLAG_ECHO ;

    TQ_INIT(tty->tty_read_queue);
    TQ_INIT(tty->tty_write_queue);
    TQ_INIT(tty->tty_second_queue);
    _printk("OK!\n");
}

#endif  /*  _CFG_TTY_ENABLE_    */