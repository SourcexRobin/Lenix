

#include <net\net.h>
#include <net\ip.h>

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

#define ALIGN(num,align)            ((num) + (align) - 1)/(align)

//  加入以太网头部后复制到驱动缓存中
result_t    etherd_send(ndev_t * ndev,nbuf_t * nbuf)
{
    return -1;
}

//  这个例子会构造出3个缓冲区
byte_t      buf[4192];

socket_t                    socket = {0};
ndev_t                      net_dev = {1500};
int                         ip_packet_size = 3*1024;

nbuf_t *    Tcp_nbuf_create(SOCKET s,const void * buf,int len)
{
    int size,left = len;
    ndev_t * ndev = &net_dev;
    nbuf_t * nbuf,* nbuf_tail= NULL;
    void * tbuf = NULL;
    byte_t * sbuf = NULL;
    int   offset = 0;
    //  每个ip包最大可以有64K，但是没必要允许一次创建64K的包
    //  假设现在限制为

    if( len <= 0 )
        return NULL;
    //  计算总长度，需要算入系统增加的tcp和ip头，其中ip头可能不止一个，因为需要分片
    left  = sizeof(ip_t)* (( len + ndev->nd_frame_max - 1) / ndev->nd_frame_max);
    left += sizeof(tcp_t) + len;

    do
    {
        //  创建第一个nbuf，需要把tcp头和ip头加进去
    }while( left );

    while( left > 0  )
    {
        //  确定发送长度
        size = ip_packet_size;
        if( size > len )
            size = len;

        while( size )
        {
            int ndsize = ndev->nd_frame_max;
            if( ndsize > size )
                ndsize = size;

            nbuf = Net_nbuf_alloc();
            if( ndsize > NBUF_SIZE )
                ;//nbuf->nb_hdr.nbh_data_len = Nbuf_ext_alloc();

            if( NULL == nbuf_tail )
                nbuf_tail = nbuf;
            else
            {
                nbuf_tail->nb_hdr.nbh_next_frag = (nbuf_hdr_t *)nbuf;
                nbuf_tail = nbuf;
            }

            memcpy(tbuf, sbuf + offset,ndsize);

            offset += ndsize;
            size -= ndsize;
        }

        left -= size;
    }

    return NULL;
}

nbuf_t *    Net_nbuf_alloc(void)           /*  分配帧缓存对象      */
{
    return malloc(sizeof(nbuf_t));
}
void        Net_nbuf_free(nbuf_t * nbuf)   /*  释放帧缓存对象      */
{
}
void *      Nbuf_ext_alloc(void)           /*  分配扩展缓存        */
{
    return malloc(2048);
}
void        Nbuf_ext_free(void * buf)      /*  释放扩展缓存        */
{

}

int sendto( SOCKET s,  const char* buf,  int len,  int flags,
            const struct sockaddr* to,  int tolen)
{
    return 0;
}
int         main()
{
    return 0;
}