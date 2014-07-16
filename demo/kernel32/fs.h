
//2014.02.20


#ifndef _FS_H_
#define _FS_H_

#include <config.h>
#include <type.h>

#include <device.h>

#define FS_FLAGS_DEVICE
#pragma pack(1)
/*  分区表格式  */
typedef struct _partition_t
{
    byte_t                  part_flag;              /* 分区标志         */
    byte_t                  part_s_head;    
    word_t                  part_s_sector:6;    
    word_t                  part_s_cylinder:10;
    byte_t                  part_type;              /* 分区类型         */
    byte_t                  part_e_head;
    word_t                  part_e_sector:6;
    word_t                  part_e_cylinder:10;
    dword_t                 part_rel_start;         /* 分区相对起始扇区 */
    dword_t                 part_total_sector;      /* 分区扇区数       */
}partition_t;

#pragma pack()

#define FS_PART_FALG_ACTIVE         0x80

/*
 *  分区类型
 */
#define FS_PART_TYPE_UNDEF          0x00
#define FS_PART_TYPE_FAT12          0x01
#define FS_PART_TYPE_NTFS           0x07
#define FS_PART_TYPE_FAT32          0x0B
#define FS_PART_TYPE_FAT16          0x0E
#define FS_PART_TYPE_FAT32_EXT      0x0F
#define FS_PART_TYPE_SFS            0x43

typedef struct _fs_partition_t
{
    device_t            *   fsp_dev;        /*  分区所在设备    */
    uint_t                  fsp_type;       /*  分区类型        */
    qword_t                 fsp_start;      /*  分区起始位置    */
    qword_t                 fsp_total;      /*  分区扇区总数    */
}fsp_t;

/*
 *  文件系统设计：
 *    1.树形文件系统
 *    2.链式文件组织形式
 *    3.文件包含控制信息
 *
 *  每个文件节点代表一个文件，含目录文件和普通文件
 */
typedef struct _file_node_t
{
    koh_t                   koh;
    /*
     *  位置信息: 
     *    所在文件系统，也就是分区，文件的起始位置（分配单元、簇等），文件在父
     *  目录文件中的具体位置（兼容访问控制信息存放在目录文件内的文件系统）
     */
    void                *   fn_fs;                  /*  所在文件系统    */
    void                *   fn_mount;               /*  挂接的文件系统  */
    dword_t                 fn_fln;                 /*  文件链接节点地址*/
    dword_t                 fn_pd_fln;              /*  父目录起始单元  */
    uint32_t                fn_pd_offset;           /*  父目录内偏移    */

    /*
     *  访问控制信息:
     *    文件长度、可读、可写、目录文件、隐藏、设备文件属性
     */
    uint64_t                fn_size;                /*  文件大小        */
    dword_t                 fn_attr;                /*  文件属性        */

    /*
     *  安全信息
     */
    int                     fn_refcnt;              /*  引用技术器      */

    /*
     *  其他辅助信息
     *    创建时间. create date,create time
     *    最后修改时间.last write date,last write time
     *    最后访问日期.last access date,
     */
    date_t                  fn_cd;                  /*  创建时间    */
    time_t                  fn_ct;
    date_t                  fn_lwd;                 /*  最后修改时间*/
    time_t                  fn_lwt;
    date_t                  fn_lad;                 /*  最后访问日期    */
}fnode_t;

#define FNODE_ATTR_DIRECTORY        0x00000001
#define FNODE_ATTR_READ             FS_OPEN_FLAG_READ
#define FNODE_ATTR_WRITE            FS_OPEN_FLAG_WRITE
#define FNODE_ATTR_HIDDEN           0x00000008
#define FNODE_ATTR_DEVICE           0x00000010

#define FNODE_GET_FS(fnode)         ((fs_t *)((fnode)->fn_fs ? \
                                     (fnode)->fn_fs:(fnode)->fn_mount))
#define FNODE_GET_FLN(fnode)        ((fnode)->fn_fln)
#define FNODE_INC_REF_COUNT(fnode)  do{ ++((fnode)->fn_refcnt); }while(0)
#define FNODE_IS_DIRECTORY(fnode)   ( (fnode)->fn_attr & FNODE_ATTR_DIRECTORY)

/*
 *  文件系统功能：其根本功能是管理磁盘空间，但是提出文件的概念后，就等同于文件
 *  管理。对于管理文件系统，基本的要求是能找到任何文件，能操作任何文件。能找到
 *  文件最简单的方式是文件系统驱动提供一种遍历文件系统的方式，通过逐个比较的方
 *  法就可以找到所有文件，在高级一点的系统可以包含查找信息，例如索引，可以提高
 *  查找制定名称的文件的效率。能操作文件，最基本的是文件的读写，对于目录文件，
 *  还要可以创建目录、删除目录、改目录名、改目录属性。还要对分区进行初始化，也
 *  就是格式化。
 *    1.操作文件: 读、写、
 *    2.操作目录: 创建、删除、改名、改属性
 *    3.遍历方法: 在目录中查找、获得第一个目录、获得下一个目录
      4.初始化  : 格式化
 */
