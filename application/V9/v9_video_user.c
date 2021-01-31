/*
 * v9_video_user.c
 *
 *  Created on: Mar 16, 2019
 *      Author: zhurish
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




int __user_debug_flag = V9_USER_DEBUG_ERROR|V9_USER_DEBUG_WARN|V9_USER_DEBUG_EVENT;



v9_user_group_t _group_tbl[ID_INDEX(APP_BOARD_CALCU_4)];
static void 		*_user_mutex = NULL;
/***********************************************************************************/
/***********************************************************************************/
static int v9_video_group_init()
{
	memset(_group_tbl, 0, sizeof(_group_tbl));

	if(os_file_access(V9_USER_GROUP_FILE) == OK)
	{
		os_read_file(V9_USER_GROUP_FILE, _group_tbl, sizeof(_group_tbl));
	}
	else
	{
		_group_tbl[ID_INDEX(APP_BOARD_CALCU_1)].gtbl[0].groupid = GROUP_ACTIVE_BIT|GROUP_INDEX(0);
		strcpy(_group_tbl[ID_INDEX(APP_BOARD_CALCU_1)].gtbl[0].groupname, "黑名单");
		_group_tbl[ID_INDEX(APP_BOARD_CALCU_1)].gtbl[1].groupid = GROUP_ACTIVE_BIT|GROUP_INDEX(1);
		strcpy(_group_tbl[ID_INDEX(APP_BOARD_CALCU_1)].gtbl[1].groupname, "白名单");

		_group_tbl[ID_INDEX(APP_BOARD_CALCU_2)].gtbl[0].groupid = GROUP_ACTIVE_BIT|GROUP_INDEX(0);
		strcpy(_group_tbl[ID_INDEX(APP_BOARD_CALCU_2)].gtbl[0].groupname, "黑名单");
		_group_tbl[ID_INDEX(APP_BOARD_CALCU_2)].gtbl[1].groupid = GROUP_ACTIVE_BIT|GROUP_INDEX(1);
		strcpy(_group_tbl[ID_INDEX(APP_BOARD_CALCU_2)].gtbl[1].groupname, "白名单");

		_group_tbl[ID_INDEX(APP_BOARD_CALCU_3)].gtbl[0].groupid = GROUP_ACTIVE_BIT|GROUP_INDEX(0);
		strcpy(_group_tbl[ID_INDEX(APP_BOARD_CALCU_3)].gtbl[0].groupname, "黑名单");
		_group_tbl[ID_INDEX(APP_BOARD_CALCU_3)].gtbl[1].groupid = GROUP_ACTIVE_BIT|GROUP_INDEX(1);
		strcpy(_group_tbl[ID_INDEX(APP_BOARD_CALCU_3)].gtbl[1].groupname, "白名单");

		_group_tbl[ID_INDEX(APP_BOARD_CALCU_4)].gtbl[0].groupid = GROUP_ACTIVE_BIT|GROUP_INDEX(0);
		strcpy(_group_tbl[ID_INDEX(APP_BOARD_CALCU_4)].gtbl[0].groupname, "黑名单");
		_group_tbl[ID_INDEX(APP_BOARD_CALCU_4)].gtbl[1].groupid = GROUP_ACTIVE_BIT|GROUP_INDEX(1);
		strcpy(_group_tbl[ID_INDEX(APP_BOARD_CALCU_4)].gtbl[1].groupname, "白名单");

		os_write_file(V9_USER_GROUP_FILE, _group_tbl, sizeof(_group_tbl));
	}
	return OK;
}

