/*
 * voip_dbase.c
 *
 *  Created on: Mar 16, 2019
 *      Author: zhurish
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

#include "x5_b_global.h"
#ifdef X5B_APP_DATABASE

#ifdef PL_PJSIP_MODULE
#include "voip_app.h"
#endif
#include "x5b_dbase.h"

static LIST *dbase_table = NULL;
static os_mutex_t *dbase_mutex = NULL;

static int voip_dbase_update_save(void);
static int voip_dbase_load_from_file(void);

#ifdef PL_OPENWRT_UCI
static int voip_ubus_dbase_select_one(BOOL enable);
#endif
static BOOL _voip_dbase = FALSE;

int voip_dbase_enable(BOOL enable)
{
	_voip_dbase = enable;
	return OK;
}


BOOL voip_dbase_isenable()
{
	return FALSE;//_voip_dbase;
}

int voip_dbase_clean(void)
{
	NODE node;
	voip_dbase_t *dbtest = NULL;
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	for (dbtest = (voip_dbase_t *) lstFirst(dbase_table);
			dbtest != NULL; dbtest = (voip_dbase_t *) lstNext(&node))
	{
		node = dbtest->node;
		if (dbtest)
		{
			lstDelete(dbase_table, (NODE *) dbtest);
			XFREE(MTYPE_VOIP_DBTEST, dbtest);
		}
	}
#ifdef X5B_DBASE_FILE
	remove(X5B_DBASE_FILE);
#endif
	sync();
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	voip_card_clean();
	return OK;
}

int voip_dbase_exit()
{
	voip_dbase_clean();
	if(dbase_mutex)
	{
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
		if(os_mutex_exit(dbase_mutex)==OK)
			dbase_mutex = NULL;
	}
	if(dbase_table)
	{
		XFREE(MTYPE_VOIP_TOP, dbase_table);
		dbase_table = NULL;
	}
	voip_card_exit();
	return OK;
}




int voip_dbase_load()
{
	if (dbase_table == NULL)
	{
		dbase_table = XMALLOC(MTYPE_VOIP_TOP, sizeof(LIST));
		if (dbase_table)
		{
			if(dbase_mutex == NULL)
				dbase_mutex = os_mutex_init();
			lstInit(dbase_table);
			if(dbase_mutex)
				os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
			voip_dbase_load_from_file();
			if(dbase_mutex)
				os_mutex_unlock(dbase_mutex);
			if (access(X5B_DBASE_FILE, F_OK) == 0)
				_voip_dbase = TRUE;

			voip_card_load();
#ifdef PL_OPENWRT_UCI
			voip_ubus_dbase_select_one(FALSE);
#endif
			return OK;
		}
		return ERROR;
	}
	return OK;
}

#ifdef X5B_DBASE_FILE
static int voip_dbase_read_one(int fd, voip_dbase_t *node)
{
	voip_dbase_t *addnode = NULL;
	if(read(fd, node, sizeof(voip_dbase_t)) == sizeof(voip_dbase_t))
	{
		addnode = XMALLOC(MTYPE_VOIP_DBTEST, sizeof(voip_dbase_t));
		if(!addnode)
			return ERROR;
		memset(addnode, 0, sizeof(voip_dbase_t));
		memcpy(addnode, node, sizeof(voip_dbase_t));
		lstAdd(dbase_table, (NODE *) addnode);
		return OK;
	}
	return ERROR;
}
#endif

static int voip_dbase_load_from_file(void)
{
#ifdef X5B_DBASE_FILE
	int ret = OK, fd = 0;
	voip_dbase_t dbase;
	char tmp[4];
	if (dbase_table == NULL)
		return ERROR;
	if(ret == 0)
	{
		fd = open(X5B_DBASE_FILE, O_RDONLY);
		if(fd <= 0)
		{
			return ERROR;
		}
		if(read(fd, tmp, sizeof(tmp)) == 4)
			_voip_dbase = atoi(tmp);
		while(ret == OK)
		{
			memset(&dbase, 0, sizeof(voip_dbase_t));
			ret = voip_dbase_read_one(fd, &dbase);
		}
		close(fd);
		return OK;
	}
	return ERROR;
#else
	return OK;
#endif
}

#ifdef X5B_DBASE_FILE
static int voip_dbase_write_list(int fd)
{
	int ret = 0;
	NODE node;
	voip_dbase_t *dbase = NULL;
	for (dbase = (voip_dbase_t *) lstFirst(dbase_table);
			dbase != NULL; dbase = (voip_dbase_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			ret = write(fd, dbase, sizeof(voip_dbase_t));
			if(ret != sizeof(voip_dbase_t))
				break;
		}
	}
	return ret;
}
#endif
static int voip_dbase_update_save(void)
{
#ifdef X5B_DBASE_FILE
	int ret = 0, fd = 0;
	if (dbase_table == NULL)
		return ERROR;
	if(lstCount(dbase_table) == 0)
	{
		if(access(X5B_DBASE_FILE, F_OK) == 0)
			remove(X5B_DBASE_FILE);
		sync();
		return OK;
	}
	if(ret == 0)
	{
		char tmp[4];
		fd = open(X5B_DBASE_FILE".tmp", O_RDWR|O_CREAT, 0644);
		if(fd <= 0)
		{
			return ERROR;
		}

		if(write(fd, tmp, sizeof(tmp)) == 4)
			_voip_dbase = atoi(tmp);

		if(voip_dbase_write_list(fd) != sizeof(voip_dbase_t))
		{
			close(fd);
			remove(X5B_DBASE_FILE".tmp");
			sync();
			return ERROR;
		}
		close(fd);
		rename(X5B_DBASE_FILE".tmp", X5B_DBASE_FILE);
		sync();
		return OK;
	}
	return ERROR;
#else
	return OK;
#endif
}



/***********************************************/
voip_dbase_t * voip_dbase_node_lookup_by_username(char *username, char *user_id)
{
	int i = 0;
	char name[APP_USERNAME_MAX];
	char userid[APP_ID_MAX];
	NODE node;
	voip_dbase_t *dbase = NULL;
	if (dbase_table == NULL)
		return NULL;
	memset(name, 0, sizeof(name));
	memset(userid, 0, sizeof(userid));
	if(username)
		strncpy(name, username, MIN(sizeof(name), strlen(username)));
	if(user_id)
		strncpy(userid, user_id, MIN(sizeof(userid), strlen(user_id)));
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	for (dbase = (voip_dbase_t *) lstFirst(dbase_table);
			dbase != NULL; dbase = (voip_dbase_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
			{
				if(dbase->phonetab[i].use_flag == 0)
					continue;
				if(username && user_id)
				{
					if( (memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0) &&
						(memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0) )
					{
						if(dbase_mutex)
							os_mutex_unlock(dbase_mutex);
						return dbase;
					}
				}
				else
				{
					if(username)
					{
						if( (memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0))
						{
							if(dbase_mutex)
								os_mutex_unlock(dbase_mutex);
							return dbase;
						}
					}
					else if(user_id)
					{
						//zlog_debug(MODULE_APP, "===========in-userid:%s userid:%s", userid, dbase->phonetab[i].user_id);

						if( (memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0) )
						{
							if(dbase_mutex)
								os_mutex_unlock(dbase_mutex);
							return dbase;
						}
					}
				}
			}
		}
	}
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	return NULL;
}

