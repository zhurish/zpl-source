#ifndef __ZPL_VIDHAL_IVE_H__
#define __ZPL_VIDHAL_IVE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#ifdef ZPL_HISIMPP_MODULE

#include "zpl_hal_hisi.h"

#define VIDEO_WIDTH 352
#define VIDEO_HEIGHT 288
#define IVE_ALIGN 16
#define IVE_CHAR_CALW 8
#define IVE_CHAR_CALH 8
#define IVE_CHAR_NUM (IVE_CHAR_CALW *IVE_CHAR_CALH)
#define IVE_FILE_NAME_LEN 256
#define IVE_RECT_NUM   64
#define VPSS_CHN_NUM 2

#define SAMPLE_ALIGN_BACK(x, a)     ((a) * (((x) / (a))))

typedef struct hiZPL_VIDHAL_IVE_SWITCH_S
{
   zpl_bool bVenc;
   zpl_bool bVo;
}ZPL_VIDHAL_IVE_SWITCH_S;

typedef struct hiZPL_VIDHAL_IVE_RECT_S
{
    POINT_S astPoint[4];
} ZPL_VIDHAL_IVE_RECT_S;

typedef struct hiZPL_VIDHAL_RECT_ARRAY_S
{
    zpl_uint16 u16Num;
    ZPL_VIDHAL_IVE_RECT_S astRect[IVE_RECT_NUM];
} ZPL_VIDHAL_RECT_ARRAY_S;

typedef struct hiIVE_LINEAR_DATA_S
{
    zpl_int32 s32LinearNum;
    zpl_int32 s32ThreshNum;
    POINT_S* pstLinearPoint;
} IVE_LINEAR_DATA_S;

typedef struct hiZPL_VIDHAL_IVE_DRAW_RECT_MSG_S
{
    VIDEO_FRAME_INFO_S stFrameInfo;
    ZPL_VIDHAL_RECT_ARRAY_S stRegion;
} ZPL_VIDHAL_IVE_DRAW_RECT_MSG_S;

/******************************************************************************
* function :Read file
******************************************************************************/
int zpl_vidhal_ive_readfile(IVE_IMAGE_S* pstImg, FILE* pFp);
/******************************************************************************
* function :Write file
******************************************************************************/
int zpl_vidhal_ive_writefile(IVE_IMAGE_S* pstImg, FILE* pFp);
/******************************************************************************
* function :Calc stride
******************************************************************************/
zpl_uint16 zpl_vidhal_ive_CalcStride(zpl_uint32 u32Width, zpl_uint8 u8Align);
/******************************************************************************
* function : Copy blob to rect
******************************************************************************/
zpl_void zpl_vidhal_ive_blob2rect(IVE_CCBLOB_S *pstBlob, ZPL_VIDHAL_RECT_ARRAY_S *pstRect,
                                            zpl_uint16 u16RectMaxNum,zpl_uint16 u16AreaThrStep,
                                            zpl_uint32 u32SrcWidth, zpl_uint32 u32SrcHeight,
                                            zpl_uint32 u32DstWidth,zpl_uint32 u32DstHeight);
/******************************************************************************
* function : Create ive image
******************************************************************************/
int zpl_vidhal_ive_create_image(IVE_IMAGE_S* pstImg, IVE_IMAGE_TYPE_E enType,
                                   zpl_uint32 u32Width, zpl_uint32 u32Height);
/******************************************************************************
* function : Create memory info
******************************************************************************/
int zpl_vidhal_ive_create_meminfo(IVE_MEM_INFO_S* pstMemInfo, zpl_uint32 u32Size);
/******************************************************************************
* function : Create ive image by cached
******************************************************************************/
int zpl_vidhal_ive_create_image_cached(IVE_IMAGE_S* pstImg,
        IVE_IMAGE_TYPE_E enType, zpl_uint32 u32Width, zpl_uint32 u32Height);
/******************************************************************************
* function : Create IVE_DATA_S
******************************************************************************/
int zpl_vidhal_ive_create_data(IVE_DATA_S* pstData,zpl_uint32 u32Width, zpl_uint32 u32Height);
/******************************************************************************
* function : Dma frame info to  ive image
******************************************************************************/
int zpl_vidhal_ive_dma_image(VIDEO_FRAME_INFO_S *pstFrameInfo,IVE_DST_IMAGE_S *pstDst,zpl_bool bInstant);

int zpl_vidhal_ive_dma_image2(IVE_SRC_IMAGE_S *pstSrc, IVE_DST_IMAGE_S *pstDst, zpl_bool bInstant);
/******************************************************************************
* function : Call vgs to fill rect
******************************************************************************/
int zpl_vidhal_vgs_fillrect(VIDEO_FRAME_INFO_S* pstFrmInfo, ZPL_VIDHAL_RECT_ARRAY_S* pstRect, zpl_uint32 u32Color);

int zpl_vidhal_ive_dma_frame(VIDEO_FRAME_INFO_S *SrcFrmInfo, VIDEO_FRAME_INFO_S *DstFrmInfo);
int zpl_vidhal_ive_dma_image2frame(IVE_DST_IMAGE_S *pstSrc, VIDEO_FRAME_INFO_S *pstFrameInfo, zpl_bool bInstant);


#endif


#ifdef __cplusplus
}
#endif
#endif


