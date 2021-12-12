/*
 * web_rtsp_html.c
 *
 *  Created on: Apr 13, 2019
 *      Author: zhurish
 */

#define HAS_BOOL 1
#include "zpl_include.h"
#include "module.h"
#include "memory.h"
#include "zassert.h"
#include "command.h"
#include "prefix.h"
#include "host.h"
#include "log.h"
#include "vty.h"
#include "vty_user.h"


#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

//#ifdef ZPL_APP_MODULE
#include "application.h"


/*
<th>板卡 ID</th>
<th>视频流ID</th>
<th>视频源</th>
<th>用户名</th>
<th>帧率</th>
<th>RTSP状态</th>
<th>解码状态</th>
<th>连接状态</th>
*/
static int _web_video_stream_one_detail(Webs *wp, v9_video_stream_t *pstNode, zpl_bool detail)
{
	web_assert(wp);
	if (pstNode)
	{
		char		rtsp_status[128];
		memset(rtsp_status, 0, sizeof(rtsp_status));
#ifdef V9_VIDEO_SDK_API
		switch (pstNode->rtsp_status)
		{
			case EAIS_DEVICE_STATUS_ONLINE:					// 在线或RTSP有效
				sprintf (rtsp_status,"%s","ACTIVE");
				break;
			case EAIS_DEVICE_STATUS_OFFLINIE:				// 离线或RTSP无效
				sprintf (rtsp_status,"%s", "INACTIVE");
				break;
			case EAIS_DEVICE_STATUS_AUTH_ERR:					// 鉴权失败
				sprintf (rtsp_status,"%s","Auth Faild");
				break;
			case EAIS_DEVICE_STATUS_PARSE_ERR:				// RTSP不兼容
				sprintf (rtsp_status,"%s","RTSP Incompatible");
				break;
			default:
				sprintf (rtsp_status,"%s","Unknown");
				break;
		}
#else
		sprintf (rtsp_status,"%s","Unknown");
#endif
		if(wp->iValue > 0)
			websWrite(wp, "%s", ",");

		websWrite(wp, "{\"ID\":\"%d\", \"ch\":\"%d\", \"url\":\"%s\","
			"\"username\":\"%s\", \"fps\":\"%d\", \"rtsp\":\"%s\", \"decode\":\"%s\", "
			"\"connect\":\"%s\", \"stream\":\"%s\", \"secondary\":\"%s\"}",
			V9_APP_BOARD_HW_ID(pstNode->id),
			pstNode->ch,
			/*inet_address(pstNode->address), pstNode->port,*/
			strlen(pstNode->video_url) ? pstNode->video_url:" ",
			strlen(pstNode->username) ? pstNode->username:" ",
			pstNode->fps,
			rtsp_status,
			(pstNode->decode_status==zpl_true) ? "OK":"Failed",
			pstNode->connect ? "OK":"Failed",
			str_isempty(pstNode->mainstream, sizeof(pstNode->mainstream))? " ":pstNode->mainstream,
			str_isempty(pstNode->secondary, sizeof(pstNode->secondary))? " ":pstNode->secondary);
		wp->iValue++;
	}
	return OK;
}


static int web_video_stream_all(Webs *wp, char *path, char *query)
{
	zpl_uint32 i = 0, j = 0;
	char *strval = NULL;
	zpl_uint32 id = 0;
	web_assert(wp);
	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Board ID Value");
		return web_return_text_plain(wp, ERROR);
	}
	id = atoi(strval);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
	v9_video_board_lock();
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use &&
				v9_video_board[i].active &&
				v9_video_board[i].id == V9_APP_BOARD_CALCU_ID(id))
		{
			if(v9_video_board[i].id != APP_BOARD_MAIN)
				v9_video_sdk_get_rtsp_status_api(v9_video_board[i].id);

			for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
			{
				if(v9_video_board[i].channel[j] && v9_video_board[i].video_stream[j])
				{
					_web_video_stream_one_detail(wp, v9_video_board[i].video_stream[j], zpl_false);
				}
			}
		}
	}
	v9_video_board_unlock();
	wp->iValue = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	return OK;
}

