/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: fat.h
//  创建时间: 2014-06-17        创建者: 罗斌
//  修改时间:                   修改者: 
//  主要功能: 测试一些基本的功能
//  说    明: 
//
//  命名规则: 
//  变更记录:
//  版 本 号    |     时间      |  作  者       |  主要变化记录
//=============================================================================
//  00.00.000   |  2014-06-17   |  罗  斌       |  创建文件
///////////////////////////////////////////////////////////////////////////////
*/



#ifndef _FAT_H_
#define _FAT_H_

typedef char                int8_t;
typedef short               int16_t;
typedef long                int32_t;
typedef __int64             int64_t;

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned long       uint32_t;
typedef unsigned __int64    uint64_t;

typedef unsigned char       byte_t;
typedef unsigned short      word_t;
typedef unsigned long       dword_t;
typedef unsigned __int64    qword_t;


#define RESULT_NO_ERR       (0)
#define RESULT_SUCCEED      (0)
#define RESULT_FAILED       (-1)

/*  短格式的时间类型    */
typedef struct _short_date_t
{
    word_t                  sd_day:5;
    word_t                  sd_month:4;
    word_t                  sd_year:7;          /*  1980至2107年       */
}sdate_t;

typedef struct _short_time_t
{
    word_t                  st_second:5;        /*  精度为2秒           */
    word_t                  st_minute:6;
    word_t                  st_hour:5;
}stime_t;


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



#endif  /*  _FAT_H_ */