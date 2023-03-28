/*
 * zpl_vidhal_hdmi.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include <zpl_vidhal.h>
#include "zpl_vidhal_hdmi.h"


#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"



/*******************************
* GLOBAL vars for mipi_tx
*******************************/

combo_dev_cfg_t MIPI_TX_720X576_50_CONFIG =
{
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .output_mode = OUTPUT_MODE_DSI_VIDEO,
    .output_format = OUT_FORMAT_RGB_24_BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .vid_pkt_size     = 720,
        .vid_hsa_pixels   = 64,
        .vid_hbp_pixels   = 68,
        .vid_hline_pixels = 864,
        .vid_vsa_lines    = 5,
        .vid_vbp_lines    = 39,
        .vid_vfp_lines    = 5,
        .vid_active_lines = 576,
        .edpi_cmd_size    = 0,
    },
    .phy_data_rate = 459,
    .pixel_clk = 27000,
};


combo_dev_cfg_t MIPI_TX_1280X720_60_CONFIG =
{
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .output_mode = OUTPUT_MODE_DSI_VIDEO,
    .output_format = OUT_FORMAT_RGB_24_BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .vid_pkt_size     = 1280,
        .vid_hsa_pixels   = 40,
        .vid_hbp_pixels   = 220,
        .vid_hline_pixels = 1650,
        .vid_vsa_lines    = 5,
        .vid_vbp_lines    = 20,
        .vid_vfp_lines    = 5,
        .vid_active_lines = 720,
        .edpi_cmd_size    = 0,
    },
    .phy_data_rate = 459,
    .pixel_clk = 74250,
};

combo_dev_cfg_t MIPI_TX_1024X768_60_CONFIG =
{
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .output_mode = OUTPUT_MODE_DSI_VIDEO,
    .output_format = OUT_FORMAT_RGB_24_BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .vid_pkt_size     = 1024,
        .vid_hsa_pixels   = 136,
        .vid_hbp_pixels   = 160,
        .vid_hline_pixels = 1344,
        .vid_vsa_lines    = 6,
        .vid_vbp_lines    = 29,
        .vid_vfp_lines    = 3,
        .vid_active_lines = 768,
        .edpi_cmd_size    = 0,
    },
    .phy_data_rate = 495,//486
    .pixel_clk = 65000,
};

combo_dev_cfg_t MIPI_TX_1280x1024_60_CONFIG =
{
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .output_mode = OUTPUT_MODE_DSI_VIDEO,
    .output_format = OUT_FORMAT_RGB_24_BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .vid_pkt_size     = 1280,
        .vid_hsa_pixels   = 112,
        .vid_hbp_pixels   = 248,
        .vid_hline_pixels = 1688,
        .vid_vsa_lines    = 3,
        .vid_vbp_lines    = 38,
        .vid_vfp_lines    = 1,
        .vid_active_lines = 1024,
        .edpi_cmd_size    = 0,
    },
    .phy_data_rate = 495,//486
    .pixel_clk = 108000,
};


combo_dev_cfg_t MIPI_TX_1920X1080_60_CONFIG =
{
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .output_mode = OUTPUT_MODE_DSI_VIDEO,
    .output_format = OUT_FORMAT_RGB_24_BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .vid_pkt_size = 1920,
        .vid_hsa_pixels = 44,
        .vid_hbp_pixels = 148,
        .vid_hline_pixels = 2200,
        .vid_vsa_lines = 5,
        .vid_vbp_lines = 36,
        .vid_vfp_lines = 4,
        .vid_active_lines = 1080,
        .edpi_cmd_size = 0,
    },
    .phy_data_rate = 945,
    .pixel_clk = 148500,
};

combo_dev_cfg_t MIPI_TX_720X1280_60_CONFIG =
{
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .output_mode = OUTPUT_MODE_DSI_VIDEO,
    .output_format = OUT_FORMAT_RGB_24_BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .vid_pkt_size     = 720, // hact
        .vid_hsa_pixels   = 24,  // hsa
        .vid_hbp_pixels   = 99,  // hbp
        .vid_hline_pixels = 943, // hact + hsa + hbp + hfp
        .vid_vsa_lines    = 4,   // vsa
        .vid_vbp_lines    = 20,  // vbp
        .vid_vfp_lines    = 8,   // vfp
        .vid_active_lines = 1280,// vact
        .edpi_cmd_size    = 0,
    },
    .phy_data_rate = 459,
    .pixel_clk = 74250,
};

