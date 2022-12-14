/*
 * v9_video_board.c
 *
 *  Created on: 2019年11月26日
 *      Author: DELL
 */



#include "zebra.h"
#include "zmemory.h"
#include "command.h"
#include "zmemory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "nsm_interface.h"
#include "log.h"
#include "vty.h"
#include "vty_user.h"
#include "eloop.h"

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



v9_video_board_t *v9_video_board = NULL;
static void 		*board_mutex = NULL;
/*********************************************************************/
void v9_video_board_lock()
{
	if(board_mutex)
	{
		os_mutex_lock(board_mutex, OS_WAIT_FOREVER);
	}
}
void v9_video_board_unlock()
{
	if(board_mutex)
	{
		os_mutex_unlock(board_mutex);
	}
}

int v9_video_board_init()
{
	if(!board_mutex)
	{
		board_mutex = os_mutex_name_init("board_mutex");
	}
	if(!v9_video_board)
	{
		v9_video_board = XMALLOC(MTYPE_VIDEO_MEDIA, sizeof(v9_video_board_t) * V9_APP_BOARD_MAX);
		if(v9_video_board)
		{
			memset(v9_video_board, 0, sizeof(v9_video_board_t) * V9_APP_BOARD_MAX);

			v9_video_board_add(APP_BOARD_MAIN);
			v9_video_board_address(APP_BOARD_MAIN, APP_BOARD_ADDRESS_PREFIX + APP_BOARD_ADDRESS_MAIN, 0);
			v9_board_init(APP_BOARD_MAIN, &v9_video_board[APP_BOARD_MAIN-1].board);

			v9_video_board_add(APP_BOARD_CALCU_1);
			v9_video_board_address(APP_BOARD_CALCU_1, APP_BOARD_ADDRESS_PREFIX + APP_BOARD_CALCU_1, V9_VIDEO_SDK_PORT);
			v9_board_init(APP_BOARD_CALCU_1, &v9_video_board[APP_BOARD_CALCU_1-1].board);
			v9_video_sdk_init(&v9_video_board[APP_BOARD_CALCU_1-1].sdk, &v9_video_board[APP_BOARD_CALCU_1-1]);

			v9_video_board_add(APP_BOARD_CALCU_2);
			v9_video_board_address(APP_BOARD_CALCU_2, APP_BOARD_ADDRESS_PREFIX + APP_BOARD_CALCU_2, V9_VIDEO_SDK_PORT);
			v9_board_init(APP_BOARD_CALCU_2, &v9_video_board[APP_BOARD_CALCU_2-1].board);
			v9_video_sdk_init(&v9_video_board[APP_BOARD_CALCU_2-1].sdk, &v9_video_board[APP_BOARD_CALCU_2-1]);

			v9_video_board_add(APP_BOARD_CALCU_3);
			v9_video_board_address(APP_BOARD_CALCU_3, APP_BOARD_ADDRESS_PREFIX + APP_BOARD_CALCU_3, V9_VIDEO_SDK_PORT);
			v9_board_init(APP_BOARD_CALCU_3, &v9_video_board[APP_BOARD_CALCU_3-1].board);
			v9_video_sdk_init(&v9_video_board[APP_BOARD_CALCU_3-1].sdk, &v9_video_board[APP_BOARD_CALCU_3-1]);

			v9_video_board_add(APP_BOARD_CALCU_4);
			v9_video_board_address(APP_BOARD_CALCU_4, APP_BOARD_ADDRESS_PREFIX + APP_BOARD_CALCU_4, V9_VIDEO_SDK_PORT);
			v9_board_init(APP_BOARD_CALCU_4, &v9_video_board[APP_BOARD_CALCU_4-1].board);
			v9_video_sdk_init(&v9_video_board[APP_BOARD_CALCU_4-1].sdk, &v9_video_board[APP_BOARD_CALCU_4-1]);


			return OK;
		}
		return ERROR;
	}
	return OK;
}

int v9_video_board_exit()
{
	if(v9_video_board)
	{
		XFREE(MTYPE_VIDEO_MEDIA, v9_video_board);
		v9_video_board = NULL;
	}
	if(board_mutex)
	{
		os_mutex_exit(board_mutex);
		board_mutex = NULL;
	}
	return OK;
}

