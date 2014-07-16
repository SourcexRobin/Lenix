/*
////////////////////////////////////////////////////////////////////////////////
//                              Lenix实时操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: consol.c 
//  创建时间:                   创建者: 罗斌
//  修改时间: 2012-12-10        修改者: 罗斌
//  主要功能: 提供基本的PC控制台功能
//  说    明: 
//
//  变更记录:
//  版 本 号:   |    时  间  |   作  者     | 主要变化记录
//==============================================================================
//              | 2014-02-02 |   罗  斌     |
//              | 2012-09-08 |   罗  斌     | 修改con_set_byte,get_byte,
//                                            scroll_up，由嵌入式汇编改为C语言
//              | 2012-08-14 |   罗  斌     | 光标位置为输入位置
////////////////////////////////////////////////////////////////////////////////
*/
//  2012-08-14 
//  2012-09-08 
#include <lio.h>
#include <machine\pc.h>

#if _CFG_CPU_ == _CFG_CPU_X86_

#if _CPU_WORD_ == 16

#define VGA_TEXT_BUFFER             ((word_t far *)0xB8000000)
#define VGA_GRAPHIC_BUFFER          ((word_t far *)0xA0000000)
#define CON_SET_BYTE(pos,c,attr)    (VGA_TEXT_BUFFER[pos] = (attr) | (c))

#else

#define VGA_TEXT_BUFFER             ((word_t *)0xB8000)
#define VGA_GRAPHIC_BUFFER          ((word_t *)0xA0000)
#define CON_SET_BYTE(pos,c,attr)    (VGA_TEXT_BUFFER[pos] = (attr) | (c))

#endif /*  _CPU_WORD_ */
#endif /*  _CFG_CPU_  */
static word_t               con_attr    = 0x0700;
static int                  con_x_scale = 80,       /*  x轴分辨率   */
                            con_y_scale = 25;       /*  y轴分辨率   */
static int                  con_pos     = 0;        /*  当前光标位置 */

/*
////////////////////////////////////////////////////////////////////////////////
////////////////////
//  CRT操作部分
*/
static
byte_t      Con_read_reg(int idx)
{
    Io_outb(PC_CON_REG_IDX,(byte_t)idx);
    return Io_inb(PC_CON_REG_DATA);

}

static
void        Con_write_reg(int idx,byte_t data)
{
    Io_outb(PC_CON_REG_IDX,(byte_t)idx);
    Io_outb(PC_CON_REG_DATA,data);
}

static
int    Con_get_vbuf_start()
{
    return  (int)(Con_read_reg(0x0C) * 100 + Con_read_reg(0x0D));
}

static
int         Con_set_vbuf_start(int start)
{
    int pvs; /* 原显存位置 prev vbuf start*/

    pvs = Con_read_reg(0x0C) * 0x100 + Con_read_reg(0x0D);
    
    Con_write_reg(0xC,(byte_t)((start >> 8)&0xFF));
    Con_write_reg(0xD,(byte_t)(start&0xFF));

    return pvs;
}

static
int         Con_get_colum()
{
    return (int)Con_read_reg(0x13) * 2;
}

static
int         Con_get_cursor()
{
    return (int)(Con_read_reg(0x0E) * 0x100 + Con_read_reg(0x0F));
}

static
int         Con_set_cursor(int pos)
{
    int pcp;    /* 原光标位置.prev cursor position*/

    pcp = Con_read_reg(0x0E) * 0x100 + Con_read_reg(0x0F);

    Con_write_reg(0xE,(byte_t)(((pos) >> 8)&0xFF));
    Con_write_reg(0xF,(byte_t)((pos)&0xFF));

    return pcp;
}

static
int         Con_set_cursor_xy(int x,int y)
{
    int pcp;

    pcp = Con_read_reg(0x0E) * 0x100 + Con_read_reg(0x0F);

    x  += con_x_scale * y;
    
    Con_write_reg(0xE,(byte_t)((x >> 8)&0xFF));
    Con_write_reg(0xF,(byte_t)( x & 0xFF));

    return pcp;
}


/*
 *  Con_set_byte:向屏幕指定位置写入一个字节
 */
void        Con_set_byte(int pos,byte_t dat,byte_t attr)
{
    VGA_TEXT_BUFFER[pos] = ( ((word_t)attr) << 8 ) | dat;
}

byte_t      Con_get_byte(int pos)
{
    return (byte_t)(VGA_TEXT_BUFFER[pos]);
}

byte_t      Con_attr_set(byte_t attr)
{
    byte_t  pca; /*  原属性,prev consol attr*/
    
    pca = (byte_t)(con_attr >> 8);
    con_attr = attr << 8 ;
    return pca;
}
/*
 *  Con_cls:清屏。
 */
