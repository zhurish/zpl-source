/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __RTSP_H__
#define __RTSP_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"

#ifdef ZPL_LIBRTSP_MODULE
#include "zpl_media.h"
#include "zpl_media_internal.h"
typedef zpl_media_channel_t rtsp_media_t;//服务端指向zpl_media_channel_t， 客户端指向zpl_client_media_t
#endif

#if defined(_MSC_VER)
# if defined(DLLBUILD)
/* define DLLBUILD when building the DLL */
#  define RTSP_API __declspec(dllexport)
# else
#  define RTSP_API extern //__declspec(dllimport)
# endif
#else
# define RTSP_API extern
#endif



#ifndef max
#define max(a,b)    (a)>(b)?(a):(b)
#endif


typedef enum
{
    RTSP_METHOD_NONE            = 0,
    RTSP_METHOD_OPTIONS         = 1,
    RTSP_METHOD_DESCRIBE        = 2,
    RTSP_METHOD_SETUP           = 3,
    RTSP_METHOD_TEARDOWN        = 4,
    RTSP_METHOD_PLAY            = 5,
    RTSP_METHOD_PAUSE           = 6,
    RTSP_METHOD_SCALE           = 7,
    RTSP_METHOD_GET_PARAMETER   = 8,
    RTSP_METHOD_SET_PARAMETER   = 9,
} rtsp_method;


typedef enum {
    RTSP_TRANSPORT_UNICAST = 1,
    RTSP_TRANSPORT_MULTICAST,
}rtsp_transport_type_t;

// transport
typedef enum {
    RTSP_TRANSPORT_RTP_NONE= 0,
    RTSP_TRANSPORT_RTP_UDP = 1,
    RTSP_TRANSPORT_RTP_TCP,
    RTSP_TRANSPORT_RTP_RAW,
    RTSP_TRANSPORT_RTP_TLS,
    RTSP_TRANSPORT_RTP_RTPOVERRTSP,
    RTSP_TRANSPORT_RTP_MULTCAST,
}rtsp_transport_rtp_t;

// transport mode
typedef enum {
    RTSP_TRANSPORT_PLAY = 1,
    RTSP_TRANSPORT_RECORD
}rtsp_transport_mode_t;


typedef enum
{
    RTSP_STATE_CODE_100=100,   //Continue
    RTSP_STATE_CODE_200=200,   //OK
    RTSP_STATE_CODE_201=201,   //Created
    RTSP_STATE_CODE_250=250,   //Low on Storage Space
    RTSP_STATE_CODE_300=300,   //Multiple Choices
    RTSP_STATE_CODE_301=301,   //Moved Permanently
    RTSP_STATE_CODE_302=302,   //Moved Temporarily
    RTSP_STATE_CODE_303=303,   //See Other
    RTSP_STATE_CODE_304=304,   //Not Modified
    RTSP_STATE_CODE_305=305,   //Use Proxy
    RTSP_STATE_CODE_400=400,   //Bad Request
    RTSP_STATE_CODE_401=401,   //Unauthorized
    RTSP_STATE_CODE_402=402,   //Payment Required
    RTSP_STATE_CODE_403=403,   //Forbidden
    RTSP_STATE_CODE_404=404,   //Not Found
    RTSP_STATE_CODE_405=405,   //Method Not Allowed
    RTSP_STATE_CODE_406=406,   //Not Acceptable
    RTSP_STATE_CODE_407=407,   //Proxy Authentication Required
    RTSP_STATE_CODE_408=408,   //Request Time-out
    RTSP_STATE_CODE_410=410,   //Gone
    RTSP_STATE_CODE_411=411,   //Length Required
    RTSP_STATE_CODE_412=412,   //Precondition Failed
    RTSP_STATE_CODE_413=413,   //Request Entity Too Large
    RTSP_STATE_CODE_414=414,   //Request-URI Too Large
    RTSP_STATE_CODE_415=415,   //Unsupported Media Type
    RTSP_STATE_CODE_451=451,   //Parameter Not Understood
    RTSP_STATE_CODE_452=452,   //Conference Not Found
    RTSP_STATE_CODE_453=453,   //Not Enough Bandwidth
    RTSP_STATE_CODE_454=454,   //Session Not Found
    RTSP_STATE_CODE_455=455,   //Method Not Valid in This State
    RTSP_STATE_CODE_456=456,   //Header Field Not Valid for Resource
    RTSP_STATE_CODE_457=457,   //Invalid Range
    RTSP_STATE_CODE_458=458,   //Parameter Is Read-Only
    RTSP_STATE_CODE_459=459,   //Aggregate operation not allowed
    RTSP_STATE_CODE_460=460,   //Only aggregate operation allowed
    RTSP_STATE_CODE_461=461,   //Unsupported transport
    RTSP_STATE_CODE_462=462,   //Destination unreachable
    RTSP_STATE_CODE_500=500,   //Internal Server Error
    RTSP_STATE_CODE_501=501,   //Not Implemented
    RTSP_STATE_CODE_502=502,   //Bad Gateway
    RTSP_STATE_CODE_503=503,   //Service Unavailable
    RTSP_STATE_CODE_504=504,   //Gateway Time-out
    RTSP_STATE_CODE_505=505,   //RTSP Version not supported
    RTSP_STATE_CODE_551=551,   //Option not supported

}rtsp_code;

#define RTSP_HDR_VER                "RTSP/1.0"
#define RTSP_SRV_NAME               "rtsp-v0.1"
#define RTSP_METHOD_CHECK(n)        ( ((n) >= RTSP_METHOD_OPTIONS) && ((n) <= RTSP_METHOD_SET_PARAMETER) )


#define RTSP_DEBUG_ENABLE

#if defined(RTSP_DEBUG_ENABLE)
#define RTSP_DEBUG(format, ...) {fprintf (stdout, "%s [%d]", __func__, __LINE__);fprintf (stdout, format, ##__VA_ARGS__);fflush(stdout);}
#define RTSP_INFO(format, ...)  {fprintf (stdout, "%s [%d]", __func__, __LINE__);fprintf (stdout, format, ##__VA_ARGS__);fflush(stdout);}
#define RTSP_WARN(format, ...)  {fprintf (stdout, "%s [%d]", __func__, __LINE__);fprintf (stdout, format, ##__VA_ARGS__);fflush(stdout);}
#define RTSP_ERROR(format, ...) {fprintf (stdout, "%s [%d]", __func__, __LINE__);fprintf (stdout, format, ##__VA_ARGS__);fflush(stdout);}
#define RTSP_TRACE(format, ...) {fprintf (stdout, "%s [%d]", __func__, __LINE__);fprintf (stdout, format, ##__VA_ARGS__);fflush(stdout);}
#define RTSP_INTER_FUNC()       {fprintf (stdout, "%s [%d]", __func__, __LINE__);fflush(stdout);}
#define RTSP_LEVEL_FUNC()       {fprintf (stdout, "%s [%d]", __func__, __LINE__);fflush(stdout);}
#else
#define RTSP_DEBUG(format, ...)
#define RTSP_INFO(format, ...)
#define RTSP_WARN(format, ...)
#define RTSP_ERROR(format, ...)
#define RTSP_TRACE(format, ...)
#endif


#ifdef __cplusplus
}
#endif

#endif /* __RTSP_SDP_H__ */
