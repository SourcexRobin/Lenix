

// 2013-12-11
#ifndef _LENIX_IP_H_
#define _LENIX_IP_H_


#include <net\net.h>

#define _LITTLE_ENDIAN_

#define SOCKET_MAX                  256

/*  网络插口*/
typedef struct _socket_t
{
    int                     sock_dev_id;        /*  对应的网络设备  */

    int                     sock_protocol;      /*  协议类型    */
    word_t                  sock_src_port;      /*  源端口      */
    word_t                  sock_dst_port;      /*  目标端口    */
    dword_t                 sock_src_ip;        /*  源ip地址    */
    dword_t                 sock_dst_ip;        /*  目标ip地址  */
}socket_t;

typedef int                 SOCKET;

typedef struct in_addr
{
    dword_t                 ia_ip;
}in_addr_t;

typedef struct _ip_t
{
#ifdef _LITTLE_ENDIAN_  
    byte_t                  ip_ihl:4;       /*  首部长度.ip header leng     */
    byte_t                  ip_version:4;   /*  版本                        */
#else  
    byte_t                  ip_version:4; 
    byte_t                  ip_ihl:4;    
#endif  
    byte_t                  ip_tos;         /*  服务类型 type of service    */
    word_t                  ip_tot_len;     /*  总长度                      */
    word_t                  ip_id;          /*  标志，重组使用              */
    word_t                  ip_frag_off;    /*  分片偏移,重组使用           */
    byte_t                  ip_ttl;         /*  生存时间 time total live    */
    byte_t                  ip_protocol;    /*  协议                        */
    word_t                  ip_chk_sum;     /*  检验和                      */
    struct in_addr          ip_src_addr;    /*  源IP地址                    */
    struct in_addr          ip_dst_addr;    /*  目的IP地址                  */
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

// 定义以太网首部

  

typedef struct ehhdr  

{  

unsigned char eh_dst[6];   /* destination ethernet addrress */ 

unsigned char eh_src[6];   /* source ethernet addresss */ 

unsigned short eh_type;   /* ethernet pachet type */ 

}EHHDR, *PEHHDR;  

typedef struct _arp_t
{
    word_t                  arp_hrd_type;   /*  硬件类型 hardware type                  */
    word_t                  arp_protocol;   /*  协议                                    */
    byte_t                  arp_hal;        /*  硬件地址长度 hadrware address length    */
    byte_t                  arp_pal;        /*  协议地址长度 protocol address length    */
    word_t                  arp_op;         /*  操作 operation                          */
}arp_t;

typedef struct _tcp_t
{
    word_t                  tcp_src_port;    //源端口号  
    word_t                  tcp_dst_port;    //目的端口号  
    dword_t                 tcp_seq_no;        //序列号  
    dword_t                 tcp_ack_no;        //确认号  
#ifdef _LITTLE_ENDIAN_  
    byte_t                  tcp_reserved_1:4; //保留6位中的4位首部长度  
    byte_t                  tcp_thl:4;        //tcp头部长度  
    byte_t                  tcp_flag:6;       //6位标志  
    byte_t                  tcp_reseverd_2:2; //保留6位中的2位  
#else  
    byte_t                  tcp_thl:4;        //tcp头部长度  
    byte_t                  tcp_reserved_1:4; //保留6位中的4位首部长度  
    byte_t                  tcp_reseverd_2:2; //保留6位中的2位  
    byte_t                  tcp_flag:6;       //6位标志   
#endif  
    word_t                  tcp_wnd_size;    //16位窗口大小  
    word_t                  tcp_chk_sum;     //16位TCP检验和  
    word_t                  tcp_urgt_ptr;      //16为紧急指针
}tcp_t;

#define TCP_DATA(tcp)               ((void *)((byte_t *)(tcp) + sizeof(tcp_t)))

typedef struct _udp_t
{
    word_t                  udp_src_port;   /*  源端口号                    */
    word_t                  udp_dst_port;   /*  目的端口号                  */
    word_t                  udp_uhl;        /*  头部长度 udp header length  */
    word_t                  udp_chk_sum;    /*  16位udp检验和               */
}udp_t;

#define UDP_DATA(udp)               ((void *)((byte_t *)(udp) + sizeof(udp_t)))

/*  由arp协议创建  */
typedef struct _route_t
{
    dword_t ip;
    npa_t      des_npa[16];
}route_t;

/*  从ip地址转换到物理地址，不一定是以太网的mac地址 */
byte_t *    Map_ip_to_mac(dword_t ip);

socket_t                    socket_pool[SOCKET_MAX];

/*
 *  1、不应该限制程序每次发送的数据，但是网络层不能满足
 *  2、所以要限制每次生成的nbuf长度
 * /
static 
nbuf_t *    Tcp_nbuf_create(ndev,tcp,const void * buffer,int len)
{
    int left,
        ns; n send;
    offset;
    max_size;
    
    /*  先创建含tcp头的nbuf* /
    left = len;
    ns = ndev->nd_frame_max;
    if( ns > len - 20 )
    {
        nbuf = ;
        if( ns > 128 - (ip + tcp头的长度 ) )
            分配扩展缓存;
        ip = NBUF_GET_BUF(nbuf);
        tcp = IP_TCP_HDR(ip);

        //  填写ip、tcp头
        //  复制数据
    }
    
    left -= ns;
    //  创建剩余的分组
    while( left )
    {
        ns = ndev->nd_frame_max;
        if( ns > left )
            ns = left;

        /*  创建* /
        nbuf = ;


        left -= send_len;
    }
}

//  tcp需要一个检查进程，用来检查

static
int         Tcp_nbuf_copy(SOCKET s,nbuf_t * nbuf,void * buffer,int len);

Udp_nbuf_create;
Udp_nbuf_copy;

Arp_nbuf_create;
Arp_nbuf_copy;

/* socket标准api    * /
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

        /*  插入设备发送队列  * /
        Ndev_insert(ndev,nbuf);
        
        left -= nsend;
    }
}
int         resv();

int         sendto();
int         sendfrom(); /* */

#endif  /*  _LENIX_IP_H_ */