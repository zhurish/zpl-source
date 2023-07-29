/*
 * zpl_vidhal_region.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_internal.h"

int zpl_vidhal_region_mst_load_bmp(const char *filename, zpl_media_bitmap_t *pstBitmap, zpl_bool bFil, 
        zpl_uint32 u16FilColor, zpl_uint32 enPixelFormat)
{
#ifdef ZPL_HISIMPP_MODULE    
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;
    int s32BytesPerPix = 2;
    zpl_uint8* pu8Data;
    int R_Value;
    int G_Value;
    int B_Value;
    int Gr_Value;
    zpl_uint8  Value_tmp;
    zpl_uint8  Value;
    int s32Width;

    if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0)
    {
        if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            printf("GetBmpInfo err!\n");
        return HI_FAILURE;
    }

     if(enPixelFormat == PIXEL_FORMAT_ARGB_4444)
    {
        Surface.enColorFmt =OSD_COLOR_FMT_RGB4444;
    }
    else if(enPixelFormat == PIXEL_FORMAT_ARGB_1555 || enPixelFormat == PIXEL_FORMAT_ARGB_2BPP)
    {
        Surface.enColorFmt =OSD_COLOR_FMT_RGB1555;
    }
    else if(enPixelFormat == PIXEL_FORMAT_ARGB_8888)
    {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
        s32BytesPerPix = 4;
    }
    else
    {
        if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            printf("enPixelFormat err %d \n",enPixelFormat);
        return HI_FAILURE;
    }

    pstBitmap->pData = malloc(s32BytesPerPix * (bmpInfo.bmiHeader.biWidth) * (bmpInfo.bmiHeader.biHeight));

    if (NULL == pstBitmap->pData)
    {
        if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            printf("malloc osd memroy err!\n");
        return HI_FAILURE;
    }


    CreateSurfaceByBitMap(filename, &Surface, (zpl_uint8*)(pstBitmap->pData));

    pstBitmap->u32Width = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;
    pstBitmap->enPixelFormat = enPixelFormat;

    zpl_uint32 i = 0, j = 0, k = 0;
    zpl_uint8*  pu8Temp = NULL;
    if (PIXEL_FORMAT_ARGB_2BPP == enPixelFormat)
    {
        s32Width = DIV_UP (bmpInfo.bmiHeader.biWidth,4);
        pu8Data = malloc((s32Width)* (bmpInfo.bmiHeader.biHeight));
        if (NULL == pu8Data)
        {
            if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
                printf("malloc osd memroy err!\n");
            return HI_FAILURE;
        }
    }
    if (PIXEL_FORMAT_ARGB_2BPP != enPixelFormat)
    {

        zpl_uint16* pu16Temp = NULL;

        pu16Temp = (zpl_uint16*)pstBitmap->pData;

        if (bFil)
        {
            for (i = 0; i < pstBitmap->u32Height; i++)
            {
                for (j = 0; j < pstBitmap->u32Width; j++)
                {
                    if (u16FilColor == *pu16Temp)
                    {
                        *pu16Temp &= 0x7FFF;
                    }

                    pu16Temp++;
                }
            }
        }
    }
    else
    {
        zpl_uint16 *pu16Temp = NULL;
        pu16Temp = (zpl_uint16*)pstBitmap->pData;
        pu8Temp = (zpl_uint8*)pu8Data;
        for (i = 0; i < pstBitmap->u32Height; i++)
        {
            for (j = 0; j < pstBitmap->u32Width/4; j++)
            {
                Value = 0;
                for (k = j; k < j + 4; k++)
                {
                    B_Value = *pu16Temp & 0x001F;
                    G_Value = *pu16Temp >> 5 & 0x001F;
                    R_Value = *pu16Temp >> 10 & 0x001F;
                    pu16Temp++;
                    Gr_Value = (R_Value * 299 + G_Value * 587 + B_Value * 144 + 500) / 1000;
                    if (Gr_Value > 16)
                    {
                        Value_tmp = 0x01;
                    }
                    else
                    {
                        Value_tmp = 0x00;
                    }
                    Value = (Value << 2) + Value_tmp;
                }
                *pu8Temp = Value;
                pu8Temp++;
            }
        }
        free(pstBitmap->pData);
        pstBitmap->pData = pu8Data;
    }

    return HI_SUCCESS;
    #else
    return ERROR;
    #endif
}

int zpl_vidhal_region_load_canvas(const char* filename, zpl_media_bitmap_t* pstBitmap, zpl_bool bFil,
                               zpl_uint32 u16FilColor, zpl_video_size_t* pstSize, 
                               zpl_uint32 u32Stride, zpl_uint32 enPixelFmt)
{
    #ifdef ZPL_HISIMPP_MODULE
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;

    if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0)
    {
        if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            printf("GetBmpInfo err!\n");
        return HI_FAILURE;
    }

    if (PIXEL_FORMAT_ARGB_1555 == enPixelFmt)
    {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
    }
    else if (PIXEL_FORMAT_ARGB_4444 == enPixelFmt)
    {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB4444;
    }
    else if (PIXEL_FORMAT_ARGB_8888 == enPixelFmt)
    {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
    }
    else
    {
        if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            printf("Pixel format is not support!\n");
        return HI_FAILURE;
    }

    if (NULL == pstBitmap->pData)
    {
        if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            printf("malloc osd memroy err!\n");
        return HI_FAILURE;
    }

    CreateSurfaceByCanvas(filename, &Surface, (zpl_uint8*)(pstBitmap->pData), pstSize->width, pstSize->height, u32Stride);

    pstBitmap->u32Width = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;

    if (PIXEL_FORMAT_ARGB_1555 == enPixelFmt)
    {
        pstBitmap->enPixelFormat = PIXEL_FORMAT_ARGB_1555;
    }
    else if (PIXEL_FORMAT_ARGB_4444 == enPixelFmt)
    {
        pstBitmap->enPixelFormat = PIXEL_FORMAT_ARGB_4444;
    }
    else if (PIXEL_FORMAT_ARGB_8888 == enPixelFmt)
    {
        pstBitmap->enPixelFormat = PIXEL_FORMAT_ARGB_8888;
    }

    int i, j;
    zpl_uint16* pu16Temp;
    pu16Temp = (zpl_uint16*)pstBitmap->pData;

    if (bFil)
    {
        for (i = 0; i < pstBitmap->u32Height; i++)
        {
            for (j = 0; j < pstBitmap->u32Width; j++)
            {
                if (u16FilColor == *pu16Temp)
                {
                    *pu16Temp &= 0x7FFF;
                }

                pu16Temp++;
            }
        }

    }
    return HI_SUCCESS;
    #else
    return ERROR;
    #endif
}



#ifdef ZPL_HISIMPP_MODULE
static int zpl_vidhal_region_chn_attribute_default(zpl_media_video_hwregion_t *region, RGN_CHN_ATTR_S *stChnAttr)
{
    /*set the chn config*/
    stChnAttr->bShow = HI_FALSE;
    switch(region->rgn_type)
    {
        case ZPL_OVERLAY_RGN:
            stChnAttr->bShow = HI_TRUE;
            stChnAttr->enType = OVERLAY_RGN;

            stChnAttr->unChnAttr.stOverlayChn.u32BgAlpha = region->bg_alpha;//背景透明，0-128;越小越透明；
            stChnAttr->unChnAttr.stOverlayChn.u32FgAlpha = region->fg_alpha;//图像透明，0-128;越小越透明；

            stChnAttr->unChnAttr.stOverlayChn.stQpInfo.bQpDisable = HI_FALSE;
            stChnAttr->unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
            stChnAttr->unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;

            stChnAttr->unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16;
            stChnAttr->unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width = 16;
            stChnAttr->unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 0;
            stChnAttr->unChnAttr.stOverlayChn.stInvertColor.enChgMod = LESSTHAN_LUM_THRESH;
            stChnAttr->unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_FALSE;
            stChnAttr->unChnAttr.stOverlayChn.u16ColorLUT[0] = 0;
            stChnAttr->unChnAttr.stOverlayChn.u16ColorLUT[1] = 0;
            stChnAttr->unChnAttr.stOverlayChn.enAttachDest = ATTACH_JPEG_MAIN;
            break;

        case ZPL_OVERLAYEX_RGN:
            stChnAttr->bShow = HI_FALSE;
            stChnAttr->enType = OVERLAYEX_RGN;
            stChnAttr->unChnAttr.stOverlayExChn.u32BgAlpha = region->fg_alpha;
            stChnAttr->unChnAttr.stOverlayExChn.u32FgAlpha = region->bg_alpha;
            break;

        case ZPL_COVER_RGN:
            stChnAttr->bShow = HI_FALSE;
            stChnAttr->enType = COVER_RGN;
            stChnAttr->unChnAttr.stCoverChn.enCoverType = AREA_RECT;

            stChnAttr->unChnAttr.stCoverChn.stRect.u32Height = region->m_rect.height;
            stChnAttr->unChnAttr.stCoverChn.stRect.u32Width  = region->m_rect.width;
            stChnAttr->unChnAttr.stCoverChn.u32Color      = 0x0000ffff;
            stChnAttr->unChnAttr.stCoverChn.enCoordinate = RGN_ABS_COOR;
            break;

        case ZPL_COVEREX_RGN:
            stChnAttr->bShow = HI_FALSE;
            stChnAttr->enType = COVEREX_RGN;
            stChnAttr->unChnAttr.stCoverExChn.enCoverType = AREA_RECT;
            stChnAttr->unChnAttr.stCoverExChn.stRect.u32Height = region->m_rect.height;
            stChnAttr->unChnAttr.stCoverExChn.stRect.u32Width  = region->m_rect.width;
            stChnAttr->unChnAttr.stCoverExChn.u32Color      = 0x0000ffff;
            break;

        case ZPL_MOSAIC_RGN:
            stChnAttr->enType = MOSAIC_RGN;
            stChnAttr->unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_32;
            stChnAttr->unChnAttr.stMosaicChn.stRect.u32Height = region->m_rect.height;
            stChnAttr->unChnAttr.stMosaicChn.stRect.u32Width  = region->m_rect.width;
            break;
        default:
                break;
    }
    if(ZPL_OVERLAY_RGN == region->rgn_type)
    {
        stChnAttr->unChnAttr.stOverlayChn.stPoint.s32X = region->m_rect.x;
        stChnAttr->unChnAttr.stOverlayChn.stPoint.s32Y = region->m_rect.y;
        stChnAttr->unChnAttr.stOverlayChn.u32Layer = region->rgn_layer;
    }
    if(ZPL_OVERLAYEX_RGN == region->rgn_type)
    {
        stChnAttr->unChnAttr.stOverlayExChn.stPoint.s32X = region->m_rect.x;
        stChnAttr->unChnAttr.stOverlayExChn.stPoint.s32Y = region->m_rect.y;
        stChnAttr->unChnAttr.stOverlayExChn.u32Layer = region->rgn_layer;
    }
    if(ZPL_COVER_RGN == region->rgn_type)
    {
        stChnAttr->unChnAttr.stCoverChn.stRect.s32X = region->m_rect.x;
        stChnAttr->unChnAttr.stCoverChn.stRect.s32Y = region->m_rect.y;
        stChnAttr->unChnAttr.stCoverChn.u32Layer = region->rgn_layer;
    }
    if(ZPL_COVEREX_RGN == region->rgn_type)
    {
        stChnAttr->unChnAttr.stCoverExChn.stRect.s32X = region->m_rect.x;
        stChnAttr->unChnAttr.stCoverExChn.stRect.s32Y = region->m_rect.y;
        stChnAttr->unChnAttr.stCoverExChn.u32Layer = region->rgn_layer;
    }
    if(ZPL_MOSAIC_RGN == region->rgn_type)
    {
        stChnAttr->unChnAttr.stMosaicChn.stRect.s32X = region->m_rect.x;
        stChnAttr->unChnAttr.stMosaicChn.stRect.s32Y = region->m_rect.y;
        stChnAttr->unChnAttr.stMosaicChn.u32Layer = region->rgn_layer;
    }
    zm_msg_debug(" area attr [%d,%d %d,%d] layer %d alpha %d", region->m_rect.x, region->m_rect.y, 
        region->m_rect.width, region->m_rect.height, region->rgn_layer, region->bg_alpha);
    return OK;
}
#endif