combo_dev_cfg_t MIPI_TX_1080X1920_60_CONFIG =
{
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .output_mode = OUTPUT_MODE_DSI_VIDEO,
    .output_format = OUT_FORMAT_RGB_24_BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .vid_pkt_size     = 1080,
        .vid_hsa_pixels   = 8,
        .vid_hbp_pixels   = 20,
        .vid_hline_pixels = 1238,
        .vid_vsa_lines    = 10,
        .vid_vbp_lines    = 26,
        .vid_vfp_lines    = 16,
        .vid_active_lines = 1920,
        .edpi_cmd_size = 0,
    },
    .phy_data_rate = 945,
    .pixel_clk = 148500,
};



int zpl_vidhal_hdmi_getwh(VO_INTF_SYNC_E enIntfSync, zpl_uint32* pu32W, zpl_uint32* pu32H, zpl_uint32* pu32Frm)
{
    switch (enIntfSync)
    {
        case VO_OUTPUT_PAL       :
            *pu32W = 720;
            *pu32H = 576;
            *pu32Frm = 25;
            break;
        case VO_OUTPUT_NTSC      :
            *pu32W = 720;
            *pu32H = 480;
            *pu32Frm = 30;
            break;
        case VO_OUTPUT_1080P24   :
            *pu32W = 1920;
            *pu32H = 1080;
            *pu32Frm = 24;
            break;
        case VO_OUTPUT_1080P25   :
            *pu32W = 1920;
            *pu32H = 1080;
            *pu32Frm = 25;
            break;
        case VO_OUTPUT_1080P30   :
            *pu32W = 1920;
            *pu32H = 1080;
            *pu32Frm = 30;
            break;
        case VO_OUTPUT_720P50    :
            *pu32W = 1280;
            *pu32H = 720;
            *pu32Frm = 50;
            break;
        case VO_OUTPUT_720P60    :
            *pu32W = 1280;
            *pu32H = 720;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_1080I50   :
            *pu32W = 1920;
            *pu32H = 1080;
            *pu32Frm = 50;
            break;
        case VO_OUTPUT_1080I60   :
            *pu32W = 1920;
            *pu32H = 1080;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_1080P50   :
            *pu32W = 1920;
            *pu32H = 1080;
            *pu32Frm = 50;
            break;
        case VO_OUTPUT_1080P60   :
            *pu32W = 1920;
            *pu32H = 1080;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_576P50    :
            *pu32W = 720;
            *pu32H = 576;
            *pu32Frm = 50;
            break;
        case VO_OUTPUT_480P60    :
            *pu32W = 720;
            *pu32H = 480;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_800x600_60:
            *pu32W = 800;
            *pu32H = 600;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_1024x768_60:
            *pu32W = 1024;
            *pu32H = 768;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_1280x1024_60:
            *pu32W = 1280;
            *pu32H = 1024;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_1366x768_60:
            *pu32W = 1366;
            *pu32H = 768;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_1440x900_60:
            *pu32W = 1440;
            *pu32H = 900;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_1280x800_60:
            *pu32W = 1280;
            *pu32H = 800;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_1600x1200_60:
            *pu32W = 1600;
            *pu32H = 1200;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_1680x1050_60:
            *pu32W = 1680;
            *pu32H = 1050;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_1920x1200_60:
            *pu32W = 1920;
            *pu32H = 1200;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_640x480_60:
            *pu32W = 640;
            *pu32H = 480;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_960H_PAL:
            *pu32W = 960;
            *pu32H = 576;
            *pu32Frm = 50;
            break;
        case VO_OUTPUT_960H_NTSC:
            *pu32W = 960;
            *pu32H = 480;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_1920x2160_30:
            *pu32W = 1920;
            *pu32H = 2160;
            *pu32Frm = 30;
            break;
        case VO_OUTPUT_2560x1440_30:
            *pu32W = 2560;
            *pu32H = 1440;
            *pu32Frm = 30;
            break;
        case VO_OUTPUT_2560x1600_60:
            *pu32W = 2560;
            *pu32H = 1600;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_3840x2160_30    :
            *pu32W = 3840;
            *pu32H = 2160;
            *pu32Frm = 30;
            break;
        case VO_OUTPUT_3840x2160_60    :
            *pu32W = 3840;
            *pu32H = 2160;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_320x240_60    :
            *pu32W = 320;
            *pu32H = 240;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_320x240_50    :
            *pu32W = 320;
            *pu32H = 240;
            *pu32Frm = 50;
            break;
        case VO_OUTPUT_240x320_50    :
            *pu32W = 240;
            *pu32H = 320;
            *pu32Frm = 50;
            break;
        case VO_OUTPUT_240x320_60    :
            *pu32W = 240;
            *pu32H = 320;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_800x600_50    :
            *pu32W = 800;
            *pu32H = 600;
            *pu32Frm = 50;
            break;
        case VO_OUTPUT_720x1280_60    :
            *pu32W = 720;
            *pu32H = 1280;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_1080x1920_60    :
            *pu32W = 1080;
            *pu32H = 1920;
            *pu32Frm = 60;
            break;
        case VO_OUTPUT_7680x4320_30    :
            *pu32W = 7680;
            *pu32H = 4320;
            *pu32Frm = 30;
            break;
        case VO_OUTPUT_USER    :
            *pu32W = 720;
            *pu32H = 576;
            *pu32Frm = 25;
            break;
        default:
            if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
                zm_msg_err("vo enIntfSync %d not support!\n", enIntfSync);
            return HI_FAILURE;
    }


    return HI_SUCCESS;
}


