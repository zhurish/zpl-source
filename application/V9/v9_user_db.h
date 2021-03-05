#ifndef __V9_USER_SQLDB_H__
#define __V9_USER_SQLDB_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "sqlite3.h"
#include "v9_video.h"
#include "v9_video_disk.h"

#define V9_SQLDB_DEBUG_MSG		0X01
#define V9_SQLDB_DEBUG_DB		0X02
#define V9_SQLDB_DEBUG_STATE	0X04
#define V9_SQLDB_DEBUG_DBCMD	0X100

#define V9_SQLDB_DEBUG(n)		(V9_SQLDB_DEBUG_ ## n & _sqldb_debug)
#define V9_SQLDB_DEBUG_ON(n)	(_sqldb_debug |= (V9_SQLDB_DEBUG_ ## n ))
#define V9_SQLDB_DEBUG_OFF(n)	(_sqldb_debug &= ~(V9_SQLDB_DEBUG_ ## n ))



#define V9_USER_DB_FILE		"userdb.db"

#define V9_USER_DB_TBL		"userdb_%d"
//#define V9_USER_DB_DIR		"user"


//PRAGMA table_info([videocaptable]); 查看表格信息
#define V9_USER_DB_TBL_ST		"CREATE TABLE "V9_USER_DB_TBL " ( \
		userid CHARACTER(64) UNIQUE PRIMARY KEY, \
		username CHARACTER(64) NOT NULL,\
		gender BOOLEAN NOT NULL,\
		groupid INTEGER NOT NULL,\
		datetime DATETIME NOT NULL,\
		key BLOB,\
		url VARCHAR(128) NOT NULL,\
		text VARCHAR(128)\
		);"




extern ospl_uint32 _sqldb_debug;



extern sqlite3 * v9_user_sqldb_open(ospl_uint32 id);
extern int v9_user_sqldb_close(sqlite3 *db, ospl_uint32 id);

extern int v9_user_sqldb_count(sqlite3 *db, ospl_uint32 id, int group, int *pValue);

extern int v9_user_sqldb_add(sqlite3 *db, ospl_uint32 id, ospl_bool gender, int group, char *user, char *user_id, char *pic, char *text);
//extern int v9_user_sqldb_add_qurey(sqlite3 *db, ospl_uint32 id, ospl_bool gender, int group, char *user, char *user_id, char *pic);

extern int v9_user_sqldb_update(sqlite3 *db, ospl_uint32 id, ospl_bool gender, int group, char *user, char *user_id, char *pic, char *text);

extern int v9_user_sqldb_key_update(sqlite3 *db, ospl_uint32 id, char *user_id, sql_snapfea_key *key);

extern int v9_user_sqldb_key_select(sqlite3 *db, ospl_uint32 id, char *user_id, sql_snapfea_key *key);

extern int v9_user_sqldb_del(sqlite3 *db, ospl_uint32 id, char *user_id);

extern int v9_user_sqldb_del_group(sqlite3 *db, ospl_uint32 id, int group);
extern int v9_user_sqldb_cleanup(ospl_uint32 id);

extern int v9_user_sqldb_lookup_user(sqlite3 *db,ospl_uint32 id, char *user_id, void *user);
extern int v9_user_sqldb_foreach(sqlite3 *db, ospl_uint32 id, int groupid, char *user_id, void * cb, void *pVoid);


extern int v9_user_sqldb_key_foreach(sqlite3 *db, ospl_uint32 id, sql_snapfea_key *key, char *user_id, ospl_float* p_fResult);

#ifdef V9_SQLDB_TEST
extern int v9_user_sqldb_test(ospl_uint32 i, void *pArg);
#endif


#ifdef __cplusplus
}
#endif

#endif /* __V9_USER_SQLDB_H__ */
