/*
 * zpl_media_format.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_FORMAT_H__
#define __ZPL_MEDIA_FORMAT_H__


#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
	ZPL_VIDEO_FORMAT_NONE,
	ZPL_VIDEO_FORMAT_CIF,
    ZPL_VIDEO_FORMAT_360P,      /* 640 * 360 */
    ZPL_VIDEO_FORMAT_D1_PAL,    /* 720 * 576 */
    ZPL_VIDEO_FORMAT_D1_NTSC,   /* 720 * 480 */
    ZPL_VIDEO_FORMAT_720P,      /* 1280 * 720  */
    ZPL_VIDEO_FORMAT_1080P,     /* 1920 * 1080 */
    ZPL_VIDEO_FORMAT_2560x1440,
    ZPL_VIDEO_FORMAT_2592x1520,
    ZPL_VIDEO_FORMAT_2592x1536,
    ZPL_VIDEO_FORMAT_2592x1944,
    ZPL_VIDEO_FORMAT_2688x1536,
    ZPL_VIDEO_FORMAT_2716x1524,
    ZPL_VIDEO_FORMAT_3840x2160,
    ZPL_VIDEO_FORMAT_4096x2160,
    ZPL_VIDEO_FORMAT_3000x3000,
    ZPL_VIDEO_FORMAT_4000x3000,
    ZPL_VIDEO_FORMAT_7680x4320,
    ZPL_VIDEO_FORMAT_3840x8640,
    
	ZPL_VIDEO_FORMAT_640X480,

	ZPL_VIDEO_FORMAT_MAX,
	ZPL_VIDEO_FORMAT_BUTT = ZPL_VIDEO_FORMAT_MAX,
} ZPL_VIDEO_FORMAT_E;

typedef enum
{
    ZPL_VIDEO_FRAMERATE_25 = 25,
    ZPL_VIDEO_FRAMERATE_30 = 30,
    ZPL_VIDEO_FRAMERATE_50 = 50,
    ZPL_VIDEO_FRAMERATE_60 = 60,
    ZPL_VIDEO_FRAMERATE_100 = 100,
    ZPL_VIDEO_FRAMERATE_120 = 120,
}ZPL_VIDEO_FRAMERATE_E;

typedef enum
{
    ZPL_AUDIO_FRAMERATE_25 = 25,
    ZPL_AUDIO_FRAMERATE_30 = 30,
    ZPL_AUDIO_FRAMERATE_50 = 50,
    ZPL_AUDIO_FRAMERATE_60 = 60,
    ZPL_AUDIO_FRAMERATE_100 = 100,
    ZPL_AUDIO_FRAMERATE_120 = 120,

}ZPL_AUDIO_FRAMERATE_E;


typedef enum  {
    ZPL_AUDIO_SAMPLE_RATE_8000   = 8000,    /* 8K samplerate */
    ZPL_AUDIO_SAMPLE_RATE_12000  = 12000,   /* 12K samplerate */
    ZPL_AUDIO_SAMPLE_RATE_11025  = 11025,   /* 11.025K samplerate */
    ZPL_AUDIO_SAMPLE_RATE_16000  = 16000,   /* 16K samplerate */
    ZPL_AUDIO_SAMPLE_RATE_22050  = 22050,   /* 22.050K samplerate */
    ZPL_AUDIO_SAMPLE_RATE_24000  = 24000,   /* 24K samplerate */
    ZPL_AUDIO_SAMPLE_RATE_32000  = 32000,   /* 32K samplerate */
    ZPL_AUDIO_SAMPLE_RATE_44100  = 44100,   /* 44.1K samplerate */
    ZPL_AUDIO_SAMPLE_RATE_48000  = 48000,   /* 48K samplerate */
    ZPL_AUDIO_SAMPLE_RATE_64000  = 64000,   /* 64K samplerate */
    ZPL_AUDIO_SAMPLE_RATE_96000  = 96000,   /* 96K samplerate */
    ZPL_AUDIO_SAMPLE_RATE_BUTT,
} ZPL_AUDIO_SAMPLE_RATE_E;

typedef enum  {
    ZPL_AUDIO_BIT_WIDTH_8   = 0,   /* 8bit width */
    ZPL_AUDIO_BIT_WIDTH_16  = 1,   /* 16bit width */
    ZPL_AUDIO_BIT_WIDTH_24  = 2,   /* 24bit width */
    ZPL_AUDIO_BIT_WIDTH_BUTT,
} ZPL_AUDIO_BIT_WIDTH_E;

#define ZPL_VIDEO_FRAMERATE_DEFAULT     ZPL_VIDEO_FRAMERATE_30

#define ZPL_AUDIO_FRAMERATE_DEFAULT     ZPL_AUDIO_FRAMERATE_50
#define ZPL_AUDIO_CLOCK_RATE_DEFAULT    ZPL_AUDIO_SAMPLE_RATE_8000

const char *zpl_media_format_name(int key);
/* 获取视频分辨率大小 resolution ratio */
int zpl_media_video_format_resolution(ZPL_VIDEO_FORMAT_E format, zpl_video_size_t* pstSize);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_FORMAT_H__ */
