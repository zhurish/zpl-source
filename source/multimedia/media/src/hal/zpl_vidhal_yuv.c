/*
 * zpl_vidhal_yuv.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_internal.h"

#ifdef ZPL_HISIMPP_MODULE



static VB_POOL g_hPool = VB_INVALID_POOLID;
static DUMP_MEMBUF_S g_stMem = {0};
static zpl_uint32 g_u32BlkSize = 0;

#ifdef CONFIG_HI_TDE_SUPPORT
static VB_POOL g_hPool2 = VB_INVALID_POOLID;
static DUMP_MEMBUF_S g_stMem2 = {0};
static zpl_uint32 g_u32BlkSize2 = 0;
#endif

static zpl_char *g_pUserPageAddr = HI_NULL;

typedef struct frame_node_t
{
    unsigned char *mem;
    unsigned int   length;
    unsigned int   used;
    unsigned int   index;
    struct frame_node_t *next;
} frame_node_t;


static HI_VOID sample_yuv_get_buf_size(const VIDEO_FRAME_S *pVFrame, zpl_uint32 *pSize)
{
    PIXEL_FORMAT_E enPixelFormat = pVFrame->enPixelFormat;

    if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == enPixelFormat)
    {
        *pSize = pVFrame->u32Stride[0] * pVFrame->u32Height * 3 / 2;
    }
    else if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == enPixelFormat)
    {
        *pSize = pVFrame->u32Stride[0] * pVFrame->u32Height * 2;
    }
    else if (PIXEL_FORMAT_YUV_400 == enPixelFormat)
    {
        *pSize = pVFrame->u32Stride[0] * pVFrame->u32Height;
    }

    return;
}



#ifdef CONFIG_HI_TDE_SUPPORT
static zpl_int32 sample_yuv_prepare_vb(const VIDEO_FRAME_S *pVFrameInfo, VIDEO_FRAME_S *pVFrame)
{
    zpl_uint32 u32Width = pVFrameInfo->u32Width;
    zpl_uint32 u32Height = pVFrameInfo->u32Height;
    zpl_uint32 u32Align = 32;
    PIXEL_FORMAT_E enPixelFormat = pVFrameInfo->enPixelFormat;
    DATA_BITWIDTH_E enBitWidth = DYNAMIC_RANGE_SDR8;
    COMPRESS_MODE_E enCmpMode = COMPRESS_MODE_NONE;
    VB_CAL_CONFIG_S stCalConfig = { 0 };
    VB_POOL_CONFIG_S stVbPoolCfg = { 0 };

    COMMON_GetPicBufferConfig(u32Width, u32Height, enPixelFormat, enBitWidth, enCmpMode, u32Align, &stCalConfig);

    g_u32BlkSize2 = stCalConfig.u32VBSize;

    stVbPoolCfg.u64BlkSize  = g_u32BlkSize2;
    stVbPoolCfg.u32BlkCnt   = 1;
    stVbPoolCfg.enRemapMode = VB_REMAP_MODE_NONE;

    /*create comm vb pool*/
    g_hPool2 = HI_MPI_VB_CreatePool(&stVbPoolCfg);
    if (VB_INVALID_POOLID == g_hPool2)
    {
        if(ZPL_MEDIA_DEBUG(YUV, EVENT) && ZPL_MEDIA_DEBUG(YUV, HARDWARE))
            printf("HI_MPI_VB_CreatePool failed!\n");
        return HI_FAILURE;
    }

    g_stMem2.hPool = g_hPool2;

    while (VB_INVALID_HANDLE == (g_stMem2.hBlock = HI_MPI_VB_GetBlock(g_stMem2.hPool, g_u32BlkSize2, HI_NULL)))
    {
        ;
    }

    g_stMem2.u64PhyAddr = HI_MPI_VB_Handle2PhysAddr(g_stMem2.hBlock);

    g_stMem2.pVirAddr = (zpl_uint8*)HI_MPI_SYS_Mmap(g_stMem2.u64PhyAddr, g_u32BlkSize2);
    if (HI_NULL == g_stMem2.pVirAddr)
    {
        if(ZPL_MEDIA_DEBUG(YUV, EVENT) && ZPL_MEDIA_DEBUG(YUV, HARDWARE))
            printf("HI_MPI_SYS_Mmap failed!\n");
        HI_MPI_VB_ReleaseBlock(g_stMem2.hBlock);
        g_stMem2.hPool = VB_INVALID_POOLID;
        HI_MPI_VB_DestroyPool(g_hPool2);
        g_hPool2 = VB_INVALID_POOLID;
        return HI_FAILURE;
    }

    pVFrame->u64PhyAddr[0] = g_stMem2.u64PhyAddr;
    pVFrame->u64PhyAddr[1] = pVFrame->u64PhyAddr[0] + stCalConfig.u32MainYSize;

    pVFrame->u64VirAddr[0] = (zpl_uint64)(HI_UL)g_stMem2.pVirAddr;
    pVFrame->u64VirAddr[1] = pVFrame->u64VirAddr[0] + stCalConfig.u32MainYSize;

    pVFrame->u32Width  = u32Width;
    pVFrame->u32Height = u32Height;
    pVFrame->u32Stride[0] = stCalConfig.u32MainStride * 2; //packed 422
    pVFrame->u32Stride[1] = stCalConfig.u32MainStride;

    pVFrame->enCompressMode = COMPRESS_MODE_NONE;
    pVFrame->enPixelFormat  = pVFrameInfo->enPixelFormat;
    pVFrame->enVideoFormat  = VIDEO_FORMAT_LINEAR;
    pVFrame->enDynamicRange = pVFrameInfo->enDynamicRange;

    pVFrame->u64PTS = pVFrameInfo->u64PTS;
    pVFrame->u32TimeRef = pVFrameInfo->u32TimeRef;

    return HI_SUCCESS;
}

