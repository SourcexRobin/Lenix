/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: fs_fat32.c
//  ����ʱ��: 2014-03-11        ������: �ޱ�
//  �޸�ʱ��: 2012-11-29        �޸���: �ޱ�
//  ��Ҫ����: FAT32�ļ�ϵͳ������
//  ˵    ��: 
//  FAT32��Ҫ�����Ŀ��
//      �ļ������ �����䡢���ա�����
//      Ŀ¼��     �����ҡ���������������ȡ������
//      ��ͨ�ļ�   ����λ������д
//      Ŀ¼�ļ�   ����������λ������ɾ����
//  ��������: ģ������_�����ʶ_����_����
//  �����¼:
//  �� �� ��    |     ʱ��      |  ��  ��       |  ��Ҫ�仯��¼
//=============================================================================
//  00.00.000   |  2014-03-11   |  ��  ��       |  �����ļ�
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
 *  UNICODEת��ΪANSI
 *  uc: UNICODE���룬unicode
 *  a : ANSI������ַ��ansi addreess
 *  �ļ�ϵͳ�б������UNICODE�����ǲ���ϵͳȴʹ��GBK2312���룬��Ҫ�ṩת������
 *  Ŀǰֱ�Ӹ�ֵ
 */
#define FAT32_UNICODE_TO_ANSI(uc,a) do{ if( (uc) < 127 ) *(a)++ = (char)(uc);\
                                        else *((word_t *)(a))++ = (uc);\
                                      }while(0)
/*  �������UNICODE�����ֽ��������ʵ�Ǵ���ģ�ֻ��Ϊ�˱�ʾ����� */
#define FAT32_PRINTK(uc)            do{ _printk("%c",(char)(uc));\
                                        if( (uc) > 127 ) \
                                          _printk("%c",(char)((uc) >> 8 ));\
                                      }while(0)

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  ���ԡ���������
*/

/*  2014-03-13����ʾFAT������Ϣ */
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

/*  2014-03-13����ʾBPB32��Ϣ */
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

/*  2014-03-15  ��ʾĿ¼����Ϣ  */
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

/*  2014-03-15  ����Ŀ¼���ڴ���Ϣ  */
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
//  �������ܺ���
*/