int v9_video_board_add(zpl_uint32 id)
{
	zpl_uint32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use == zpl_false)
		{
			v9_video_board[i].use = zpl_true;
			v9_video_board[i].id = id;
			return OK;
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not add video board %d (ID) data.", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}

int v9_video_board_del(zpl_uint32 id)
{
	zpl_uint32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			v9_video_board[i].active = zpl_false;
			v9_video_board[i].use = zpl_false;
			v9_video_board[i].id = 0;
			return OK;
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not delete video board %d (ID) data.", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}

v9_video_board_t * v9_video_board_lookup(zpl_uint32 id)
{
	zpl_uint32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			return &v9_video_board[i];
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not lookup video board %d (ID) data.", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}

/*
 * SDK 连接上的时候调用设置
 */
int v9_video_board_active(zpl_uint32 id, zpl_bool enable)
{
	zpl_uint32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			v9_video_board[i].active = enable;
			if(enable)
			{
				v9_video_board[i].board.active = zpl_true;
				//v9_video_board[i].board.autoip = zpl_true;
			}
			return OK;
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not active video board %d (ID) data.", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}

/*
 * 通过SDK连接状态判定板卡是否可操作
 */
zpl_bool v9_video_board_isactive(zpl_uint32 id)
{
	zpl_uint32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			return v9_video_board[i].active;
		}
	}
	return zpl_false;
}

/*
 * 通过串口传过来的板卡参数；判定板卡是否在线等状态
 */
zpl_bool v9_board_ready(v9_video_board_t *vboard)
{
	if(vboard && vboard->board.online &&
						vboard->board.power &&
						vboard->board.active &&
						vboard->board.autoip /*&&
						vboard->board.startup*/)
					return zpl_true;
	return zpl_false;
}


int v9_board_set_ready(v9_video_board_t *vboard)
{
	if(vboard)
	{
		vboard->board.online = zpl_true;
		vboard->board.power = zpl_true;
		vboard->board.active = zpl_true;
		vboard->board.autoip = zpl_true;
		return OK;
	}
	return ERROR;
}


int v9_video_board_address(zpl_uint32 id, zpl_uint32 address, zpl_uint16 port)
{
	zpl_uint32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			v9_video_board[i].address = address;
			v9_video_board[i].port = port;
			return OK;
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not update video board %d (ID) address.", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}

int v9_video_board_get_vch(zpl_uint32 id)
{
	zpl_uint32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			return v9_video_board[i].channel_cnt;
		}
	}
	return 0;//ERROR;
}


int v9_video_board_disabled(zpl_uint32 id, zpl_bool enable)
{
	zpl_uint32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			v9_video_board[i].disabled = enable;
			return OK;
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not disabled video board %d (ID) data.", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}

zpl_bool v9_video_board_isdisabled(zpl_uint32 id)
{
	zpl_uint32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			return v9_video_board[i].disabled;
		}
	}
	return zpl_false;
}
/********************************************************************/
/********************************************************************/
static int v9_video_board_stream_hw_sync(struct eloop *eloop)
{
	zpl_uint32 i = 0, j = 0;
	//sdk = ELOOP_ARG(eloop);
	if(!v9_video_board)
		return ERROR;
	v9_video_board[0].t_timeout = NULL;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board[i].use && !v9_video_board[i].disabled &&
				v9_video_board[i].active && v9_video_board[i].id != APP_BOARD_MAIN)
		{
			for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
			{
				if(v9_video_board[i].channel[j] == zpl_true &&
						v9_video_board[i].video_stream[j] != NULL &&
						strlen(v9_video_board[i].video_stream[j]->video_url) &&
						v9_video_board[i].video_stream[j]->hw_sync == zpl_false)
				{
					if(v9_video_sdk_add_vch_api(v9_video_board[i].id,
												v9_video_board[i].video_stream[j]->ch,
												v9_video_board[i].video_stream[j]->video_url) == OK)
						v9_video_board[i].video_stream[j]->hw_sync = zpl_true;
				}
			}
		}
	}

	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board[i].use && !v9_video_board[i].disabled &&
				v9_video_board[i].active && v9_video_board[i].id != APP_BOARD_MAIN)
		{
			for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
			{
				if(v9_video_board[i].channel[j] == zpl_true &&
						v9_video_board[i].video_stream[j] != NULL &&
						strlen(v9_video_board[i].video_stream[j]->video_url) &&
						v9_video_board[i].video_stream[j]->hw_sync == zpl_false)
				{
					if(v9_video_board[0].sdk.master)
						v9_video_board[0].t_timeout = eloop_add_timer(v9_video_board[0].sdk.master,
																   v9_video_board_stream_hw_sync, NULL, 3);
					return OK;
				}
			}
		}
	}
	return OK;
}

