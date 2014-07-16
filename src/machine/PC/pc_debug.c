/*
////////////////////////////////////////////////////////////////////////////////
//                             LenixǶ��ʽ����ϵͳ
//                         2011 - 2012 @ Դ���빤����
//                                �������а�Ȩ
//                     ***----------------------------***
//  ��    ��: proc.c
//  ����ʱ��: 2011-07-02        ������: �ޱ�
//  �޸�ʱ��: 2012-11-29        �޸���: �ޱ�
//  ��Ҫ����: �ṩ���̹�����
//  ˵    ��: ��16λ��32λ֮����ֲ��ʱ����Ҫע������ַ������޸ģ���Ҫ�ǳ�����
//            ���ʱ����%d��%ldѡ��
//
//  �����¼:
//  �� �� ��  |   ʱ  ��    |   ��  ��      | ��Ҫ�仯��¼
//==============================================================================
//  00.00.000 | 2011-07-02  |   ��  ��      | ��һ��
////////////////////////////////////////////////////////////////////////////////
*/

#include <config.h>

#ifdef _CFG_DEBUG_

#include <lio.h>

#ifdef _SLASH_
    #include <machine/machine.h>
    #include <machine/pc.h>
#else
    #include <machine\machine.h>
    #include <machine\pc.h>
    #include <kernel\clock.h>
    #include <kernel\proc.h>
#endif  /*  _SLASH_ */


/*
////////////////////////////////////////////////////////////////////////////////
//  ��  ��: Proc_msg
//  ��  ��: �ڿ���̨��ʾ��ǰ������Ϣ
//  ��  ��: ��
//  ����ֵ: ��
//  ע  ��: ������ʾ��Ϣ��Ҫֱ�ӷ��ʽ��̳أ������̳�Ϊ����Ϊ��̬�����Խ��ú�����
//          ���ڱ��ļ��У���Ϊ��׼�ṩ��
//
//  �����¼:
//  ʱ  ��      |  ����         |  ˵��
//==============================================================================
//  2012-01-01  |  �ޱ�         |  ��һ��
////////////////////////////////////////////////////////////////////////////////
*/
void        Proc_msg(void)
{
    char                    msg[64];    /*  ��ʾ�ַ���������                */
    proc_t              *   proc;
    int                     line;       /*  ����Ļ�ϵ��к�                  */
    int                     i;
    
    line = 23;
    
    for( i = 0 , proc = Proc_pool(); i < PROC_MAX; i++,proc++)
    {
        if( proc->proc_entry )
        {
            _sprintf(msg,"pid:%3d prio:%2d sp:%P sf:%6d name: %s \n",
                proc->proc_pid,proc->proc_priority,proc->proc_sp,
                proc->proc_sched_factor,proc->proc_name);
            Con_write_string(25,line--,msg,TEXT_COLOR_GREEN);
        }
    }
}

void        Clk_msg(void)
{
    char                    msg[40];
    _sprintf(msg,"lenix ticks: %8d",Clk_get_ticks());
    Con_write_string(50,0,msg,0x07);
    Proc_msg();
}

/*  ��Proc_sched�е���  */
void        Proc_sched_msg(void * p1,void *p2 )
{
    static uint32_t         cnt     = 0;
    char                    msg[40] = {0};

    p1 = p1;
    p2 = p2;
    _sprintf(msg,"sched times: %8d ",++cnt);

    Con_write_string(40,24,msg,TEXT_COLOR_RED|TEXT_COLOR_BLUE);
}



#endif  /*  _CFG_DEBUG_ */
