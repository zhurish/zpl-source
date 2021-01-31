/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** h264-config.h
** 
**
** -------------------------------------------------------------------------*/
#ifndef __H264_CONFIG_HPP__
#define __H264_CONFIG_HPP__

#undef PL_LIBX264_MODULE

#ifdef PL_OPENH264_MODULE
/* OpenH264: */
#include <wels/codec_api.h>
#include <wels/codec_app_def.h>

#define OPENH264_VER_AT_LEAST(maj, min) \
    ((OPENH264_MAJOR  > (maj)) || \
     (OPENH264_MAJOR == (maj) && OPENH264_MINOR >= (min)))

#endif


#ifdef PL_LIBX264_MODULE
/* x264: */
#include <x264.h>
#endif


typedef struct v4l2_codec_map_s
{
    int v4l2_fmt;
    int h264_fmt;
} v4l2_codec_map_t;


int v4l2_pixlfmt_h264(int fmt);
int v4l2_h264_pixlfmt(int fmt);

/*
YUV格式有两大类：planar和packed。
对于planar的YUV格式，先连续存储所有像素点的Y，紧接着存储所有像素点的U，随后是所有像素点的V。
对于packed的YUV格式，每个像素点的Y,U,V是连续交*存储的。

YUV 4:4:4采样，每一个Y对应一组UV分量。
YUV 4:2:2采样，每两个Y共用一组UV分量。 
YUV 4:2:0采样，每四个Y共用一组UV分量。

YUVY 格式 （属于YUV422）
YUYV为YUV422采样的存储格式中的一种，相邻的两个Y共用其相邻的两个Cb、Cr，分析，对于像素点Y'00、Y'01 而言，其Cb、Cr的值均为 Cb00、Cr00，其他的像素点的YUV取值依次类推

UYVY 格式 （属于YUV422）
UYVY格式也是YUV422采样的存储格式中的一种，只不过与YUYV不同的是UV的排列顺序不一样而已，还原其每个像素点的YUV值的方法与上面一样。

YUV422P（属于YUV422）
YUV422P也属于YUV422的一种，它是一种Plane模式，即平面模式，并不是将YUV数据交错存储，而是先存放所有的Y分量，然后存储所有的U（Cb）分量，最后存储所有的V（Cr）分量，如上图所示。其每一个像素点的YUV值提取方法也是遵循YUV422格式的最基本提取方法，即两个Y共用一个UV。比如，对于像素点Y'00、Y'01 而言，其Cb、Cr的值均为 Cb00、Cr00。

YV12，YU12格式（属于YUV420）
YU12和YV12属于YUV420格式，也是一种Plane模式，将Y、U、V分量分别打包，依次存储。其每一个像素点的YUV数据提取遵循YUV420格式的提取方式，即4个Y分量共用一组UV。注意，上图中，Y'00、Y'01、Y'10、Y'11共用Cr00、Cb00，其他依次类推。

NV12、NV21（属于YUV420）

NV12和NV21属于YUV420格式，是一种two-plane模式，即Y和UV分为两个Plane，但是UV（CbCr）为交错存储，而不是分为三个plane。其提取方式与上一种类似，即Y'00、Y'01、Y'10、Y'11共用Cr00、Cb00


*/
#if 0
/**
 * Types of media frame.
 */
typedef enum media_frame_type
{
    PJMEDIA_FRAME_TYPE_NONE,	    /**< No frame.		*/
    PJMEDIA_FRAME_TYPE_AUDIO,	    /**< Normal audio frame.	*/
    PJMEDIA_FRAME_TYPE_EXTENDED,    /**< Extended audio frame.	*/
    PJMEDIA_FRAME_TYPE_VIDEO        /**< Video frame.           */

} media_frame_type;


/**
 * This structure describes a media frame.
 */
typedef struct video_frame
{
    pjmedia_frame_type	 type;	    /**< Frame type.			    */
    void		*buf;	    /**< Pointer to buffer.		    */
    pj_size_t		 size;	    /**< Frame size in bytes.		    */
    pj_timestamp	 timestamp; /**< Frame timestamp.		    */
    pj_uint32_t		 bit_info;  /**< Bit info of the frame, sample case:
					 a frame may not exactly start and end
					 at the octet boundary, so this field
					 may be used for specifying start &
					 end bit offset.		    */
} video_frame;
#endif

#endif /* __H264_CONFIG_HPP__ */