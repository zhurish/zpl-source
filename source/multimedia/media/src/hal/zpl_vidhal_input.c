/*
 * zpl_vidhal_input.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_internal.h"

#ifdef ZPL_HISIMPP_MODULE
static VI_DEV_BIND_PIPE_S stGlobalDevBindPipe = {0};
#endif

int zpl_vidhal_inputchn_crop(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_bool out, zpl_video_size_t cropsize)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    if (input_chn == ZPL_INVALID_VAL)
    {
        if (out)
        {
            CROP_INFO_S stCropInfo;
            s32Ret = HI_MPI_VI_GetPipePreCrop(input_pipe, &stCropInfo);
            if (s32Ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                    zm_msg_err(" INPUT Pipe (%d) GetPreCrop failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
            stCropInfo.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
            stCropInfo.stRect.s32X = (stCropInfo.stRect.u32Width - cropsize.width) / 2;
            stCropInfo.stRect.s32Y = (stCropInfo.stRect.u32Height - cropsize.height) / 2;
            stCropInfo.stRect.u32Width = cropsize.width;
            stCropInfo.stRect.u32Height = cropsize.height;
            s32Ret = HI_MPI_VI_SetPipePreCrop(input_pipe, &stCropInfo);
            if (s32Ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                    zm_msg_err(" INPUT Pipe (%d) SetPreCrop failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
            return HI_SUCCESS;
        }
        else
        {
            CROP_INFO_S stCropInfo;
            s32Ret = HI_MPI_VI_GetPipePostCrop(input_pipe, &stCropInfo);
            if (s32Ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                    zm_msg_err(" INPUT Pipe (%d) GetPostCrop failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
            stCropInfo.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
            stCropInfo.stRect.s32X = (stCropInfo.stRect.u32Width - cropsize.width) / 2;
            stCropInfo.stRect.s32Y = (stCropInfo.stRect.u32Height - cropsize.height) / 2;
            stCropInfo.stRect.u32Width = cropsize.width;
            stCropInfo.stRect.u32Height = cropsize.height;
            s32Ret = HI_MPI_VI_SetPipePostCrop(input_pipe, &stCropInfo);
            if (s32Ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                    zm_msg_err(" INPUT Pipe (%d) SetPostCrop failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
            return HI_SUCCESS;
        }
    }
    else
    {
        if (out)
        {
            VI_CROP_INFO_S stCropInfo;
            s32Ret = HI_MPI_VI_GetChnCrop(input_pipe, input_chn, &stCropInfo);
            if (s32Ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zm_msg_err(" INPUT Channel (%d %d) GetCrop failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
            stCropInfo.bEnable = cropsize.width ? HI_TRUE : HI_FALSE;
            stCropInfo.enCropCoordinate = VI_CROP_ABS_COOR;
            stCropInfo.stCropRect.s32X = (stCropInfo.stCropRect.u32Width - cropsize.width) / 2;
            stCropInfo.stCropRect.s32Y = (stCropInfo.stCropRect.u32Height - cropsize.height) / 2;
            stCropInfo.stCropRect.u32Width = cropsize.width;
            stCropInfo.stCropRect.u32Height = cropsize.height;
            s32Ret = HI_MPI_VI_SetChnCrop(input_pipe, input_chn, &stCropInfo);
            if (s32Ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zm_msg_err(" INPUT Channel (%d %d) SetCrop failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
            return HI_SUCCESS;
        }
    }
    return HI_FAILURE;
#else
    return ERROR;
#endif
}

int zpl_vidhal_inputchn_fish_eye(zpl_int32 input_pipe, zpl_int32 input_chn, void *LMF)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;

    if (input_chn == ZPL_INVALID_VAL)
    {
        FISHEYE_CONFIG_S stInfo;
        s32Ret = HI_MPI_VI_GetPipeFisheyeConfig(input_pipe, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                zm_msg_err(" INPUT Pipe (%d) GetFisheye failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        memcpy(stInfo.au16LMFCoef, LMF, sizeof(stInfo.au16LMFCoef));
        s32Ret = HI_MPI_VI_SetPipeFisheyeConfig(input_pipe, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zm_msg_err(" INPUT Pipe (%d) SetFisheye failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        return HI_SUCCESS;
    }
    else
    {
        FISHEYE_ATTR_S stInfo;
        s32Ret = HI_MPI_VI_GetExtChnFisheye(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zm_msg_err(" INPUT Channel (%d %d) GetCrop failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_VI_SetExtChnFisheye(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zm_msg_err(" INPUT Channel (%d %d) GetCrop failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        return HI_SUCCESS;
    }
    return HI_FAILURE;
#else
    return ERROR;
#endif
}
int zpl_vidhal_inputchn_rotation(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_uint32 rotation)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    if (input_chn != ZPL_INVALID_VAL)
    {

        s32Ret = HI_MPI_VI_SetChnRotation(input_pipe, input_chn, rotation);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zm_msg_err(" INPUT Channel (%d %d) Rotation failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        return HI_SUCCESS;
    }
    return HI_FAILURE;
#else
    return ERROR;
#endif
}

int zpl_vidhal_inputchn_rotation_angle(zpl_int32 input_pipe, zpl_int32 input_chn, void *p)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    if (input_chn != ZPL_INVALID_VAL)
    {
        {
            VI_ROTATION_EX_ATTR_S stViRotationExAttr;
            s32Ret = HI_MPI_VI_GetChnRotationEx(input_pipe, input_chn, &stViRotationExAttr);
            if (s32Ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zm_msg_err(" INPUT Channel (%d %d) get RotationEx failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
            s32Ret = HI_MPI_VI_SetChnRotationEx(input_pipe, input_chn, &stViRotationExAttr);
            if (s32Ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zm_msg_err(" INPUT Channel (%d %d) set RotationEx failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
        }
    }
    return HI_FAILURE;
#else
    return ERROR;
#endif
}

int zpl_vidhal_inputchn_ldc(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_video_size_t cropsize) // 畸形矫正
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    if (input_chn != ZPL_INVALID_VAL)
    {
        VI_LDC_ATTR_S stInfo;

        s32Ret = HI_MPI_VI_GetChnLDCAttr(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zm_msg_err(" INPUT Channel (%d %d) get LDCAttr failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_VI_SetChnLDCAttr(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zm_msg_err(" INPUT Channel (%d %d) set LDCAttr failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return HI_FAILURE;
#else
    return ERROR;
#endif
}

int zpl_vidhal_inputchn_spread(zpl_int32 input_pipe, zpl_int32 input_chn, void *p)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    if (input_chn != ZPL_INVALID_VAL)
    {
        SPREAD_ATTR_S stInfo;
        s32Ret = HI_MPI_VI_GetChnSpreadAttr(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zm_msg_err(" INPUT Channel (%d %d) get Spread failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_VI_SetChnSpreadAttr(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zm_msg_err(" INPUT Channel (%d %d) set Spread failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return HI_FAILURE;
#else
    return ERROR;
#endif
}

int zpl_vidhal_inputchn_mirror_flip(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_bool mirror, zpl_bool flip) //
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    if (input_chn != ZPL_INVALID_VAL)
    {
        VI_CHN_ATTR_S stInfo;
        s32Ret = HI_MPI_VI_GetChnAttr(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zm_msg_err(" INPUT Channel (%d %d) get chnattr failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        stInfo.bMirror = mirror;
        stInfo.bFlip = flip;
        s32Ret = HI_MPI_VI_SetChnAttr(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zm_msg_err(" INPUT Channel (%d %d) set chnattr failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return HI_FAILURE;
#else
    return ERROR;
#endif
}



int zpl_vidhal_inputchn_pipe_frame_recvfrom(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *input)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32MilliSec = 200;
    VIDEO_FRAME_INFO_S stFrmInfo_input;
    stFrmInfo_input.u32PoolId = -1U;
    stFrmInfo_input.stVFrame.u64PhyAddr[0] = 0U;
    if (HI_MPI_VI_GetPipeFrame(input->input_pipe, &stFrmInfo_input, -1) == HI_SUCCESS)
    {
        zpl_video_size_t input_size;
        // zpl_media_video_encode_t *venc_ptr = input->venc_ptr;
        /* 1.1 mmap frame */
        input_size.width = stFrmInfo_input.stVFrame.u32Width;
        input_size.height = stFrmInfo_input.stVFrame.u32Height;
        zpl_int32 datasize = input_size.width * input_size.height * 3 / 2;
