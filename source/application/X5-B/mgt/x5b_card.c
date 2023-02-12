#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "nsm_include.h"


#include "x5_b_global.h"
#ifdef X5B_APP_DATABASE
#include "x5b_dbase.h"

static LIST *card_table = NULL;
static os_mutex_t *card_mutex = NULL;

static int voip_card_update_save(void);
static int voip_card_load_from_file(void);

int card_id_string_to_hex(const char *id, zpl_uint32 len, zpl_uint8 *cardNumber)
{
	char tmp[8], cid[64];
	zpl_uint32 i = 0, offset = 0;
	if(cardNumber == NULL)
		return 0;
	memset(tmp, 0, sizeof(tmp));
	memset(cid, 0, sizeof(cid));
	if(len < 14)
	{
		memcpy(cardNumber, id, len);
		return len;
	}
	if(len == 14)
		memcpy(cid, id, len);
		//memcpy(cid + 2, id, len);
	else if(len == 16)
		memcpy(cid, id, len);

	for(i = 0; i < len; i+=2)
	{
		tmp[0]=cid[i];
		tmp[1]=cid[i+1];
		cardNumber[offset] = strtol(tmp, NULL, 16);
		offset+=1;
	}
	return offset;
}



int voip_card_clean(void)
{
	NODE node;
	face_card_t *dbtest = NULL;
	if(card_mutex)
		os_mutex_lock(card_mutex, OS_WAIT_FOREVER);
	for (dbtest = (face_card_t *) lstFirst(card_table);
			dbtest != NULL; dbtest = (face_card_t *) lstNext(&node))
	{
		node = dbtest->node;
		if (dbtest)
		{
			lstDelete(card_table, (NODE *) dbtest);
			XFREE(MTYPE_VOIP_DBTEST, dbtest);
		}
	}
#ifdef X5B_DBCARD_FILE
	remove(X5B_DBCARD_FILE);
#endif
	sync();
	if(card_mutex)
		os_mutex_unlock(card_mutex);
	return OK;
}

int voip_card_exit()
{
	voip_card_clean();
	if(card_mutex)
	{
		os_mutex_lock(card_mutex, OS_WAIT_FOREVER);
		if(os_mutex_destroy(card_mutex)==OK)
			card_mutex = NULL;
	}
	if(card_table)
	{
		XFREE(MTYPE_VOIP_TOP, card_table);
		card_table = NULL;
	}
	return OK;
}




int voip_card_load()
{
	if (card_table == NULL)
	{
		card_table = XMALLOC(MTYPE_VOIP_TOP, sizeof(LIST));
		if (card_table)
		{
			if(card_mutex == NULL)
				card_mutex = os_mutex_name_create("card_mutex");
			lstInit(card_table);
			if(card_mutex)
				os_mutex_lock(card_mutex, OS_WAIT_FOREVER);
			voip_card_load_from_file();
			if(card_mutex)
				os_mutex_unlock(card_mutex);

			voip_card_web_select_all();
			return OK;
		}
		return ERROR;
	}
	return OK;
}

#ifdef X5B_DBCARD_FILE
static int voip_card_read_one(int fd, face_card_t *node)
{
	face_card_t *addnode = NULL;
	if(read(fd, node, sizeof(face_card_t)) == sizeof(face_card_t))
	{
		addnode = XMALLOC(MTYPE_VOIP_DBTEST, sizeof(face_card_t));
		if(!addnode)
			return ERROR;
		memset(addnode, 0, sizeof(face_card_t));
		memcpy(addnode, node, sizeof(face_card_t));
		lstAdd(card_table, (NODE *) addnode);
		return OK;
	}
	return ERROR;
}
#endif

static int voip_card_load_from_file(void)
{
#ifdef X5B_DBCARD_FILE
	int ret = OK, fd = 0;
	face_card_t dbase;
	if (card_table == NULL)
		return ERROR;
	if(ret == 0)
	{
		fd = open(X5B_DBCARD_FILE, O_RDONLY);
		if(fd <= 0)
		{
			return ERROR;
		}
		while(ret == OK)
		{
			memset(&dbase, 0, sizeof(face_card_t));
			ret = voip_card_read_one(fd, &dbase);
		}
		close(fd);
		return OK;
	}
	return ERROR;
#else
	return OK;
#endif
}