static HI_VOID sample_yuv_release_vb(HI_VOID)
{
    if (HI_NULL != g_stMem2.pVirAddr)
    {
        HI_MPI_SYS_Munmap((HI_VOID*)g_stMem2.pVirAddr, g_u32BlkSize2);
        g_stMem2.pVirAddr = HI_NULL;
        g_stMem2.u64PhyAddr = 0;
    }

    if (VB_INVALID_POOLID != g_stMem2.hPool)
    {
        HI_MPI_VB_ReleaseBlock(g_stMem2.hBlock);
        g_stMem2.hPool = VB_INVALID_POOLID;
    }

    if (VB_INVALID_POOLID != g_hPool2)
    {
        HI_MPI_VB_DestroyPool(g_hPool2);
        g_hPool2 = VB_INVALID_POOLID;
    }

    return;
}

static zpl_int32 sample_yuv_do_tde_job(const VIDEO_FRAME_S *pVFrameIn, VIDEO_FRAME_S *pVFrameOut)
{
    zpl_int32 s32Ret = HI_FAILURE;
    TDE_HANDLE tdeHandle;
    TDE2_OPT_S stOpt = { 0 };
    TDE2_SURFACE_S stSurface = { 0 };
    TDE2_SURFACE_S stDestSurface = { 0 };
    TDE2_RECT_S stSrcRect = { 0 };
    TDE2_RECT_S stDestRect = { 0 };

    tdeHandle = HI_TDE2_BeginJob();
    if (HI_ERR_TDE_INVALID_HANDLE == tdeHandle)
    {
        if(ZPL_MEDIA_DEBUG(YUV, EVENT) && ZPL_MEDIA_DEBUG(YUV, HARDWARE))
            printf("HI_TDE2_BeginJob failed!\n");
        return HI_FAILURE;
    }

    /*prepare the in/out image info for TDE.*/
    stSurface.PhyAddr = pVFrameIn->u64PhyAddr[0];
    stSurface.enColorFmt = TDE2_COLOR_FMT_JPG_YCbCr422MBHP;
    stSurface.u32Height = pVFrameIn->u32Height;
    stSurface.u32Width = pVFrameIn->u32Width;
    stSurface.u32Stride = pVFrameIn->u32Stride[0];
    stSurface.CbCrPhyAddr = pVFrameIn->u64PhyAddr[1];
    stSurface.u32CbCrStride = pVFrameIn->u32Stride[1];

    stSrcRect.s32Xpos = 0;
    stSrcRect.s32Ypos = 0;
    stSrcRect.u32Width = stSurface.u32Width;
    stSrcRect.u32Height = stSurface.u32Height;

    stDestSurface.PhyAddr = pVFrameOut->u64PhyAddr[0];
    stDestSurface.enColorFmt = TDE2_COLOR_FMT_PKGVYUY;
    stDestSurface.u32Height = pVFrameOut->u32Height;
    stDestSurface.u32Width = pVFrameOut->u32Width;
    stDestSurface.u32Stride = pVFrameOut->u32Stride[0];

    stDestRect.s32Xpos = 0;
    stDestRect.s32Ypos = 0;
    stDestRect.u32Width = stDestSurface.u32Width;
    stDestRect.u32Height = stDestSurface.u32Height;

    s32Ret = HI_TDE2_Bitblit(tdeHandle, HI_NULL, HI_NULL, &stSurface, &stSrcRect, &stDestSurface, &stDestRect, &stOpt);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(YUV, EVENT) && ZPL_MEDIA_DEBUG(YUV, HARDWARE))
            printf("HI_TDE2_Bitblit failed:0x%x\n", s32Ret);
        HI_TDE2_CancelJob(tdeHandle);
        return s32Ret;
    }

    s32Ret = HI_TDE2_EndJob(tdeHandle, HI_FALSE, HI_TRUE, 1000);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(YUV, EVENT) && ZPL_MEDIA_DEBUG(YUV, HARDWARE))
            printf("HI_TDE2_EndJob failed:0x%x\n", s32Ret);
        HI_TDE2_CancelJob(tdeHandle);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static zpl_int32 sample_yuv_sp422_to_p422(const VIDEO_FRAME_S *pVFrame, frame_node_t *pFNode)
{
    zpl_int32 s32Ret = HI_FAILURE;
    VIDEO_FRAME_INFO_S stFrmInfo = { 0 };
    zpl_uint8 *node_ptr = pFNode->mem;
    zpl_uint32 h = 0;
    zpl_char *y_ptr = HI_NULL;

    s32Ret = HI_TDE2_Open();
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(YUV, EVENT) && ZPL_MEDIA_DEBUG(YUV, HARDWARE))
            printf("HI_TDE2_Open failed:0x%x\n", s32Ret);
        return s32Ret;
    }

    if (sample_yuv_prepare_vb(pVFrame, &stFrmInfo.stVFrame) != HI_SUCCESS)
    {
        HI_TDE2_Close();
        return HI_FAILURE;
    }

    /*convert sp422 to packed422(YUYV) by TDE.*/
    if (sample_yuv_do_tde_job(pVFrame, &stFrmInfo.stVFrame) != HI_SUCCESS)
    {
        sample_yuv_release_vb();
        HI_TDE2_Close();
        return HI_FAILURE;
    }

    HI_TDE2_Close();

    pFNode->used = 0;
    for (h = 0; h < stFrmInfo.stVFrame.u32Height; ++h)
    {
        y_ptr = (zpl_char *)(HI_UL)stFrmInfo.stVFrame.u64VirAddr[0] + h * stFrmInfo.stVFrame.u32Stride[0];

        if (pFNode->used + stFrmInfo.stVFrame.u32Width <= pFNode->length)
        {
            memcpy(node_ptr + pFNode->used, y_ptr, stFrmInfo.stVFrame.u32Width * 2);
            pFNode->used += stFrmInfo.stVFrame.u32Width * 2;
        }
    }

    sample_yuv_release_vb();

    return HI_SUCCESS;
}
#else /*else if #ifdef CONFIG_HI_TDE_SUPPORT*/
static zpl_int32 sample_yuv_sp422_to_p422(const VIDEO_FRAME_S *pVFrame, frame_node_t *pFNode)
{
    zpl_uint32 w = 0;
    zpl_uint32 h = 0;
    zpl_char *pVBufVirt_Y = HI_NULL;
    zpl_char *pVBufVirt_C = HI_NULL;
    zpl_char *y_ptr = HI_NULL;
    zpl_char *uv_ptr = HI_NULL;
    zpl_uint8 *node_ptr = pFNode->mem;
    pFNode->used = 0;

    pVBufVirt_Y = g_pUserPageAddr;
    pVBufVirt_C = pVBufVirt_Y + pVFrame->u32Stride[0] * pVFrame->u32Height;

    for (h = 0; h < pVFrame->u32Height; ++h)
    {
        y_ptr = pVBufVirt_Y + h * pVFrame->u32Stride[0];
        uv_ptr = pVBufVirt_C + h * pVFrame->u32Stride[1];

        for (w = 0; w < pVFrame->u32Width; w += 2)
        {
            if (pFNode->used + 4 <= pFNode->length)
            {

                node_ptr[pFNode->used] = *(y_ptr + w);
                node_ptr[pFNode->used + 2] = *(y_ptr + w + 1);
                node_ptr[pFNode->used + 1] = *(uv_ptr + w + 1);
                node_ptr[pFNode->used + 3] = *(uv_ptr + w);
                pFNode->used += 4;
            }
        }
    }

    return HI_SUCCESS;
}
#endif /*end of #ifdef CONFIG_HI_TDE_SUPPORT*/

