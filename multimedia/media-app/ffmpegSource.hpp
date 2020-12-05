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
#ifndef __FFMPEG_SOURCE_HPP__
#define __FFMPEG_SOURCE_HPP__

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

#include "ffmpegDevice.hpp"
#include "ffmpegEncoder.hpp"


class ffmpegSource {
    public:
        ffmpegSource();
        ~ffmpegSource() = default;
        int ffmpegSourceInit();
        int doGetFrameDataSize();
        int doGetFrameData(void *p, int size);
        int doGetFrame();
        void doGetFrameDataFree();
        int ffmpegSourceDestroy();
        AVPacket enc_pkt;
    private:
		//int doEncoderFrameData();	
        ffmpegDevice    *ffmpeg_Device = nullptr;
        ffmpegEncoder   *ffmpeg_Encode = nullptr;
        bool    good_frame = false;
        bool    init_flag = false;
       
};


#endif /* __FFMPEG_SOURCE_HPP__ */