static int __v9_video_stream_add(v9_video_board_t *board, v9_video_stream_t *value)
{
	zpl_uint32 i = 0;
	zassert(board);
	zassert(value);
	for(i = 0; i < V9_APP_CHANNEL_MAX; i++)
	{
		if(board->video_stream[i] == NULL /*&& value->video_url*/)
		{
			board->channel[i] = zpl_true;
			board->video_stream[i] = value;
			value->id = board->id;
			board->video_load += V9_APP_VIDEO_LOAD(value->fps, 1);
			if(strlen(value->video_url))
			{
				if(v9_video_board_isactive(board->id))
				{
					if(v9_video_sdk_add_vch_api(board->id, value->ch, value->video_url) == OK)
						value->hw_sync = zpl_true;
				}
				if(value->hw_sync == zpl_false)
				{
					if(v9_video_board[0].t_timeout)
					{
						eloop_cancel(v9_video_board[0].t_timeout);
						v9_video_board[0].t_timeout = NULL;
					}
					if(v9_video_board[0].sdk.master)
						v9_video_board[0].t_timeout = eloop_add_timer(v9_video_board[0].sdk.master,
																   v9_video_board_stream_hw_sync, NULL, 3);
				}
			}
			return OK;
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not add video stream to board %d(ID).", V9_APP_BOARD_HW_ID(board->id));
	return ERROR;
}

static int __v9_video_stream_del(v9_video_board_t *board, v9_video_stream_t *value)
{
	zpl_uint32 i = 0;
	zassert(board);
	zassert(value);
	for(i = 0; i < V9_APP_CHANNEL_MAX; i++)
	{
		if(board->video_stream[i] && board->video_stream[i] == value)
		{
			board->video_load -= V9_APP_VIDEO_LOAD(value->fps, 1);
			board->channel[i] = zpl_false;
			value->id = 0;
			board->video_stream[i] = NULL;
			if(value->hw_sync)
				return v9_video_sdk_del_vch_api(board->id, value->ch);
			return OK;
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not del video stream from board %d(ID).", V9_APP_BOARD_HW_ID(board->id));
	return ERROR;
}

static v9_video_stream_t * __v9_video_stream_lookup(v9_video_board_t *board, v9_video_stream_t *value)
{
	zpl_uint32 i = 0;
	zassert(board);
	zassert(value);
	for(i = 0; i < V9_APP_CHANNEL_MAX; i++)
	{
		if(board->video_stream[i] && board->video_stream[i] == value)
		{
			return board->video_stream[i];
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not lookup video stream on board %d(ID).", V9_APP_BOARD_HW_ID(board->id));
	return NULL;
}
/********************************************************************/
/********************************************************************/
v9_video_stream_t * v9_video_board_stream_lookup_by_id_and_ch(zpl_uint8 id, zpl_uint8 ch)
{
	zpl_uint32 i = 0;
	if(id > V9_APP_BOARD_MAX)
		return NULL;
	v9_video_board_t *board = v9_video_board_lookup(id);
	if(!board)
		return NULL;
	if(board->channel_cnt == 0)
		return NULL;
	for(i = 0; i < V9_APP_CHANNEL_MAX; i++)
	{
		if(board->video_stream[i] && board->video_stream[i] && board->video_stream[i]->ch == ch)
		{
			return board->video_stream[i];
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not lookup video stream by board ID:%d VCH:%d.", V9_APP_BOARD_HW_ID(id), ch);
	return NULL;
}


int v9_video_board_stream_status_change(zpl_uint8 id, zpl_uint8 ch, int rtsp, int decode)
{
	v9_video_stream_t * stream = v9_video_board_stream_lookup_by_id_and_ch(id, ch);
	if(stream)
	{
		//stream->connect;				//视频流连接状态
		//stream->dev_status = rtsp;
		if(rtsp != -1)
			stream->rtsp_status = rtsp; // ==1 OK ==2 ERROR
		if(decode != -1)
			stream->decode_status = 1 - decode; // ==0 OK ==1 ERROR								// 解码状态

		if(stream->rtsp_status == EAIS_DEVICE_STATUS_ONLINE || stream->decode_status == zpl_true)
			stream->connect = zpl_true;
		else
			stream->connect = zpl_false;
		//stream->change;
		return OK;
	}
	return ERROR;
}
/*
 * 获取最小负载的计算板ID
 * 计算板应该是激活正在运行的
 */
int v9_video_board_get_minload()
{
	zpl_uint32 i = 0, video_load = 0xfffffff0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && !v9_video_board[i].disabled &&
				v9_video_board[i].id && v9_video_board[i].active)
		{
			video_load = MIN(v9_video_board[i].video_load, video_load);
		}
	}
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && !v9_video_board[i].disabled &&
				v9_video_board[i].id && v9_video_board[i].active)
		{
			if(video_load == v9_video_board[i].video_load)
				return v9_video_board[i].id;
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not get min vidoe load board ID.");
	return ERROR;
}


int v9_video_board_stream_alloc(zpl_uint32 id)
{
	zpl_uint32 i = 0, j = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && v9_video_board[i].id == id)
		{
			for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
			{
				if(v9_video_board[i].channel[j] == 0 && v9_video_board[i].video_stream[j] == NULL)
				{
					return j + 1;
				}
			}
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not alloc video stream by board ID:%d.", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}


int v9_video_board_stream_add(zpl_uint32 id, v9_video_stream_t *value, zpl_bool load)
{
	zpl_uint32 i = 0;
	//zlog_debug(MODULE_APP," %s ID=%d", __func__, V9_APP_BOARD_HW_ID(id));
	//zassert(board);
	zassert(value);
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && v9_video_board[i].id == id)
		{

			if(__v9_video_stream_lookup(&v9_video_board[i], value))
			{
				return ERROR;
			}
			else
			{
				//zlog_debug(MODULE_APP," %s BID=%d", __func__, V9_APP_BOARD_HW_ID(v9_video_board[i].id));
				if(__v9_video_stream_add(&v9_video_board[i], value) == OK)
				{
					v9_video_board[i].channel_cnt++;
					return OK;
				}
				return ERROR;
			}
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not add video stream to board ID:%d.", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}

int v9_video_board_stream_del(zpl_uint32 id, v9_video_stream_t *value)
{
	zpl_uint32 i = 0;
	//zassert(board);
	zassert(value);
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && v9_video_board[i].id == id)
		{
			if(!__v9_video_stream_lookup(&v9_video_board[i], value))
				return ERROR;
			else
			{
				if(__v9_video_stream_del(&v9_video_board[i], value) == OK)
				{
					v9_video_board[i].channel_cnt--;
					return OK;
				}
				return ERROR;
			}
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not del video stream from board ID:%d.", V9_APP_BOARD_HW_ID(id));
	return ERROR;
}

int v9_video_board_stream_lookup(zpl_uint32 id, v9_video_stream_t *value)
{
	zpl_uint32 i = 0;
	//zassert(board);
	zassert(value);
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && v9_video_board[i].id == id)
		{
			if(!__v9_video_stream_lookup(&v9_video_board[i], value))
				return ERROR;
			else
			{
				return OK;
			}
		}
	}
	return ERROR;
}

/********************************************************************/
/********************************************************************/
#if 0
int v9_video_stream_split(char *url, zpl_uint8 *ch, zpl_uint32 *address, zpl_uint16 *port,
							char *username, char *password, char *param, char *secondary)
{
	os_url_t spliurl;
	memset(&spliurl, 0, sizeof(os_url_t));
	if (os_url_split(url, &spliurl) != OK)
	{
		os_url_free(&spliurl);
		return ERROR;
	}
	//os_url_debug_test("rtsp://admin:abc123456@192.168.3.64:554/av0_0");
	//os_url_debug_test("rtsp://admin:abc123456@192.168.1.64/av0_0");
	if(spliurl.host)
	{
		if(address)
			*address = ntohl(inet_addr(spliurl.host));
	}
	if(spliurl.port != 0)
	{
		if(port)
			*port = spliurl.port;
	}
	else
	{
		if(port)
			*port = 554;
	}
	if(spliurl.user)
	{
		if(username)
			strcpy(username, spliurl.user);
	}
	if(spliurl.pass)
	{
		if(password)
			strcpy(password, spliurl.pass);
	}
	if(spliurl.filename)//av0_0
	{
		if(param)
			strcpy(param, spliurl.filename);
		if(strstr(spliurl.filename, "channel"))
		{
			char *b = strstr(spliurl.filename, "channel");
			if(b)
			{
				b += strlen("channel")+1;
				if(ch && b)
					*ch = atoi(b);
			}
			if(strstr(spliurl.filename, "subtype"))//大华
			{
				//rtsp://username:password@ip:port/cam/realmonitor?channel=1&subtype=0
				//请求某设备的通道2的辅码流，Url如下
				//rtsp://admin:admin@10.12.4.84:554/cam/realmonitor?channel=2&subtype=1
			}
		}
		else if(strstr(spliurl.filename, "av"))//海康
		{
			//rtsp://admin:abc123456@192.168.1.64:554/av0_0
			//rtsp://admin:12345@192.0.0.64/mpeg4/ch1/sub/av0_0
			if(ch)
				*ch = atoi(spliurl.filename+4) + 1;
/*			if(secondary)
			{
				strcpy(secondary, spliurl.filename);
			}*/
		}
	}
	os_url_free(&spliurl);
	return OK;
}
#endif
/********************************************************************/
/********************************************************************/
int v9_video_board_stream_cleanup()
{
	zpl_uint32 i = 0, j = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use)
		{
			for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
			{
				if(v9_video_board[i].video_stream[j] && v9_video_board[i].channel[j])
				{
					v9_video_board[i].video_load -= V9_APP_VIDEO_LOAD(v9_video_board[i].video_stream[j]->fps, 1);
					v9_video_board[i].channel[i] = zpl_false;
					if(v9_video_board[i].video_stream[j]->hw_sync)
						v9_video_sdk_del_vch_api(v9_video_board[i].id, v9_video_board[i].video_stream[j]->ch);
					v9_video_board[i].channel_cnt--;
					v9_video_board_stream_free_api(v9_video_board[i].video_stream[i]);
					v9_video_board[i].video_stream[j]->id = 0;
					v9_video_board[i].video_stream[i] = NULL;
				}
			}
		}
	}
	return OK;
}
/********************************************************************/
v9_video_stream_t * v9_video_board_stream_alloc_api(zpl_uint8 ch, zpl_uint32 address, zpl_uint16 port,
												char *username, char *password, zpl_uint32 fps,
												char *param, char *secondary)
{
	v9_video_stream_t *value = XMALLOC(MTYPE_VIDEO_STREAM, sizeof(v9_video_stream_t));
	if(value)
	{
		os_memset(value, 0, sizeof(v9_video_stream_t));
		value->ch		= ch;					//通道
		value->address	= address;			//IP 地址
		value->port		= port ? port:RTSP_PORT_DEFAULT;				//RTSP 端口号
		value->fps = fps ? fps:V9_APP_VIDEO_FPS_DEFAULT;
		value->proto = V9_VIDEO_PROTO_RTSP;				//协议 RTSP
		if(username)
			os_strcpy(value->username, username);		//用户名
		if(password)
			os_strcpy(value->password, password);		//密码

		if(param)
			os_strcpy(value->mainstream, param);		//用户名
		if(secondary)
			os_strcpy(value->secondary, secondary);		//密码

		value->connect = zpl_false;				//视频流连接状态
		value->dev_status = 0;					// EAIS设备状态，默认离线。 ENUM_EAIS_DEVICE_STATUS
		value->rtsp_status = 0;									// 通道RTSP状态 ENUM_EAIS_DEVICE_STATUS
		value->decode_status = 0;									// 解码状态
		value->change = zpl_false;
		value->hw_sync = zpl_false;
		if(param)
		{
			if(username)
				sprintf(value->video_url, "rtsp://%s:%s@%s:%d/%s", username, password,
						inet_address(address), port? port:RTSP_PORT_DEFAULT, value->mainstream);
			else
				sprintf(value->video_url, "rtsp://%s:%d/%s",
						inet_address(address), port? port:RTSP_PORT_DEFAULT, value->mainstream);
		}
		else
		{
			//XFREE(MTYPE_VIDEO_STREAM, value);
			//return NULL;
		}
		if(strlen(value->video_url))
			V9_DEBUG("video_url:%s", value->video_url);
/*
		if(username)
			sprintf(value->video_url, "rtsp://%s:%s@%s:%d/av0_%d", username, password,
					inet_address(address), port? port:RTSP_PORT_DEFAULT, V9_APP_BOARD_HW_CH(ch));
		else
			sprintf(value->video_url, "rtsp://%s:%d/av0_%d",
					inet_address(address), port? port:RTSP_PORT_DEFAULT, V9_APP_BOARD_HW_CH(ch));
*/
		return value;
	}
	return NULL;
}

v9_video_stream_t * v9_video_board_stream_lookup_api(zpl_uint8 ch, zpl_uint32 address, zpl_uint16 port)
{
	zpl_uint32 i = 0, j = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && v9_video_board[i].id != APP_BOARD_MAIN)
		{
			for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
			{
				if(v9_video_board[i].video_stream[j] && v9_video_board[i].channel[j])
				{
					if(ch)
					{
						if(v9_video_board[i].video_stream[j]->ch == ch  &&
									v9_video_board[i].video_stream[j]->address == address &&
									v9_video_board[i].video_stream[j]->port == port)
						{
							return v9_video_board[i].video_stream[i];
						}
					}
					else
					{
						if(v9_video_board[i].video_stream[j]->address == address &&
							v9_video_board[i].video_stream[j]->port == port)
							return v9_video_board[i].video_stream[i];
					}
				}
			}
		}
	}
	return NULL;
}


int v9_video_board_stream_update_api(zpl_uint8 id, zpl_uint8 ch, char *param, char *secondary)
{
	v9_video_board_t * board = NULL;
	v9_video_stream_t * stream = NULL;
	board = v9_video_board_lookup(id);
	stream = v9_video_board_stream_lookup_by_id_and_ch(id, ch);
	if(board && stream)
	{
		//zpl_uint32 i = 0;
		if(param)
		{
			memset(stream->mainstream, 0, sizeof(stream->mainstream));
			os_strcpy(stream->mainstream, param);		//用户名
		}
		if(secondary)
		{
			memset(stream->secondary, 0, sizeof(stream->secondary));
			os_strcpy(stream->secondary, secondary);		//密码
		}

		if(strlen(stream->mainstream))
		{
			if(strlen(stream->username))
				sprintf(stream->video_url, "rtsp://%s:%s@%s:%d/%s", stream->username, stream->password,
						inet_address(stream->address), stream->port? stream->port:RTSP_PORT_DEFAULT, stream->mainstream);
			else
				sprintf(stream->video_url, "rtsp://%s:%d/%s",
						inet_address(stream->address), stream->port? stream->port:RTSP_PORT_DEFAULT, stream->mainstream);
		}
		if(v9_video_board_isactive(board->id))
		{
			if(v9_video_sdk_add_vch_api(board->id, stream->ch, stream->video_url) == OK)
				stream->hw_sync = zpl_true;
		}
		if(stream->hw_sync == zpl_false)
		{
			if(v9_video_board[0].t_timeout)
			{
				eloop_cancel(v9_video_board[0].t_timeout);
				v9_video_board[0].t_timeout = NULL;
			}
			if(v9_video_board[0].sdk.master)
				v9_video_board[0].t_timeout = eloop_add_timer(v9_video_board[0].sdk.master,
														   v9_video_board_stream_hw_sync, NULL, 3);
		}

/*		for(i = 0; i < V9_APP_CHANNEL_MAX; i++)
		{
			if(board->video_stream[i] != NULL && board->video_stream[i] == stream)
			{
				if(strlen(board->video_stream[i]->video_url))
					return v9_video_sdk_add_vch_api(board->id, board->video_stream[i]->ch, board->video_stream[i]->video_url);
				return OK;
			}
		}*/
		return OK;
	}
	return ERROR;
}


int v9_video_board_stream_free_api(v9_video_stream_t *v)
{
	if(v)
	{
/*		if(v->video_url)
			XFREE(MTYPE_VIDEO_STREAM, v->video_url);*/
		XFREE(MTYPE_VIDEO_STREAM, v);
		return OK;
	}
	return ERROR;
}

/*
 * 根据视频流信息获取计算板ID
 */
int v9_video_board_ID_lookup_api_by_video_stream(zpl_uint8 ch, zpl_uint32 address, zpl_uint16 port)
{
	zpl_uint32 i = 0, j = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].active && v9_video_board[i].use && !v9_video_board[i].disabled)
		{
			for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
			{
				if(v9_video_board[i].video_stream[j] && v9_video_board[i].channel[j])
				{
					if(ch)
					{
						if(v9_video_board[i].video_stream[j]->ch == ch  &&
									v9_video_board[i].video_stream[j]->address == address &&
									v9_video_board[i].video_stream[j]->port == port)
						{
							return v9_video_board[i].id;
						}
					}
					else
					{
						if(v9_video_board[i].video_stream[j]->address == address &&
							v9_video_board[i].video_stream[j]->port == port)
							return v9_video_board[i].id;
					}
				}
			}
		}
	}
	if(V9_APP_DEBUG(BOARD_EVENT))
		zlog_debug(MODULE_APP," can not lookup video stream by url (%s:%d/vd-%d).", inet_address(address), port, ch);
	return ERROR;
}
/********************************************************************/
/********************************************************************/

