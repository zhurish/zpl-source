/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** rtsp_server.h
** 
** RTSP server
**
** -------------------------------------------------------------------------*/
#include <sstream>
#include <iostream>
#include <cstring>

extern "C"
{
#include <stdint.h>
#include <stdio.h>
#include <linux/videodev2.h>
};

#include "h264Encoder.hpp"

#ifdef PL_LIBX264_MODULE
//#define X264_CSP_NONE           0x0000  /* Invalid mode     */
static v4l2_codec_map_t v4l2_codec_map[] = 
{
    //{V4L2_PIX_FMT_YUYV, X264_CSP_I400},/* monochrome 4:0:0 */
    {V4L2_PIX_FMT_YUV420, X264_CSP_I420},/* yuv 4:2:0 planar */
    {V4L2_PIX_FMT_YVU420, X264_CSP_YV12},/* yvu 4:2:0 planar */
    {V4L2_PIX_FMT_NV12, X264_CSP_NV12},/* yuv 4:2:0, with one y plane and one packed u+v */
    {V4L2_PIX_FMT_NV21, X264_CSP_NV21},/* yuv 4:2:0, with one y plane and one packed v+u */
    {V4L2_PIX_FMT_YUV422M, X264_CSP_I422},/* yuv 4:2:2 planar */
    {V4L2_PIX_FMT_YUV422P, X264_CSP_YV16},/* yvu 4:2:2 planar */
    {V4L2_PIX_FMT_NV16, X264_CSP_NV16},/* yuv 4:2:2, with one y plane and one packed u+v */
    {V4L2_PIX_FMT_YUYV, X264_CSP_YUYV},/* yuyv 4:2:2 packed */
    {V4L2_PIX_FMT_UYVY, X264_CSP_UYVY},/* uyvy 4:2:2 packed */
    //{V4L2_PIX_FMT_YUYV, X264_CSP_V210},/* 10-bit yuv 4:2:2 packed in 32 */
    {V4L2_PIX_FMT_YUV444M, X264_CSP_I444},/* yuv 4:4:4 planar */
    {V4L2_PIX_FMT_YVU444M, X264_CSP_YV24},/* yvu 4:4:4 planar */
    {V4L2_PIX_FMT_BGR24, X264_CSP_BGR},/* packed bgr 24bits */
    {V4L2_PIX_FMT_BGR32, X264_CSP_BGRA},/* packed bgr 32bits */
    {V4L2_PIX_FMT_RGB24, X264_CSP_RGB},/* packed rgb 24bits */

    /* HSV formats */
    {V4L2_PIX_FMT_HSV24, X264_CSP_NONE},
    {V4L2_PIX_FMT_HSV32, X264_CSP_NONE},

    {V4L2_PIX_FMT_MPEG, X264_CSP_NONE},/* MPEG-1/2/4 Multiplexed */
    {V4L2_PIX_FMT_H264, X264_CSP_NONE},/* H264 with start codes */
    {V4L2_PIX_FMT_H264_NO_SC, X264_CSP_NONE},/* H264 without start codes */
    {V4L2_PIX_FMT_H264_MVC, X264_CSP_NONE},/* H264 MVC */

    {V4L2_PIX_FMT_H263, X264_CSP_NONE},/* H263          */
    {V4L2_PIX_FMT_MPEG1, X264_CSP_NONE},/* MPEG-1 ES     */
    {V4L2_PIX_FMT_MPEG2, X264_CSP_NONE},/* MPEG-2 ES     */
    {V4L2_PIX_FMT_MPEG4, X264_CSP_NONE},/* MPEG-4 part 2 ES */
    {V4L2_PIX_FMT_VP8, X264_CSP_NONE},/* VP8 */
    {V4L2_PIX_FMT_VP9, X264_CSP_NONE},/* VP9 */
    {V4L2_PIX_FMT_HEVC, X264_CSP_NONE},/* HEVC aka H.265 */
};
#endif