#ifdef X5B_DBCARD_FILE
static int voip_card_write_list(int fd)
{
	int ret = 0;
	NODE node;
	face_card_t *dbase = NULL;
	for (dbase = (face_card_t *) lstFirst(card_table);
			dbase != NULL; dbase = (face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			ret = write(fd, dbase, sizeof(face_card_t));
			if(ret != sizeof(face_card_t))
				break;
		}
	}
	return ret;
}
#endif
static int voip_card_update_save(void)
{
#ifdef X5B_DBCARD_FILE
	int ret = 0, fd = 0;
	if (card_table == NULL)
		return ERROR;
	if(lstCount(card_table) == 0)
	{
		if(access(X5B_DBCARD_FILE, F_OK) == 0)
			remove(X5B_DBCARD_FILE);
		sync();
		return OK;
	}
	if(ret == 0)
	{
		fd = open(X5B_DBCARD_FILE".tmp", O_RDWR|O_CREAT, 0644);
		if(fd <= 0)
		{
			return ERROR;
		}
		if(voip_card_write_list(fd) != sizeof(face_card_t))
		{
			close(fd);
			remove(X5B_DBCARD_FILE".tmp");
			sync();
			return ERROR;
		}
		close(fd);
		rename(X5B_DBCARD_FILE".tmp", X5B_DBCARD_FILE);
		sync();
		return OK;
	}
	return ERROR;
#else
	return OK;
#endif
}


static int voip_card_add_one_node(face_card_t *dbase)
{
	lstAdd(card_table, (NODE *) dbase);
	return OK;
}

static int voip_card_del_one_node(face_card_t *dbase)
{
	lstDelete(card_table, (NODE *) dbase);
	XFREE(MTYPE_VOIP_DBTEST, dbase);
	return OK;
}


/***********************************************/
face_card_t * voip_card_node_lookup_by_username(char *username, char *user_id)
{
	char name[APP_USERNAME_MAX];
	char userid[APP_ID_MAX];
	NODE node;
	face_card_t *dbase = NULL;
	if (card_table == NULL)
		return NULL;
	memset(name, 0, sizeof(name));
	memset(userid, 0, sizeof(userid));
	if(username)
		strncpy(name, username, MIN(sizeof(name), strlen(username)));
	if(user_id)
		strncpy(userid, user_id, MIN(sizeof(name), strlen(user_id)));
	if(card_mutex)
		os_mutex_lock(card_mutex, OS_WAIT_FOREVER);
	for (dbase = (face_card_t *) lstFirst(card_table);
			dbase != NULL; dbase = (face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			if(username && user_id)
			{
				if( (memcmp(dbase->username, name, sizeof(name)) == 0) &&
						(memcmp(dbase->user_id, userid, sizeof(userid)) == 0) )
				{
					if(card_mutex)
						os_mutex_unlock(card_mutex);
					return dbase;
				}
			}
			else
			{
				if(username)
				{
					if( (memcmp(dbase->username, name, sizeof(name)) == 0))
					{
						if(card_mutex)
							os_mutex_unlock(card_mutex);
						return dbase;
					}
				}
				else if(user_id)
				{
					if( (memcmp(dbase->user_id, userid, sizeof(userid)) == 0) )
					{
						if(card_mutex)
							os_mutex_unlock(card_mutex);
						return dbase;
					}
				}
			}
		}
	}
	if(card_mutex)
		os_mutex_unlock(card_mutex);
	return NULL;
}

face_card_t * voip_card_node_lookup_by_cardid(char *carid)
{
	char card_id[APP_CARD_ID_MAX];
	NODE node;
	face_card_t *dbase = NULL;
	if (card_table == NULL)
		return NULL;
	zassert(carid != NULL);
	memset(card_id, 0, sizeof(card_id));
	if(carid)
		strncpy(card_id, carid, MIN(sizeof(card_id), strlen(carid)));
	if(card_mutex)
		os_mutex_lock(card_mutex, OS_WAIT_FOREVER);
	for (dbase = (face_card_t *) lstFirst(card_table);
			dbase != NULL; dbase = (face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			if( (strncasecmp(dbase->card_id, card_id, sizeof(card_id)) == 0))
			{
				if(card_mutex)
					os_mutex_unlock(card_mutex);
				zlog_debug(MODULE_APP, "=========%s==========%s", __func__,card_id);
				return dbase;
			}
		}
	}
	if(card_mutex)
		os_mutex_unlock(card_mutex);
	zlog_debug(MODULE_APP, "=========%s=====can not get===%s", __func__,card_id);
	return NULL;
}

