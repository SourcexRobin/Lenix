/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenix实时操作系统
//                              2011 - 2012 @ 源代码工作室
//                                     保留所有版权
//                          ***----------------------------***
//       名  称 : tty.c 
//     创建时间 : 2011-07-12       创建者  : 罗斌
//     修改时间 : 2013-12-05       修改者  : 罗斌
//
//     主要功能 : 提供从字符终端的输入输出功能，作为Lenix默认支持的硬件设备
//
//      说  明  : 
//
//  版本变化记录:
//  版本号      |    时  间     |    作  者     | 主要变化记录
//========================================================================================
//              |  2014-02-02   |    罗  斌     | 移植到32位时发现退格可以删除提示符。要改
//              |  2013-12-05   |
//  00.00.000   |  2011-07-12   |    罗  斌     | 第一版
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
 *  能否假设使用到交互功能的时候，实时性能已经不重要
 *  2012.12.12  
 */
static void Tty_copy_to_cook(tty_t * tty)
{
    byte_t                  c       = 0;
    CRITICAL_DECLARE(tty->tty_lock);

    CRITICAL_BEGIN();

    /*  输入缓冲区有数据才能对其进行加工  */
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
//  名  称 : Tty_put_char
//
//  功  能 : 
//
//  参  数 : 
//      ttyid               : int
//      说明: tty编号
//
//      c                   : byte_t
//      说明: 需要放入tty的数据
//
//  返回值 : 
//      类型 :result_t 
//      说明 : 
//
//  注  意: 在中断内调用
//
//  变更记录:
//  时间        |    作  者     |  说  明
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
	 *  1、普通tty缓冲区满则丢弃数据，因为并不一定有等待数据的进程
	 *  2、如果是交互终端，则需要留下回车符和退格符的空间  
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
    /*  定义了数据加工标志，则要对数据进行加工。  */
    if( tty->tty_termios.temo_iflags & TERMIOS_IFLAG_NEED_COOK )
        Tty_copy_to_cook(tty);
    else
    {
		TQ_INC((tty->tty_read_queue).tq_tail);
		TQ_PUT_CHAR(tty->tty_second_queue,c);
    }

    /*  如果终端定义了回显，也就是实时输出，则需要立即输出获得的数据  */
    if( tty->tty_termios.temo_oflags & TERMIOS_OFLAG_ECHO )
    {
        /*
         *  如果进入密码模式，一般的字符显示改为*号
         *  控制字符不改变显示
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
		/*  交互终端对于回车符需要唤醒等待输入的进程 */
		if( CHAR_CR == c )
			TTY_WAKEUP(tty);
		break;
	default:
		/*  一般的tty在缓冲区满的时候，需要唤醒等待的进程  */
		if( TQ_LEFT(tty->tty_second_queue) == 0 )
			TTY_WAKEUP(tty);
		break;
	}

    return RESULT_SUCCEED;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
//  名  称 : Tty_read
//
//  功  能 : 
//
//  参  数 : 
//      ttyid               : int
//      说明: tty编号
//
//      c                   : byte_t
//      说明: 需要放入tty的数据
//
//  返回值 : 
//      类型 :result_t 
//      说明 : 
//
//  注  意: 
//
//  变更记录:
//  时间        |    作  者     |  说  明
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

    /*  终止条件是缓冲区满    */
    TTY_LOCK(tty);
    while( size )
    {
        CRITICAL_BEGIN();
        if( TQ_IS_EMPTY(tty->tty_second_queue) )
        {	/*  缓冲区空需要等待数据  */
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
			{	/*  如果TTY是终端，遇到回车符需要返回。将回车符修改为0  */
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
     *  初始化主TTY设备。
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