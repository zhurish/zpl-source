/*
 * zpl_vidhal_isp.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include <zpl_vidhal.h>
#include "zpl_vidhal_isp.h"
#include <sys/prctl.h>
#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"
#endif

#ifdef ZPL_HISIMPP_MODULE

#define MAX_SENSOR_NUM 2
#define ISP_MAX_DEV_NUM 4
/*
~ # cat /dev/logmpp
<4>[   isp] [Func]:ISP_GetFrameInfo [Line]:4441 [Info]:ISP[0] TransBuf doesn't initialized!
<4>[   isp] [Func]:ISP_DRV_NormalRegsCfgSensor [Line]:3056 [Info]:NULL point Normal!
<4>[   isp] [Func]:ISP_DRV_RegConfigSensor [Line]:3163 [Info]:ISP_DRV_NormalRegsCfgSensor failure!
<4>[   isp] [Func]:ISP_DRV_NormalRegsCfgSensor [Line]:3056 [Info]:NULL point Normal!
<4>[   isp] [Func]:ISP_DRV_RegConfigSensor [Line]:3163 [Info]:ISP_DRV_NormalRegsCfgSensor failure!
*/
static pthread_t g_IspPid[ISP_MAX_DEV_NUM] = {0};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX327_2M_30FPS =
    {
        {0, 0, 1920, 1080},
        {1920, 1080},
        30,
        BAYER_RGGB,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX327_MIPI_2M_30FPS_WDR2TO1_LINE =
    {
        {0, 0, 1920, 1080},
        {1920, 1080},
        30,
        BAYER_RGGB,
        WDR_MODE_2To1_LINE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX307_MIPI_2M_30FPS =
    {
        {0, 0, 1920, 1080},
        {1920, 1080},
        30,
        BAYER_RGGB,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX307_MIPI_2M_30FPS_WDR2TO1_LINE =
    {
        {0, 0, 1920, 1080},
        {1920, 1080},
        30,
        BAYER_RGGB,
        WDR_MODE_2To1_LINE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_MN34220_LVDS_2M_30FPS =
    {
        {0, 0, 1920, 1080},
        {1920, 1080},
        30,
        BAYER_GRBG,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX335_MIPI_5M_30FPS =
    {
        {0, 0, 2592, 1944},
        {2592, 1944},
        30,
        BAYER_RGGB,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX335_MIPI_5M_30FPS_WDR2TO1 =
    {
        {0, 0, 2592, 1944},
        {2592, 1944},
        30,
        BAYER_RGGB,
        WDR_MODE_2To1_LINE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX335_MIPI_4M_30FPS =
    {
        {0, 0, 2592, 1536},
        {2592, 1944},
        30,
        BAYER_RGGB,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX335_MIPI_4M_30FPS_WDR2TO1 =
    {
        {0, 0, 2592, 1536},
        {2592, 1944},
        30,
        BAYER_RGGB,
        WDR_MODE_2To1_LINE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX458_MIPI_8M_30FPS =
    {
        {0, 0, 3840, 2160},
        {3840, 2160},
        30,
        BAYER_RGGB,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_SC4210_MIPI_3M_30FPS =
    {
        {0, 0, 2560, 1440},
        {2560, 1440},
        30,
        BAYER_BGGR,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_SC4210_MIPI_3M_30FPS_WDR2TO1 =
    {
        {0, 0, 2560, 1440},
        {2560, 1440},
        30,
        BAYER_BGGR,
        WDR_MODE_2To1_LINE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX458_MIPI_12M_20FPS =
    {
        {0, 0, 4000, 3000},
        {4000, 3000},
        19.98,
        BAYER_RGGB,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX458_MIPI_4M_60FPS =
    {
        {0, 0, 2716, 1524},
        {2716, 1524},
        59.94,
        BAYER_RGGB,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX458_MIPI_4M_40FPS =
    {
        {0, 0, 2716, 1524},
        {2716, 1524},
        40.62,
        BAYER_RGGB,
        WDR_MODE_NONE,
        1,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX458_MIPI_2M_90FPS =
    {
        {0, 0, 1920, 1080},
        {1920, 1080},
        90.11,
        BAYER_RGGB,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX458_MIPI_1M_129FPS =
    {
        {0, 0, 1280, 720},
        {1280, 720},
        128.80,
        BAYER_RGGB,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_OS04B10_MIPI_4M_30FPS =
    {
        {0, 0, 2560, 1440},
        {2560, 1440},
        30,
        BAYER_GBRG,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_OS05A_MIPI_4M_30FPS =
    {
        {0, 0, 2688, 1536},
        {2688, 1944},
        30,
        BAYER_BGGR,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_OS05A_MIPI_4M_30FPS_WDR2TO1 =
    {
        {0, 0, 2688, 1536},
        {2688, 1944},
        30,
        BAYER_BGGR,
        WDR_MODE_2To1_LINE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_OS08A10_MIPI_8M_30FPS =
    {
        {0, 0, 3840, 2160},
        {3840, 2160},
        30,
        BAYER_BGGR,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_GC2053_MIPI_2M_30FPS =
    {
        {0, 0, 1920, 1080},
        {1920, 1080},
        30,
        BAYER_RGGB,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_OV12870_MIPI_1M_240FPS =
    {
        {0, 0, 1280, 720},
        {1280, 720},
        240,
        BAYER_BGGR,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_OV12870_MIPI_2M_120FPS =
    {
        {0, 0, 1920, 1080},
        {1920, 1080},
        120,
        BAYER_BGGR,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_OV12870_MIPI_8M_30FPS =
    {
        {0, 0, 3840, 2160},
        {3840, 2160},
        30,
        BAYER_BGGR,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_OV12870_MIPI_12M_30FPS =
    {
        {0, 0, 4000, 3000},
        {4000, 3000},
        30,
        BAYER_BGGR,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX415_MIPI_8M_30FPS =
    {
        {0, 0, 3840, 2160},
        {3840, 2160},
        30,
        BAYER_GBRG,
        WDR_MODE_NONE,
        0,
};

static ISP_PUB_ATTR_S ISP_PUB_ATTR_IMX415_MIPI_8M_20FPS =
    {
        {0, 0, 3840, 2160},
        {3840, 2160},
        20,
        BAYER_GBRG,
        WDR_MODE_NONE,
        1,
};

int zpl_vidhal_sensor_get_ispattr(ZPL_SENSOR_TYPE_E snstype, void *p)
{
    ISP_PUB_ATTR_S *pstPubAttr = (ISP_PUB_ATTR_S *)p;
    switch (snstype)
    {
    case SONY_IMX327_MIPI_2M_30FPS_12BIT:
    case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX327_2M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
    case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX327_MIPI_2M_30FPS_WDR2TO1_LINE, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX307_MIPI_2M_30FPS_12BIT:
    case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX307_MIPI_2M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
    case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX307_MIPI_2M_30FPS_WDR2TO1_LINE, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX335_MIPI_5M_30FPS_12BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX335_MIPI_5M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX335_MIPI_4M_30FPS_12BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX335_MIPI_4M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX335_MIPI_5M_30FPS_WDR2TO1, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX335_MIPI_4M_30FPS_WDR2TO1, sizeof(ISP_PUB_ATTR_S));
        break;

    case SMART_SC4210_MIPI_3M_30FPS_12BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_SC4210_MIPI_3M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case SMART_SC4210_MIPI_3M_30FPS_10BIT_WDR2TO1:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_SC4210_MIPI_3M_30FPS_WDR2TO1, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX458_MIPI_8M_30FPS_10BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX458_MIPI_8M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX458_MIPI_12M_20FPS_10BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX458_MIPI_12M_20FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX458_MIPI_4M_60FPS_10BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX458_MIPI_4M_60FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX458_MIPI_4M_40FPS_10BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX458_MIPI_4M_40FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX458_MIPI_2M_90FPS_10BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX458_MIPI_2M_90FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX458_MIPI_1M_129FPS_10BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX458_MIPI_1M_129FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case PANASONIC_MN34220_LVDS_2M_30FPS_12BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_MN34220_LVDS_2M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case OMNIVISION_OS04B10_MIPI_4M_30FPS_10BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_OS04B10_MIPI_4M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_OS05A_MIPI_4M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_OS05A_MIPI_4M_30FPS_WDR2TO1, sizeof(ISP_PUB_ATTR_S));
        break;

    case OMNIVISION_OS08A10_MIPI_8M_30FPS_10BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_OS08A10_MIPI_8M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_GC2053_MIPI_2M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_1M_240FPS_10BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_OV12870_MIPI_1M_240FPS, sizeof(ISP_PUB_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_2M_120FPS_10BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_OV12870_MIPI_2M_120FPS, sizeof(ISP_PUB_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_8M_30FPS_10BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_OV12870_MIPI_8M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_12M_30FPS_10BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_OV12870_MIPI_12M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    case SONY_IMX415_MIPI_8M_30FPS_12BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX415_MIPI_8M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;
    case SONY_IMX415_MIPI_8M_20FPS_12BIT:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX415_MIPI_8M_20FPS, sizeof(ISP_PUB_ATTR_S));
        break;

    default:
        memcpy(pstPubAttr, &ISP_PUB_ATTR_IMX327_2M_30FPS, sizeof(ISP_PUB_ATTR_S));
        break;
    }

    return HI_SUCCESS;
}

static ISP_SNS_OBJ_S *zpl_vidhal_isp_get_sendor_obj(ZPL_SENSOR_TYPE_E enSnsType)
{
    zm_msg_debug("================zpl_vidhal_isp_get_sendor_obj==enSnsType=%d\n", enSnsType);
    switch (enSnsType)
    {
#if 1
    case SONY_IMX327_MIPI_2M_30FPS_12BIT:
        return &stSnsImx327Obj;
    case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT:
        return &stSnsImx327_2l_Obj;
#else
    case SONY_IMX327_MIPI_2M_30FPS_12BIT:
    case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
        return &stSnsImx327Obj;

    case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT:
    case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        return &stSnsImx327_2l_Obj;

    case SONY_IMX307_MIPI_2M_30FPS_12BIT:
    case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
        return &stSnsImx307Obj;

    case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
    case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        return &stSnsImx307_2l_Obj;

    case SONY_IMX335_MIPI_5M_30FPS_12BIT:
    case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
    case SONY_IMX335_MIPI_4M_30FPS_12BIT:
    case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
        return &stSnsImx335Obj;

    case SMART_SC4210_MIPI_3M_30FPS_12BIT:
    case SMART_SC4210_MIPI_3M_30FPS_10BIT_WDR2TO1:
        return &stSnsSc4210Obj;

    case SONY_IMX458_MIPI_8M_30FPS_10BIT:
    case SONY_IMX458_MIPI_12M_20FPS_10BIT:
    case SONY_IMX458_MIPI_4M_60FPS_10BIT:
    case SONY_IMX458_MIPI_4M_40FPS_10BIT:
    case SONY_IMX458_MIPI_2M_90FPS_10BIT:
    case SONY_IMX458_MIPI_1M_129FPS_10BIT:
        return &stSnsImx458Obj;

    case PANASONIC_MN34220_LVDS_2M_30FPS_12BIT:
        return &stSnsMn34220Obj;

    case OMNIVISION_OS04B10_MIPI_4M_30FPS_10BIT:
        return &stSnsOs04b10_2lObj;

    case OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT:
    case OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1:
        return &stSnsOs05aObj;

    case OMNIVISION_OS08A10_MIPI_8M_30FPS_10BIT:
        return &stSnsOS08A10Obj;

    case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        return &stSnsGc2053Obj;
    case OMNIVISION_OV12870_MIPI_1M_240FPS_10BIT:
    case OMNIVISION_OV12870_MIPI_2M_120FPS_10BIT:
    case OMNIVISION_OV12870_MIPI_8M_30FPS_10BIT:
    case OMNIVISION_OV12870_MIPI_12M_30FPS_10BIT:
        return &stSnsOv12870Obj;

    case SONY_IMX415_MIPI_8M_30FPS_12BIT:
    case SONY_IMX415_MIPI_8M_20FPS_12BIT:
        return &stSnsImx415Obj;
#endif
    default:
        return HI_NULL;
    }
}

static void *zpl_vidhal_isp_thread(void *param)
{
    HI_S32 s32Ret;
    ISP_DEV IspDev;
    HI_CHAR szThreadName[20];

    IspDev = (ISP_DEV)param;

    snprintf(szThreadName, 20, "ISP%d_RUN", IspDev);
    prctl(PR_SET_NAME, szThreadName, 0, 0, 0);

    zm_msg_debug("ISP Dev %d running\n", IspDev);
    s32Ret = HI_MPI_ISP_Run(IspDev);

    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("HI_MPI_ISP_Run failed with %#x!\n", s32Ret);
    }

    return NULL;
}

/******************************************************************************
* funciton : ISP init
******************************************************************************/

static int zpl_vidhal_isp_Aelib_Callback(ISP_DEV IspDev)
{
    ALG_LIB_S stAeLib;
    memset(&stAeLib, 0, sizeof(ALG_LIB_S));
    stAeLib.s32Id = IspDev;
    strncpy(stAeLib.acLibName, HI_AE_LIB_NAME, strlen(HI_AE_LIB_NAME));
    ZPL_CHECK_RET(HI_MPI_AE_Register(IspDev, &stAeLib), "aelib register call back");
    return HI_SUCCESS;
}

static int zpl_vidhal_isp_Aelib_UnCallback(ISP_DEV IspDev)
{
    ALG_LIB_S stAeLib;
    memset(&stAeLib, 0, sizeof(ALG_LIB_S));
    stAeLib.s32Id = IspDev;
    strncpy(stAeLib.acLibName, HI_AE_LIB_NAME, strlen(HI_AE_LIB_NAME));
    ZPL_CHECK_RET(HI_MPI_AE_UnRegister(IspDev, &stAeLib), "aelib unregister call back");
    return HI_SUCCESS;
}

static int zpl_vidhal_isp_Awblib_Callback(ISP_DEV IspDev)
{
    ALG_LIB_S stAwbLib;
    memset(&stAwbLib, 0, sizeof(ALG_LIB_S));
    stAwbLib.s32Id = IspDev;
    strncpy(stAwbLib.acLibName, HI_AWB_LIB_NAME, strlen(HI_AWB_LIB_NAME));
    ZPL_CHECK_RET(HI_MPI_AWB_Register(IspDev, &stAwbLib), "awblib register call back");
    return HI_SUCCESS;
}

static int zpl_vidhal_isp_Awblib_UnCallback(ISP_DEV IspDev)
{
    ALG_LIB_S stAwbLib;
    memset(&stAwbLib, 0, sizeof(ALG_LIB_S));
    stAwbLib.s32Id = IspDev;
    strncpy(stAwbLib.acLibName, HI_AWB_LIB_NAME, strlen(HI_AWB_LIB_NAME));
    ZPL_CHECK_RET(HI_MPI_AWB_UnRegister(IspDev, &stAwbLib), "awblib unregister call back");
    return HI_SUCCESS;
}

static int zpl_vidhal_isp_sensor_regiter_callback(ISP_DEV IspDev, ZPL_SENSOR_TYPE_E snstype)
{
    ALG_LIB_S stAeLib;
    ALG_LIB_S stAwbLib;
    const ISP_SNS_OBJ_S *pstSnsObj;
    HI_S32 s32Ret = -1;


    if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
        zm_msg_info("============= snstype=%d ispdev=%d\n", snstype, IspDev);
    pstSnsObj = zpl_vidhal_isp_get_sendor_obj(snstype);

    if (HI_NULL == pstSnsObj)
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("snstype %d not exist!\n", snstype);
        return HI_FAILURE;
    }
    memset(&stAeLib, 0, sizeof(ALG_LIB_S));
    memset(&stAwbLib, 0, sizeof(ALG_LIB_S));
    stAeLib.s32Id = IspDev;
    stAwbLib.s32Id = IspDev;
    strncpy(stAeLib.acLibName, HI_AE_LIB_NAME, strlen(HI_AE_LIB_NAME));
    strncpy(stAwbLib.acLibName, HI_AWB_LIB_NAME, strlen(HI_AWB_LIB_NAME));
    //  strncpy(stAfLib.acLibName, HI_AF_LIB_NAME, sizeof(HI_AF_LIB_NAME));

    if (pstSnsObj->pfnRegisterCallback != HI_NULL)
    {
        s32Ret = pstSnsObj->pfnRegisterCallback(IspDev, &stAeLib, &stAwbLib);

        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                zm_msg_err("sensor_register_callback failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("sensor_register_callback failed with HI_NULL!\n");
    }
    return HI_SUCCESS;
}

static int zpl_vidhal_isp_sensor_unregiter_callback(ISP_DEV IspDev, ZPL_SENSOR_TYPE_E snstype)
{
    ALG_LIB_S stAeLib;
    ALG_LIB_S stAwbLib;
    const ISP_SNS_OBJ_S *pstSnsObj;
    HI_S32 s32Ret = -1;
    memset(&stAeLib, 0, sizeof(ALG_LIB_S));
    memset(&stAwbLib, 0, sizeof(ALG_LIB_S));

    pstSnsObj = zpl_vidhal_isp_get_sendor_obj(snstype);

    if (HI_NULL == pstSnsObj)
    {
        return HI_FAILURE;
    }

    stAeLib.s32Id = IspDev;
    stAwbLib.s32Id = IspDev;
    strncpy(stAeLib.acLibName, HI_AE_LIB_NAME, strlen(HI_AE_LIB_NAME));
    strncpy(stAwbLib.acLibName, HI_AWB_LIB_NAME, strlen(HI_AWB_LIB_NAME));
    //   strncpy(stAfLib.acLibName, HI_AF_LIB_NAME, sizeof(HI_AF_LIB_NAME));

    if (pstSnsObj->pfnUnRegisterCallback != HI_NULL)
    {
        s32Ret = pstSnsObj->pfnUnRegisterCallback(IspDev, &stAeLib, &stAwbLib);

        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                zm_msg_err("sensor_unregister_callback failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("sensor_unregister_callback failed with HI_NULL!\n");
    }

    return HI_SUCCESS;
}

/******************************************************************************
* funciton : ISP Run
******************************************************************************/
static int zpl_vidhal_isp_start_hw(ISP_DEV IspDev)
{
    HI_S32 s32Ret = 0;
    pthread_attr_t *pstAttr = NULL;

    s32Ret = pthread_create(&g_IspPid[IspDev], pstAttr, zpl_vidhal_isp_thread, (HI_VOID *)IspDev);

    if (0 != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("create isp running thread failed!, error: %d, %s\r\n", s32Ret, strerror(s32Ret));
        goto out;
    }

out:

    if (NULL != pstAttr)
    {
        pthread_attr_destroy(pstAttr);
    }

    return s32Ret;
}
/******************************************************************************
* funciton : stop ISP, and stop isp thread
******************************************************************************/
static int zpl_vidhal_isp_stop_hw(ISP_DEV IspDev, ZPL_SENSOR_TYPE_E snstype)
{
    if (g_IspPid[IspDev])
    {
        HI_MPI_ISP_Exit(IspDev);
        pthread_join(g_IspPid[IspDev], NULL);
        zpl_vidhal_isp_Awblib_UnCallback(IspDev);
        zpl_vidhal_isp_Aelib_UnCallback(IspDev);
        zpl_vidhal_isp_sensor_unregiter_callback(IspDev, snstype);
        g_IspPid[IspDev] = 0;
    }

    return;
}

static int zpl_vidhal_isp_bind_sensor(ISP_DEV IspDev, ZPL_SENSOR_TYPE_E enSnsType, HI_S8 s8SnsDev)
{
    ISP_SNS_COMMBUS_U uSnsBusInfo;
    ISP_SNS_TYPE_E enBusType;
    const ISP_SNS_OBJ_S *pstSnsObj;
    HI_S32 s32Ret;

    pstSnsObj = zpl_vidhal_isp_get_sendor_obj(enSnsType);

    if (HI_NULL == pstSnsObj)
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("enSnsType %d not exist!\n", enSnsType);
        return HI_FAILURE;
    }

    enBusType = ISP_SNS_I2C_TYPE;

    if (ISP_SNS_I2C_TYPE == enBusType)
    {
        uSnsBusInfo.s8I2cDev = s8SnsDev;
    }
    else
    {
        uSnsBusInfo.s8SspDev.bit4SspDev = s8SnsDev;
        uSnsBusInfo.s8SspDev.bit4SspCs = 0;
    }

    if (HI_NULL != pstSnsObj->pfnSetBusInfo)
    {
        s32Ret = pstSnsObj->pfnSetBusInfo(IspDev, uSnsBusInfo);

        if (s32Ret != HI_SUCCESS)
        {
            if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                zm_msg_err("set sensor bus info failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("not support set sensor bus info!\n");
        return HI_FAILURE;
    }

    return s32Ret;
}
#endif

int zpl_vidhal_isp_start_one(zpl_vidhal_isp_sensor_t *sensor)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 i;
    HI_BOOL bNeedPipe;
    HI_S32 s32Ret = HI_SUCCESS;
    VI_PIPE ViPipe;
    ISP_PUB_ATTR_S stPubAttr;
    VI_PIPE_ATTR_S stPipeAttr;

    zpl_vidhal_sensor_get_pipeattr(sensor->sns_type, &stPipeAttr);
    if (VI_PIPE_BYPASS_BE == stPipeAttr.enPipeBypassMode)
    {
        return HI_SUCCESS;
    }

    ViPipe = sensor->vipipe;

    zpl_vidhal_sensor_get_ispattr(sensor->sns_type, &stPubAttr);
    stPubAttr.enWDRMode = sensor->enWDRMode;

    if (WDR_MODE_NONE == sensor->enWDRMode)
    {
        bNeedPipe = HI_TRUE;
    }
    else
    {
        bNeedPipe = (i > 0) ? HI_FALSE : HI_TRUE;
    }

    if (HI_TRUE != bNeedPipe)
    {
        return HI_SUCCESS;
    }

    s32Ret = zpl_vidhal_isp_sensor_regiter_callback(ViPipe, sensor->sns_type);

    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("register sensor %d to ISP %d failed\n", sensor->sns_type, ViPipe);
        zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
        return HI_FAILURE;
    }

    if (((sensor->bDoublePipe) && (sensor->SnapPipe == ViPipe)) || (sensor->bMultiPipe && i > 0))
    {
        s32Ret = zpl_vidhal_isp_bind_sensor(ViPipe, sensor->sns_type, -1);

        if (HI_SUCCESS != s32Ret)
        {
            if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                zm_msg_err("register sensor %d bus id %d failed\n", sensor->sns_type, sensor->s32BusId);
            zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
            return HI_FAILURE;
        }
    }
    else
    {
        s32Ret = zpl_vidhal_isp_bind_sensor(ViPipe, sensor->sns_type, sensor->s32BusId);

        if (HI_SUCCESS != s32Ret)
        {
            if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                zm_msg_err("register sensor %d bus id %d failed\n", sensor->sns_type, sensor->s32BusId);
            zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
            return HI_FAILURE;
        }
    }
    s32Ret = zpl_vidhal_isp_Aelib_Callback(ViPipe);

    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("SAMPLE_COMM_ISP_Aelib_Callback failed\n");
        zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
        return HI_FAILURE;
    }

    s32Ret = zpl_vidhal_isp_Awblib_Callback(ViPipe);

    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("SAMPLE_COMM_ISP_Awblib_Callback failed\n");
        zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_ISP_MemInit(ViPipe);

    if (s32Ret != HI_SUCCESS)
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("Init Ext memory failed with(pipe=%d) %#x!\n", ViPipe, s32Ret);
        zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_ISP_SetPubAttr(ViPipe, &stPubAttr);
    if (s32Ret != HI_SUCCESS)
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("SetPubAttr failed with %#x!\n", s32Ret);
        zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
        return HI_FAILURE;
    }
#if 0
    ISP_DEHAZE_ATTR_S stDehazeAttr;
    s32Ret = HI_MPI_ISP_GetDehazeAttr(ViPipe, &stDehazeAttr);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("GetDehazeAttr failed with %#x!\n", s32Ret);
        zpl_vidhal_isp_stop_hw(ViPipe, u32SnsId, sensor->sns_type);
        return HI_FAILURE;
    }
    stDehazeAttr.bEnable = HI_TRUE;
    stDehazeAttr.bUserLutEnable = HI_FALSE;;    /* RW;Range:[0,1];0:Auto Lut 1:User Lut */
    stDehazeAttr.enOpType = OP_TYPE_AUTO;
    stDehazeAttr.stManual.u8strength = 128;
    stDehazeAttr.stAuto.u8strength = 128;
    stDehazeAttr.u16TmprfltIncrCoef = 8; /* RW, Range: [0x0, 0x80].filter increase coeffcient. */
    stDehazeAttr.u16TmprfltDecrCoef = 64; /* RW, Range: [0x0, 0x80].filter decrease coeffcient. */

    s32Ret = HI_MPI_ISP_SetDehazeAttr( ViPipe, &stDehazeAttr);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("SetDehazeAttr failed with %#x!\n", s32Ret);
        zpl_vidhal_isp_stop_hw(ViPipe, u32SnsId, sensor->sns_type);
        return HI_FAILURE;
    }
    ISP_MODULE_CTRL_U unModCtrl;
    
    HI_MPI_ISP_GetModuleControl(ViPipe, &unModCtrl);
    unModCtrl.bitBypassDehaze = 1;
    unModCtrl.u64Key |= (1<<5);
    HI_MPI_ISP_SetModuleControl(ViPipe, &unModCtrl);
#endif
    s32Ret = HI_MPI_ISP_Init(ViPipe);
    if (s32Ret != HI_SUCCESS)
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("ISP Init failed with %#x!\n", s32Ret);
        zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
        return HI_FAILURE;
    }

    s32Ret = zpl_vidhal_isp_start_hw(ViPipe);
    if (s32Ret != HI_SUCCESS)
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err("ISP Run failed with %#x!\n", s32Ret);
        zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
        return HI_FAILURE;
    }

    return s32Ret;
#else
    return 0;
#endif
}

int zpl_vidhal_isp_start(zpl_vidhal_isp_sensor_t *sensor)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 i;
    HI_BOOL bNeedPipe;
    HI_S32 s32Ret = HI_SUCCESS;
    VI_PIPE ViPipe;
    ISP_PUB_ATTR_S stPubAttr;
    VI_PIPE_ATTR_S stPipeAttr;

    zpl_vidhal_sensor_get_pipeattr(sensor->sns_type, &stPipeAttr);
    if (VI_PIPE_BYPASS_BE == stPipeAttr.enPipeBypassMode)
    {
        return HI_SUCCESS;
    }

    for (i = 0; i < WDR_MAX_PIPE_NUM; i++)
    {
        if (sensor->pipe[i] >= 0 && sensor->pipe[i] < VI_MAX_PIPE_NUM)
        {
            ViPipe = sensor->pipe[i];

            zpl_vidhal_sensor_get_ispattr(sensor->sns_type, &stPubAttr);
            stPubAttr.enWDRMode = sensor->enWDRMode;

            if (WDR_MODE_NONE == sensor->enWDRMode)
            {
                bNeedPipe = HI_TRUE;
            }
            else
            {
                bNeedPipe = (i > 0) ? HI_FALSE : HI_TRUE;
            }

            if (HI_TRUE != bNeedPipe)
            {
                continue;
            }

            s32Ret = zpl_vidhal_isp_sensor_regiter_callback(ViPipe, sensor->sns_type);

            if (HI_SUCCESS != s32Ret)
            {
                if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("register sensor %d to ISP %d failed\n", sensor->sns_type, ViPipe);
                zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
                return HI_FAILURE;
            }

            if (((sensor->bDoublePipe) && (sensor->SnapPipe == ViPipe)) || (sensor->bMultiPipe && i > 0))
            {
                s32Ret = zpl_vidhal_isp_bind_sensor(ViPipe, sensor->sns_type, -1);

                if (HI_SUCCESS != s32Ret)
                {
                    if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("register sensor %d bus id %d failed\n", sensor->sns_type, sensor->s32BusId);
                    zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
                    return HI_FAILURE;
                }
            }
            else
            {
                s32Ret = zpl_vidhal_isp_bind_sensor(ViPipe, sensor->sns_type, sensor->s32BusId);

                if (HI_SUCCESS != s32Ret)
                {
                    if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                        zm_msg_err("register sensor %d bus id %d failed\n", sensor->sns_type, sensor->s32BusId);
                    zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
                    return HI_FAILURE;
                }
            }
            s32Ret = zpl_vidhal_isp_Aelib_Callback(ViPipe);

            if (HI_SUCCESS != s32Ret)
            {
                if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("SAMPLE_COMM_ISP_Aelib_Callback failed\n");
                zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
                return HI_FAILURE;
            }

            s32Ret = zpl_vidhal_isp_Awblib_Callback(ViPipe);

            if (HI_SUCCESS != s32Ret)
            {
                if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("SAMPLE_COMM_ISP_Awblib_Callback failed\n");
                zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
                return HI_FAILURE;
            }

            s32Ret = HI_MPI_ISP_MemInit(ViPipe);

            if (s32Ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("Init Ext memory failed with(pipe=%d) %#x!\n", ViPipe, s32Ret);
                zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
                return HI_FAILURE;
            }

            s32Ret = HI_MPI_ISP_SetPubAttr(ViPipe, &stPubAttr);

            if (s32Ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("SetPubAttr failed with %#x!\n", s32Ret);
                zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
                return HI_FAILURE;
            }

            s32Ret = HI_MPI_ISP_Init(ViPipe);

            if (s32Ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("ISP Init failed with %#x!\n", s32Ret);
                zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
                return HI_FAILURE;
            }

            s32Ret = zpl_vidhal_isp_start_hw(ViPipe);

            if (s32Ret != HI_SUCCESS)
            {
                if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
                    zm_msg_err("ISP Run failed with %#x!\n", s32Ret);
                zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
                return HI_FAILURE;
            }
        }
    }
    return s32Ret;
#else
    return 0;
#endif
}

int zpl_vidhal_isp_stop_one(zpl_vidhal_isp_sensor_t *sensor)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 i;
    HI_BOOL bNeedPipe;
    VI_PIPE ViPipe;

    ViPipe = sensor->vipipe;

    if (WDR_MODE_NONE == sensor->enWDRMode)
    {
        bNeedPipe = HI_TRUE;
    }
    else
    {
        bNeedPipe = (i > 0) ? HI_FALSE : HI_TRUE;
    }
    zpl_vidhal_isp_sensor_unregiter_callback(ViPipe, sensor->sns_type);
    if (HI_TRUE != bNeedPipe)
    {
        return HI_SUCCESS;
    }

    zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);

    return HI_SUCCESS;
#else
    return 0;
#endif
}

int zpl_vidhal_isp_stop(zpl_vidhal_isp_sensor_t *sensor)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 i;
    HI_BOOL bNeedPipe;
    VI_PIPE ViPipe;

    for (i = 0; i < WDR_MAX_PIPE_NUM; i++)
    {
        if (sensor->pipe[i] >= 0 && sensor->pipe[i] < VI_MAX_PIPE_NUM)
        {
            ViPipe = sensor->pipe[i];

            if (WDR_MODE_NONE == sensor->enWDRMode)
            {
                bNeedPipe = HI_TRUE;
            }
            else
            {
                bNeedPipe = (i > 0) ? HI_FALSE : HI_TRUE;
            }
            zpl_vidhal_isp_sensor_unregiter_callback(ViPipe, sensor->sns_type);
            if (HI_TRUE != bNeedPipe)
            {
                continue;
            }

            zpl_vidhal_isp_stop_hw(ViPipe, sensor->sns_type);
        }
    }

    return HI_SUCCESS;
#else
    return 0;
#endif
}

int zpl_vidhal_isp_setparam(zpl_int32 pipe, zpl_uint32 frameRate)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;
    ISP_CTRL_PARAM_S stIspCtrlParam;
    s32Ret = HI_MPI_ISP_GetCtrlParam(pipe, &stIspCtrlParam);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err(" HI_MPI_ISP_GetCtrlParam  failed(%s)", zpl_syshal_strerror(s32Ret));
        return s32Ret;
    }
    HI_U32 u32FrameRate = frameRate ? frameRate : 30U;
    stIspCtrlParam.u32StatIntvl = u32FrameRate / 30;

    s32Ret = HI_MPI_ISP_SetCtrlParam(pipe, &stIspCtrlParam);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ISP, EVENT) && ZPL_MEDIA_DEBUG(ISP, HARDWARE))
            zm_msg_err(" HI_MPI_ISP_SetCtrlParam failed(%s)", zpl_syshal_strerror(s32Ret));
        return s32Ret;
    }
    return HI_SUCCESS;
#else
    return 0;
#endif
}
