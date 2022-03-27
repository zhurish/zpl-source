/*
 * web_facelib_html.c
 *
 *  Created on: Apr 13, 2019
 *      Author: zhurish
 */

#define HAS_BOOL 1
#include "zpl_include.h"
#include "module.h"
#include "zmemory.h"
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

//#ifdef ZPL_APP_MODULE
#include "application.h"


typedef struct
{
	//zpl_uint8 flag;
	zpl_uint8 offset;
	zpl_uint32 count;
	zpl_uint32 total;
	zpl_uint8 board;
	int group;
	void *pweb;
}v9_web_user_t;

#define V9_VIDEO_PAGE_MAX	24


static int v9_video_callback_user(v9_video_user_t *user, void *v9web_user)
{
	v9_web_user_t *v9_web_user = v9web_user;
	web_assert(user);
	web_assert(v9web_user);
	Webs *wp = v9_web_user->pweb;
	if(user && wp)
	{
		char *picname = NULL;

		v9_web_user->count++;
		if(v9_web_user->count < v9_web_user->offset)
		{
			return OK;
		}
		if(wp->iValue < V9_VIDEO_PAGE_MAX)
		{
			if(wp->iValue > 0)
				websWrite(wp, "%s", ",");

			//picname = strrchr(user->picname, '/');
			//if(picname)
			//	picname++;
			//else
				picname = user->picname;

			websWrite(wp, "{\"name\":\"%s\", \"gender\":\"%s\", \"ID\":\"%s\"," \
				"\"BID\":%d, \"group\":%d, \"url\":\"%s%\",\"text\":\"%s%\"}",
				user->username,
				user->gender ? "男":"女",
				user->userid,
				V9_APP_BOARD_HW_ID(user->ID),
				user->group,
				picname,
				user->text);

			wp->iValue++;

			if((wp->iValue + 1) == V9_VIDEO_PAGE_MAX)
			{
				//printf("----------------%s-----------------iValue = %d\r\n", __func__, wp->iValue);
				v9_web_user->offset += (wp->iValue + 1);
				return OK;
			}
		}
		return OK;
	}
	else
	{
		_WEB_DBG_TRAP("%s:wp=%s user=%s\r\n",__func__,wp?"full":"null",user?"full":"null");
	}
	return ERROR;
}


static int web_facelib_all_detail(Webs *wp, char *path, char *query)
{
	web_assert(wp);
	char *tmp = NULL;
	zpl_uint32 id = 1;
	int group = 0;
	websAppPrivate_t *websPrivData = websAppPrivateGet(wp->wid);
	web_assert(websPrivData);
	v9_web_user_t *web_user = websPrivData->private_data[WEB_USER_PRIVATE_INDEX];
	if(web_user == NULL)
	{
		websPrivData->private_data[WEB_USER_PRIVATE_INDEX] = web_user = walloc(sizeof(v9_web_user_t));
		if(websPrivData->private_data[WEB_USER_PRIVATE_INDEX] == NULL)
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Can not Alloc Memory");
			return web_return_application_json_array(wp, ERROR, "未能获取缓存", NULL);
			//return web_return_text_plain(wp, ERROR);
		}
		memset(websPrivData->private_data[WEB_USER_PRIVATE_INDEX], 0, sizeof(v9_web_user_t));
		web_user = websPrivData->private_data[WEB_USER_PRIVATE_INDEX];
	}
	wp->iValue = 0;

	tmp = webs_get_var(wp, "ID", NULL);
	if (tmp == NULL)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Board ID Value");
		return web_return_application_json_array(wp, ERROR, "获取板卡ID失败", NULL);
		//return web_return_text_plain(wp, ERROR);
	}
	if(tmp)
	{
		//_WEB_DBG_TRAP("%s: ID=%s\r\n", __func__, tmp);
		id = atoi(tmp);
	}
	tmp = webs_get_var(wp, "group", NULL);
	if (tmp == NULL)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Group ID Value");
		return web_return_application_json_array(wp, ERROR, "获取分组ID失败", NULL);
		//return web_return_text_plain(wp, '[');
	}
	if(tmp)
	{
		//_WEB_DBG_TRAP("%s: group=%s\r\n", __func__, tmp);
		group = atoi(tmp);
	}
	tmp = webs_get_var(wp, "ACTION", NULL);
	if (tmp == NULL)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get ACTION Value");
		return web_return_application_json_array(wp, ERROR, "获取分页索引失败", NULL);
		//return web_return_text_plain(wp, ERROR);
	}
	if(tmp)
	{
		web_user->count = 0;
		web_user->offset = V9_VIDEO_PAGE_MAX * atoi(tmp) - V9_VIDEO_PAGE_MAX;//根据每页24项设置偏移
		//web_user->flag = 1;
	}
	if(group != web_user->group || web_user->board != id)//板卡或者分组ID变动，清除参数
	{
		web_user->total = 0;
		web_user->group = group;
		web_user->offset = 0;
		web_user->board = id;
	}
	if(web_user->total == 0)//获取分组人脸库数量
	{
		if(v9_video_user_count(V9_APP_BOARD_CALCU_ID(id), group, &web_user->total) == ERROR ||
				web_user->total == 0)
		{
			return web_return_application_json_array(wp, ERROR, "该分组没有数据", NULL);
		}
	}
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	//websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteHeader (wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);
	//websWrite(wp, "%s", "[");

	websWrite (wp, "{\"response\":\"OK\", \"msg\":\"%d\", \"data\":[", web_user->total);
	web_user->pweb = wp;

	v9_video_user_foreach(V9_APP_BOARD_CALCU_ID(id), group, v9_video_callback_user, web_user);