typedef struct _fs_driver_t
{
    uint_t                  fs_type;
    result_t           (*   fs_initial)     (struct _file_system_t * fs);

    /*
     *  文件操作：读、写
     */
    int                (*   fs_fnode_read)  (const fnode_t* fnode,
                                             uint64_t       offset,
                                             void         * buffer,
                                             int            size);
    /*
    size_t             (*   fs_file_write)();
    */
    /*
     *  遍历操作：在目录中查找、查找第一个目录、查找下一个目录
     */
    result_t           (*   fs_fnode_find)  (const fnode_t* parents,
                                             const char   * name,
                                             dword_t        flags,
                                             fnode_t      * fnode);
    /*
    result_t           (*   fs_fnode_first) (fnode_t      * parents,
                                             fnode_t     ** fnode);
    result_t           (*   fs_fnode_next)  (fnode_t      * parents,
                                             fnode_t     ** fnode);
    result_t           (*   fs_fnode_parents)(fnode_t     * fnode,
                                             fnode_t      * parents);
    */
    /*
     *  目录操作：创建、删除、改名、改属性
     */
    result_t           (*   fs_fnode_create)(const fnode_t* parents,
                                             const char   * name,
                                             dword_t        attrs,
                                             fnode_t      * fnode);
    result_t           (*   fs_fnode_update)(const fnode_t* fnode);

    /*      * /
    result_t           (*   fs_fnode_del)   (fnode_t      * parents,
                                             const char   * name,
                                             dword_t        flags);
    result_t           (*   fs_fnode_rename)(fnode_t      * parents,
                                             const char   * name);
    result_t           (*   fs_fnode_attrs) (fnode_t      * parents,
                                             dword_t        attrs);

    /*  文件操作接口    
    fnode_t            (*   fs_file_open)();
    size_t             (*   fs_file_seek)();*/
}fsdrv_t;

#define FS_CREATE_ATTRS_DIRECTORY   0x00000001
#define FS_CREATE_ATTRS_READ        FS_OPEN_FLAG_READ
#define FS_CREATE_ATTRS_WRITE       FS_OPEN_FLAG_WRITE

typedef struct _file_system_t
{
    fsp_t                   fs_part;                /*  文件系统所在分区    */
    fnode_t                 fs_root;                /*  根文件节点          */
    fsdrv_t             *   fs_drv;                 /*  文件系统驱动对象    */
    byte_t                  fs_param_blk[512];      /*  分区参数块          */

    int                     fs_ref;
    lock_t                  fs_lock;

}fs_t;

/*  文件对象参数 */
typedef struct _file_t
{
    kot_t                   koh;
    dword_t                 f_attr;
    void                *   f_object;
    offset_t                f_ptr;
}file_t;

#define FILE_ATTR_DEVICE            0x00000001
#define FILE_ATTR_READ              0x00000002
#define FILE_ATTR_WRITE             0x00000004

#define FILE_ATTR_RDWR              (FILE_ATTR_READ|FILE_ATTR_WRITE)

#define FILE_FNODE(file)            ((fnode_t *)((file_t *)(file)->f_object))
/*  文件信息，遍历文件夹使用    */
typedef struct _file_information_t
{
    char                    fi_name[256];
    const char          *   fi_fliter;      /*  过滤字符串，* ? */
    dword_t                 fi_attr;
    qword_t                 fi_size;
    date_t                  fi_cd;
    time_t                  fi_ct;
    date_t                  fi_lwd;
    time_t                  fi_lwt;
}finfo_t;

#define FS_START_SECTOR(fs)         ((fs)->fs_part.fsp_start)
#define FS_TOTAL_SECTORS(fs)        ((fs)->fs_part.fsp_total)
#define FS_PARAM_BLK(fs)            ((void *)((fs)->fs_param_blk))
#define FS_DEVICE(fs)               ((fs)->fs_part.fsp_dev)
#define FS_DRV(fs)                  ((fs)->fs_drv)

#define FS_LOCK(fs)                 Lck_lock(&fs->fs_lock)
#define FS_FREE(fs)                 Lck_free(&fs->fs_lock)
/*
 *  1、支持多个分区
 *  统一文件系统
 */
typedef struct _unify_file_system_t
{
    fnode_t             *   ufs_root;
    qword_t                 ufs_size;   /*  容量    */
}ufs_t;

void        Fs_initial(void);
/*  路径操作API */

/*  文件操作API */

/*
 *  打开文件的标志，可读、可写、仅打开存在的文件
 *  文件的标志来源于开发者的要求
 */
#define FS_OPEN_FLAG_EXIST          0x00000001
#define FS_OPEN_FLAG_READ           0x00000002
#define FS_OPEN_FLAG_WRITE          0x00000004
#define FS_OPEN_FLAG_DIRECTORY      0x00000100
#define FS_OPEN_FLAG_SHARE          0x00000010

#define FS_OPEN_FLAG_RDWR_MASK      (FS_OPEN_FLAG_READ|FS_OPEN_FLAG_WRITE)

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  文件系统API
*/
handle_t    Fs_open(const char * path,dword_t flags);
int         Fs_read(handle_t file,void * buffer,int size);
uint_t      Fs_write(handle_t file,const void * buffer,uint_t size);
uint_t      Fs_seek(handle_t file,dword_t offset,int origin);
uint_t      Fs_ioctrl(handle_t file,int cmd,void * buffer);

/*  遍历目录*/
handle_t    Fs_find_first(const char * path,finfo_t * finfo);
result_t    Fs_find_next(handle_t directory,finfo_t * finfo);
#endif  /*  _FS_H_  */