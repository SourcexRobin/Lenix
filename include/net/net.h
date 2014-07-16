




//  2013-12-11
#ifndef _LENIX_NET_H_
#define _LENIX_NET_H_

//#include <type.h>
//#include <ipc.h>

#include <result.h>

typedef unsigned char       byte_t;
typedef unsigned short      word_t;
typedef unsigned long       dword_t;

typedef byte_t              uint8_t;
typedef word_t              uint16_t;
typedef dword_t             uint32_t;

typedef int                 result_t;

#define NULL                0
#define NBUF_SIZE                   224
#define NBUF_EXT_LEN                (2048)


/*
 *  网络缓存，实际上就是帧缓存，理论上需要适应多种物理网络设备，因为用在
 *  adsl和以太网中的物理帧是不一样的，
 *  在网络上传输，每次只能传输一定长度的数据。而应用程序则有可能一次发送多余这个长度的
 *  数据，例如以太网每次最多传输1500字节的数据，而应用程序有可能发送2K的数据，因此需要
 *  拆分
 *  nbuf = net buffer;
 *  发送数据：需要知道一个数据包有多少数据
 *  接受数据：需要知道还剩多少数据没有传递给客户端
 */
typedef struct _nbuf_hdr_t
{
    struct _nbuf_hdr_t  *   nbh_next_frag;      /*  下一个分片，用于组成数据包      */
    struct _nbuf_hdr_t  *   nbh_next_packet;    /*  下一个数据包，用于组成发送链表  */
    word_t                  nbh_data_len;       /*  数据长度    */
    word_t                  nbh_flags;          /*  标志位      */
}nbuf_hdr_t;


typedef struct _nbuf_t
{
    nbuf_hdr_t              nb_hdr;
    union{
        struct
        {
            int             nb_eb;
        }nb_ext_buf;
        byte_t              nb_buf[NBUF_SIZE];
    }data;
}nbuf_t;

#define NBUF_GET_BUF(nbuf)          (void *)((nbuf)->nb_ext_buf ? \
                                     (nbuf)->nb_ext_buf : (nbuf)->nb_buf)
/*  每次*/
#define NBUF_MAX_LEN                8192
int                         nbuf_size;/*  每个缓存最大数据长度 */

nbuf_t *    Net_nbuf_alloc(void);           /*  分配帧缓存对象      */
void        Net_nbuf_free(nbuf_t * nbuf);   /*  释放帧缓存对象      */
void *      Nbuf_ext_alloc(void);           /*  分配扩展缓存        */
void        Nbuf_ext_free(void * buf);      /*  释放扩展缓存        */

int         Nbuf_copy;
/*  网络设备物理地址，不一定是以太网的max地址  */
typedef struct _net_phical_address_t
{
    byte_t                  npa_pa[16];
}npa_t;

typedef struct _net_device_t
{
    int                     nd_frame_max;   /*  网络设备的最大数据帧  */
    npa_t                   nd_npa;
    //mutex_t                 nd_lock_send;
    nbuf_t              *   nd_nbuf_send;   /*  需要发送的数据链表  */
    //mutex_t                 nd_lock_resv;
    nbuf_t              *   nd_nbuf_resv;

    result_t           (*   nd_send)(struct _net_device_t * ndev,nbuf_t * nbuf);
    result_t           (*   nd_resv)(struct _net_device_t * ndev,nbuf_t * nbuf);
}ndev_t;


result_t    Ndev_insert(ndev_t * ndev,nbuf_t * nbuf);
result_t    Ndev_send();
result_t    Ndev_resv();

#endif  /*  _LENIX_NET_H_  */