voip_dbase_t * voip_dbase_node_lookup_by_phonenumber(char *phone)
{
	int i = 0;
	char lphone[APP_USERNAME_MAX];
	NODE node;
	voip_dbase_t *dbase = NULL;
	if (dbase_table == NULL)
		return NULL;
	zassert(phone != NULL);
	memset(lphone, 0, sizeof(lphone));
	if(phone)
		strncpy(lphone, phone, MIN(sizeof(lphone), strlen(phone)));
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	for (dbase = (voip_dbase_t *) lstFirst(dbase_table);
			dbase != NULL; dbase = (voip_dbase_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
			{
				if(dbase->phonetab[i].use_flag == 0)
					continue;
				if( (memcmp(dbase->phonetab[i].phone, lphone, sizeof(lphone)) == 0))
				{
					if(dbase_mutex)
						os_mutex_unlock(dbase_mutex);
					return dbase;
				}
			}
		}
	}
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	return NULL;
}


static voip_dbase_t * voip_dbase_node_lookup_by_room(u_int8 building, u_int8 unit, u_int16 room_number)
{
	NODE node;
	voip_dbase_t *dbase = NULL;
	if (dbase_table == NULL)
		return NULL;
	for (dbase = (voip_dbase_t *) lstFirst(dbase_table);
			dbase != NULL; dbase = (voip_dbase_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			if (dbase->building == building && dbase->unit == unit &&
					dbase->room_number == room_number)
			{
				return dbase;
			}
		}
	}
	return NULL;
}




static int voip_dbase_add_one_node(voip_dbase_t *dbase)
{
	lstAdd(dbase_table, (NODE *) dbase);
	return OK;
}

static int voip_dbase_del_one_node(voip_dbase_t *dbase)
{
	lstDelete(dbase_table, (NODE *) dbase);
	XFREE(MTYPE_VOIP_DBTEST, dbase);
	return OK;
}

static int voip_dbase_add_phonenumber(voip_dbase_t *dbase, char *number, char *username, char *user_id)
{
	int i = 0;
	for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
	{
		if(strlen(dbase->phonetab[i].phone) == 0)
		{
			strcpy(dbase->phonetab[i].phone, number);
			if(username)
			{
				memset(dbase->phonetab[i].username, 0, sizeof(dbase->phonetab[i].username));
				strcpy(dbase->phonetab[i].username, username);
			}
			if(user_id)
			{
				memset(dbase->phonetab[i].user_id, 0, sizeof(dbase->phonetab[i].user_id));
				strcpy(dbase->phonetab[i].user_id, user_id);
			}
			dbase->phonetab[i].use_flag = 1;
			dbase->number++;
			return OK;
		}
	}
	return ERROR;
}

static int voip_dbase_del_phonenumber(voip_dbase_t *dbase, char *number, char *username, char *user_id)
{
	int i = 0;
	char phone[APP_ID_MAX];
	char name[APP_USERNAME_MAX];
	char userid[APP_ID_MAX];
	memset(phone, 0, sizeof(phone));
	memset(name, 0, sizeof(name));
	memset(userid, 0, sizeof(userid));
	if(number)
		strncpy(phone, number, MIN(sizeof(phone), strlen(number)));
	if(username)
		strncpy(name, username, MIN(sizeof(name), strlen(username)));
	if(user_id)
		strncpy(userid, user_id, MIN(sizeof(name), strlen(user_id)));

	for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
	{
		if(number && username && user_id)
		{
			if ( (memcmp(dbase->phonetab[i].phone, phone, sizeof(phone)) == 0) &&
				(memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0) &&
				(memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0) )
			{
				dbase->phonetab[i].use_flag = 0;
				memset(dbase->phonetab[i].phone, 0, sizeof(dbase->phonetab[i].phone));
				memset(dbase->phonetab[i].username, 0, sizeof(dbase->phonetab[i].username));
				memset(dbase->phonetab[i].user_id, 0, sizeof(dbase->phonetab[i].user_id));
				dbase->number--;
				if(dbase->number == 0)
					voip_dbase_del_one_node(dbase);
				return OK;
			}
		}
		else
		{
			if(number)
			{
				if (memcmp(dbase->phonetab[i].phone, phone, sizeof(phone)) == 0)
				{
					dbase->phonetab[i].use_flag = 0;
					memset(dbase->phonetab[i].phone, 0, sizeof(dbase->phonetab[i].phone));
					memset(dbase->phonetab[i].username, 0, sizeof(dbase->phonetab[i].username));
					memset(dbase->phonetab[i].user_id, 0, sizeof(dbase->phonetab[i].user_id));
					dbase->number--;
					if(dbase->number == 0)
						voip_dbase_del_one_node(dbase);
					return OK;
				}
			}
			else if(username)
			{
				if (memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0)
				{
					dbase->phonetab[i].use_flag = 0;
					memset(dbase->phonetab[i].phone, 0, sizeof(dbase->phonetab[i].phone));
					memset(dbase->phonetab[i].username, 0, sizeof(dbase->phonetab[i].username));
					memset(dbase->phonetab[i].user_id, 0, sizeof(dbase->phonetab[i].user_id));
					dbase->number--;
					if(dbase->number == 0)
						voip_dbase_del_one_node(dbase);
					return OK;
				}
			}
			else if(user_id)
			{
				if (memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0)
				{
					dbase->phonetab[i].use_flag = 0;
					memset(dbase->phonetab[i].phone, 0, sizeof(dbase->phonetab[i].phone));
					memset(dbase->phonetab[i].username, 0, sizeof(dbase->phonetab[i].username));
					memset(dbase->phonetab[i].user_id, 0, sizeof(dbase->phonetab[i].user_id));
					dbase->number--;
					if(dbase->number == 0)
						voip_dbase_del_one_node(dbase);
					return OK;
				}
			}
		}
	}
	return ERROR;
}