static int voip_card_debug_info(char *hdr, face_card_t *card)
{
	zlog_debug(MODULE_APP, "=========%s==========", hdr);
	zlog_debug(MODULE_APP, "username:%s", strlen(card->username)? card->username:" ");
	//zlog_debug(MODULE_APP, "username:%d", card->card_type);
	zlog_debug(MODULE_APP, "user_id:%s", strlen(card->user_id)? card->user_id:" ");
	zlog_debug(MODULE_APP, "card_id:%s", strlen(card->card_id)? card->card_id:" ");
	zlog_debug(MODULE_APP, "img_id:%s",strlen(card->img_id)? card->img_id:" ");
	zlog_debug(MODULE_APP, "face_id:%d", card->face_id);
	zlog_debug(MODULE_APP, "start_time:%d", card->start_time);
	zlog_debug(MODULE_APP, "stop_time:%d", card->stop_time);
	zlog_debug(MODULE_APP, "card_type:%d", card->card_type);
	zlog_debug(MODULE_APP, "make_card:%d", card->make_card);
	zlog_debug(MODULE_APP, "make_face:%d", card->make_face);
	zlog_debug(MODULE_APP, "===================");
	return OK;
}
int voip_card_add_cardid(face_card_t *card)
{
/*	if(voip_card_node_lookup_by_cardid(carid))
		return ERROR;*/
	face_card_t *dbase = XMALLOC(MTYPE_VOIP_DBTEST, sizeof(face_card_t));
	if(!dbase)
		return ERROR;
	memset(dbase, 0, sizeof(face_card_t));

/*	if(username)
		strcpy(dbase->username, username);
	if(user_id)
		strcpy(dbase->user_id, user_id);
	if(carid)
		strcpy(dbase->card_id, carid);*/
	memcpy(dbase, card, sizeof(face_card_t));
	if(card_mutex)
		os_mutex_lock(card_mutex, OS_WAIT_FOREVER);
	voip_card_add_one_node(dbase);
	voip_card_debug_info("add", dbase);
	voip_card_update_save();
	if(card_mutex)
		os_mutex_unlock(card_mutex);
	voip_card_web_select_all();
	return OK;
}

int voip_card_del_cardid(char *username, char *user_id, char *carid)
{
	face_card_t *dbase = NULL;
	if(carid)
	{
		dbase = voip_card_node_lookup_by_cardid(carid);
		if(!dbase)
		{
			zlog_debug(MODULE_APP,"==== can not lookup carid by card id:%s", carid);
			return ERROR;
		}
		if(card_mutex)
			os_mutex_lock(card_mutex, OS_WAIT_FOREVER);
		voip_card_del_one_node(dbase);
		voip_card_update_save();
		if(card_mutex)
			os_mutex_unlock(card_mutex);
		voip_card_web_select_all();
		return OK;
	}
	else
	{
		dbase = voip_card_node_lookup_by_username(username, user_id);
		if(!dbase)
			return ERROR;
		if(card_mutex)
			os_mutex_lock(card_mutex, OS_WAIT_FOREVER);
		voip_card_del_one_node(dbase);
		voip_card_update_save();
		if(card_mutex)
			os_mutex_unlock(card_mutex);
		voip_card_web_select_all();
		return OK;
	}
	return ERROR;
}


