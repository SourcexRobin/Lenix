
//2012.1.13

#include <lmemory.h>
#include <proc.h>
#include <gui\graph.h>


static graph_t              graph[2];

//2013.1.18
int         Graph_clip_line_rect(int x1, int y1, int x2, int y2,
                                 rect_t * rect,
                                 int * nx1, int * ny1, int * nx2, int * ny2)  
{
   // 创建并设置起点p1和终点p2的位置代码p1code和p2code   
   int p1code = 0;  
   int p2code = 0;  
 
   int left,top,right,bottom;

   left     = rect->rect_left;
   top      = rect->rect_top;
   right    = rect->rect_right;
   bottom   = rect->rect_bottom;

   if (y1 < top)  
       p1code |= 8;  
   else if (y1 > bottom)  
       p1code |= 4;  
 
   if (x1 < left)  
       p1code |= 1;  
   else if (x1 > right)  
       p1code |= 2;  
 
   if (y2 < top)  
       p2code |= 8;  
   else if (y2 > bottom)  
       p2code |= 4;  
 
   if (x2 < left)  
       p2code |= 1;  
   else if (x2 > right)  
       p2code |= 2;  
 
   // 过滤同侧情况   
   if ((p1code & p2code))  
       return 0;  
 
   // 完全保留   
   if (p1code == 0 && p2code == 0)  
   {  
       *nx1 = x1;  
       *ny1 = y1;  
       *nx2 = x2;  
       *ny2 = y2;  
       return 1;  
   }  
     
   // 计算np1的坐标*nx1, *ny1   
   switch(p1code)  
   {  
   case 0: // C   
       *nx1 = x1;  
       *ny1 = y1;  
       break;  
   case 8: // N   
       *ny1 = top;  
       *nx1 = (int)(x1 + (top - y1) * (x2 - x1) / (double)(y2 - y1) + 0.5);  
       break;  
   case 4: // S   
       *ny1 = bottom;  
       *nx1 = (int)(x1 + (bottom - y1) * (x2 - x1) / (double)(y2 - y1) + 0.5);  
       break;  
   case 1: // W   
       *nx1 = left;  
       *ny1 = (int)(y1 + (left - x1) * (y2 - y1) / (double)(x2 - x1) + 0.5);  
       break;  
   case 2: // E   
       *nx1 = right;  
       *ny1 = (int)(y1 + (right - x1) * (y2 - y1) / (double)(x2 - x1) + 0.5);  
       break;  
   case 9: // NW   
       // 先和求N的一样   
       *ny1 = top;  
       *nx1 = (int)(x1 + (top - y1) * (x2 - x1) / (double)(y2 - y1) + 0.5);  
 
       // 然后判断结果   
       if (*nx1 < left || *nx1 > right) // 上面的假设错误，需要算与左边线的交点   
       {  
           *nx1 = left;  
           *ny1 = (int)(y1 + (left - x1) * (y2 - y1) / (double)(x2 - x1) + 0.5);  
       }  
       break;  
   case 10: // NE   
       *ny1 = top;  
       *nx1 = (int)(x1 + (top - y1) * (x2 - x1) / (double)(y2 - y1) + 0.5);  
 
       if (*nx1 < left || *nx1 > right)  
       {  
           *nx1 = right;  
           *ny1 = (int)(y1 + (right - x1) * (y2 - y1) / (double)(x2 - x1) + 0.5);  
       }  
       break;  
   case 6: // SE   
       *ny1 = bottom;  
       *nx1 = (int)(x1 + (bottom - y1) * (x2 - x1) / (double)(y2 - y1) + 0.5);  
 
       if (*nx1 < left || *nx1 > right)  
       {  
            *nx1 = right;  
            *ny1 = (int)(y1 + (right - x1) * (y2 - y1) / (double)(x2 - x1) + 0.5);  
       }  
       break;  
    case 5: // SW   
        *ny1 = bottom;  
        *nx1 = (int)(x1 + (bottom - y1) * (x2 - x1) / (double)(y2 - y1) + 0.5);  
  
        if (*nx1 < left || *nx1 > right)  
        {  
            *nx1 = left;  
            *ny1 = (int)(y1 + (left - x1) * (y2 - y1) / (double)(x2 - x1) + 0.5);  
        }  
        break;  
    }  
  
    // 计算np2的坐标*nx2, *ny2   
    switch(p2code)  
    {  
    case 0: // C   
        *nx2 = x2;  
        *ny2 = y2;  
        break;  
    case 8: // N   
        *ny2 = top;  
        *nx2 = (int)(x1 + (top - y1) * (x2 - x1) / (double)(y2 - y1) + 0.5);  
        break;  
    case 4: // S   
        *ny2 = bottom;  
        *nx2 = (int)(x1 + (bottom - y1) * (x2 - x1) / (double)(y2 - y1) + 0.5);  
        break;  
    case 1: // W   
        *nx2 = left;  
        *ny2 = (int)(y1 + (left - x1) * (y2 - y1) / (double)(x2 - x1) + 0.5);  
        break;  
    case 2: // E   
        *nx2 = right;  
        *ny2 = (int)(y1 + (right - x1) * (y2 - y1) / (double)(x2 - x1) + 0.5);  
        break;  
    case 9: // NW   
        
        // 先和求N的一样   
        *ny2 = top;  
        *nx2 = (int)(x1 + (top - y1) * (x2 - x1) / (double)(y2 - y1) + 0.5);  
  
        // 然后判断结果   
        if (*nx2 < left || *nx2 > right) // 上面的假设错误，需要算与左边线的交点   
        {  
            *nx2 = left;  
            *ny2 = (int)(y1 + (left - x1) * (y2 - y1) / (double)(x2 - x1) + 0.5);  
        }  
        break;  
    case 10: // NE   
        *ny2 = top;  
        *nx2 = (int)(x1 + (top - y1) * (x2 - x1) / (double)(y2 - y1) + 0.5);  
  
        if (*nx2 < left || *nx2 > right)  
        {  
            *nx2 = right;  
            *ny2 = (int)(y1 + (right - x1) * (y2 - y1) / (double)(x2 - x1) + 0.5);  
        }  
        break;  
    case 6: // SE   
        *ny2 = bottom;  
        *nx2 = (int)(x1 + (bottom - y1) * (x2 - x1) / (double)(y2 - y1) + 0.5);  
  
        if (*nx2 < left || *nx2 > right)  
        {  
            *nx2 = right;  
            *ny2 = (int)(y1 + (right - x1) * (y2 - y1) / (double)(x2 - x1) + 0.5);  
        }  
        break;  
    case 5: // SW   
        *ny2 = bottom;  
        *nx2 = (int)(x1 + (bottom - y1) * (x2 - x1) / (double)(y2 - y1) + 0.5);  
  
        if (*nx2 < left || *nx2 > right)  
        {  
            *nx2 = left;  
            *ny2 = (int)(y1 + (left - x1) * (y2 - y1) / (double)(x2 - x1) + 0.5);  
        }  
        break;  
    }  
  
    // 过滤一种直线与矩形不相交的特殊情况   
    if (*nx1 < left   || *nx1 > right   || *ny1 < top    || *ny1 > bottom  ||  
        *nx2 < left   || *nx2 > right   || *ny2 < top    || *ny2 > bottom)  
    {  
        return 0;  
    }  
  
    return 1;  
} 

