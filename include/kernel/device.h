/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    嵌入式操作系统
//                             2011 , 罗  斌，源代码工作室
//                                     保留所有版权
//                          ***---------------------------***
//
//  文件名称    : device.h
//  文件创建者  : 罗  斌
//  文件创建时间:
//  最后修改者  : 罗  斌
//  最后修改时间:
//
//  主要功能    : 提供统一的设备访问接口。
//
//  说明        : Lenix 是面向嵌入式系统，嵌入式系统设备都可以提前知道，而且嵌入式系统不会
//                出现设备变化的情况，因此不用考虑动态增减设备。
//                
//
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//========================================================================================
//              |   2012-06-27  |  罗斌         |  增加注释
//  00.00.000   |   2011-02-09  |  罗斌         |  xxxxxx
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
/*
    
*/
#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <config.h>
#include <type.h>

#ifdef _SLASH_
    #include <kernel/ipc.h>
#else
    #include <kernel\ipc.h>
#endif

#define DEV_IO_ADDR_SIZE            2
#define DEV_DATA_SIZE               32

struct _device_t;
/*
 *  设备驱动对象，这是一个简化的驱动模型
 *  Device Driver Object，简写为DDO.
 */
typedef struct _ddo_t
{
    result_t              (*ddo_open )  (struct _device_t * );
    result_t              (*ddo_close)  (struct _device_t * );
    int                   (*ddo_read )  (struct _device_t * ,offset_t,void *,
                                         int);
    int                   (*ddo_write)  (struct _device_t * ,offset_t,
                                         const void *,int);
    result_t              (*ddo_ctrl )  (struct _device_t * ,byte_t,void * );
}ddo_t;

typedef ddo_t               driver_t;
/*
 *  IO请求包.
 */
typedef struct _iorp_t
{
    struct _iorp_t      *   iorp_prev,
                        *   iorp_next;
    offset_t                iorp_pos;           /*  IO地址          */
    void                *   iorp_buffer;        /*  IO缓冲区        */
    size_t                  iorp_size;          /*  IO缓冲区长度    */
    int                     iorp_cmd;           /*  操作命令        */
}iorp_t;

typedef struct _device_t
{
    struct _device_t    *   dev_prev,
                        *   dev_next;
    struct _device_t    *   dev_sub_list;       /*  子设备列表      */
    char                    dev_name[12];
    result_t              (*dev_entry)(struct _device_t * ,int,void *);
    uint8_t                 dev_ivtid;          /*  设备中断号      */
    uint8_t                 dev_avl;            /*  软件可用位      */
    uint8_t                 dev_flag;           /*  设备标志        */
    int8_t                  dev_ref_cnt;        /*  设备打开计数    */
    int                     dev_mode;
    lock_t                  dev_lock;           /*  设备需要互斥访问，
                                                 *  访问锁
                                                 */
    /*iorp_t              *   dev_iorp;           /*  io请求包列表*/
    ddo_t                   dev_ddo;            /*  设备驱动对象    */
    offset_t                dev_capcity;        /*  设备容量        */
    void                *   dev_io_addr[2];     /*  设备io地址      */
    byte_t                  dev_data[32];       /*  设备数据空间。对象只提
                                                 *  供32个字节
                                                 */
    void                *   dev_data_ext;       /*  扩展数据空间。如果对象本
                                                 *  身的数据空间不够，可以另
                                                 *  外提供数据空间
                                                 */
}device_t;

/*
 *    IO模式不同，可能会导致中断处理程序不同。因此在改变设备IO模式时，要同时
 *  注意设备中断处理程序
 */
#define DEV_FLAG_IO_MASK            0x03
#define DEV_IO_UNDEF                0
#define DEV_IO_PIO                  1
#define DEV_IO_INT                  2
#define DEV_IO_DMA                  3
#define DEV_FLAG_BLOCK              0x04

#define DEV_ENTRY_FLAG_REG          0
#define DEV_ENTRY_FLAG_UNREG        1

#define DEV_IO_READ                 1
#define DEV_IO_WRITE                2
#define DEV_IO_RDWR                 (DEV_IO_READ|DEV_IO_WRITE)

#define DEV_MODE_ACTIVED            1
#define DEV_MODE_WRITE              2
#define DEV_MODE_READ               4

#define DEV_SEEK_SET                0
#define DEV_SEEK_CUR                1
#define DEV_SEEK_END                2
/*
#define SEEK_SET                    DEV_SEEK_SET
#define SEEK_CUR                    DEV_SEEK_CUR
#define SEEK_END                    DEV_SEEK_END
*/
#define DEV_LOCK(dev)               Lck_lock(&((dev)->dev_lock))
#define DEV_FREE(dev)               Lck_free(&((dev)->dev_lock))
#define DEV_IS_LOCKED(dev)          ((dev)->dev_lock.lck_status)

#define DEV_ACTIVE(dev)             do{ (dev)->dev_mode |= DEV_MODE_ACTIVED;\
                                      }while(0)
#define DEV_IS_ACTIVED(dev)         ((dev)->dev_mode & DEV_MODE_ACTIVED)

#define DEV_NAME(dev)               ((dev)->dev_name)
#define DEV_DATA(dev)               ((void *)(((dev)->dev_data_ext)?\
                                     ((dev)->dev_data_ext):(dev)->dev_data))

void        Dev_initial(void);
void        Dev_sdl_lock(void);
void        Dev_sdl_free(void);
void *      Dev_set_date(device_t * device,void * data,uint_t size);

result_t    Dev_registe(const char  * name,
                        result_t   (* entry)(struct _device_t *,int,void *),
                        void        * param);
result_t    Dev_unregiste(const char * name,void * param);

void *      Dev_isp(device_t * device,uint8_t ivtid,isp_t isp);

device_t *  Dev_open    (const char * name,int mode);
result_t    Dev_close   (device_t * device);
int         Dev_read    (device_t * device,offset_t pos,void * buffer,
                         int size);
int         Dev_write   (device_t * device,offset_t pos,const void * buffer,
                         int size);
result_t    Dev_ctrl    (device_t * device,byte_t cmd,void * arg);

device_t *  Dev_sdl_first(void);
device_t *  Dev_sdl_next(device_t * device);

#endif  /*  _DEVICE_H_  */