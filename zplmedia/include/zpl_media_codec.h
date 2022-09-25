/*
 * zpl_media_codec.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_CODEC_H__
#define __ZPL_MEDIA_CODEC_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_media_format.h"

/* We just coyp this value of payload type from RTP/RTSP definition */
typedef enum {

    ZPL_AUDIO_CODEC_NONE          = -1,
    ZPL_VIDEO_CODEC_NONE          = -1,
    RTP_MEDIA_PAYLOAD_NONE        = -1,
    /* AUDIO */
    ZPL_AUDIO_CODEC_PCMU          = 0,
    ZPL_AUDIO_CODEC_1016          = 1,
    ZPL_AUDIO_CODEC_G721          = 2,
    ZPL_AUDIO_CODEC_GSM           = 3,
    ZPL_AUDIO_CODEC_G723          = 4,
    ZPL_AUDIO_CODEC_DVI4_8K       = 5,
    ZPL_AUDIO_CODEC_DVI4_16K      = 6,
    ZPL_AUDIO_CODEC_LPC           = 7,
    ZPL_AUDIO_CODEC_PCMA          = 8,
    ZPL_AUDIO_CODEC_G722          = 9,
    ZPL_AUDIO_CODEC_S16BE_STEREO  = 10,
    ZPL_AUDIO_CODEC_S16BE_MONO    = 11,
    ZPL_AUDIO_CODEC_QCELP         = 12,
    ZPL_AUDIO_CODEC_CN            = 13,
    ZPL_AUDIO_CODEC_MPEGAUDIO     = 14,
    ZPL_AUDIO_CODEC_G728          = 15,
    ZPL_AUDIO_CODEC_DVI4_3        = 16,
    ZPL_AUDIO_CODEC_DVI4_4        = 17,
    ZPL_AUDIO_CODEC_G729          = 18,
    ZPL_AUDIO_CODEC_G711A         = 19,
    ZPL_AUDIO_CODEC_G711U         = 20,
    ZPL_AUDIO_CODEC_G726          = 21,
    ZPL_AUDIO_CODEC_G729A         = 22,
    ZPL_AUDIO_CODEC_LPCM          = 23,
    ZPL_AUDIO_CODEC_CelB          = 25,
    ZPL_AUDIO_CODEC_JPEG          = 26,
    ZPL_AUDIO_CODEC_CUSM          = 27,
    ZPL_AUDIO_CODEC_NV            = 28,
    ZPL_AUDIO_CODEC_PICW          = 29,
    ZPL_AUDIO_CODEC_CPV           = 30,

    ZPL_AUDIO_CODEC_AMR           = 1000,
    ZPL_AUDIO_CODEC_AMRWB         ,
    ZPL_AUDIO_CODEC_PRORES        ,
    ZPL_AUDIO_CODEC_OPUS          ,
    ZPL_AUDIO_CODEC_D_GSM_HR      ,
    ZPL_AUDIO_CODEC_D_GSM_EFR     ,
    ZPL_AUDIO_CODEC_D_L8          ,
    ZPL_AUDIO_CODEC_D_RED         ,

    /* RTP AUDIO PAYLOAD */
    RTP_MEDIA_PAYLOAD_PCMU        = ZPL_AUDIO_CODEC_PCMU,
    RTP_MEDIA_PAYLOAD_1016        = ZPL_AUDIO_CODEC_1016,
    RTP_MEDIA_PAYLOAD_G721        = ZPL_AUDIO_CODEC_G721,
    RTP_MEDIA_PAYLOAD_GSM         = ZPL_AUDIO_CODEC_GSM,
    RTP_MEDIA_PAYLOAD_G723        = ZPL_AUDIO_CODEC_G723,
    RTP_MEDIA_PAYLOAD_DVI4_8K     = ZPL_AUDIO_CODEC_DVI4_8K,
    RTP_MEDIA_PAYLOAD_DVI4_16K    = ZPL_AUDIO_CODEC_DVI4_16K,
    RTP_MEDIA_PAYLOAD_LPC         = ZPL_AUDIO_CODEC_LPC,
    RTP_MEDIA_PAYLOAD_PCMA        = ZPL_AUDIO_CODEC_PCMA,
    RTP_MEDIA_PAYLOAD_G722        = ZPL_AUDIO_CODEC_G722,
    RTP_MEDIA_PAYLOAD_S16BE_STEREO= ZPL_AUDIO_CODEC_S16BE_STEREO,
    RTP_MEDIA_PAYLOAD_S16BE_MONO  = ZPL_AUDIO_CODEC_S16BE_MONO,
    RTP_MEDIA_PAYLOAD_QCELP       = ZPL_AUDIO_CODEC_QCELP,
    RTP_MEDIA_PAYLOAD_CN          = ZPL_AUDIO_CODEC_CN,
    RTP_MEDIA_PAYLOAD_MPEGAUDIO   = ZPL_AUDIO_CODEC_MPEGAUDIO,
    RTP_MEDIA_PAYLOAD_G728          = ZPL_AUDIO_CODEC_G728,
    RTP_MEDIA_PAYLOAD_DVI4_3        = ZPL_AUDIO_CODEC_DVI4_3,
    RTP_MEDIA_PAYLOAD_DVI4_4        = ZPL_AUDIO_CODEC_DVI4_4,
    RTP_MEDIA_PAYLOAD_G729          = ZPL_AUDIO_CODEC_G729,
    RTP_MEDIA_PAYLOAD_G711A         = ZPL_AUDIO_CODEC_G711A,
    RTP_MEDIA_PAYLOAD_G711U         = ZPL_AUDIO_CODEC_G711U,
    RTP_MEDIA_PAYLOAD_G726          = ZPL_AUDIO_CODEC_G726,
    RTP_MEDIA_PAYLOAD_G729A         = ZPL_AUDIO_CODEC_G729A,
    RTP_MEDIA_PAYLOAD_LPCM          = ZPL_AUDIO_CODEC_LPCM,
    RTP_MEDIA_PAYLOAD_CelB          = ZPL_AUDIO_CODEC_CelB,
    RTP_MEDIA_PAYLOAD_JPEG          = ZPL_AUDIO_CODEC_JPEG,
    RTP_MEDIA_PAYLOAD_CUSM          = ZPL_AUDIO_CODEC_CUSM,
    RTP_MEDIA_PAYLOAD_NV            = ZPL_AUDIO_CODEC_NV,
    RTP_MEDIA_PAYLOAD_PICW          = ZPL_AUDIO_CODEC_PICW,
    RTP_MEDIA_PAYLOAD_CPV           = ZPL_AUDIO_CODEC_CPV,

    RTP_MEDIA_PAYLOAD_AMR          = ZPL_AUDIO_CODEC_AMR,
    RTP_MEDIA_PAYLOAD_AMRWB        = ZPL_AUDIO_CODEC_AMRWB,
    RTP_MEDIA_PAYLOAD_PRORES       = ZPL_AUDIO_CODEC_PRORES,
    RTP_MEDIA_PAYLOAD_OPUS         = ZPL_AUDIO_CODEC_OPUS,
    RTP_MEDIA_PAYLOAD_D_GSM_HR     = ZPL_AUDIO_CODEC_D_GSM_HR,
    RTP_MEDIA_PAYLOAD_D_GSM_EFR    = ZPL_AUDIO_CODEC_D_GSM_EFR,
    RTP_MEDIA_PAYLOAD_D_L8         = ZPL_AUDIO_CODEC_D_L8,
    RTP_MEDIA_PAYLOAD_D_RED        = ZPL_AUDIO_CODEC_D_RED,


    /* VIDEO */
    ZPL_VIDEO_CODEC_H261          = 31,
    ZPL_VIDEO_CODEC_MPEGVIDEO     = 32,
    ZPL_VIDEO_CODEC_MPEG2TS       = 33,
    ZPL_VIDEO_CODEC_H263          = 34,
    ZPL_VIDEO_CODEC_SPEG          = 35,
    ZPL_VIDEO_CODEC_MPEG2VIDEO    = 36,
    ZPL_VIDEO_CODEC_AAC           = 37,
    ZPL_VIDEO_CODEC_WMA9STD       = 38,
    ZPL_VIDEO_CODEC_HEAAC         = 39,
    ZPL_VIDEO_CODEC_PCM_VOICE     = 40,
    ZPL_VIDEO_CODEC_PCM_AUDIO     = 41,
    ZPL_VIDEO_CODEC_MP3           = 43,
    ZPL_VIDEO_CODEC_ADPCMA        = 49,
    ZPL_VIDEO_CODEC_AEC           = 50,
    ZPL_VIDEO_CODEC_X_LD          = 95,
    ZPL_VIDEO_CODEC_H264          = 96,
    ZPL_VIDEO_CODEC_JPEG          = 97,
    
    ZPL_VIDEO_CODEC_D_VDVI        = 3000,
    ZPL_VIDEO_CODEC_D_BT656       ,
    ZPL_VIDEO_CODEC_D_H263_1998   ,
    ZPL_VIDEO_CODEC_D_MP1S        ,
    ZPL_VIDEO_CODEC_D_MP2P        ,
    ZPL_VIDEO_CODEC_D_BMPEG       ,

    ZPL_VIDEO_CODEC_MJPEG         = 4000,
    ZPL_VIDEO_CODEC_MP4VIDEO      ,
    ZPL_VIDEO_CODEC_MP4AUDIO      ,
    ZPL_VIDEO_CODEC_VC1           ,
    ZPL_VIDEO_CODEC_JVC_ASF       ,
    ZPL_VIDEO_CODEC_D_AVI         ,
    ZPL_VIDEO_CODEC_DIVX3         ,
    ZPL_VIDEO_CODEC_AVS           ,
    ZPL_VIDEO_CODEC_REAL8         ,
    ZPL_VIDEO_CODEC_REAL9         ,
    ZPL_VIDEO_CODEC_VP6           ,

    ZPL_VIDEO_CODEC_VP6F          ,
    ZPL_VIDEO_CODEC_VP6A          ,
    ZPL_VIDEO_CODEC_SORENSON      ,
    ZPL_VIDEO_CODEC_H265          ,
    ZPL_VIDEO_CODEC_VP8           ,
    ZPL_VIDEO_CODEC_VP9           ,
    ZPL_VIDEO_CODEC_MVC           ,
    ZPL_VIDEO_CODEC_PNG           ,

    /* RTP VIDEO PAYLOAD */
    RTP_MEDIA_PAYLOAD_H261          = ZPL_VIDEO_CODEC_H261,
    RTP_MEDIA_PAYLOAD_MPEGVIDEO     = ZPL_VIDEO_CODEC_MPEGVIDEO,
    RTP_MEDIA_PAYLOAD_MPEG2TS       = ZPL_VIDEO_CODEC_MPEG2TS,
    RTP_MEDIA_PAYLOAD_H263          = ZPL_VIDEO_CODEC_H263,
    RTP_MEDIA_PAYLOAD_SPEG          = ZPL_VIDEO_CODEC_SPEG,
    RTP_MEDIA_PAYLOAD_MPEG2VIDEO    = ZPL_VIDEO_CODEC_MPEG2VIDEO,
    RTP_MEDIA_PAYLOAD_AAC           = ZPL_VIDEO_CODEC_AAC,
    RTP_MEDIA_PAYLOAD_WMA9STD       = ZPL_VIDEO_CODEC_WMA9STD,
    RTP_MEDIA_PAYLOAD_HEAAC         = ZPL_VIDEO_CODEC_HEAAC,
    RTP_MEDIA_PAYLOAD_PCM_VOICE     = ZPL_VIDEO_CODEC_PCM_VOICE,
    RTP_MEDIA_PAYLOAD_PCM_AUDIO     = ZPL_VIDEO_CODEC_PCM_AUDIO,
    RTP_MEDIA_PAYLOAD_MP3           = ZPL_VIDEO_CODEC_MP3,
    RTP_MEDIA_PAYLOAD_ADPCMA        = ZPL_VIDEO_CODEC_ADPCMA,
    RTP_MEDIA_PAYLOAD_AEC           = ZPL_VIDEO_CODEC_H261,
    RTP_MEDIA_PAYLOAD_X_LD          = ZPL_VIDEO_CODEC_X_LD,
    RTP_MEDIA_PAYLOAD_H264          = ZPL_VIDEO_CODEC_H264,
    RTP_MEDIA_PAYLOAD_D_VDVI        = ZPL_VIDEO_CODEC_D_VDVI,
    RTP_MEDIA_PAYLOAD_BT656         = ZPL_VIDEO_CODEC_D_BT656,
    RTP_MEDIA_PAYLOAD_H263_1998     = ZPL_VIDEO_CODEC_D_H263_1998,
    RTP_MEDIA_PAYLOAD_MP1S          = ZPL_VIDEO_CODEC_D_MP1S,
    RTP_MEDIA_PAYLOAD_D_MP2P        = ZPL_VIDEO_CODEC_D_MP2P,
    RTP_MEDIA_PAYLOAD_D_BMPEG       = ZPL_VIDEO_CODEC_D_BMPEG,
    RTP_MEDIA_PAYLOAD_MJPEG         = ZPL_VIDEO_CODEC_MJPEG,
    RTP_MEDIA_PAYLOAD_MP4VIDEO      = ZPL_VIDEO_CODEC_MP4VIDEO,
    RTP_MEDIA_PAYLOAD_MP4AUDIO      = ZPL_VIDEO_CODEC_MP4AUDIO,
    RTP_MEDIA_PAYLOAD_VC1           = ZPL_VIDEO_CODEC_VC1,
    RTP_MEDIA_PAYLOAD_JVC_ASF       = ZPL_VIDEO_CODEC_JVC_ASF,
    RTP_MEDIA_PAYLOAD_D_AVI         = ZPL_VIDEO_CODEC_D_AVI,
    RTP_MEDIA_PAYLOAD_DIVX3         = ZPL_VIDEO_CODEC_DIVX3,
    RTP_MEDIA_PAYLOAD_AVS           = ZPL_VIDEO_CODEC_AVS,
    RTP_MEDIA_PAYLOAD_REAL8         = ZPL_VIDEO_CODEC_REAL8,
    RTP_MEDIA_PAYLOAD_REAL9         = ZPL_VIDEO_CODEC_REAL9,
    RTP_MEDIA_PAYLOAD_VP6           = ZPL_VIDEO_CODEC_VP6,
    RTP_MEDIA_PAYLOAD_VP6F          = ZPL_VIDEO_CODEC_VP6F,
    RTP_MEDIA_PAYLOAD_VP6A          = ZPL_VIDEO_CODEC_VP6A,
    RTP_MEDIA_PAYLOAD_SORENSON      = ZPL_VIDEO_CODEC_SORENSON,
    RTP_MEDIA_PAYLOAD_H265          = ZPL_VIDEO_CODEC_H265,
    RTP_MEDIA_PAYLOAD_VP8           = ZPL_VIDEO_CODEC_VP8,
    RTP_MEDIA_PAYLOAD_VP9           = ZPL_VIDEO_CODEC_VP9,
    RTP_MEDIA_PAYLOAD_MVC           = ZPL_VIDEO_CODEC_MVC,
    RTP_MEDIA_PAYLOAD_PNG           = ZPL_VIDEO_CODEC_PNG,


    ZPL_AUDIO_CODEC_MAX     = 6000,
    ZPL_VIDEO_CODEC_MAX     = ZPL_AUDIO_CODEC_MAX,
    RTP_MEDIA_PAYLOAD_MAX   = ZPL_AUDIO_CODEC_MAX,
} ZPL_PAYLOAD_TYPE_E, ZPL_VIDEO_CODEC_E, ZPL_AUDIO_CODEC_E, RTP_MEDIA_TYPE_E;


