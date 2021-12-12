/*
 * zpl_vidhal_pixfmt.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDHAL_PIXFMT_H__
#define __ZPL_VIDHAL_PIXFMT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
YUV的存储格式
通常分为：
1.打包(Packed)格式；
像素点的Y、U、V是连续交替存储的。

YUV/YUV/YUV/YUV/YUV/YUV....
1
2.平面(Planar)格式；
像素点的Y、U、V，是分别连续存储的。相当于将YUV拆成三个平面存储。

YYYYYY/UUUUUU/VVVVVV...
1
3.平面格式又有Semi-Planar的类别；
像素点的Y是连续存储的，U、V分量是交叉存放的。

YYYYYY/UV/UV/UV/UV/UV/UV...

主流有：
1.YUV444：每一个Y分量对应一组UV分量，平均一个像素占用8+8+8=24位。
2.YUV422：每两个Y分量共用一组UV分量，8+4+4=16位。

YUVY / UYVY / YUV422P / YUY2  ，此处要注意，YUY2是packed打包格式的。
1
3.YUV420：每四个Y分量共用一组UV分量，8+4+0=12位。

YUV420P ：YV12、YU12 是一种Planar格式。两者的区别是：YU12存储顺序是YUV，即YCbCr。YV12，存储顺序是YVU，即YCrCb。
YUV420SP：NV12、NV21 是一种Semi-Planar格式。NV12是IOS的模式，NV12是Android的模式。 其实nv系列，都属于semi-plane系列
1
2
4.YUV411：每四个Y分量共用一组UV分量，8+2+2=12位
*/
typedef enum
{
#if 0    
    ZPL_VIDEO_PIXEL_FORMAT_NONE = -1,
    ZPL_VIDEO_PIXEL_FORMAT_YUV420P,   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    ZPL_VIDEO_PIXEL_FORMAT_YUYV422,   ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    ZPL_VIDEO_PIXEL_FORMAT_YUV422P,   ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    ZPL_VIDEO_PIXEL_FORMAT_YUV444P,   ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    ZPL_VIDEO_PIXEL_FORMAT_YUV410P,   ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    ZPL_VIDEO_PIXEL_FORMAT_YUV411P,   ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)

    ZPL_VIDEO_PIXEL_FORMAT_YUVJ420P,  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of ZPL_VIDEO_PIXEL_FORMAT_YUV420P and setting color_range
    ZPL_VIDEO_PIXEL_FORMAT_YUVJ422P,  ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of ZPL_VIDEO_PIXEL_FORMAT_YUV422P and setting color_range
    ZPL_VIDEO_PIXEL_FORMAT_YUVJ444P,  ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of ZPL_VIDEO_PIXEL_FORMAT_YUV444P and setting color_range
    ZPL_VIDEO_PIXEL_FORMAT_UYVY422,   ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    ZPL_VIDEO_PIXEL_FORMAT_UYYVYY411, ///< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3

    ZPL_VIDEO_PIXEL_FORMAT_NV12,      ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    ZPL_VIDEO_PIXEL_FORMAT_NV21,      ///< as above, but U and V bytes are swapped


    ZPL_VIDEO_PIXEL_FORMAT_YUV440P,   ///< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    ZPL_VIDEO_PIXEL_FORMAT_YUVJ440P,  ///< planar YUV 4:4:0 full scale (JPEG), deprecated in favor of ZPL_VIDEO_PIXEL_FORMAT_YUV440P and setting color_range
    ZPL_VIDEO_PIXEL_FORMAT_YUVA420P,  ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)

    ZPL_VIDEO_PIXEL_FORMAT_YUV420P16LE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV420P16BE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV422P16LE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV422P16BE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV444P16LE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV444P16BE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
   
    /**
     * The following 12 formats have the disadvantage of needing 1 format for each bit depth.
     * Notice that each 9/10 bits sample is stored in 16 bits with extra padding.
     * If you want to support multiple bit depths, then using ZPL_VIDEO_PIXEL_FORMAT_YUV420P16* with the bpp stored separately is better.
     */
    ZPL_VIDEO_PIXEL_FORMAT_YUV420P9BE, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV420P9LE, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV420P10BE,///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV420P10LE,///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV422P10BE,///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV422P10LE,///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV444P9BE, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV444P9LE, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV444P10BE,///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV444P10LE,///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV422P9BE, ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV422P9LE, ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian

    ZPL_VIDEO_PIXEL_FORMAT_YUVA422P,  ///< planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)
    ZPL_VIDEO_PIXEL_FORMAT_YUVA444P,  ///< planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)
    ZPL_VIDEO_PIXEL_FORMAT_YUVA420P9BE,  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUVA420P9LE,  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUVA422P9BE,  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUVA422P9LE,  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUVA444P9BE,  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUVA444P9LE,  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUVA420P10BE, ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    ZPL_VIDEO_PIXEL_FORMAT_YUVA420P10LE, ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    ZPL_VIDEO_PIXEL_FORMAT_YUVA422P10BE, ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    ZPL_VIDEO_PIXEL_FORMAT_YUVA422P10LE, ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    ZPL_VIDEO_PIXEL_FORMAT_YUVA444P10BE, ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    ZPL_VIDEO_PIXEL_FORMAT_YUVA444P10LE, ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    ZPL_VIDEO_PIXEL_FORMAT_YUVA420P16BE, ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    ZPL_VIDEO_PIXEL_FORMAT_YUVA420P16LE, ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    ZPL_VIDEO_PIXEL_FORMAT_YUVA422P16BE, ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    ZPL_VIDEO_PIXEL_FORMAT_YUVA422P16LE, ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    ZPL_VIDEO_PIXEL_FORMAT_YUVA444P16BE, ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    ZPL_VIDEO_PIXEL_FORMAT_YUVA444P16LE, ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)

    ZPL_VIDEO_PIXEL_FORMAT_YVYU422,   ///< packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb

    ZPL_VIDEO_PIXEL_FORMAT_YUV420P12BE, ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV420P12LE, ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV420P14BE, ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV420P14LE, ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV422P12BE, ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV422P12LE, ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV422P14BE, ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV422P14LE, ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV444P12BE, ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV444P12LE, ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV444P14BE, ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV444P14LE, ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian


    ZPL_VIDEO_PIXEL_FORMAT_YUV440P10LE, ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV440P10BE, ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV440P12LE, ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUV440P12BE, ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian

 
    ZPL_VIDEO_PIXEL_FORMAT_YUVA422P12BE, ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), 12b alpha, big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUVA422P12LE, ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), 12b alpha, little-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUVA444P12BE, ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), 12b alpha, big-endian
    ZPL_VIDEO_PIXEL_FORMAT_YUVA444P12LE, ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), 12b alpha, little-endian
