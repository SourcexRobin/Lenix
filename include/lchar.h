/*
/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenixʵʱ����ϵͳ
//                              2011 - 2012 @ Դ���빤����
//                                     �������а�Ȩ
//                          ***----------------------------***
//       ��  �� : lchar.h 
//     ����ʱ�� : 2011-07-02       ������  : �ޱ�
//     �޸�ʱ�� : 2012-11-25       �޸���  : �ޱ�
//
//     ��Ҫ���� : �ṩ��Ӳ���޹ص��ַ��������� 
//
//      ˵  ��  : 
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
//========================================================================================
//              |   2012-07-02  |  �ޱ�         |  ��type.h�з���
//////////////////////////////////////////////////////////////////////////////////////////
*/

#ifndef _LENIX_CHAR_H_
#define _LENIX_CHAR_H_

/*
 *  CHAR_BACK   : �˸��  
 *  CHAR_TAB    : �Ʊ��  
 *  CHAR_NL     : ���з�  
 *  CHAR_VT     : ��ֱ�Ʊ��  
 *  CHAR_FF     :   
 *  CHAR_CR     : �س���  
 *  CHAR_DEL    : ɾ����  
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