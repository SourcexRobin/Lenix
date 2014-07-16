
#include <type.h>

void *      _memzero(void * des,size_t size)
{
    register byte_t     *   d = des;

    while(size--)
        *d++= 0;

    return des;
}

void *      _memset(void * des,char v,size_t size)
{
    register char       *   d = des;

    while( size-- )
        *d++ = v;

    return des;
}

int         _memcmp(const void * des,const void * src,size_t size)
{
    register const char *   d = des;
    register const char *   s = src;

    while( size-- && *d == *s)
    {
        d++;
        s++;
    }

    return *d - *s;
}

void *      _memcpy(void * des,const void * src,size_t size)
{
    register byte_t         *   d = des;
    register const byte_t   *   s = src;

    if( 0 == size || d == s ) return des;

    if( (uint_t)d < (uint_t)s )
    {
        while( size-- )
            *d++ = *s++;
    }
    else
    {
        d += size - 1;
        s += size - 1;
        while( size-- )
            *d-- = *s--;
    }
    return des;
}
