/*
 * web_util_html.c
 *
 *  Created on: 2019年8月23日
 *      Author: DELL
 */

#include "zebra.h"
#include "module.h"
#include "memory.h"
#include "zassert.h"
#include "command.h"
#include "prefix.h"
#include "host.h"
#include "log.h"
#include "vty.h"


#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"


int _web_app_debug = 0;



const char *webs_get_var(Webs *wp, const char *var, const char *defaultGetValue)
{
	const char *value = websGetVar(wp, var, defaultGetValue);
	if(value != NULL)
	{
		if(strlen(value) <= 0)
		{
			return NULL;
		}
		if(all_space(value))
			return NULL;
		if(strcasecmp(value, "undefined") == 0)
			return NULL;

		return str_trim(value);
	}
	return NULL;
}


int web_return_text_plain(Webs *wp, int ret)
{
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	if(ret == ERROR)
		websWrite(wp, "%s", "ERROR");
	else if(ret == OK)
		websWrite(wp, "%s", "OK");
	else if(ret == '[')
		websWrite(wp, "%s", "[]");
	websDone(wp);
	return ret;
}

int web_return_application_json(Webs *wp, char * json)
{
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);
	websWrite(wp, json);
	websDone(wp);
	return OK;
}

int web_return_text_plain_json(Webs *wp, char * json)
{
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, json);
	websDone(wp);
	return OK;
}



const char * web_type_string(web_app_t *wp)
{
	if(wp)
	{
		if(wp->webtype == WEB_TYPE_HOME_WIFI)
		{
			return "homewifi";
		}
		else if(wp->webtype == WEB_TYPE_HOME_SWITCH)
		{
			return "switch";
		}
		else if(wp->webtype == WEB_TYPE_HOME_ROUTE)
		{
			return "route";
		}
	}
	return "unknow";
}


const char * web_os_type_string(web_app_t *wp)
{
	if(wp)
	{
		if(wp->webos == WEB_OS_OPENWRT)
		{
			return "openwrt";
		}
		else if(wp->webos == WEB_OS_LINUX)
		{
			return "linux";
		}
	}
	return "unknow";
}


web_type web_type_get()
{
	if(web_app)
	{
		return web_app->webtype;
	}
	return WEB_TYPE_HOME_WIFI;
}

web_os web_os_get()
{
	if(web_app)
	{
		return web_app->webos;
	}
	return WEB_OS_OPENWRT;
}


int webs_username_password_update(void *pwp, char *username, char *password)
{
#if ME_GOAHEAD_AUTO_LOGIN
	char encodedPassword[128];
	memset(encodedPassword, 0, sizeof(encodedPassword));
	/*
	 * Password Encoded
	 */
	//webserver encoded cipher md5 realm goahead.com username admin password admin
	if(web_app_gopass_api(username, password, "md5", "goahead.com", encodedPassword) != 0 )
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_WEB, "Can not Encoded password Value");
		return ERROR;
	}
	websSetUserPassword(username, encodedPassword);
	web_app_auth_save_api();
#endif

#if !ME_GOAHEAD_AUTO_LOGIN
	if(vty_user_create(NULL, username, password, FALSE , TRUE ) != CMD_SUCCESS)
	{
		if(WEB_IS_DEBUG(EVENT))
		{
			zlog_debug(ZLOG_WEB, " Can not Change User Password for '%s'", username);
		}
		return ERROR;
	}
#endif
	return OK;
}
