/*
 * zpl_vidhal_vpss.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_internal.h"


int zpl_vidhal_vpss_channel_frame_recvfrom(zpl_media_video_vpsschn_t *vpss)
{
#ifdef ZPL_HISIMPP_MODULE

    zpl_int32 s32MilliSec = 2000;
    VIDEO_FRAME_INFO_S stFrmInfo_vpss;
    stFrmInfo_vpss.u32PoolId = -1U;
    int s32Ret = HI_MPI_VPSS_GetChnFrame(vpss->vpss_group, vpss->vpss_channel, &stFrmInfo_vpss, s32MilliSec);
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
				zm_msg_debug(" VPSS Channel (%d/%d) Get Stream Total Size=%d", vpss->vpss_group, vpss->vpss_channel, datasize);
			}
		}
		vpss->dbg_recv_count++;
#endif
        //zm_msg_debug(" =====VPSS Channel (%d/%d) Get Stream Total Size=%d", vpss->vpssgrp->vpss_group, vpss->vpss_channel, datasize);
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
            HI_MPI_VPSS_ReleaseChnFrame(vpss->vpss_group, vpss->vpss_channel, &stFrmInfo_vpss);
        return OK;
    }
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
	if(vpss->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
	{
		vpss->dbg_recv_count = 0;
		if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE) && ZPL_MEDIA_DEBUG(VPSS, RECV))
		{
			zm_msg_warn(" VPSS Channel (%d/%d) ret %x", vpss->vpss_group, vpss->vpss_channel, s32Ret);
		}
	}
	vpss->dbg_recv_count++;
#endif	
#endif
    return ERROR;
}



int zpl_vidhal_vpss_channel_frame_sendto(zpl_media_video_vpsschn_t *vpsschn, void *p, zpl_int s32MilliSec)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = HI_MPI_VPSS_SendFrame(vpsschn->vpss_group, 0, p, s32MilliSec);
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_SEND_DETAIL
	if(vpsschn->dbg_send_count == ZPL_VIDEO_VIDHAL_DEBUG_SEND_DETAIL)
	{
		vpsschn->dbg_send_count = 0;
		if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE) && 
			ZPL_MEDIA_DEBUG(VPSSGRP, SEND) && ZPL_MEDIA_DEBUG(VPSSGRP, DETAIL))
			zm_msg_err(" Frame sendto VPSS Group (%d %d)", vpsschn->vpss_group, 0);
	}
	vpsschn->dbg_send_count++;
#endif
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
            zm_msg_err(" Frame sendto VPSS Group (%d %d) failed(%s)", vpsschn->vpss_group, 0, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return ERROR;
#endif
}

int zpl_vidhal_vpss_channel_update_fd(zpl_media_video_vpsschn_t *vpss)
{
#ifdef ZPL_HISIMPP_MODULE
    if (vpss->vpss_channel >= 0 && vpss->vpssfd != ZPL_SOCKET_INVALID)
    {
        ipstack_type(vpss->vpssfd) = IPSTACK_OS;
        ipstack_fd(vpss->vpssfd) = HI_MPI_VPSS_GetChnFd(vpss->vpss_group, vpss->vpss_channel);
		if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, DETAIL))
			zm_msg_debug(" video VPSS channel %d/%d fd %d\n", vpss->vpss_group, vpss->vpss_channel, ipstack_fd(vpss->vpssfd));
        return HI_SUCCESS;
    }
    return HI_FAILURE;
#else
    return OK;
#endif
}

int zpl_vidhal_vpss_channel_create(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_media_video_vpsschn_t *vpss)
{
#ifdef ZPL_HISIMPP_MODULE    
    int ret = 0;
    int flag = ZPL_MEDIA_HALRES_GET(vpssgrp, -1, VPSSGRP);    
    if(!ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_ACTIVE))
    {
        VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
        stVpssGrpAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
        stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        stVpssGrpAttr.u32MaxW = vpss->input_size.width;//1920;//vidsize.width;
        stVpssGrpAttr.u32MaxH = vpss->input_size.height;//1080;//vidsize.height;
        stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
        stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
        stVpssGrpAttr.bNrEn = HI_TRUE;
        stVpssGrpAttr.stNrAttr.enNrType = VPSS_NR_TYPE_VIDEO;
        stVpssGrpAttr.stNrAttr.enNrMotionMode = NR_MOTION_MODE_NORMAL;
        stVpssGrpAttr.stNrAttr.enCompressMode = COMPRESS_MODE_FRAME;

        if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zm_msg_debug(" VPSS Group %d input width:%d height:%d src framerate:%d dst framerate:%d", 
                    vpssgrp, vpss->input_size.width, vpss->input_size.height, 
                    stVpssGrpAttr.stFrameRate.s32SrcFrameRate, stVpssGrpAttr.stFrameRate.s32DstFrameRate);

        ret = HI_MPI_VPSS_CreateGrp(vpssgrp, &stVpssGrpAttr);
        if (ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zm_msg_err(" VPSS Group (%d) Create failed(%s)", vpssgrp, zpl_syshal_strerror(ret));
            return HI_FAILURE;
        }
        ZPL_SET_BIT(flag, ZPL_MEDIA_STATE_ACTIVE);
        ZPL_MEDIA_HALRES_SET(vpssgrp, -1, flag, VPSSGRP);
    }
    
    flag = ZPL_MEDIA_HALRES_GET(vpssgrp, vpsschn, VPSSCHN); 
    if(!ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_ACTIVE))
    {
        VPSS_CHN_ATTR_S stVpssChnAttr;
        stVpssChnAttr.u32Width = vpss->output_size.width;
        stVpssChnAttr.u32Height = vpss->output_size.height;
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

        if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zm_msg_debug(" VPSS Group %d Channel %d output width:%d height:%d src framerate:%d dst framerate:%d", 
                    vpssgrp, vpsschn, vpss->output_size.width, vpss->output_size.height, 
                    stVpssChnAttr.stFrameRate.s32SrcFrameRate, stVpssChnAttr.stFrameRate.s32DstFrameRate);

        ret = HI_MPI_VPSS_SetChnAttr(vpssgrp, vpsschn, &stVpssChnAttr);
        if (ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zm_msg_err(" VPSS channel (%d %d) Create failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(ret));
            return HI_FAILURE;
        }
        ZPL_SET_BIT(flag, ZPL_MEDIA_STATE_ACTIVE);
        ZPL_MEDIA_HALRES_SET(vpssgrp, vpsschn, flag, VPSSCHN);
        return ret;
    }
    return ret;
#else
    return OK;
#endif
}

int zpl_vidhal_vpss_channel_destroy(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_media_video_vpsschn_t *vpss)
{
#ifdef ZPL_HISIMPP_MODULE    
    int ret = 0;
    int flag = ZPL_MEDIA_HALRES_GET(vpssgrp, vpsschn, VPSSCHN);  
    if(ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_ACTIVE))
    {
        ret = HI_MPI_VPSS_DisableChn(vpssgrp, vpsschn);
        if (ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zm_msg_err(" VPSS channel (%d %d) Disable failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(ret));
            return HI_FAILURE;
        }
        ZPL_CLR_BIT(flag, ZPL_MEDIA_STATE_ACTIVE);
        ZPL_MEDIA_HALRES_SET(vpssgrp, vpsschn, flag, VPSSCHN);
    }
    flag = ZPL_MEDIA_HALRES_GET(vpssgrp, -1, VPSSGRP); 
    if(vpss->grp_bind_count==1 && ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_ACTIVE))
    {
        ret = HI_MPI_VPSS_DestroyGrp(vpssgrp);
        if (ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zm_msg_err(" VPSS Group (%d) Destroy failed(%s)", vpssgrp, zpl_syshal_strerror(ret));
            return HI_FAILURE;
        }
        ZPL_CLR_BIT(flag, ZPL_MEDIA_STATE_ACTIVE);
        ZPL_MEDIA_HALRES_SET(vpssgrp, 0, flag, VPSSGRP);
    }
    return ret;
#else
    return OK;
#endif
}

int zpl_vidhal_vpss_channel_start(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_media_video_vpsschn_t *vpss)
{
#ifdef ZPL_HISIMPP_MODULE  
    int ret = 0;
    int flag = ZPL_MEDIA_HALRES_GET(vpssgrp, vpsschn, VPSSGRP);  
    if(vpss->grp_bind_count==1 && !ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_START))
    {
        ret = HI_MPI_VPSS_StartGrp(vpssgrp);
        if (ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zm_msg_err(" VPSS Group (%d) Start failed(%s)", vpssgrp, zpl_syshal_strerror(ret));
            return HI_FAILURE;
        }
        ZPL_SET_BIT(flag, ZPL_MEDIA_STATE_START);
        ZPL_MEDIA_HALRES_SET(vpssgrp, 0, flag, VPSSGRP);
    }  
    flag = ZPL_MEDIA_HALRES_GET(vpssgrp, vpsschn, VPSSCHN);  
    if (vpsschn != ZPL_INVALID_VAL && !ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_START))
    {
        ret = HI_MPI_VPSS_EnableChn(vpssgrp, vpsschn);
        if (ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zm_msg_err(" VPSS channel (%d %d) Disable failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(ret));
            return HI_FAILURE;
        }
        ZPL_SET_BIT(flag, ZPL_MEDIA_STATE_START);
        ZPL_MEDIA_HALRES_SET(vpssgrp, vpsschn, flag, VPSSCHN);
        if (vpsschn >= 0)
            ipstack_fd(vpss->vpssfd) = HI_MPI_VPSS_GetChnFd(vpssgrp, vpsschn);
    }
    return ret;
#else
    return OK;
#endif
}

int zpl_vidhal_vpss_channel_stop(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_media_video_vpsschn_t *vpss)
{
#ifdef ZPL_HISIMPP_MODULE    
    int ret = 0;
    int flag = ZPL_MEDIA_HALRES_GET(vpssgrp, vpsschn, VPSSCHN); 
    if (ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_START))
    {
        HI_MPI_VPSS_CloseFd();
        ret = HI_MPI_VPSS_DisableChn(vpssgrp, vpsschn);
        if (ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zm_msg_err(" VPSS channel (%d %d) Disable failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(ret));
            return HI_FAILURE;
        }
        ZPL_CLR_BIT(flag, ZPL_MEDIA_STATE_START);
        ZPL_MEDIA_HALRES_SET(vpssgrp, vpsschn, flag, VPSSGRP);
    }
    flag = ZPL_MEDIA_HALRES_GET(vpssgrp, vpsschn, VPSSGRP); 
    if(vpss->grp_bind_count==1 && ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_START))
    {
        ret = HI_MPI_VPSS_StopGrp(vpssgrp);
        if (ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zm_msg_err(" VPSS Group (%d) Stop failed(%s)", vpssgrp, zpl_syshal_strerror(ret));
            return HI_FAILURE;
        }
        ZPL_CLR_BIT(flag, ZPL_MEDIA_STATE_START);
        ZPL_MEDIA_HALRES_SET(vpssgrp, 0, flag, VPSSGRP);
    }
    return ret;
#else
    return OK;
#endif
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
                zm_msg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpAttr failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.stFrameRate.s32SrcFrameRate = srcframerate;        /* RW; source frame rate */
        info.stFrameRate.s32DstFrameRate = dstframerate;        /* RW; dest frame rate */
        s32Ret = HI_MPI_VPSS_SetGrpAttr(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zm_msg_err(" VPSS Group (%d) Set GrpAttr failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get ChnAttr failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.stFrameRate.s32SrcFrameRate = srcframerate;        /* RW; source frame rate */
        info.stFrameRate.s32DstFrameRate = dstframerate;        /* RW; dest frame rate */
        s32Ret =  HI_MPI_VPSS_SetChnAttr(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zm_msg_err(" VPSS  Channel (%d %d) Set ChnAttr failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpSharpen failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        info.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;

        s32Ret = HI_MPI_VPSS_SetGrpSharpen(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zm_msg_err(" VPSS Group (%d) Set GrpSharpen failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zm_msg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_VPSS_SetGrpCrop(vpssgrp, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSSGRP, EVENT) && ZPL_MEDIA_DEBUG(VPSSGRP, HARDWARE))
                zm_msg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_VPSS_SetChnCrop(vpssgrp, vpsschn, &info);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(VPSS, EVENT) && ZPL_MEDIA_DEBUG(VPSS, HARDWARE))
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Get GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Group (%d) Set GrpCrop failed(%s)", vpssgrp, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS Channel (%d %d) Get Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
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
                zm_msg_err(" VPSS  Channel (%d %d) Set Crop failed(%s)", vpssgrp, vpsschn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return s32Ret;
#else
    return ERROR;
#endif
}