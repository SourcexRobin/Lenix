/*
///////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2014 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: fs_test.c  
//  ����ʱ��: 2014-03-09        ������: �ޱ�
//  �޸�ʱ��:                   �޸���: 
//  ��Ҫ����: �黺�����
//  ˵    ��: 
//
//  �����¼:
//  �� �� ��    |     ʱ��      |  ��  ��       |  ��Ҫ�仯��¼
//=============================================================================
//  00.00.000   |  2014-03-09   |  ��  ��       |  �����ļ�
///////////////////////////////////////////////////////////////////////////////
*/

#include <lenix.h>
#include <blkbuf.h>
#include "fs.h"

STACK_DEFINE(fst);

/*  2014-03-11  ����ʽʱ��ת��Ϊ�̸�ʽʱ��  */
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

/*  2014.4.20 ���Դ����ļ���Ŀ¼  */
void        Fs_create_test(void *param)
{
    /*  ����һ�����ļ�*/
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