/*
//-----------------------------------------------------------------------------
//  ������ֱ�Ӹ�����΢��ٷ��ĵ�,����������ΪFat_name_chksum
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
//  �ļ������������������䡢���ա�����
*/

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Fat32_cluster_alloc
//  ��    ��: ������ôء�
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT�����豸
//     fat_t *      |   fat         |   ��������ָ��
//     uint32_t *   |   clusters    |   �ػ���
//     uint_t       |   count       |   Ҫ���������
//  �� �� ֵ: ���ط���صĸ�����
//  ע    ��: ���һ�η���128�ء�
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-14  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
static
int     Fat32_cluster_alloc(device_t * hd      ,fat_t    * fat,
                            uint32_t * clusters,uint_t     count)
{
    uint64_t        fsa         = 0;    /*  fat sector address  */
    blkbuf_t      * sbb         = NULL; /*  sector blkbuf       */
    uint32_t      * clusbuf     = NULL;
    uint_t          cnt         = 0,    /*  �ѷ��������        */
                    i           = 0,
                    j           = 0;
    BBUF_DECLARE();

    count   = count > 128 ? 128 : count;
    fsa     = FAT32_FAT_FIRST(fat);
    /*  ɨ��FAT������0���ͼ�¼�����ҽ�����Ϊռ�� */
    for( i = 0 ;  i < fat->fat_fat_size && cnt < count ; i++ ,fsa++)
    {
        BBUF_READ_SECTOR(clusbuf,hd,fsa+i);
        if( NULL == clusbuf )
            goto fat32_cluster_alloc_end;
        BBUF_LOCK();
        /*  ����������ÿ������������128���ص�ַ */
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
//  ��    ��: Fat32_cluster_free
//  ��    ��: �ͷŴء�
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT�����豸
//     fat_t *      |   fat         |   ��������ָ��
//     uint32_t *   |   clusters    |   �ػ���
//     uint_t       |   count       |   Ҫ���������
//  �� �� ֵ: ���ط���صĸ�����
//  ע    ��: ���һ�η���128�ء�
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-14  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
static
int         Fat32_cluster_free(device_t * hd      ,fat_t    * fat,
                               uint32_t * clusters,uint_t     count)
{
    uint64_t        fsa         = 0;    /*  FAT������ַ��fat sector address */
    blkbuf_t      * sbb         = NULL; /*  �����黺�棬sector blkbuf       */
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
//  ��    ��: Fat32_cluster_link
//  ��    ��: ���Ӵء�
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT�����豸
//     fat_t *      |   fat         |   ��������ָ��
//     uint32_t     |   startclus   |   ��ʼ�غ�
//     uint32_t *   |   clusters    |   �ػ���
//     uint_t       |   count       |   Ҫ���������
//  �� �� ֵ: ���ط���صĸ�����
//  ע    ��: ���һ�η���128�ء�
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-05-31  |   ��  ��      |  ��һ��
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
//  Ŀ¼�������������������������ȡ������
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
//  ��    ��: Fat32_directory_to_name
//  ��    ��: ��ȡĿ¼���ơ�
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     fat_dir_t *  |   dir         |   Ŀ¼���棬���ṩ��������ļ����ȵĿ�
//                  |               | �䣬������Ŀ¼���Դ��26���ַ�����Ҫ�ṩ
//                  |               | 11����
//     char *       |   name        |   �ļ�����������Ӧ��֤256���ֽڻ�����
//  �� �� ֵ: ���ز���name�ĵ�ַ��
//  ע    ��: �������ļ���һͬ����
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-15  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
static
char *      Fat32_directory_to_name(fat_dir_t * dir,char * name)
{
    int             i       = 0,    /*  index                       */
                    lndi    = 0;    /*  long name directory index   */
    fdir_lnentry_t* lndir   = NULL; /*  long name directory         */
    char          * retname = name; /*  return name                 */

    /*  �жϳ������ļ���    */
    if( FATDIR_IS_LONG_NAME_DIR(dir) )
    {
        /*  ����ռ�õ�Ŀ¼��    */
        i = 0;
        while( FATDIR_IS_LONG_NAME_DIR(dir + i) )
            i++;
        goto fat32_get_long_name;
    }
    /*  ������ļ���    */
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

    /*  �����ļ����������Ƿ�����Ŀ¼λ�ã���Ҫ������  */
fat32_get_long_name:
    if( i < 0 )
        goto fat32_get_name_end;
    /*  ��������Ŀ¼�����Ķ��ļ���Ŀ¼  */
    if( !FATDIR_IS_LONG_NAME_DIR(dir+i) )
    {
        i--;
        goto fat32_get_long_name;
    }
    lndir = (fdir_lnentry_t *)(dir + i--);
    /*  ����Ŀ¼���ļ����ֳ���3�����֣�����Ҫ�ֿ����� */
    /*  �����1���� */
    for( lndi = 0 ; lndi < 5 ; lndi++ )
    {
        if( 0 == lndir->fdl_name1[lndi] )
            goto fat32_get_name_end;
        //FAT32_PRINTK(lndir->fdl_name1[lndi]);
        FAT32_UNICODE_TO_ANSI(lndir->fdl_name1[lndi],name);
    }
    /*  �����2���� */
    for( lndi = 0 ; lndi < 6 ; lndi++ )
    {
        if( 0 == lndir->fdl_name2[lndi] )
            goto fat32_get_name_end;
        //FAT32_PRINTK(lndir->fdl_name2[lndi]);
        FAT32_UNICODE_TO_ANSI(lndir->fdl_name2[lndi],name);
    }
    /*  �����3���� */
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
//  ��ͨ�ļ�������������λ������д
*/

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Fat32_next_cluster
//  ��    ��: ȡ���������һ����
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT�����豸
//     fat_t *      |   fat         |   ��������ָ��
//     uint32_t     |   cluster     |   �ص�ַ
//  �� �� ֵ: ���شص�ַ��
//  ע    ��: 
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-14  |   ��  ��      |  ��һ��
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
//  ��    ��: Fat32_offset_to_clus
//  ��    ��: ���ļ�ƫ�ƶ�λ�غš�
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT�����豸
//     fat_t *      |   fat         |   ��������ָ��
//     uint32_t     |   dirclus     |   Ŀ¼��ʼ��
//     uint32_t     |   offset      |   Ŀ¼�ļ��ڵ�ƫ�ơ�
//  �� �� ֵ: �ص�ַ��
//  ע    ��: ��Ҫ��FAT32_OFFSET_TO_SECT��FAT32_OFFSET_TO_DIR_IDX���ʹ��

//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-15  |   ��  ��      |  ��һ��
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
//  ��    ��: Fat32_read_file
//  ��    ��: ��Ŀ¼���ݡ�
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT�����豸
//     fat_t *      |   fat         |   ��������ָ��
//     uint32_t     |   dirclus     |   Ŀ¼��ʼ��
//     uint32_t     |   filesize    |   �ļ�����
//     dword_t      |   flag        |   �ļ����ͱ�־��1��ʾĿ¼�ļ�
//     uint32_t     |   offset      |   �ļ�ƫ�ƣ�ָ�룩
//     int          |   size        |   ��Ҫ�����ֽ�����Ҳ����ÿ������2G
//     void *       |   buffer      |   ������
//  �� �� ֵ: �ɹ����ض����ֽ�����ʧ�ܷ��ظ�ֵ��
//  ע    ��: ����0Ҳ��ʾ�ɹ������ļ�ָ�����ļ�ĩβʱ����ֻ�ܶ���0�ֽڡ�
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-19  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
static
int         Fat32_read_file(device_t * hd,fat_t * fat,uint32_t dirclus,
                            uint32_t filesize,dword_t flag,
                            uint32_t offset,int size,   /*  ����Χ */
                            void * buffer)
{
    uint64_t        csa         = 0;    /*  cluster sector address  */
    uint32_t        ss          = 0,    /*  sector start            */
                    sdi         = 0;    /*  start directory idx     */
    uint_t          i           = 0,
                    fo          = 0;    /*  file offset             */
    int             dircnt      = 0,
                    nc          = 0,    /*  number of copy          */
                    left        = 0;    /*  ʣ���ֽ���              */
    byte_t        * des         = buffer;
    byte_t        * sectbuf     = NULL;
    BBUF_DECLARE();

    _printk("FAT32 read!\n");
    left    = MIN(0xFFFFFFFF - offset,(uint_t)ABS(size));
    /*
     *    ���Ҫ������Ŀ¼�ļ����ļ�����������2M���ڣ���������΢������Ŀ¼�ļ�
     *  ֻ�ܰ���64K��Ŀ¼
     */
    if( flag & 1 )
        filesize = 2 * M;

    /*  ��λ��Ϣ: �غš������š�������ƫ�� */
    dirclus = Fat32_offset_to_clus(hd,fat,dirclus,offset);
    ss      = FAT32_OFFSET_TO_SECT(fat,offset);
    fo      = offset % fat->fat_bytes_per_sect;

    /*  �����ļ��� */
fat32_read_file_begin:
    csa = FAT32_CLUS_TO_SECT(fat,dirclus);
    /*  ������������  */
    for(i = ss ; 
        i < fat->fat_sects_per_clus && left && offset < filesize; 
        i++ )
    {
        BBUF_READ_SECTOR(sectbuf,hd,csa+i);
        if( NULL == sectbuf)
            goto fat32_read_file_end;
        /*
         *    �������������ݸ������ļ���������ʣ�໺����������ʣ���ֽڡ��ļ�ʣ
         *  �೤�ȡ���Ҫȡ��������������Сֵ��
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
//  ��    ��: Fat32_write_file
//  ��    ��: ��Ŀ¼���ݡ�
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT�����豸
//     fat_t *      |   fat         |   ��������ָ��
//     uint32_t     |   dirclus     |   Ŀ¼��ʼ��
//     dword_t      |   flag        |   �ļ����͵ı�־
//     uint32_t     |   offset      |   �ļ�ƫ�ƣ�ָ�룩
//     int          |   size        |   ��Ҫ�����ֽ�����Ҳ����ÿ������2G
//     void *       |   buffer      |   ������
//  �� �� ֵ: �ɹ����ض����ֽ�����ʧ�ܷ��ظ�ֵ��
//  ע    ��: 
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-05-31  |   ��  ��      |  ��һ��
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
    /*  �ļ����ȵ������Ƿ��������ӣ��粻����ֹͣд��  */
    if( fms - pos < fat->fat_clus_size )
        goto fat32_write_file_end;
    /*  ����д�룬��Ҫȷ����ǰ����Ч    */
    if( tmp == 0 || tmp >= FAT32_END_OF_CLUS)
    {
        /*  ����ռ�  */
        if( Fat32_cluster_alloc(hd,fat,&tmp,1) != 1 )
            goto fat32_write_file_end;
        /*  �����·���Ŀռ�    */
        if( 0 == *dirclus )
            *dirclus = tmp;
        else
            Fat32_cluster_link(hd,fat,clus,&tmp,1);
    }
    clus = tmp;
    csa = FAT32_CLUS_TO_SECT(fat,clus);
    /*  �ڷ�Χ�ڣ�ִ��д��*/
    if( pos + fat->fat_clus_size >= offset )
    {
        ss = FAT32_OFFSET_TO_SECT(fat,offset);
        so = offset / fat->fat_bytes_per_sect;
        /*  �������д��  */
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
//  Ŀ¼�ļ�������������������λ������ɾ����
*/


#define Fat32_dir_find              Fat32_find_directory
/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Fat32_find_directory
//  ��    ��: ��ȷ����Ŀ¼��
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT�����豸
//     fat_t *      |   fat         |   ��������ָ��
//     uint32_t     |   dirclus     |   Ŀ¼�ļ�����ʼ��
//     const char * |   name        |   �����ҵ�Ŀ¼����
//     int          |   flags       |   ���ұ�־
//             FAT32_DIR_FIND_FILE  |   �������ļ�
//
//  �������: 
//     fat_dir_t *  |   dir         |   Ŀ¼���棬���ṩ��������ļ����ȵĿ�
//                  |               | �䣬������Ŀ¼���Դ��26���ַ�����Ҫ�ṩ
//                  |               | 11����
//     uint32_t *   |   offset      |   Ŀ��Ŀ¼��Ŀ¼�ļ��ڵ�ƫ��
//  �� �� ֵ: �ɹ�����RESULT_SUCCEED��
//  ע    ��: ��Ҫ��FAT32_OFFSET_TO_SECT��FAT32_OFFSET_TO_DIR_IDX���ʹ��

//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-15  |   ��  ��      |  ��һ��
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
    /*  ����Ŀ¼�ļ� */
fat32_dir_find_begin:
    /*  ������������*/
    csa = FAT32_CLUS_TO_SECT(fat,clus);
    for( i = 0 ; i < fat->fat_sects_per_clus ; i++ )
    {
        /*  ����������Ŀ¼  */
        BBUF_READ_SECTOR(dirbuf,hd,csa + i);
        if( NULL == dirbuf )
            goto fat32_dir_find_end;
        BBUF_LOCK();
        for( di = 0  ; di < 16 ; di++)
        {
            /*  ����Ŀ¼�ļ�ĩβ*/
            if( 0 == *dirbuf[di].fdir_name )
                goto fat32_dir_find_release;
            /*  ��Ŀ¼δʹ��    */
            if( 0xE5 == *dirbuf[di].fdir_name )
            {
                dircnt = 0;
                _memzero(dir,sizeof(fat_dir_t)*11);
                continue;
            }
            /*  ��������˵��������ЧĿ¼��������� */
            dir[dircnt++] = dirbuf[di];
            if( FATDIR_IS_LONG_NAME_DIR(dirbuf+di) )
                continue;
            /*  �����ȷҪ������ļ� */
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

            /*  ������Ϻ����� */
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
//  ��    ��: Fat32_first_directory
//  ��    ��: �Ӹ�����ƫ�Ʋ��ҵ�һ��Ŀ¼��
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT�����豸
//     fat_t *      |   fat         |   ��������ָ��
//     uint32_t     |   dirclus     |   Ŀ¼��ʼ��
//     fat_dir_t *  |   dir         |   Ŀ¼���棬���ṩ��������ļ����ȵĿ�
//                  |               | �䣬������Ŀ¼���Դ��26���ַ�����Ҫ�ṩ
//                  |               | 11����
//     uint32_t *   |   offset      |   ƫ��ָ�롣���������ã����ú���ʱ����Ϊ
//                                  | ������㣬�������ʱ���������ҵ�Ŀ¼��Ŀ
//                                  | ¼�ļ��ڵ�ƫ�ơ�
//  �� �� ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���RESULT_FAILED��
//  ע    ��: ͨ���ú������Խ���Ŀ¼�ı��������Բο����´���:
//  uint32_t        offset = 0;
//  fat_dir_t       dir[FATDIR_MAX_DIRCTORYS];
//
//  while( RESULT_SUCCEED == Fat32_first_directory(hd,fat,dirclus,dir,&offset)
//  {
//      ......
//  }
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-15  |   ��  ��      |  ��һ��
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

    /*  ������Ҫ��λ */
    dirclus = Fat32_offset_to_clus(hd,fat,dirclus,*offset);
    ss      = FAT32_OFFSET_TO_SECT(fat,*offset);
    sdi     = FAT32_OFFSET_TO_DIR_IDX(fat,*offset);
    /*  ����Ŀ¼�ļ� */
fat32_dir_get_first_begin:
    csa = FAT32_CLUS_TO_SECT(fat,dirclus);
    /*  ��������  */
    for( i = ss ; i < fat->fat_sects_per_clus ; i++ )
    {
        if( NULL == (sbb = Bbuf_read(hd,csa+i) ) )
            goto fat32_dir_get_first_end;
        dirbuf = BBUF_TO_SECTOR(sbb,csa+i);
        /*  ����������Ŀ¼  */
        for(di = sdi ; di < FAT32_DIRECTORYS_PER_SECTOR(fat) ;di++)
        {
            *offset += FAT32_DIRECTORY_SIZE;
            /*  ����Ŀ¼�ļ�ĩβ����ֹ���� */
            if( 0 == *dirbuf[di].fdir_name )
            {
                Bbuf_release(sbb);
                goto fat32_dir_get_first_end;
            }
            /*  ����δʹ�õ�Ŀ¼    */
            if( 0xE5 == *dirbuf[di].fdir_name )
                continue;
            
            /*  ��������˵��������ЧĿ¼����¼ƫ�� */
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
//  ��    ��: Fat32_create_directory
//  ��    ��: ����Ŀ¼��
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     device_t *   |   hd          |   FAT�����豸
//     fat_t *      |   fat         |   ��������ָ��
//     uint32_t     |   dirclus     |   Ŀ¼��ʼ��
//     const char * |   name        |   Ŀ¼����
//     dword_t      |   attr        |   ��Ҫ������Ŀ¼����
//
//  �� �� ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���������
//  ע    ��:   ��������ɲ��ҵ����ÿռ䣬����Ŀ¼��Ϣд�룬��������������ø�
//            ����ǰ��Ӧ����Ƿ�������ʹ��Fat32_find_directory�����.
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-05-31  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
static
result_t    Fat32_create_directory(device_t * hd,fat_t * fat,uint32_t dirclus,
                                   const char * name,dword_t attr)
{
    fdb_t           fdb;
    uint32_t        wo;     /*  write offsetĿ¼�ļ���д��λ��*/
    uint_t          nd;     /*  number of directoris */
    Fat32_name_to_directory(name,&fdb);

    /*  ����������Ŀ¼�ռ�  */
    if( 0 == ( wo = Fat32_get_free_directory(hd,fat,dirclus,nd) ) )
        return RESULT_FAILED;

    if( Fat32_file_write(hd,fat,dirclus,wo,64*K,NULL,nd) < 0 )
        return RESULT_FAILED;

    /*  �����Ŀ¼�ļ���Ҫ����һ�أ�����������Ŀ¼*/
    return RESULT_SUCCEED;
}
/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  �ļ�ϵͳ�����ӿ�
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
    /*  ��ʼ���ļ�ϵͳ����ĸ�Ŀ¼�ļ��ڵ�  */
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
    /*  ��¼Ŀ¼��Ŀ¼�ļ��е�λ��  */
    fnode->fn_pd_fln    = FAT32_GET_FIRST_CLUS(parents);
    fnode->fn_pd_offset = pdo;

    return RESULT_SUCCEED;
}

/*  2014.4.15  ���贫�����ʱ���Ѿ���֤�Ƿǿհ׷���ͷ*/
static
int         Fat32_is_short_dir_name(const char * name)
{
    const char    * n = name;
    int             i;
    /*  �ϸ����8.3��׼���ļ���С�ڵ���8�ֽڣ���չ��С�ڵ���3�ֽڣ��м䲻���пո� */
    
    /*  ����ļ������� */
    for( i = 0 ; i < 8 && *n && '.' != *n ; i++,n++)
    {
        /*  ���ļ�����������ֿհ׷���������Ҫʹ�ó��ļ���*/
        if( ' ' == *n )
            return  0;
    }
    if( *n &&  '.' != *n )
        return 0;
    /*�����չ���������м�ĵ����*/
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
    /*  �ļ�������8���ַ��ģ��ո����   */
    for( ; i < 8 ; i++)
        *sn++ = ' ';
    /*  �ҵ����һ��'.' */
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
    /*  �ļ������㣬�ո����   */
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
    /*  �����Ƿ���������������򷵻�ʧ��  */
    if( RESULT_SUCCEED == 
        Fat32_find_directory(FS_DEVICE(fs),FS_PARAM_BLK(fs),
            (uint32_t)FAT32_GET_FIRST_CLUS(parents),name,0,dir,&pdo) )
        goto fat32_fnode_create_end;
    _printk("fat32 create fnode.  file not exist! \n");

fat32_fnode_create_end:
    return result;
}

/*  ��ʧ�ܷ��ظ������ɹ����طǸ��������ļ�β��ʱ�򣬾����ڶ���ȷ������û�ж�������*/
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