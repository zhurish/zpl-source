/*
 * zpl_media_text.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_bmp.h"
#include "zpl_media_text.h"

#define BITMAP_A(c)     ((c)>>24)&0xff
#define BITMAP_R(c)     ((c)>>16)&0xff
#define BITMAP_G(c)     ((c)>>8)&0xff
#define BITMAP_B(c)     ((c))&0xff

static int zpl_media_text_bitmap_build_pixel(zpl_media_bitmap_t *vidbitmap, zpl_uint32 x,
                                             zpl_uint32 y, zpl_uint32 color);
static int zpl_media_text_bitmap_size(zpl_media_text_t *text);

#ifdef ZPL_FREETYPE_ENABLE

static FT_Face    _freetype_face = NULL;
static FT_Library _freetype_library = NULL;
static FT_UInt    _freetype_automic = 0;


static int zpl_freetype_text_init(zpl_media_fontmap_t *fontbitmap, char *filename)
{
    FT_Error error;
    //char*         filename = "C:/WINDOWS/Fonts/Corbel.ttf";
    if(_freetype_library && _freetype_face)
    {
        _freetype_automic++;
        return OK;
    }
    error = FT_Init_FreeType(&_freetype_library); /* initialize library */
    if (error)
    {
        return ERROR;
    }
    /* error handling omitted */
    if(filename)
        error = FT_New_Face(_freetype_library, filename, 0, &_freetype_face); /* create face object */
    else 
        error = FT_New_Face(_freetype_library, ZPL_FREETYPE_PATH, 0, &_freetype_face);   
    
    if (error)
    {
        return ERROR;
    }
    /* error handling omitted */
    //error = FT_Select_Charmap(face, ft_encoding_unicode);
    FT_Select_Charmap(_freetype_face, FT_ENCODING_UNICODE);
    if (error)
    {
        return ERROR;
    }
    error = FT_Set_Pixel_Sizes(_freetype_face, 16, 0);
    if (error)
    {
        return ERROR;
    }
    /* error handling omitted */
    return OK;
}

static int zpl_freetype_text_exit(zpl_media_fontmap_t *fontbitmap)
{
    if (fontbitmap && _freetype_face && _freetype_automic == 0)
    {
        FT_Done_Face(_freetype_face);
        _freetype_face = NULL;
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

static void zpl_freetype_text_draw_key(zpl_media_bitmap_t *vidbitmap, zpl_void *pdata, zpl_uint32 fw, zpl_uint32 fh,
                                       zpl_uint32 x, zpl_uint32 y, zpl_uint32 color)
{
    FT_Bitmap *bitmap = (FT_Bitmap *)pdata;
    FT_UInt i, j, p, q;
    FT_UInt x_max = x + bitmap->width;
    FT_UInt y_max = y + bitmap->rows;

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
                zpl_media_text_bitmap_build_pixel(vidbitmap, i, j, color);
            }
            //else
            //    printf("#");
        }
        printf("\r\n");
    }
}
#else
/*输出字模的函数*/
/*函数的功能是输出一个宽度为w,高度为h的字模到屏幕的 (x,y) 坐标出,文字的颜色为 color,文字的点阵数据为 pdata 所指*/
static void zpl_freetype_text_draw_key(zpl_media_bitmap_t *vidbitmap, zpl_void *pdata, zpl_uint32 fw, zpl_uint32 fh,
                                       zpl_uint32 x, zpl_uint32 y, zpl_uint32 color)
{
    zpl_uint32 i = 0;                                   /* 控制行 */
    zpl_uint32 j = 0;                                   /* 控制一行中的8个点 */
    zpl_uint32 k = 0;                                   /* 一行中的第几个"8个点"了 */
    zpl_uint32 nc = 0;                                  /* 到点阵数据的第几个字节了 */
    zpl_uint32 cols = 0;                                /* 控制列 */
    zpl_uint32 mask[8] = {128, 64, 32, 16, 8, 4, 2, 1}; /* 位屏蔽字 */
    zpl_uint32 ifw = (fw + 7) / 8 * 8;                  /* 重新计算w */
    zpl_uint8 *pdata8 = pdata; 
    nc = 0;
    for (i = 0; i < fh; i++)
    {
        cols = 0;
        for (k = 0; k < ifw / 8; k++)
        {
            for (j = 0; j < 8; j++)
            {
                if (pdata8[nc] & mask[j])
                    zpl_media_text_bitmap_build_pixel(vidbitmap, x + cols, y + i, color);
                cols++;
            }
            nc++;
        }
    }
}
#endif

