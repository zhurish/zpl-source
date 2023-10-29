/*
 * web_upgrade_html.c
 *
 *  Created on: 2020年3月25日
 *      Author: zhurish
 */

#define HAS_BOOL 1
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "zassert.h"
#include "command.h"
#include "prefix.h"
#include "host.h"
#include "log.h"
#include "vty.h"
#include "vty_user.h"


#include "web_api.h"
#include "web_jst.h"
#include "web_app.h"

#ifdef ZPL_APP_MODULE
#include "application.h"
#endif /* ZPL_APP_MODULE */


static int web_upgrade_handle(Webs *wp, void *p)
{
	int id = 0, ret = 0;
	char *strval = NULL;
	char filetmp[512];
	memset (filetmp, 0, sizeof(filetmp));
	strval = webs_get_var (wp, T("button-ID"), T(""));
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
			return web_return_text_plain (wp, ret, NULL);
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
		sprintf(upgradecmd, "sysupgrade %s", file);
		free(file);
		super_system(upgradecmd);
	}
	return OK;
}

static int web_system_action_clean_job(void *a)
{
	char *file = a;
	if (file)
	{
		char upgradecmd[512];
		sprintf (upgradecmd, "sysupgrade -n %s", file);
		free (file);
		super_system (upgradecmd);
	}
	return OK;
}


static int sys_upgrade_cb(Webs *wp, WebsUpload *up, void *p)
{
	char *strval = NULL;
	char uploadfile[256];
	memset(uploadfile, 0, sizeof(uploadfile));
	sprintf(uploadfile, "%s/%s", WEB_UPLOAD_BASE, up->clientFilename);

	if (rename(up->filename, uploadfile) < 0)
	{
		return ERROR;
	}
	sync();

	strval = webs_get_var (wp, T("clean"), T(""));
	if (NULL == strval)
	{
		return ERROR;
	}
	if(strstr(strval, "true"))
	{
		if(strstr(uploadfile, "rtthread"))
		{
			int ret = 0;
			system("chmod 777 /tmp/app/tftpboot/rtthread.rbl");
			ret = 1;
			if(ret == 1)
				return OK;
			//if(ret == 1)
			return ERROR;
		}
		else if(strstr(uploadfile, "sysupgrade"))
		{
			os_job_add(OS_JOB_NONE,web_system_action_clean_job, strdup(uploadfile));
			return OK;
		}
	}
	else if(strstr(strval, "false"))
	{
		if(strstr(uploadfile, "sysupgrade"))
		{
			os_job_add(OS_JOB_NONE,web_system_action_job, strdup(uploadfile));
			return OK;
		}
	}
	return ERROR;
}

int web_upgrade_app(void)
{
	web_button_add_hook("upgrade", "upgrade", web_upgrade_handle, NULL);//计算板升级
	web_upload_add_hook("upgrade_filename", "sysupgrade", sys_upgrade_cb, NULL);
	web_upload_add_hook("upgrade_filename", "bios", sys_upgrade_cb, NULL);
	return 0;
}
