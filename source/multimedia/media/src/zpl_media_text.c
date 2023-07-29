/*
 * zpl_media_text.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"

#ifdef ZPL_FREETYPE_MODULE
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#endif

#define BITMAP_A(c)     ((c)>>24)&0xff
#define BITMAP_R(c)     ((c)>>16)&0xff
#define BITMAP_G(c)     ((c)>>8)&0xff
#define BITMAP_B(c)     ((c))&0xff


#ifdef ZPL_FREETYPE_MODULE

static FT_Face    _gl_freetype_face = NULL;
static FT_Library _freetype_library = NULL;
static FT_UInt    _freetype_automic = 0;


static int zpl_freetype_text_init(zpl_media_fontmap_t *fontbitmap, char *filename)
{
    FT_Error error;
    //char*         filename = "C:/WINDOWS/Fonts/Corbel.ttf";
    if(_freetype_library && _gl_freetype_face)
    {
        _freetype_automic++;
        return OK;
    }
    error = FT_Init_FreeType(&_freetype_library); /* initialize library */
    if (error)
    {
        zm_msg_error("freetype init failed.");
        return ERROR;
    }
    /* error handling omitted */
    if(filename)
        error = FT_New_Face(_freetype_library, filename, 0, &_gl_freetype_face); /* create face object */
    else 
        error = FT_New_Face(_freetype_library, ZPL_FREETYPE_PATH, 0, &_gl_freetype_face);   
    
    if (error)
    {
        zm_msg_error("freetype new face failed.");
        return ERROR;
    }
    /* error handling omitted */
    //error = FT_Select_Charmap(face, ft_encoding_unicode);
    FT_Select_Charmap(_gl_freetype_face, FT_ENCODING_UNICODE);
    if (error)
    {
        zm_msg_error("freetype select charmap failed.");
        return ERROR;
    }
    error = FT_Set_Pixel_Sizes(_gl_freetype_face, 16, 0);
    if (error)
    {
        zm_msg_error("freetype set pixel failed.");
        return ERROR;
    }
    /* error handling omitted */
    return OK;
}

static int zpl_freetype_text_exit(zpl_media_fontmap_t *fontbitmap)
{
    if (fontbitmap && _gl_freetype_face && _freetype_automic == 0)
    {
        FT_Done_Face(_gl_freetype_face);
        _gl_freetype_face = NULL;
    }

    if (_freetype_library && _freetype_automic == 0)
    {
        FT_Done_FreeType(_freetype_library);
        _freetype_library = NULL;
    }
    if(_freetype_automic)
        _freetype_automic--;
    return OK;
}
#endif

