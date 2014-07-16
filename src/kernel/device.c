/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: device.c
//  ����ʱ��: 2011-02-09        ������: �ޱ�
//  �޸�ʱ��: 2014-03-20        �޸���: �ޱ�
//  ��Ҫ����: �ṩͳһ���豸����ӿڡ�
//  ˵    ��: Lenixϵͳ�µ��豸ʹ�û�������
//              1��Ҫʹ���豸���������ע��
//              2���豸ע���ͨ�����豸�ӿ�������豸
//
//  �����¼:
//  �� �� ��    |     ʱ��      |  ��  ��       | ��Ҫ�仯��¼
//=============================================================================
//              |   2014-03-20  |  ��  ��       | �޸Ķ�д�ӿڵķ���ֵ����
//              |   2014-02-20  |  ��  ��       | �����豸��������
//              |   2012-12-02  |  ��  ��       | �ع����޸����豸��������
//              |   2012-06-27  |  ��  ��       | �����豸ע�ắ��
//  00.00.000   |   2011-02-09  |  ��  ��       | ��һ��
///////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>

#ifdef _CFG_DEVICE_ENABLE_

#include <assert.h>
#include <lmemory.h>
#include <lstring.h>

#include <device.h>

#ifdef _SLASH_
    #include <machine/machine.h>
#else
    #include <machine\machine.h>
    #include <driver\pc_vga.h>
#endif

static device_t             dev_pool[DEVICE_MAX];   /*  ϵͳ�豸�����    */
static device_t         *   dev_fdol;               /*  �����豸�����б�  */
static device_t         *   dev_sdl;                /*  ϵͳ�豸�б�      */
static spin_lock_t          dev_sdl_lock;

#ifdef _CFG_SMP_

static spin_lock_t          dev_fdol_lock;

#endif  /*  _CFG_SMP_   */

#define DEV_ZERO(dev)               _memzero(dev,sizeof(device_t))
#define LAST_DEVICE                 (&dev_pool[DEVICE_MAX-1])

