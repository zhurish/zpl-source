#include "zebra.h"
#include "zmemory.h"
#include "command.h"
#include "zmemory.h"
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

static void 		*_db_cap_mutex = NULL;


void v9_video_db_lock()
{
	if(_db_cap_mutex)
	{
		os_mutex_lock(_db_cap_mutex, OS_WAIT_FOREVER);
	}
}
void v9_video_db_unlock()
{
	if(_db_cap_mutex)
	{
		os_mutex_unlock(_db_cap_mutex);
	}
}

static char * v9_video_sqldb_file(zpl_uint32 id)
{
	static char dirbase[V9_APP_DIR_NAME_MAX];
	memset(dirbase, 0, sizeof(dirbase));
	snprintf(dirbase, sizeof(dirbase), "%s/%s", v9_video_disk_db_dir(id), V9_VIDEO_DB_FILE);
    //snprintf(dirbase, sizeof(dirbase), V9_VIDEO_DB_FILE, id);
    return dirbase;
}

static sqlite3 * v9_video_sqldb_init(zpl_uint32 id)
{
	int ret = 0;
	sqlite3 *db = NULL;
	if(os_file_access(v9_video_sqldb_file(id)) != OK)
	{
		FILE *fp = fopen(v9_video_sqldb_file(id), "ab+");
		if(fp)
			fclose(fp);
	}
	ret = sqlite3_open(v9_video_sqldb_file(id), &db);
	if( ret != SQLITE_OK )
	{
		if(V9_SQLDB_DEBUG(MSG))
			zlog_err(MODULE_APP, "Can't open database: %s(%s)", v9_video_sqldb_file(id), sqlite3_errmsg(db));
		sqlite3_close(db);
		return NULL;
	}
	return db;
}

static  int v9_video_sqldb_exit(sqlite3 *db)
{
	if( db )
	{
		sqlite3_close(db);
		return OK;
	}
	return ERROR;
}

static  int v9_video_sqldb_sync(sqlite3 *db, zpl_uint32 id)
{
	if( db )
	{
		sqlite3_db_cacheflush(db);
		return OK;
	}
	return ERROR;
}

static zpl_bool v9_video_sqldb_table_exist(sqlite3 *db, zpl_uint32 table)
{
	char sqlcmd[256];
	char *zErrMsg =NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	if(table == 0)
		snprintf(sqlcmd, sizeof(sqlcmd), "select * from "V9_VIDEO_SNAP_TBL_NAME";");
	else
		snprintf(sqlcmd, sizeof(sqlcmd), "select * from "V9_VIDEO_WARN_TBL_NAME";");
	if(sqlite3_exec(db, sqlcmd, NULL, NULL, &zErrMsg) == SQLITE_OK)
		return zpl_true;
	if(V9_SQLDB_DEBUG(MSG))
	{
		if(table == 0)
			zlog_err(MODULE_APP, " SQL Table '%s' exist check (%s)", V9_VIDEO_SNAP_TBL_NAME, zErrMsg);
		else
			zlog_err(MODULE_APP, " SQL Table '%s' exist check (%s)", V9_VIDEO_WARN_TBL_NAME, zErrMsg);
	}
	sqlite3_free(zErrMsg);
	return zpl_false;
}




static int v9_video_sqldb_table_create(sqlite3 *db, zpl_uint32 table)
{
	char sqlcmd[1024];
	char *zErrMsg =NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	if(table == 0)
		snprintf(sqlcmd, sizeof(sqlcmd), "CREATE TABLE "V9_VIDEO_SNAP_TBL_ST);
	else
		snprintf(sqlcmd, sizeof(sqlcmd), "CREATE TABLE "V9_VIDEO_WARN_TBL_ST);
	if(sqlite3_exec(db, sqlcmd, NULL, NULL, &zErrMsg) == SQLITE_OK)
		return OK;
	if(V9_SQLDB_DEBUG(MSG))
	{
		if(table == 0)
			zlog_err(MODULE_APP, " SQL Create Table '%s'(%s)", V9_VIDEO_SNAP_TBL_NAME, zErrMsg);
		else
			zlog_err(MODULE_APP, " SQL Create Table '%s'(%s)", V9_VIDEO_WARN_TBL_NAME, zErrMsg);
	}
	sqlite3_free(zErrMsg);
	return ERROR;
}


int v9_video_sqldb_load(zpl_uint32 id)
{
	if(!_db_cap_mutex)
	{
		_db_cap_mutex = os_mutex_name_init("_db_cap_mutex");
	}
	return OK;
}

sqlite3 * v9_video_sqldb_open(zpl_uint32 id, zpl_uint32 table)
{
	sqlite3 *db = NULL;
	if(db == NULL)
		db = v9_video_sqldb_init( id);
	if(!db)
		return NULL;
	if(v9_video_sqldb_table_exist(db, table) != zpl_true)
	{
		v9_video_sqldb_table_create(db, table);
	}
	return db;
}

int v9_video_sqldb_close(sqlite3 *db, zpl_uint32 id)
{
	v9_video_sqldb_sync(db, id);
	v9_video_sqldb_exit(db);
	return OK;
}

