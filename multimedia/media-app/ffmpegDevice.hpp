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
#ifndef __FFMPEG_DEVICE_HPP__
#define __FFMPEG_DEVICE_HPP__

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

class ffmpegDevice {
    public:
        ffmpegDevice();
        int ffmpegDeviceInit(int width, int height, int fps);
        int ffmpegDeviceOpen(const char *inputFormat, const char *ul);
        int ffmpegDeviceDecoderOpen();
        int ffmpegDeviceReady();
        int ffmpegDeviceGetFrame();
        int ffmpegDeviceDecoderFrame();
        int ffmpegDeviceDecoderFrameFinish();
        int ffmpegDeviceDestroy();

        AVFrame             *pFrameYUV = nullptr;//申请AVFrame，用于yuv视频
    private:
        int v_width, v_height, v_fps;

        AVFormatContext    *m_AVInputCtx = nullptr;
        AVCodecContext     *m_DecoderCodecCtx = nullptr;
        AVCodec            *m_DecoderCodec = nullptr;


        AVFrame             *m_raw_frame = nullptr;  //申请AVFrame，用于原始视频

        struct SwsContext   *m_ImgConvertCtx = nullptr;

        AVPacket *dec_pkt = nullptr;
        int     framecnt, enc_count;
        int     videoindex;
};


#endif /* __FFMPEG_DEVICE_HPP__ */