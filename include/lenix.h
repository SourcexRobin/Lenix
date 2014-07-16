/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: lenix.h
//  ����ʱ��: 2011-12-02        ������: �ޱ�
//  �޸�ʱ��: 2012-11-29        �޸���: �ޱ�
//  ��Ҫ����: �ṩ����Lenix��ͷ�ļ���
//  ˵    ��: 
//
//  �����¼:
//  �� �� ��    |     ʱ��      |  ��  ��       | ��Ҫ�仯��¼
//=============================================================================
//              |   2014-02-26  |  ��  ��       | ����koum.h
//              |   2012-07-31  |  ��  ��       | ���ӷ�windows·��֧��
//              |   2011-12-02  |  ��  ��       | ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
#ifndef _LENIX_H_
#define _LENIX_H_

#include <const.h>
#include <config.h>
#include <type.h>
#include <cpu.h>
#include <result.h>
#include <assert.h>

#include <lmemory.h>
#include <lchar.h>
#include <lstring.h>
#include <larg.h>
#include <lio.h>
#include <slist.h>
#include <ltime.h>      /*  2014-03-16 ���     */
#include <lmath.h>      /*  2014-03-16 ���     */

#include <koum.h>
#include <proc.h>
#include <ipc.h>
#include <clock.h>
#include <mm.h>
#include <sys.h>
#include <device.h>
#include <shell.h>
#include <tty.h>


void        Lenix_initial(void);
void        Lenix_start(void);
void        Lenix_start_hook(void);

#ifdef _SLASH_

    #include <gui/graph.h>
    #include <gui/window.h>
    #include <machine/machine.h>

    #include <driver/pc_ata.h>
    #include <driver/pc_vga.h>

#else

    #include <gui\graph.h>
    #include <gui\window.h>
    #include <machine\machine.h>

    #include <driver\pc_ata.h>
    #include <driver\pc_vga.h>

#endif  /*  _SLASH_     */
#endif  /*  _LENIX_H_   */