int v9_video_sqldb_add_node(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, zpl_bool gender, int group,
							char *user, char *user_id, zpl_uint32 channel, zpl_uint32 age, char *pic, char *video)
{
	char sqlcmd[1024];
	char *zErrMsg =NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	if(table == 0)
    {
		snprintf(sqlcmd, sizeof(sqlcmd), "INSERT INTO "V9_VIDEO_SNAP_TBL_NAME " \
        		(channel,datetime,targettype,gender,age,picurl) VALUES( \
        		%d,\
        		'%s',\
        		%d,\
        		%d,\
        		%d,\
        		'%s'\
        		);", channel, os_time_fmt("sql", os_time(NULL)), 1, gender ? 1:0, age, pic);
    }
   else
    {
	   snprintf(sqlcmd, sizeof(sqlcmd), "INSERT INTO "V9_VIDEO_WARN_TBL_NAME " (userid,"
        		"username,gender,groupid,channel,datetime,similarity,targettype,picurl,videourl) VALUES( \
        		'%s',\
        		'%s',\
        		%d,\
        		%d,\
        		%d,\
        		'%s',\
        		%f,\
        		%d,\
        		'%s',\
        		'%s'\
        		);", user_id, user, gender ? 1:0, group, channel, os_time_fmt("sql", os_time(NULL)), 0.45, 1, pic, video);
    }

    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
	if(sqlite3_exec(db, sqlcmd, NULL, NULL, &zErrMsg) == SQLITE_OK)
		return OK;

	if(V9_SQLDB_DEBUG(MSG))
		zlog_err(MODULE_APP, " SQL INSERT Info(%s)", zErrMsg);

	sqlite3_free(zErrMsg);
	return ERROR;
}


int v9_video_sqldb_keyvalue_update(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, zpl_uint32 nid, sql_snapfea_key *key)
{
	char sqlcmd[512];
	sqlite3_stmt *stmt = NULL;
	zpl_uint32 tiid = nid;
	memset (sqlcmd, 0, sizeof(sqlcmd));

	if(nid <= 0)
		tiid = (zpl_uint32)sqlite3_last_insert_rowid(db);

	if(table == 0)
	{
		snprintf(sqlcmd, sizeof(sqlcmd), "UPDATE "V9_VIDEO_SNAP_TBL_NAME " SET \
    		keyvalue=? \
    		WHERE id = %d;", tiid);
	}
	else
	{
		snprintf(sqlcmd, sizeof(sqlcmd), "UPDATE "V9_VIDEO_WARN_TBL_NAME " SET \
    		keyvalue=? \
    		WHERE id = %d;", tiid);
	}

    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

	if (sqlite3_prepare (db, sqlcmd, strlen (sqlcmd), &stmt, NULL) != SQLITE_OK)
	{
		if(V9_SQLDB_DEBUG(MSG))
			zlog_err(MODULE_APP, " SQL Prepare fail(%s)", sqlite3_errmsg (db));
		sqlite3_finalize (stmt);
		return ERROR;
	}
	if (sqlite3_bind_blob (stmt, 1, key->feature.feature_data,
						   key->feature_len * sizeof(zpl_float),
						   SQLITE_STATIC) != SQLITE_OK)
	{
		if(V9_SQLDB_DEBUG(MSG))
			zlog_err(MODULE_APP, " SQL Bind fail(%s)", sqlite3_errmsg (db));
		sqlite3_finalize (stmt);
		return ERROR;
	}
	if (sqlite3_step (stmt) == SQLITE_DONE)
	{
		sqlite3_finalize (stmt);
		return OK;
	}
	if(V9_SQLDB_DEBUG(MSG))
		zlog_err(MODULE_APP, " SQL Step fail(%s)", sqlite3_errmsg (db));
	sqlite3_finalize (stmt);
	return ERROR;
}


static int v9_video_sqldb_count_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
   zpl_uint32 i;
	int *value = NotUsed;
   for(i=0; i<argc; i++)
	{
	   if(value && argv[i])
		   *value = atoi(argv[i]);
	}
   return 0;
}

/*
 * 获取表格行数
 */
int v9_video_sqldb_count(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, int *pValue)
{
	int value = 0;
	char sqlcmd[512];
	char *zErrMsg =NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	if(table == 0)
    {
		snprintf(sqlcmd, sizeof(sqlcmd), "SELECT count(*) FROM "V9_VIDEO_SNAP_TBL_NAME ";");
    }
	else
    {
    	snprintf(sqlcmd, sizeof(sqlcmd), "SELECT count(*) FROM "V9_VIDEO_WARN_TBL_NAME ";");
    }
	if(sqlite3_exec(db, sqlcmd, v9_video_sqldb_count_callback, &value, &zErrMsg) == SQLITE_OK)
	{
		if(pValue)
			*pValue = value;
		return OK;
	}
	if(V9_SQLDB_DEBUG(MSG))
	{
		if(table == 0)
			zlog_err(MODULE_APP, " SQL SELECT count on Table '%s'(%s)", V9_VIDEO_SNAP_TBL_NAME, zErrMsg);
		else
			zlog_err(MODULE_APP, " SQL SELECT count on Table '%s'(%s)", V9_VIDEO_WARN_TBL_NAME, zErrMsg);
	}
	sqlite3_free(zErrMsg);
	return ERROR;
}





/*
 * 删除
 */
//删除小于某个时间的记录
static int v9_video_sqldb_delete_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	zpl_uint32 i = 0, j = 0, k = 0;
	zpl_uint32 *bid = NotUsed;
	if(!bid || !(*bid))
		return 0;
	for (i = 0; i < argc; i++)
	{
		if (azColName[i] && strcasecmp (azColName[i], "username") == 0)
		{
			k = 1;
		}
		if (azColName[i] && ( (strcasecmp (azColName[i], "picurl") == 0) || (strcasecmp (azColName[i], "videourl") == 0) ) )
		{
			if (argv[i])
			{
				if (strncasecmp (argv[i], "/nfsroot/", 6) == 0)
				{
					v9_video_disk_urlpath(*bid, argv[i] + strlen ("/nfsroot/"));
				}
				else
				{
					remove(argv[i]);
				}
			}
			j++;
			if(strcasecmp (azColName[i], "videourl") == 0)
				j++;
		}
		if(k == 1 && j == 2)
			break;
		else
		{
			if(j == 1)
				break;
		}
	}
	return 0;
}

