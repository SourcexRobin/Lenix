/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: fs_fat32.c
//  创建时间: 2014-03-11        创建者: 罗斌
//  修改时间: 2012-11-29        修改者: 罗斌
//  主要功能: FAT32文件系统驱动。
//  说    明: 
//  FAT32需要处理的目标
//      文件分配表 ：分配、回收、链接
//      目录项     ：查找、增长、缩减、提取、生成
//      普通文件   ：定位、读、写
//      目录文件   ：遍历、定位、增、删、改
//  命名规则: 模块名称_子类标识_动词_名词
//  变更记录:
//  版 本 号    |     时间      |  作  者       |  主要变化记录
//=============================================================================
//  00.00.000   |  2014-03-11   |  罗  斌       |  创建文件
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>
#include <const.h>
#include <result.h>
#include <assert.h>

#include <ltime.h>
#include <lio.h>
#include <blkbuf.h>

#include "fs.h"
#include "fs_fat.h"

/*
 *  UNICODE转换为ANSI
 *  uc: UNICODE代码，unicode
 *  a : ANSI变量地址，ansi addreess
 *  文件系统中保存的是UNICODE，但是操作系统却使用GBK2312编码，需要提供转换函数
 *  目前直接赋值
 */
#define FAT32_UNICODE_TO_ANSI(uc,a) do{ if( (uc) < 127 ) *(a)++ = (char)(uc);\
                                        else *((word_t *)(a))++ = (uc);\
                                      }while(0)
/*  调试输出UNICODE，分字节输出，其实是错误的，只是为了表示输出了 */
#define FAT32_PRINTK(uc)            do{ _printk("%c",(char)(uc));\
                                        if( (uc) > 127 ) \
                                          _printk("%c",(char)((uc) >> 8 ));\
                                      }while(0)

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  调试、辅助函数
*/

/*  2014-03-13，显示FAT对象信息 */
static
void        Fat_information(fat_t * fat)
{
    _printk("FAT32 informatino:\n");
    _printk("start sector =%ld; total sectors =%ld\n",
        fat->fat_start_sector,fat->fat_total_sectors);
    _printk("start data sector =%d; total dat sectors =%d\n",
        fat->fat_start_data_sector,fat->fat_total_data_sectors);
    _printk("total clusters =%d",fat->fat_total_clus);
}

/*  2014-03-13，显示BPB32信息 */
static
void        Fat_bpb32_information(bpb32_t * bpb)
{
    _printk("FAT32 BPB information:\n");
    _printk("boot signature =%02X; volume id =%08X; volume lable =%s\n",
        bpb->bpb32_bs_boot_sig,bpb->bpb32_bs_vol_id,bpb->bpb32_bs_vol_lab);
    _printk("bytes per sector =%d; sectors per cluster =%d\n",
        bpb->bpb32_bytes_per_sect,bpb->bpb32_sects_per_clus);
    _printk("total sectors =%d; fat number =%d; fat size =%d\n",
        bpb->bpb32_total_sect32,bpb->bpb32_num_of_fats,bpb->bpb32_fat_size32);
    _printk("rsvd sectors =%d; root cluster =%d\n",
        bpb->bpb32_rsvd_sectors,bpb->bpb32_root_clus);
}

/*  2014-03-15  显示目录的信息  */
static
void        Fat32_directory_information(fat_dir_t * dir)
{
    if( FATDIR_IS_LONG_NAME_DIR(dir) )
        return ;
    _printk("%-12s ",dir->fdir_name);
    if( dir->fdir_attr & FATDIR_ATTR_DIRECTORY )
        _printk("<DIR> ");
    else
        _printk("      ");
    _printk("%-9d %02d-%02d-%02d %02d:%02d %02X\n",
        dir->fdir_fsize,
        (dir->fdir_lwd.sd_year + 1980) % 100,
        dir->fdir_lwd.sd_month,dir->fdir_lwd.sd_day,
        dir->fdir_lwt.st_hour,dir->fdir_lwt.st_minute,
        dir->fdir_attr);
}

/*  2014-03-15  导出目录的内存信息  */
static
void        Fat32_dump_dir(fat_dir_t * dir,int n)
{
    int i = 0;
    for( ; i < n ; i++)
        _mprintf(dir + i,sizeof(fat_dir_t));
}

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  基本功能函数
*/

