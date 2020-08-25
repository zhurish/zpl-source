/*
 * v9_serial.c
 *
 *  Created on: 2019年11月13日
 *      Author: DELL
 */

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
#include "table.h"
#include "vector.h"
#include "eloop.h"
#include "tty_com.h"
#ifdef PL_UBUS_MODULE
#include "ubus_sync.h"
#endif
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


v9_serial_t *v9_serial = NULL;





int v9_app_hex_debug(v9_serial_t *mgt, char *hdr, int rx)
{
	char buf[1200];
	char tmp[16];
	u_int8 *p = NULL;
	int i = 0;
	int len = 0;
	zassert(mgt != NULL);
	zassert(hdr != NULL);
	if(rx)
	{
		len = (int)mgt->len;
		p = (u_int8 *)mgt->buf;
	}
	else
	{
		len = (int)mgt->slen;
		p = (u_int8 *)mgt->sbuf;
	}
	memset(buf, 0, sizeof(buf));
	for(i = 0; i < MIN(len, 128); i++)
	{
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "0x%02x ", (u_int8)p[i]);
		if(i%6 == 0)
			strcat(buf, " ");
		if(i%12 == 0)
			strcat(buf, "\r\n");
		strcat(buf, tmp);
	}
	zlog_debug(ZLOG_APP, "%s : %s%s", hdr, buf, (len>128) ? "...":" ");
	return OK;
}


static int v9_app_read_handle(v9_serial_t *mgt)
{
	zassert(mgt != NULL);
	//v9_cmd_respone_t res;
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)mgt->buf;
	if(hdr->id != APP_BOARD_MAIN)
	{
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	switch (v9_cmd_get (mgt))
	{

		case V9_APP_CMD_REBOOT:
		case V9_APP_CMD_RESET:
		case V9_APP_CMD_SHUTDOWN:
			//zlog_debug(ZLOG_APP, "MSG V9_CMD_REBOOT -> ACK seqnum = %d", hdr->seqnum);
			v9_cmd_handle_reboot (mgt);
			break;

		case V9_APP_CMD_KEEPALIVE:
			//zlog_debug(ZLOG_APP, "MSG V9_CMD_KEEPALIVE -> ACK seqnum = %d", hdr->seqnum);
			v9_cmd_handle_keepalive (mgt);
			break;

		case V9_APP_CMD_SET_ROUTE:
			//zlog_debug(ZLOG_APP, "MSG V9_APP_CMD_SET_ROUTE -> ACK seqnum = %d", hdr->seqnum);
			v9_cmd_handle_route (mgt);
			break;

/*
		case V9_APP_CMD_GET_STATUS:
			zlog_debug(ZLOG_APP, "MSG V9_CMD_GET_STATUS -> RES seqnum = %d", hdr->seqnum);
			v9_cmd_handle_status(mgt);
			break;
*/

		case V9_APP_CMD_SEND_LOAD:
			//zlog_debug(ZLOG_APP, "MSG V9_APP_CMD_SEND_LOAD -> RES seqnum = %d", hdr->seqnum);
			v9_cmd_handle_board(mgt);
			break;

		case V9_APP_CMD_AUTOIP:
			//zlog_debug(ZLOG_APP, "MSG V9_APP_CMD_AUTOIP -> RES seqnum = %d", hdr->seqnum);
			v9_cmd_handle_autoip(mgt);
			break;

		case V9_APP_CMD_STARTUP:
			//zlog_debug(ZLOG_APP, "MSG V9_APP_CMD_STARTUP -> RES seqnum = %d", hdr->seqnum);
			v9_cmd_handle_startup(mgt);
			break;

		case V9_APP_CMD_PASS_RESET:
			//zlog_debug(ZLOG_APP, "MSG V9_APP_CMD_STARTUP -> RES seqnum = %d", hdr->seqnum);
			v9_cmd_handle_pass_reset(mgt);
			break;
#ifndef V9_SLIPNET_ENABLE
		case V9_APP_CMD_SYNC_TIME:
			//zlog_debug(ZLOG_APP, "MSG V9_APP_CMD_STARTUP -> RES seqnum = %d", hdr->seqnum);
			v9_cmd_handle_sntp_sync(mgt);
			break;
#endif

		case V9_APP_CMD_DEVICE:
			//zlog_debug(ZLOG_APP, "MSG V9_APP_CMD_SEND_LOAD -> RES seqnum = %d", hdr->seqnum);
			v9_cmd_handle_device(mgt);
			break;
		case V9_APP_CMD_DOWNLOAD_OTA:
			//zlog_debug(ZLOG_APP, "MSG V9_APP_CMD_SEND_LOAD -> RES seqnum = %d", hdr->seqnum);
			v9_cmd_update_bios_ack(mgt);
			break;

		default:
			if(V9_APP_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "TAG HDR = 0x%x(len=%d) (id=%d(id=%d) seqnum=%d)", v9_cmd_get (mgt),
						mgt->len, hdr->id, mgt->id, mgt->seqnum);

			v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
			break;
	}
	return OK;
}


