/*
 * zpl_vidhal.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_input.h"
#include "zpl_vidhal_vpss.h"
#include "zpl_vidhal_venc.h"

#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"
#endif

int zpl_video_hal_scale(void *inframe, void *outframe, zpl_video_size_t vidsize)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    VIDEO_FRAME_INFO_S *in_frame = (VIDEO_FRAME_INFO_S *)inframe;
    VIDEO_FRAME_INFO_S *out_frame = (VIDEO_FRAME_INFO_S *)outframe;
    s32Ret = zpl_vidhal_vgs_scale_job(in_frame, out_frame, VGS_SCLCOEF_NORMAL, vidsize.width, vidsize.height);
    if (s32Ret != HI_SUCCESS)
    {
        //if (VIDEO_ISDEBUG(ERROR))
        {
            zpl_media_debugmsg_debug("vgs scale error:%s", zpl_syshal_strerror(s32Ret));
        }
        return ERROR;
    }
    return OK;
#else
    return ERROR;
#endif
}

int zpl_video_hal_exchange(void *inframe, zpl_video_size_t vidsize)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    VIDEO_FRAME_INFO_S *in_frame = (VIDEO_FRAME_INFO_S *)inframe;
    VIDEO_FRAME_INFO_S *out_frame = NULL; //(VIDEO_FRAME_INFO_S *)outframe;
    s32Ret = zpl_vidhal_vgs_scale_job(in_frame, out_frame, VGS_SCLCOEF_NORMAL, vidsize.width, vidsize.height);
    if (s32Ret != HI_SUCCESS)
    {
        if (ZPL_MEDIA_DEBUG(IVE, ERROR))
        {
            zpl_media_debugmsg_debug("vgs scale error:%s", zpl_syshal_strerror(s32Ret));
        }
        return ERROR;
    }
    return OK;
#else
    return ERROR;
#endif
}

/* 把src图片覆盖到 dst 图片的一个位置，*/
int zpl_video_hal_orign(void *srcframe, void *dstframe, zpl_point_t vidpoint)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    VIDEO_FRAME_INFO_S *in_frame = (VIDEO_FRAME_INFO_S *)srcframe;
    VIDEO_FRAME_INFO_S *out_frame = (VIDEO_FRAME_INFO_S *)dstframe;

    IVE_SRC_IMAGE_S stSrcImage;
    IVE_DST_IMAGE_S stDstImage;

    //out_frame->stVFrame.u64PTS = task.src_yuv_frame1->stVFrame.u64PTS;
    {
        stDstImage.enType = IVE_IMAGE_TYPE_YUV420SP;
        // Y分量
        int in_frame_y_offset = 0;
        stSrcImage.au64PhyAddr[0] = in_frame->stVFrame.u64PhyAddr[0];
        stSrcImage.u32Width = in_frame->stVFrame.u32Width;
        stSrcImage.u32Height = in_frame->stVFrame.u32Height;
        stSrcImage.au32Stride[0] = in_frame->stVFrame.u32Stride[0];
        //右上角
        in_frame_y_offset = vidpoint.x + (vidpoint.y) * out_frame->stVFrame.u32Stride[0] + (out_frame->stVFrame.u32Width - in_frame->stVFrame.u32Width);

        stDstImage.au64PhyAddr[0] = out_frame->stVFrame.u64PhyAddr[0] + in_frame_y_offset;
        stDstImage.u32Width = in_frame->stVFrame.u32Width;
        stDstImage.u32Height = in_frame->stVFrame.u32Height;
        stDstImage.au32Stride[0] = out_frame->stVFrame.u32Stride[0];
        s32Ret = zpl_vidhal_ive_dma_image2(&stSrcImage, &stDstImage, HI_FALSE);

        if (HI_SUCCESS != s32Ret)
        {
            if (ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(IVE, HARDWARE))
                zpl_media_debugmsg_err("ds_dma_mode_copy fail,Error(%#x),in_frame->u32Width:%d,in_frame->u32Height:%d,out_frame.u32Width:%d,out_frame.u32Height:%d\n", s32Ret, in_frame->stVFrame.u32Width,
                                       in_frame->stVFrame.u32Height, out_frame->stVFrame.u32Width, out_frame->stVFrame.u32Height);
            return ERROR;
        }

        // UV分量
        stSrcImage.au64PhyAddr[0] = in_frame->stVFrame.u64PhyAddr[1];
        stSrcImage.u32Width = in_frame->stVFrame.u32Width;
        stSrcImage.u32Height = in_frame->stVFrame.u32Height / 2;
        stSrcImage.au32Stride[0] = in_frame->stVFrame.u32Stride[1];
        //右上角
        in_frame_y_offset = (vidpoint.x) + (vidpoint.y / 2) * (out_frame->stVFrame.u32Stride[1]) + (out_frame->stVFrame.u32Width - in_frame->stVFrame.u32Width);

        stDstImage.au64PhyAddr[0] = out_frame->stVFrame.u64PhyAddr[1] + in_frame_y_offset;
        stDstImage.u32Width = stSrcImage.u32Width;
        stDstImage.u32Height = stSrcImage.u32Height;
        stDstImage.au32Stride[0] = out_frame->stVFrame.u32Stride[1];
        s32Ret = zpl_vidhal_ive_dma_image2(&stSrcImage, &stDstImage, HI_FALSE);
        if (HI_SUCCESS != s32Ret)
        {
            if (ZPL_MEDIA_DEBUG(IVE, EVENT) && ZPL_MEDIA_DEBUG(IVE, HARDWARE))
                zpl_media_debugmsg_err("ds_dma_mode_copy fail,Error(%#x)\n", s32Ret);
            return ERROR;
        }
    }
    return OK;
