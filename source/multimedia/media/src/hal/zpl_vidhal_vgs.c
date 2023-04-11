/*
 * zpl_vidhal_isp.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_internal.h"

#ifdef ZPL_HISIMPP_MODULE


ZPL_VGS_VB_INFO g_stInImgVbInfo;
ZPL_VGS_VB_INFO g_stOutImgVbInfo;

int zpl_vidhal_vgs_vb_release(void)
{
    if (HI_TRUE == g_stInImgVbInfo.bVbUsed)
    {
        HI_MPI_SYS_Munmap((HI_VOID*)g_stInImgVbInfo.pu8VirAddr, g_stInImgVbInfo.u32VbSize);
        HI_MPI_VB_ReleaseBlock(g_stInImgVbInfo.VbHandle);
        g_stInImgVbInfo.bVbUsed = HI_FALSE;
    }

    if (HI_TRUE == g_stOutImgVbInfo.bVbUsed)
    {
        HI_MPI_SYS_Munmap((HI_VOID*)g_stOutImgVbInfo.pu8VirAddr, g_stOutImgVbInfo.u32VbSize);
        HI_MPI_VB_ReleaseBlock(g_stOutImgVbInfo.VbHandle);
        g_stOutImgVbInfo.bVbUsed = HI_FALSE;
    }
    return OK;
}

static zpl_int32 _zpl_vidhal_vgs_GetFrameVb(const ZPL_VB_BASE_INFO_S *pstVbInfo, const VB_CAL_CONFIG_S *pstVbCalConfig,
                                    VIDEO_FRAME_INFO_S *pstFrameInfo, ZPL_VGS_VB_INFO *pstVgsVbInfo)
{
    zpl_uint64 u64PhyAddr = 0;

    pstVgsVbInfo->VbHandle = HI_MPI_VB_GetBlock(VB_INVALID_POOLID, pstVbCalConfig->u32VBSize, HI_NULL);
    if (VB_INVALID_HANDLE == pstVgsVbInfo->VbHandle)
    {
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VB_GetBlock failed!\n");
        return HI_FAILURE;
    }
    pstVgsVbInfo->bVbUsed = HI_TRUE;

    u64PhyAddr = HI_MPI_VB_Handle2PhysAddr(pstVgsVbInfo->VbHandle);
    if (0 == u64PhyAddr)
    {
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VB_Handle2PhysAddr failed!.\n");
        HI_MPI_VB_ReleaseBlock(pstVgsVbInfo->VbHandle);
        pstVgsVbInfo->bVbUsed = HI_FALSE;
        return HI_FAILURE;
    }

    pstVgsVbInfo->pu8VirAddr = (zpl_uint8*)HI_MPI_SYS_Mmap(u64PhyAddr, pstVbCalConfig->u32VBSize);
    if (HI_NULL == pstVgsVbInfo->pu8VirAddr)
    {
        if(ZPL_MEDIA_DEBUG(SYS, EVENT) && ZPL_MEDIA_DEBUG(SYS, HARDWARE))
            zm_msg_err("HI_MPI_SYS_Mmap failed!.\n");
        HI_MPI_VB_ReleaseBlock(pstVgsVbInfo->VbHandle);
        pstVgsVbInfo->bVbUsed = HI_FALSE;
        return HI_FAILURE;
    }
    pstVgsVbInfo->u32VbSize = pstVbCalConfig->u32VBSize;

    pstFrameInfo->enModId = HI_ID_VGS;
    pstFrameInfo->u32PoolId = HI_MPI_VB_Handle2PoolId(pstVgsVbInfo->VbHandle);

    pstFrameInfo->stVFrame.u32Width       = pstVbInfo->u32Width;
    pstFrameInfo->stVFrame.u32Height      = pstVbInfo->u32Height;
    pstFrameInfo->stVFrame.enField        = VIDEO_FIELD_FRAME;
    pstFrameInfo->stVFrame.enPixelFormat  = pstVbInfo->enPixelFormat;
    pstFrameInfo->stVFrame.enVideoFormat  = VIDEO_FORMAT_LINEAR;
    pstFrameInfo->stVFrame.enCompressMode = pstVbInfo->enCompressMode;
    pstFrameInfo->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;
    pstFrameInfo->stVFrame.enColorGamut   = COLOR_GAMUT_BT601;

    pstFrameInfo->stVFrame.u32HeaderStride[0]  = pstVbCalConfig->u32HeadStride;
    pstFrameInfo->stVFrame.u32HeaderStride[1]  = pstVbCalConfig->u32HeadStride;
    pstFrameInfo->stVFrame.u32HeaderStride[2]  = pstVbCalConfig->u32HeadStride;
    pstFrameInfo->stVFrame.u64HeaderPhyAddr[0] = u64PhyAddr;
    pstFrameInfo->stVFrame.u64HeaderPhyAddr[1] = pstFrameInfo->stVFrame.u64HeaderPhyAddr[0] + pstVbCalConfig->u32HeadYSize;
    pstFrameInfo->stVFrame.u64HeaderPhyAddr[2] = pstFrameInfo->stVFrame.u64HeaderPhyAddr[1];
    pstFrameInfo->stVFrame.u64HeaderVirAddr[0] = (zpl_uint64)(HI_UL)pstVgsVbInfo->pu8VirAddr;
    pstFrameInfo->stVFrame.u64HeaderVirAddr[1] = pstFrameInfo->stVFrame.u64HeaderVirAddr[0] + pstVbCalConfig->u32HeadYSize;
    pstFrameInfo->stVFrame.u64HeaderVirAddr[2] = pstFrameInfo->stVFrame.u64HeaderVirAddr[1];

    pstFrameInfo->stVFrame.u32Stride[0]  = pstVbCalConfig->u32MainStride;
    pstFrameInfo->stVFrame.u32Stride[1]  = pstVbCalConfig->u32MainStride;
    pstFrameInfo->stVFrame.u32Stride[2]  = pstVbCalConfig->u32MainStride;
    pstFrameInfo->stVFrame.u64PhyAddr[0] = pstFrameInfo->stVFrame.u64HeaderPhyAddr[0] + pstVbCalConfig->u32HeadSize;
    pstFrameInfo->stVFrame.u64PhyAddr[1] = pstFrameInfo->stVFrame.u64PhyAddr[0] + pstVbCalConfig->u32MainYSize;
    pstFrameInfo->stVFrame.u64PhyAddr[2] = pstFrameInfo->stVFrame.u64PhyAddr[1];
    pstFrameInfo->stVFrame.u64VirAddr[0] = pstFrameInfo->stVFrame.u64HeaderVirAddr[0] + pstVbCalConfig->u32HeadSize;
    pstFrameInfo->stVFrame.u64VirAddr[1] = pstFrameInfo->stVFrame.u64VirAddr[0] + pstVbCalConfig->u32MainYSize;
    pstFrameInfo->stVFrame.u64VirAddr[2] = pstFrameInfo->stVFrame.u64VirAddr[1];

    return HI_SUCCESS;
}

int zpl_vidhal_vgs_GetYUVBufferCfg(const ZPL_VB_BASE_INFO_S *pstVbBaseInfo, VB_CAL_CONFIG_S *pstVbCalConfig)
{
    COMMON_GetPicBufferConfig(pstVbBaseInfo->u32Width, pstVbBaseInfo->u32Height, pstVbBaseInfo->enPixelFormat,
                              DATA_BITWIDTH_8, pstVbBaseInfo->enCompressMode, pstVbBaseInfo->u32Align, pstVbCalConfig);

    return OK;
}

int zpl_vidhal_vgs_GetFrameVb(const ZPL_VB_BASE_INFO_S *pstVbInfo,
                                    VIDEO_FRAME_INFO_S *pstFrameInfo, ZPL_VGS_VB_INFO *pstVgsVbInfo)
{              
    VB_CAL_CONFIG_S pstVbCalConfig;
    zpl_vidhal_vgs_GetYUVBufferCfg(pstVbInfo, &pstVbCalConfig);                     
    int s32Ret = _zpl_vidhal_vgs_GetFrameVb(pstVbInfo, &pstVbCalConfig, pstFrameInfo, pstVgsVbInfo);
    if (s32Ret != HI_SUCCESS)
    {
        return ERROR;
    }
    return OK;
}

int zpl_vidhal_vgs_scale_job(VIDEO_FRAME_INFO_S *inframe, VIDEO_FRAME_INFO_S *outframe, 
    VGS_SCLCOEF_MODE_E enVgsSclCoefMode, zpl_uint32 u32Width, zpl_uint32 u32Height)
{
    zpl_int32 s32Ret = HI_FAILURE;

    VGS_HANDLE hHandle = -1;
    VGS_TASK_ATTR_S stVgsTaskAttr;

    memcpy(&stVgsTaskAttr.stImgIn, inframe, sizeof(VIDEO_FRAME_INFO_S));
    if(outframe == NULL)
    {
        memcpy(&stVgsTaskAttr.stImgOut, &stVgsTaskAttr.stImgIn, sizeof(VIDEO_FRAME_INFO_S));
        stVgsTaskAttr.stImgOut.stVFrame.u32Width = u32Width;
        stVgsTaskAttr.stImgOut.stVFrame.u32Height = u32Height;
    }
    else
        memcpy(&stVgsTaskAttr.stImgOut, outframe, sizeof(VIDEO_FRAME_INFO_S));
    /************************************************
    step3:  Create VGS job
    *************************************************/
    s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_BeginJob failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }

    /************************************************
    step4:  Add VGS task
    *************************************************/
    s32Ret = HI_MPI_VGS_AddScaleTask(hHandle, &stVgsTaskAttr, enVgsSclCoefMode);
    if (s32Ret != HI_SUCCESS)
    {
        HI_MPI_VGS_CancelJob(hHandle);
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_AddScaleTask failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }
    /************************************************
    step5:  Start VGS work
    *************************************************/
    s32Ret = HI_MPI_VGS_EndJob(hHandle);
    if (s32Ret != HI_SUCCESS)
    {
        HI_MPI_VGS_CancelJob(hHandle);
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_EndJob failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }
    return s32Ret;
}

