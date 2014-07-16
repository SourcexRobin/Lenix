/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    嵌入式操作系统
//                             2011 , 罗  斌，源代码工作室
//                                     保留所有版权
//                          ***---------------------------***
//
//  文件名称    : slist.h
//  文件创建者  : 罗  斌
//  文件创建时间:
///  最后修改者  : 罗  斌
//  最后修改时间:
//
//  主要功能    : 提供基本的链表操作函数。
//
//  说明        : 
//
//  版本变化记录:
//  版本号      |    时间        |  作者       |  主要变化记录
//========================================================================================
//              |  2011-08-27   |  罗斌        |
//              |  2011-08-23   |  罗斌        |  第一版
//////////////////////////////////////////////////////////////////////////////////////////
*/


#ifndef _LIST_H_
#define _LIST_H_


#include <type.h>
#include <result.h>

#define LIST_ZERO(list)             do{(list)->ls_head = NULL;(list)->ls_tail = NULL;}while(0)

#define LIST_SCAN(list,cn)          for((cn) = (list)->ls_head ; (cn) ; (cn) = (cn)->ln_next)

#define LIST_HEAD(list)             ((list)->ls_head)
#define LIST_TAIL(list)             ((list)->ls_tail)

result_t        List_add_head       (list_head_t * list,list_node_t * ln);
result_t        List_add_tail       (list_head_t * list,list_node_t * ln);
result_t        List_del_head       (list_head_t * list,list_node_t ** ln);
result_t        List_del_tail       (list_head_t * list,list_node_t ** ln);
result_t        List_del_at         (list_head_t * list,list_node_t * ln);
result_t        List_insert_before  (list_head_t * list,list_node_t * cn,list_node_t * ln);
result_t        List_insert_after   (list_head_t * list,list_node_t * cn,list_node_t * ln);

list_node_t *   List_get_head(list_head_t *  list);
list_node_t *   List_get_tail(list_head_t *  list);

#endif  /*  _LIST_H_    */