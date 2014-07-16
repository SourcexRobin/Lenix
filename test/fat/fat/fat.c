

#include "fat.h"

#define UNINAME_MAX     128

word_t unicode_map[64*1024];

word_t      Asc_to_unicode(word_t asc)
{
    if( asc < 128 )
        return asc;
    return unicode_map[asc];
}

//  将文件名转换为保存到磁盘中的格式
int         Fat32_name_to_fdb(const char * name,fdb_t * fdb)
{
    const char * n = name;
    word_t  uniname[UNINAME_MAX];/* UNICODE格式的名称*/
    int ni;

    /*  将文件名转换为unicode*/
    for( ni = 0 ; ni < UNINAME_MAX - 1 ; ni++)
    {
        if( *n > 0 )
        {
            uniname[ni] = Asc_to_unicode(*n);
            n += 1;
        }
        else
        {
            uniname[ni] = Asc_to_unicode(*((word_t *)n));
            n += 2;
        }
    }
    uniname[ni] = 0;

    /*  构造短文件名 */

    /*  将两部分文件名写入FDB*/

    return -1;
}