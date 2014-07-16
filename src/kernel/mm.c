/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                   Lenixʵʱ����ϵͳ
//                              2011 - 2012 @ Դ���빤����
//                                     �������а�Ȩ
//                          ***----------------------------***
//       ��  �� : mm.c
//     ����ʱ�� : 2011-07-12       ������  : �ޱ�
//     �޸�ʱ�� : 2012-12-19       �޸���  : �ޱ�
//
//  ��Ҫ����    : �ṩ���̹�����
//
//  ˵��        :
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����        |  ��Ҫ�仯��¼
//========================================================================================
//              |   2012-12-19  |  �ޱ�        |  �����ٽ�α���
//              |   2012-11-07  |  �ޱ�        |  ��д����ع������Ľṹ�����ƶ���
//  00.00.000   |   2011-07-12  |  �ޱ�        |  ��һ��
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
//  ��  ��: 
//
//  ��  ��: 
//
//  ��  ��: 
//      ��ʽ˵����
//      ���ƣ���9�п�ʼ        ���ͣ���29�п�ʼ
//                             ˵������29�п�ʼ
//
//  ����ֵ: 
//    ����: 
//    ˵��: 
//
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
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
        grain = 8;  //  ���Ʒ��䵥Ԫ
        
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
//  ��  ��: 
//
//  ��  ��: 
//
//  ��  ��: 
//      ��ʽ˵����
//      ���ƣ���9�п�ʼ        ���ͣ���29�п�ʼ
//                             ˵������29�п�ʼ
//
//  ����ֵ: 
//    ����: 
//    ˵��: 
//
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
//========================================================================================
//  xxxx-xx-xx  |               |
//
//////////////////////////////////////////////////////////////////////////////////////////
*/
void    *   Heap_alloc(heap_t * heap,size_t   size)
{
    size_t                  use     = 0;            /*  ��Ҫռ�õ��ڴ浥Ԫ����      */
    hmnh_t              *   hmn     = NULL;         /*  ѭ������                    */
    hmnh_t              *   nhmn    = NULL;         /*  �µĶ��ڴ�ڵ㣬new HMN     */
    CRITICAL_DECLARE(heap->heap_lock);

    use =  HEAP_GRAIN_ALIGN(size,heap) / MAU_SIZE;

    CRITICAL_BEGIN();
    /*
     *  ����С�ڵ�����������
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
//  ��  ��: 
//
//  ��  ��: 
//
//  ��  ��: 
//      ��ʽ˵����
//      ���ƣ���9�п�ʼ        ���ͣ���29�п�ʼ
//                             ˵������29�п�ʼ
//
//  ����ֵ: 
//    ����: 
//    ˵��: 
//
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
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
//  ��  ��: 
//
//  ��  ��: 
//
//  ��  ��: 
//      ��ʽ˵����
//      ���ƣ���9�п�ʼ        ���ͣ���29�п�ʼ
//                             ˵������29�п�ʼ
//
//  ����ֵ: 
//    ����: 
//    ˵��: 
//
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
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
     *  ����Ŀռ������ڵ���1024
     */
    if( size < 1024 )
        return RESULT_FAILED;

    FM_ZERO(fm);

    //  �����ȵ���Ϊ64�ֽڵ�����������С64�ֽڣ����512�ֽ�
    grain = ((grain & 7 ) + 1 ) * 64;
#ifdef _CFG_BOUND_CHECK_
    grain += 64;
#endif
    fe  = buffer;
    buf = (byte_t *)buffer + size;
    buf = (byte_t *)((uint_t  )buf & ~7); //  ����Ϊ8�ֽڶ��룬�����˷�

    
    /*  ������Ի��ֳ����ڴ������  */
    for( ; (uint_t  )(fe + 1) < (uint_t  )(buf - grain) ; fe++,buf -= grain)
        fm->fm_max++;

    /*  ��ʼ���������  */
    fe = buffer;
    fm->fm_head         = fe;
    fm->fm_tail         = fe;
    fm->fm_free         = fm->fm_max;
    fm->fm_size         = size;
    fm->fm_grain        = grain;
    fm->fm_fehead       = fe;
    fm->fm_buffer       = buf;

    /*  �����п���֯��һ������  */
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
//  ��  ��: 
//
//  ��  ��: 
//
//  ��  ��: 
//      ��ʽ˵����
//      ���ƣ���9�п�ʼ        ���ͣ���29�п�ʼ
//                             ˵������29�п�ʼ
//
//  ����ֵ: 
//    ����: 
//    ˵��: 
//
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
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
//  ��  ��: 
//
//  ��  ��: 
//
//  ��  ��: 
//      ��ʽ˵����
//      ���ƣ���9�п�ʼ        ���ͣ���29�п�ʼ
//                             ˵������29�п�ʼ
//
//  ����ֵ: 
//    ����: 
//    ˵��: 
//
//  ע  ��: 
//
//  �����¼:
//  ʱ��        |  ��  ��       |  ˵��
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
     *  �ڴ��Ϸ���У��
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
