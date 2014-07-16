/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: printf.c
//  ����ʱ��: 2012-11-26        ������: �ޱ�
//  �޸�ʱ��: 2014-02-01        �޸���: �ޱ�
//  ��Ҫ����: �ṩ�������ַ���������
//
//  ˵    ��: ���ļ��ṩ������������ܡ�
//
//  �仯��¼:
//  �� �� ��    |   ʱ  ��    |   ��  ��    |  ��Ҫ�仯��¼
//=============================================================================
//              | 2014-02-01  |   ��  ��    |  ����_printk����
//              | 2011-12-26  |   ��  ��    |  
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
 *  �����ַ�������������������ض���
 *  buf : ������
 *  c   : �ַ�
 *  n   : ��ǰ����������
 *  1�� �绺�������ڣ����򻺳������
 *  2�� �绺���������ڣ������׼����豸�����
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
 *  ����ת�ַ����Ļ���������Ҫ�ṩ˫���ȸ������Ļ�������Ԥ��64λȥ�����ź�С��
 *  �㹲��78λ���ֿ��ã�Ӧ���㹻��
 */
#define MAX_NUM_BUF                 80

/*
 *  һ��ʼ�Ϳ��ǻ���������
 */
int     _nvsprintf(char * buffer,const char * fmt,va_list al,int bufsize)
{
    char          * buf             = buffer;   /*      */
    int             status          = 0;        /*  0��ʾɨ���ʽ�ַ�����
                                                    1��ʾ�������           */

    char            pad             = 0;        /*      */
    char            pad_space       = ' ';      /*      */

    int             upcase          = 0;
    int             flag            = 0;        /*  ��ʽ��־                */
    int             field_width     = 0;        /*  ��ʽ�����              */
    int             precision       = 0;        /*  ��ʽ���ȣ���������      */
    char            qualifier       = 0;        /*  ����ʽ��־              */
    unsigned int    sign            = 0;        /*  ���ű�־������10������Ч*/
    unsigned int    base            = 0;        /*  ���ֽ���                */
    const char    * alphabet        = NULL;

    int             count           = 0;
    int             i               = 0;
    char            num_buf[MAX_NUM_BUF]     = {0};

    /*
     *  ֱ����ʽ���ַ����������
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
         *  ȡ��ʽ����־
         */
        for( ; *fmt ; fmt++)
        {
            switch(*fmt)
            {
            case '0':    flag |= FLAG_ZERO_PAD; pad = '0';  break;
            case '-':    flag |= FLAG_LEFT_ALIGN;           break;
            case ' ':    flag |= FLAG_SPACE;                break;
            case '+':    flag |= FLAG_PLUS;                 break;
            default :    goto get_flag_end; /*  ���ǿ�ʶ��ķ��ţ�������*/
            }
        }
get_flag_end:

        /*
         *  ȡ�ֶο��
         */
        field_width = 0;
        for( ; CHAR_IS_DIGITAL(*fmt) ; fmt++)
            field_width = field_width * 10 + *fmt - '0';

        /*
         *  ȡ������
         */
        precision = 0;
        if( *fmt == '.')
        {
            fmt++;
            for( ; CHAR_IS_DIGITAL(*fmt) ; fmt++)
                precision = precision * 10 + *fmt - '0';
        }

        /*
         *  ȡ����ʽ
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
             *  ������ʽ����
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
             *  �����Ҷ���
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
             *  �����Ҷ���
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
             *  ����˫����
             */
        }
        break;
        default:
        {
            /*
             *  ��������
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
//  ��  ��: _printk
//  ��  ��: ֱ������ʾ�豸�������û��TTY֧��ʱ�ܹ������Ϣ��
//  ��  ��: 
//      fmt         : const char *  |   ��ʽ�ַ���
//  ����ֵ: ����������ַ�������
//  ע  ��: ��������������ַ���1024�����ڡ�
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
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
         *  ��ʾ16������
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
         *  ��ʾ��Ӧ�ַ�
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