int zpl_vidhal_hdmi_dev_start(zpl_int32 VoDev, void* pstPubAttr)
{
    zpl_int32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MPI_VO_SetPubAttr(VoDev, pstPubAttr);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
            zm_msg_err("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_Enable(VoDev);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
            zm_msg_err("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

int zpl_vidhal_hdmi_dev_stop(zpl_int32 VoDev)
{
    zpl_int32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MPI_VO_Disable(VoDev);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
            zm_msg_err("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}
int zpl_vidhal_hdmi_layer_start(zpl_int32 VoLayer, const void* pstLayerAttr)
{
    zpl_int32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, pstLayerAttr);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
            zm_msg_err("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
            zm_msg_err("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

int zpl_vidhal_hdmi_layer_stop(zpl_int32 VoLayer)
{
    zpl_int32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MPI_VO_DisableVideoLayer(VoLayer);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
            zm_msg_err("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

int zpl_vidhal_hdmi_channel_start(zpl_int32 VoLayer, ZPL_HDMI_VO_MODE_E enMode)
{
    zpl_int32 i;
    zpl_int32 s32Ret    = HI_SUCCESS;
    zpl_uint32 u32WndNum = 0;
    zpl_uint32 u32Square = 0;
    zpl_uint32 u32Row    = 0;
    zpl_uint32 u32Col    = 0;
    zpl_uint32 u32Width  = 0;
    zpl_uint32 u32Height = 0;
    VO_CHN_ATTR_S         stChnAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;

    switch (enMode)
    {
        case VO_MODE_1MUX:
            u32WndNum = 1;
            u32Square = 1;
            break;
        case VO_MODE_2MUX:
            u32WndNum = 2;
            u32Square = 2;
            break;
        case VO_MODE_4MUX:
            u32WndNum = 4;
            u32Square = 2;
            break;
        case VO_MODE_8MUX:
            u32WndNum = 8;
            u32Square = 3;
            break;
        case VO_MODE_9MUX:
            u32WndNum = 9;
            u32Square = 3;
            break;
        case VO_MODE_16MUX:
            u32WndNum = 16;
            u32Square = 4;
            break;
        case VO_MODE_25MUX:
            u32WndNum = 25;
            u32Square = 5;
            break;
        case VO_MODE_36MUX:
            u32WndNum = 36;
            u32Square = 6;
            break;
        case VO_MODE_49MUX:
            u32WndNum = 49;
            u32Square = 7;
            break;
        case VO_MODE_64MUX:
            u32WndNum = 64;
            u32Square = 8;
            break;
        case VO_MODE_2X4:
            u32WndNum = 8;
            u32Square = 3;
            u32Row    = 4;
            u32Col    = 2;
            break;
        default:
            if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
            zm_msg_err("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
            zm_msg_err("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    u32Width  = stLayerAttr.stImageSize.u32Width;
    u32Height = stLayerAttr.stImageSize.u32Height;
    if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
        zm_msg_debug("u32Width:%d, u32Height:%d, u32Square:%d\n", u32Width, u32Height, u32Square);
    for (i = 0; i < u32WndNum; i++)
    {
        if( enMode == VO_MODE_1MUX  ||
            enMode == VO_MODE_2MUX  ||
            enMode == VO_MODE_4MUX  ||
            enMode == VO_MODE_8MUX  ||
            enMode == VO_MODE_9MUX  ||
            enMode == VO_MODE_16MUX ||
            enMode == VO_MODE_25MUX ||
            enMode == VO_MODE_36MUX ||
            enMode == VO_MODE_49MUX ||
            enMode == VO_MODE_64MUX )
        {
            stChnAttr.stRect.s32X       = ALIGN_DOWN((u32Width / u32Square) * (i % u32Square), 2);
            stChnAttr.stRect.s32Y       = ALIGN_DOWN((u32Height / u32Square) * (i / u32Square), 2);
            stChnAttr.stRect.u32Width   = ALIGN_DOWN(u32Width / u32Square, 2);
            stChnAttr.stRect.u32Height  = ALIGN_DOWN(u32Height / u32Square, 2);
            stChnAttr.u32Priority       = 0;
            stChnAttr.bDeflicker        = HI_FALSE;
        }
        else if(enMode == VO_MODE_2X4)
        {
            stChnAttr.stRect.s32X       = ALIGN_DOWN((u32Width / u32Col) * (i % u32Col), 2);
            stChnAttr.stRect.s32Y       = ALIGN_DOWN((u32Height / u32Row) * (i / u32Col), 2);
            stChnAttr.stRect.u32Width   = ALIGN_DOWN(u32Width / u32Col, 2);
            stChnAttr.stRect.u32Height  = ALIGN_DOWN(u32Height / u32Row, 2);
            stChnAttr.u32Priority       = 0;
            stChnAttr.bDeflicker        = HI_FALSE;
        }

        s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, i, &stChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
                zm_msg_err("%s(%d):failed with %#x!\n", \
                   __FUNCTION__, __LINE__,  s32Ret);
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VO_EnableChn(VoLayer, i);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
                zm_msg_err("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

int zpl_vidhal_hdmi_channel_stop(zpl_int32 VoLayer, ZPL_HDMI_VO_MODE_E enMode)
{
    zpl_int32 i;
    zpl_int32 s32Ret    = HI_SUCCESS;
    zpl_uint32 u32WndNum = 0;

    switch (enMode)
    {
        case VO_MODE_1MUX:
        {
            u32WndNum = 1;
            break;
        }
        case VO_MODE_2MUX:
        {
            u32WndNum = 2;
            break;
        }
        case VO_MODE_4MUX:
        {
            u32WndNum = 4;
            break;
        }
        case VO_MODE_8MUX:
        {
            u32WndNum = 8;
            break;
        }
        default:
            if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
                zm_msg_err("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
    }


    for (i = 0; i < u32WndNum; i++)
    {
        s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
        if (s32Ret != HI_SUCCESS)
        {
            if(ZPL_MEDIA_DEBUG(HDMI, EVENT) && ZPL_MEDIA_DEBUG(HDMI, HARDWARE))
                zm_msg_err("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return s32Ret;
}


int zpl_vidhal_hdmi_HdmiConvertSync(VO_INTF_SYNC_E enIntfSync,
    HI_HDMI_VIDEO_FMT_E *penVideoFmt)
{
    switch (enIntfSync)
    {
        case VO_OUTPUT_PAL:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_PAL;
            break;
        case VO_OUTPUT_NTSC:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_NTSC;
            break;
        case VO_OUTPUT_1080P24:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_24;
            break;
        case VO_OUTPUT_1080P25:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_25;
            break;
        case VO_OUTPUT_1080P30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_30;
            break;
        case VO_OUTPUT_720P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_720P_50;
            break;
        case VO_OUTPUT_720P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_720P_60;
            break;
        case VO_OUTPUT_1080I50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_50;
            break;
        case VO_OUTPUT_1080I60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_60;
            break;
        case VO_OUTPUT_1080P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_50;
            break;
        case VO_OUTPUT_1080P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
            break;
        case VO_OUTPUT_576P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_576P_50;
            break;
        case VO_OUTPUT_480P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_480P_60;
            break;
        case VO_OUTPUT_800x600_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_800X600_60;
            break;
        case VO_OUTPUT_1024x768_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1024X768_60;
            break;
        case VO_OUTPUT_1280x1024_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X1024_60;
            break;
        case VO_OUTPUT_1366x768_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1366X768_60;
            break;
        case VO_OUTPUT_1440x900_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1440X900_60;
            break;
        case VO_OUTPUT_1280x800_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X800_60;
            break;
        case VO_OUTPUT_1920x2160_30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1920x2160_30;
            break;
        case VO_OUTPUT_1600x1200_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1600X1200_60;
            break;
        case VO_OUTPUT_1920x1200_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1920X1200_60;
            break;
        case VO_OUTPUT_2560x1440_30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_2560x1440_30;
            break;
        case VO_OUTPUT_2560x1600_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_2560x1600_60;
            break;
        case VO_OUTPUT_3840x2160_30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_30;
            break;
        case VO_OUTPUT_3840x2160_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_60;
        break;
        default:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
            break;
    }

    return;
}


#define CHECK_RET(express,name)\
    do{\
        HI_S32 Ret;\
        Ret = express;\
        if (HI_SUCCESS != Ret)\
        {\
            printf("\033[0;31m%s failed at %s: LINE: %d with %#x!\033[0;39m\n", name, __FUNCTION__, __LINE__, Ret);\
            return Ret;\
        }\
    }while(0)
int zpl_vidhal_hdmi_HdmiStart(VO_INTF_SYNC_E enIntfSync)
{
    HI_HDMI_ATTR_S      stAttr;
    HI_HDMI_VIDEO_FMT_E enVideoFmt;
    HI_HDMI_ID_E        enHdmiId    = HI_HDMI_ID_0;

    zpl_vidhal_hdmi_HdmiConvertSync(enIntfSync, &enVideoFmt);

    CHECK_RET(HI_MPI_HDMI_Init(), "HI_MPI_HDMI_Init");
    CHECK_RET(HI_MPI_HDMI_Open(enHdmiId), "HI_MPI_HDMI_Open");
    CHECK_RET(HI_MPI_HDMI_GetAttr(enHdmiId, &stAttr), "HI_MPI_HDMI_GetAttr");
    stAttr.bEnableHdmi           = HI_TRUE;
    stAttr.bEnableVideo          = HI_TRUE;
    stAttr.enVideoFmt            = enVideoFmt;
    stAttr.enVidOutMode          = HI_HDMI_VIDEO_MODE_YCBCR444;
    stAttr.enDeepColorMode       = HI_HDMI_DEEP_COLOR_24BIT;
    stAttr.bxvYCCMode            = HI_FALSE;
    stAttr.enOutCscQuantization  = HDMI_QUANTIZATION_LIMITED_RANGE;

    stAttr.bEnableAudio          = HI_FALSE;
    stAttr.enSoundIntf           = HI_HDMI_SND_INTERFACE_I2S;
    stAttr.bIsMultiChannel       = HI_FALSE;

    stAttr.enBitDepth            = HI_HDMI_BIT_DEPTH_16;

    stAttr.bEnableAviInfoFrame   = HI_TRUE;
    stAttr.bEnableAudInfoFrame   = HI_TRUE;
    stAttr.bEnableSpdInfoFrame   = HI_FALSE;
    stAttr.bEnableMpegInfoFrame  = HI_FALSE;

    stAttr.bDebugFlag            = HI_FALSE;
    stAttr.bHDCPEnable           = HI_FALSE;

    stAttr.b3DEnable             = HI_FALSE;
    stAttr.enDefaultMode         = HI_HDMI_FORCE_HDMI;

    CHECK_RET(HI_MPI_HDMI_SetAttr(enHdmiId, &stAttr), "HI_MPI_HDMI_SetAttr");
    CHECK_RET(HI_MPI_HDMI_Start(enHdmiId), "HI_MPI_HDMI_Start");

    return HI_SUCCESS;
}

/*
* Name : SAMPLE_COMM_VO_HdmiStartByDyRg
* Desc : Another function to start hdmi, according to video's dynamic range.
*/
int zpl_vidhal_hdmi_HdmiStartByDyRg(VO_INTF_SYNC_E enIntfSync, DYNAMIC_RANGE_E enDyRg)
{
    HI_HDMI_ATTR_S          stAttr;
    HI_HDMI_VIDEO_FMT_E     enVideoFmt;
    HI_HDMI_ID_E            enHdmiId    = HI_HDMI_ID_0;

    zpl_vidhal_hdmi_HdmiConvertSync(enIntfSync, &enVideoFmt);

    CHECK_RET(HI_MPI_HDMI_Init(), "HI_MPI_HDMI_Init");
    CHECK_RET(HI_MPI_HDMI_Open(enHdmiId), "HI_MPI_HDMI_Open");
    CHECK_RET(HI_MPI_HDMI_GetAttr(enHdmiId, &stAttr), "HI_MPI_HDMI_GetAttr");
    stAttr.bEnableHdmi           = HI_TRUE;
    stAttr.bEnableVideo          = HI_TRUE;
    stAttr.enVideoFmt            = enVideoFmt;
    stAttr.enVidOutMode          = HI_HDMI_VIDEO_MODE_YCBCR444;
    switch(enDyRg)
    {
        case DYNAMIC_RANGE_SDR8:
            stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_24BIT;
            break;
        case DYNAMIC_RANGE_HDR10:
            stAttr.enVidOutMode    = HI_HDMI_VIDEO_MODE_YCBCR422;
            break;
        default:
            stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_24BIT;
            break;
    }
    stAttr.bxvYCCMode            = HI_FALSE;
    stAttr.enOutCscQuantization  = HDMI_QUANTIZATION_LIMITED_RANGE;

    stAttr.bEnableAudio          = HI_FALSE;
    stAttr.enSoundIntf           = HI_HDMI_SND_INTERFACE_I2S;
    stAttr.bIsMultiChannel       = HI_FALSE;

    stAttr.enBitDepth            = HI_HDMI_BIT_DEPTH_16;

    stAttr.bEnableAviInfoFrame   = HI_TRUE;
    stAttr.bEnableAudInfoFrame   = HI_TRUE;
    stAttr.bEnableSpdInfoFrame   = HI_FALSE;
    stAttr.bEnableMpegInfoFrame  = HI_FALSE;

    stAttr.bDebugFlag            = HI_FALSE;
    stAttr.bHDCPEnable           = HI_FALSE;

    stAttr.b3DEnable             = HI_FALSE;
    stAttr.enDefaultMode         = HI_HDMI_FORCE_HDMI;

    CHECK_RET(HI_MPI_HDMI_SetAttr(enHdmiId, &stAttr), "HI_MPI_HDMI_SetAttr");
    CHECK_RET(HI_MPI_HDMI_Start(enHdmiId), "HI_MPI_HDMI_Start");

    return HI_SUCCESS;
}


int zpl_vidhal_hdmi_HdmiStop(HI_VOID)
{
    HI_HDMI_ID_E enHdmiId = HI_HDMI_ID_0;

    HI_MPI_HDMI_Stop(enHdmiId);
    HI_MPI_HDMI_Close(enHdmiId);
    HI_MPI_HDMI_DeInit();

    return HI_SUCCESS;
}
#endif