static int zpl_media_text_bitmap_clean(zpl_media_bitmap_t *vidbitmap, zpl_uint32 color)
{
    if (vidbitmap->pData)
    {
        memset(vidbitmap->pData, color & 0xff, vidbitmap->u32len);
        return OK;
    }
    return ERROR;
}

static int zpl_media_text_bitmap_build_pixel(zpl_media_bitmap_t *vidbitmap, zpl_uint32 x,
                                             zpl_uint32 y, zpl_uint32 color)
{
    zpl_uint16 *bmp16 = NULL;
    zpl_uint32 *bmp32 = NULL;
    switch(vidbitmap->enPixelFormat)
    {
    case BITMAP_PIXEL_FORMAT_RGB_444:
        bmp16 = (zpl_uint16 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_R(color)>>4)<<8)|((BITMAP_G(color)>>4)<<4)|(BITMAP_B(color)>>4);
        break;
    case BITMAP_PIXEL_FORMAT_RGB_555:
        bmp16 = (zpl_uint16 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_R(color)>>3)<<10)|((BITMAP_G(color)>>3)<<5)|(BITMAP_B(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_RGB_565:
        bmp16 = (zpl_uint16 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_R(color)>>3)<<11)|((BITMAP_G(color)>>2)<<5)|(BITMAP_B(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_RGB_888:
        bmp32 = (zpl_uint32 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 4));
        *bmp32 = ((BITMAP_R(color))<<16)|((BITMAP_G(color))<<8)|(BITMAP_B(color));
        break;
        ////
    case BITMAP_PIXEL_FORMAT_BGR_444:
        bmp16 = (zpl_uint16 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_B(color)>>4)<<8)|((BITMAP_G(color)>>4)<<4)|(BITMAP_R(color)>>4);
        break;
    case BITMAP_PIXEL_FORMAT_BGR_555:
        bmp16 = (zpl_uint16 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_B(color)>>3)<<10)|((BITMAP_G(color)>>3)<<5)|(BITMAP_R(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_BGR_565:
        bmp16 = (zpl_uint16 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_B(color)>>3)<<11)|((BITMAP_G(color)>>2)<<5)|(BITMAP_R(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_BGR_888:
        bmp32 = (zpl_uint32 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 4));
        *bmp32 = ((BITMAP_B(color))<<16)|((BITMAP_G(color))<<8)|(BITMAP_R(color));
        break;
        ////
    case BITMAP_PIXEL_FORMAT_ARGB_1555:
        bmp16 = (zpl_uint16 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_A(color))<<15)|((BITMAP_R(color)>>3)<<10)|((BITMAP_G(color)>>3)<<5)|(BITMAP_B(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_4444:
        bmp16 = (zpl_uint16 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_A(color))<<12)|((BITMAP_R(color)>>4)<<8)|((BITMAP_G(color)>>4)<<4)|(BITMAP_B(color)>>4);
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_8565:
        bmp32 = (zpl_uint32 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 4));
        *bmp32 = ((BITMAP_A(color))<<16)|((BITMAP_R(color)>>3)<<11)|((BITMAP_G(color)>>2)<<5)|(BITMAP_B(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_8888:
        bmp32 = (zpl_uint32 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 4));
        *bmp32 = ((BITMAP_A(color))<<24)|((BITMAP_R(color))<<16)|((BITMAP_G(color))<<8)|(BITMAP_B(color));
        break;

        ////
    case BITMAP_PIXEL_FORMAT_ABGR_1555:
        bmp16 = (zpl_uint16 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_A(color))<<15)|((BITMAP_B(color)>>3)<<10)|((BITMAP_G(color)>>3)<<5)|(BITMAP_R(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_4444:
        bmp16 = (zpl_uint16 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
        *bmp16 = ((BITMAP_A(color))<<12)|((BITMAP_B(color)>>4)<<8)|((BITMAP_G(color)>>4)<<4)|(BITMAP_R(color)>>4);
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_8565:
        bmp32 = (zpl_uint32 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 4));
        *bmp32 = ((BITMAP_A(color))<<16)|((BITMAP_B(color)>>3)<<11)|((BITMAP_G(color)>>2)<<5)|(BITMAP_R(color)>>3);
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_8888:
        bmp32 = (zpl_uint32 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 4));
        *bmp32 = ((BITMAP_A(color))<<24)|((BITMAP_B(color))<<16)|((BITMAP_G(color))<<8)|(BITMAP_R(color));
        break;
    }
    return OK;
}



