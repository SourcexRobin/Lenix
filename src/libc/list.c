/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      Lenix
//                                                  嵌入式操作系统
//                                           2011 , 罗  斌，源代码工作室
//                                                   保留所有版权
//                                        ***---------------------------***
//
//  文件名称    : cpu.h
//  最后修改者  : 罗  斌
//  最后修改时间:
//
//  主要功能    : 提供抽象CPU的基本操作.
//
//  说明        : 该文件声明的函数都是移植时需要实现的基本功能。
//
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//======================================================================================================================
//  00.00.000   |   2011-08-27  |  罗斌         |  调试完善
//  00.00.000   |   2011-08-23  |  罗斌         |  创建文件
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
        return List_del_head(list,&prev);   //  这里prev当作临时变量使用

    if( list->ls_tail == ln)
        return List_del_tail(list,&prev);   //  这里prev当作临时变量使用

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

