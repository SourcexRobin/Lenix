/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      Lenix
//                                                  Ƕ��ʽ����ϵͳ
//                                           2011 , ��  ��Դ���빤����
//                                                   �������а�Ȩ
//                                        ***---------------------------***
//
//  �ļ�����    : cpu.h
//  ����޸���  : ��  ��
//  ����޸�ʱ��:
//
//  ��Ҫ����    : �ṩ����CPU�Ļ�������.
//
//  ˵��        : ���ļ������ĺ���������ֲʱ��Ҫʵ�ֵĻ������ܡ�
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
//======================================================================================================================
//  00.00.000   |   2011-08-27  |  �ޱ�         |  ��������
//  00.00.000   |   2011-08-23  |  �ޱ�         |  �����ļ�
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/

#include <slist.h>
#include <result.h>

list_node_t *    List_get_head(list_head_t *  list)
{    
    return list->ls_head;    
}

list_node_t *    List_get_tail(list_head_t *  list)
{    
    return list->ls_tail;    
}

result_t List_add_head(list_head_t *  list,list_node_t * ln)
{
    if( NULL == list->ls_head )
    {
        list->ls_head   = ln;
        list->ls_tail   = ln;
        ln->ln_next     = NULL;
        ln->ln_prev     = NULL;
        return RESULT_SUCCEED;
    }

    ln->ln_prev             = 0 ;
    ln->ln_next             = list->ls_head;
    list->ls_head->ln_prev  = ln;
    list->ls_head           = ln;

    return RESULT_SUCCEED;
}

result_t List_add_tail(list_head_t *  list,list_node_t * ln)
{
    if( NULL == list->ls_head )
    {
        list->ls_head   = ln;
        list->ls_tail   = ln;
        ln->ln_next     = NULL;
        ln->ln_prev     = NULL;
        return RESULT_SUCCEED;
    }

    ln->ln_prev             = list->ls_tail ;
    ln->ln_next             = 0;
    list->ls_tail->ln_next  = ln;
    list->ls_tail           = ln;

    return RESULT_SUCCEED;
}

result_t    List_del_head(list_head_t * list,list_node_t ** ln)
{
    list_node_t * next;

    if( list->ls_head == NULL)
        return RESULT_FAILED;

    *ln = list->ls_head;

    next = list->ls_head->ln_next;

    (*ln)->ln_next    = NULL;
    (*ln)->ln_prev    = NULL;

    if( next == NULL)
    {
        list->ls_head = NULL;
        list->ls_tail = NULL;
    }
    else
    {
        next->ln_prev = NULL;
        list->ls_head = next;
    }

    return RESULT_SUCCEED;
}

result_t    List_del_tail(list_head_t * list,list_node_t ** ln)
{
    list_node_t * prev;

    if( list->ls_head == NULL)
        return RESULT_FAILED;

    *ln = list->ls_tail;

    prev = list->ls_tail->ln_prev;

    (*ln)->ln_next      = NULL;
    (*ln)->ln_prev      = NULL;

    if( prev == NULL)
    {
        list->ls_head   = NULL;
        list->ls_tail   = NULL;
    }
    else
    {
        prev->ln_next   = NULL;
        list->ls_tail   = prev;
    }

    return RESULT_SUCCEED;
}

result_t    List_del_at(list_head_t * list,list_node_t * ln)
{
    list_node_t         *   prev,
                        *   next;

    if( list == NULL)
        return RESULT_FAILED;

    if( list->ls_head == ln )
        return List_del_head(list,&prev);   //  ����prev������ʱ����ʹ��

    if( list->ls_tail == ln)
        return List_del_tail(list,&prev);   //  ����prev������ʱ����ʹ��

    prev            = ln->ln_prev;
    next            = ln->ln_next;
    prev->ln_next   = next;
    next->ln_prev   = prev;
    ln->ln_next     = NULL;
    ln->ln_prev     = NULL;

    return RESULT_SUCCEED;
}

result_t    List_insert_before(list_head_t * list,list_node_t * cn,list_node_t * ln)
{
    list_node_t * prev ;

    if( list == NULL)
        return RESULT_FAILED;

    if( list->ls_head == cn )
        return List_add_head(list,ln);

    prev = cn->ln_prev;

    prev->ln_next   = ln;
    ln->ln_prev     = prev;

    ln->ln_next     = cn;
    cn->ln_prev     = ln;

    return RESULT_SUCCEED;
}

result_t        List_insert_after(list_head_t * list,list_node_t * cn,list_node_t * ln)
{
    list_node_t * next;
    if( list == NULL)
        return RESULT_FAILED;

    if( list->ls_tail == cn)
        return List_add_tail(list,ln);

    next = cn->ln_next;

    cn->ln_next     = ln;
    ln->ln_prev     = cn;
    ln->ln_next     = next;
    next->ln_prev   = ln;

    return RESULT_SUCCEED;
}

