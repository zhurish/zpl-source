/*
 * web_facelib_html.c
 *
 *  Created on: Apr 13, 2019
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


static int v9_video_callback_user(v9_video_user_t *user, void *pweb)
{
	Webs *wp = pweb;
	if(user && wp)
	{
		char *picname = NULL;
		if(wp->iValue > 0)
			websWrite(wp, "%s", ",");

		picname = strrchr(user->picname, '/');
		if(picname)
			picname++;
		else
			picname = user->picname;

		websWrite(wp, "{\"name\":\"%s\", \"gender\":\"%s\", \"ID\":\"%s\"," \
			"\"BID\":%d, \"group\":\"%s\", \"pic\":\"%s%\"}",
			user->username,
			user->gender ? "男":"女",
			user->userid,
			user->ID,
			user->group ? "白名单":"黑名单",
			picname);
		wp->iValue++;
		return OK;
	}
	return ERROR;
}


static int web_facelib_all_detail(Webs *wp, char *path, char *query)
{
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;

	v9_video_user_foreach(v9_video_callback_user, wp);

	wp->iValue = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	return OK;
}

/*
static int web_video_facelib_add(Webs *wp, char *path, char *query)
{
	int ret = 0;
	v9_video_channel_t facelib;
	char *value = NULL;
	memset(&facelib, 0, sizeof(v9_video_channel_t));

	value = webs_get_var(wp, T("address"), T(""));
	if (NULL == value)
	{
		return web_return_text_plain(wp, ERROR);
	}
	facelib.address = ntohl(inet_addr(value));

	value = webs_get_var(wp, T("username"), T(""));
	if (NULL != value)
	{
		strcpy(facelib.username, value);
	}
	value = webs_get_var(wp, T("password"), T(""));
	if (NULL != value)
	{
		strcpy(facelib.password, value);
	}

	value = webs_get_var(wp, T("rstpport"), T(""));
	if (NULL != value)
	{
		facelib.port = atoi(value);
	}
	value = webs_get_var(wp, T("fps"), T(""));
	if (NULL != value)
	{
		facelib.fps = atoi(value);
	}
	value = webs_get_var(wp, T("boardid"), T(""));
	if (NULL != value)
	{
		facelib.id = atoi(value);
	}
	ret =  OK;//v9_video_facelib_add_api(facelib.id, facelib.ch, facelib.address, facelib.port,
	//							   facelib.username, facelib.password, facelib.fps);
	if(ret == OK)
		return web_return_text_plain(wp, OK);
	else
		return web_return_text_plain(wp, ERROR);
}
*/


static int web_video_facelib_delete_one(Webs *wp, char *path, char *query, int type)
{
	int ret = 0;
	char *strID = NULL;
	strID = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strID)
	{
		ret = ERROR;
		goto err_out;
	}

	ret = v9_video_user_del_user(0, strID);

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

static int web_video_facelib_delete(Webs *wp, void *p)
{
	return web_video_facelib_delete_one(wp, NULL, NULL, 0);
}




static void web_video_facelib_and_upload(Webs *wp, char *path, char *query)
{
	u_int32 id = 0;
	BOOL gender = FALSE;
	int group = 0, ret = ERROR;
	char *user = NULL;
	char *user_id = NULL;
	char *pic = NULL;
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
/*			if (tmp != NULL)
				printf("%s: group=%s\r\n", __func__, tmp);*/
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
			pic = uploadfile;

			printf("rename %s -> %s", up->filename, uploadfile);
			sync();
			ret = v9_video_user_add_user( id,  gender,  group, user, user_id, pic);
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

int web_facelib_app(void)
{
	websFormDefine("allfacelib", web_facelib_all_detail);
	web_button_add_hook("facelib", "delete", web_video_facelib_delete, NULL);
	web_button_add_hook("facelib", "loadpic", web_video_facelib_show, NULL);

	websDefineAction("facelib", web_video_facelib_and_upload);
	//websFormDefine("addfacelib", web_video_facelib_add);
	return 0;
}
//#endif
