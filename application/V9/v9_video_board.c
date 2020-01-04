/*
 * v9_video_board.c
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


//Newifi-D1
v9_video_board_t *v9_video_board = NULL;

/*********************************************************************/
int v9_video_board_init()
{
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
			v9_video_board_address(APP_BOARD_CALCU_1, APP_BOARD_ADDRESS_PREFIX + APP_BOARD_CALCU_1, 8888);
			v9_board_init(APP_BOARD_CALCU_1, &v9_video_board[APP_BOARD_CALCU_1-1].board);

			v9_video_board_add(APP_BOARD_CALCU_2);
			v9_video_board_address(APP_BOARD_CALCU_2, APP_BOARD_ADDRESS_PREFIX + APP_BOARD_CALCU_2, 8888);
			v9_board_init(APP_BOARD_CALCU_2, &v9_video_board[APP_BOARD_CALCU_2-1].board);

			v9_video_board_add(APP_BOARD_CALCU_3);
			v9_video_board_address(APP_BOARD_CALCU_3, APP_BOARD_ADDRESS_PREFIX + APP_BOARD_CALCU_3, 8888);
			v9_board_init(APP_BOARD_CALCU_3, &v9_video_board[APP_BOARD_CALCU_3-1].board);

			v9_video_board_add(APP_BOARD_CALCU_4);
			v9_video_board_address(APP_BOARD_CALCU_4, APP_BOARD_ADDRESS_PREFIX + APP_BOARD_CALCU_4, 8888);
			v9_board_init(APP_BOARD_CALCU_4, &v9_video_board[APP_BOARD_CALCU_4-1].board);

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
	return OK;
}

int v9_video_board_add(u_int32 id)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use == FALSE)
		{
			v9_video_board[i].use = TRUE;
			v9_video_board[i].id = id;
			return OK;
		}
	}
	zlog_debug(ZLOG_APP," can not add video board %d (ID) data.", id);
	return ERROR;
}

int v9_video_board_del(u_int32 id)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			v9_video_board[i].active = FALSE;
			v9_video_board[i].use = FALSE;
			v9_video_board[i].id = 0;
			return OK;
		}
	}
	zlog_debug(ZLOG_APP," can not delete video board %d (ID) data.", id);
	return ERROR;
}

v9_video_board_t * v9_video_board_lookup(u_int32 id)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			return &v9_video_board[i];
		}
	}
	zlog_debug(ZLOG_APP," can not lookup video board %d (ID) data.", id);
	return ERROR;
}

int v9_video_board_active(u_int32 id, BOOL enable)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			v9_video_board[i].active = enable;
			return OK;
		}
	}
	zlog_debug(ZLOG_APP," can not active video board %d (ID) data.", id);
	return ERROR;
}

BOOL v9_video_board_isactive(u_int32 id)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			return v9_video_board[i].active;
		}
	}
	return FALSE;
}

int v9_video_board_address(u_int32 id, u_int32 address, u_int16 port)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			v9_video_board[i].address = address;
			v9_video_board[i].port = port;
			return OK;
		}
	}
	zlog_debug(ZLOG_APP," can not update video board %d (ID) address.", id);
	return ERROR;
}

int v9_video_board_get_vch(u_int32 id)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			//v9_video_board[i].address;
			return v9_video_board[i].channel_cnt;
		}
	}
	//zlog_debug(ZLOG_APP," can not update video board %d (ID) address.", id);
	return 0;//ERROR;
}


int v9_video_board_disabled(u_int32 id, BOOL enable)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			v9_video_board[i].disabled = enable;
			return OK;
		}
	}
	zlog_debug(ZLOG_APP," can not disabled video board %d (ID) data.", id);
	return ERROR;
}

BOOL v9_video_board_isdisabled(u_int32 id)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].id == id)
		{
			return v9_video_board[i].disabled;
		}
	}
	return FALSE;
}
/********************************************************************/
/********************************************************************/
static int v9_video_board_video_channel_add(v9_video_board_t *board, v9_video_channel_t *value)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_CHANNEL_MAX; i++)
	{
		if(board->video_stream[i] == NULL)
		{
			board->channel[i] = TRUE;
			board->video_stream[i] = value;
			value->id = board->id;
			board->video_load += V9_APP_VIDEO_LOAD(value->fps, 1);
			return v9_video_sdk_add_vch_api(board->id, value->ch, value->video_url);
			//return OK;
		}
	}
	zlog_debug(ZLOG_APP," can not add video stream to board %d(ID).", board->id);
	return ERROR;
}