#ifdef PL_OPENH264_MODULE
//#define X264_CSP_NONE           0x0000  /* Invalid mode     */
static v4l2_codec_map_t v4l2_codec_map[] = 
{
    //{V4L2_PIX_FMT_YUYV, videoFormatRGB},///< rgb color formats
    //{V4L2_PIX_FMT_YUYV, videoFormatRGBA},
    {V4L2_PIX_FMT_RGB555, videoFormatRGB555},
    {V4L2_PIX_FMT_RGB565, videoFormatRGB565},
    {V4L2_PIX_FMT_BGR32, videoFormatBGR},
    {V4L2_PIX_FMT_ABGR32, videoFormatBGRA},
    {V4L2_PIX_FMT_ABGR32, videoFormatABGR},
    {V4L2_PIX_FMT_ARGB32, videoFormatARGB},
    {V4L2_PIX_FMT_YUYV, videoFormatYUY2},///< yuv color formats
    {V4L2_PIX_FMT_YVYU, videoFormatYVYU},
    {V4L2_PIX_FMT_UYVY, videoFormatUYVY},
    {V4L2_PIX_FMT_YUV420, videoFormatI420},///< the same as IYUV
    {V4L2_PIX_FMT_YVU420, videoFormatYV12},
    //{V4L2_PIX_FMT_YUYV, videoFormatInternal},///< only used in SVC decoder testbed
    {V4L2_PIX_FMT_NV12, videoFormatNV12},///< new format for output by DXVA decoding

    /* HSV formats */
    {V4L2_PIX_FMT_HSV24, 0},
    {V4L2_PIX_FMT_HSV32, 0},

    {V4L2_PIX_FMT_MPEG, 0},/* MPEG-1/2/4 Multiplexed */
    {V4L2_PIX_FMT_H264, 0},/* H264 with start codes */
    {V4L2_PIX_FMT_H264_NO_SC, 0},/* H264 without start codes */
    {V4L2_PIX_FMT_H264_MVC, 0},/* H264 MVC */

    {V4L2_PIX_FMT_H263, 0},/* H263          */
    {V4L2_PIX_FMT_MPEG1, 0},/* MPEG-1 ES     */
    {V4L2_PIX_FMT_MPEG2, 0},/* MPEG-2 ES     */
    {V4L2_PIX_FMT_MPEG4, 0},/* MPEG-4 part 2 ES */
    {V4L2_PIX_FMT_VP8, 0},/* VP8 */
    {V4L2_PIX_FMT_VP9, 0},/* VP9 */
    {V4L2_PIX_FMT_HEVC, 0},/* HEVC aka H.265 */
};
#endif
        
int v4l2_pixlfmt_h264(int fmt)
{
#if defined(PL_OPENH264_MODULE) || defined(PL_LIBX264_MODULE)
    unsigned int i = 0;
    for(i = 0; i< sizeof(v4l2_codec_map)/sizeof(v4l2_codec_map[0]); i++)
    {
        if(v4l2_codec_map[i].v4l2_fmt == fmt)
        {
            return v4l2_codec_map[i].h264_fmt;
        }
    }
#endif
    return -1;
}      

int v4l2_h264_pixlfmt(int fmt)
{
#if defined(PL_OPENH264_MODULE) || defined(PL_LIBX264_MODULE)
    unsigned int i = 0;
    for(i = 0; i< sizeof(v4l2_codec_map)/sizeof(v4l2_codec_map[0]); i++)
    {
        if(v4l2_codec_map[i].h264_fmt == fmt)
        {
            return v4l2_codec_map[i].v4l2_fmt;
        }
    }
#endif
    return -1;
} 



h264Encoder::h264Encoder():videoEncoder()
{
}

h264Encoder::~h264Encoder()
{
    videoEncoderDestroy();
}

int h264Encoder::videoEncoderSetup(const int width, const int height, const int fmt, const int fps)
{
    this->m_width = width;
    this->m_height = height;
    this->m_fmt = fmt;
    this->m_fps = fps;
    this->m_dfmt = fmt;
    if(v4l2_pixlfmt_h264(fmt) == -1)
    {
        std::cout << "Not support this V4L2 FIX Format." << std::endl;
        return -1;
    }
#ifdef PL_OPENH264_MODULE
    return openh264_encoder_setup(width, height, v4l2_pixlfmt_h264(fmt), fps);
#endif
#ifdef PL_LIBX264_MODULE
    return h264_encoder_setup(width, height, v4l2_pixlfmt_h264(fmt), fps);
#endif
}

int h264Encoder::videoEncoderInput(const unsigned char *frame, const int len,const  bool keyframe)
{
#ifdef PL_LIBX264_MODULE
    return h264_encoder_input(frame, len, keyframe);
#endif
#ifdef PL_OPENH264_MODULE
    return openh264_encoder_input(frame, len, keyframe);
#endif
}

