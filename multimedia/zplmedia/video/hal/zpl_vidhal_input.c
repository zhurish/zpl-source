/*
 * zpl_vidhal_input.c
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
#include "zpl_vidhal_sensor.h"

#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"

static int zpl_vidhal_input_dev_enable(zpl_int32 dev)
{
    VI_DEV_ATTR_S stViDevAttr;
    zpl_vidhal_sensor_get_devattr(ZPL_SENSOR_TYPE_DEFAULT, &stViDevAttr);

    zpl_int32 s32Ret = HI_MPI_VI_SetDevAttr(dev, &stViDevAttr);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err(" DEV (%d) SetDevAttr failed(%s)", dev, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VI_EnableDev(dev);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err(" DEV (%d) Enable failed(%s)", dev, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static int zpl_vidhal_input_dev_disable(zpl_int32 dev)
{
    zpl_int32 s32Ret = HI_MPI_VI_DisableDev(dev);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err(" DEV (%d) Disable failed(%s)", dev, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}



static int zpl_vidhal_input_pipe_connect_dev(zpl_int32 *pipe, zpl_int32 dev)
{
    zpl_int32              i;
    zpl_int32              s32PipeCnt = 0;
    zpl_int32              s32Ret;
    VI_DEV_BIND_PIPE_S  stDevBindPipe = {0};
    for (i = 0; i < VI_MAX_PIPE_NUM; i++)
    {
        stDevBindPipe.PipeId[i] = -1;
        stDevBindPipe.u32Num = 0;
    }
    for (i = 0; i < VI_MAX_PIPE_NUM; i++)
    {
        if (pipe[i] >= 0  && pipe[i] < VI_MAX_PIPE_NUM)
        {
            stDevBindPipe.PipeId[s32PipeCnt] = pipe[i];
            s32PipeCnt++;
            stDevBindPipe.u32Num = s32PipeCnt;
        }
    }
    s32Ret = HI_MPI_VI_SetDevBindPipe(dev, &stDevBindPipe);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err(" DEV (%d) BindPipe failed(%s)", dev, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }

    return s32Ret;
}

static int _zpl_vidhal_input_pipe_create(zpl_int32 input_pipe)
{
    zpl_int32 s32Ret;
    VI_PIPE_ATTR_S stPipeAttr;
    zpl_vidhal_sensor_get_pipeattr(ZPL_SENSOR_TYPE_DEFAULT, &stPipeAttr);

    stPipeAttr.bNrEn    = HI_TRUE;
    stPipeAttr.bIspBypass    = HI_FALSE;//HI_TRUE;//
    s32Ret = HI_MPI_VI_CreatePipe(input_pipe, &stPipeAttr);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
            zpl_media_debugmsg_err(" INPUT pipe (%d) Create failed(%s(%x))", input_pipe, zpl_syshal_strerror(s32Ret),s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
}


static int _zpl_vidhal_input_pipe_start(zpl_int32 input_pipe)
{
    zpl_int32 s32Ret;

    s32Ret = HI_MPI_VI_StartPipe(input_pipe);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
            zpl_media_debugmsg_err(" INPUT pipe (%d) Start failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
        HI_MPI_VI_DestroyPipe(input_pipe);
        return HI_FAILURE;
    }

    return s32Ret;
}

static int _zpl_vidhal_input_pipe_stop(zpl_int32 input_pipe)
{
    zpl_int32  s32Ret;
    s32Ret = HI_MPI_VI_StopPipe(input_pipe);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
            zpl_media_debugmsg_err(" INPUT pipe (%d) Stop failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
}

static int _zpl_vidhal_input_pipe_destroy(zpl_int32 input_pipe)
{
    zpl_int32  s32Ret;
    s32Ret = HI_MPI_VI_DestroyPipe(input_pipe);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
            zpl_media_debugmsg_err(" INPUT pipe (%d) Destroy failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
}


static int _zpl_vidhal_input_channel_create(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_video_size_t *vidsize)
{
    zpl_int32 s32Ret;

    if(input_chn >= VI_MAX_PHY_CHN_NUM)
    {
        VI_EXT_CHN_ATTR_S stChnAttr;
        zpl_vidhal_sensor_get_extchnattr(ZPL_SENSOR_TYPE_DEFAULT, &stChnAttr);
        //input->input_size.width = stChnAttr.stSize.u32Width;
        //input->input_size.height = stChnAttr.stSize.u32Height;
        if(vidsize->width)
        {
			stChnAttr.stSize.u32Width = vidsize->width;
			stChnAttr.stSize.u32Height = vidsize->height;
        }
        stChnAttr.s32BindChn = 0;//绑定的物理通道号
        stChnAttr.enSource = VI_EXT_CHN_SOURCE_TAIL;//扩展通道图像来源物理通道处理后
		stChnAttr.u32Depth = 4;
            s32Ret = HI_MPI_VI_SetExtChnAttr(input_pipe, input_chn, (VI_EXT_CHN_ATTR_S *)&stChnAttr);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zpl_media_debugmsg_err(" INPUT channel (%d %d) Create failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
            

    }
    else
    {
    
            VI_CHN_ATTR_S stChnAttr;
            zpl_vidhal_sensor_get_chnattr(ZPL_SENSOR_TYPE_DEFAULT, &stChnAttr);
            //input->input_size.width = stChnAttr.stSize.u32Width;
            //input->input_size.height = stChnAttr.stSize.u32Height;
			if(vidsize->width)
			{
				stChnAttr.stSize.u32Width = vidsize->width;
				stChnAttr.stSize.u32Height = vidsize->height;
			}
            //stChnAttr.stBasAttr.stSacleAttr.stBasSize.u32Width = input->sacle_size.width;
            //stChnAttr.stBasAttr.stSacleAttr.stBasSize.u32Height = input->sacle_size.height;
			stChnAttr.u32Depth = 4;
            s32Ret = HI_MPI_VI_SetChnAttr(input_pipe, input_chn, (VI_CHN_ATTR_S*)&stChnAttr);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zpl_media_debugmsg_err(" INPUT channel (%d %d) Create failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
            
    }
    return s32Ret;
}


static int zpl_vidhal_input_chn_start(zpl_int32 input_pipe, zpl_int32 input_chn)
{
    zpl_int32 s32Ret;

    s32Ret = HI_MPI_VI_EnableChn(input_pipe, input_chn);

    if (s32Ret != HI_SUCCESS)
    {
        HI_MPI_VI_DisableChn(input_pipe, input_chn);
        if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
            zpl_media_debugmsg_err(" INPUT channel (%d %d) enable failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }

    return s32Ret;
}

static int zpl_vidhal_input_chn_stop(zpl_int32 input_pipe, zpl_int32 input_chn)
{
    zpl_int32  s32Ret;
    s32Ret = HI_MPI_VI_DisableChn(input_pipe, input_chn);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
            zpl_media_debugmsg_err(" INPUT channel (%d %d) disable failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
}

#endif



int zpl_vidhal_input_crop(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_bool out, zpl_video_size_t cropsize)
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
                if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                    zpl_media_debugmsg_err(" INPUT Pipe (%d) GetPreCrop failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
            stCropInfo.bEnable =  cropsize.width ? HI_TRUE:HI_FALSE;
            stCropInfo.stRect.s32X = (stCropInfo.stRect.u32Width - cropsize.width) / 2;
            stCropInfo.stRect.s32Y = (stCropInfo.stRect.u32Height - cropsize.height) / 2;
            stCropInfo.stRect.u32Width = cropsize.width;
            stCropInfo.stRect.u32Height = cropsize.height;
            s32Ret = HI_MPI_VI_SetPipePreCrop(input_pipe, &stCropInfo);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                    zpl_media_debugmsg_err(" INPUT Pipe (%d) SetPreCrop failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
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
                if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                    zpl_media_debugmsg_err(" INPUT Pipe (%d) GetPostCrop failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
            stCropInfo.bEnable =  cropsize.width ? HI_TRUE:HI_FALSE;
            stCropInfo.stRect.s32X = (stCropInfo.stRect.u32Width - cropsize.width) / 2;
            stCropInfo.stRect.s32Y = (stCropInfo.stRect.u32Height - cropsize.height) / 2;
            stCropInfo.stRect.u32Width = cropsize.width;
            stCropInfo.stRect.u32Height = cropsize.height;
            s32Ret = HI_MPI_VI_SetPipePostCrop(input_pipe, &stCropInfo);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                    zpl_media_debugmsg_err(" INPUT Pipe (%d) SetPostCrop failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
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
                if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zpl_media_debugmsg_err(" INPUT Channel (%d %d) GetCrop failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
            stCropInfo.bEnable =  cropsize.width ? HI_TRUE:HI_FALSE;
            stCropInfo.enCropCoordinate = VI_CROP_ABS_COOR;
            stCropInfo.stCropRect.s32X = (stCropInfo.stCropRect.u32Width - cropsize.width) / 2;
            stCropInfo.stCropRect.s32Y = (stCropInfo.stCropRect.u32Height - cropsize.height) / 2;
            stCropInfo.stCropRect.u32Width = cropsize.width;
            stCropInfo.stCropRect.u32Height = cropsize.height;
            s32Ret = HI_MPI_VI_SetChnCrop(input_pipe, input_chn, &stCropInfo);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zpl_media_debugmsg_err(" INPUT Channel (%d %d) SetCrop failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
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

int zpl_vidhal_input_fish_eye(zpl_int32 input_pipe, zpl_int32 input_chn, void *LMF)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;

    if (input_chn == ZPL_INVALID_VAL)
    {
        FISHEYE_CONFIG_S stInfo;
        s32Ret = HI_MPI_VI_GetPipeFisheyeConfig(input_pipe, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
                zpl_media_debugmsg_err(" INPUT Pipe (%d) GetFisheye failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        memcpy(stInfo.au16LMFCoef, LMF, sizeof(stInfo.au16LMFCoef));
        s32Ret = HI_MPI_VI_SetPipeFisheyeConfig(input_pipe, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zpl_media_debugmsg_err(" INPUT Pipe (%d) SetFisheye failed(%s)", input_pipe, zpl_syshal_strerror(s32Ret));
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
            if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zpl_media_debugmsg_err(" INPUT Channel (%d %d) GetCrop failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_VI_SetExtChnFisheye(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zpl_media_debugmsg_err(" INPUT Channel (%d %d) GetCrop failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        return HI_SUCCESS;
    }
    return HI_FAILURE;
#else
    return ERROR;
#endif
}
int zpl_vidhal_input_rotation(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_uint32 rotation)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    if (input_chn != ZPL_INVALID_VAL)
    {

        s32Ret = HI_MPI_VI_SetChnRotation(input_pipe, input_chn, rotation);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zpl_media_debugmsg_err(" INPUT Channel (%d %d) Rotation failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        return HI_SUCCESS;
    }
    return HI_FAILURE;
#else
    return ERROR;
#endif
}

int zpl_vidhal_input_rotation_angle(zpl_int32 input_pipe, zpl_int32 input_chn, void *p)
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
                if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zpl_media_debugmsg_err(" INPUT Channel (%d %d) get RotationEx failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
            s32Ret = HI_MPI_VI_SetChnRotationEx(input_pipe, input_chn, &stViRotationExAttr);
            if (s32Ret != HI_SUCCESS)
            {
                if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                    zpl_media_debugmsg_err(" INPUT Channel (%d %d) set RotationEx failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
                return HI_FAILURE;
            }
        }
    }
    return HI_FAILURE;
#else
    return ERROR;
#endif
}

int zpl_vidhal_input_ldc(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_video_size_t cropsize) //畸形矫正
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    if (input_chn != ZPL_INVALID_VAL)
    {
        VI_LDC_ATTR_S stInfo;

        s32Ret = HI_MPI_VI_GetChnLDCAttr(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zpl_media_debugmsg_err(" INPUT Channel (%d %d) get LDCAttr failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_VI_SetChnLDCAttr(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zpl_media_debugmsg_err(" INPUT Channel (%d %d) set LDCAttr failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return HI_FAILURE;
#else
    return ERROR;
#endif
}

int zpl_vidhal_input_spread(zpl_int32 input_pipe, zpl_int32 input_chn, void *p)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    if (input_chn != ZPL_INVALID_VAL)
    {
        SPREAD_ATTR_S stInfo;
        s32Ret = HI_MPI_VI_GetChnSpreadAttr(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zpl_media_debugmsg_err(" INPUT Channel (%d %d) get Spread failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_VI_SetChnSpreadAttr(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zpl_media_debugmsg_err(" INPUT Channel (%d %d) set Spread failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return HI_FAILURE;
#else
    return ERROR;
#endif
}

int zpl_vidhal_input_mirror_flip(zpl_int32 input_pipe, zpl_int32 input_chn, zpl_bool mirror, zpl_bool flip) //
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    if (input_chn != ZPL_INVALID_VAL)
    {
        VI_CHN_ATTR_S stInfo;
        s32Ret = HI_MPI_VI_GetChnAttr(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zpl_media_debugmsg_err(" INPUT Channel (%d %d) get chnattr failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        stInfo.bMirror = mirror;
        stInfo.bFlip = flip;
        s32Ret = HI_MPI_VI_SetChnAttr(input_pipe, input_chn, &stInfo);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
                zpl_media_debugmsg_err(" INPUT Channel (%d %d) set chnattr failed(%s)", input_pipe, input_chn, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return HI_FAILURE;
#else
    return ERROR;
#endif
}




#ifdef ZPL_HISIMPP_MODULE
static int zpl_vidhal_input_pipe_read_frame(zpl_int32 pipe, void *p)
{
    int s32Ret = HI_MPI_VI_GetPipeFrame(pipe, p, -1);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
            zpl_media_debugmsg_err(" INPUT Pipe (%d) get pipe frame failed(%s)", pipe, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
}

static int zpl_vidhal_input_pipe_release_frame(zpl_int32 pipe, void *p)
{
    int s32Ret = HI_MPI_VI_ReleasePipeFrame(pipe, p);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE))
            zpl_media_debugmsg_err(" INPUT Pipe (%d) release pipe frame failed(%s)", pipe, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
}

static int zpl_vidhal_input_chn_read_frame(zpl_int32 pipe, zpl_int32 chn, void *p, zpl_int s32MilliSec)
{
    int s32Ret = HI_MPI_VI_GetChnFrame(pipe, chn, p, s32MilliSec);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
            zpl_media_debugmsg_err(" INPUT Channel (%d %d) get frame failed(%s)", pipe, chn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret; 
}

static int zpl_vidhal_input_chn_release_frame(zpl_int32 pipe, zpl_int32 chn, void *p)
{
    int s32Ret = HI_MPI_VI_ReleaseChnFrame(pipe, chn, p);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE))
            zpl_media_debugmsg_err(" INPUT Channel (%d %d) release frame failed(%s)", pipe, chn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
}
#endif



int zpl_vidhal_input_pipe_frame_recvfrom(zpl_video_input_pipe_t *input)
{
    #ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32MilliSec = 200;
    VIDEO_FRAME_INFO_S stFrmInfo_input;
    stFrmInfo_input.u32PoolId = -1U;
	stFrmInfo_input.stVFrame.u64PhyAddr[0] = 0U;
    if(zpl_vidhal_input_pipe_read_frame(input->input_pipe, &stFrmInfo_input) == HI_SUCCESS)
    {
        zpl_video_size_t input_size;
        //zpl_video_encode_t *venc_ptr = input->venc_ptr;
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
        zpl_media_hardadap_handle(&input->callback, &stFrmInfo_input,  s32MilliSec);
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
		if(input->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
		{
			input->dbg_recv_count = 0;
			if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE) && ZPL_MEDIA_DEBUG(INPUT, RECV))
			{
				zpl_media_debugmsg_warn(" INPUT Pipe (%d) read frame datasize %d", input->input_pipe, datasize);
			}
		}
		input->dbg_recv_count++;
#endif
        //zpl_media_debugmsg_warn(" ----------------------INPUT Pipe (%d) read frame datasize %d", input->input_pipe, datasize);
        /*
        if(input->input_sendto)
            (input->input_sendto)(input->toid, input->tochn, &stFrmInfo_input,  s32MilliSec);
        */
        if(stFrmInfo_input.u32PoolId != -1U)
            zpl_vidhal_input_pipe_release_frame(input->input_pipe, &stFrmInfo_input);
        return OK;
    }
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
	if(input->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
	{
		input->dbg_recv_count = 0;
		if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, HARDWARE) && ZPL_MEDIA_DEBUG(INPUTPIPE, RECV))
		{
			zpl_media_debugmsg_warn(" INPUT Pipe (%d) read frame failed", input->input_pipe);
		}
	}
	input->dbg_recv_count++;
