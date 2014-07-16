




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
 *  ���绺�棬ʵ���Ͼ���֡���棬��������Ҫ��Ӧ�������������豸����Ϊ����
 *  adsl����̫���е�����֡�ǲ�һ���ģ�
 *  �������ϴ��䣬ÿ��ֻ�ܴ���һ�����ȵ����ݡ���Ӧ�ó������п���һ�η��Ͷ���������ȵ�
 *  ���ݣ�������̫��ÿ����ഫ��1500�ֽڵ����ݣ���Ӧ�ó����п��ܷ���2K�����ݣ������Ҫ
 *  ���
 *  nbuf = net buffer;
 *  �������ݣ���Ҫ֪��һ�����ݰ��ж�������
 *  �������ݣ���Ҫ֪����ʣ��������û�д��ݸ��ͻ���
 */
typedef struct _nbuf_hdr_t
{
    struct _nbuf_hdr_t  *   nbh_next_frag;      /*  ��һ����Ƭ������������ݰ�      */
    struct _nbuf_hdr_t  *   nbh_next_packet;    /*  ��һ�����ݰ���������ɷ�������  */
    word_t                  nbh_data_len;       /*  ���ݳ���    */
    word_t                  nbh_flags;          /*  ��־λ      */
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
/*  ÿ��*/
#define NBUF_MAX_LEN                8192
int                         nbuf_size;/*  ÿ������������ݳ��� */

nbuf_t *    Net_nbuf_alloc(void);           /*  ����֡�������      */
void        Net_nbuf_free(nbuf_t * nbuf);   /*  �ͷ�֡�������      */
void *      Nbuf_ext_alloc(void);           /*  ������չ����        */
void        Nbuf_ext_free(void * buf);      /*  �ͷ���չ����        */

int         Nbuf_copy;
/*  �����豸�����ַ����һ������̫����max��ַ  */
typedef struct _net_phical_address_t
{
    byte_t                  npa_pa[16];
}npa_t;

typedef struct _net_device_t
{
    int                     nd_frame_max;   /*  �����豸���������֡  */
    npa_t                   nd_npa;
    //mutex_t                 nd_lock_send;
    nbuf_t              *   nd_nbuf_send;   /*  ��Ҫ���͵���������  */
    //mutex_t                 nd_lock_resv;
    nbuf_t              *   nd_nbuf_resv;

    result_t           (*   nd_send)(struct _net_device_t * ndev,nbuf_t * nbuf);
    result_t           (*   nd_resv)(struct _net_device_t * ndev,nbuf_t * nbuf);
}ndev_t;


result_t    Ndev_insert(ndev_t * ndev,nbuf_t * nbuf);
result_t    Ndev_send();
result_t    Ndev_resv();

#endif  /*  _LENIX_NET_H_  */