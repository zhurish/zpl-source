#include "zpl_media.h"
#include "zpl_media_internal.h"


#define MEDIA_CODEC_NAME_MAX    32
struct media_codec_desc
{
  unsigned int type;
  const char string[MEDIA_CODEC_NAME_MAX];
};

#define MEDIA_CODEC_DESC_ENTRY(T,S) { (T), (S) }
static const struct media_codec_desc media_codec_descmap[] = {
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_NONE,        "NONE"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_NONE,        "NONE"),
    MEDIA_CODEC_DESC_ENTRY(RTP_MEDIA_PAYLOAD_NONE,      "NONE"),
        /* AUDIO */
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_PCMU,       "audio.PCMU"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_1016,       "audio.1016"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_G721,       "audio.G721"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_GSM,       "audio.GSM"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_G723,       "audio.G723"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_DVI4_8K,       "audio.DVI4_8K"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_DVI4_16K,       "audio.DVI4_16K"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_LPC,       "audio.LPC"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_PCMA,       "audio.PCMA"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_G722,       "audio.G722"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_S16BE_STEREO,       "audio.S16BE_STEREO"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_S16BE_MONO,       "audio.S16BE_MONO"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_QCELP,       "audio.QCELP"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_CN,       "audio.CN"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_MPEGAUDIO,       "audio.MPEGAUDIO"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_G728,       "audio.G728"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_DVI4_3,       "audio.DVI4_3"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_DVI4_4,       "audio.DVI4_4"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_G729,       "audio.G729"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_G711A,       "audio.G711A"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_G711U,       "audio.G711U"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_G726,       "audio.G726"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_G729A,       "audio.G729A"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_LPCM,       "audio.LPCM"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_CelB,       "audio.CelB"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_JPEG,       "audio.JPEG"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_CUSM,       "audio.CUSM"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_NV,       "audio.NV"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_PICW,       "audio.PICW"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_CPV,       "audio.CPV"),

    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_AMR,       "audio.AMR"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_AMRWB,       "audio.AMRWB"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_PRORES,       "audio.PRORES"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_OPUS,       "audio.OPUS"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_D_GSM_HR,       "audio.D_GSM_HR"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_D_GSM_EFR,       "audio.D_GSM_EFR"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_D_L8,       "audio.D_L8"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_D_RED,       "audio.D_RED"),

        /* RTP AUDIO PAYLOAD */

        /* VIDEO */
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_H261,        "video.H261"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_MPEGVIDEO,        "video.MPEGVIDEO"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_MPEG2TS,       "video.MPEG2TS"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_H263,       "video.H263"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_SPEG,       "video.SPEG"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_MPEG2VIDEO,       "video.MPEG2VIDEO"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_AAC,       "video.AAC"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_WMA9STD,       "video.WMA9STD"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_HEAAC,       "video.HEAAC"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_PCM_VOICE,       "video.PCM_VOICE"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_PCM_AUDIO,       "video.PCM_AUDIO"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_MP3,       "video.MP3"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_ADPCMA,       "video.ADPCMA"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_AEC,       "video.AEC"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_X_LD,       "video.X_LD"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_H264,       "video.H264"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_JPEG,       "video.JPEG"),
        
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_D_VDVI,       "video.D_VDVI"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_D_BT656,       "video.D_BT656"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_D_H263_1998,       "video.D_H263_1998"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_D_MP1S,       "video.D_MP1S"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_D_MP2P,       "video.D_MP2P"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_D_BMPEG,       "video.D_BMPEG"),

    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_MJPEG,       "video.MJPEG"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_MP4VIDEO,       "video.MP4VIDEO"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_MP4AUDIO,       "video.MP4AUDIO"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_VC1,       "video.VC1"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_JVC_ASF,       "video.JVC_ASF"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_D_AVI,       "video.D_AVI"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_DIVX3,       "video.DIVX3"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_AVS,       "video.AVS"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_REAL8,       "video.REAL8"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_REAL9,       "video.REAL9"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_VP6,       "video.VP6"),

    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_VP6F,       "video.VP6F"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_VP6A,       "video.VP6A"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_SORENSON,   "video.SORENSON"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_H265,       "video.H265"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_VP8,       "video.VP8"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_VP9,       "video.VP9"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_MVC,       "video.MVC"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_PNG,       "video.PNG"),

        /* RTP VIDEO PAYLOAD */
    MEDIA_CODEC_DESC_ENTRY(ZPL_AUDIO_CODEC_MAX,       "MAX"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_CODEC_MAX,       "MAX"),
    MEDIA_CODEC_DESC_ENTRY(RTP_MEDIA_PAYLOAD_MAX,      "MAX"),
};


const char *zpl_media_codec_name(int codec)
{
    int i = 0;
    for(i = 0; i < sizeof(media_codec_descmap)/sizeof(media_codec_descmap[0]); i++)
    {
        if(media_codec_descmap[i].type == codec)
            return media_codec_descmap[i].string;
    }
    return "none";
}

