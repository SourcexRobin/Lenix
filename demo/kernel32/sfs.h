//  2014-03-08
//  简易文件系统 simple file system

//  在文件节点中的内容应该是不修改的内容，变化的内容应在目录节点中
//  文件节点应该管理文件的分配单元列表，块链表，一个单元4K，用32位标识单元数，一共可以表示16T
//  目录节点应管理文件的具体信息，名称、创建时间、最后修改时间、文件长度、访问控制

//  简单文件系统划分为4个部分，
//  1、引导单元，最前面的8个分配单元，也就是预留32K的空间给引导程序使用，
//  2、分配单元位图，记录分配单元的使用情况，通过扫描位图查找可用分配单元
//  3、文件链接节点位图，记录文件节点使用情况,
//  4、文件链接节点区域，形成文件链表。采用块链表
//  5、可用分配单元区域
//  关键是文件链接节点与可用分配单元之间的关系，设计为非一一对应关系，按填满一个分配单元设计
//  8*4096 = 一个单元 总数就是，总分配单元数/(8*4096)*xx,一个单元可以表示32K个链接节点，也就是
//  最小需要4K*32K = 128M，最小分区。512G的分区需要位图，需要4K个单元，需要16M的位图空间，

//  将分区内空间，全部划分为4K的分配单元，分配单元数 = 分区空间 / 4K
//  分配单元位图占用，每个分配单元可以记录的可用分配单元数，8*4096,
//      分配单元位图占用的分配单元 = (分配单元数 + 4095 * 8 - 1) / 8/4K
//  文件节点数，最保险的方法，为每个分配单元配置一个文件节点。每个文件节点32字节，那么空间利用率为
//  4K/32
//  文件项128字节   

/*
 *  基本名称缩写：
 *  AU  :分配单元，Allocate Unit
 *  FLN :文件链接节点，File Link Node
 *  DFI :目录文件项，Directory File Item
 */
#define SFS_AU_SIZE                 4096
#define SFS_SLN_PER_AU              128
#define SFS_BITS_PER_AU             (4096*8)
/*
 *  简易文件系统模型，管理空间的分配、文件链接、路径层级
 */
typedef struct _sfs_fs_parameter_t
{
    byte_t                  fsp_jump_code[4];   /*  跳转程序空间        */
    byte_t                  fsp_name[16];       /*  分区名称            */
    word_t                  fsp_bytes_per_sect; /*  每扇区字节数        */
    word_t                  fsp_sects_per_au;   /*  每分配单元字节数    */
    qword_t                 fsp_total_sects;    /*  扇区总数            */
    dword_t                 fsp_total_au;       /*  分区内分配单元总数  */
    dword_t                 fsp_total_fln;      /*  链接节点数          */
    dword_t                 fsp_au_boot;        /*  引导单元数量        */
    dword_t                 fsp_au_map;         /*  分配单元位图占用    */
    dword_t                 fsp_au_fln_map;     /*  链接节点占用单元数  */
    dword_t                 fsp_au_fln;
    dword_t                 fsp_au_user;        /*  可用分配单元数      */
    dword_t                 fsp_root_fln;       /*  根目录链接节点      */
}fsp_t;
/*
 *  文件链接节点，用以组成块链表。
 */
typedef struct _sfs_file_link_node_t
{
    dword_t                 fln_aus[7];         /*  分配单元数组        */
    dword_t                 fln_next;           /*  下一个文件链接节点  */
}fln_t;

/*
 *  0至7分配单元用于引导了，肯定不会使用
 */
#define SFS_AU_INVALID              0x00000000

/*
 *  目录文件项，目录项为128字节，每一个代表一个文件，目录是特殊的文件
 *  目录文件限制最大长度为56K，也就是每个目录最多可以有2个链接节点，
 *  最多有448个文件（目录）
 *  最长支持100个字节的文件名，末尾以0填充
 */
typedef struct _sfs_directory_file_item_t
{
    byte_t                  dfi_name[100];      /*  文件名称            */
    byte_t                  dfi_type[4];        /*  文件类型            */
    dword_t                 dfi_attrs;          /*  文件属性            */
    qword_t                 dfi_size;           /*  文件大小            */
    dword_t                 dfi_first_fln;      /*  文件的第一个链接节点*/
    sdate_t                 dfi_create_date;    /*  创建时间            */
    stime_t                 dfi_create_time;
    sdate_t                 dfi_write_date;     /*  最后写时间          */
    stime_t                 dfi_write_time;
}dfi_t;

#define SFS_DFI_ATTR_DIRECTORY      0x00000001
#define SFS_DFI_ATTR_READ           0x00000002
#define SFS_DFI_ATTR_WRITE          0x00000004
#define SFS_DFI_ATTR_EXECUTE        0x00000008
#define SFS_DFI_ATTR_SYSTEM         0x00000010
#define SFS_DFI_ATTR_HIDDEN         0x00000020