static int zpl_media_text_build_pixel(zpl_media_bitmap_t *vidbitmap, zpl_int32 x,
                                             zpl_int32 y, zpl_uint32 color)
{
    zpl_uint16 *bmp16 = NULL;
    zpl_uint32 *bmp32 = NULL;
    switch(vidbitmap->enPixelFormat)
    {
    case BITMAP_PIXEL_FORMAT_RGB_444:
        bmp16 = (zpl_uint16 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_R(color)>>4)<<8)|((BITMAP_G(color)>>4)<<4)|(BITMAP_B(color)>>4);
        break;
    case BITMAP_PIXEL_FORMAT_RGB_555:
        bmp16 = (zpl_uint16 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_R(color)>>3)<<10)|((BITMAP_G(color)>>3)<<5)|(BITMAP_B(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_RGB_565:
        bmp16 = (zpl_uint16 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_R(color)>>3)<<11)|((BITMAP_G(color)>>2)<<5)|(BITMAP_B(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_RGB_888:
        bmp32 = (zpl_uint32 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 4));
        *bmp32 = ((BITMAP_R(color))<<16)|((BITMAP_G(color))<<8)|(BITMAP_B(color));
        break;
        ////
    case BITMAP_PIXEL_FORMAT_BGR_444:
        bmp16 = (zpl_uint16 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_B(color)>>4)<<8)|((BITMAP_G(color)>>4)<<4)|(BITMAP_R(color)>>4);
        break;
    case BITMAP_PIXEL_FORMAT_BGR_555:
        bmp16 = (zpl_uint16 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_B(color)>>3)<<10)|((BITMAP_G(color)>>3)<<5)|(BITMAP_R(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_BGR_565:
        bmp16 = (zpl_uint16 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_B(color)>>3)<<11)|((BITMAP_G(color)>>2)<<5)|(BITMAP_R(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_BGR_888:
        bmp32 = (zpl_uint32 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 4));
        *bmp32 = ((BITMAP_B(color))<<16)|((BITMAP_G(color))<<8)|(BITMAP_R(color));
        break;
        ////
    case BITMAP_PIXEL_FORMAT_ARGB_1555:
        bmp16 = (zpl_uint16 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_A(color))<<15)|((BITMAP_R(color)>>3)<<10)|((BITMAP_G(color)>>3)<<5)|(BITMAP_B(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_4444:
        bmp16 = (zpl_uint16 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_A(color))<<12)|((BITMAP_R(color)>>4)<<8)|((BITMAP_G(color)>>4)<<4)|(BITMAP_B(color)>>4);
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_8565:
        bmp32 = (zpl_uint32 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 4));
        *bmp32 = ((BITMAP_A(color))<<16)|((BITMAP_R(color)>>3)<<11)|((BITMAP_G(color)>>2)<<5)|(BITMAP_B(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_8888:
        bmp32 = (zpl_uint32 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 4));
        *bmp32 = ((BITMAP_A(color))<<24)|((BITMAP_R(color))<<16)|((BITMAP_G(color))<<8)|(BITMAP_B(color));
        break;

        ////
    case BITMAP_PIXEL_FORMAT_ABGR_1555:
        bmp16 = (zpl_uint16 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_A(color))<<15)|((BITMAP_B(color)>>3)<<10)|((BITMAP_G(color)>>3)<<5)|(BITMAP_R(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_4444:
        bmp16 = (zpl_uint16 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_A(color))<<12)|((BITMAP_B(color)>>4)<<8)|((BITMAP_G(color)>>4)<<4)|(BITMAP_R(color)>>4);
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_8565:
        bmp32 = (zpl_uint32 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 4));
        *bmp32 = ((BITMAP_A(color))<<16)|((BITMAP_B(color)>>3)<<11)|((BITMAP_G(color)>>2)<<5)|(BITMAP_R(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_8888:
        bmp32 = (zpl_uint32 *)((char*)vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 4));
        *bmp32 = ((BITMAP_A(color))<<24)|((BITMAP_B(color))<<16)|((BITMAP_G(color))<<8)|(BITMAP_R(color));
        break;
    }
    return OK;
}


/*输出字模的函数*/
/*函数的功能是输出一个宽度为w,高度为h的字模到屏幕的 (x,y) 坐标出,文字的颜色为 color,文字的点阵数据为 pdata 所指*/
#ifdef ZPL_FREETYPE_MODULE
static void zpl_media_text_build_char(zpl_media_bitmap_t *vidbitmap, zpl_int32 x,
                                            zpl_int32 y, zpl_media_fontmap_t *fontbitmap)
{
    FT_Bitmap *bitmap = (FT_Bitmap *)fontbitmap->m_fontbitmap;
    FT_Int i = 0, j = 0, p = 0, q = 0;
    FT_Int x_max = x + bitmap->width;
    FT_Int y_max = y + bitmap->rows;

    /* for simplicity, we assume that `bitmap->pixel_mode' */
    /* is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)   */

    for (i = x, p = 0; i < x_max; i++, p++)
    {
        for (j = y, q = 0; j < y_max; j++, q++)
        {
            if (i < 0 || j < 0 || i >= vidbitmap->u32Width || j >= vidbitmap->u32Height)
                continue;
            if (bitmap->buffer[q * bitmap->width + p])
            {
                //printf("*");
                zpl_media_text_build_pixel(vidbitmap, i, j, fontbitmap->m_fontcolor);
            }
            //else
            //    printf("#");
        }
        //printf("\r\n");
    }
}

