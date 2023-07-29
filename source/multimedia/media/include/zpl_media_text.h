/*
 * zpl_media_text.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_TEXT_H__
#define __ZPL_MEDIA_TEXT_H__

#ifdef __cplusplus
extern "C" {
#endif


//#define ZPL_FREETYPE_MODULE



#define ZPL_FREETYPE_PATH       "/nfsroot/DejaVuSans.ttf"

#define BITMAP_ARGB(a,r,g,b)     (((a)<<24)|((r)<<16)|((g)<<8)|(b))

typedef enum {
    ZPL_FONT_SIZE_16X16 = 0,
    ZPL_FONT_SIZE_24X24,
    ZPL_FONT_SIZE_32X32,
    ZPL_FONT_SIZE_48X48,
} ZPL_FONT_SIZE_E;

typedef enum  {
    BITMAP_PIXEL_FORMAT_RGB_444 = 0,
    BITMAP_PIXEL_FORMAT_RGB_555,
    BITMAP_PIXEL_FORMAT_RGB_565,
    BITMAP_PIXEL_FORMAT_RGB_888,

    BITMAP_PIXEL_FORMAT_BGR_444,
    BITMAP_PIXEL_FORMAT_BGR_555,
    BITMAP_PIXEL_FORMAT_BGR_565,
    BITMAP_PIXEL_FORMAT_BGR_888,

    BITMAP_PIXEL_FORMAT_ARGB_1555,
    BITMAP_PIXEL_FORMAT_ARGB_4444,
    BITMAP_PIXEL_FORMAT_ARGB_8565,
    BITMAP_PIXEL_FORMAT_ARGB_8888,

    BITMAP_PIXEL_FORMAT_ABGR_1555,
    BITMAP_PIXEL_FORMAT_ABGR_4444,
    BITMAP_PIXEL_FORMAT_ABGR_8565,
    BITMAP_PIXEL_FORMAT_ABGR_8888,
}BITMAP_PIXEL_FORMAT_E;

typedef struct  {
    BITMAP_PIXEL_FORMAT_E enPixelFormat;  /* Bitmap's pixel format */
    zpl_uint32 u32Width;               /* Bitmap's width */
    zpl_uint32 u32Height;              /* Bitmap's height */
    zpl_void*  pData;      /* Address of Bitmap's data */
    zpl_uint32 u32len;
    zpl_uint8  stride;
} zpl_media_bitmap_t;

typedef struct 
{
    zpl_uint8      m_fontwidth;     //字体大小
    zpl_uint8      m_fontheight;    //字体大小
    zpl_uint8      m_fontsplit;     //字体间隔
    zpl_uint8      m_fontline;      //字体行间距
    zpl_uint32     m_fontcolor;     //字体颜色 0x0000 白， 0xffffff 黑
    zpl_uint32     m_bgcolor;       //字体颜色
    zpl_uint8      m_bold:1;
    zpl_uint8      m_res:7;
    zpl_void       *m_fontbitmap;
}zpl_media_fontmap_t;


typedef struct 
{
    ZPL_FONT_SIZE_E f_fmt;
    zpl_media_fontmap_t   m_font;
    zpl_media_bitmap_t    *m_bitmap;       //位图信息
}zpl_media_text_t;
/***************************************************************/
/***************************************************************/
extern zpl_media_text_t * zpl_media_text_create(zpl_media_bitmap_t *bitmap, zpl_char *filename);
extern int zpl_media_text_destroy(zpl_media_text_t *text);

extern int zpl_media_text_attr(zpl_media_text_t *text, zpl_uint32 pixel, zpl_uint32 color, zpl_uint32 bgcolor, zpl_uint8 fontsplit, zpl_uint8 fontline);

extern int zpl_media_text_show(zpl_media_text_t *text, zpl_bool bold, zpl_uint32 color, const char *format,...);



#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_TEXT_H__ */
