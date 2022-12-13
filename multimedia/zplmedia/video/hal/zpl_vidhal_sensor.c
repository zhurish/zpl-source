/*
 * zpl_vidhal_sensor.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_sensor.h"

#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"
#endif

#ifdef ZPL_HISIMPP_MODULE
/**/
static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX327_12BIT_2M_WDR2to1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_DOL,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};

static combo_dev_attr_t MIPI_2lane_CHN1_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR =
{
    .devno = 1,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {1, 3, -1, -1}
        }
    }
};

static combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_IMX327_12BIT_2M_WDR2to1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_DOL,
            {0, 2, -1, -1}
        }
    }
};

static combo_dev_attr_t MIPI_2lane_CHN1_SENSOR_IMX327_12BIT_2M_WDR2to1_ATTR =
{
    .devno = 1,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_DOL,
            {1, 3, -1, -1}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX307_12BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX307_12BIT_2M_WDR2to1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_DOL,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_IMX307_12BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};

static combo_dev_attr_t MIPI_2lane_CHN1_SENSOR_IMX307_12BIT_2M_NOWDR_ATTR =
{
    .devno = 1,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {1, 3, -1, -1}
        }
    }
};

static combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_IMX307_12BIT_2M_WDR2to1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_DOL,
            {0, 2, -1, -1}
        }
    }
};

static combo_dev_attr_t MIPI_2lane_CHN1_SENSOR_IMX307_12BIT_2M_WDR2to1_ATTR =
{
    .devno = 1,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_DOL,
            {1, 3, -1, -1}
        }
    }
};

