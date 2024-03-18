/**
 * @file      : rtmp-client-api.h
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-03-14
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
 */
#ifndef _rtmp_client_api_h_
#define _rtmp_client_api_h_

#if defined(__cplusplus)
extern "C" {
#endif
#include "rtmpsys.h"
#include "rtmp.h"
#include "rtmphttp.h"

typedef struct rtmp_client_s
{
    RTMP    _rtmp;
    RTMPPacket _packet;
    int _stream_id;
    void    *media_channel;
}rtmp_client_t;

rtmp_client_t *rtmp_client_create(void *mchn, char *url);
int rtmp_client_start(rtmp_client_t *rtmp);
int rtmp_client_pause(rtmp_client_t *rtmp, int val);
int rtmp_client_destroy(rtmp_client_t *rtmp);

int rtmp_client_send_packet(rtmp_client_t *rtmp, int type, char *data, int len, int timestamp) ;
int rtmp_client_send(rtmp_client_t *rtmp, int type, char *data, int len, int timestamp);


int rtmp_send_h264_packet(rtmp_client_t *rtmp, int nultype, char *data, int size, int nTimeStamp) ;

#if defined(__cplusplus)
}
#endif
#endif /* !_rtmp_client_api_h_ */
