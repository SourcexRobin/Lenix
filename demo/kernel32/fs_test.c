/*
///////////////////////////////////////////////////////////////////////////////
//                             Lenix嵌入式操作系统
//                         2011 - 2014 @ 源代码工作室
//                                保留所有版权
//                     ***----------------------------***
//  名    称: fs_test.c  
//  创建时间: 2014-03-09        创建者: 罗斌
//  修改时间:                   修改者: 
//  主要功能: 块缓存测试
//  说    明: 
//
//  变更记录:
//  版 本 号    |     时间      |  作  者       |  主要变化记录
//=============================================================================
//  00.00.000   |  2014-03-09   |  罗  斌       |  建立文件
///////////////////////////////////////////////////////////////////////////////
*/

#include <lenix.h>
#include <blkbuf.h>
#include "fs.h"

STACK_DEFINE(fst);

/*  2014-03-11  长格式时间转换为短格式时间  */
stime_t     Time_to_stime(time_t * time)
{
    stime_t         stime;

    stime.st_hour   = time->time_hour;
    stime.st_minute = time->time_minute;
    stime.st_hour   = time->time_hour;

    return stime;
}

sdate_t     Date_to_sdate(date_t * date)
{
    sdate_t         sdate;

    sdate.sd_year   = date->date_year - 1980;
    sdate.sd_month  = date->date_month;
    sdate.sd_day    = date->date_day;

    return sdate;
}

void        Con_print_char(byte_t c);
void        Clk_msg(void);
void        Shell_cmd_initial(void);

void        Fs_test(void * param)
{
    handle_t        file;
    char            buf[100];
    char          * name = "/shemox/system/kerncfg.xml";
    int             nr;
    _printf("file system test...\n");
    file = Fs_open(name,FS_OPEN_FLAG_READ);
    if( INVALID_HANDLE == file)
    {
        _printf("%s open failed!\n",name);
        return ;
    }
    _printf("%s open OK!\n",name);
    if( (nr = Fs_read(file,buf,100) ) >= 0 )
    {
        buf[nr] = 0;
        _printf("file read: %s\n",buf);
        //_mprintf(buf,100);
        
    }
    else
    {
        _printf("file read failed!\n");
    }
}

/*  2014.4.20 测试创建文件和目录  */
void        Fs_create_test(void *param)
{
    /*  创建一个新文件*/
    handle_t        file    = INVALID_HANDLE;
    char          * name    = "/open.txt";
    int             nr      = 0;

    _printf("file system create file test...\n");
    file = Fs_open(name,FS_OPEN_FLAG_READ | FS_OPEN_FLAG_WRITE);
    if( INVALID_HANDLE == file)
    {
        _printf("%s open failed!\n",name);
        return ;
    }
    _printf("%s open OK!\n",name);
}

void        User_initial(void)
{
    int         hd = 0;

    Tty_echo_hook_set(TTY_MAJOR,Con_print_char);

    //Clk_ticks_hook_set(Clk_msg);

    Shell_cmd_initial();

    Bbuf_initial();


    if( Dev_registe("ata",Ata_entry,&hd) != RESULT_SUCCEED )
        _printf("ata register failed!\n");

    Fs_initial();

    PROC_CREATE(fst,60,3,Fs_create_test,NULL);
}

void        main(void)
{
    User_initial();

}