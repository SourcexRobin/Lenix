

#include <type.h>

/*  2014-03-11 BCD��ת��������  */
byte_t      Bcd_to_bin(byte_t num)
{
    return (num >> 4) * 10 + (num & 0xF);
}
