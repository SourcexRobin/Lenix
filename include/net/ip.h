

// 2013-12-11
#ifndef _LENIX_IP_H_
#define _LENIX_IP_H_


#include <net\net.h>

#define _LITTLE_ENDIAN_

#define SOCKET_MAX                  256

/*  ������*/
typedef struct _socket_t
{
    int                     sock_dev_id;        /*  ��Ӧ�������豸  */

    int                     sock_protocol;      /*  Э������    */
    word_t                  sock_src_port;      /*  Դ�˿�      */
    word_t                  sock_dst_port;      /*  Ŀ��˿�    */
    dword_t                 sock_src_ip;        /*  Դip��ַ    */
    dword_t                 sock_dst_ip;        /*  Ŀ��ip��ַ  */
}socket_t;

typedef int                 SOCKET;

typedef struct in_addr
{
    dword_t                 ia_ip;
}in_addr_t;

typedef struct _ip_t
{
#ifdef _LITTLE_ENDIAN_  
    byte_t                  ip_ihl:4;       /*  �ײ�����.ip header leng     */
    byte_t                  ip_version:4;   /*  �汾                        */
#else  
    byte_t                  ip_version:4; 
    byte_t                  ip_ihl:4;    
#endif  
    byte_t                  ip_tos;         /*  �������� type of service    */
    word_t                  ip_tot_len;     /*  �ܳ���                      */
    word_t                  ip_id;          /*  ��־������ʹ��              */
    word_t                  ip_frag_off;    /*  ��Ƭƫ��,����ʹ��           */
    byte_t                  ip_ttl;         /*  ����ʱ�� time total live    */
    byte_t                  ip_protocol;    /*  Э��                        */
    word_t                  ip_chk_sum;     /*  �����                      */
    struct in_addr          ip_src_addr;    /*  ԴIP��ַ                    */
    struct in_addr          ip_dst_addr;    /*  Ŀ��IP��ַ                  */
}ip_t;

#define IP_PTR(ptr)             ((ip_t *)(ptr))
#define IP_TCP_HDR(ip)          ((tcp_t *)(((byte_t *)(ip)) + (ip)->ip_ihl))
#define IP_UDP_HDR(ip)          ((udp_t *)IP_TCP_HDR(ip))
#define IP_TCP_DATA_LEN
#define IP_UDP_DATA_LEN

#define EPT_IP   0x0800    /* type: IP */ 

#define EPT_ARP   0x0806    /* type: ARP */ 

#define EPT_RARP 0x8035    /* type: RARP */  

#define ARP_HARDWARE 0x0001    /* Dummy type for 802.3 frames */ 

#define ARP_REQUEST 0x0001    /* ARP request */ 

#define ARP_REPLY 0x0002    /* ARP reply */  

// ������̫���ײ�

  

typedef struct ehhdr  

{  

unsigned char eh_dst[6];   /* destination ethernet addrress */ 

unsigned char eh_src[6];   /* source ethernet addresss */ 

unsigned short eh_type;   /* ethernet pachet type */ 

}EHHDR, *PEHHDR;  

typedef struct _arp_t
{
    word_t                  arp_hrd_type;   /*  Ӳ������ hardware type                  */
    word_t                  arp_protocol;   /*  Э��                                    */
    byte_t                  arp_hal;        /*  Ӳ����ַ���� hadrware address length    */
    byte_t                  arp_pal;        /*  Э���ַ���� protocol address length    */
    word_t                  arp_op;         /*  ���� operation                          */
}arp_t;

typedef struct _tcp_t
{
    word_t                  tcp_src_port;    //Դ�˿ں�  
    word_t                  tcp_dst_port;    //Ŀ�Ķ˿ں�  
    dword_t                 tcp_seq_no;        //���к�  
    dword_t                 tcp_ack_no;        //ȷ�Ϻ�  
#ifdef _LITTLE_ENDIAN_  
    byte_t                  tcp_reserved_1:4; //����6λ�е�4λ�ײ�����  
    byte_t                  tcp_thl:4;        //tcpͷ������  
    byte_t                  tcp_flag:6;       //6λ��־  
    byte_t                  tcp_reseverd_2:2; //����6λ�е�2λ  
#else  
    byte_t                  tcp_thl:4;        //tcpͷ������  
    byte_t                  tcp_reserved_1:4; //����6λ�е�4λ�ײ�����  
    byte_t                  tcp_reseverd_2:2; //����6λ�е�2λ  
    byte_t                  tcp_flag:6;       //6λ��־   
#endif  
    word_t                  tcp_wnd_size;    //16λ���ڴ�С  
    word_t                  tcp_chk_sum;     //16λTCP�����  
    word_t                  tcp_urgt_ptr;      //16Ϊ����ָ��
}tcp_t;

#define TCP_DATA(tcp)               ((void *)((byte_t *)(tcp) + sizeof(tcp_t)))

typedef struct _udp_t
{
    word_t                  udp_src_port;   /*  Դ�˿ں�                    */
    word_t                  udp_dst_port;   /*  Ŀ�Ķ˿ں�                  */
    word_t                  udp_uhl;        /*  ͷ������ udp header length  */
    word_t                  udp_chk_sum;    /*  16λudp�����               */
}udp_t;

#define UDP_DATA(udp)               ((void *)((byte_t *)(udp) + sizeof(udp_t)))

/*  ��arpЭ�鴴��  */
typedef struct _route_t
{
    dword_t ip;
    npa_t      des_npa[16];
}route_t;

/*  ��ip��ַת���������ַ����һ������̫����mac��ַ */
byte_t *    Map_ip_to_mac(dword_t ip);

socket_t                    socket_pool[SOCKET_MAX];

/*
 *  1����Ӧ�����Ƴ���ÿ�η��͵����ݣ���������㲻������
 *  2������Ҫ����ÿ�����ɵ�nbuf����
 * /
static 
nbuf_t *    Tcp_nbuf_create(ndev,tcp,const void * buffer,int len)
{
    int left,
        ns; n send;
    offset;
    max_size;
    
    /*  �ȴ�����tcpͷ��nbuf* /
    left = len;
    ns = ndev->nd_frame_max;
    if( ns > len - 20 )
    {
        nbuf = ;
        if( ns > 128 - (ip + tcpͷ�ĳ��� ) )
            ������չ����;
        ip = NBUF_GET_BUF(nbuf);
        tcp = IP_TCP_HDR(ip);

        //  ��дip��tcpͷ
        //  ��������
    }
    
    left -= ns;
    //  ����ʣ��ķ���
    while( left )
    {
        ns = ndev->nd_frame_max;
        if( ns > left )
            ns = left;

        /*  ����* /
        nbuf = ;


        left -= send_len;
    }
}

//  tcp��Ҫһ�������̣��������

static
int         Tcp_nbuf_copy(SOCKET s,nbuf_t * nbuf,void * buffer,int len);

Udp_nbuf_create;
Udp_nbuf_copy;

Arp_nbuf_create;
Arp_nbuf_copy;

/* socket��׼api    * /
SOCKET      socket();
int         connect();
int         bind();
int         accept();

int         send()
{
    nsend;
    while( left )
    {
        nsend = nbuf_size;

        if( nsend > left )
            nsend = left;

        nbuf = Tcp_nbuf_create(s,buf,nsend)

        /*  �����豸���Ͷ���  * /
        Ndev_insert(ndev,nbuf);
        
        left -= nsend;
    }
}
int         resv();

int         sendto();
int         sendfrom(); /* */

#endif  /*  _LENIX_IP_H_ */