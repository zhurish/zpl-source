/*
 * zpl_vidhal_vpss.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_vpss.h"
#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"
#endif

static int _zpl_vidhal_vpssgrp_create(zpl_int32 vpssgrp, zpl_video_size_t vidsize)
{
#ifdef ZPL_HISIMPP_MODULE
    VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
    stVpssGrpAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVpssGrpAttr.u32MaxW = 1920;//vidsize.width;
    stVpssGrpAttr.u32MaxH = 1080;//vidsize.height;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.stNrAttr.enNrType = VPSS_NR_TYPE_VIDEO;
    stVpssGrpAttr.stNrAttr.enNrMotionMode = NR_MOTION_MODE_NORMAL;
    stVpssGrpAttr.stNrAttr.enCompressMode = COMPRESS_MODE_FRAME;
    int s32Ret = HI_MPI_VPSS_CreateGrp(vpssgrp, &stVpssGrpAttr);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
            zpl_media_debugmsg_err(" VPSS Group (%d) Create failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return OK;
#endif
}

static int _zpl_vidhal_vpssgrp_start(zpl_int32 vpssgrp)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = HI_MPI_VPSS_StartGrp(vpssgrp);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
            zpl_media_debugmsg_err(" VPSS Group (%d) Start failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return OK;
#endif
}

static int _zpl_vidhal_vpssgrp_stop(zpl_int32 vpssgrp)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = HI_MPI_VPSS_StopGrp(vpssgrp);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
            zpl_media_debugmsg_err(" VPSS Group (%d) Stop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return OK;
#endif
}
static int _zpl_vidhal_vpssgrp_destroy(zpl_int32 vpssgrp)
{

#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = HI_MPI_VPSS_DestroyGrp(vpssgrp);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
            zpl_media_debugmsg_err(" VPSS Group (%d) Destroy failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return OK;
#endif
}

static int zpl_vidhal_vpsschn_create(zpl_int32 vpssgrp, zpl_media_video_vpss_channel_t *vpss)
{
#ifdef ZPL_HISIMPP_MODULE
    VPSS_CHN_ATTR_S stVpssChnAttr;
    stVpssChnAttr.u32Width = vpss->input_size.width;
    stVpssChnAttr.u32Height = vpss->input_size.height;
    stVpssChnAttr.enChnMode = VPSS_CHN_MODE_USER;
    stVpssChnAttr.enCompressMode = COMPRESS_MODE_NONE; //COMPRESS_MODE_SEG;
    stVpssChnAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stVpssChnAttr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
    stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
    stVpssChnAttr.u32Depth = 4;
    stVpssChnAttr.bMirror = HI_FALSE;
    stVpssChnAttr.bFlip = HI_FALSE;
    stVpssChnAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
    stVpssChnAttr.stAspectRatio.enMode = ASPECT_RATIO_NONE;
    int s32Ret = HI_MPI_VPSS_SetChnAttr(vpssgrp, vpss->vpss_channel, &stVpssChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
            zpl_media_debugmsg_err(" VPSS channel (%d %d) Create failed(%s)", vpssgrp, vpss->vpss_channel, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return OK;
#endif
}

static int zpl_vidhal_vpsschn_start(zpl_int32 vpssgrp, zpl_int32 vpsschn)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = HI_MPI_VPSS_EnableChn(vpssgrp, vpsschn);
    if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, DETAIL))
    {
        zpl_media_debugmsg_debug(" VPSS channel (%d %d) stop", vpssgrp, vpsschn);
    }
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
            zpl_media_debugmsg_err(" VPSS channel (%d %d) Enable failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return OK;
#endif
}
static int zpl_vidhal_vpsschn_stop(zpl_int32 vpssgrp, zpl_int32 vpsschn)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = HI_MPI_VPSS_DisableChn(vpssgrp, vpsschn);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
            zpl_media_debugmsg_err(" VPSS channel (%d %d) Disable failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return OK;
#endif
}

#ifdef ZPL_HISIMPP_MODULE
static int zpl_vidhal_vpssgrp_read_frame(zpl_int32 vpssgrp, void *p)
{
    int s32Ret = HI_MPI_VPSS_GetGrpFrame(vpssgrp, 0, p);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
            zpl_media_debugmsg_err(" VPSS Group Get GrpFrame (%d) failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
}

static int zpl_vidhal_vpssgrp_release_frame(zpl_int32 vpssgrp, void *p)
{
    int s32Ret = HI_MPI_VPSS_ReleaseGrpFrame(vpssgrp, 0, p);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
            zpl_media_debugmsg_err(" VPSS Group Release GrpFrame (%d) failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
}

static int zpl_vidhal_vpsschn_read_frame(zpl_int32 vpssgrp, zpl_int32 vpsschn, void *p, zpl_int s32MilliSec)
{
    int s32Ret = HI_MPI_VPSS_GetChnFrame(vpssgrp, vpsschn, p, s32MilliSec);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
            zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Frame failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
}

static int zpl_vidhal_vpsschn_release_frame(zpl_int32 vpssgrp, zpl_int32 vpsschn, void *p)
{
    int s32Ret = HI_MPI_VPSS_ReleaseChnFrame(vpssgrp, vpsschn, p);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
            zpl_media_debugmsg_err(" VPSS Channel (%d %d) Release Frame  failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
}
#endif

int zpl_vidhal_vpssgrp_frame_recvfrom(zpl_media_video_vpssgrp_t *vpssgrp)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32MilliSec = 200;
    VIDEO_FRAME_INFO_S stFrmInfo_vpss;
    stFrmInfo_vpss.u32PoolId = -1U;
    int s32Ret = zpl_vidhal_vpssgrp_read_frame(vpssgrp->vpss_group, &stFrmInfo_vpss);
    if (s32Ret == HI_SUCCESS)
    {
        zpl_video_size_t input_size;
        //zpl_media_video_encode_t *venc_ptr = vpss->venc_ptr;
        /* 1.1 mmap frame */
        input_size.width = stFrmInfo_vpss.stVFrame.u32Width;
        input_size.height = stFrmInfo_vpss.stVFrame.u32Height;
        zpl_int32 datasize = input_size.width * input_size.height * 3 / 2;
		
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
		if(vpssgrp->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
		{
			vpssgrp->dbg_recv_count = 0;
			if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE) && ZPL_MEDIA_DEBUG(VPSSGRP, RECV))
			{
				zpl_media_debugmsg_debug(" VPSS Group (%d) Get Stream Total Size=%d", vpssgrp->vpss_group, datasize);
			}
		}
		vpssgrp->dbg_recv_count++;
