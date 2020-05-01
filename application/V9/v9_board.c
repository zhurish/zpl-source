#include "zebra.h"
#include "network.h"
#include "vty.h"
#include "if.h"
#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "str.h"
#include "vty.h"
#include "table.h"
#include "vector.h"
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




int v9_board_init(u_int8 id, v9_board_t *board)
{
	switch(id)
	{
	case APP_BOARD_MAIN:
		board->use = TRUE;
		board->id = APP_BOARD_MAIN;
		board->online = TRUE;
		board->power = TRUE;
		board->active = TRUE;
		board->ip = APP_BOARD_ADDRESS_PREFIX + APP_BOARD_ADDRESS_MAIN;

		board->temp = 65;	//温度
		board->vch = 0;			//处理视频路数
		break;
	case APP_BOARD_CALCU_1:
		board->use = TRUE;
		board->id = APP_BOARD_CALCU_1;
		board->ip = APP_BOARD_ADDRESS_PREFIX + APP_BOARD_CALCU_1;

		board->temp = 85;	//温度
		board->vch = 0;			//处理视频路数
		break;
	case APP_BOARD_CALCU_2:
		board->use = TRUE;
		board->id = APP_BOARD_CALCU_2;
		board->ip = APP_BOARD_ADDRESS_PREFIX + APP_BOARD_CALCU_2;

		board->temp = 85;	//温度
		board->vch = 0;			//处理视频路数
		break;
	case APP_BOARD_CALCU_3:
		board->use = TRUE;
		board->id = APP_BOARD_CALCU_3;
		board->ip = APP_BOARD_ADDRESS_PREFIX + APP_BOARD_CALCU_3;

		board->temp = 85;	//温度
		board->vch = 0;			//处理视频路数
		break;
	case APP_BOARD_CALCU_4:
		board->use = TRUE;
		board->id = APP_BOARD_CALCU_4;
		board->ip = APP_BOARD_ADDRESS_PREFIX + APP_BOARD_CALCU_4;

		board->temp = 85;	//温度
		board->vch = 0;			//处理视频路数
		break;
	default:
		break;
	}
	return OK;
}

v9_board_t * v9_board_lookup(u_int8 id)
{
	u_int32 i =0;
	if(v9_video_board)
	{
		for(i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if(v9_video_board[i].id == id)
			{
				return &v9_video_board[i].board;
			}
		}
	}
	return NULL;
}

int v9_board_update_board(u_int8 id, v9_board_t *board)
{
	int i =0;
	if(v9_video_board)
	{
		for(i = 0; i < V9_APP_BOARD_MAX; i++)
		{
			if(v9_video_board[i].id == id)
			{
				if(board)
					memcpy(&v9_video_board[i].board, board, sizeof(v9_board_t));
				return OK;
			}
		}
	}
	return ERROR;
}


static void _v9_app_board_show(struct vty * vty, int id, int debug)
{
	int i = 0;
	for (i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if (v9_video_board[i].board.use == TRUE)
		{
			if ((id == 0) || (id > 0 && id == v9_video_board[i].board.id))
			{
				vty_out(vty, "----------------------------------------------------%s", VTY_NEWLINE);
				if(v9_video_board[i].id == APP_BOARD_MAIN)
					vty_out(vty, " Main Board ID              : %d%s", V9_APP_BOARD_HW_ID(v9_video_board[i].id), VTY_NEWLINE);
				else
					vty_out(vty, " Board ID                   : %d%s", V9_APP_BOARD_HW_ID(v9_video_board[i].id), VTY_NEWLINE);
				vty_out(vty, "  IP Address                : %s%s", inet_address(v9_video_board[i].address), VTY_NEWLINE);
				vty_out(vty, "  Online Status             : %s%s", v9_video_board[i].board.online ? "TRUE":"FALSE", VTY_NEWLINE);
				vty_out(vty, "  Power Status              : %s%s", v9_video_board[i].board.online ? "TRUE":"FALSE", VTY_NEWLINE);
				vty_out(vty, "  Active Status             : %s%s", v9_video_board[i].board.active ? "TRUE":"FALSE", VTY_NEWLINE);
				if (debug)
				{
					vty_out(vty, "  AutoIP Status             : %s%s", v9_video_board[i].board.autoip ? "TRUE":"FALSE", VTY_NEWLINE);
					vty_out(vty, "  Startup Status            : %s%s", v9_video_board[i].board.startup ? "TRUE":"FALSE", VTY_NEWLINE);
				}
				vty_out(vty, "  Board Temp                : %d C%s", v9_video_board[i].board.temp, VTY_NEWLINE);
				vty_out(vty, "  Vidio Channel             : %d%s", v9_video_board[i].board.vch, VTY_NEWLINE);

				vty_out(vty, "  CPU Load                  : %d.%d%%%s", (v9_video_board[i].board.cpuload>>8)&0xff,
						(v9_video_board[i].board.cpuload)&0xff, VTY_NEWLINE);

				vty_out(vty, "  Memory Total              : %d MB%s", v9_video_board[i].board.memtotal, VTY_NEWLINE);
				vty_out(vty, "  Memory Uses               : %d%%%s", v9_video_board[i].board.memload, VTY_NEWLINE);
				if ((APP_BOARD_MAIN == id) || (v9_video_board[i].id == APP_BOARD_MAIN) )
				{
					vty_out(vty, "  Disk1 Total               : %d MB%s", v9_video_board[i].board.disktatol1, VTY_NEWLINE);
					vty_out(vty, "  Disk1 Uses                : %d%%%s", v9_video_board[i].board.diskload1, VTY_NEWLINE);
					vty_out(vty, "  Disk2 Total               : %d MB%s", v9_video_board[i].board.disktatol2, VTY_NEWLINE);
					vty_out(vty, "  Disk2 Uses                : %d%%%s", v9_video_board[i].board.diskload2, VTY_NEWLINE);
				}
				if (debug)
				{
					vty_out(vty, "  Time Sync                 : %s%s", v9_video_board[i].board.synctime ? "TRUE":"FALSE", VTY_NEWLINE);
					vty_out(vty, "  Keep Alive                : %d%s", v9_video_board[i].board.cnt, VTY_NEWLINE);
				}
				if (id > 0)
				{
					vty_out(vty, "----------------------------------------------------%s", VTY_NEWLINE);
					vty_out(vty, "%s", VTY_NEWLINE);
					return;
				}
				vty_out(vty, "----------------------------------------------------%s", VTY_NEWLINE);
				vty_out(vty, "%s", VTY_NEWLINE);
			}
		}
	}

	vty_out(vty, "%s", VTY_NEWLINE);
	return;
}

int v9_board_show(struct vty * vty, int id, int debug)
{
	_v9_app_board_show(vty,  id,  debug);
	return 0;
}
