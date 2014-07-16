/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: ipc.h
//  创建时间: 2011-07-12        创建者: 罗斌
//  修改时间: 2012-11-29        修改者: 罗斌
//  主要功能: 提供基本的进程同步机制，以及进程间通信机制。
//  说    明: 
//
//  变更记录:
//  版 本 号    |     时间      |  作  者       | 主要变化记录
//=============================================================================
//              |  2014-02-16   |  罗  斌       | 根据KOUM修改
//  00.00.000   |  2011-07-12   |  罗  斌       | 第一版
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
//    为内核和应用程序提供一种可以直接操作对象指针的同步机制。可以在内核对象中
//  直接嵌入锁对象，也可以在应用程序中直接定义锁对象.
*/

typedef struct _lock_t
{
    volatile int            lck_status;         /*  锁状态                  */
#ifdef _CFG_SMP_
    spin_lock_t             lck_lock;
#endif
    struct _proc_t      *   lck_user;           /*  占用锁的进程            */
    int                     lck_user_prio;      /*  占锁进程的原始优先级    */
    int                     lck_priority;       /*  对象当前优先级          */
}lock_t;

void        Lck_lock(lock_t * lck);
void        Lck_free(lock_t * lck);

/*
//////////////////////////////////////////////////////////////////////////////
////////////////////
//  互斥
*/

typedef struct _mutex_t
{
    koh_t                   koh;
    volatile int            mtx_status;
#ifdef _CFG_SMP_
    spin_lock_t             mtx_lock;
#endif
    flag_t                  mtx_flag;
    proc_t              *   mtx_user;           /*  当前占用进程            */
    int                     mtx_user_prio;      /*  占用进程的原始优先级    */
    int                     mtx_priority;       /*  当前优先级              */
    proc_list_t             mtx_wait;           /*  等待进程                */
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
//  信号量（多值信号）
*/

/*
 *    在进程等待信号量时，同样有可能出现优先级反转的问题。但Lenix选择不对信号
 *  量的优先级反转问题进行处理。
 *     1、信号量多用于2个及以上资源的控制。在出现优先级反转的问题时，会有可能
 *  出现多个进程已经获得信号的情况，这时需要提高哪一个进程的优先级呢。可以任意
 *  选择，但选择出来的进程可能是处于非运行态，这样不能解决优先级反转的问题。或
 *  者当前优先级最高的运行态进程，但是这需要实时跟踪。要实现这一点需要大量的资
 *  源。
 *     2、当一个进程拥有多个信号（也就是资源）时，要如何识别出来同样是比较复杂
 *  的问题。
 *    综合这些，Lenix决定不处理信号量的优先级反转问题。
 */
typedef struct _semaphore_t
{
    koh_t                   koh;
    long                    sema_count;     /*  信号量当前值            */
    long                    sema_max;       /*  信号量最大值            */
#ifdef _CFG_SMP_
    spin_lock_t             sema_lock;
#endif
    proc_list_t             sema_wait;      /*  等待列表                */
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
//  消息
*/

typedef struct _message_t
{
    int                     msg_pid;            /*  发送进程id              */
    uint_t                  msg_type;           /*  消息类型                */
    dword_t                 msg_param32;        /*  32位消息参数            */
    qword_t                 msg_param64;        /*  64位消息参数            */
}message_t;


typedef struct _message_box_t
{
    koh_t                   koh;
    char                    mb_name[OBJ_NAME_LEN]; /*  消息对象名称         */
    proc_t              *   mb_owner;           /*  邮箱拥有者              */
    int                     mb_ref;             /*  引用计数器              */
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
//  事件
*/

typedef struct _event_t
{
    struct _event_t     *   evn_res;        /*  资源管理链表        */
#ifdef _CFG_SMP
    volatile spin_lock_t    evn_lock;
#endif  /*  _CFG_SMP_   */
    uint32_t                evn_arrive;     /*  已经到达的事件          */
    uint32_t                evn_map;        /*  事件位图                */
    proc_t              *   evn_user;       /*  占用的进程              */
    int                     evn_src_prio;   /*  占用进程的原始优先级    */
    int                     evn_priority;   /*  当前优先级              */
    proc_t              *   evn_wait;       /*  等待事件对象的进程      */
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