int zpl_vidhal_vgs_cover_job(VIDEO_FRAME_INFO_S *inframe, VGS_ADD_COVER_S pstVgsAddCover)
{
    zpl_int32 s32Ret = HI_FAILURE;

    VGS_HANDLE hHandle = -1;
    VGS_TASK_ATTR_S stVgsTaskAttr;

    memcpy(&stVgsTaskAttr.stImgIn, inframe, sizeof(VIDEO_FRAME_INFO_S));

    memcpy(&stVgsTaskAttr.stImgOut, &stVgsTaskAttr.stImgIn, sizeof(VIDEO_FRAME_INFO_S));
    /************************************************
    step3:  Create VGS job
    *************************************************/
    s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_BeginJob failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }

    /************************************************
    step4:  Add VGS task
    *************************************************/
    s32Ret = HI_MPI_VGS_AddCoverTask(hHandle, &stVgsTaskAttr, &pstVgsAddCover);
    if (s32Ret != HI_SUCCESS)
    {
        HI_MPI_VGS_CancelJob(hHandle);
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_AddScaleTask failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }
    /************************************************
    step5:  Start VGS work
    *************************************************/
    s32Ret = HI_MPI_VGS_EndJob(hHandle);
    if (s32Ret != HI_SUCCESS)
    {
        HI_MPI_VGS_CancelJob(hHandle);
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_EndJob failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }
    return s32Ret;
}

