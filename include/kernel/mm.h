/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenix实时操作系统
//                              2011 - 2012 @ 源代码工作室
//                                     保留所有版权
//                          ***----------------------------***
//       名  称 : mm.h
//     创建时间 : 2011-07-12       创建者  : 罗斌
//     修改时间 : 2012-12-19       修改者  : 罗斌
//
//  主要功能    : 提供基本内存管理功能
//
//  说明        :
//
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//========================================================================================
//              |   2011-07-12  |  罗斌         |  第一版
//////////////////////////////////////////////////////////////////////////////////////////
*/
#ifndef _MM_H_
#define _MM_H_

#include <type.h>
#include <result.h>

#define MAU_SIZE                    (sizeof(mau_t))

/*  用于内存越界检查  */
#if CPU_WORD == 16

    #define BOUND_CHECK_SIGN_HEAD   0xA5A5
    #define BOUND_CHECK_SIGN_TAIL   0x5A5A
    #define BOUND_CHECK_OK          0xFFFF

#elif CPU_WORD == 32

    #define BOUND_CHECK_SIGN_HEAD   0xA5A5A5A5
    #define BOUND_CHECK_SIGN_TAIL   0x5A5AA5A5
    #define BOUND_CHECK_OK          0xFFFFFFFF

#elif CPU_WORD == 64

    #define BOUND_CHECK_SIGN_HEAD   0xA5A5A5A5A5A5A5A5
    #define BOUND_CHECK_SIGN_TAIL   0x5A5AA5A5A5A5A5A5
    #define BOUND_CHECK_OK          0xFFFFFFFFFFFFFFFF

#endif  /*  CPU_WORD    */

#define DEFAULT_HEAP_GRAIN          1
/*
 *  堆内存节点头
 *  Heap Memory Node Header
 */
typedef struct _hmnh_t
{
    struct _hmnh_t *        hmnh_next;
    struct _hmnh_t *        hmnh_prev;
    size_t                  hmnh_used;
    size_t                  hmnh_free;
}hmnh_t;

/*
 *  内存分配单元:memory alloc unit
 *  简写为:mau
 */
typedef hmnh_t              mau_t;
/*
 *  堆对象
 */
typedef struct _heap_t
{
    hmnh_t              *   heap_hmml;              /*  堆起始地址    */
    hmnh_t              *   heap_current;           /*  当前分配单元  */
    size_t                  heap_grain;             /*  分配粒度     */
#ifdef _CFG_SMP_
    spin_lock_t             heap_lock;
#endif  /*  _CFG_SMP_   */
}heap_t;

#define HEAP_INITIAL(heap,buf,size) Heap_initial(heap,buf,size,DEFAULT_HEAP_GRAIN)
#define HEAP_GRAIN_ALIGN(size,heap) ((size + heap->heap_grain - 1) & \
                                        (~(heap->heap_grain - 1)))


result_t    Heap_initial    (heap_t * heap,void   * buffer,uint32_t leng,uint_t   grain);
void   *    Heap_alloc      (heap_t * heap,size_t   size);
result_t    Heap_free       (heap_t * heap,void   * m);


/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//
*/
/*
 *  定长内存节点头
 *  Fixed Size Memory Node Header
 */
typedef union    _fsmnh_t
{
    union _fsmnh_t      *   fsmnh_next;
    void                *   fsmnh_ptr;
}fsmnh_t;

/*
 *  定长内存管理对象
 */
typedef struct _fixed_mm_t
{
    fsmnh_t             *   fm_head;        /*  空闲列表头           */
    fsmnh_t             *   fm_tail;        /*  空闲内存块列表尾     */
    int                     fm_max;         /*  最大空闲块数         */
    int                     fm_free;        /*  当前可用空闲快数     */
    int                     fm_grain;       /*  分配粒度             */

    /*  内存管理对象参数 */
    fsmnh_t             *   fm_fehead;      /*  内存块节点           */
    uint_t                  fm_size;        /*  内存块长度           */
    void                *   fm_buffer;      /*  内存块               */
#ifdef _CFG_SMP_
    spin_lock_t             fm_lock;
#endif  /*  _CFG_SMP_   */
}fixed_mm_t;

#define FM_ZERO(fm)                 _memzero(fm,sizeof(fixed_mm_t))

/*
 *  内存地址转换分配节点
 *  Address TO Fixed Entry 
 *  首先计算地址与可分配空间首地址的距离，由于计算机除法的特点，没有余数，会造成
 *  地址不对齐，也能计算出正确的FSMN，所以在地址上加了一个偏移量，保证计算的正确
 */
#define FM_ADDR_DIST(fm,addr)       (((uint_t)(addr) + (fm)->fm_grain - 1 - \
                                        (uint_t)((fm)->fm_buffer))

#define FM_ATOFE(fm,addr)           ( (fm)->fm_fehead + \
                                        FM_ADDR_DIST(fm,addr)/ (fm)->fm_grain))

#define FM_FETOA(fm,fe)             ( (fsmnh_t *)((byte_t *)((fm)->fm_buffer) + \
                                      ((fe) - (fm)->fm_fehead) * (fm)->fm_grain))
result_t    Fm_initial  (fixed_mm_t * fm,void   * buffer,uint_t   size,int   grain);
void    *   Fm_alloc    (fixed_mm_t * fm);
result_t    Fm_free     (fixed_mm_t * fm,void   * m);


#endif /*   _MM_H_  */