//  2014.2.20
void        Dev_sdl_lock(void)
{
    while( Cpu_tas((int *)&dev_sdl_lock,0,1) )
        ;
}
void        Dev_sdl_free(void)
{
    dev_sdl_lock = 0;
}

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  �豸����Ļ�������
*/

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_get
//  ��    ��: �����豸����
//  ��    ��: ��
//  �� �� ֵ: �ɹ����ط�NULL��ʧ�ܷ���NULL��
//  ע    ��: 
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2012-11-04  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
static
device_t *  Dev_get(void)
{
    device_t      * dev = dev_fdol;
    CRITICAL_DECLARE(dev_fdol_lock);

    CRITICAL_BEGIN();
    if( dev )
    {
        dev_fdol            = dev_fdol->dev_next;
        dev_fdol->dev_prev  = NULL;
        dev->dev_prev       = NULL;
        dev->dev_next       = NULL;
    }
    CRITICAL_END();

    return dev;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_put
//  ��    ��: �ͷ��豸����
//  ��    ��: ��
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   �豸����ָ��
//  �� �� ֵ: ��
//  ע    ��: 
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2012-11-04  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
static
void        Dev_put(device_t * device)
{
    CRITICAL_DECLARE(dev_fdol_lock);

    CRITICAL_BEGIN();
    DEV_ZERO(device);
    if( dev_fdol )
    {
        device->dev_next    = dev_fdol; 
        dev_fdol->dev_prev   = device;
    }
    dev_fdol = device;
    CRITICAL_END();
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_sdl_add
//  ��    ��: ��ϵͳ�豸�б�������豸�����б�ͷ��ӡ�
//  ��    ��: ��
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   �豸����ָ��
//  �� �� ֵ: ��
//  ע    ��: 
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2012-11-04  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
static
void        Dev_sdl_add(device_t * device)
{
    if(dev_sdl)
    {
        device->dev_next  = dev_sdl;
        dev_sdl->dev_prev = device;
    }
        
    dev_sdl = device;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_sdl_del
//  ��    ��: ��ϵͳ�豸�б���ɾ���豸��
//  ��    ��: ��
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   �豸����ָ��
//  �� �� ֵ: ��
//  ע    ��: 
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2012-11-04  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
static
void        Dev_sdl_del(device_t * device)
{
    if( device == dev_sdl)
    {
        dev_sdl = dev_sdl->dev_next;
        if( dev_sdl )
            dev_sdl->dev_prev = NULL;
    }
    else
    {
        device->dev_prev->dev_next = device->dev_next;
        if( device->dev_next )
            device->dev_next->dev_prev = device->dev_prev;
    }
    _memzero(device,sizeof(device_t));
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_sdl_del
//  ��    ��: �������ֲ����豸��
//  ��    ��: ��
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     const char * |   devname     |   �豸����
//  �� �� ֵ: �ҵ����ط�NULL����������NULL
//  ע    ��: �����ִ�Сд
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2012-11-04  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
static
device_t *  Dev_query_by_name(const char * devname)
{
    device_t      * device = dev_sdl;

    for( ; device ; device = device->dev_next)
    {
        /*  ������ͬ���Ҵ��ڼ���״̬��������Ϊ�ҵ��豸��*/
        if(_namecmp(device->dev_name,devname) == 0 && DEV_IS_ACTIVED(device))
            break;
    }

    return device;
}

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  �豸����API
*/


/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_initial
//  ��    ��: �豸ģ���ʼ����
//  ��    ��: ��
//  �� �� ֵ: �ޡ�
//  ע    ��: ��ϵͳ�����豸������֯��Ϊһ����������
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2012-06-29  |   ��  ��      |  ���������豸�����б������̡�
//  2012-01-09  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void        Dev_initial(void)
{
    device_t      * device      = NULL;

    dev_fdol        = NULL;
    dev_sdl         = NULL;
#ifdef _CFG_SMP_
    dev_fdol_lock   = 0;
    dev_sdl_lock    = 0;
#endif  /*  _CFG_SMP_   */

    _memzero(dev_pool,DEVICE_MAX * sizeof(device_t));
    /*
     *  ���������豸�����б�
     */
    for( device = dev_pool ; device <= LAST_DEVICE ; device++)
        Dev_put(device);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_set_date
//  ��    ��: �����豸����Ĳ����顣
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   �豸����ָ��
//      void *      |   data        |   �豸������
//      uint_t      |   size        |   �����鳤��
//  �� �� ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���RESULT_FAILED��
//  ע    ��: ����ṩ�Ĳ��������32�ֽڣ����ṩ�Ĳ��������Ϊ���ÿռ�
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2012-01-09  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void *      Dev_set_date(device_t * device,void * data,uint_t size)
{
    ASSERT(device);
    ASSERT(data);
    ASSERT(size);

#ifdef _CFG_CHECK_PARAMETER_
    if( NULL == device  )
    {
        PROC_SET_ERR(ERR_DEV_INVALID);
        return NULL;
    }
    if( NULL == data )
    {
        PROC_SET_ERR(ERR_DATA_INVALID);
        return NULL;
    }
    if( 0 == size)
    {
        PROC_SET_ERR(ERR_DATA_SIZE_INVALID);
        return NULL;
    }
#endif  /*  _CFG_CHECK_PARAMETER_   */

    _memzero(device->dev_data,32);
    if( size > 32 )
        device->dev_data_ext = data;
    else
    {
        _memcpy(device->dev_data,data,size);
        device->dev_data_ext = NULL;
    }

    return DEV_DATA(device);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_register
//  ��    ��: ��ϵͳע���豸��
//  ��    ��: ��
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|------------------------------------------
//    const char *  |    name       |   �豸����.������11���ַ�,�����ִ�Сд��
//                                  | ����Ψһ
//  result_t (*)()  |    entry      |   �豸�������
//      void *      |    param      |   �豸��ʼ������ָ�룬���ݸ��������entry
//  �� �� ֵ: �ɹ����ط�NULL��ʧ�ܷ���NULL
//  ˵    ��:   ���豸������ں����в��ܺ���dev_sdlister��dev_unregister��
//          dev_open��dev_close�ĵ���
//  �����¼:
//  ʱ    ��    |  ����         |  ˵��
//=============================================================================
//  2012-11-04  |               |  �޸����豸���ҷ�ʽ
//  2012-06-27  |  �ޱ�         | 
//  2012-01-09  |               |
///////////////////////////////////////////////////////////////////////////////
*/
result_t  Dev_registe(const char * name,
                      result_t  (* entry)(struct _device_t *,int,void *),
                      void       * param)
{
    device_t            *   device      = NULL;
    CRITICAL_DECLARE(dev_sdl_lock);

    ASSERT(name);
    ASSERT(entry);
#ifdef _CFG_CHECK_PARAMETER_
    if( NULL == name )
    {
        PROC_SET_ERR(ERR_NAME_INVALID);
        return ERR_NAME_INVALID;
    }
    if( NULL == entry )
    {
        PROC_SET_ERR(ERR_DEV_ENTRY_INVALID);
        return ERR_DEV_ENTRY_INVALID;
    }
#endif  /* _CFG_CHECK_PARAMETER_    */
    /*
     *    ���Ҹ��豸�����Ƿ���ڣ�������ڣ�����ע���豸��Lenix��֤ϵͳ����
     * ������Ψһ
     */
    CRITICAL_BEGIN();
    /*
     *  ���ҵ����ƣ�˵������ͬ���豸������ע�ᡣ
     */
    if( NULL != ( device = Dev_query_by_name(name) ) )
    {
        PROC_SET_ERR(ERR_DEV_EXIST);
        CRITICAL_END();
        return ERR_DEV_EXIST;
    }
    /*
     *  ��������˵��ϵͳ��û�������豸������ע�ᡣ
     */
    if( NULL == (device = Dev_get() ) ) 
    {
        CRITICAL_END();
        PROC_SET_ERR(ERR_SOURCE_EXHAUST);
        return ERR_SOURCE_EXHAUST;
    }
    DEV_ZERO(device);
    _nstrcpy(device->dev_name,name,12);
    device->dev_entry = entry;
    Dev_sdl_add(device);
    CRITICAL_END();
    /*
     *  �˳��ٽ�α����󣬲���ִ��������ڳ���
     */
    if( entry(device,DEV_ENTRY_FLAG_REG,param) != RESULT_SUCCEED )
    {
        /*
         *  ������ڳ���ִ��ʧ�ܣ�˵���豸û�г�ʼ���ɹ������豸��sdl��ɾ��
         */
        CRITICAL_BEGIN();
        Dev_sdl_del(device);
        CRITICAL_END();
        Dev_put(device);
        PROC_SET_ERR(ERR_DEV_REG_FAILED);
        return ERR_DEV_REG_FAILED;
    }
    /*
     *  ������ڳ���ɹ���ɺ󣬲ſ��Լ����豸��
     *  ����ʱ���������豸����Ϊ�������п��ܴ��豸��
     */
    DEV_ACTIVE(device);
    PROC_NO_ERR();
    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_unregiste
//  ��    ��: ж���豸��
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     const char * |   name        |   �豸��������
//      void *      |   param       |   ж�ز���
//  �� �� ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���RESULT_FAILED��
//  ע    ��: 
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2012-11-04  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Dev_unregiste(const char * name,void * param)
{
    device_t      * device      = NULL;
    result_t     (* entry)(device_t *,int,void *);
    CRITICAL_DECLARE(dev_sdl_lock);

    ASSERT(name);

#ifdef _CFG_CHECK_PARAMETER_
    if( NULL == name )
    {
        PROC_SET_ERR(ERR_NAME_INVALID);
        return ERR_NAME_INVALID;
    }
#endif  /*  _CFG_CHECK_PARAMETER_   */

    CRITICAL_BEGIN();
    if( NULL == ( device = Dev_query_by_name(name) ) )
    {
        PROC_SET_ERR(ERR_DEV_NOT_EXIST);
        CRITICAL_END();
        return ERR_DEV_NOT_EXIST;
    }
    /*
     *  �豸�Ѿ�����������ж�ء�
     */
    if( DEV_IS_LOCKED(device) )
    {
        PROC_SET_ERR(ERR_BUSY);
        CRITICAL_END();
        return ERR_BUSY;
    }
    entry = device->dev_entry;
    Dev_sdl_del(device);
    CRITICAL_END();
    Dev_put(device);
    /*
     *  ϵͳ�Ѿ�������Դ������������ڳ�����豸����������
     */
    if( entry(device,DEV_ENTRY_FLAG_UNREG,param) != RESULT_SUCCEED )
    {
        PROC_SET_ERR(ERR_DEV_UNREG_FAILED);
        return ERR_DEV_UNREG_FAILED;
    }
    PROC_NO_ERR();

    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_isr
//  ��    ��: �����豸���жϷ������
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   �豸����ָ��
//      uint8_t     |   ivtid       |   Lenix�豸ģ�͵��ж��������
//      isr_t       |   isr         |   �жϷ������
//  �� �� ֵ: �ɹ����ط�NULL��ʧ�ܷ���NULL��
//  ע    ��: 
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2012-11-04  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
void *      Dev_isr(device_t * device,uint8_t ivtid,isr_t isr)
{
    isr_t           pisr    = NULL;   /*  prev isr    */

    ASSERT( device );
    ASSERT( ivtid < IVT_MAX);

#ifdef _CFG_CHECK_PARAMETER_
    if( NULL == device )
    {
        PROC_SET_ERR(ERR_DEV_INVALID);
        return NULL;
    }
    if( ivtid >= IVT_MAX )
    {
        PROC_SET_ERR(ERR_OUT_OF_IVTID);
        return NULL;
    }
#endif  /*  _CFG_CHECK_PARAMETER_   */

    DEV_LOCK(device);
    device->dev_ivtid = ivtid;
    pisr = Machine_ivt_set(ivtid,isr);
    DEV_FREE(device);
    PROC_NO_ERR();

    return pisr;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_open
//  ��    ��: ���豸��
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//     const char * |   name        |   �豸����
//      int         |   mode        |   ��ģʽ
//  �� �� ֵ: �ɹ����ط�NULL��ʧ�ܷ���NULL��
//  ע    ��: 
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2012-11-04  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
device_t *  Dev_open   (const char * name,int mode)
{
    device_t      * device      = NULL;
    CRITICAL_DECLARE(dev_sdl_lock);

    CRITICAL_BEGIN();
    if( NULL == ( device = Dev_query_by_name(name) ) )
    {
        PROC_SET_ERR(ERR_DEV_NOT_EXIST);
        CRITICAL_END();
        return NULL;
    }

    DEV_LOCK(device);
    /*
     *    �����������豸�󣬲����˳��ٽ�α�����������˳��ٽ�α�������������
     *  �������п��ܳ���������ǰ���������л����豸��ж�أ������豸��Ч��
     */
    CRITICAL_END();
    if( RESULT_SUCCEED != device->dev_ddo.ddo_open(device) )
    {
        DEV_FREE(device);
        PROC_SET_ERR(ERR_DEV_OPEN_FAILED);
        mode = mode;
        return NULL;
    }
    device->dev_ref_cnt++;
    DEV_FREE(device);
    PROC_NO_ERR();

    return device;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_close
//  ��    ��: �ر��豸��
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   �豸����ָ��
//  �� �� ֵ: �ɹ�����RESULT_SUCCEED��ʧ�ܷ���������
//  ע    ��: ���ü�����Ϊ0ʱ���Ż���������Ĺرսӿ�
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2012-11-04  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
result_t    Dev_close  (device_t * device)
{
    result_t        result      = RESULT_SUCCEED;

    ASSERT(device);
    ASSERT(device->dev_ref_cnt > 0 );

#ifdef _CFG_CHECK_PARAMETER_
    if( NULL == device  )
    {
        PROC_SET_ERR(ERR_DEV_INVALID);
        return ERR_DEV_INVALID;
    }
    if( device->dev_ref_cnt < 1 )
    {
        PROC_SET_ERR(ERR_REF_CNT_INVALID);
        return ERR_REF_CNT_INVALID;
    }
#endif  /*  _CFG_CHECK_PARAMETER_   */
    
    DEV_LOCK(device);
    if( --device->dev_ref_cnt == 0 )
    {
        if( RESULT_SUCCEED != device->dev_ddo.ddo_close(device) )
            result = ERR_DEV_CLOSE_FAILED;
    }
    DEV_FREE(device);
    PROC_SET_ERR(result);

    return result;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_read
//  ��    ��: ���豸��
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   �豸����ָ��
//      offset_t    |   offset      |   ������豸��ʼλ�õ�ƫ��
//      void *      |   buffer      |   ������ָ��
//      int         |   size        |   ��Ҫ�������������ֽ�Ϊ��λ
//  �� �� ֵ: �ɹ����طǸ�ֵ��ʧ�ܷ��ظ�ֵ��
//  ע    ��:   �����ṩλ�ò�������Ϊ��ֱ�Ӳ����豸�������Ǵ������ļ�������ȡ
//            �õ�ǰλ�ò�����
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-20  |   ��  ��      |  �������Ⱥͷ���ֵ���������ͣ���Ӧ�ļ�ϵͳ
//  2012-11-04  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
int         Dev_read(device_t * device,offset_t offset,void * buffer,int size)
{
    int             nr      = -1;    /*  number of read*/
    int          (* read)(device_t *,offset_t,void *,int);

    ASSERT(device);
    ASSERT(buffer);
    ASSERT(size >= 0 );
#ifdef _CFG_CHECK_PARAMETER_
    if( NULL == device  )
    {
        PROC_SET_ERR(ERR_DEV_INVALID);
        return ERR_DEV_INVALID;
    }
#endif  /*  _CFG_CHECK_PARAMETER_   */
    read = device->dev_ddo.ddo_read;
    if( read )
    {
        DEV_LOCK(device);
        nr = read(device,offset,buffer,size);
        DEV_FREE(device);
    }

    return nr;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_write
//  ��    ��: д�豸��
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   �豸����ָ��
//      offset_t    |   offset      |   ������豸��ʼλ�õ�ƫ��
//      void *      |   buffer      |   ������ָ��
//      int         |   size        |   ��Ҫ�������������ֽ�Ϊ��λ
//  �� �� ֵ: �ɹ����طǸ�ֵ��ʧ�ܷ��ظ�ֵ��
//  ע    ��:   �����ṩλ�ò�������Ϊ��ֱ�Ӳ����豸�������Ǵ������ļ�������ȡ
//            �õ�ǰλ�ò�����
//
//  �����¼:
//  ʱ    ��    |   ��  ��      |  ˵��
//=============================================================================
//  2014-03-20  |   ��  ��      |  �������Ⱥͷ���ֵ���������ͣ���Ӧ�ļ�ϵͳ
//  2012-11-04  |   ��  ��      |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
int         Dev_write(device_t * device,offset_t offset,const void * buffer,
                      int size)
{
    int             nw      = -1;   /*  number of write*/
    int          (* write)(device_t *,offset_t,const void *,int);

    ASSERT(device);
    ASSERT(buffer);
#ifdef _CFG_CHECK_PARAMETER_
    if( NULL == device  )
    {
        PROC_SET_ERR(ERR_DEV_INVALID);
        return ERR_DEV_INVALID;
    }
#endif  /*  _CFG_CHECK_PARAMETER_   */
    write   = device->dev_ddo.ddo_write;
    if( write )
    {
        DEV_LOCK(device);
        nw = write(device,offset,buffer,size);
        DEV_FREE(device);
    }

    return nw;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
//  ��  ��: 
//
//  ��  ��: 
//
//  ��  ��: 
//      device              : device_t *
//                          :
//
//  ����ֵ:  ��
//
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
//========================================================================================
//  2012-11-04  |               |
//////////////////////////////////////////////////////////////////////////////////////////
*/
result_t    Dev_ctrl    (device_t * device,byte_t cmd,void * arg)
{
    result_t                result      = RESULT_FAILED;
    result_t              (*ctrl )(device_t *,byte_t,void *);

#ifdef _CFG_CHECK_PARAMETER_
    if( NULL == device  )
    {
        PROC_SET_ERR(ERR_DEV_INVALID);
        return ERR_DEV_INVALID;
    }
#endif  /*  _CFG_CHECK_PARAMETER_   */

    ctrl    = device->dev_ddo.ddo_ctrl;
    if( ctrl )
    {
        DEV_LOCK(device);
        result = ctrl(device,cmd,arg);
        DEV_FREE(device);
    }

    return result;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_first
//  ��    ��: ���ϵͳ�е�һ���豸��
//  ��    ��: ��
//  �� �� ֵ: �ɹ����ط�NULL��ʧ�ܷ���NULL
//  ˵    ��:   Ϊ�ṩ�豸���������룬��Dev_sdl_next�������ʹ�ã�ʹ��ǰҪ����ϵͳ
//          �豸����ֹϵͳ�豸�仯�����巽ʽ��������:
//          Dev_sdl_lock();
//          device_t * device = Dev_sdl_first();
//          while( device )
//          {
//              /*  �����豸�Ĵ��룬Ҫ���٣���ʹ�豸�仯��Ƶ��    *-/
//              device = Dev_sdl_next(device);
//          }
//          Dev_sdl_free();
//            ���������У����鲻Ҫ�޸��豸����
//  �����¼:
//  ʱ    ��    |  ����         |  ˵��
//=============================================================================
//  2014-02-20  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
device_t *  Dev_sdl_first(void)
{
    return dev_sdl;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  ��    ��: Dev_next
//  ��    ��: �����һ���豸��
//  ��    ��: 
//      ��������    |   ��������    |   ��Ҫ˵��
//  ----------------|---------------|------------------------------------------
//     device_t *   |    device     |   �豸ָ��
//  �� �� ֵ: �ɹ����ط�NULL��ʧ�ܷ���NULL
//  ˵    ��: ��Dev_sdl_first����ʹ��
//  �����¼:
//  ʱ    ��    |  ����         |  ˵��
//=============================================================================
//  2014-02-20  |  �ޱ�         |  ��һ��
///////////////////////////////////////////////////////////////////////////////
*/
device_t *  Dev_sdl_next(device_t * device)
{
    if( NULL == device )
        return NULL;
    return device->dev_next;
}

#endif /*   _CFG_DEVICE_ENABLE_ */
