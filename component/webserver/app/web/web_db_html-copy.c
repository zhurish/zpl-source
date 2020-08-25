/*
 * web_dbuser_html.c
 *
 *  Created on: 2020年3月28日
 *      Author: zhurish
 */

#define HAS_BOOL 1
#include "zebra.h"
#include "module.h"
#include "memory.h"
#include "zassert.h"
#include "command.h"
#include "prefix.h"
#include "host.h"
#include "log.h"
#include "vty.h"
#include "vty_user.h"


#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

//#ifdef PL_APP_MODULE
#include "application.h"





//static v9_cap_keywork_t keywork;

static char * web_video_picpath(int id, char *picpath)
{
	static char pathff[V9_APP_DIR_NAME_MAX];
	memset(pathff, 0, sizeof(pathff));
	sprintf(pathff, "%s/%s", v9_video_disk_capdb_dir(id), picpath);
	return pathff;
}

static void web_video_keywork_free(v9_cap_keywork_t *keywork)
{
	if(keywork->get_outid)
	{
		XFREE (MTYPE_VIDEO_TMP, keywork->get_outid);
		keywork->get_outid = NULL;
	}
	v9_app_snapfea_key_free(&keywork->key);
}

static v9_cap_keywork_t * web_video_keywork_get(Webs *wp)
{
	v9_cap_keywork_t *keywork = wp->private_data[WEB_CAPDB_PRIVATE_INDEX];
	if(keywork == NULL)
	{
		wp->private_data[WEB_CAPDB_PRIVATE_INDEX] = keywork = walloc(sizeof(v9_cap_keywork_t));
		if(wp->private_data[WEB_CAPDB_PRIVATE_INDEX] == NULL)
		{
			return NULL;
		}
		memset(wp->private_data[WEB_CAPDB_PRIVATE_INDEX], 0, sizeof(v9_cap_keywork_t));
		keywork = wp->private_data[WEB_CAPDB_PRIVATE_INDEX];
		wp->private_free[WEB_CAPDB_PRIVATE_INDEX] = web_video_keywork_free;
	}
	return keywork;
}


static int web_video_snap_keywork_get(Webs *wp, char *path, char *query, v9_cap_keywork_t *keyw)
{
	char *strval = NULL;
	web_assert(wp);
	web_assert(keyw);
	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_WEB, "Can not Get Board ID Value");
		return web_return_application_json_msg(wp, ERROR, "获取不到板卡ID", NULL);
		//return web_return_text_plain(wp, ERROR);
	}
	//return web_return_application_json_msg(wp, ERROR, "Can not Get Board ID Value", NULL);
	keyw->id = atoi(strval);
	keyw->id = V9_APP_BOARD_CALCU_ID(keyw->id);

	strval = webs_get_var(wp, T("ch"), T(""));
	if (NULL != strval)
	{
		keyw->channel = atoi(strval);
	}

	strval = webs_get_var(wp, T("gender"), T(""));
	if (NULL != strval)
	{
		keyw->gender = atoi(strval) + 1;
	}

	strval = webs_get_var(wp, T("starttime"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_WEB, "Can not Get starttime Value");
		return web_return_application_json_msg(wp, ERROR, "获取起始时间失败", NULL);
	}
	keyw->starttime = os_timestamp_spilt(0,  strval);

	strval = webs_get_var(wp, T("endtime"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_WEB, "Can not Get endtime Value");
		return web_return_application_json_msg(wp, ERROR, "获取结束时间失败", NULL);
	}
	keyw->endtime = os_timestamp_spilt(0,  strval);

	strval = webs_get_var(wp, T("age"), T(""));
	if (NULL != strval)
	{
		keyw->age = atoi(strval) + 1;
	}
	_WEB_DBG_TRAP("%s:starttime=%d endtime=%d\r\n",__func__,keyw->starttime,keyw->endtime);
	return OK;
}

