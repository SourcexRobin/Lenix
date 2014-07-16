//  2014-03-10  建立文件

#include <config.h>
#include <type.h>

#ifndef _FS_FAT32_H_
#define _FS_FAT32_H_

#include "fs.h"

#define FAT_DIR_NAME_LENG           11
#define FAT32_FILE_SIZE_MAX         (0xFFFFFFFF)
#pragma pack(1)
/*  FAT32文件参数块*/
typedef struct _bpb_fat32_t
{
    byte_t                  bpb32_jmp_boot[3];
    byte_t                  bpb32_OME_name[8];
    word_t                  bpb32_bytes_per_sect;
    byte_t                  bpb32_sects_per_clus;
    word_t                  bpb32_rsvd_sectors;
    byte_t                  bpb32_num_of_fats;
    word_t                  bpb32_root_entry_cnt;
    word_t                  bpb32_total_sect16;
    byte_t                  bpb32_media;
    word_t                  bpb32_fat_size16;
    word_t                  bpb32_sect_per_track;
    word_t                  bpb32_num_of_heads;
    dword_t                 bpb32_hidden_sectors;
    dword_t                 bpb32_total_sect32;
    dword_t                 bpb32_fat_size32;
    word_t                  bpb32_ext_flag;
    word_t                  bpb32_fs_ver;
    dword_t                 bpb32_root_clus;
    word_t                  bpb32_fs_info;
    word_t                  bpb32_bk_boot_sect;
    byte_t                  bpb32_reserved[12];
    byte_t                  bpb32_bs_drv_num;
    byte_t                  bpb32_bs_rsvd1;
    byte_t                  bpb32_bs_boot_sig;
    dword_t                 bpb32_bs_vol_id;
    byte_t                  bpb32_bs_vol_lab[11];
    byte_t                  bpb32_bs_fs_type[8];
}bpb32_t;

#define FAT32_CLUS_SIZE(bpb)        ( (bpb)->bpb32_bytes_per_sect * \
                                      (bpb)->bpb32_sects_per_clus )
#define FAT32_START_DATA_SECTOR(bpb) \
                                    ( (bpb)->bpb32_rsvd_sectors + \
                                      (bpb)->bpb32_num_of_fats *  \
                                      (bpb)->bpb32_fat_size32 )
#define FAT32_TOTAL_DATA_SECTORS(bpp) \
                                    ( (bpb)->bpb32_total_sect32 -  \
                                      FAT32_START_DATA_SECTOR(bpb) )
#define FAT32_TOTAL_CLUS(bpb)       ( FAT32_TOTAL_DATA_SECTORS(bpp) / \
                                      (bpb)->bpb32_sects_per_clus )
typedef struct _fat32_fsinfo_t
{
    dword_t                 fsi_lead_sig;
    byte_t                  fsi_rsvd1[480];
    dword_t                 fsi_struct_sig;
    dword_t                 fsi_free_count;
    dword_t                 fsi_next_free;
    byte_t                  fsi_rsvd2[12];
    dword_t                 fsi_trail_sig;
}fat32_fsinfo_t;

#define FSI_LEAD_SIG                0x41615252
#define FSI_STRUCT_SIG              0x61417272
#define FSI_TRAIL_SIG               0xAA550000

#define FSI_IS_VALID(fsi)           (FSI_LEAD_SIG  != (fsi)->fsi_lead_sig  ||\
                                     FSI_STRUCT_SIG!= (fsi)->fsi_struct_sig||\
                                     FSI_TRAIL_SIG != (fsi)->fsi_trail_sig  )
typedef struct _fat_directory_t
{
    byte_t                  fdir_name[11];
    byte_t                  fdir_attr;
    byte_t                  fdir_nt_rsvd;
    byte_t                  fdir_cct;       /* create_time_tench;   */
    stime_t                 fdir_ct;        /* create_time; */
    sdate_t                 fdir_cd;        /* create_date; */
    sdate_t                 fdir_lad;       /* last_access_date;    */
    uint16_t                fdir_fch;       /* first_clus_high; */
    stime_t                 fdir_lwt;       /* last_write_time; */
    sdate_t                 fdir_lwd;       /* last_write_date; */
    uint16_t                fdir_fcl;       /* first_clus_low;  */
    uint32_t                fdir_fsize;     /* file size    */
}fat_dir_t;

typedef struct _fat_dir_buffer_t
{
    int                     fdb_start;
    fat_dir_t               fdb_buffer[11];
}fdb_t;

#define FATDIR_CREAT_TIME(dir,lt)   do{}while(0)
#define FATDIR_WRITE_TIME(dir,lt)   do{}while(0)
#define FATDIR_LAST_ACCESS_TIME(dir,lt) \
                                    do{}while(0)
#define FATDIR_FIRST_CLUSTER(dir)   ((dir)->fdir_fch*0x10000+(dir)->fdir_fcl)


typedef struct _fat_directory_long_name_entry_t
{
    byte_t                  fdl_order;
    word_t                  fdl_name1[5];
    byte_t                  fdl_attr;
    byte_t                  fdl_type;
    byte_t                  fdl_chk_sum;
    word_t                  fdl_name2[6];
    uint16_t                fdl_fcl;
    word_t                  fdl_name3[2];
}fdir_lnentry_t;

#pragma pack()

#define FATDIR_ATTR_READ_ONLY       0x01
#define FATDIR_ATTR_HIDDEN          0x02
#define FATDIR_ATTR_SYSTEM          0x04
#define FATDIR_ATTR_VOLUME_ID       0x08
#define FATDIR_ATTR_DIRECTORY       0x10
#define FATDIR_ATTR_ARCHIVE         0x20
#define FATDIR_ATTR_LONG_NAME       0x0F
#define FATDIR_ATTR_LONG_NAME_MASK  0x3F
#define FATDIR_LAST_LONG_ENTRY      0x40

