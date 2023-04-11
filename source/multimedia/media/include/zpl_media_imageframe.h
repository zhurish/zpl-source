/*
 * zpl_media_event.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_EVENT_H__
#define __ZPL_MEDIA_EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif



/* we ONLY define picture format used, all unused will be deleted! */
typedef enum ZPL_PIXEL_FORMAT_E {
    ZPL_PIXEL_FORMAT_RGB_444 = 0,
    ZPL_PIXEL_FORMAT_RGB_555,
    ZPL_PIXEL_FORMAT_RGB_565,
    ZPL_PIXEL_FORMAT_RGB_888,

    ZPL_PIXEL_FORMAT_BGR_444,
    ZPL_PIXEL_FORMAT_BGR_555,
    ZPL_PIXEL_FORMAT_BGR_565,
    ZPL_PIXEL_FORMAT_BGR_888,

    ZPL_PIXEL_FORMAT_ARGB_1555,
    ZPL_PIXEL_FORMAT_ARGB_4444,
    ZPL_PIXEL_FORMAT_ARGB_8565,
    ZPL_PIXEL_FORMAT_ARGB_8888,
    ZPL_PIXEL_FORMAT_ARGB_2BPP,

    ZPL_PIXEL_FORMAT_ABGR_1555,
    ZPL_PIXEL_FORMAT_ABGR_4444,
    ZPL_PIXEL_FORMAT_ABGR_8565,
    ZPL_PIXEL_FORMAT_ABGR_8888,

    ZPL_PIXEL_FORMAT_RGB_BAYER_8BPP,
    ZPL_PIXEL_FORMAT_RGB_BAYER_10BPP,
    ZPL_PIXEL_FORMAT_RGB_BAYER_12BPP,
    ZPL_PIXEL_FORMAT_RGB_BAYER_14BPP,
    ZPL_PIXEL_FORMAT_RGB_BAYER_16BPP,


    ZPL_PIXEL_FORMAT_YVU_PLANAR_422,
    ZPL_PIXEL_FORMAT_YVU_PLANAR_420,
    ZPL_PIXEL_FORMAT_YVU_PLANAR_444,

    ZPL_PIXEL_FORMAT_YVU_SEMIPLANAR_422,
    ZPL_PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    ZPL_PIXEL_FORMAT_YVU_SEMIPLANAR_444,

    ZPL_PIXEL_FORMAT_YUV_SEMIPLANAR_422,
    ZPL_PIXEL_FORMAT_YUV_SEMIPLANAR_420,
    ZPL_PIXEL_FORMAT_YUV_SEMIPLANAR_444,

    ZPL_PIXEL_FORMAT_YUYV_PACKAGE_422,
    ZPL_PIXEL_FORMAT_YVYU_PACKAGE_422,
    ZPL_PIXEL_FORMAT_UYVY_PACKAGE_422,
    ZPL_PIXEL_FORMAT_VYUY_PACKAGE_422,
    ZPL_PIXEL_FORMAT_YYUV_PACKAGE_422,
    ZPL_PIXEL_FORMAT_YYVU_PACKAGE_422,
    ZPL_PIXEL_FORMAT_UVYY_PACKAGE_422,
    ZPL_PIXEL_FORMAT_VUYY_PACKAGE_422,
    ZPL_PIXEL_FORMAT_VY1UY0_PACKAGE_422,

    ZPL_PIXEL_FORMAT_YUV_400,
    ZPL_PIXEL_FORMAT_UV_420,

    /* SVP data format */
    ZPL_PIXEL_FORMAT_BGR_888_PLANAR,
    ZPL_PIXEL_FORMAT_HSV_888_PACKAGE,
    ZPL_PIXEL_FORMAT_HSV_888_PLANAR,
    ZPL_PIXEL_FORMAT_LAB_888_PACKAGE,
    ZPL_PIXEL_FORMAT_LAB_888_PLANAR,
    ZPL_PIXEL_FORMAT_S8C1,
    ZPL_PIXEL_FORMAT_S8C2_PACKAGE,
    ZPL_PIXEL_FORMAT_S8C2_PLANAR,
    ZPL_PIXEL_FORMAT_S8C3_PLANAR,
    ZPL_PIXEL_FORMAT_S16C1,
    ZPL_PIXEL_FORMAT_U8C1,
    ZPL_PIXEL_FORMAT_U16C1,
    ZPL_PIXEL_FORMAT_S32C1,
    ZPL_PIXEL_FORMAT_U32C1,
    ZPL_PIXEL_FORMAT_U64C1,
    ZPL_PIXEL_FORMAT_S64C1,

    ZPL_PIXEL_FORMAT_BUTT
} ZPL_PIXEL_FORMAT_E;