static int v9_video_board_stream_show_one(struct vty *vty, v9_video_stream_t *pstNode, zpl_bool detail)
{
	if (pstNode)
	{
		vty_out (vty, "-------------------------------------------%s", VTY_NEWLINE);
		vty_out (vty, "  Channel ID            : %d:%d%s", V9_APP_BOARD_HW_ID(pstNode->id), pstNode->ch, VTY_NEWLINE);
		vty_out (vty, "   Address              : %s:%d%s", inet_address(pstNode->address), pstNode->port, VTY_NEWLINE);
		vty_out (vty, "   FPS                  : %d%s", pstNode->fps, VTY_NEWLINE);
		vty_out (vty, "   Connect              : %s%s", pstNode->connect ? "zpl_true":"zpl_false", VTY_NEWLINE);
		vty_out (vty, "   HW Sync              : %s%s", pstNode->hw_sync ? "zpl_true":"zpl_false", VTY_NEWLINE);
		if(strlen(pstNode->username) && strlen(pstNode->password))
		{
			zpl_uint8 passecrypt[64];
			memset(passecrypt, 0, sizeof(passecrypt));
			md5_encrypt_password(pstNode->password, passecrypt);
			vty_out (vty, "   Username             : %s%s", pstNode->username, VTY_NEWLINE);
			vty_out (vty, "   Password             : %s%s", passecrypt, VTY_NEWLINE);
		}
#ifdef V9_VIDEO_SDK_API
		if(pstNode->type == V9_VIDEO_STREAM_TYPE_STATIC)
			vty_out (vty, "   Type                 : %s%s", "Static", VTY_NEWLINE);
		else if(pstNode->type == V9_VIDEO_STREAM_TYPE_DYNAMIC)
			vty_out (vty, "   Type                 : %s%s", "Dynamic", VTY_NEWLINE);
		else
			vty_out (vty, "   Type                 : %s%s", "Unknown", VTY_NEWLINE);

		if(detail)
		{
			vty_out (vty, "   Protocol             : %s%s", "RTSP", VTY_NEWLINE);
			switch (pstNode->rtsp_status)
			{
				case EAIS_DEVICE_STATUS_ONLINE:					// 在线或RTSP有效
					vty_out (vty,"   RTSP Status          : %s%s","ACTIVE", VTY_NEWLINE);
					break;
				case EAIS_DEVICE_STATUS_OFFLINIE:				// 离线或RTSP无效
					vty_out (vty,"   RTSP Status          : %s%s", "INACTIVE", VTY_NEWLINE);
					break;
				case EAIS_DEVICE_STATUS_AUTH_ERR:					// 鉴权失败
					vty_out (vty,"   RTSP Status          : %s%s","Auth Failed", VTY_NEWLINE);
					break;
				case EAIS_DEVICE_STATUS_PARSE_ERR:				// RTSP不兼容
					vty_out (vty,"   RTSP Status          : %s%s","RTSP Incompatible", VTY_NEWLINE);
					break;
				default:
					vty_out (vty,"   RTSP Status          : %s%s","Unknown", VTY_NEWLINE);
					break;
			}
			if (pstNode->decode_status == zpl_true)
				vty_out (vty, "   Decode Status        : %s%s","Successful ", VTY_NEWLINE);
			else
				vty_out (vty, "   Decode Status        : %s%s","ABORT", VTY_NEWLINE);
			//if(pstNode->video_url)
				vty_out (vty, "   Video URL            : %s%s",pstNode->video_url, VTY_NEWLINE);
/*			else
				vty_out (vty, "   Video URL            : null%s", VTY_NEWLINE);*/

			if(strlen(pstNode->mainstream))
				vty_out (vty, "   RTSP Main Param      : %s%s", pstNode->mainstream, VTY_NEWLINE);
			if(strlen(pstNode->secondary))
				vty_out (vty, "   RTSP Second Param    : %s%s", pstNode->secondary, VTY_NEWLINE);
		}
#endif
		vty_out (vty, "-------------------------------------------%s", VTY_NEWLINE);
	}
	return OK;
}

