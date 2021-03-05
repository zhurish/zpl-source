#ifndef __V9_VIDEO_SQLDB_H__
#define __V9_VIDEO_SQLDB_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "sqlite3.h"
#include "v9_video.h"
#include "v9_video_disk.h"


//#define V9_VIDEO_DB_FILE		"capdb.db"
#define V9_VIDEO_DB_FILE		"face.db"

//PRAGMA table_info([videocaptable]); 查看表格信息
//PRAGMA table_info([snapcaptable]);
#define V9_VIDEO_SNAP_TBL_NAME	"snapcaptable"
#define V9_VIDEO_WARN_TBL_NAME	"videocaptable"

//id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
#define V9_VIDEO_SNAP_TBL_ST		\
			"snapcaptable (\
					id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\
					channel INTEGER NOT NULL,\
					datetime DATETIME NOT NULL,\
					keyvalue BLOB,\
					targettype INTEGER NOT NULL,\
					gender BOOLEAN NOT NULL,\
					age INTEGER,\
					mood CHARACTER(16),\
					complexion CHARACTER(16),\
					minority CHARACTER(16),\
					picurl VARCHAR(128) NOT NULL,\
					reserved1 BLOB,\
					reserved2 BLOB,\
					reserved3 BLOB\
    		);"

//id INTEGER UNIQUE PRIMARY KEY IDENTITY(1,1)
#define V9_VIDEO_WARN_TBL_ST		\
			"videocaptable (\
					id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\
					username CHARACTER(64) NOT NULL,\
					userid CHARACTER(64) NOT NULL,\
					groupid INTEGER NOT NULL,\
					channel INTEGER NOT NULL,\
					datetime DATETIME NOT NULL,\
					keyvalue BLOB,\
					similarity FLOAT NOT NULL,\
					gender BOOLEAN NOT NULL,\
					targettype INTEGER NOT NULL,\
					age INTEGER,\
					mood CHARACTER(16),\
					complexion CHARACTER(16),\
					minority CHARACTER(16),\
					picurl VARCHAR(128) NOT NULL,\
					videourl VARCHAR(128) NOT NULL,\
					reserved1 BLOB,\
					reserved2 BLOB,\
					reserved3 BLOB\
    		);"




typedef struct
{
	ospl_uint8			table;
	ospl_uint8			ID;
	ospl_uint32			keyid;
	ospl_uint8			channel;
	ospl_uint8			group;					// 所属组ID  0： 黑名单 1： 白名单
	char			username[APP_USERNAME_MAX];			// 姓名
	char			userid[APP_USERNAME_MAX];				// 证件号
	ospl_uint8			gender;					// 人员性别  0： 女 1： 男
	char 			datetime[32];
	ospl_uint8 			age;
	char 			mood[16];
	char 			complexion[16];
	char 			minority[16];

	char			picname[V9_APP_VIDEO_URL_MAX];
	char			videoname[V9_APP_VIDEO_URL_MAX];

	sql_snapfea_key	key;					// 预留位，便于拓展，默认置空

}v9_video_cap_t;


typedef struct
{
	ospl_uint8		table;
	ospl_uint8		id;
	ospl_uint32			keyid;
	char			username[APP_USERNAME_MAX];			// 姓名
	char			userid[APP_USERNAME_MAX];				// 证件号
	ospl_uint8		channel;
	ospl_uint8		group;					// 所属组ID  0： 黑名单 1： 白名单
	ospl_uint8		gender;					// 人员性别  0： 女 1： 男
	ospl_uint32 		starttime;
	ospl_uint32 		endtime;
	ospl_uint8 		age;

	ospl_uint32 		*get_outid;//获取的ID
	ospl_uint32 		get_idcnt;
	ospl_uint32 		get_index;
	ospl_uint32 		get_page;

	sql_snapfea_key	key;					// 预留位，便于拓展，默认置空
	ospl_uint32 		limit;					//数量限制
}v9_cap_keywork_t;

void v9_video_db_lock();
void v9_video_db_unlock();
int v9_video_sqldb_load(ospl_uint32 id);
sqlite3 * v9_video_sqldb_open(ospl_uint32 id, ospl_uint32 table);
int v9_video_sqldb_close(sqlite3 *db, ospl_uint32 id);
int v9_video_sqldb_count(sqlite3 *db, ospl_uint32 id, ospl_uint32 table, int *pValue);

/*
 * 删除
 */
int v9_video_sqldb_del_by_datetime(sqlite3 *db, ospl_uint32 id, ospl_uint32 table, char *datetime);
int v9_video_sqldb_del_by_channel(sqlite3 *db, ospl_uint32 id, ospl_uint32 table, ospl_uint32 channel);
int v9_video_sqldb_del_by_gender(sqlite3 *db, ospl_uint32 id, ospl_uint32 table, ospl_bool gender);

int v9_video_sqldb_del_by_userid(sqlite3 *db, ospl_uint32 id, ospl_uint32 table, char * userid);
int v9_video_sqldb_del_by_index_id(sqlite3 *db, ospl_uint32 id, ospl_uint32 table, ospl_uint32 start, ospl_uint32 end);

//清除最早插入的数据（limit：数量）
int v9_video_sqldb_select_by_oldid(sqlite3 *db, ospl_uint32 id, ospl_uint32 table, ospl_uint32 limit);
/*
 * 查询
 */
/*
 * 根据条件获取ID表
 */
int v9_video_sqldb_select_count_by_keywork(sqlite3 *db, v9_cap_keywork_t *keywork, int *pValue);
int v9_video_sqldb_select_by_keywork(sqlite3 *db, v9_cap_keywork_t *keywork,
									 ospl_uint32 *outid, ospl_uint32 *cnt);

/*
 * 获取最新添加的ID表
 */
int v9_video_sqldb_select_by_new(sqlite3 *db, v9_cap_keywork_t *keywork,
									 ospl_uint32 *outid, ospl_uint32 *cnt);
/*
 * 根据特征值获取表记录ID
 */
int v9_video_sqldb_select_by_keyvalue(sqlite3 *db, ospl_uint32 id, ospl_uint32 table, ospl_uint32 limit,
									  sql_snapfea_key *key, ospl_uint32 *outid, ospl_uint32 *cnt);
/*
 * 根据记录ID或特征值
 */
int v9_video_sqldb_get_keyvalue(sqlite3 *db, ospl_uint32 id, ospl_uint32 table, ospl_uint32 inid, sql_snapfea_key *key);

/*
 * 根据ID获取抓拍或告警视频信息
 */
int v9_video_sqldb_get_capinfo(sqlite3 *db,ospl_uint32 id, ospl_uint32 table, ospl_uint32 inid, void *pArgv);

/*
 * 根据用户ID获取表记录ID
 */
int v9_video_sqldb_select_by_userid(sqlite3 *db, ospl_uint32 id, ospl_uint32 table,
									char *userid, ospl_uint32 *outid, ospl_uint32 *cnt);



int v9_video_sqldb_add_node(sqlite3 *db, ospl_uint32 id, ospl_uint32 table, ospl_bool gender, int group,
							char *user, char *user_id, ospl_uint32 channel, ospl_uint32 age, char *pic, char *video);

int v9_video_sqldb_keyvalue_update(sqlite3 *db, ospl_uint32 id, ospl_uint32 table, ospl_uint32 nid, sql_snapfea_key *key);

#ifdef V9_SQLDB_TEST
int v9_video_sqldb_test(ospl_uint32 i, int tbl, void *pArg);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __V9_VIDEO_SQLDB_H__ */