static int zpl_media_text_bitmap_build_char(zpl_media_bitmap_t *vidbitmap, zpl_uint32 x,
                                            zpl_uint32 y, zpl_media_fontmap_t *fontbitmap)
{
    zpl_freetype_text_draw_key(vidbitmap, fontbitmap->m_fontbitmap, fontbitmap->m_fontwidth,
                               fontbitmap->m_fontheight, x, y, fontbitmap->m_fontcolor);
    return OK;
}
#ifdef ZPL_FREETYPE_ENABLE
static int zpl_media_text_bitmap_buildaa(zpl_media_bitmap_t *vidbitmap, zpl_char *osdstr, zpl_media_fontmap_t *fontbitmap)
{
    FT_Error error = 0;
    zpl_char *str = osdstr;
    zpl_uint32 n;
    FT_Vector     pen;
    FT_GlyphSlot  slot;
    FT_Int32 ftloagflag = FT_LOAD_RENDER;
    if(fontbitmap->m_fontwidth > 18)
        ftloagflag |= FT_LOAD_FORCE_AUTOHINT;

    // 粗体
    slot = ((FT_Face)(_freetype_face))->glyph;

    pen.x = 0 * fontbitmap->m_fontwidth;
    pen.y = /*( vidbitmap->u32Height - 5)*/1 * fontbitmap->m_fontwidth;

    for ( n = 0; n < strlen(osdstr); n++ )
    {
        /* set transformation */
        FT_Set_Transform(_freetype_face, NULL, &pen );
        /* load glyph image into the slot (erase previous one) */
        error = FT_Load_Char(_freetype_face, str[n], ftloagflag );
        if ( error )
            continue;
        slot = ((FT_Face)_freetype_face)->glyph;
        fontbitmap->m_fontbitmap = &slot->bitmap;
        zpl_media_text_bitmap_build_char(vidbitmap, slot->bitmap_left, slot->bitmap_top * (n+1)/*vidbitmap->u32Height - slot->bitmap_top*/ , fontbitmap);

        /* increment pen position */
        pen.x += slot->advance.x + fontbitmap->m_fontsplit;
        pen.y += slot->advance.y;
        if( (pen.x + fontbitmap->m_fontsplit) <= vidbitmap->u32Width)
        {
            pen.x = 0;
            pen.y += fontbitmap->m_fontline;
        }
    }
    return OK;
}

