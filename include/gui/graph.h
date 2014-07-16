


//2013.01.10

#ifndef _LEGRAPH_H_
#define _LEGRAPH_H_

#include <const.h>
#include <config.h>
#include <type.h>
#include <result.h>


/*
 *  基本颜色定义
 */
#define COLOR_BLACK                 0x00000000
#define COLOR_WHITE                 0x00FFFFFF

typedef struct _resolution_t
{
    uint32_t                reslu_mode_no;      /*  分辨率模式编号  */
    uint16_t                reslu_horizontal,   /*  横向分辨率  */
                            reslu_vertical;     /*  纵向分辨率  */
}resolution_t;

typedef char *              text_t;

typedef uint32_t            color_t;    /*  直接定义为32位    */

#ifdef _CFG_ALPHA_
#define COLOR_RGB(a,r,g,b)          (((color_t)(a) & 0xFF) << 24 | \
                                     ((color_t)(r) & 0xFF) << 16 | \
                                     ((color_t)(g) & 0xFF) <<  8 | \
                                     ((color_t)(b) & 0xFF) )
#else

#define COLOR_RGB(a,r,g,b)          (((color_t)(r) & 0xFF) << 16 | \
                                     ((color_t)(g) & 0xFF) <<  8 | \
                                     ((color_t)(b) & 0xFF) )
#endif

/*  取颜色的各个分量    */
#define COLOR_ALPHA(color)          (((color) >> 24 ) & 0xFF)
#define COLOR_RED(color)            (((color) >> 16 ) & 0xFF)
#define COLOR_GREEN(color)          (((color) >>  8 ) & 0xFF)
#define COLOR_BLUE(color)           (((color)       ) & 0xFF)

/*  32位色到16位色转换  */
#define COLOR_16(color)             ( (uint16_t)\
                                      (((color) >> 8) & 0xF800 ) | \
                                      (((color) >> 5) & 0x07E0 ) | \
                                      (((color) >> 3) & 0x001F) )
#define COLOR_8(color)              

/*
 *  v : vbuf
 *  o : offset
 *  c : color
 */
#define COLOR_SET_8(v,o,c,cb)       do{ (v)[o] = (c);}while(FALSE)
#define COLOR_SET_16(v,o,c,cb)      do{ (v)[(o) / 2] = (c);}while(FALSE)
#define COLOR_SET_24(v,o,c,cb)      do{ byte_t * tc = (byte_t *)&(c); \
                                        (v)[(o) + 2] = tc[2];\
                                        (v)[(o) + 1] = tc[1];\
                                        (v)[(o) + 0] = tc[0];\
                                      }while(FALSE)

typedef struct _point_t
{
    int16_t                 pt_x;
    int16_t                 pt_y;
}point_t;

typedef struct _gsize_t
{
    uint_t                  size_x;
    uint_t                  size_y;
}gsize_t;

typedef struct _rect_t
{
    int16_t                 rect_left,
                            rect_top,
                            rect_right,
                            rect_bottom;
}rect_t;


typedef struct _pen_t
{
    color_t                 pen_color;
}pen_t;

typedef struct _brush_t
{
    color_t                 brs_color;
    int                     brs_width;
}brush_t;

typedef struct _font_t
{
    int                     font_height,
                            font_width;
}font_t;

#define VBUF_CNT                    4
#define SCREEN_CNT                  4

/*  线性物理地址转换为段:偏移式地址*/
#define PA_TO_SA(p)                 ( ((((uint32_t)(p)) & 0xF0000) * 0x1000 ) + \
                                      (((uint32_t)(p)) & 0xFFFF) )
#define SA_TA_PA(fp)                ( (((uint32_t)(fp) & 0xFFFF0000) / 0x1000) + \
                                      (uint_t)(fp)& 0xFFFF)

/*
 *  显存地址
 */
#if _CPU_WORD_ == 64 || _CPU_WORD_ == 32
typedef color_t *           vbuf_t;
#else
typedef color_t far *       vbuf_t;
#endif

typedef struct _vbuf_range_t
{
#if CPU_WORD == 64 || CPU_WORD == 32
    uint_t                  vr_start;
    uint_t                  vr_end;
#else
    uint32_t                vr_start;
    uint32_t                vr_end;
#endif
}vbuf_range_t;

/*
 *  屏幕
 */
typedef struct _screen_t
{
    int                     srn_flag;
    volatile int            srn_lock;
    vbuf_t                  srn_buffer;
}screen_t;

#define SRN_FLAG_
#define SRN_VBUF32(srn)

typedef struct _graph_t
{
    vbuf_range_t            g_vbuf_range[VBUF_CNT];       /*  显存的物理地址 */
    screen_t                g_screen[SCREEN_CNT];
    spin_lock_t             g_screen_lock;
    int                     g_bytes_of_scanline;    /*  扫描线字节数 */
    int                     g_bytes_of_x_vbuf;      /*  横轴显示缓存字节数   */
    int                     g_current_page;     /*  当前页面    */
    uint16_t                g_reslu_x,
                            g_reslu_y;

    vbuf_range_t        *   g_vr_actived;                 /*  活动显存地址  */
    int                     g_mode; /*  当前图形模式  */

    result_t           (*   g_mode_set)     (struct _graph_t *,int);
    int                (*   g_page_set)     (struct _graph_t *,int);

    void               (*   g_refresh_enable)(struct _graph_t *);
    void               (*   g_refresh_disable)(struct _graph_t *);

    /*  显示屏幕，也就是将屏幕内容复制到显存，实际完成显示  */
    result_t           (*   g_screen_display)(struct _graph_t *,screen_t *);

    /*  直接写显存的操作 */
    void               (*   g_pixel_set)    (struct _graph_t *,int,int,color_t);
    color_t            (*   g_pixel_get)    (struct _graph_t *,point_t *);

    void               (*   g_rect)         (struct _graph_t *,int,int,int,int,color_t);
    void               (*   g_fill_rect)    (struct _graph_t *,int,int,int,int,color_t);

    void               (*   g_line)         (struct _graph_t *,int,int,int,int,color_t);
    void               (*   g_circle)       (struct _graph_t *,int,int,int,int,color_t);
}graph_t;

#define VGA_MODE_320_200_256        0x13
#define VGA_MODE_640_480_16         0X12

#define VESA_MODE_640_480_256       0X101
#define VESA_MODE_640_480_32K       0X110
#define VESA_MODE_640_480_64K       0X111
#define VESA_MODE_640_480_16M       0X112


/*  需要移植  */
#define PIXEL_SET(buf,xs,x,y,color) do{ \
                                        ((color_t *)(buf))[(y)*(xs) + (x)] = color; \
                                      }while(FALSE)

#define PIXEL_GET(buf,xs,x,y,color) do{ \
                                        color = ((color_t *)(buf))[(y)*(xs) + (x)]; \
                                      }while(FALSE)


void        Graph_initial(void);
graph_t *   Graph_get(int id);

/*  获得可用屏幕  */
screen_t *  Graph_screen_get(graph_t * graph);
void        Graph_screen_put(graph_t * graph,screen_t * screen);

//  2013.1.15
pen_t *     Pen_create(pen_t * pen,color_t color);

//  2013.1.15
brush_t *   Brush_create(brush_t * brush,int width,color_t color);

int         Graph_clip_line_rect(int x1, int y1, int x2, int y2,
                                 rect_t * rect,
                                 int * nx1, int * ny1, int * nx2, int * ny2)  ;

#endif  /*  _LEGRAPH_H_ */