static int web_video_stream_detail(Webs *wp, void *p)
{
	zpl_uint32 i = 0, j = 0;
	//zpl_uint32 id = 0;
	//char *value = NULL;
	web_assert(wp);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
	v9_video_board_lock();
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && v9_video_board[i].active)
		{
			for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
			{
				if(v9_video_board[i].channel[j] && v9_video_board[i].video_stream[j])
				{
					_web_video_stream_one_detail(wp, v9_video_board[i].video_stream[j], zpl_false);
				}
			}
		}
	}
	v9_video_board_unlock();
	wp->iValue = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	return OK;
}
/*
static int web_video_stream_detail(Webs *wp, char *path, char *query)
{
	zpl_uint32 i = 0;
	zpl_uint32 id = 0;
	char *value = NULL;
	value = webs_get_var(wp, T("ID"), T(""));
	if (NULL == value)
	{
		return web_return_text_plain(wp, ERROR);
	}
	zlog_debug(MODULE_APP,"web_board_stream_detail: ID=%s", value);
	id = atoi(value);
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && v9_video_board[i].active)
		{
			if( (id && v9_video_board[i].id == id) )
			{
				web_board_card_stream_detail(wp, &v9_video_board[i]);
				return OK;
			}
		}
	}
	return OK;
}
*/
static int web_video_stream_add(Webs *wp, char *path, char *query)
{
	int ret = 0;
	v9_video_stream_t stream;
	char *value = NULL;
	web_assert(wp);
	memset(&stream, 0, sizeof(v9_video_stream_t));

	value = webs_get_var(wp, T("address"), T(""));
	if (NULL == value)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Address Value");
		return web_return_text_plain(wp, ERROR);
	}
	stream.address = ntohl(ipstack_inet_addr(value));

	value = webs_get_var(wp, T("username"), T(""));
	if (NULL != value)
	{
		strcpy(stream.username, value);
	}
	value = webs_get_var(wp, T("password"), T(""));
	if (NULL != value)
	{
		strcpy(stream.password, value);
	}

	value = webs_get_var(wp, T("rstpport"), T(""));
	if (NULL != value)
	{
		stream.port = atoi(value);
	}
/*	value = webs_get_var(wp, T("fps"), T(""));
	if (NULL != value)
	{
		stream.fps = atoi(value);
	}*/
	stream.fps = 0;
	value = webs_get_var(wp, T("boardid"), T(""));
	if (NULL != value)
	{
		stream.id = atoi(value);
	}
	value = webs_get_var(wp, T("channel"), T(""));
	if (NULL != value)
	{
		stream.ch = atoi(value);
	}

	value = webs_get_var(wp, T("stream"), T(""));
	if (NULL != value)
	{
		strcpy(stream.mainstream, value);
	}
	value = webs_get_var(wp, T("secondary"), T(""));
	if (NULL != value)
	{
		strcpy(stream.secondary, value);
	}
	if(!v9_video_board_isactive(V9_APP_BOARD_CALCU_ID(stream.id)))
	{
		if(WEB_IS_DEBUG(EVENT))
				zlog_debug(MODULE_WEB, "Board ID %d is not active", V9_APP_BOARD_CALCU_ID(stream.id));
		return web_return_text_plain(wp, ERROR);
	}
	ret =  v9_video_board_stream_add_api(stream.id, stream.ch, stream.address, stream.port,
								   stream.username, stream.password, stream.fps,
								   strlen(stream.mainstream)?stream.mainstream:NULL,
								   strlen(stream.secondary)?stream.secondary:NULL);
	if(ret == OK)
	{
		vty_execute_shell("write memory");
		return web_return_text_plain(wp, OK);
	}
	else
	{
		if(WEB_IS_DEBUG(EVENT))
				zlog_debug(MODULE_WEB, "Can not Add RTSP Stream");
		return web_return_text_plain(wp, ERROR);
	}
}