#endif

    #endif
    return ERROR;
}

int zpl_vidhal_input_channel_frame_recvfrom(zpl_video_input_channel_t *input)
{
    #ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32MilliSec = 200;
    VIDEO_FRAME_INFO_S stFrmInfo_input;
    stFrmInfo_input.u32PoolId = -1U;
	stFrmInfo_input.stVFrame.u64PhyAddr[0] = 0U;
    if(zpl_vidhal_input_chn_read_frame(input->inputpipe->input_pipe, input->input_chn, &stFrmInfo_input,  s32MilliSec) == HI_SUCCESS)
    {
        zpl_video_size_t input_size;
        //zpl_video_encode_t *venc_ptr = input->venc_ptr;
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
        zpl_media_hardadap_handle(&input->callback, &stFrmInfo_input,  s32MilliSec);
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
		if(input->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
		{
			input->dbg_recv_count = 0;
			if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE) && ZPL_MEDIA_DEBUG(INPUT, RECV))
			{
				zpl_media_debugmsg_warn(" INPUT Channel (%d/%d) read frame datasize %d", input->inputpipe->input_pipe, input->input_chn, datasize);
			}
		}
		input->dbg_recv_count++;
#endif
        //zpl_media_debugmsg_warn(" ------------------INPUT Channel (%d/%d) read frame datasize %d", input->inputpipe->input_pipe, input->input_chn, datasize);
        /*
        if(input->input_sendto)
            (input->input_sendto)(input->toid, input->tochn, &stFrmInfo_input,  s32MilliSec);
        */
        if(stFrmInfo_input.u32PoolId != -1U)
            zpl_vidhal_input_chn_release_frame(input->inputpipe->input_pipe, input->input_chn, &stFrmInfo_input);
        return OK;
    }
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
	if(input->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
	{
		input->dbg_recv_count = 0;
		if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, HARDWARE) && ZPL_MEDIA_DEBUG(INPUT, RECV))
		{
			zpl_media_debugmsg_warn(" INPUT Channel (%d/%d) read frame failed", input->inputpipe->input_pipe, input->input_chn);
		}
	}
	input->dbg_recv_count++;
