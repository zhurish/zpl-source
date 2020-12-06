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

class h264Encoder  : public videoEncoder {
    public:
        h264Encoder();
        ~h264Encoder();

        int videoEncoderSetup(const int width, const int height, const int fmt, const int fps);
        int videoEncoderInput(const unsigned char *frame, const int len, const bool keyframe);
        int videoEncoderOutput(unsigned char *frame, const int len);
        unsigned char * videoEncoderOutput();
        int videoEncoderOutputSize(const bool clear=true);
        int videoEncoderDestroy();

#ifdef PL_LIBX264_MODULE
        x264_nal_t *m_h264_nal = nullptr;
#endif
    private:
#ifdef PL_LIBX264_MODULE
        int h264_encoder_setup(const int width, const int height, const int fmt, const int fps);
        int h264_encoder_input(const unsigned char *frame, const int len, const bool keyframe);
        int h264_encoder_output(unsigned char *frame, const int len);
#endif
#ifdef PL_OPENH264_MODULE
        int openh264_encoder_destroy();
        int openh264_encoder_setup(const int width, const int height, const int fmt, const int fps);
        int openh264_encoder_input(const unsigned char *frame, const int len, const bool keyframe);
        int openh264_encoder_output(unsigned char *frame, const int len);
        int yuyv_yuv422_to_yuv420(const unsigned char yuv422[], unsigned char yuv420[], const int width, const int height);
#endif
    private:

        int m_dfmt = 0;
#ifdef PL_OPENH264_MODULE
        /* Encoder state */
        ISVCEncoder		*m_encoder = nullptr;
        SSourcePicture		m_esrc_pic;
        //unsigned		enc_input_size;
        //unsigned char		*enc_frame_whole;
        //unsigned		enc_frame_size;
        //unsigned		enc_processed;
        //pj_timestamp		ets;
        SFrameBSInfo		m_sfbsinfo;
        int			m_ilayer = 0;
        //unsigned int		iDLayerQp;
        //SSliceArgument	sSliceArgument;
        char *m_out_frame_payload = nullptr;
        unsigned char *m_yuv420 = nullptr;
#endif
#ifdef PL_LIBX264_MODULE
        x264_param_t m_param;
        x264_picture_t m_pic_in;
        x264_t *m_h264 = nullptr;
        int m_frame_size = 0;
        int m_plane_0 = 0;
        int m_plane_1 = 0;
        int m_plane_2 = 0;
#endif
};

int openh264_test();

#endif /* __H264_ENCODER_HPP__ */