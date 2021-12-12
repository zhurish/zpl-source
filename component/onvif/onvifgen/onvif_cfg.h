#ifndef __ONVIF_CFG_H__
#define __ONVIF_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define _CRT_SECURE_NO_WARNINGS
#include "soapH.h"

struct onvif_VideoSource
{
    struct onvif__VideoSource *next;
    struct tt__VideoSource VideoSource;
};

struct onvif_VideoSourceMode
{
    struct onvif_VideoSourceMode *next;
    struct trt__VideoSourceMode VideoSourceMode;
};

struct onvif_VideoSourceConfig
{
    struct onvif_VideoSourceConfig *next;
    struct tt__VideoSourceConfiguration VideoSourceConfig;
};

struct onvif_VideoEncoderConfig
{
    struct onvif_VideoEncoderConfig *next;
    struct tt__VideoEncoderConfiguration VideoSourceConfig;
};


struct onvif_config
{
    struct onvif_VideoSource            *onvif_video_source;
    struct onvif_VideoSourceMode        *onvif_video_source_mode;
    struct onvif_VideoSourceConfig      *onvif_video_source_config;
    struct onvif_VideoEncoderConfig     *onvif_video_encoder_config;
};


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ONVIF_CFG_H__ */