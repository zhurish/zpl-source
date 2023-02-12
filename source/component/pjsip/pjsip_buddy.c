/*
 * pjsip_buddy.c
 *
 *  Created on: 2019年10月19日
 *      Author: zhurish
 */

#include "auto_include.h"
#include <zplos_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"


#include "pjsip_buddy.h"


static LIST *pjsip_buddy_table = NULL;
static os_mutex_t *pjsip_buddy_mutex = NULL;

static int pjsip_buddy_load_file(void);

int pjsip_buddy_clean(void)
{
	NODE node;
	pjsip_buddy_t *dbtest = NULL;
	if(pjsip_buddy_mutex)
		os_mutex_lock(pjsip_buddy_mutex, OS_WAIT_FOREVER);
	for (dbtest = (pjsip_buddy_t *) lstFirst(pjsip_buddy_table);
			dbtest != NULL; dbtest = (pjsip_buddy_t *) lstNext(&node))
	{
		node = dbtest->node;
		if (dbtest)
		{
			lstDelete(pjsip_buddy_table, (NODE *) dbtest);
			XFREE(MTYPE_VOIP_DBTEST, dbtest);
		}
	}
#ifdef BUDDY_DBASE_FILE
	remove(BUDDY_DBASE_FILE);
#endif
	sync();
	if(pjsip_buddy_mutex)
		os_mutex_unlock(pjsip_buddy_mutex);
	return OK;
}

int pjsip_buddy_exit()
{
	pjsip_buddy_clean();
	if(pjsip_buddy_mutex)
	{
		os_mutex_lock(pjsip_buddy_mutex, OS_WAIT_FOREVER);
		if(os_mutex_destroy(pjsip_buddy_mutex)==OK)
			pjsip_buddy_mutex = NULL;
	}
	if(pjsip_buddy_table)
	{
		XFREE(MTYPE_VOIP_TOP, pjsip_buddy_table);
		pjsip_buddy_table = NULL;
	}
	return OK;
}




int pjsip_buddy_load()
{
	if (pjsip_buddy_table == NULL)
	{
		pjsip_buddy_table = XMALLOC(MTYPE_VOIP_TOP, sizeof(LIST));
		if (pjsip_buddy_table)
		{
			if(pjsip_buddy_mutex == NULL)
				pjsip_buddy_mutex = os_mutex_name_create("pjsip_buddy_mutex");
			lstInit(pjsip_buddy_table);
			if(pjsip_buddy_mutex)
				os_mutex_lock(pjsip_buddy_mutex, OS_WAIT_FOREVER);
			pjsip_buddy_load_file();
			if(pjsip_buddy_mutex)
				os_mutex_unlock(pjsip_buddy_mutex);
			return OK;
		}
		return ERROR;
	}
	return OK;
}

#ifdef BUDDY_DBASE_FILE
static int pjsip_buddy_read_one(int fd, pjsip_buddy_t *node)
{
	pjsip_buddy_t *addnode = NULL;
	if(read(fd, node, sizeof(pjsip_buddy_t)) == sizeof(pjsip_buddy_t))
	{
		addnode = XMALLOC(MTYPE_VOIP_DBTEST, sizeof(pjsip_buddy_t));
		if(!addnode)
			return ERROR;
		memset(addnode, 0, sizeof(pjsip_buddy_t));
		memcpy(addnode, node, sizeof(pjsip_buddy_t));
		lstAdd(pjsip_buddy_table, (NODE *) addnode);
		return OK;
	}
	return ERROR;
}
#endif