#if 0
        zpl_uint8 *pbuf = zpl_sys_iommap(stFrmInfo_input.stVFrame.u64PhyAddr[0], datasize);
        if(pbuf && input->input_frame_handle)
            (input->input_frame_handle)(pbuf,  datasize, input_size);
        /* 4. ummap frame */
        zpl_sys_munmap(pbuf, datasize);
        pbuf = NULL;
#endif
        zpl_media_hardadap_handle(&input->callback, &stFrmInfo_input, s32MilliSec);
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
        if (input->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
        {
            input->dbg_recv_count = 0;
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE) && ZPL_MEDIA_DEBUG(INPUT, RECV))
            {
                zm_msg_warn(" INPUT Pipe (%d) read frame datasize %d", input->input_pipe, datasize);
            }
        }
        input->dbg_recv_count++;
#endif
        if (stFrmInfo_input.u32PoolId != -1U)
            HI_MPI_VI_ReleasePipeFrame(input->input_pipe, &stFrmInfo_input);
        return OK;
    }
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
    if (input->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
    {
        input->dbg_recv_count = 0;
        if (ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE) && ZPL_MEDIA_DEBUG(INPUTPIPE, RECV))
        {
            zm_msg_warn(" INPUT Pipe (%d) read frame failed", input->input_pipe);
        }
    }
    input->dbg_recv_count++;
