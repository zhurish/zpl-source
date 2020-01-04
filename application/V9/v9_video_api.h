/*
 * v9_video.h
 *
 *  Created on: 2019年11月26日
 *      Author: DELL
 */

#ifndef __V9_VIDEO_API_H__
#define __V9_VIDEO_API_H__

int v9_video_stream_add_api(u_int8 id, u_int8 ch, u_int32 address, u_int16 port,
							   char *username, char *password, u_int32 fps);
int v9_video_stream_del_api(u_int8 id, u_int8 ch, u_int32 address, u_int16 port);


#endif /* __V9_VIDEO_API_H__ */