static int voip_dbase_add_one_room(u_int8 building, u_int8 unit, u_int16 room_number)
{
	voip_dbase_t *dbase = XMALLOC(MTYPE_VOIP_DBTEST, sizeof(voip_dbase_t));
	if(!dbase)
		return ERROR;
	memset(dbase, 0, sizeof(voip_dbase_t));
	dbase->building = building;
	dbase->unit = unit;
	dbase->room_number = room_number;
	return voip_dbase_add_one_node(dbase);
}

static int voip_dbase_del_one_room(u_int8 building, u_int8 unit, u_int16 room_number)
{
	voip_dbase_t * dbase = voip_dbase_node_lookup_by_room(building, unit, room_number);
	if(dbase)
	{
		return voip_dbase_del_one_node(dbase);
	}
	return ERROR;
}

voip_dbase_t * voip_dbase_lookup_by_room(u_int8 building, u_int8 unit, u_int16 room_number)
{
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	voip_dbase_t * dbase = voip_dbase_node_lookup_by_room( building,  unit,  room_number);
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	return dbase;
}

int voip_dbase_add_room(u_int8 building, u_int8 unit, u_int16 room_number)
{
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	int ret = voip_dbase_add_one_room( building,  unit, room_number);
	if(ret == OK)
		ret = voip_dbase_update_save();
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	return ret;
}

int voip_dbase_del_room(u_int8 building, u_int8 unit, u_int16 room_number)
{
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	int ret = voip_dbase_del_one_room( building, unit, room_number);
	if(ret == OK)
		ret = voip_dbase_update_save();
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	return ERROR;
}

int voip_dbase_del_user(char *user_id)
{
	int ret = ERROR;
	//u_int8 building;
	//u_int8 unit;
	//u_int16 room_number;
	voip_dbase_t * dbase = NULL;
	if(user_id)
	{
		dbase = voip_dbase_node_lookup_by_username(NULL, user_id);
		if(dbase == NULL)
			return ERROR;
	}
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	if(dbase)
	{
		ret = voip_dbase_del_phonenumber(dbase, NULL, NULL, user_id);
		if(ret == OK)
		{
			ret = voip_dbase_update_save();
			if(dbase_mutex)
				os_mutex_unlock(dbase_mutex);
			return ret;
		}
	}
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	return ERROR;
}

int voip_dbase_add_room_phone(u_int8 building, u_int8 unit, u_int16 room_number, char *phone, char *username, char *user_id)
{
	int ret = ERROR;
	if(phone)
	{
		if(voip_dbase_node_lookup_by_phonenumber(phone) != NULL)
		{
			return ERROR;
		}
	}
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	voip_dbase_t * dbase = voip_dbase_node_lookup_by_room(building, unit, room_number);

	if(dbase)
	{
		ret = voip_dbase_add_phonenumber(dbase, phone, username, user_id);
		if(ret == OK)
		{
			ret = voip_dbase_update_save();
			if(dbase_mutex)
				os_mutex_unlock(dbase_mutex);
			return ret;
		}
	}
	else
	{
		ret = voip_dbase_add_one_room( building,  unit, room_number);
		dbase = voip_dbase_node_lookup_by_room(building, unit, room_number);
		if(dbase)
		{
			ret = voip_dbase_add_phonenumber(dbase, phone, username, user_id);
			if(ret == OK)
			{
				ret = voip_dbase_update_save();
				if(dbase_mutex)
					os_mutex_unlock(dbase_mutex);
				return ret;
			}
		}
	}
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	return ERROR;
}

int voip_dbase_del_room_phone(u_int8 building, u_int8 unit, u_int16 room_number,
		char *phone, char *username, char *user_id)
{
	int ret = ERROR;
	voip_dbase_t * dbase = NULL;
	if(phone)
	{
		dbase = voip_dbase_node_lookup_by_phonenumber(phone);
		if(dbase == NULL)
			return ERROR;
	}
	else if(username && user_id)
	{
		dbase = voip_dbase_node_lookup_by_username(username, user_id);
		if(dbase == NULL)
			return ERROR;
	}
	else if(user_id)
	{
		dbase = voip_dbase_node_lookup_by_username(NULL, user_id);
		if(dbase == NULL)
			return ERROR;
	}
	else
	{
		if(dbase_mutex)
			os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
		dbase = voip_dbase_node_lookup_by_room(building, unit, room_number);
		if(dbase == NULL)
		{
			if(dbase_mutex)
				os_mutex_unlock(dbase_mutex);
			return ERROR;
		}
	}
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	if(dbase)
	{
		ret = voip_dbase_del_phonenumber(dbase, phone, username, user_id);
		if(ret == OK)
		{
			ret = voip_dbase_update_save();
			if(dbase_mutex)
				os_mutex_unlock(dbase_mutex);
			return ret;
		}
	}
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	return ERROR;
}

