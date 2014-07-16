//  2014-03-08
//  �����ļ�ϵͳ simple file system

//  ���ļ��ڵ��е�����Ӧ���ǲ��޸ĵ����ݣ��仯������Ӧ��Ŀ¼�ڵ���
//  �ļ��ڵ�Ӧ�ù����ļ��ķ��䵥Ԫ�б�������һ����Ԫ4K����32λ��ʶ��Ԫ����һ�����Ա�ʾ16T
//  Ŀ¼�ڵ�Ӧ�����ļ��ľ�����Ϣ�����ơ�����ʱ�䡢����޸�ʱ�䡢�ļ����ȡ����ʿ���

//  ���ļ�ϵͳ����Ϊ4�����֣�
//  1��������Ԫ����ǰ���8�����䵥Ԫ��Ҳ����Ԥ��32K�Ŀռ����������ʹ�ã�
//  2�����䵥Ԫλͼ����¼���䵥Ԫ��ʹ�������ͨ��ɨ��λͼ���ҿ��÷��䵥Ԫ
//  3���ļ����ӽڵ�λͼ����¼�ļ��ڵ�ʹ�����,
//  4���ļ����ӽڵ������γ��ļ��������ÿ�����
//  5�����÷��䵥Ԫ����
//  �ؼ����ļ����ӽڵ�����÷��䵥Ԫ֮��Ĺ�ϵ�����Ϊ��һһ��Ӧ��ϵ��������һ�����䵥Ԫ���
//  8*4096 = һ����Ԫ �������ǣ��ܷ��䵥Ԫ��/(8*4096)*xx,һ����Ԫ���Ա�ʾ32K�����ӽڵ㣬Ҳ����
//  ��С��Ҫ4K*32K = 128M����С������512G�ķ�����Ҫλͼ����Ҫ4K����Ԫ����Ҫ16M��λͼ�ռ䣬

//  �������ڿռ䣬ȫ������Ϊ4K�ķ��䵥Ԫ�����䵥Ԫ�� = �����ռ� / 4K
//  ���䵥Ԫλͼռ�ã�ÿ�����䵥Ԫ���Լ�¼�Ŀ��÷��䵥Ԫ����8*4096,
//      ���䵥Ԫλͼռ�õķ��䵥Ԫ = (���䵥Ԫ�� + 4095 * 8 - 1) / 8/4K
//  �ļ��ڵ�������յķ�����Ϊÿ�����䵥Ԫ����һ���ļ��ڵ㡣ÿ���ļ��ڵ�32�ֽڣ���ô�ռ�������Ϊ
//  4K/32
//  �ļ���128�ֽ�   

/*
 *  ����������д��
 *  AU  :���䵥Ԫ��Allocate Unit
 *  FLN :�ļ����ӽڵ㣬File Link Node
 *  DFI :Ŀ¼�ļ��Directory File Item
 */
#define SFS_AU_SIZE                 4096
#define SFS_SLN_PER_AU              128
#define SFS_BITS_PER_AU             (4096*8)
/*
 *  �����ļ�ϵͳģ�ͣ�����ռ�ķ��䡢�ļ����ӡ�·���㼶
 */
typedef struct _sfs_fs_parameter_t
{
    byte_t                  fsp_jump_code[4];   /*  ��ת����ռ�        */
    byte_t                  fsp_name[16];       /*  ��������            */
    word_t                  fsp_bytes_per_sect; /*  ÿ�����ֽ���        */
    word_t                  fsp_sects_per_au;   /*  ÿ���䵥Ԫ�ֽ���    */
    qword_t                 fsp_total_sects;    /*  ��������            */
    dword_t                 fsp_total_au;       /*  �����ڷ��䵥Ԫ����  */
    dword_t                 fsp_total_fln;      /*  ���ӽڵ���          */
    dword_t                 fsp_au_boot;        /*  ������Ԫ����        */
    dword_t                 fsp_au_map;         /*  ���䵥Ԫλͼռ��    */
    dword_t                 fsp_au_fln_map;     /*  ���ӽڵ�ռ�õ�Ԫ��  */
    dword_t                 fsp_au_fln;
    dword_t                 fsp_au_user;        /*  ���÷��䵥Ԫ��      */
    dword_t                 fsp_root_fln;       /*  ��Ŀ¼���ӽڵ�      */
}fsp_t;
/*
 *  �ļ����ӽڵ㣬������ɿ�����
 */
typedef struct _sfs_file_link_node_t
{
    dword_t                 fln_aus[7];         /*  ���䵥Ԫ����        */
    dword_t                 fln_next;           /*  ��һ���ļ����ӽڵ�  */
}fln_t;

/*
 *  0��7���䵥Ԫ���������ˣ��϶�����ʹ��
 */
#define SFS_AU_INVALID              0x00000000

/*
 *  Ŀ¼�ļ��Ŀ¼��Ϊ128�ֽڣ�ÿһ������һ���ļ���Ŀ¼��������ļ�
 *  Ŀ¼�ļ�������󳤶�Ϊ56K��Ҳ����ÿ��Ŀ¼��������2�����ӽڵ㣬
 *  �����448���ļ���Ŀ¼��
 *  �֧��100���ֽڵ��ļ�����ĩβ��0���
 */
typedef struct _sfs_directory_file_item_t
{
    byte_t                  dfi_name[100];      /*  �ļ�����            */
    byte_t                  dfi_type[4];        /*  �ļ�����            */
    dword_t                 dfi_attrs;          /*  �ļ�����            */
    qword_t                 dfi_size;           /*  �ļ���С            */
    dword_t                 dfi_first_fln;      /*  �ļ��ĵ�һ�����ӽڵ�*/
    sdate_t                 dfi_create_date;    /*  ����ʱ��            */
    stime_t                 dfi_create_time;
    sdate_t                 dfi_write_date;     /*  ���дʱ��          */
    stime_t                 dfi_write_time;
}dfi_t;

#define SFS_DFI_ATTR_DIRECTORY      0x00000001
#define SFS_DFI_ATTR_READ           0x00000002
#define SFS_DFI_ATTR_WRITE          0x00000004
#define SFS_DFI_ATTR_EXECUTE        0x00000008
#define SFS_DFI_ATTR_SYSTEM         0x00000010
#define SFS_DFI_ATTR_HIDDEN         0x00000020