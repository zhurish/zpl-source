/*
 * pjapp_buddy.c
 *
 *  Created on: 2019年10月19日
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "vty_include.h"
#include <pjsua-lib/pjsua.h>
#include "pjsua_app_config.h"
#include "pjsua_app_cb.h"
#include "pjsua_app_common.h"
#include "pjsua_app_cfgapi.h"
#include "pjsip_buddy.h"


static LIST *pjapp_buddy_table = NULL;
static os_mutex_t *pjapp_buddy_mutex = NULL;

static int pjapp_buddy_load_file(void);

int pjapp_buddy_clean(void)
{
	NODE node;
	pjapp_buddy_t *dbtest = NULL;
	if(pjapp_buddy_mutex)
		os_mutex_lock(pjapp_buddy_mutex, OS_WAIT_FOREVER);
	for (dbtest = (pjapp_buddy_t *) lstFirst(pjapp_buddy_table);
			dbtest != NULL; dbtest = (pjapp_buddy_t *) lstNext(&node))
	{
		node = dbtest->node;
		if (dbtest)
		{
			lstDelete(pjapp_buddy_table, (NODE *) dbtest);
			XFREE(MTYPE_VOIP_DBTEST, dbtest);
		}
	}
#ifdef BUDDY_DBASE_FILE
	remove(BUDDY_DBASE_FILE);
#endif
	sync();
	if(pjapp_buddy_mutex)
		os_mutex_unlock(pjapp_buddy_mutex);
	return PJ_SUCCESS;
}

int pjapp_buddy_exit(void)
{
	pjapp_buddy_clean();
	if(pjapp_buddy_mutex)
	{
		os_mutex_lock(pjapp_buddy_mutex, OS_WAIT_FOREVER);
		if(os_mutex_destroy(pjapp_buddy_mutex)==PJ_SUCCESS)
			pjapp_buddy_mutex = NULL;
	}
	if(pjapp_buddy_table)
	{
		XFREE(MTYPE_VOIP_TOP, pjapp_buddy_table);
		pjapp_buddy_table = NULL;
	}
	return PJ_SUCCESS;
}




int pjapp_buddy_load(void)
{
	if (pjapp_buddy_table == NULL)
	{
		pjapp_buddy_table = XMALLOC(MTYPE_VOIP_TOP, sizeof(LIST));
		if (pjapp_buddy_table)
		{
			if(pjapp_buddy_mutex == NULL)
				pjapp_buddy_mutex = os_mutex_name_create("pjapp_buddy_mutex");
			lstInit(pjapp_buddy_table);
			if(pjapp_buddy_mutex)
				os_mutex_lock(pjapp_buddy_mutex, OS_WAIT_FOREVER);
			pjapp_buddy_load_file();
			if(pjapp_buddy_mutex)
				os_mutex_unlock(pjapp_buddy_mutex);
			return PJ_SUCCESS;
		}
		return ERROR;
	}
	return PJ_SUCCESS;
}

#ifdef BUDDY_DBASE_FILE
static int pjapp_buddy_read_one(int fd, pjapp_buddy_t *node)
{
	pjapp_buddy_t *addnode = NULL;
	if(read(fd, node, sizeof(pjapp_buddy_t)) == sizeof(pjapp_buddy_t))
	{
		addnode = XMALLOC(MTYPE_VOIP_DBTEST, sizeof(pjapp_buddy_t));
		if(!addnode)
			return ERROR;
		memset(addnode, 0, sizeof(pjapp_buddy_t));
		memcpy(addnode, node, sizeof(pjapp_buddy_t));
		lstAdd(pjapp_buddy_table, (NODE *) addnode);
		return PJ_SUCCESS;
	}
	return ERROR;
}
#endif

static int pjapp_buddy_load_file(void)
{
#ifdef BUDDY_DBASE_FILE
	int ret = PJ_SUCCESS, fd = 0;
	pjapp_buddy_t dbase;
	if (pjapp_buddy_table == NULL)
		return ERROR;
	if(ret == 0)
	{
		fd = open(BUDDY_DBASE_FILE, O_RDONLY);
		if(fd <= 0)
		{
			return ERROR;
		}
		while(ret == PJ_SUCCESS)
		{
			memset(&dbase, 0, sizeof(pjapp_buddy_t));
			ret = pjapp_buddy_read_one(fd, &dbase);
		}
		close(fd);
		return PJ_SUCCESS;
	}
	return ERROR;
#else
	return PJ_SUCCESS;
#endif
}

#ifdef BUDDY_DBASE_FILE
static int pjapp_buddy_write_list(int fd)
{
	int ret = 0;
	NODE node;
	pjapp_buddy_t *dbase = NULL;
	for (dbase = (pjapp_buddy_t *) lstFirst(pjapp_buddy_table);
			dbase != NULL; dbase = (pjapp_buddy_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			ret = write(fd, dbase, sizeof(pjapp_buddy_t));
			if(ret != sizeof(pjapp_buddy_t))
				break;
		}
	}
	return ret;
}
#endif

int pjapp_buddy_update_save(void)
{
#ifdef BUDDY_DBASE_FILE
	int ret = 0, fd = 0;
	if (pjapp_buddy_table == NULL)
		return ERROR;
	if(lstCount(pjapp_buddy_table) == 0)
	{
		if(access(BUDDY_DBASE_FILE, F_OK) == 0)
			remove(BUDDY_DBASE_FILE);
		sync();
		return PJ_SUCCESS;
	}
	if(ret == 0)
	{
		///char tmp[4];
		fd = open(BUDDY_DBASE_FILE".tmp", O_RDWR|O_CREAT, 0644);
		if(fd <= 0)
		{
			return ERROR;
		}
		if(pjapp_buddy_write_list(fd) != sizeof(pjapp_buddy_t))
		{
			close(fd);
			remove(BUDDY_DBASE_FILE".tmp");
			sync();
			return ERROR;
		}
		close(fd);
		rename(BUDDY_DBASE_FILE".tmp", BUDDY_DBASE_FILE);
		sync();
		return PJ_SUCCESS;
	}
	return ERROR;
#else
	return PJ_SUCCESS;
#endif
}



/***********************************************/
static pjapp_buddy_t * pjapp_buddy_node_lookup_by_username(char *username)
{
	int i = 0;
	char name[BUDDY_USERNAME_MAX];
	NODE node;
	pjapp_buddy_t *dbase = NULL;
	if (pjapp_buddy_table == NULL)
		return NULL;
	memset(name, 0, sizeof(name));
	if(username)
		strncpy(name, username, MIN(sizeof(name), strlen(username)));
	if(pjapp_buddy_mutex)
		os_mutex_lock(pjapp_buddy_mutex, OS_WAIT_FOREVER);
	for (dbase = (pjapp_buddy_t *) lstFirst(pjapp_buddy_table);
			dbase != NULL; dbase = (pjapp_buddy_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
			{
				if(dbase->pjapp_buddy_phone[i].active == 0)
					continue;
				if(username)
				{
					if( (memcmp(dbase->username, name, sizeof(name)) == 0))
					{
						if(pjapp_buddy_mutex)
							os_mutex_unlock(pjapp_buddy_mutex);
						return dbase;
					}
				}
			}
		}
	}
	if(pjapp_buddy_mutex)
		os_mutex_unlock(pjapp_buddy_mutex);
	return NULL;
}