int voip_dbase_update_room_phone(u_int8 building, u_int8 unit, u_int16 room_number,
										char *phone, char *username, char *user_id)
{
	int ret = ERROR;
	voip_dbase_t * dbase = NULL;
	dbase = voip_dbase_node_lookup_by_username(NULL, user_id);
	if(dbase == NULL)
	{
		return voip_dbase_add_room_phone( building,  unit,  room_number, phone, username, user_id);
	}
	else
	{
		if( dbase->building != building ||
			dbase->unit != unit ||
			dbase->room_number != room_number)
		{
			ret = voip_dbase_del_phonenumber(dbase, NULL, NULL, user_id);
			return voip_dbase_add_room_phone( building,  unit,  room_number, phone, username, user_id);
		}
		else
		{
			if(dbase_mutex)
				os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
			ret = voip_dbase_del_phonenumber(dbase, NULL, NULL, user_id);
			ret = voip_dbase_add_phonenumber(dbase, phone, username, user_id);
			if(ret == OK)
			{
				ret = voip_dbase_update_save();
				if(dbase_mutex)
					os_mutex_unlock(dbase_mutex);
				return ret;
			}
		}
	}
	return ERROR;
}


int voip_dbase_get_room_phone(u_int8 building, u_int8 unit, u_int16 room_number, char *phone)
{
	int i = 0, num = 0;
	zassert(phone != NULL);
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	voip_dbase_t * dbase = voip_dbase_node_lookup_by_room(building, unit, room_number);
	if(dbase)
	{
		for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
		{
			if(dbase->phonetab[i].use_flag == 0)
				continue;
			if (strlen(dbase->phonetab[i].phone) != 0)
			{
				if(num != 0)
					strcat(phone, ":");
				strcat(phone, dbase->phonetab[i].phone);
				num++;
			}
		}
		if(dbase_mutex)
			os_mutex_unlock(dbase_mutex);
		return num;
	}
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	return 0;
}
#ifdef PL_PJSIP_MODULE
int voip_dbase_get_call_phone(u_int8 building, u_int8 unit, u_int16 room_number, void *call_phone)
{
	int i = 0, num = 0;
	call_phone_t *phone = (call_phone_t *)call_phone;
	zassert(phone != NULL);
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	voip_dbase_t * dbase = voip_dbase_node_lookup_by_room(building, unit, room_number);
	if(dbase)
	{
		for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
		{
			if(dbase->phonetab[i].use_flag == 0)
				continue;
			if (strlen(dbase->phonetab[i].phone) != 0)
			{
				strcpy(phone[num].phone, dbase->phonetab[i].phone);
				strcpy(phone[num].username, dbase->phonetab[i].username);
				strcpy(phone[num].user_id, dbase->phonetab[i].user_id);
				num++;
			}
		}
		if(dbase_mutex)
			os_mutex_unlock(dbase_mutex);
		return num;
	}
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	return 0;
}
#endif
int voip_dbase_get_user_by_phone(char *phone, char *username, char *user_id)
{
	int i = 0;//, num = 0;
	char lphone[APP_USERNAME_MAX];
	memset(lphone, 0, sizeof(lphone));
	zassert(phone != NULL);
	if(phone)
		strncpy(lphone, phone, MIN(sizeof(lphone), strlen(phone)));
	voip_dbase_t * dbase = voip_dbase_node_lookup_by_phonenumber(phone);
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	if(dbase)
	{
		for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
		{
			if(dbase->phonetab[i].use_flag == 0)
				continue;
			if( (memcmp(dbase->phonetab[i].phone, lphone, sizeof(lphone)) == 0))
			{
				if(username)
					strcpy(username, dbase->phonetab[i].username);
				if(user_id)
					strcpy(user_id, dbase->phonetab[i].user_id);
				if(dbase_mutex)
					os_mutex_unlock(dbase_mutex);
				return OK;
			}
		}
	}
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	return ERROR;
}

int voip_dbase_get_phone_by_user(u_int8 building, u_int8 unit, u_int16 room_number, char *username, char *user_id, char *phone)
{
	int i = 0, num = 0;
	char name[APP_USERNAME_MAX];
	char userid[APP_ID_MAX];
	memset(name, 0, sizeof(name));
	memset(userid, 0, sizeof(userid));
	zassert(phone != NULL);
	if(username)
		strncpy(name, username, MIN(sizeof(name), strlen(username)));
	if(user_id)
		strncpy(userid, user_id, MIN(sizeof(name), strlen(user_id)));
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	voip_dbase_t * dbase = voip_dbase_node_lookup_by_room(building, unit, room_number);
	if(dbase)
	{
		for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
		{
			if(dbase->phonetab[i].use_flag == 0)
				continue;
			if(username && user_id)
			{
				if ((memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0) &&
						(memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0))
				{
					if (strlen(dbase->phonetab[i].phone) != 0)
					{
						if(num != 0)
							strcat(phone, ":");
						strcat(phone, dbase->phonetab[i].phone);
						//strcat(phone, ":");
						num++;
					}
				}
			}
			else if(username)
			{
				if (memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0)
				{
					if (strlen(dbase->phonetab[i].phone) != 0)
					{
						if(num != 0)
							strcat(phone, ":");
						strcat(phone, dbase->phonetab[i].phone);
						//strcat(phone, ":");
						num++;
					}
				}
			}
			else if(user_id)
			{
				if (memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0)
				{
					if (strlen(dbase->phonetab[i].phone) != 0)
					{
						if(num != 0)
							strcat(phone, ":");
						strcat(phone, dbase->phonetab[i].phone);
						//strcat(phone, ":");
						num++;
					}
				}
			}
		}
		if(dbase_mutex)
			os_mutex_unlock(dbase_mutex);
		return num;
	}
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	return 0;
}