int h264Encoder::videoEncoderOutput(unsigned char *frame, const int len)
{
#ifdef PL_LIBX264_MODULE
    return h264_encoder_output(frame, len);
#endif
#ifdef PL_OPENH264_MODULE
    return openh264_encoder_output(frame, len);
#endif
}

unsigned char *h264Encoder::videoEncoderOutput()
{
#ifdef PL_LIBX264_MODULE
    return m_h264_nal->p_payload;
#endif
#ifdef PL_OPENH264_MODULE
    return (unsigned char *)m_out_frame_payload;
#endif
}

int h264Encoder::videoEncoderOutputSize(const bool clear)
{
    int ret = 0;

    ret = (int)m_out_size;
    if (clear)
        m_out_size = 0;
    return ret;
}

int h264Encoder::videoEncoderDestroy()
{
#ifdef PL_LIBX264_MODULE
    if (m_h264)
        x264_encoder_close(m_h264);
    x264_picture_clean(&m_pic_in);
#endif
#ifdef PL_OPENH264_MODULE
    openh264_encoder_destroy();
#endif
    return 0;
}

#ifdef PL_LIBX264_MODULE
int h264Encoder::h264_encoder_setup(const int width, const int height, const int fmt, const int fps)
{
    int _x264_profile_index = 0;
    x264_param_default(&m_param);
    /* Get default params for preset/tuning */
    if (x264_param_default_preset(&m_param, "medium", NULL) < 0)
        return -1;

    /* Configure non-default params */
    //param.i_bitdepth = 8;
    m_param.i_csp = fmt; //X264_CSP_I420;
    m_param.i_width = width;
    m_param.i_height = height;
    //m_param.b_vfr_input = 0;
    //m_param.b_repeat_headers = 1;
    //m_param.b_annexb = 1;
    m_param.i_fps_num = fps;
    //m_param.i_fps_dep = 1;
    //m_param.i_keyint_max = 10;
    //m_param.rc.i_bitrate = 1200;
    //m_param.rc.i_rc_method = X264_RC_ABR;

    m_plane_2 = m_plane_1 = m_plane_0 = m_frame_size = width * height;

    if (i_csp == X264_CSP_I444)
    {
        m_frame_size += (m_frame_size << 1);
        _x264_profile_index = 5;
    }
    else if (i_csp == X264_CSP_I422)
    {
        //plane_2 = plane_1 = (plane_0 >> 1);
        m_frame_size = (m_frame_size * 2);
        _x264_profile_index = 4;
    }
    else //if(i_csp == X264_CSP_I420)
    {
        //frame_size += (frame_size/4)*2;
        //frame_size += (frame_size >> 1);
        m_plane_2 = m_plane_1 = (m_plane_0 >> 2);
        m_frame_size = (m_frame_size * 3 / 2);
        _x264_profile_index = 2;
    }
    //m_frame_size = m_plane_0 + m_plane_1 + m_plane_2;
    /* Apply profile restrictions. */
    if (x264_param_apply_profile(&m_param, x264_profile_names[_x264_profile_index] /*"high422"*/) < 0)
        return -1;
    //x264_picture_init( &m_pic_in);
    if (x264_picture_alloc(&m_pic_in, m_param.i_csp, m_param.i_width, m_param.i_height) < 0)
        return -1;
    m_h264 = x264_encoder_open(&m_param);
    if (!m_h264)
        return -1;
    return 0;
}

