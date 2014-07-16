

#include <config.h>
#include <assert.h>

#include <device.h>
#include <blkbuf.h>
#include <lio.h>
#include <koum.h>

#include "fs.h"
#include "fs_fat.h"

#define NR_INODE
/*ϵͳ����·��
/.device/  ת�����ϵͳ�豸��
/.lhd/ ת������ڵ��б�
*/

/*  ϵͳ����򿪵�����ļ���  */
//fnode_t     fs_fnode_pool[NR_FNODE];
//fnode_t     * fs_root;  /*  ϵͳ���ļ�Ŀ¼��Ҳ�����������ĸ�Ŀ¼  */
/*  ����-·��ӳ�������ͨ���������windows��·����ʾ����*/
#define FS_PART_MAX                 16
#define FS_DRV_MAX                  4
#define FS_FNDOE_MAX                64
#define FS_FILE_MAX                 32

static int                  fs_cnt;
static int                  fs_active;
static fs_t                 fs_pool[FS_PART_MAX];
static int                  fs_drv_cnt;
static fsdrv_t              fs_drv_pool[FS_DRV_MAX];
static fnode_t          *   fs_root;
static fnode_t              fs_fnode_pool[FS_FNDOE_MAX];
static file_t               fs_file_pool[FS_FILE_MAX];

#ifdef _CFG_SMP_
static spin_lock_t          fs_fnode_lock;
static spin_lock_t          fs_file_lock;
#endif  /*  _CFG_SMP_   */

static
void        Fs_fnode_information(fnode_t * fnode)
{
    _printk("fs: %08X mount: %08X\n",fnode->fn_fs,fnode->fn_mount);
    _printk("attr: %08X   ",fnode->fn_attr);
    _printk("file size: %ld\n",fnode->fn_size);
}
static
void        Fs_partition_infomation(partition_t * part)
{
    _printk("flag=%02X type=%02X start=%8d total=%8d\n",
        part->part_flag,part->part_type,
        part->part_rel_start,part->part_total_sector);
}

static
void        Fs_information(void)
{
    int             i;

    _printk("total file system(partition): %d\n",fs_cnt);
    _printk("active file system: %d\n",fs_active);
    _printk("total file system driver: %d\n",fs_drv_cnt);
    _printk("partition information:\n");
    for( i = 0  ; i < fs_cnt ; i++)
    {
        _printk("    type=%02X,start=%-10ld,total=%-10ld \n",
            fs_pool[i].fs_part.fsp_type,
            fs_pool[i].fs_part.fsp_start,
            fs_pool[i].fs_part.fsp_total);
    }
}

/*  2014-03-10 ����fat32��չ����*/
static
void        Fs_fat32_ext(device_t * blkdev,dword_t start)
{
    blkbuf_t      * blkbuf      = NULL;
    partition_t   * part        = NULL;
    fsp_t         * fs_part     = NULL;
    int             i           = 0;

    if( NULL == ( blkbuf = Bbuf_read(blkdev,start) ) )
        return ;
    part = (partition_t *)((byte_t *)BBUF_TO_SECTOR(blkbuf,start) + 0x1BE );
    for( ; i < 4 ; i++ )
    {
        if( part[i].part_type == FS_PART_TYPE_UNDEF )
            continue;
        if( part[i].part_type == FS_PART_TYPE_FAT32_EXT )
            Fs_fat32_ext(blkdev,start + part[i].part_rel_start);
        else
        {
            fs_part = (fsp_t *)(fs_pool + fs_cnt++);
            fs_part->fsp_dev    = blkdev;
            fs_part->fsp_type   = part[i].part_type;
            fs_part->fsp_start  = part[i].part_rel_start + start;
            fs_part->fsp_total  = part[i].part_total_sector;
        }
    }
    Bbuf_release(blkbuf);
}

