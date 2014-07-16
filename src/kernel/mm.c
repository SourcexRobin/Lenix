/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenix实时操作系统
//                              2011 - 2012 @ 源代码工作室
//                                     保留所有版权
//                          ***----------------------------***
//       名  称 : mm.c
//     创建时间 : 2011-07-12       创建者  : 罗斌
//     修改时间 : 2012-12-19       修改者  : 罗斌
//
//  主要功能    : 提供进程管理功能
//
//  说明        :
//
//  版本变化记录:
//  版本号      |     时间      |  作者        |  主要变化记录
//========================================================================================
//              |   2012-12-19  |  罗斌        |  增加临界段保护
//              |   2012-11-07  |  罗斌        |  因写书而重构，更改结构体名称而已
//  00.00.000   |   2011-07-12  |  罗斌        |  第一版
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
#include <config.h>
#include <lmemory.h>

#include <proc.h>
#include <mm.h>

/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  名  称: 
//
//  作  用: 
//
//  参  数: 
//      格式说明：
//      名称，第9列开始        类型，第29列开始
//                             说明：第29列开始
//
//  返回值: 
//    类型: 
//    说明: 
//
//  注  意: 
//
//  变更记录:
//  时间        |  作  者       |  说明
//========================================================================================
//  xxxx-xx-xx  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
result_t    Heap_initial(heap_t * heap,void   * buffer,uint32_t leng,uint_t   grain)
{
    
    if( heap == NULL || buffer == NULL || leng < MIN_HEAP_SIZE )
        return RESULT_FAILED;

    if( grain > 8 ) 
        grain = 8;  //  控制分配单元
        
    heap->heap_grain                = MAU_SIZE * ( 1 << grain );
    heap->heap_hmml                 = buffer;
    heap->heap_current              = heap->heap_hmml;
    heap->heap_hmml->hmnh_next      = heap->heap_hmml;
    heap->heap_hmml->hmnh_prev      = heap->heap_hmml;
    heap->heap_hmml->hmnh_used      = 1;
    heap->heap_hmml->hmnh_free      = leng / MAU_SIZE - 1;

    return RESULT_SUCCEED;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  名  称: 
//
//  作  用: 
//
//  参  数: 
//      格式说明：
//      名称，第9列开始        类型，第29列开始
//                             说明：第29列开始
//
//  返回值: 
//    类型: 
//    说明: 
//
//  注  意: 
//
//  变更记录:
//  时间        |  作  者       |  说明
//========================================================================================
//  xxxx-xx-xx  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
void    *   Heap_alloc(heap_t * heap,size_t   size)
{
    size_t                  use     = 0;            /*  需要占用的内存单元数量      */
    hmnh_t              *   hmn     = NULL;         /*  循环算子                    */
    hmnh_t              *   nhmn    = NULL;         /*  新的堆内存节点，new HMN     */
    CRITICAL_DECLARE(heap->heap_lock);

    use =  HEAP_GRAIN_ALIGN(size,heap) / MAU_SIZE;

    CRITICAL_BEGIN();
    /*
     *  跳过小于等于所需数量
     */
    for(    hmn = heap->heap_current ; 
            hmn->hmnh_next != heap->heap_current && hmn->hmnh_free <= use ; 
            hmn = hmn->hmnh_next)
        ;

    if( hmn->hmnh_free <= use )
    {
        CRITICAL_END();
        return NULL;
    }

    nhmn                        = hmn + hmn->hmnh_used;
    nhmn->hmnh_used             = 1 + use;
    nhmn->hmnh_free             = hmn->hmnh_free - 1 - use;
    hmn->hmnh_free              = 0;

    nhmn->hmnh_next             = hmn->hmnh_next;
    nhmn->hmnh_prev             = hmn;
    hmn->hmnh_next->hmnh_prev   = nhmn;
    hmn->hmnh_next              = nhmn;

    CRITICAL_END();
    return nhmn + 1;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  名  称: 
//
//  作  用: 
//
//  参  数: 
//      格式说明：
//      名称，第9列开始        类型，第29列开始
//                             说明：第29列开始
//
//  返回值: 
//    类型: 
//    说明: 
//
//  注  意: 
//
//  变更记录:
//  时间        |  作  者       |  说明
//========================================================================================
//  xxxx-xx-xx  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
result_t    Heap_free(heap_t * heap,void   * m)
{
    hmnh_t              *   hmn         = NULL;
    hmnh_t              *   hmnh_prev   = NULL;
    hmnh_t              *   hmnh_free   = (hmnh_t *)m - 1;
    CRITICAL_DECLARE(heap->heap_lock);

    if( NULL == heap || NULL == m)
        return RESULT_FAILED;
        
    hmnh_prev   = heap->heap_hmml;
    hmn         = hmnh_prev->hmnh_next;

    for( ; hmn != heap->heap_hmml ; hmnh_prev = hmn ,hmn = hmn->hmnh_next)
    {
        if( hmn == hmnh_free)
            break;
    }

    if( hmn != hmnh_free)
    {
        CRITICAL_END();
        return RESULT_FAILED;
    }

    hmnh_prev->hmnh_free       += hmn->hmnh_free + hmn->hmnh_used;
    hmnh_prev->hmnh_next        = hmn->hmnh_next;
    hmn->hmnh_next->hmnh_prev   = hmnh_prev;
 
    heap->heap_current      = hmnh_prev;
    
    CRITICAL_END();
    return RESULT_SUCCEED;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//
*/


/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  名  称: 
//
//  作  用: 
//
//  参  数: 
//      格式说明：
//      名称，第9列开始        类型，第29列开始
//                             说明：第29列开始
//
//  返回值: 
//    类型: 
//    说明: 
//
//  注  意: 
//
//  变更记录:
//  时间        |  作  者       |  说明
//========================================================================================
//  xxxx-xx-xx  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
result_t    Fm_initial(fixed_mm_t * fm,void   * buffer,uint_t   size,int   grain)
{

    fsmnh_t             *   fe      = NULL;
    byte_t              *   buf     = NULL;
    int                     i       = 0;

    /*
     *  管理的空间必须大于等于1024
     */
    if( size < 1024 )
        return RESULT_FAILED;

    FM_ZERO(fm);

    //  将粒度调整为64字节的整数倍，最小64字节，最大512字节
    grain = ((grain & 7 ) + 1 ) * 64;
#ifdef _CFG_BOUND_CHECK_
    grain += 64;
#endif
    fe  = buffer;
    buf = (byte_t *)buffer + size;
    buf = (byte_t *)((uint_t  )buf & ~7); //  调整为8字节对齐，不算浪费

    
    /*  计算可以划分出的内存块数量  */
    for( ; (uint_t  )(fe + 1) < (uint_t  )(buf - grain) ; fe++,buf -= grain)
        fm->fm_max++;

    /*  初始化对象参数  */
    fe = buffer;
    fm->fm_head         = fe;
    fm->fm_tail         = fe;
    fm->fm_free         = fm->fm_max;
    fm->fm_size         = size;
    fm->fm_grain        = grain;
    fm->fm_fehead       = fe;
    fm->fm_buffer       = buf;

    /*  将空闲块组织成一个链表  */
    for( fe++,i = 1 ; i < fm->fm_max ; i++,fe++)
    {
        fm->fm_tail->fsmnh_next     = fe;
        fm->fm_tail                 = fe;
        fe->fsmnh_next              = NULL;
    }
    
    return RESULT_SUCCEED;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  名  称: 
//
//  作  用: 
//
//  参  数: 
//      格式说明：
//      名称，第9列开始        类型，第29列开始
//                             说明：第29列开始
//
//  返回值: 
//    类型: 
//    说明: 
//
//  注  意: 
//
//  变更记录:
//  时间        |  作  者       |  说明
//========================================================================================
//  xxxx-xx-xx  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
void    *   Fm_alloc(fixed_mm_t * fm)
{
    fsmnh_t             *   fe      = NULL;
    CRITICAL_DECLARE(fm->fm_lock);

    CRITICAL_BEGIN();

    if( fm->fm_free < 1 )
    {
        CRITICAL_END();
        return NULL;
    }

    fe = fm->fm_head;

    fm->fm_head = fe->fsmnh_next;
    fm->fm_free--;

    if( 0 == fm->fm_free )
    {
        fm->fm_head = NULL;
        fm->fm_tail = NULL;
    }

    fe->fsmnh_ptr = FM_FETOA(fm,fe);

    CRITICAL_END();
#ifdef _CFG_BOUND_CHECK_

#else
    return fe->fsmnh_ptr;
#endif  /*  _CFG_BOUND_CHECK_   */
}


/*
//////////////////////////////////////////////////////////////////////////////////////////
//
//  名  称: 
//
//  作  用: 
//
//  参  数: 
//      格式说明：
//      名称，第9列开始        类型，第29列开始
//                             说明：第29列开始
//
//  返回值: 
//    类型: 
//    说明: 
//
//  注  意: 
//
//  变更记录:
//  时间        |  作  者       |  说明
//========================================================================================
//  xxxx-xx-xx  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
result_t    Fm_free(fixed_mm_t * fm,void   * m)
{
    fsmnh_t             *   fe      = NULL;
    CRITICAL_DECLARE(fm->fm_lock);

    if( NULL == fm || NULL == m )
        return RESULT_FAILED;

    fe = FM_ATOFE(fm,m);

    /*
     *  内存块合法性校验
     */
    if( fe->fsmnh_ptr != (fsmnh_t *)m )
        return RESULT_FAILED;

    CRITICAL_BEGIN();

    if( 0 == fm->fm_free)
    {
        fm->fm_head    = fe;
        fm->fm_tail    = fe;
        fm->fm_free++;
        CRITICAL_END();
        return RESULT_SUCCEED;
    }

    fe->fsmnh_next              = NULL;
    fm->fm_tail->fsmnh_next     = fe;
    fm->fm_tail                 = fe;
    fm->fm_free++;
        
    CRITICAL_END();
    return RESULT_SUCCEED;
}