int h264Encoder::h264_encoder_input(const unsigned char *frame, const int len, const bool keyframe)
{
    int i_nal = 0;
    x264_picture_t pic_out;
    x264_picture_init(&pic_out);
    std::cout << "====================== frame-len:" << len << "frame-size:" << frame_size << std::endl;
    if (len == m_frame_size)
    {
        /* Read input frame */
        if (m_param.i_csp == X264_CSP_I422)
        {
            int i = 0;
            int y_i = 0, u_i = 0, v_i = 0;
            for (i = 0; i < len;)
            {
                m_pic_in.img.plane[0][y_i++] = frame[i];
                i++;
                m_pic_in.img.plane[1][u_i++] = frame[i];
                i++;
                m_pic_in.img.plane[0][y_i++] = frame[i];
                i++;
                m_pic_in.img.plane[2][v_i++] = frame[i];
                i++;
            }
        }
        else
        {
            memcpy(m_pic_in.img.plane[0], frame, m_plane_0);
            memcpy(m_pic_in.img.plane[1], frame + m_plane_0, m_plane_1);
            memcpy(m_pic_in.img.plane[2], frame + m_plane_0 + m_plane_1, m_plane_2);
        }
        std::cout << "====================== x264_encoder_encode" << std::endl;
        m_out_size = x264_encoder_encode(m_h264, &m_h264_nal, &i_nal, &m_pic_in, &pic_out);
        if (m_out_size < 0)
            return -1;
        else if (m_out_size)
        {
            //if( !fwrite( nal->p_payload, out_frame_size, 1, stdout ) )
            //    goto fail;
            m_pic_in.i_pts++;
            return m_out_size;
        }
    }
    else
    {
        /* Flush delayed frames */
        while (x264_encoder_delayed_frames(m_h264))
        {
            m_out_size = x264_encoder_encode(m_h264, &m_h264_nal, &i_nal, NULL, &pic_out);
            if (m_out_size < 0)
                return -1;
            else if (m_out_size)
            {
                //encode_data.clear();
                //if( !fwrite( nal->p_payload, out_frame_size, 1, stdout ) )
                //    goto fail;
                return m_out_size;
            }
        }
    }
    return -1;
}

int h264Encoder::h264_encoder_output(unsigned char *frame, const int len)
{
    if (m_out_size > 0)
    {
        int ret = (m_out_size > len) ? len : m_out_size;
        memmove(frame, m_h264_nal->p_payload, ret);
        m_out_size = 0;
        return ret;
    }
    return -1;
}
#endif

#ifdef PL_OPENH264_MODULE
struct SLayerPEncCtx
{
    unsigned int iDLayerQp;
    SSliceArgument sSliceArgument;
};

int h264Encoder::openh264_encoder_destroy()
{
    if(m_esrc_pic.iColorFormat != v4l2_pixlfmt_h264(this->m_dfmt))
    {
        if(m_yuv420 != nullptr)
            delete [] m_yuv420;
    }

    if (m_out_frame_payload != nullptr)
        delete [] m_out_frame_payload;
    if (m_encoder != nullptr)
        WelsDestroySVCEncoder(m_encoder);
    return 0;
}

