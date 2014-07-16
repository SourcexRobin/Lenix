/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: device.c
//  创建时间: 2011-02-09        创建者: 罗斌
//  修改时间: 2014-03-20        修改者: 罗斌
//  主要功能: 提供统一的设备管理接口。
//  说    明: Lenix系统下的设备使用基本方法
//              1、要使用设备，必须进行注册
//              2、设备注册后，通过打开设备接口来获得设备
//
//  变更记录:
//  版 本 号    |     时间      |  作  者       | 主要变化记录
//=============================================================================
//              |   2014-03-20  |  罗  斌       | 修改读写接口的返回值类型
//              |   2014-02-20  |  罗  斌       | 引入设备遍历机制
//              |   2012-12-02  |  罗  斌       | 重构，修改了设备驱动名称
//              |   2012-06-27  |  罗  斌       | 修正设备注册函数
//  00.00.000   |   2011-02-09  |  罗  斌       | 第一版
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

static device_t             dev_pool[DEVICE_MAX];   /*  系统设备对象池    */
static device_t         *   dev_fdol;               /*  空闲设备对象列表  */
static device_t         *   dev_sdl;                /*  系统设备列表      */
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
//  设备管理的基本功能
*/

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Dev_get
//  作    用: 分配设备对象。
//  参    数: 无
//  返 回 值: 成功返回非NULL，失败返回NULL。
//  注    意: 
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2012-11-04  |   罗  斌      |  第一版
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
//  名    称: Dev_put
//  作    用: 释放设备对象。
//  参    数: 无
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   设备对象指针
//  返 回 值: 无
//  注    意: 
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2012-11-04  |   罗  斌      |  第一版
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
//  名    称: Dev_sdl_add
//  作    用: 向系统设备列表中添加设备，在列表头添加。
//  参    数: 无
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   设备对象指针
//  返 回 值: 无
//  注    意: 
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2012-11-04  |   罗  斌      |  第一版
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
//  名    称: Dev_sdl_del
//  作    用: 从系统设备列表中删除设备。
//  参    数: 无
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   设备对象指针
//  返 回 值: 无
//  注    意: 
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2012-11-04  |   罗  斌      |  第一版
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
//  名    称: Dev_sdl_del
//  作    用: 根据名字查找设备。
//  参    数: 无
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     const char * |   devname     |   设备名称
//  返 回 值: 找到返回非NULL，其他返回NULL
//  注    意: 不区分大小写
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2012-11-04  |   罗  斌      |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
static
device_t *  Dev_query_by_name(const char * devname)
{
    device_t      * device = dev_sdl;

    for( ; device ; device = device->dev_next)
    {
        /*  名称相同并且处于激活状态，才能认为找到设备。*/
        if(_namecmp(device->dev_name,devname) == 0 && DEV_IS_ACTIVED(device))
            break;
    }

    return device;
}

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  设备管理API
*/


/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Dev_initial
//  作    用: 设备模块初始化。
//  参    数: 无
//  返 回 值: 无。
//  注    意: 将系统可用设备对象组织成为一个单向链表。
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2012-06-29  |   罗  斌      |  修正可用设备对象列表建立过程。
//  2012-01-09  |   罗  斌      |  第一版
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
     *  建立可用设备对象列表
     */
    for( device = dev_pool ; device <= LAST_DEVICE ; device++)
        Dev_put(device);
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Dev_set_date
//  作    用: 设置设备对象的参数块。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   设备对象指针
//      void *      |   data        |   设备参数块
//      uint_t      |   size        |   参数块长度
//  返 回 值: 成功返回RESULT_SUCCEED，失败返回RESULT_FAILED。
//  注    意: 如果提供的参数块大于32字节，则提供的参数块必须为永久空间
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2012-01-09  |   罗  斌      |  第一版
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
//  名    称: Dev_register
//  作    用: 向系统注册设备。
//  参    数: 无
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|------------------------------------------
//    const char *  |    name       |   设备名称.不超过11个字符,不区分大小写，
//                                  | 必须唯一
//  result_t (*)()  |    entry      |   设备驱动入口
//      void *      |    param      |   设备初始化参数指针，传递给驱动入口entry
//  返 回 值: 成功返回非NULL，失败返回NULL
//  说    明:   在设备驱动入口函数中不能含有dev_sdlister、dev_unregister、
//          dev_open、dev_close的调用
//  变更记录:
//  时    间    |  作者         |  说明
//=============================================================================
//  2012-11-04  |               |  修改了设备查找方式
//  2012-06-27  |  罗斌         | 
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
     *    查找该设备名称是否存在，如果存在，则不能注册设备。Lenix保证系统中设
     * 备名称唯一
     */
    CRITICAL_BEGIN();
    /*
     *  查找到名称，说明存在同名设备，不能注册。
     */
    if( NULL != ( device = Dev_query_by_name(name) ) )
    {
        PROC_SET_ERR(ERR_DEV_EXIST);
        CRITICAL_END();
        return ERR_DEV_EXIST;
    }
    /*
     *  到达这里说明系统中没有重名设备，可以注册。
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
     *  退出临界段保护后，才能执行驱动入口程序
     */
    if( entry(device,DEV_ENTRY_FLAG_REG,param) != RESULT_SUCCEED )
    {
        /*
         *  驱动入口程序执行失败，说明设备没有初始化成功，则将设备从sdl中删除
         */
        CRITICAL_BEGIN();
        Dev_sdl_del(device);
        CRITICAL_END();
        Dev_put(device);
        PROC_SET_ERR(ERR_DEV_REG_FAILED);
        return ERR_DEV_REG_FAILED;
    }
    /*
     *  驱动入口程序成功完成后，才可以激活设备。
     *  激活时，不用锁设备，因为激活后才有可能打开设备。
     */
    DEV_ACTIVE(device);
    PROC_NO_ERR();
    return RESULT_SUCCEED;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Dev_unregiste
