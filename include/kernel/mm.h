/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenixʵʱ����ϵͳ
//                              2011 - 2012 @ Դ���빤����
//                                     �������а�Ȩ
//                          ***----------------------------***
//       ��  �� : mm.h
//     ����ʱ�� : 2011-07-12       ������  : �ޱ�
//     �޸�ʱ�� : 2012-12-19       �޸���  : �ޱ�
//
//  ��Ҫ����    : �ṩ�����ڴ������
//
//  ˵��        :
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
//========================================================================================
//              |   2011-07-12  |  �ޱ�         |  ��һ��
//////////////////////////////////////////////////////////////////////////////////////////
*/
#ifndef _MM_H_
#define _MM_H_

#include <type.h>
#include <result.h>

#define MAU_SIZE                    (sizeof(mau_t))

/*  �����ڴ�Խ����  */
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
 *  ���ڴ�ڵ�ͷ
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
 *  �ڴ���䵥Ԫ:memory alloc unit
 *  ��дΪ:mau
 */
typedef hmnh_t              mau_t;
/*
 *  �Ѷ���
 */
typedef struct _heap_t
{
    hmnh_t              *   heap_hmml;              /*  ����ʼ��ַ    */
    hmnh_t              *   heap_current;           /*  ��ǰ���䵥Ԫ  */
    size_t                  heap_grain;             /*  ��������     */
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
 *  �����ڴ�ڵ�ͷ
 *  Fixed Size Memory Node Header
 */
typedef union    _fsmnh_t
{
    union _fsmnh_t      *   fsmnh_next;
    void                *   fsmnh_ptr;
}fsmnh_t;

/*
 *  �����ڴ�������
 */
typedef struct _fixed_mm_t
{
    fsmnh_t             *   fm_head;        /*  �����б�ͷ           */
    fsmnh_t             *   fm_tail;        /*  �����ڴ���б�β     */
    int                     fm_max;         /*  �����п���         */
    int                     fm_free;        /*  ��ǰ���ÿ��п���     */
    int                     fm_grain;       /*  ��������             */

    /*  �ڴ���������� */
    fsmnh_t             *   fm_fehead;      /*  �ڴ��ڵ�           */
    uint_t                  fm_size;        /*  �ڴ�鳤��           */
    void                *   fm_buffer;      /*  �ڴ��               */
#ifdef _CFG_SMP_
    spin_lock_t             fm_lock;
#endif  /*  _CFG_SMP_   */
}fixed_mm_t;

#define FM_ZERO(fm)                 _memzero(fm,sizeof(fixed_mm_t))

/*
 *  �ڴ��ַת������ڵ�
 *  Address TO Fixed Entry 
 *  ���ȼ����ַ��ɷ���ռ��׵�ַ�ľ��룬���ڼ�����������ص㣬û�������������
 *  ��ַ�����룬Ҳ�ܼ������ȷ��FSMN�������ڵ�ַ�ϼ���һ��ƫ��������֤�������ȷ
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