int v9_video_sqldb_del_by_datetime(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, char *datetime)
{
	char sqlcmd[256];
	char *zErrMsg =NULL;
	zpl_uint32 bid = id;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	if(table == 0)
		snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_SNAP_TBL_NAME " \
    		WHERE datetime <= '%s';", datetime);
	else
		snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_WARN_TBL_NAME " \
    		WHERE datetime <= '%s';", datetime);

    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
	if(sqlite3_exec(db, sqlcmd, v9_video_sqldb_delete_callback, &bid, &zErrMsg) == SQLITE_OK)
	{
		sync();
		return OK;
	}
	if(V9_SQLDB_DEBUG(MSG))
	{
		if(table == 0)
			zlog_err(MODULE_APP, " SQL DELETE by datetime(%s) on Table '%s'(%s)", datetime, V9_VIDEO_SNAP_TBL_NAME, zErrMsg);
		else
			zlog_err(MODULE_APP, " SQL DELETE by datetime(%s) on Table '%s'(%s)", datetime, V9_VIDEO_WARN_TBL_NAME, zErrMsg);
	}
	sqlite3_free(zErrMsg);
	return ERROR;
}

int v9_video_sqldb_del_by_channel(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, zpl_uint32 channel)
{
	char sqlcmd[256];
	char *zErrMsg =NULL;
	zpl_uint32 bid = id;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	if(table == 0)
		snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_SNAP_TBL_NAME " \
    		WHERE channel = %d;", channel);
	else
    	snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_WARN_TBL_NAME " \
    		WHERE channel = %d;", channel);
    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
	if(sqlite3_exec(db, sqlcmd, v9_video_sqldb_delete_callback, &bid, &zErrMsg) == SQLITE_OK)
	{
		sync();
		return OK;
	}
	if(V9_SQLDB_DEBUG(MSG))
	{
		if(table == 0)
			zlog_err(MODULE_APP, " SQL DELETE by channel(%d) on Table '%s'(%s)", channel, V9_VIDEO_SNAP_TBL_NAME, zErrMsg);
		else
			zlog_err(MODULE_APP, " SQL DELETE by channel(%d) on Table '%s'(%s)", channel, V9_VIDEO_WARN_TBL_NAME, zErrMsg);
	}
	sqlite3_free(zErrMsg);
	return ERROR;
}

int v9_video_sqldb_del_by_gender(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, zpl_bool gender)
{
	char sqlcmd[256];
	char *zErrMsg =NULL;
	zpl_uint32 bid = id;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	if(table == 0)
    	snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_SNAP_TBL_NAME " \
    		WHERE gender = %d;", gender);
	else
    	snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_WARN_TBL_NAME " \
    		WHERE gender = %d;", gender);

    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
	if(sqlite3_exec(db, sqlcmd, v9_video_sqldb_delete_callback, &bid, &zErrMsg) == SQLITE_OK)
	{
		sync();
		return OK;
	}
	if(V9_SQLDB_DEBUG(MSG))
	{
		if(table == 0)
			zlog_err(MODULE_APP, " SQL DELETE by gender(%d) on Table '%s'(%s)", gender, V9_VIDEO_SNAP_TBL_NAME, zErrMsg);
		else
			zlog_err(MODULE_APP, " SQL DELETE by gender(%d) on Table '%s'(%s)", gender, V9_VIDEO_WARN_TBL_NAME, zErrMsg);
	}
	sqlite3_free(zErrMsg);
	return ERROR;
}

int v9_video_sqldb_del_by_userid(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, char * userid)
{
	char sqlcmd[256];
	char *zErrMsg =NULL;
	zpl_uint32 bid = id;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	if(table == 0)
		snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_SNAP_TBL_NAME " \
    		WHERE userid = '%s';", userid);
	else
    	snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_WARN_TBL_NAME " \
    		WHERE userid = '%s';", userid);

    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
	if(sqlite3_exec(db, sqlcmd, v9_video_sqldb_delete_callback, &bid, &zErrMsg) == SQLITE_OK)
	{
		sync();
		return OK;
	}
	if(V9_SQLDB_DEBUG(MSG))
	{
		if(table == 0)
			zlog_err(MODULE_APP, " SQL DELETE by userid(%s) on Table '%s'(%s)", userid, V9_VIDEO_SNAP_TBL_NAME, zErrMsg);
		else
			zlog_err(MODULE_APP, " SQL DELETE by userid(%s) on Table '%s'(%s)", userid, V9_VIDEO_WARN_TBL_NAME, zErrMsg);
	}
	sqlite3_free(zErrMsg);
	return ERROR;
}

int v9_video_sqldb_del_by_index_id(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, zpl_uint32 start, zpl_uint32 end)
{
	char sqlcmd[256];
	char *zErrMsg =NULL;
	zpl_uint32 bid = id;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	if(table == 0)
    {
    	if(start && end && (start < end))
        	snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_SNAP_TBL_NAME " \
        		WHERE id IN ( %d, %d );", start, end);
    	else
    		snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_SNAP_TBL_NAME " \
    		    WHERE id = %d;", start);
    }
    else
    {
    	if(start && end && (start < end))
        	snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_WARN_TBL_NAME " \
        		WHERE id IN ( %d, %d );", start, end);
    	else
    		snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_WARN_TBL_NAME " \
    		    WHERE id = %d;", start);
    }

    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
	if(sqlite3_exec(db, sqlcmd, v9_video_sqldb_delete_callback, &bid, &zErrMsg) == SQLITE_OK)
	{
		sync();
		return OK;
	}
	if(V9_SQLDB_DEBUG(MSG))
	{
		if(table == 0)
			zlog_err(MODULE_APP, " SQL DELETE by start(%d-%d) on Table '%s'(%s)", start, end, V9_VIDEO_SNAP_TBL_NAME, zErrMsg);
		else
			zlog_err(MODULE_APP, " SQL DELETE by start(%d-%d) on Table '%s'(%s)", start, end, V9_VIDEO_WARN_TBL_NAME, zErrMsg);
	}
	sqlite3_free(zErrMsg);
	return ERROR;
}



