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


#include "v9_device.h"
#include "v9_util.h"
#include "v9_video.h"
#include "v9_serial.h"
#include "v9_slipnet.h"
#include "v9_cmd.h"

#include "v9_video_disk.h"
#include "v9_user_db.h"
#include "v9_video_db.h"

#include "v9_board.h"
#include "v9_video_sdk.h"
#include "v9_video_user.h"
#include "v9_video_board.h"
#include "v9_video_api.h"

//"rtsp://admin:abc123456@192.168.3.64:554/av0_0",
//"rtsp://admin:abc123456@192.168.1.64:554/av0_0",
//rtsp://admin:abc123456@192.168.1.108:554/cam/realmonitor?channel=1&subtype=0
/*
海康：
　　rtsp://[username]:[password]@[ip]:[port]/[codec]/[channel]/[subtype]/av_stream
说明：
username: 用户名。例如admin。
password: 密码。例如12345。
ip: 为设备IP。例如 192.0.0.64。
port: 端口号默认为554，若为默认可不填写。
codec：有h264、MPEG-4、mpeg4这几种。
channel: 通道号，起始为1。例如通道1，则为ch1。
subtype: 码流类型，主码流为main，辅码流为sub。

例如，请求海康摄像机通道1的主码流，Url如下
主码流：
rtsp://admin:12345@192.0.0.64:554/h264/ch1/main/av_stream
rtsp://admin:12345@192.0.0.64:554/MPEG-4/ch1/main/av_stream
子码流：
rtsp://admin:12345@192.0.0.64/mpeg4/ch1/sub/av_stream
rtsp://admin:12345@192.0.0.64/h264/ch1/sub/av_stream

大华：
rtsp://username:password@ip:port/cam/realmonitor?channel=1&subtype=0
说明:
username: 用户名。例如admin。
password: 密码。例如admin。
ip: 为设备IP。例如 10.7.8.122。
port: 端口号默认为554，若为默认可不填写。
channel: 通道号，起始为1。例如通道2，则为channel=2。
subtype: 码流类型，主码流为0（即subtype=0），辅码流为1（即subtype=1）。

例如，请求某设备的通道2的辅码流，Url如下
rtsp://admin:admin@10.12.4.84:554/cam/realmonitor?channel=2&subtype=1
 */
int v9_video_board_stream_add_api(u_int8 id, u_int8 ch, u_int32 address, u_int16 port,
							   char *username, char *password, u_int32 fps, char *param, char *secondary)
{
	int iid = id, chid = ch, ret = 0;
	v9_video_stream_t *stream = NULL;
	v9_video_board_lock();
	if(id)
		iid = V9_APP_BOARD_CALCU_ID(id);

	if(!iid)
		iid = v9_video_board_get_minload();
	if(ch == 0)
	{
		chid = v9_video_board_stream_alloc(iid);
	}
	else
	{
		if(v9_video_board_stream_lookup_by_id_and_ch(iid, chid))
		{
			v9_video_board_unlock();
			zlog_debug(ZLOG_APP," this channel is already exist.");
			return ERROR;
		}
	}

	V9_DEBUG("id:%d ch:%d address:0x%x port:%d username:%s password:%s", iid, chid, address, port, username, password);

	if(chid > 0)
		stream = v9_video_board_stream_alloc_api( chid,  address,  port, username, password, fps, param, secondary);
	if(stream)
	{
		//if(id && ch)
		//{
		stream->type = V9_VIDEO_STREAM_TYPE_STATIC;
/*
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
*/
		ret = v9_video_board_stream_add( iid,  stream, TRUE);
		v9_video_board_unlock();
		return ret;
	}
	v9_video_board_unlock();
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(ZLOG_APP," can not alloc video channel data.");
	return ERROR;
}


int v9_video_board_stream_del_api(u_int8 id, u_int8 ch, u_int32 address, u_int16 port)
{
	int ret = 0;
	v9_video_stream_t *stream = NULL;
	v9_video_board_lock();
	if(id && ch)
		stream = v9_video_board_stream_lookup_by_id_and_ch(V9_APP_BOARD_CALCU_ID(id), ch);
	else
		stream = v9_video_board_stream_lookup_api(ch,  address,  port);
	if(stream)
	{
		if(v9_video_board_stream_del(stream->id, stream) == OK)
		{
			ret = v9_video_board_stream_free_api(stream);
			v9_video_board_unlock();
			return ret;
		}
		else
		{
			if(V9_APP_DEBUG(BOARD_EVENT))
				zlog_debug(ZLOG_APP," can not delete this video channel data(%d:%d).", id, ch);
		}
	}
	v9_video_board_unlock();
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(ZLOG_APP," can not lookup this video channel(%d:%d).", id, ch);
	return ERROR;
}

int v9_video_board_stream_cleanup_api()
{
	int ret = 0;
	v9_video_board_lock();
	ret = v9_video_board_stream_cleanup();
	v9_video_board_unlock();
	return ret;
}