#endif

    #endif
    return ERROR;
}


int zpl_vidhal_input_pipe_create(zpl_video_input_pipe_t *input)
{
    #ifdef ZPL_HISIMPP_MODULE
    zpl_int32 vipipe[VI_MAX_PIPE_NUM] = { -1 };
    //zpl_vidhal_isp_sensor_t ispsensor;
    zpl_video_resources_get_pipe(vipipe);

    //MIPI Start
	zpl_vidhal_mipi_start(ZPL_SNSDEV_0, ZPL_MIPIDEV_0, ZPL_SENSOR_TYPE_DEFAULT);

    zpl_syshal_vivpss_mode_set(input->input_pipe, VI_ONLINE_VPSS_ONLINE);
    //zpl_syshal_vivpss_mode_set(input->input_pipe, VI_ONLINE_VPSS_OFFLINE);
    //zpl_syshal_vivpss_mode_set(input->input_pipe, VI_OFFLINE_VPSS_OFFLINE);
    
    zpl_vidhal_input_dev_enable(input->input_dev);
    vipipe[0] = 0;
    vipipe[1] = -1;
    vipipe[2] = -1;
    vipipe[3] = -1;
    zpl_vidhal_input_pipe_connect_dev(vipipe, input->input_dev);

    _zpl_vidhal_input_pipe_create(input->input_pipe);
    //_zpl_vidhal_input_pipe_start(input->input_pipe);
	
zpl_media_debugmsg_debug("==================%s dev=%d pipe=%d", __func__,input->input_dev, input->input_pipe);

/*
    //_zpl_vidhal_input_channel_create(input->input_pipe, 0, &input->input_size);
    //zpl_vidhal_input_chn_start(input->input_pipe, 0);
    zpl_vidhal_isp_setparam(input->input_pipe, 30);

    ispsensor.sns_id = 0;                     //底层设备编号
    ispsensor.isp_dev = 0;                    //底层硬件pipe
    ispsensor.sns_type = SONY_IMX327_MIPI_2M_30FPS_12BIT;                   //底层通道号
    ispsensor.sns_type = SONY_IMX327_2L_MIPI_2M_30FPS_12BIT;
    //ispsensor.pipe[VI_MAX_PIPE_NUM];
    ispsensor.enWDRMode = 0;
    ispsensor.bMultiPipe = 0;
    ispsensor.SnapPipe = 0;
    ispsensor.bDoublePipe = 0;
    ispsensor.s32BusId = 0;
    memcpy(ispsensor.pipe, vipipe, sizeof(zpl_int32)*VI_MAX_PIPE_NUM);

    zpl_vidhal_isp_start(&ispsensor);
    //start vi
*/
    return OK;
    #else
    return ERROR;
    #endif
}