int v9_video_sqldb_select_by_oldid(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, zpl_uint32 limit)
{
	char sqlcmd[1024];
	zpl_uint32 bid = id;
	char *zErrMsg =NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));

	if(table == 0)
    {
		//DELETE FROM snapcaptable WHERE id IN ( SELECT id FROM snapcaptable ORDER BY id ASC LIMIT 5)
		//根据ID升序排列返回有限个结果
    	snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_SNAP_TBL_NAME " WHERE id IN ("
				 "SELECT id FROM "V9_VIDEO_SNAP_TBL_NAME " ORDER BY id ASC LIMIT %d);", limit);
    }
	else
    {
    	snprintf(sqlcmd, sizeof(sqlcmd), "DELETE FROM "V9_VIDEO_WARN_TBL_NAME " WHERE id IN ("
				 "SELECT id FROM "V9_VIDEO_WARN_TBL_NAME " ORDER BY id ASC LIMIT %d);", limit);
    }

    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

	if(sqlite3_exec(db, sqlcmd, v9_video_sqldb_delete_callback, &bid, &zErrMsg) == SQLITE_OK)
	{
		sync();
		return OK;
	}
	if(V9_SQLDB_DEBUG(MSG))
	{
		if(table == 0)
			zlog_err(MODULE_APP, " SQL DELETE by start count(%d) on Table '%s'(%s)", limit, V9_VIDEO_SNAP_TBL_NAME, zErrMsg);
		else
			zlog_err(MODULE_APP, " SQL DELETE by start count(%d) on Table '%s'(%s)", limit, V9_VIDEO_WARN_TBL_NAME, zErrMsg);
	}
	sqlite3_free(zErrMsg);
	return ERROR;
}














/*
 * 查询
 */
static int v9_video_sqldb_select_sqlcmd_by_keywork(v9_cap_keywork_t *keywork, char *sqlcmd, int maxsize)
{
	zpl_uint32 offset = 0, i = 0;
	char *sqlstr = sqlcmd;
	if (keywork->limit)
	{
		if (keywork->table == 0)
		{
			snprintf (
					sqlstr + offset,
					maxsize - offset,
					"SELECT id,channel FROM "V9_VIDEO_SNAP_TBL_NAME " WHERE id IN ( ");
		}
		else
		{
			snprintf (
					sqlstr + offset,
					maxsize - offset,
					"SELECT id,channel FROM "V9_VIDEO_WARN_TBL_NAME " WHERE id IN ( ");
		}
		offset = strlen (sqlstr);
		if (keywork->table == 0)
		{
			snprintf (sqlstr + offset, maxsize - offset,
					  "SELECT id FROM "V9_VIDEO_SNAP_TBL_NAME " WHERE ");
		}
		else
		{
			snprintf (sqlstr + offset, maxsize - offset,
					  "SELECT id FROM "V9_VIDEO_WARN_TBL_NAME " WHERE ");
		}
	}
	else
	{
		if (keywork->table == 0)
		{
			snprintf (
					sqlstr + offset, maxsize - offset,
					"SELECT id,channel FROM "V9_VIDEO_SNAP_TBL_NAME " WHERE ");
		}
		else
		{
			snprintf (
					sqlstr + offset, maxsize - offset,
					"SELECT id,channel FROM "V9_VIDEO_WARN_TBL_NAME " WHERE ");
		}
	}

	if(offset >= maxsize)
		return ERROR;
	offset = strlen (sqlstr);
	if (keywork->channel != 0)
	{
		snprintf (sqlstr + offset, maxsize - offset, " channel=%d",
				  keywork->channel - 1);
		i++;
	}
	offset = strlen (sqlstr);
	if (keywork->gender == 1 || keywork->gender == 2)
	{
		snprintf (sqlstr + offset, maxsize - offset, " %s gender=%d",
				  i ? "and" : " ", keywork->gender - 1);
		i++;
	}
	if(offset >= maxsize)
		return ERROR;
	//offset = strlen(sqlstr);
	//if(keywork->group == 0||keywork->group == 1)
	//	snprintf(sqlstr + offset, maxsize - offset, " groupid=%d and", keywork->group);

	offset = strlen (sqlstr);
	if (keywork->starttime != 0)
	{
		snprintf (sqlstr + offset, maxsize - offset,
				  " %s datetime >= '%s'", i ? "and" : " ",
				  os_time_fmt ("sql", keywork->starttime));
		i++;
	}
	if(offset >= maxsize)
		return ERROR;
	offset = strlen (sqlstr);
	if (keywork->endtime != 0)
	{
		snprintf (sqlstr + offset, maxsize - offset,
				  " %s datetime <= '%s'", i ? "and" : " ",
				  os_time_fmt ("sql", keywork->endtime));
		i++;
	}
	offset = strlen (sqlstr);
	if(offset >= maxsize)
		return ERROR;
	if (keywork->age != 0)
	{
		snprintf (sqlstr + offset, maxsize - offset, " %s age=%d",
				  i ? "and" : " ", keywork->age - 1);
		i++;
	}
	if (keywork->table != 0)
	{
		offset = strlen (sqlstr);
		if (keywork->group != 0)
		{
			snprintf (sqlstr + offset, maxsize - offset,
					  " %s groupid=%d", i ? "and" : " ", keywork->group - 1);
			i++;
		}

		offset = strlen (sqlstr);
		if(offset >= maxsize)
			return ERROR;
		if (strlen (keywork->username))
		{
			snprintf (sqlstr + offset, maxsize - offset,
					  " %s username='%s'", i ? "and" : " ", keywork->username);
			i++;
		}

		offset = strlen (sqlstr);
		if(offset >= maxsize)
			return ERROR;
		if (strlen (keywork->userid))
		{
			snprintf (sqlstr + offset, maxsize - offset,
					  " %s userid='%s'", i ? "and" : " ", keywork->userid);
			i++;
		}
	}
	if (keywork->limit)
	{
		offset = strlen (sqlstr);
		if(offset >= maxsize)
			return ERROR;
		snprintf (sqlstr + offset, maxsize - offset,
				  ") ORDER BY id DESC LIMIT %d", keywork->limit);
	}
	strcat (sqlstr, ";");
	if(offset >= maxsize)
		return ERROR;
	return OK;
}