//  2013.1.18  使用具体的物理地址
result_t    Graph_set_vbuf(graph_t * graph,int id)
{
    if( id < 0 || id > VBUF_CNT )
        return RESULT_FAILED;
    
    graph->g_vr_actived = graph->g_vbuf_range + id;

    return RESULT_FAILED;
}

//  2013.1.18
vbuf_range_t  *   Graph_vr_get(graph_t * graph,vbuf_range_t  *vr)
{
    *vr = *graph->g_vr_actived;

    return vr;
}

//2013.01.14
void    Graph_initial(void)
{
    _memzero(graph,sizeof(graph_t) * 2);
}
//2013.01.13
graph_t *   Graph_get(int id)
{
    id = id;
    return graph;
}

//2013.01.14
screen_t *  Graph_screen_get(graph_t * graph)
{
    screen_t            *   screen  = NULL;
    int                     i       = 0;
    CRITICAL_DECLARE(graph->g_screen_lock);


    screen = graph->g_screen;

    CRITICAL_BEGIN();
    
    for( ; i < SCREEN_CNT ; i++,screen++)
    {
        if( screen->srn_lock == 0 )
        {
            screen->srn_lock = 1;
            break;
        }
    }
    CRITICAL_END();

    if( i >= SCREEN_CNT)
        return NULL;

    return screen;
}

//2013.01.14
void        Graph_screen_put(graph_t * graph ,screen_t * screen)
{
    CRITICAL_DECLARE(graph->g_screen_lock);

    CRITICAL_BEGIN();
    
    screen->srn_lock = 0;

    CRITICAL_END();

    graph = graph;
}

//  2013.1.15
pen_t *     Pen_create(pen_t * pen,color_t color)
{
    if( NULL == pen )
        return NULL;

    pen->pen_color = color;

    return pen;
}

//  2013.1.15
brush_t *   Brush_create(brush_t * brush,int width,color_t color)
{
    if( NULL == brush )
        return NULL;

    brush->brs_color = color;
    brush->brs_width = width;

    return brush;
}