int zpl_vidhal_input_pipe_update_fd(zpl_video_input_pipe_t *input)
{
#ifdef ZPL_HISIMPP_MODULE
    if(input->input_pipe >= 0)
    {
        ipstack_type(input->pipefd) = IPSTACK_OS;
        ipstack_fd(input->pipefd) = HI_MPI_VI_GetPipeFd(input->input_pipe);
		if(ZPL_MEDIA_DEBUG(INPUTPIPE, EVENT) && ZPL_MEDIA_DEBUG(INPUTPIPE, DETAIL))
			zpl_media_debugmsg_debug(" video INPUT pipe %d fd %d\n", input->input_pipe, input->pipefd);
        return OK;
    }
    return ERROR;
#else
    return ERROR;
#endif
}

int zpl_vidhal_input_channel_update_fd(zpl_video_input_channel_t *input)
{
#ifdef ZPL_HISIMPP_MODULE
    if(input->input_chn >= 0 && input->inputpipe->input_pipe >= 0)
    {
        ipstack_type(input->chnfd) = IPSTACK_OS;
        ipstack_fd(input->chnfd) = HI_MPI_VI_GetChnFd(input->inputpipe->input_pipe, input->input_chn); 
		if(ZPL_MEDIA_DEBUG(INPUT, EVENT) && ZPL_MEDIA_DEBUG(INPUT, DETAIL))
			zpl_media_debugmsg_debug(" video INPUT channel %d/%d fd %d\n", input->inputpipe->input_pipe, input->input_chn, input->chnfd);
        return OK;
    }
    return ERROR;
#else
    return ERROR;
#endif
}