int v9_video_sqldb_select_count_by_keywork(sqlite3 *db, v9_cap_keywork_t *keywork, int *pValue)
{
	int value = 0;
	char sqlcmd[1024];
	char *zErrMsg = NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));

	value = v9_video_sqldb_select_sqlcmd_by_keywork(keywork, sqlcmd, sizeof(sqlcmd));
	if(value != OK)
	{
		zlog_err(MODULE_APP, " SQL Format SQL CMD fail");
		return ERROR;
	}
	if (keywork->limit)
	{
		if(pValue)
			*pValue = 8192;
		return OK;
	}
	//id,channel SELECT id,channel
	memset(sqlcmd, 0, strlen("SELECT id,channel"));
	memcpy(sqlcmd, "SELECT  count(*) ", strlen("SELECT  count(*) "));

    //if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

	if(sqlite3_exec(db, sqlcmd, v9_video_sqldb_count_callback, &value, &zErrMsg) == SQLITE_OK)
	{
		if(pValue)
			*pValue = value;
		//printf("=======================%s============================value=%d sqlcmd=%s\r\n", __func__, value, sqlcmd);
		return OK;
	}
	if(V9_SQLDB_DEBUG(MSG))
	{
		if(keywork->table == 0)
			zlog_err(MODULE_APP, " SQL SELECT count on Table '%s'(%s)", V9_VIDEO_SNAP_TBL_NAME, zErrMsg);
		else
			zlog_err(MODULE_APP, " SQL SELECT count on Table '%s'(%s)", V9_VIDEO_WARN_TBL_NAME, zErrMsg);
	}
	sqlite3_free(zErrMsg);
	return ERROR;
}
/*
 * 根据条件获取ID表
 */
int v9_video_sqldb_select_by_keywork(sqlite3 *db, v9_cap_keywork_t *keywork,
									 zpl_uint32 *outid, zpl_uint32 *cnt)
{
	int ret = 0, i = 0;
	char sqlcmd[1024];
	sqlite3_stmt *stmt = NULL;
	zpl_uint32 len = 0;
	void *pReadBolbData = NULL;
	//zpl_float fresult = 0.0f;
	memset(sqlcmd, 0, sizeof(sqlcmd));

	ret = v9_video_sqldb_select_sqlcmd_by_keywork(keywork, sqlcmd, sizeof(sqlcmd));
	if(ret != OK)
	{
		zlog_err(MODULE_APP, " SQL Format SQL CMD fail");
		return ERROR;
	}
	//select * from student where sex='男' and age=20;
    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

	if (sqlite3_prepare(db, sqlcmd, strlen(sqlcmd), &stmt, NULL) != SQLITE_OK)
    {
    	//if(V9_SQLDB_DEBUG(DB))
    		zlog_err(MODULE_APP, " SQL Prepare fail(%s)", sqlite3_errmsg (db));
        return ERROR;
    }
	i = 0;
    ret = sqlite3_step(stmt);
 	while(ret == SQLITE_ROW)
 	{
 		len = sqlite3_column_bytes(stmt, 0);
 		pReadBolbData = sqlite3_column_blob(stmt, 0);
		if(outid && len && pReadBolbData)
		{
			outid[i++] = atoi(pReadBolbData);
		}
		if(cnt && i >= *cnt)
			break;
 		ret = sqlite3_step(stmt);
    }

 	sqlite3_finalize(stmt);

 	if(i == 0)
 		return ERROR;
 	if(cnt)
 		*cnt = i;
 	return OK;
}



int v9_video_sqldb_select_by_new(sqlite3 *db, v9_cap_keywork_t *keywork,
									 zpl_uint32 *outid, zpl_uint32 *cnt)
{
	int ret = 0, i = 0;//, offset = 0;
	char sqlcmd[1024];
	sqlite3_stmt *stmt = NULL;
	zpl_uint32 len = 0;
	void *pReadBolbData = NULL;
	//zpl_float fresult = 0.0f;
	memset(sqlcmd, 0, sizeof(sqlcmd));

	if(keywork->table == 0)
    {
		//根据ID降序排列返回有限个结果
    	snprintf(sqlcmd, sizeof(sqlcmd), "SELECT id,channel FROM "V9_VIDEO_SNAP_TBL_NAME " ORDER BY id DESC LIMIT %d;", keywork->limit);
    }
	else
    {
		snprintf(sqlcmd, sizeof(sqlcmd), "SELECT id,channel FROM "V9_VIDEO_WARN_TBL_NAME " ORDER BY id DESC LIMIT %d;",  keywork->limit);
    }

    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);

	if (sqlite3_prepare(db, sqlcmd, strlen(sqlcmd), &stmt, NULL) != SQLITE_OK)
    {
    	//if(V9_SQLDB_DEBUG(DB))
    		zlog_err(MODULE_APP, " SQL Prepare fail(%s)", sqlite3_errmsg (db));
        return ERROR;
    }
	i = 0;
    ret = sqlite3_step(stmt);
 	while(ret == SQLITE_ROW)
 	{
 		len = sqlite3_column_bytes(stmt, 0);
 		pReadBolbData = sqlite3_column_blob(stmt, 0);
		if(outid && len && pReadBolbData)
		{
			outid[i++] = atoi(pReadBolbData);
		}
 		ret = sqlite3_step(stmt);
    }

 	sqlite3_finalize(stmt);

 	if(i == 0)
 		return ERROR;
 	if(cnt)
 		*cnt = i;
 	return OK;
}
/*
 * 根据特征值获取ID表
 */
