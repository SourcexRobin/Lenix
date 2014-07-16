/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    嵌入式操作系统
//                             2011 , 罗  斌，源代码工作室
//                                     保留所有版权
//                          ***---------------------------***
//
//  文件名称    : pc_ata.h
//  文件创建者  : 罗  斌
//  文件创建时间:
//  最后修改者  : 罗  斌
//  最后修改时间:
//
//  主要功能    : 提供基本的内存操作函数。
//
//  说明        : 
//
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//========================================================================================
//              |   2012-06-27  |  罗斌         |  建立文件，尚未完成
//////////////////////////////////////////////////////////////////////////////////////////
*/

#ifndef _ATA_H_
#define _ATA_H_

#include <config.h>
#include <type.h>

#ifdef _SLASH_
    #include <kernel/device.h>
#else
    #include <kernel\device.h>
#endif  /*  _SLASH_ */

typedef struct _ata_t
{
    void                *   ata_io_addr;    /*  io地址      */
    uint32_t                ata_flag;       /*  设备标志    */
    qword_t                 ata_size;       /*  设备容量    */
}ata_t;

#define ATA_FLAG_SLAVE              0x00000001
#define ATA_FLAG_ADDR48             0x00000002

/*
//////////////////////////////////////////////////////////////////////////////
////////////////////
//  ATA设备状态
*/

/*
 *  BSY: BUSY
 */
#define ATA_STATUS_BSY              0x80
/*
 *  DRDY: DEVICE READY
 *
 *  以下摘自ATA技术文档
 *  5.15.6.2 DRDY (Device ready)
 *  The DRDY bit shall be cleared to zero by the device:
 *      1) when power-on, hardware, or software reset or DEVICE RESET or 
 *    EXECUTE DEVICE DIAGNOSTIC commands for devices implementing the PACKET
 *    command feature set.
 *    When the DRDY bit is cleared to zero, the device shall accept and 
 *  attempt to execute commands as described in volume 2, clause 3.
 *    The DRDY bit shall be set to one by the device:
 *      1) when the device is capable of accepting all commands for devices 
 *        not implementing the PACKET command feature set;
 *      2) prior to command completion except the DEVICE RESET or EXECUTE 
 *        DEVICE DIAGNOSTIC command for devices implementing the PACKET 
 *        feature set.
 *    When the DRDY bit is set to one:
 *      1) the device shall accept and attempt to execute all implemented 
 *        commands;设备应接受并尝试执行所有已实现的命令。
 *      2) devices that implement the Power Management feature set shall 
 *        maintain the DRDY bit set to one when they are in the Idle or 
 *        Standby modes.
 */
#define ATA_STATUS_DRDY             0x40
/*
 *  DF: DEVICE FAULT
 */
#define ATA_STATUS_DF               0x20
/*
 *  SERV:SERVICE
 */
#define ATA_STATUS_SERV             0x10
/*
 *  DRQ:  DATA REUEST
 *
 *  以下摘自ATA技术文档
 *  5.15.6.5 DRQ (Data request)
 *    DRQ indicates that the device is ready to transfer data between the host
 *  and the device. After the host has written the Command register the device
 *  shall either set the BSY bit to one or the DRQ bit to one, until command 
 *  completion or the device has performed a bus release for an overlapped 
 *  command.
 *    The DRQ bit shall be set to one by the device:
 *      1) when BSY is set to one and data is ready for PIO transfer;
 *      2) during the data transfer of DMA commands either the BSY bit, the 
 *        DRQ bit, or both shall be set to one.
 *    When the DRQ bit is set to one, the host may:
 *      1) transfer data via PIO mode;
 *      2) transfer data via DMA mode if DMARQ and DMACK- are asserted.
 *    The DRQ bit shall be cleared to zero by the device:
 *      1) when the last word of the data transfer occurs;
 *      2) when the last word of the command packet transfer occurs for a 
 *        PACKET command.
 *    When the DRQ bit is cleared to zero, the host may:
 *      1) transfer data via DMA mode if DMARQ and DMACK- are asserted and BSY
 *        is set to one.
 */
#define ATA_STATUS_DRQ              0x08
/*
 *  ERR:ERROR
 */
#define ATA_STATUS_ERR              0x01

/*
 *  ICRC: INVALED CRC
 */
#define ATA_ERR_ICRC                0x80
/*
 *  UNC:  DATA UNCORRECTABLR
 */
#define ATA_ERR_UNC                 0x40
/*
 *  WP: WRITE PROTECTED
 */
#define ATA_ERR_WP                  0x40
/*
 *  MC:  MEDIA CHANGED
 */
#define ATA_ERR_MC                  0x20
/*
 *  IDNF:  ADDRESS NOT FOUND
 */
#define ATA_ERR_IDNF                0x10
/*
 *  MCR:MEDIA CHANG REQUEST
 */
#define ATA_ERR_MCR                 0x08
/*
 *  ABRT:ABORT
 */
#define ATA_ERR_ABRT                0x04
/*
 *  NM:NO MEDIA
 */
#define ATA_ERR_NM                  0x02

#define ATA_OUTPUT_MASK             0xE9
#define ATA_STATUS_NORMAL           (ATA_STATUS_BSY | ATA_STATUS_DRDY | \
                                     ATA_STATUS_DF  | ATA_STATUS_DRQ  | \
                                     ATA_STATUS_ERR)
/*
//////////////////////////////////////////////////////////////////////////////
////////////////////
//  ATA设备命令
*/

#define ATA_CMD_READ_SECTORS        0x20
#define ATA_CMD_READEX
#define ATA_CMD_WRITE_SECTORS       0x30
#define ATA_CMD_WRITEEX
#define ATA_CMD_IDENTIFY_DEVICE     0xEC



#define ATA_IS_MASTER(ata)          (((ata)->ata_flag & 1 ) == 0 )
#define ATA_IS_SLAVE(ata)           ( (ata)->ata_flag & 1 ) 

/*
//////////////////////////////////////////////////////////////////////////////
////////////////////
//  ATA寄存器定义
*/
#define ATA_REG_COMMAND             7
#define ATA_REG_STATUS              7
#define ATA_REG_DATA                0
#define ATA_REG_DEVICE              6
#define ATA_REG_DEV_CTRL            6
#define ATA_REG_ERR                 1
#define ATA_REG_FEATURES            1
#define ATA_REG_LBA_HIGH            5
#define ATA_REG_LBA_MID             4
#define ATA_REG_LBA_LOW             3
#define ATA_REG_SECT_CNT            2
/*
 *  获得ATA的命令、状态、数据、扇区数、扇区、错误端口
 */
#define ATA_PORT_BASE(ata)          ((byte_t *)((ata)->ata_io_addr))
#define ATA_PORT_CMD(ata)           ((void *)(ATA_PORT_BASE(ata) + 7 ))
#define ATA_PORT_STATUS(ata)        ((void *)(ATA_PORT_BASE(ata) + 7 ))
#define ATA_PORT_DATA(ata)          ((void *)(ATA_PORT_BASE(ata) + 0 ))
#define ATA_PORT_CNT(ata)           ((void *)(ATA_PORT_BASE(ata) + 2 ))
#define ATA_PORT_SECT(ata)          ((void *)(ATA_PORT_BASE(ata) + 3 ))
#define ATA_PORT_ERR(ata)           ((void *)(ATA_PORT_BASE(ata) + 1 ))

result_t    Ata_entry(device_t * device,int flag,void * param);

#endif  /*  _ATA_H_     */