//  作    用: 卸载设备。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     const char * |   name        |   设备对象名称
//      void *      |   param       |   卸载参数
//  返 回 值: 成功返回RESULT_SUCCEED，失败返回RESULT_FAILED。
//  注    意: 
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2012-11-04  |   罗  斌      |  第一版
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
     *  设备已经锁定，则不能卸载。
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
     *  系统已经回收资源，调用驱动入口程序对设备作最后的清理
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
//  名    称: Dev_isr
//  作    用: 设置设备的中断服务程序。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   设备对象指针
//      uint8_t     |   ivtid       |   Lenix设备模型的中断向量编号
//      isr_t       |   isr         |   中断服务程序
//  返 回 值: 成功返回非NULL，失败返回NULL。
//  注    意: 
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2012-11-04  |   罗  斌      |  第一版
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
//  名    称: Dev_open
//  作    用: 打开设备。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//     const char * |   name        |   设备名称
//      int         |   mode        |   打开模式
//  返 回 值: 成功返回非NULL，失败返回NULL。
//  注    意: 
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2012-11-04  |   罗  斌      |  第一版
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
     *    必须在锁定设备后，才能退出临界段保护。如果先退出临界段保护，在锁定设
     *  备，则有可能出现在锁定前发生任务切换，设备被卸载，导致设备无效。
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
//  名    称: Dev_close
//  作    用: 关闭设备。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   设备对象指针
//  返 回 值: 成功返回RESULT_SUCCEED，失败返回其他。
//  注    意: 引用计数器为0时，才会调用驱动的关闭接口
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2012-11-04  |   罗  斌      |  第一版
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
//  名    称: Dev_read
//  作    用: 读设备。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   设备对象指针
//      offset_t    |   offset      |   相对于设备起始位置的偏移
//      void *      |   buffer      |   缓冲区指针
//      int         |   size        |   需要读的数量，以字节为单位
//  返 回 值: 成功返回非负值，失败返回负值。
//  注    意:   必须提供位置参数，因为是直接操作设备，并不是从类似文件对象中取
//            得当前位置参数。
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-20  |   罗  斌      |  调整长度和返回值的数据类型，适应文件系统
//  2012-11-04  |   罗  斌      |  第一版
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
//  名    称: Dev_write
//  作    用: 写设备。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|-----------------------------------------
//      device_t *  |   device      |   设备对象指针
//      offset_t    |   offset      |   相对于设备起始位置的偏移
//      void *      |   buffer      |   缓冲区指针
//      int         |   size        |   需要读的数量，以字节为单位
//  返 回 值: 成功返回非负值，失败返回负值。
//  注    意:   必须提供位置参数，因为是直接操作设备，并不是从类似文件对象中取
//            得当前位置参数。
//
//  变更记录:
//  时    间    |   作  者      |  说明
//=============================================================================
//  2014-03-20  |   罗  斌      |  调整长度和返回值的数据类型，适应文件系统
//  2012-11-04  |   罗  斌      |  第一版
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
//  名  称: 
//
//  作  用: 
//
//  参  数: 
//      device              : device_t *
//                          :
//
//  返回值:  无
//
//  注  意: 
//
//  变更记录:
//  时间        |  作  者       |  说明
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
//  名    称: Dev_first
//  作    用: 获得系统中第一个设备。
//  参    数: 无
//  返 回 值: 成功返回非NULL，失败返回NULL
//  说    明:   为提供设备遍历而引入，与Dev_sdl_next函数配合使用，使用前要锁定系统
//          设备，防止系统设备变化。具体方式可以如下:
//          Dev_sdl_lock();
//          device_t * device = Dev_sdl_first();
//          while( device )
//          {
//              /*  处理设备的代码，要快速，即使设备变化不频繁    *-/
//              device = Dev_sdl_next(device);
//          }
//          Dev_sdl_free();
//            遍历过程中，建议不要修改设备参数
//  变更记录:
//  时    间    |  作者         |  说明
//=============================================================================
//  2014-02-20  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
device_t *  Dev_sdl_first(void)
{
    return dev_sdl;
}

/*
///////////////////////////////////////////////////////////////////////////////
//  名    称: Dev_next
//  作    用: 获得下一个设备。
//  参    数: 
//      参数类型    |   参数名称    |   简要说明
//  ----------------|---------------|------------------------------------------
//     device_t *   |    device     |   设备指针
//  返 回 值: 成功返回非NULL，失败返回NULL
//  说    明: 与Dev_sdl_first配套使用
//  变更记录:
//  时    间    |  作者         |  说明
//=============================================================================
//  2014-02-20  |  罗斌         |  第一版
///////////////////////////////////////////////////////////////////////////////
*/
device_t *  Dev_sdl_next(device_t * device)
{
    if( NULL == device )
        return NULL;
    return device->dev_next;
}

#endif /*   _CFG_DEVICE_ENABLE_ */