int v9_video_sqldb_select_by_keyvalue(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, zpl_uint32 limit, sql_snapfea_key *key, zpl_uint32 *outid, zpl_uint32 *cnt)
{
	int ret = 0, i = 0, offset = 0;
    char sqlcmd[512];
    sqlite3_stmt *stmt = NULL;
	zpl_uint32 len = 0;
	void *pReadBolbData = NULL;
	zpl_float fresult = 0.0f;
    memset(sqlcmd, 0, sizeof(sqlcmd));
	if(limit)
	{
		if(table == 0)
		{
			snprintf(sqlcmd + offset, sizeof(sqlcmd) - offset,
					 "SELECT id,channel FROM "V9_VIDEO_SNAP_TBL_NAME " WHERE id IN ( ");
		}
		else
		{
			snprintf(sqlcmd + offset, sizeof(sqlcmd) - offset,
					 "SELECT id,channel FROM "V9_VIDEO_WARN_TBL_NAME " WHERE id IN ( ");
		}
		offset = strlen(sqlcmd);
		if(table == 0)
		{
			snprintf(sqlcmd + offset, sizeof(sqlcmd) - offset, "SELECT id FROM "V9_VIDEO_SNAP_TBL_NAME);
		}
		else
		{
			snprintf(sqlcmd + offset, sizeof(sqlcmd) - offset, "SELECT id FROM "V9_VIDEO_WARN_TBL_NAME);
		}
	}
	else
	{
		if(table == 0)
		{
			snprintf(sqlcmd + offset, sizeof(sqlcmd) - offset, "SELECT id,keyvalue FROM "V9_VIDEO_SNAP_TBL_NAME);
		}
		else
		{
			snprintf(sqlcmd + offset, sizeof(sqlcmd) - offset, "SELECT id,keyvalue FROM "V9_VIDEO_WARN_TBL_NAME);
		}
	}
	if(limit)
	{
		offset = strlen(sqlcmd);
		snprintf(sqlcmd + offset, sizeof(sqlcmd) - offset, ") ORDER BY id DESC LIMIT %d", limit);
	}
	strcat(sqlcmd, ";");

    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
    if (sqlite3_prepare(db, sqlcmd, strlen(sqlcmd), &stmt, NULL) != SQLITE_OK)
    {
    	//if(V9_SQLDB_DEBUG(DB))
    		zlog_err(MODULE_APP, " SQL Prepare fail(%s)", sqlite3_errmsg (db));
        return ERROR;
    }

    ret = sqlite3_step(stmt);
 	while(/*ret == SQLITE_DONE ||*/ ret == SQLITE_ROW)
 	{
 		len = sqlite3_column_bytes(stmt, 1);
 		pReadBolbData = sqlite3_column_blob(stmt, 1);
 		if(key && len && pReadBolbData)
 		{
			// 获取特征值余弦相似度
 			if(key->feature_memcmp)
 			{
 				ret = (key->feature_memcmp)(key->feature.feature_data, pReadBolbData, MIN(len/sizeof(zpl_float),key->feature_len) , &fresult);
 				if( ret == OK)
 				{
 					//zlog_debug(MODULE_APP, "======================%s:keyw->key.input_value=%f\r\n",__func__, key->input_value);
 					if((double)fabs((double)fresult) >= (double)(key->input_value))
 					{
 						key->output_result = fresult;
 				 		len = sqlite3_column_bytes(stmt, 0);
 						pReadBolbData = sqlite3_column_blob(stmt, 0);
 						if(outid && key && len && pReadBolbData)
 						{
 							outid[i++] = atoi(pReadBolbData);
 						}
 					}
 				}
 			}
 			else
 			{
 				zlog_warn(MODULE_APP, " feature cmp func is null");
 			}
 		}
		if(cnt && i >= *cnt)
			break;
 		ret = sqlite3_step(stmt);
    }

 	sqlite3_finalize(stmt);
 	if(i == 0)
 		return ERROR;
 	if(cnt)
 		*cnt = i;
 	return OK;
}

/*
 * 取特征值
 */
int v9_video_sqldb_get_keyvalue(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, zpl_uint32 inid, sql_snapfea_key *key)
{
	int ret = 0;
    char sqlcmd[512];
    sqlite3_stmt *stmt = NULL;
    memset(sqlcmd, 0, sizeof(sqlcmd));

    if(table == 0)
    {
    	snprintf(sqlcmd, sizeof(sqlcmd), "SELECT id,keyvalue FROM "V9_VIDEO_SNAP_TBL_NAME " WHERE id = %d;", inid);
    }
    else
    {
		snprintf(sqlcmd, sizeof(sqlcmd), "SELECT id,keyvalue FROM "V9_VIDEO_WARN_TBL_NAME " WHERE id = %d;", inid);
    }
    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
    if (sqlite3_prepare(db, sqlcmd, strlen(sqlcmd), &stmt, NULL) != SQLITE_OK)
    {
    	if(V9_SQLDB_DEBUG(DB))
    		zlog_err(MODULE_APP, " SQL Prepare fail(%s)", sqlite3_errmsg (db));
        return ERROR;
    }
    ret = sqlite3_step(stmt);
 	if(/*ret == SQLITE_DONE || */ret == SQLITE_ROW)
 	{
 		zpl_uint32 len = sqlite3_column_bytes(stmt, 1);
 		void *pReadBolbData = sqlite3_column_blob(stmt, 1);

 		if(key && len && pReadBolbData)
 		{
			key->feature_len = len/sizeof(zpl_float);
			//key->feature.feature_data = key->feature.ckey_data = XMALLOC(MTYPE_VIDEO_KEY,len + 1);
			if(len > APP_FEATURE_MAX)
			{
				key->feature.feature_data = key->feature.ckey_data = XREALLOC(MTYPE_VIDEO_KEY,
					key->feature.ckey_data, len + sizeof(zpl_float));									// 特征值
			}
			if(key->feature.ckey_data && pReadBolbData)
			{
				memcpy(key->feature.feature_data, pReadBolbData, len);
			}
	        sqlite3_finalize(stmt);
	        return OK;
 		}
     }
	if(V9_SQLDB_DEBUG(DB))
		zlog_debug(MODULE_APP, " SQL Step Finalize(%s)", sqlite3_errmsg (db));
 	sqlite3_finalize(stmt);
	return ERROR;
}