static int pjsip_buddy_load_file(void)
{
#ifdef BUDDY_DBASE_FILE
	int ret = OK, fd = 0;
	pjsip_buddy_t dbase;
	if (pjsip_buddy_table == NULL)
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
			memset(&dbase, 0, sizeof(pjsip_buddy_t));
			ret = pjsip_buddy_read_one(fd, &dbase);
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
static int pjsip_buddy_write_list(int fd)
{
	int ret = 0;
	NODE node;
	pjsip_buddy_t *dbase = NULL;
	for (dbase = (pjsip_buddy_t *) lstFirst(pjsip_buddy_table);
			dbase != NULL; dbase = (pjsip_buddy_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			ret = write(fd, dbase, sizeof(pjsip_buddy_t));
			if(ret != sizeof(pjsip_buddy_t))
				break;
		}
	}
	return ret;
}
#endif

int pjsip_buddy_update_save(void)
{
#ifdef BUDDY_DBASE_FILE
	int ret = 0, fd = 0;
	if (pjsip_buddy_table == NULL)
		return ERROR;
	if(lstCount(pjsip_buddy_table) == 0)
	{
		if(access(BUDDY_DBASE_FILE, F_OK) == 0)
			remove(BUDDY_DBASE_FILE);
		sync();
		return OK;
	}
	if(ret == 0)
	{
		///char tmp[4];
		fd = open(BUDDY_DBASE_FILE".tmp", O_RDWR|O_CREAT, 0644);
		if(fd <= 0)
		{
			return ERROR;
		}
		if(pjsip_buddy_write_list(fd) != sizeof(pjsip_buddy_t))
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
static pjsip_buddy_t * pjsip_buddy_node_lookup_by_username(char *username)
{
	int i = 0;
	char name[BUDDY_USERNAME_MAX];
	NODE node;
	pjsip_buddy_t *dbase = NULL;
	if (pjsip_buddy_table == NULL)
		return NULL;
	memset(name, 0, sizeof(name));
	if(username)
		strncpy(name, username, MIN(sizeof(name), strlen(username)));
	if(pjsip_buddy_mutex)
		os_mutex_lock(pjsip_buddy_mutex, OS_WAIT_FOREVER);
	for (dbase = (pjsip_buddy_t *) lstFirst(pjsip_buddy_table);
			dbase != NULL; dbase = (pjsip_buddy_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
			{
				if(dbase->pjsip_buddy_phone[i].active == 0)
					continue;
				if(username)
				{
					if( (memcmp(dbase->username, name, sizeof(name)) == 0))
					{
						if(pjsip_buddy_mutex)
							os_mutex_unlock(pjsip_buddy_mutex);
						return dbase;
					}
				}
			}
		}
	}
	if(pjsip_buddy_mutex)
		os_mutex_unlock(pjsip_buddy_mutex);
	return NULL;
}

pjsip_buddy_t * pjsip_buddy_node_lookup_by_phonenumber(char *phone)
{
	int i = 0;
	char lphone[BUDDY_USERNAME_MAX];
	NODE node;
	pjsip_buddy_t *dbase = NULL;
	if (pjsip_buddy_table == NULL)
		return NULL;
	zassert(phone != NULL);
	memset(lphone, 0, sizeof(lphone));
	if(phone)
		strncpy(lphone, phone, MIN(sizeof(lphone), strlen(phone)));
	if(pjsip_buddy_mutex)
		os_mutex_lock(pjsip_buddy_mutex, OS_WAIT_FOREVER);
	for (dbase = (pjsip_buddy_t *) lstFirst(pjsip_buddy_table);
			dbase != NULL; dbase = (pjsip_buddy_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
			{
				if(dbase->pjsip_buddy_phone[i].active == 0)
					continue;
				if( (memcmp(dbase->pjsip_buddy_phone[i].phone, lphone, sizeof(lphone)) == 0))
				{
					if(pjsip_buddy_mutex)
						os_mutex_unlock(pjsip_buddy_mutex);
					return dbase;
				}
			}
		}
	}
	if(pjsip_buddy_mutex)
		os_mutex_unlock(pjsip_buddy_mutex);
	return NULL;
}

pjsip_buddy_t * pjsip_buddy_node_lookup_by_private_ID(int (*pri_cmp)(void *p1, void *p2), void *p2)
{
	//int i = 0;
	NODE node;
	pjsip_buddy_t *dbase = NULL;
	if (pjsip_buddy_table == NULL)
		return NULL;
	if(pjsip_buddy_mutex)
		os_mutex_lock(pjsip_buddy_mutex, OS_WAIT_FOREVER);
	for (dbase = (pjsip_buddy_t *) lstFirst(pjsip_buddy_table);
			dbase != NULL; dbase = (pjsip_buddy_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase && pri_cmp)
		{
			if(((pri_cmp)(dbase->pVoid, p2)) == 0)
			{
				if(pjsip_buddy_mutex)
					os_mutex_unlock(pjsip_buddy_mutex);
				return dbase;
			}
		}
	}
	if(pjsip_buddy_mutex)
		os_mutex_unlock(pjsip_buddy_mutex);
	return NULL;
}


pjsip_buddy_t * pjsip_buddy_lookup_by_username(char *username)
{
	return pjsip_buddy_node_lookup_by_username(username);
}

int pjsip_buddy_username_add(char *username, zpl_uint32	userid)
{
	pjsip_buddy_t *user = pjsip_buddy_node_lookup_by_username(username);
	if(user)
		return ERROR;
	user = XMALLOC(MTYPE_VOIP_DBTEST, sizeof(pjsip_buddy_t));
	memset(user, 0, sizeof(pjsip_buddy_t));

	memcpy(user->username, username, MIN(strlen(username),sizeof(user->username)));
	//user->userid;
	lstAdd(pjsip_buddy_table, (NODE *) user);
	return OK;
}

int pjsip_buddy_username_del(char *username, zpl_uint32 userid)
{
	pjsip_buddy_t *user = pjsip_buddy_node_lookup_by_username(username);
	if(!user)
		return ERROR;
	lstDelete(pjsip_buddy_table, (NODE *) user);
	return OK;
}
/*******************************************************************************/
/*******************************************************************************/
static int _pjsip_buddy_username_lookup_phone_num(pjsip_buddy_t *user, char *phone)
{
	int i = 0;
	char lphone[BUDDY_USERNAME_MAX];
	memset(lphone, 0, sizeof(lphone));
	if(phone)
		strncpy(lphone, phone, MIN(sizeof(lphone), strlen(phone)));
	for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
	{
		if(user->pjsip_buddy_phone[i].active == 0)
			continue;
		if( (memcmp(user->pjsip_buddy_phone[i].phone, lphone, sizeof(lphone)) == 0))
		{
			return i;
		}
	}
	return ERROR;
}

static int _pjsip_buddy_username_add_phone_num(pjsip_buddy_t *user, char *phone)
{
	int i = 0;
	if(_pjsip_buddy_username_lookup_phone_num(user, phone) == ERROR)
		return ERROR;
	for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
	{
		if(user->pjsip_buddy_phone[i].active == 0)
		{
			user->pjsip_buddy_phone[i].active = 1;
			strncpy(user->pjsip_buddy_phone[i].phone, phone, MIN(BUDDY_PHONE_MAX, strlen(phone)));
			return OK;
		}
	}
	return ERROR;
}

static int _pjsip_buddy_username_del_phone_num(pjsip_buddy_t *user, char *phone)
{
	int i = 0;
	char lphone[BUDDY_USERNAME_MAX];
	memset(lphone, 0, sizeof(lphone));
	if(phone)
		strncpy(lphone, phone, MIN(sizeof(lphone), strlen(phone)));
	for(i = 0; i < BUDDY_MULTI_NUMBER_MAX; i++)
	{
		if(user->pjsip_buddy_phone[i].active == 0)
			continue;
		if( (memcmp(user->pjsip_buddy_phone[i].phone, lphone, sizeof(lphone)) == 0))
		{
			user->pjsip_buddy_phone[i].active = 0;
			memset(user->pjsip_buddy_phone[i].phone, 0, BUDDY_PHONE_MAX);
			return OK;
		}
	}
	return ERROR;
}

int pjsip_buddy_username_add_phone(char *username, char *phone)
{
	pjsip_buddy_t *user = pjsip_buddy_node_lookup_by_username(username);
	if(!user)
		return ERROR;
	return _pjsip_buddy_username_add_phone_num(user, phone);
}

int pjsip_buddy_username_del_phone(char *username, char *phone)
{
	pjsip_buddy_t *user = pjsip_buddy_node_lookup_by_username(username);
	if(!user)
		return ERROR;
	return _pjsip_buddy_username_del_phone_num(user, phone);
}
