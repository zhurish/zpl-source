/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** h264Decoder.h
** 
** RTSP server
**
** -------------------------------------------------------------------------*/
#ifndef __H264_DECODER_HPP__
#define __H264_DECODER_HPP__


#include "h264_config.hpp"


#include "videoDecoder.hpp"

class h264Decoder:public videoDecoder {
    public:
        h264Decoder();
        ~h264Decoder();
        int videoDecoderSetup(const int width, const int height, const int fmt, const int fps);
        int videoDecoderInput(const unsigned char *frame, const int len);
        int videoDecoderOutput(unsigned char *frame, const int len);
        unsigned char * videoDecoderOutput();
        int videoDecoderOutputSize(const bool clear=true);
        int videoDecoderDestroy();

    private:
#ifdef PL_OPENH264_MODULE
        int openh264_decoder_destroy();
        int openh264_decoder_setup(const int width, const int height, const int fmt, const int fps);
        int openh264_decoder_input(const unsigned char *frame, const int len);
        int openh264_decoder_output( unsigned char *frame, const int len);
        int openh264_write_yuv(unsigned char *buf,
                     const int dst_len,
                     const unsigned char* pData[3],
                     const int *iStride,
                     const int iWidth,
                     const int iHeight);
        int  openh264_got_decoded_frame(
					   const unsigned char *pData[3],
					   const SBufferInfo *sDstBufInfo,
					   uint64_t *timestamp);
#endif
    private:

#ifdef PL_OPENH264_MODULE
        /* Encoder state */
        ISVCDecoder *m_decoder = nullptr;
        unsigned char *m_out_frame_payload = nullptr;
#endif
};


#endif /* __H264_DECODER_HPP__ */