static HI_VOID sample_yuv_sp420_to_p420(const VIDEO_FRAME_S *pVFrame, frame_node_t *pFNode)
{
    zpl_uint32 w = 0;
    zpl_uint32 h = 0;
    zpl_char *pVBufVirt_Y = HI_NULL;
    zpl_char *pVBufVirt_C = HI_NULL;
    zpl_char *y_ptr = HI_NULL;
    zpl_char *uv_ptr = HI_NULL;
    zpl_uint8 *node_ptr = pFNode->mem;
    pFNode->used = 0;
#if (1 == UVC_SAVE_FILE)
    zpl_uint8 TmpBuff[8192];
#endif

    pVBufVirt_Y = g_pUserPageAddr;
    pVBufVirt_C = pVBufVirt_Y + pVFrame->u32Stride[0] * pVFrame->u32Height;

    for (h = 0; h < pVFrame->u32Height; ++h)
    {
        y_ptr = pVBufVirt_Y + h * pVFrame->u32Stride[0];

        if (pFNode->used + pVFrame->u32Width <= pFNode->length)
        {
            memcpy(node_ptr + pFNode->used, y_ptr, pVFrame->u32Width);
            pFNode->used += pVFrame->u32Width;
        }
#if (1 == UVC_SAVE_FILE)
        fwrite(y_ptr, pVFrame->u32Width, 1, g_pfd);
        fflush(g_pfd);
#endif
    }

    for (h = 0; h < pVFrame->u32Height / 2; ++h)
    {
        uv_ptr = pVBufVirt_C + h * pVFrame->u32Stride[1];

        for (w = 0; w < pVFrame->u32Width; w += 2)
        {
            if (pFNode->used + 1 <= pFNode->length)
            {
                node_ptr[pFNode->used] = *(uv_ptr + w + 1);
                pFNode->used++;
            }
#if (1 == UVC_SAVE_FILE)
            TmpBuff[w / 2] = *(uv_ptr + w + 1);
#endif
        }
#if (1 == UVC_SAVE_FILE)
        fwrite(TmpBuff, pVFrame->u32Width / 2, 1, g_pfd);
        fflush(g_pfd);
#endif
    }

    for (h = 0; h < pVFrame->u32Height / 2; ++h)
    {
        uv_ptr = pVBufVirt_C + h * pVFrame->u32Stride[1];

        for (w = 0; w < pVFrame->u32Width; w += 2)
        {
            if (pFNode->used + 1 <= pFNode->length)
            {
                node_ptr[pFNode->used] = *(uv_ptr + w);
                pFNode->used++;
            }
#if (1 == UVC_SAVE_FILE)
            TmpBuff[w / 2] = *(uv_ptr + w);
#endif
        }
#if (1 == UVC_SAVE_FILE)
        fwrite(TmpBuff, pVFrame->u32Width / 2, 1, g_pfd);
        fflush(g_pfd);
#endif
    }

    return;
}



