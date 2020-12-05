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




class h264Decoder {
    public:
        h264Decoder();
        ~h264Decoder();
        int h264DecoderSetup(int width, int height, int fmt, int fps);
        int h264DecoderInput(char *frame, int len);
        int h264DecoderOutput(char *frame, int len);
        unsigned char * h264DecoderOutput();
        int h264DecoderOutputSize(bool clear=true);
        int h264DecoderDestroy();

    private:
#ifdef PL_OPENH264_MODULE
        int openh264_decoder_destroy();
        int openh264_decoder_setup(int width, int height, int fmt, int fps);
        int openh264_decoder_input(char *frame, int len);
        int openh264_decoder_output(char *frame, int len);
        int openh264_write_yuv(unsigned char *buf,
                     unsigned dst_len,
                     unsigned char* pData[3],
                     int iStride[2],
                     int iWidth,
                     int iHeight);
        int  openh264_got_decoded_frame(
					   unsigned char *pData[3],
					   SBufferInfo *sDstBufInfo,
					   uint64_t *timestamp,
					   unsigned out_size,
					   char *output);
#endif
    private:
        int v_width, v_height;
        unsigned m_out_frame_size = 0;
#ifdef PL_OPENH264_MODULE
        /* Encoder state */
        ISVCDecoder *m_decoder = nullptr;
        char *m_out_frame_payload = nullptr;
#endif
};


#endif /* __H264_DECODER_HPP__ */