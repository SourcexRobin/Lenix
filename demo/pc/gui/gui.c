/*  �ļ�ʱ�� 2013.1.13   */

#include <dos.h>
#include <lenix.h>

#define RESLU_X                     320
#define RESLU_Y                     200

color_t far                 m_vbuf0[1];
color_t far                 m_vbuf1[1];
color_t far                 m_vbuf2[1];
color_t far                 m_vbuf3[1];

color_t far             *   m_vbuf[4] = {m_vbuf0,m_vbuf1,m_vbuf2,m_vbuf3};


int         Vga_page_set(graph_t * graph,int n)
{
    int         page;
    union REGS inregs;    
    

    page = graph->g_current_page;
    
    inregs.x.bx = 0;
    inregs.x.dx = n;
    inregs.x.ax = 0x4F05;

    int86(0x10,&inregs,&inregs);

    if( inregs.x.ax != 0x004F ) //���ò��ɹ�  
        return -1;
    
    graph->g_current_page = n;

    return page;
}

void        Vga_display_enable(void)
{
}

void        Vga_refresh_disable(void)
{
}


#define PIXEL_SET_256()         do{ uint_t offset  = 0; \
                                    uint_t seg     = RESLU_X * 50; \
                                    vbuf = (byte_t far *)PA_TO_SA(graph->g_vbuf_range[0].vr_start);\
                                    offset = (uint_t)pt.pt_x * (uint_t)pt.pt_y;\
                                    m_vbuf[offset / seg][offset % seg] = color;\
                                   }while(FALSE)

void        Vga_pixel_set_256(graph_t * graph,int x,int y,color_t color)
{
    uint_t                  offset      = x * y;
    uint_t                  seg         = RESLU_X * 50;

    graph = graph;

    if( x < 0 || y < 0 || x > graph->g_reslu_x || y > graph->g_reslu_y )
        return ;
        
    m_vbuf[offset / seg][offset % seg] = color;
}



result_t    Vga_screen_display_256(graph_t * graph,screen_t * screen)
{
    byte_t far          *   vbuf    = NULL;
    uint_t                     i       = 0;
    uint_t n;
    uint_t seg = 0;
    vbuf    = (byte_t far *)PA_TO_SA(graph->g_vbuf_range[0].vr_start);
    screen = screen;
    //mvbuf   = screen->srn_buffer;
        
    //_printf("%lp %lp\n",vbuf,mvbuf); 

    for( n = 0 ; n < 4 ; n++)
    {
        seg = n * RESLU_X * 50;

        for( i = 0; i < RESLU_X * 50 ; i++)
        {
            vbuf[seg + i] = (byte_t)m_vbuf[n][i];
        }
    }

    return RESULT_FAILED;
}

void        Vga_rect(graph_t * graph,int left,int top,int right,int bottom,color_t color)
{
    /*  ��ʾ˳�� ���ϣ����ϣ����£����£�����  * /
    byte_t                  dflag[4]    = {1,1,1,1};   /* 4���߶�Ҫ��ʾ  * /
    byte_t far          *   vbuf        = NULL;
    point_t                 pt[4]       = {0};
    int                     n           = 0;    /*  ��Ҫ��ʾ�ĵ���  */

    if( left > right )
        SWAP(int,left,right);
    if( top > bottom )
        SWAP(int,top,bottom);

    graph->g_line(graph,left,top,right,top,color);
    graph->g_line(graph,right - 1,top,right - 1,bottom,color);
    graph->g_line(graph,right,bottom - 1,left,bottom - 1,color);
    graph->g_line(graph,left,bottom,left,top,color);


}

void        Vga_fill_rect(graph_t * graph,int left,int top,int right,int bottom,color_t color)
{
    if( left > right )
        SWAP(int,left,right);
    if( top > bottom )
        SWAP(int,top,bottom);

    for( ; top < bottom ; top++)
        graph->g_line(graph,left,top,right,top,color);
}


void        Vga_line256(graph_t * graph,int x1,int y1,int x2,int y2,color_t color)
{
    byte_t far          *   vbuf    = NULL;
    //int                     x,y;
    int                     cx = 0,
                            cy = 0;
    byte_t  clr = (byte_t)color;

    cx      = graph->g_reslu_x;
    cy      = graph->g_reslu_y;
    vbuf    = (byte_t far *)PA_TO_SA(graph->g_vbuf_range[0].vr_start);
    

    /*  ˮƽ��  */
    if( y1 == y2 )
    {
        if( x1 > x2 )
            SWAP(int,x1,x2);

        if( x1 < 0 ) x1 = 0;
        if( x2 > cx) x2 = cx;

        y1 = y1 * cx + x1;

        while( x1 <= x2 )
        {
            vbuf[y1++] = clr;
            x1++;
        }

        return ;
    }

    /*  ��ֱ��  */
    if( x1 == x2 )
    {
        if( y1 > y2 )
            SWAP(int,y1,y2);

        if( y1 < 0 ) y1 = 0;
        if( y2 > cy) y2 = cy;

        x1 = y1 * cx + x1;
        while( y1 <= y2 )
        {
            vbuf[x1] = clr;
            x1 += cx;
            y1++;
        }
        return ;
    }

    /*  ��֤�������һ���  */
    if( x1 > x2 )
    {
        SWAP(int,x1,x2);
        SWAP(int,y1,y2);
    }

}
/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  64Kɫ���� 
*/