int voip_card_update_cardid(char *user_id, face_card_t *info)
{
	face_card_t *dbase = NULL;
	if(user_id)
	{
		dbase = voip_card_node_lookup_by_username(NULL, user_id);
		//dbase = voip_card_node_lookup_by_cardid(carid);
		if(!dbase)
			return ERROR;
		if(card_mutex)
			os_mutex_lock(card_mutex, OS_WAIT_FOREVER);

		if(strlen(info->username))
		{
			memset(dbase->username, 0, sizeof(dbase->username));
			strcpy(dbase->username, info->username);
		}
		if(strlen(info->user_id))
		{
			memset(dbase->user_id, 0, sizeof(dbase->user_id));
			strcpy(dbase->user_id, info->user_id);
		}

		if(strlen(info->card_id))
		{
			memset(dbase->card_id, 0, sizeof(dbase->card_id));
			strcpy(dbase->card_id, info->card_id);
		}
		dbase->start_time = info->start_time;
		dbase->stop_time = info->stop_time;
		dbase->card_type = info->card_type;
		dbase->make_card = info->make_card;
		dbase->make_face = info->make_face;
		zlog_debug(MODULE_APP, "===================%s -> type=%d", __func__, dbase->card_type);
		voip_card_update_save();
		if(card_mutex)
			os_mutex_unlock(card_mutex);
		voip_card_web_select_all();
		return OK;
	}
	return ERROR;
}

int voip_card_update_cardid_by_userid(char *user_id, face_card_t *info)
{
	face_card_t *dbase = NULL;
	if(user_id)
	{
		dbase = voip_card_node_lookup_by_username(NULL, user_id);
		//dbase = voip_card_node_lookup_by_cardid(carid);
		if(!dbase)
			return ERROR;
		if(card_mutex)
			os_mutex_lock(card_mutex, OS_WAIT_FOREVER);

		if(strlen(info->username))
		{
			memset(dbase->username, 0, sizeof(dbase->username));
			strcpy(dbase->username, info->username);
		}
		if(strlen(info->card_id))
		{
			memset(dbase->card_id, 0, sizeof(dbase->card_id));
			strcpy(dbase->card_id, info->card_id);
		}
		dbase->start_time = info->start_time;
		dbase->stop_time = info->stop_time;
		dbase->card_type = info->card_type;
		dbase->make_card = info->make_card;
/*		if(strlen(info->user_id))
		{
			memset(dbase->user_id, 0, sizeof(dbase->user_id));
			strcpy(dbase->user_id, info->user_id);
		}*/

/*		if(strlen(info->img_id))
		{
			memset(dbase->img_id, 0, sizeof(dbase->img_id));
			strcpy(dbase->img_id, info->img_id);
		}
		dbase->start_time = info->start_time;
		dbase->stop_time = info->stop_time;
		dbase->card_type = info->card_type;
		dbase->make_card = info->make_card;
		dbase->make_face = info->make_face;*/
		//zlog_debug(MODULE_APP, "===================%s -> type=%d", __func__, dbase->card_type);
		voip_card_debug_info("add card", dbase);
		voip_card_update_save();
		if(card_mutex)
			os_mutex_unlock(card_mutex);
		voip_card_web_select_all();
		return OK;
	}
	return ERROR;
}

int voip_card_update_face_by_userid(char *user_id, face_card_t *info)
{
	face_card_t *dbase = NULL;
	if(user_id)
	{
		dbase = voip_card_node_lookup_by_username(NULL, user_id);
		//dbase = voip_card_node_lookup_by_cardid(carid);
		if(!dbase)
			return ERROR;
		if(card_mutex)
			os_mutex_lock(card_mutex, OS_WAIT_FOREVER);

		if(strlen(info->username))
		{
			memset(dbase->username, 0, sizeof(dbase->username));
			strcpy(dbase->username, info->username);
		}
		if(strlen(info->img_id))
		{
			memset(dbase->img_id, 0, sizeof(dbase->img_id));
			strcpy(dbase->img_id, info->img_id);
		}
/*		dbase->start_time = info->start_time;
		dbase->stop_time = info->stop_time;
		dbase->card_type = info->card_type;*/
		dbase->make_face = info->make_face;
		dbase->face_id = info->face_id;

/*		if(strlen(info->user_id))
		{
			memset(dbase->user_id, 0, sizeof(dbase->user_id));
			strcpy(dbase->user_id, info->user_id);
		}*/

/*		if(strlen(info->img_id))
		{
			memset(dbase->img_id, 0, sizeof(dbase->img_id));
			strcpy(dbase->img_id, info->img_id);
		}
		dbase->start_time = info->start_time;
		dbase->stop_time = info->stop_time;
		dbase->card_type = info->card_type;
		dbase->make_card = info->make_card;
		dbase->make_face = info->make_face;*/
		voip_card_debug_info("add face", dbase);
		//zlog_debug(MODULE_APP, "===================%s -> type=%d", __func__, dbase->card_type);
		voip_card_update_save();
		if(card_mutex)
			os_mutex_unlock(card_mutex);
		voip_card_web_select_all();
		return OK;
	}
	return ERROR;
}