int zpl_vidhal_vgs_osd_job(VIDEO_FRAME_INFO_S *inframe, VIDEO_FRAME_INFO_S *outframe, 
    VGS_ADD_OSD_S vgsosd)
{
    zpl_int32 s32Ret = HI_FAILURE;

    VGS_HANDLE hHandle = -1;
    VGS_TASK_ATTR_S stVgsTaskAttr;

    memcpy(&stVgsTaskAttr.stImgIn, inframe, sizeof(VIDEO_FRAME_INFO_S));
    if(outframe == NULL)
        memcpy(&stVgsTaskAttr.stImgOut, &stVgsTaskAttr.stImgIn, sizeof(VIDEO_FRAME_INFO_S));
    else
        memcpy(&stVgsTaskAttr.stImgOut, outframe, sizeof(VIDEO_FRAME_INFO_S));
    /************************************************
    step3:  Create VGS job
    *************************************************/
    s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_BeginJob failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }

    /************************************************
    step4:  Add VGS task
    *************************************************/
    s32Ret = HI_MPI_VGS_AddOsdTask(hHandle, &stVgsTaskAttr, &vgsosd);
    if (s32Ret != HI_SUCCESS)
    {
        HI_MPI_VGS_CancelJob(hHandle);
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_AddScaleTask failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }
    /************************************************
    step5:  Start VGS work
    *************************************************/
    s32Ret = HI_MPI_VGS_EndJob(hHandle);
    if (s32Ret != HI_SUCCESS)
    {
        HI_MPI_VGS_CancelJob(hHandle);
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_EndJob failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }
    return s32Ret;
}

