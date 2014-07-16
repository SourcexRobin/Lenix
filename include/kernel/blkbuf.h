/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: blkbuf.h  块缓存 block buffer
//  创建时间: 2014-02-28        创建者: 罗斌
//  修改时间:                   修改者: 
//  主要功能: 提供进程管理功能。
//  说    明: 
//
//  变更记录:
//  版 本 号    |     时间      |  作  者       |  主要变化记录
//=============================================================================
//  00.00.000   |  2014-02-22   |  罗  斌       |  建立文件
///////////////////////////////////////////////////////////////////////////////
*/

#ifndef _BLKBUF_H_
#define _BLKBUF_H_

#include <config.h>
#include <type.h>

#include <ipc.h>
#include <device.h>

/*
 *  块缓存对象块快缓存的状态，
 *    1、是否写过。写入过则要写回块设备，没写过可以直接回收。
 *    2、数据是否有效。比如，刚分配块对象，尚未从块设备中读出数据，但是，只要不
 *  在读出数据前将块缓存插入散列表中，就不会被找到。
 */
typedef struct _blkbuf_t
{
    struct _blkbuf_t    *   bb_prev,
                        *   bb_next;        /*  管理列表节点    */
    struct _blkbuf_t    *   bb_bucket_prev,
                        *   bb_bucket_next; /*  散列表节点      */
    dword_t                 bb_flags;       /*  标志            */
    void                *   bb_buffer;      /*  节点缓存        */
    lock_t                  bb_lock;        /*  对象锁          */
    int                     bb_refcnt;      /*  引用计数器      */
    device_t            *   bb_device;      /*  设备对象        */
    offset_t                bb_address;     /*  对应地址        */
}blkbuf_t;

/*
 *  块缓存锁定后，不能淘汰。
 */
#define BBUF_FLAGS_DIRTED           0x00000001

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  单个块缓存对象操作接口
*/
#define BBUF_DIRTED(bb)             do{ (bb)->bb_flags |= BBUF_FLAGS_DIRTED;\
                                      }while(0)
#define BBUF_LOCKED(bb)             do{ (bb)->bb_flags |= BBUF_FLAGS_LOCKED;\
                                      }while(0)
#define BBUF_TO_SECTOR(bb,addr)     ((void *)((byte_t * )((bb)->bb_buffer)+\
                                      512 * ((addr) & 7)))
#define BBUF_DECLARE()              blkbuf_t * _t_bb_ = NULL
#define BBUF_READ_SECTOR(buf,dev,addr)  \
                                    do{ _t_bb_ = Bbuf_read(dev,addr);\
                                        if( _t_bb_ )\
                                        (buf) = BBUF_TO_SECTOR(_t_bb_,addr); \
                                        else (buf) = NULL;\
                                      }while(0)
#define BBUF_RELEASE()              Bbuf_release(_t_bb_)
#define BBUF_LOCK()                 Lck_lock(&_t_bb_->bb_lock)
#define BBUF_FREE()                 Lck_free(&_t_bb_->bb_lock)



/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  模块使用接口
*/
void        Bbuf_initial(void);
blkbuf_t *  Bbuf_read(device_t * device,offset_t address);
result_t    Bbuf_release(blkbuf_t * blkbuf);
result_t    Bbuf_sync(void);

#endif  /*  _BLKBUF_H_  */