//  cb��ɫռ���ֽڵ���λ��
#define VGA_PIXEL_SET(graph,cb,vbuf,ptr,color)   do{ \
        uint_t offset,page;\
        page    = (uint_t)(ptr >> 16);\
        offset  = (uint_t)ptr;\
        if( page != graph->g_current_page ) Vga_page_set(graph,page);\
        COLOR_SET_##cb(vbuf,offset,color,cb);\
    }while(FALSE)

#define VGA_XY_TO_PTR(graph,x,y,cb) ( (uint32_t)((graph)->g_bytes_of_scanline) * \
                                      ((uint32_t)(y)) + (x) * ((cb) / 8))

void        Vga_pixel_set_64K(graph_t * graph,int x,int y,color_t color)
{
    uint32_t                ptr         = x * y;
    uint16_t far        *   vbuf        = NULL;
    uint16_t                tc      = COLOR_16(color);

    /*
     *  ������Ļ��Χ,
     */
    if( x < 0 || y < 0 || x > graph->g_reslu_x || y > graph->g_reslu_y )
        return ;

    ptr     = VGA_XY_TO_PTR(graph,x,y,16);
    vbuf    = (uint16_t far *)PA_TO_SA(graph->g_vr_actived->vr_start);

    VGA_PIXEL_SET(graph,16,vbuf,ptr,tc);
}

#define ABS(a)                      ((a) < 0 ? -(a) : a)
void        Vga_line_64K(graph_t * graph,int x1,int y1,int x2,int y2,color_t color)
{
    uint16_t far        *   vbuf    = NULL;
    uint32_t                ptr     = 0;
    uint16_t                tc      = COLOR_16(color);

    int                     dx      = 0, 
                            dy      = 0;
    int                     x_inc   = 2, 
                            y_inc   = 0 ;   /* �������ۼӱ��� */
    int                     interchange,loop,p;

    dx      = graph->g_reslu_x;
    dy      = graph->g_reslu_y;
    vbuf    = (uint16_t far *)PA_TO_SA(graph->g_vr_actived->vr_start);
    

    /*  ˮƽ��  */
    if( y1 == y2 && y1 < dy)
    {
        if( x1 > x2 )
            SWAP(int,x1,x2);

        if( x1 < 0 ) x1 = 0;
        if( x2 > dx) x2 = dx;

        ptr = VGA_XY_TO_PTR(graph,x1,y2,16);

        while( x1 < x2 )
        {
            VGA_PIXEL_SET(graph,16,vbuf,ptr,tc);
            ptr += 2;
            x1++;
        }

        return ;
    }

    /*  ��ֱ��  */
    if( x1 == x2 && x1 < dx)
    {
        if( y1 > y2 )
            SWAP(int,y1,y2);

        if( y1 < 0 ) y1 = 0;
        if( y2 > dy) y2 = dy;

        ptr = VGA_XY_TO_PTR(graph,x1,y1,16);
        dx *= 2;
        while( y1 < y2 )
        {
            VGA_PIXEL_SET(graph,16,vbuf,ptr,tc);
            ptr += dx;
            y1++;
        }
        return ;
    }

    /*
     *  ��һ���ֱ��,��֤�������һ���
     */
    if( x1 > x2 )
    {
        SWAP(int,x1,x2);
        SWAP(int,y1,y2);
    }

    /*
     *  ������ֵֻ�����ֵ�й�
     */
    dx = ABS(x2 - x1);
    dy = ABS(y2 - y1); 

    if (y2 - y1 > 0)
        y_inc = graph->g_bytes_of_scanline;
    else
        y_inc = -graph->g_bytes_of_scanline;

    /*
     *  ����ʼ��
     */
    ptr = VGA_XY_TO_PTR(graph,x1,y1,16);
    VGA_PIXEL_SET(graph,16,vbuf,ptr,tc);

    /*
     *  �������޵Ľ�ƽ����
     */
    if (dx == dy)
    {
        for (loop = 0; loop < dx; loop++)
        {
            ptr += x_inc;
            ptr += y_inc;
            VGA_PIXEL_SET(graph,16,vbuf,ptr,tc);
        }
    }
    else
    {
        /*
         *  ����һ�������µ�ֱ�߻���
         *  ��б�ʾ���ֵС��1,���轻��
         */
        if (dx > dy)
            interchange = FALSE;
        // ��б�ʾ���ֵ����1,����dx��dyִ�м���,���������ۼ�ʱ��Ҫ����Ӧ���
        else
        {
            SWAP(int,dx,dy);
            interchange = TRUE;
        }

        p = - dx;
        for (loop = 0; loop < dx; loop++)
        {
            // ���ȴ���������
            if (interchange)
                ptr += y_inc;    // ��������x��y��ÿ��ѭ������yֵ
            else
                ptr += x_inc;    // ��û�н���,��ÿ��ѭ������xֵ

            p += 2 * dy;    // ��Ϊÿ��ѭ������Ҫ����2dy,��ʼֵʱ��Ҫ������������

            if (p >= 0)
            {// ��ʱ��Ҫ������һ����
                if (interchange)
                    ptr += x_inc;
                else
                    ptr += y_inc;

                p -= 2 * dx;
            }
            VGA_PIXEL_SET(graph,16,vbuf,ptr,tc);
        }
    }
}
//void        Vga_line(graph_t * graph,int x,int y,color_t color)
//{
//}

