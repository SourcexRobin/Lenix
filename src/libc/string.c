

#include <type.h>
#include <lchar.h>

int         _strlen(const char * string)
{
    register const char *   str = string;

    while( *str )   str++;

    return str - string;
}

char *      _strcat(char * des,const char * src)
{
    register char       *   d = des;
    register const char *   s = src;

    if( d == s ) return d;

    while( *d )  d++;

    while( *s ) *d++ = *s++;

    *d = 0;

    return des;
}

char *      _nstrcat(char * des,const char * src,size_t size)
{
    register char       *   d = des;
    register const char *   s = src;

    if( d == s ) return d;

    while( *d && size )
    {
        d++;
        size--;
    }

    while( *s && size-- ) *d++ = *s++;

    *d = 0;

    return des;
}

/*  缓冲区必须足够，但是因为参数并不能控制，因此存在溢出风险    */
char *      _strcpy(char * des,const char * src)
{
    register char       *   d = des;
    register const char *   s = src;

    if( d == s ) return d;

    if( (uint_t)d < (uint_t)s )
    {
        while( *s ) *d++ = *s++;
        *d = 0;
    }
    else
    {
        while( *s ) s++;
        d += s - src;
        while( s >= src ) *d-- = *s--;
    }

    return des;

}

char *      _nstrcpy(char * des,const char * src,size_t size)
{
    register char       *   d   = des;
    register const char *   s   = src;

    if( 0 == size && d == s ) return d;

    if( (uint_t)d < (uint_t)s )
    {
        while( *s && --size ) *d++ = *s++;;
        *d = 0;
    }
    else
    {
        while( *s && --size ) s++;
        d += s - src;
        *d = 0;
        while( s >= src ) *d-- = *s--;
    }

    return des;
}

int         _strcmp(const char * str1,const char * str2)
{
    register const char *   s1 = str1;
    register const char *   s2 = str2;

    if( s1 == s2 ) return 0;

    while( *s1 && *s1 == *s2 )
    {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

//  2012/6/28
//  名称比较，不区分大小写
int         _namecmp(const char * name1,const char * name2)
{
    register char           n1,
                            n2;
    while( *name1 )
    {
        n1 = *name1++;
        n2 = *name2++;

        if( n1 >= 'a' && n1 <= 'z' ) n1 &= ~0x20;       /*  如果是小写字母，则转换为大写    */
        if( n2 >= 'a' && n2 <= 'z' ) n2 &= ~0x20;       

        if( n1 != n2 )
            return n1 - n2;
    }

    return *name1 - *name2;
}

/*  2012.12.11  */
int         _atoh       (const char * num)
{
    int                     n       = 0;
    int                     base    = 0;

    while(CHAR_IS_SPACE(*num))
        num++;

    while( _is_hex(*num) )
    {
        char c = *num++;
        if( c >= '0' && c <= '9' )
            base = c - '0';
        else if ( c >= 'a' && c <= 'f' )
            base = (c - 'a') + 10;
        else
            base = (c - 'A') + 10;

        n = n*16 + base;
    }

    return n;
}

/*  2012.12.11  */
int         _atoi       (const char * num)
{
    int                     n       = 0;
    int                     sign    = 0;

    while(CHAR_IS_SPACE(*num))
        num++;

    if( '-' == *num )
    {
        sign = 1;
        num++;
    }

    while( *num >= '0' && *num <= '9' )
    {
        char c = *num++;
        if( c >= '0' && c <= '9' )
            n = n * 10 + c - '0';
    }


    return sign ? -n : n;
}