/*
	printf("----------------%s-----------------total=%d offset=%d count=%d iValue = %d\r\n", __func__,
		   web_user->total,
		   web_user->offset, web_user->count, wp->iValue);
*/

/*	if(wp->iValue < V9_VIDEO_PAGE_MAX || web_user->offset == web_user->total)//最后一页清除书数据
	{
		memset(web_user, 0, sizeof(v9_web_user_t));
	}*/
	wp->iValue = 0;
	//websWrite(wp, "%s", "]");
	websWrite (wp, "%s", "]}");
	websDone(wp);
	return OK;
}


static int web_video_facelib_delete(Webs *wp, void *p)
{
	zpl_uint32 id = 0;
	int ret = 0;
	char *strID = NULL;
	char *tmp = NULL;
	web_assert(wp);
	strID = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strID)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get User ID Value");
		ret = ERROR;
		goto err_out;
	}
	tmp = webs_get_var(wp, "BID", NULL);
	if(tmp)
	{
		//_WEB_DBG_TRAP("%s: BID=%s\r\n", __func__, tmp);
		id = atoi(tmp);
	}
	else
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Board ID Value");
		ret = ERROR;
		goto err_out;
	}
	if(strstr(strID, ",") == NULL)
		ret = v9_video_user_del_user(V9_APP_BOARD_CALCU_ID(id), strID);
	else
	{
		char *strid = strID;
		int i = 0, j = 0;
		char tmpid[64];
		memset(tmpid, 0 ,sizeof(tmpid));
		while(i < strlen(strID))
		{
			if(strid[i] != ',')
				tmpid[j++] = strid[i];
			else
			{
				ret |= v9_video_user_del_user(V9_APP_BOARD_CALCU_ID(id), tmpid);
				memset(tmpid, 0 ,sizeof(tmpid));
				j = 0;
			}
			i++;
		}
		if(strlen(tmpid))
		{
			ret |= v9_video_user_del_user(V9_APP_BOARD_CALCU_ID(id), tmpid);
			memset(tmpid, 0 ,sizeof(tmpid));
		}
	}
	if(ret == ERROR)
	{
		if(WEB_IS_DEBUG(EVENT))
			zlog_debug(MODULE_WEB, "Can not Del User");
	}
err_out:
	if(ret != OK)
		return ERROR;//
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);

	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	if(ret == OK)
		websWrite(wp, "%s", "OK");
	else
		websWrite(wp, "%s", "ERROR");
	websDone(wp);
	return OK;
}