#endif

        //zpl_media_debugmsg_debug(" ==========================VPSS Group (%d) Get Stream Total Size=%d", vpssgrp->vpss_group, datasize);
        #if 0
        zpl_uint8 *pbuf = zpl_sys_iommap(stFrmInfo_vpss.stVFrame.u64PhyAddr[0], datasize);
        if (pbuf && vpssgrp->vpss_frame_handle)
            (vpssgrp->vpss_frame_handle)(pbuf, datasize, input_size);
        /* 4. ummap frame */
        zpl_sys_munmap(pbuf, datasize);
        pbuf = NULL;
        #endif
        zpl_media_hardadap_handle(&vpssgrp->callback, &stFrmInfo_vpss, s32MilliSec);
        /*
        if(vpss->vpss_sendto)
            (vpss->vpss_sendto)(vpss->toid, vpss->tochn, &stFrmInfo_vpss,  s32MilliSec);
        */
        if (stFrmInfo_vpss.u32PoolId != -1U)
            zpl_vidhal_vpssgrp_release_frame(vpssgrp->vpss_group, &stFrmInfo_vpss);
        return OK;
    }

#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
	if(vpssgrp->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
	{
		vpssgrp->dbg_recv_count = 0;
		if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE) && ZPL_MEDIA_DEBUG(VPSSGRP, RECV))
		{
			zpl_media_debugmsg_warn(" VPSS Group (%d) ret %x", vpssgrp->vpss_group, s32Ret);
		}
	}
	vpssgrp->dbg_recv_count++;
#endif
#endif
    return ERROR;
}