/*
 * 显示计算板视频流信息（计算板在线激活的）
 */
int v9_video_board_stream_show(struct vty *vty, zpl_uint32 id, zpl_bool detail)
{
	zpl_uint32 i = 0, j = 0;
	v9_video_board_lock();
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && !v9_video_board[i].disabled && v9_video_board[i].active)
		{
#ifdef V9_VIDEO_SDK_API
			if(v9_video_board[i].id != APP_BOARD_MAIN)
				v9_video_sdk_get_rtsp_status_api(v9_video_board[i].id);
#endif
			if( (id != 0 && v9_video_board[i].id == id) )
			{
				for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
				{
					if(v9_video_board[i].channel[j] == zpl_true && v9_video_board[i].video_stream[j] != NULL)
					{
						v9_video_board_stream_show_one(vty, v9_video_board[i].video_stream[j],  detail);
					}
				}
				v9_video_board_unlock();
				return OK;
			}
			else if(id == 0)
			{
				for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
				{
					if(v9_video_board[i].channel[j] == zpl_true && v9_video_board[i].video_stream[j] != NULL)
					{
						v9_video_board_stream_show_one(vty, v9_video_board[i].video_stream[j],  detail);
					}
				}
			}
		}
	}
	v9_video_board_unlock();
	return OK;
}

