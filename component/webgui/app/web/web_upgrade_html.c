/*
 * web_upgrade_html.c
 *
 *  Created on: 2020年3月25日
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

#ifdef PL_APP_MODULE
#include "application.h"
#endif /* PL_APP_MODULE */

#if 0
static int web_upgrade_action(Webs *wp, char *path, char *query, char *uploadfile)
{
	char *tmp = NULL;

	tmp = webs_get_var(wp, "gender", NULL);
	if(tmp)
	{
		//gender = atoi(tmp);
	}
	return OK;
}




static void web_upgrade(Webs *wp, char *path, char *query)
{
	int ret = 0;
	//char *tmp = NULL;
	WebsKey *s = NULL;
	WebsUpload *up = NULL;
	char uploadfile[256];
	if (scaselessmatch(wp->method, "POST"))
	{
		for (s = hashFirst(wp->files); s; s = hashNext(wp->files, s))
		{
			up = s->content.value.symbol;

			memset(uploadfile, 0, sizeof(uploadfile));
			sprintf(uploadfile, "%s/%s", WEB_UPLOAD_BASE, up->clientFilename);
			if (rename(up->filename, uploadfile) < 0)
			{
				web_return_text_plain(wp, ERROR);
				return ;
			}
			sync();
			ret = web_upgrade_action(wp, path, query, uploadfile);
		}
		web_return_text_plain(wp, ret);
		return ;
	}
	web_return_text_plain(wp, ERROR);
	return ;
}
#endif

static int web_upgrade_handle(Webs *wp, void *p)
{
	int id = 0, ret = 0;
	char *strval = NULL;
	char filetmp[512];
	memset (filetmp, 0, sizeof(filetmp));
	strval = webs_get_var (wp, T("BTNID"), T(""));
	if (NULL != strval)
	{
		if (strstr (strval, "upgrade"))
		{
			strval = webs_get_var (wp, T("filename"), T(""));
			memset (filetmp, 0, sizeof(filetmp));
			if (NULL == strval)
			{
				return ERROR;
			}
			snprintf (filetmp, sizeof(filetmp), "%s/%s", WEB_UPLOAD_BASE, strval);

			strval = webs_get_var (wp, T("ID"), T(""));
			if (NULL == strval)
			{
				return ERROR;
			}
			id = atoi(strval);
#ifdef V9_VIDEO_SDK_API
			if(id == 0)
			{
				ret = 0;
				ret |= v9_video_sdk_update_api(APP_BOARD_CALCU_1, filetmp);
				ret |= v9_video_sdk_update_api(APP_BOARD_CALCU_2, filetmp);
				ret |= v9_video_sdk_update_api(APP_BOARD_CALCU_3, filetmp);
				ret |= v9_video_sdk_update_api(APP_BOARD_CALCU_4, filetmp);
			}
			else
			{
				ret = v9_video_sdk_update_api(V9_APP_BOARD_CALCU_ID(id), filetmp);
			}
#endif
			return web_return_text_plain (wp, ret);
		}
	}
	return ERROR; //;
}

static int web_system_action_job(void *a)
{
	char *file = a;
	if(file)
	{
		char upgradecmd[512];
		os_sleep(1);
		memset(upgradecmd, 0, sizeof(upgradecmd));
		sprintf(upgradecmd, "sysupgrade -n %s", file);
		free(file);
		super_system(upgradecmd);
	}
	return OK;
}

static int sys_upgrade_cb(Webs *wp, WebsUpload *up, void *p)
{
	char uploadfile[256];
	memset(uploadfile, 0, sizeof(uploadfile));
	sprintf(uploadfile, "%s/%s", WEB_UPLOAD_BASE, up->clientFilename);

	if (rename(up->filename, uploadfile) < 0)
	{
		return ERROR;
	}
	sync();
	if(strstr(uploadfile, "sysupgrade"))
	{
		os_job_add(web_system_action_job, strdup(uploadfile));
		return OK;
	}
	return ERROR;
}

int web_upgrade_app(void)
{
	//websDefineAction("upgrade", web_upgrade);
	web_button_add_hook("upgrade", "upgrade", web_upgrade_handle, NULL);
	web_upload_add_hook("upgrade_filename", "sysupgrade", sys_upgrade_cb, NULL);
	return 0;
}