int zpl_vidhal_vpss_channel_frame_recvfrom(zpl_media_video_vpss_channel_t *vpss)
{
#ifdef ZPL_HISIMPP_MODULE

    zpl_int32 s32MilliSec = 2000;
    VIDEO_FRAME_INFO_S stFrmInfo_vpss;
    stFrmInfo_vpss.u32PoolId = -1U;
    int s32Ret = zpl_vidhal_vpsschn_read_frame(vpss->vpssgrp->vpss_group, vpss->vpss_channel, &stFrmInfo_vpss, s32MilliSec);
    if (s32Ret == HI_SUCCESS)
    {
        zpl_video_size_t input_size;
        //zpl_media_video_encode_t *venc_ptr = vpss->venc_ptr;
        /* 1.1 mmap frame */
        input_size.width = stFrmInfo_vpss.stVFrame.u32Width;
        input_size.height = stFrmInfo_vpss.stVFrame.u32Height;
        zpl_int32 datasize = input_size.width * input_size.height * 3 / 2;
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
		if(vpss->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
		{
			vpss->dbg_recv_count = 0;
			if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE) && ZPL_MEDIA_DEBUG(VPSS, RECV))
			{
				zpl_media_debugmsg_debug(" VPSS Channel (%d/%d) Get Stream Total Size=%d", vpss->vpssgrp->vpss_group, vpss->vpss_channel, datasize);
			}
		}
		vpss->dbg_recv_count++;
#endif
        //zpl_media_debugmsg_debug(" =====VPSS Channel (%d/%d) Get Stream Total Size=%d", vpss->vpssgrp->vpss_group, vpss->vpss_channel, datasize);
#if 0
        zpl_uint8 *pbuf = zpl_sys_iommap(stFrmInfo_vpss.stVFrame.u64PhyAddr[0], datasize);
        if (pbuf && vpss->vpss_frame_handle)
            (vpss->vpss_frame_handle)(pbuf, datasize, input_size);
        /* 4. ummap frame */
        zpl_sys_munmap(pbuf, datasize);
        pbuf = NULL;
#endif        
        zpl_media_hardadap_handle(&vpss->callback, &stFrmInfo_vpss, s32MilliSec);
        /*if(vpss->vpss_sendto)
            (vpss->vpss_sendto)(vpss->toid, vpss->tochn, &stFrmInfo_vpss,  s32MilliSec);
        */
        if (stFrmInfo_vpss.u32PoolId != -1U)
            zpl_vidhal_vpsschn_release_frame(vpss->vpssgrp->vpss_group, vpss->vpss_channel, &stFrmInfo_vpss);
        return OK;
    }
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
	if(vpss->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
	{
		vpss->dbg_recv_count = 0;
		if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE) && ZPL_MEDIA_DEBUG(VPSS, RECV))
		{
			zpl_media_debugmsg_warn(" VPSS Channel (%d/%d) ret %x", vpss->vpssgrp->vpss_group, vpss->vpss_channel, s32Ret);
		}
	}
	vpss->dbg_recv_count++;
#endif	
#endif
    return ERROR;
}



int zpl_vidhal_vpssgrp_frame_sendto(zpl_media_video_vpssgrp_t *vpssgrp, void *p, zpl_int s32MilliSec)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = HI_MPI_VPSS_SendFrame(vpssgrp->vpss_group, 0, p, s32MilliSec);
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_SEND_DETAIL
	if(vpssgrp->dbg_send_count == ZPL_VIDEO_VIDHAL_DEBUG_SEND_DETAIL)
	{
		vpssgrp->dbg_send_count = 0;
		if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE) && 
			ZPL_MEDIA_DEBUG(VPSSGRP, SEND) && ZPL_MEDIA_DEBUG(VPSSGRP, DETAIL))
			zpl_media_debugmsg_err(" Frame sendto VPSS Group (%d %d)", vpssgrp->vpss_group, 0);
	}
	vpssgrp->dbg_send_count++;
#endif
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
            zpl_media_debugmsg_err(" Frame sendto VPSS Group (%d %d) failed(%s)", vpssgrp->vpss_group, 0, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_channel_update_fd(zpl_media_video_vpss_channel_t *vpss)
{
#ifdef ZPL_HISIMPP_MODULE
    if (vpss->vpss_channel >= 0)
    {
        ipstack_type(vpss->vpssfd) = IPSTACK_OS;
        ipstack_fd(vpss->vpssfd) = HI_MPI_VPSS_GetChnFd(vpss->vpssgrp->vpss_group, vpss->vpss_channel);
		if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, DETAIL))
			zpl_media_debugmsg_debug(" video VPSS channel %d/%d fd %d\n", vpss->vpssgrp->vpss_group, vpss->vpss_channel, vpss->vpssfd);
        return HI_SUCCESS;
    }
    return HI_FAILURE;
#else
    return OK;
#endif
}

int zpl_vidhal_vpssgrp_create(zpl_media_video_vpssgrp_t *vpss)
{
    if (vpss->vpss_group == ZPL_INVALID_VAL)
    {
        return ERROR;
    }
    if (_zpl_vidhal_vpssgrp_create(vpss->vpss_group, vpss->input_size) != OK)
        return ERROR;
    return OK;
}

int zpl_vidhal_vpss_channel_create(zpl_media_video_vpss_channel_t *vpss)
{
    if (vpss->vpss_channel != ZPL_INVALID_VAL)
    {
        if (zpl_vidhal_vpsschn_create(vpss->vpssgrp->vpss_group, vpss) != OK)
            return ERROR;
    }
    return OK;
}
int zpl_vidhal_vpssgrp_destroy(zpl_media_video_vpssgrp_t *vpss)
{
    if (vpss->vpss_group == ZPL_INVALID_VAL)
    {
        return ERROR;
    }
    if (_zpl_vidhal_vpssgrp_destroy(vpss->vpss_group) != OK)
        return ERROR;
    return OK;
}