/*
 * 获取行的列元素
 */
static int v9_video_sqldb_get_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	zpl_uint32 i;
	v9_video_cap_t *user = NotUsed;
	for (i = 0; i < argc; i++)
	{
		if (azColName[i] && strcasecmp (azColName[i], "id")==0)
		{
			if (argv[i])
				user->keyid = atoi (argv[i]);
		}
		else if (azColName[i] && strcasecmp (azColName[i], "channel")==0)
		{
			if (argv[i])
				user->channel = atoi (argv[i]);
		}
		else if (azColName[i] && strcasecmp (azColName[i], "datetime")==0)
		{
			if (argv[i])
				strcpy (user->datetime, argv[i]);
				//strncpy (user->datetime, argv[i], 16/*strlen("2020-05-05 11:15")*/);
		}
		else if (azColName[i] && strcasecmp (azColName[i], "gender")==0)
		{
			if (argv[i])
				user->gender = atoi (argv[i]);
		}
		else if (azColName[i] && strcasecmp (azColName[i], "age")==0)
		{
			if (argv[i])
				user->age = atoi (argv[i]);
		}
		else if (azColName[i] && strcasecmp (azColName[i], "mood")==0)
		{
			if (argv[i])
				strcpy (user->mood, argv[i]);
		}
		else if (azColName[i] && strcasecmp (azColName[i], "complexion")==0)
		{
			if (argv[i])
				strcpy (user->complexion, argv[i]);
		}
		else if (azColName[i] && strcasecmp (azColName[i], "minority")==0)
		{
			if (argv[i])
				strcpy (user->minority, argv[i]);
		}
		else if (azColName[i] && strcasecmp (azColName[i], "picurl")==0)
		{
			if (argv[i])
			{
				if(strncasecmp(argv[i], "/nfsroot/", 6)==0)
				{
					strcpy (user->picname, argv[i] + strlen("/nfsroot/"));
				}
				else
				{
					strcpy (user->picname, argv[i]);
				}
			}
		}
		if (user->table != 0)
		{
			if (azColName[i] && strcasecmp (azColName[i], "username")==0)
			{
				if (argv[i])
					strcpy (user->username, argv[i]);
			}
			else if (azColName[i] && strcasecmp (azColName[i], "userid")==0)
			{
				if (argv[i])
					strcpy (user->userid, argv[i]);
			}
			else if (azColName[i] && strcasecmp (azColName[i], "groupid")==0)
			{
				if (argv[i])
					user->group = atoi (argv[i]);
			}
			else if (azColName[i] && strcasecmp (azColName[i], "videourl")==0)
			{
				if (argv[i])
				{
					if(strncasecmp(argv[i], "/nfsroot/", 6)==0)
					{
						strcpy (user->videoname, argv[i] + strlen("/nfsroot/"));
					}
					else
					{
						strcpy (user->videoname, argv[i]);
					}
				}
			}
			else if (azColName[i] && strcasecmp (azColName[i], "similarity")==0)
			{
				if (argv[i])
					user->key.output_result = atof (argv[i]);
			}
		}
	}
	return 0;
}

/*
 * 根据ID获取抓拍或告警视频信息
 */
int v9_video_sqldb_get_capinfo(sqlite3 *db,zpl_uint32 id, zpl_uint32 table, zpl_uint32 inid, void *pArgv)
{
	char sqlcmd[1024];
	char *zErrMsg =NULL;
	v9_video_cap_t *user = pArgv;
	memset(sqlcmd, 0, sizeof(sqlcmd));
	user->table = table;
	user->ID = id;
	if(table == 0)
    {
    	snprintf(sqlcmd, sizeof(sqlcmd), "SELECT id,channel,datetime,gender,age,mood,complexion,"
    			"minority,picurl FROM "V9_VIDEO_SNAP_TBL_NAME " WHERE id = %d;", inid);
        if(V9_SQLDB_DEBUG(DBCMD))
        	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
		if(sqlite3_exec(db, sqlcmd, v9_video_sqldb_get_callback, pArgv, &zErrMsg) == SQLITE_OK)
		{
			//if(v9_video_sqldb_get_keyvalue(db,  id,  table,  inid, &user->key) == SQLITE_OK)
				return OK;
/*			else
			{
				if(V9_SQLDB_DEBUG(MSG))
				{
					if(table == 0)
						zlog_err(MODULE_APP, " SQL DELETE by ID(%d) on Table '%s'(%s)", inid, V9_VIDEO_SNAP_TBL_NAME, sqlite3_errmsg (db));
					else
						zlog_err(MODULE_APP, " SQL DELETE by ID(%d) on Table '%s'(%s)", inid, V9_VIDEO_WARN_TBL_NAME, sqlite3_errmsg (db));
				}
				return ERROR;
			}*/
		}
    }
	else
    {
    	snprintf(sqlcmd, sizeof(sqlcmd), "SELECT id,username,userid,groupid,channel,"
    			"datetime,gender,similarity,age,mood,complexion,minority,picurl,"
    			"videourl FROM "V9_VIDEO_WARN_TBL_NAME " WHERE id = %d;", inid);
        if(V9_SQLDB_DEBUG(DBCMD))
        	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
		if(sqlite3_exec(db, sqlcmd, v9_video_sqldb_get_callback, pArgv, &zErrMsg) == SQLITE_OK)
		{
			//if(v9_video_sqldb_get_keyvalue(db,  id,  table,  inid, &user->key) == SQLITE_OK)
				return OK;
/*			else
			{
				if(V9_SQLDB_DEBUG(MSG))
				{
					if(table == 0)
						zlog_err(MODULE_APP, " SQL DELETE by ID(%d) on Table '%s'(%s)", inid, V9_VIDEO_SNAP_TBL_NAME, sqlite3_errmsg (db));
					else
						zlog_err(MODULE_APP, " SQL DELETE by ID(%d) on Table '%s'(%s)", inid, V9_VIDEO_WARN_TBL_NAME, sqlite3_errmsg (db));
				}
				return ERROR;
			}*/
		}
    }
	if(V9_SQLDB_DEBUG(MSG))
	{
		if(table == 0)
			zlog_err(MODULE_APP, " SQL DELETE by ID(%d) on Table '%s'(%s)", inid, V9_VIDEO_SNAP_TBL_NAME, zErrMsg);
		else
			zlog_err(MODULE_APP, " SQL DELETE by ID(%d) on Table '%s'(%s)", inid, V9_VIDEO_WARN_TBL_NAME, zErrMsg);
	}
	sqlite3_free(zErrMsg);
	return ERROR;
}