/* 计算一行文字外框坐标点 */
static int zpl_media_text_line_auto_size(zpl_media_fontmap_t *fontbitmap, FT_Face face, zpl_char *osdstr, int len, FT_BBox *abbox)
{
    int i;
    int error;
    FT_BBox bbox;
    FT_BBox glyph_bbox;
    FT_Vector pen;
    FT_Glyph glyph;
    FT_GlyphSlot slot = face->glyph;
    /* 初始化 */
    bbox.xMin = bbox.yMin = 32000;
    bbox.xMax = bbox.yMax = -32000;
    /* 指定原点为(0, 0) */
    pen.x = 0;
    pen.y = 0;
    /* 计算每个字符的bounding box */
    /* 先translate, 再load char, 就可以得到它的外框了 */
    for (i = 0; i < len; i++)
    {
        if(isprint(osdstr[i]))
        {
            /* 转换：transformation */
            FT_Set_Transform(face, 0, &pen);
            /* 加载位图: load glyph image into the slot (erase previous one) */
            error = FT_Load_Char(face, osdstr[i], FT_LOAD_RENDER);
            if (error)
            {
                printf("FT_Load_Char error\n");
                return ERROR;
            }
            /* 取出glyph */
            error = FT_Get_Glyph(face->glyph, &glyph);
            if (error)
            {
                printf("FT_Get_Glyph error!\n");
                return ERROR;
            }
            /* 从glyph得到外框: bbox */
            FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &glyph_bbox);
            /* 更新外框 */
            if (glyph_bbox.xMin < bbox.xMin)
                bbox.xMin = glyph_bbox.xMin;

            if (glyph_bbox.yMin < bbox.yMin)
                bbox.yMin = glyph_bbox.yMin;

            if (glyph_bbox.xMax > bbox.xMax)
                bbox.xMax = glyph_bbox.xMax;

            if (glyph_bbox.yMax > bbox.yMax)
                bbox.yMax = glyph_bbox.yMax;

            /* 计算下一个字符的原点: increment pen position */
            pen.x += slot->advance.x + fontbitmap->m_fontsplit;
            pen.y += slot->advance.y;
        }
    }
    //printf("==========(%d/%d, %d/%d)==========\r\n", bbox.xMin,bbox.xMin,bbox.xMax,bbox.yMax);
    /* return string bbox */
    *abbox = bbox;
    return OK;
}
#endif