static int web_video_snap_handle(Webs *wp, char *path, char *query)
{
	int ret = 0;
	sqlite3 * db = NULL;
	v9_video_stream_t *stream = NULL;
	web_assert(wp);
	v9_cap_keywork_t *keywork = web_video_keywork_get(wp);
	if(keywork == NULL)
	{
		return web_return_application_json_msg(wp, ERROR, "无法申请内存", NULL);
	}
	wp->iValue = 0;

	if(keywork->get_idcnt == keywork->get_index && keywork->get_outid)
	{
		XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
		keywork.get_outid = NULL;
	}
	v9_app_snapfea_key_free(&keywork.key);
	memset (&keywork, 0, sizeof(v9_cap_keywork_t));
	if (web_video_snap_keywork_get (wp, path, query, &keywork) != OK)
	{
		memset (&keywork, 0, sizeof(v9_cap_keywork_t));
		return ERROR;//web_return_application_json(wp, "[\"response\":\"ERROR\", \"data\":\"can not open sql db\"]");
		//return web_return_text_plain (wp, ERROR);
	}
	keywork.limit = 180;
	keywork.table = 0;
	db = v9_video_sqldb_open (keywork.id, keywork.table);
	if (db)
	{
		keywork.get_outid = XMALLOC (MTYPE_VIDEO_TMP, 4 * 8192);
		if (!keywork.get_outid)
		{
			memset (&keywork, 0, sizeof(v9_cap_keywork_t));
			v9_video_sqldb_close (db, keywork.id);
			return web_return_application_json_msg(wp, ERROR, "无法申请内存", NULL);
			//return web_return_text_plain (wp, ERROR);
		}
		/*
		 * 根据条件获取ID表
		 */
		ret = v9_video_sqldb_select_by_keywork (db, &keywork, keywork.get_outid,
												&keywork.get_idcnt);
		v9_app_snapfea_key_free(&keywork.key);
		if (ret != OK)
		{
			if(WEB_IS_DEBUG(EVENT))
				zlog_debug(ZLOG_WEB, "Can not Select by Keywork Value");
			XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
			keywork.get_outid = NULL;
			v9_video_sqldb_close (db, keywork.id);
			memset (&keywork, 0, sizeof(v9_cap_keywork_t));
			return web_return_application_json_msg(wp, ERROR, "没有查询到相关数据", NULL);
			//return web_return_text_plain (wp, ERROR);
		}

		/*
		 * 根据ID获取抓拍或告警视频信息
		 */
		u_int32 i = 0;
		v9_video_cap_t cap;
		memset (&cap, 0, sizeof(v9_video_cap_t));

		websSetStatus (wp, 200);
		websWriteHeaders (wp, -1, 0);
		websWriteHeader (wp, "Content-Type", "application/json");
		websWriteEndHeaders (wp);
		//websWrite (wp, "%s", "[");
		//websWrite (wp, "%s", "\"response\":\"OK\", \"msg\":\"OK\", \"data\":[");
		websWrite (wp, "{\"response\":\"OK\", \"msg\":\"%d\", \"data\":[", keywork.get_idcnt);

		for (i = keywork.get_index; i < keywork.get_idcnt; i++)
		{
			memset (&cap, 0, sizeof(v9_video_cap_t));
			ret = v9_video_sqldb_get_capinfo (db, keywork.id, keywork.table,
											  keywork.get_outid[i], &cap);
			v9_app_snapfea_key_free(&cap.key);
			if (ret == OK)
			{
				//V9_DB_DEBUG(" %s :id=%d datatime=%s url=%s\n",__func__, keywork.get_outid[i], cap.datetime, cap.picname);
				if (wp->iValue > 0)
					websWrite (wp, "%s", ",");

				stream = v9_video_board_stream_lookup_by_id_and_ch(keywork.id, cap.channel+1);

				websWrite (
						wp,
						"{\"id\":%d, \"groupname\":\"%s\", \"gender\":\"%s\", \"age\":\"%d\", \"address\":\"%s\", \"channel\":%d, \"datetime\":\"%s\", \"url\":\"%s\"}",
						cap.keyid,
						v9_video_usergroup_idtoname(keywork.id, cap.group)?v9_video_usergroup_idtoname(keywork.id, cap.group):"Unknow",
						cap.gender ? "男" : "女",
						cap.age, stream?inet_address(stream->address):"0.0.0.0", cap.channel+1, cap.datetime,
						!str_isempty(cap.picname, sizeof(cap.picname)) ? web_video_picpath(keywork.id, cap.picname):" ");
						//strlen(cap.picname) ? web_video_picpath(keywork.id, cap.picname):" "/*v9_video_disk_capdb_dir(keywork.id), cap.picname*/);

				wp->iValue++;
			}
			else
			{
				if(WEB_IS_DEBUG(EVENT))
					zlog_debug(ZLOG_WEB, "Can not Select by index id Value");
			}
/*			if(wp->iValue == 20)
				break;*/
		}
		wp->iValue = 0;
		websWrite (wp, "%s", "]}");
		websDone (wp);
		XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
		keywork.get_outid = NULL;
		v9_app_snapfea_key_free(&keywork.key);
		v9_video_sqldb_close (db, keywork.id);
		memset (&keywork, 0, sizeof(v9_cap_keywork_t));
		return OK;
	}
	else
	{
		if(keywork.get_outid)
		{
			XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
			keywork.get_outid = NULL;
		}
		v9_app_snapfea_key_free(&keywork.key);
		memset (&keywork, 0, sizeof(v9_cap_keywork_t));
		return web_return_application_json_msg(wp, ERROR, "打开数据库失败", NULL);
		//return web_return_text_plain (wp, ERROR);
	}
	return OK;
}



