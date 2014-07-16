
//2014.02.20


#ifndef _FS_H_
#define _FS_H_

#include <config.h>
#include <type.h>

#include <device.h>

#define FS_FLAGS_DEVICE
#pragma pack(1)
/*  �������ʽ  */
typedef struct _partition_t
{
    byte_t                  part_flag;              /* ������־         */
    byte_t                  part_s_head;    
    word_t                  part_s_sector:6;    
    word_t                  part_s_cylinder:10;
    byte_t                  part_type;              /* ��������         */
    byte_t                  part_e_head;
    word_t                  part_e_sector:6;
    word_t                  part_e_cylinder:10;
    dword_t                 part_rel_start;         /* ���������ʼ���� */
    dword_t                 part_total_sector;      /* ����������       */
}partition_t;

#pragma pack()

#define FS_PART_FALG_ACTIVE         0x80

/*
 *  ��������
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
    device_t            *   fsp_dev;        /*  ���������豸    */
    uint_t                  fsp_type;       /*  ��������        */
    qword_t                 fsp_start;      /*  ������ʼλ��    */
    qword_t                 fsp_total;      /*  ������������    */
}fsp_t;

/*
 *  �ļ�ϵͳ��ƣ�
 *    1.�����ļ�ϵͳ
 *    2.��ʽ�ļ���֯��ʽ
 *    3.�ļ�����������Ϣ
 *
 *  ÿ���ļ��ڵ����һ���ļ�����Ŀ¼�ļ�����ͨ�ļ�
 */
typedef struct _file_node_t
{
    koh_t                   koh;
    /*
     *  λ����Ϣ: 
     *    �����ļ�ϵͳ��Ҳ���Ƿ������ļ�����ʼλ�ã����䵥Ԫ���صȣ����ļ��ڸ�
     *  Ŀ¼�ļ��еľ���λ�ã����ݷ��ʿ�����Ϣ�����Ŀ¼�ļ��ڵ��ļ�ϵͳ��
     */
    void                *   fn_fs;                  /*  �����ļ�ϵͳ    */
    void                *   fn_mount;               /*  �ҽӵ��ļ�ϵͳ  */
    dword_t                 fn_fln;                 /*  �ļ����ӽڵ��ַ*/
    dword_t                 fn_pd_fln;              /*  ��Ŀ¼��ʼ��Ԫ  */
    uint32_t                fn_pd_offset;           /*  ��Ŀ¼��ƫ��    */

    /*
     *  ���ʿ�����Ϣ:
     *    �ļ����ȡ��ɶ�����д��Ŀ¼�ļ������ء��豸�ļ�����
     */
    uint64_t                fn_size;                /*  �ļ���С        */
    dword_t                 fn_attr;                /*  �ļ�����        */

    /*
     *  ��ȫ��Ϣ
     */
    int                     fn_refcnt;              /*  ���ü�����      */

    /*
     *  ����������Ϣ
     *    ����ʱ��. create date,create time
     *    ����޸�ʱ��.last write date,last write time
     *    ����������.last access date,
     */
    date_t                  fn_cd;                  /*  ����ʱ��    */
    time_t                  fn_ct;
    date_t                  fn_lwd;                 /*  ����޸�ʱ��*/
    time_t                  fn_lwt;
    date_t                  fn_lad;                 /*  ����������    */
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
 *  �ļ�ϵͳ���ܣ�����������ǹ�����̿ռ䣬��������ļ��ĸ���󣬾͵�ͬ���ļ�
 *  �������ڹ����ļ�ϵͳ��������Ҫ�������ҵ��κ��ļ����ܲ����κ��ļ������ҵ�
 *  �ļ���򵥵ķ�ʽ���ļ�ϵͳ�����ṩһ�ֱ����ļ�ϵͳ�ķ�ʽ��ͨ������Ƚϵķ�
 *  ���Ϳ����ҵ������ļ����ڸ߼�һ���ϵͳ���԰���������Ϣ�������������������
 *  �����ƶ����Ƶ��ļ���Ч�ʡ��ܲ����ļ�������������ļ��Ķ�д������Ŀ¼�ļ���
 *  ��Ҫ���Դ���Ŀ¼��ɾ��Ŀ¼����Ŀ¼������Ŀ¼���ԡ���Ҫ�Է������г�ʼ����Ҳ
 *  ���Ǹ�ʽ����
 *    1.�����ļ�: ����д��
 *    2.����Ŀ¼: ������ɾ����������������
 *    3.��������: ��Ŀ¼�в��ҡ���õ�һ��Ŀ¼�������һ��Ŀ¼
      4.��ʼ��  : ��ʽ��
 */
typedef struct _fs_driver_t
{
    uint_t                  fs_type;
    result_t           (*   fs_initial)     (struct _file_system_t * fs);

    /*
     *  �ļ�����������д
     */
    int                (*   fs_fnode_read)  (const fnode_t* fnode,
                                             uint64_t       offset,
                                             void         * buffer,
                                             int            size);
    /*
    size_t             (*   fs_file_write)();
    */
    /*
     *  ������������Ŀ¼�в��ҡ����ҵ�һ��Ŀ¼��������һ��Ŀ¼
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
     *  Ŀ¼������������ɾ����������������
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

    /*  �ļ������ӿ�    
    fnode_t            (*   fs_file_open)();
    size_t             (*   fs_file_seek)();*/
}fsdrv_t;

#define FS_CREATE_ATTRS_DIRECTORY   0x00000001
#define FS_CREATE_ATTRS_READ        FS_OPEN_FLAG_READ
#define FS_CREATE_ATTRS_WRITE       FS_OPEN_FLAG_WRITE

typedef struct _file_system_t
{
    fsp_t                   fs_part;                /*  �ļ�ϵͳ���ڷ���    */
    fnode_t                 fs_root;                /*  ���ļ��ڵ�          */
    fsdrv_t             *   fs_drv;                 /*  �ļ�ϵͳ��������    */
    byte_t                  fs_param_blk[512];      /*  ����������          */

    int                     fs_ref;
    lock_t                  fs_lock;

}fs_t;

/*  �ļ�������� */
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
/*  �ļ���Ϣ�������ļ���ʹ��    */
typedef struct _file_information_t
{
    char                    fi_name[256];
    const char          *   fi_fliter;      /*  �����ַ�����* ? */
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
 *  1��֧�ֶ������
 *  ͳһ�ļ�ϵͳ
 */
typedef struct _unify_file_system_t
{
    fnode_t             *   ufs_root;
    qword_t                 ufs_size;   /*  ����    */
}ufs_t;

void        Fs_initial(void);
/*  ·������API */

/*  �ļ�����API */

/*
 *  ���ļ��ı�־���ɶ�����д�����򿪴��ڵ��ļ�
 *  �ļ��ı�־��Դ�ڿ����ߵ�Ҫ��
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
//  �ļ�ϵͳAPI
*/
handle_t    Fs_open(const char * path,dword_t flags);
int         Fs_read(handle_t file,void * buffer,int size);
uint_t      Fs_write(handle_t file,const void * buffer,uint_t size);
uint_t      Fs_seek(handle_t file,dword_t offset,int origin);
uint_t      Fs_ioctrl(handle_t file,int cmd,void * buffer);

/*  ����Ŀ¼*/
handle_t    Fs_find_first(const char * path,finfo_t * finfo);
result_t    Fs_find_next(handle_t directory,finfo_t * finfo);
#endif  /*  _FS_H_  */