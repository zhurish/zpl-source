/*
 * zpl_video_region.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_video_region.h"

static LIST *video_encode_list = NULL;
static os_mutex_t *video_encode_mutex = NULL;



static int zpl_video_region_freetype_init(zpl_fontmapaa_t *fontbitmap, char* filename)
{
  FT_Error      error;
  //char*         filename = "C:/WINDOWS/Fonts/Corbel.ttf";

  error = FT_Init_FreeType( &_freetype_library );              /* initialize library */
  if(error)
  {
      return ERROR;
  }
  /* error handling omitted */

  error = FT_New_Face( _freetype_library, filename, 0, &_freetype_face );/* create face object */
  if(error)
  {
      return ERROR;
  }
  /* error handling omitted */
  //error = FT_Select_Charmap(face, ft_encoding_unicode);
  FT_Select_Charmap(_freetype_face, FT_ENCODING_UNICODE);
  if(error)
  {
      return ERROR;
  }
  error = FT_Set_Pixel_Sizes(_freetype_face, 16, 0);
  if(error)
  {
      return ERROR;
  }
  fontbitmap->m_fontbitmap = _freetype_face;
  /* error handling omitted */
  return OK;
}

static int zpl_video_region_freetype_exit(zpl_fontmapaa_t *fontbitmap)
{
    if(fontbitmap && fontbitmap->m_fontbitmap)
    {
        FT_Done_Face( fontbitmap->m_fontbitmap );
        _freetype_face = fontbitmap->m_fontbitmap = NULL;
    }
    /*
    if(_freetype_face)
    {
        FT_Done_Face( _freetype_face );
        _freetype_face = NULL;
    }
    */
    if(_freetype_library)
    {
        FT_Done_FreeType( _freetype_library );
        _freetype_library = NULL;
    }
    return OK;
}
#endif

static int zpl_video_region_bitmap_init(zpl_video_size_t vidsize, zpl_uint32 pixel, zpl_uint32 bg, zpl_bitmap_t *vidbitmap)
{
    int bitsize = vidsize.width * vidsize.height * 2;
    vidbitmap->u32Width = vidsize.width;
    vidbitmap->u32Height = vidsize.height;
    vidbitmap->enPixelFormat = pixel;
    vidbitmap->pData = malloc(bitsize);
    if (vidbitmap->pData == NULL)
        return ERROR;
    memset(vidbitmap->pData, bg, bitsize);
    return OK;
}



int zpl_video_region_exit(zpl_video_region_t *region)
{

#ifdef ZPL_FREETYPE_MODULE
    zpl_video_region_freetype_exit(&region->m_font)
#endif
    return OK;
}

static int zpl_video_region_bitmap_exit(zpl_bitmap_t *vidbitmap)
{
    if (vidbitmap->pData != NULL)
        free(vidbitmap->pData);
    vidbitmap->pData = NULL;
    return OK;
}

static int zpl_video_region_bitmap_clean(zpl_bitmap_t *vidbitmap, zpl_uint32 color)
{
    if (vidbitmap->pData)
    {
        memset(vidbitmap->pData, color & 0xff, vidbitmap->u32Width * vidbitmap->u32Height * 2);
        return OK;
    }
    return ERROR;
}

static int zpl_video_region_bitmap_build_pixel(zpl_bitmap_t *vidbitmap, zpl_uint32 x,
                                               zpl_uint32 y, zpl_uint32 color)
{
    zpl_uint16 *bmp = (zpl_uint16 *)(vidbitmap->pData + ((x + vidbitmap->u32Width * y) * 2));
    *bmp = color;
    return OK;
}

