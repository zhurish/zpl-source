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
#ifdef __cplusplus
extern "C" {
#endif
#include "ospl_type.h"
#ifdef __cplusplus
}
#endif
class h264Decoder:public videoDecoder {
    public:
        h264Decoder();
        ~h264Decoder();
        int videoDecoderSetup(const int width, const int height, const int fmt, const int fps);
        int videoDecoderInput(const ospl_uint8 *frame, const int len);
        int videoDecoderOutput(ospl_uint8 *frame, const int len);
        ospl_uint8 * videoDecoderOutput();
        int videoDecoderOutputSize(const bool clear=true);
        int videoDecoderDestroy();

    private:
#ifdef PL_OPENH264_MODULE
        int openh264_decoder_destroy();
        int openh264_decoder_setup(const int width, const int height, const int fmt, const int fps);
        int openh264_decoder_input(const ospl_uint8 *frame, const int len);
        int openh264_decoder_output( ospl_uint8 *frame, const int len);
        int openh264_write_yuv(ospl_uint8 *buf,
                     const int dst_len,
                     const ospl_uint8* pData[3],
                     const int *iStride,
                     const int iWidth,
                     const int iHeight);
        int  openh264_got_decoded_frame(
					   const ospl_uint8 *pData[3],
					   const SBufferInfo *sDstBufInfo,
					   uint64_t *timestamp);
#endif
    private:

#ifdef PL_OPENH264_MODULE
        /* Encoder state */
        ISVCDecoder *m_decoder = nullptr;
        ospl_uint8 *m_out_frame_payload = nullptr;
#endif
};


#endif /* __H264_DECODER_HPP__ */