/* Type of the ZPL_IMAGE_S data.Aded by tanbing 2013-7-22 */
typedef enum ZPL_IMAGE_TYPE_E {
    ZPL_IMAGE_TYPE_U8C1 = 0x0,
    ZPL_IMAGE_TYPE_S8C1 = 0x1,

    ZPL_IMAGE_TYPE_YUV420SP = 0x2, /* YUV420 SemiPlanar */
    ZPL_IMAGE_TYPE_YUV422SP = 0x3, /* YUV422 SemiPlanar */
    ZPL_IMAGE_TYPE_YUV420P = 0x4,  /* YUV420 Planar */
    ZPL_IMAGE_TYPE_YUV422P = 0x5,  /* YUV422 planar */

    ZPL_IMAGE_TYPE_S8C2_PACKAGE = 0x6,
    ZPL_IMAGE_TYPE_S8C2_PLANAR = 0x7,

    ZPL_IMAGE_TYPE_S16C1 = 0x8,
    ZPL_IMAGE_TYPE_U16C1 = 0x9,

    ZPL_IMAGE_TYPE_U8C3_PACKAGE = 0xa,
    ZPL_IMAGE_TYPE_U8C3_PLANAR = 0xb,

    ZPL_IMAGE_TYPE_S32C1 = 0xc,
    ZPL_IMAGE_TYPE_U32C1 = 0xd,

    ZPL_IMAGE_TYPE_S64C1 = 0xe,
    ZPL_IMAGE_TYPE_U64C1 = 0xf,

    ZPL_IMAGE_TYPE_BUTT

} ZPL_IMAGE_TYPE_E;

/* Definition of the ZPL_IMAGE_S. Added by Tan Bing, 2013-7-22. */
typedef struct ZPL_IMAGE_S {
    zpl_uint64 au64PhyAddr[3];   /* RW;The physical address of the image */
    zpl_uint64 au64VirAddr[3];   /* RW;The virtual address of the image */
    zpl_uint32 au32Stride[3];    /* RW;The stride of the image */
    zpl_uint32 u32Width;         /* RW;The width of the image */
    zpl_uint32 u32Height;        /* RW;The height of the image */
    ZPL_IMAGE_TYPE_E enType; /* RW;The type of the image */
} ZPL_IMAGE_S;

typedef ZPL_IMAGE_S ZPL_SRC_IMAGE_S;
typedef ZPL_IMAGE_S ZPL_DST_IMAGE_S;

/*
* Definition of the ZPL_MEM_INFO_S.This struct special purpose for input or ouput, such as Hist, CCL, ShiTomasi.
* Added by Chen Quanfu, 2013-7-23.
*/
typedef struct ZPL_MEM_INFO_S {
    zpl_uint64 u64PhyAddr; /* RW;The physical address of the memory */
    zpl_uint64 u64VirAddr; /* RW;The virtual address of the memory */
    zpl_uint32 u32Size;    /* RW;The size of memory */
} ZPL_MEM_INFO_S;
typedef ZPL_MEM_INFO_S ZPL_SRC_MEM_INFO_S;
typedef ZPL_MEM_INFO_S ZPL_DST_MEM_INFO_S;

/* Data struct ,created by Chen Quanfu 2013-07-19 */
typedef struct ZPL_DATA_S {
    zpl_uint64 u64PhyAddr; /* RW;The physical address of the data */
    zpl_uint64 u64VirAddr; /* RW;The virtaul address of the data */

    zpl_uint32 u32Stride; /* RW;The stride of 2D data by byte */
    zpl_uint32 u32Width;  /* RW;The width of 2D data by byte */
    zpl_uint32 u32Height; /* RW;The height of 2D data by byte */

    zpl_uint32 u32Reserved;
} ZPL_DATA_S;

typedef ZPL_DATA_S ZPL_SRC_DATA_S;
typedef ZPL_DATA_S ZPL_DST_DATA_S;

typedef struct hiVIDEO_FRAME_S {
    zpl_uint32              u32Width;
    zpl_uint32              u32Height;
    ZPL_PIXEL_FORMAT_E      enPixelFormat;
    zpl_uint32              u32HeaderStride[3];
    zpl_uint32              u32Stride[3];
    zpl_uint32              u32ExtStride[3];

    zpl_uint64              u64HeaderPhyAddr[3];
    zpl_uint64              u64HeaderVirAddr[3];
    zpl_uint64              u64PhyAddr[3];
    zpl_uint64              u64VirAddr[3];
    zpl_uint64              u64ExtPhyAddr[3];
    zpl_uint64              u64ExtVirAddr[3];

    zpl_int16              s16OffsetTop;        /* top offset of show area */
    zpl_int16              s16OffsetBottom;    /* bottom offset of show area */
    zpl_int16              s16OffsetLeft;        /* left offset of show area */
    zpl_int16              s16OffsetRight;        /* right offset of show area */

    zpl_uint32              u32MaxLuminance;
    zpl_uint32              u32MinLuminance;

    zpl_uint32              u32TimeRef;
    zpl_uint64              u64PTS;
} VIDEO_FRAME_S;


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_EVENT_H__ */
