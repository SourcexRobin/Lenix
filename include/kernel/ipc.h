/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: ipc.h
//  ����ʱ��: 2011-07-12        ������: �ޱ�
//  �޸�ʱ��: 2012-11-29        �޸���: �ޱ�
//  ��Ҫ����: �ṩ�����Ľ���ͬ�����ƣ��Լ����̼�ͨ�Ż��ơ�
//  ˵    ��: 
//
//  �����¼:
//  �� �� ��    |     ʱ��      |  ��  ��       | ��Ҫ�仯��¼
//=============================================================================
//              |  2014-02-16   |  ��  ��       | ����KOUM�޸�
//  00.00.000   |  2011-07-12   |  ��  ��       | ��һ��
///////////////////////////////////////////////////////////////////////////////
*/


#ifndef _IPC_H_
#define _IPC_H_

#include <const.h>
#include <type.h>
#include <result.h>
#include <lmemory.h>
#include <lstring.h>

#include <koum.h>
#include <proc.h>

/*
//////////////////////////////////////////////////////////////////////////////
////////////////////
//    Ϊ�ں˺�Ӧ�ó����ṩһ�ֿ���ֱ�Ӳ�������ָ���ͬ�����ơ��������ں˶�����
//  ֱ��Ƕ��������Ҳ������Ӧ�ó�����ֱ�Ӷ���������.
*/

typedef struct _lock_t
{
    volatile int            lck_status;         /*  ��״̬                  */
#ifdef _CFG_SMP_
    spin_lock_t             lck_lock;
#endif
    struct _proc_t      *   lck_user;           /*  ռ�����Ľ���            */
    int                     lck_user_prio;      /*  ռ�����̵�ԭʼ���ȼ�    */
    int                     lck_priority;       /*  ����ǰ���ȼ�          */
}lock_t;

void        Lck_lock(lock_t * lck);
void        Lck_free(lock_t * lck);

/*
//////////////////////////////////////////////////////////////////////////////
////////////////////
//  ����
*/

typedef struct _mutex_t
{
    koh_t                   koh;
    volatile int            mtx_status;
#ifdef _CFG_SMP_
    spin_lock_t             mtx_lock;
#endif
    flag_t                  mtx_flag;
    proc_t              *   mtx_user;           /*  ��ǰռ�ý���            */
    int                     mtx_user_prio;      /*  ռ�ý��̵�ԭʼ���ȼ�    */
    int                     mtx_priority;       /*  ��ǰ���ȼ�              */
    proc_list_t             mtx_wait;           /*  �ȴ�����                */
}mutex_t;

#define Mutex_destroy               Koum_release
#define MTX_FLAG_USED               1
#define MUTEX_INITIAL(mutex)        _memzero(mutex,sizeof(mutex_t))
//#define MUTEX_FREE(mutex)            do{ MUTEX_INITIAL(mutex); 
//                                           (mutex)->mtx_value = -1 ; } while(0)

void        Mutex_initial(void);
handle_t    Mutex_create(void);
result_t    Mutex_get(handle_t handle);
result_t    Mutex_put(handle_t handle);
/*
//////////////////////////////////////////////////////////////////////////////
////////////////////
//  �ź�������ֵ�źţ�
*/

/*
 *    �ڽ��̵ȴ��ź���ʱ��ͬ���п��ܳ������ȼ���ת�����⡣��Lenixѡ�񲻶��ź�
 *  �������ȼ���ת������д���
 *     1���ź���������2����������Դ�Ŀ��ơ��ڳ������ȼ���ת������ʱ�����п���
 *  ���ֶ�������Ѿ�����źŵ��������ʱ��Ҫ�����һ�����̵����ȼ��ء���������
 *  ѡ�񣬵�ѡ������Ľ��̿����Ǵ��ڷ�����̬���������ܽ�����ȼ���ת�����⡣��
 *  �ߵ�ǰ���ȼ���ߵ�����̬���̣���������Ҫʵʱ���١�Ҫʵ����һ����Ҫ��������
 *  Դ��
 *     2����һ������ӵ�ж���źţ�Ҳ������Դ��ʱ��Ҫ���ʶ�����ͬ���ǱȽϸ���
 *  �����⡣
 *    �ۺ���Щ��Lenix�����������ź��������ȼ���ת���⡣
 */
typedef struct _semaphore_t
{
    koh_t                   koh;
    long                    sema_count;     /*  �ź�����ǰֵ            */
    long                    sema_max;       /*  �ź������ֵ            */
#ifdef _CFG_SMP_
    spin_lock_t             sema_lock;
#endif
    proc_list_t             sema_wait;      /*  �ȴ��б�                */
}semaphore_t;

