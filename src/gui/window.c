

//2012.1.13

#include <lmemory.h>
#include <assert.h>

#include <gui\window.h>

/*
 *  系统提供的固定对象，方便使用
 */
const pen_t                 win_pen[2];
const brush_t               win_brush[2];


static window_t             win_pool[WINDOW_MAX];

static window_t             win_system;         /*  系统窗体    */
static window_t             win_task_bar;       /*  任务栏窗口   */

static window_t         *   win_focus;          /*  焦点窗口，接收消息的窗体  */

static color_t              win_desktop;        /*  桌面底色    */
static color_t              win_background;     /*  窗体背景色  */
static color_t              win_color_title_bar;

static int                  win_task_bar_height;

static graph_t          *   win_graph ;

// 2013.1.19
//static
//result_t    Winlist_instert(window_t * winlist,window_t * window)
//{
//    return RESULT_FAILED;
//}
//// 2013.1.19
//static 
//result_t    Winlist_delete(window_t * winlist,window_t * window)
//{
//    return RESULT_FAILED;
//}
//// 2013.1.19
//static 
//result_t    Winlist_add_head(window_t * winlist,window_t * window)
//{
//    return RESULT_FAILED;
//}
//// 2013.1.19
//static
//result_t    Winlist_add_tail(window_t * winlist,window_t * window)
//{
//    return RESULT_FAILED;
//}
//// 2013.1.19
//static
//window_t *  Winlist_get_head(window_t * winlist)
//{
//    return RESULT_FAILED;
//}
//// 2013.1.19
//static
//window_t *  Winlist_get_tail(window_t * winlist)
//{
//    return RESULT_FAILED;
//}
//
//// 2013.1.19
//void        Window_post_message(window_t * window,uint_t msg,
//                                uint32_t param32,uint64_t param64)
//{
//}

// 2013.1.19
window_t *  Window_get_focus(void)
{
    return win_focus;
}

// 2013.1.19
static
window_t *  Window_alloc(void)
{
    return NULL;
}

// 2013.1.19
static
result_t    Window_free(window_t * window)
{
    window = window;
    return RESULT_FAILED;
}

// 2013.1.19
window_t *  Window_create(text_t * name,window_t * parent,
                          int x,int y,uint_t width,uint_t height,
                          uint32_t style)
{
    window_t            *   window      = NULL;

    name = name;
    if( NULL == ( window = Window_alloc() ) )
        return NULL;

    /*
     *  如果父窗口为NULL，则用系统窗体作为父窗体
     *  但应该是用进程的主窗体做父窗体 
     */
    if( NULL == parent ) 
        parent = &win_system;
    
    //WIN_INSERT(parent->win_sub_window,window);

    WIN_RECT_LEFT(window)           = x;
    WIN_RECT_TOP(window)            = y;
    WIN_RECT_RIGHT(window)          = x + width;
    WIN_RECT_BOTTOM(window)         = y + height;
    window->win_style               = style;
    window->win_pen                 = (pen_t *)win_pen;
    window->win_brush               = (brush_t *)win_brush;
    window->win_color_client_bkgd   = WINCLR_CLIENT_BKGD_DEF;

    return window;
}



// 2013.1.19
void        Window_move(window_t * window,rect_t * rect,rect_t * prev)
{
    if( prev)
        *prev = window->win_rect;
    window->win_rect = *rect;
}

// 2013.1.19
result_t    Window_get_client_rect(window_t * window,rect_t * rect)
{
    if( NULL == window || NULL == rect )
        return RESULT_FAILED;
    
    Window_rect(window,rect);

    if( window->win_style & WIN_STYLE_3D )
    {
        rect->rect_right    -= 8;
        rect->rect_bottom   -= 8;
    }

    if( window->win_style & WIN_STYLE_CLIENT )
    {
        rect->rect_right    -= 6;
        rect->rect_bottom   -= 6;
    }

    if( window->win_style & WIN_STYLE_TITLE_BAR )
        rect->rect_bottom   -= win_task_bar_height + 2;

    return RESULT_SUCCEED;
}


// 2013.1.19
point_t *   Window_position_get(window_t * window,point_t * point)
{
    point->pt_x = WIN_RECT_LEFT(window);
    point->pt_y = WIN_RECT_TOP(window);

    return point;
}

// 2013.1.19
void        Window_position_set(window_t * window,point_t * point,point_t * prev)
{
    if( prev )
        return ;
    window = window;
    point = point;
}

// 2013.1.19
gsize_t *   Window_size_get(window_t * window,gsize_t * size)
{
    size->size_x = WIN_RECT_WIDTH(window); 
    size->size_y = WIN_RECT_HEIGHT(window); 

    return size;
}

// 2013.1.19
void        Window_size_set(window_t * window,gsize_t * size,gsize_t * prev)
{
    int                     cx      = 0,
                            cy      = 0;

    ASSERT(window);
    ASSERT(size);

    if( prev )
    {
        prev->size_x = WIN_RECT_WIDTH(window); 
        prev->size_y = WIN_RECT_HEIGHT(window); 
    }

    window->win_rect.rect_right     = window->win_rect.rect_right + size->size_x; 
    window->win_rect.rect_bottom    = window->win_rect.rect_top + size->size_y; 
}

