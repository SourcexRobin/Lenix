/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      Lenix
//                                                  Ƕ��ʽ����ϵͳ
//                                           2011 , ��  ��Դ���빤����
//                                                   �������а�Ȩ
//
//  �ļ�����    : template.c
//  �ļ�������  : 
//  ����޸���  : ��  ��
//  ����޸�ʱ��:
//
//  ��Ҫ����    :
//
//  ˵��        : ��Ӧ�ó����ṩһ��ģ�壬ʹ��ʱ�޸�Ϊ���ļ����޸�Ϊuserapp.c����
//
//  �汾�仯��¼:
//  �汾��      |     ʱ��      |  ����         |  ��Ҫ�仯��¼
//======================================================================================================================
//  00.00.000   |   2011-02-09  |  �ޱ�         |  xxxxxx
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
#include <dos.h>
#include <lenix.h>

#define USER_APP_STACK              2048

byte_t                      app_stack1[USER_APP_STACK];
byte_t                      app_stack2[USER_APP_STACK];


void        User_initial(void);
result_t    Vga_initial(graph_t * graph);
void        Con_print_char(byte_t c);

void        app1(void * param)
{
    graph_t             *   graph = Graph_get(0);
    param = param;

    graph->g_line(graph,111,360,640,0,0);
    graph->g_line(graph,0,0,100,100,0);
    graph->g_line(graph,100,0,0,100,0);

    //screen_t            *   screen = NULL;
    //int x,y,i;
    //param = param;
    //screen = screen;
    //if( Vga_initial(graph) != RESULT_SUCCEED)
    //    return ;

    //graph->g_mode_set(graph,VESA_MODE_640_480_64K);

    //screen = Graph_screen_get(graph);

    ////_printf("%lp %lp\n",screen->srn_buffer,PA_TO_SA(graph->g_vbuf_range[0].vr_start));
    //
    ////graph->g_screen_display(graph,screen);

    //graph->g_fill_rect(graph,0,0,640,480,COLOR_RGB(192,192,192));
    //graph->g_line(graph,10,10,150,10,8);
    //graph->g_line(graph,50,150,50,11,COLOR_RGB(255,0,0));


    ////graph->g_fill_rect(graph,10,10,150,150,1);
    ////Graph_screen_put(graph,screen);
}


int         main(void)
{
    //Lenix_initial();

    User_initial();

    //Lenix_start();

    return 1;
}

void        User_initial(void)
{
    //Tty_echo_hook_set(TTY_MAJOR,Con_print_char);
    graph_t             *   graph = Graph_get(0);
    Graph_initial();
    
    if( Vga_initial(graph) != RESULT_SUCCEED)
        return ;

    graph->g_mode_set(graph,VESA_MODE_640_480_64K);

    Window_initial();

    app1(NULL);
    //Proc_create("app1",60,3,app1,0,
    //    MAKE_STACK(app_stack1,USER_APP_STACK),
    //    STACK_SIZE(app_stack1,USER_APP_STACK));

}