int h264Encoder::openh264_encoder_setup(const int width, const int height, const int fmt, const int fps)
{
    /* encoder allocation */
    int rc = WelsCreateSVCEncoder(&m_encoder);
    if (rc != 0)
        return -1;
    SEncParamExt eprm;
    SSpatialLayerConfig *elayer = &eprm.sSpatialLayers[0];
    SLayerPEncCtx elayer_ctx;
    SDecodingParam sDecParam = {0};
    SliceModeEnum eSliceMode = SM_SINGLE_SLICE; //eprm.sSpatialLayers[0].sSliceArgument.uiSliceMode;
    /*
     * Encoder
     */
    /* Init encoder parameters */
    m_encoder->GetDefaultParams(&eprm);
    eSliceMode = eprm.sSpatialLayers[0].sSliceArgument.uiSliceMode = SM_SINGLE_SLICE;

    eprm.iComplexityMode = MEDIUM_COMPLEXITY;
    eprm.sSpatialLayers[0].uiProfileIdc = PRO_BASELINE;
    eprm.iPicWidth = m_width;
    eprm.iUsageType = CAMERA_VIDEO_REAL_TIME;
    eprm.iPicHeight = m_height;
    eprm.fMaxFrameRate = fps;
    eprm.iTemporalLayerNum = 1;
    eprm.uiIntraPeriod = 0;               /* I-Frame interval in frames */
    eprm.eSpsPpsIdStrategy = CONSTANT_ID; //INCREASING_ID;
#if OPENH264_VER_AT_LEAST(1, 4)
    eprm.eSpsPpsIdStrategy = CONSTANT_ID;
#else
    //eprm.bEnableSpsPpsIdAddition = 0;
#endif
    eprm.bEnableFrameCroppingFlag = true;
    eprm.iLoopFilterDisableIdc = 0;
    eprm.iLoopFilterAlphaC0Offset = 0;
    eprm.iLoopFilterBetaOffset = 0;
    eprm.iMultipleThreadIdc = 1;
    //eprm.bEnableRc			= 1;
    eprm.iTargetBitrate = 5000000; //param->enc_fmt.det.vid.avg_bps;
    eprm.bEnableFrameSkip = 1;
    eprm.bEnableDenoise = 0;
    eprm.bEnableSceneChangeDetect = 1;
    eprm.bEnableBackgroundDetection = 1;
    eprm.bEnableAdaptiveQuant = 1;
    eprm.bEnableLongTermReference = 0;
    eprm.iLtrMarkPeriod = 30;
    eprm.bPrefixNalAddingCtrl = false;
    eprm.iSpatialLayerNum = 1;
    //eprm.uiMaxNalSize			= param->enc_mtu;

    memset(&elayer_ctx, 0, sizeof(SLayerPEncCtx));
    elayer_ctx.iDLayerQp = 24;
    elayer_ctx.sSliceArgument.uiSliceMode = SM_SINGLE_SLICE; //SM_SIZELIMITED_SLICE;

    elayer_ctx.sSliceArgument.uiSliceNum = 1;
    elayer_ctx.sSliceArgument.uiSliceMbNum[0] = 960;
    elayer_ctx.sSliceArgument.uiSliceMbNum[1] = 0;
    elayer_ctx.sSliceArgument.uiSliceMbNum[2] = 0;
    elayer_ctx.sSliceArgument.uiSliceMbNum[3] = 0;
    elayer_ctx.sSliceArgument.uiSliceMbNum[4] = 0;
    elayer_ctx.sSliceArgument.uiSliceMbNum[5] = 0;
    elayer_ctx.sSliceArgument.uiSliceMbNum[6] = 0;
    elayer_ctx.sSliceArgument.uiSliceMbNum[7] = 0;

    eprm.iEntropyCodingModeFlag = 1;
    if (eSliceMode != SM_SINGLE_SLICE && eSliceMode != SM_SIZELIMITED_SLICE) //SM_SIZELIMITED_SLICE don't support multi-thread now
        eprm.iMultipleThreadIdc = 2;

    if (eSliceMode == SM_SIZELIMITED_SLICE)
    {
        eprm.sSpatialLayers[0].sSliceArgument.uiSliceSizeConstraint = 600;
        eprm.uiMaxNalSize = 1500;
        eprm.iMultipleThreadIdc = 4;
        eprm.bUseLoadBalancing = false;
    }
    if (eSliceMode == SM_FIXEDSLCNUM_SLICE)
    {
        eprm.sSpatialLayers[0].sSliceArgument.uiSliceNum = 4;
        eprm.iMultipleThreadIdc = 4;
        eprm.bUseLoadBalancing = false;
    }
    if (eprm.iEntropyCodingModeFlag)
    {
        eprm.sSpatialLayers[0].uiProfileIdc = PRO_MAIN;
    }
    elayer->iVideoWidth = eprm.iPicWidth;
    elayer->iVideoHeight = eprm.iPicHeight;
    elayer->fFrameRate = eprm.fMaxFrameRate;
    elayer->uiProfileIdc = eprm.sSpatialLayers[0].uiProfileIdc;
    elayer->iSpatialBitrate = eprm.iTargetBitrate;
    elayer->iDLayerQp = elayer_ctx.iDLayerQp;
    elayer->sSliceArgument.uiSliceMode = elayer_ctx.sSliceArgument.uiSliceMode;

    memcpy(&elayer->sSliceArgument,
           &elayer_ctx.sSliceArgument,
           sizeof(SSliceArgument));
    memcpy(&elayer->sSliceArgument.uiSliceMbNum[0],
           &elayer_ctx.sSliceArgument.uiSliceMbNum[0],
           sizeof(elayer_ctx.sSliceArgument.uiSliceMbNum));

    /* Init input picture */
    
    //m_esrc_pic.iColorFormat = fmt;
    m_esrc_pic.iColorFormat	= videoFormatI420;
    m_esrc_pic.uiTimeStamp = 0;
    m_esrc_pic.iPicWidth = eprm.iPicWidth;
    m_esrc_pic.iPicHeight = eprm.iPicHeight;
    m_esrc_pic.iStride[0] = m_esrc_pic.iPicWidth;
    //if (fmt == videoFormatI420)
        m_esrc_pic.iStride[1] = m_esrc_pic.iStride[2] = m_esrc_pic.iStride[0] >> 1;
    //videoFormatYUY2
    //enc_input_size = esrc_pic.iPicWidth *esrc_pic.iPicHeight * 3 >> 1;

    /* Initialize encoder */
    rc = m_encoder->InitializeExt(&eprm);
    if (rc != cmResultSuccess)
    {
        printf("SVC encoder Initialize failed, rc=%d", rc);
        return -1;
    }

    int videoFormat = m_esrc_pic.iColorFormat;
    rc = m_encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);
    if (rc != cmResultSuccess)
    {
        printf("SVC encoder SetOption videoFormat failed, "
               "rc=%d",
               rc);
        return -1;
    }
    return 0;
}