static int v9_video_board_video_channel_del(v9_video_board_t *board, v9_video_channel_t *value)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_CHANNEL_MAX; i++)
	{
		if(board->video_stream[i] && board->video_stream[i] == value)
		{
			board->video_load -= V9_APP_VIDEO_LOAD(value->fps, 1);
			board->channel[i] = FALSE;
			value->id = 0;
			board->video_stream[i] = NULL;
			return v9_video_sdk_del_vch_api(board->id, value->ch);
			//return OK;
		}
	}
	zlog_debug(ZLOG_APP," can not del video stream from board %d(ID).", board->id);
	return ERROR;
}

static v9_video_channel_t * v9_video_board_video_channel_lookup(v9_video_board_t *board, v9_video_channel_t *value)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_CHANNEL_MAX; i++)
	{
		if(board->video_stream[i] && board->video_stream[i] == value)
		{
			return board->video_stream[i];
		}
	}
	zlog_debug(ZLOG_APP," can not lookup video stream on board %d(ID).", board->id);
	return NULL;
}

v9_video_channel_t * v9_video_board_video_channel_lookup_by_id_and_ch(u_int8 id, u_int8 ch)
{
	u_int32 i = 0;
	if(id > V9_APP_BOARD_MAX)
		return NULL;
	v9_video_board_t *board = v9_video_board_lookup(id);
	for(i = 0; i < V9_APP_CHANNEL_MAX; i++)
	{
		if(board->video_stream[i] && board->video_stream[i] && board->video_stream[i]->ch == ch)
		{
			return board->video_stream[i];
		}
	}
	zlog_debug(ZLOG_APP," can not lookup video stream by board ID:%d VCH:%d.", id, ch);
	return NULL;
}
/********************************************************************/
/********************************************************************/
int v9_video_board_get_minload()
{
	u_int32 i = 0, video_load = 0xfffffff0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && !v9_video_board[i].disabled && v9_video_board[i].id && v9_video_board[i].active)
		{
			video_load = MIN(v9_video_board[i].video_load, video_load);
		}
	}
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && !v9_video_board[i].disabled && v9_video_board[i].id && v9_video_board[i].active)
		{
			if(video_load == v9_video_board[i].video_load)
				return v9_video_board[i].id;
		}
	}
	zlog_debug(ZLOG_APP," can not get min vidoe load board ID.");
	return ERROR;
}

int v9_video_board_channel_alloc(u_int32 id)
{
	u_int32 i = 0, j = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && !v9_video_board[i].disabled && v9_video_board[i].active && v9_video_board[i].id == id)
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
	zlog_debug(ZLOG_APP," can not alloc video stream by board ID:%d.", id);
	return ERROR;
}

int v9_video_board_channel_add(u_int32 id, v9_video_channel_t *value)
{
	u_int32 i = 0;
	zlog_debug(ZLOG_APP," %s ID=%d", __func__, id);
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && !v9_video_board[i].disabled && v9_video_board[i].active && v9_video_board[i].id == id)
		{
			if(v9_video_board_video_channel_lookup(&v9_video_board[i], value))
			{
				return ERROR;
			}
			else
			{
				zlog_debug(ZLOG_APP," %s BID=%d", __func__, v9_video_board[i].id);
				if(v9_video_board_video_channel_add(&v9_video_board[i], value) == OK)
				{
					v9_video_board[i].channel_cnt++;
					return OK;
				}
				return ERROR;
			}
		}
	}
	zlog_debug(ZLOG_APP," can not add video stream to board ID:%d.", id);
	return ERROR;
}

int v9_video_board_channel_del(u_int32 id, v9_video_channel_t *value)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && !v9_video_board[i].disabled && v9_video_board[i].active && v9_video_board[i].id == id)
		{
			if(!v9_video_board_video_channel_lookup(&v9_video_board[i], value))
				return ERROR;
			else
			{
				if(v9_video_board_video_channel_del(&v9_video_board[i], value) == OK)
				{
					v9_video_board[i].channel_cnt--;
					return OK;
				}
				return ERROR;
			}
		}
	}
	zlog_debug(ZLOG_APP," can not del video stream from board ID:%d.", id);
	return ERROR;
}