#if 0
static void web_video_facelib_and_upload(Webs *wp, char *path, char *query)
{
	zpl_uint32 id = 0;
	zpl_bool gender = zpl_false;
	int group = 0, ret = ERROR;
	char *user = NULL;
	char *user_id = NULL;
	//char *pic = NULL;
	char *tmp = NULL;
	WebsKey *s = NULL;
	WebsUpload *up = NULL;
	char uploadfile[256];
	if (scaselessmatch(wp->method, "POST"))
	{
		for (s = hashFirst(wp->files); s; s = hashNext(wp->files, s))
		{
			up = s->content.value.symbol;
/*
			user = webs_get_var(wp, "upload_pic", NULL);
			if (id != NULL)
				printf("%s: upload_pic=%s\r\n", __func__, user);
*/
			user = webs_get_var(wp, "name", NULL);
/*			if (user != NULL)
				printf("%s: name=%s\r\n", __func__, user);*/
			tmp = webs_get_var(wp, "gender", NULL);
/*			if (tmp != NULL)
				printf("%s: gender=%s\r\n", __func__, tmp);*/
			if(tmp)
			{
				gender = atoi(tmp);
			}
			user_id = webs_get_var(wp, "ID", NULL);
/*			if (user_id != NULL)
				printf("%s: ID=%s\r\n", __func__, user_id);*/
			tmp = webs_get_var(wp, "group", NULL);
/*			if (tmp != NULL)
				printf("%s: group=%s\r\n", __func__, tmp);*/
			if(tmp)
			{
				group = atoi(tmp);
			}
			tmp = webs_get_var(wp, "BID", NULL);
			if (tmp != NULL)
				printf("%s: ID=%s\r\n", __func__, tmp);
			if(tmp)
			{
				id = atoi(tmp);
			}
/*
			printf("%s: FILE=%s\r\n", __func__, s->name.value.string); //input=file id=
			printf("%s: FILENAME=%s\r\n", __func__, up->filename); //缓存文件
			printf("%s: CLIENT=%s\r\n", __func__, up->clientFilename); //实际文件名称
			printf("%s: TYPE=%s\r\n", __func__, up->contentType);
			printf("%s: SIZE=%d\r\n", __func__, up->size);
			printf("%s/%s", WEB_UPLOAD_BASE, up->clientFilename);
*/
			memset(uploadfile, 0, sizeof(uploadfile));
			sprintf(uploadfile, "%s/%s", WEB_UPLOAD_BASE, up->clientFilename);
			if (rename(up->filename, uploadfile) < 0)
			{
				web_return_text_plain(wp, ERROR);
				return ;
			}
			//pic = up->clientFilename;
			//pic = uploadfile;

			printf("rename %s -> %s", up->filename, uploadfile);
			sync();
			ret = v9_video_user_add_user(V9_APP_BOARD_CALCU_ID(id),  gender,  group, user, user_id, uploadfile);

		}
		web_return_text_plain(wp, ret);
		return ;
	}
	web_return_text_plain(wp, ERROR);
	return ;
}


static int web_video_facelib_show(Webs *wp, void *p)
{

	int ret = 0;
	char *strID = NULL;
	strID = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strID)
	{
		ret = ERROR;
		goto err_out;
	}

/*	ret = v9_video_user_del_user(0, strID);*/
	super_system("cp /tmp/app/tftpboot/113418.jpg /tmp/app/www/cache/face.jpg");
	sync();
	ret = OK;
err_out:
	if(ret != OK)
		return ERROR;//
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);

	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	if(ret == OK)
		websWrite(wp, "%s", "../cache/face.jpg");
	else
		websWrite(wp, "%s", "ERROR");
	websDone(wp);
	return OK;
}
#endif