static int web_video_pic_keywork_get(Webs *wp, char *path, char *query, v9_cap_keywork_t *keyw)
{
	char *strval = NULL;
	WebsKey *s = NULL;
	WebsUpload *up = NULL;
	char uploadfile[256];
	web_assert(wp);
	web_assert(keyw);
	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_WEB, "Can not Get Board ID Value");
		return web_return_application_json_msg(wp, ERROR, "获取不到板卡ID", NULL);
		//return web_return_text_plain(wp, ERROR);
	}
	keyw->id = atoi(strval);
	keyw->id = V9_APP_BOARD_CALCU_ID(keyw->id);

	strval = webs_get_var(wp, T("ch"), T(""));
	if (NULL != strval)
	{
		keyw->channel = atoi(strval);
	}
/*
	strval = webs_get_var(wp, T("pic"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	//keyw->gender = atoi(strval);
*/
	strval = webs_get_var(wp, T("similarity"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_WEB, "Can not Get similarity Value");
		return web_return_application_json_msg(wp, ERROR, "获取相似度失败", NULL);
		//return web_return_text_plain(wp, ERROR);
	}
	keyw->key.input_value = (float)(atoi(strval)/100);

	zlog_debug(ZLOG_WEB, "======================%s:keyw->key.input_value=%f(%s)\r\n",__func__, keyw->key.input_value, strval);

	strval = webs_get_var(wp, T("starttime"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_WEB, "Can not Get starttime Value");
		return web_return_application_json_msg(wp, ERROR, "获取起始时间失败", NULL);
		//return web_return_text_plain(wp, ERROR);
	}
	keyw->starttime = os_timestamp_spilt(0,  strval);

	strval = webs_get_var(wp, T("endtime"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_WEB, "Can not Get endtime Value");
		return web_return_application_json_msg(wp, ERROR, "获取结束时间失败", NULL);
		//return web_return_text_plain(wp, ERROR);
	}
	keyw->endtime = os_timestamp_spilt(0,  strval);

	_WEB_DBG_TRAP("%s:starttime=%d endtime=%d\r\n",__func__,keyw->starttime,keyw->endtime);

	if(v9_app_snapfea_key_alloc(&keyw->key, FALSE) != OK)
	{
		return web_return_application_json_msg(wp, ERROR, "获取特征点存储失败", NULL);
	}

	if (scaselessmatch(wp->method, "POST"))
	{
		for (s = hashFirst(wp->files); s; s = hashNext(wp->files, s))
		{
			up = s->content.value.symbol;
			memset(uploadfile, 0, sizeof(uploadfile));
			sprintf(uploadfile, "%s/%s", WEB_UPLOAD_BASE, up->clientFilename);
			if (rename(up->filename, uploadfile) < 0)
			{
				web_return_application_json_msg(wp, ERROR, "获取上传图像失败", NULL);
				//web_return_text_plain(wp, ERROR);
				return ERROR;
			}
			//pic = up->clientFilename;
			//strcpy(pic, uploadfile);
			sync();
#ifdef V9_VIDEO_SDK_API
			//获取图片特征值
			if(v9_video_sdk_get_keyvalue_api(keyw->id, uploadfile, &keyw->key) == OK)
			{
				if(keyw->key.feature_len)
					return OK;
				else
				{
					v9_app_snapfea_key_free(&keyw->key);
					if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
						zlog_debug(ZLOG_WEB, "Can not Get keyvalue Value by pic '%s'", uploadfile);
					return ERROR;
				}
			}
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(ZLOG_WEB, "Can not Get keyvalue Value by pic '%s'", uploadfile);
			return ERROR;
			//printf("rename %s -> %s", up->filename, uploadfile);
#else
			return OK;
#endif
			//ret = v9_video_user_add_user(V9_APP_BOARD_CALCU_ID(id),  gender,  group, user, user_id, uploadfile);

		}
	}
	return ERROR;
}