#define FATDIR_IS_EMPTY(fdir)       (((fdir)->fidr_name[0] == 0) || \
                                     ((fdir)->fidr_name[0] == 0xE5))
#define FATDIR_IS_END(fidr)         ( (fdir)->fidr_name[0] == 0 )

#define FATDIR_LONG_NAME_MASK(dir)  ((dir)->fdir_attr & \
                                     FATDIR_ATTR_LONG_NAME_MASK)
#define FATDIR_IS_LONG_NAME_DIR(dir) \
                                    (FATDIR_ATTR_LONG_NAME == \
                                     FATDIR_LONG_NAME_MASK(dir))
typedef struct _fat_t
{
    /*  分区基本参数    */
    uint64_t                fat_start_sector;       /*  分区起始扇区    */
    uint64_t                fat_total_sectors;      /*  分区大小        */
    uint32_t                fat_total_clus;         /*  总簇数          */
    uint32_t                fat_free_clus;          /*  可用簇数量      */
    uint32_t                fat_next_free;          /*  下一个可用簇    */
    uint32_t                fat_root_clus;          /*  根目录起始簇    */  
    uint32_t                fat_start_data_sector;  /*  相对起始数据扇区*/
    uint32_t                fat_total_data_sectors; /*  相对总数据扇区数*/


    /*  FAT参数         */
    uint_t                  fat_bytes_per_sect;     /*  每扇区字节数    */
    uint_t                  fat_sects_per_clus;     /*  每簇扇区数      */
    uint_t                  fat_clus_size;          /*  簇大小，字节    */
    uint_t                  fat_rsvd_sectors;       /*  保留扇区数      */
    uint_t                  fat_num_of_fat;         /*  FAT个数         */
    uint_t                  fat_fat_size;           /*  FAT占用扇区数   */
    uint_t                  fat_root_dir_sectors;   /*  根目录扇区数，
                                                     *  仅对FAT16有效
                                                     */
}fat_t;

/*  指针在文件中簇号，注意：这是在文件中的编号  */
#define FAT_CLUS_NUM(fptr,fat)      ((fptr) / (fat)->fat_clus_size)
/*  文件指针在簇内的偏移                        */
#define FAT_CLUS_OFFSET(fptr,fat)   ((fptr) % (fat)->fat_clus_size)
#define FAT_CLUS_SECT_NUM(fptr,fat) (FAT_CLUS_NUM(fptr,fat) / 512)
#define FAT_CLUS_SECT_OFFSET(fptr,fat)  \
                                    (FAT_CLUS_NUM(fptr,fat) % 512)

#define FAT32_FAT_FIRST(fat)        ( (fat)->fat_start_sector + \
                                      (fat)->fat_rsvd_sectors )
/*  分区内扇区号转换为硬盘扇区号        */
#define FAT_TO_HD_ADDR(fat,sect)    ((fat)->fat_start_sector + (sect))
/*  簇号转换为分区内扇区号              */
#define FAT32_CLUS_FIRST_SECT(fat,clus) \
                                    (((clus)-2)*(fat)->fat_sects_per_clus + \
                                     (fat)->fat_start_data_sector)
/*  簇号转换为硬盘扇区号                */
#define FAT32_CLUS_TO_SECT(fat,clus) \
                                    (FAT_TO_HD_ADDR(fat, \
                                     FAT32_CLUS_FIRST_SECT(fat,clus)))
/*  簇号在文件分配表的位置，用扇区表示  */
#define FAT32_THIS_SECT_NUM(fat,clus) \
                                    ((fat)->fat_rsvd_sectors + \
                                     ((clus) * 4) / 512 )
/*  簇号在文件分派表的位置，扇区内偏移  */
#define FAT32_THIS_SECT_OFFSET(clus) \
                                    (((clus) * 4 ) & 511 )
/*  簇号在硬盘中的扇区号                */
#define FAT32_TSC_TO_HDA(fat,clus)  (FAT_TO_HD_ADDR((fat),\
                                     FAT32_THIS_SECT_NUM((fat),(clus))))

#define FAT32_DIRECTORY_SIZE        (sizeof(fat_dir_t))
#define FATDIR_MAX_DIRCTORYS        11

#define FAT32_OFFSET_TO_SECT(fat,offset)    \
                                    (((offset)%(fat)->fat_clus_size)/ \
                                     (fat)->fat_bytes_per_sect )
#define FAT32_OFFSET_TO_DIR_IDX(fat,offset) \
                                    (((offset)%(fat)->fat_bytes_per_sect)/ \
                                      FAT32_DIRECTORY_SIZE)

#define FAT32_DIRECTORYS_PER_SECTOR(fat)  \
                                    ((fat)->fat_bytes_per_sect / \
                                          FAT32_DIRECTORY_SIZE)

#define FAT32_DIR_FIND_FILE             0x00000001

#define FAT_END_OF_CLUS_12              0x0FF8
#define FAT_END_OF_CLUS_16              0xFFF8
#define FAT32_END_OF_CLUS               0x0FFFFFF8    

#define FAT_BAD_CLUS_16                 0xFFF7
#define FAT_BAD_CLUS_32                 0x0FFFFFF7

#define FAT_CLEAN_SHUT_16               0x8000
#define FAT_HD_ERR_16                   0x4000
#define FAT_CLEAN_SHUT_32               0x08000000
#define FAT_HD_ERR_32                   0x04000000

#define FAT_DIR_MAX_SIZE                (64*K*32)


result_t    Fat32_register(fsdrv_t * fsdrv);

#endif  /*  _FS_FAT32_H_    */