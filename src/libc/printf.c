/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: printf.c
//  创建时间: 2012-11-26        创建者: 罗斌
//  修改时间: 2014-02-01        修改者: 罗斌
//  主要功能: 提供基本的字符操作函数
//
//  说    明: 本文件提供了输入输出功能。
//
//  变化记录:
//  版 本 号    |   时  间    |   作  者    |  主要变化记录
//=============================================================================
//              | 2014-02-01  |   罗  斌    |  增加_printk函数
//              | 2011-12-26  |   罗  斌    |  
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>

#include <type.h>
#include <lchar.h>
#include <lstring.h>
#include <larg.h>

int         Tty_write(int,const void * ,size_t);
int         Con_print_string(const char * string);

#ifdef _CFG_TTY_ENABLE_
    #define TTY_WRITE               Tty_write
#else
    #define TTY_WRITE(ttyid,buffer,size)
#endif  /*  _CFG_TTY_ENABLE_    */

/*
 *  处理字符输出，在这个宏内完成重定向。
 *  buf : 缓冲区
 *  c   : 字符
 *  n   : 当前缓冲区长度
 *  1、 如缓冲区存在，则向缓冲区输出
 *  2、 如缓冲区不存在，则向标准输出设备输出。
 */
#define OUTPUT_CHAR(buf,c,n)        do{if( NULL == (buf) ){  \
                                        if( '\b' == (c) ) count--; \
                                        else count++; \
                                          TTY_WRITE(TTY_MAJOR,&(c),1);}\
                                       else{ \
                                          if( (n) ){\
                                            if('\b' == (c) && (buf)>buffer){\
                                              --(buf);++(n) ;} \
                                            else {*(buf)++ = (c);--(n);}}} \
                                      }while(0)

static const char           alphabet_A[20]      = "0123456789ABCDEF";
static const char           alphabet_a[20]      = "0123456789abcdef";


#define FLAG_ZERO_PAD               1
#define FLAG_LEFT_ALIGN             2
#define FLAG_SPACE                  4
#define FLAG_PLUS                   8

/*
 *  数字转字符串的缓冲区，需要提供双精度浮点数的缓冲区，预计64位去掉符号和小数
 *  点共有78位数字可用，应该足够。
 */
#define MAX_NUM_BUF                 80

/*
 *  一开始就考虑缓冲区长度
 */