static int zpl_media_text_bitmap_build(zpl_media_bitmap_t *vidbitmap, zpl_char *osdstr, zpl_media_fontmap_t *fontbitmap)
{
    FT_Error error = 0;
    zpl_char *str = osdstr;
    zpl_uint32 n = 0;
    FT_Vector     pen;
    FT_GlyphSlot  slot;
    FT_Matrix     matrix;
    FT_Int32 ftloagflag = FT_LOAD_RENDER;
    if(fontbitmap->m_fontwidth > 18)
        ftloagflag |= FT_LOAD_FORCE_AUTOHINT;

    // 粗体
    slot = ((FT_Face)(_freetype_face))->glyph;
    double angle = (0.0/360)*3.14159*2;
    matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
    matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
    matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
    matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);
    //pen指示的是笛卡尔坐标，原点是在左下角，在lcd原点一般在左上角，在字体文件或者字体函数中用的都是笛卡尔坐标，
    //这里需要进行转换为lcd的坐标，笛卡尔与lcd的x同向，因此x' = x（lcd），y坐标下，笛卡尔与lcd的y坐标相加等于lcd宽度，
    //其中target_height相当于宽度，单位是1/64point，因此乘上64

    pen.x = fontbitmap->m_fontwidth * 64;
    pen.y = ((vidbitmap->u32Height-fontbitmap->m_fontheight))*64;
    while(n < strlen(osdstr))
    {
        if(str[n] == '\n' || str[n] == '\r')
        {
            pen.x = fontbitmap->m_fontwidth * 64;
            pen.y += fontbitmap->m_fontsplit + fontbitmap->m_fontheight*64;
        }
        if(isprint(str[n]))
        {
            /* set transformation */
            FT_Set_Transform( _freetype_face, NULL, &pen );
            /* load glyph image into the slot (erase previous one) */
            error = FT_Load_Char( _freetype_face, str[n], ftloagflag );
            if ( error )
                continue;
            slot = ((FT_Face)_freetype_face)->glyph;
            fontbitmap->m_fontbitmap = &slot->bitmap;
            zpl_media_text_bitmap_build_char(vidbitmap, slot->bitmap_left, vidbitmap->u32Height-slot->bitmap_top , fontbitmap);

            /* increment pen position */
            pen.x += slot->advance.x;// + fontbitmap->m_fontsplit;
            pen.y += slot->advance.y;
            /*if( (pen.x + fontbitmap->m_fontsplit) <= vidbitmap->u32Width)
            {
                pen.x = 0;
                pen.y += fontbitmap->m_fontline;
            }*/
        }
        n++;
    }
    return OK;
}
#else
static int zpl_media_text_bitmap_build(zpl_media_bitmap_t *vidbitmap, zpl_char *osdstr, zpl_media_fontmap_t *fontbitmap)
{
    zpl_char *str = osdstr;
    zpl_uint32 fw =16, fh = 16;
    zpl_uint32 x = 0, y = 0;
    zpl_uint8 *keybitmap;
    while(*str != '\0')
    {
        //TODO GET FONT BITMAP
        zpl_media_text_bitmap_build_char(vidbitmap, x, y, fontbitmap);
        x += fw + fontbitmap->m_fontsplit;
        if( (x + fw + fontbitmap->m_fontsplit) <= vidbitmap->u32Width)
        {
            x = 0;
            y += fh + fontbitmap->m_fontline;
        }
    }
    return OK;
}
#endif


int zpl_media_text_bitmap_attr(zpl_media_text_t *text, zpl_uint32 bgcolor)
{
    text->m_font.m_bgcolor = bgcolor;
    return OK;
}

int zpl_media_text_font_attr(zpl_media_text_t *text, zpl_uint8 fontsplit, zpl_uint8 fontline)
{
    text->m_font.m_fontsplit = fontsplit;    //字体间隔
    text->m_font.m_fontline = fontline;      //字体行间距
    return OK;
}

int zpl_media_text_bitmap_show(zpl_media_text_t *text, zpl_bool bshow, zpl_char *osdstring, zpl_uint32 pixel,
                               zpl_uint32 color, zpl_bool bold)
{
    text->m_font.m_bold = bold;
    text->m_font.m_fontwidth = pixel;     //字体大小
    text->m_font.m_fontheight= pixel;     //字体大小
    text->m_font.m_fontcolor = color;     //字体颜色
    if(zpl_media_text_bitmap_size(text) != OK)
        return ERROR;
#ifdef ZPL_FREETYPE_ENABLE
    FT_Error error = 0;
    error = FT_Set_Pixel_Sizes(_freetype_face, pixel, pixel);
    if (error)
    {
        return ERROR;
    }
#endif
    zpl_media_text_bitmap_clean(&text->m_bitmap, text->m_font.m_bgcolor);
    zpl_media_text_bitmap_build(&text->m_bitmap,  osdstring, &text->m_font);
    return OK;
}

