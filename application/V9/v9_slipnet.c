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




static int v9_app_slipnet_read_eloop(struct eloop *eloop)
{
	int len = 0;
	v9_serial_t *mgt = ELOOP_ARG(eloop);
	zassert(mgt != NULL);

	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);

	mgt->r_slipnet = NULL;
	memset(mgt->buf, 0, sizeof(mgt->buf));

	len = tty_com_slip_read(mgt->slipnet, mgt->buf, sizeof(mgt->buf));
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

		//v9_app_read_handle(mgt);
	}
	mgt->r_slipnet = eloop_add_read(mgt->master, v9_app_slipnet_read_eloop, mgt, mgt->slipnet->fd);

	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return OK;
}





static int v9_slipnet_default(v9_serial_t *serial)
{
	memset(serial->slipnet, 0, sizeof(struct tty_com));

	strcpy(serial->slipnet->devname, V9_SERIAL_CTL_NAME);
	serial->slipnet->speed = 115200;		// speed bit
	serial->slipnet->databit = DATA_8BIT;	// data bit
	serial->slipnet->stopbit = STOP_1BIT;	// stop bit
	serial->slipnet->parity = PARITY_NONE;		// parity
	serial->slipnet->flow_control = FLOW_CTL_NONE;// flow control

	serial->slipnet->encapsulation = NULL;
	serial->slipnet->decapsulation = NULL;
	serial->status = 0;
	return OK;
}

static int _v9_slipnet_hw_init(v9_serial_t *serial)
{
	if (serial != NULL)
	{
		serial->slipnet = XMALLOC(MTYPE_VTY, sizeof(struct tty_com));
		if (serial->slipnet)
		{
			v9_slipnet_default (serial);
			return OK;
		}
	}
	return ERROR;
}

static int _v9_slipnet_hw_exit(v9_serial_t *serial)
{
	if(serial && serial->slipnet)
	{
		if(tty_isopen(serial->slipnet))
			tty_com_close(serial->slipnet);
		XFREE(MTYPE_VTY, serial->slipnet);
	}
	return OK;
}



int v9_app_slipnet_init(v9_serial_t *serial, char *devname, u_int32 speed)
{
	if(_v9_slipnet_hw_init(serial) == OK)
	{
		if(master_eloop[MODULE_APP_START] == NULL)
		{
			master_eloop[MODULE_APP_START] = eloop_master_module_create(MODULE_APP_START);
			serial->master = master_eloop[MODULE_APP_START];
		}
		memset(serial->slipnet->devname, 0, sizeof(v9_serial->slipnet->devname));
		strcpy(serial->slipnet->devname, devname);
		serial->slipnet->speed = speed;		// speed bit

		if(!tty_isopen(serial->slipnet))
		{
			if(tty_com_open(serial->slipnet) == OK)
			{
				if(serial->r_slipnet)
				{
					eloop_cancel(serial->r_slipnet);
					serial->r_slipnet = NULL;
				}

				serial->r_slipnet = eloop_add_read(serial->master, v9_app_slipnet_read_eloop, serial, serial->slipnet->fd);
				return OK;
			}
		}
		else
		{
			if(serial->r_slipnet)
			{
				eloop_cancel(serial->r_slipnet);
				serial->r_slipnet = NULL;
			}
			serial->r_slipnet = eloop_add_read(serial->master, v9_app_slipnet_read_eloop, serial, serial->slipnet->fd);
			return OK;
		}
	}
	return ERROR;
}

int v9_app_slipnet_exit(v9_serial_t *serial)
{
	if(serial)
	{
		if(serial->r_slipnet)
		{
			eloop_cancel(serial->r_slipnet);
			serial->r_slipnet = NULL;
		}
		_v9_slipnet_hw_exit(serial);
	}
	return OK;
}
