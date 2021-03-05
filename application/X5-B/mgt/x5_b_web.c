/*
 * x5_b_ubus.c
 *
 *  Created on: 2019年4月4日
 *      Author: DELL
 */

#include "zebra.h"
#include "memory.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "vector.h"
#include "eloop.h"
#include "network.h"
#include "vty.h"
#include "cJSON.h"
#include "os_uci.h"
#include "ubus_sync.h"

#include "x5_b_global.h"
#include "x5_b_cmd.h"
#include "x5_b_app.h"
#include "x5_b_util.h"
#include "x5_b_ctl.h"
#include "x5_b_json.h"
#include "x5_b_web.h"
#include "x5b_dbase.h"


int x5b_app_open_option_action(void *p, ospl_bool save, ospl_bool face)
{
	//char tmp[128];
	int ret = 0;
	x5b_app_open_t *openconf = p;
	ConfiglockType info;
	if(x5b_app_open->wiggins != openconf->wiggins)
	{
		ret = x5b_app_wiggins_setting(NULL, openconf->wiggins, E_CMD_TO_A);
		if(ret != OK)
		{
			zlog_debug(MODULE_APP, "Can not setting wiggins to IO Center");
			return ERROR;
		}
		os_msleep(500);
		if(x5b_app_mgt)
			x5b_app_mgt->app_a.wiggins = openconf->wiggins;
	}

	info.RelayOpenTime = htonl(openconf->waitopen);
	info.RelayOpenWaitOpenDoorTime = htonl(openconf->openhold);
	info.DoorKeepOpenTime = htonl(openconf->waitclose);
	info.DoorSensorOutPutLevle = openconf->outrelay ? 1: 0;
	info.LockRole = 2;
	info.TamperAlarm = openconf->tamperalarm ? 1: 0;
	info.DoorKeepOpentTimeOutAlarm = openconf->openalarm ? 1: 0;
	info.rev = x5b_app_global->doorcontact ? 1: 0;

	if(info.DoorKeepOpentTimeOutAlarm == 0)
		info.DoorKeepOpenTime = 0XFF;

	if(openconf->opentype != x5b_app_open->opentype && face)
	{
		ret = OK;
		if(x5b_app_mgt && x5b_app_mode_X5CM())
			ret |= x5b_app_open_option(NULL, &openconf->opentype, E_CMD_TO_C);
		if(ret != OK)
		{
			zlog_debug(MODULE_APP, "Can not setting Door Open Type to Control Center");
			return ERROR;
		}
		x5b_app_open->opentype = openconf->opentype;
	}
	ret = x5b_app_open_option(NULL, &info, E_CMD_TO_A);
	if(ret != OK)
	{
		zlog_debug(MODULE_APP, "Can not setting Door Open Configure to IO Center");
		return ERROR;
	}

	if(x5b_app_open)
	{
		if(save)
		{
			x5b_app_open->wiggins = openconf->wiggins;
			memcpy(x5b_app_open, openconf, sizeof(x5b_app_open_t));
			x5b_app_open_config_save();
		}
		return OK;
	}
	return ERROR;
}



int x5b_app_face_config_action(void *info, ospl_bool save)
{
	if(x5b_app_mgt && x5b_app_mode_X5CM())
	{
		if(x5b_app_face_config_json(NULL, info, E_CMD_TO_C) == OK)
		{
			if(save)
			{
				memcpy(x5b_app_face, info, sizeof(make_face_config_t));
				x5b_app_face_config_save();
			}
			return OK;
		}
		else
		{
			zlog_debug(MODULE_APP, "Can not setting Face Configure to Control Center");
			return ERROR;
		}
	}
	else
	{
		if(save)
		{
			memcpy(x5b_app_face, info, sizeof(make_face_config_t));
			x5b_app_face_config_save();
		}
		return OK;
	}
	return ERROR;
}

int x5b_app_global_config_action(void *info, ospl_bool save)
{
	x5b_app_global_t *glinfo = info;
	if(x5b_app_mgt && x5b_app_mode_X5CM())
	{
		if(x5b_app_device_json(x5b_app_mgt, glinfo->devicename,
				glinfo->docking_platform_address,
				glinfo->out_direction?"out":"in", glinfo->topf, E_CMD_TO_C) == OK)
		//if(x5b_app_global_device_config(x5b_app_mgt, E_CMD_TO_C) == OK)
		{
			if(save)
			{
				memcpy(x5b_app_global, glinfo, sizeof(x5b_app_global_t));
				x5b_app_global_mode_config_save();
			}
			return OK;
		}
		else
		{
			zlog_debug(MODULE_APP, "Can not setting Global Configure to Control Center");
			return ERROR;
		}
	}
	else
	{
		if(save)
		{
			memcpy(x5b_app_global, info, sizeof(x5b_app_global_t));
			x5b_app_global_mode_config_save();
		}
		return OK;
	}
	return ERROR;
}


/*int x5b_app_A_update_test(char *filename)
{
	char path[128];
	if(!filename || strlen(filename) <= 0)
	{
		zlog_debug(MODULE_APP, "x5b_app_A_update_test filename is null");
		return ERROR;
	}
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "file install-%s", filename);
	return x5b_app_A_update_handle(path);
}*/
/*

int x5b_app_A_unit_test_set_api(ospl_bool enable)
{
	ospl_uint32 len = 0;
	x5b_app_mgt_t *mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, E_CMD_TO_A);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(MODULE_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(MODULE_APP, "A Module Unit Test MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}

	x5b_app_hdr_make(mgt);
	len = os_tlv_set_zero(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_UNIT_TEST), 0);
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(MODULE_APP, "A Module Unit Test MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}
*/