#else
    return ERROR;
#endif
}

int zpl_video_hal_memblock_init(zpl_media_memblock_t *pmembuf, zpl_uint32 blocksize)
{
    if (!pmembuf)
    {
        return ERROR;
    }
    //memset(pmembuf, 0, sizeof(zpl_media_memblock_t));
    pmembuf->uBlkSize = (zpl_uint32)blocksize;
#ifdef ZPL_HISIMPP_MODULE
    pmembuf->hBlock = HI_MPI_VB_GetBlock((VB_POOL)(-1), pmembuf->uBlkSize, NULL);
    if (VB_INVALID_HANDLE == pmembuf->hBlock)
    {
        zpl_video_hal_memblock_deinit(pmembuf);
        return ERROR;
    }
    pmembuf->hPool = HI_MPI_VB_Handle2PoolId(pmembuf->hBlock);
    if (VB_INVALID_POOLID == pmembuf->hPool)
    {
        zpl_video_hal_memblock_deinit(pmembuf);
        return ERROR;
    }
    pmembuf->uPhyAddr = HI_MPI_VB_Handle2PhysAddr(pmembuf->hBlock);
    if (0U == pmembuf->uPhyAddr)
    {
        zpl_video_hal_memblock_deinit(pmembuf);
        return ERROR;
    }
    pmembuf->pVirAddr = (HI_U8 *)HI_MPI_SYS_Mmap(pmembuf->uPhyAddr, pmembuf->uBlkSize);
    if (pmembuf->pVirAddr == HI_NULL)
    {
        zpl_video_hal_memblock_deinit(pmembuf);
        return ERROR;
    }
#endif
    return OK;
}

int zpl_video_hal_memblock_deinit(zpl_media_memblock_t *pmembuf)
{
    if (!pmembuf)
    {
        return ERROR;
    }
#ifdef ZPL_HISIMPP_MODULE
    if (HI_NULL != pmembuf->pVirAddr)
    {
        HI_MPI_SYS_Munmap((HI_VOID *)pmembuf->pVirAddr, pmembuf->uBlkSize);
        pmembuf->uPhyAddr = HI_NULL;
        pmembuf->pVirAddr = HI_NULL;
        pmembuf->uBlkSize = 0U;
    }
    if (VB_INVALID_HANDLE != pmembuf->hBlock)
    {
        HI_MPI_VB_ReleaseBlock(pmembuf->hBlock);
        pmembuf->hBlock = VB_INVALID_HANDLE;
    }
    pmembuf->hPool = VB_INVALID_POOLID;
#endif
    return OK;
}