int v9_video_board_channel_lookup(u_int32 id, v9_video_channel_t *value)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && !v9_video_board[i].disabled && v9_video_board[i].active && v9_video_board[i].id == id)
		{
			if(!v9_video_board_video_channel_lookup(&v9_video_board[i], value))
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
v9_video_channel_t * v9_video_channel_alloc_api(u_int8 ch, u_int32 address, u_int16 port,
												char *username, char *password, u_int32 fps)
{
	v9_video_channel_t *value = XMALLOC(MTYPE_VIDEO_STREAM, sizeof(v9_video_channel_t));
	if(value)
	{
		os_memset(value, 0, sizeof(v9_video_channel_t));
		value->ch		= ch;					//通道
		value->address	= address;			//IP 地址
		value->port		= port ? port:RTSP_PORT_DEFAULT;				//RTSP 端口号
		value->fps = fps ? fps:V9_APP_VIDEO_FPS_DEFAULT;
		if(username)
			os_strcpy(value->username, username);		//用户名
		if(password)
			os_strcpy(value->password, password);		//密码
		value->proto = V9_VIDEO_PROTO_RTSP;				//协议 RTSP
		if(username)
			sprintf(value->video_url, "rtsp://%s:%s@%s:%d/av0_%d", username, password,
					inet_address(address), port? port:RTSP_PORT_DEFAULT, ch - 1);
		else
			sprintf(value->video_url, "rtsp://%s:%d/av0_%d",
					inet_address(address), port? port:RTSP_PORT_DEFAULT, ch - 1);
		//value->camera = NULL;

		return value;
	}
	return NULL;
}

v9_video_channel_t * v9_video_channel_lookup_api(u_int8 ch, u_int32 address, u_int16 port)
{
	u_int32 i = 0, j = 0;
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

/*int v9_video_channel_del_api(u_int8 ch, u_int32 address, u_int16 port)
{
	u_int32 i = 0, j = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].active && v9_video_board[i].use)
		{
			for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
			{
				if(v9_video_board[i].video_stream[j] && v9_video_board[i].channel[j])
				{
					if(v9_video_board[i].video_stream[j]->ch == ch &&
							v9_video_board[i].video_stream[j]->address == address &&
							v9_video_board[i].video_stream[j]->port == port)
					{
						v9_video_board[i].video_load -= V9_APP_VIDEO_LOAD(v9_video_board[i].video_stream[j]->fps, 1);
						v9_video_board[i].channel[j] = FALSE;
						v9_video_board[i].channel_cnt--;
						XFREE(MTYPE_VIDEO_STREAM, v9_video_board[i].video_stream[j]);
						v9_video_board[i].video_stream[j] = NULL;
						return OK;
					}

				}
			}
		}
	}
	return ERROR;
}*/

int v9_video_channel_free_api(v9_video_channel_t *v)
{
	if(v)
	{
		XFREE(MTYPE_VIDEO_STREAM, v);
		return OK;
	}
	return ERROR;
}

int v9_video_board_ID_lookup_api_by_video_channel(u_int8 ch, u_int32 address, u_int16 port)
{
	u_int32 i = 0, j = 0;
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
/*					if(v9_video_board[i].video_stream[j]->ch == ch &&
							v9_video_board[i].video_stream[j]->address == address &&
							v9_video_board[i].video_stream[j]->port == port)
					return v9_video_board[i].id;*/
				}
			}
		}
	}
	zlog_debug(ZLOG_APP," can not lookup video stream by url (%s:%d/vd-%d).", inet_address(address), port, ch);
	return ERROR;
}
/********************************************************************/
/********************************************************************/

static int v9_video_channel_show_one(struct vty *vty, v9_video_channel_t *pstNode, BOOL detail)
{
	if (pstNode)
	{
		vty_out (vty, "-------------------------------------------%s", VTY_NEWLINE);
		vty_out (vty, "  Channel ID            : %d:%d%s", pstNode->id, pstNode->ch, VTY_NEWLINE);
		vty_out (vty, "   Address              : %s:%d%s", inet_address(pstNode->address), pstNode->port, VTY_NEWLINE);
		vty_out (vty, "   FPS                  : %d%s", pstNode->fps, VTY_NEWLINE);
		vty_out (vty, "   Connect              : %s%s", pstNode->connect ? "TRUE":"FALSE", VTY_NEWLINE);

		if(strlen(pstNode->username) && strlen(pstNode->password))
		{
			vty_out (vty, "   Username             : %s%s", pstNode->username, VTY_NEWLINE);
			vty_out (vty, "   Password             : %s%s", pstNode->password, VTY_NEWLINE);
		}
		if(pstNode->type == V9_VIDEO_STREAM_TYPE_STATIC)
			vty_out (vty, "   Type                 : %s%s", "Static", VTY_NEWLINE);
		else if(pstNode->type == V9_VIDEO_STREAM_TYPE_STATIC)
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
			if (pstNode->decode_status == 0)
				vty_out (vty, "   Decode Status        : %s%s","Successful ", VTY_NEWLINE);
			else
				vty_out (vty, "   Decode Status        : %s%s","ABORT", VTY_NEWLINE);

			vty_out (vty, "   Video URL            : %s%s",pstNode->video_url, VTY_NEWLINE);
		}
		vty_out (vty, "-------------------------------------------%s", VTY_NEWLINE);
	}
	return OK;
}

