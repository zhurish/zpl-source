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

#include "application.h"

static LIST *user_table = NULL;
static os_mutex_t *user_mutex = NULL;

static int v9_video_user_update_save(void);
static int v9_video_user_load_from_file(void);


int v9_video_user_clean(void)
{
	NODE node;
	v9_video_user_t *dbtest = NULL;
	if(user_mutex)
		os_mutex_lock(user_mutex, OS_WAIT_FOREVER);
	for (dbtest = (v9_video_user_t *) lstFirst(user_table);
			dbtest != NULL; dbtest = (v9_video_user_t *) lstNext(&node))
	{
		node = dbtest->node;
		if (dbtest)
		{
			lstDelete(user_table, (NODE *) dbtest);
			XFREE(MTYPE_VIDEO_DATA, dbtest);
		}
	}
#ifdef V9_USER_DB_FILE
	remove(V9_USER_DB_FILE);
#endif
	sync();
	if(user_mutex)
		os_mutex_unlock(user_mutex);
	return OK;
}

int v9_video_user_exit()
{
	v9_video_user_clean();
	if(user_mutex)
	{
		os_mutex_lock(user_mutex, OS_WAIT_FOREVER);
		if(os_mutex_exit(user_mutex)==OK)
			user_mutex = NULL;
	}
	if(user_table)
	{
		XFREE(MTYPE_VIDEO_TOP, user_table);
		user_table = NULL;
	}
	return OK;
}




int v9_video_user_load()
{
	if (user_table == NULL)
	{
		user_table = XMALLOC(MTYPE_VIDEO_TOP, sizeof(LIST));
		if (user_table)
		{
			if(user_mutex == NULL)
				user_mutex = os_mutex_init();
			lstInit(user_table);
			if(user_mutex)
				os_mutex_lock(user_mutex, OS_WAIT_FOREVER);
			v9_video_user_load_from_file();
			if(user_mutex)
				os_mutex_unlock(user_mutex);
			return OK;
		}
		return ERROR;
	}
	return OK;
}

#ifdef V9_USER_DB_FILE
static int v9_video_user_read_one(int fd, v9_video_user_t *node)
{
	v9_video_user_t *addnode = NULL;
	if(read(fd, node, sizeof(v9_video_user_t)) == sizeof(v9_video_user_t))
	{
		addnode = XMALLOC(MTYPE_VIDEO_DATA, sizeof(v9_video_user_t));
		if(!addnode)
			return ERROR;
		memset(addnode, 0, sizeof(v9_video_user_t));
		memcpy(addnode, node, sizeof(v9_video_user_t));
		addnode->node.previous = NULL;
		addnode->node.next = NULL;
		lstAdd(user_table, (NODE *) addnode);
		return OK;
	}
	return ERROR;
}
#endif

static int v9_video_user_load_from_file(void)
{
#ifdef V9_USER_DB_FILE
	int ret = OK, fd = 0;
	v9_video_user_t dbase;
	if (user_table == NULL)
		return ERROR;
	if(ret == 0)
	{
		fd = open(V9_USER_DB_FILE, O_RDONLY);
		if(fd <= 0)
		{
			return ERROR;
		}
		while(ret == OK)
		{
			memset(&dbase, 0, sizeof(v9_video_user_t));
			ret = v9_video_user_read_one(fd, &dbase);
		}
		close(fd);
		return OK;
	}
	return ERROR;
#else
	return OK;
#endif
}

#ifdef V9_USER_DB_FILE
static int v9_video_user_write_list(int fd)
{
	int ret = 0;
	NODE node;
	v9_video_user_t *dbase = NULL;
	for (dbase = (v9_video_user_t *) lstFirst(user_table);
			dbase != NULL; dbase = (v9_video_user_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			ret = write(fd, dbase, sizeof(v9_video_user_t));
			if(ret != sizeof(v9_video_user_t))
				break;
		}
	}
	return ret;
}
#endif
static int v9_video_user_update_save(void)
{
#ifdef V9_USER_DB_FILE
	int ret = 0, fd = 0;
	if (user_table == NULL)
		return ERROR;
	if(lstCount(user_table) == 0)
	{
		if(access(V9_USER_DB_FILE, F_OK) == 0)
			remove(V9_USER_DB_FILE);
		sync();
		return OK;
	}
	if(ret == 0)
	{
		fd = open(V9_USER_DB_FILE".tmp", O_RDWR|O_CREAT, 0644);
		if(fd <= 0)
		{
			return ERROR;
		}
		if(v9_video_user_write_list(fd) != sizeof(v9_video_user_t))
		{
			close(fd);
			remove(V9_USER_DB_FILE".tmp");
			sync();
			return ERROR;
		}
		close(fd);
		rename(V9_USER_DB_FILE".tmp", V9_USER_DB_FILE);
		sync();
		return OK;
	}
	return ERROR;
#else
	return OK;
#endif
}



