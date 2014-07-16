/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenix实时操作系统
//                              2011 - 2012 @ 源代码工作室
//                                     保留所有版权
//                          ***----------------------------***
//       名  称 : pc.h 
//     创建时间 : 2011-02-09       创建者  : 罗斌
//     修改时间 : 2012-11-25       修改者  : 罗斌
//
//     主要功能 : 提供PC硬件相关的常数，函数 
//
//      说  明  : 
//
//  版本变化记录:
//  版本号      |    时  间     |    作  者     | 主要变化记录
//========================================================================================
//  00.00.000   |  2011-02-09   |    罗  斌     | 第一版
//////////////////////////////////////////////////////////////////////////////////////////
*/
#ifndef _PC_H_
#define _PC_H_

#include <config.h>
#include <type.h>
#include <lchar.h>
#include <cpu.h>

#ifdef _SLASH_
    #include <arch/x86.h>
#else
    #include <arch\x86.h>
#endif  /*  _SLASH_ */

/*  CMOS端口        */
#define PC_CMOS_REG                 ((void *)0x0070)
#define PC_CMOS_DATA                ((void *)0x0071)
/*
 *  8259端口        
 *  M表示主控制器，Master
 *  S表示从控制器，Slavery
 */
#define PC_8259_RS_M                ((void *)0x0020)
#define PC_8259_MASK_M              ((void *)0x0021)
#define PC_8259_RS_S                ((void *)0x00A0)
#define PC_8259_MASK_S              ((void *)0x00A1)
/*  DMA端口         */

/*  键盘、鼠标端口  */
#define PC_KB_DATA                  ((void *)0x0060)
#define PC_KB_STAT_CMD              ((void *)0x0064)
/*  硬盘系统端口    */
#define PC_ATA_CONTROLOR1           ((void *)0x01F0)
#define PC_ATA_CTRL1_REG            ((void *)0x03F6)
#define PC_ATA_CONTROLOR2           ((void *)0x0170)
#define PC_ATA_CTRL2_REG            ((void *)0x0376)

/*  VGA系统端口     */
#define PC_CON_REG_IDX              ((void *)0x03D4)
#define PC_CON_REG_DATA             ((void *)0x03D5)

/*  串行口端口      */
#define PC_UART0                    ((void *)0x03F8)
#define PC_UART1                    ((void *)0x02F8)
#define PC_UART2                    ((void *)0x03E8)
#define PC_UART3                    ((void *)0x03F8)

/*  PCI端口         */
#define PC_CONFIG_ADDRESS           ((void *)0x0CF8)
#define PC_DATA_ADDRESS             ((void *)0x0CFC)

/*  中断分配        */
#define IRQ_BASE                    0x20
#define IRQ_CLOCK                   (IRQ_BASE       )
#define IRQ_KEYBOARD                (IRQ_BASE + 1   )
#define IRQ_COM2                    (IRQ_BASE + 3   )
#define IRQ_COM1                    (IRQ_BASE + 4   )
#define IRQ_IDE_MASTER              (IRQ_BASE + 14  )
#define IRQ_IDE_SLAVRY              (IRQ_BASE + 15  )

dword_t     Ivt_set(int id,dword_t handle);
dword_t     Ivt_get(int id);

void        Irq_clock(void);
void        Irq_keyboard(void);
void        Irq_com1(void);

void        Pic_initial(void);
void        Pc_enable_clock(void);
void        Pc_disable_clock(void);
void        Pc_enable_keyboard(void);
void        Pc_disable_keyboard(void);
void        Pc_enable_com1(void);
void        Pc_disable_com1(void);
void        Pc_clock_frequency(uint16_t frequency);

void        Sys_call_entry(void);
uint_t      Sys_fork(void);

#define TEXT_COLOR_RED              0x4
#define TEXT_COLOR_GREEN            0x2
#define TEXT_COLOR_BLUE             0x1

byte_t      Con_attr_set(byte_t attr);
void        Con_cls(void);
int         Con_write_string(int x,int y ,const char * string,byte_t attr);
void        Con_write(int x,int y,const void * buffer,int size,byte_t attr);
void        Con_print_char(byte_t c);
int         Con_print_string(const char * string);
void        Con_initial(void);

/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  键盘部分
*/
typedef byte_t    (* kb_handler)(byte_t sc);

#define PC_KBLED_CAPSLOCK           0x04
#define PC_KBLED_NUMLOCK            0x02
#define PC_KBLED_SCROLLLOCK         0x01

// 键盘状态
#define KB_STATE_LEFT_CTRL          0x00000001
#define KB_STATE_RIGHT_CTRL         0x00000002
#define KB_STATE_LEFT_ALT           0x00000004
#define KB_STATE_RIGHT_ALT          0x00000008
#define KB_STATE_LEFT_SHIFT         0x00000010
#define KB_STATE_RIGHT_SHIFT        0x00000020

#define KB_STATE_SHIFT              ( KB_STATE_LEFT_SHIFT | KB_STATE_RIGHT_SHIFT )
#define KB_STATE_CTRL               ( KB_STATE_LEFT_CTRL  | KB_STATE_RIGHT_CTRL  )
#define KB_STATE_ALT                ( KB_STATE_LEFT_ALT   | KB_STATE_RIGHT_ALT   )

#define IS_LETTER(c)                (((c)>='a' && (c)<='z') || ((c)>='A' && (c)<='Z'))
#define IS_SHIFT_PRESS(State)       (( State ) & KB_STATE_SHIFT )
#define IS_CTRL_PRESS(State)        (( State ) & KB_STATE_CTRL  )
#define IS_ALT_PRESS(State)         (( State ) & KB_STATE_ALT   )

#define WAIT_KB()                   do{ while( Io_inb(0x64) & 2 ); } while(0)
#define KB_WAIT_WRITE()             do{ while( Io_inb(0x64) & 2 ) Io_delay(); } while(0)
#define KB_WAIT_READ()              do{ while( Io_inb(0x64) & 1 ) Io_delay(); } while(0)

extern volatile dword_t    keyboard_status;
void        Kb_initial(void);

/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  串行口部分
*/

#define RS_BPS_9600                 12
#define RS_BPS_14400                8
#define RS_BPS_19200                6
#define RS_BPS_28800                4
#define RS_BPS_38400                3

void        Rs_initial(void);

#endif  /*  _PC_H_  */