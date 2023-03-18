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

#define ZPL_AUDIO_FRAMERATE_DEFAULT ZPL_AUDIO_FRAMERATE_50
#define ZPL_VIDEO_FRAMERATE_DEFAULT ZPL_VIDEO_FRAMERATE_30


const char *zpl_media_format_name(int key);
/* 获取视频分辨率大小 resolution ratio */
int zpl_media_video_format_resolution(ZPL_VIDEO_FORMAT_E format, zpl_video_size_t* pstSize);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_FORMAT_H__ */