int zpl_vidhal_vpss_channel_destroy(zpl_media_video_vpss_channel_t *vpss)
{
    if (vpss->vpss_channel != ZPL_INVALID_VAL)
    {
        if (zpl_vidhal_vpsschn_stop(vpss->vpssgrp->vpss_group, vpss->vpss_channel) != OK)
            return ERROR;
    }
    return OK;
}
int zpl_vidhal_vpssgrp_start(zpl_media_video_vpssgrp_t *vpss)
{
    if (vpss->vpss_group == ZPL_INVALID_VAL)
    {
        return ERROR;
    }
    if (_zpl_vidhal_vpssgrp_start(vpss->vpss_group) != OK)
        return ERROR;
    return OK;
}

int zpl_vidhal_vpss_channel_start(zpl_media_video_vpss_channel_t *vpss)
{
    if (vpss->vpss_channel != ZPL_INVALID_VAL)
    {
        if (zpl_vidhal_vpsschn_start(vpss->vpssgrp->vpss_group, vpss->vpss_channel) != OK)
            return ERROR;
    }
#ifdef ZPL_HISIMPP_MODULE
    if (vpss->vpss_channel >= 0)
        ipstack_fd(vpss->vpssfd) = HI_MPI_VPSS_GetChnFd(vpss->vpssgrp->vpss_group, vpss->vpss_channel);
#endif
    return OK;
}
int zpl_vidhal_vpssgrp_stop(zpl_media_video_vpssgrp_t *vpss)
{
    if (vpss->vpss_group == ZPL_INVALID_VAL)
    {
        return ERROR;
    }
    if (_zpl_vidhal_vpssgrp_stop(vpss->vpss_group) != OK)
        return ERROR;
    return OK;
}
int zpl_vidhal_vpss_channel_stop(zpl_media_video_vpss_channel_t *vpss)
{
    if (vpss->vpss_channel != ZPL_INVALID_VAL)
    {
#ifdef ZPL_HISIMPP_MODULE
        HI_MPI_VPSS_CloseFd();
#endif
        if (zpl_vidhal_vpsschn_stop(vpss->vpssgrp->vpss_group, vpss->vpss_channel) != OK)
            return ERROR;
    }
    return OK;
}

int zpl_vidhal_vpss_crop(zpl_int32 vpssgrp, zpl_int32  vpsschn, zpl_video_size_t cropsize) //裁剪
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if (vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}


int zpl_vidhal_vpss_frc(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_uint32 srcframerate, zpl_uint32 dstframerate)     //帧率控制
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_GRP_ATTR_S info;
        s32Ret = HI_MPI_VPSS_GetGrpAttr(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpAttr failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.stFrameRate.s32SrcFrameRate = srcframerate;        /* RW; source frame rate */
        info.stFrameRate.s32DstFrameRate = dstframerate;        /* RW; dest frame rate */
        s32Ret = HI_MPI_VPSS_SetGrpAttr(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpAttr failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CHN_ATTR_S  info;
        s32Ret =  HI_MPI_VPSS_GetChnAttr(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get ChnAttr failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.stFrameRate.s32SrcFrameRate = srcframerate;        /* RW; source frame rate */
        info.stFrameRate.s32DstFrameRate = dstframerate;        /* RW; dest frame rate */
        s32Ret =  HI_MPI_VPSS_SetChnAttr(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set ChnAttr failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_sharpen(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize) //锐化
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_GRP_SHARPEN_ATTR_S info;
        s32Ret = HI_MPI_VPSS_SetGrpSharpen(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpSharpen failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;

        s32Ret = HI_MPI_VPSS_SetGrpSharpen(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpSharpen failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_3DNR(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_scale(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize)        //缩放
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_ldc(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize)          //畸形矫正
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_spread(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize)       //伸开
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_cover(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize)        //封面/封面
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_overlayex(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize)    //覆盖层
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_mosaic(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize)       //马赛克
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_mirror_flip(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_bool mirror, zpl_bool flip)  //镜像/翻转
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_aspect_ratio(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize) //纵横比
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_rotation(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_uint32 rotation)     //固定角度旋转、任意角度旋转
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_fish_eye(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize)     //鱼眼校正
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_compression(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_video_size_t cropsize)  //压缩解压
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    if ( vpsschn == ZPL_INVALID_VAL)
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        VPSS_CROP_INFO_S info;
        s32Ret = HI_MPI_VPSS_GetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
        info.enCropCoordinate = VPSS_CROP_ABS_COOR;
        info.stCropRect.s32X = (info.stCropRect.u32Width - cropsize.width) / 2;
        info.stCropRect.s32Y = (info.stCropRect.u32Height - cropsize.height) / 2;
        info.stCropRect.u32Width = cropsize.width;
        info.stCropRect.u32Height = cropsize.height;
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zpl_media_debugmsg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}