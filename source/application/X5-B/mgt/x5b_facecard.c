/*
 * x5b_user.c
 *
 *  Created on: 2019年5月15日
 *      Author: DELL
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "nsm_include.h"

#include "x5_b_global.h"
#ifdef X5B_APP_DATABASE

#include "x5b_dbase.h"
#include "x5b_facecard.h"

static LIST *facecard_table = NULL;
static os_mutex_t *facecard_mutex = NULL;

static int x5b_user_update_save(void);
static int x5b_user_load_from_file(void);


int x5b_user_clean(void)
{
	NODE node;
	user_face_card_t *dbtest = NULL;
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	for (dbtest = (user_face_card_t *) lstFirst(facecard_table);
			dbtest != NULL; dbtest = (user_face_card_t *) lstNext(&node))
	{
		node = dbtest->node;
		if (dbtest)
		{
			lstDelete(facecard_table, (NODE *) dbtest);
			XFREE(MTYPE_VOIP_DBTEST, dbtest);
		}
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return OK;
}

int x5b_user_exit()
{
	x5b_user_clean();
	if(facecard_mutex)
	{
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
		if(os_mutex_destroy(facecard_mutex)==OK)
			facecard_mutex = NULL;
	}
	if(facecard_table)
	{
		XFREE(MTYPE_VOIP_TOP, facecard_table);
		facecard_table = NULL;
	}
	voip_card_exit();
	return OK;
}




int x5b_user_load()
{
	if (facecard_table == NULL)
	{
		facecard_table = XMALLOC(MTYPE_VOIP_TOP, sizeof(LIST));
		if (facecard_table)
		{
			if(facecard_mutex == NULL)
				facecard_mutex = os_mutex_name_create("facecard_mutex");
			lstInit(facecard_table);
			if(facecard_mutex)
				os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
			x5b_user_load_from_file();
			if(facecard_mutex)
				os_mutex_unlock(facecard_mutex);
			voip_facecard_web_select_all();
			return OK;
		}
		return ERROR;
	}
	return OK;
}

#ifdef X5B_DBFACECARD_FILE
static int x5b_user_read_one(int fd, user_face_card_t *node)
{
	user_face_card_t *addnode = NULL;
	if(read(fd, node, sizeof(user_face_card_t)) == sizeof(user_face_card_t))
	{
		addnode = XMALLOC(MTYPE_VOIP_DBTEST, sizeof(user_face_card_t));
		if(!addnode)
			return ERROR;
		memset(addnode, 0, sizeof(user_face_card_t));
		memcpy(addnode, node, sizeof(user_face_card_t));
		lstAdd(facecard_table, (NODE *) addnode);
		return OK;
	}
	return ERROR;
}
#endif

static int x5b_user_load_from_file(void)
{
#ifdef X5B_DBFACECARD_FILE
	int ret = OK, fd = 0;
	user_face_card_t dbase;
	//char tmp[4];
	if (facecard_table == NULL)
		return ERROR;
	if(ret == 0)
	{
		fd = open(X5B_DBFACECARD_FILE, O_RDONLY);
		if(fd <= 0)
		{
			return ERROR;
		}
		while(ret == OK)
		{
			memset(&dbase, 0, sizeof(user_face_card_t));
			ret = x5b_user_read_one(fd, &dbase);
		}
		close(fd);
		return OK;
	}
	return ERROR;
#else
	return OK;
#endif
}

#ifdef X5B_DBFACECARD_FILE
static int x5b_user_write_list(int fd)
{
	int ret = 0;
	NODE node;
	user_face_card_t *dbase = NULL;
	for (dbase = (user_face_card_t *) lstFirst(facecard_table);
			dbase != NULL; dbase = (user_face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			ret = write(fd, dbase, sizeof(user_face_card_t));
			if(ret != sizeof(user_face_card_t))
				break;
		}
	}
	return ret;
}
#endif
static int x5b_user_update_save(void)
{
#ifdef X5B_DBFACECARD_FILE
	int ret = 0, fd = 0;
	if (facecard_table == NULL)
		return ERROR;
	if(lstCount(facecard_table) == 0)
	{
		if(access(X5B_DBFACECARD_FILE, F_OK) == 0)
			remove(X5B_DBFACECARD_FILE);
		sync();
		return OK;
	}
	if(ret == 0)
	{
		//char tmp[4];
		fd = open(X5B_DBFACECARD_FILE".tmp", O_RDWR|O_CREAT, 0644);
		if(fd <= 0)
		{
			return ERROR;
		}

		if(x5b_user_write_list(fd) != sizeof(user_face_card_t))
		{
			close(fd);
			remove(X5B_DBFACECARD_FILE".tmp");
			sync();
			return ERROR;
		}
		close(fd);
		rename(X5B_DBFACECARD_FILE".tmp", X5B_DBFACECARD_FILE);
		sync();
		return OK;
	}
	return ERROR;
#else
	return OK;
#endif
}



/***********************************************/
user_face_card_t * x5b_user_lookup_by_username(char *username, char *userid)
{
	//zpl_uint32 i = 0;
	char name[APP_USERNAME_MAX];
	char user_id[APP_ID_MAX];
	NODE node;
	user_face_card_t *dbase = NULL;
	if (facecard_table == NULL)
		return NULL;
	memset(name, 0, sizeof(name));
	memset(user_id, 0, sizeof(user_id));
	if(username)
		strncpy(name, username, MIN(sizeof(name), strlen(username)));
	if(userid)
		strncpy(user_id, userid, MIN(sizeof(user_id), strlen(userid)));
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	for (dbase = (user_face_card_t *) lstFirst(facecard_table);
			dbase != NULL; dbase = (user_face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			if(username)
			{
				if( (memcmp(dbase->username, name, sizeof(name)) == 0))
				{
					if(facecard_mutex)
						os_mutex_unlock(facecard_mutex);
					return dbase;
				}
			}
			else if(userid)
			{
				if( (memcmp(dbase->userid, user_id, sizeof(user_id)) == 0) )
				{
					if(facecard_mutex)
						os_mutex_unlock(facecard_mutex);
					return dbase;
				}
			}
		}
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return NULL;
}
#ifndef APP_CARDID_UINT_64
user_face_card_t * x5b_user_lookup_by_cardid(zpl_int8 *cardid)
#else
user_face_card_t * x5b_user_lookup_by_cardid(zpl_uint64 cardid)
#endif
{
	zpl_uint32 i = 0;
#ifndef APP_CARDID_UINT_64
	zpl_uint8 lphone[APP_CARD_ID_MAX];
#endif
	NODE node;
	user_face_card_t *dbase = NULL;
	if (facecard_table == NULL)
		return NULL;
#ifndef APP_CARDID_UINT_64
	zassert(cardid != NULL);
	memset(lphone, 0, sizeof(lphone));
	if(cardid)
		strncpy(lphone, cardid, MIN(sizeof(lphone), strlen(cardid)));
#endif
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	for (dbase = (user_face_card_t *) lstFirst(facecard_table);
			dbase != NULL; dbase = (user_face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			for(i = 0; i < APP_MULTI_CARD_MAX; i++)
			{
				if(dbase->cardtbl[i].use_flag == 0)
					continue;
#ifndef APP_CARDID_UINT_64
				if( (strncasecmp(dbase->cardtbl[i].cardid, lphone, sizeof(lphone)) == 0))
#else
				if( dbase->cardtbl[i].cardid == cardid)
#endif
				{
					if(facecard_mutex)
						os_mutex_unlock(facecard_mutex);
					return dbase;
				}
			}
		}
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return NULL;
}


user_face_card_t * x5b_user_lookup_by_faceid(zpl_uint32 faceid)
{
	zpl_uint32 i = 0;
	NODE node;
	user_face_card_t *dbase = NULL;
	if (facecard_table == NULL)
		return NULL;
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	for (dbase = (user_face_card_t *) lstFirst(facecard_table);
			dbase != NULL; dbase = (user_face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			if (dbase)
			{
				for(i = 0; i < APP_MULTI_FACE_MAX; i++)
				{
					if(dbase->facetbl[i].use_flag == 0)
						continue;
					if(dbase->facetbl[i].faceid == faceid)
					{
						if(facecard_mutex)
							os_mutex_unlock(facecard_mutex);
						return dbase;
					}
				}
			}
		}
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return NULL;
}

/**************************************************************/


static int x5b_user_add_one_node(user_face_card_t *dbase)
{
	lstAdd(facecard_table, (NODE *) dbase);
	return OK;
}

static int x5b_user_del_one_node(user_face_card_t *dbase)
{
	lstDelete(facecard_table, (NODE *) dbase);
	XFREE(MTYPE_VOIP_DBTEST, dbase);
	return OK;
}
/**************************************************************/
#ifndef APP_CARDID_UINT_64
static int x5b_user_add_card_hw(user_face_card_t *dbase, zpl_int8 *cardid, zpl_uint32 start, zpl_uint32 stop, zpl_uint8 type)
#else
static int x5b_user_add_card_hw(user_face_card_t *dbase, zpl_uint64 cardid, zpl_uint32 start, zpl_uint32 stop, zpl_uint8 type)
#endif
{
	zpl_uint32 i = 0;
	for(i = 0; i < APP_MULTI_CARD_MAX; i++)
	{
		if(dbase->cardtbl[i].use_flag == 0)
		{
#ifndef APP_CARDID_UINT_64
			strncpy(dbase->cardtbl[i].cardid, cardid,
					MIN(APP_CARD_ID_MAX, strlen(cardid)));

			//strcpy(dbase->cardtbl[i].cardid, cardid);
			dbase->cardtbl[i].start_time = start;
			dbase->cardtbl[i].stop_time = stop;
			zlog_debug(MODULE_APP, "=================>cardid=%s(l=%d)", dbase->cardtbl[i].cardid,
					   (int)strlen(dbase->cardtbl[i].cardid));
#else
			dbase->cardtbl[i].cardid = cardid;
			dbase->cardtbl[i].start_time = start;
			dbase->cardtbl[i].stop_time = stop;
			zlog_debug(MODULE_APP, "=================>cardid=%08x", dbase->cardtbl[i].cardid);
#endif
			zlog_debug(MODULE_APP, "=================>start=%d", dbase->cardtbl[i].start_time);
			zlog_debug(MODULE_APP, "=================>stop=%d", dbase->cardtbl[i].stop_time);
			dbase->cardtbl[i].card_type = type;
			dbase->cardtbl[i].use_flag = 1;
			dbase->card_max++;
			return OK;
		}
	}
	return ERROR;
}
#ifndef APP_CARDID_UINT_64
static int x5b_user_del_card_hw(user_face_card_t *dbase, zpl_int8 *cardid)
#else
static int x5b_user_del_card_hw(user_face_card_t *dbase, zpl_uint64 cardid)
#endif
{
	zpl_uint32 i = 0;
	for(i = 0; i < APP_MULTI_CARD_MAX; i++)
	{
		if(dbase->cardtbl[i].use_flag == 1)
		{
#ifndef APP_CARDID_UINT_64
			if(strncasecmp(dbase->cardtbl[i].cardid, cardid, sizeof(dbase->cardtbl[i].cardid)) == 0)
			{
				dbase->cardtbl[i].start_time = 0;
				dbase->cardtbl[i].stop_time = 0;
				dbase->cardtbl[i].card_type = 0;
				dbase->cardtbl[i].use_flag = 0;
				dbase->card_max--;
				memset(dbase->cardtbl[i].cardid, 0, sizeof(dbase->cardtbl[i].cardid));
				return OK;
			}
#else
			if(dbase->cardtbl[i].cardid == cardid)
			{
				dbase->cardtbl[i].start_time = 0;
				dbase->cardtbl[i].stop_time = 0;
				dbase->cardtbl[i].card_type = 0;
				dbase->cardtbl[i].use_flag = 0;
				dbase->card_max--;
				dbase->cardtbl[i].cardid = 0;
				return OK;
			}
#endif
		}
	}
	return ERROR;
}

static int x5b_user_del_card_hw_cid(user_face_card_t *dbase, zpl_uint8 cid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < APP_MULTI_CARD_MAX; i++)
	{
		if(dbase->cardtbl[i].use_flag == 1 && i == cid)
		{
			dbase->cardtbl[i].start_time = 0;
			dbase->cardtbl[i].stop_time = 0;
			dbase->cardtbl[i].card_type = 0;
			dbase->cardtbl[i].use_flag = 0;
			dbase->card_max--;
#ifndef APP_CARDID_UINT_64
			memset(dbase->cardtbl[i].cardid, 0, sizeof(dbase->cardtbl[i].cardid));
#else
			dbase->cardtbl[i].cardid = 0;
#endif
			return OK;
		}
	}
	return ERROR;
}
/**************************************************************/
static int x5b_user_add_face_hw(user_face_card_t *dbase, char *img, zpl_uint32 faceid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < APP_MULTI_FACE_MAX; i++)
	{
		if(dbase->facetbl[i].use_flag == 0)
		{
			strncpy(dbase->facetbl[i].imgid, img,
					MIN(sizeof(dbase->facetbl[i].imgid),strlen(img)));
			//strcpy(dbase->facetbl[i].imgid, img);

			dbase->facetbl[i].faceid = faceid;
			dbase->facetbl[i].use_flag = 1;
			dbase->face_max++;
			return OK;
		}
	}
	return ERROR;
}

static int x5b_user_del_face_hw(user_face_card_t *dbase, zpl_uint32 faceid)
{
	zpl_uint32 i = 0;
	for(i = 0; i < APP_MULTI_FACE_MAX; i++)
	{
		if(dbase->facetbl[i].use_flag == 1)
		{
			if(dbase->facetbl[i].faceid == faceid)
			{
				dbase->face_max--;
				dbase->facetbl[i].use_flag = 0;
				memset(dbase->facetbl[i].imgid, 0, sizeof(dbase->facetbl[i].imgid));
				return OK;
			}
		}
	}
	return ERROR;
}

/**************************************************************/
user_face_card_t * x5b_user_add(char *username, char *userid)
{
	user_face_card_t *dbase = XMALLOC(MTYPE_VOIP_DBTEST, sizeof(user_face_card_t));
	if(!dbase)
		return NULL;
	memset(dbase, 0, sizeof(user_face_card_t));

	if(userid && x5b_user_lookup_by_username(NULL, userid))
	{
		return NULL;
	}
	if(username)
		strcpy(dbase->username, username);
	if(userid)
		strcpy(dbase->userid, userid);
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	x5b_user_add_one_node(dbase);
	x5b_user_update_save();
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return dbase;
}

int x5b_user_del(char *username, char *userid)
{
	user_face_card_t *dbase = x5b_user_lookup_by_username(NULL, userid);
	if(dbase == NULL)
		return ERROR;
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	x5b_user_del_one_node(dbase);
	x5b_user_update_save();
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return OK;
}

/*************************************************************************/
#ifndef APP_CARDID_UINT_64
int x5b_user_add_card(char *username, char *userid, zpl_int8 *cardid, zpl_uint32 start, zpl_uint32 stop, zpl_uint8 type)
#else
int x5b_user_add_card(char *username, char *userid, zpl_uint64 cardid, zpl_uint32 start, zpl_uint32 stop, zpl_uint8 type)
#endif
{
	int ret = ERROR;
	user_face_card_t *dbase = x5b_user_lookup_by_username(NULL, userid);
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	if(dbase)
	{
		ret = x5b_user_add_card_hw(dbase,  cardid,  start,  stop,  type);
		if(ret == OK)
		{
			x5b_user_update_save();
			if(facecard_mutex)
				os_mutex_unlock(facecard_mutex);
			return ret;
		}
	}
	else
	{
		dbase = x5b_user_add(username, userid);
		if(dbase)
		{
			ret = x5b_user_add_card_hw(dbase,  cardid,  start,  stop,  type);
			if(ret == OK)
			{
				x5b_user_update_save();
				if(facecard_mutex)
					os_mutex_unlock(facecard_mutex);
				return ret;
			}
		}
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return ERROR;
}

#ifndef APP_CARDID_UINT_64
int x5b_user_update_card(char *username, char *userid, zpl_int8 *cardid, zpl_uint32 start,
					  zpl_uint32 stop, zpl_uint8 type, zpl_uint8 cid)
#else
int x5b_user_update_card(char *username, char *userid, zpl_uint64 cardid, zpl_uint32 start,
					  zpl_uint32 stop, zpl_uint8 type, zpl_uint8 cid)
#endif
{
	int ret = ERROR;
	user_face_card_t *dbase = x5b_user_lookup_by_username(NULL, userid);
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	if(dbase)
	{
		x5b_user_del_card_hw_cid(dbase,  cid);
		zlog_debug(MODULE_APP,"===============delete frist and add");
		ret = x5b_user_add_card_hw(dbase,  cardid,  start,  stop,  type);
		if(ret == OK)
		{
			x5b_user_update_save();
			if(facecard_mutex)
				os_mutex_unlock(facecard_mutex);
			return ret;
		}
	}
	else
	{
		dbase = x5b_user_add(username, userid);
		if(dbase)
		{
			ret = x5b_user_add_card_hw(dbase,  cardid,  start,  stop,  type);
			if(ret == OK)
			{
				x5b_user_update_save();
				if(facecard_mutex)
					os_mutex_unlock(facecard_mutex);
				return ret;
			}
		}
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return ERROR;
}

#ifndef APP_CARDID_UINT_64
int x5b_user_del_card(char *username, char *userid, zpl_int8 *cardid)
#else
int x5b_user_del_card(char *username, char *userid, zpl_uint64 cardid)
#endif
{
	int ret = ERROR;
	user_face_card_t *dbase = x5b_user_lookup_by_username(NULL, userid);
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	if(dbase)
	{
		ret = x5b_user_del_card_hw(dbase,  cardid);
		if(ret == OK)
		{
			x5b_user_update_save();
			if(facecard_mutex)
				os_mutex_unlock(facecard_mutex);
			return ret;
		}
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return ERROR;
}

#ifndef APP_CARDID_UINT_64
int x5b_user_get_card(char *username, char *userid, zpl_int8 *cardid, zpl_uint8 cid)
#else
int x5b_user_get_card(char *username, char *userid, zpl_uint64 *cardid, zpl_uint8 cid)
#endif
{
	//int ret = ERROR;
	user_face_card_t *dbase = x5b_user_lookup_by_username(NULL, userid);
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	if(dbase)
	{
		zpl_uint32 i = 0;
		for(i = 0; i < APP_MULTI_CARD_MAX; i++)
		{
			if(dbase->cardtbl[i].use_flag == 1 && i == cid)
			{
/*				dbase->cardtbl[i].start_time = 0;
				dbase->cardtbl[i].stop_time = 0;
				dbase->cardtbl[i].card_type = 0;
				dbase->cardtbl[i].use_flag = 0;
				dbase->card_max--;*/
				if(cardid)
				{
#ifndef APP_CARDID_UINT_64
					strncpy(cardid, dbase->cardtbl[i].cardid, strlen(dbase->cardtbl[i].cardid));
#else
					*cardid = dbase->cardtbl[i].cardid;
#endif
				}
				if(facecard_mutex)
					os_mutex_unlock(facecard_mutex);
				//memset(dbase->cardtbl[i].cardid, 0, sizeof(dbase->cardtbl[i].cardid));
				return OK;
			}
		}
/*		ret = x5b_user_del_card_hw(dbase,  cardid);
		if(ret == OK)
		{
			x5b_user_update_save();
			if(facecard_mutex)
				os_mutex_unlock(facecard_mutex);
			return ret;
		}*/
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return ERROR;
}
/*************************************************************************/
int x5b_user_add_face(char *username, char *userid, char *img, zpl_uint32 faceid)
{
	int ret = ERROR;
	user_face_card_t *dbase = x5b_user_lookup_by_username(NULL, userid);
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	if(dbase)
	{
		ret = x5b_user_add_face_hw(dbase,  img,  faceid);
		if(ret == OK)
		{
			x5b_user_update_save();
			if(facecard_mutex)
				os_mutex_unlock(facecard_mutex);
			return ret;
		}
	}
	else
	{
		dbase = x5b_user_add(username, userid);
		if(dbase)
		{
			ret = x5b_user_add_face_hw(dbase,   img,  faceid);
			if(ret == OK)
			{
				x5b_user_update_save();
				if(facecard_mutex)
					os_mutex_unlock(facecard_mutex);
				return ret;
			}
		}
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return ERROR;
}

int x5b_user_update_face(char *username, char *userid, char *img, zpl_uint32 faceid)
{
	int ret = ERROR;
	user_face_card_t *dbase = x5b_user_lookup_by_username(NULL, userid);
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	if(dbase)
	{
		x5b_user_del_face_hw(dbase,  faceid);
		zlog_debug(MODULE_APP,"===============delete frist and add");
		ret = x5b_user_add_face_hw(dbase,  img,  faceid);
		if(ret == OK)
		{
			x5b_user_update_save();
			if(facecard_mutex)
				os_mutex_unlock(facecard_mutex);
			return ret;
		}
	}
	else
	{
		dbase = x5b_user_add(username, userid);
		if(dbase)
		{
			ret = x5b_user_add_face_hw(dbase,   img,  faceid);
			if(ret == OK)
			{
				x5b_user_update_save();
				if(facecard_mutex)
					os_mutex_unlock(facecard_mutex);
				return ret;
			}
		}
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return ERROR;
}

int x5b_user_get_face(char *username, char *user_id, zpl_uint32 *face_id, zpl_uint8 fid)
{
	//int ret = ERROR;
	user_face_card_t *dbase = x5b_user_lookup_by_username(NULL, user_id);
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	if(dbase)
	{
		zpl_uint32 i = 0;
		for(i = 0; i < APP_MULTI_FACE_MAX; i++)
		{
			if(dbase->facetbl[i].use_flag == 1 && i == fid)
			{
				if(face_id)
				{
					*face_id = dbase->facetbl[i].faceid;
				}
				if(facecard_mutex)
					os_mutex_unlock(facecard_mutex);
				return OK;
			}
		}
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return ERROR;
}

int x5b_user_del_face(char *username, char *userid, zpl_uint32 faceid)
{
	int ret = ERROR;
	user_face_card_t *dbase = x5b_user_lookup_by_username(NULL, userid);
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	if(dbase)
	{
		ret = x5b_user_del_face_hw(dbase,  faceid);
		if(ret == OK)
		{
			x5b_user_update_save();
			if(facecard_mutex)
				os_mutex_unlock(facecard_mutex);
			return ret;
		}
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return ERROR;
}
/*************************************************************************/
/*************************************************************************/
#ifndef APP_CARDID_UINT_64
user_card_t * x5b_user_get_card_info(zpl_int8 *cardid)
#else
user_card_t * x5b_user_get_card_info(zpl_uint64 cardid)
#endif
{
	zpl_uint32 i = 0;
#ifndef APP_CARDID_UINT_64
	zpl_uint8 lphone[APP_CARD_ID_MAX];
#endif
	NODE node;
	user_face_card_t *dbase = NULL;
	if (facecard_table == NULL)
		return NULL;
#ifndef APP_CARDID_UINT_64
	zassert(cardid != NULL);
	memset(lphone, 0, sizeof(lphone));
	if(cardid)
		strncpy(lphone, cardid, MIN(sizeof(lphone), strlen(cardid)));
#endif
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	for (dbase = (user_face_card_t *) lstFirst(facecard_table);
			dbase != NULL; dbase = (user_face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			for(i = 0; i < APP_MULTI_CARD_MAX; i++)
			{
				if(dbase->cardtbl[i].use_flag == 0)
					continue;
#ifndef APP_CARDID_UINT_64
				if( (strncasecmp(dbase->cardtbl[i].cardid, lphone, sizeof(lphone)) == 0))
#else
				if( dbase->cardtbl[i].cardid == cardid)
#endif
				{
					if(facecard_mutex)
						os_mutex_unlock(facecard_mutex);
					return &dbase->cardtbl[i];
				}
			}
		}
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return NULL;
}


user_face_t * x5b_user_get_face_info(zpl_uint32 faceid)
{
	zpl_uint32 i = 0;
	NODE node;
	user_face_card_t *dbase = NULL;
	if (facecard_table == NULL)
		return NULL;
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	for (dbase = (user_face_card_t *) lstFirst(facecard_table);
			dbase != NULL; dbase = (user_face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			if (dbase)
			{
				for(i = 0; i < APP_MULTI_FACE_MAX; i++)
				{
					if(dbase->facetbl[i].use_flag == 0)
						continue;
					if(dbase->facetbl[i].faceid == faceid)
					{
						if(facecard_mutex)
							os_mutex_unlock(facecard_mutex);
						return &dbase->facetbl[i];
					}
				}
			}
		}
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return NULL;
}
/*************************************************************************/
int x5b_user_save_config(void)
{
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	x5b_user_update_save();
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return OK;
}
/*************************************************************************/
/*************************************************************************/

static char *facecard_time_fmt (char *fmt, zpl_time_t t)
{
	zpl_uint32 len = 0;
	struct tm tm;
	static char data[128];
	zpl_time_t ticlock = t;
	os_memset(data, 0, sizeof(data));
	os_memset(&tm, 0, sizeof(tm));
	//localtime_r(&ticlock, &tm);
    if(os_tmtime_get (OS_TMTIME_LOCAL, ticlock, &tm) != OK)
    {
    	return "UNKNOWN";
    }
	len = strftime(data, sizeof(data), "%Y-%m-%dT%H:%M:%S", &tm);
	if(len > 0)
		return data;
	return "UNKNOWN";
}

static int voip_facecard_cli_show_detail(struct vty *vty, user_face_card_t *card, zpl_bool detail)
{
	if(vty)
	{
		zpl_uint32 i = 0;
		char ptype[16];
		char imgid[16];
		char starttime[128];
#ifdef APP_CARDID_UINT_64
		char caridstr[128];
#endif
		sprintf(imgid, "%d", card->face_max);
		for(i = 0; i < APP_MULTI_CARD_MAX; i++)
		{
			if(card->cardtbl[i].use_flag == 0)
				continue;
			memset(ptype, 0, sizeof(ptype));
			if(card->cardtbl[i].card_type == 1)
				sprintf(ptype, "%s", "Blacklist");
			else if(card->cardtbl[i].card_type == 2)
				sprintf(ptype, "%s", "Whitelist");
			else
				sprintf(ptype, "%s", "Unknow");

			memset(starttime, 0, sizeof(starttime));
			snprintf(starttime, sizeof(starttime), "%s", facecard_time_fmt("iso",card->cardtbl[i].start_time + OS_SEC_HOU_V(0)));
			if(detail)
			{
#ifdef APP_CARDID_UINT_64
				memset(caridstr, 0, sizeof(caridstr));
				snprintf(caridstr, sizeof(caridstr), "%x", card->cardtbl[i].cardid);

				vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s %s%s",
						card->username,
						card->userid,
						caridstr,
						starttime,
						facecard_time_fmt("iso",card->cardtbl[i].stop_time + OS_SEC_HOU_V(0)),
						ptype, imgid, card->cardtbl[i].imgid, VTY_NEWLINE);
#else
				vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s %s%s",
						card->username,
						card->userid,
						card->cardtbl[i].cardid,
						starttime,
						facecard_time_fmt("iso",card->cardtbl[i].stop_time + OS_SEC_HOU_V(0)),
						ptype, imgid, card->facetbl[i].imgid, VTY_NEWLINE);
#endif
			}
			else
			{
#ifdef APP_CARDID_UINT_64
				memset(caridstr, 0, sizeof(caridstr));
				snprintf(caridstr, sizeof(caridstr), "%x", card->cardtbl[i].cardid);

				vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s%s",
						card->username,
						card->userid,
						caridstr,
						starttime,
						facecard_time_fmt("iso",card->cardtbl[i].stop_time + OS_SEC_HOU_V(0)),
						ptype, imgid, VTY_NEWLINE);
#else
				vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s%s",
						card->username,
						card->userid,
						card->cardtbl[i].cardid,
						starttime,
						facecard_time_fmt("iso",card->cardtbl[i].stop_time + OS_SEC_HOU_V(0)),
						ptype, imgid, VTY_NEWLINE);
#endif
			}
		}
	}
	return OK;
}

int voip_facecard_cli_show_all(struct vty *vty, zpl_bool detail)
{
	NODE node;
	user_face_card_t *dbase = NULL;
	if (facecard_table == NULL)
		return OK;

	if(lstCount(facecard_table) == 0)
	{
		vty_out(vty, "%s", VTY_NEWLINE);
		return OK;
	}
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	if(lstCount(facecard_table))
	{
		//2019-05-21T15:24:00
		if(detail)
		{
			vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s %-12s%s", "------------", "------------",
					"--------------------", "--------------------", "--------------------", "------------",
					"------------", "--------------------", VTY_NEWLINE);
			vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s %-12s%s", "User Name", "User ID", "Card ID",
					"Start Time", "Stop Time", "Card Type", "Face IMG Num", "Face IMG Name", VTY_NEWLINE);
			vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s %-12s%s", "------------", "------------",
					"--------------------", "--------------------", "--------------------", "------------",
					"------------", "--------------------", VTY_NEWLINE);
		}
		else
		{
			vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s%s", "------------", "------------",
					"--------------------", "--------------------", "--------------------", "------------",
					"------------", VTY_NEWLINE);
			vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s%s", "User Name", "User ID", "Card ID",
					"Start Time", "Stop Time", "Card Type", "Face IMG Num", VTY_NEWLINE);
			vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s%s", "------------", "------------",
					"--------------------", "--------------------", "--------------------", "------------",
					"------------", VTY_NEWLINE);
		}
	}

	for (dbase = (user_face_card_t *) lstFirst(facecard_table);
			dbase != NULL; dbase = (user_face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			voip_facecard_cli_show_detail(vty, dbase, detail);
		}
	}
	if(lstCount(facecard_table))
	{
		vty_out(vty, "%s", VTY_NEWLINE);
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return OK;
}
/*************************************************************************/
/*************************************************************************/
static int voip_facecard_web_select_detail(FILE *fp, user_face_card_t *card)
{
	if(fp)
	{
		zpl_uint32 i = 0;
		zpl_uint16 room_number = 0;
		char phone[32];
		memset(phone, 0, sizeof(phone));
		voip_dbase_get_room_phone_by_user(card->userid, &room_number, phone, NULL);

		for(i = 0; i < APP_MULTI_CARD_MAX; i++)
		{
			if(card->cardtbl[i].use_flag == 0)
				continue;
			if(strlen(card->username))
				fprintf(fp, "%s ", card->username);
			else
				fprintf(fp, ", ");

			if(strlen(card->userid))
				fprintf(fp, "%s ", card->userid);
			else
				fprintf(fp, ", ");


			if(strlen(phone))
				fprintf(fp, "%s ", phone);
			else
				fprintf(fp, ", ");

			if(room_number)
				fprintf(fp, "%04d ", room_number);
			else
				fprintf(fp, ", ");
#ifndef APP_CARDID_UINT_64
			if(strlen(card->cardtbl[i].cardid))
				fprintf(fp, "%s ", card->cardtbl[i].cardid);
			else
				fprintf(fp, ", ");
#else
			if(strlen(card->cardtbl[i].cardid))
				fprintf(fp, "%s ", card->cardtbl[i].cardid);
			else
				fprintf(fp, ", ");
#endif
			if(card->cardtbl[i].start_time)
				fprintf(fp, "%s ", facecard_time_fmt("iso",card->cardtbl[i].start_time + OS_SEC_HOU_V(0)));
			else
				fprintf(fp, ", ");

			if(card->cardtbl[i].stop_time)
				fprintf(fp, "%s ", facecard_time_fmt("iso",card->cardtbl[i].stop_time + OS_SEC_HOU_V(0)));
			else
				fprintf(fp, ", ");

			if(card->cardtbl[i].card_type == 1)
				fprintf(fp, "%s ", "Blacklist");
			else if(card->cardtbl[i].card_type == 2)
				fprintf(fp, "%s ", "Whitelist");
			else
				fprintf(fp, "Unknow ");

			fprintf(fp, "%d ", card->face_max);

	/*		if(strlen(card->imgid))
				fprintf(fp, "%s ", card->imgid);
			else
				fprintf(fp, ", ");*/

			fprintf(fp, "\n");
			fflush(fp);
		}
	}
	return OK;
}

int voip_facecard_web_select_all(void)
{
	FILE *fp = NULL;
	NODE node;
	user_face_card_t *dbase = NULL;
	if (facecard_table == NULL)
		return OK;
	remove("/tmp/app/tmp/userauth.txt");
	if(lstCount(facecard_table))
	{
		fp = fopen("/tmp/app/tmp/userauth.txt", "a");
	}
	if(facecard_mutex)
		os_mutex_lock(facecard_mutex, OS_WAIT_FOREVER);
	for (dbase = (user_face_card_t *) lstFirst(facecard_table);
			dbase != NULL; dbase = (user_face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			voip_facecard_web_select_detail(fp, dbase);
		}
	}
	if(lstCount(facecard_table) && fp)
	{
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}
	if(facecard_mutex)
		os_mutex_unlock(facecard_mutex);
	return OK;
}
#endif
