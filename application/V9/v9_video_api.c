/*
 * v9_video.c
 *
 *  Created on: 2019年11月26日
 *      Author: DELL
 */

#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "interface.h"
#include "log.h"
#include "vty.h"


#include "application.h"


int v9_video_stream_add_api(u_int8 id, u_int8 ch, u_int32 address, u_int16 port,
							   char *username, char *password, u_int32 fps)
{
	int iid = id, chid = ch;
	v9_video_channel_t *stream = NULL;

	if(id)
		iid = APP_BOARD_CALCU_ID(id);

	if(!iid)
		iid = v9_video_board_get_minload();
	if(ch == 0)
	{
		chid = v9_video_board_channel_alloc(iid);
	}
	else
	{
		if(v9_video_board_video_channel_lookup_by_id_and_ch(iid, chid))
		{
			zlog_debug(ZLOG_APP," this channel is already exist.");
			return ERROR;
		}
	}

	if(chid > 0)
		stream = v9_video_channel_alloc_api( chid,  address,  port, username, password, fps);
	if(stream)
	{
		if(id && ch)
		{
			stream->type = V9_VIDEO_STREAM_TYPE_STATIC;
		}
		else if(id && !ch)
		{
			stream->type = V9_VIDEO_STREAM_TYPE_DYNAMIC_CH;
		}
		else if(!id && ch)
		{
			stream->type = V9_VIDEO_STREAM_TYPE_DYNAMIC_ID;
		}
		else
			stream->type = V9_VIDEO_STREAM_TYPE_DYNAMIC;

		return v9_video_board_channel_add( iid,  stream);
	}
	zlog_debug(ZLOG_APP," can not alloc video channel data.");
	return ERROR;
}


int v9_video_stream_del_api(u_int8 id, u_int8 ch, u_int32 address, u_int16 port)
{
	v9_video_channel_t *stream = NULL;
	if(id && ch)
		stream = v9_video_board_video_channel_lookup_by_id_and_ch(APP_BOARD_CALCU_ID(id), ch);
	else
		stream = v9_video_channel_lookup_api(ch,  address,  port);
	if(stream)
	{
		if(v9_video_board_channel_del(stream->id, stream) == OK)
			return v9_video_channel_free_api(stream);
		else
			zlog_debug(ZLOG_APP," can not delete this video channel data.");
	}
	zlog_debug(ZLOG_APP," can not lookup this video channel.");
	return ERROR;
}