int     _nvsprintf(char * buffer,const char * fmt,va_list al,int bufsize)
{
    char          * buf             = buffer;   /*      */
    int             status          = 0;        /*  0表示扫描格式字符串，
                                                    1表示处理参数           */

    char            pad             = 0;        /*      */
    char            pad_space       = ' ';      /*      */

    int             upcase          = 0;
    int             flag            = 0;        /*  格式标志                */
    int             field_width     = 0;        /*  格式化宽度              */
    int             precision       = 0;        /*  格式精度，浮点数用      */
    char            qualifier       = 0;        /*  长格式标志              */
    unsigned int    sign            = 0;        /*  符号标志，仅对10进制有效*/
    unsigned int    base            = 0;        /*  数字进制                */
    const char    * alphabet        = NULL;

    int             count           = 0;
    int             i               = 0;
    char            num_buf[MAX_NUM_BUF]     = {0};

    /*
     *  直至格式化字符串处理完毕
     */
    for( ; *fmt && bufsize > 0; fmt++ )
    {
        char c = *fmt;
        
        if( 0 == status )
        {
            if( '%' == c )
                status = 1;
            else
                OUTPUT_CHAR(buf,c,bufsize);
            continue;
        }

        flag    = 0;
        pad     = ' ';

        /*
         *  取格式化标志
         */
        for( ; *fmt ; fmt++)
        {
            switch(*fmt)
            {
            case '0':    flag |= FLAG_ZERO_PAD; pad = '0';  break;
            case '-':    flag |= FLAG_LEFT_ALIGN;           break;
            case ' ':    flag |= FLAG_SPACE;                break;
            case '+':    flag |= FLAG_PLUS;                 break;
            default :    goto get_flag_end; /*  不是可识别的符号，则跳出*/
            }
        }
get_flag_end:

        /*
         *  取字段宽度
         */
        field_width = 0;
        for( ; CHAR_IS_DIGITAL(*fmt) ; fmt++)
            field_width = field_width * 10 + *fmt - '0';

        /*
         *  取精度域
         */
        precision = 0;
        if( *fmt == '.')
        {
            fmt++;
            for( ; CHAR_IS_DIGITAL(*fmt) ; fmt++)
                precision = precision * 10 + *fmt - '0';
        }

        /*
         *  取长格式
         */
        qualifier = 0;
        if( *fmt == 'h' || *fmt == 'l' || *fmt == 'L' )
            qualifier = *fmt++;

        sign        = 0;
        base        = 10;
        alphabet    = alphabet_A;
        upcase      = 0;

        switch( *fmt )
        {
        case 'i':
        case 'd':
            sign = 1;
            goto num_handle;
        case 'u':
            goto num_handle;
        case 'S':
            upcase = 1;
        case 's':
            goto string_handle;
        case 'p':
            alphabet    = alphabet_a;
        case 'P':
            base        = 16;
            pad         = '0';
            field_width = sizeof(int) * 2;
            goto num_handle;
        case 'x':
            alphabet    = alphabet_a;
        case 'X':
            base        = 16;
            goto num_handle;
        case 'o':
            base        = 8;
            goto num_handle;
            
        case 'f':
            goto float_handle;

        case 'C':
            upcase = 1;
        case 'c':
            i = va_arg(al,char);
            if( upcase )
                i = _up_case((char)i);
            OUTPUT_CHAR(buf,(char)i,bufsize);
            break;
        default:
            OUTPUT_CHAR(buf,c,bufsize);
            break;
        }

        status = 0;
        continue;

num_handle:
        switch(qualifier)
        {
        case 'h':
        case 'l':
        case 'L':
        {
            /*
             *  处理长格式数字
             */
            ularge_t        num     = va_arg(al,ularge_t);
            int             len     = 0;
            
            i = MAX_NUM_BUF - 1;
            if( sign && (large_t)num < 0 )
                num = ~num + 1;
            else
                sign = 0;

            do
            {
                num_buf[i--] = alphabet[num % base];
                num /= base;
            }while( num );

            if( sign )
                num_buf[i--] = '-';
            else if( 0 == sign && (flag & FLAG_PLUS) )
                num_buf[i--] = '+';

            len = MAX_NUM_BUF - i - 1;
            /*
             *  处理右对齐
             */
            if( !(flag & FLAG_LEFT_ALIGN) )
            {
                while( len < field_width-- && bufsize > 0 )
                    OUTPUT_CHAR(buf,pad,bufsize);
            }

            for( i++ ; i < MAX_NUM_BUF && bufsize > 0 ; i++)
                OUTPUT_CHAR(buf,num_buf[i],bufsize);
                
            while( len < field_width-- && bufsize > 0 )
                OUTPUT_CHAR(buf,pad_space,bufsize);
        }
            break;
        default:
        {
            uint_t          num     = va_arg(al,uint_t);
            int             len     = 0;

            i = MAX_NUM_BUF - 1;

            if( sign && (int)num < 0 )
                num = ~num + 1;
            else
                sign = 0;

            do
            {
                num_buf[i--] = alphabet[num % base];
                num /= base;
            }while( num );

            if( sign )
                num_buf[i--] = '-';
            else if( 0 == sign && (flag & FLAG_PLUS) )
                num_buf[i--] = '+';
            len = MAX_NUM_BUF - i - 1;
            /*
             *  处理右对齐
             */
            if( !(flag & FLAG_LEFT_ALIGN) )
            {
                while( len < field_width-- && bufsize > 0 )
                    OUTPUT_CHAR(buf,pad,bufsize);
            }

            for( i++ ; i < MAX_NUM_BUF && bufsize > 0 ; i++)
                OUTPUT_CHAR(buf,num_buf[i],bufsize);
                
            while( len < field_width-- && bufsize > 0 )
                OUTPUT_CHAR(buf,pad_space,bufsize);
        }
            break;
        }
        status = 0;
        continue;
string_handle:
        {
            char        *   str = va_arg(al,char *);
            int             len = _strlen(str);
            char            c   = 0;

            if( !(flag & FLAG_LEFT_ALIGN) )
            {
                while( len < field_width-- && bufsize > 0 )
                    OUTPUT_CHAR(buf,pad_space,bufsize);
            }

            for( ; *str && bufsize > 0  ; str++ )
            {
                c = *str;
                if( upcase )
                    c = _up_case(c);

                OUTPUT_CHAR(buf,c,bufsize);
            }

            while( len < field_width-- && bufsize > 0 )
                OUTPUT_CHAR(buf,pad_space,bufsize);

        }
        status = 0;
        continue;
float_handle:
        switch(qualifier)
        {
        case 'h':
        case 'l':
        case 'L':
        {
            /*
             *  处理双精度
             */
        }
        break;
        default:
        {
            /*
             *  处理单精度
             */
        }
            break;
        }
        status = 0;
        continue;
    }

    if( buffer ) 
    {
        *buf = 0;
        return buf - buffer;
    }
    return count;
}