int voip_dbase_get_room_phone_by_user(char *user_id, u_int16 *room_number,
		char *phone, char *username)
{
	int ret = ERROR, i = 0;
	voip_dbase_t * dbase = NULL;
	if (user_id)
	{
		dbase = voip_dbase_node_lookup_by_username (NULL, user_id);

		//zlog_debug(MODULE_APP, "===========userid:%s room=%d", user_id, dbase? dbase->room_number:0);

		if (dbase == NULL)
			return ERROR;
	}
	if (dbase)
	{
		char userid[APP_ID_MAX];
		memset (userid, 0, sizeof(userid));
		if (user_id)
			strncpy (userid, user_id, MIN(sizeof(userid), strlen (user_id)));

		for (i = 0; i < APP_MULTI_NUMBER_MAX; i++)
		{
			if (dbase->phonetab[i].use_flag == 0)
				continue;

			if (user_id)
			{
				if (memcmp (dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0)
				{
					if (strlen (dbase->phonetab[i].phone) != 0)
					{
						if (phone)
							strcpy (phone, dbase->phonetab[i].phone);
					}
					if (strlen (dbase->phonetab[i].username) != 0)
					{
						if (username)
							strcpy (username, dbase->phonetab[i].username);
					}
					if(room_number)
						*room_number = dbase->room_number;
					if (dbase_mutex)
						os_mutex_unlock (dbase_mutex);
					return OK;
				}
			}
		}
		if (dbase_mutex)
			os_mutex_unlock (dbase_mutex);
		return ret;
	}
	if (dbase_mutex)
		os_mutex_unlock (dbase_mutex);
	return ERROR;
}



static int voip_dbase_show_room_phone_1(struct vty *vty, u_int8 building, u_int8 unit, u_int16 room_number)
{
	int i =0, cnt = 0;
	NODE node;
	voip_dbase_t *dbase = NULL;
	s_int8 building_str[8], unit_str[8], room_number_str[8];

	memset(building_str, 0, sizeof(building_str));
	memset(unit_str, 0, sizeof(unit_str));
	memset(room_number_str, 0, sizeof(room_number_str));

	sprintf(building_str, "%d", building);
	sprintf(unit_str, "%d", unit);
	sprintf(room_number_str, "%04d", room_number);

	for (dbase = (voip_dbase_t *) lstFirst(dbase_table);
			dbase != NULL; dbase = (voip_dbase_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			if(building && unit && room_number)
			{
				if(building == dbase->building && unit == dbase->unit && dbase->room_number == room_number)
				{
					for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
					{
						if(dbase->phonetab[i].use_flag == 0)
							continue;
						if (strlen(dbase->phonetab[i].phone) != 0)
						{
							if(vty->res0)
								vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																	room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																	dbase->phonetab[i].user_id, VTY_NEWLINE);
							else
								vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
									room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
									dbase->phonetab[i].user_id, VTY_NEWLINE);
							cnt++;
						}
					}
				}
			}
			else if((building && unit) || (building && room_number) || (unit && room_number))
			{
				if((building && unit))
				{
					if(building == dbase->building && unit == dbase->unit)
					{
						memset(room_number_str, 0, sizeof(room_number_str));
						sprintf(room_number_str, "%04d", dbase->room_number);
						for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
						{
							if(dbase->phonetab[i].use_flag == 0)
								continue;
							if (strlen(dbase->phonetab[i].phone) != 0)
							{
								if(vty->res0)
									vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																		room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																		dbase->phonetab[i].user_id, VTY_NEWLINE);
								else
									vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
										room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
										dbase->phonetab[i].user_id, VTY_NEWLINE);
								cnt++;
							}
						}
					}
				}
				else if((building && room_number))
				{
					if(building == dbase->building && room_number == dbase->room_number)
					{
						memset(unit_str, 0, sizeof(unit_str));
						sprintf(unit_str, "%d", dbase->unit);
						for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
						{
							if(dbase->phonetab[i].use_flag == 0)
								continue;
							if (strlen(dbase->phonetab[i].phone) != 0)
							{
								if(vty->res0)
									vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																		room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																		dbase->phonetab[i].user_id, VTY_NEWLINE);
								else
									vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
										room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
										dbase->phonetab[i].user_id, VTY_NEWLINE);
								cnt++;
							}
						}
					}
				}
				else if((unit && room_number))
				{
					if(unit == dbase->unit && room_number == dbase->room_number)
					{
						memset(building_str, 0, sizeof(building_str));
						sprintf(building_str, "%04d", dbase->building);
						for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
						{
							if(dbase->phonetab[i].use_flag == 0)
								continue;
							if (strlen(dbase->phonetab[i].phone) != 0)
							{
								if(vty->res0)
									vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																		room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																		dbase->phonetab[i].user_id, VTY_NEWLINE);
								else
									vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
										room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
										dbase->phonetab[i].user_id, VTY_NEWLINE);
								cnt++;
							}
						}
					}
				}
			}
		}
	}
	return cnt;
}

