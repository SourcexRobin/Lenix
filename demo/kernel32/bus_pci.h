/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: bus_pci.h 
//  ����ʱ��: 2014-02-03        ������: �ޱ�
//  �޸�ʱ��:                   �޸���: 
//
//  ��Ҫ����: �ṩpci��صĶ��壬���ݽṹ�������������˿ڵȵ�
//
//  ˵    ��: 
//
//  �����¼:
//  �� �� ��    |   ʱ  ��   |   ��  ��     | ��Ҫ�仯��¼
//=============================================================================
//  00.00.000   | 2014-02-03 |   ��  ��     | 
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>

#ifndef _BUS_PCI_H_
#define _BUS_PCI_H_

#include <type.h>

#define PCI_MAX                     16

/*
 *  PCI�豸����
 */
typedef struct _pci_t
{
    byte_t                  pci_flags;          /*  �豸��־    */
    byte_t                  pci_bus;            /*  ���ߺ�      */
    byte_t                  pci_device;         /*  �豸��      */
    byte_t                  pci_function;       /*  ���ܺ�      */
    dword_t                 pci_reg0;           /*  �Ĵ���0     */
}pci_t;

#define PCI_ACTIVE                  0x01

/*
 *  PCI���������ʽ
 */
typedef struct _pc_pci_config_t
{
    byte_t                  ppc_type:2;         /*  ����        */
    byte_t                  ppc_register:6;     /*  �Ĵ�������  */
    byte_t                  ppc_function:3;     /*  ����        */
    byte_t                  ppc_device:5;       /*  �豸��      */
    byte_t                  ppc_bus;            /*  ���ߺ�      */
    byte_t                  ppc_rsvd:7;         /*  ����        */
    byte_t                  ppc_enable:1;       /*  ����        */
}pci_config_t;

/*
 *  ����PCI��������
 *  c:  ����˫��
 *  b:  ���ߺ�,bufs
 *  d:  �豸��,device
 *  f:  ���ܺ�,function
 *  r:  �Ĵ���������register
 */
#define PCI_MAKE_CMD(c,b,d,f,r)     do{ (c)  = 0x80000000; \
                                        (c) |= ((b) & 0xFF) << 16; \
                                        (c) |= ((d) & 0x1F) << 11; \
                                        (c) |= ((f) & 0x07) <<  8; \
                                        (c) |= ((r) & 0x3F);}while(0)
/*
 *  PCI�豸ͷ
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