int zpl_vidhal_region_set_bitmap(zpl_media_video_hwregion_t *region, zpl_media_bitmap_t *pstBitmap)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret;
    BITMAP_S bitmap;
                                   /* Bitmap's pixel format */
    bitmap.u32Width = pstBitmap->u32Width;   /* Bitmap's width */
    bitmap.u32Height = pstBitmap->u32Height; /* Bitmap's height */
    bitmap.pData = pstBitmap->pData;         /* Address of Bitmap's data */
    switch (pstBitmap->enPixelFormat)
    {
    case BITMAP_PIXEL_FORMAT_RGB_444:
        bitmap.enPixelFormat = PIXEL_FORMAT_RGB_444;
        break;
    case BITMAP_PIXEL_FORMAT_RGB_555:
        bitmap.enPixelFormat = PIXEL_FORMAT_RGB_555;
        break;
    case BITMAP_PIXEL_FORMAT_RGB_565:
        bitmap.enPixelFormat = PIXEL_FORMAT_RGB_565;
        break;
    case BITMAP_PIXEL_FORMAT_RGB_888:
        bitmap.enPixelFormat = PIXEL_FORMAT_RGB_888;
        break;

    case BITMAP_PIXEL_FORMAT_BGR_444:
        bitmap.enPixelFormat = PIXEL_FORMAT_BGR_444;
        break;
    case BITMAP_PIXEL_FORMAT_BGR_555:
        bitmap.enPixelFormat = PIXEL_FORMAT_BGR_555;
        break;
    case BITMAP_PIXEL_FORMAT_BGR_565:
        bitmap.enPixelFormat = PIXEL_FORMAT_BGR_565;
        break;
    case BITMAP_PIXEL_FORMAT_BGR_888:
        bitmap.enPixelFormat = PIXEL_FORMAT_BGR_888;
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_1555:
        bitmap.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_4444:
        bitmap.enPixelFormat = PIXEL_FORMAT_ARGB_4444;
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_8565:
        bitmap.enPixelFormat = PIXEL_FORMAT_ARGB_8565;
        break;
    case BITMAP_PIXEL_FORMAT_ARGB_8888:
        bitmap.enPixelFormat = PIXEL_FORMAT_ARGB_8888;
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_1555:
        bitmap.enPixelFormat = PIXEL_FORMAT_ABGR_1555;
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_4444:
        bitmap.enPixelFormat = PIXEL_FORMAT_ABGR_4444;
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_8565:
        bitmap.enPixelFormat = PIXEL_FORMAT_ABGR_8565;
        break;
    case BITMAP_PIXEL_FORMAT_ABGR_8888:
        bitmap.enPixelFormat = PIXEL_FORMAT_ABGR_8888;
        break;
    }
    s32Ret = HI_MPI_RGN_SetBitMap(region->rgn_handle, &bitmap);
    if (HI_SUCCESS != s32Ret)
    {
        // if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
        zm_msg_err("HI_MPI_RGN_SetBitMap failed, %s:%#x! rng=%d pdata=%p\n", zpl_syshal_strerror(s32Ret), s32Ret, region->rgn_handle, bitmap.pData);
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_region_update_canvas(zpl_media_video_hwregion_t *region)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret;
    RGN_CANVAS_INFO_S stCanvasInfo;
    s32Ret = HI_MPI_RGN_GetCanvasInfo(region->rgn_handle, &stCanvasInfo);
    if(HI_SUCCESS != s32Ret)
    {
        //if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            zm_msg_err("HI_MPI_RGN_GetCanvasInfo failed, %s:%#x!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_RGN_UpdateCanvas(region->rgn_handle);
    if(HI_SUCCESS != s32Ret)
    {
        //if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            zm_msg_err("HI_MPI_RGN_UpdateCanvas failed, %s:%#x!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return ERROR;
#endif 
}

int zpl_vidhal_region_attachtochannel(zpl_media_video_hwregion_t *region, zpl_bool attach)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret;
    MPP_CHN_S stChn;

    stChn.enModId = region->rgn_chn.modId; 
    stChn.s32DevId = region->rgn_chn.devId; 
    stChn.s32ChnId = region->rgn_chn.chnId; 

    if(attach)
    {
        RGN_CHN_ATTR_S stChnAttr;
        memset(&stChnAttr, 0, sizeof(RGN_CHN_ATTR_S));
        zpl_vidhal_region_chn_attribute_default(region, &stChnAttr);

        s32Ret = HI_MPI_RGN_AttachToChn(region->rgn_handle, &stChn, &stChnAttr);
        if(HI_SUCCESS != s32Ret)
        {
            //if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
                zm_msg_err("HI_MPI_RGN_AttachToChn failed, %s:%#x!\n", zpl_syshal_strerror(s32Ret), s32Ret);
            return HI_FAILURE;
        }
    }
    else
    {
        s32Ret = HI_MPI_RGN_DetachFromChn(region->rgn_handle, &stChn);
        if(HI_SUCCESS != s32Ret)
        {
            //if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
                zm_msg_err("HI_MPI_RGN_DetachFromChn failed, %s:%#x!\n", zpl_syshal_strerror(s32Ret), s32Ret);
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif 
}




int zpl_vidhal_region_channel_show(zpl_media_video_hwregion_t *region, zpl_bool show)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    stChn.enModId = region->rgn_chn.modId; 
    stChn.s32DevId = region->rgn_chn.devId; 
    stChn.s32ChnId = region->rgn_chn.chnId; 

    s32Ret = HI_MPI_RGN_GetDisplayAttr(region->rgn_handle, &stChn, &stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        //if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            zm_msg_err("HI_MPI_RGN_GetDisplayAttr failed, %s:%#x!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        return HI_FAILURE;
    }
    if(ZPL_OVERLAY_RGN == region->rgn_type)
    {
        stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = region->fg_alpha;//Alpha 位为 1 的像素点的透明度，0-128;越小越透明；
        stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = region->bg_alpha;//Alpha 位为 0 的像素点的透明度，0-128;越小越透明；

        //stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = region->m_rect.height;
        //stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width = region->m_rect.width;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = region->m_rect.x;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = region->m_rect.y;
        stChnAttr.unChnAttr.stOverlayChn.u32Layer = region->rgn_layer;
    }
    if(ZPL_OVERLAYEX_RGN == region->rgn_type)
    {
        stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha = region->fg_alpha;
        stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha = region->bg_alpha;
        stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = region->m_rect.x;
        stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = region->m_rect.y;
        stChnAttr.unChnAttr.stOverlayExChn.u32Layer = region->rgn_layer;
    }
    if(ZPL_COVER_RGN == region->rgn_type)
    {
        stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = region->m_rect.height;
        stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = region->m_rect.width;
        stChnAttr.unChnAttr.stCoverChn.u32Color      = 0x0000ffff;        
        stChnAttr.unChnAttr.stCoverChn.stRect.s32X = region->m_rect.x;
        stChnAttr.unChnAttr.stCoverChn.stRect.s32Y = region->m_rect.y;
        stChnAttr.unChnAttr.stCoverChn.u32Layer = region->rgn_layer;
    }
    if(ZPL_COVEREX_RGN == region->rgn_type)
    {
        stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = region->m_rect.height;
        stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = region->m_rect.width;
        stChnAttr.unChnAttr.stCoverExChn.u32Color      = 0x0000ffff;
        stChnAttr.unChnAttr.stCoverExChn.stRect.s32X = region->m_rect.x;
        stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y = region->m_rect.y;
        stChnAttr.unChnAttr.stCoverExChn.u32Layer = region->rgn_layer;
    }
    if(ZPL_MOSAIC_RGN == region->rgn_type)
    {
        stChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_32;
        stChnAttr.unChnAttr.stMosaicChn.stRect.u32Height = region->m_rect.height;
        stChnAttr.unChnAttr.stMosaicChn.stRect.u32Width  = region->m_rect.width;
        stChnAttr.unChnAttr.stMosaicChn.stRect.s32X = region->m_rect.x;
        stChnAttr.unChnAttr.stMosaicChn.stRect.s32Y = region->m_rect.y;
        stChnAttr.unChnAttr.stMosaicChn.u32Layer = region->rgn_layer;
    }    
    stChnAttr.bShow = (show) ? HI_TRUE : HI_FALSE;
    s32Ret = HI_MPI_RGN_SetDisplayAttr(region->rgn_handle, &stChn, &stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        //if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            zm_msg_err("HI_MPI_RGN_SetDisplayAttr failed, %s:%#x!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return ERROR;
#endif 
}

int zpl_vidhal_region_update(zpl_media_video_hwregion_t *region)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret;
    RGN_ATTR_S stRegion;
    s32Ret = HI_MPI_RGN_GetAttr(region->rgn_handle, &stRegion);
    if(HI_SUCCESS != s32Ret)
    {
        //if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            zm_msg_err("HI_MPI_RGN_GetAttr failed, %s:%#x!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        return HI_FAILURE;
    }

	if(region->rgn_type == ZPL_OVERLAY_RGN)
	{
		//stRegion.enType = OVERLAY_RGN;
		//stRegion.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_ARGB_1555;
		stRegion.unAttr.stOverlay.stSize.u32Height = region->m_rect.height;
		stRegion.unAttr.stOverlay.stSize.u32Width  = region->m_rect.width;
		stRegion.unAttr.stOverlay.u32BgColor = region->bg_color;
		//stRegion.unAttr.stOverlay.u32CanvasNum = 2;
        zm_msg_debug("HI_MPI_RGN_SetAttr overlay width %d height %d %d\n", region->m_rect.width, region->m_rect.height, region->bg_color);
	}
	else if(region->rgn_type == ZPL_OVERLAYEX_RGN)
	{
		//stRegion.enType = OVERLAYEX_RGN;
		//stRegion.unAttr.stOverlayEx.enPixelFmt = PIXEL_FORMAT_ARGB_1555;
		stRegion.unAttr.stOverlayEx.stSize.u32Height = region->m_rect.height;
		stRegion.unAttr.stOverlayEx.stSize.u32Width  = region->m_rect.width;
		stRegion.unAttr.stOverlayEx.u32BgColor = region->bg_color;
		//stRegion.unAttr.stOverlayEx.u32CanvasNum =2;
        zm_msg_debug("HI_MPI_RGN_SetAttr overlayex width %d height %d %d\n", region->m_rect.width, region->m_rect.height, region->bg_color);
	}
    s32Ret = HI_MPI_RGN_SetAttr(region->rgn_handle, &stRegion);
    if(HI_SUCCESS != s32Ret)
    {
        //if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            zm_msg_err("HI_MPI_RGN_SetAttr failed, %s:%#x!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return ERROR;
#endif 
}

int zpl_vidhal_region_create(zpl_media_video_hwregion_t *region)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret;
    RGN_ATTR_S stRegion;
	if(region->rgn_type == ZPL_COVEREX_RGN)
	{
		stRegion.enType = COVEREX_RGN;
	}
	else if(region->rgn_type == ZPL_MOSAIC_RGN)
	{
		stRegion.enType = MOSAIC_RGN;
	}
	else if(region->rgn_type == ZPL_COVER_RGN)
	{
		stRegion.enType = COVER_RGN;
	}
	else if(region->rgn_type == ZPL_OVERLAY_RGN)
	{
		stRegion.enType = OVERLAY_RGN;
		stRegion.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_ARGB_1555;
		stRegion.unAttr.stOverlay.stSize.u32Height = region->m_rect.height;
		stRegion.unAttr.stOverlay.stSize.u32Width  = region->m_rect.width;
		stRegion.unAttr.stOverlay.u32BgColor = region->bg_color;
		stRegion.unAttr.stOverlay.u32CanvasNum = 2;
        zm_msg_debug("HI_MPI_RGN_Create overlay width %d height %d 0x%x\n", region->m_rect.width, region->m_rect.height, region->bg_color);
	}
	else if(region->rgn_type == ZPL_OVERLAYEX_RGN)
	{
		stRegion.enType = OVERLAYEX_RGN;
		stRegion.unAttr.stOverlayEx.enPixelFmt = PIXEL_FORMAT_ARGB_1555;
		stRegion.unAttr.stOverlayEx.stSize.u32Height = region->m_rect.height;
		stRegion.unAttr.stOverlayEx.stSize.u32Width  = region->m_rect.width;
		stRegion.unAttr.stOverlayEx.u32BgColor = region->bg_color;
		stRegion.unAttr.stOverlayEx.u32CanvasNum =2;
        zm_msg_debug("HI_MPI_RGN_Create overlayex width %d height %d 0x%x\n", region->m_rect.width, region->m_rect.height, region->bg_color);
	}

    s32Ret = HI_MPI_RGN_Create(region->rgn_handle, &stRegion);
    if(HI_SUCCESS != s32Ret)
    {
        //if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            zm_msg_err("HI_MPI_RGN_Create failed, %s:%#x!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return ERROR;
#endif 
}



int zpl_vidhal_region_destroy(zpl_media_video_hwregion_t *region)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret;
    s32Ret = HI_MPI_RGN_Destroy(region->rgn_handle);
    if(HI_SUCCESS != s32Ret)
    {
        //if(ZPL_MEDIA_DEBUG(REGION, EVENT) && ZPL_MEDIA_DEBUG(REGION, HARDWARE))
            zm_msg_err("HI_MPI_RGN_Destroy failed, %s:%#x!\n", zpl_syshal_strerror(s32Ret), s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return ERROR;
#endif    
}