#endif

#endif
    return ERROR;
}

int zpl_vidhal_inputchn_frame_recvfrom(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *input)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32MilliSec = 200;
    VIDEO_FRAME_INFO_S stFrmInfo_input;
    stFrmInfo_input.u32PoolId = -1U;
    stFrmInfo_input.stVFrame.u64PhyAddr[0] = 0U;
    if (HI_MPI_VI_GetChnFrame(input->input_pipe, input->input_chn, &stFrmInfo_input, s32MilliSec) == HI_SUCCESS)
    {

        zpl_video_size_t input_size;
        // zpl_media_video_encode_t *venc_ptr = input->venc_ptr;
        /* 1.1 mmap frame */
        input_size.width = stFrmInfo_input.stVFrame.u32Width;
        input_size.height = stFrmInfo_input.stVFrame.u32Height;
        zpl_int32 datasize = input_size.width * input_size.height * 3 / 2;
#if 0
        zpl_uint8 *pbuf = zpl_sys_iommap(stFrmInfo_input.stVFrame.u64PhyAddr[0], datasize);
        if(pbuf && input->input_frame_handle)
            (input->input_frame_handle)(pbuf,  datasize, input_size);
        /* 4. ummap frame */
        zpl_sys_munmap(pbuf, datasize);
        pbuf = NULL;
#endif
        zpl_media_hardadap_handle(&input->callback, &stFrmInfo_input, s32MilliSec);
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
        if (input->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
        {
            input->dbg_recv_count = 0;
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE) && ZPL_MEDIA_DEBUG(INPUT, RECV))
            {
                zm_msg_warn(" INPUT Channel (%d/%d) read frame datasize %d", input->input_pipe, input->input_chn, datasize);
            }
        }
        input->dbg_recv_count++;
#endif

        if (stFrmInfo_input.u32PoolId != -1U)
            HI_MPI_VI_ReleaseChnFrame(input->input_pipe, input->input_chn, &stFrmInfo_input);
        return OK;
    }
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
    if (input->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
    {
        input->dbg_recv_count = 0;
        if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE) && ZPL_MEDIA_DEBUG(INPUT, RECV))
        {
            zm_msg_warn(" INPUT Channel (%d/%d) read frame failed", input->input_pipe, input->input_chn);
        }
    }
    input->dbg_recv_count++;
#endif

#endif
    return ERROR;
}

int zpl_vidhal_inputchn_pipe_update_fd(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *input)
{
#ifdef ZPL_HISIMPP_MODULE
    if (input->input_pipe >= 0 && input->pipefd != ZPL_SOCKET_INVALID)
    {
        ipstack_type(input->pipefd) = IPSTACK_OS;
        ipstack_fd(input->pipefd) = HI_MPI_VI_GetPipeFd(input->input_pipe);
        if (ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, DETAIL))
            zm_msg_debug(" video INPUT pipe %d fd %d\n", input->input_pipe, ipstack_fd(input->pipefd));
        return OK;
    }
    return ERROR;
