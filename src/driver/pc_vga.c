

// 2012/6/29 建立文件

#include <result.h>

#include <lio.h>

#ifdef _SLASH_
    #include <machine/pc.h>
    #include <driver/pc_vga.h>
#else
    #include <machine\pc.h>
    #include <driver\pc_vga.h>
#endif

/*
 *  vga驱动部分
 */
static result_t Vga_open    (struct _device_t * device)
{
    _printf("LDM vga driver open \n");
    if( NULL == device )
        return RESULT_FAILED;
    return RESULT_SUCCEED;
}
static result_t Vga_close   (struct _device_t * device)
{
    _printf("LDM vga driver close\n");
    if( NULL == device )
        return RESULT_FAILED;

    return RESULT_SUCCEED;
}

static
int         Vga_read    (device_t   * device,offset_t   pos,
                         void       * buffer,int        size)
{
    vga_t               *   vga     = NULL;
    byte_t              *   dbuf    = buffer;
    byte_t FAR          *   sbuf    = NULL;
    int                     left    = size;
    uint32_t                range   = 0;

    _printf("LDM vga driver read\n");
    if( NULL == device )
        return RESULT_FAILED;

    vga     = DEV_DATA(device);
    range   = vga->vga_scale_x * vga->vga_scale_y * 2;

    if( pos >= range )
        return 0;
    
    sbuf    = (byte_t FAR *)vga->vga_buffer;

    while(left > 0 && pos < range )
    {
        *dbuf++ = sbuf[pos++];
        left--;
    }
    
    return size - left;
}
static 
int         Vga_write   (device_t   * device,offset_t   pos,
                         const void * buffer,int        size)
{
    vga_t               *   vga     = NULL;
    byte_t FAR          *   dbuf    = NULL;
    const byte_t        *   sbuf    = buffer;
    int                     left    = size;
    uint32_t                range   = 0;

    _printf("LDM vga driver write\n");
    if( NULL == device )
        return RESULT_FAILED;

    vga     = DEV_DATA(device);
    range   = vga->vga_scale_x * vga->vga_scale_y * 2;

    if( pos >= range )
        return 0;
    
    dbuf    = (byte_t FAR *)vga->vga_buffer;

    while(left > 0 && pos < range )
    {
        dbuf[pos++] = *sbuf++;
        left--;
    }
    
    return size - left;
}


static result_t Vga_ctrl    (struct _device_t * device,byte_t cmd,void * arg)
{
    vga_t               *   vga     = NULL;

    _printf("LDM vga driver ctrl\n");
    if( NULL == device )
        return RESULT_FAILED;

    vga = DEV_DATA(device);

    switch(cmd)
    {
    case VGA_CMD_SET_ATTR:
        arg = arg;
        vga = vga;
        return RESULT_FAILED;
    }
    return RESULT_FAILED;
}

result_t    Vga_entry(device_t * device,int flag,void * param)
{
    vga_t               *   vga     = NULL;

    _printf("LDM vga driver entry\n");
    if( NULL == device )
    {
        param = param;
        return RESULT_FAILED;
    }

    switch(flag)
    {
    case DEV_ENTRY_FLAG_REG:
        _printf("LDM vga driver entry registe\n");
        device->dev_data_ext = NULL;
        vga = DEV_DATA(device);

        vga->vga_buffer     = (word_t FAR *)0xB8000000;
        vga->vga_scale_x    = 80;
        vga->vga_scale_y    = 25;
        vga->vga_attr       = 0x07;

        device->dev_ddo.ddo_open    = Vga_open;
        device->dev_ddo.ddo_close   = Vga_close;
        device->dev_ddo.ddo_read    = Vga_read;
        device->dev_ddo.ddo_write   = Vga_write;
        device->dev_ddo.ddo_ctrl    = Vga_ctrl;

        return RESULT_SUCCEED;
    case DEV_ENTRY_FLAG_UNREG:
        _printf("LDM vga driver entry unregiste\n");
        vga = DEV_DATA(device);
        return RESULT_SUCCEED;
    }

    return RESULT_FAILED;
}



