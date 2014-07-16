/*
/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenix实时操作系统
//                              2011 - 2012 @ 源代码工作室
//                                     保留所有版权
//                          ***----------------------------***
//       名  称 : lchar.h 
//     创建时间 : 2011-07-02       创建者  : 罗斌
//     修改时间 : 2012-11-25       修改者  : 罗斌
//
//     主要功能 : 提供与硬件无关的字符操作函数 
//
//      说  明  : 
//
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//========================================================================================
//              |   2012-07-02  |  罗斌         |  从type.h中分离
//////////////////////////////////////////////////////////////////////////////////////////
*/

#ifndef _LENIX_CHAR_H_
#define _LENIX_CHAR_H_

/*
 *  CHAR_BACK   : 退格符  
 *  CHAR_TAB    : 制表符  
 *  CHAR_NL     : 换行符  
 *  CHAR_VT     : 垂直制表符  
 *  CHAR_FF     :   
 *  CHAR_CR     : 回车符  
 *  CHAR_DEL    : 删除符  
 */
#define CHAR_BACK                   8
#define CHAR_TAB                    9
#define CHAR_NL                     10
#define CHAR_VT                     11
#define CHAR_FF                     12
#define CHAR_CR                     13
#define CHAR_DEL                    127

#define CHAR_UP_CASE(c)             ( (c) & ~0x20 )
#define CHAR_LW_CASE(c)             ( (c) |  0x20 )

#define CHAR_IS_UP_CASE(c)          ( (c) >= 'A' && (c) <= 'Z' )
#define CHAR_IS_LW_CASE(c)          ( (c) >= 'a' && (c) <= 'z' )

#define CHAR_IS_DIGITAL(c)          ( (c) >= '0' && (c) <= '9' )
#define CHAR_IS_SPACE(c)            ( (c) == ' ' || (c) == '\t')

char        _up_case    (char c);
char        _lw_case    (char c);
int         _is_up_case (char c);
int         _is_lw_case (char c);
int         _is_hex     (char c);
int         _is_digital (char c);


#endif  /*  _LENIX_CHAR_H_  */