#else
    return ERROR;
#endif
}

int zpl_vidhal_inputchn_update_fd(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *input)
{
#ifdef ZPL_HISIMPP_MODULE
    if (input->input_chn >= 0 && input->input_pipe >= 0 && input->chnfd != ZPL_SOCKET_INVALID)
    {
        ipstack_type(input->chnfd) = IPSTACK_OS;
        ipstack_fd(input->chnfd) = HI_MPI_VI_GetChnFd(input->input_pipe, input->input_chn);
        if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, DETAIL))
            zm_msg_debug(" video INPUT channel %d/%d fd %d\n", input->input_pipe, input->input_chn, ipstack_fd(input->chnfd));
        return OK;
    }
    return ERROR;
#else
    return ERROR;
#endif
}

#ifdef ZPL_HISIMPP_MODULE
static int zpl_vidhal_inputdev_bindpipe(int devnum, int pipe, zpl_bool enable)
{
    zpl_int32 i = 0, s32Ret = -1;
    VI_DEV_BIND_PIPE_S stDevBindPipe = {0};
    if(stGlobalDevBindPipe.u32Num == 0)
    {
        for (i = 0; i < VI_MAX_PHY_PIPE_NUM; i++)
        {
            stGlobalDevBindPipe.PipeId[i] = -1;
        }
        stGlobalDevBindPipe.u32Num = 1;
    }
    if(enable)
    {
        stGlobalDevBindPipe.PipeId[pipe] = pipe;
    }
    else
    {
        stGlobalDevBindPipe.PipeId[pipe] = -1;
    }
    memset(&stDevBindPipe, 0, sizeof(VI_DEV_BIND_PIPE_S));
 
    stDevBindPipe.u32Num = 0;
    for (i = 0; i < VI_MAX_PHY_PIPE_NUM; i++)
    {
        if (stGlobalDevBindPipe.PipeId[i] >= 0  && stGlobalDevBindPipe.PipeId[i] < VI_MAX_PHY_PIPE_NUM)
        {
            stDevBindPipe.PipeId[stDevBindPipe.u32Num] = stGlobalDevBindPipe.PipeId[i];
            stDevBindPipe.u32Num++;
            zm_msg_debug(" DEV (%d) BindPipe %d", devnum, stGlobalDevBindPipe.PipeId[i]);
        }
    }

    s32Ret = HI_MPI_VI_SetDevBindPipe(devnum, &stDevBindPipe);
    if (s32Ret != HI_SUCCESS)
    {
        if (ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zm_msg_err(" DEV (%d) BindPipe failed(%s)", devnum, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
}
#endif

int zpl_vidhal_inputchn_create(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *input)
{
#ifdef ZPL_HISIMPP_MODULE
    int ret = 0;
    int flag = ZPL_MEDIA_HALRES_GET(0, input->devnum, DEV );
    if(!ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_ACTIVE))
    {
        VI_DEV_ATTR_S stViDevAttr;
        zpl_vidhal_mipi_start(input->inputdev.snsdev, input->inputdev.mipidev, input->inputdev.snstype /*ZPL_SENSOR_TYPE_DEFAULT*/);

        zpl_vidhal_sensor_get_devattr(input->inputdev.snstype, &stViDevAttr);
        ret = HI_MPI_VI_SetDevAttr(input->devnum, &stViDevAttr);
        if (ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
                zm_msg_err(" DEV (%d) SetDevAttr failed(%s)", input->devnum, zpl_syshal_strerror(ret));
            return HI_FAILURE;
        }
        ret = HI_MPI_VI_EnableDev(input->devnum);
        if (ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
                zm_msg_err(" DEV (%d) Enable failed(%s)", input->devnum, zpl_syshal_strerror(ret));
            return HI_FAILURE;
        }
        ZPL_SET_BIT(flag, ZPL_MEDIA_STATE_ACTIVE);
        ZPL_MEDIA_HALRES_SET(-1, input->devnum, flag, DEV);

        zpl_vidhal_inputdev_bindpipe(input->devnum, input_pipe, zpl_true);
    }
    flag = ZPL_MEDIA_HALRES_GET(-1, input_pipe, VIPIPE);
    if (input_pipe >= 0 && !ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_ACTIVE))
    {
        VI_PIPE_ATTR_S stPipeAttr;
        zpl_vidhal_sensor_get_pipeattr(input->inputdev.snstype, &stPipeAttr);
        //sensor 输入图像大小
        //stPipeAttr.u32MaxW;                /* RW;Range[VI_PIPE_MIN_WIDTH, VI_PIPE_MAX_WIDTH];Maximum width */
        //stPipeAttr.u32MaxH;                /* RW;Range[VI_PIPE_MIN_HEIGHT, VI_PIPE_MAX_HEIGHT];Maximum height */
        stPipeAttr.bNrEn = HI_TRUE;
        stPipeAttr.bIspBypass = HI_FALSE; // HI_TRUE;//
        ret = HI_MPI_VI_CreatePipe(input_pipe, &stPipeAttr);
        if (ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                zm_msg_err(" INPUT pipe (%d) Create failed(%s(%x))", input_pipe, zpl_syshal_strerror(ret), ret);
            return HI_FAILURE;
        }
        ZPL_SET_BIT(flag, ZPL_MEDIA_STATE_ACTIVE);
        ZPL_MEDIA_HALRES_SET(-1, input_pipe, flag, VIPIPE);
    }
    /*目标图像大小
    Hi3516AV300:stSize 需不大于 3840*2160,否则报错。
    Hi3516DV300:stSize 需不大于 2688*1944,否则报错。
    Hi3516CV500:stSize 需不大于 2304*1296,否则报错。*/
    if (input_channel >= VI_MAX_PHY_CHN_NUM)
    {
        VI_EXT_CHN_ATTR_S stextChnAttr;
        VI_CHN_ATTR_S stChnAttr;
        flag = ZPL_MEDIA_HALRES_GET(-1, input->input_chn, INPUTCHN);
        if(!ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_ACTIVE))
        {
            zpl_vidhal_sensor_get_chnattr(input->inputdev.snstype, &stChnAttr);
            input->input_chn = 0;
            stChnAttr.u32Depth = 4;
            ret = HI_MPI_VI_SetChnAttr(input->input_pipe, input->input_chn, (VI_CHN_ATTR_S *)&stChnAttr);
            if (ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zm_msg_err(" INPUT channel (%d %d) Create failed(%s)", input->input_pipe, input->input_chn, zpl_syshal_strerror(ret));
                return HI_FAILURE;
            }
            ZPL_SET_BIT(flag, ZPL_MEDIA_STATE_ACTIVE);
            ZPL_MEDIA_HALRES_SET(-1, input->input_chn, flag, INPUTCHN);
        }
        else
        {
            ret = HI_MPI_VI_GetChnAttr(input->input_pipe, input->input_chn, (VI_CHN_ATTR_S *)&stChnAttr);
            if (ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zm_msg_err(" INPUT channel (%d %d) Create failed(%s)", input->input_pipe, input->input_chn, zpl_syshal_strerror(ret));
                return HI_FAILURE;
            }
        }
        stextChnAttr.stSize.u32Width = stChnAttr.stSize.u32Width;
        stextChnAttr.stSize.u32Height = stChnAttr.stSize.u32Height;
        stextChnAttr.enPixFormat = stChnAttr.enPixelFormat;
        stextChnAttr.enDynamicRange = stChnAttr.enDynamicRange;
        stextChnAttr.enCompressMode = stChnAttr.enCompressMode;
        stextChnAttr.u32Depth = stChnAttr.u32Depth;
        stextChnAttr.stFrameRate = stChnAttr.stFrameRate;
        input->input_size.width = stChnAttr.stSize.u32Width;
        input->input_size.height = stChnAttr.stSize.u32Height;
        if (input->output_size.width)
        {
            stextChnAttr.stSize.u32Width = input->output_size.width;
            stextChnAttr.stSize.u32Height = input->output_size.height;
        }
        stextChnAttr.s32BindChn = input->input_chn;     // 绑定的物理通道号
        stextChnAttr.enSource = VI_EXT_CHN_SOURCE_TAIL; // 扩展通道图像来源物理通道处理后
        flag = ZPL_MEDIA_HALRES_GET(-1, input->input_extchn, INPUTCHN);
        if(!ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_ACTIVE))
        {
            ret = HI_MPI_VI_SetExtChnAttr(input->input_pipe, input->input_extchn, (VI_EXT_CHN_ATTR_S *)&stextChnAttr);
            if (ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zm_msg_err(" INPUT channel (%d %d) Create failed(%s)", input->input_pipe, input->input_extchn, zpl_syshal_strerror(ret));
                return HI_FAILURE;
            }
            ZPL_SET_BIT(flag, ZPL_MEDIA_STATE_ACTIVE);
            ZPL_MEDIA_HALRES_SET(-1, input->input_extchn, flag, INPUTCHN);
        }
    }
    else
    {
        VI_CHN_ATTR_S stChnAttr;
        zpl_vidhal_sensor_get_chnattr(input->inputdev.snstype, &stChnAttr);
        input->input_size.width = stChnAttr.stSize.u32Width;
        input->input_size.height = stChnAttr.stSize.u32Height;
        //目标图像小于sensor图像进行裁剪
        if (input->output_size.width && 
            stChnAttr.stSize.u32Width >= input->output_size.width && 
            stChnAttr.stSize.u32Height >= input->output_size.height)
        {
            stChnAttr.stSize.u32Width = input->output_size.width;
            stChnAttr.stSize.u32Height = input->output_size.height;
        }
        else
        {

        }
        stChnAttr.u32Depth = 4;
        flag = ZPL_MEDIA_HALRES_GET(-1, input_channel, INPUTCHN);
        flag = ZPL_MEDIA_HALRES_GET(-1, input_channel, INPUTCHN);
        if(!ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_ACTIVE))
        {
            ret = HI_MPI_VI_SetChnAttr(input->input_pipe, input->input_chn, (VI_CHN_ATTR_S *)&stChnAttr);
            if (ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zm_msg_err(" INPUT channel (%d %d) Create failed(%s)", input->input_pipe, input->input_chn, zpl_syshal_strerror(ret));
                return HI_FAILURE;
            }
            ZPL_SET_BIT(flag, ZPL_MEDIA_STATE_ACTIVE);
            ZPL_MEDIA_HALRES_SET(-1, input_channel, flag, INPUTCHN);
        }
    }
    return ret;