static combo_dev_attr_t LVDS_4lane_CHN0_SENSOR_MN34220_12BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_LVDS,
    .data_rate  = MIPI_DATA_RATE_X1,
    .img_rect   = {0, 0, 1920, 1080},

    .lvds_attr =
    {
        .input_data_type   = DATA_TYPE_RAW_12BIT,
        .wdr_mode          = HI_WDR_MODE_NONE,
        .sync_mode         = LVDS_SYNC_MODE_SOF,
        .vsync_attr        = {LVDS_VSYNC_NORMAL, 0, 0},
        .fid_attr          = {LVDS_FID_NONE, HI_TRUE},
        .data_endian       = LVDS_ENDIAN_BIG,
        .sync_code_endian  = LVDS_ENDIAN_BIG,
        .lane_id = {0, 2, 1, 3},
        .sync_code = {
            {
                {0x002, 0x003, 0x000, 0x001},      // lane 0
                {0x002, 0x003, 0x000, 0x001},
                {0x002, 0x003, 0x000, 0x001},
                {0x002, 0x003, 0x000, 0x001}
            },

            {
                {0x012, 0x013, 0x010, 0x011},      // lane 1
                {0x012, 0x013, 0x010, 0x011},
                {0x012, 0x013, 0x010, 0x011},
                {0x012, 0x013, 0x010, 0x011}
            },

            {
                {0x006, 0x007, 0x004, 0x005},      // lane2
                {0x006, 0x007, 0x004, 0x005},
                {0x006, 0x007, 0x004, 0x005},
                {0x006, 0x007, 0x004, 0x005}
            },

            {
                {0x016, 0x017, 0x014, 0x015},      // lane3
                {0x016, 0x017, 0x014, 0x015},
                {0x016, 0x017, 0x014, 0x015},
                {0x016, 0x017, 0x014, 0x015}
            },
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX335_12BIT_5M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2592, 1944},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX335_10BIT_5M_WDR2TO1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2592, 1944},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_VC,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX335_12BIT_4M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 204, 2592, 1536},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX335_10BIT_4M_WDR2TO1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 204, 2592, 1536},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_VC,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_SC4210_12BIT_3M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2560, 1440},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_SC4210_10BIT_3M_WDR2TO1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2560, 1440},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_VC,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX458_10BIT_8M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 3840, 2160},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX458_10BIT_12M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 4000, 3000},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX458_10BIT_4M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2716, 1524},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX458_10BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX458_10BIT_1M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1280, 720},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_OS04B10_10BIT_4M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2560, 1440},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1,-1, -1}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_OS05A_12BIT_4M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 204, 2688, 1536},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_OS05A_10BIT_4M_WDR2TO1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 204, 2688, 1536},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_VC,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_OS08A10_10BIT_8M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 3840, 2160},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_GC2053_10BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, -1, -1}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_OV12870_10BIT_8M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 3840, 2160},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_OV12870_10BIT_12M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 4000, 3000},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_OV12870_10BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_OV12870_10BIT_1M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1280, 720},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX415_12BIT_8M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 3840, 2160},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static VI_DEV_ATTR_S DEV_ATTR_IMX327_2M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920, 1080},
    {
        {
            {1920 , 1080},

        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_IMX307_2M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920, 1080},
    {
        {
            {1920 , 1080},

        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_MN34220_2M_BASE =
{
    VI_MODE_LVDS,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920 , 1080},
    {
        {
            {1920 , 1080},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_IMX335_5M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {2592 , 1944},
    {
        {
            {2592 , 1944},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1944
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_IMX335_4M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {2592 , 1536},
    {
        {
            {2592 , 1536},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1536
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_SC4210_3M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {2560 , 1440},
    {
        {
            {2560 , 1440},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1440
    },
    DATA_RATE_X1
};


static VI_DEV_ATTR_S DEV_ATTR_IMX458_8M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {3840 , 2160},
    {
        {
            {3840 , 2160},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        2160
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_IMX458_12M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {4000 , 3000},
    {
        {
            {4000 , 3000},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        3000
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_IMX458_4M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {2716 , 1524},
    {
        {
            {2716 , 1524},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1524
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_IMX458_2M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920 , 1080},
    {
        {
            {1920 , 1080},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_IMX458_1M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1280 , 720},
    {
        {
            {1280 , 720},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        720
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_OS04B10_4M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {2560 , 1440},
    {
        {
            {2560 , 1440},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1440
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_OS05A_4M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {2688 , 1536},
    {
        {
            {2688 , 1536},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1536
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_OS08A10_8M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {3840 , 2160},
    {
        {
            {3840 , 2160},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        2160
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_GC2053_2M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920 , 1080},
    {
        {
            {1920 , 1080},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_OV12870_8M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {3840 , 2160},
    {
        {
            {3840 , 2160},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        2160
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_OV12870_12M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {4000 , 3000},
    {
        {
            {4000 , 3000},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        3000
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_OV12870_2M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920 , 1080},
    {
        {
            {1920 , 1080},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_OV12870_1M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1280 , 720},
    {
        {
            {1280 , 720},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        720
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S DEV_ATTR_IMX415_8M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {3840 , 2160},
    {
        {
            {3840 , 2160},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        2160
    },
    DATA_RATE_X1
};

static VI_PIPE_ATTR_S PIPE_ATTR_1920x1080_RAW12_420 =
{
    /* bBindDev bYuvSkip */
    VI_PIPE_BYPASS_NONE, HI_FALSE,HI_FALSE,
    1920, 1080,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    1920, 1080,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_1920x1080_RAW12_420_3DNR_CHN0 =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE,HI_FALSE,
    1920, 1080,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_12,
    HI_TRUE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_CHN0,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2592x1944_RAW12_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2592, 1944,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    {-1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2592x1944_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2592, 1944,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_10,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    {-1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2592x1536_RAW12_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2592, 1536,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    {-1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2592x1536_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2592, 1536,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_10,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    {-1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2560x1440_RAW12_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2560, 1440,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    {-1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2560x1440_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2560, 1440,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_10,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    {-1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_3840x2160_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    3840, 2160,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_10,
    HI_TRUE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_3840x2160_RAW12_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    3840, 2160,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_12,
    HI_TRUE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_4000x3000_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    4000, 3000,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_10,
    HI_TRUE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2716x1524_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2716, 1524,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_10,
    HI_TRUE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_1920x1080_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    1920, 1080,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_10,
    HI_TRUE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_1280x720_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    1280, 720,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_10,
    HI_TRUE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2688x1536_RAW12_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2688, 1536,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    {-1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2688x1536_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2688, 1536,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_10,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    {-1, -1}
};


static VI_CHN_ATTR_S CHN_ATTR_1920x1080_422_SDR8_LINEAR =
{
    {1920, 1080},
    PIXEL_FORMAT_YVU_SEMIPLANAR_422,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_1920x1080_420_SDR8_LINEAR =
{
    {1920, 1080},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_1920x1080_400_SDR8_LINEAR =
{
    {1920, 1080},
    PIXEL_FORMAT_YUV_400,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_2592x1944_420_SDR8_LINEAR =
{
    {2592, 1944},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    {-1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_2592x1536_420_SDR8_LINEAR =
{
    {2592, 1536},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    {-1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_2560x1440_420_SDR8_LINEAR =
{
    {2560, 1440},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    {-1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_4000x3000_420_SDR8_LINEAR =
{
    {4000, 3000},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_3840x2160_420_SDR8_LINEAR =
{
    {3840, 2160},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_2716x1524_420_SDR8_LINEAR =
{
    {2716, 1524},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_1280x720_420_SDR8_LINEAR =
{
    {1280, 720},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_2688x1536_420_SDR8_LINEAR =
{
    {2688, 1536},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    {-1, -1}
};

#endif
/**/

int zpl_vidhal_sensor_get_comboattr(ZPL_SENSOR_TYPE_E snstype, zpl_uint32 MipiDev, void *p)
{
#ifdef ZPL_HISIMPP_MODULE
    combo_dev_attr_t *pstComboAttr = (combo_dev_attr_t *)p;
    switch (snstype)
    {
    case SONY_IMX327_MIPI_2M_30FPS_12BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;

    case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX327_12BIT_2M_WDR2to1_ATTR, sizeof(combo_dev_attr_t));
        break;

    case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT:
        if (0 == MipiDev)
        {
            memcpy(pstComboAttr, &MIPI_2lane_CHN0_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        }
        else if (1 == MipiDev)
        {
            memcpy(pstComboAttr, &MIPI_2lane_CHN1_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        }

        break;

    case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        if (0 == MipiDev)
        {
            memcpy(pstComboAttr, &MIPI_2lane_CHN0_SENSOR_IMX327_12BIT_2M_WDR2to1_ATTR, sizeof(combo_dev_attr_t));
        }
        else if (1 == MipiDev)
        {
            memcpy(pstComboAttr, &MIPI_2lane_CHN1_SENSOR_IMX327_12BIT_2M_WDR2to1_ATTR, sizeof(combo_dev_attr_t));
        }

        break;

    case SONY_IMX307_MIPI_2M_30FPS_12BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX307_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;

    case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX307_12BIT_2M_WDR2to1_ATTR, sizeof(combo_dev_attr_t));
        break;

    case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
        if (0 == MipiDev)
        {
            memcpy(pstComboAttr, &MIPI_2lane_CHN0_SENSOR_IMX307_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        }
        else if (1 == MipiDev)
        {
            memcpy(pstComboAttr, &MIPI_2lane_CHN1_SENSOR_IMX307_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        }

        break;

    case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        if (0 == MipiDev)
        {
            memcpy(pstComboAttr, &MIPI_2lane_CHN0_SENSOR_IMX307_12BIT_2M_WDR2to1_ATTR, sizeof(combo_dev_attr_t));
        }
        else if (1 == MipiDev)
        {
            memcpy(pstComboAttr, &MIPI_2lane_CHN1_SENSOR_IMX307_12BIT_2M_WDR2to1_ATTR, sizeof(combo_dev_attr_t));
        }

        break;

    case PANASONIC_MN34220_LVDS_2M_30FPS_12BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &LVDS_4lane_CHN0_SENSOR_MN34220_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;

    case SONY_IMX335_MIPI_5M_30FPS_12BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX335_12BIT_5M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;

    case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX335_10BIT_5M_WDR2TO1_ATTR, sizeof(combo_dev_attr_t));
        break;

    case SONY_IMX335_MIPI_4M_30FPS_12BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX335_12BIT_4M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;

    case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX335_10BIT_4M_WDR2TO1_ATTR, sizeof(combo_dev_attr_t));
        break;

    case SMART_SC4210_MIPI_3M_30FPS_12BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_SC4210_12BIT_3M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;

    case SMART_SC4210_MIPI_3M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_SC4210_10BIT_3M_WDR2TO1_ATTR, sizeof(combo_dev_attr_t));
        break;

    case SONY_IMX458_MIPI_8M_30FPS_10BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX458_10BIT_8M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;
    case SONY_IMX458_MIPI_12M_20FPS_10BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX458_10BIT_12M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;
    case SONY_IMX458_MIPI_4M_60FPS_10BIT:
    case SONY_IMX458_MIPI_4M_40FPS_10BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX458_10BIT_4M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;
    case SONY_IMX458_MIPI_2M_90FPS_10BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX458_10BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;
    case SONY_IMX458_MIPI_1M_129FPS_10BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX458_10BIT_1M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;

    case OMNIVISION_OS04B10_MIPI_4M_30FPS_10BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN0_SENSOR_OS04B10_10BIT_4M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;

    case OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_OS05A_12BIT_4M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;
    case OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_OS05A_10BIT_4M_WDR2TO1_ATTR, sizeof(combo_dev_attr_t));
        break;

    case OMNIVISION_OS08A10_MIPI_8M_30FPS_10BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_OS08A10_10BIT_8M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;

    case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN0_SENSOR_GC2053_10BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;
    case OMNIVISION_OV12870_MIPI_8M_30FPS_10BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_OV12870_10BIT_8M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;
    case OMNIVISION_OV12870_MIPI_12M_30FPS_10BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_OV12870_10BIT_12M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;
    case OMNIVISION_OV12870_MIPI_2M_120FPS_10BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_OV12870_10BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;
    case OMNIVISION_OV12870_MIPI_1M_240FPS_10BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_OV12870_10BIT_1M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;

    case SONY_IMX415_MIPI_8M_30FPS_12BIT:
    case SONY_IMX415_MIPI_8M_20FPS_12BIT:
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX415_12BIT_8M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
        break;

    default:
        zpl_media_debugmsg_error("not support enSnsType: %d\n", snstype);
        memcpy_s(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
    }
#endif
    return OK;
}

int zpl_vidhal_sensor_get_pipeattr(ZPL_SENSOR_TYPE_E snstype, void *p)
{
#ifdef ZPL_HISIMPP_MODULE
    VI_PIPE_ATTR_S *pstPipeAttr = (VI_PIPE_ATTR_S *)p;
    switch (snstype)
    {
    case SONY_IMX327_MIPI_2M_30FPS_12BIT:
    case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT:
    case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;

    case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        pstPipeAttr->enPixFmt = PIXEL_FORMAT_RGB_BAYER_10BPP;
        pstPipeAttr->enBitWidth = DATA_BITWIDTH_10;
        break;

    case SONY_IMX307_MIPI_2M_30FPS_12BIT:
    case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;

    case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
    case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        pstPipeAttr->enPixFmt = PIXEL_FORMAT_RGB_BAYER_10BPP;
        pstPipeAttr->enBitWidth = DATA_BITWIDTH_10;
        break;

    case PANASONIC_MN34220_LVDS_2M_30FPS_12BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;

    case SONY_IMX335_MIPI_5M_30FPS_12BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2592x1944_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;

    case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2592x1944_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;

    case SONY_IMX335_MIPI_4M_30FPS_12BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2592x1536_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;

    case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2592x1536_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;

    case SMART_SC4210_MIPI_3M_30FPS_12BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2560x1440_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;

    case SMART_SC4210_MIPI_3M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2560x1440_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;

    case OMNIVISION_OS08A10_MIPI_8M_30FPS_10BIT:
    case SONY_IMX458_MIPI_8M_30FPS_10BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_3840x2160_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;
    case SONY_IMX458_MIPI_12M_20FPS_10BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_4000x3000_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;
    case SONY_IMX458_MIPI_4M_60FPS_10BIT:
    case SONY_IMX458_MIPI_4M_40FPS_10BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2716x1524_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;
    case SONY_IMX458_MIPI_2M_90FPS_10BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;
    case SONY_IMX458_MIPI_1M_129FPS_10BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1280x720_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;
    case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_8M_30FPS_10BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_3840x2160_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_12M_30FPS_10BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_4000x3000_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_2M_120FPS_10BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_1M_240FPS_10BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1280x720_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;

    case OMNIVISION_OS04B10_MIPI_4M_30FPS_10BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2560x1440_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;

    case OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2688x1536_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;
    case OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2688x1536_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;

    case SONY_IMX415_MIPI_8M_30FPS_12BIT:
    case SONY_IMX415_MIPI_8M_20FPS_12BIT:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_3840x2160_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
        break;

    default:
        memcpy_s(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
    }
#endif
    return OK;
}

int zpl_vidhal_sensor_get_devattr(ZPL_SENSOR_TYPE_E snstype, void *p)
{
#ifdef ZPL_HISIMPP_MODULE
    VI_DEV_ATTR_S *pstViDevAttr = (VI_DEV_ATTR_S *)p;
    switch (snstype)
    {
    case SONY_IMX327_MIPI_2M_30FPS_12BIT:
    case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT:
    case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX327_2M_BASE, sizeof(VI_DEV_ATTR_S));
        break;

    case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX327_2M_BASE, sizeof(VI_DEV_ATTR_S));
        pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
        break;

    case SONY_IMX307_MIPI_2M_30FPS_12BIT:
    case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX307_2M_BASE, sizeof(VI_DEV_ATTR_S));
        break;

    case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
    case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX307_2M_BASE, sizeof(VI_DEV_ATTR_S));
        pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
        break;

    case PANASONIC_MN34220_LVDS_2M_30FPS_12BIT:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_MN34220_2M_BASE, sizeof(VI_DEV_ATTR_S));
        break;

    case SONY_IMX335_MIPI_5M_30FPS_12BIT:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX335_5M_BASE, sizeof(VI_DEV_ATTR_S));
        break;

    case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX335_5M_BASE, sizeof(VI_DEV_ATTR_S));
        pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
        break;

    case SONY_IMX335_MIPI_4M_30FPS_12BIT:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX335_4M_BASE, sizeof(VI_DEV_ATTR_S));
        break;

    case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX335_4M_BASE, sizeof(VI_DEV_ATTR_S));
        pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
        break;

    case SMART_SC4210_MIPI_3M_30FPS_12BIT:
    case SMART_SC4210_MIPI_3M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_SC4210_3M_BASE, sizeof(VI_DEV_ATTR_S));
        break;

    case SONY_IMX458_MIPI_8M_30FPS_10BIT:
        memcpy(pstViDevAttr, &DEV_ATTR_IMX458_8M_BASE, sizeof(VI_DEV_ATTR_S));
        break;
    case SONY_IMX458_MIPI_12M_20FPS_10BIT:
        memcpy(pstViDevAttr, &DEV_ATTR_IMX458_12M_BASE, sizeof(VI_DEV_ATTR_S));
        break;
    case SONY_IMX458_MIPI_4M_60FPS_10BIT:
    case SONY_IMX458_MIPI_4M_40FPS_10BIT:
        memcpy(pstViDevAttr, &DEV_ATTR_IMX458_4M_BASE, sizeof(VI_DEV_ATTR_S));
        break;
    case SONY_IMX458_MIPI_2M_90FPS_10BIT:
        memcpy(pstViDevAttr, &DEV_ATTR_IMX458_2M_BASE, sizeof(VI_DEV_ATTR_S));
        break;
    case SONY_IMX458_MIPI_1M_129FPS_10BIT:
        memcpy(pstViDevAttr, &DEV_ATTR_IMX458_1M_BASE, sizeof(VI_DEV_ATTR_S));
        break;
    case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_GC2053_2M_BASE, sizeof(VI_DEV_ATTR_S));
        pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
        break;
    case OMNIVISION_OV12870_MIPI_8M_30FPS_10BIT:
        memcpy(pstViDevAttr, &DEV_ATTR_OV12870_8M_BASE, sizeof(VI_DEV_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_12M_30FPS_10BIT:
        memcpy(pstViDevAttr, &DEV_ATTR_OV12870_12M_BASE, sizeof(VI_DEV_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_2M_120FPS_10BIT:
        memcpy(pstViDevAttr, &DEV_ATTR_OV12870_2M_BASE, sizeof(VI_DEV_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_1M_240FPS_10BIT:
        memcpy(pstViDevAttr, &DEV_ATTR_OV12870_1M_BASE, sizeof(VI_DEV_ATTR_S));
        break;

    case OMNIVISION_OS04B10_MIPI_4M_30FPS_10BIT:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_OS04B10_4M_BASE, sizeof(VI_DEV_ATTR_S));
        break;

    case OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_OS05A_4M_BASE, sizeof(VI_DEV_ATTR_S));
        break;
    case OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_OS05A_4M_BASE, sizeof(VI_DEV_ATTR_S));
        pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
        break;

    case OMNIVISION_OS08A10_MIPI_8M_30FPS_10BIT:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_OS08A10_8M_BASE, sizeof(VI_DEV_ATTR_S));
        pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
        break;

    case SONY_IMX415_MIPI_8M_30FPS_12BIT:
    case SONY_IMX415_MIPI_8M_20FPS_12BIT:
        memcpy(pstViDevAttr, &DEV_ATTR_IMX415_8M_BASE, sizeof(VI_DEV_ATTR_S));
        break;

    default:
        memcpy_s(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX327_2M_BASE, sizeof(VI_DEV_ATTR_S));
    }
#endif
    return OK;
}

int zpl_vidhal_sensor_get_chnattr(ZPL_SENSOR_TYPE_E snstype, void *p)
{
#ifdef ZPL_HISIMPP_MODULE
    VI_CHN_ATTR_S *pstChnAttr = (VI_CHN_ATTR_S *)p;
    switch (snstype)
    {
    case SONY_IMX327_MIPI_2M_30FPS_12BIT:
    case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
    case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT:
    case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1920x1080_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;

    case SONY_IMX307_MIPI_2M_30FPS_12BIT:
    case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
    case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
    case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1920x1080_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;

    case PANASONIC_MN34220_LVDS_2M_30FPS_12BIT:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1920x1080_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;

    case SONY_IMX335_MIPI_5M_30FPS_12BIT:
    case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_2592x1944_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;

    case SONY_IMX335_MIPI_4M_30FPS_12BIT:
    case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_2592x1536_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;

    case SMART_SC4210_MIPI_3M_30FPS_12BIT:
    case SMART_SC4210_MIPI_3M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_2560x1440_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;

    case OMNIVISION_OS08A10_MIPI_8M_30FPS_10BIT:
    case SONY_IMX458_MIPI_8M_30FPS_10BIT:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_3840x2160_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;
    case SONY_IMX458_MIPI_12M_20FPS_10BIT:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_4000x3000_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;
    case SONY_IMX458_MIPI_4M_60FPS_10BIT:
    case SONY_IMX458_MIPI_4M_40FPS_10BIT:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_2716x1524_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;
    case SONY_IMX458_MIPI_2M_90FPS_10BIT:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1920x1080_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;
    case SONY_IMX458_MIPI_1M_129FPS_10BIT:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1280x720_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;
    case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1920x1080_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_8M_30FPS_10BIT:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_3840x2160_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_12M_30FPS_10BIT:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_4000x3000_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_2M_120FPS_10BIT:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1920x1080_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;
    case OMNIVISION_OV12870_MIPI_1M_240FPS_10BIT:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1280x720_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;

    case OMNIVISION_OS04B10_MIPI_4M_30FPS_10BIT:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_2560x1440_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;

    case OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT:
    case OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_2688x1536_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;

    case SONY_IMX415_MIPI_8M_30FPS_12BIT:
    case SONY_IMX415_MIPI_8M_20FPS_12BIT:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_3840x2160_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
        break;

    default:
        memcpy_s(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1920x1080_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
    }
#endif
    return OK;
}

int zpl_vidhal_sensor_get_extchnattr(ZPL_SENSOR_TYPE_E snstype, void *p)
{
    return ERROR;
}