static int voip_dbase_show_room_phone_2(struct vty *vty, u_int8 building, u_int8 unit, u_int16 room_number)
{
	int i =0, cnt = 0;
	NODE node;
	voip_dbase_t *dbase = NULL;
	s_int8 building_str[8], unit_str[8], room_number_str[8];

	memset(building_str, 0, sizeof(building_str));
	memset(unit_str, 0, sizeof(unit_str));
	memset(room_number_str, 0, sizeof(room_number_str));

	sprintf(building_str, "%d", building);
	sprintf(unit_str, "%d", unit);
	sprintf(room_number_str, "%04d", room_number);

	for (dbase = (voip_dbase_t *) lstFirst(dbase_table);
			dbase != NULL; dbase = (voip_dbase_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			if(building || unit || room_number)
			{
				if(building)
				{
					if(building == dbase->building)
					{
						memset(unit_str, 0, sizeof(unit_str));
						sprintf(unit_str, "%d", dbase->unit);
						memset(room_number_str, 0, sizeof(room_number_str));
						sprintf(room_number_str, "%04d", dbase->room_number);
						for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
						{
							if(dbase->phonetab[i].use_flag == 0)
								continue;
							if (strlen(dbase->phonetab[i].phone) != 0)
							{
								if(vty->res0)
									vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																		room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																		dbase->phonetab[i].user_id, VTY_NEWLINE);
								else
									vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
										room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
										dbase->phonetab[i].user_id, VTY_NEWLINE);
								cnt++;
							}
						}
					}
				}
				else if(unit)
				{
					if(unit == dbase->unit)
					{
						memset(building_str, 0, sizeof(building_str));
						sprintf(building_str, "%d", dbase->building);
						memset(room_number_str, 0, sizeof(room_number_str));
						sprintf(room_number_str, "%04d", dbase->room_number);
						for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
						{
							if(dbase->phonetab[i].use_flag == 0)
								continue;
							if (strlen(dbase->phonetab[i].phone) != 0)
							{
								if(vty->res0)
									vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																		room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																		dbase->phonetab[i].user_id, VTY_NEWLINE);
								else
									vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
										room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
										dbase->phonetab[i].user_id, VTY_NEWLINE);
								cnt++;
							}
						}
					}
				}
				else if(room_number)
				{
					if(room_number == dbase->room_number)
					{
						memset(building_str, 0, sizeof(building_str));
						sprintf(building_str, "%d", dbase->building);
						memset(unit_str, 0, sizeof(unit_str));
						sprintf(unit_str, "%d", dbase->unit);
						for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
						{
							if(dbase->phonetab[i].use_flag == 0)
								continue;
							if (strlen(dbase->phonetab[i].phone) != 0)
							{
								if(vty->res0)
									vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																		room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																		dbase->phonetab[i].user_id, VTY_NEWLINE);
								else
									vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
										room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
										dbase->phonetab[i].user_id, VTY_NEWLINE);
								cnt++;
							}
						}
					}
				}
			}
			else
			{
				memset(building_str, 0, sizeof(building_str));
				sprintf(building_str, "%d", dbase->building);
				memset(unit_str, 0, sizeof(unit_str));
				sprintf(unit_str, "%d", dbase->unit);
				memset(room_number_str, 0, sizeof(room_number_str));
				sprintf(room_number_str, "%04d", dbase->room_number);
				for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
				{
					if(dbase->phonetab[i].use_flag == 0)
						continue;
					if (strlen(dbase->phonetab[i].phone) != 0)
					{
						if(vty->res0)
							vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																dbase->phonetab[i].user_id, VTY_NEWLINE);
						else
							vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
								room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
								dbase->phonetab[i].user_id, VTY_NEWLINE);
						cnt++;
					}
				}
			}
		}
	}
	return cnt;
}

