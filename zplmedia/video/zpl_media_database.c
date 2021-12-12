#include "zpl_media_database.h"

#ifdef ZPL_SQLITE_MODULE

int zpl_media_database_create(zpl_media_db_t *zdb, const char *tbl)
{
    int result;
    char * errmsg = NULL;
    result = sqlite3_exec( zdb->db, tbl, NULL, NULL, &errmsg);
    if(result != SQLITE_OK )
    {
        printf( "创建表失败，错误码:%d，错误原因:%s/n", result, errmsg );
        return ERROR;
    }
    return OK;
}

int zpl_media_insert(zpl_media_db_t *zdb, int captureid, const char *datetime, int channel, const char *urlpath)
{
    int result;
    char * errmsg = NULL;
    char sqlcmd[512];
    memset(sqlcmd, 0, sizeof(sqlcmd));
    sprintf(sqlcmd, MEDIA_DATABASE_INSERT, captureid,datetime,channel,urlpath);
    result = sqlite3_exec( zdb->db, sqlcmd, NULL, NULL, &errmsg);
    if(result != SQLITE_OK )
    {
        printf( "创建表失败，错误码:%d，错误原因:%s/n", result, errmsg );
        return ERROR;
    }
    return OK;
}

int zpl_media_deletebyid(zpl_media_db_t *zdb, int id)
{
    int result;
    char * errmsg = NULL;
    char sqlcmd[512];
    memset(sqlcmd, 0, sizeof(sqlcmd));
    sprintf(sqlcmd, MEDIA_DATABASE_DELETE_BY_ID, id);
    result = sqlite3_exec( zdb->db, sqlcmd, NULL, NULL, &errmsg);
    if(result != SQLITE_OK )
    {
        printf( "创建表失败，错误码:%d，错误原因:%s/n", result, errmsg );
        return ERROR;
    }
    return OK;
}

int zpl_media_deletebycaptureid(zpl_media_db_t *zdb, int id)
{
    int result;
    char * errmsg = NULL;
    char sqlcmd[512];
    memset(sqlcmd, 0, sizeof(sqlcmd));
    sprintf(sqlcmd, MEDIA_DATABASE_DELETE_BY_CAPTUREID, id);
    result = sqlite3_exec( zdb->db, sqlcmd, NULL, NULL, &errmsg);
    if(result != SQLITE_OK )
    {
        printf( "创建表失败，错误码:%d，错误原因:%s/n", result, errmsg );
        return ERROR;
    }
    return OK;
}

int zpl_media_deletebydatetime(zpl_media_db_t *zdb, const char *sdatetime, const char *edatetime)
{
    int result;
    char * errmsg = NULL;
    char sqlcmd[512];
    memset(sqlcmd, 0, sizeof(sqlcmd));
    if(sdatetime && edatetime)
        sprintf(sqlcmd, MEDIA_DATABASE_DELETE_BY_DATATIME "<= '%s' && datetime >= '%s';", edatetime, sdatetime);
    else if(!sdatetime && edatetime)
        sprintf(sqlcmd, MEDIA_DATABASE_DELETE_BY_DATATIME "<= '%s';", edatetime);
    else if(sdatetime && edatetime)
        sprintf(sqlcmd, MEDIA_DATABASE_DELETE_BY_DATATIME " >= '%s';", sdatetime);

    result = sqlite3_exec( zdb->db, sqlcmd, NULL, NULL, &errmsg);
    if(result != SQLITE_OK )
    {
        printf( "创建表失败，错误码:%d，错误原因:%s/n", result, errmsg );
        return ERROR;
    }
    return OK;
}
int zpl_media_deletebychannel(zpl_media_db_t *zdb, int channel)
{
    int result;
    char * errmsg = NULL;
    char sqlcmd[512];
    memset(sqlcmd, 0, sizeof(sqlcmd));
    sprintf(sqlcmd, MEDIA_DATABASE_DELETE_BY_CHANNEL, channel);
    result = sqlite3_exec( zdb->db, sqlcmd, NULL, NULL, &errmsg);
    if(result != SQLITE_OK )
    {
        printf( "创建表失败，错误码:%d，错误原因:%s/n", result, errmsg );
        return ERROR;
    }
    return OK;
}

//sqlite> SELECT * FROM COMPANY ORDER BY SALARY ASC/DESC;
static int select_callback(void *data, int argc, char **argv, char **azColName)
{
    int i = 0, j = 0;
    zpl_media_table_t *table = data;
    for(i=0; i<argc; i++)
    {
        if(i && i%5 == 0)
            j++;
        if(azColName[i] && strcmp(azColName[i], "id")==0)
        {
            if(argv[i])
                table[j].id = atoi(argv[i]);
        }
        else if(azColName[i] && strcmp(azColName[i], "captureid")==0)
        {
            if(argv[i])
                table[j].captureid = atoi(argv[i]);
        }
        else if(azColName[i] && strcmp(azColName[i], "datetime")==0)
        {
            if(argv[i])
                strcpy(table[j].urlpath,argv[i]);
        }
        else if(azColName[i] && strcmp(azColName[i], "channel")==0)
        {
            if(argv[i])
                table[j].channel = atoi(argv[i]);
        }
        else if(azColName[i] && strcmp(azColName[i], "urlpath")==0)
        {
            if(argv[i])
                strcpy(table[j].urlpath,argv[i]);
        }
        else if(azColName[i] && strcmp(azColName[i], "text")==0)
        {
            if(argv[i])
            {
                strcpy(table[j].text,argv[i]);
            }
        }
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int zpl_media_select(zpl_media_db_t *zdb, zpl_media_table_t *table, int num, bool sort)
{
    int result;
    char * errmsg = NULL;
    char sqlcmd[512];
    memset(sqlcmd, 0, sizeof(sqlcmd));
    sprintf(sqlcmd, "SELECT id,captureid,datetime,channel,urlpath from capturetbl ORDER BY datetime %s LIMIT %d;", sort?"ASC":"DESC", num);
    result = sqlite3_exec( zdb->db, sqlcmd, select_callback, table, &errmsg);
    if(result != SQLITE_OK )
    {
        printf( "创建表失败，错误码:%d，错误原因:%s/n", result, errmsg );
        return ERROR;
    }
    return OK;
}

#endif