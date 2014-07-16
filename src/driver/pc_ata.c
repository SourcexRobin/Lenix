/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: pc_ata.c
//  ����ʱ��: 2012-06-27        ������: �ޱ�
//  �޸�ʱ��: 2014-02-22        �޸���: �ޱ�
//  ��Ҫ����: �ṩ���̹����ܡ�
//  ˵    ��: 
//
//  �����¼:
//  �� �� ��    |     ʱ��      |  ��  ��       |  ��Ҫ�仯��¼
//=============================================================================
//  00.00.000   |  2012-06-27   |  ��  ��       |  �����ļ�
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
//  ATA��������
*/
#define ATA_COMMAND(ata,cmd)            Io_outb(ATA_PORT_CMD(ata),cmd)
#define ATA_STATUS(ata)                 Io_inb(ATA_PORT_STATUS(ata))
#define ATA_ERROR(ata)                  Io_inb(ATA_PORT_ERR(ata))

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Ata_device_ready
//  ��  ��: �豸����������ʹ�á�
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     ata_t *      |   ata         |   ata����
//
//  ����ֵ: ���÷���RESULT_SUCCEED��ʧ�ܷ���RESULT_FAILED��
//  ˵  ��: 
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-02-22  |   ��  ��      |  ��һ��
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
//  ��  ��: Ata_data_request
//  ��  ��: ���ݾ�����
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     ata_t *      |   ata         |   ata����
//  ����ֵ: ���÷���RESULT_SUCCEED��ʧ�ܷ���RESULT_FAILED��
//  ˵  ��: 
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-02-22  |   ��  ��      |  ��һ��
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
//  ��  ��: Ata_size
//  ��  ��: ��ô��̴�С��
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     void *       |   port        |   ata�˿�
//
//  ����ֵ: �ɹ����ش��̴�С��ʧ�ܷ���0��
//  ˵  ��: ͨ����ȡata�豸�����������Ӳ�̴�С��
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-02-21  |   ��  ��      |  ��һ��
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
     *  28λata��ַ����
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
     *  48λata��ַ����
     */

ata_size_end:
    return size;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Ata_command
//  ��  ��: ���������������Ļ���������
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     ata_t *      |   ata         |   ata����ָ��
//     byte_t       |   cmd         |   ����
//     qword_t      |   address     |   �����ַ
//     byte_t       |   sectors     |   ������
//
//  ����ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���RESULT_FAILED��
//  ˵  ��: ͨ���������������͡�
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-02-22  |   ��  ��      |  ��һ��
//  2012-06-28  |   ��  ��      |  ��������
///////////////////////////////////////////////////////////////////////////////
*/
static 
result_t    Ata_command(ata_t * ata,byte_t cmd,qword_t address,byte_t sectors)
{
    byte_t        * addr    = NULL;             /*  ���ֽڷ�ʽ��ʾ�ĵ�ַ  */
    byte_t        * port    = NULL;             /*  �˿�        */
    result_t        result  = RESULT_FAILED;    /*  ����ֵ      */

    if( ata->ata_flag & ATA_FLAG_ADDR48 )
        goto ata_addr48;
    /*
     *  28λata��ַ����
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
     *  48λata��ַ����
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
//  ATA��������
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


//  ��ATA�豸��2012/06/27
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
         *  ��֧�ֵ������������ʧ��
         */
        return RESULT_FAILED;
    }

    return RESULT_FAILED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��  ��: Ata_entry
//  ��  ��: ������ڡ�
//  ��  ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   device      |   �豸����ָ��
//     int          |   flag        |   ��ڱ�־
//             DEV_ENTRY_FLAG_REG   |   ע��ʱ����ϵͳ����
//             DEV_ENTRY_FLAG_UNREG |   ж��ʱ����ϵͳ����
//     void *       |   param       |   ��ʼ������������Ϊ����ָ�룬��ֵ��0��
//                                  | ʼ�����ڲ���IDE��0��ʾ��һ�����������̣�
//                                  | 1��ʾ��һ�������Ĵ��̡��Դ�����
//  ����ֵ:   ��
//  ˵  ��:   
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-02-21  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Ata_entry(device_t * device,int flag,void * param)
{
    void          * aa[4]   = {PC_ATA_CONTROLOR1,
                               PC_ATA_CONTROLOR1,
                               PC_ATA_CONTROLOR2,
                               PC_ATA_CONTROLOR2};  /*  ata address */
    ata_t         * ata     = NULL;                 /*  ata����     */
    int             id      = *(int *)param & 3;    /*  �豸���    */

    switch(flag)
    {
    case DEV_ENTRY_FLAG_REG:
        /*
         *  ��ϵͳע��
         */
        device->dev_data_ext = NULL;
        ata = DEV_DATA(device);
        /*
         *  �������������io��ַ����������Ϣ���Լ����̴�С
         */
        ata->ata_io_addr = aa[id];
        if( id & 1 )
            ata->ata_flag |= ATA_FLAG_SLAVE;
        if( 0 == ( ata->ata_size = Ata_size(ata) ) )
            return RESULT_FAILED;
        /*
         *  ����DDO������Ľӿ�
         */
        device->dev_ddo.ddo_open    = Ata_open;
        device->dev_ddo.ddo_close   = Ata_close;
        device->dev_ddo.ddo_read    = Ata_read;
        device->dev_ddo.ddo_write   = Ata_write;
        device->dev_ddo.ddo_ctrl    = Ata_ctrl;
        /*  ���豸����Ϊ���豸��PIO��ʽ*/
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

