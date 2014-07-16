/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenix实时操作系统
//                              2011 - 2012 @ 源代码工作室
//                                     保留所有版权
//                          ***----------------------------***
//       名  称 : keyborad.c
//     创建时间 : 已经不记得       创建者  : 罗斌
//     修改时间 : 2012-11-24       修改者  : 罗斌
//
//     主要功能 : pc键盘的驱动程序
//
//      说  明  : 
//
//  版本变化记录:
//  版本号      |    时  间     |    作  者     | 主要变化记录
//========================================================================================
//  00.00.000   |  2011-07-12   |    罗  斌     | 第一版
//////////////////////////////////////////////////////////////////////////////////////////
*/
#include <config.h>

#include <result.h>
#include <lio.h>

#ifdef _SLASH_
    #include <machine/machine.h>
    #include <machine/pc.h>
#else
    #include <machine\machine.h>
    #include <machine\pc.h>
#endif  /*  _SLASH_ */

result_t    Tty_put_char(int ttyid,byte_t c);

byte_t    none               (byte_t sc);
byte_t    HandleNormal       (byte_t sc);
byte_t    Handle_left_shift  (byte_t sc);
byte_t    Handle_right_shift (byte_t sc);
byte_t    Handle_left_ctrl   (byte_t sc);
byte_t    Handle_left_alt    (byte_t sc);
byte_t    Handle_caps_lock   (byte_t sc);
byte_t    Handle_minor       (byte_t sc);
byte_t    Handle_plus        (byte_t sc);
byte_t    Handle_gray0       (byte_t sc);
byte_t    Handle_gray1       (byte_t sc);
byte_t    Handle_gray2       (byte_t sc);
byte_t    Handle_gray3       (byte_t sc);
byte_t    Handle_gray4       (byte_t sc);
byte_t    Handle_gray5       (byte_t sc);
byte_t    Handle_gray6       (byte_t sc);
byte_t    Handle_gray7       (byte_t sc);
byte_t    Handle_gray8       (byte_t sc);
byte_t    Handle_gray9       (byte_t sc);
byte_t    Handle_graydot     (byte_t sc);
byte_t    Handle_numlock     (byte_t sc);
byte_t    Handle_scrlck      (byte_t sc);
//
//  键盘扫描码处理程序影射表
//
const kb_handler    kb_handle_tab[128] = {        
        //    0                1                2                3    
        //    4                5                6                7    
        //    8                9                A                B    
        //    C                D                E                F
/*    0    */    
            none,            none,            HandleNormal,    HandleNormal,
            HandleNormal,    HandleNormal,    HandleNormal,    HandleNormal,
            HandleNormal,    HandleNormal,    HandleNormal,    HandleNormal,
            HandleNormal,    HandleNormal,    HandleNormal,    HandleNormal,
/*    1    */    
            HandleNormal,    HandleNormal,    HandleNormal,    HandleNormal,
            HandleNormal,    HandleNormal,    HandleNormal,    HandleNormal,
            HandleNormal,    HandleNormal,    HandleNormal,    HandleNormal,
            HandleNormal,    Handle_left_ctrl,HandleNormal,    HandleNormal,
/*  2    */
            HandleNormal,    HandleNormal,    HandleNormal,    HandleNormal,
            HandleNormal,    HandleNormal,    HandleNormal,    HandleNormal,
            HandleNormal,    HandleNormal,    Handle_left_shift,    HandleNormal,
            HandleNormal,    HandleNormal,    HandleNormal,    HandleNormal,
/*  3    */                           
            HandleNormal,    HandleNormal,    HandleNormal,    HandleNormal,
            HandleNormal,    HandleNormal,    Handle_right_shift,HandleNormal,
            Handle_left_alt, HandleNormal,    Handle_caps_lock,HandleNormal,
            HandleNormal,    none,            HandleNormal,    HandleNormal,
/*  4    */    
            none,            none,            none,            none,
            none,            Handle_numlock,  Handle_scrlck,   Handle_gray7,
            Handle_gray8,    Handle_gray9,    Handle_minor,    Handle_gray4,
            Handle_gray5,    Handle_gray6,    Handle_plus,     Handle_gray1,
/*  5    */
            Handle_gray2,    Handle_gray3,    Handle_gray0,    Handle_graydot,
            none,            none,            none,            none,
            none,            none,            none,            none,
            none,            none,            none,            none,
/*  6    */
            none,            none,            none,            none,
            none,            none,            none,            none,
            none,            none,            none,            none,
            none,            none,            none,            none,
/*  7    */
            none,            none,            none,            none,
            none,            none,            none,            none,
            none,            none,            none,            none,
            none,            none,            none,            none,
};