#ifdef ZPL_FREETYPE_MODULE
static void zpl_video_region_bitmap_build_draw_key(zpl_bitmap_t *vidbitmap, FT_Bitmap*  bitmap, zpl_uint32 fw, zpl_uint32 fh, 
             FT_Int x,
             FT_Int y,
             zpl_uint32 color)
{
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;

  /* for simplicity, we assume that `bitmap->pixel_mode' */
  /* is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)   */

  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
      if ( i < 0      || j < 0       ||
           i >= vidbitmap->u32Width || j >= vidbitmap->u32Height )
        continue;
        if(bitmap->buffer[q * bitmap->width + p])
            zpl_video_region_bitmap_build_pixel(vidbitmap, i, j, color);
        //image[j][i] |= bitmap->buffer[q * bitmap->width + p];
    }
  }
}
#else
/*输出字模的函数*/
/*函数的功能是输出一个宽度为w,高度为h的字模到屏幕的 (x,y) 坐标出,文字的颜色为 color,文字的点阵数据为 pdata 所指*/
static void zpl_video_region_bitmap_build_draw_key(zpl_bitmap_t *vidbitmap, zpl_uint8 *pdata, zpl_uint32 fw, zpl_uint32 fh, 
    zpl_uint32 x, zpl_uint32 y, zpl_uint32 color)
{
    zpl_uint32 i = 0; /* 控制行 */
    zpl_uint32 j = 0; /* 控制一行中的8个点 */
    zpl_uint32 k = 0; /* 一行中的第几个"8个点"了 */
    zpl_uint32 nc = 0; /* 到点阵数据的第几个字节了 */
    zpl_uint32 cols = 0; /* 控制列 */
    zpl_uint32 mask[8] = {128, 64, 32, 16, 8, 4, 2, 1}; /* 位屏蔽字 */
    zpl_uint32 ifw = (fw + 7) / 8 * 8; /* 重新计算w */
    nc = 0;
    for (i = 0; i < fh; i++)
    {
        cols = 0;
        for (k = 0; k < ifw / 8; k++)
        {
            for (j = 0; j < 8; j++)
            {
                if (pdata[nc] & mask[j])
                    zpl_video_region_bitmap_build_pixel(vidbitmap, x + cols, y + i, color);
                cols++;
            }
            nc++;
        }
    }
}
#endif