static int web_video_snap_pichandle(Webs *wp, char *path, char *query)
{
	int ret = 0;
	sqlite3 * db = NULL;
	//char *strval = NULL;
	v9_video_stream_t *stream = NULL;
	web_assert(wp);

	wp->iValue = 0;
	if(keywork.get_outid)
	{
		XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
		keywork.get_outid = NULL;
	}
	v9_app_snapfea_key_free(&keywork.key);
	memset (&keywork, 0, sizeof(v9_cap_keywork_t));
	if (web_video_pic_keywork_get (wp, path, query, &keywork) != OK)
	{
		memset (&keywork, 0, sizeof(v9_cap_keywork_t));
		return ERROR;//web_return_text_plain (wp, ERROR);
	}
	keywork.table = 0;
	db = v9_video_sqldb_open (keywork.id, keywork.table);
	keywork.get_index = 0;
	if (db)
	{
		keywork.get_outid = XMALLOC (MTYPE_VIDEO_TMP, 4 * 8192);
		if (!keywork.get_outid)
		{
			memset (&keywork, 0, sizeof(v9_cap_keywork_t));
			v9_video_sqldb_close (db, keywork.id);
			return web_return_application_json_msg(wp, ERROR, "无法申请内存", NULL);
		}
		/*
		 * 根据条件获取ID表
		 */
		zlog_debug(ZLOG_WEB, "======================%s:keyw->key.input_value=%f\r\n",__func__, keywork.key.input_value);
		keywork.key.feature_memcmp = v9_video_sdk_get_sosine_similarity_api;
		//keywork.key.input_value = 0.80;
		ret = v9_video_sqldb_select_by_keyvalue (db, keywork.id, keywork.table, keywork.limit,
												 &keywork.key,
												 keywork.get_outid,
												 &keywork.get_idcnt);
		v9_app_snapfea_key_free(&keywork.key);
		//ret = v9_video_sqldb_select_by_keywork(db, &keywork, keywork.get_outid, &keywork.get_idcnt);

		if (ret != OK)
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(ZLOG_WEB, "Can not Select db by keyvalue Value");
			XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
			keywork.get_outid = NULL;
			v9_app_snapfea_key_free(&keywork.key);
			v9_video_sqldb_close (db, keywork.id);
			memset (&keywork, 0, sizeof(v9_cap_keywork_t));
			return web_return_application_json_msg(wp, ERROR, "没有查询到相关数据", NULL);
		}

		/*
		 * 根据ID获取抓拍或告警视频信息
		 */
		u_int32 i = 0;
		v9_video_cap_t cap;
		memset (&cap, 0, sizeof(v9_video_cap_t));

		websSetStatus (wp, 200);
		websWriteHeaders (wp, -1, 0);
		websWriteHeader (wp, "Content-Type", "application/json");
		websWriteEndHeaders (wp);
		//websWrite (wp, "%s", "[");
		websWrite (wp, "{\"response\":\"OK\", \"msg\":\"%d\", \"data\":[", keywork.get_idcnt);

		for (i = keywork.get_index; i < keywork.get_idcnt; i++)
		{
			ret = v9_video_sqldb_get_capinfo (db, keywork.id, keywork.table,
											  keywork.get_outid[i], &cap);
			v9_app_snapfea_key_free(&cap.key);
			if (ret == OK)
			{
				if (wp->iValue > 0)
					websWrite (wp, "%s", ",");

				stream = v9_video_board_stream_lookup_by_id_and_ch(keywork.id, cap.channel+1);

				websWrite (
						wp,
						"{\"id\":%d, \"groupname\":\"%s\", \"gender\":\"%s\", \"age\":\"%d\", \"address\":\"%s\",\"channel\":%d, \"datetime\":\"%s\", \"url\":\"%s\"}",
						cap.keyid,
						v9_video_usergroup_idtoname(keywork.id, cap.group)?v9_video_usergroup_idtoname(keywork.id, cap.group):"Unknow",
						cap.gender ? "男" : "女",
						cap.age, stream?inet_address(stream->address):"0.0.0.0", cap.channel+1, cap.datetime,
						!str_isempty(cap.picname, sizeof(cap.picname)) ? web_video_picpath(keywork.id, cap.picname):" ");
						//strlen(cap.picname) ? web_video_picpath(keywork.id, cap.picname):" "/*v9_video_disk_capdb_dir(keywork.id), cap.picname*/);

				wp->iValue++;

			}
			else
			{
				if(WEB_IS_DEBUG(EVENT))
					zlog_debug(ZLOG_WEB, "Can not Select by index id Value");
			}
/*			if(wp->iValue == 20)
				break;*/
		}
		wp->iValue = 0;
		websWrite (wp, "%s", "]}");
		websDone (wp);
		XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
		keywork.get_outid = NULL;
		v9_app_snapfea_key_free(&keywork.key);
		v9_video_sqldb_close (db, keywork.id);
		memset (&keywork, 0, sizeof(v9_cap_keywork_t));
		return OK;
	}
	else
	{
		if(keywork.get_outid)
		{
			XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
			keywork.get_outid = NULL;
		}
		v9_app_snapfea_key_free(&keywork.key);
		memset (&keywork, 0, sizeof(v9_cap_keywork_t));
		return web_return_application_json_msg(wp, ERROR, "打开数据库失败", NULL);
		//return web_return_text_plain (wp, ERROR);
	}
	return OK;
}