typedef enum
{
	ZPL_VIDEO_CODEC_PROFILE_BASELINE,	// h264/jpeg/mjpeg
	ZPL_VIDEO_CODEC_PROFILE_MAIN,		// h264/h265
	ZPL_VIDEO_CODEC_PROFILE_HIGH,		// h264/h265
	ZPL_VIDEO_CODEC_PROFILE_SVC,		// h264
} ZPL_VIDEO_CODEC_PROFILE_E;

typedef enum
{
	ZPL_BIT_RATE_NONE,
	ZPL_BIT_RATE_CBR,		//固定码率：Constant Bit Rate
	ZPL_BIT_RATE_VBR,		//可变码率：Variable Bit Rate
	ZPL_BIT_RATE_ABR,		//平均码率：Average Bit Rate
	ZPL_BIT_RATE_MAX,
} ZPL_BIT_RATE_E;

typedef enum
{
    ZPL_VENC_RC_CBR = 0,
    ZPL_VENC_RC_VBR,
    ZPL_VENC_RC_AVBR,
    ZPL_VENC_RC_QVBR,
    ZPL_VENC_RC_CVBR,
    ZPL_VENC_RC_QPMAP,
    ZPL_VENC_RC_FIXQP
} ZPL_VENC_RC_E;

/* the gop mode */
typedef enum {
    ZPL_VENC_GOPMODE_NORMALP    = 0,     /* NORMALP */
    ZPL_VENC_GOPMODE_DUALP      = 1,     /* DUALP;  Not support for Hi3556AV100 */
    ZPL_VENC_GOPMODE_SMARTP     = 2,     /* SMARTP; Not support for Hi3556AV100 */
    ZPL_VENC_GOPMODE_ADVSMARTP  = 3,     /* ADVSMARTP ; Only used for Hi3559AV100 */
    ZPL_VENC_GOPMODE_BIPREDB    = 4,     /* BIPREDB ;Only used for Hi3559AV100/Hi3519AV100 */
    ZPL_VENC_GOPMODE_LOWDELAYB  = 5,     /* LOWDELAYB; Not support */

    ZPL_VENC_GOPMODE_BUTT,
} ZPL_VENC_GOP_MODE_E;