static int zpl_media_text_line_culc(zpl_char *osdstr, int len, zpl_uint8 *line_num, zpl_uint8 *line_off)
{
    int nline = 1, n = 0, flag = 0;
    line_off[0] = 0;
    while(n < len )
    {
        if(osdstr[n] == '\n' || osdstr[n] == '\r')
        {
            if(flag == 0)
            {
                nline++;//计算行数
                flag = 1;
            }
        }
        else
        {
            if(flag)
            {
                flag = 0;
                line_off[nline-1] = n;
            }
            line_num[nline-1]++;//计算该行字节数
        }
        n++;
    }
    return nline;
}
#ifdef ZPL_FREETYPE_MODULE
static int zpl_media_text_build(zpl_media_bitmap_t *vidbitmap, zpl_char *osdstr, int len, zpl_media_fontmap_t *fontbitmap)
{
    FT_Error error = 0;
    zpl_char *str = osdstr;
    zpl_uint32 n = 0, nline = 0, i = 0;
    zpl_uint8 line_num[32], line_off[32];
    FT_Vector     pen;
    FT_GlyphSlot  slot = NULL;
    FT_Matrix     matrix;
    FT_BBox line_bbox;
    FT_Int32 ftloagflag = FT_LOAD_RENDER;
    if(fontbitmap->m_fontwidth > 18)
        ftloagflag |= FT_LOAD_FORCE_AUTOHINT;
    //text->m_font.m_bold
    // 粗体
    //slot = ((_gl_freetype_face))->glyph;
    double angle = (0.0/360.0)*3.14159*2;
    matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
    matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
    matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
    matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);
    //pen指示的是笛卡尔坐标，原点是在左下角，在lcd原点一般在左上角，在字体文件或者字体函数中用的都是笛卡尔坐标，
    //这里需要进行转换为lcd的坐标，笛卡尔与lcd的x同向，因此x' = x（lcd），y坐标下，笛卡尔与lcd的y坐标相加等于lcd宽度，
    //其中target_height相当于宽度，单位是1/64point，因此乘上64
    //LCD显示使用LCD坐标系，freetype库使用笛卡尔坐标系。注意不同坐标系下的坐标转换。
    //已知LCD区域，通常是LCD屏幕大小(H, V)（宽、高）。那么，LCD坐标系下的(x, y)，对应笛卡尔坐标系下(x, V - y)
    memset(line_num, 0, sizeof(line_num));
    memset(line_off, 0, sizeof(line_off));

    nline = zpl_media_text_line_culc(osdstr, len, line_num, line_off);

    for(i = 0; i < nline; i++)
    {
        zpl_media_text_line_auto_size(fontbitmap, _gl_freetype_face, osdstr+line_off[i], line_num[i], &line_bbox);
        n = 0;
        str = osdstr+line_off[i];
        pen.x = (3 - line_bbox.xMin) * 64; /* 单位: 1/64像素 */
        pen.y = (vidbitmap->u32Height - line_bbox.yMax - i*(fontbitmap->m_fontheight + fontbitmap->m_fontline) - 3) * 64; /* 单位: 1/64像素 */
        //pen.x = fontbitmap->m_fontwidth * 64;
        //pen.y = ((vidbitmap->u32Height-fontbitmap->m_fontheight))*64;
        while(n < line_num[i] )
        {
            if(isprint(str[n]))
            {
                /* set transformation */
                FT_Set_Transform( _gl_freetype_face, &matrix, &pen );
                /* load glyph image into the slot (erase previous one) */
                error = FT_Load_Char( _gl_freetype_face, str[n], ftloagflag );
                if ( error )
                {
                    zm_msg_error("freetype load char failed.");
                    n++;
                    return ERROR;
                }
                slot = (_gl_freetype_face)->glyph;
                //slot = ((FT_Face)fontbitmap->m_fontface)->glyph;   
                fontbitmap->m_fontbitmap = &slot->bitmap;
                //if(slot->bitmap_left > fontbitmap->m_fontsplit)
                zpl_media_text_build_char(vidbitmap, slot->bitmap_left, ((int)vidbitmap->u32Height - slot->bitmap_top) , fontbitmap);

                /* increment pen position */
                pen.x += slot->advance.x + fontbitmap->m_fontsplit;
                pen.y += slot->advance.y;
            }
            n++;
        }
    }
    return OK;
}
#endif
/*字符大小*/
static int zpl_media_text_font_size(zpl_media_text_t *text, zpl_uint32 pixel)
{
    if(pixel == ZPL_FONT_SIZE_16X16)
    {
        text->m_font.m_fontwidth = 16;     //字体大小
        text->m_font.m_fontheight = 16;    //字体大小
    }
    else if(pixel == ZPL_FONT_SIZE_24X24)
    {
        text->m_font.m_fontwidth = 24;     //字体大小
        text->m_font.m_fontheight = 24;    //字体大小
    }
    else if(pixel == ZPL_FONT_SIZE_32X32)
    {
        text->m_font.m_fontwidth = 32;     //字体大小
        text->m_font.m_fontheight = 32;    //字体大小
    }
    else if(pixel == ZPL_FONT_SIZE_48X48)
    {
        text->m_font.m_fontwidth = 48;     //字体大小
        text->m_font.m_fontheight = 48;    //字体大小
    }
    return OK;
}