static int v9_app_read_eloop(struct eloop *eloop)
{
	int len = 0;
	v9_serial_t *mgt = ELOOP_ARG(eloop);
	zassert(mgt != NULL);

	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);

	mgt->r_thread = NULL;
	memset(mgt->buf, 0, sizeof(mgt->buf));

	len = tty_com_read(mgt->tty, mgt->buf, sizeof(mgt->buf));
	if (len <= 0)
	{
		if (len < 0)
		{
			if (ERRNO_IO_RETRY(errno))
			{
				zlog_err(ZLOG_APP, "RECV mgt on socket (%s)", strerror(errno));
				//mgt->reset_thread = eloop_add_timer_msec(mgt->master, x5b_app_reset_eloop, mgt, 100);
				if(mgt->mutex)
					os_mutex_unlock(mgt->mutex);
				return OK;
			}
		}
	}
	else
	{
		mgt->len = len;
		if(len <= V9_APP_HDR_LEN  || len > V9_APP_HDR_LEN_MAX)
		{
			zlog_err(ZLOG_APP, "MSG from %d byte", mgt->len);
			v9_app_hex_debug(mgt, "RECV", TRUE);

			mgt->r_thread = eloop_add_read(mgt->master, v9_app_read_eloop, mgt, mgt->tty->fd);

			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			return OK;
		}

		if(V9_APP_DEBUG(RECV))
		{
			zlog_debug(ZLOG_APP, "MSG from %d byte", mgt->len);
			v9_app_hex_debug(mgt, "RECV", TRUE);
		}
		v9_app_read_handle(mgt);
	}
	mgt->r_thread = eloop_add_read(mgt->master, v9_app_read_eloop, mgt, mgt->tty->fd);

	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return OK;
}




#ifdef PL_UBUS_MODULE
static int v9_job_cb_action(void *p)
{
	int ret = 0;
	os_msleep(500);
	v9_serial_t *serial = p;
	if(!serial)
		return 0;

	if(strstr(serial->tmpbuf, "ntp"))
	{
		if(strstr(serial->tmpbuf + 4, "sync"))
		{
#ifdef V9_SLIPNET_ENABLE
			u_int32 timesp = os_time(NULL);
			ret = v9_cmd_sync_time_to_rtc(serial, timesp);
#else
			if(serial->timer_sync == 0)
				serial->timer_sync = 1;
#endif /* V9_SLIPNET_ENABLE */
		}
	}
	else if(strstr(serial->tmpbuf, "led"))
	{
#ifdef V9_SLIPNET_ENABLE
		//static u_int8 led = 0;
		if(strstr(serial->tmpbuf, "up"))
		{
			//if(led == 0)
			{
				//led = 1;
				ret = v9_cmd_sync_led(serial, 1, 1);
			}
		}
		else if(strstr(serial->tmpbuf, "down"))
		{
			//if(led == 1)
			{
				//led = 0;
				ret = v9_cmd_sync_led(serial, 1, 0);
			}
		}
#endif
	}
	memset(serial->tmpbuf, 0, sizeof(serial->tmpbuf));
	return ret;
}

static int v9_ntp_time_update_cb(void *p, char *buf, int len)
{
	if(!v9_serial)
		return 0;
	memset(v9_serial->tmpbuf, 0, sizeof(v9_serial->tmpbuf));
	strncpy(v9_serial->tmpbuf, buf, MIN(sizeof(v9_serial->tmpbuf), len));
	os_job_add(v9_job_cb_action, v9_serial);
	return 0;
}
#endif /* PL_UBUS_MODULE */


static int v9_serial_default(v9_serial_t *serial)
{
	memset(serial->tty, 0, sizeof(struct tty_com));

	strcpy(serial->tty->devname, V9_SERIAL_CTL_NAME);
	serial->tty->speed = 115200;		// speed bit
	serial->tty->databit = DATA_8BIT;	// data bit
	serial->tty->stopbit = STOP_1BIT;	// stop bit
	serial->tty->parity = PARITY_NONE;		// parity
	serial->tty->flow_control = FLOW_CTL_NONE;// flow control
	serial->tty->mode = TTY_COM_MODE_SLIP;

	serial->tty->encapsulation = NULL;
	serial->tty->decapsulation = NULL;
	serial->status = 0;
	serial->debug = V9_APP_DEBUG_EVENT;
	serial->id = APP_BOARD_MAIN;
	return OK;
}