/******************************************************************************/
/******************************************************************************/
static int web_video_warn_keywork_get(Webs *wp, char *path, char *query, v9_cap_keywork_t *keyw)
{
	char *strval = NULL;
	web_assert(wp);
	web_assert(keyw);
	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_WEB, "Can not Get Board ID Value");
		return web_return_application_json_msg(wp, ERROR, "获取不到板卡ID", NULL);
	}
	keyw->id = atoi(strval);
	keyw->id = V9_APP_BOARD_CALCU_ID(keyw->id);

	strval = webs_get_var(wp, T("ch"), T(""));
	if (NULL != strval)
	{
		keyw->channel = atoi(strval);
	}

	strval = webs_get_var(wp, T("group"), T(""));
	if (NULL != strval)
	{
		keyw->group = atoi(strval) + 1;
	}

	strval = webs_get_var(wp, T("starttime"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_WEB, "Can not Get starttime Value");
		return web_return_application_json_msg(wp, ERROR, "获取起始时间失败", NULL);
	}
	keyw->starttime = os_timestamp_spilt(0,  strval);

	strval = webs_get_var(wp, T("endtime"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_WEB, "Can not Get endtime Value");
		return web_return_application_json_msg(wp, ERROR, "获取结束时间失败", NULL);
	}
	keyw->endtime = os_timestamp_spilt(0,  strval);

	strval = webs_get_var(wp, T("username"), T(""));
	if (NULL != strval)
	{
		strcpy(keyw->username, strval);
	}

	strval = webs_get_var(wp, T("userid"), T(""));
	if (NULL != strval)
	{
		strcpy(keyw->userid, strval);
	}

	_WEB_DBG_TRAP("%s:starttime=%d endtime=%d\r\n",__func__,keyw->starttime,keyw->endtime);
	return OK;
}


