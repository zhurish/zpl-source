/*
 * v9_video.c
 *
 *  Created on: 2019年11月28日
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
#include "zmemory.h"
#include "prefix.h"
#include "str.h"
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



int v9_sqldb_debug_api(zpl_bool enable, zpl_uint32 flag)
{
	if(enable)
		_sqldb_debug |= flag;
	else
		_sqldb_debug &= ~(flag);
	return OK;
}

int v9_user_debug_api(zpl_bool enable, zpl_uint32 flag)
{
	if(enable)
		__user_debug_flag |= flag;
	else
		__user_debug_flag &= ~(flag);
	return OK;
}

int v9_video_sdk_debug_api(zpl_bool enable, zpl_uint32 flag)
{
	if(enable)
		__sdk_debug_flag |= flag;
	else
		__sdk_debug_flag &= ~(flag);
	return OK;
}

int v9_serial_debug_api(zpl_bool enable, zpl_uint32 flag)
{
	if(v9_serial)
	{
		if(enable)
			v9_serial->debug |= flag;
		else
			v9_serial->debug &= ~(flag);
	}
	return OK;
}


int v9_video_debug_config(struct vty *vty, zpl_bool detail)
{
	if(V9_SQLDB_DEBUG(MSG))
		vty_out (vty, " debug video sqldb msg%s", VTY_NEWLINE);
	if(V9_SQLDB_DEBUG(DB))
		vty_out (vty, " debug video sqldb db%s", VTY_NEWLINE);
	if(V9_SQLDB_DEBUG(STATE))
		vty_out (vty, " debug video sqldb state%s", VTY_NEWLINE);
	if(V9_SQLDB_DEBUG(DBCMD))
		vty_out (vty, " debug video sqldb sqlcmd%s", VTY_NEWLINE);

	if(V9_USER_DEBUG(EVENT))
		vty_out (vty, " debug video user event%s", VTY_NEWLINE);
	if(V9_USER_DEBUG(ERROR))
		vty_out (vty, " debug video user error%s", VTY_NEWLINE);
	if(V9_USER_DEBUG(WARN))
		vty_out (vty, " debug video user warn%s", VTY_NEWLINE);

	if(V9_SDK_DEBUG(EVENT))
		vty_out (vty, " debug video sdk event%s", VTY_NEWLINE);
	if(V9_SDK_DEBUG(ERROR))
		vty_out (vty, " debug video sdk error%s", VTY_NEWLINE);
	if(V9_SDK_DEBUG(WARN))
		vty_out (vty, " debug video sdk warn%s", VTY_NEWLINE);
	if(V9_SDK_DEBUG(WEB))
		vty_out (vty, " debug video sdk web%s", VTY_NEWLINE);
	if(V9_SDK_DEBUG(MSG))
		vty_out (vty, " debug video sdk msg%s", VTY_NEWLINE);
	if(V9_SDK_DEBUG(STATE))
		vty_out (vty, " debug video sdk state%s", VTY_NEWLINE);

	if(V9_APP_DEBUG(EVENT))
		vty_out (vty, " debug video serial event%s", VTY_NEWLINE);
	if(V9_APP_DEBUG(HEX))
		vty_out (vty, " debug video serial hex%s", VTY_NEWLINE);
	if(V9_APP_DEBUG(RECV))
		vty_out (vty, " debug video serial resv%s", VTY_NEWLINE);
	if(V9_APP_DEBUG(SEND))
		vty_out (vty, " debug video serial send%s", VTY_NEWLINE);
	if(V9_APP_DEBUG(UPDATE))
		vty_out (vty, " debug video serial update%s", VTY_NEWLINE);
	if(V9_APP_DEBUG(TIME))
		vty_out (vty, " debug video serial timer%s", VTY_NEWLINE);
	if(V9_APP_DEBUG(WEB))
		vty_out (vty, " debug video serial web%s", VTY_NEWLINE);
	if(V9_APP_DEBUG(MSG))
		vty_out (vty, " debug video serial msg%s", VTY_NEWLINE);
	if(V9_APP_DEBUG(STATE))
		vty_out (vty, " debug video serial state%s", VTY_NEWLINE);
	if(V9_APP_DEBUG(UCI))
		vty_out (vty, " debug video serial uci%s", VTY_NEWLINE);
	return OK;
}

int v9_app_snapfea_key_alloc(sql_snapfea_key *key, zpl_bool nomem)
{
	if(nomem)
		return OK;
	key->feature.feature_data = key->feature.ckey_data = XMALLOC(MTYPE_VIDEO_KEY, APP_FEATURE_MAX * sizeof(zpl_float));									// 特征值
	if(key->feature.ckey_data)
	{
		return OK;
	}
	return ERROR;
}
int v9_app_snapfea_key_free(sql_snapfea_key *key)
{								// 特征值
	if(key->feature.ckey_data)
	{
		key->feature_len = 0;
		XFREE(MTYPE_VIDEO_KEY, key->feature.feature_data);
		key->feature.feature_data = key->feature.ckey_data = NULL;
		return OK;
	}
	return ERROR;
}


char * v9_app_age_string(zpl_uint32 age)
{
	if(age == 0)
		return "儿童";
	else if(age == 1)
		return "少年";
	else if(age == 2)
		return "青年";
	else if(age == 3)
		return "中年";
	else if(age == 4)
		return "老年";
/*	if(age == 0)
		return "0";
	else if(age == 1)
		return "1";
	else if(age == 2)
		return "2";
	else if(age == 3)
		return "3";
	else if(age == 4)
		return "4";*/
	return "Unknow";
}


int v9_app_module_init()
{
	if(master_eloop[MODULE_APP_START] == NULL)
		master_eloop[MODULE_APP_START] = eloop_master_module_create(MODULE_APP_START);

	v9_video_disk_dir_init();
	//v9_board_init();
	v9_video_sqldb_load(0);

	v9_video_board_init();

	//v9_video_sdk_init();

	v9_video_user_load();

	v9_serial_init(V9_SERIAL_CTL_NAME, 115200);

	return OK;
}

int v9_app_module_exit()
{
	v9_video_db_lock();
	v9_video_board_lock();
	v9_video_board_exit();
	v9_video_user_exit();
	v9_serial_exit();
	return OK;
}


int v9_app_module_task_init(void)
{
	v9_video_sdk_task_init ();
	return v9_serial_task_init();
}
int v9_app_module_task_exit(void)
{
	return v9_serial_task_exit();
}