int zpl_media_text_bitmap_auto_size(zpl_media_text_t *text, zpl_char *osdstring, zpl_uint32 pixel, zpl_rect_t *m_rect)
{
    zpl_uint32 len = strlen(osdstring);
    zpl_uint8      m_fontwidth = 16;     //字体大小
    zpl_uint8      m_fontheight = 16;    //字体大小
    zpl_char *p = osdstring;
    zpl_uint8 linenum = 1;
    while(p && *p != '\0' && len)
    {
        if(*p == '\n')
            linenum++;
        p++;
        len--;  
    }
    len = strlen(osdstring);
    if(pixel == ZPL_FONT_SIZE_16X16)
    {
        m_fontwidth = 16;     //字体大小
        m_fontheight = 16;    //字体大小
    }
    else if(pixel == ZPL_FONT_SIZE_24X24)
    {
        m_fontwidth = 24;     //字体大小
        m_fontheight = 24;    //字体大小
    }
    else if(pixel == ZPL_FONT_SIZE_32X32)
    {
        m_fontwidth = 32;     //字体大小
        m_fontheight = 32;    //字体大小
    }
    else if(pixel == ZPL_FONT_SIZE_48X48)
    {
        m_fontwidth = 48;     //字体大小
        m_fontheight = 48;    //字体大小
    }
    if(text)
    {
        len = m_fontwidth * len + text->m_font.m_fontsplit*(len-1);
        m_rect->width = len + 6;
        if(linenum < 1)
            m_rect->height = m_fontheight  + 6;
        else
            m_rect->height = m_fontheight * linenum + text->m_font.m_fontline*(linenum-1) + 6;
    }
    else
    {
        len = m_fontwidth * len;
        m_rect->width = len + 6;
        if(linenum < 1)
            m_rect->height = m_fontheight  + 6;
        else
            m_rect->height = m_fontheight * linenum + 6;
    }
    return OK;
}


static int zpl_media_text_bitmap_size(zpl_media_text_t *text)
{
    int bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 2;

    switch(text->m_bitmap.enPixelFormat)
    {
    case BITMAP_PIXEL_FORMAT_RGB_444:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_RGB_555:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_RGB_565:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_RGB_888:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 4;
        break;
        ////
    case BITMAP_PIXEL_FORMAT_BGR_444:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_BGR_555:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_BGR_565:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_BGR_888:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 4;
        break;
        ////
    case BITMAP_PIXEL_FORMAT_ARGB_1555:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_4444:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_8565:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 4;
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_8888:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 4;
        break;

        ////
    case BITMAP_PIXEL_FORMAT_ABGR_1555:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_4444:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 2;
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_8565:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 4;
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_8888:
        bitsize = text->m_bitmap.u32Width * text->m_bitmap.u32Height * 4;
        break;
    }
    if(text->m_bitmap.u32len != bitsize)
    {
        if(text->m_bitmap.pData)
            free(text->m_bitmap.pData);
        text->m_bitmap.pData = malloc(bitsize);
        if (text->m_bitmap.pData == NULL)
            return ERROR;
    }
    else
    {
        if(text->m_bitmap.pData == NULL)
            text->m_bitmap.pData = malloc(bitsize);
        if (text->m_bitmap.pData == NULL)
            return ERROR;
    }
    return OK;
}


zpl_media_text_t * zpl_media_text_bitmap_create(zpl_char *filename)
{
    zpl_media_text_t *text = malloc(sizeof(zpl_media_text_t));
    if(text)
    {
        memset(text, 0, sizeof(zpl_media_text_t));

        text->m_bitmap.enPixelFormat = BITMAP_PIXEL_FORMAT_RGB_565;
#ifdef ZPL_FREETYPE_ENABLE
        if(zpl_freetype_text_init(&text->m_font, filename) == OK)
        {
            return text;
        }
        free(text);
        text = NULL;
#endif
        return text;
    }
    return NULL;
}

int zpl_media_text_bitmap_destroy(zpl_media_text_t *text)
{
#ifdef ZPL_FREETYPE_ENABLE
    if(zpl_freetype_text_exit(&text->m_font) != OK)
    {
        return ERROR;
    }
#endif
    if(text->m_bitmap.pData)
        free(text->m_bitmap.pData);
    free(text);
    text = NULL;
    return OK;
}

/*
#include "loadbmp.h"

int zpl_media_text_bitmap_text()
{
    zpl_media_text_t *text = zpl_media_text_bitmap_create(NULL);

    if(text)
    {

        zpl_video_channel_attr(text, 1, 1, 0, 0xffff);
        zpl_video_channel_osd_show(text, "ABCD", 16, 0XBB, false);
        UGL_BITMAP_ID *bmp = uglBitMapCreate("./abc.bmp", text->m_bitmap.u32Width, text->m_bitmap.u32Height, BITMAP_BITCOUNT_16);
        uglBitMapWrite(bmp, text->m_bitmap.pData, text->m_bitmap.u32len);//
        uglBitMapClose(bmp);//

        zpl_media_text_bitmap_destroy(text);
    }
}
*/