/*  2014-03-11 ̽��ϵͳ�д��ڵķ���*/
static
void        Fs_detect_partition(void)
{
    device_t      * blkdev      = NULL;
    blkbuf_t      * blkbuf      = NULL;
    partition_t   * part        = NULL;
    fsp_t         * fs_part     = NULL;
    int             i           = 0;

    /*  ע���ļ�ϵͳ���� */

    /*  ��ʼ���ڼ䣬�������������̣���˲���Ҫ����ϵͳ�豸�б�  */
    blkdev = Dev_sdl_first();
    for( blkdev = Dev_sdl_first() ; blkdev ; blkdev = Dev_sdl_next(blkdev) )
    {
        if( !( blkdev->dev_flag & DEV_FLAG_BLOCK ) )
            continue;
        if( NULL == ( blkbuf  = Bbuf_read(blkdev,0) ) )
            continue;
        part    = (partition_t *)((byte_t *)blkbuf->bb_buffer + 0x1BE );
        for( i = 0 ; i < 4 && fs_cnt < FS_PART_MAX; i++)
        {
            if( fs_active == -1 && part[i].part_flag == FS_PART_FALG_ACTIVE)
                fs_active = fs_cnt;
            if( part[i].part_type == FS_PART_TYPE_UNDEF )
                continue;
            /*  ����FAT32��չ����Ҫ���⴦�� */
            if( part[i].part_type == FS_PART_TYPE_FAT32_EXT )
                Fs_fat32_ext(blkdev,part[i].part_rel_start);
            else
            {
                fs_part = (fsp_t *)(fs_pool + fs_cnt++);
                fs_part->fsp_dev    = blkdev;
                fs_part->fsp_type   = part[i].part_type;
                fs_part->fsp_start  = part[i].part_rel_start;
                fs_part->fsp_total  = part[i].part_total_sector;
            }
        }
        Bbuf_release(blkbuf);
    }
}

/*  �ļ�ϵͳ�����������ע�ᡢ���ҡ�����*/
static
fsdrv_t *   Fs_get_driver(uint_t type )
{
    int             i = 0;

    for( i = 0 ; i < FS_DRV_MAX ; i++)
        if( type == fs_drv_pool[i].fs_type )
            return fs_drv_pool + i;
    
    return NULL;
}
/*  2014-03-11 ָ����ʼ��*/
static
void        Fs_specify(void)
{
    int             i;
    for( i = 0 ; i < fs_cnt ; i++)
    {
        fs_pool[i].fs_drv = Fs_get_driver(fs_pool[i].fs_part.fsp_type);
        if( NULL != fs_pool[i].fs_drv )
            fs_pool[i].fs_drv->fs_initial(fs_pool + i);
    }
}

/*  2014-03-11  �ļ�ϵͳ����ע��  */
result_t    Fs_register(result_t (* fsreg)(fsdrv_t * fsdrv))
{
    int             i;

    for( i = 0 ; i < FS_DRV_MAX ; i++)
        if( FS_PART_TYPE_UNDEF == fs_drv_pool[i].fs_type )
            break;
    if( i >= FS_DRV_MAX )
        return RESULT_FAILED;
    if( fsreg(fs_drv_pool + i) == RESULT_FAILED )
        return RESULT_FAILED;   
    fs_drv_cnt++;
    ASSERT(FS_PART_TYPE_UNDEF != fs_drv_pool[i].fs_type);
    return RESULT_SUCCEED;
}

#define FS_IS_PREFIX(c)             ( (c) == ' ' || (c) == '/' )
#define FS_TRIM_PREFIX(p)           do{ while(*(p) && FS_IS_PREFIX(*(p))) \
                                      (p)++;}while(0);
#define FS_TRIM_SPACE(p)            do{ while(*(p) && *(p) == ' ') (p)++; \
                                      }while(0)
#define FS_TRIM_SEPARATOR(p)        do{ while(*(p) && *(p) == '/') (p)++; \
                                      }while(0)

static
const char* Fs_get_subpath(const char * path,char * subpath)
{
    const char * p          = path;
    char       * sp         = subpath;
    char       * lastspace  = NULL;
    
    FS_TRIM_SEPARATOR(p);
    FS_TRIM_SPACE(p);
    while( *p && *p != '/' )
    {
        if( *p == ' ' )
        {
            if( lastspace == NULL )
                lastspace = sp;
        }
        else
            lastspace = NULL;
        *sp++ = *p++;
    }
    *sp = 0;
    if( lastspace )
        *lastspace = 0;
    
    return 0 == *p ? NULL : p;
}

static
int         Fs_is_device_path(const char * path)
{
    return 0;
}

int         _namecmpn(const char * name1,const char * name2,int n)
{
    while( n )
    {
        if( _up_case(*name1) != _up_case(*name2 ) )
            return *name1 - *name2;
        name1++;
        name2++;
        n--;
    }
    return 0;
}

