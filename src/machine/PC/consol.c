/*
////////////////////////////////////////////////////////////////////////////////
//                              Lenixʵʱ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: consol.c 
//  ����ʱ��:                   ������: �ޱ�
//  �޸�ʱ��: 2012-12-10        �޸���: �ޱ�
//  ��Ҫ����: �ṩ������PC����̨����
//  ˵    ��: 
//
//  �����¼:
//  �� �� ��:   |    ʱ  ��  |   ��  ��     | ��Ҫ�仯��¼
//==============================================================================
//              | 2014-02-02 |   ��  ��     |
//              | 2012-09-08 |   ��  ��     | �޸�con_set_byte,get_byte,
//                                            scroll_up����Ƕ��ʽ����ΪC����
//              | 2012-08-14 |   ��  ��     | ���λ��Ϊ����λ��
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
static int                  con_x_scale = 80,       /*  x��ֱ���   */
                            con_y_scale = 25;       /*  y��ֱ���   */
static int                  con_pos     = 0;        /*  ��ǰ���λ�� */

/*
////////////////////////////////////////////////////////////////////////////////
////////////////////
//  CRT��������
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
    int pvs; /* ԭ�Դ�λ�� prev vbuf start*/

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
    int pcp;    /* ԭ���λ��.prev cursor position*/

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
 *  Con_set_byte:����Ļָ��λ��д��һ���ֽ�
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
    byte_t  pca; /*  ԭ����,prev consol attr*/
    
    pca = (byte_t)(con_attr >> 8);
    con_attr = attr << 8 ;
    return pca;
}
/*
 *  Con_cls:������
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
 *  ���ù��
 */
void        Con_corsur(void)
{
    Con_set_cursor(con_pos);
}

/*
 *  ����
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
 *  �س�
 */
void        Con_cr(void)
{
}
/*
 *  ����
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
 *  �Ʊ��
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
 *  �˸�
 */
void        Con_back(void)
{
    word_t                  attr = con_attr;

    if( con_pos <= 0 )
        return ;

    while( --con_pos)
    {
        if( Con_get_byte(con_pos) )     /*  �涨����0����Ҫ����ִ���˸�  */
            break;
         CON_SET_BYTE(con_pos,0x20,attr);
    }

    CON_SET_BYTE(con_pos,0x20,attr);
}

/*
 *  ɾ��
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
 *  Con_write_string:����Ļ������ݣ�����0��ֹͣ������������ݲ�һ�����ַ�����
 *  ��Ȼ���ҵı���������ַ�����
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
 *  Con_write:����Ļ���ָ�����ȵ����ݣ���һ�����ַ���
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