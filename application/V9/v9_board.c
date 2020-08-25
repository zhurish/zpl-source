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
	zassert(board);
	memset(&bios_device, 0, sizeof(bios_device));
	bios_device.cmd = 0;
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
	char tmp[128];
	u_int glen = 0, tlen = 0, mlen = 0;
	vty_out(vty, "BIOS Information : %s",VTY_NEWLINE);
	vty_out(vty, " Serial NO                  : %s%s", bios_device.serialno, VTY_NEWLINE);
	vty_out(vty, " Device ID                  : %s%s", bios_device.deviceid, VTY_NEWLINE);
	vty_out(vty, " Manufacturer               : %s%s", bios_device.manufacturer, VTY_NEWLINE);
	vty_out(vty, " Version                    : %s-%s%s",
			 bios_device.kervel_version,
			 bios_device.app_version,
			 VTY_NEWLINE);
	vty_out(vty, " Buildtime                  : %s%s", bios_device.buildtime, VTY_NEWLINE);
	vty_out(vty, "%s", VTY_NEWLINE);

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

				mlen = (v9_video_board[i].board.memtotal) & 0X000003FF;
				glen = (v9_video_board[i].board.memtotal >> 10) & 0X000003FF;
				tlen = (v9_video_board[i].board.memtotal >> 20) & 0X000003FF;
				memset(tmp, 0, sizeof(tmp));
				if (tlen > 0)
				{
					snprintf(tmp, sizeof(tmp), "%d.%02d T", tlen, glen);
				}
				else if (glen > 0)
				{
					snprintf(tmp, sizeof(tmp), "%d.%02d G", glen, mlen);
				}
				else
					snprintf(tmp, sizeof(tmp), "%d MB", v9_video_board[i].board.memtotal);

				vty_out(vty, "  Memory Total              : %s%s", tmp, VTY_NEWLINE);

				vty_out(vty, "  Memory Uses               : %d%%%s", v9_video_board[i].board.memload, VTY_NEWLINE);

				if ((APP_BOARD_MAIN == id) || (v9_video_board[i].id == APP_BOARD_MAIN) )
				{
					mlen = (v9_video_board[i].board.disktatol1) & 0X000003FF;
					glen = (v9_video_board[i].board.disktatol1 >> 10) & 0X000003FF;
					tlen = (v9_video_board[i].board.disktatol1 >> 20) & 0X000003FF;
					memset(tmp, 0, sizeof(tmp));
					if (tlen > 0)
					{
						snprintf(tmp, sizeof(tmp), "%d.%02d T", tlen, glen);
					}
					else if (glen > 0)
					{
						snprintf(tmp, sizeof(tmp), "%d.%02d G", glen, mlen);
					}
					else
						snprintf(tmp, sizeof(tmp), "%d MB", v9_video_board[i].board.disktatol1);

					vty_out(vty, "  Disk1 Total               : %s%s", tmp, VTY_NEWLINE);
					vty_out(vty, "  Disk1 Uses                : %d%%%s", v9_video_board[i].board.diskload1, VTY_NEWLINE);

					mlen = (v9_video_board[i].board.disktatol2) & 0X000003FF;
					glen = (v9_video_board[i].board.disktatol2 >> 10) & 0X000003FF;
					tlen = (v9_video_board[i].board.disktatol2 >> 20) & 0X000003FF;
					memset(tmp, 0, sizeof(tmp));
					if (tlen > 0)
					{
						snprintf(tmp, sizeof(tmp), "%d.%02d T", tlen, glen);
					}
					else if (glen > 0)
					{
						snprintf(tmp, sizeof(tmp), "%d.%02d G", glen, mlen);
					}
					else
						snprintf(tmp, sizeof(tmp), "%d MB", v9_video_board[i].board.disktatol2);
					vty_out(vty, "  Disk2 Total               : %s%s", tmp, VTY_NEWLINE);
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
	v9_video_board_lock();
	_v9_app_board_show(vty,  id,  debug);
	v9_video_board_unlock();
	return 0;
}