pjapp_buddy_t * pjapp_buddy_node_lookup_by_phonenumber(char *phone)
{
	int i = 0;
	char lphone[BUDDY_USERNAME_MAX];
	NODE node;
	pjapp_buddy_t *dbase = NULL;
	if (pjapp_buddy_table == NULL)
		return NULL;
	zassert(phone != NULL);
	memset(lphone, 0, sizeof(lphone));
	if(phone)
		strncpy(lphone, phone, MIN(sizeof(lphone), strlen(phone)));
	if(pjapp_buddy_mutex)
		os_mutex_lock(pjapp_buddy_mutex, OS_WAIT_FOREVER);
	for (dbase = (pjapp_buddy_t *) lstFirst(pjapp_buddy_table);
			dbase != NULL; dbase = (pjapp_buddy_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
			{
				if(dbase->pjapp_buddy_phone[i].active == 0)
					continue;
				if( (memcmp(dbase->pjapp_buddy_phone[i].phone, lphone, sizeof(lphone)) == 0))
				{
					if(pjapp_buddy_mutex)
						os_mutex_unlock(pjapp_buddy_mutex);
					return dbase;
				}
			}
		}
	}
	if(pjapp_buddy_mutex)
		os_mutex_unlock(pjapp_buddy_mutex);
	return NULL;
}