static int web_video_facelib_add(Webs *wp, char *path, char *query)
{
	zpl_uint32 id = 0;
	zpl_bool gender = zpl_false;
	int group = 0, ret = ERROR;
	char *user = NULL;
	char *user_id = NULL;
	char *tmp = NULL;
	char *text = NULL;
	char uploadfile[256];
	char tempfile[256];
	web_assert(wp);
	user = webs_get_var(wp, "name", NULL);
	if(!user)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get User Name Value");
		return web_return_text_plain(wp, ERROR);
	}
	tmp = webs_get_var(wp, "gender", NULL);
	if(!tmp)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get gender Value");
		return web_return_text_plain(wp, ERROR);
	}
	gender = atoi(tmp);
	user_id = webs_get_var(wp, "ID", NULL);
	if(!user_id)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get User ID Value");
		return web_return_text_plain(wp, ERROR);
	}
	tmp = webs_get_var(wp, "group", NULL);
	if(!tmp)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(tmp)
	{
		group = atoi(tmp);
	}

	tmp = webs_get_var(wp, "BID", NULL);
	if(!tmp)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Board ID Value");
		return web_return_text_plain(wp, ERROR);
	}
	if(tmp)
	{
		id = atoi(tmp);
	}
	tmp = webs_get_var(wp, "upload_pic", NULL);
	if(!tmp)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Pic Name Value");
		return web_return_text_plain(wp, ERROR);
	}


	text = webs_get_var(wp, "text", NULL);

	memset(uploadfile, 0, sizeof(uploadfile));
	memset(tempfile, 0, sizeof(tempfile));
	//sprintf(uploadfile, "%s/%s", WEB_UPLOAD_BASE, tmp);
	sprintf(tempfile, "%s/%s", V9_USER_DB_DIR, tmp);

	if (os_file_access(tempfile) != OK)
	{
		return web_return_text_plain(wp, ERROR);
	}

	memset(uploadfile, 0, sizeof(uploadfile));
	snprintf(uploadfile, sizeof(uploadfile), "%s/table%d", V9_USER_DB_DIR, (id));
	if(access(uploadfile, F_OK) != 0)
		mkdir(uploadfile, 0644);


	tmp = strrchr(tmp, '.');

	memset(uploadfile, 0, sizeof(uploadfile));
	snprintf(uploadfile, sizeof(uploadfile), "%s/table%d/%s%s", V9_USER_DB_DIR,
			 (id), user_id, tmp);

	//sprintf(uploadfile, "%s/%s%s", V9_USER_DB_DIR, user_id, tmp);
	rename(tempfile, uploadfile);
	sync();
	_WEB_DBG_TRAP("%s: mv %s %s\r\n", __func__, tempfile, uploadfile);


	ret = v9_video_user_add_user(V9_APP_BOARD_CALCU_ID(id),  gender,  group, user, user_id, uploadfile, text);
	if(ret == ERROR)
	{
		remove(uploadfile);
		sync();
		if(WEB_IS_DEBUG(EVENT))
			zlog_debug(MODULE_WEB, "Can not Add User");
	}
	return web_return_text_plain(wp, ret);
}


/***********************************************************************************/
/***********************************************************************************/
static int web_facegroup_all(Webs *wp, char *path, char *query)
{
	char *tmp = NULL;
	zpl_uint32 i = 0;
	zpl_uint32 id = 1;
	web_assert(wp);
	tmp = webs_get_var(wp, "ID", NULL);
	if (tmp == NULL)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Board ID Value");
		return web_return_text_plain(wp, ERROR);
	}
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	wp->iValue = 0;
	websWrite(wp, "%s", "[");
	if(tmp)
	{
		id = V9_APP_BOARD_CALCU_ID(atoi(tmp));
	}
	for(i = 0; i < APP_GROUP_MAX; i++)
	{
		//if(GROUP_ACTIVE(_group_tbl[ID_INDEX(id)].ID))
		{
			if(GROUP_ACTIVE(_group_tbl[ID_INDEX(id)].gtbl[i].groupid))
			{
				if(wp->iValue > 0)
					websWrite(wp, "%s", ",");

				websWrite(wp, "{\"groupname\":\"%s\", \"group\":%d}",
						  _group_tbl[ID_INDEX(id)].gtbl[i].groupname,
						  GROUP_INDEX(_group_tbl[ID_INDEX(id)].gtbl[i].groupid));

				wp->iValue++;
			}
		}
	}
	wp->iValue = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	return OK;
}