int h264Encoder::yuyv_yuv422_to_yuv420(const unsigned char yuv422[], unsigned char yuv420[], const int width, const int height)
{
    int ynum = width * height;
    int i, j, k = 0;
    //得到Y分量
    for (i = 0; i < ynum; i++)
    {
        yuv420[i] = yuv422[i * 2];
    }
    //得到U分量
    for (i = 0; i < height; i++)
    {
        if ((i % 2) != 0)
            continue;
        for (j = 0; j < (width / 2); j++)
        {
            if ((4 * j + 1) > (2 * width))
                break;
            yuv420[ynum + k * 2 * width / 4 + j] = yuv422[i * 2 * width + 4 * j + 1];
        }
        k++;
    }
    k = 0;
    //得到V分量
    for (i = 0; i < height; i++)
    {
        if ((i % 2) == 0)
            continue;
        for (j = 0; j < (width / 2); j++)
        {
            if ((4 * j + 3) > (2 * width))
                break;
            yuv420[ynum + ynum / 4 + k * 2 * width / 4 + j] = yuv422[i * 2 * width + 4 * j + 3];
        }
        k++;
    }
    return 1;
}

int h264Encoder::openh264_encoder_input(const unsigned char *frame, const int len,const  bool keyframe)
{
    int rc = 0;
    //if (opt && opt->force_keyframe) {
    //enc->ForceIntraFrame(true);
    //}
    if(m_esrc_pic.iColorFormat != v4l2_pixlfmt_h264(this->m_dfmt))
    {
        if(m_yuv420 == nullptr)
        {
            m_yuv420 = new unsigned char [len + 50];
        }
        if(m_yuv420 != nullptr)
            yuyv_yuv422_to_yuv420((unsigned char*)frame, m_yuv420, m_width, m_height);
        m_esrc_pic.pData[0] = (unsigned char *)m_yuv420;
    }
    else
    {
        m_esrc_pic.pData[0] = (unsigned char *)frame;
    }

    m_esrc_pic.iPicWidth = m_width;
    m_esrc_pic.iPicHeight = m_height;
    if (m_esrc_pic.iColorFormat == videoFormatI420)
    {
        m_esrc_pic.pData[1] = m_esrc_pic.pData[0] + (m_esrc_pic.iPicWidth * m_esrc_pic.iPicHeight);
        m_esrc_pic.pData[2] = m_esrc_pic.pData[1] + (m_esrc_pic.iPicWidth * m_esrc_pic.iPicHeight >> 2);
    }
    memset(&m_sfbsinfo, 0, sizeof(SFrameBSInfo));
    rc = m_encoder->EncodeFrame(&m_esrc_pic, &m_sfbsinfo);
    if (rc != cmResultSuccess)
    {
        printf("EncodeFrame() error, ret: %d", rc);
        return -1;
    }

    if (m_sfbsinfo.eFrameType == videoFrameTypeSkip)
    {
        /*
	    output->size = 0;
	    output->type = PJMEDIA_FRAME_TYPE_NONE;
	    output->timestamp = input->timestamp;
        */
        return 0;
    }

    //ets = input->timestamp;
    m_ilayer = 0;
    int layer_size[MAX_LAYER_NUM_OF_FRAME] = {0};
    //enc_frame_size = enc_processed = 0;

    for (m_ilayer = 0; m_ilayer < m_sfbsinfo.iLayerNum; m_ilayer++)
    {
        for (int i = 0; i < m_sfbsinfo.sLayerInfo[m_ilayer].iNalCount; i++)
            layer_size[m_ilayer] += m_sfbsinfo.sLayerInfo[m_ilayer].pNalLengthInByte[i];
        m_out_size += layer_size[m_ilayer];
    }
    /*
    if ((ret = ff_alloc_packet2(avctx, avpkt, size, size))) {
        av_log(avctx, AV_LOG_ERROR, "Error getting output packet\n");
        return ret;
    }
    */
    if (m_out_frame_payload == nullptr)
        m_out_frame_payload = new char[m_out_size + 1];
    m_out_size = 0;
    for (m_ilayer = 0; m_ilayer < m_sfbsinfo.iLayerNum; m_ilayer++)
    {
        memcpy(m_out_frame_payload + m_out_size, m_sfbsinfo.sLayerInfo[m_ilayer].pBsBuf, layer_size[m_ilayer]);
        m_out_size += layer_size[m_ilayer];
    }
    //avpkt->pts = frame->pts;
    //if ( m_sfbsinfo.eFrameType == videoFrameTypeIDR)
    //    avpkt->flags |= AV_PKT_FLAG_KEY;
    //*got_packet = 1;
    return m_out_size;
    /*
    if (whole) 
    {
        SLayerBSInfo* pLayerBsInfo;
        unsigned i = 0;

        // Find which layer with biggest payload 
        ilayer = 0;
         m_out_size =  m_sfbsinfo.sLayerInfo[0].pNalLengthInByte[0];
        for (i=0; i < (unsigned) m_sfbsinfo.iLayerNum; ++i) 
        {
            unsigned j;
            pLayerBsInfo = &bsi.sLayerInfo[i];
            for (j=0; j < (unsigned)pLayerBsInfo->iNalCount; ++j) 
            {
                if (pLayerBsInfo->pNalLengthInByte[j] > (int) m_out_size) 
                {
                     m_out_size = pLayerBsInfo->pNalLengthInByte[j];
                     m_ilayer = i;
                }
            }
        }

        pLayerBsInfo = & m_sfbsinfo.sLayerInfo[ m_ilayer];
        if (pLayerBsInfo == NULL) 
        {
            //output->size = 0;
            //output->type = PJMEDIA_FRAME_TYPE_NONE;
            return 0;
        }

        payload = pLayerBsInfo->pBsBuf;
         m_out_size = 0;
        for (int inal = pLayerBsInfo->iNalCount - 1; inal >= 0; --inal) {
             m_out_size += pLayerBsInfo->pNalLengthInByte[inal];
        }

        //if ( m_out_size > out_size)
        //    return -1;//PJMEDIA_CODEC_EFRMTOOSHORT;
        
        //output->type = PJMEDIA_FRAME_TYPE_VIDEO;
        //output->size = out_frame_size;
        //output->timestamp = input->timestamp;
        //pj_memcpy(output->buf, payload,  m_out_size);
        
        return 0;
    }
    */
    return -1;
}