/********************************************************************/
static int v9_video_board_show_one(struct vty *vty, v9_video_board_t *board, zpl_bool detail)
{
	vty_out (vty, "-------------------------------------------%s", VTY_NEWLINE);
	if(board->disabled)
	{
		if(board->id == APP_BOARD_MAIN)
			vty_out (vty, "  Main Board ID         : %d disabled%s", V9_APP_BOARD_HW_ID(board->id), VTY_NEWLINE);
		else
			vty_out (vty, "  Board ID              : %d disabled%s", V9_APP_BOARD_HW_ID(board->id), VTY_NEWLINE);
		vty_out (vty, "   Address              : %s:%d%s", inet_address(board->address), board->port, VTY_NEWLINE);
		//if(detail)
		{
			//vty_out (vty, "   Channel              : %d%s", board->active ? "zpl_true":"zpl_false", VTY_NEWLINE);
			vty_out (vty, "   Active               : %s%s", board->active ? "zpl_true":"zpl_false", VTY_NEWLINE);
		}
		vty_out (vty, "-------------------------------------------%s", VTY_NEWLINE);
		return OK;
	}
	if(board->id == APP_BOARD_MAIN)
		vty_out (vty, "  Main Board ID         : %d enable%s", V9_APP_BOARD_HW_ID(board->id), VTY_NEWLINE);
	else
		vty_out (vty, "  Board ID              : %d enable%s", V9_APP_BOARD_HW_ID(board->id), VTY_NEWLINE);
	vty_out (vty, "   Address              : %s:%d%s", inet_address(board->address), board->port, VTY_NEWLINE);
	vty_out (vty, "   Load                 : %d%s", board->video_load, VTY_NEWLINE);
	if(detail)
	{
		if(board->channel_cnt)
		{
			zpl_uint32 i = 0;
			char tmp[32];
			char tmpbuf[128];
			memset(tmpbuf, 0, sizeof(tmpbuf));
			for(i = 0; i < V9_APP_CHANNEL_MAX; i++)
			{
				if(board->channel[i])
				{
					memset(tmp, 0, sizeof(tmp));
					if(i!=0)
						strcat(tmpbuf, ",");
					sprintf(tmp, "%d", i+1);
					strcat(tmpbuf, tmp);
				}
			}
			vty_out (vty, "   Channel              : %d(%s)%s", board->channel_cnt, tmpbuf, VTY_NEWLINE);
		}
		else
			vty_out (vty, "   Channel              : %d%s", board->channel_cnt, VTY_NEWLINE);
	}
	else
		vty_out (vty, "   Channel              : %d%s", board->channel_cnt, VTY_NEWLINE);

	//if(detail)
	{
		//vty_out (vty, "   Channel              : %d%s", board->active ? "zpl_true":"zpl_false", VTY_NEWLINE);
		vty_out (vty, "   Active               : %s%s", board->active ? "zpl_true":"zpl_false", VTY_NEWLINE);
	}
	vty_out (vty, "-------------------------------------------%s", VTY_NEWLINE);
	return OK;
}