/* 
 *  记录键盘状态，包括：左、右Ctrl键，左、右Alt键，左、右Shift键
 *  Caps Lock状态，NumLock状态
 */
volatile dword_t    keyboard_status = -1;    /* 键盘状态    */
volatile byte_t     led;    /*  键盘指示灯      */
volatile uint_t     e0e1;   /*  扩展字节标志    */
volatile uint_t     gray;   /*  小键盘标志      */

volatile void   *   keyboard_do_irq;

// 小写字母表
static const byte_t ascii_tab[] = 
{
        //    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
/*  0    */  0 ,0x1B, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',0x2d,0x3d,0x08,0x09,
/*  1    */ 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',0x5b,0x5d,0x0d,0x00, 'a', 's',
/*  2    */ 'd', 'f', 'g', 'h', 'j', 'k', 'l',0x3b,'\'',0x60,0x00,0x5c, 'z', 'x', 'c', 'v',
/*  3    */ 'b', 'n', 'm',0x2c,0x2e,0x2f,0xff,0x2a,0xff,0x20,0x00,0x00,0x00,0x00,0x00,0x00,                           
/*  4    */0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2d,0x00,0x00,0x00,0x00,0x00,
/*  5    */0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*  6    */0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*  7    */0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
/*  8    */
/*  9    */
/*  A    */
/*  B    */
/*  C    */
/*  D    */
/*  E    */
/*  F    */};

    // 大写字母表
static const byte_t ascii_tabs[]= 
{
        //    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
/*  0    */  0 ,0x1B, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',0x5F,0x2B,0x08,0x09,
/*  1    */ 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',0x7B,0x7D,0x0d,0xff, 'A', 'S',
/*  2    */ 'D', 'F', 'G', 'H', 'J', 'K', 'L',0x3A, '"',0x7E,0xff,0x7C, 'Z', 'X', 'C', 'V',
/*  3    */ 'B', 'N', 'M',0x3C,0x2e,0x3F,0xff,0x2a,0xff,0x20,0xff,0x00,0x00,0x00,0x00,0x00,                           
/*  4    */0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2d,0x00,0x00,0x00,0x00,0x00,
/*  5    */0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*  6    */0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*  7    */0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
/*  8    */
/*  9    */
/*  A    */
/*  B    */
/*  C    */
/*  D    */
/*  E    */
/*  F    */
};


void     Pc_keyboard_led_set(byte_t ledstat)
{
    Io_outb((void *)0x60,0xED);
    Io_delay();
    Io_outb((void *)0x60,ledstat);
}
byte_t      none(byte_t sc)
{
    sc = 0;     /*  这个写法是为了在BC3.1中，避免出现变量未引用的警告，以下也有类似情况   */
    return sc;
}

byte_t   HandleNormal(byte_t sc)
{
    byte_t    ascii        = 0;

    sc &= 0x7F;
    /*
     *  shift按键按下，则更换字符表
     */
    if(IS_SHIFT_PRESS(keyboard_status))
        ascii = ascii_tabs[sc];
    else
        ascii = ascii_tab [sc];

    if( (led & PC_KBLED_CAPSLOCK) && IS_LETTER(ascii) )
        ascii ^= 0x20;

    return ascii;
}
/*
    左shift键处理程序
*/
byte_t   Handle_left_shift(byte_t sc)
{
    /*
     *  前导存在E0，说明这是一个过渡指示字符
     */
    if( e0e1 & 1)
    {
        gray = 1;
        return 0;
    }

    keyboard_status ^= KB_STATE_LEFT_SHIFT;
    sc = 0;
    return sc;
}
/*
    右shift键处理程序
*/

byte_t   Handle_right_shift(byte_t sc)
{
    keyboard_status ^= KB_STATE_RIGHT_SHIFT;
    sc = 0;
    return sc;
}
/*
    大小写转换键处理程序
*/
byte_t   Handle_caps_lock(byte_t sc)
{
    if( sc & 0x80)
        return 0;

    led ^= PC_KBLED_CAPSLOCK;

    Pc_keyboard_led_set(led);

    return 0;
}

/*
    左ctrl键处理程序
*/
byte_t   Handle_left_ctrl(byte_t sc)
{
    sc = 0;
    keyboard_status ^= KB_STATE_LEFT_CTRL;
    return sc;
}
/*
    左alt键处理程序
*/
byte_t   Handle_left_alt(byte_t sc)
{
    sc = 0;
    keyboard_status ^= KB_STATE_LEFT_ALT;
    return sc;
}