int zpl_vidhal_input_pipe_start(zpl_video_input_pipe_t *input)
{
    #ifdef ZPL_HISIMPP_MODULE
    _zpl_vidhal_input_pipe_start(input->input_pipe);
    zpl_vidhal_input_pipe_update_fd(input);
	zpl_media_debugmsg_debug("==================%s dev=%d pipe=%d", __func__,input->input_dev, input->input_pipe);
    #endif
    return OK;
}

int zpl_vidhal_input_channel_create(zpl_video_input_channel_t *input)
{
    #ifdef ZPL_HISIMPP_MODULE
    _zpl_vidhal_input_channel_create(input->inputpipe->input_pipe, input->input_chn, &input->input_size);
	zpl_media_debugmsg_debug("==================%s pipe=%d chn=%d", __func__,input->inputpipe->input_pipe, input->input_chn);
    #endif
    return OK;
}

int zpl_vidhal_input_channel_start(zpl_video_input_channel_t *input)
{
    #ifdef ZPL_HISIMPP_MODULE
    zpl_vidhal_isp_sensor_t ispsensor;
    zpl_vidhal_input_chn_start(input->inputpipe->input_pipe, input->input_chn);
    zpl_vidhal_input_channel_update_fd(input);

    zpl_vidhal_isp_setparam(input->inputpipe->input_pipe, 30);
	
zpl_media_debugmsg_debug("==================%s pipe=%d chn=%d", __func__,input->inputpipe->input_pipe, input->input_chn);

    //ispsensor.sns_id = 0;                     //底层设备编号
    ispsensor.isp_dev = ZPL_ISPDEV_0;                    //底层硬件pipe
    ispsensor.sns_type = ZPL_SENSOR_TYPE_DEFAULT;//SONY_IMX327_MIPI_2M_30FPS_12BIT;                   //底层通道号
    ispsensor.sns_type = ZPL_SENSOR_TYPE_DEFAULT;//SONY_IMX327_2L_MIPI_2M_30FPS_12BIT;
    //ispsensor.pipe[VI_MAX_PIPE_NUM];
    ispsensor.enWDRMode = 0;
    ispsensor.bMultiPipe = 0;
    ispsensor.SnapPipe = 0;
    ispsensor.bDoublePipe = 0;
    ispsensor.s32BusId = ZPL_BUSID_0;
    ispsensor.pipe[0] = input->inputpipe->input_pipe;
    ispsensor.pipe[1] = -1;
    ispsensor.pipe[2] = -1;
    ispsensor.pipe[3] = -1;
    ispsensor.vipipe = input->inputpipe->input_pipe;
    //zpl_vidhal_isp_start(&ispsensor);
    zpl_vidhal_isp_start_one(&ispsensor);

    #endif
    return OK;
}