static int web_video_facegroup_handle(Webs *wp, char *path, char *query)
{
	zpl_uint32 id = 0;
	int ret = ERROR;

	char *tmp = NULL;
	web_assert(wp);
	tmp = webs_get_var(wp, "ID", NULL);
	if(!tmp)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Board ID Value");
		return web_return_text_plain(wp, ERROR);
	}
	id = V9_APP_BOARD_CALCU_ID(atoi(tmp));


	tmp = webs_get_var(wp, "ACTION", NULL);
	if(!tmp)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get ACTION Value");
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(tmp, "add"))
	{
		tmp = webs_get_var(wp, "groupname", NULL);
		if(!tmp)
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Can not Get groupname Value");
			return web_return_text_plain(wp, ERROR);
		}
		ret = v9_video_usergroup_add( id, tmp);
		if(ret == ERROR)
		{
			if(WEB_IS_DEBUG(EVENT))
				zlog_debug(MODULE_WEB, "Can not Add Group");
		}
		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);

		if(ret != ERROR)
		{
			websWrite(wp,
				"{\"response\":\"%s\", \"group\":%d, \"groupname\":\"%s\"}", "OK", ret, tmp);
		}
		else
		{
			websWrite(wp,
				"{\"response\":\"%s\", \"group\":%d, \"groupname\":\"%s\"}", "ERROR", 0, tmp);
		}
		websDone(wp);
		return OK;
	}
	else if(strstr(tmp, "del"))
	{
		tmp = webs_get_var(wp, "group", NULL);
		if(!tmp)
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Can not Get Group ID Value");
			return web_return_text_plain(wp, ERROR);
		}
		ret = v9_video_usergroup_del( id, atoi(tmp));
		if(ret == ERROR)
		{
			if(WEB_IS_DEBUG(EVENT))
				zlog_debug(MODULE_WEB, "Can not Del Group");
		}
	}
	else if(strstr(tmp, "rename"))
	{
		int group = 0;
		tmp = webs_get_var(wp, "group", NULL);
		if(!tmp)
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Can not Get Group ID Value");
			return web_return_text_plain(wp, ERROR);
		}
		group = atoi(tmp);
		tmp = webs_get_var(wp, "groupname", NULL);
		if(!tmp)
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Can not Get Group Name Value");
			return web_return_text_plain(wp, ERROR);
		}
		ret = v9_video_usergroup_rename(id, group, tmp);
		if(ret == ERROR)
		{
			if(WEB_IS_DEBUG(EVENT))
				zlog_debug(MODULE_WEB, "Can not Rename Group");
		}
		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);

		if(ret != ERROR)
		{
			websWrite(wp,
				"{\"response\":\"%s\", \"group\":%d, \"groupname\":\"%s\"}", "OK", ret, tmp);
		}
		else
		{
			websWrite(wp,
				"{\"response\":\"%s\", \"group\":%d, \"groupname\":\"%s\"}", "ERROR", 0, tmp);
		}
		websDone(wp);
		return OK;
	}
	else
		return web_return_text_plain(wp, ERROR);
	return web_return_text_plain(wp, ret);
}
/***********************************************************************************/
/***********************************************************************************/

int web_facelib_app(void)
{
	//memset(&web_user, 0, sizeof(web_user));
	websFormDefine("allfacelib", web_facelib_all_detail);
	web_button_add_hook("facelib", "delete", web_video_facelib_delete, NULL);
	//web_button_add_hook("facelib", "loadpic", web_video_facelib_show, NULL);

	//websDefineAction("facelib", web_video_facelib_and_upload);
	websDefineAction("facelib", web_video_facelib_add);

	websFormDefine("facegroupall", web_facegroup_all);
	websFormDefine("facegroup", web_video_facegroup_handle);

	return 0;
}
//#endif