/***********************************************/
static v9_video_user_t * v9_video_user_node_lookup_by_userid(int id, char *user_id)
{
	int flags  = 0;
	char userid[APP_USERNAME_MAX];
	NODE node;
	v9_video_user_t *dbase = NULL;
	if (user_table == NULL)
		return NULL;
	memset(userid, 0, sizeof(userid));
	if(user_id)
		strncpy(userid, user_id, MIN(sizeof(userid), strlen(user_id)));
	for (dbase = (v9_video_user_t *) lstFirst(user_table);
			dbase != NULL; dbase = (v9_video_user_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			if(id && (dbase->ID == id))
				flags = 1;
			else if(id == 0)
				flags = 1;
			if( flags && (memcmp(dbase->userid, userid, sizeof(userid)) == 0) )
			{
				return dbase;
			}
		}
	}
	return NULL;
}







v9_video_user_t * v9_video_user_lookup_by_userid(int id, char *user_id)
{
	v9_video_user_t * dbase = NULL;
	if(user_mutex)
		os_mutex_lock(user_mutex, OS_WAIT_FOREVER);
	dbase = v9_video_user_node_lookup_by_userid( id, user_id);
	if(user_mutex)
		os_mutex_unlock(user_mutex);
	return dbase;
}

int v9_video_user_add_user(u_int32 id, BOOL gender, int group, char *user, char *user_id, char *pic)
{
	int ret = 0;
	v9_video_user_t * dbase = NULL;
	printf("%s: id=%d gender=%d group=%d user=%s user_id=%s pic=%s", __func__, id, gender, group, user, user_id, pic);
	if(user_mutex)
		os_mutex_lock(user_mutex, OS_WAIT_FOREVER);
	if(v9_video_user_node_lookup_by_userid( id,  user_id))
	{
		//printf("%s: id=%d gender=%d group=%d user=%s user_id=%s pic=%s", __func__, id, gender, group, user, user_id, pic);
		if(user_mutex)
			os_mutex_unlock(user_mutex);
		return ERROR;
	}
	dbase = XMALLOC(MTYPE_VIDEO_DATA, sizeof(v9_video_user_t));
	if(!dbase)
	{
		return ERROR;
	}
	memset(dbase, 0, sizeof(v9_video_user_t));

	dbase->ID = id;
	dbase->group = group;										// 所属组ID  0： 黑名单 1： 白名单
	strcpy(dbase->username, user);			// 姓名
	strcpy(dbase->userid, user_id);				// 证件号
	dbase->gender = gender;										// 人员性别  0： 女 1： 男
	if(pic)
		strcpy(dbase->picname, pic);

	if(dbase->ID == 0)
	{
#ifndef V9_USER_DB_NOSDK
		ret = v9_video_sdk_add_user_all_api(dbase->gender, dbase->group, dbase->username, dbase->userid, dbase->picname, FALSE);
#else
		ret = OK;
#endif
		if(ret == OK)
		{
			lstAdd(user_table, (NODE *) dbase);
			ret = OK;
			if(ret == OK)
				ret = v9_video_user_update_save();
		}
		else
			ret = ERROR;
	}
	else
	{
#ifndef V9_USER_DB_NOSDK
		if(v9_video_sdk_add_user_api(dbase->ID, dbase->gender, dbase->group, dbase->username, dbase->userid, dbase->picname, FALSE) == OK)
#endif
		{
			lstAdd(user_table, (NODE *) dbase);
			ret = OK;
			if(ret == OK)
				ret = v9_video_user_update_save();
		}
#ifndef V9_USER_DB_NOSDK
		else
			ret = ERROR;
#endif
	}
	if(user_mutex)
		os_mutex_unlock(user_mutex);
	return ret;
}