static int voip_dbase_show_room_phone_3(struct vty *vty, u_int8 building, u_int8 unit, u_int16 room_number, char *username, char *user_id)
{
	int i =0, cnt = 0;
	NODE node;
	voip_dbase_t *dbase = NULL;
	s_int8 building_str[8], unit_str[8], room_number_str[8];
	char name[APP_USERNAME_MAX];
	char userid[APP_ID_MAX];
	memset(name, 0, sizeof(name));
	memset(userid, 0, sizeof(userid));

	if(username)
		strncpy(name, username, MIN(sizeof(name), strlen(username)));
	if(user_id)
		strncpy(userid, user_id, MIN(sizeof(name), strlen(user_id)));

	memset(building_str, 0, sizeof(building_str));
	memset(unit_str, 0, sizeof(unit_str));
	memset(room_number_str, 0, sizeof(room_number_str));

	sprintf(building_str, "%d", building);
	sprintf(unit_str, "%d", unit);
	sprintf(room_number_str, "%04d", room_number);

	for (dbase = (voip_dbase_t *) lstFirst(dbase_table);
			dbase != NULL; dbase = (voip_dbase_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			if(building || unit || room_number)
			{
				if(building)
				{
					if(building == dbase->building)
					{
						memset(unit_str, 0, sizeof(unit_str));
						sprintf(unit_str, "%d", dbase->unit);
						memset(room_number_str, 0, sizeof(room_number_str));
						sprintf(room_number_str, "%04d", dbase->room_number);
						for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
						{
							if(username && user_id)
							{
								if ( (memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0) &&
									(memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0) )
								{
									if(vty->res0)
										vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																			room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																			dbase->phonetab[i].user_id, VTY_NEWLINE);
									else
										vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
											room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
											dbase->phonetab[i].user_id, VTY_NEWLINE);
									cnt++;
								}
							}
							else if(username)
							{
								if (memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0)
								{
									if(vty->res0)
										vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																			room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																			dbase->phonetab[i].user_id, VTY_NEWLINE);
									else
										vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
											room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
											dbase->phonetab[i].user_id, VTY_NEWLINE);
									cnt++;
								}
							}
							else if(user_id)
							{
								if (memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0)
								{
									if(vty->res0)
										vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																			room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																			dbase->phonetab[i].user_id, VTY_NEWLINE);
									else
										vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
											room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
											dbase->phonetab[i].user_id, VTY_NEWLINE);
									cnt++;
								}
							}
						}
					}
				}
				else if(unit)
				{
					if(unit == dbase->unit)
					{
						memset(building_str, 0, sizeof(building_str));
						sprintf(building_str, "%d", dbase->building);
						memset(room_number_str, 0, sizeof(room_number_str));
						sprintf(room_number_str, "%04d", dbase->room_number);
						for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
						{
							if(username && user_id)
							{
								if ( (memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0) &&
									(memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0) )
								{
									if(vty->res0)
										vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																			room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																			dbase->phonetab[i].user_id, VTY_NEWLINE);
									else
										vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
											room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
											dbase->phonetab[i].user_id, VTY_NEWLINE);
									cnt++;
								}
							}
							else if(username)
							{
								if (memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0)
								{
									if(vty->res0)
										vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																			room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																			dbase->phonetab[i].user_id, VTY_NEWLINE);
									else
										vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
											room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
											dbase->phonetab[i].user_id, VTY_NEWLINE);
									cnt++;
								}
							}
							else if(user_id)
							{
								if (memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0)
								{
									if(vty->res0)
										vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																			room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																			dbase->phonetab[i].user_id, VTY_NEWLINE);
									else
										vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
											room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
											dbase->phonetab[i].user_id, VTY_NEWLINE);
									cnt++;
								}
							}
						}
					}
				}
				else if(room_number)
				{
					if(room_number == dbase->room_number)
					{
						memset(building_str, 0, sizeof(building_str));
						sprintf(building_str, "%d", dbase->building);
						memset(unit_str, 0, sizeof(unit_str));
						sprintf(unit_str, "%d", dbase->unit);
						for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
						{
							if(username && user_id)
							{
								if ( (memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0) &&
									(memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0) )
								{
									if(vty->res0)
										vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																			room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																			dbase->phonetab[i].user_id, VTY_NEWLINE);
									else
										vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
											room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
											dbase->phonetab[i].user_id, VTY_NEWLINE);
									cnt++;
								}
							}
							else if(username)
							{
								if (memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0)
								{
									if(vty->res0)
										vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																			room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																			dbase->phonetab[i].user_id, VTY_NEWLINE);
									else
										vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
											room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
											dbase->phonetab[i].user_id, VTY_NEWLINE);
									cnt++;
								}
							}
							else if(user_id)
							{
								if (memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0)
								{
									if(vty->res0)
										vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																			room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																			dbase->phonetab[i].user_id, VTY_NEWLINE);
									else
										vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
											room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
											dbase->phonetab[i].user_id, VTY_NEWLINE);
									cnt++;
								}
							}
						}
					}
				}
			}
			else
			{
				memset(building_str, 0, sizeof(building_str));
				sprintf(building_str, "%d", dbase->building);
				memset(unit_str, 0, sizeof(unit_str));
				sprintf(unit_str, "%d", dbase->unit);
				memset(room_number_str, 0, sizeof(room_number_str));
				sprintf(room_number_str, "%04d", dbase->room_number);
				for(i = 0; i < APP_MULTI_NUMBER_MAX; i++)
				{
					if(username && user_id)
					{
						if ( (memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0) &&
							(memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0) )
						{
							if(vty->res0)
								vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																	room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																	dbase->phonetab[i].user_id, VTY_NEWLINE);
							else
								vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
									room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
									dbase->phonetab[i].user_id, VTY_NEWLINE);
							cnt++;
						}
					}
					else if(username)
					{
						if (memcmp(dbase->phonetab[i].username, name, sizeof(name)) == 0)
						{
							if(vty->res0)
								vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																	room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																	dbase->phonetab[i].user_id, VTY_NEWLINE);
							else
								vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
									room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
									dbase->phonetab[i].user_id, VTY_NEWLINE);
							cnt++;
						}
					}
					else if(user_id)
					{
						if (memcmp(dbase->phonetab[i].user_id, userid, sizeof(userid)) == 0)
						{
							if(vty->res0)
								vty_out(vty, "%s %s %s %s %s %s%s", building_str, unit_str,
																	room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
																	dbase->phonetab[i].user_id, VTY_NEWLINE);
							else
								vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", building_str, unit_str,
									room_number_str, dbase->phonetab[i].phone, dbase->phonetab[i].username,
									dbase->phonetab[i].user_id, VTY_NEWLINE);
							cnt++;
						}
					}
				}
			}
		}
	}
	return cnt;
}

int voip_dbase_show_room_phone(struct vty *vty, u_int8 building, u_int8 unit, u_int16 room_number, char *username, char *user_id)
{
	int cnt = 0;
	if(dbase_mutex)
		os_mutex_lock(dbase_mutex, OS_WAIT_FOREVER);
	if(lstCount(dbase_table) && (!vty->res0)/* && vty->type != VTY_FILE*/)
	{
		vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", "--------", "--------", "--------",
				"----------------", "----------------", "----------------", VTY_NEWLINE);
		vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", "building", "unit", "room", "phone", "UserName", "User ID", VTY_NEWLINE);
		vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", "--------", "--------", "--------",
				"----------------", "----------------", "----------------", VTY_NEWLINE);
	}
	if(username || user_id)
	{
		cnt = voip_dbase_show_room_phone_3(vty,  building,  unit,  room_number, username, user_id);
	}
	else if((building && unit && room_number))
	{
		cnt = voip_dbase_show_room_phone_1(vty,  building,  unit,  room_number);
	}
	else
	{
		cnt = voip_dbase_show_room_phone_2(vty,  building,  unit,  room_number);
	}


	if(cnt && vty->type != VTY_FILE && (!vty->res0))
	{
		vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", "--------", "--------", "--------",
				"----------------", "----------------", "----------------", VTY_NEWLINE);
		vty_out(vty, " result : %d %s", cnt, VTY_NEWLINE);
		vty_out(vty, " %-8s %-8s %-8s %-16s %-16s %-16s %s", "--------", "--------", "--------",
				"----------------", "----------------", "----------------", VTY_NEWLINE);
	}
	if(lstCount(dbase_table))
		vty_out(vty, "%s", VTY_NEWLINE);
	if(dbase_mutex)
		os_mutex_unlock(dbase_mutex);
	return 0;
}