/* 计算整个框图大小 */
static int zpl_media_text_rect_auto_size(zpl_media_text_t *text, zpl_char *osdstring, int ilen)
{
    int32_t linenum = 1, i = 0, max_line_len = 0;
    zpl_uint8 line_num[32], line_off[32];
    memset(line_num, 0, sizeof(line_num));
    memset(line_off, 0, sizeof(line_off));
    #ifdef ZPL_FREETYPE_MODULE
    int rect_x = 0, line_y[32] = {0}, rect_y = 0;
    FT_BBox line_bbox;
    #endif
    linenum = zpl_media_text_line_culc(osdstring, ilen, line_num, line_off);
    if(linenum < 1)
        return ERROR;
    for(i = 0; i < linenum; i++)
    {
        #ifdef ZPL_FREETYPE_MODULE
        zpl_media_text_line_auto_size(&text->m_font, _gl_freetype_face, osdstring+line_off[i], line_num[i], &line_bbox);
        rect_x = MAX(rect_x, (line_bbox.xMax-line_bbox.xMin));
        line_y[i] = (line_bbox.yMax-line_bbox.yMin);
        max_line_len = MAX(max_line_len, line_num[i]);
        //zm_msg_debug("area max_line_len %d line_num[%d] %d line_y[%d] %d", max_line_len, i, line_num[i], i, line_y[i]);
        #else
        max_line_len = MAX(max_line_len, line_num[i]);
        #endif
    }
    if(text->m_bitmap)
    {
        #ifdef ZPL_FREETYPE_MODULE
        text->m_bitmap->u32Width = rect_x + text->m_font.m_fontheight/2;
        for(i = 0; i < linenum; i++)
        {
            rect_y += line_y[i];
        }
        text->m_bitmap->u32Height = rect_y + text->m_font.m_fontline*(linenum-1) + 6;
        #else
        text->m_bitmap->u32Width = text->m_font.m_fontwidth * max_line_len + text->m_font.m_fontsplit*(max_line_len-1);
        text->m_bitmap->u32Height = text->m_font.m_fontheight * linenum + text->m_font.m_fontline*(linenum-1) + 6;
        #endif
        text->m_bitmap->u32Width = ZPL_BITMAP_ALIGN(text->m_bitmap->u32Width);
        text->m_bitmap->u32Height = ZPL_BITMAP_ALIGN(text->m_bitmap->u32Height);
        //zm_msg_debug("area text rect %dx%d", text->m_bitmap->u32Width, text->m_bitmap->u32Height);
        return OK;
    }
    return ERROR;
}

static int zpl_media_text_bitmap_size(zpl_media_text_t *text)
{
    int bitsize = 0;
    switch(text->m_bitmap->enPixelFormat)
    {
    case BITMAP_PIXEL_FORMAT_RGB_444:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_RGB_555:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_RGB_565:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_RGB_888:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 4;
        break;
        ////
    case BITMAP_PIXEL_FORMAT_BGR_444:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_BGR_555:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_BGR_565:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_BGR_888:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 4;
        break;
        ////
    case BITMAP_PIXEL_FORMAT_ARGB_1555:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_4444:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_8565:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 4;
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_8888:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 4;
        break;

        ////
    case BITMAP_PIXEL_FORMAT_ABGR_1555:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_4444:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_8565:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 4;
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_8888:
        bitsize = text->m_bitmap->u32Width * text->m_bitmap->u32Height * 4;
        break;
    }
    if(text->m_bitmap->u32len != bitsize)
    {
        if(text->m_bitmap->pData)
            free(text->m_bitmap->pData);
        text->m_bitmap->pData = malloc(bitsize);
        if (text->m_bitmap->pData == NULL)
            return ERROR;
    }
    else
    {
        if(text->m_bitmap->pData == NULL)
            text->m_bitmap->pData = malloc(bitsize);
        if (text->m_bitmap->pData == NULL)
            return ERROR;
    }
    //zm_msg_debug("area text bitmap %dx%d size %d", text->m_bitmap->u32Width, text->m_bitmap->u32Height, bitsize);
    text->m_bitmap->u32len = bitsize;
    return OK;
}

