/*
 * zpl_vidhal_vgs.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDHAL_VGS_H__
#define __ZPL_VIDHAL_VGS_H__

#ifdef __cplusplus
extern "C" {
#endif


#ifdef ZPL_HISIMPP_MODULE

typedef struct
{
    VB_BLK VbHandle;
    zpl_uint8 *pu8VirAddr;
    zpl_uint32 u32VbSize;
    HI_BOOL bVbUsed;
} ZPL_VGS_VB_INFO;

typedef struct
{
    PIXEL_FORMAT_E      enPixelFormat;
    zpl_uint32              u32Width;
    zpl_uint32              u32Height;
    zpl_uint32              u32Align;
    COMPRESS_MODE_E     enCompressMode;
}ZPL_VB_BASE_INFO_S;


int zpl_vidhal_vgs_GetYUVBufferCfg(const ZPL_VB_BASE_INFO_S *pstVbBaseInfo, VB_CAL_CONFIG_S *pstVbCalConfig);

int zpl_vidhal_vgs_GetFrameVb(const ZPL_VB_BASE_INFO_S *pstVbInfo,
                                    VIDEO_FRAME_INFO_S *pstFrameInfo, ZPL_VGS_VB_INFO *pstVgsVbInfo);
int zpl_vidhal_vgs_vb_release(void);


int zpl_vidhal_vgs_scale_job(VIDEO_FRAME_INFO_S *inframe, VIDEO_FRAME_INFO_S *outframe, 
    VGS_SCLCOEF_MODE_E enVgsSclCoefMode, zpl_uint32 u32Width, zpl_uint32 u32Height);
int zpl_vidhal_vgs_cover_job(VIDEO_FRAME_INFO_S *inframe, VGS_ADD_COVER_S pstVgsAddCover);
int zpl_vidhal_vgs_osd_job(VIDEO_FRAME_INFO_S *inframe, VIDEO_FRAME_INFO_S *outframe, 
    VGS_ADD_OSD_S vgsosd);
int zpl_vidhal_vgs_drawline_job(VIDEO_FRAME_INFO_S *inframe, VIDEO_FRAME_INFO_S *outframe, 
    VGS_DRAW_LINE_S pstVgsDrawLine);
int zpl_vidhal_vgs_rotation_job(VIDEO_FRAME_INFO_S *inframe, VIDEO_FRAME_INFO_S *outframe, 
    ROTATION_E penRotationAngle);

    
#endif
#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_VGS_H__ */
