/*
 * voip_buddy.c
 *
 *  Created on: 2019年10月19日
 *      Author: zhurish
 */

#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"


#include "voip_buddy.h"


static LIST *buddy_table = NULL;
static os_mutex_t *buddy_mutex = NULL;

static int buddy_dbase_load_file(void);

int buddy_dbase_clean(void)
{
	NODE node;
	buddy_user_t *dbtest = NULL;
	if(buddy_mutex)
		os_mutex_lock(buddy_mutex, OS_WAIT_FOREVER);
	for (dbtest = (buddy_user_t *) lstFirst(buddy_table);
			dbtest != NULL; dbtest = (buddy_user_t *) lstNext(&node))
	{
		node = dbtest->node;
		if (dbtest)
		{
			lstDelete(buddy_table, (NODE *) dbtest);
			XFREE(MTYPE_VOIP_DBTEST, dbtest);
		}
	}
#ifdef BUDDY_DBASE_FILE
	remove(BUDDY_DBASE_FILE);
#endif
	sync();
	if(buddy_mutex)
		os_mutex_unlock(buddy_mutex);
	return OK;
}

int buddy_dbase_exit()
{
	buddy_dbase_clean();
	if(buddy_mutex)
	{
		os_mutex_lock(buddy_mutex, OS_WAIT_FOREVER);
		if(os_mutex_exit(buddy_mutex)==OK)
			buddy_mutex = NULL;
	}
	if(buddy_table)
	{
		XFREE(MTYPE_VOIP_TOP, buddy_table);
		buddy_table = NULL;
	}
	return OK;
}




int buddy_dbase_load()
{
	if (buddy_table == NULL)
	{
		buddy_table = XMALLOC(MTYPE_VOIP_TOP, sizeof(LIST));
		if (buddy_table)
		{
			if(buddy_mutex == NULL)
				buddy_mutex = os_mutex_init();
			lstInit(buddy_table);
			if(buddy_mutex)
				os_mutex_lock(buddy_mutex, OS_WAIT_FOREVER);
			buddy_dbase_load_file();
			if(buddy_mutex)
				os_mutex_unlock(buddy_mutex);
			return OK;
		}
		return ERROR;
	}
	return OK;
}

#ifdef BUDDY_DBASE_FILE
static int buddy_dbase_read_one(int fd, buddy_user_t *node)
{
	buddy_user_t *addnode = NULL;
	if(read(fd, node, sizeof(buddy_user_t)) == sizeof(buddy_user_t))
	{
		addnode = XMALLOC(MTYPE_VOIP_DBTEST, sizeof(buddy_user_t));
		if(!addnode)
			return ERROR;
		memset(addnode, 0, sizeof(buddy_user_t));
		memcpy(addnode, node, sizeof(buddy_user_t));
		lstAdd(buddy_table, (NODE *) addnode);
		return OK;
	}
	return ERROR;
}
#endif

static int buddy_dbase_load_file(void)
{
#ifdef BUDDY_DBASE_FILE
	int ret = OK, fd = 0;
	buddy_user_t dbase;
	if (buddy_table == NULL)
		return ERROR;
	if(ret == 0)
	{
		fd = open(BUDDY_DBASE_FILE, O_RDONLY);
		if(fd <= 0)
		{
			return ERROR;
		}
		while(ret == OK)
		{
			memset(&dbase, 0, sizeof(buddy_user_t));
			ret = buddy_dbase_read_one(fd, &dbase);
		}
		close(fd);
		return OK;
	}
	return ERROR;
#else
	return OK;
#endif
}

#ifdef BUDDY_DBASE_FILE
static int buddy_dbase_write_list(int fd)
{
	int ret = 0;
	NODE node;
	buddy_user_t *dbase = NULL;
	for (dbase = (buddy_user_t *) lstFirst(buddy_table);
			dbase != NULL; dbase = (buddy_user_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			ret = write(fd, dbase, sizeof(buddy_user_t));
			if(ret != sizeof(buddy_user_t))
				break;
		}
	}
	return ret;
}
#endif

static int buddy_dbase_update_save(void)
{
#ifdef BUDDY_DBASE_FILE
	int ret = 0, fd = 0;
	if (buddy_table == NULL)
		return ERROR;
	if(lstCount(buddy_table) == 0)
	{
		if(access(BUDDY_DBASE_FILE, F_OK) == 0)
			remove(BUDDY_DBASE_FILE);
		sync();
		return OK;
	}
	if(ret == 0)
	{
		char tmp[4];
		fd = open(BUDDY_DBASE_FILE".tmp", O_RDWR|O_CREAT, 0644);
		if(fd <= 0)
		{
			return ERROR;
		}
		if(buddy_dbase_write_list(fd) != sizeof(buddy_user_t))
		{
			close(fd);
			remove(BUDDY_DBASE_FILE".tmp");
			sync();
			return ERROR;
		}
		close(fd);
		rename(BUDDY_DBASE_FILE".tmp", BUDDY_DBASE_FILE);
		sync();
		return OK;
	}
	return ERROR;
#else
	return OK;
#endif
}



/***********************************************/
static buddy_user_t * buddy_dbase_node_lookup_by_username(char *username)
{
	int i = 0;
	char name[BUDDY_USERNAME_MAX];
	NODE node;
	buddy_user_t *dbase = NULL;
	if (buddy_table == NULL)
		return NULL;
	memset(name, 0, sizeof(name));
	if(username)
		strncpy(name, username, MIN(sizeof(name), strlen(username)));
	if(buddy_mutex)
		os_mutex_lock(buddy_mutex, OS_WAIT_FOREVER);
	for (dbase = (buddy_user_t *) lstFirst(buddy_table);
			dbase != NULL; dbase = (buddy_user_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
			{
				if(dbase->buddy_phone[i].active == 0)
					continue;
				if(username)
				{
					if( (memcmp(dbase->username, name, sizeof(name)) == 0))
					{
						if(buddy_mutex)
							os_mutex_unlock(buddy_mutex);
						return dbase;
					}
				}
			}
		}
	}
	if(buddy_mutex)
		os_mutex_unlock(buddy_mutex);
	return NULL;
}

