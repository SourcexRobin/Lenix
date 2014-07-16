/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    Ƕ��ʽ����ϵͳ
//                             2011 , ��  ��Դ���빤����
//                                     �������а�Ȩ
//                          ***---------------------------***
//
//  �ļ�����    : device.h
//  �ļ�������  : ��  ��
//  �ļ�����ʱ��:
//  ����޸���  : ��  ��
//  ����޸�ʱ��:
//
//  ��Ҫ����    : �ṩͳһ���豸���ʽӿڡ�
//
//  ˵��        : Lenix ������Ƕ��ʽϵͳ��Ƕ��ʽϵͳ�豸��������ǰ֪��������Ƕ��ʽϵͳ����
//                �����豸�仯���������˲��ÿ��Ƕ�̬�����豸��
//                
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
//========================================================================================
//              |   2012-06-27  |  �ޱ�         |  ����ע��
//  00.00.000   |   2011-02-09  |  �ޱ�         |  xxxxxx
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
 *  �豸������������һ���򻯵�����ģ��
 *  Device Driver Object����дΪDDO.
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
 *  IO�����.
 */
typedef struct _iorp_t
{
    struct _iorp_t      *   iorp_prev,
                        *   iorp_next;
    offset_t                iorp_pos;           /*  IO��ַ          */
    void                *   iorp_buffer;        /*  IO������        */
    size_t                  iorp_size;          /*  IO����������    */
    int                     iorp_cmd;           /*  ��������        */
}iorp_t;

typedef struct _device_t
{
    struct _device_t    *   dev_prev,
                        *   dev_next;
    struct _device_t    *   dev_sub_list;       /*  ���豸�б�      */
    char                    dev_name[12];
    result_t              (*dev_entry)(struct _device_t * ,int,void *);
    uint8_t                 dev_ivtid;          /*  �豸�жϺ�      */
    uint8_t                 dev_avl;            /*  �������λ      */
    uint8_t                 dev_flag;           /*  �豸��־        */
    int8_t                  dev_ref_cnt;        /*  �豸�򿪼���    */
    int                     dev_mode;
    lock_t                  dev_lock;           /*  �豸��Ҫ������ʣ�
                                                 *  ������
                                                 */
    /*iorp_t              *   dev_iorp;           /*  io������б�*/
    ddo_t                   dev_ddo;            /*  �豸��������    */
    offset_t                dev_capcity;        /*  �豸����        */
    void                *   dev_io_addr[2];     /*  �豸io��ַ      */
    byte_t                  dev_data[32];       /*  �豸���ݿռ䡣����ֻ��
                                                 *  ��32���ֽ�
                                                 */
    void                *   dev_data_ext;       /*  ��չ���ݿռ䡣�������
                                                 *  ������ݿռ䲻����������
                                                 *  ���ṩ���ݿռ�
                                                 */
}device_t;

/*
 *    IOģʽ��ͬ�����ܻᵼ���жϴ������ͬ������ڸı��豸IOģʽʱ��Ҫͬʱ
 *  ע���豸�жϴ������
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