int zpl_vidhal_vgs_drawline_job(VIDEO_FRAME_INFO_S *inframe, VIDEO_FRAME_INFO_S *outframe, 
    VGS_DRAW_LINE_S pstVgsDrawLine)
{
    zpl_int32 s32Ret = HI_FAILURE;

    VGS_HANDLE hHandle = -1;
    VGS_TASK_ATTR_S stVgsTaskAttr;

    memcpy(&stVgsTaskAttr.stImgIn, inframe, sizeof(VIDEO_FRAME_INFO_S));
    if(outframe == NULL)
        memcpy(&stVgsTaskAttr.stImgOut, &stVgsTaskAttr.stImgIn, sizeof(VIDEO_FRAME_INFO_S));
    else
        memcpy(&stVgsTaskAttr.stImgOut, outframe, sizeof(VIDEO_FRAME_INFO_S));
    /************************************************
    step3:  Create VGS job
    *************************************************/
    s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_BeginJob failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }

    /************************************************
    step4:  Add VGS task
    *************************************************/
    s32Ret = HI_MPI_VGS_AddDrawLineTask(hHandle, &stVgsTaskAttr, &pstVgsDrawLine);
    if (s32Ret != HI_SUCCESS)
    {
        HI_MPI_VGS_CancelJob(hHandle);
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_AddScaleTask failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }
    /************************************************
    step5:  Start VGS work
    *************************************************/
    s32Ret = HI_MPI_VGS_EndJob(hHandle);
    if (s32Ret != HI_SUCCESS)
    {
        HI_MPI_VGS_CancelJob(hHandle);
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_EndJob failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }
    return s32Ret;
}

int zpl_vidhal_vgs_rotation_job(VIDEO_FRAME_INFO_S *inframe, VIDEO_FRAME_INFO_S *outframe, 
    ROTATION_E penRotationAngle)
{
    zpl_int32 s32Ret = HI_FAILURE;

    VGS_HANDLE hHandle = -1;
    VGS_TASK_ATTR_S stVgsTaskAttr;

    memcpy(&stVgsTaskAttr.stImgIn, inframe, sizeof(VIDEO_FRAME_INFO_S));
    if(outframe == NULL)
        memcpy(&stVgsTaskAttr.stImgOut, &stVgsTaskAttr.stImgIn, sizeof(VIDEO_FRAME_INFO_S));
    else
        memcpy(&stVgsTaskAttr.stImgOut, outframe, sizeof(VIDEO_FRAME_INFO_S));
    /************************************************
    step3:  Create VGS job
    *************************************************/
    s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_BeginJob failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }

    /************************************************
    step4:  Add VGS task
    *************************************************/
    s32Ret = HI_MPI_VGS_AddRotationTask(hHandle, &stVgsTaskAttr, penRotationAngle);
    if (s32Ret != HI_SUCCESS)
    {
        HI_MPI_VGS_CancelJob(hHandle);
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_AddScaleTask failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }
    /************************************************
    step5:  Start VGS work
    *************************************************/
    s32Ret = HI_MPI_VGS_EndJob(hHandle);
    if (s32Ret != HI_SUCCESS)
    {
        HI_MPI_VGS_CancelJob(hHandle);
        if(ZPL_MEDIA_DEBUG(VGS, EVENT) && ZPL_MEDIA_DEBUG(VGS, HARDWARE))
            zm_msg_err("HI_MPI_VGS_EndJob failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }
    return s32Ret;
}


#endif