int zpl_vidhal_input_pipe_stop(zpl_video_input_pipe_t *input)
{
    #ifdef ZPL_HISIMPP_MODULE
    _zpl_vidhal_input_pipe_stop(input->input_pipe);
    #endif
    return OK;
}

int zpl_vidhal_input_channel_stop(zpl_video_input_channel_t *input)
{
    #ifdef ZPL_HISIMPP_MODULE
    if(zpl_vidhal_input_chn_stop(input->inputpipe->input_pipe, input->input_chn) == OK)
    {
    }
    #endif
    return OK;
}

int zpl_vidhal_input_pipe_destroy(zpl_video_input_pipe_t *input)
{
    #ifdef ZPL_HISIMPP_MODULE
    if(_zpl_vidhal_input_pipe_destroy(input->input_pipe) == OK)
       ;// VIDHAL_RES_FLAG_UNSET(input->res_flag[0], INIT);

    zpl_vidhal_input_dev_disable(input->input_dev);
	zpl_vidhal_mipi_stop(ZPL_SNSDEV_0, ZPL_MIPIDEV_0);
    HI_MPI_VI_CloseFd();
    return OK;
    #else
    return ERROR;
    #endif
}

int zpl_vidhal_input_channel_destroy(zpl_video_input_channel_t *input)
{
    zpl_vidhal_input_channel_stop(input);
    return OK;
}