int zpl_media_codec_key(const char *name)
{
    int i = 0;
    char namekey[MEDIA_CODEC_NAME_MAX];
    memset(namekey, 0, sizeof(namekey));
    strcpy(namekey, name);
    for(i = 0; i < sizeof(media_codec_descmap)/sizeof(media_codec_descmap[0]); i++)
    {
        if(strncasecmp(media_codec_descmap[i].string, namekey, sizeof(namekey)) == 0)
            return media_codec_descmap[i].type;
    }
    return RTP_MEDIA_PAYLOAD_NONE;
}

static const struct media_codec_desc media_format_descmap[] = {
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_NONE,        "NONE"),
	MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_CIF,         "CIF"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_360P,        "360P(640*360)"),/* 640 * 360 */
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_D1_PAL,      "D1_PAL(720*576)"),/* 720 * 576 */
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_D1_NTSC,     "D1_NTSC(720*480)"),/* 720 * 480 */
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_720P,        "720P(1280*720)"),/* 1280 * 720  */
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_1080P,       "1080P(1920*1080)"),/* 1920 * 1080 */
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_2560x1440,   "2560x1440"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_2592x1520,   "2592x1520"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_2592x1536,   "2592x1944"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_2592x1944,   "2592x1944"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_2688x1536,   "2688x1536"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_2716x1524,   "2716x1524"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_3840x2160,   "3840x2160"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_4096x2160,   "4096x2160"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_3000x3000,   "3000x3000"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_4000x3000,   "4000x3000"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_7680x4320,   "7680x4320"),
    MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_3840x8640,   "3840x8640"),
	MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_640X480,     "640X480"),
	MEDIA_CODEC_DESC_ENTRY(ZPL_VIDEO_FORMAT_MAX,         "MAX"),
};

const char *zpl_media_format_name(int key)
{
    int i = 0;
    for(i = 0; i < sizeof(media_format_descmap)/sizeof(media_format_descmap[0]); i++)
    {
        if(media_format_descmap[i].type == key)
            return media_format_descmap[i].string;
    }
    return "none";
}

int zpl_media_video_format_resolution(ZPL_VIDEO_FORMAT_E format, zpl_video_size_t* pstSize)
{
    switch (format)
    {
        case ZPL_VIDEO_FORMAT_CIF:   /* 352 * 288 */
            pstSize->width  = 352;
            pstSize->height = 288;
            break;

        case ZPL_VIDEO_FORMAT_360P:   /* 640 * 360 */
            pstSize->width  = 640;
            pstSize->height = 360;
            break;

        case ZPL_VIDEO_FORMAT_D1_PAL:   /* 720 * 576 */
            pstSize->width  = 720;
            pstSize->height = 576;
            break;

        case ZPL_VIDEO_FORMAT_D1_NTSC:   /* 720 * 480 */
            pstSize->width  = 720;
            pstSize->height = 480;
            break;

        case ZPL_VIDEO_FORMAT_720P:   /* 1280 * 720 */
            pstSize->width  = 1280;
            pstSize->height = 720;
            break;

        case ZPL_VIDEO_FORMAT_1080P:  /* 1920 * 1080 */
            pstSize->width  = 1920;
            pstSize->height = 1080;
            break;

        case ZPL_VIDEO_FORMAT_2592x1520:
            pstSize->width  = 2592;
            pstSize->height = 1520;
            break;

        case ZPL_VIDEO_FORMAT_2592x1944:
            pstSize->width  = 2592;
            pstSize->height = 1944;
            break;

        case ZPL_VIDEO_FORMAT_2592x1536:
            pstSize->width  = 2592;
            pstSize->height = 1536;
            break;

        case ZPL_VIDEO_FORMAT_2560x1440:
            pstSize->width  = 2560;
            pstSize->height = 1440;
            break;

        case ZPL_VIDEO_FORMAT_2716x1524:
            pstSize->width  = 2716;
            pstSize->height = 1524;
            break;

        case ZPL_VIDEO_FORMAT_3840x2160:
            pstSize->width  = 3840;
            pstSize->height = 2160;
            break;

        case ZPL_VIDEO_FORMAT_3000x3000:
            pstSize->width  = 3000;
            pstSize->height = 3000;
            break;

        case ZPL_VIDEO_FORMAT_4000x3000:
            pstSize->width  = 4000;
            pstSize->height = 3000;
            break;

        case ZPL_VIDEO_FORMAT_4096x2160:
            pstSize->width  = 4096;
            pstSize->height = 2160;
            break;

        case ZPL_VIDEO_FORMAT_7680x4320:
            pstSize->width  = 7680;
            pstSize->height = 4320;
            break;
        case ZPL_VIDEO_FORMAT_3840x8640:
            pstSize->width = 3840;
            pstSize->height = 8640;
            break;

        case ZPL_VIDEO_FORMAT_2688x1536:
            pstSize->width  = 2688;
            pstSize->height = 1536;
            break;

        default:
            return ERROR;
    }

    return OK;
}