int h264Encoder::openh264_encoder_output(unsigned char *frame, const int len)
{
    if (m_out_size > 0 && m_out_frame_payload != nullptr)
    {
        int ret = (m_out_size > len) ? len : m_out_size;
        memmove(frame, m_out_frame_payload, ret);
        m_out_size = 0;
        return ret;
    }
    return -1;
}
#endif



int openh264_test()
{
    unsigned char *buf;
    int rsize = 0;
    FILE *fp = fopen("bus_cif.yuv", "r");
    FILE *rfp = fopen("aa.h264", "ab");
    h264Encoder *h264Core = new h264Encoder();
    rsize = 352 * 288 * 3;
    buf = new unsigned char [rsize];
    h264Core->videoEncoderSetup(352, 288, V4L2_PIX_FMT_YUV420/*V4L2_PIX_FMT_YUYV*/, 30);
    int ret = 0;
    while(1)
    {
        ret = fread(buf, rsize, 1, fp);
        ret = h264Core->videoEncoderInput((unsigned char *)buf, ret, false);
        if (ret > 0)
        {
            
            fwrite((char *)h264Core->videoEncoderOutput(), h264Core->videoEncoderOutputSize(true), 1, rfp);
        }
        if(feof(fp)!=0)
            break;
    }
    fflush(rfp);
    fclose(fp);
    fclose(rfp);
    delete [] buf;
    delete h264Core;
    return 0;
}







#if 0
/**
 * 最简单的基于X264的视频编码器
 * Simplest X264 Encoder
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序可以YUV格式的像素数据编码为H.264码流，是最简单的
 * 基于libx264的视频编码器
 *
 * This software encode YUV data to H.264 bitstream.
 * It's the simplest encoder example based on libx264.
 */
#include <stdio.h>
#include <stdlib.h>

#include "stdint.h"