int v9_video_sqldb_select_by_userid(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, char *userid, zpl_uint32 *outid, zpl_uint32 *cnt)
{
	int ret = 0, i = 0;
	char sqlcmd[256];
	sqlite3_stmt *stmt = NULL;
	zpl_uint32 len = 0;
	void *pReadBolbData = NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));

	if(table == 0)
    {
    	snprintf(sqlcmd, sizeof(sqlcmd), "SELECT id,channel FROM "V9_VIDEO_SNAP_TBL_NAME " where userid='%s';", userid);
    }
	else
    {
		snprintf(sqlcmd, sizeof(sqlcmd), "SELECT id,channel FROM "V9_VIDEO_WARN_TBL_NAME " where userid='%s';", userid);
    }
    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
	if (sqlite3_prepare(db, sqlcmd, strlen(sqlcmd), &stmt, NULL) != SQLITE_OK)
    {
    	if(V9_SQLDB_DEBUG(DB))
    		zlog_err(MODULE_APP, " SQL Prepare fail(%s)", sqlite3_errmsg (db));
        return ERROR;
    }
   ret = sqlite3_step(stmt);
 	while(/*ret == SQLITE_DONE || */ret == SQLITE_ROW)
 	{
 		len = sqlite3_column_bytes(stmt, 0);
 		pReadBolbData = sqlite3_column_blob(stmt, 0);
 		if(outid && len && pReadBolbData)
 		{
 			outid[i++] = atoi(pReadBolbData);
 		}
 		ret = sqlite3_step(stmt);
	}
 	sqlite3_finalize(stmt);
 	if(i == 0)
 		return ERROR;
 	if(cnt)
 		*cnt = i;
 	return OK;
}





#ifdef V9_SQLDB_TEST
/*int v9_video_sqldb_del_by_index_id()
{
	sqlite3 * db = v9_video_sqldb_open(APP_BOARD_CALCU_1, tbl);
	char sqlcmd[256];
	char *zErrMsg =NULL;
	memset(sqlcmd, 0, sizeof(sqlcmd));

    		snprintf(sqlcmd, sizeof(sqlcmd), "update "V9_VIDEO_WARN_TBL_NAME "set similarity='%.f';");

    if(V9_SQLDB_DEBUG(DBCMD))
    	zlog_debug(MODULE_APP, "SQL:%s", sqlcmd);
	if(sqlite3_exec(db, sqlcmd, NULL, NULL, &zErrMsg) == SQLITE_OK)
	{
		sync();
		return OK;
	}
	if(V9_SQLDB_DEBUG(MSG))
	{
		if(table == 0)
			zlog_err(MODULE_APP, " SQL DELETE by start(%d-%d) on Table '%s'(%s)", start, end, V9_VIDEO_SNAP_TBL_NAME, zErrMsg);
		else
			zlog_err(MODULE_APP, " SQL DELETE by start(%d-%d) on Table '%s'(%s)", start, end, V9_VIDEO_WARN_TBL_NAME, zErrMsg);
	}
	sqlite3_free(zErrMsg);
	return ERROR;
}*/

int v9_video_sqldb_test(zpl_uint32 i, int tbl, void *pArg)
{
	sqlite3 * db = v9_video_sqldb_open(APP_BOARD_CALCU_1, tbl);
	if(db)
	{
		if(i == 1)
		{
			v9_video_sqldb_add_node(db, APP_BOARD_CALCU_1, tbl, 1, 1, "abc0", "1234567890", 1, 23, "/app/www/res/logo.png", "/app/www/res/logo1.png");
			v9_video_sqldb_add_node(db, APP_BOARD_CALCU_1, tbl, 0, 1, "abc1", "1234567891", 2, 43, "/app/www/res/logo.png", "/app/www/res/logo2.png");
			v9_video_sqldb_add_node(db, APP_BOARD_CALCU_1, tbl, 1, 1, "abc2", "1234567892", 1, 13, "/app/www/res/logo.png", "/app/www/res/logo3.png");
			v9_video_sqldb_add_node(db, APP_BOARD_CALCU_1, tbl, 0, 1, "abc3", "1234567893", 3, 33, "/app/www/res/logo.png", "/app/www/res/logo4.png");
			v9_video_sqldb_add_node(db, APP_BOARD_CALCU_1, tbl, 1, 1, "abc4", "1234567894", 1, 53, "/app/www/res/logo.png", "/app/www/res/logo5.png");
			v9_video_sqldb_add_node(db, APP_BOARD_CALCU_1, tbl, 1, 1, "abc5", "1234567895", 5, 28, "/app/www/res/logo.png", "/app/www/res/logo6.png");
		}
		else if(i == 2)
		{
/*			int v9_video_sqldb_del_by_datetime(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, char *datetime);
			int v9_video_sqldb_del_by_channel(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, zpl_uint32 channel);
			int v9_video_sqldb_del_by_gender(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, zpl_bool gender);

			int v9_video_sqldb_del_by_userid(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, char * userid);
			int v9_video_sqldb_del_by_index_id(sqlite3 *db, zpl_uint32 id, zpl_uint32 table, zpl_uint32 start, zpl_uint32 end);*/
		}
		v9_video_sqldb_close(db, APP_BOARD_CALCU_1);
	}
	return OK;
}
#endif