// 2013.1.18
void        Window_line(window_t * window,point_t * point)
{
    int x1,y1,x2,y2;

    x1 = window->win_current_pt.pt_x;
    y1 = window->win_current_pt.pt_y;
    x2 = point->pt_x;
    y2 = point->pt_y;

    window->win_current_pt = *point;

    if( Graph_clip_line_rect(x1+ WIN_RECT_LEFT(window),y1 + WIN_RECT_TOP(window),
        x2+ WIN_RECT_LEFT(window),y2 + WIN_RECT_TOP(window),
        &window->win_rect,
        &x1,&y1,&x2,&y2) )
    {
        win_graph->g_line(win_graph,x1 ,y1,x2 ,y2,WIN_PEN_COLOR(window));
    }

}

// 2013.1.17
void        Window_draw_border(window_t * window)
{
    int left,top,right,bottom;
        
    left    = window->win_rect.rect_left;
    right   = window->win_rect.rect_right;
    top     = window->win_rect.rect_top;
    bottom  = window->win_rect.rect_bottom;

    win_graph->g_line(win_graph,left,top,right,top,COLOR_BLACK);
    win_graph->g_line(win_graph,left,bottom - 1,right,bottom - 1, COLOR_BLACK);   
    win_graph->g_line(win_graph,left,top,left,bottom, COLOR_BLACK);   
    win_graph->g_line(win_graph,right - 1,top,right - 1,bottom, COLOR_BLACK);   
}

// 2013.1.17
void        Window_draw_frame(window_t * window,rect_t * rect,int flag)
{
    int left,top,right,bottom;

    left    = WIN_RECT_LEFT(window)     + rect->rect_left     ;
    right   = WIN_RECT_LEFT(window)     + rect->rect_right    ;
    top     = WIN_RECT_TOP(window)      + rect->rect_top      ;
    bottom  = WIN_RECT_TOP(window)      + rect->rect_bottom   ;

    if( flag )
    {
        /*  画上边界 */
        win_graph->g_line(win_graph,left,top,right,top,WINCLR_3D_LIGHT);
        win_graph->g_line(win_graph,left,top + 1,right,top + 1,WINCLR_3D_LIGHT);   

        /*  画下边界 */
        win_graph->g_line(win_graph,left,bottom - 1,right,bottom - 1,WINCLR_3D_DARK);   
        win_graph->g_line(win_graph,left,bottom - 2,right,bottom - 2,WINCLR_3D_MID);   

        /*  画左边界 */
        win_graph->g_line(win_graph,left,top,left,bottom - 1,WINCLR_3D_LIGHT);   
        win_graph->g_line(win_graph,left + 1,top + 1,left + 1,bottom - 2,WINCLR_3D_LIGHT);   

        /*  画右边界 */
        win_graph->g_line(win_graph,right - 1,top,right - 1,bottom,WINCLR_3D_DARK);   
        win_graph->g_line(win_graph,right - 2,top + 1,right - 2,bottom - 1,WINCLR_3D_MID);
    }
    else
    {
        /*  画上边界 */
        win_graph->g_line(win_graph,left,top,right,top,WINCLR_3D_DARK);
        win_graph->g_line(win_graph,left,top + 1,right,top + 1, WINCLR_3D_MID);   

        /*  画下边界 */
        win_graph->g_line(win_graph,left,bottom - 1,right,bottom - 1, WINCLR_3D_LIGHT);   
        win_graph->g_line(win_graph,left,bottom - 2,right,bottom - 2, WINCLR_3D_LIGHT);   

        /*  画左边界 */
        win_graph->g_line(win_graph,left,top,left,bottom,WINCLR_3D_DARK);   
        win_graph->g_line(win_graph,left + 1,top + 1,left + 1,bottom - 1,WINCLR_3D_MID);   

        /*  画右边界 */
        win_graph->g_line(win_graph,right - 1,top,right - 1,bottom, WINCLR_3D_LIGHT);   
        win_graph->g_line(win_graph,right - 2,top + 1,right - 2,bottom - 1,WINCLR_3D_LIGHT);   
    }
}

//  2013.1.19 绘制标题栏
static
void        Window_title_bar(window_t * window)
{
    int                     left,
                            top,
                            right,
                            bottom;

    left    = WIN_RECT_LEFT(window)     + 2;
    right   = WIN_RECT_RIGHT(window)    - 2;
    top     = WIN_RECT_TOP(window)      + 2;
    bottom  = WIN_RECT_TOP(window)      + 2 + win_task_bar_height ;

    /* 如果定义了3D显示，需要调整，*/
    if( window->win_style & WIN_STYLE_3D )
    {
        left        += 2;
        right       -= 2;
        top         += 2;
        bottom      += 2;
    }

    win_graph->g_fill_rect(win_graph,left,top,right,bottom,win_color_title_bar);
    win_graph->g_rect(win_graph,--left,--top,++right,++bottom,win_background);
    win_graph->g_rect(win_graph,--left,--top,++right,bottom,win_background);

}