static int zpl_video_region_bitmap_build_char(zpl_bitmap_t *vidbitmap, zpl_uint32 x,
                    zpl_uint32 y, zpl_fontmapaa_t *fontbitmap)
{
    zpl_video_region_bitmap_build_draw_key(vidbitmap, fontbitmap->m_fontbitmap, fontbitmap->m_fontwidth, 
                fontbitmap->m_fontheight, x, y, fontbitmap->m_fontcolor);
    return OK;
}
#ifdef ZPL_FREETYPE_MODULE
static int zpl_video_region_bitmap_build(zpl_bitmap_t *vidbitmap, zpl_uint8 *osdstr, zpl_fontmapaa_t *fontbitmap)
{
    zpl_uint8 *str = osdstr;
    zpl_uint32 n;
    FT_Vector     pen; 
    FT_GlyphSlot  slot;
    slot = fontbitmap->m_fontface->glyph;
    pen.x = 0 * fontbitmap->m_fontwidth;
    pen.y = /*( vidbitmap->u32Height - 5)*/1 * fontbitmap->m_fontwidth;
    for ( n = 0; n < strlen(osdstr); n++ )
    {
        /* set transformation */
        FT_Set_Transform( fontbitmap->m_fontface, NULL, &pen );
        /* load glyph image into the slot (erase previous one) */
        error = FT_Load_Char( fontbitmap->m_fontface, str[n], FT_LOAD_RENDER );
        if ( error )
            continue;  
        slot = ((FT_Face)fontbitmap->m_fontface)->glyph;    
        fontbitmap->m_fontbitmap = &slot->bitmap;
        zpl_video_region_bitmap_build_char(vidbitmap, slot->bitmap_left, slot->bitmap_top * (n+1)/*vidbitmap->u32Height - slot->bitmap_top*/ , fontbitmap);

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
#else
static int zpl_video_region_bitmap_build(zpl_bitmap_t *vidbitmap, zpl_uint8 *osdstr, zpl_fontmapaa_t *fontbitmap)
{
    zpl_uint8 *str = osdstr;
    zpl_uint32 fw =16, fh = 16;
    zpl_uint32 x = 0, y = 0;
    zpl_uint8 *keybitmap;
    while(*str != '\0')
    {
        //TODO GET FONT BITMAP
        zpl_video_region_bitmap_build_char(vidbitmap, x, y, fontbitmap);
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
zpl_video_region_t *zpl_video_region_lookup(zpl_void *channel,
                                                    ZPL_VIDEO_REGION_TYPE_E type, zpl_uint32 index)
{
    zpl_media_channel_t *chn = (zpl_media_channel_t *)channel;
    if (!chn)
        return NULL;
    if (type == ZPL_VIDEO_RGN_OSD_CHANNEL)
        return chn->vdregion[ZPL_VIDEO_RGN_OSD_CHANNEL];
    else if (type == ZPL_VIDEO_RGN_OSD_TIME)
        return chn->vdregion[ZPL_VIDEO_RGN_OSD_TIME];
    else if (type == ZPL_VIDEO_RGN_OSD_BITRATE)
        return chn->vdregion[ZPL_VIDEO_RGN_OSD_BITRATE];

    else if (type == ZPL_VIDEO_RGN_OVERLAY)
        return chn->vdregion[ZPL_VIDEO_RGN_OVERLAY + index];
    else if (type == ZPL_VIDEO_RGN_MOSAIC)
        return chn->vdregion[ZPL_VIDEO_RGN_MOSAIC + index];

    else if (type == ZPL_VIDEO_RGN_INTERESTED)
        return chn->vdregion[ZPL_VIDEO_RGN_INTERESTED + index];
    return NULL;
}

zpl_video_region_t *zpl_video_region_create(zpl_void *channel, ZPL_VIDEO_REGION_TYPE_E type, zpl_uint32 index)
{
    zpl_media_channel_t *chn = (zpl_media_channel_t *)channel;
    zpl_video_region_t *rgn = malloc(sizeof(zpl_video_region_t));
    if (rgn)
    {
        memset(rgn, 0, sizeof(zpl_video_region_t));
        rgn->m_active = zpl_true;
        rgn->m_type = type;
        //rgn->m_bitmap;
        //rgn->m_bitmap_back;
        if (type == ZPL_VIDEO_RGN_OSD_CHANNEL)
            chn->vdregion[ZPL_VIDEO_RGN_OSD_CHANNEL] = rgn;
        else if (type == ZPL_VIDEO_RGN_OSD_TIME)
            chn->vdregion[ZPL_VIDEO_RGN_OSD_TIME] = rgn;
        else if (type == ZPL_VIDEO_RGN_OSD_BITRATE)
            chn->vdregion[ZPL_VIDEO_RGN_OSD_BITRATE] = rgn;
        else if (type == ZPL_VIDEO_RGN_OVERLAY)
            chn->vdregion[ZPL_VIDEO_RGN_OVERLAY + index] = rgn;
        else if (type == ZPL_VIDEO_RGN_MOSAIC)
            chn->vdregion[ZPL_VIDEO_RGN_MOSAIC + index] = rgn;
        else if (type == ZPL_VIDEO_RGN_INTERESTED)
            chn->vdregion[ZPL_VIDEO_RGN_INTERESTED + index] = rgn;

    #ifdef ZPL_FREETYPE_MODULE
        if(_freetype_library == NULL)
            zpl_video_region_freetype_init(rgn, "./abcd");
    #endif
        return rgn;
    }
    return NULL;
}

int zpl_video_region_destroy(zpl_void *channel,
                                     ZPL_VIDEO_REGION_TYPE_E type, zpl_uint32 index)
{
    zpl_media_channel_t *chn = (zpl_media_channel_t *)channel;
    if (type == ZPL_VIDEO_RGN_OSD_CHANNEL)
    {
        if (chn->vdregion[ZPL_VIDEO_RGN_OSD_CHANNEL])
        {
            zpl_video_region_bitmap_exit(&chn->vdregion[ZPL_VIDEO_RGN_OSD_CHANNEL]->m_bitmap);
            free(chn->vdregion[ZPL_VIDEO_RGN_OSD_CHANNEL]);
            chn->vdregion[ZPL_VIDEO_RGN_OSD_CHANNEL] = NULL;
            return OK;
        }
    }
    else if (type == ZPL_VIDEO_RGN_OSD_TIME)
    {
        if (chn->vdregion[ZPL_VIDEO_RGN_OSD_TIME])
        {
            zpl_video_region_bitmap_exit(&chn->vdregion[ZPL_VIDEO_RGN_OSD_TIME]->m_bitmap);
            free(chn->vdregion[ZPL_VIDEO_RGN_OSD_TIME]);
            chn->vdregion[ZPL_VIDEO_RGN_OSD_TIME] = NULL;
            return OK;
        }
    }
    else if (type == ZPL_VIDEO_RGN_OSD_BITRATE)
    {
        if (chn->vdregion[ZPL_VIDEO_RGN_OSD_BITRATE])
        {
            zpl_video_region_bitmap_exit(&chn->vdregion[ZPL_VIDEO_RGN_OSD_BITRATE]->m_bitmap);
            free(chn->vdregion[ZPL_VIDEO_RGN_OSD_BITRATE]);
            chn->vdregion[ZPL_VIDEO_RGN_OSD_BITRATE] = NULL;
            return OK;
        }
    }
    else if (type == ZPL_VIDEO_RGN_OVERLAY)
    {
        if (chn->vdregion[ZPL_VIDEO_RGN_OVERLAY + index])
        {
            zpl_video_region_bitmap_exit(&chn->vdregion[ZPL_VIDEO_RGN_OVERLAY + index]->m_bitmap);
            free(chn->vdregion[ZPL_VIDEO_RGN_OVERLAY + index]);
            chn->vdregion[ZPL_VIDEO_RGN_OVERLAY + index] = NULL;
            return OK;
        }
    }

    else if (type == ZPL_VIDEO_RGN_MOSAIC)
    {
        if (chn->vdregion[ZPL_VIDEO_RGN_MOSAIC + index])
        {
            zpl_video_region_bitmap_exit(&chn->vdregion[ZPL_VIDEO_RGN_MOSAIC + index]->m_bitmap);
            free(chn->vdregion[ZPL_VIDEO_RGN_MOSAIC + index]);
            chn->vdregion[ZPL_VIDEO_RGN_MOSAIC + index] = NULL;
            return OK;
        }
    }
    else if (type == ZPL_VIDEO_RGN_INTERESTED)
    {
        if (chn->vdregion[ZPL_VIDEO_RGN_INTERESTED + index])
        {
            zpl_video_region_bitmap_exit(&chn->vdregion[ZPL_VIDEO_RGN_INTERESTED + index]->m_bitmap);
            free(chn->vdregion[ZPL_VIDEO_RGN_INTERESTED + index]);
            chn->vdregion[ZPL_VIDEO_RGN_INTERESTED + index] = NULL;
            return OK;
        }
    }
    return ERROR;
}

int zpl_video_region_setup(zpl_video_region_t *region, zpl_rect_t rect, zpl_uint32 fontsize)
{
    zpl_video_size_t vidsize;
    region->m_rect = rect;         //区域位置大小
    region->m_fontsize = fontsize; //字体大小
    vidsize.width = rect.width;
    vidsize.height = rect.height;
    #ifdef ZPL_FREETYPE_MODULE
    if(_freetype_face != NULL)
        region->m_font.m_fontbitmap = _freetype_face;
    #endif
    zpl_video_region_bitmap_init(vidsize, OSD_COLOR_FMT_RGB1555, 0, &region->mbitmap);
    return OK;
}

int zpl_video_region_active(zpl_video_region_t *region)
{
    zpl_vidhal_region_create(region);
    return OK;
}
int zpl_video_region_inactive(zpl_video_region_t *region)
{
    zpl_vidhal_region_destroy(region);
    return OK;
}

/*
int zpl_vidhal_region_attachtochannel(zpl_video_region_t *region, zpl_uint32 modeid, zpl_uint32 devid, 
        zpl_uint32 chnid, zpl_bool attach);

int zpl_vidhal_region_update_attribute(zpl_video_region_t *region);        
int zpl_vidhal_region_update_channel_attribute(zpl_video_region_t *region, zpl_uint32 modeid, 
        zpl_uint32 devid, zpl_uint32 chnid);
*/

int zpl_video_region_show(zpl_video_region_t *region, zpl_bool show)
{
    zpl_video_region_bitmap_clean(&region->m_bitmap, 0);
    zpl_video_region_bitmap_build(&region->m_bitmap, "abcd", &region->m_font);
    //int zpl_vidhal_region_channel_show(zpl_video_region_t *region, zpl_uint32 modeid,
    //    zpl_uint32 devid, zpl_uint32 chnid, zpl_bool show)
    return OK;
}

int zpl_video_channel_osd_show(zpl_video_region_t *region, zpl_char *osdstring, zpl_bool show)
{
    zpl_video_region_bitmap_clean(&region->m_bitmap, 0);
    zpl_video_region_bitmap_build(&region->m_bitmap, osdstring, &region->m_font);
    return OK;
}

int zpl_video_region_update(zpl_video_region_t *region, zpl_point_t point)
{
    return OK;
}