#if defined(__cplusplus)
extern "C"
{
#include "x264.h"
};
#else
#include "x264.h"
#endif
 
 
int main(int argc, char** argv)
{
 
         int ret;
         int y_size;
         int i,j;
 
         //FILE* fp_src  = fopen("../cuc_ieschool_640x360_yuv444p.yuv", "rb");
         FILE* fp_src  = fopen("../cuc_ieschool_640x360_yuv420p.yuv", "rb");
 
         FILE* fp_dst = fopen("cuc_ieschool.h264", "wb");
        
         //Encode 50 frame
         //if set 0, encode all frame
         int frame_num=50;
         int csp=X264_CSP_I420;
         int width=640,height=360;
 
         int iNal   = 0;
         x264_nal_t* pNals = NULL;
         x264_t* pHandle   = NULL;
         x264_picture_t* pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
         x264_picture_t* pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
         x264_param_t* pParam = (x264_param_t*)malloc(sizeof(x264_param_t));
        
         //Check
         if(fp_src==NULL||fp_dst==NULL){
                   printf("Error open files.\n");
                   return -1;
         }
 
         x264_param_default(pParam);
         pParam->i_width   = width;
         pParam->i_height  = height;
         /*
         //Param
         pParam->i_log_level  = X264_LOG_DEBUG;
         pParam->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;
         pParam->i_frame_total = 0;
         pParam->i_keyint_max = 10;
         pParam->i_bframe  = 5;
         pParam->b_open_gop  = 0;
         pParam->i_bframe_pyramid = 0;
         pParam->rc.i_qp_constant=0;
         pParam->rc.i_qp_max=0;
         pParam->rc.i_qp_min=0;
         pParam->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
         pParam->i_fps_den  = 1;
         pParam->i_fps_num  = 25;
         pParam->i_timebase_den = pParam->i_fps_num;
         pParam->i_timebase_num = pParam->i_fps_den;
         */
         pParam->i_csp=csp;
         x264_param_apply_profile(pParam, x264_profile_names[5]);
        
         pHandle = x264_encoder_open(pParam);
   
         x264_picture_init(pPic_out);
         x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height);
 
         //ret = x264_encoder_headers(pHandle, &pNals, &iNal);
 
         y_size = pParam->i_width * pParam->i_height;
         //detect frame number
         if(frame_num==0){
                   fseek(fp_src,0,SEEK_END);
                   switch(csp){
                   case X264_CSP_I444:frame_num=ftell(fp_src)/(y_size*3);break;
                   case X264_CSP_I420:frame_num=ftell(fp_src)/(y_size*3/2);break;
                   default:printf("Colorspace Not Support.\n");return -1;
                   }
                   fseek(fp_src,0,SEEK_SET);
         }
        
         //Loop to Encode
         for( i=0;i<frame_num;i++){
                   switch(csp){
                   case X264_CSP_I444:{
                            fread(pPic_in->img.plane[0],y_size,1,fp_src);         //Y
                            fread(pPic_in->img.plane[1],y_size,1,fp_src);         //U
                            fread(pPic_in->img.plane[2],y_size,1,fp_src);         //V
                            break;}
                   case X264_CSP_I420:{
                            fread(pPic_in->img.plane[0],y_size,1,fp_src);         //Y
                            fread(pPic_in->img.plane[1],y_size/4,1,fp_src);     //U
                            fread(pPic_in->img.plane[2],y_size/4,1,fp_src);     //V
                            break;}
                   default:{
                            printf("Colorspace Not Support.\n");
                            return -1;}
                   }
                   pPic_in->i_pts = i;
 
                   ret = x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out);
                   if (ret< 0){
                            printf("Error.\n");
                            return -1;
                   }
 
                   printf("Succeed encode frame: %5d\n",i);
 
                   for ( j = 0; j < iNal; ++j){
                             fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);
                   }
         }
         i=0;
         //flush encoder
         while(1){
                   ret = x264_encoder_encode(pHandle, &pNals, &iNal, NULL, pPic_out);
                   if(ret==0){
                            break;
                   }
                   printf("Flush 1 frame.\n");
                   for (j = 0; j < iNal; ++j){
                            fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);
                   }
                   i++;
         }
         x264_picture_clean(pPic_in);
         x264_encoder_close(pHandle);
         pHandle = NULL;
 
         free(pPic_in);
         free(pPic_out);
         free(pParam);
 
         fclose(fp_src);
         fclose(fp_dst);
 
         return 0;
}
#endif