//  2013.1.19
void        Window_client(window_t * window)
{
    rect_t                  rect = {0};
    int                     left,
                            top,
                            right,
                            bottom;
    int                     i;

    /*
     *  画四个边界
     */
    Window_get_client_rect(window,&rect);

    left    = WIN_RECT_LEFT(window);
    top     = WIN_RECT_TOP(window);
    right   = WIN_RECT_RIGHT(window);
    bottom  = WIN_RECT_BOTTOM(window);

    if( window->win_style & WIN_STYLE_TITLE_BAR )
    {
        top                 += win_task_bar_height + 2;
        
        rect.rect_top       += win_task_bar_height + 2;
        rect.rect_bottom    += win_task_bar_height + 2;
    }

    if( window->win_style & WIN_STYLE_3D )
    {
        left                += 5;
        top                 += 5;
        right               -= 5;
        bottom              -= 4;

        rect.rect_left      += 5;
        rect.rect_top       += 5;
        rect.rect_right     += 9;
        rect.rect_bottom    += 10;
        Window_draw_frame(window,&rect,WIN_SUNKEN);
    }

    win_graph->g_fill_rect(win_graph,left + 2,top + 2,right - 2,bottom - 2,
        window->win_color_client_bkgd);

    for( i = 0 ; i < 3 ; i++)
    {
        left--;
        top--;
        right++;
        bottom++;
        win_graph->g_rect(win_graph,left,top,right,bottom,window->win_color);
    }
}

//  2013.1.19
result_t    Window_rect(window_t * window,rect_t * rect)
{
    if( NULL == window || NULL == rect )
        return RESULT_FAILED;

    rect->rect_left     = 0;
    rect->rect_top      = 0;
    rect->rect_right    = WIN_RECT_WIDTH(window);
    rect->rect_bottom   = WIN_RECT_HEIGHT(window);

    return RESULT_SUCCEED;
}
// 2013.1.17
void        Window_draw(window_t * window)
{
    rect_t                  rect;
    /*  填充窗体背景  */
    if( window->win_style & WIN_STYLE_FILE_BKGD )
        win_graph->g_fill_rect(win_graph,
            window->win_rect.rect_left,window->win_rect.rect_top,
            window->win_rect.rect_right,window->win_rect.rect_bottom,
            window->win_color);

    /*  3D显示   */
    if( window->win_style & WIN_STYLE_3D )
    {
        Window_rect(window,&rect);
        Window_draw_frame(window,&rect,WIN_RAISED);
    }

    /*  标题栏   */
    if( window->win_style & WIN_STYLE_TITLE_BAR )
        Window_title_bar(window);

    /*  3D显示   */
    if( window->win_style & WIN_STYLE_CLIENT )
        Window_client(window);

    /*  画窗体边框   */
    if( window->win_style & WIN_STYLE_BORDER )
        Window_draw_border(window);
}

// 2013.1.17
void        Window_initial(void)
{
    window_t                * window = win_pool;

    _memzero(win_pool,sizeof(window_t) * WINDOW_MAX);
    _memzero(&win_system,sizeof(window_t));
    _memzero(&win_task_bar,sizeof(window_t));

    win_desktop             = WINCLR_DESKTOP_DEFUALT;
    win_background          = WINCLR_BACKGROUND_DEFAULT;
    win_color_title_bar     = WINCLR_TITLE_BAR_DEFAULT;

    win_task_bar_height     = 20;

    /*  主窗体无边框狂，平面  */
    win_system.win_rect.rect_right      = 640;
    win_system.win_rect.rect_bottom     = 480;
    win_system.win_color                = WINCLR_DESKTOP_DEFUALT;
    win_system.win_style                = WIN_STYLE_FILE_BKGD;

    /*  任务栏无边框狂，立体  */
    win_task_bar.win_rect.rect_left     = 0;
    win_task_bar.win_rect.rect_top      = 480 - 2 - win_task_bar_height;
    win_task_bar.win_rect.rect_right    = 640;
    win_task_bar.win_rect.rect_bottom   = 480 + 2;

    win_task_bar.win_color              = WINCLR_BACKGROUND_DEFAULT;
    win_task_bar.win_style              = WIN_STYLE_FILE_BKGD | WIN_STYLE_3D;
    
    window->win_rect.rect_left          = 200;
    window->win_rect.rect_top           = 100;
    window->win_rect.rect_right         = 400;
    window->win_rect.rect_bottom        = 270;
    window->win_color_client_bkgd       = WINCLR_CLIENT_BKGD_DEF;
    window->win_color                   = WINCLR_BACKGROUND_DEFAULT;
    window->win_style                   = WIN_STYLE_3D | WIN_STYLE_TITLE_BAR | 
                                          WIN_STYLE_CLIENT;
    
    win_graph = Graph_get(0);

    /*  先画桌面，在画任务栏  */
    Window_draw(&win_system);
    Window_draw(&win_task_bar);
    Window_draw(window);
}