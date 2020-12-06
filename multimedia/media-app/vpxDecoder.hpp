/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** vpxDecoder.h
** 
**
** -------------------------------------------------------------------------*/
#ifndef __VPX_DECODER_HPP__
#define __VPX_DECODER_HPP__

/* VPX */
extern "C"
{
#include <vpx/vpx_encoder.h>
#include <vpx/vpx_decoder.h>
#include <vpx/vp8cx.h>
#include <vpx/vp8dx.h>
};

#include "videoDecoder.hpp"

class vpxDecoder:public videoDecoder {
    public:
        vpxDecoder();
        ~vpxDecoder();
        int videoDecoderSetup(const int width, const int height, const int fmt, const int fps);
        int videoDecoderInput(const unsigned char *frame, const int len);
        int videoDecoderOutput(unsigned char *frame, const int len);
        unsigned char * videoDecoderOutput();
        int videoDecoderOutputSize(const bool clear=true);
        int videoDecoderDestroy();


    private:
        int vpx_decode_framed(const unsigned char *packets, const int in_size,
                                  unsigned char *output, unsigned out_size);
    private:
        /* Decoder */

        vpx_codec_ctx_t 		dec;
        unsigned char			*dec_buf;
        unsigned			    dec_buf_size;
};


#endif /* __VPX_DECODER_HPP__ */