typedef enum {
    ZPL_VIDEO_FRAME_TYPE_NONE     = 0,               /* NORMAL */
    ZPL_VIDEO_FRAME_TYPE_NORMAL   = 1,                /* NORMAL */
    ZPL_VIDEO_FRAME_TYPE_KEY      = 2,                /* */

    ZPL_VIDEO_FRAME_TYPE_BSLICE,                         /* H264/H265 B SLICE types */
    ZPL_VIDEO_FRAME_TYPE_PSLICE,                         /* H264/H265 P SLICE types */
    ZPL_VIDEO_FRAME_TYPE_ISLICE,                         /* H264/H265 I SLICE types */
    ZPL_VIDEO_FRAME_TYPE_IDRSLICE,                       /* H264/H265 IDR SLICE types */
    ZPL_VIDEO_FRAME_TYPE_SEI,                         /* H264/H265 SEI types */
    ZPL_VIDEO_FRAME_TYPE_SPS,                         /* H264/H265 SPS types */
    ZPL_VIDEO_FRAME_TYPE_PPS,                         /* H264/H265 PPS types */
    ZPL_VIDEO_FRAME_TYPE_VPS,                         /* H265 VPS types */

    ZPL_VIDEO_FRAME_TYPE_ECS,                            /* JPEGE ECS types */
    ZPL_VIDEO_FRAME_TYPE_APP,                            /* JPEGE APP types */
    ZPL_VIDEO_FRAME_TYPE_VDO,                            /* JPEGE VDO types */
    ZPL_VIDEO_FRAME_TYPE_PIC,                            /* JPEGE PIC types */
    ZPL_VIDEO_FRAME_TYPE_DCF,                            /* JPEGE DCF types */
    ZPL_VIDEO_FRAME_TYPE_DCF_PIC,                       /* JPEGE DCF PIC types */	
} ZPL_VIDEO_FRAME_TYPE_E;


typedef struct 
{
	ZPL_VIDEO_FORMAT_E	format;	
	zpl_video_size_t	vidsize;		//视频大小
	ZPL_VIDEO_CODEC_E 	enctype;		//编码类型
	zpl_uint32			framerate;		//帧率
	zpl_uint32			bitrate;		//码率
    zpl_uint32			profile;        //编码等级
	ZPL_BIT_RATE_E		bitrate_type;	//码率类型
    zpl_uint32		    ikey_rate;	    //I帧间隔
	ZPL_VENC_RC_E		enRcMode;	
	ZPL_VENC_GOP_MODE_E	gopmode;
    zpl_uint8           packetization_mode; //封包解包模式
}zpl_video_codec_t __attribute__ ((packed));

typedef struct
{
    ZPL_AUDIO_CODEC_E 	enctype;		//编码类型
    zpl_uint32			framerate;		//帧率
    zpl_uint32			bitrate;		//码率
    ZPL_BIT_RATE_E		bitrate_type;	//码率类型
}zpl_audio_codec_t __attribute__ ((packed));

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_CODEC_H__ */