static
fnode_t *   Fs_alloc_fnode(void)
{
    int             i       = 0;
    fnode_t       * fnode   = fs_fnode_pool;
    CRITICAL_DECLARE(fs_fnode_lock);

    CRITICAL_BEGIN();
    for( i = 0 ; i < FS_FNDOE_MAX ; i++,fnode++)
    {
        if( NULL == fnode->fn_fs && NULL == fnode->fn_mount )
        {
            fnode->fn_fs        = (fs_t *)-1;
            fnode->fn_mount     = NULL;
            fnode->fn_refcnt    = 1; /*  �������������*/
            fnode->fn_attr      = 0;
            break;
        }
    }
    CRITICAL_END();

    return i < FS_FNDOE_MAX ? fnode : NULL;
}

static
void        Fs_free_fnode(fnode_t * fnode)
{
    CRITICAL_DECLARE(fs_fnode_lock);

    ASSERT( fnode );
    ASSERT( fnode->fn_refcnt > 0 );
    CRITICAL_BEGIN();
    /*  û�����ú������ͷ�  */
    if( --fnode->fn_refcnt == 0 )
        _memzero(fnode,sizeof(fnode_t));
    CRITICAL_END();
}

/*  2014-03-24 */
static
file_t *    Fs_alloc_file(void)
{
    int             i = 0;
    CRITICAL_DECLARE(fs_file_lock);

    CRITICAL_BEGIN();
    for( i = 0 ; i < FS_FILE_MAX ; i++)
    {
        if( fs_file_pool[i].f_object == NULL )
        {
            fs_file_pool[i].f_object = (void *)-1;
            break;
        }
    }
    CRITICAL_END();

    return i < FS_FILE_MAX ? fs_file_pool + i : NULL ;
}

/*  2014-03-24 */
static
void        Fs_free_file(file_t * file)
{
    CRITICAL_DECLARE(fs_file_lock);

    ASSERT(file);
    ASSERT(file->f_object);
    CRITICAL_BEGIN();
    _memzero(file,sizeof(file_t));
    CRITICAL_END();
}

/*  2014-03-24 */
static
result_t    Fs_release_file(file_t * file)
{
    void          * object  = NULL;

    ASSERT(file);
    if( NULL == file )
        return RESULT_FAILED;
    object = file->f_object;
    Fs_free_file(file);
    if( file->f_attr & FILE_ATTR_DEVICE )
        return Dev_close(file->f_object);
    else
        Fs_free_fnode(file->f_object);
    return RESULT_SUCCEED;
}

/*  �ж��ļ��ڵ��Ƿ��Ѿ���  */
static
fnode_t *   Fs_is_open(fnode_t * fnode)
{
    return NULL;
}

static
handle_t    Fs_open_device(const char * name,dword_t flags)
{
    file_t        * file        = NULL;
    device_t      * device      = NULL;
    handle_t        handle      = INVALID_HANDLE;
    int             mode        = 0;

    _printk("open device flags:%08X\n",flags);
    if( flags & FNODE_ATTR_READ )
        mode |= DEV_MODE_READ;
    if( flags & FNODE_ATTR_WRITE)
        mode |= DEV_MODE_WRITE;
    if( INVALID_HANDLE == ( device = Dev_open(name,mode) ) )
    {
        _printk("open device failed!\n");
        return INVALID_HANDLE;
    }
    if( NULL == ( file = Fs_alloc_file() ) )
    {
        Dev_close(device);
        return INVALID_HANDLE;
    }
    file->f_object  = device;
    file->f_ptr     = 0;
    file->f_attr    = FILE_ATTR_DEVICE;
    file->f_attr   |= flags & FS_OPEN_FLAG_RDWR_MASK;

    if( INVALID_HANDLE == (handle = Koum_add(file,Fs_release_file,kot_file,0)))
    {
        Dev_close(device);
        Fs_free_file(file);
        return INVALID_HANDLE;
    }

    return handle;
}