int voip_card_lookup_by_cardid_userid(char *carid, char *userid)
{
	face_card_t *dbase = voip_card_node_lookup_by_cardid(carid);
	if(dbase)
	{
		char user_id[APP_ID_MAX];
		memset(user_id, 0, sizeof(user_id));
		if(userid)
		{
			strncpy(user_id, userid, MIN(sizeof(user_id), strlen(userid)));
			if(card_mutex)
				os_mutex_lock(card_mutex, OS_WAIT_FOREVER);
			if( (memcmp(dbase->user_id, user_id, sizeof(user_id)) != 0) )
			{
				if(card_mutex)
					os_mutex_unlock(card_mutex);
				return OK;
			}
			if(card_mutex)
				os_mutex_unlock(card_mutex);
		}
	}
	return ERROR;
}

static int show_voip_card_debug_info(struct vty *vty, face_card_t *card)
{
	vty_out(vty, "===================%s", VTY_NEWLINE);
	vty_out(vty, "username:%s%s", strlen(card->username)? card->username:" ", VTY_NEWLINE);
	//vty_out(vty, "username:%d", card->card_type);
	vty_out(vty, "user_id:%s%s", strlen(card->username)? card->user_id:" ", VTY_NEWLINE);
	vty_out(vty, "card_id:%s%s", strlen(card->username)? card->card_id:" ", VTY_NEWLINE);
	vty_out(vty, "img_id:%s%s",strlen(card->username)? card->img_id:" ", VTY_NEWLINE);
	vty_out(vty, "face_id:%d%s", card->face_id, VTY_NEWLINE);
	vty_out(vty, "start_time:%d%s", card->start_time, VTY_NEWLINE);
	vty_out(vty, "stop_time:%d%s", card->stop_time, VTY_NEWLINE);
	vty_out(vty, "card_type:%d%s", card->card_type, VTY_NEWLINE);
	vty_out(vty, "make_card:%d%s", card->make_card, VTY_NEWLINE);
	vty_out(vty, "make_face:%d%s", card->make_face,VTY_NEWLINE);
	vty_out(vty, "===================%s", VTY_NEWLINE);
	return OK;
}

int show_voip_card_info(struct vty *vty)
{
	NODE node;
	face_card_t *dbase = NULL;
	if (card_table == NULL)
		return OK;

	if(lstCount(card_table) == 0)
	{
		vty_out(vty, "%s", VTY_NEWLINE);
		return OK;
	}
	if(card_mutex)
		os_mutex_lock(card_mutex, OS_WAIT_FOREVER);
	if(lstCount(card_table))
	{
		//2019-05-21T15:24:00
/*		vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s%s", "------------", "------------",
				"--------------------", "--------------------", "--------------------", "------------",
				"------------", VTY_NEWLINE);
		vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s%s", "User Name", "User ID", "Card ID",
				"Start Time", "Stop Time", "Card Type", "Face IMG Id", VTY_NEWLINE);
		vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s%s", "------------", "------------",
				"--------------------", "--------------------", "--------------------", "------------",
				"------------", VTY_NEWLINE);*/
	}

	for (dbase = (face_card_t *) lstFirst(card_table);
			dbase != NULL; dbase = (face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			show_voip_card_debug_info(vty, dbase);
		}
	}
	if(lstCount(card_table))
	{
		vty_out(vty, "%s", VTY_NEWLINE);
	}
	if(card_mutex)
		os_mutex_unlock(card_mutex);
	return OK;
}

static char *card_time_fmt (char *fmt, zpl_time_t t)
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