byte_t    Handle_minor            (byte_t sc)
{
    sc = '-';
    return sc;
}
byte_t    Handle_plus        (byte_t sc)
{
    sc = '+';
    return sc;
}

byte_t    Handle_gray0            (byte_t sc)
{
    sc = 0;
    if( !gray && (led & PC_KBLED_NUMLOCK) )
        return '0';
    gray = 0;
    return sc;
}
byte_t    Handle_gray1            (byte_t sc)
{
    sc = 0;
    if( !gray && led & PC_KBLED_NUMLOCK )
        return '1';
    gray = 0;
    return sc;
}
byte_t    Handle_gray2            (byte_t sc)
{
    sc = 0;
    if( !gray && led & PC_KBLED_NUMLOCK )
        return '2';
    gray = 0;
    return sc;
}
byte_t    Handle_gray3            (byte_t sc)
{
    sc = 0;
    if( !gray && led & PC_KBLED_NUMLOCK )
        return '3';
    gray = 0;
    return sc;
}
byte_t    Handle_gray4            (byte_t sc)
{
    sc = 0;
    if( !gray && led & PC_KBLED_NUMLOCK )
        return '4';
    gray = 0;
    return sc;
}
byte_t    Handle_gray5            (byte_t sc)
{
    sc = 0;
    if( !gray && led & PC_KBLED_NUMLOCK )
        return '5';
    gray = 0;
    return sc;
}
byte_t    Handle_gray6            (byte_t sc)
{
    sc = 0;
    if( !gray && led & PC_KBLED_NUMLOCK )
        return '6';
    gray = 0;
    return sc;
}
byte_t    Handle_gray7            (byte_t sc)
{
    sc = 0;
    if( !gray && led & PC_KBLED_NUMLOCK )
        return '7';
    gray = 0;
    return sc;
}
byte_t    Handle_gray8            (byte_t sc)
{
    sc = 0;
    if( !gray && led & PC_KBLED_NUMLOCK )
        return '8';
    
    gray = 0;
    return sc;
}
byte_t    Handle_gray9            (byte_t sc)
{
    sc = 0;
    if( !gray && led & PC_KBLED_NUMLOCK )
        return '9';
    gray = 0;
    return sc;
}
byte_t    Handle_graydot        (byte_t sc)
{
    sc = 0;
    if( !gray && led & PC_KBLED_NUMLOCK )
        return '.';
    gray = 0;
    return sc;
}
byte_t    Handle_numlock        (byte_t sc)
{
    if( sc & 0x80 ) return 0;
    led ^= PC_KBLED_NUMLOCK;
    Pc_keyboard_led_set(led);
    return 0;
}
byte_t    Handle_scrlck        (byte_t sc)
{
    if( sc & 0x80 ) return 0;
    led ^= PC_KBLED_SCROLLLOCK;
    Pc_keyboard_led_set(led);
    return 0;
}

void        Kb_handle(dword_t kbstat,byte_t ascii,byte_t sc)
{
#ifdef _CFG_DEBUG_
    /**/
    char            msg[64];

    _sprintf(msg,"keyboard irq.stat: %08X ascii: %c %4d sc: %02X  \n",
        keyboard_status,ascii,ascii,sc & 0xFF);
    
    Con_write_string(30,1,msg,0x07);
    /*****/
#endif  /*_CFG_DEBUG_*/
    /****/
   
    if( sc & 0x80 ) return ;

    if( ascii == 0 )
    {
        kbstat = kbstat; /*  避免编译器产生变量未使用的警告 */
        return ;
    }

#ifdef _CFG_TTY_ENABLE_
    /*
     *  启用TTY时，向主TTY发送数据
     */
    Tty_put_char(TTY_MAJOR,ascii);
#endif  /*  _CFG_TTY_ENABLE */
}

int         Kb_do_irq(int notuse,int sc)
{
    byte_t          ascii;    

    if( interrupt_nest > IRQ_NEST_MAX )
    {
        notuse = notuse;
        Machine_interrupt_mis();
        return 0;
    }

    ascii = kb_handle_tab[sc & 0x7F ]((byte_t)sc);

    Kb_handle(keyboard_status,ascii,sc);
    
    return 0;
}

void        Kb_initial(void)
{
    keyboard_status = 0;    // 键盘状态
    keyboard_do_irq = 0;
    
    led     = PC_KBLED_NUMLOCK;
    e0e1    = 0;
    gray    = 0;
    
    /*
     *  在VPC2007中，该函数会导致VPC2007无响应
     */
    //Pc_keyboard_led_set(led);    
    
    Machine_ivt_set(ISR_KEYBOARD,Kb_do_irq);
}