buddy_user_t * buddy_dbase_node_lookup_by_phonenumber(char *phone)
{
	int i = 0;
	char lphone[BUDDY_USERNAME_MAX];
	NODE node;
	buddy_user_t *dbase = NULL;
	if (buddy_table == NULL)
		return NULL;
	zassert(phone != NULL);
	memset(lphone, 0, sizeof(lphone));
	if(phone)
		strncpy(lphone, phone, MIN(sizeof(lphone), strlen(phone)));
	if(buddy_mutex)
		os_mutex_lock(buddy_mutex, OS_WAIT_FOREVER);
	for (dbase = (buddy_user_t *) lstFirst(buddy_table);
			dbase != NULL; dbase = (buddy_user_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
			{
				if(dbase->buddy_phone[i].active == 0)
					continue;
				if( (memcmp(dbase->buddy_phone[i].phone, lphone, sizeof(lphone)) == 0))
				{
					if(buddy_mutex)
						os_mutex_unlock(buddy_mutex);
					return dbase;
				}
			}
		}
	}
	if(buddy_mutex)
		os_mutex_unlock(buddy_mutex);
	return NULL;
}

buddy_user_t * buddy_dbase_node_lookup_by_private_ID(int (*pri_cmp)(void *p1, void *p2), void *p2)
{
	int i = 0;
	NODE node;
	buddy_user_t *dbase = NULL;
	if (buddy_table == NULL)
		return NULL;
	if(buddy_mutex)
		os_mutex_lock(buddy_mutex, OS_WAIT_FOREVER);
	for (dbase = (buddy_user_t *) lstFirst(buddy_table);
			dbase != NULL; dbase = (buddy_user_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase && pri_cmp)
		{
			if(((pri_cmp)(dbase->pVoid, p2)) == 0)
			{
				if(buddy_mutex)
					os_mutex_unlock(buddy_mutex);
				return dbase;
			}
		}
	}
	if(buddy_mutex)
		os_mutex_unlock(buddy_mutex);
	return NULL;
}


buddy_user_t * buddy_dbase_lookup_by_username(char *username)
{
	return buddy_dbase_node_lookup_by_username(username);
}

int buddy_dbase_username_add(char *username, zpl_uint32	userid)
{
	buddy_user_t *user = buddy_dbase_node_lookup_by_username(username);
	if(user)
		return ERROR;
	user = XMALLOC(MTYPE_VOIP_DBTEST, sizeof(buddy_user_t));
	memset(user, 0, sizeof(buddy_user_t));

	memcpy(user->username, username, MIN(strlen(username),sizeof(user->username)));
	//user->userid;
	lstAdd(buddy_table, (NODE *) user);
	return OK;
}

int buddy_dbase_username_del(char *username, zpl_uint32 userid)
{
	buddy_user_t *user = buddy_dbase_node_lookup_by_username(username);
	if(!user)
		return ERROR;
	lstDelete(buddy_table, (NODE *) user);
	return OK;
}
/*******************************************************************************/
/*******************************************************************************/
static int _buddy_dbase_username_lookup_phone_num(buddy_user_t *user, char *phone)
{
	int i = 0;
	char lphone[BUDDY_USERNAME_MAX];
	memset(lphone, 0, sizeof(lphone));
	if(phone)
		strncpy(lphone, phone, MIN(sizeof(lphone), strlen(phone)));
	for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
	{
		if(user->buddy_phone[i].active == 0)
			continue;
		if( (memcmp(user->buddy_phone[i].phone, lphone, sizeof(lphone)) == 0))
		{
			return i;
		}
	}
	return ERROR;
}

static int _buddy_dbase_username_add_phone_num(buddy_user_t *user, char *phone)
{
	int i = 0;
	if(_buddy_dbase_username_lookup_phone_num(user, phone) == ERROR)
		return ERROR;
	for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
	{
		if(user->buddy_phone[i].active == 0)
		{
			user->buddy_phone[i].active = 1;
			strncpy(user->buddy_phone[i].phone, phone, MIN(BUDDY_PHONE_MAX, strlen(phone)));
			return OK;
		}
	}
	return ERROR;
}

static int _buddy_dbase_username_del_phone_num(buddy_user_t *user, char *phone)
{
	int i = 0;
	char lphone[BUDDY_USERNAME_MAX];
	memset(lphone, 0, sizeof(lphone));
	if(phone)
		strncpy(lphone, phone, MIN(sizeof(lphone), strlen(phone)));
	for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
	{
		if(user->buddy_phone[i].active == 0)
			continue;
		if( (memcmp(user->buddy_phone[i].phone, lphone, sizeof(lphone)) == 0))
		{
			user->buddy_phone[i].active = 0;
			memset(user->buddy_phone[i].phone, 0, BUDDY_PHONE_MAX);
			return OK;
		}
	}
	return ERROR;
}

int buddy_dbase_username_add_phone(char *username, char *phone)
{
	buddy_user_t *user = buddy_dbase_node_lookup_by_username(username);
	if(!user)
		return ERROR;
	return _buddy_dbase_username_add_phone_num(user, phone);
}

int buddy_dbase_username_del_phone(char *username, char *phone)
{
	buddy_user_t *user = buddy_dbase_node_lookup_by_username(username);
	if(!user)
		return ERROR;
	return _buddy_dbase_username_del_phone_num(user, phone);
}