static int voip_card_cli_show_detail(struct vty *vty, face_card_t *card)
{
	if(vty)
	{
		char ptype[16];
		char starttime[128];
		memset(ptype, 0, sizeof(ptype));
		if(card->card_type == 1)
			sprintf(ptype, "%s", "Blacklist");
		else if(card->card_type == 2)
			sprintf(ptype, "%s", "Whitelist");
		else
			sprintf(ptype, "%s", "Unknow");

		memset(starttime, 0, sizeof(starttime));
		snprintf(starttime, sizeof(starttime), "%s", card_time_fmt("iso",card->start_time + OS_SEC_HOU_V(0)));


		vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s%s",
				card->username,
				card->user_id,
				card->card_id,
				starttime,
				card_time_fmt("iso",card->stop_time + OS_SEC_HOU_V(0)),
				ptype, card->img_id, VTY_NEWLINE);
	}
	return OK;
}

int voip_card_cli_show_all(struct vty *vty)
{
	NODE node;
	face_card_t *dbase = NULL;
	if (card_table == NULL)
		return OK;

	if(lstCount(card_table) == 0)
	{
		vty_out(vty, "%s", VTY_NEWLINE);
		return OK;
	}
	if(card_mutex)
		os_mutex_lock(card_mutex, OS_WAIT_FOREVER);
	if(lstCount(card_table))
	{
		//2019-05-21T15:24:00
		vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s%s", "------------", "------------",
				"--------------------", "--------------------", "--------------------", "------------",
				"------------", VTY_NEWLINE);
		vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s%s", "User Name", "User ID", "Card ID",
				"Start Time", "Stop Time", "Card Type", "Face IMG Id", VTY_NEWLINE);
		vty_out(vty, "%-12s %-12s  %-20s %-20s  %-20s  %-12s %-12s%s", "------------", "------------",
				"--------------------", "--------------------", "--------------------", "------------",
				"------------", VTY_NEWLINE);
	}

	for (dbase = (face_card_t *) lstFirst(card_table);
			dbase != NULL; dbase = (face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			voip_card_cli_show_detail(vty, dbase);
		}
	}
	if(lstCount(card_table))
	{
		vty_out(vty, "%s", VTY_NEWLINE);
	}
	if(card_mutex)
		os_mutex_unlock(card_mutex);
	return OK;
}


static int voip_card_web_select_detail(FILE *fp, face_card_t *card)
{
	if(fp)
	{
		if(strlen(card->username))
			fprintf(fp, "%s ", card->username);
		else
			fprintf(fp, ", ");

		if(strlen(card->user_id))
			fprintf(fp, "%s ", card->user_id);
		else
			fprintf(fp, ", ");

		if(strlen(card->card_id))
			fprintf(fp, "%s ", card->card_id);
		else
			fprintf(fp, ", ");

		if(card->start_time)
			fprintf(fp, "%s ", card_time_fmt("iso",card->start_time + OS_SEC_HOU_V(0)));
		else
			fprintf(fp, ", ");

		if(card->stop_time)
			fprintf(fp, "%s ", card_time_fmt("iso",card->stop_time + OS_SEC_HOU_V(0)));
		else
			fprintf(fp, ", ");

		if(card->card_type == 1)
			fprintf(fp, "%s ", "Blacklist");
		else if(card->card_type == 2)
			fprintf(fp, "%s ", "Whitelist");
		else
			fprintf(fp, "Unknow ");


		if(strlen(card->img_id))
			fprintf(fp, "%s ", card->img_id);
		else
			fprintf(fp, ", ");

		fprintf(fp, "\n");
		fflush(fp);
	}
	return OK;
}

int voip_card_web_select_all(void)
{
	FILE *fp = NULL;
	NODE node;
	face_card_t *dbase = NULL;
	if (card_table == NULL)
		return OK;
	remove("/tmp/app/tmp/face-card.txt");
	if(lstCount(card_table))
	{
		fp = fopen("/tmp/app/tmp/face-card.txt", "a");
	}
	if(card_mutex)
		os_mutex_lock(card_mutex, OS_WAIT_FOREVER);
	for (dbase = (face_card_t *) lstFirst(card_table);
			dbase != NULL; dbase = (face_card_t *) lstNext(&node))
	{
		node = dbase->node;
		if (dbase)
		{
			voip_card_web_select_detail(fp, dbase);
		}
	}
	if(lstCount(card_table) && fp)
	{
		fprintf(fp, "\n");
		fflush(fp);
		fclose(fp);
	}
	if(card_mutex)
		os_mutex_unlock(card_mutex);
	return OK;
}
#endif
