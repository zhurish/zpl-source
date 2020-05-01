#ifndef __V9_VIDEO_SQLDB_H__
#define __V9_VIDEO_SQLDB_H__


#include "sqlite3.h"
#include "v9_video.h"
#include "v9_video_disk.h"


//#define V9_VIDEO_DB_FILE		"capdb.db"
#define V9_VIDEO_DB_FILE		"face.db"

//PRAGMA table_info([videocaptable]); 查看表格信息
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
					gender BOOLEAN NOT NULL,\
					similarity FLOAT NOT NULL,\
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
	u_int8			table;
	u_int8			ID;
	u_int32			keyid;
	u_int8			channel;
	u_int8			group;					// 所属组ID  0： 黑名单 1： 白名单
	char			username[APP_USERNAME_MAX];			// 姓名
	char			userid[APP_USERNAME_MAX];				// 证件号
	u_int8			gender;					// 人员性别  0： 女 1： 男
	char 			datetime[16];
	u_int8 			age;
	char 			mood[16];
	char 			complexion[16];
	char 			minority[16];

	char			picname[V9_APP_VIDEO_URL_MAX];
	char			videoname[V9_APP_VIDEO_URL_MAX];

	sql_snapfea_key	key;					// 预留位，便于拓展，默认置空

}v9_video_cap_t;


typedef struct
{
	u_int8		table;
	u_int8		id;
	u_int32			keyid;
	char			username[APP_USERNAME_MAX];			// 姓名
	char			userid[APP_USERNAME_MAX];				// 证件号
	u_int8		channel;
	u_int8		group;					// 所属组ID  0： 黑名单 1： 白名单
	u_int8		gender;					// 人员性别  0： 女 1： 男
	u_int32 		starttime;
	u_int32 		endtime;
	u_int8 		age;

	u_int32 		*get_outid;
	u_int32 		get_idcnt;
	u_int32 		get_index;

	sql_snapfea_key	key;					// 预留位，便于拓展，默认置空

}v9_cap_keywork_t;


int v9_video_sqldb_load(u_int32 id);
sqlite3 * v9_video_sqldb_open(u_int32 id, u_int32 table);
int v9_video_sqldb_close(sqlite3 *db, u_int32 id);
int v9_video_sqldb_count(sqlite3 *db, u_int32 id, u_int32 table, int *pValue);

/*
 * 删除
 */
int v9_video_sqldb_del_by_datetime(sqlite3 *db, u_int32 id, u_int32 table, char *datetime);
int v9_video_sqldb_del_by_channel(sqlite3 *db, u_int32 id, u_int32 table, u_int32 channel);
int v9_video_sqldb_del_by_gender(sqlite3 *db, u_int32 id, u_int32 table, BOOL gender);

int v9_video_sqldb_del_by_userid(sqlite3 *db, u_int32 id, u_int32 table, char * userid);
int v9_video_sqldb_del_by_index_id(sqlite3 *db, u_int32 id, u_int32 table, u_int32 start, u_int32 end);


/*
 * 查询
 */
/*
 * 根据条件获取ID表
 */
int v9_video_sqldb_select_by_keywork(sqlite3 *db, v9_cap_keywork_t *keywork,
									 u_int32 *outid, u_int32 *cnt);

/*
 * 获取最新添加的ID表
 */
int v9_video_sqldb_select_by_new(sqlite3 *db, v9_cap_keywork_t *keywork,
									 u_int32 *outid, u_int32 *cnt);
/*
 * 根据特征值获取表记录ID
 */
int v9_video_sqldb_select_by_keyvalue(sqlite3 *db, u_int32 id, u_int32 table,
									  sql_snapfea_key *key, u_int32 *outid, u_int32 *cnt);
/*
 * 根据记录ID或特征值
 */
int v9_video_sqldb_get_keyvalue(sqlite3 *db, u_int32 id, u_int32 table, u_int32 inid, sql_snapfea_key *key);

/*
 * 根据ID获取抓拍或告警视频信息
 */
int v9_video_sqldb_get_capinfo(sqlite3 *db,u_int32 id, u_int32 table, u_int32 inid, void *pArgv);

/*
 * 根据用户ID获取表记录ID
 */
int v9_video_sqldb_select_by_userid(sqlite3 *db, u_int32 id, u_int32 table,
									char *userid, u_int32 *outid, u_int32 *cnt);



int v9_video_sqldb_add_node(sqlite3 *db, u_int32 id, u_int32 table, BOOL gender, int group,
							char *user, char *user_id, u_int32 channel, u_int32 age, char *pic, char *video);

int v9_video_sqldb_keyvalue_update(sqlite3 *db, u_int32 id, u_int32 table, u_int32 nid, sql_snapfea_key *key);

#ifdef V9_SQLDB_TEST
int v9_video_sqldb_test(int i, int tbl, void *pArg);
#endif

#endif /* __V9_VIDEO_SQLDB_H__ */
