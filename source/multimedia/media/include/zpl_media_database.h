#ifndef __ZPL_MEDIA_DATABASE_H__
#define __ZPL_MEDIA_DATABASE_H__

#ifdef __cplusplus
extern "C" {
#endif


#ifdef ZPL_SQLITE_MODULE
#include <sqlite3.h>

#define MEDIA_DATABASE_TABLE    "create table capturetbl(\
    id INT primary key autoincrement,\
    captureid INT,\
    datetime DATETIME NOT NULL,\
    channel INT NOT NULL,\
    urlpath VARCHAR(256) NOT NULL,\
    text BLOB);"

#define MEDIA_DATABASE_INSERT    "INSERT INTO capturetbl(\
    captureid,datetime,channel,urlpath) VALUES(%d,'%s',%d,'%s');"

#define MEDIA_DATABASE_DELETE_BY_ID    "DELETE FROM capturetbl where ID=%d;"
#define MEDIA_DATABASE_DELETE_BY_CAPTUREID   "DELETE FROM capturetbl where captureid=%d;"
#define MEDIA_DATABASE_DELETE_BY_CHANNEL     "DELETE FROM capturetbl where channel=%d;"
#define MEDIA_DATABASE_DELETE_BY_DATATIME    "DELETE FROM capturetbl where datetime "

typedef struct zpl_media_table_s
{
    zpl_uint32      id;
    zpl_uint32      captureid;
    zpl_uint32      datetime;
    zpl_uint32      channel;
    zpl_uint8       urlpath[256];
    zpl_uint8       *text;
}zpl_media_table_t;

typedef struct zpl_media_db_s
{
    sqlite3         *db;

}zpl_media_db_t;


int zpl_media_database_create(zpl_media_db_t *zdb, const char *tbl);
int zpl_media_insert(zpl_media_db_t *zdb, int captureid, const char *datetime, int channel, const char *urlpath);
int zpl_media_deletebyid(zpl_media_db_t *zdb, int id);
int zpl_media_deletebycaptureid(zpl_media_db_t *zdb, int id);
int zpl_media_deletebydatetime(zpl_media_db_t *zdb, const char *sdatetime, const char *edatetime);
int zpl_media_deletebychannel(zpl_media_db_t *zdb, int channel);
int zpl_media_select(zpl_media_db_t *zdb, zpl_media_table_t *table, int num, bool sort);


#endif

#ifdef __cplusplus
}
#endif




#endif // __ZPL_MEDIA_DATABASE_H__