int v9_video_usergroup_add(u_int32 id, const char * groupname)
{
	u_int32 i = 0;
	if(id != APP_BOARD_CALCU_1 &&
			id != APP_BOARD_CALCU_2 &&
			id != APP_BOARD_CALCU_3 &&
			id != APP_BOARD_CALCU_4)
	{
		return ERROR;
	}
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	for(i = 0; i < APP_GROUP_MAX; i++)
	{
		//if(GROUP_ACTIVE(_group_tbl[ID_INDEX(id)].ID))
		{
			if(!GROUP_ACTIVE(_group_tbl[ID_INDEX(id)].gtbl[i].groupid))
			{
				if(v9_video_sdk_add_group_api(id, GROUP_INDEX(i), groupname) == OK)
				{
					_group_tbl[ID_INDEX(id)].gtbl[i].groupid = GROUP_ACTIVE_BIT|GROUP_INDEX(i);
					strcpy(_group_tbl[ID_INDEX(id)].gtbl[i].groupname, groupname);
					os_write_file(V9_USER_GROUP_FILE, _group_tbl, sizeof(_group_tbl));
					if(_user_mutex)
						os_mutex_unlock(_user_mutex);
					return GROUP_INDEX(_group_tbl[ID_INDEX(id)].gtbl[i].groupid);
				}
				else
				{
					if(_user_mutex)
						os_mutex_unlock(_user_mutex);
					return ERROR;
				}
			}
		}
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ERROR;
}

int v9_video_usergroup_del(u_int32 id, const int group)
{
	u_int32 i = 0;
	if(id != APP_BOARD_CALCU_1 &&
			id != APP_BOARD_CALCU_2 &&
			id != APP_BOARD_CALCU_3 &&
			id != APP_BOARD_CALCU_4)
	{
		return ERROR;
	}
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	for(i = 0; i < APP_GROUP_MAX; i++)
	{
		//if(GROUP_ACTIVE(_group_tbl[ID_INDEX(id)].ID))
		{
			if(GROUP_ACTIVE(_group_tbl[ID_INDEX(id)].gtbl[i].groupid))
			{
				if(GROUP_INDEX(_group_tbl[ID_INDEX(id)].gtbl[i].groupid) == GROUP_INDEX(group))
				{
					if(v9_video_sdk_del_group_api(id, GROUP_INDEX(group)) == OK)
					{
						_group_tbl[ID_INDEX(id)].gtbl[i].groupid = 0;
						memset(_group_tbl[ID_INDEX(id)].gtbl[i].groupname, 0,
							   sizeof(_group_tbl[ID_INDEX(id)].gtbl[i].groupname));
						os_write_file(V9_USER_GROUP_FILE, _group_tbl, sizeof(_group_tbl));
						if(_user_mutex)
							os_mutex_unlock(_user_mutex);
						return OK;
					}
					else
					{
						if(_user_mutex)
							os_mutex_unlock(_user_mutex);
						return ERROR;
					}
				}
			}
		}
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ERROR;
}

int v9_video_usergroup_rename(u_int32 id, const int group, const char * groupname)
{
	u_int32 i = 0;
	if(id != APP_BOARD_CALCU_1 &&
			id != APP_BOARD_CALCU_2 &&
			id != APP_BOARD_CALCU_3 &&
			id != APP_BOARD_CALCU_4)
	{
		return ERROR;
	}
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	for(i = 0; i < APP_GROUP_MAX; i++)
	{
		//if(GROUP_ACTIVE(_group_tbl[ID_INDEX(id)].ID))
		{
			if(GROUP_ACTIVE(_group_tbl[ID_INDEX(id)].gtbl[i].groupid))
			{
				if(GROUP_INDEX(_group_tbl[ID_INDEX(id)].gtbl[i].groupid) == GROUP_INDEX(group))
				{
					if(v9_video_sdk_add_group_api(id, GROUP_INDEX(group), groupname) == OK)
					{
						//_group_tbl[ID_INDEX(id)].gtbl[i].groupid = 0;
						memset(_group_tbl[ID_INDEX(id)].gtbl[i].groupname, 0,
							   sizeof(_group_tbl[ID_INDEX(id)].gtbl[i].groupname));
						strcpy(_group_tbl[ID_INDEX(id)].gtbl[i].groupname, groupname);
						os_write_file(V9_USER_GROUP_FILE, _group_tbl, sizeof(_group_tbl));
						if(_user_mutex)
							os_mutex_unlock(_user_mutex);
						return GROUP_INDEX(_group_tbl[ID_INDEX(id)].gtbl[i].groupid);
					}
					else
					{
						if(_user_mutex)
							os_mutex_unlock(_user_mutex);
						return ERROR;
					}
				}
			}
		}
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ERROR;
}


const char * v9_video_usergroup_idtoname(u_int32 id, const int group)
{
	u_int32 i = 0;
	if(id != APP_BOARD_CALCU_1 &&
			id != APP_BOARD_CALCU_2 &&
			id != APP_BOARD_CALCU_3 &&
			id != APP_BOARD_CALCU_4)
	{
		return NULL;
	}
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	for(i = 0; i < APP_GROUP_MAX; i++)
	{
		if(GROUP_ACTIVE(_group_tbl[ID_INDEX(id)].gtbl[i].groupid))
		{
			if(GROUP_INDEX(_group_tbl[ID_INDEX(id)].gtbl[i].groupid) == GROUP_INDEX(group))
			{
				if(_user_mutex)
					os_mutex_unlock(_user_mutex);
					return _group_tbl[ID_INDEX(id)].gtbl[i].groupname;
			}
		}
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return NULL;
}

u_int32 v9_video_usergroup_nametoid(u_int32 id, const char * groupname)
{
	u_int32 i = 0;
	if(id != APP_BOARD_CALCU_1 &&
			id != APP_BOARD_CALCU_2 &&
			id != APP_BOARD_CALCU_3 &&
			id != APP_BOARD_CALCU_4)
	{
		return ERROR;
	}
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	for(i = 0; i < APP_GROUP_MAX; i++)
	{
		if(GROUP_ACTIVE(_group_tbl[ID_INDEX(id)].gtbl[i].groupid))
		{
			//strcasecmp
			if(strcmp(_group_tbl[ID_INDEX(id)].gtbl[i].groupname, groupname) == 0)
			{
				if(_user_mutex)
					os_mutex_unlock(_user_mutex);
				return GROUP_INDEX(_group_tbl[ID_INDEX(id)].gtbl[i].groupid);
			}
		}
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ERROR;
}


int v9_video_usergroup_show(u_int32 id, struct vty *vty)
{
	u_int32 i = 0;
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	if(id == APP_BOARD_CALCU_1 ||
			id == APP_BOARD_CALCU_2 ||
			id == APP_BOARD_CALCU_3 ||
			id == APP_BOARD_CALCU_4)
	{
		vty_out(vty, "Video User Group List :(Board:%d)%s", ID_INDEX(id), VTY_NEWLINE);
		for(i = 0; i < APP_GROUP_MAX; i++)
		{
			//if(GROUP_ACTIVE(_group_tbl[ID_INDEX(id)].ID))
			{
				if(GROUP_ACTIVE(_group_tbl[ID_INDEX(id)].gtbl[i].groupid))
				{
					vty_out(vty, " group id:%d name %s %s",
						GROUP_INDEX(_group_tbl[ID_INDEX(id)].gtbl[i].groupid),
						_group_tbl[ID_INDEX(id)].gtbl[i].groupname, VTY_NEWLINE);
				}
			}
		}
	}
	else
	{
		u_int32 j = 0;
		vty_out(vty, "Video User Group List:%s", VTY_NEWLINE);
		for(j = APP_BOARD_CALCU_1; j <= APP_BOARD_CALCU_4; j++)
		{
			vty_out(vty, " Video User Group List :(Board:%d)%s", ID_INDEX(id), VTY_NEWLINE);
			for(i = 0; i < APP_GROUP_MAX; i++)
			{
				if(GROUP_ACTIVE(_group_tbl[ID_INDEX(id)].gtbl[i].groupid))
				{
					vty_out(vty, "  group id:%d name %s %s",
						GROUP_INDEX(_group_tbl[ID_INDEX(id)].gtbl[i].groupid),
						_group_tbl[ID_INDEX(id)].gtbl[i].groupname, VTY_NEWLINE);
				}
			}
		}
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ERROR;
}

/***********************************************************************************/
int v9_video_user_load()
{
	sqlite3 * db = v9_user_sqldb_open(2);
	v9_user_sqldb_close(db, 2);
	v9_video_group_init();
	if(!_user_mutex)
	{
		_user_mutex = os_mutex_init();
	}
	return OK;
}

int v9_video_user_clean(void)
{
	v9_user_sqldb_cleanup(1);
	sqlite3 * db = v9_user_sqldb_open(2);
	v9_user_sqldb_close(db, 2);
	return OK;
}

int v9_video_user_exit()
{
	if(_user_mutex)
	{
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
		os_mutex_exit(_user_mutex);
		_user_mutex = NULL;
	}
	return OK;
}


int v9_video_user_count(u_int32 id, int group, int *pValue)
{
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	sqlite3 * db = v9_user_sqldb_open(id);
	if(db)
	{
		if(v9_user_sqldb_count(db, id, group, pValue) == OK)
		{
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return OK;
		}
		v9_user_sqldb_close(db, id);
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ERROR;
}

int v9_video_user_add_user(u_int32 id, BOOL gender, int group, char *user, char *user_id, char *pic, char *text)
{
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	sqlite3 * db = v9_user_sqldb_open(id);
	if(db)
	{
		v9_video_user_t usertmp;
		memset(&usertmp, 0, sizeof(v9_video_user_t));
		if(v9_user_sqldb_lookup_user(db, id, user_id, &usertmp) == OK)
		{
			if(V9_USER_DEBUG(WARN))
				zlog_warn(MODULE_APP, "this user(%s(%s)) is already exist.", user, user_id);
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return ERROR;
		}

		if(v9_video_sdk_add_user_api(id, gender, group,
									 user, user_id, pic, text, TRUE) != OK)
		{
			if(V9_USER_DEBUG(WARN))
				zlog_warn(MODULE_APP, "this user(%s(%s)) is can't set hw board.", user, user_id);
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return ERROR;
		}

		if(v9_user_sqldb_add(db, id, gender, group, user, user_id, pic, text) == OK)
		{
			sql_snapfea_key key;
			memset(&key, 0, sizeof(sql_snapfea_key));
			if(v9_app_snapfea_key_alloc(&key, FALSE) != OK)
			{
				v9_video_sdk_del_user_api(id, group, user_id);
				v9_user_sqldb_del(db, id, user_id);
				v9_user_sqldb_close(db, id);
				if(_user_mutex)
					os_mutex_unlock(_user_mutex);
				return ERROR;
			}
			//获取图片特征值
			if(v9_video_sdk_get_keyvalue_api( id, pic, &key) != OK)
			{
				v9_video_sdk_del_user_api(id, group, user_id);
				v9_app_snapfea_key_free(&key);
				v9_user_sqldb_del(db, id, user_id);
				v9_user_sqldb_close(db, id);
				if(_user_mutex)
					os_mutex_unlock(_user_mutex);
				return ERROR;
			}
			if(v9_user_sqldb_key_update(db, id, user_id, &key) != OK)
			{
				v9_video_sdk_del_user_api(id, group, user_id);
				v9_app_snapfea_key_free(&key);
				v9_user_sqldb_del(db, id, user_id);
				v9_user_sqldb_close(db, id);
				if(_user_mutex)
					os_mutex_unlock(_user_mutex);
				return ERROR;
			}
			v9_app_snapfea_key_free(&key);
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return OK;
		}
		v9_user_sqldb_close(db, id);
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ERROR;
}

int v9_video_user_update_user(u_int32 id, BOOL gender, int group, char *user, char *user_id, char *pic, char *text)
{
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	sqlite3 * db = v9_user_sqldb_open(id);
	if(db)
	{
		v9_video_user_t usertmp;
		memset(&usertmp, 0, sizeof(v9_video_user_t));
		//printf("========%s===========\r\n", __func__);
		if(v9_user_sqldb_lookup_user(db, id, user_id, &usertmp) != OK)
		{
			if(V9_USER_DEBUG(WARN))
				zlog_warn(MODULE_APP, "this user(%s(%s)) is not exist.", user, user_id);
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return ERROR;
		}
		if(v9_video_sdk_add_user_api(id, gender, group,
									 user, user_id, pic, text, FALSE) != OK)
		{
			if(V9_USER_DEBUG(WARN))
				zlog_warn(MODULE_APP, "this user(%s(%s)) is can't update hw board.", user, user_id);
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return ERROR;
		}
		if(v9_user_sqldb_update(db, id, gender, group, user, user_id, pic, text) == OK)
		{
			sql_snapfea_key key;
			memset(&key, 0, sizeof(sql_snapfea_key));
			if(v9_app_snapfea_key_alloc(&key, FALSE) != OK)
			{
				v9_user_sqldb_close(db, id);
				if(_user_mutex)
					os_mutex_unlock(_user_mutex);
				return ERROR;
			}
			//获取图片特征值
			if(v9_video_sdk_get_keyvalue_api( id, pic, &key) != OK)
			{
				v9_app_snapfea_key_free(&key);
				v9_user_sqldb_close(db, id);
				if(_user_mutex)
					os_mutex_unlock(_user_mutex);
				return ERROR;
			}
			if(v9_user_sqldb_key_update(db, id, user_id, &key) != OK)
			{
				v9_app_snapfea_key_free(&key);
				v9_user_sqldb_close(db, id);
				if(_user_mutex)
					os_mutex_unlock(_user_mutex);
				return ERROR;
			}
			v9_app_snapfea_key_free(&key);
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return OK;
		}
		v9_user_sqldb_close(db, id);
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ERROR;
}

int v9_video_user_del_user(u_int32 id, char *user_id)
{
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	sqlite3 * db = v9_user_sqldb_open(id);
	if(db)
	{
		v9_video_user_t usertmp;
		memset(&usertmp, 0, sizeof(v9_video_user_t));
		//printf("========%s===========\r\n", __func__);
		if(v9_user_sqldb_lookup_user(db, id, user_id, &usertmp) != OK)
		{
			if(V9_USER_DEBUG(WARN))
				zlog_warn(MODULE_APP, "this user(USER ID(%s)) is not exist.", user_id);
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return ERROR;
		}
		if(v9_video_sdk_del_user_api(id, 0, user_id) != OK)
		{
			if(V9_USER_DEBUG(WARN))
				zlog_warn(MODULE_APP, "this user(%s) is can't delete from hw board.", user_id);
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return ERROR;
		}
		if(v9_user_sqldb_del(db, id, user_id) == OK)
		{
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return OK;
		}
		v9_user_sqldb_close(db, id);
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ERROR;
}

int v9_video_user_del_group(u_int32 id,  u_int8 group)
{
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	sqlite3 * db = v9_user_sqldb_open(id);
	if(db)
	{
		if(v9_video_sdk_del_group_api(id, group) != OK)
		{
			if(V9_USER_DEBUG(WARN))
				zlog_warn(MODULE_APP, "can't delete all user from hw board.");
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return ERROR;
		}
		if(v9_user_sqldb_del_group(db, id, group) == OK)
		{
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return OK;
		}
		v9_user_sqldb_close(db, id);
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ERROR;
}


int v9_video_user_lookup_user(u_int32 id, char *user_id, v9_video_user_t *user)
{
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	sqlite3 * db = v9_user_sqldb_open(id);
	if(db)
	{
		//memset(&usertmp, 0, sizeof(v9_video_user_t));
		//printf("========%s===========\r\n", __func__);
		if(v9_user_sqldb_lookup_user(db, id, user_id, user) == OK)
		{
			//sql_snapfea_key *key
/*			if(v9_user_sqldb_key_select(db,  id, user_id, &user->key) != OK)
			{
				v9_user_sqldb_close(db, id);
				return ERROR;
			}*/
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return OK;
		}
		v9_user_sqldb_close(db, id);
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ERROR;
}

int v9_video_user_lookup_user_url(u_int32 id, char *user_id, v9_video_user_t *user)
{
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	sqlite3 * db = v9_user_sqldb_open(id);
	if(db)
	{
		//memset(&usertmp, 0, sizeof(v9_video_user_t));
		//printf("========%s===========\r\n", __func__);
		if(v9_user_sqldb_lookup_user(db, id, user_id, user) == OK)
		{
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return OK;
		}
		v9_user_sqldb_close(db, id);
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ERROR;
}

int v9_video_user_foreach(u_int32 id, int groupid, v9_vidoe_callback cb, void *pVoid)
{
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	sqlite3 * db = v9_user_sqldb_open(id);
	if(db)
	{
		if(v9_user_sqldb_foreach(db, id, groupid, NULL, cb, pVoid) == OK)
		{
			v9_user_sqldb_close(db, id);
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return OK;
		}
		v9_user_sqldb_close(db, id);
	}
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ERROR;
}

int v9_video_user_show(struct vty *vty, BOOL detail)
{
	return OK;
}


/*
 * 批量导入
 */

static int v9_video_user_rules_split(const char *rulesfile, char *path)
{
	if(os_file_access(rulesfile) == OK)
	{
		FILE *f = NULL;
		char *s = NULL;
		char buf[512];
		f = fopen (rulesfile, "r");
		if (f)
		{
			while (fgets (buf, sizeof(buf), f))
			{
				if(strstr(buf, "HaveRules"))
				{
					for (s = buf + strlen (buf); (s > buf) && isspace((int ) *(s - 1));
							s--)
						;
					*s = '\0';
					s = buf + strlen("HaveRules") + 1;
					if(s && *s != '\0')
					{
						fclose (f);
						return atoi(s);
					}
				}
				else if(strstr(buf, "FaceDirectName"))
				{
					s = buf + strlen("FaceDirectName") + 1;
					if(path)
						strcpy(path, s);
				}
			}
			fclose (f);
			return ERROR;
		}
	}
	return ERROR;
}


static int v9_video_filename_split(const char *filename, int *gender, int *group,
								   char *user, char *user_id, char *text)
{
	int ret = ERROR;
	if(strchr_count(filename, '_') == 3)
	{
		ret = sscanf(filename, "%d_%[^_]_%[^_]_%d", group, user, user_id, gender);
	}
	else if(strchr_count(filename, '_') == 4)
	{
		ret = sscanf(filename, "%d_%[^_]_%[^_]_%d_%[^.]", group, user, user_id, gender, text);
	}
	return ret;
}

static int v9_video_user_add_dir(sqlite3 * db, u_int32 id, BOOL gender, int group, char *user,
								 char *user_id, char *pic, char *text)
{
	if(db)
	{
		v9_video_user_t usertmp;
		memset(&usertmp, 0, sizeof(v9_video_user_t));
		//printf("========%s===========\r\n", __func__);
		if(v9_user_sqldb_lookup_user(db, id, user_id, &usertmp) == OK)
		{
			if(V9_USER_DEBUG(WARN))
				zlog_warn(MODULE_APP, "this user(%s(%s)) is already exist.", user, user_id);
			return ERROR;
		}

		if(v9_video_sdk_add_user_api(id, gender, group,
									 user, user_id, pic, text, TRUE) != OK)
		{
			if(V9_USER_DEBUG(WARN))
				zlog_warn(MODULE_APP, "this user(%s(%s)) is can't set hw board.", user, user_id);
			return ERROR;
		}

		if(v9_user_sqldb_add(db, id, gender, group, user, user_id, pic, text) == OK)
		{
			sql_snapfea_key key;
			memset(&key, 0, sizeof(sql_snapfea_key));
			if(v9_app_snapfea_key_alloc(&key, FALSE) != OK)
			{
				v9_video_sdk_del_user_api(id, group, user_id);
				v9_user_sqldb_del(db, id, user_id);
				return ERROR;
			}
			//获取图片特征值
			if(v9_video_sdk_get_keyvalue_api( id, pic, &key) != OK)
			{
				v9_app_snapfea_key_free(&key);
				v9_video_sdk_del_user_api(id, group, user_id);
				v9_user_sqldb_del(db, id, user_id);
				return ERROR;
			}
			if(v9_user_sqldb_key_update(db, id, user_id, &key) != OK)
			{
				v9_app_snapfea_key_free(&key);
				v9_video_sdk_del_user_api(id, group, user_id);
				v9_user_sqldb_del(db, id, user_id);
				return ERROR;
			}
			v9_app_snapfea_key_free(&key);
			return OK;
		}
	}
	return ERROR;
}

static int v9_video_emptydir_clean(const char *dirpath, int flag)
{
	char *str = dirpath + strlen("/mnt/diska1/board1/user/");
	char edirpath[APP_PATH_MAX];
	os_rmdir(dirpath, flag);
	if(str)
	{
		memset(edirpath, 0, sizeof(edirpath));
		snprintf(edirpath, sizeof(edirpath), "/tmp/app/tftpboot/%s", str);
		os_rmdir(edirpath, flag);
	}
	return OK;
}
/*
 * id: 板卡ID
 * rules：是否按规则
 * dirpath：文件所在的绝对路径
 */
static int v9_video_dirfile(sqlite3 * db, u_int32 id, BOOL rules, const char *dirpath)
{
	DIR *dir = NULL;
	struct dirent *d = NULL;
	struct stat sb;
	v9_video_user_t user;
	int ret = -1;
	char pathfile[APP_PATH_MAX*2];
	/* Open directory. */
	dir = opendir (dirpath);
	if (!dir)
	{
		if(V9_USER_DEBUG(WARN))
			zlog_warn(MODULE_APP,"can not open %s", dirpath);
		v9_video_emptydir_clean(dirpath, 1);
		return ERROR;
	}
	/* Walk through the directory. */
	while ((d = readdir (dir)) != NULL)
	{
		//把当前目录.，上一级目录..及隐藏文件都去掉，避免死循环遍历目录
		if(strncmp(d->d_name, ".", 1) == 0)
		{
			continue;
		}
		//printf("--------------- %s filename=%s:\r\n", __func__, d->d_name);//不带目录的文件名称
		memset(pathfile, 0, sizeof(pathfile));
		snprintf(pathfile, sizeof(pathfile), "%s/%s", dirpath, d->d_name);

		//cp  app/tftpboot/aabbcc/* /mnt/diska1/board1/user/

		//判断该文件是否是目录，及是否已搜索了三层，这里我定义只搜索了三层目录，太深就不搜了，省得搜出太多文件
		if(stat(pathfile, &sb) >= 0 && S_ISDIR(sb.st_mode))
		{
			continue;
		}
		/* See if this is a process */
		if(os_file_access(pathfile) == OK)
		{
			memset(&user, 0, sizeof(v9_video_user_t));

			ret = v9_video_filename_split(d->d_name, &user.gender, &user.group,
										  &user.username, &user.userid, &user.text);
			if(ret != ERROR)
			{
				char uploadfile[128];
				char *tmp = strrchr(pathfile, '.');
				memset(uploadfile, 0, sizeof(uploadfile));
				snprintf(uploadfile, sizeof(uploadfile), "%s/table%d/%s%s", V9_USER_DB_DIR,
						 V9_APP_DB_ID_ABS(id), user.userid, tmp);

				rename(pathfile, uploadfile);
				sync();
				if(os_file_access(uploadfile) == OK)
				{
				ret = v9_video_user_add_dir(db, id, user.gender, user.group, user.username, user.userid,
											uploadfile, strlen(user.text)?user.text:NULL);
				if(ret != OK && V9_USER_DEBUG(WARN))
					zlog_warn(MODULE_APP," can not add user '%s' ID '%s' pic %s", user.username, user.userid, d->d_name);
				}
			}
		}
	}
	closedir (dir);
	v9_video_emptydir_clean(dirpath, 1);
	//printf("--------------- %s os_rmdir dirpath=%s:\r\n", __func__, dirpath);//不带目录的文件名称
	return ret;
}



/*
 * id: 板卡ID
 * dirpath：文件所在的绝对路径
 */
int v9_video_user_dir_add(u_int32 id, const char *dirpath)
{
	int ret = 0;
	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);
	char rulesfile[APP_PATH_MAX];
	memset(rulesfile, 0, sizeof(rulesfile));
	snprintf(rulesfile, sizeof(rulesfile), "%s/%s", dirpath, "Rules.txt");
	ret = v9_video_user_rules_split(rulesfile, NULL);
	if(ret != ERROR)
	{
		sqlite3 * db = v9_user_sqldb_open(id);
		if(db)
		{
			ret = v9_video_dirfile(db, id, ret, dirpath);
			v9_user_sqldb_close(db, id);
			//cp  app/tftpboot/aabbcc/* /mnt/diska1/board1/user/
			if(_user_mutex)
				os_mutex_unlock(_user_mutex);
			return ret;
		}
		else
		{
			v9_video_emptydir_clean(dirpath, 1);
		}
		if(V9_USER_DEBUG(WARN))
			zlog_debug(MODULE_APP, "Can not open sql db");
	}
	else
	{
		v9_video_emptydir_clean(dirpath, 1);
	}
	if(V9_USER_DEBUG(WARN))
		zlog_debug(MODULE_APP, "Can not open %s",rulesfile);
	if(_user_mutex)
		os_mutex_unlock(_user_mutex);
	return ret;
}

