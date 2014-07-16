/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: const.h
//  创建时间: 2012-12-20        创建者: 罗斌
//  修改时间:                   修改者: 
//  主要功能: 提供最基础的常数定义，以及Lenix预定义的宏。
//
//  说    明: 
//
//  变化记录:
//  版 本 号    |   时  间    |   作  者    |  主要变化记录
//=============================================================================
//              | 2014-02-26  |   罗  斌    |  移入Lenix预定义宏
//              | 2012-12-20  |   罗  斌    |  增加组件配置
///////////////////////////////////////////////////////////////////////////////
*/

#ifndef _CONST_H_
#define _CONST_H_

#define K                           ((uint_t)1024)
#define M                           (K*K)
#define G                           (((uint32_t)*K) * ((uint32_t)M))

#define NULL                        ((void *)0)
#define TRUE                        1
#define FALSE                       0
#define INVALID_HANDLE              0

/*
 *  系统优先级数量，Lenix设定了64个优先级
 */
#define PROC_PRIORITY_MAX           64
/*
 *  PROC_INVALID_PRIORITY   :  无效优先级
 *  PROC_INVALID_PRIONUM    :  无效优先数
 */
#define PROC_INVALID_PRIORITY       255
#define PROC_INVALID_PRIONUM        255

#define MIN_HEAP_SIZE               8192

/*
 *  锁机制状态
 */
#define LOCK_STATUS_FREE            0
#define LOCK_STATUS_LOCKED          1


#define INT8_MIN                    0x80
#define INT8_MAX                    0x7F
#define UINT8_MAX                   0xFF

#define INT16_MIN                   0x8000
#define INT16_MAX                   0x7FFF
#define UINT16_MAX                  0xFFFF

#define INT32_MIN                   0x80000000
#define INT32_MAX                   0x7FFFFFFF
#define UINT32_MAX                  0xFFFFFFFF

#define INT64_MIN                   0x8000000000000000
#define INT64_MAX                   0x7FFFFFFFFFFFFFFF
#define UINT64_MAX                  0xFFFFFFFFFFFFFFFF

#define OBJ_NAME_LEN                12

/*
 *  块缓存散列表的桶数量。
 *    选择这个数量并没有经过严格的数学证明，仅是感觉上认为采用素数可以让数据块
 *  的分布更为平均。可以采用的数量可以是137、257、311几个，哦，这也没有经过严格
 *  的证明。这里选择了311，对应的16进制数为0x137，对应的2进制数为100110111B，觉
 *  得有数学上的美感。
 */
#define BBUF_BUCKETS                311

/*
 *  块缓存大小。
 *    采用这一数值考虑了一般应用中操作数据量的大小，以及一定的扩展性。当然，还
 *  考虑了分页状态下，实现虚拟内存管理中交换操作的便利性。
 *    在平均的状态下，311个桶，每个桶中有8个数据块，则一共311*8*4K = 9952K，约
 *  为10M的缓存空间。在这个缓存范围内，可轻松达到微秒级的性能。
 */
#define BBUF_SIZE                   4096

/*
///////////////////////////////////////////////////////////////////////////////
////////////////////
//  Lenix预订定义的宏
*/

#define ABS(n)                      ((n)>0?(n):-(n))
#define MAX(a,b)                    ((a)>(b)?(a):(b))
#define MIN(a,b)                    ((a)<(b)?(a):(b))

/*
 *  取得结构（联合）类型中成员的偏移量。
 *  t: 类型名称(type)
 *  m: 成员名称(member)
 */
#define OFFSET_OF(t,m)              ((uint_t)(&(((t*)(0))->(m))))
/*
 *  从结构（联合）类型的成员指针返回结构（联合）类型的指针
 *  t: 类型名称(type)
 *  m: 成员名称(member)
 *  p: 结构（联合）类型的成员指针(pointer)
 */
#define TYPE_OF(t,m,p)              ((t*)((uint_t)(p) - OFFSET_OF(t,m)))

/*
 *  交换
 */
#define SWAP(type,a,b)              do{type temp = a; a = b ; b = temp;\
                                      }while(FALSE)

#endif  /*  _COSNT_H_   */

