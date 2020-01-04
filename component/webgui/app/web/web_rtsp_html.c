/*
 * web_rtsp_html.c
 *
 *  Created on: Apr 13, 2019
 *      Author: zhurish
 */

#define HAS_BOOL 1
#include "zebra.h"
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

//#ifdef PL_APP_MODULE
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
static int _web_video_stream_one_detail(Webs *wp, v9_video_channel_t *pstNode, BOOL detail)
{
	if (pstNode)
	{
		char		rtsp_status[128];
		memset(rtsp_status, 0, sizeof(rtsp_status));
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

		if(wp->iValue > 0)
			websWrite(wp, "%s", ",");

		websWrite(wp, "{\"ID\":\"%d\", \"ch\":\"%d\", \"url\":\"%s:%d\","
			"\"username\":\"%s\", \"fps\":\"%d\", \"rtsp\":\"%s\", \"decode\":\"%s\", \"connect\":\"%s\"}",
			pstNode->id,
			pstNode->ch,
			inet_address(pstNode->address), pstNode->port,
			strlen(pstNode->username) ? pstNode->username:" ",
			pstNode->fps,
			rtsp_status,
			(pstNode->decode_status==0) ? "OK":"Failed",
			pstNode->connect ? "OK":"Failed");
		wp->iValue++;
	}
	return OK;
}



static int web_video_stream_all(Webs *wp, char *path, char *query)
{
	u_int32 i = 0, j = 0;
	//u_int32 id = 0;
	//char *value = NULL;

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;

	for(i = 0; i < V9_APP_VIDEO_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && v9_video_board[i].active)
		{
			for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
			{
				if(v9_video_board[i].channel[j] && v9_video_board[i].video_stream[j])
				{
					_web_video_stream_one_detail(wp, v9_video_board[i].video_stream[j], FALSE);
				}
			}
		}
	}
	wp->iValue = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	return OK;
}

static int web_video_stream_detail(Webs *wp, void *p)
{
	u_int32 i = 0, j = 0;
	//u_int32 id = 0;
	//char *value = NULL;

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;

	for(i = 0; i < V9_APP_VIDEO_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && v9_video_board[i].active)
		{
			for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
			{
				if(v9_video_board[i].channel[j] && v9_video_board[i].video_stream[j])
				{
					_web_video_stream_one_detail(wp, v9_video_board[i].video_stream[j], FALSE);
				}
			}
		}
	}
	wp->iValue = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	return OK;
}
/*
static int web_video_stream_detail(Webs *wp, char *path, char *query)
{
	u_int32 i = 0;
	u_int32 id = 0;
	char *value = NULL;
	value = webs_get_var(wp, T("ID"), T(""));
	if (NULL == value)
	{
		return web_return_text_plain(wp, ERROR);
	}
	zlog_debug(ZLOG_APP,"web_board_stream_detail: ID=%s", value);
	id = atoi(value);
	for(i = 0; i < V9_APP_VIDEO_BOARD_MAX; i++)
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
	v9_video_channel_t stream;
	char *value = NULL;
	memset(&stream, 0, sizeof(v9_video_channel_t));

	value = webs_get_var(wp, T("address"), T(""));
	if (NULL == value)
	{
		return web_return_text_plain(wp, ERROR);
	}
	stream.address = ntohl(inet_addr(value));

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

	ret =  v9_video_stream_add_api(stream.id, stream.ch, stream.address, stream.port,
								   stream.username, stream.password, stream.fps);
	if(ret == OK)
		return web_return_text_plain(wp, OK);
	else
		return web_return_text_plain(wp, ERROR);
}


static int web_video_stream_delete_one(Webs *wp, char *path, char *query, int type)
{
	int ret = 0;
	char *strID = NULL;
	char *strch = NULL;
	u_int8 id = 0;
	u_int8 ch = 0;
	strID = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strID)
	{
		ret = ERROR;
		goto err_out;
	}
	strch = webs_get_var(wp, T("ch"), T(""));
	if (NULL == strch)
	{
		ret = ERROR;
		goto err_out;
	}
	id = atoi(strID);
	ch = atoi(strch);

	ret = v9_video_stream_del_api(id, ch, 0, 0);

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
	return web_video_stream_delete_one(wp, NULL, NULL, 0);
}

int web_rtsp_app(void)
{
	//websFormDefine("boardstream-detail", web_video_stream_detail);
	web_button_add_hook("streamtbl", "detail", web_video_stream_detail, NULL);
	websFormDefine("allvideostream", web_video_stream_all);

	websFormDefine("addstream", web_video_stream_add);
	web_button_add_hook("streamtbl", "delete", web_video_stream_delete, NULL);
	return 0;
}
//#endif