/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  64Kɫ���� 
*/

void        Vga_pixel_set_16M(graph_t * graph,int x,int y,color_t color)
{
    uint32_t                ptr         = 0;
    byte_t far          *   vbuf        = NULL;

    /*
     *  ������Ļ��Χ,
     */
    if( x < 0 || y < 0 || x > graph->g_reslu_x || y > graph->g_reslu_y )
        return ;

    ptr     = VGA_XY_TO_PTR(graph,x,y,24);
    vbuf    = (byte_t far *)PA_TO_SA(graph->g_vr_actived->vr_start);

    VGA_PIXEL_SET(graph,24,vbuf,ptr,color);
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
////////////////////
//  ͨ�ò��� 
*/
result_t    Vga_mode_set(graph_t * graph,int mode)
{
    result_t                result = RESULT_SUCCEED;
    union REGS inregs;    
    
    if( mode >= 0x100)
    {
        /*
         *  VESA
         */
        inregs.x.bx = mode;
        inregs.x.ax = 0x4f02;
        int86(0x10,&inregs,&inregs);

        if(inregs.x.ax!=0x004f) //���ò��ɹ�        
            result = RESULT_SUCCEED;
    }
    else
    {
        /*
         *  VGA
         */
        inregs.x.ax = mode;

        int86(0x10,&inregs,&inregs);
    }

    if( RESULT_SUCCEED != result )
        return RESULT_FAILED;
    
    Vga_page_set(graph,0);

    graph->g_mode = mode;

    switch( mode )
    {
    case VGA_MODE_640_480_16:
        return RESULT_FAILED;
    case VESA_MODE_640_480_256:
        graph->g_reslu_x = 640;
        graph->g_reslu_y = 480;
        return RESULT_SUCCEED;
    case VESA_MODE_640_480_32K:
        return RESULT_FAILED;

    case VESA_MODE_640_480_64K:
        graph->g_reslu_x            = 640;
        graph->g_reslu_y            = 480;
        graph->g_bytes_of_scanline  = 640*2;
        graph->g_bytes_of_x_vbuf    = 640*2;

        graph->g_pixel_set          = Vga_pixel_set_64K;
        graph->g_line               = Vga_line_64K;

        return RESULT_SUCCEED;

    case VESA_MODE_640_480_16M:
        graph->g_reslu_x            = 640;
        graph->g_reslu_y            = 480;
        graph->g_bytes_of_scanline  = 2048;
        graph->g_bytes_of_x_vbuf    = 640*3;

        graph->g_pixel_set          = Vga_pixel_set_16M;
        //graph->g_line               = Vga_line_64K;

        return RESULT_SUCCEED;
    case VGA_MODE_320_200_256:
        graph->g_reslu_x = 320;
        graph->g_reslu_y = 200;
        return RESULT_FAILED;
    }
    return RESULT_FAILED;
}



result_t    Vga_initial(graph_t * graph)
{
    graph->g_vbuf_range[0].vr_start = 0xA0000;
    graph->g_vbuf_range[0].vr_end   = 0xAFFFF;
    graph->g_vr_actived             = graph->g_vbuf_range;


    graph->g_mode_set       = Vga_mode_set;
    graph->g_fill_rect      = Vga_fill_rect;
    graph->g_rect           = Vga_rect;


    //graph->g_pixel_set      = Vga_pixel_set;
    //graph->g_screen_display = Vga_screen_display_256;
    //graph->g_line           = Vga_line256;

    return RESULT_SUCCEED;
}