static int web_video_warn_handle(Webs *wp, char *path, char *query)
{
	int ret = 0;
	sqlite3 * db = NULL;
	v9_video_user_t user;
	v9_video_stream_t *stream = NULL;
	web_assert(wp);

	wp->iValue = 0;
	if(keywork.get_outid)
	{
		XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
		keywork.get_outid = NULL;
	}
	v9_app_snapfea_key_free(&keywork.key);
	memset (&keywork, 0, sizeof(v9_cap_keywork_t));
	if (web_video_warn_keywork_get (wp, path, query, &keywork) != OK)
	{
		memset (&keywork, 0, sizeof(v9_cap_keywork_t));
		return ERROR;//web_return_text_plain (wp, ERROR);
	}
	keywork.table = 1;
	db = v9_video_sqldb_open (keywork.id, keywork.table);
	if (db)
	{
		keywork.get_outid = XMALLOC (MTYPE_VIDEO_TMP, 4 * 8192);
		if (!keywork.get_outid)
		{
			memset (&keywork, 0, sizeof(v9_cap_keywork_t));
			v9_video_sqldb_close (db, keywork.id);
			return web_return_application_json_msg(wp, ERROR, "无法申请内存", NULL);
		}
		/*
		 * 根据条件获取ID表
		 */
		ret = v9_video_sqldb_select_by_keywork (db, &keywork, keywork.get_outid,
												&keywork.get_idcnt);
		v9_app_snapfea_key_free(&keywork.key);
		if (ret != OK)
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(ZLOG_WEB, "Can not Select db by keywork Value");
			XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
			keywork.get_outid = NULL;
			v9_video_sqldb_close (db, keywork.id);
			memset (&keywork, 0, sizeof(v9_cap_keywork_t));
			return web_return_application_json_msg(wp, ERROR, "没有查询到相关数据", NULL);
		}

		/*
		 * 根据ID获取抓拍或告警视频信息
		 */
		u_int32 i = 0;

		v9_video_cap_t cap;
		//keywork.table = 1;
		memset (&cap, 0, sizeof(v9_video_cap_t));

		websSetStatus (wp, 200);
		websWriteHeaders (wp, -1, 0);
		websWriteHeader (wp, "Content-Type", "application/json");
		websWriteEndHeaders (wp);
		//websWrite (wp, "%s", "[");
		websWrite (wp, "{\"response\":\"OK\", \"msg\":\"%d\", \"data\":[", keywork.get_idcnt);
		for (i = keywork.get_index; i < keywork.get_idcnt; i++)
		{
			ret = v9_video_sqldb_get_capinfo (db, keywork.id, keywork.table,
											  keywork.get_outid[i], &cap);
			v9_app_snapfea_key_free(&cap.key);
			if (ret == OK)
			{
				if (wp->iValue > 0)
					websWrite (wp, "%s", ",");

				memset(&user, 0, sizeof(v9_video_user_t));
				if(!str_isempty(cap.userid, sizeof(cap.userid))/*strlen(cap.userid)*/)
				{
					//printf("========%s===========(%s)\r\n", __func__, cap.userid);
					v9_video_user_lookup_user_url(keywork.id, cap.userid, &user);
					v9_app_snapfea_key_free(&user.key);
				}

				stream = v9_video_board_stream_lookup_by_id_and_ch(keywork.id, cap.channel+1);
				//if(wp->iValue == 0)
				websWrite (
						wp,
						"{\"id\":%d, \"username\":\"%s\", \"userid\":\"%s\", \"groupname\":\"%s\", \"key\":\"%d\", \"address\":\"%s\",\"channel\":%d, "
						"\"datetime\":\"%s\", \"url\":\"%s\", \"keyurl\":\"%s\", \"videourl\":\"%s\"}",
						cap.keyid, cap.username, cap.userid,
						v9_video_usergroup_idtoname(keywork.id, cap.group)?v9_video_usergroup_idtoname(keywork.id, cap.group):"Unknow",
						(int)cap.key.output_result,
						stream?inet_address(stream->address):"0.0.0.0", cap.channel+1,
						cap.datetime,
						!str_isempty(cap.picname, sizeof(cap.picname)) ? web_video_picpath(keywork.id, cap.picname):" ",
						!str_isempty(user.picname, sizeof(user.picname))? user.picname:" ",
						!str_isempty(cap.videoname, sizeof(cap.videoname)) ? web_video_picpath(keywork.id, cap.videoname):" ");
/*						strlen(cap.picname) ? web_video_picpath(keywork.id, cap.picname):" ",
						strlen(user.picname)?user.picname:" ",
						strlen(cap.videoname) ? web_video_picpath(keywork.id, cap.videoname):" ");*/

				wp->iValue++;
			}
			else
			{
				if(WEB_IS_DEBUG(EVENT))
					zlog_debug(ZLOG_WEB, "Can not Select by index id Value");
			}
/*			if(wp->iValue == 20)
				break;*/
		}

		wp->iValue = 0;
		websWrite (wp, "%s", "]}");
		websDone (wp);
		XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
		keywork.get_outid = NULL;
		v9_app_snapfea_key_free(&keywork.key);
		v9_video_sqldb_close (db, keywork.id);
		memset (&keywork, 0, sizeof(v9_cap_keywork_t));
		return OK;
	}
	else
	{
		if(keywork.get_outid)
		{
			XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
			keywork.get_outid = NULL;
		}
		v9_app_snapfea_key_free(&keywork.key);
		memset (&keywork, 0, sizeof(v9_cap_keywork_t));
		return web_return_application_json_msg(wp, ERROR, "打开数据库失败", NULL);
		//return web_return_text_plain (wp, ERROR);
	}
	return OK;
}