int v9_video_channel_show(struct vty *vty, u_int32 id, BOOL detail)
{
	u_int32 i = 0, j = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && !v9_video_board[i].disabled && v9_video_board[i].active)
		{
			if( (id && v9_video_board[i].id == id) )
			{
				for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
				{
					if(v9_video_board[i].channel[j] && v9_video_board[i].video_stream[j])
					{
						v9_video_channel_show_one(vty, v9_video_board[i].video_stream[j],  detail);
					}
				}
				return OK;
			}
			else if(id == 0)
			{
				for(j = 0; j < V9_APP_CHANNEL_MAX; j++)
				{
					if(v9_video_board[i].channel[j] == TRUE && v9_video_board[i].video_stream[j] != NULL)
					{
						v9_video_channel_show_one(vty, v9_video_board[i].video_stream[j],  detail);
					}
				}
			}
		}
	}
	return OK;
}

/********************************************************************/
static int v9_video_board_show_one(struct vty *vty, v9_video_board_t *board, BOOL detail)
{
	vty_out (vty, "-------------------------------------------%s", VTY_NEWLINE);
	if(board->disabled)
	{
		vty_out (vty, "  Board ID              : %d disabled%s", board->id, VTY_NEWLINE);
		vty_out (vty, "   Address              : %s:%d%s", inet_address(board->address), board->port, VTY_NEWLINE);
		//if(detail)
		{
			//vty_out (vty, "   Channel              : %d%s", board->active ? "TRUE":"FALSE", VTY_NEWLINE);
			vty_out (vty, "   Active               : %s%s", board->active ? "TRUE":"FALSE", VTY_NEWLINE);
		}
		vty_out (vty, "-------------------------------------------%s", VTY_NEWLINE);
		return OK;
	}
	vty_out (vty, "  Board ID              : %d enable%s", board->id, VTY_NEWLINE);
	vty_out (vty, "   Address              : %s:%d%s", inet_address(board->address), board->port, VTY_NEWLINE);
	vty_out (vty, "   Load                 : %d%s", board->video_load, VTY_NEWLINE);
	if(detail)
	{
		if(board->channel_cnt)
		{
			u_int32 i = 0;
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
		//vty_out (vty, "   Channel              : %d%s", board->active ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out (vty, "   Active               : %s%s", board->active ? "TRUE":"FALSE", VTY_NEWLINE);
	}
	vty_out (vty, "-------------------------------------------%s", VTY_NEWLINE);
	return OK;
}

int v9_video_board_show(struct vty *vty, BOOL detail)
{
	u_int32 i = 0;
	for(i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if(v9_video_board && v9_video_board[i].use && !v9_video_board[i].disabled/*&& v9_video_board[i].active*/)
		{
			v9_video_board_show_one(vty, &v9_video_board[i],  detail);
		}
	}
	return OK;
}
/****************************** SDK **********************************/
/*
"video stream channel <1-4> ip A.B.C.D port <1-65530> username USE password PASS fps <480-1080>",
"video stream channel <1-4> ip A.B.C.D port <1-65530> username USE password PASS",
"video stream ip A.B.C.D port <1-65530> username USE password PASS",

*/

static int v9_video_channel_write_config_one(struct vty *vty, v9_video_channel_t *pstNode)
{
	if (pstNode)
	{
		int len = 0;
		char confstr[1024];
		memset(confstr, 0, sizeof(confstr));

		len = snprintf(confstr, sizeof(confstr), "%s", "video stream ");

		if(pstNode->type == V9_VIDEO_STREAM_TYPE_STATIC)
		{
			snprintf(confstr + len, sizeof(confstr) - len, "board %d channel %d ",pstNode->id, pstNode->ch);
		}
		else if(pstNode->type == V9_VIDEO_STREAM_TYPE_DYNAMIC_CH)
		{
			snprintf(confstr + len, sizeof(confstr) - len, "board %d channel 0 ",pstNode->id);
		}
		else if(pstNode->type == V9_VIDEO_STREAM_TYPE_DYNAMIC_ID)
		{
			snprintf(confstr + len, sizeof(confstr) - len, "board 0 channel %d ",pstNode->ch);
		}
		else if(pstNode->type == V9_VIDEO_STREAM_TYPE_DYNAMIC)
		{
			snprintf(confstr + len, sizeof(confstr) - len, "board 0 channel 0 ");
		}

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
			snprintf(confstr + len, sizeof(confstr) - len, "username %s password %s ",
					 pstNode->username, pstNode->password);
		}
/*
		len = strlen(confstr);
		snprintf(confstr + len, sizeof(confstr) - len, "fps %d", pstNode->fps);
*/
		vty_out (vty, " %s%s",confstr, VTY_NEWLINE);
	}
	return OK;
}

void * v9_video_app_tmp()
{
	return v9_video_board;
}

int v9_video_channel_write_config(struct vty *vty)
{
	u_int32 i = 0, j = 0;
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
							v9_video_channel_write_config_one(vty, v9_video_board[i].video_stream[j]);
						}
					}
				}
				else
				{
					vty_out (vty, " video board %d disabled%s", v9_video_board[i].id, VTY_NEWLINE);
				}
			}
		}
	}
	return OK;
}