pjapp_buddy_t * pjapp_buddy_node_lookup_by_private_ID(int (*pri_cmp)(void *p1, void *p2), void *p2)
{
	//int i = 0;
	NODE node;
	pjapp_buddy_t *dbase = NULL;
	if (pjapp_buddy_table == NULL)
		return NULL;
	if(pjapp_buddy_mutex)
		os_mutex_lock(pjapp_buddy_mutex, OS_WAIT_FOREVER);
	for (dbase = (pjapp_buddy_t *) lstFirst(pjapp_buddy_table);
			dbase != NULL; dbase = (pjapp_buddy_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase && pri_cmp)
		{
			if(((pri_cmp)(dbase->pVoid, p2)) == 0)
			{
				if(pjapp_buddy_mutex)
					os_mutex_unlock(pjapp_buddy_mutex);
				return dbase;
			}
		}
	}
	if(pjapp_buddy_mutex)
		os_mutex_unlock(pjapp_buddy_mutex);
	return NULL;
}


pjapp_buddy_t * pjapp_buddy_lookup_by_username(char *username)
{
	return pjapp_buddy_node_lookup_by_username(username);
}

int pjapp_buddy_username_add(char *username, pj_uint32_t	userid)
{
	pjapp_buddy_t *user = pjapp_buddy_node_lookup_by_username(username);
	if(user)
		return ERROR;
	user = XMALLOC(MTYPE_VOIP_DBTEST, sizeof(pjapp_buddy_t));
	memset(user, 0, sizeof(pjapp_buddy_t));

	memcpy(user->username, username, MIN(strlen(username),sizeof(user->username)));
	//user->userid;
	lstAdd(pjapp_buddy_table, (NODE *) user);
	return PJ_SUCCESS;
}

int pjapp_buddy_username_del(char *username, pj_uint32_t userid)
{
	pjapp_buddy_t *user = pjapp_buddy_node_lookup_by_username(username);
	if(!user)
		return ERROR;
	lstDelete(pjapp_buddy_table, (NODE *) user);
	return PJ_SUCCESS;
}
/*******************************************************************************/
/*******************************************************************************/
static int _pjapp_buddy_username_lookup_phone_num(pjapp_buddy_t *user, char *phone)
{
	int i = 0;
	char lphone[BUDDY_USERNAME_MAX];
	memset(lphone, 0, sizeof(lphone));
	if(phone)
		strncpy(lphone, phone, MIN(sizeof(lphone), strlen(phone)));
	for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
	{
		if(user->pjapp_buddy_phone[i].active == 0)
			continue;
		if( (memcmp(user->pjapp_buddy_phone[i].phone, lphone, sizeof(lphone)) == 0))
		{
			return i;
		}
	}
	return ERROR;
}

static int _pjapp_buddy_username_add_phone_num(pjapp_buddy_t *user, char *phone)
{
	int i = 0;
	if(_pjapp_buddy_username_lookup_phone_num(user, phone) == ERROR)
		return ERROR;
	for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
	{
		if(user->pjapp_buddy_phone[i].active == 0)
		{
			user->pjapp_buddy_phone[i].active = 1;
			strncpy(user->pjapp_buddy_phone[i].phone, phone, MIN(BUDDY_PHONE_MAX, strlen(phone)));
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

static int _pjapp_buddy_username_del_phone_num(pjapp_buddy_t *user, char *phone)
{
	int i = 0;
	char lphone[BUDDY_USERNAME_MAX];
	memset(lphone, 0, sizeof(lphone));
	if(phone)
		strncpy(lphone, phone, MIN(sizeof(lphone), strlen(phone)));
	for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
	{
		if(user->pjapp_buddy_phone[i].active == 0)
			continue;
		if( (memcmp(user->pjapp_buddy_phone[i].phone, lphone, sizeof(lphone)) == 0))
		{
			user->pjapp_buddy_phone[i].active = 0;
			memset(user->pjapp_buddy_phone[i].phone, 0, BUDDY_PHONE_MAX);
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

int pjapp_buddy_username_add_phone(char *username, char *phone)
{
	pjapp_buddy_t *user = pjapp_buddy_node_lookup_by_username(username);
	if(!user)
		return ERROR;
	return _pjapp_buddy_username_add_phone_num(user, phone);
}

int pjapp_buddy_username_del_phone(char *username, char *phone)
{
	pjapp_buddy_t *user = pjapp_buddy_node_lookup_by_username(username);
	if(!user)
		return ERROR;
	return _pjapp_buddy_username_del_phone_num(user, phone);
}
