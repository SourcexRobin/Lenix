/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: pc_ata.c
//  创建时间: 2012-06-27        创建者: 罗斌
//  修改时间: 2014-02-22        修改者: 罗斌
//  主要功能: 提供进程管理功能。
//  说    明: 
//
//  变更记录:
//  版 本 号    |     时间      |  作  者       |  主要变化记录
//=============================================================================
//  00.00.000   |  2012-06-27   |  罗  斌       |  建立文件
///////////////////////////////////////////////////////////////////////////////
*/

#include <result.h>
#include <lio.h>

#ifdef _SLASH_
    #include <machine/pc.h>
    #include <driver/pc_ata.h>
#else
    #include <machine\pc.h>
    #include <driver\pc_ata.h>
#endif

#define ATA_RETRY_TIMES             10000

/*
//////////////////////////////////////////////////////////////////////////////
////////////////////
//  ATA基本操作
*/
#define ATA_COMMAND(ata,cmd)            Io_outb(ATA_PORT_CMD(ata),cmd)
#define ATA_STATUS(ata)                 Io_inb(ATA_PORT_STATUS(ata))
#define ATA_ERROR(ata)                  Io_inb(ATA_PORT_ERR(ata))

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Ata_device_ready
//  作  用: 设备就绪，可以使用。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     ata_t *      |   ata         |   ata对象
//
//  返回值: 可用返回RESULT_SUCCEED，失败返回RESULT_FAILED。
//  说  明: 
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-02-22  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
result_t    Ata_device_ready(ata_t * ata)
{
    int             retry   = ATA_RETRY_TIMES;
    result_t        result  = RESULT_FAILED;

    while( retry-- > 0 )
    {
        if( ATA_STATUS(ata) & ATA_STATUS_DRDY )
        {
            result = RESULT_SUCCEED;
            break;
        }
        Io_delay();
    }
    return result;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Ata_data_request
//  作  用: 数据就绪。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     ata_t *      |   ata         |   ata对象
//  返回值: 可用返回RESULT_SUCCEED，失败返回RESULT_FAILED。
//  说  明: 
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-02-22  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static 
result_t    Ata_data_request(ata_t * ata)
{
    int             retry   = ATA_RETRY_TIMES;
    result_t        result  = RESULT_FAILED;

    while( retry-- > 0 )
    {
        if( ATA_STATUS(ata) & ATA_STATUS_DRQ )
        {
            result = RESULT_SUCCEED;
            break;
        }
        Io_delay();
   }
    return result;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Ata_size
//  作  用: 获得磁盘大小。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     void *       |   port        |   ata端口
//
//  返回值: 成功返回磁盘大小，失败返回0。
//  说  明: 通过读取ata设备参数表来获得硬盘大小。
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-02-21  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
qword_t     Ata_size(ata_t * ata)
{
    byte_t          dev     = 0xE0;
    byte_t        * port    = NULL;
    byte_t          err     = 0;
    qword_t         size    = 0;
    word_t          buf[256]= {0,};

    if( ata->ata_flag & ATA_FLAG_ADDR48 )
        goto ata_addr48;
    /*
     *  28位ata地址处理。
     */
    _memzero(buf,512);
    port = ata->ata_io_addr;
    if( ata->ata_flag & ATA_FLAG_SLAVE )
        dev = 0xF0;
    if( Ata_device_ready(ata) != RESULT_SUCCEED )
        goto ata_size_end;
    Io_outb(port + 2,0);
    Io_outb(port + 3,0);
    Io_outb(port + 4,0);
    Io_outb(port + 5,0);
    Io_outb(port + 6,dev);
    Io_outb(port + 7,ATA_CMD_IDENTIFY_DEVICE);
    if( Ata_data_request(ata) != RESULT_SUCCEED )
        goto ata_size_end;
    Io_inw_buffer(port,buf,256);
    size = *((dword_t *)(buf + 60));
    goto ata_size_end;
ata_addr48:
    /*
     *  48位ata地址处理
     */

ata_size_end:
    return size;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Ata_command
//  作  用: 向控制器发送命令的基本参数。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     ata_t *      |   ata         |   ata对象指针
//     byte_t       |   cmd         |   命令
//     qword_t      |   address     |   命令地址
//     byte_t       |   sectors     |   扇区数
//
//  返回值: 成功返回RESULT_SUCCEED，失败返回RESULT_FAILED。
//  说  明: 通过这个函数简化命令发送。
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-02-22  |   罗  斌      |  第一版
//  2012-06-28  |   罗  斌      |  建立函数
///////////////////////////////////////////////////////////////////////////////
*/
static 
result_t    Ata_command(ata_t * ata,byte_t cmd,qword_t address,byte_t sectors)
{
    byte_t        * addr    = NULL;             /*  以字节方式表示的地址  */
    byte_t        * port    = NULL;             /*  端口        */
    result_t        result  = RESULT_FAILED;    /*  返回值      */

    if( ata->ata_flag & ATA_FLAG_ADDR48 )
        goto ata_addr48;
    /*
     *  28位ata地址处理。
     */
    addr = (byte_t *)&address;
    port = ATA_PORT_CNT(ata);
    if( ATA_IS_SLAVE(ata) )
        address = (address & 0x0FFFFFFF) | 0xF0000000;
    else
        address = (address & 0x0FFFFFFF) | 0xE0000000;
    if( Ata_device_ready(ata) != RESULT_SUCCEED )
        goto ata_cmd_end;
    Io_outb(port++,sectors);
    Io_outb(port++,addr[0]);
    Io_outb(port++,addr[1]);
    Io_outb(port++,addr[2]);
    Io_outb(port++,addr[3]);
    Io_outb(port  ,cmd    );
    result = RESULT_SUCCEED;
    goto ata_cmd_end;
ata_addr48:
    /*
     *  48位ata地址处理
     */
ata_cmd_end:
    return result;
}


static 
result_t    Ata_seek_sector(ata_t * ata,offset_t pos)
{
    ata = ata;
    pos = pos;
    return RESULT_FAILED;
}


static 
int         Ata_read_sectors(ata_t * ata,offset_t pos,void * buffer,
                             byte_t sectors)
{
    byte_t          left = sectors;

    if( pos < 0 )
        return 0;
    while(left && (qword_t)pos < ata->ata_size)
    {
        if( Ata_command(ata,ATA_CMD_READ_SECTORS,pos,1) != RESULT_SUCCEED )
            break;
        if( Ata_data_request(ata) != RESULT_SUCCEED )
            break;
        Io_inw_buffer(ATA_PORT_DATA(ata),buffer,256);
        ++pos;
        --left;
        ((byte_t *)(buffer)) += 512;
    }

    return sectors - left;
}

static 
int         Ata_write_sectors(ata_t * ata,offset_t pos,void * buffer,
                              int sectors)
{
    byte_t          left = sectors;

    if( pos < 0 )
        return 0;
    while(left && (qword_t)pos < ata->ata_size)
    {
        if( Ata_command(ata,ATA_CMD_READ_SECTORS,pos,1) != RESULT_SUCCEED )
            break;
        if( Ata_data_request(ata) != RESULT_SUCCEED )
            break;
        Io_outw_buffer(ATA_PORT_DATA(ata),buffer,256);
        ++pos;
        --left;
        ((byte_t *)(buffer)) += 512;
    }

    return sectors - left;
}



/*
//////////////////////////////////////////////////////////////////////////////
////////////////////
//  ATA驱动部分
*/
static result_t Ata_open    (struct _device_t * device)
{
    device = device;
    return RESULT_SUCCEED;
}

static result_t Ata_close   (struct _device_t * device)
{
    device = device;
    return RESULT_SUCCEED;
}


//  读ATA设备，2012/06/27
static 
int         Ata_read(device_t   * device,offset_t   pos,
                     void       * buffer,int        size)
{
    ata_t         * ata     = NULL;
    byte_t        * buf     = NULL;
    int             left    = 0;

    ata     = DEV_DATA(device);
    buf     = buffer;
    left    = size;
    if( pos < 0 )
        return 0;
    while(left >= 512 && (qword_t)pos < ata->ata_size )
    {
        if( Ata_read_sectors(ata,pos,buf,1) < 1 )
            break;
        ++pos;
        left    -= 512;
        buf     += 512;
    }

    return size - left;
}

static
int         Ata_write   (device_t   * device,offset_t   pos,
                         const void * buffer,int        size)
{
    ata_t         * ata     = NULL;
    const byte_t  * buf     = NULL;
    int             left    = 0;

    ata     = DEV_DATA(device);
    buf     = buffer;
    left    = size;
    if( pos < 0 )
        return -1;
    while(left >= 512 && (qword_t)pos < ata->ata_size)
    {
        if( Ata_write_sectors(ata,pos,(void *)buf,1) < 1 )
            break;
        ++pos;
        left    -= 512;
        buf     += 512;
    }

    return size - left;
}


static result_t Ata_ctrl    (struct _device_t * device,byte_t cmd,void * arg)
{
    ata_t               *   ata = NULL;

    ata = DEV_DATA(device);

    switch(cmd)
    {
    case 1:
        ata = ata;
        arg = arg;
        break;
    default:
        /*
         *  不支持的命令都看作操作失败
         */
        return RESULT_FAILED;
    }

    return RESULT_FAILED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名  称: Ata_entry
//  作  用: 驱动入口。
//  参  数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   device      |   设备对象指针
//     int          |   flag        |   入口标志
//             DEV_ENTRY_FLAG_REG   |   注册时，有系统传入
//             DEV_ENTRY_FLAG_UNREG |   卸载时，由系统传入
//     void *       |   param       |   初始化参数。解释为整数指针，数值从0开
//                                  | 始。对于并口IDE，0表示第一控制器的主盘，
//                                  | 1表示第一控制器的从盘。以此类推
//  返回值:   无
//  说  明:   
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-02-21  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Ata_entry(device_t * device,int flag,void * param)
{
    void          * aa[4]   = {PC_ATA_CONTROLOR1,
                               PC_ATA_CONTROLOR1,
                               PC_ATA_CONTROLOR2,
                               PC_ATA_CONTROLOR2};  /*  ata address */
    ata_t         * ata     = NULL;                 /*  ata对象     */
    int             id      = *(int *)param & 3;    /*  设备编号    */

    switch(flag)
    {
    case DEV_ENTRY_FLAG_REG:
        /*
         *  向系统注册
         */
        device->dev_data_ext = NULL;
        ata = DEV_DATA(device);
        /*
         *  解析参数，获得io地址、主从盘信息，以及磁盘大小
         */
        ata->ata_io_addr = aa[id];
        if( id & 1 )
            ata->ata_flag |= ATA_FLAG_SLAVE;
        if( 0 == ( ata->ata_size = Ata_size(ata) ) )
            return RESULT_FAILED;
        /*
         *  设置DDO对象定义的接口
         */
        device->dev_ddo.ddo_open    = Ata_open;
        device->dev_ddo.ddo_close   = Ata_close;
        device->dev_ddo.ddo_read    = Ata_read;
        device->dev_ddo.ddo_write   = Ata_write;
        device->dev_ddo.ddo_ctrl    = Ata_ctrl;
        /*  将设备设置为块设备和PIO方式*/
        device->dev_flag        = DEV_FLAG_BLOCK | DEV_IO_PIO;
        device->dev_capcity     = ata->ata_size;
        device->dev_io_addr[0]  = ata->ata_io_addr;
        device->dev_ivtid       = ISP_DISK;
        
        return RESULT_SUCCEED;
    case DEV_ENTRY_FLAG_UNREG:
        ata = DEV_DATA(device);
        return RESULT_SUCCEED;
    }
    return RESULT_FAILED;
}

