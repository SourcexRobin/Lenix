/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    Ƕ��ʽ����ϵͳ
//                             2011 , ��  ��Դ���빤����
//                                     �������а�Ȩ
//                          ***---------------------------***
//
//  �ļ�����    : pc_vga.h
//  �ļ�������  : ��  ��
//  �ļ�����ʱ��:
//  ����޸���  : ��  ��
//  ����޸�ʱ��:
//
//  ��Ҫ����    : �ṩ�������ڴ����������
//
//  ˵��        : 
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
//========================================================================================
//              |   2012-06-29  |  �ޱ�         |  ��һ�档����Ѿ����ԣ���δʵ�־��幦��
//////////////////////////////////////////////////////////////////////////////////////////
*/


#ifndef _PC_VGA_H_
#define _PC_VGA_H_

#include <config.h>
#include <type.h>

#ifdef _SLASH_
    #include <kernel/device.h>
#else
    #include <kernel\device.h>
#endif  /*  _SLASH_ */

typedef struct _vga_t
{
    void FAR            *   vga_buffer;
    byte_t                  vga_mode;           /*  ��ʾģʽ    */
    byte_t                  vga_attr;
    uint16_t                vga_scale_x;        /*  ����ֱ���  */
    uint16_t                vga_scale_y;        /*  ����ֱ���  */
}vga_t;

#define VGA_CMD_UNDEF               0
#define VGA_CMD_SET_ATTR            1

result_t    Vga_entry(device_t * device,int flag,void * param);

#endif  /*  _PC_VGA_H_  */