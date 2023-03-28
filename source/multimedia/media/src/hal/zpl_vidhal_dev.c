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
#endif

int zpl_vidhal_inputdev_bindpipe(zpl_media_video_inputchn_t *input, int pipe, zpl_bool enable)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 i = 0, s32Ret = -1;
    VI_DEV_BIND_PIPE_S stDevBindPipe = {0};

    for (i = 0; i < VI_MAX_PHY_PIPE_NUM; i++)
    {
        stDevBindPipe.PipeId[i] = -1;
        stDevBindPipe.u32Num = 0;
    }
    s32Ret = HI_MPI_VI_GetDevBindPipe(input->devnum, &stDevBindPipe);
    if (s32Ret != HI_SUCCESS)
    {
        if (ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zm_msg_err(" DEV (%d) BindPipe failed(%s)", input->devnum, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    if (stDevBindPipe.u32Num == 0)
    {
        if (enable == zpl_false)
        {
            stDevBindPipe.u32Num = 0;
        }
        else
        {
            stDevBindPipe.PipeId[stDevBindPipe.u32Num] = pipe;
            stDevBindPipe.u32Num++;
        }
    }
    else
    {
        s32Ret = 0;
        if (enable == zpl_false)
        {
            for (i = 0; (i < stDevBindPipe.u32Num) && (i < VI_MAX_PHY_PIPE_NUM); i++)
            {
                if (stDevBindPipe.PipeId[i] >= 0 &&
                    stDevBindPipe.PipeId[i] < VI_MAX_PHY_PIPE_NUM &&
                    stDevBindPipe.PipeId[i] == pipe)
                {
                    stDevBindPipe.PipeId[i] = -1;
                    stDevBindPipe.u32Num--;
                    break;
                }
            }
        }
        else
        {
            for (i = 0; (i < stDevBindPipe.u32Num) && (i < VI_MAX_PHY_PIPE_NUM); i++)
            {
                if (stDevBindPipe.PipeId[i] >= 0 &&
                    stDevBindPipe.PipeId[i] < VI_MAX_PHY_PIPE_NUM &&
                    stDevBindPipe.PipeId[i] != pipe)
                {
                    s32Ret = 1;
                    break;
                }
            }
            if (s32Ret)
            {
                stDevBindPipe.PipeId[stDevBindPipe.u32Num] = pipe;
                stDevBindPipe.u32Num++;
            }
        }
    }
    s32Ret = HI_MPI_VI_SetDevBindPipe(input->devnum, &stDevBindPipe);

    if (s32Ret != HI_SUCCESS)
    {
        if (ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zm_msg_err(" DEV (%d) BindPipe failed(%s)", input->devnum, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return OK;
#endif
}

int zpl_vidhal_inputdev_start(zpl_media_video_inputchn_t *input, zpl_bool enable)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    if (enable)
    {
        zpl_int32 i = 0;
        zpl_int32 vipipe[VI_MAX_PHY_PIPE_NUM] = {-1};
        VI_DEV_BIND_PIPE_S stDevBindPipe = {0};

        //zpl_video_resources_get_pipe(vipipe);

        s32Ret = HI_MPI_VI_EnableDev(input->devnum);

        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
                zm_msg_err(" DEV (%d) Enable failed(%s)", input->devnum, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        for (i = 0; i < VI_MAX_PIPE_NUM; i++)
        {
            stDevBindPipe.PipeId[i] = -1;
            stDevBindPipe.u32Num = 0;
        }
        for (i = 0; i < VI_MAX_PIPE_NUM; i++)
        {
            if (vipipe[i] >= 0 && vipipe[i] < VI_MAX_PIPE_NUM)
            {
                stDevBindPipe.PipeId[stDevBindPipe.u32Num] = vipipe[i];
                stDevBindPipe.u32Num++;
            }
        }
        s32Ret = HI_MPI_VI_SetDevBindPipe(input->devnum, &stDevBindPipe);

        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
                zm_msg_err(" DEV (%d) BindPipe failed(%s)", input->devnum, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        return s32Ret;
    }
    else
    {
        HI_MPI_VI_CloseFd();
        s32Ret = HI_MPI_VI_DisableDev(input->devnum);
        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
                zm_msg_err(" DEV (%d) Disable failed(%s)", input->devnum, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        return s32Ret;
    }
#else
    return OK;
#endif
}

int zpl_vidhal_inputdev_create(zpl_media_video_inputchn_t *input)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 i = 0;
    zpl_int32 s32Ret = 0;
    zpl_int32 vipipe[VI_MAX_PHY_PIPE_NUM] = {-1};
    VI_DEV_BIND_PIPE_S stDevBindPipe = {0};
    VI_DEV_ATTR_S stViDevAttr;

    //zpl_video_resources_get_pipe(vipipe);

    // MIPI Start
    zpl_vidhal_mipi_start(input->inputdev.snsdev, input->inputdev.mipidev, input->inputdev.snstype /*ZPL_SENSOR_TYPE_DEFAULT*/);

    zpl_vidhal_sensor_get_devattr(input->inputdev.snstype, &stViDevAttr);

    s32Ret = HI_MPI_VI_SetDevAttr(input->devnum, &stViDevAttr);

    if (s32Ret != HI_SUCCESS)
    {
        if (ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zm_msg_err(" DEV (%d) SetDevAttr failed(%s)", input->devnum, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VI_EnableDev(input->devnum);

    if (s32Ret != HI_SUCCESS)
    {
        if (ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zm_msg_err(" DEV (%d) Enable failed(%s)", input->devnum, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    for (i = 0; i < VI_MAX_PIPE_NUM; i++)
    {
        stDevBindPipe.PipeId[i] = -1;
        stDevBindPipe.u32Num = 0;
    }
    for (i = 0; i < VI_MAX_PIPE_NUM; i++)
    {
        if (vipipe[i] >= 0 && vipipe[i] < VI_MAX_PIPE_NUM)
        {
            stDevBindPipe.PipeId[stDevBindPipe.u32Num] = vipipe[i];
            stDevBindPipe.u32Num++;
        }
    }
    s32Ret = HI_MPI_VI_SetDevBindPipe(input->devnum, &stDevBindPipe);

    if (s32Ret != HI_SUCCESS)
    {
        if (ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zm_msg_err(" DEV (%d) BindPipe failed(%s)", input->devnum, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return OK;
#endif
}

int zpl_vidhal_inputdev_destroy(zpl_media_video_inputchn_t *input)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    HI_MPI_VI_CloseFd();
    s32Ret = HI_MPI_VI_DisableDev(input->devnum);
    if (s32Ret != HI_SUCCESS)
    {
        if (ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zm_msg_err(" DEV (%d) Disable failed(%s)", input->devnum, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    zpl_vidhal_mipi_stop(input->inputdev.snsdev, input->inputdev.mipidev);

    return OK;
#else
    return ERROR;
#endif
}

/*
SAMPLE_VENC_VI_Init
    SAMPLE_VENC_SYS_Init
        SAMPLE_COMM_SYS_Init
            HI_MPI_VB_SetConfig
            HI_MPI_VB_Init
            HI_MPI_SYS_Init
    SAMPLE_COMM_VI_SetParam
        HI_MPI_SYS_GetVIVPSSMode
        HI_MPI_SYS_SetVIVPSSMode
    SAMPLE_COMM_VI_GetFrameRateBySensor
    HI_MPI_ISP_GetCtrlParam
    HI_MPI_ISP_SetCtrlParam
    SAMPLE_COMM_VI_StartVi
        SAMPLE_COMM_VI_StartMIPI
            SAMPLE_COMM_VI_GetMipiLaneDivideMode
            SAMPLE_COMM_VI_SetMipiHsMode
            SAMPLE_COMM_VI_EnableMipiClock
            SAMPLE_COMM_VI_ResetMipi
            SAMPLE_COMM_VI_EnableSensorClock
            SAMPLE_COMM_VI_ResetSensor
            SAMPLE_COMM_VI_SetMipiAttr
            SAMPLE_COMM_VI_UnresetMipi
            SAMPLE_COMM_VI_UnresetSensor
        SAMPLE_COMM_VI_SetParam
            HI_MPI_SYS_SetVIVPSSMode
        SAMPLE_COMM_VI_CreateVi
            SAMPLE_COMM_VI_CreateSingleVi
                SAMPLE_COMM_VI_StartDev
                    HI_MPI_VI_SetDevAttr
                    HI_MPI_VI_EnableDev
                SAMPLE_COMM_VI_BindPipeDev
                    HI_MPI_VI_SetDevBindPipe
                SAMPLE_COMM_VI_StartViPipe
                    HI_MPI_VI_CreatePipe
                    HI_MPI_VI_StartPipe
                SAMPLE_COMM_VI_StartViChn
                    HI_MPI_VI_SetChnAttr
                    HI_MPI_VI_EnableChn
        SAMPLE_COMM_VI_CreateIsp
            SAMPLE_COMM_VI_StartIsp
                SAMPLE_COMM_ISP_Sensor_Regiter_callback
                SAMPLE_COMM_ISP_BindSns
                SAMPLE_COMM_ISP_Aelib_Callback
                SAMPLE_COMM_ISP_Awblib_Callback
                HI_MPI_ISP_MemInit
                HI_MPI_ISP_SetPubAttr
                HI_MPI_ISP_Init
                SAMPLE_COMM_ISP_Run
SAMPLE_VENC_VPSS_Init
    SAMPLE_COMM_VPSS_Start

SAMPLE_COMM_VI_Bind_VPSS
SAMPLE_COMM_VENC_Start
SAMPLE_COMM_VPSS_Bind_VENC
*/