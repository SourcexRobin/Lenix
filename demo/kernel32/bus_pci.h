/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: bus_pci.h 
//  创建时间: 2014-02-03        创建者: 罗斌
//  修改时间:                   修改者: 
//
//  主要功能: 提供pci相关的定义，数据结构、操作函数，端口等等
//
//  说    明: 
//
//  变更记录:
//  版 本 号    |   时  间   |   作  者     | 主要变化记录
//=============================================================================
//  00.00.000   | 2014-02-03 |   罗  斌     | 
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>

#ifndef _BUS_PCI_H_
#define _BUS_PCI_H_

#include <type.h>

#define PCI_MAX                     16

/*
 *  PCI设备对象
 */
typedef struct _pci_t
{
    byte_t                  pci_flags;          /*  设备标志    */
    byte_t                  pci_bus;            /*  总线号      */
    byte_t                  pci_device;         /*  设备号      */
    byte_t                  pci_function;       /*  功能号      */
    dword_t                 pci_reg0;           /*  寄存器0     */
}pci_t;

#define PCI_ACTIVE                  0x01

/*
 *  PCI配置命令格式
 */
typedef struct _pc_pci_config_t
{
    byte_t                  ppc_type:2;         /*  类型        */
    byte_t                  ppc_register:6;     /*  寄存器索引  */
    byte_t                  ppc_function:3;     /*  功能        */
    byte_t                  ppc_device:5;       /*  设备号      */
    byte_t                  ppc_bus;            /*  总线号      */
    byte_t                  ppc_rsvd:7;         /*  保留        */
    byte_t                  ppc_enable:1;       /*  允许        */
}pci_config_t;

/*
 *  构造PCI访问命令
 *  c:  命令双字
 *  b:  总线号,bufs
 *  d:  设备号,device
 *  f:  功能号,function
 *  r:  寄存器索引，register
 */
#define PCI_MAKE_CMD(c,b,d,f,r)     do{ (c)  = 0x80000000; \
                                        (c) |= ((b) & 0xFF) << 16; \
                                        (c) |= ((d) & 0x1F) << 11; \
                                        (c) |= ((f) & 0x07) <<  8; \
                                        (c) |= ((r) & 0x3F);}while(0)
/*
 *  PCI设备头
 */
typedef struct _pci_config_header_type0_t
{
    word_t                  pch_vendor_id;
    word_t                  pch_device_id;
    byte_t                  pch_revision_id;
    byte_t                  pch_class_code[3];
    byte_t                  pch_cache_line_size;
    byte_t                  pch_latency_timer;
    byte_t                  pch_header_type;
    byte_t                  pch_bist;
    dword_t                 pch_bar[6];
    dword_t                 pch_cardbus_cis_pointer;
    word_t                  pch_subsys_vender_id;
    word_t                  pch_subsys_id;
    dword_t                 pch_ex_rom_baseaddr;
    byte_t                  pch_capa_pointer;
    byte_t                  pch_rsvd1[3];
    dword_t                 pch_rsvd2;
    byte_t                  pch_int_line;
    byte_t                  pch_int_pin;
    byte_t                  pch_min_gnt;
    byte_t                  pch_max_lat;
}pci_cfg_header0_t;

void *      Pci_config_cfg(void * cfgaddr);
void *      Pci_config_data(void * dataaddr);
dword_t     Pci_register_read (pci_t * pci,uint_t regidx);
void        Pci_register_write(pci_t * pci,uint_t regidx,dword_t data);
void        Pci_initial(void);

#endif /*  _BUS_PCI_H_  */
