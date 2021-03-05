/*
 * v9_video.h
 *
 *  Created on: 2019年11月26日
 *      Author: DELL
 */

#ifndef __V9_VIDEO_API_H__
#define __V9_VIDEO_API_H__

#ifdef __cplusplus
extern "C" {
#endif

int v9_video_board_stream_add_api(ospl_uint8 id, ospl_uint8 ch, ospl_uint32 address, ospl_uint16 port,
							   char *username, char *password, ospl_uint32 fps, char *param, char *secondary);
int v9_video_board_stream_del_api(ospl_uint8 id, ospl_uint8 ch, ospl_uint32 address, ospl_uint16 port);

int v9_video_board_stream_cleanup_api();

#ifdef __cplusplus
}
#endif

#endif /* __V9_VIDEO_API_H__ */
