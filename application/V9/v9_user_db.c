#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "nsm_interface.h"
#include "log.h"
#include "vty.h"

#include "sqlite3.h"
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

ospl_uint32 _sqldb_debug = V9_SQLDB_DEBUG_MSG | V9_SQLDB_DEBUG_DB
		| V9_SQLDB_DEBUG_STATE;






static char * v9_user_sqldb_file(ospl_uint32 id)
{
	ospl_uint32 iid = APP_BOARD_CALCU_1;
	static char dirbase[V9_APP_DIR_NAME_MAX];
	memset(dirbase, 0, sizeof(dirbase));
	//snprintf(dirbase, sizeof(dirbase), "%s/%s", v9_video_disk_base_dir(APP_BOARD_CALCU_1), V9_USER_DB_FILE);
	//snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_DIR_BASE"/%s", V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id), V9_USER_DB_FILE);
	//snprintf(dirbase, sizeof(dirbase), V9_VIDEO_DB_FILE, id);
	snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_DIR_BASE"/%s",
		V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(iid), V9_USER_DB_FILE);
	return dirbase;
}

static sqlite3 * v9_user_sqldb_init(ospl_uint32 id)
{
	int ret = 0;
	sqlite3 *db = NULL;
	if (os_file_access(v9_user_sqldb_file(id)) != OK)
	{
		FILE *fp = fopen(v9_user_sqldb_file(id), "ab+");
		if (fp)
			fclose(fp);
	}
	ret = sqlite3_open(v9_user_sqldb_file(id), &db);
	if (ret != SQLITE_OK)
	{
		if (V9_SQLDB_DEBUG(MSG))
			zlog_err(MODULE_APP, "Can't open database: %s(%s)",
					v9_user_sqldb_file(id), sqlite3_errmsg(db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
		sqlite3_close(db);
		return NULL;
	}
	return db;
}

static int v9_user_sqldb_exit(sqlite3 *db)
{
	if (db)
	{
		sqlite3_close(db);
		return OK;
	}
	return ERROR;
}

static int v9_user_sqldb_sync(sqlite3 *db, ospl_uint32 id)
{
	if (db)
	{
		sqlite3_db_cacheflush(db);
		return OK;
	}
	return ERROR;
}

static ospl_bool v9_user_sqldb_table_exist(sqlite3 *db, ospl_uint32 id)
{
	char sqlcmd[256];
	char *zErrMsg = NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	snprintf(sqlcmd, sizeof(sqlcmd), "select * from "V9_USER_DB_TBL,
			V9_APP_DB_ID_ABS(id));
	if (V9_SQLDB_DEBUG(DBCMD))
		zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
	if (sqlite3_exec(db, sqlcmd, NULL, NULL, &zErrMsg) == SQLITE_OK)
		return ospl_true;
	if (V9_SQLDB_DEBUG(MSG))
		zlog_err(MODULE_APP, " SQL Table "V9_USER_DB_TBL" exist check (%s)",
				V9_APP_DB_ID_ABS(id), zErrMsg);
	sqlite3_free(zErrMsg);
	return ospl_false;
}

static int v9_user_sqldb_table_create(sqlite3 *db, ospl_uint32 id)
{
	char sqlcmd[512];
	char *zErrMsg = NULL;

	memset(sqlcmd, 0, sizeof(sqlcmd));
	snprintf(sqlcmd, sizeof(sqlcmd), "%s/table%d", V9_USER_DB_DIR, V9_APP_DB_ID_ABS(id));
	if(access(sqlcmd, F_OK) != 0)
		mkdir(sqlcmd, 0644);

	memset(sqlcmd, 0, sizeof(sqlcmd));
	snprintf(sqlcmd, sizeof(sqlcmd), V9_USER_DB_TBL_ST, V9_APP_DB_ID_ABS(id));

	if (V9_SQLDB_DEBUG(DBCMD))
		zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
	if (sqlite3_exec(db, sqlcmd, NULL, NULL, &zErrMsg) == SQLITE_OK)
		return OK;
	if (V9_SQLDB_DEBUG(MSG))
		zlog_err(MODULE_APP, " SQL Create Table(%s)", zErrMsg);
	sqlite3_free(zErrMsg);
	return ERROR;
}

sqlite3 * v9_user_sqldb_open(ospl_uint32 id)
{
	sqlite3 *db = NULL;
	if (db == NULL)
		db = v9_user_sqldb_init(id);
	if (!db)
		return NULL;
	if (v9_user_sqldb_table_exist(db, id) != ospl_true)
	{
		v9_user_sqldb_table_create(db, id);
	}
	return db;
}

int v9_user_sqldb_close(sqlite3 *db, ospl_uint32 id)
{
	v9_user_sqldb_sync(db, id);
	v9_user_sqldb_exit(db);
	return OK;
}

static int v9_user_sqldb_count_callback(void *NotUsed, int argc, char **argv,
		char **azColName)
{
	ospl_uint32 i;
	int *value = NotUsed;
	for (i = 0; i < argc; i++)
	{
		if (value && argv[i])
			*value = atoi(argv[i]);
		//printf(" %s:%s = %s\n", __func__, azColName[i], argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

int v9_user_sqldb_count(sqlite3 *db, ospl_uint32 id, int group, int *pValue)
{
	int value = 0;
	char sqlcmd[512];
	char *zErrMsg = NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	if (group >= 0)
	{
		snprintf(sqlcmd, sizeof(sqlcmd),
				"SELECT count(*) FROM "V9_USER_DB_TBL " WHERE groupid = %d;",
				V9_APP_DB_ID_ABS(id), group);
	}
	else
		snprintf(sqlcmd, sizeof(sqlcmd),
				"SELECT count(*) FROM "V9_USER_DB_TBL ";",
				V9_APP_DB_ID_ABS(id));

	if (V9_SQLDB_DEBUG(DBCMD))
		zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

	if (sqlite3_exec(db, sqlcmd, v9_user_sqldb_count_callback, &value,
			&zErrMsg) == SQLITE_OK)
	{
		if (pValue)
			*pValue = value;
		return OK;
	}
	if (V9_SQLDB_DEBUG(MSG))
		zlog_err(MODULE_APP, " SQL SELECT count(%s)", zErrMsg);
	sqlite3_free(zErrMsg);
	return ERROR;
}

int v9_user_sqldb_add(sqlite3 *db, ospl_uint32 id, ospl_bool gender, int group,
		char *user, char *user_id, char *pic, char *text)
{
	char sqlcmd[1024];
	char *zErrMsg = NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	if(text)
		snprintf(sqlcmd, sizeof(sqlcmd),
			"INSERT INTO "V9_USER_DB_TBL " (userid,username,gender,groupid,datetime,url,text) VALUES( \
    		'%s',\
    		'%s',\
    		%d,\
    		%d,\
    		'%s',\
    		'%s',\
    		'%s'\
    		);",
			V9_APP_DB_ID_ABS(id), user_id, user, gender ? 1 : 0, group,
			os_time_fmt("sql", os_time(NULL)), pic, text ? text : " ");
	else
		snprintf(sqlcmd, sizeof(sqlcmd),
			"INSERT INTO "V9_USER_DB_TBL " (userid,username,gender,groupid,datetime,url) VALUES( \
    		'%s',\
    		'%s',\
    		%d,\
    		%d,\
    		'%s',\
    		'%s'\
    		);",
			V9_APP_DB_ID_ABS(id), user_id, user, gender ? 1 : 0, group,
			os_time_fmt("sql", os_time(NULL)), pic);

	if (V9_SQLDB_DEBUG(DBCMD))
		zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

	if (sqlite3_exec(db, sqlcmd, NULL, NULL, &zErrMsg) == SQLITE_OK)
		return OK;
	if (V9_SQLDB_DEBUG(MSG))
		zlog_err(MODULE_APP, " SQL INSERT Info(%s)", zErrMsg);
	sqlite3_free(zErrMsg);
	return ERROR;
}

int v9_user_sqldb_update(sqlite3 *db, ospl_uint32 id, ospl_bool gender, int group,
		char *user, char *user_id, char *pic, char *text)
{
	char sqlcmd[1024];
	char *zErrMsg = NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	if(text)
		snprintf(sqlcmd, sizeof(sqlcmd),
			"UPDATE "V9_USER_DB_TBL " SET \
    		username ='%s',\
    		gender =%d,\
    		groupid =%d,\
    		datetime ='%s',\
    		url ='%s', \
    		text ='%s' \
    		WHERE userid = '%s';",
			V9_APP_DB_ID_ABS(id), user, gender ? 1 : 0, group,
			os_time_fmt("sql", os_time(NULL)), pic, text ? text : " ", user_id);
	else
		snprintf(sqlcmd, sizeof(sqlcmd),
			"UPDATE "V9_USER_DB_TBL " SET \
    		username ='%s',\
    		gender =%d,\
    		groupid =%d,\
    		datetime ='%s',\
    		url ='%s' \
    		WHERE userid = '%s';",
			V9_APP_DB_ID_ABS(id), user, gender ? 1 : 0, group,
			os_time_fmt("sql", os_time(NULL)), pic, user_id);

	if (V9_SQLDB_DEBUG(DBCMD))
		zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

	if (sqlite3_exec(db, sqlcmd, NULL, NULL, &zErrMsg) == SQLITE_OK)
		return OK;
	if (V9_SQLDB_DEBUG(MSG))
		zlog_err(MODULE_APP, " SQL UPDATE Info(%s)", zErrMsg);
	sqlite3_free(zErrMsg);
	return ERROR;
}

int v9_user_sqldb_key_update(sqlite3 *db, ospl_uint32 id, char *user_id,
		sql_snapfea_key *key)
{
	char sqlcmd[512];
	sqlite3_stmt *stmt = NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	snprintf(sqlcmd, sizeof(sqlcmd),
			"UPDATE "V9_USER_DB_TBL " SET \
    		key=? \
    		WHERE userid = '%s';",
			V9_APP_DB_ID_ABS(id), user_id);

	if (V9_SQLDB_DEBUG(DBCMD))
		zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

	if (sqlite3_prepare(db, sqlcmd, strlen(sqlcmd), &stmt, NULL) != SQLITE_OK)
	{
		if (V9_SQLDB_DEBUG(MSG))
			zlog_err(MODULE_APP, " SQL Prepare fail(%s)", sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		return ERROR;
	}
	if (sqlite3_bind_blob(stmt, 1, key->feature.feature_data,
			key->feature_len * sizeof(ospl_float),
			SQLITE_STATIC) != SQLITE_OK)
	{
		if (V9_SQLDB_DEBUG(MSG))
			zlog_err(MODULE_APP, " SQL Bind fail(%s)", sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		return ERROR;
	}
	if (sqlite3_step(stmt) == SQLITE_DONE)
	{
		sqlite3_finalize(stmt);
		return OK;
	}
	if (V9_SQLDB_DEBUG(MSG))
		zlog_err(MODULE_APP, " SQL Step fail(%s)", sqlite3_errmsg(db));
	sqlite3_finalize(stmt);
	return ERROR;
}

int v9_user_sqldb_key_select(sqlite3 *db, ospl_uint32 id, char *user_id,
		sql_snapfea_key *key)
{
	int ret = 0;
	char sqlcmd[512];
	sqlite3_stmt *stmt = NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));

	snprintf(sqlcmd, sizeof(sqlcmd),
			"SELECT username,key FROM "V9_USER_DB_TBL " WHERE userid = '%s';",
			V9_APP_DB_ID_ABS(id), user_id);

	if (V9_SQLDB_DEBUG(DBCMD))
		zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

	if (sqlite3_prepare(db, sqlcmd, strlen(sqlcmd), &stmt, NULL) != SQLITE_OK)
	{
		if (V9_SQLDB_DEBUG(MSG))
			zlog_err(MODULE_APP, " SQL Prepare fail(%s)", sqlite3_errmsg(db));
		return ERROR;
	}
	ret = sqlite3_step(stmt);
	if (/*ret == SQLITE_DONE || */ret == SQLITE_ROW)
	{
		ospl_uint32 len = sqlite3_column_bytes(stmt, 1);
		void *pReadBolbData = sqlite3_column_blob(stmt, 1);

		if (key && len && pReadBolbData)
		{
			if(len > APP_FEATURE_MAX)
			{
				key->feature.feature_data = key->feature.ckey_data = XREALLOC(MTYPE_VIDEO_KEY,
					key->feature.ckey_data, len + sizeof(ospl_float));									// 特征值
			}
			if (key->feature.ckey_data && pReadBolbData)
			{
				key->feature_len = len / sizeof(ospl_float);
				memcpy(key->feature.feature_data, pReadBolbData, len);
				V9_DB_DEBUG(" %s :len=%d data[0]=%f\n", __func__,
						key->feature_len, key->feature.feature_data[0]);
			}
			sqlite3_finalize(stmt);
			return OK;
		}
	}
	if (V9_SQLDB_DEBUG(MSG))
		zlog_err(MODULE_APP, " SQL Step Finalize(%s)", sqlite3_errmsg(db));
	sqlite3_finalize(stmt);
	return ERROR;
}

/*static int v9_user_sqldb_del_callback(void *pArgv, ospl_uint32 type, char const * name,
		char const * tblname, sqlite3_int64 aaaa)
{
	printf(" %s :type=%d, name=%s, tblname=%s\n", __func__, type, name,
			tblname);
	return OK;
}*/

static int v9_user_sqldb_del_callback(void *NotUsed, int argc, char **argv,
		char **azColName)
{
	ospl_uint32 i;
	for (i = 0; i < argc; i++)
	{
		if (azColName[i] && strcasecmp(azColName[i], "url") == 0)
		{
			if(argv[i])
				remove(argv[i]);
			return 0;
		}
	}
	return 0;
}

int v9_user_sqldb_cleanup(ospl_uint32 id)
{
/*	if(_user_mutex)
		os_mutex_lock(_user_mutex, OS_WAIT_FOREVER);

	if(_user_mutex)
		os_mutex_unlock(_user_mutex);*/
	remove(v9_user_sqldb_file(id));
	system("rm "V9_USER_DB_DIR"/* -rf");
	sync();
	return 0;
}

int v9_user_sqldb_del(sqlite3 *db, ospl_uint32 id, char *user_id)
{
	char sqlcmd[256];
	char *zErrMsg = NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	snprintf(sqlcmd, sizeof(sqlcmd),
			"DELETE FROM "V9_USER_DB_TBL " WHERE userid = '%s';",
			V9_APP_DB_ID_ABS(id), user_id);

	if (V9_SQLDB_DEBUG(DBCMD))
		zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

	//sqlite3_update_hook(db, v9_user_sqldb_del_callback, NULL);
	if (sqlite3_exec(db, sqlcmd, v9_user_sqldb_del_callback, NULL, &zErrMsg) == SQLITE_OK)
	{
		sync();
		return OK;
	}
	if (V9_SQLDB_DEBUG(MSG))
		zlog_err(MODULE_APP, " SQL DELETE Info by userid:%s(%s)", user_id,
				zErrMsg);
	sqlite3_free(zErrMsg);
	return ERROR;
}

int v9_user_sqldb_del_group(sqlite3 *db, ospl_uint32 id, int group)
{
	char sqlcmd[256];
	char *zErrMsg = NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	snprintf(sqlcmd, sizeof(sqlcmd),
			"DELETE FROM "V9_USER_DB_TBL " WHERE groupid = %d;",
			V9_APP_DB_ID_ABS(id), group);

	if (V9_SQLDB_DEBUG(DBCMD))
		zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

	//sqlite3_update_hook(db, v9_user_sqldb_del_callback, NULL);
	if (sqlite3_exec(db, sqlcmd, v9_user_sqldb_del_callback, NULL, &zErrMsg) == SQLITE_OK)
	{
		sync();
		return OK;
	}
	if (V9_SQLDB_DEBUG(MSG))
		zlog_err(MODULE_APP, " SQL DELETE Info by group:%d(%s)", group, zErrMsg);
	sqlite3_free(zErrMsg);
	return ERROR;
}

static int v9_user_sqldb_lookup_callback(void *NotUsed, int argc, char **argv,
		char **azColName)
{
	ospl_uint32 i;
	v9_video_user_t *user = NotUsed;
	for (i = 0; i < argc; i++)
	{
		if (azColName[i] && strcasecmp(azColName[i], "userid") == 0)
		{
			if (argv[i])
				strcpy(user->userid, argv[i]);
		}
		else if (azColName[i] && strcasecmp(azColName[i], "username") == 0)
		{
			if (argv[i])
				strcpy(user->username, argv[i]);
		}
		else if (azColName[i] && strcasecmp(azColName[i], "gender") == 0)
		{
			if (argv[i])
				user->gender = atoi(argv[i]);
		}
		else if (azColName[i] && strcasecmp(azColName[i], "groupid") == 0)
		{
			if (argv[i])
				user->group = atoi(argv[i]);
		}
		else if (azColName[i] && strcasecmp(azColName[i], "url") == 0)
		{
			if (argv[i])
				strcpy(user->picname, argv[i]);
		}
		else if (azColName[i] && strcasecmp(azColName[i], "text") == 0)
		{
			if (argv[i])
				strcpy(user->text, argv[i]);
		}
		//sql_snapfea_key	key;
	}
	//printf("========%s===========%s\r\n", __func__, user->userid);
	return 0;
}

int v9_user_sqldb_lookup_user(sqlite3 *db, ospl_uint32 id, char *user_id,
		void *pArgv)
{
	char sqlcmd[512];
	char *zErrMsg = NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	memset(pArgv, 0, sizeof(v9_video_user_t));
	if (user_id)
	{
		snprintf(sqlcmd, sizeof(sqlcmd),
				"SELECT userid,username,gender,groupid,url FROM "V9_USER_DB_TBL " WHERE userid = '%s';",
				V9_APP_DB_ID_ABS(id), user_id);
		if (V9_SQLDB_DEBUG(DBCMD))
			zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

		if (sqlite3_exec(db, sqlcmd, v9_user_sqldb_lookup_callback, pArgv,
				&zErrMsg) == SQLITE_OK)
		{
			v9_video_user_t *user = pArgv;
			if (strlen(user->userid))
				return OK;
			return ERROR;
		}
	}
	else
	{
		snprintf(sqlcmd, sizeof(sqlcmd),
				"SELECT userid,username,gender,groupid,datetime,url,text FROM "V9_USER_DB_TBL ";",
				V9_APP_DB_ID_ABS(id));

		if (V9_SQLDB_DEBUG(DBCMD))
			zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

		if (sqlite3_exec(db, sqlcmd, v9_user_sqldb_lookup_callback, pArgv,
				&zErrMsg) == SQLITE_OK)
		{
			v9_video_user_t *user = pArgv;
			if (strlen(user->userid))
				return OK;
			return ERROR;

		}
	}
	if (V9_SQLDB_DEBUG(MSG))
		zlog_err(MODULE_APP, " SQL GET Info by userid:%s(%s)", user_id, zErrMsg);
	sqlite3_free(zErrMsg);
	return ERROR;
}

struct user_sqldb_foreach
{
	ospl_uint8 id;
	v9_video_user_t user;
	v9_vidoe_callback cb;
	void *pVoid;
};

static int v9_user_sqldb_callback(void *NotUsed, int argc, char **argv,
		char **azColName)
{
	ospl_uint32 i = 0;
	struct user_sqldb_foreach *user_sqldb = NotUsed;

	for (i = 0; i < argc; i++)
	{
		if (azColName[i] && strcasecmp(azColName[i], "userid") == 0)
		{
			if (argv[i])
				strcpy(user_sqldb->user.userid, argv[i]);
		}
		else if (azColName[i] && strcasecmp(azColName[i], "username") == 0)
		{
			if (argv[i])
				strcpy(user_sqldb->user.username, argv[i]);
		}
		else if (azColName[i] && strcasecmp(azColName[i], "gender") == 0)
		{
			if (argv[i])
				user_sqldb->user.gender = atoi(argv[i]);
		}
		else if (azColName[i] && strcasecmp(azColName[i], "groupid") == 0)
		{
			if (argv[i])
				user_sqldb->user.group = atoi(argv[i]);
		}
		else if (azColName[i] && strcasecmp(azColName[i], "url") == 0)
		{
			if (argv[i])
				strcpy(user_sqldb->user.picname, argv[i]);
		}
		else if (azColName[i] && strcasecmp(azColName[i], "text") == 0)
		{
			if (argv[i])
				strcpy(user_sqldb->user.text, argv[i]);
		}
		user_sqldb->user.ID = user_sqldb->id;
	}
	if (i)
	{
		if (user_sqldb->cb)
			(user_sqldb->cb)(&user_sqldb->user, user_sqldb->pVoid);
	}
	return 0;
}

int v9_user_sqldb_foreach(sqlite3 *db, ospl_uint32 id, int groupid, char *user_id,
		void * cb, void *pVoid)
{
	char sqlcmd[512];
	char *zErrMsg = NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	struct user_sqldb_foreach user_sqldb;

	memset(&user_sqldb, 0, sizeof(user_sqldb));
	user_sqldb.cb = cb;
	user_sqldb.pVoid = pVoid;
	user_sqldb.id = id;
	//user_sqldb.cb = cb;

	if (user_id)
	{
		snprintf(sqlcmd, sizeof(sqlcmd),
				"SELECT userid,username,gender,groupid,datetime,url,text FROM "V9_USER_DB_TBL " WHERE userid = '%s';",
				V9_APP_DB_ID_ABS(id), user_id);
		if (V9_SQLDB_DEBUG(DBCMD))
			zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
		if (sqlite3_exec(db, sqlcmd, v9_user_sqldb_callback, &user_sqldb,
				&zErrMsg) == SQLITE_OK)
			return OK;
	}
	else
	{
		if (groupid != ERROR)
			snprintf(sqlcmd, sizeof(sqlcmd),
					"SELECT userid,username,gender,groupid,datetime,url,text FROM "V9_USER_DB_TBL " WHERE groupid = %d;",
					V9_APP_DB_ID_ABS(id), groupid);
		else
			snprintf(sqlcmd, sizeof(sqlcmd),
					"SELECT userid,username,gender,groupid,datetime,url,text FROM "V9_USER_DB_TBL ";",
					V9_APP_DB_ID_ABS(id));
		if (V9_SQLDB_DEBUG(DBCMD))
			zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
		if (sqlite3_exec(db, sqlcmd, v9_user_sqldb_callback, &user_sqldb,
				&zErrMsg) == SQLITE_OK)
			return OK;
	}
	if (V9_SQLDB_DEBUG(MSG))
		zlog_err(MODULE_APP, " SQL Foreach Info (%s)", zErrMsg);
	sqlite3_free(zErrMsg);
	return ERROR;
}

int v9_user_sqldb_key_foreach(sqlite3 *db, ospl_uint32 id,
		sql_snapfea_key *key, char *user_id, ospl_float* p_fResult)
{
	int ret = 0;
	char sqlcmd[512];
	sqlite3_stmt *stmt = NULL;
	ospl_uint32 len = 0;
	void *pReadBolbData = NULL;
	ospl_float fresult = 0.0f;

	memset(sqlcmd, 0, sizeof(sqlcmd));

	snprintf(sqlcmd, sizeof(sqlcmd),
			"SELECT userid,username,gender,groupid,datetime,key,url,text FROM "V9_USER_DB_TBL " ;",
			V9_APP_DB_ID_ABS(id));

	if (V9_SQLDB_DEBUG(DBCMD))
		zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

	if (sqlite3_prepare(db, sqlcmd, strlen(sqlcmd), &stmt, NULL) != SQLITE_OK)
	{
		if (V9_SQLDB_DEBUG(MSG))
			zlog_err(MODULE_APP, " SQL Prepare fail(%s)", sqlite3_errmsg(db));
		return ERROR;
	}
	ret = sqlite3_step(stmt);
	while (/*ret == SQLITE_DONE || */ret == SQLITE_ROW)
	{
		len = sqlite3_column_bytes(stmt, 5);
		pReadBolbData = sqlite3_column_blob(stmt, 5);

		if (key && len && pReadBolbData)
		{
			if (key->feature_memcmp)
			{
				ret = (key->feature_memcmp)(key->feature.feature_data, pReadBolbData,
						MIN(len / sizeof(ospl_float), key->feature_len), &fresult);
				if (ret == OK)
				{
					//if (fresult >= value)
					if((double)fabs((double)fresult) >= (double)key->input_value)
					{
						if (p_fResult)
							*p_fResult = fresult;
						len = sqlite3_column_bytes(stmt, 0);
						pReadBolbData = sqlite3_column_blob(stmt, 0);
						if (key && len && pReadBolbData)
						{
							strcpy(user_id, pReadBolbData);
						}
					}
				}
			}
 			else
 			{
 				zlog_warn(MODULE_APP, " feature cmp func is null");
 			}
		}

		ret = sqlite3_step(stmt);
	}

	sqlite3_finalize(stmt);
	return OK;
}

#ifdef V9_SQLDB_TEST
int v9_user_sqldb_test(ospl_uint32 i, void *pArg)
{
	//struct vty *vty = pArg;
	int pValue = 0;
	sqlite3 * db = v9_user_sqldb_init(2);
	if (db)
	{
		if (v9_user_sqldb_table_exist(db, 2) != ospl_true)
		{
			v9_user_sqldb_table_create(db, 2);
		}
		if (i == 1)
		{
			v9_user_sqldb_add(db, 2, 1, 1, "abc0", "1234567890",
					"/app/www/res/logo.png", NULL);
			v9_user_sqldb_add(db, 2, 0, 2, "abc1", "1234567891",
					"/app/www/res/logo.png", NULL);
			v9_user_sqldb_add(db, 2, 1, 2, "abc2", "1234567892",
					"/app/www/res/logo.png", NULL);
			v9_user_sqldb_add(db, 2, 0, 1, "abc3", "1234567893",
					"/app/www/res/logo.png", NULL);
			v9_user_sqldb_add(db, 2, 0, 1, "abc4", "1234567894",
					"/app/www/res/logo.png", NULL);

			v9_user_sqldb_count(db, 2, -1, &pValue);
		}
		else if (i == 2)
		{
			v9_user_sqldb_update(db, 2, 0, 2, "abc0", "1234567890",
					"/app/www/res/logo.png", NULL);
			v9_user_sqldb_update(db, 2, 1, 3, "abc1", "1234567891",
					"/app/www/res/logo.png", NULL);
			v9_user_sqldb_update(db, 2, 0, 1, "abc2gg", "1234567892",
					"/app/www/res/logo.png", NULL);
		}
		else if (i == 3)
		{
			// 获取图片特征向量
			/*			LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetPictureFeature(int p_nLoginHandle, const ST_SDKPictureInfo* p_pstPictureInfo, ST_SDKSnapFea* p_pstSnapFea);

			 // 获取图片特征向量
			 LIB_SDK_DLL_EXPORT int __stdcall EAIS_SDK_GetPictureFeature(int p_nLoginHandle, const ST_SDKPictureInfo* p_pstPictureInfo, ST_SDKSnapFea* p_pstSnapFea);

			 v9_user_sqldb_update(db, 1, 0, 2, "abc0", "1234567890", "/app/www/res/logo.png");
			 v9_user_sqldb_update(db, 1, 1, 3, "abc1", "1234567891", "/app/www/res/logo.png");
			 v9_user_sqldb_update(db, 1, 0, 1, "abc2gg", "1234567892", "/app/www/res/logo.png");*/
		}

		else if (i == 4)
		{
			v9_user_sqldb_del(db, 2, "1234567893");
			v9_user_sqldb_del(db, 2, "1234567894");
		}
		else
		{
			//v9_user_sqldb_show(db, 1, "1234567890", vty);

			//vty_out(vty, "======================%s", VTY_NEWLINE);
			//v9_user_sqldb_show(db, 1, "1234567890", vty);
		}
		v9_user_sqldb_sync(db, 2);
		v9_user_sqldb_exit(db);
	}
	return OK;
}
#endif