/*
//-----------------------------------------------------------------------------
//  本函数直接复制于微软官方文档,函数名更改为Fat_name_chksum
// ChkSum()
//   Returns an unsigned byte checksum computed on an unsigned byte
// array. The array must be 11 bytes long and is assumed to contain
// a name stored in the format of a MS-DOS directory entry.
//   Passed: pFcbName Pointer to an unsigned byte array assumed to be
// 11 bytes long.
//   Returns: Sum An 8-bit unsigned checksum of the array pointed
// to by pFcbName.
//-----------------------------------------------------------------------------
*/
static
unsigned char Fat_name_chksum (unsigned char *pFcbName)
{
    short           FcbNameLen;
    unsigned char   Sum;

    Sum = 0;
    for (FcbNameLen=11; FcbNameLen!=0; FcbNameLen--) {
        // NOTE: The operation is an unsigned char rotate right
        Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *pFcbName++;
    }
    
    return (Sum);
}

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  文件分配表操作函数：分配、回收、遍历
*/

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Fat32_cluster_alloc
//  作    用: 分配可用簇。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT所在设备
//     fat_t *      |   fat         |   分区对象指针
//     uint32_t *   |   clusters    |   簇缓存
//     uint_t       |   count       |   要求分配数量
//  返 回 值: 返回分配簇的个数。
//  注    意: 最大一次分配128簇。
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-14  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
int     Fat32_cluster_alloc(device_t * hd      ,fat_t    * fat,
                            uint32_t * clusters,uint_t     count)
{
    uint64_t        fsa         = 0;    /*  fat sector address  */
    blkbuf_t      * sbb         = NULL; /*  sector blkbuf       */
    uint32_t      * clusbuf     = NULL;
    uint_t          cnt         = 0,    /*  已分配计数器        */
                    i           = 0,
                    j           = 0;
    BBUF_DECLARE();

    count   = count > 128 ? 128 : count;
    fsa     = FAT32_FAT_FIRST(fat);
    /*  扫描FAT，遇到0，就记录，并且将其置为占用 */
    for( i = 0 ;  i < fat->fat_fat_size && cnt < count ; i++ ,fsa++)
    {
        BBUF_READ_SECTOR(clusbuf,hd,fsa+i);
        if( NULL == clusbuf )
            goto fat32_cluster_alloc_end;
        BBUF_LOCK();
        /*  遍历扇区，每个扇区保存有128个簇地址 */
        for( j = 0 ; j < 128 && cnt < count; j++ )
        {
            if( 0 == clusbuf[j] )
            {
                clusters[cnt++] = (uint32_t)fsa;
                clusters[j]     = FAT32_END_OF_CLUS;
                BBUF_DIRTED(sbb);
                fat->fat_free_clus--;
            }
        }
        BBUF_FREE();
        BBUF_RELEASE();
    }

fat32_cluster_alloc_end:
    return cnt;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Fat32_cluster_free
//  作    用: 释放簇。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT所在设备
//     fat_t *      |   fat         |   分区对象指针
//     uint32_t *   |   clusters    |   簇缓存
//     uint_t       |   count       |   要求分配数量
//  返 回 值: 返回分配簇的个数。
//  注    意: 最大一次分配128簇。
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-14  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
int         Fat32_cluster_free(device_t * hd      ,fat_t    * fat,
                               uint32_t * clusters,uint_t     count)
{
    uint64_t        fsa         = 0;    /*  FAT扇区地址，fat sector address */
    blkbuf_t      * sbb         = NULL; /*  扇区块缓存，sector blkbuf       */
    byte_t        * clusbuf     = NULL;
    uint_t          offset      = 0;
    uint_t          cnt         = 0,
                    i;

    for( i = 0 ; i < count ; i++ )
    {
        fsa = FAT32_THIS_SECT_NUM(fat,clusters[i]);
        if( NULL == (sbb = Bbuf_read(hd,fsa) ) )
            goto fat32_cluster_free_end;

        clusbuf = BBUF_TO_SECTOR(sbb,fsa);
        offset  = FAT32_THIS_SECT_OFFSET(clusters[i]);
        
        Lck_lock(&sbb->bb_lock);
        *(uint32_t *)(clusbuf + offset) = 0;
        BBUF_DIRTED(sbb);
        Lck_free(&sbb->bb_lock);
        
        Bbuf_release(sbb);
        
        fat->fat_free_clus++;
        cnt++;
    }

fat32_cluster_free_end:
    return cnt;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Fat32_cluster_link
//  作    用: 链接簇。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT所在设备
//     fat_t *      |   fat         |   分区对象指针
//     uint32_t     |   startclus   |   起始簇号
//     uint32_t *   |   clusters    |   簇缓存
//     uint_t       |   count       |   要求分配数量
//  返 回 值: 返回分配簇的个数。
//  注    意: 最大一次分配128簇。
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-05-31  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
result_t    Fat32_cluster_link(device_t * hd,fat_t * fat,uint32_t startclus,
                               uint32_t * clusters,uint_t count)
{
    return -1;
}
/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  目录项操作函数：增长、缩减、提取、生成
*/

typedef word_t *            (* to_unicode_t)(void * src,word_t * des);

static
word_t *    Gbk2312_to_unicode(void * src,word_t * des)
{
    return des;
}
static to_unicode_t         to_unicode = Gbk2312_to_unicode;
static
word_t      Ascii_to_unicode(char c)
{
    return 0;
}

static
byte_t      Unicode_to_ascii(word_t unicode)
{
    return 0;
}

#define Fat32_get_directory_name    Fat32_directory_to_name
/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Fat32_directory_to_name
//  作    用: 获取目录名称。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     fat_dir_t *  |   dir         |   目录缓存，需提供满足最大文件长度的空
//                  |               | 间，按长名目录可以存放26个字符，需要提供
//                  |               | 11个。
//     char *       |   name        |   文件名缓冲区，应保证256个字节或以上
//  返 回 值: 返回参数name的地址。
//  注    意: 长、短文件名一同处理。
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-15  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
char *      Fat32_directory_to_name(fat_dir_t * dir,char * name)
{
    int             i       = 0,    /*  index                       */
                    lndi    = 0;    /*  long name directory index   */
    fdir_lnentry_t* lndir   = NULL; /*  long name directory         */
    char          * retname = name; /*  return name                 */

    /*  判断长、短文件名    */
    if( FATDIR_IS_LONG_NAME_DIR(dir) )
    {
        /*  计算占用的目录数    */
        i = 0;
        while( FATDIR_IS_LONG_NAME_DIR(dir + i) )
            i++;
        goto fat32_get_long_name;
    }
    /*  处理短文件名    */
    for( i = 0 ; i < 11 ; i++)
    {
        if( i < 8  && dir->fdir_name[i] == 0x20 )
            continue;
        if( i < 11 && dir->fdir_name[i] == 0x20 )
            goto fat32_get_name_end;
        if( i == 8 && dir->fdir_name[8] != 0x20 )
            *name++ = '.';
        *name++ = dir->fdir_name[i];
    }
    goto fat32_get_name_end;

    /*  处理长文件名，由于是反向安排目录位置，需要反向处理  */
fat32_get_long_name:
    if( i < 0 )
        goto fat32_get_name_end;
    /*  跳过长名目录附带的短文件名目录  */
    if( !FATDIR_IS_LONG_NAME_DIR(dir+i) )
    {
        i--;
        goto fat32_get_long_name;
    }
    lndir = (fdir_lnentry_t *)(dir + i--);
    /*  长名目录将文件名分成了3个部分，所以要分开处理 */
    /*  处理第1部分 */
    for( lndi = 0 ; lndi < 5 ; lndi++ )
    {
        if( 0 == lndir->fdl_name1[lndi] )
            goto fat32_get_name_end;
        //FAT32_PRINTK(lndir->fdl_name1[lndi]);
        FAT32_UNICODE_TO_ANSI(lndir->fdl_name1[lndi],name);
    }
    /*  处理第2部分 */
    for( lndi = 0 ; lndi < 6 ; lndi++ )
    {
        if( 0 == lndir->fdl_name2[lndi] )
            goto fat32_get_name_end;
        //FAT32_PRINTK(lndir->fdl_name2[lndi]);
        FAT32_UNICODE_TO_ANSI(lndir->fdl_name2[lndi],name);
    }
    /*  处理第3部分 */
    for( lndi = 0 ; lndi < 2 ; lndi++ )
    {
        if( 0 == lndir->fdl_name3[lndi] )
            goto fat32_get_name_end;
        //FAT32_PRINTK(lndir->fdl_name3[lndi]);
        FAT32_UNICODE_TO_ANSI(lndir->fdl_name3[lndi],name);
    }
    goto fat32_get_long_name;

fat32_get_name_end:
    *name = 0;
    return retname;
}

static
fdb_t *     Fat32_name_to_directory(const char * name,fdb_t * fdb)
{
    word_t          un[128] = {0};  /*  unicode name    */

    return fdb;
}

static
result_t    Fat32_directory_inc_space(device_t * hd,fat_t * fat,
                                      fat_dir_t * dir,uint_t size)
{
    return -1;
}

static
result_t    Fat32_directory_dec_space()
{
    return -1;
}
/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  普通文件操作函数：定位、读、写
*/

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Fat32_next_cluster
//  作    用: 取簇链表的下一个。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT所在设备
//     fat_t *      |   fat         |   分区对象指针
//     uint32_t     |   cluster     |   簇地址
//  返 回 值: 返回簇地址。
//  注    意: 
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-14  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
uint32_t    Fat32_next_cluster(device_t * hd,fat_t * fat,
                               uint32_t cluster)
{
    blkbuf_t      * sbb = NULL; /**/
    byte_t        * buf = NULL;
    uint64_t        addr= 0;

    addr = FAT32_TSC_TO_HDA(fat,cluster);
    if( NULL == (sbb = Bbuf_read(hd,addr) ) )
        return 0x0FFFFFF8;
    buf  = BBUF_TO_SECTOR(sbb,addr);
    addr = (*(uint32_t *)(buf+FAT32_THIS_SECT_OFFSET(cluster))) & 0x0FFFFFFF;
    Bbuf_release(sbb);

    return (uint32_t)addr;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Fat32_offset_to_clus
//  作    用: 从文件偏移定位簇号。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT所在设备
//     fat_t *      |   fat         |   分区对象指针
//     uint32_t     |   dirclus     |   目录起始簇
//     uint32_t     |   offset      |   目录文件内的偏移。
//  返 回 值: 簇地址。
//  注    意: 需要与FAT32_OFFSET_TO_SECT、FAT32_OFFSET_TO_DIR_IDX配合使用

//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-15  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
uint32_t    Fat32_offset_to_clus(device_t * hd,fat_t * fat,
                                 uint32_t   dirclus,uint32_t offset)
{
    uint32_t        ptr = 0;

    while( ptr + fat->fat_clus_size < offset )
    {
        dirclus = Fat32_next_cluster(hd,fat,dirclus);
        ptr += fat->fat_clus_size;
    }

    return dirclus;
}
#define Fat32_read                  Fat32_file_read
/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Fat32_read_file
//  作    用: 读目录内容。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT所在设备
//     fat_t *      |   fat         |   分区对象指针
//     uint32_t     |   dirclus     |   目录起始簇
//     uint32_t     |   filesize    |   文件长度
//     dword_t      |   flag        |   文件类型标志，1表示目录文件
//     uint32_t     |   offset      |   文件偏移（指针）
//     int          |   size        |   需要读的字节数，也就是每次最多读2G
//     void *       |   buffer      |   缓冲区
//  返 回 值: 成功返回读的字节数，失败返回负值。
//  注    意: 返回0也表示成功，当文件指针在文件末尾时，就只能读出0字节。
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-19  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
int         Fat32_read_file(device_t * hd,fat_t * fat,uint32_t dirclus,
                            uint32_t filesize,dword_t flag,
                            uint32_t offset,int size,   /*  读范围 */
                            void * buffer)
{
    uint64_t        csa         = 0;    /*  cluster sector address  */
    uint32_t        ss          = 0,    /*  sector start            */
                    sdi         = 0;    /*  start directory idx     */
    uint_t          i           = 0,
                    fo          = 0;    /*  file offset             */
    int             dircnt      = 0,
                    nc          = 0,    /*  number of copy          */
                    left        = 0;    /*  剩余字节数              */
    byte_t        * des         = buffer;
    byte_t        * sectbuf     = NULL;
    BBUF_DECLARE();

    _printk("FAT32 read!\n");
    left    = MIN(0xFFFFFFFF - offset,(uint_t)ABS(size));
    /*
     *    如果要读的是目录文件，文件长度限制在2M以内，这是由于微软限制目录文件
     *  只能包含64K个目录
     */
    if( flag & 1 )
        filesize = 2 * M;

    /*  定位信息: 簇号、扇区号、扇区内偏移 */
    dirclus = Fat32_offset_to_clus(hd,fat,dirclus,offset);
    ss      = FAT32_OFFSET_TO_SECT(fat,offset);
    fo      = offset % fat->fat_bytes_per_sect;

    /*  遍历文件簇 */
fat32_read_file_begin:
    csa = FAT32_CLUS_TO_SECT(fat,dirclus);
    /*  遍历醋内扇区  */
    for(i = ss ; 
        i < fat->fat_sects_per_clus && left && offset < filesize; 
        i++ )
    {
        BBUF_READ_SECTOR(sectbuf,hd,csa+i);
        if( NULL == sectbuf)
            goto fat32_read_file_end;
        /*
         *    决定扇区内数据复制量的几个条件：剩余缓冲区、扇区剩余字节、文件剩
         *  余长度。需要取这三个条件的最小值。
         */
        nc = MIN(fat->fat_bytes_per_sect - fo,filesize - offset);
        nc = MIN(nc,left);
        _memcpy(des,sectbuf + fo,nc);
        BBUF_RELEASE();

        fo       = 0;
        left    -= nc;
        sectbuf += nc;
        offset  += nc;
    }
    ss = 0;
    dirclus = Fat32_next_cluster(hd,fat,dirclus);
    if( dirclus < FAT32_END_OF_CLUS )
        goto fat32_read_file_begin;

fat32_read_file_end:
    return size - left;
}


/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Fat32_write_file
//  作    用: 读目录内容。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT所在设备
//     fat_t *      |   fat         |   分区对象指针
//     uint32_t     |   dirclus     |   目录起始簇
//     dword_t      |   flag        |   文件类型的标志
//     uint32_t     |   offset      |   文件偏移（指针）
//     int          |   size        |   需要读的字节数，也就是每次最多读2G
//     void *       |   buffer      |   缓冲区
//  返 回 值: 成功返回读的字节数，失败返回负值。
//  注    意: 
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-05-31  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
int         Fat32_write_file(device_t * hd,fat_t * fat,uint32_t * dirclus,
                             dword_t flag,
                             uint32_t offset,int size,
                             void * buffer)
                             
{
    uint64_t        csa         = 0;
    uint32_t        ss          = 0,
                    so          = 0;
    int             left        = 0,
                    nw          = 0 /* number of write */;
    uint32_t        pos         = 0;
    uint32_t        clus        = *dirclus,
                    tmp,
                    fms         = FAT32_FILE_SIZE_MAX/*  file max size*/;
    byte_t        * buf         = buffer;
    byte_t        * sectbuf     = NULL;
    BBUF_DECLARE();
    
    ASSERT(hd);
    ASSERT(fat);
    ASSERT(buffer);
    ASSERT(size >= 0);

    tmp = *dirclus;
    if( flag & 1 )
        fms = 2*M;

fat32_write_file_begin:
    /*  文件长度的上限是否允许增加，如不允许，停止写入  */
    if( fms - pos < fat->fat_clus_size )
        goto fat32_write_file_end;
    /*  允许写入，就要确保当前簇有效    */
    if( tmp == 0 || tmp >= FAT32_END_OF_CLUS)
    {
        /*  分配空间  */
        if( Fat32_cluster_alloc(hd,fat,&tmp,1) != 1 )
            goto fat32_write_file_end;
        /*  链接新分配的空间    */
        if( 0 == *dirclus )
            *dirclus = tmp;
        else
            Fat32_cluster_link(hd,fat,clus,&tmp,1);
    }
    clus = tmp;
    csa = FAT32_CLUS_TO_SECT(fat,clus);
    /*  在范围内，执行写入*/
    if( pos + fat->fat_clus_size >= offset )
    {
        ss = FAT32_OFFSET_TO_SECT(fat,offset);
        so = offset / fat->fat_bytes_per_sect;
        /*  逐个扇区写入  */
        for( i = ss ; i < fat->fat_sects_per_clus && left > 0 ; i++ )
        {
            BBUF_READ_SECTOR(sectbuf,hd,csa+i);
            if( NULL == sectbuf )
                goto fat32_write_file_end;
            nw = 0;
            _memcpy(sectbuf + so,buf,nw);
            BBUF_RELEASE();

            so      = 0;
            offset += nw;
            buf    += nw;
        }
    }
    
    tmp = Fat32_next_cluster(hd,fat,clus);
    pos += fat->fat_clus_size;
    goto fat32_write_file_begin;

fat32_write_file_end:
    return size - left;
}

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  目录文件操作函数：遍历、定位、增、删、改
*/


#define Fat32_dir_find              Fat32_find_directory
/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Fat32_find_directory
//  作    用: 精确查找目录。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT所在设备
//     fat_t *      |   fat         |   分区对象指针
//     uint32_t     |   dirclus     |   目录文件的起始簇
//     const char * |   name        |   待查找的目录名称
//     int          |   flags       |   查找标志
//             FAT32_DIR_FIND_FILE  |   仅查找文件
//
//  输出参数: 
//     fat_dir_t *  |   dir         |   目录缓存，需提供满足最大文件长度的空
//                  |               | 间，按长名目录可以存放26个字符，需要提供
//                  |               | 11个。
//     uint32_t *   |   offset      |   目标目录在目录文件内的偏移
//  返 回 值: 成功返回RESULT_SUCCEED。
//  注    意: 需要与FAT32_OFFSET_TO_SECT、FAT32_OFFSET_TO_DIR_IDX配合使用

//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-15  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
result_t    Fat32_find_directory(device_t * hd,fat_t * fat,uint32_t dirclus,
                                 const char * name,int flags,
                                 fat_dir_t * dir,uint32_t * offset)
{
    result_t        result      = RESULT_FAILED;
    uint64_t        csa         = 0;/* cluster sector address*/
    uint32_t        clus        = dirclus;
    uint_t          i           = 0,    /* sectoer index    */
                    ci          = 0,    /* cluster index    */
                    di          = 0;    /* directory index  */
    int             dircnt      = 0;
    fat_dir_t     * dirbuf      = NULL;
    char            dirname[256];
    BBUF_DECLARE();

    if( dirclus == 0 )
        return -1;
    _memzero(dir,sizeof(fat_dir_t)*11);
    /*  遍历目录文件 */
fat32_dir_find_begin:
    /*  遍历簇内扇区*/
    csa = FAT32_CLUS_TO_SECT(fat,clus);
    for( i = 0 ; i < fat->fat_sects_per_clus ; i++ )
    {
        /*  遍历扇区内目录  */
        BBUF_READ_SECTOR(dirbuf,hd,csa + i);
        if( NULL == dirbuf )
            goto fat32_dir_find_end;
        BBUF_LOCK();
        for( di = 0  ; di < 16 ; di++)
        {
            /*  到达目录文件末尾*/
            if( 0 == *dirbuf[di].fdir_name )
                goto fat32_dir_find_release;
            /*  本目录未使用    */
            if( 0xE5 == *dirbuf[di].fdir_name )
            {
                dircnt = 0;
                _memzero(dir,sizeof(fat_dir_t)*11);
                continue;
            }
            /*  到达这里说明发现有效目录，检测属性 */
            dir[dircnt++] = dirbuf[di];
            if( FATDIR_IS_LONG_NAME_DIR(dirbuf+di) )
                continue;
            /*  如果明确要求查找文件 */
            if( flags & FAT32_DIR_FIND_FILE )
                if( dirbuf[di].fdir_attr & FATDIR_ATTR_DIRECTORY )
                    continue;
            //Fat32_directory_information(dirbuf + di);
            Fat32_get_directory_name(dir,dirname);
            if( _namecmp(name,dirname) == 0 )
            {
                //_printk("name found!\n");
                *offset = ci * fat->fat_clus_size + 
                          i * fat->fat_bytes_per_sect +
                          di * sizeof(fat_dir_t);
                result = RESULT_SUCCEED;
                goto fat32_dir_find_release;
            }

            /*  处理完毕后清零 */
            _memzero(dir,sizeof(fat_dir_t)*11);
            dircnt = 0;
        }
        BBUF_FREE();
        BBUF_RELEASE();
    }
    ci++;
    clus = Fat32_next_cluster(hd,fat,clus);
    if( clus < FAT32_END_OF_CLUS )
        goto fat32_dir_find_begin;
    goto fat32_dir_find_end;
fat32_dir_find_release:
    BBUF_FREE();
    BBUF_RELEASE();
fat32_dir_find_end:
    return result;
}


/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Fat32_first_directory
//  作    用: 从给定的偏移查找第一个目录。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT所在设备
//     fat_t *      |   fat         |   分区对象指针
//     uint32_t     |   dirclus     |   目录起始簇
//     fat_dir_t *  |   dir         |   目录缓存，需提供满足最大文件长度的空
//                  |               | 间，按长名目录可以存放26个字符，需要提供
//                  |               | 11个。
//     uint32_t *   |   offset      |   偏移指针。有两个作用，调用函数时，作为
//                                  | 查找起点，函数完成时，保存有找到目录在目
//                                  | 录文件内的偏移。
//  返 回 值: 成功返回RESULT_SUCCEED，失败返回RESULT_FAILED。
//  注    意: 通过该函数可以进行目录的遍历，可以参考如下代码:
//  uint32_t        offset = 0;
//  fat_dir_t       dir[FATDIR_MAX_DIRCTORYS];
//
//  while( RESULT_SUCCEED == Fat32_first_directory(hd,fat,dirclus,dir,&offset)
//  {
//      ......
//  }
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-15  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
result_t    Fat32_first_directory(device_t * hd,fat_t * fat,uint32_t dirclus,
                                  fat_dir_t * dir,uint32_t * offset)
{
    result_t        result      = RESULT_FAILED;
    blkbuf_t      * sbb         = NULL;
    uint64_t        csa         = 0;/* cluster sector address*/
    uint32_t        ss          = 0,
                    sdi         = 0;
    uint_t          i           = 0,
                    di          = 0; /* */
    int             dircnt      = 0;
    fat_dir_t     * dirbuf      = NULL;

    /*  首先需要定位 */
    dirclus = Fat32_offset_to_clus(hd,fat,dirclus,*offset);
    ss      = FAT32_OFFSET_TO_SECT(fat,*offset);
    sdi     = FAT32_OFFSET_TO_DIR_IDX(fat,*offset);
    /*  遍历目录文件 */
fat32_dir_get_first_begin:
    csa = FAT32_CLUS_TO_SECT(fat,dirclus);
    /*  遍历扇区  */
    for( i = ss ; i < fat->fat_sects_per_clus ; i++ )
    {
        if( NULL == (sbb = Bbuf_read(hd,csa+i) ) )
            goto fat32_dir_get_first_end;
        dirbuf = BBUF_TO_SECTOR(sbb,csa+i);
        /*  遍历扇区内目录  */
        for(di = sdi ; di < FAT32_DIRECTORYS_PER_SECTOR(fat) ;di++)
        {
            *offset += FAT32_DIRECTORY_SIZE;
            /*  到达目录文件末尾，终止查找 */
            if( 0 == *dirbuf[di].fdir_name )
            {
                Bbuf_release(sbb);
                goto fat32_dir_get_first_end;
            }
            /*  忽略未使用的目录    */
            if( 0xE5 == *dirbuf[di].fdir_name )
                continue;
            
            /*  到达这里说明发现有效目录，记录偏移 */
            dir[dircnt++] = dirbuf[di];
            if( FATDIR_IS_LONG_NAME_DIR(dirbuf+di) )
                continue;
            result = RESULT_SUCCEED;
            Bbuf_release(sbb);
            goto fat32_dir_get_first_end;
        }
        Bbuf_release(sbb);
        sdi = 0;
    }
    ss = 0;
    dirclus = Fat32_next_cluster(hd,fat,dirclus);
    if( dirclus < FAT32_END_OF_CLUS )
        goto fat32_dir_get_first_begin;

fat32_dir_get_first_end:
    return result;
}

static
uint32_t    Fat32_get_free_directory(device_t * hd,fat_t * fat,uint32_t dirclus,
                                     uint_t numofdir)
{
    return 0;
}
/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Fat32_create_directory
//  作    用: 创建目录。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT所在设备
//     fat_t *      |   fat         |   分区对象指针
//     uint32_t     |   dirclus     |   目录起始簇
//     const char * |   name        |   目录名称
//     dword_t      |   attr        |   需要创建的目录属性
//
//  返 回 值: 成功返回RESULT_SUCCEED，失败返回其他。
//  注    意:   函数仅完成查找到可用空间，并将目录信息写入，不检测重名。调用该
//            函数前，应检测是否重名，使用Fat32_find_directory来检测.
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-05-31  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
result_t    Fat32_create_directory(device_t * hd,fat_t * fat,uint32_t dirclus,
                                   const char * name,dword_t attr)
{
    fdb_t           fdb;
    uint32_t        wo;     /*  write offset目录文件的写入位置*/
    uint_t          nd;     /*  number of directoris */
    Fat32_name_to_directory(name,&fdb);

    /*  查找连续的目录空间  */
    if( 0 == ( wo = Fat32_get_free_directory(hd,fat,dirclus,nd) ) )
        return RESULT_FAILED;

    if( Fat32_file_write(hd,fat,dirclus,wo,64*K,NULL,nd) < 0 )
        return RESULT_FAILED;

    /*  如果是目录文件还要分配一簇，并创建基本目录*/
    return RESULT_SUCCEED;
}
/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  文件系统驱动接口
*/
static 
result_t    Fat32_initial(fs_t * fs)
{
    blkbuf_t      * blkbuf  = NULL;
    bpb32_t       * bpb     = NULL;
    fat_t         * fat     = NULL;

    _printk("FAT32 initial...\n");
    _printk("start=%ld total=%ld\n",
        fs->fs_part.fsp_start,fs->fs_part.fsp_total);
    if( NULL == (blkbuf=Bbuf_read(fs->fs_part.fsp_dev,fs->fs_part.fsp_start)))
        return RESULT_FAILED;
    bpb = BBUF_TO_SECTOR(blkbuf,fs->fs_part.fsp_start);
    fat = (fat_t *)fs->fs_param_blk;
    fat->fat_start_sector       = FS_START_SECTOR(fs);
    fat->fat_total_sectors      = FS_TOTAL_SECTORS(fs);
    fat->fat_clus_size          = FAT32_CLUS_SIZE(bpb);
    fat->fat_start_data_sector  = FAT32_START_DATA_SECTOR(bpb);
    fat->fat_total_data_sectors = FAT32_TOTAL_DATA_SECTORS(bpp);
    fat->fat_total_clus         = FAT32_TOTAL_CLUS(bpb);
    fat->fat_bytes_per_sect     = bpb->bpb32_bytes_per_sect;
    fat->fat_sects_per_clus     = bpb->bpb32_sects_per_clus;
    fat->fat_fat_size           = bpb->bpb32_fat_size32;
    fat->fat_num_of_fat         = bpb->bpb32_num_of_fats;
    fat->fat_root_clus          = bpb->bpb32_root_clus;
    fat->fat_rsvd_sectors       = bpb->bpb32_rsvd_sectors;
    fat->fat_root_dir_sectors   = 0;
    Bbuf_release(blkbuf);

    fat->fat_free_clus          = 0;
    fat->fat_next_free          = 0;
    /*  初始化文件系统对象的根目录文件节点  */
    fs->fs_root.fn_mount        = NULL;
    fs->fs_root.fn_fs           = fs;
    fs->fs_root.fn_fln          = fat->fat_root_clus;
    fs->fs_root.fn_size         = 0;
    fs->fs_root.fn_attr         = FATDIR_ATTR_DIRECTORY;
    return RESULT_SUCCEED;
}


#define FAT32_GET_FIRST_CLUS    FNODE_GET_FLN
static
result_t    Fat32_fnode_find(fnode_t     * parents,
                             const char  * name,
                             dword_t       flags,
                             fnode_t     * fnode)
{
    fat_dir_t       dir[FATDIR_MAX_DIRCTORYS];
    fs_t          * fs  = NULL;
    dword_t         pdo = 0;        /* parents directory offset */
    int             i   = 0;

    _printk("FAT32 fnode find\n");
    ASSERT( parents );
    ASSERT( fnode );

    if( NULL == name)
        return RESULT_FAILED;
    fs = FNODE_GET_FS(parents);
    if( RESULT_SUCCEED != 
        Fat32_dir_find(FS_DEVICE(fs),FS_PARAM_BLK(fs),
            (uint32_t)FAT32_GET_FIRST_CLUS(parents),name,flags,dir,&pdo) )
        return RESULT_FAILED;
    for( i = 0 ; i < FATDIR_MAX_DIRCTORYS ; i++)
    {
        if( !FATDIR_IS_LONG_NAME_DIR(dir+i) )
            break;
    }
    //Fat32_directory_information(dir + i);
    fnode->fn_attr |= FNODE_ATTR_READ|FNODE_ATTR_WRITE;
    if( dir[i].fdir_attr & FATDIR_ATTR_READ_ONLY )
        fnode->fn_attr &= ~FNODE_ATTR_WRITE;
    if( dir[i].fdir_attr & FATDIR_ATTR_DIRECTORY )
        fnode->fn_attr |= FNODE_ATTR_DIRECTORY;
    if( dir[i].fdir_attr & FATDIR_ATTR_HIDDEN)
        fnode->fn_attr |= FNODE_ATTR_HIDDEN;
    fnode->fn_fs        = parents->fn_fs;
    fnode->fn_mount     = NULL;
    fnode->fn_fln       = FATDIR_FIRST_CLUSTER(dir + i);
    fnode->fn_size      = dir[i].fdir_fsize;
    Sdate_to_date(dir[i].fdir_cd,&fnode->fn_cd);
    Stime_to_time(dir[i].fdir_ct,&fnode->fn_ct);
    Sdate_to_date(dir[i].fdir_lwd,&fnode->fn_lwd);
    Stime_to_time(dir[i].fdir_lwt,&fnode->fn_lwt);
    Sdate_to_date(dir[i].fdir_lad,&fnode->fn_lad);
    /*  记录目录在目录文件中的位置  */
    fnode->fn_pd_fln    = FAT32_GET_FIRST_CLUS(parents);
    fnode->fn_pd_offset = pdo;

    return RESULT_SUCCEED;
}

/*  2014.4.15  假设传入参数时，已经保证是非空白符开头*/
static
int         Fat32_is_short_dir_name(const char * name)
{
    const char    * n = name;
    int             i;
    /*  严格符合8.3标准，文件名小于等于8字节，扩展名小于等于3字节，中间不能有空格 */
    
    /*  检测文件名部分 */
    for( i = 0 ; i < 8 && *n && '.' != *n ; i++,n++)
    {
        /*  段文件名不允许出现空白符，存在则要使用长文件名*/
        if( ' ' == *n )
            return  0;
    }
    if( *n &&  '.' != *n )
        return 0;
    /*检测扩展名，过滤中间的点符号*/
    while( *n && '.' == *n )
        n++;
    for( i = 0 ; i < 3 && *n ; i++,n++)
    {
        if( ' ' == *n )
            return 0;
    }
    if( 0 == *n )
        return 1;
    return 0;
}

static
char *      Fat32_name_to_short_name(char * shortname,const char * name)
{
    const char    * n   = name;
    char          * sn  = shortname;
    int             i   = 0;

    for( i = 0 ; *n && i < 8 && '.' != *n; i++,n++)
    {
        if( *n >= 'a' && *n <= 'z' ) *sn++ = _up_case(*n);
        else *sn++ = *n;
    }
    /*  文件名不足8个字符的，空格填充   */
    for( ; i < 8 ; i++)
        *sn++ = ' ';
    /*  找到最后一个'.' */
    while( *n ) n++;
    n--;
    while( '.' != *n && (uint_t)n >= (uint_t)name) n--;
    if( *n != '.') 
        goto make_short_name_end;
    for( n++ ; *n && i < 11 ;i++, n++)
    {
        if( *n >= 'a' && *n <= 'z' ) *sn++ = _up_case(*n);
        else *sn++ = *n;
    }
make_short_name_end:
    /*  文件名不足，空格填充   */
    for( ; i < 11 ; i++)
        *sn++ = ' ';

    return shortname;
}

/*  2014.4.15  */
static
result_t    Fat32_fnode_create(fnode_t      * parents,
                               const char   * name,
                               dword_t        attrs,
                               fnode_t      * fnode)
{
    fat_dir_t       dir[FATDIR_MAX_DIRCTORYS];
    result_t        result  = RESULT_FAILED;
    uint32_t        pdo     = 0;        /* parents directory offset */
    fs_t          * fs      = NULL;
    //int             i   = 0;

    _printk("fat32 create fnode\n");
    if( NULL == name)
        return RESULT_FAILED;
    fs = FNODE_GET_FS(parents);
    /*  查找是否存在重名，存在则返回失败  */
    if( RESULT_SUCCEED == 
        Fat32_find_directory(FS_DEVICE(fs),FS_PARAM_BLK(fs),
            (uint32_t)FAT32_GET_FIRST_CLUS(parents),name,0,dir,&pdo) )
        goto fat32_fnode_create_end;
    _printk("fat32 create fnode.  file not exist! \n");

fat32_fnode_create_end:
    return result;
}

/*  读失败返回负数，成功返回非负，到达文件尾的时候，就属于读正确，但是没有读出数据*/
static
int         Fat32_fnode_read(fnode_t      * fnode,
                             uint64_t       offset,
                             void         * buffer,
                             int            size)
{
    fs_t          * fs      = NULL;
    
    ASSERT(fnode);
    ASSERT(buffer);
    ASSERT(size>0);
    _printk("FAT32 fnode read!\n");
    fs = FNODE_GET_FS(fnode);

    return -1;
}

result_t    Fat32_register(fsdrv_t * fsdrv)
{
    fsdrv->fs_type          = FS_PART_TYPE_FAT32;
    fsdrv->fs_initial       = Fat32_initial;
    fsdrv->fs_fnode_find    = Fat32_fnode_find;
    fsdrv->fs_fnode_create  = Fat32_fnode_create;
    fsdrv->fs_fnode_read    = Fat32_fnode_read;

    return RESULT_SUCCEED;
}