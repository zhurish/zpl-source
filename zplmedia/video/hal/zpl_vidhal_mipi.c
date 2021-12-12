/*
 * zpl_vidhal_mipi.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_mipi.h"
#include "zpl_vidhal_sensor.h"
#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"


#define MIPI_DEV_NODE "/dev/hi_mipi"
#define MAX_FRAME_WIDTH 8192

static int zpl_vidhal_sensor_reset(zpl_int32 snsdev)
{
    zpl_int32 fd;
    zpl_int32 s32Ret;
    sns_rst_source_t SnsDev = snsdev;
    fd = open(MIPI_DEV_NODE, O_RDWR);
    if (fd < 0)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("open hi_mipi dev failed\n");
        return -1;
    }
    s32Ret = ioctl(fd, HI_MIPI_RESET_SENSOR, &SnsDev);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("HI_MIPI_SET_HS_MODE failed\n");
    }
    close(fd);
    return s32Ret;
}

static int zpl_vidhal_sensor_unreset(zpl_int32 snsdev)
{
    zpl_int32 s32Ret = HI_SUCCESS;
    zpl_int32 fd;
    sns_rst_source_t SnsDev = snsdev;
    fd = open(MIPI_DEV_NODE, O_RDWR);
    if (fd < 0)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }
    s32Ret = ioctl(fd, HI_MIPI_UNRESET_SENSOR, &SnsDev);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("HI_MIPI_UNRESET_SENSOR failed\n");
        goto EXIT;
    }
EXIT:
    close(fd);
    return s32Ret;
}

static int zpl_vidhal_sensor_clock(zpl_int32 snsdev, zpl_bool enable)
{
    zpl_int32 s32Ret = HI_SUCCESS;
    zpl_int32 fd;
    sns_rst_source_t SnsDev = snsdev;
    fd = open(MIPI_DEV_NODE, O_RDWR);
    if (fd < 0)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }
    s32Ret = ioctl(fd, enable ? HI_MIPI_ENABLE_SENSOR_CLOCK : HI_MIPI_DISABLE_SENSOR_CLOCK, &SnsDev);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("HI_MIPI_ENABLE_SENSOR_CLOCK failed\n");
        goto EXIT;
    }
EXIT:
    close(fd);
    return s32Ret;
}

static int zpl_vidhal_mipi_hsmode_set(lane_divide_mode_t enHsMode)
{
    zpl_int32 fd;
    zpl_int32 s32Ret;
    fd = open(MIPI_DEV_NODE, O_RDWR);
    if (fd < 0)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("open hi_mipi dev failed\n");
        return -1;
    }
    s32Ret = ioctl(fd, HI_MIPI_SET_HS_MODE, &enHsMode);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("HI_MIPI_SET_HS_MODE failed\n");
    }
    close(fd);
    return s32Ret;
}

static int zpl_vidhal_mipi_clock(zpl_int32 devno, zpl_bool enable)
{
    zpl_int32 s32Ret = HI_SUCCESS;
    zpl_int32 fd;
    combo_dev_t           MipiDev = devno;
    fd = open(MIPI_DEV_NODE, O_RDWR);
    if (fd < 0)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }
    s32Ret = ioctl(fd, enable ? HI_MIPI_ENABLE_MIPI_CLOCK : HI_MIPI_DISABLE_MIPI_CLOCK, &MipiDev);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("MIPI_ENABLE_CLOCK %d failed\n", MipiDev);
        goto EXIT;
    }
EXIT:
    close(fd);

    return s32Ret;
}

static int zpl_vidhal_mipi_reset(zpl_int32 devno)
{
    zpl_int32 fd;
    zpl_int32 s32Ret;
    combo_dev_t           MipiDev = devno;
    fd = open(MIPI_DEV_NODE, O_RDWR);
    if (fd < 0)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("open hi_mipi dev failed\n");
        return -1;
    }
    s32Ret = ioctl(fd, HI_MIPI_RESET_MIPI, &MipiDev);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("HI_MIPI_SET_HS_MODE failed\n");
    }
    close(fd);
    return s32Ret;
}

static int zpl_vidhal_mipi_unreset(zpl_int32 devno)
{
    zpl_int32 s32Ret = HI_SUCCESS;
    zpl_int32 fd;
    combo_dev_t           MipiDev = devno;
    fd = open(MIPI_DEV_NODE, O_RDWR);
    if (fd < 0)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }
    s32Ret = ioctl(fd, HI_MIPI_UNRESET_MIPI, &MipiDev);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("HI_MIPI_UNRESET_MIPI failed\n");
        goto EXIT;
    }
EXIT:
    close(fd);
    return s32Ret;
}


static int zpl_vidhal_mipi_set_attr(zpl_int32 devno, combo_dev_attr_t *stcomboDevAttr)
{
    zpl_int32 s32Ret = HI_SUCCESS;
    zpl_int32 fd;
    combo_dev_t           MipiDev = devno;
    fd = open(MIPI_DEV_NODE, O_RDWR);
    if (fd < 0)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }
    stcomboDevAttr->devno = MipiDev;
    s32Ret = ioctl(fd, HI_MIPI_SET_DEV_ATTR, stcomboDevAttr);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("HI_MIPI_UNRESET_MIPI failed\n");
        goto EXIT;
    }
EXIT:
    close(fd);
    return s32Ret;
}
#endif


static zpl_uint32 mipi_Lane_divide_mode(ZPL_SENSOR_TYPE_E snstype)
{
#ifdef ZPL_HISIMPP_MODULE  
    lane_divide_mode_t lane_divide_mode;

    if ((SONY_IMX327_2L_MIPI_2M_30FPS_12BIT == snstype)
        || (SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1 == snstype))
    {
        lane_divide_mode = LANE_DIVIDE_MODE_1;
    }
    else
    {
        lane_divide_mode = LANE_DIVIDE_MODE_0;
    }
    return (zpl_uint32)lane_divide_mode;
#else
    return 0;
#endif
}
/*****************************************************************************
* function : init mipi
*****************************************************************************/
int zpl_vidhal_mipi_start(zpl_int32 snsdev, zpl_int32 mipmdev, ZPL_SENSOR_TYPE_E snstype)
{
#ifdef ZPL_HISIMPP_MODULE    
    zpl_int32 s32Ret = HI_SUCCESS;
    combo_dev_attr_t stcomboDevAttr;
    lane_divide_mode_t lane_divide_mode = mipi_Lane_divide_mode(snstype);

    zpl_vidhal_sensor_get_comboattr(snstype, mipmdev, &stcomboDevAttr);


    s32Ret = zpl_vidhal_mipi_hsmode_set(lane_divide_mode);

    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("SAMPLE_COMM_VI_SetMipiHsMode failed!\n");

        return HI_FAILURE;
    }

    s32Ret = zpl_vidhal_mipi_clock(mipmdev, zpl_true);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("SAMPLE_COMM_VI_EnableMipiClock failed!\n");

        return HI_FAILURE;
    }

    s32Ret = zpl_vidhal_mipi_reset(mipmdev);

    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("SAMPLE_COMM_VI_ResetMipi failed!\n");

        return HI_FAILURE;
    }

    s32Ret = zpl_vidhal_sensor_clock(snsdev, zpl_true);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("SAMPLE_COMM_VI_EnableSensorClock failed!\n");

        return HI_FAILURE;
    }

    s32Ret = zpl_vidhal_sensor_reset(snsdev);

    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("SAMPLE_COMM_VI_ResetSensor failed!\n");

        return HI_FAILURE;
    }

    s32Ret = zpl_vidhal_mipi_set_attr(mipmdev, &stcomboDevAttr);

    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("SAMPLE_COMM_VI_SetMipiAttr failed!\n");

        return HI_FAILURE;
    }

    s32Ret = zpl_vidhal_mipi_unreset(mipmdev);

    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("SAMPLE_COMM_VI_UnresetMipi failed!\n");

        return HI_FAILURE;
    }

    s32Ret = zpl_vidhal_sensor_unreset(snsdev);

    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("SAMPLE_COMM_VI_UnresetSensor failed!\n");

        return HI_FAILURE;
    }

    return HI_SUCCESS;
    #else
    return ERROR;
    #endif
}

int zpl_vidhal_mipi_stop(zpl_int32 snsdev, zpl_int32 mipmdev)
{
    #ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = HI_SUCCESS;

    s32Ret = zpl_vidhal_sensor_reset(snsdev);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("SAMPLE_COMM_VI_ResetSensor failed!\n");

        return HI_FAILURE;
    }

    s32Ret = zpl_vidhal_sensor_clock(snsdev, zpl_false);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("SAMPLE_COMM_VI_DisableSensorClock failed!\n");

        return HI_FAILURE;
    }

    s32Ret = zpl_vidhal_mipi_reset(mipmdev);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("SAMPLE_COMM_VI_ResetMipi failed!\n");

        return HI_FAILURE;
    }

    s32Ret = zpl_vidhal_mipi_clock(mipmdev, zpl_false);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(DEVICE, EVENT) && ZPL_MEDIA_DEBUG(DEVICE, HARDWARE))
            zpl_media_debugmsg_err("SAMPLE_COMM_VI_DisableMipiClock failed!\n");

        return HI_FAILURE;
    }

    return HI_SUCCESS;
    #else
    return ERROR;
    #endif
}
