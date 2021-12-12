/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** h264Encoder.h
** 
**
** -------------------------------------------------------------------------*/
#ifndef __H264_ENCODER_HPP__
#define __H264_ENCODER_HPP__

#include "videoEncoder.hpp"

#include "h264_config.hpp"
#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_type.h"
#ifdef __cplusplus
}
#endif
class h264Encoder  : public videoEncoder {
    public:
        h264Encoder();
        ~h264Encoder();

        int videoEncoderSetup(const int width, const int height, const int fmt, const int fps);
        int videoEncoderInput(const zpl_uint8 *frame, const int len, const bool keyframe);
        int videoEncoderOutput(zpl_uint8 *frame, const int len);
        zpl_uint8 * videoEncoderOutput();
        int videoEncoderOutputSize(const bool clear=true);
        int videoEncoderDestroy();

#ifdef ZPL_LIBX264_MODULE
        x264_nal_t *m_h264_nal = NULL;
#endif
    private:
#ifdef ZPL_LIBX264_MODULE
        int h264_encoder_setup(const int width, const int height, const int fmt, const int fps);
        int h264_encoder_input(const zpl_uint8 *frame, const int len, const bool keyframe);
        int h264_encoder_output(zpl_uint8 *frame, const int len);
#endif
#ifdef ZPL_OPENH264_MODULE
        int openh264_encoder_destroy();
        int openh264_encoder_setup(const int width, const int height, const int fmt, const int fps);
        int openh264_encoder_input(const zpl_uint8 *frame, const int len, const bool keyframe);
        int openh264_encoder_output(zpl_uint8 *frame, const int len);
        int yuyv_yuv422_to_yuv420(const zpl_uint8 yuv422[], zpl_uint8 yuv420[], const int width, const int height);
#endif
    private:

        int m_dfmt = 0;
#ifdef ZPL_OPENH264_MODULE
        /* Encoder state */
        ISVCEncoder		*m_encoder = NULL;
        SSourcePicture		m_esrc_pic;
        //unsigned		enc_input_size;
        //zpl_uint8		*enc_frame_whole;
        //unsigned		enc_frame_size;
        //unsigned		enc_processed;
        //pj_timestamp		ets;
        SFrameBSInfo		m_sfbsinfo;
        int			m_ilayer = 0;
        //zpl_uint32		iDLayerQp;
        //SSliceArgument	sSliceArgument;
        char *m_out_frame_payload = NULL;
        zpl_uint8 *m_yuv420 = NULL;
#endif
#ifdef ZPL_LIBX264_MODULE
        x264_param_t m_param;
        x264_picture_t m_pic_in;
        x264_t *m_h264 = NULL;
        int m_frame_size = 0;
        int m_plane_0 = 0;
        int m_plane_1 = 0;
        int m_plane_2 = 0;
#endif
};

int openh264_test();

#endif /* __H264_ENCODER_HPP__ */