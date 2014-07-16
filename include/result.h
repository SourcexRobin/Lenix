/*
//////////////////////////////////////////////////////////////////////////////////////////
//                                        Lenix
//                                    嵌入式操作系统
//                             2011 , 罗  斌，源代码工作室
//                                     保留所有版权
//                          ***---------------------------***
//
//  文件名称    : result.h
//  文件创建者  : 罗  斌
//  文件创建时间:
//  最后修改者  : 罗  斌
//  最后修改时间:
//
//  主要功能    : 提供了返回结果的定义
//
//  说明        : 返回结果以0表示没有错误，非零表示有错误发生
//              : 错误代码按分段方式管理，每个段100个编码
//  版本变化记录:
//  版本号      |     时间      |  作者         |  主要变化记录
//========================================================================================
//  00.00.000   |   2012-01-10  |  罗斌         |  创建文件
//
//////////////////////////////////////////////////////////////////////////////////////////
*/

#ifndef _RESULT_H_
#define _RESULT_H_

#define RESULT_NO_ERR               0
#define RESULT_SUCCEED              RESULT_NO_ERR

#define RESULT_ERROR                -1
#define RESULT_FAILED               -1

/*
 *  系统通用的错误代码
 */
#define ERR_NULL_POINTER            0x01
#define ERR_POINTER_INVALIDE        0x01
#define ERR_SOURCE_EXHAUST          0x02
#define ERR_NAME_INVALID            0x03
#define ERR_BUSY                    0x04
#define ERR_OUT_OF_IVTID            0X05
/*
 *  模块专属错误代码
 */
#define ERR_TYPE_PROC               0x100
#define ERR_TYPE_SEMAPHORE          0x200
#define ERR_TYPE_MUTEX              0x300
#define ERR_TYPE_MESSAGE            0x400
#define ERR_TYPE_DEVICE             0x500


#define ERR_SEMA_EXHAUST            ( ERR_TYPE_SEMAPHORE    + 1 )
#define ERR_SEMA_NOT_SYS_CREATE     ( ERR_TYPE_SEMAPHORE    + 2 )
#define ERR_SEMA_MAX_OVERFLOW       ( ERR_TYPE_SEMAPHORE    + 3 )
#define ERR_SEMA_BUSY               ( ERR_TYPE_SEMAPHORE    + 4 )
#define ERR_SEMA_KOUM_ADD           ( ERR_TYPE_SEMAPHORE    + 5 )


#define ERR_MUTEX_FULL              1
#define ERR_MUTEX_EXHAUST           ( ERR_TYPE_MUTEX        + 1 )
#define ERR_MUTEX_OUT_OF_POOL       ( ERR_TYPE_MUTEX        + 2 )
#define ERR_MUTEX_BUSY              ( ERR_TYPE_MUTEX        + 3 )
#define ERR_MUTEX_KOUM_ADD          ( ERR_TYPE_MUTEX        + 4 )


#define ERR_DEV_NOT_EXIST           ( ERR_TYPE_DEVICE       + 1 )
#define ERR_DEV_EXIST               ( ERR_TYPE_DEVICE       + 2 )
#define ERR_DEV_REG_FAILED          ( ERR_TYPE_DEVICE       + 3 )
#define ERR_DEV_UNREG_FAILED        ( ERR_TYPE_DEVICE       + 4 )
#define ERR_DEV_OPEN_FAILED         ( ERR_TYPE_DEVICE       + 5 )
#define ERR_DEV_CLOSE_FAILED        ( ERR_TYPE_DEVICE       + 6 )
#define ERR_DEV_ENTRY_INVALID       ( ERR_TYPE_DEVICE       + 7 )
#define ERR_DEV_INVALID             ( ERR_TYPE_DEVICE       + 8 )
#define ERR_DATA_INVALID            ( ERR_TYPE_DEVICE       + 9 )
#define ERR_DATA_SIZE_INVALID       ( ERR_TYPE_DEVICE       + 10)
#define ERR_REF_CNT_INVALID         ( ERR_TYPE_DEVICE       + 11)

#define ERR_MSG_NO_SPACE            ( ERR_TYPE_MESSAGE      + 1 )
#define ERR_MSG_EXIST               ( ERR_TYPE_MESSAGE      + 2 )
#define ERR_MSG_NOT_OWNER           ( ERR_TYPE_MESSAGE      + 3 )
#define ERR_MSG_NOT_EXIST           ( ERR_TYPE_MESSAGE      + 4 )
#define ERR_MSG_BUFFER_FULL         ( ERR_TYPE_MESSAGE      + 5 )
#define ERR_MSG_BUFFER_EMPTY        ( ERR_TYPE_MESSAGE      + 6 )
#define ERR_MSG_INVALID             ( ERR_TYPE_MESSAGE      + 7 )
#define ERR_MSG_NOT_SYS_CREATE      ( ERR_TYPE_MESSAGE      + 8 )
#define ERR_MSG_ITSELF              ( ERR_TYPE_MESSAGE      + 9 )

#endif /*   _RESULT_H_  */