/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Fs_open
//  ��    ��: ���ļ���
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//    const char *  |   pathname    |   ·������
//     dword_t      |   flags       |   �򿪱�־
//           FS_OPEN_FLAG_EXIST     |   ���򿪴��ڵ��ļ����޴˱�־�򴴽����ļ�
//           FS_OPEN_FLAG_READ      |   �ļ��ɶ�
//           FS_OPEN_FLAG_WRITE     |   �ļ���д
//           FS_OPEN_FLAG_DIRECTORY |   ��Ŀ¼
//  �� �� ֵ: ���ط���صĸ�����
//  ע    ��: ��·����ʼ�����豸·��ǰ׺(/../dev/)����ת���豸���� 
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-14  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
handle_t    Fs_open(const char * pathname,dword_t flags)
{
    char            subpath[256];
    const char    * path    = pathname;
    fs_t          * fs      = NULL;
    fnode_t       * fnode   = NULL,
                  * parents = NULL;
    fsdrv_t       * fsdrv   = NULL;
    device_t      * device  = NULL;
    file_t        * file    = NULL;
    result_t        result  = RESULT_FAILED;
    handle_t        handle  = INVALID_HANDLE;

    FS_TRIM_SPACE(path);
    if( *path == '/' )
    {
        FS_TRIM_SEPARATOR(path);
        path--; /*  �������һ��'/'���� */
    }
    /*  ������豸·��ǰ׺��תΪ���豸 */
    if( _namecmpn(path,"/../dev/",8) == 0 )
        return Fs_open_device(path + 8,flags);
    /*
     *  ��·����������ʼĿ¼ 
     */
    if( *path == '/' )
        parents = fs_root;
    else
    {
        /*  ����������� * /
        if( path = Fs_is_symbol(path)  )
        {
            parents = Fs_symbol();
        }*/
        goto fs_open_file_end; 
        /* parents = current->proc_current_path;   Ӧ������Ŀ¼ */
    }

    //_printk("open file\n");
    /*  ��Ҫʹ��һ�������fnode�ռ�*/
    if( NULL == ( fnode = Fs_alloc_fnode() ) )
        goto fs_open_file_end;
    FNODE_INC_REF_COUNT(parents);
    /*  ����·���к�·��ĩβ��·������Ҫ�ҵ�Ŀ¼��·��ĩβ��Ҫ�ҵ��ļ� */
    while( path = Fs_get_subpath(path,subpath) )
    {
        fs = FNODE_GET_FS(parents);
        /*  ��Ҫʹ��һ�������fnode�ռ�*/
        if( NULL == ( fnode = Fs_alloc_fnode() ) )
            goto fs_open_file_end;
        /*  ���ļ�ϵͳ */
        FS_LOCK(fs);
        result = fs->fs_drv->fs_fnode_find(parents,subpath,flags,fnode);
        FS_FREE(fs);
        if( result != RESULT_SUCCEED )
            goto fs_open_file_end;
        /*  �ҵ������ֲ���Ŀ¼  */
        if( !FNODE_IS_DIRECTORY(fnode) )
        {
            Fs_free_fnode(fnode);
            goto fs_open_file_end;
        }
        Fs_free_fnode(parents);
        parents = fnode;
        //_printk("subpath: %s-\n",subpath);
    }

    //_printk("filename: %s-\n",subpath);
    if( NULL ==( fnode = Fs_alloc_fnode() ) )
        goto fs_open_file_end;
    fs = FNODE_GET_FS(parents);
    ASSERT(fs && fs->fs_drv);
    FS_LOCK(fs);
    result = fs->fs_drv->fs_fnode_find(parents,subpath,flags,fnode);
    FS_FREE(fs);
    if( result !=RESULT_SUCCEED)
    {
        _printk("file not found!\n");
        /*
         *  Ҫ��ֻ���Ѵ��ڵ��ļ�������£��Ҳ����ļ��򷵻�
         */
        if( flags & FS_OPEN_FLAG_EXIST )
        {
            Fs_free_fnode(fnode);
            goto fs_open_file_end;
        }
        FS_LOCK(fs);
        result = fs->fs_drv->fs_fnode_create(parents,subpath,flags,fnode);
        FS_FREE(fs);
        if( result != RESULT_SUCCEED)
        {
            _printk("create new file failed!\n");
            Fs_free_fnode(fnode);
            goto fs_open_file_end;
        }
        _printk("create new file OK!\n");
    }
    /*
     *  Ҫ���Ŀ¼�����ҵ��Ĳ���Ŀ¼������ʧ��
     *  û��Ҫ���Ŀ¼�����ҵ�����Ŀ¼������ʧ��
     */
    if((flags&FS_OPEN_FLAG_DIRECTORY&&!(fnode->fn_attr&FNODE_ATTR_DIRECTORY))||
        (!(flags&FS_OPEN_FLAG_DIRECTORY)&&fnode->fn_attr&FNODE_ATTR_DIRECTORY))
    {
        Fs_free_fnode(fnode);
        goto fs_open_file_end;
    }

    /*
     *  �����ļ����󣬷��䲻�ɹ��򷵻�ʧ�ܡ�
     */
    if( NULL == ( file = Fs_alloc_file() ) )
    {
        Fs_free_fnode(fnode);
        goto fs_open_file_end;
    }
    /*  ע���ļ��ڵ������������ļ������������������*/
    file->f_object  = fnode;
    file->f_ptr     = 0;
    file->f_attr    = FILE_ATTR_RDWR;
    file->f_attr   &= (~FS_OPEN_FLAG_RDWR_MASK) | \
                      (flags & FS_OPEN_FLAG_RDWR_MASK);
    fnode->fn_attr &= (~FS_OPEN_FLAG_RDWR_MASK) | \
                      (flags & FS_OPEN_FLAG_RDWR_MASK);
    Fs_fnode_information(fnode);
    /*
     *  �����ں�ͳһ������� 
     */
    if( INVALID_HANDLE == (handle = Koum_add(file,Fs_release_file,kot_file,0)))
    {
        Fs_free_fnode(fnode);
        Fs_free_file(file);
        _printk("KOUM add failed!\n");
        goto fs_open_file_end;
    }
    //_printk("file open OK!\n");
fs_open_file_end:
    Fs_free_fnode(parents);
    return handle;
}