int v9_video_user_update_user(u_int32 id, BOOL gender, int group, char *user, char *user_id, char *pic)
{
	int ret = 0;
	v9_video_user_t * dbase = NULL;

	printf("%s: id=%d gender=%d group=%d user=%s user_id=%s pic=%s", __func__, id, gender, group, user, user_id, pic);

	if(user_mutex)
		os_mutex_lock(user_mutex, OS_WAIT_FOREVER);

	dbase = v9_video_user_node_lookup_by_userid( id,  user_id);
	if(!dbase)
	{
		printf("%s: can not lookup by user_id=%s", __func__, user_id);
		if(user_mutex)
			os_mutex_unlock(user_mutex);
		return ERROR;
	}

	dbase->ID = id;
	dbase->group = group;					// 所属组ID  0： 黑名单 1： 白名单
	memset(dbase->username, 0, sizeof(dbase->username));
	memset(dbase->username, 0, sizeof(dbase->userid));
	memset(dbase->username, 0, sizeof(dbase->picname));
	strcpy(dbase->username, user);			// 姓名
	strcpy(dbase->userid, user_id);				// 证件号
	dbase->gender = gender;										// 人员性别  0： 女 1： 男
	if(pic)
		strcpy(dbase->picname, pic);

	if(dbase->ID == 0)
	{
#ifndef V9_USER_DB_NOSDK
		ret = v9_video_sdk_add_user_all_api(dbase->gender, dbase->group, dbase->username, dbase->userid, dbase->picname, TRUE);
#else
		ret = OK;
#endif
		if(ret == OK)
		{
			ret = OK;
			if(ret == OK)
				ret = v9_video_user_update_save();
		}
		else
			ret = ERROR;
	}
	else
	{
#ifndef V9_USER_DB_NOSDK
		if(v9_video_sdk_add_user_api(dbase->ID, dbase->gender, dbase->group, dbase->username, dbase->userid, dbase->picname, TRUE) == OK)
#endif
		{
			ret = OK;
			if(ret == OK)
				ret = v9_video_user_update_save();
		}
#ifndef V9_USER_DB_NOSDK
		else
			ret = ERROR;
#endif
	}
	if(user_mutex)
		os_mutex_unlock(user_mutex);
	return ret;
}

int v9_video_user_del_user(u_int32 id, char *user_id)
{
	int ret = 0;
	v9_video_user_t * dbase = NULL;
	printf("%s: id=%d user_id=%s", __func__, id, user_id);
	if(user_mutex)
		os_mutex_lock(user_mutex, OS_WAIT_FOREVER);

	dbase = v9_video_user_node_lookup_by_userid( id, user_id);
	if(dbase)
	{
		if(id == 0)
		{
#ifndef V9_USER_DB_NOSDK
			ret = v9_video_sdk_del_user_all_api(user_id);
#endif
			if(ret == OK)
			{
				printf("%s: delete id=%d user_id=%s", __func__, id, user_id);
				lstDelete(user_table, (NODE *) dbase);
				XFREE(MTYPE_VIDEO_DATA, dbase);

				v9_video_user_update_save();
				if(user_mutex)
					os_mutex_unlock(user_mutex);
				return OK;
			}
		}
		else
		{
#ifndef V9_USER_DB_NOSDK
			if(v9_video_sdk_del_user_api(id, user_id) == OK)
#endif
			{
				printf("%s: delete id=%d user_id=%s", __func__, id, user_id);
				lstDelete(user_table, (NODE *) dbase);
				XFREE(MTYPE_VIDEO_DATA, dbase);

				v9_video_user_update_save();
				if(user_mutex)
					os_mutex_unlock(user_mutex);
				return OK;
			}
		}
	}
	if(user_mutex)
		os_mutex_unlock(user_mutex);
	return ERROR;
}


int v9_video_user_foreach(v9_vidoe_callback cb, void *pVoid)
{
	NODE node;
	v9_video_user_t *dbase = NULL;
	if (user_table == NULL)
		return NULL;
	if(user_mutex)
		os_mutex_lock(user_mutex, OS_WAIT_FOREVER);
	for (dbase = (v9_video_user_t *) lstFirst(user_table);
			dbase != NULL; dbase = (v9_video_user_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase && cb)
		{
			(cb)(dbase, pVoid);
		}
	}
	if(user_mutex)
		os_mutex_unlock(user_mutex);
	return OK;
}


static int v9_video_user_show_callback(v9_video_user_t *user, struct vty *vty)
{
	if(user && vty)
	{
		vty_out(vty, "%-16s %-4s %-16s %d       %-6s %-16s%s",
			user->username,
			user->gender ? "Men":"Women",
			user->userid,
			user->ID,
			user->group ? "白名单":"黑名单",
			user->picname, VTY_NEWLINE);
		return OK;
	}
	return ERROR;
}


int v9_video_user_show(struct vty *vty, BOOL detail)
{
	if(lstCount(user_table) > 0)
	{
		vty_out(vty, "%-16s %-4s %-16s %-6s %-6s %-16s%s",
			"Username",
			"Uender",
			"UserID",
			"Board",
			"group",
			"picname", VTY_NEWLINE);
		v9_video_user_foreach(v9_video_user_show_callback, vty);
	}
	return OK;
}