/*When saving a file, sp420 will be denoted by p420 and sp422 will be denoted by p422 in the name of the file.*/
int sample_yuv_dump(VIDEO_FRAME_S *pVBuf)
{
    PIXEL_FORMAT_E enPixelFormat = pVBuf->enPixelFormat;
    zpl_uint32  g_u32Size;
    frame_node_t *fnode = HI_NULL;
    sample_yuv_get_buf_size(pVBuf, &g_u32Size);
    g_pUserPageAddr = (zpl_char*)HI_MPI_SYS_Mmap(pVBuf->u64PhyAddr[0], g_u32Size);
    if (HI_NULL == g_pUserPageAddr)
    {
        goto ERR;
    }

    if (PIXEL_FORMAT_YVU_SEMIPLANAR_422 == enPixelFormat)
    {
        if (sample_yuv_sp422_to_p422(pVBuf, fnode) != HI_SUCCESS)
        {
            goto ERR;
        }
    }
    else if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == enPixelFormat)
    {
        sample_yuv_sp420_to_p420(pVBuf, fnode);
    }
    else
    {
    }

ERR:
    if (g_pUserPageAddr)
    {
        HI_MPI_SYS_Munmap(g_pUserPageAddr, g_u32Size);
        g_pUserPageAddr = HI_NULL;
    }
    return;
}