/*
 * 显示板卡信息
 */
int v9_video_board_show(struct vty *vty, zpl_bool detail)
{
	zpl_uint32 i = 0;
	v9_video_board_lock();
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && !v9_video_board[i].disabled/*&& v9_video_board[i].active*/)
		{
			v9_video_board_show_one(vty, &v9_video_board[i],  detail);
		}
	}
	v9_video_board_unlock();
	return OK;
}
/****************************** SDK **********************************/
/*
"video stream channel <1-4> ip A.B.C.D port <1-65530> username USE password PASS fps <480-1080>",
"video stream channel <1-4> ip A.B.C.D port <1-65530> username USE password PASS",
"video stream ip A.B.C.D port <1-65530> username USE password PASS",
*/

static int v9_video_board_stream_write_config_one(struct vty *vty, v9_video_stream_t *pstNode)
{
	if (pstNode)
	{
		zpl_uint32 len = 0;
		char confstr[1024];
		memset(confstr, 0, sizeof(confstr));

		len = snprintf(confstr, sizeof(confstr), "%s", "video stream ");
		//zlog_debug(MODULE_WEB, "=====%s====type=%d", __func__,pstNode->type);
		if(pstNode->type == V9_VIDEO_STREAM_TYPE_STATIC)
		{
			snprintf(confstr + len, sizeof(confstr) - len, "board %d channel %d ",V9_APP_BOARD_HW_ID(pstNode->id), pstNode->ch);
		}
		else if(pstNode->type == V9_VIDEO_STREAM_TYPE_DYNAMIC_CH)
		{
			snprintf(confstr + len, sizeof(confstr) - len, "board %d channel 0 ",V9_APP_BOARD_HW_ID(pstNode->id));
		}
		else if(pstNode->type == V9_VIDEO_STREAM_TYPE_DYNAMIC_ID)
		{
			snprintf(confstr + len, sizeof(confstr) - len, "board 0 channel %d ",pstNode->ch);
		}
		else if(pstNode->type == V9_VIDEO_STREAM_TYPE_DYNAMIC)
		{
			snprintf(confstr + len, sizeof(confstr) - len, "board 0 channel 0 ");
		}
		else
		{
			snprintf(confstr + len, sizeof(confstr) - len, "board %d channel %d ",V9_APP_BOARD_HW_ID(pstNode->id), pstNode->ch);
		}

		len = strlen(confstr);
		if(pstNode->port != RTSP_PORT_DEFAULT)
		{
			snprintf(confstr + len, sizeof(confstr) - len, "ip %s port %d ",inet_address(pstNode->address), pstNode->port);
		}
		else
		{
			snprintf(confstr + len, sizeof(confstr) - len, "ip %s ",inet_address(pstNode->address));
		}

		len = strlen(confstr);
		if(strlen(pstNode->username) && strlen(pstNode->password))
		{
			if(vty->type == VTY_FILE)
			{
				snprintf(confstr + len, sizeof(confstr) - len, "username %s password %s ",
						 pstNode->username, pstNode->password);
			}
			else
			{
				zpl_uint8 passecrypt[64];
				memset(passecrypt, 0, sizeof(passecrypt));
				md5_encrypt_password(pstNode->password, passecrypt);
				snprintf(confstr + len, sizeof(confstr) - len, "username %s password %s ",
						 pstNode->username, passecrypt);
			}
		}

		vty_out (vty, " %s%s",confstr, VTY_NEWLINE);

		if(strlen(pstNode->mainstream))
			vty_out (vty, " video stream board %d channel %d mainstream %s%s", V9_APP_BOARD_HW_ID(pstNode->id),
					 pstNode->ch, pstNode->mainstream, VTY_NEWLINE);
		if(strlen(pstNode->secondary))
			vty_out (vty, " video stream board %d channel %d secondary %s%s", V9_APP_BOARD_HW_ID(pstNode->id),
					 pstNode->ch, pstNode->secondary, VTY_NEWLINE);

	}
	return OK;
}

