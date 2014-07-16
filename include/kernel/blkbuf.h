/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: blkbuf.h  �黺�� block buffer
//  ����ʱ��: 2014-02-28        ������: �ޱ�
//  �޸�ʱ��:                   �޸���: 
//  ��Ҫ����: �ṩ���̹����ܡ�
//  ˵    ��: 
//
//  �����¼:
//  �� �� ��    |     ʱ��      |  ��  ��       |  ��Ҫ�仯��¼
//=============================================================================
//  00.00.000   |  2014-02-22   |  ��  ��       |  �����ļ�
///////////////////////////////////////////////////////////////////////////////
*/

#ifndef _BLKBUF_H_
#define _BLKBUF_H_

#include <config.h>
#include <type.h>

#include <ipc.h>
#include <device.h>

/*
 *  �黺������컺���״̬��
 *    1���Ƿ�д����д�����Ҫд�ؿ��豸��ûд������ֱ�ӻ��ա�
 *    2�������Ƿ���Ч�����磬�շ���������δ�ӿ��豸�ж������ݣ����ǣ�ֻҪ��
 *  �ڶ�������ǰ���黺�����ɢ�б��У��Ͳ��ᱻ�ҵ���
 */
typedef struct _blkbuf_t
{
    struct _blkbuf_t    *   bb_prev,
                        *   bb_next;        /*  �����б�ڵ�    */
    struct _blkbuf_t    *   bb_bucket_prev,
                        *   bb_bucket_next; /*  ɢ�б�ڵ�      */
    dword_t                 bb_flags;       /*  ��־            */
    void                *   bb_buffer;      /*  �ڵ㻺��        */
    lock_t                  bb_lock;        /*  ������          */
    int                     bb_refcnt;      /*  ���ü�����      */
    device_t            *   bb_device;      /*  �豸����        */
    offset_t                bb_address;     /*  ��Ӧ��ַ        */
}blkbuf_t;

/*
 *  �黺�������󣬲�����̭��
 */
#define BBUF_FLAGS_DIRTED           0x00000001

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  �����黺���������ӿ�
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
//  ģ��ʹ�ýӿ�
*/
void        Bbuf_initial(void);
blkbuf_t *  Bbuf_read(device_t * device,offset_t address);
result_t    Bbuf_release(blkbuf_t * blkbuf);
result_t    Bbuf_sync(void);

#endif  /*  _BLKBUF_H_  */
