

#include "fat.h"

#define UNINAME_MAX     128

word_t unicode_map[64*1024];

word_t      Asc_to_unicode(word_t asc)
{
    if( asc < 128 )
        return asc;
    return unicode_map[asc];
}

//  ���ļ���ת��Ϊ���浽�����еĸ�ʽ
int         Fat32_name_to_fdb(const char * name,fdb_t * fdb)
{
    const char * n = name;
    word_t  uniname[UNINAME_MAX];/* UNICODE��ʽ������*/
    int ni;

    /*  ���ļ���ת��Ϊunicode*/
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

    /*  ������ļ��� */

    /*  ���������ļ���д��FDB*/

    return -1;
}