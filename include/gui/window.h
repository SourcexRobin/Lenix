




//2013.01.10

#ifndef _LEWINDOW_H_
#define _LEWINDOW_H_

#include <slist.h>
#include <ipc.h>
#include <gui\graph.h>

#define WINDOW_MAX                  2

#define WIN_TITLE_LEN               64
#define WIN_mb_SIZE                32


#define WIN_SUNKEN                  0
#define WIN_RAISED                  1

/*
 *  桌面默认颜色
 */
#define WINCLR_DESKTOP_DEFUALT      0x00408080

/*
 *  窗体默认颜色
 */
#define WINCLR_BACKGROUND_DEFAULT   0x00C0C0C0
/*
 *  默认窗体边框颜色
 */
#define WINCLR_BORDER               0x00A0A0A0

/*
 *  默认标题栏背景色
 */
#define WINCLR_TITLE_BAR_DEFAULT    0x000000C0
/*
 *  默认客户区背景色
 */
#define WINCLR_CLIENT_BKGD_DEF      0x00FFFFFF
/*
 *  立体显示颜色
 */
#define WINCLR_3D_LIGHT             0x00DDDDDD
#define WINCLR_3D_MID               0x00A0A0A0
#define WINCLR_3D_DARK              0x00404060

#define WIN_STYLE_BORDER            0x00000001
#define WIN_STYLE_3D                0x00000002
#define WIN_STYLE_TITLE_BAR         0x00000004
#define WIN_STYLE_FILE_BKGD         0x00000008
#define WIN_STYLE_CLIENT            0x00000010
#define WIN_STYLE_MIN
#define WIN_STYLE_MAX

typedef struct _window_t
{
    list_node_t             win_list_node;
    list_t                  win_brother,
                            win_sub;

    char                    win_title[WIN_TITLE_LEN];
    color_t                 win_color;          /*  窗体颜色  */
    color_t                 win_color_client_bkgd;/*  客户区背景色  */
    rect_t                  win_rect;           /*  窗体在屏幕的位置*/
    uint32_t                win_style;          /*  窗体风格    */
    text_t                  win_text;           /*  窗体文本    */
    message_box_t           win_msg;
    message_t               win_ms[WIN_mb_SIZE];

    pen_t               *   win_pen;
    brush_t             *   win_brush;
    font_t              *   win_font;

    point_t                 win_current_pt;     /*  当前点 */
    void               (*   win_draw)(struct _window_t *);
}window_t;

#define WIN_PEN_COLOR(window)       ((window)->win_pen->pen_color)
#define WIN_RECT_LEFT(window)       ((window)->win_rect.rect_left)
#define WIN_RECT_TOP(window)        ((window)->win_rect.rect_top)
#define WIN_RECT_RIGHT(window)      ((window)->win_rect.rect_right)
#define WIN_RECT_BOTTOM(window)     ((window)->win_rect.rect_bottom)
#define WIN_RECT_WIDTH(window)      (WIN_RECT_RIGHT(window) - WIN_RECT_LEFT(window))
#define WIN_RECT_HEIGHT(window)     (WIN_RECT_BOTTOM(window) - WIN_RECT_TOP(window))

typedef struct _button_t
{
    int i;
}button_t;

typedef struct _text_box_t
{
    int i;
}text_box_t;

typedef struct _menu_item_t
{
    struct _menu_item_t *   mi_prev,
                        *   mi_next;
    struct _menu_item_t *   mi_sub_menu;
    int                     mi_style;
    text_t                  mi_text;
}menu_item_t;

typedef struct _menu_t
{
    window_t                menu_win;
    menu_item_t         *   menu_list;
}menu_t;


window_t *  Window_create(text_t * name,window_t * parent,
                          int x,int y,uint_t width,uint_t height,uint32_t style);

void        Window_move(window_t * window,rect_t * rect,rect_t * prev);
result_t    Window_get_client_rect(window_t * window,rect_t * rect);
result_t    Window_rect(window_t * window,rect_t * rect);


point_t *   Window_position_get(window_t * window,point_t * point);
void        Window_position_set(window_t * window,point_t * point,point_t * prev);
gsize_t *   Window_size_get(window_t * window,gsize_t * size);
void        Window_size_set(window_t * window,gsize_t * size,gsize_t * prev);


void        Window_draw_line(window_t * window,point_t * point);
void        Window_draw_rect(window_t * window,rect_t * rect);
void        Window_draw_circle();
void        Window_draw_text();

void        Window_fill_rect(window_t * window,rect_t * rect,brush_t * brush);

void        Window_initial(void);

#endif  /*  _LEWINDOW_H_ */