#endif
    ZPL_VIDEO_PIXEL_FORMAT_RGB_444 = 0,
    ZPL_VIDEO_PIXEL_FORMAT_RGB_555,
    ZPL_VIDEO_PIXEL_FORMAT_RGB_565,
    ZPL_VIDEO_PIXEL_FORMAT_RGB_888,

    ZPL_VIDEO_PIXEL_FORMAT_BGR_444,
    ZPL_VIDEO_PIXEL_FORMAT_BGR_555,
    ZPL_VIDEO_PIXEL_FORMAT_BGR_565,
    ZPL_VIDEO_PIXEL_FORMAT_BGR_888,

    ZPL_VIDEO_PIXEL_FORMAT_ARGB_1555,
    ZPL_VIDEO_PIXEL_FORMAT_ARGB_4444,
    ZPL_VIDEO_PIXEL_FORMAT_ARGB_8565,
    ZPL_VIDEO_PIXEL_FORMAT_ARGB_8888,
    ZPL_VIDEO_PIXEL_FORMAT_ARGB_2BPP,

    ZPL_VIDEO_PIXEL_FORMAT_ABGR_1555,
    ZPL_VIDEO_PIXEL_FORMAT_ABGR_4444,
    ZPL_VIDEO_PIXEL_FORMAT_ABGR_8565,
    ZPL_VIDEO_PIXEL_FORMAT_ABGR_8888,

    ZPL_VIDEO_PIXEL_FORMAT_RGB_BAYER_8BPP,
    ZPL_VIDEO_PIXEL_FORMAT_RGB_BAYER_10BPP,
    ZPL_VIDEO_PIXEL_FORMAT_RGB_BAYER_12BPP,
    ZPL_VIDEO_PIXEL_FORMAT_RGB_BAYER_14BPP,
    ZPL_VIDEO_PIXEL_FORMAT_RGB_BAYER_16BPP,


    ZPL_VIDEO_PIXEL_FORMAT_YVU_PLANAR_422,
    ZPL_VIDEO_PIXEL_FORMAT_YVU_PLANAR_420,
    ZPL_VIDEO_PIXEL_FORMAT_YVU_PLANAR_444,

    ZPL_VIDEO_PIXEL_FORMAT_YVU_SEMIPLANAR_422,
    ZPL_VIDEO_PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    ZPL_VIDEO_PIXEL_FORMAT_YVU_SEMIPLANAR_444,

    ZPL_VIDEO_PIXEL_FORMAT_YUV_SEMIPLANAR_422,
    ZPL_VIDEO_PIXEL_FORMAT_YUV_SEMIPLANAR_420,
    ZPL_VIDEO_PIXEL_FORMAT_YUV_SEMIPLANAR_444,

    ZPL_VIDEO_PIXEL_FORMAT_YUYV_PACKAGE_422,
    ZPL_VIDEO_PIXEL_FORMAT_YVYU_PACKAGE_422,
    ZPL_VIDEO_PIXEL_FORMAT_UYVY_PACKAGE_422,
    ZPL_VIDEO_PIXEL_FORMAT_VYUY_PACKAGE_422,
    ZPL_VIDEO_PIXEL_FORMAT_YYUV_PACKAGE_422,
    ZPL_VIDEO_PIXEL_FORMAT_YYVU_PACKAGE_422,
    ZPL_VIDEO_PIXEL_FORMAT_UVYY_PACKAGE_422,
    ZPL_VIDEO_PIXEL_FORMAT_VUYY_PACKAGE_422,
    ZPL_VIDEO_PIXEL_FORMAT_VY1UY0_PACKAGE_422,

    ZPL_VIDEO_PIXEL_FORMAT_YUV_400,
    ZPL_VIDEO_PIXEL_FORMAT_UV_420,

    /* SVP data format */
    ZPL_VIDEO_PIXEL_FORMAT_BGR_888_PLANAR,
    ZPL_VIDEO_PIXEL_FORMAT_HSV_888_PACKAGE,
    ZPL_VIDEO_PIXEL_FORMAT_HSV_888_PLANAR,
    ZPL_VIDEO_PIXEL_FORMAT_LAB_888_PACKAGE,
    ZPL_VIDEO_PIXEL_FORMAT_LAB_888_PLANAR,
    ZPL_VIDEO_PIXEL_FORMAT_S8C1,
    ZPL_VIDEO_PIXEL_FORMAT_S8C2_PACKAGE,
    ZPL_VIDEO_PIXEL_FORMAT_S8C2_PLANAR,
    ZPL_VIDEO_PIXEL_FORMAT_S8C3_PLANAR,
    ZPL_VIDEO_PIXEL_FORMAT_S16C1,
    ZPL_VIDEO_PIXEL_FORMAT_U8C1,
    ZPL_VIDEO_PIXEL_FORMAT_U16C1,
    ZPL_VIDEO_PIXEL_FORMAT_S32C1,
    ZPL_VIDEO_PIXEL_FORMAT_U32C1,
    ZPL_VIDEO_PIXEL_FORMAT_U64C1,
    ZPL_VIDEO_PIXEL_FORMAT_S64C1,

    ZPL_VIDEO_PIXEL_FORMAT_BUTT
} ZPL_VIDEO_PIXEL_FORMAT_E;


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDHAL_PIXFMT_H__ */