zpl_uint32  zpl_video_hal_frame_blksize(void *inframe)
{
    zpl_uint32 blocksize = 0;
#ifdef ZPL_HISIMPP_MODULE
    VIDEO_FRAME_INFO_S *srcframe = inframe;
    blocksize = zpl_syshal_get_membuf_size(srcframe->stVFrame.u32Width, srcframe->stVFrame.u32Height,
                            srcframe->stVFrame.enPixelFormat, 
                            DATA_BITWIDTH_8, 
                            srcframe->stVFrame.enCompressMode, 
                            DEFAULT_ALIGN);
#endif
    return blocksize;
}

void * zpl_video_hal_frame_clone(void *inframe, zpl_media_memblock_t *memblk)
{
#ifdef ZPL_HISIMPP_MODULE
    VIDEO_FRAME_INFO_S *clone_frame = NULL;
    int s32Ret       = -1;
    VIDEO_FRAME_INFO_S *srcframe = inframe;
    clone_frame = malloc(sizeof(VIDEO_FRAME_INFO_S));
    if(!memblk || !srcframe || !clone_frame)
    {
        if(clone_frame)
            free(clone_frame);
        return NULL;
    }
    
    memset(&clone_frame->stVFrame, 0x00, sizeof(VIDEO_FRAME_S));
    memcpy(&clone_frame->stVFrame, &srcframe->stVFrame, sizeof(VIDEO_FRAME_S));

    //memset(memblk, 0, sizeof(zpl_media_memblock_t));
    //memblk->hBlock = VB_INVALID_HANDLE;
    //memblk->hPool = VB_INVALID_POOLID;
    /*video_frame->base_info.u32Width = frame_width;
    video_frame->base_info.u32Height = frame_height;
    video_frame->base_info.enPixelFormat = format;
    video_frame->base_info.enCompressMode = compress_mode;
    video_frame->base_info.u32Align = 0;
    COMMON_GetPicBufferConfig(video_frame->base_info.u32Width, video_frame->base_info.u32Height,
                              video_frame->base_info.enPixelFormat,
                              DATA_BITWIDTH_8, video_frame->base_info.enCompressMode, 0, &video_frame->stCalConfig);
                              */
    //s32Ret = zpl_video_memblock_init(memblk,  memblk->uBlkSize);
    if(s32Ret != OK)
    {
        if(clone_frame)
            free(clone_frame);
       return NULL;
    }
    clone_frame->u32PoolId               = memblk->hPool;
    clone_frame->stVFrame.u32Width       = srcframe->stVFrame.u32Width;
    clone_frame->stVFrame.u32Height      = srcframe->stVFrame.u32Height;

    clone_frame->stVFrame.u32Stride[0]   = srcframe->stVFrame.u32Stride[0];
    clone_frame->stVFrame.u32Stride[1]   = srcframe->stVFrame.u32Stride[1];
    clone_frame->stVFrame.u32Stride[2]   = srcframe->stVFrame.u32Stride[2];

    clone_frame->stVFrame.u64PhyAddr[0]  = memblk->uPhyAddr;
    clone_frame->stVFrame.u64PhyAddr[1]  = clone_frame->stVFrame.u64PhyAddr[0] + (srcframe->stVFrame.u64PhyAddr[1] - srcframe->stVFrame.u64PhyAddr[0]);
    clone_frame->stVFrame.u64PhyAddr[2]  = clone_frame->stVFrame.u64PhyAddr[0] + (srcframe->stVFrame.u64PhyAddr[2] - srcframe->stVFrame.u64PhyAddr[0]);
    
    clone_frame->stVFrame.u64VirAddr[0]    = (HI_U64)memblk->pVirAddr;
    clone_frame->stVFrame.u64VirAddr[1]    = clone_frame->stVFrame.u64VirAddr[0] + (srcframe->stVFrame.u64VirAddr[1] - srcframe->stVFrame.u64VirAddr[0]);
    clone_frame->stVFrame.u64VirAddr[3]    = clone_frame->stVFrame.u64VirAddr[0] + (srcframe->stVFrame.u64VirAddr[2] - srcframe->stVFrame.u64VirAddr[0]);

/*
    clone_frame->stVFrame.u32HeaderStride[0]   = srcframe->stVFrame.u32HeaderStride[0];
    clone_frame->stVFrame.u32HeaderStride[0]  = clone_frame->stVFrame.u32HeaderStride[0] + (srcframe->stVFrame.u32HeaderStride[1] - srcframe->stVFrame.u32HeaderStride[0]);;
    clone_frame->stVFrame.u32HeaderStride[0]    = clone_frame->stVFrame.u32HeaderStride[0] + (srcframe->stVFrame.u32HeaderStride[2] - srcframe->stVFrame.u32HeaderStride[0]);

    clone_frame->stVFrame.u32ExtStride[0]   = srcframe->stVFrame.u32ExtStride[0];
    clone_frame->stVFrame.u32ExtStride[0]  = clone_frame->stVFrame.u32ExtStride[0] + (srcframe->stVFrame.u32ExtStride[1] - srcframe->stVFrame.u32ExtStride[0]);;
    clone_frame->stVFrame.u32ExtStride[0]    = clone_frame->stVFrame.u32ExtStride[0] + (srcframe->stVFrame.u32ExtStride[2] - srcframe->stVFrame.u32ExtStride[0]);

    clone_frame->stVFrame.u64HeaderPhyAddr[0]   = srcframe->stVFrame.u64HeaderPhyAddr[0];
    clone_frame->stVFrame.u64HeaderPhyAddr[0]  = clone_frame->stVFrame.u64HeaderPhyAddr[0] + (srcframe->stVFrame.u64HeaderPhyAddr[1] - srcframe->stVFrame.u64HeaderPhyAddr[0]);;
    clone_frame->stVFrame.u64HeaderPhyAddr[0]    = clone_frame->stVFrame.u64HeaderPhyAddr[0] + (srcframe->stVFrame.u64HeaderPhyAddr[2] - srcframe->stVFrame.u64HeaderPhyAddr[0]);

    clone_frame->stVFrame.u64HeaderVirAddr[0]   = srcframe->stVFrame.u64HeaderVirAddr[0];
    clone_frame->stVFrame.u64HeaderVirAddr[0]  = clone_frame->stVFrame.u64HeaderVirAddr[0] + (srcframe->stVFrame.u64HeaderVirAddr[1] - srcframe->stVFrame.u64HeaderVirAddr[0]);;
    clone_frame->stVFrame.u64HeaderVirAddr[0]    = clone_frame->stVFrame.u64HeaderVirAddr[0] + (srcframe->stVFrame.u64HeaderVirAddr[2] - srcframe->stVFrame.u64HeaderVirAddr[0]);

    clone_frame->stVFrame.u64ExtPhyAddr[0]   = srcframe->stVFrame.u64ExtPhyAddr[0];
    clone_frame->stVFrame.u64ExtPhyAddr[0]  = clone_frame->stVFrame.u64ExtPhyAddr[0] + (srcframe->stVFrame.u64ExtPhyAddr[1] - srcframe->stVFrame.u64ExtPhyAddr[0]);;
    clone_frame->stVFrame.u64ExtPhyAddr[0]    = clone_frame->stVFrame.u64ExtPhyAddr[0] + (srcframe->stVFrame.u64ExtPhyAddr[2] - srcframe->stVFrame.u64ExtPhyAddr[0]);

    clone_frame->stVFrame.u64ExtVirAddr[0]   = srcframe->stVFrame.u64ExtVirAddr[0];
    clone_frame->stVFrame.u64ExtVirAddr[0]  = clone_frame->stVFrame.u64ExtVirAddr[0] + (srcframe->stVFrame.u64ExtVirAddr[1] - srcframe->stVFrame.u64ExtVirAddr[0]);;
    clone_frame->stVFrame.u64ExtVirAddr[0]    = clone_frame->stVFrame.u64ExtVirAddr[0] + (srcframe->stVFrame.u64ExtVirAddr[2] - srcframe->stVFrame.u64ExtVirAddr[0]);
*/
    if(zpl_vidhal_ive_dma_frame(srcframe, clone_frame) == HI_SUCCESS)
        return clone_frame;
    else
    {
        //zpl_video_memblock_deinit(memblk);
        free(clone_frame);
        clone_frame = NULL;
        return NULL;
    }   
#endif 
    return NULL;
}
