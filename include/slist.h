/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    Ƕ��ʽ����ϵͳ
//                             2011 , ��  ��Դ���빤����
//                                     �������а�Ȩ
//                          ***---------------------------***
//
//  �ļ�����    : slist.h
//  �ļ�������  : ��  ��
//  �ļ�����ʱ��:
///  ����޸���  : ��  ��
//  ����޸�ʱ��:
//
//  ��Ҫ����    : �ṩ�������������������
//
//  ˵��        : 
//
//  �汾�仯��¼:
//  �汾��      |    ʱ��        |  ����       |  ��Ҫ�仯��¼
//========================================================================================
//              |  2011-08-27   |  �ޱ�        |
//              |  2011-08-23   |  �ޱ�        |  ��һ��
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