#else
    return OK;
#endif
}


int zpl_vidhal_inputchn_start(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *input)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    zpl_vidhal_isp_sensor_t ispsensor;

    int flag = ZPL_MEDIA_HALRES_GET(0, input_pipe, VIPIPE);   
    if(!ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_START))
    {
        s32Ret = HI_MPI_VI_StartPipe(input_pipe);

        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                zm_msg_err(" INPUT pipe (%d) Start failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
            HI_MPI_VI_DestroyPipe(input_pipe);
            return HI_FAILURE;
        }
        ZPL_SET_BIT(flag, ZPL_MEDIA_STATE_START);
        ZPL_MEDIA_HALRES_SET(-1, input_pipe, flag, VIPIPE);
    }
    flag = ZPL_MEDIA_HALRES_GET(0, input_channel, INPUTCHN);
    if(!ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_START))
    {
        s32Ret = HI_MPI_VI_EnableChn(input_pipe, input_channel);

        if (s32Ret != HI_SUCCESS)
        {
            HI_MPI_VI_DisableChn(input_pipe, input_channel);
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zm_msg_err(" INPUT channel (%d %d) enable failed(%s)", input_pipe, input_channel, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        ZPL_SET_BIT(flag, ZPL_MEDIA_STATE_START);
        ZPL_MEDIA_HALRES_SET(-1, input_channel, flag, INPUTCHN);
    }
    zpl_vidhal_isp_setparam(input->input_pipe, 30);

    zm_msg_debug("==================%s pipe=%d chn=%d", __func__, input->input_pipe, input->input_chn);

    // ispsensor.sns_id = 0;                     //底层设备编号
    ispsensor.isp_dev = input->input_pipe;//ZPL_ISPDEV_0;             // 底层硬件pipe
    ispsensor.sns_type = ZPL_SENSOR_TYPE_DEFAULT; // SONY_IMX327_MIPI_2M_30FPS_12BIT;                   //底层通道号
    ispsensor.sns_type = input->inputdev.snstype; // SONY_IMX327_2L_MIPI_2M_30FPS_12BIT;
    // ispsensor.pipe[VI_MAX_PIPE_NUM];
    ispsensor.enWDRMode = 0;
    ispsensor.bMultiPipe = 0;
    ispsensor.SnapPipe = 0;
    ispsensor.bDoublePipe = 0;
    ispsensor.s32BusId = ZPL_BUSID_0;
    ispsensor.pipe[0] = input->input_pipe;
    ispsensor.pipe[1] = -1;
    ispsensor.pipe[2] = -1;
    ispsensor.pipe[3] = -1;
    ispsensor.vipipe = input->input_pipe;
    // zpl_vidhal_isp_start(&ispsensor);
    zpl_vidhal_isp_start_one(&ispsensor);
    return s32Ret;
#else
    return OK;
#endif
}

int zpl_vidhal_inputchn_stop(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *input)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    int flag = ZPL_MEDIA_HALRES_GET(0, input_channel, INPUTCHN);
    if(ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_START))
    {
        s32Ret = HI_MPI_VI_DisableChn(input_pipe, input_channel);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zm_msg_err(" INPUT channel (%d %d) disable failed(%s)", input_pipe, input_channel, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        ZPL_CLR_BIT(flag, ZPL_MEDIA_STATE_START);
        ZPL_MEDIA_HALRES_SET(-1, input_channel, flag, INPUTCHN);
    }
    flag = ZPL_MEDIA_HALRES_GET(0, input_pipe, VIPIPE);
    if(ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_START))
    {
        s32Ret = HI_MPI_VI_StopPipe(input_pipe);

        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                zm_msg_err(" INPUT pipe (%d) Stop failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        ZPL_CLR_BIT(flag, ZPL_MEDIA_STATE_START);
        ZPL_MEDIA_HALRES_SET(-1, input_pipe, flag, VIPIPE);
    }
    return s32Ret;
#else
    return OK;
#endif
}