int     _sprintf(char * buffer,const char * fmt,...)
{
    va_list                 vl      = (va_list)0;
    int                     len     = 0;;
    va_start(vl,fmt);

    len = _nvsprintf(buffer,fmt,vl,INT_MAX);

    va_end(vl);

    return len;
}

int     _printf(const char * fmt,...)
{
    va_list                 vl      = (va_list)0;
    int                     len     = 0;;

    va_start(vl,fmt);

    len = _nvsprintf(NULL,fmt,vl,INT_MAX);

    va_end(vl);

    return len;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: _printk
//  作  用: 直接向显示设备输出，在没有TTY支持时能够输出信息。
//  参  数: 
//      fmt         : const char *  |   格式字符串
//  返回值: 返回输出的字符个数。
//  注  意: 这里限制了输出字符在1024个以内。
//  变更记录:
//  时间        |  作  者       |  说明
//=============================================================================
//  2014-02-01  |               |
///////////////////////////////////////////////////////////////////////////////
*/
int         _printk(const char * fmt,...)
{
    static char             buf[1024];
    va_list                 vl      = (va_list)0;
    int                     len     = 0;;

    va_start(vl,fmt);
    len = _nvsprintf(buf,fmt,vl,1024);
    va_end(vl);
    Con_print_string(buf);

    return len;
}
/*  2012-12-02  */
/*  2012-12-11  */
int         _mprintf(const void * mem,unsigned int size)
{
    const char          *   m   = (void *)((int)mem & ~0xF);
    int                     i   = 0;
    int                     n   = 0;
    size += (int)mem & 0xF;
    while( size  )
    {
        n = size;
        if( n > 16 )
            n = 16;

        _printf("%P: ",m);
        /*
         *  显示16进制数
         */
        for( i = 0 ; i < 16 ; i++)
        {
            if( i && 0 == (i % 8 ))
            {
                if( n > 8 && (m + i) >= (const char *)mem)
                    _printf("- ");
                else
                    _printf("  ");
            }
            
            if( i < n && (m +i) >= (const char *)mem )
                _printf("%02X ",0xFF & m[i]);
            else
                _printf("   ");

        }

        /*
         *  显示相应字符
         */
        _printf(" ");
        for( i = 0 ; i < n ; i++,m++)
        {
            if( m >= (const char *)mem )
            {
                if( *m > 12 && *m < 127 )
                    _printf("%c",*m);
                else
                    _printf(".");
            }
            else
                _printf(" ");
        }
        _printf("\n");
        size    -= n;
    }

    return m - (const char *)mem;
}

