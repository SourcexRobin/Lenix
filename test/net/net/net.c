

#include <net\net.h>
#include <net\ip.h>

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

#define ALIGN(num,align)            ((num) + (align) - 1)/(align)

//  ������̫��ͷ�����Ƶ�����������
result_t    etherd_send(ndev_t * ndev,nbuf_t * nbuf)
{
    return -1;
}

//  ������ӻṹ���3��������
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
    //  ÿ��ip����������64K������û��Ҫ����һ�δ���64K�İ�
    //  ������������Ϊ

    if( len <= 0 )
        return NULL;
    //  �����ܳ��ȣ���Ҫ����ϵͳ���ӵ�tcp��ipͷ������ipͷ���ܲ�ֹһ������Ϊ��Ҫ��Ƭ
    left  = sizeof(ip_t)* (( len + ndev->nd_frame_max - 1) / ndev->nd_frame_max);
    left += sizeof(tcp_t) + len;

    do
    {
        //  ������һ��nbuf����Ҫ��tcpͷ��ipͷ�ӽ�ȥ
    }while( left );

    while( left > 0  )
    {
        //  ȷ�����ͳ���
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

nbuf_t *    Net_nbuf_alloc(void)           /*  ����֡�������      */
{
    return malloc(sizeof(nbuf_t));
}
void        Net_nbuf_free(nbuf_t * nbuf)   /*  �ͷ�֡�������      */
{
}
void *      Nbuf_ext_alloc(void)           /*  ������չ����        */
{
    return malloc(2048);
}
void        Nbuf_ext_free(void * buf)      /*  �ͷ���չ����        */
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