void * v9_video_app_tmp()
{
	return v9_video_board;
}

#ifdef V9_VIDEO_SDK_API
static int v9_video_sdk_write_config_one(struct vty *vty, zpl_uint32 id, v9_video_sdk_t *sdk)
{
	return v9_video_sdk_get_config(vty, id, sdk);
}
#endif

int v9_video_board_stream_write_config(struct vty *vty)
{
	zpl_uint32 i = 0, j = 0;
	v9_video_board_lock();
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && v9_video_board[i].active)
		{
			if(v9_video_board[i].id)
			{
				if(!v9_video_board[i].disabled)
				{
					for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
					{
						if(v9_video_board[i].channel[j] && v9_video_board[i].video_stream[j])
						{
							v9_video_board_stream_write_config_one(vty, v9_video_board[i].video_stream[j]);
						}
					}
				}
				else
				{
					vty_out (vty, " video board %d disabled%s", V9_APP_BOARD_HW_ID(v9_video_board[i].id), VTY_NEWLINE);
				}
			}
		}
	}
/*
#ifdef V9_VIDEO_SDK_API
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && v9_video_board[i].active)
		{
			if(v9_video_board[i].id)
			{
				if(!v9_video_board[i].disabled)
				{
					v9_video_sdk_write_config_one(vty, v9_video_board[i].id, &v9_video_board[i].sdk);
				}
			}
		}
	}
#endif
*/
	v9_video_board_unlock();
	return OK;
}

int v9_video_sdk_config_show(struct vty *vty)
{
	zpl_uint32 i = 0;
	v9_video_board_lock();
#ifdef V9_VIDEO_SDK_API
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && v9_video_board[i].active)
		{
			if(v9_video_board[i].id)
			{
				if(!v9_video_board[i].disabled)
				{
					v9_video_sdk_write_config_one(vty, v9_video_board[i].id, &v9_video_board[i].sdk);
				}
			}
		}
	}
#endif
	v9_video_board_unlock();
	return OK;
}