static int web_video_real_warn_handle(Webs *wp, char *path, char *query)
{
	int ret = 0;
	sqlite3 * db = NULL;
	char *strval = NULL;
	v9_video_user_t user;
	v9_video_stream_t *stream = NULL;
	web_assert(wp);
	//return web_return_application_json_msg(wp, ERROR, "没有查询到相关数据", NULL);
	wp->iValue = 0;
	if(keywork.get_outid)
	{
		XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
		keywork.get_outid = NULL;
	}
	v9_app_snapfea_key_free(&keywork.key);
	memset (&keywork, 0, sizeof(v9_cap_keywork_t));

	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_WEB, "Can not Get Board ID Value");
		return web_return_application_json_msg(wp, ERROR, "获取不到板卡ID", NULL);
	}
	keywork.id = atoi(strval);
	keywork.id = V9_APP_BOARD_CALCU_ID(keywork.id);
	keywork.limit = 6;//默认实时告警返回6个结果
	//keywork.age = 6;//默认实时告警返回6个结果
	keywork.table = 1;
	db = v9_video_sqldb_open (keywork.id, keywork.table);
	if (db)
	{
		keywork.get_outid = XMALLOC (MTYPE_VIDEO_TMP, 4 * 16);
		if (!keywork.get_outid)
		{
			memset (&keywork, 0, sizeof(v9_cap_keywork_t));
			v9_video_sqldb_close (db, keywork.id);
			return web_return_application_json_msg(wp, ERROR, "无法申请内存", NULL);
		}
		/*
		 * 获取最新的有限条数据
		 */
		ret = v9_video_sqldb_select_by_new (db, &keywork, keywork.get_outid,
												&keywork.get_idcnt);

		//db = NULL;
		if (ret != OK)
		{
			v9_video_sqldb_close (db, keywork.id);
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(ZLOG_WEB, "Can not Select db by keywork Value");
			XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
			keywork.get_outid = NULL;
			v9_app_snapfea_key_free(&keywork.key);
			memset (&keywork, 0, sizeof(v9_cap_keywork_t));
			return web_return_application_json_msg(wp, ERROR, "没有查询到相关数据", NULL);
		}
	//}
	//db = v9_video_sqldb_open (keywork.id, keywork.table);
	//if (db && keywork.get_outid)
	//{
		/*
		 * 根据ID获取抓拍或告警视频信息
		 */
		u_int32 i = 0;

		v9_video_cap_t cap;
		memset (&cap, 0, sizeof(v9_video_cap_t));

		websSetStatus (wp, 200);
		websWriteHeaders (wp, -1, 0);
		websWriteHeader (wp, "Content-Type", "application/json");
		websWriteEndHeaders (wp);
		//websWrite (wp, "%s", "[\"response\":\"OK\", \"data\":");
		websWrite (wp, "{\"response\":\"OK\", \"msg\":\"%d\", \"data\":[", keywork.get_idcnt);
		for (i = keywork.get_index; i < keywork.get_idcnt; i++)
		{
			memset (&cap, 0, sizeof(v9_video_cap_t));
			ret = v9_video_sqldb_get_capinfo (db, keywork.id, keywork.table,
											  keywork.get_outid[i], &cap);
			v9_app_snapfea_key_free(&cap.key);
			if (ret == OK)
			{
				if (wp->iValue > 0)
					websWrite (wp, "%s", ",");

				memset(&user, 0, sizeof(v9_video_user_t));
				if(!str_isempty(cap.userid, sizeof(cap.userid)))
				{
					v9_video_user_lookup_user_url(keywork.id, cap.userid, &user);
					v9_app_snapfea_key_free(&user.key);
				}
				stream = v9_video_board_stream_lookup_by_id_and_ch(keywork.id, cap.channel+1);

				websWrite (
						wp,
						"{\"id\":%d, \"username\":\"%s\", \"userid\":\"%s\", \"groupname\":\"%s\", \"key\":\"%d\", \"address\":\"%s\",\"channel\":%d, "
						"\"datetime\":\"%s\", \"url\":\"%s\", \"keyurl\":\"%s\", \"videourl\":\"%s\"}",
						cap.keyid, cap.username, cap.userid,
						v9_video_usergroup_idtoname(keywork.id, cap.group)?v9_video_usergroup_idtoname(keywork.id, cap.group):"Unknow",
						(int)cap.key.output_result,
						stream?inet_address(stream->address):"0.0.0.0", cap.channel+1,
						cap.datetime,
						!str_isempty(cap.picname, sizeof(cap.picname)) ? web_video_picpath(keywork.id, cap.picname):" ",
						!str_isempty(user.picname, sizeof(user.picname)) ? user.picname:" ",
						!str_isempty(cap.videoname, sizeof(cap.videoname)) ? web_video_picpath(keywork.id, cap.picname):" ");
				/*strlen(cap.picname) ? web_video_picpath(keywork.id, cap.picname):"",
				strlen(user.picname)? user.picname:"",
				strlen(cap.videoname) ? web_video_picpath(keywork.id, cap.videoname):"");*/

				wp->iValue++;
			}
			else
			{
				if(WEB_IS_DEBUG(EVENT))
					zlog_debug(ZLOG_WEB, "Can not Select by index id Value");
			}
/*			if(wp->iValue == 20)
				break;*/
		}

		wp->iValue = 0;
		websWrite (wp, "%s", "]}");
		websDone (wp);
		XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
		keywork.get_outid = NULL;

		v9_video_sqldb_close (db, keywork.id);
		v9_app_snapfea_key_free(&cap.key);
		v9_app_snapfea_key_free(&user.key);
		v9_app_snapfea_key_free(&keywork.key);
		memset (&keywork, 0, sizeof(v9_cap_keywork_t));
		return OK;
	}
	else
	{
		if(keywork.get_outid)
		{
			XFREE (MTYPE_VIDEO_TMP, keywork.get_outid);
			keywork.get_outid = NULL;
		}
		v9_app_snapfea_key_free(&keywork.key);
		memset (&keywork, 0, sizeof(v9_cap_keywork_t));
		return web_return_application_json_msg(wp, ERROR, "打开数据库失败", NULL);
		//return web_return_text_plain (wp, ERROR);
	}
	return OK;
}

int web_db_app(void)
{
	memset(&keywork, 0, sizeof(v9_cap_keywork_t));
	websFormDefine("dbsnap", web_video_snap_handle);
	websDefineAction("dbpicsnap", web_video_snap_pichandle);
	websFormDefine("dbwarn", web_video_warn_handle);
	websFormDefine("dbrealwarn", web_video_real_warn_handle);
	return 0;
}