#define SEMA_ZERO(sema)             _memzero(sema,sizeof(semaphore_t))
#define SEMA_INITIAL(sema,max)      do{ SEMA_ZERO(sema); \
                                        (sema)->sema_max = max; \
                                        (sema)->sema_count= max;\
                                    }while(0)
#define Sema_destory        Koum_release

void        Sema_initial(void);
handle_t    Sema_create(long max);
result_t    Sema_down   (handle_t   handle);
result_t    Sema_up     (handle_t   handle);

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  ��Ϣ
*/

typedef struct _message_t
{
    int                     msg_pid;            /*  ���ͽ���id              */
    uint_t                  msg_type;           /*  ��Ϣ����                */
    dword_t                 msg_param32;        /*  32λ��Ϣ����            */
    qword_t                 msg_param64;        /*  64λ��Ϣ����            */
}message_t;


typedef struct _message_box_t
{
    koh_t                   koh;
    char                    mb_name[OBJ_NAME_LEN]; /*  ��Ϣ��������         */
    proc_t              *   mb_owner;           /*  ����ӵ����              */
    int                     mb_ref;             /*  ���ü�����              */
#ifdef _CFG_SMP_
    spin_lock_t             mb_lock;
#endif  /*  _CFG_SMP_   */
    proc_list_t             mb_send_wait;
    proc_list_t             mb_resv_wait;
    int                     mb_count;
    message_t           *   mb_buffer;
    int                     mb_send;
    int                     mb_resv;
}message_box_t;


#define MB_IS_FULL(mb)              ((((mb)->mb_send + 1) % (mb)->mb_count)==\
                                        (mb)->mb_resv )
#define MB_IS_EMPTY(mb)             ((((mb)->mb_resv + 1) % (mb)->mb_count)==\
                                        (mb)->mb_send )
#define MB_IS_VALID(mb)             ((mb)->mb_buffer)
#define MB_SEND_FORWARD(mb)         do{ (mb)->mb_send = \
                                        ((mb)->mb_send + 1) % (mb)->mb_count;\
                                      }while(0)
#define MB_RESV_FORWARD(mb)         do{ (mb)->mb_resv = \
                                        ((mb)->mb_resv + 1) % (mb)->mb_count;\
                                      }while(0)

void        Msg_initial (void);
handle_t    Msg_create  (const char * name,message_t * ms,int count);
handle_t    Msg_get     (const char * name);
result_t    Msg_send    (handle_t handle,uint_t type,
                         dword_t param32,qword_t param64);
result_t    Msg_post    (handle_t handle,uint_t type,
                         dword_t param32,qword_t param64);
result_t    Msg_resv    (handle_t handle,message_t * msg);
result_t    Msg_take    (handle_t handle,message_t * msg);
/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  �¼�
*/

typedef struct _event_t
{
    struct _event_t     *   evn_res;        /*  ��Դ��������        */
#ifdef _CFG_SMP
    volatile spin_lock_t    evn_lock;
#endif  /*  _CFG_SMP_   */
    uint32_t                evn_arrive;     /*  �Ѿ�������¼�          */
    uint32_t                evn_map;        /*  �¼�λͼ                */
    proc_t              *   evn_user;       /*  ռ�õĽ���              */
    int                     evn_src_prio;   /*  ռ�ý��̵�ԭʼ���ȼ�    */
    int                     evn_priority;   /*  ��ǰ���ȼ�              */
    proc_t              *   evn_wait;       /*  �ȴ��¼�����Ľ���      */
}event_t;

#define EVENT_MAKE(e)               ( 1L << ((e) & 31))

#define EVENT_INITIAL(evn,map)      do{ _memzero(evn,sizeof(event_t)); \
                                        (evn)->evn_map = (map); }while(0)

#define EVENT_ZERO(evn)             _memzero(evn,sizeof(event_t))
#define EVENT_FREE(evn)             do{ EVENT_ZERO(evn); \
                                       (evn)->evn_res = evn_free_list; \
                                       evn_free_list = (evn);}while(0)

void        Event_initial(void);
event_t *   Event_create(uint32_t map);
result_t    Event_destory(event_t * evn);

void        Event_reset (event_t * evn);
void        Event_set   (event_t * evn,uint32_t map);
BOOL        Event_wait  (event_t * evn,uint32_t millisecond);


#endif /*   _IPC_H_     */