#define FS_CHECK_FNODE(fnode)       ASSERT(((byte_t *)(fnode) -     \
                                       (byte_t *)fs_fnode_pool) %   \
                                       sizeof(fnode_t) == 0 &&      \
                                       (uint_t)(fnode) >            \
                                       (uint_t)fs_fnode_pool)      
int         Fs_read(handle_t file,void * buffer,int size)
{
    file_t        * f           = NULL;     /*  file            */
    device_t      * device      = NULL;
    fs_t          * fs          = NULL;     /*  file system     */
    int             nr          = -1;       /*  number of read  */

    if( NULL == ( f = Koum_handle_object(file) ) )
        goto fs_read_end;
    /*  ���ļ�ʱ��û�������  */
    if( !( f->f_attr & FILE_ATTR_READ ) )
    {
        _printk("read deny\n");
        /*  Ӧ�������������*/
        goto fs_read_end;
    }
    if( f->f_attr & FILE_ATTR_DEVICE )
    {
        _printk("device read\n");
        nr = Dev_read(f->f_object,f->f_ptr,buffer,size);
        /*if( nr < 0 ) �������������*/
    }
    else
    {
        _printk("file read\n");
        FS_CHECK_FNODE(f->f_object);
        fs = FNODE_GET_FS((fnode_t *)(f->f_object));
        ASSERT(fs && fs->fs_drv);
        FS_LOCK(fs);
        nr = fs->fs_drv->fs_fnode_read(FILE_FNODE(f),f->f_ptr,buffer,size);
        FS_FREE(fs);
    }

fs_read_end:
    return nr;
}

int         Fs_write(handle_t file,void * buffer,int size)
{
    file_t        * f           = NULL;     /*  file            */
    device_t      * device      = NULL;
    fs_t          * fs          = NULL;     /*  file system     */
    int             nr          = -1;       /*  number of read  */

    if( NULL == ( f = Koum_handle_object(file) ) )
        goto fs_write_end;
    if( f->f_attr & FILE_ATTR_DEVICE )
    {
        _printk("device write\n");
        nr = Dev_write(f->f_object,f->f_ptr,buffer,size);
        /*if( nr < 0 ) �������������*/
    }
    else
    {
        _printk("file write\n");
        FS_CHECK_FNODE(f->f_object);
        fs = FNODE_GET_FS((fnode_t *)(f->f_object));
        ASSERT(fs && fs->fs_drv);
        FS_LOCK(fs);
        nr = fs->fs_drv->fs_fnode_read(FILE_FNODE(f),f->f_ptr,buffer,size);
        FS_FREE(fs);
    }
    return -1;
}
void        Fs_initial(void)
{
    _printk("file system initial...\n");
    fs_active = -1;
    Fs_detect_partition();
    Fs_register(Fat32_register);
    /*  �����ʼ������ */
    Fs_specify();
    if( fs_active == -1 )
        _printk("no active file system!\n");
    else
        fs_root = &fs_pool[fs_active].fs_root;
    Fs_information();
    _printk("file system initial OK!\n");
}