static int _v9_serial_hw_init(void)
{
	if(v9_serial == NULL)
	{
		v9_serial = XMALLOC(MTYPE_ARP, sizeof(v9_serial_t));
		if(v9_serial != NULL)
		{
			memset(v9_serial, 0, sizeof(v9_serial_t));
			v9_serial->tty = XMALLOC(MTYPE_VTY, sizeof(struct tty_com));
			if(v9_serial->tty)
			{
				v9_serial_default(v9_serial);
				return OK;
			}
		}
	}
	return ERROR;
}

static int _v9_serial_hw_exit(void)
{
	if(v9_serial && v9_serial->tty)
	{
		if(tty_isopen(v9_serial->tty))
			tty_com_close(v9_serial->tty);

		XFREE(MTYPE_VTY, v9_serial->tty);
		XFREE(MTYPE_ARP, v9_serial);
		v9_serial = NULL;
	}
	return OK;
}



int v9_serial_init(char *devname, u_int32 speed)
{
	if(_v9_serial_hw_init() == OK)
	{
		if(master_eloop[MODULE_APP_START] == NULL)
			master_eloop[MODULE_APP_START] = eloop_master_module_create(MODULE_APP_START);

		v9_serial->master = master_eloop[MODULE_APP_START];
		v9_serial->mutex = os_mutex_init();
		memset(v9_serial->tty->devname, 0, sizeof(v9_serial->tty->devname));
		strcpy(v9_serial->tty->devname, devname);
		v9_serial->tty->speed = speed;		// speed bit

		if(!tty_isopen(v9_serial->tty))
		{
			if(tty_com_open(v9_serial->tty) == OK)
			{
				if(v9_serial->r_thread)
				{
					eloop_cancel(v9_serial->r_thread);
					v9_serial->r_thread = NULL;
				}
				v9_serial->r_thread = eloop_add_read(v9_serial->master, v9_app_read_eloop, v9_serial, v9_serial->tty->fd);
#ifdef V9_SLIPNET_ENABLE
#ifdef V9_SLIPNET_UDP
				v9_app_slipnet_init(v9_serial, V9_SLIPNET_UDPSRV_HOST, V9_SLIPNET_UDPSRV_PORT);
#else
				v9_app_slipnet_init(v9_serial, V9_SLIPNET_CTL_NAME, V9_SLIPNET_SPEED_RATE);
#endif
#endif /* V9_SLIPNET_ENABLE */

#ifdef PL_UBUS_MODULE
				ubus_sync_hook_install(v9_ntp_time_update_cb, v9_serial);
#endif /* PL_UBUS_MODULE */

				return OK;
			}
		}
		else
		{
			if(v9_serial->r_thread)
			{
				eloop_cancel(v9_serial->r_thread);
				v9_serial->r_thread = NULL;
			}
			v9_serial->r_thread = eloop_add_read(v9_serial->master, v9_app_read_eloop, v9_serial, v9_serial->tty->fd);
			return OK;
		}
	}
	return ERROR;
}

int v9_serial_exit()
{
	if(v9_serial)
	{
#ifdef V9_SLIPNET_ENABLE
		v9_app_slipnet_exit(v9_serial);
#endif /* V9_SLIPNET_ENABLE */
		if(v9_serial->mutex)
		{
			os_mutex_exit(v9_serial->mutex);
			v9_serial->mutex = NULL;
		}
		_v9_serial_hw_exit();
	}
	return OK;
}

static int v9_app_mgt_task(void *argv)
{
	zassert(argv != NULL);
	v9_serial_t *mgt = (v9_serial_t *)argv;
	zassert(mgt != NULL);
	module_setup_task(MODULE_APP_START, os_task_id_self());
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	if(!mgt->enable)
	{
		os_sleep(5);
	}

	//v9_video_sdk_restart_all();

	eloop_start_running(master_eloop[MODULE_APP_START], MODULE_APP_START);
	return OK;
}


static int v9_app_task_init (v9_serial_t *mgt)
{
	zassert(mgt != NULL);
	if(master_eloop[MODULE_APP_START] == NULL)
		mgt->master = master_eloop[MODULE_APP_START] = eloop_master_module_create(MODULE_APP_START);

	mgt->enable = TRUE;
	mgt->task_id = os_task_create("appTask", OS_TASK_DEFAULT_PRIORITY,
	               0, v9_app_mgt_task, mgt, OS_TASK_DEFAULT_STACK * 2);
	if(mgt->task_id)
		return OK;
	return ERROR;
}


int v9_serial_task_init()
{
	if(v9_serial != NULL)
	{
		v9_app_task_init(v9_serial);
	}
	return OK;
}


int v9_serial_task_exit()
{
/*	if(v9_serial)
		v9_app_module_exit();*/
	return OK;
}


