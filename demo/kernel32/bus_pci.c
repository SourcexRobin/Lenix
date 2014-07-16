/*
///////////////////////////////////////////////////////////////////////////////
//                              Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: bus_pci.c 
//  创建时间: 2014-02-03        创建者: 罗斌
//  修改时间:                   修改者: 
//
//  主要功能: pci管理函数和变量
//
//  说    明: 
//
//  变更记录
//  版本号      |   时  间   |   作  者     | 主要变化记录
//=============================================================================
//              | 2014-02-03 |   罗  斌     | 
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>
#include <cpu.h>
#include <lio.h>

#include "bus_pci.h"
#include <machine\pc.h>

static pci_t                pci_pool[PCI_MAX];  /*  系统PCI对象池   */
static void             *   pci_config;         /*  PCI系统配置端口 */
static void             *   pci_data;           /*  PCI系统数据端口 */

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Pci_msg
//  功  能: 输出PCI设备的基本信息
//  参  数: 
//      pci       : pci_t           | pci对象指针
//  返回值: 无
//  注  意: 
//  变更记录
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-04  |   罗  斌      | 第一版
///////////////////////////////////////////////////////////////////////////////
*/
static void Pci_msg(pci_t * pci)
{
    if( NULL == pci )
        return ;
    _printk("PCI flags= %02X bus= %2d device= %2d function=%d reg0= %08X\n",
        pci->pci_flags,pci->pci_bus,pci->pci_device,
        pci->pci_function,pci->pci_reg0);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Pci_config_cfg
//  功  能: 设置PCI系统的配置端口
//  参  数: 
//      cfgaddr     : void *        | 新的配置端口
//  返回值: 原配置端口
//  注  意: 传递参数NULL，不会修改当前的配置端口，即可获得当前的配置端口
//  变更记录
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-04  |   罗  斌      | 第一版
///////////////////////////////////////////////////////////////////////////////
*/
void *      Pci_config_cfg(void * cfgaddr)
{
    void          * pca = pci_config;   /*  原配置端口.prev config addr*/
    if( cfgaddr )
        pci_config = cfgaddr;
    return pca;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Pci_config_data
//  功  能: 设置PCI系统的数据端口
//  参  数: 
//      dataaddr    : void *        | 新的数据端口
//  返回值: 原数据端口
//  注  意: 传递参数NULL，不会修改当前的数据端口，即可获得当前的数据端口
//  变更记录
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-04  |   罗  斌      | 第一版
///////////////////////////////////////////////////////////////////////////////
*/
void *      Pci_config_data(void * dataaddr)
{
    void          * pda = pci_data;     /*  原数据端口.prev data addr*/
    if( dataaddr )
        pci_data = dataaddr;
    return pda;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Pci_register_read
//  功  能: 读PCI设备的寄存器
//  参  数: 
//      pci         : pci_t *       | pci对象指针
//      regidx      : uint_t        | 寄存器索引
//  返回值: 对应寄存器的值，通常返回0xFFFFFFFF表示失败
//  注  意: 
//  变更记录
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-04  |   罗  斌      | 第一版
///////////////////////////////////////////////////////////////////////////////
*/
dword_t     Pci_register_read(pci_t * pci,uint_t regidx)
{
    dword_t         cmd;

    PCI_MAKE_CMD(cmd,pci->pci_bus,pci->pci_device,pci->pci_function,regidx);
    Io_outd(pci_config,cmd);

    return Io_ind(pci_data);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Pci_register_write
//  功  能: 写PCI设备的寄存器
//  参  数: 
//      pci         : pci_t *       | pci对象指针
//      regidx      : uint_t        | 寄存器索引
//      data        : dword_t       | 需要写入的数据
//  返回值: 无
//  注  意: 
//  变更记录
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-04  |   罗  斌      | 第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Pci_register_write(pci_t * pci,uint_t regidx,dword_t data)
{
    dword_t         cmd;

    PCI_MAKE_CMD(cmd,pci->pci_bus,pci->pci_device,pci->pci_function,regidx);
    Io_outd(pci_config,cmd);
    Io_outd(pci_data,data);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Pci_get
//  功  能: 从参数给定的编号开始查找pci设备
//  参  数: 
//      idx         : int *         | 索引变量的地址
//  返回值: 找到PCI设备存在返回非NULL，找不到返回NULL。
//  注  意: 在完成PCI系统的初始化后，可以通过这个API来实现系统PCI设备的遍历。
//    可以采用以下代码实现遍历。
//    int idx = 0;
//    pci_t * pci = NULL;
//    do
//    {
//        pci = Pci_get(&idx);
//        if( pci )
//        {
//            ...
//        }
//    }while(NULL != pci);
//  变更记录
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-04  |   罗  斌      | 第一版
///////////////////////////////////////////////////////////////////////////////
*/
pci_t *     Pci_get(int * idx)
{
    int             i   = 0;
    pci_t         * pci = NULL;

    if( NULL == idx || *idx < 0 )
        return NULL;
    i = *idx;
    while( i < PCI_MAX)
    {
        pci = pci_pool + i;
        i++;
        if( pci->pci_flags & PCI_ACTIVE )
            break;
    }
    *idx = i;

    return i < PCI_MAX ? pci : NULL;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Pci_initial
//  功  能: 初始化PCI系统
//  参  数: 无
//  返回值: 无
//  注  意: 
//  变更记录
//  时  间      |   作  者      | 说  明
//=============================================================================
//  2014-02-04  |   罗  斌      | 第一版
///////////////////////////////////////////////////////////////////////////////
*/
void        Pci_initial(void)
{
    /*
     *    在系统初始化的时候，需要探测系统中存在的PCI设备。
     *    通过扫描所有总线、设备和功能号的寄存器0来确认设备是否存在。这是基于该
     *  寄存器在任何PCI设备上都存在并且只读的条件来实现的。扫描到的PCI设备顺序
     *  记录到系统的PCI设备池中
     */
    uint_t          fun     = 0,
                    dev     = 0,
                    bus     = 0;
    dword_t         cmd     = 0;
    int             idx     = 0;
    dword_t         reg0    = 0xFFFFFFFF;
    pci_t         * pci     = NULL;

    pci_config = PC_CONFIG_ADDRESS;
    pci_data   = PC_DATA_ADDRESS;
    _printk("PC PCI initial...    ");
    for( bus = 0 ; bus < 256 ; bus++)
    {
        for( dev = 0 ; dev < 32 ; dev++)
        {
            for( fun = 0 ; fun < 8 ; fun++ )
            {
                PCI_MAKE_CMD(cmd,bus,dev,fun,0);
                Io_outd(pci_config,cmd);
                reg0 = Io_ind(pci_data);
                if( reg0 != 0xFFFFFFFF)
                {
                    pci_pool[idx].pci_bus        = bus;
                    pci_pool[idx].pci_device     = dev;
                    pci_pool[idx].pci_function   = fun;
                    pci_pool[idx].pci_flags      = PCI_ACTIVE;
                    pci_pool[idx].pci_reg0       = reg0;
                    idx++;
                }
                if( idx >= PCI_MAX )
                    goto pci_initial_end;
            }
        }
    }
pci_initial_end:
    _printk("OK!\n");
#ifdef _CFG_DEBUG_
    idx = 0;
    do
    {
        pci = Pci_get(&idx);
        if( pci )
            Pci_msg(pci);
    }while(NULL != pci);
#endif /*  _CFG_DEBUG  */
}