int zpl_vidhal_inputchn_destroy(zpl_int32 input_pipe, zpl_int32 input_channel, zpl_media_video_inputchn_t *input)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    int flag = ZPL_MEDIA_HALRES_GET(0, input_channel, INPUTCHN);
    if(ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_ACTIVE))
    {
        s32Ret = HI_MPI_VI_DisableChn(input_pipe, input_channel);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zm_msg_err(" INPUT channel (%d %d) disable failed(%s)", input_pipe, input_channel, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        ZPL_CLR_BIT(flag, ZPL_MEDIA_STATE_ACTIVE);
        ZPL_MEDIA_HALRES_SET(-1, input_channel, flag, INPUTCHN);
    }
    flag = ZPL_MEDIA_HALRES_GET(0, input_pipe, VIPIPE);
    if(ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_ACTIVE))
    {
        s32Ret = HI_MPI_VI_DestroyPipe(input_pipe);

        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                zm_msg_err(" INPUT pipe (%d) Destroy failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        ZPL_CLR_BIT(flag, ZPL_MEDIA_STATE_ACTIVE);
        ZPL_MEDIA_HALRES_SET(-1, input_pipe, flag, VIPIPE);
    }
    flag = ZPL_MEDIA_HALRES_GET(0, input->devnum, DEV );
    if(ZPL_TST_BIT(flag, ZPL_MEDIA_STATE_ACTIVE))
    {
        HI_MPI_VI_CloseFd();
        s32Ret = HI_MPI_VI_DisableDev(input->devnum);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
                zm_msg_err(" DEV (%d) Enable failed(%s)", input->devnum, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        ZPL_CLR_BIT(flag, ZPL_MEDIA_STATE_ACTIVE);
        ZPL_MEDIA_HALRES_SET(-1, input->devnum, flag, DEV);

          zpl_vidhal_mipi_stop(input->inputdev.snsdev, input->inputdev.mipidev);
    }
    return s32Ret;
#else
    return OK;
#endif
}