void        Con_cls(void)
{
    int                     x       = 0;
    word_t                  attr    = con_attr;

    while ( x < con_x_scale * con_y_scale )
         CON_SET_BYTE(x++,0x20,attr);

    con_pos = 0;
    Con_set_cursor(con_pos);
}

/*
 *  设置光标
 */
void        Con_corsur(void)
{
    Con_set_cursor(con_pos);
}

/*
 *  卷屏
 */
void        Con_scroll_up(int lines)
{
    word_t FAR          *   vbuf    = VGA_TEXT_BUFFER;
    int                     src     = 0,
                            des     = 0,
                            ncopy   = 0;
    word_t                  attr    = con_attr;

    if( lines < 1 )
        return ;

    src     = con_x_scale * lines ;
    ncopy   = (25 - MIN(lines,25)) * con_x_scale ;

    for( ; des < ncopy ; des++)
        vbuf[des] = vbuf[src + des];
    
    ncopy = con_x_scale * con_y_scale;

    while( des < ncopy )
        vbuf[des++] = attr;  

    con_pos -= lines * con_x_scale;
}

/*
 *  回车
 */
void        Con_cr(void)
{
}
/*
 *  换行
 */
void        Con_nl(void)
{
    int                     pad     = 0;
    word_t                  attr    = con_attr;

    pad = con_x_scale - con_pos % con_x_scale;

    CON_SET_BYTE(con_pos++,0x20,attr);

    while(--pad)
         CON_SET_BYTE(con_pos++,0,attr);

    if( con_pos >= con_y_scale * con_x_scale )
        Con_scroll_up(1);
}
/*
 *  制表符
 */
void        Con_tab(void)
{
    int                     pad     = 0;
    word_t                  attr    = con_attr;

    pad = 8 - con_pos % 8;

    Con_set_byte(con_pos++,0x20,(byte_t)attr);

    while(--pad)
    {
        CON_SET_BYTE(con_pos++,0,attr);
        if( con_pos >= con_y_scale * con_x_scale )
            Con_scroll_up(1);
    }
}
/*
 *  退格
 */
void        Con_back(void)
{
    word_t                  attr = con_attr;

    if( con_pos <= 0 )
        return ;

    while( --con_pos)
    {
        if( Con_get_byte(con_pos) )     /*  规定遇到0，则要继续执行退格  */
            break;
         CON_SET_BYTE(con_pos,0x20,attr);
    }

    CON_SET_BYTE(con_pos,0x20,attr);
}

/*
 *  删除
 */
void        Con_del(void)
{
}

void        Con_write_char(int pos,byte_t c,byte_t attr)
{
    word_t                  ta = ((word_t)attr) << 8;

    switch(c)
    {
    case CHAR_NL:
    case CHAR_CR:
    case CHAR_TAB:    
    case CHAR_BACK:
    case CHAR_DEL:
        break;
    default:
        CON_SET_BYTE(pos,c,ta);
        break;
    }
}

/*
 *  Con_write_string:向屏幕输出数据，遇到0则停止输出，但是数据不一定是字符串，
 *  当然，我的本意是输出字符串。
 */
int         Con_write_string(int x,int y ,const char * string,byte_t attr)
{
    const char          *   str     = string;

    x  += y * con_x_scale;
    
    while( *str )
        Con_write_char(x++,(byte_t)*str++,attr);
    
    return str - string;
}

/*
 *  Con_write:向屏幕输出指定长度的数据，不一定是字符。
 */
void        Con_write(int x,int y,const void * buffer,int size,byte_t attr)
{
    const byte_t        *   buf     = buffer;

    x  += y * con_x_scale;
    
    for( ;size--; )
        Con_write_char(x++,*buf++,attr);
}

static
void        Con_print(byte_t c,byte_t attr)
{
    switch(c)
    {
    case CHAR_NL:        Con_nl();        break;
    case CHAR_CR:        Con_cr();        break;
    case CHAR_TAB:       Con_tab();       break;
    case CHAR_BACK:      Con_back();      break;
    case CHAR_DEL:       Con_del();       break;
    default:
         CON_SET_BYTE(con_pos++,c,attr << 8);
        if( con_pos >= con_x_scale * con_y_scale)
            Con_scroll_up(1);
        break;
    }
    Con_corsur();
}

void        Con_print_char(byte_t c)
{
    Con_print(c,(byte_t)(con_attr >> 8));
}

int         Con_print_string(const char * string)
{
    const char          *   str     = string;
    byte_t                  attr    = (byte_t)(con_attr >> 8);

    while( *str)
        Con_print(*str++,attr);
    
    return str - string;
}

void        Con_initial(void)
{
    _printk("PC consol initial...   ");
    con_attr    = 0x0700;
    con_x_scale = 80;
    con_y_scale = 25;
    _printk("OK!\n");
}