static int web_video_stream_delete_one(Webs *wp, char *path, char *query, zpl_uint32 type)
{
	int ret = 0;
	char *strID = NULL;
	char *strch = NULL;
	zpl_uint8 id = 0;
	zpl_uint8 ch = 0;
	web_assert(wp);
	strID = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strID)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Board ID Value");
		ret = ERROR;
		goto err_out;
	}
	strch = webs_get_var(wp, T("ch"), T(""));
	if (NULL == strch)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Channel Value");
		ret = ERROR;
		goto err_out;
	}
	id = atoi(strID);
	if(strstr(strch, ",") == NULL)
	{
		ch = atoi(strch);
		ret = v9_video_board_stream_del_api(id, ch, 0, 0);
	}
	else
	{
		int i = 0, j = 0;
		char *str = strch;
		char tmp[16];
		memset(tmp, 0 ,sizeof(tmp));
		while(i < strlen(strch))
		{
			if(str[i] != ',')
				tmp[j++] = str[i];
			else
			{
				ch = atoi(tmp);
				ret |= v9_video_board_stream_del_api(id, ch, 0, 0);
				memset(tmp, 0 ,sizeof(tmp));
				j = 0;
			}
			i++;
		}
		if(strlen(tmp))
		{
			ch = atoi(tmp);
			ret |= v9_video_board_stream_del_api(id, ch, 0, 0);
			memset(tmp, 0 ,sizeof(tmp));
		}
	}
	if(ret != OK)
	{
		if(WEB_IS_DEBUG(EVENT))
			zlog_debug(MODULE_WEB, "Can not Del RTSP Stream");
	}
	else
	{
		vty_execute_shell("write memory");
	}
err_out:
	if(ret != OK)
		return ERROR;//
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);

	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	if(ret == OK)
		websWrite(wp, "%s", "OK");
	else
		websWrite(wp, "%s", "ERROR");
	websDone(wp);
	return OK;
}

static int web_video_stream_delete(Webs *wp, void *p)
{
	web_assert(wp);
	return web_video_stream_delete_one(wp, NULL, NULL, 0);
}