#ifdef PL_OPENWRT_UCI
static int voip_ubus_dbase_sync_one(BOOL badd)
{
	int	 ret = ERROR;
	u_int8 building = 0;
	u_int8 unit = 0;
	u_int16 room_number = 0;

	char 			phone[APP_ID_MAX];
	char 			username[APP_USERNAME_MAX];
	char 			user_id[APP_ID_MAX];

	memset(phone, 0, sizeof(phone));
	memset(username, 0, sizeof(username));
	memset(user_id, 0, sizeof(user_id));
/*
	ret |= os_uci_get_integer("dbase.roomdb.building", &building);

	ret |= os_uci_get_integer("dbase.roomdb.unit", &unit);
*/
	ret |= os_uci_get_string("dbase.roomdb.username", username);

	ret |= os_uci_get_string("dbase.roomdb.userid", user_id);

	ret |= os_uci_get_integer("dbase.roomdb.room", &room_number);

	ret |= os_uci_get_string("dbase.roomdb.phone", phone);
	if(badd)
	{
		if(strlen(phone) && strlen(username) && strlen(user_id))
		{
			if(voip_dbase_node_lookup_by_username(NULL, user_id))
			{
				zlog_err(MODULE_APP, "This User ID(%s) is already exist.", user_id);
				return ERROR;
			}
			//if(badd)
				voip_dbase_add_room_phone(building, unit, room_number, phone, username, user_id);
			//else
			//	voip_dbase_del_room_phone(building, unit, room_number, phone, username, user_id);
		}
	}
	else
	{
		if(strlen(phone) || strlen(username) || strlen(user_id))
			voip_dbase_del_room_phone(building, unit, room_number, strlen(phone)?phone:NULL, strlen(username)?username:NULL, strlen(user_id)?user_id:NULL);
	}
	//os_uci_set_string("dbase.roomdb.username", NULL);
	//os_uci_set_string("dbase.roomdb.user_id", NULL);
	//os_uci_set_integer("dbase.roomdb.room");
	os_uci_del("dbase", "roomdb", "username", NULL);
	os_uci_del("dbase", "roomdb", "userid", NULL);
	os_uci_del("dbase", "roomdb", "room", NULL);
	os_uci_del("dbase", "roomdb", "phone", NULL);
	//os_uci_set_string("dbase.roomdb.phone", NULL);
	os_uci_save_config("dbase");
	return OK;
}

/*static int voip_ubus_dbase_select_swap(char *input, char *output)
{
	int i = 0, j = 0, sph = 0, n = 0;
	FILE *fi, *fo;
	char ibuf[512];
	char obuf[512];
	fi = fopen(input, "r");
	fo = fopen(input, "w");
	if (fi && fo)
	{
		memset(ibuf, 0, sizeof(ibuf));
		memset(obuf, 0, sizeof(obuf));
		while (fgets(ibuf, sizeof(ibuf), fi))
		{
			if(strstr(ibuf, "------")||strstr(ibuf, "building"))
				continue;
			for (i = 0; i < strlen(ibuf); i++)
			{
				if(isspace(ibuf[i]))
				{
					if(sph == 0 && n == 1)
					{
						obuf[j++] = ibuf[i];
						sph = 1;
					}
				}
				else
				{
					obuf[j++] = ibuf[i];
					sph = 0;
					n = 1;
				}
			}
			obuf[j++]='\0';
			fputs(obuf, fo);
			fflush(fo);
			memset(ibuf, 0, sizeof(ibuf));
			memset(obuf, 0, sizeof(obuf));
		}
		fclose(fi);
		fclose(fo);
	}
	return OK;
}*/


static int voip_ubus_dbase_select_one(BOOL selsel)
{
	int	 ret = ERROR;
	u_int8 building = 0;
	u_int8 unit = 0;
	u_int16 room_number = 0;
	int fd;
	struct vty *file_vty = NULL;
	char 			username[APP_USERNAME_MAX];
	char 			user_id[APP_ID_MAX];
	remove("/tmp/app/tmp/search.txt");
	fd = open("/tmp/app/tmp/search.txt", O_WRONLY | O_CREAT, CONFIGFILE_MASK);
	if (fd < 0)
	{
		return ERROR;
	}

	memset(username, 0, sizeof(username));
	memset(user_id, 0, sizeof(user_id));
/*
	ret |= os_uci_get_integer("dbase.roomdb.building", &building);

	ret |= os_uci_get_integer("dbase.roomdb.unit", &unit);
*/
	ret |= os_uci_get_string("dbase.roomdb.username", username);

	ret |= os_uci_get_string("dbase.roomdb.userid", user_id);

	ret |= os_uci_get_integer("dbase.roomdb.room", &room_number);

	file_vty = vty_new();
	if(file_vty)
	{
		file_vty->fd = fd;
		file_vty->wfd = fd;
		file_vty->type = VTY_FILE;
		file_vty->fd_type = OS_STACK;
		file_vty->res0 = TRUE;
		if(selsel)
			voip_dbase_show_room_phone(file_vty,  building,  unit,  room_number,
					strlen(username)?username:NULL, strlen(user_id)?user_id:NULL);
		else
			voip_dbase_show_room_phone(file_vty,  building,  unit,  room_number,
					NULL, NULL);

		vty_close(file_vty);

/*		voip_ubus_dbase_select_swap("/tmp/app/tmp/search.tmp", "/tmp/app/tmp/search.txt");
		remove("/tmp/app/tmp/search.tmp");*/
		sync();
		return OK;
	}
	else
		remove("/tmp/app/tmp/search.txt");
	return OK;
}
#endif

int voip_ubus_dbase_sync(int cmd)
{
#ifdef PL_OPENWRT_UCI
	if(cmd == 1)
	{
		voip_ubus_dbase_sync_one(TRUE);
		voip_ubus_dbase_select_one(FALSE);
	}
	else if(cmd == -1)
	{
		voip_ubus_dbase_sync_one(FALSE);
		voip_ubus_dbase_select_one(FALSE);
	}
	else if(cmd == 2)
		voip_ubus_dbase_select_one(TRUE);
#endif
	return OK;
}

#endif

