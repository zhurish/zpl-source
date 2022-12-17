/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __RTSP_SDP_UTIL_H__
#define __RTSP_SDP_UTIL_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_rtsp_def.h"
#include "zpl_rtsp.h"
#ifdef ZPL_BUILD_LINUX
#include <syslog.h>
#else
#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */
#define	LOG_DEBUG	7	/* debug-level messages */
#endif


typedef int (*rtsp_log_callback)(int, char*);
/*
 * rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1
 * rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&rtsp
 * rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&multcast
 * rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&tls
 * rtsp://user:pass@192.168.1.1/media/channel=1&level=1
 * rtsp://192.168.1.1:9988/media/channel=1&level=1
 */
#if 0
typedef struct rtsp_urlpath_s
{
    char        username[64];
    char        password[64];
    char        hostname[128];
    uint16_t    port;
    int32_t     channel;
    int32_t     level;
    char        path[128];
    char        url[128];
    uint16_t    mode;
}rtsp_urlpath_t;


RTSP_API void rtsp_url_stream_path(const char *url, rtsp_urlpath_t *);
#else
RTSP_API void rtsp_url_stream_path(const char *url, os_url_t *);
#endif
RTSP_API int rtsp_authenticate_option(const char *auth, const char *username, const char *password);

RTSP_API void rtsp_log_cb(rtsp_log_callback);

RTSP_API void rtsp_vlog(const char *file, const char *func, const int line, int livel, const char *fmt,...);



#define rtsp_log_trace(format, ...)         rtsp_vlog (__FILE__, __FUNCTION__, __LINE__, LOG_DEBUG+1, format, ##__VA_ARGS__)
#define rtsp_log_debug(format, ...)         rtsp_vlog (__FILE__, __FUNCTION__, __LINE__, LOG_DEBUG, format, ##__VA_ARGS__)
#define rtsp_log_notice(format, ...)        rtsp_vlog (__FILE__, __FUNCTION__, __LINE__, LOG_NOTICE, format, ##__VA_ARGS__)
#define rtsp_log_info(format, ...)          rtsp_vlog (__FILE__, __FUNCTION__, __LINE__, LOG_INFO, format, ##__VA_ARGS__)
#define rtsp_log_warn(format, ...)          rtsp_vlog (__FILE__, __FUNCTION__, __LINE__, LOG_WARNING, format, ##__VA_ARGS__)
#define rtsp_log_error(format, ...)         rtsp_vlog (__FILE__, __FUNCTION__, __LINE__, LOG_ERR, format, ##__VA_ARGS__)



#if defined(RTSP_DEBUG_ENABLE)
RTSP_API int rtsp_urlpath_test(char *url);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __RTSP_SDP_H__ */