static int web_ffmpeg_find(const char *cmd, const char *findstr, const char *findstr1)
{
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	if(super_output_system(cmd, buf, sizeof(buf)) == OK)
	{
		if(strstr(buf, findstr) && strstr(buf, findstr1))
			return 1;
	}
	return ERROR;
}
//static int web_video_stream_show_one(Webs *wp, char *path, char *query)
static int web_video_stream_show_one(Webs *wp, void *p)
{
	static int sid = 0, sch = 0;
	char ffmpegcmd[1024];
	v9_video_stream_t *stream = NULL;
	char *strval = NULL;
	zpl_uint8 id = 0;
	zpl_uint8 ch = 0;
	int ffmpegpid = 0;
	web_assert(wp);
	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Board ID Value");
		return web_return_text_plain(wp, ERROR);
	}
	id = atoi(strval);
	strval = webs_get_var(wp, T("ch"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Channel Value");
		return web_return_text_plain(wp, ERROR);
	}
	ch = atoi(strval);

	ffmpegpid = web_ffmpeg_find("ps | grep ffmpeg", "ffmpeg", "rtsp");

	if(sid && sch && sid == id && sch == ch)
	{
		if(ffmpegpid != ERROR && ffmpegpid > 0)
		{
			if(WEB_IS_DEBUG(EVENT))
				zlog_debug(MODULE_WEB, "ffmpeg already running");
			return web_return_text_plain(wp, OK);
		}
	}
	if(id && ch)
	{
		stream = v9_video_board_stream_lookup_by_id_and_ch(V9_APP_BOARD_CALCU_ID(id), ch);
		if(stream && (stream->id != APP_BOARD_MAIN))
			v9_video_sdk_get_rtsp_status_api(stream->id);
	}
	if(stream && stream->decode_status==zpl_true && !str_isempty(stream->secondary, sizeof(stream->secondary)))
	{
		sid = id;
		sch = ch;

		if(ffmpegpid != ERROR && ffmpegpid > 0)
		{
			memset(ffmpegcmd, 0, sizeof(ffmpegcmd));
			sprintf(ffmpegcmd, "killall -9 ffmpeg >/dev/null 2>&1");

			super_system(ffmpegcmd);
		}
		memset(ffmpegcmd, 0, sizeof(ffmpegcmd));
		//rtsp://%s:%s@%s:%d/mpeg4/ch33/sub/av0_%d
		if(!str_isempty(stream->username, sizeof(stream->username)) && !str_isempty(stream->secondary, sizeof(stream->secondary)))
			sprintf(ffmpegcmd, "nohup ffmpeg -i 'rtsp://%s:%s@%s:%d/%s' -an -vcodec copy  -f flv rtmp://127.0.0.1:1935/live >/dev/null 2>&1 &",
					stream->username, stream->password,
					inet_address(stream->address), stream->port, stream->secondary);
		else if(!str_isempty(stream->secondary, sizeof(stream->secondary)))
			sprintf(ffmpegcmd, "nohup ffmpeg -i 'rtsp://%s:%d/%s' -an -vcodec copy  -f flv rtmp://127.0.0.1:1935/live >/dev/null 2>&1 &",
					inet_address(stream->address), stream->port, stream->secondary);


		zlog_debug(MODULE_WEB, "ffmpeg exe:'%s'", ffmpegcmd);
		super_system(ffmpegcmd);

		os_msleep(500);
		ffmpegpid = web_ffmpeg_find("ps | grep ffmpeg", "ffmpeg", "rtsp");

		if(ffmpegpid != ERROR && ffmpegpid > 0)
		{
			if(WEB_IS_DEBUG(EVENT))
				zlog_debug(MODULE_WEB, "ffmpeg running ok");
			return web_return_text_plain(wp, OK);
		}
	}
	else
	{
		if(stream && str_isempty(stream->secondary, sizeof(stream->secondary)))
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Stream Secondary Param null.");
		}
		sid = 0;
		sch = 0;
		if(ffmpegpid != ERROR && ffmpegpid > 0)
		{
			memset(ffmpegcmd, 0, sizeof(ffmpegcmd));
			sprintf(ffmpegcmd, "killall -9 ffmpeg >/dev/null 2>&1");

			super_system(ffmpegcmd);
		}

		if(stream && stream->decode_status!=zpl_true)
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "decode Status Failed");
		}
		else
		{
			if(WEB_IS_DEBUG(EVENT))
				zlog_debug(MODULE_WEB, "Can lookup stream id=%d ch=%d",V9_APP_BOARD_CALCU_ID(id), ch);
		}
		return web_return_text_plain(wp, ERROR);
	}
	return web_return_text_plain(wp, ERROR);
}


/***********************************************************************************/
//ffmpeg -i "rtsp://admin:abc123456@192.168.1.64:554/mpeg4/ch33/sub/av0_0" -an -vcodec copy  -f flv rtmp://127.0.0.1:1935/live
/***********************************************************************************/



int web_rtsp_app(void)
{
	//websFormDefine("boardstream-detail", web_video_stream_detail);
	web_button_add_hook("streamtbl", "detail", web_video_stream_detail, NULL);
	websFormDefine("allvideostream", web_video_stream_all);

	websFormDefine("addstream", web_video_stream_add);
	web_button_add_hook("streamtbl", "delete", web_video_stream_delete, NULL);
	web_button_add_hook("streamtbl", "videoshow", web_video_stream_show_one, NULL);
	return 0;
}
//#endif
