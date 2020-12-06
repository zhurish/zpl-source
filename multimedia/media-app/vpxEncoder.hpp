/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** vpxEncoder.h
** 
**
** -------------------------------------------------------------------------*/
#ifndef __VPX_ENCODER_HPP__
#define __VPX_ENCODER_HPP__

/* VPX */
extern "C"
{
#include <vpx/vpx_encoder.h>
#include <vpx/vpx_decoder.h>
#include <vpx/vp8cx.h>
#include <vpx/vp8dx.h>
};
#include "videoEncoder.hpp"

#define VIDEO_FORMAT_VP8  1
#define VIDEO_FORMAT_VP9  2

#define DEFAULT_FPS 15
#define DEFAULT_AVG_BITRATE 200000
#define DEFAULT_MAX_BITRATE 200000
#define MAX_RX_RES 1200


class vpxEncoder : public videoEncoder {
    public:
        vpxEncoder();
        ~vpxEncoder();

        int videoEncoderSetup(const int width, const int height, const int fmt, const int fps);
        int videoEncoderInput(const unsigned char *frame, const int len, const bool keyframe);
        int videoEncoderOutput(unsigned char *frame, const int len);
        unsigned char * videoEncoderOutput();
        int videoEncoderOutputSize(const bool clear=true);
        int videoEncoderDestroy();

    private:
        int vpx_encode_framed(const unsigned char *packets, const int in_size,
                                  unsigned char *output, unsigned out_size, bool force_keyframe);
    private:

        /* Encoder */
        vpx_codec_ctx_t enc;
        vpx_codec_iter_t enc_iter;
        unsigned enc_input_size;
        unsigned char *enc_frame_whole;
        unsigned enc_frame_size;
        unsigned enc_processed;
        bool enc_frame_is_keyframe;
        uint64_t ets;
};


#endif /* __VPX_ENCODER_HPP__ */