static int zpl_media_text_clean(zpl_media_bitmap_t *vidbitmap, zpl_uint32 color)
{
    if (vidbitmap && vidbitmap->pData)
    {
        memset(vidbitmap->pData, color & 0xff, vidbitmap->u32len);
        return OK;
    }
    if(vidbitmap == NULL)
        return OK;
    return ERROR;
}

static int zpl_media_text_show_string(zpl_media_text_t *text, zpl_bool bold, zpl_char *osdstring, int len)
{
    int ret = 0;

    if(text->m_font.m_fontwidth == 0)
        zpl_media_text_font_size(text, ZPL_FONT_SIZE_16X16); 
    text->m_font.m_bold = bold;
#ifdef ZPL_FREETYPE_MODULE
    ret = FT_Set_Pixel_Sizes(_gl_freetype_face, text->m_font.m_fontwidth, text->m_font.m_fontheight);
    if (ret)
    {
        zm_msg_error("freetype set pixel size failed.");
        return ERROR;
    }
#endif  

    if(zpl_media_text_rect_auto_size(text, osdstring, len) != OK)
    {
        return ERROR;
    }
    if(text->m_bitmap && zpl_media_text_bitmap_size(text) != OK)
    {
        return ERROR;
    }
    if(text->m_bitmap)
        zpl_media_text_clean(text->m_bitmap, text->m_font.m_bgcolor);
    return zpl_media_text_build(text->m_bitmap, osdstring, len, &text->m_font);
}


int zpl_media_text_show(zpl_media_text_t *text, zpl_bool bold, zpl_uint32 color, const char *format,...)
{
    int ret = 0;
    char textbuf[2048];
	va_list args;
	va_start(args, format);
    memset(textbuf, 0, sizeof(textbuf));
	ret = snprintf(textbuf, sizeof(textbuf), format, args);
	va_end(args);
    if(ret > 0 && ret < sizeof(textbuf))
    {
        return zpl_media_text_show_string(text, bold, textbuf, ret);
    }
    return ERROR;
}

int zpl_media_text_attr(zpl_media_text_t *text, zpl_uint32 pixel, zpl_uint32 color, zpl_uint32 bgcolor, zpl_uint8 fontsplit, zpl_uint8 fontline)
{
    text->m_font.m_fontsplit = fontsplit;    //字体间隔
    text->m_font.m_fontline = fontline;      //字体行间距
    text->m_font.m_fontcolor = color;
    text->m_font.m_bgcolor = bgcolor;
    zpl_media_text_font_size(text, pixel);    
    return OK;
}

zpl_media_text_t * zpl_media_text_create(zpl_media_bitmap_t *bitmap, zpl_char *filename)
{
    zpl_media_text_t *text = malloc(sizeof(zpl_media_text_t));
    if(text)
    {
        memset(text, 0, sizeof(zpl_media_text_t));
        if(bitmap)
            text->m_bitmap = bitmap;
        if(text->m_bitmap)
            text->m_bitmap->enPixelFormat = BITMAP_PIXEL_FORMAT_ARGB_1555;
#ifdef ZPL_FREETYPE_MODULE
        if(zpl_freetype_text_init(&text->m_font, filename) == OK)
        {
            return text;
        }
        free(text);
        text = NULL;
#endif
        zm_msg_error("text bitmap create failed.");
        return text;
    }
    return NULL;
}

int zpl_media_text_destroy(zpl_media_text_t *text)
{
#ifdef ZPL_FREETYPE_MODULE
    if(zpl_freetype_text_exit(&text->m_font) != OK)
    {
        zm_msg_error("text bitmap destroy failed.");
        return ERROR;
    }
#endif
    if(text->m_bitmap && text->m_bitmap->pData)
    {
        free(text->m_bitmap->pData);
        text->m_bitmap->pData = NULL;
    }
    free(text);
    text = NULL;
    return OK;
}


/*

*/