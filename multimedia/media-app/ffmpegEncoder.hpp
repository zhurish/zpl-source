/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** ffmpegEncoder.h
** 
** ffmpegEncoder
**
** -------------------------------------------------------------------------*/
#ifndef __FFMPEG_ENCODER_HPP__
#define __FFMPEG_ENCODER_HPP__


//#define FFMPEG_ENCODE_OUTPUT_FILE

extern "C"
{
#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "libavutil/mathematics.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/imgutils.h"
};
#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_type.h"
#ifdef __cplusplus
}
#endif


class ffmpegEncoder{
    public:
        ffmpegEncoder();
        int ffmpegEncoderInit(int width, int height, int fmt, int fps);
        int ffmpegEncoderOpen(int enc/*, std::function <int(void*, int)> Callback*/);
        int ffmpegEncoderFrame(AVFrame *input, AVPacket *out);
        int ffmpegEncoderFrameFinish();
        int ffmpegEncoderDestroy();

    private:
        int m_width;	
        int m_height;
        int m_fmt;
        int m_fps;

        AVFormatContext    *m_AVOutputCtx = nullptr;
        AVCodecContext     *m_CodecCtx = nullptr;
        AVCodec            *m_Codec = nullptr;

        AVOutputFormat      *m_OutputFmt = nullptr;
	    AVStream            *m_video_st = nullptr;

        int     framecnt, enc_count;
        int     videoindex;
};


#endif /* __FFMPEG_ENCODER_HPP__ */