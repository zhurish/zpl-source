/**
 * @file      : rtsp_server.h
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-03-18
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
 */
#ifndef __LIVERTSP_SERVER_H__
#define __LIVERTSP_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rtsp_srv_s rtsp_srv_t;

rtsp_srv_t *rtsp_server_create(char *dir, int (*logcb)(const char *fmt,...));
int rtsp_server_destroy(rtsp_srv_t *info);
int rtsp_server_start(rtsp_srv_t *info, int ourPort, unsigned int reclamationTestSeconds);
int rtsp_server_event_loop(rtsp_srv_t *info);
int rtsp_server_event_loop_interval(rtsp_srv_t *info, unsigned int maxDelayTime);
int rtsp_server_add_username(rtsp_srv_t *info, const char *username, const char *password);
int rtsp_server_add_session(rtsp_srv_t *info, const char *sessionName, void *subSession);
int rtsp_server_delete_session(rtsp_srv_t *info, const char *sessionName, void *subSession);
int rtsp_server_update_session(rtsp_srv_t *info, const char *sessionName, void *subSession);
int rtsp_server_subsession_create(rtsp_srv_t *info, const char *codecname, const char *streamName, const char *inputFileName);

#ifdef __cplusplus
}
#endif

#endif /* __LIVERTSP_SERVER_H__ */
