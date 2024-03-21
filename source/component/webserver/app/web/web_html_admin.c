/**
 * @file      : web_html_admin.c
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-02-05
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
 */
#define HAS_BOOL 1
#include "goahead.h"
#include "webutil.h"

#include "vty.h"
#include "vty_user.h"

#include "web_api.h"
#include "web_app.h"

#if 0//ME_GOAHEAD_AUTO_LOGIN
static bool webs_authentication_verify(Webs *wp, cchar *username, cchar *password)
{
    web_assert(wp);
    web_assert(wp->route);
    web_assert(username);
    web_assert(password);

    if (!wp->route || !wp->route->verify) {
        return 0;
    }
    wfree(wp->username);
    wp->username = sclone(username);
    wfree(wp->password);
    wp->password = sclone(password);

    if (!(wp->route->verify)(wp)) {
        web_trace(WEBS_ERROR, "Password does not match");
        return 0;
    }
    web_trace(WEBS_INFO, "Authentication Successful for %s", username);
    return 1;
}
#endif

static int web_admin_username_del(Webs *wp, void *p)
{
	#if ME_GOAHEAD_JSON	
	cJSON *root = websGetJsonVar(wp);
	if(root)
	{
		char *username = cJSON_GetStringValue(root, "username");
		if(username)
		{
			if(web_app_username_lookup_api(username) == OK && vty_user_lookup(username))
			{
				if(web_app_username_del_api(username) == OK && vty_user_delete(NULL, username, zpl_true, zpl_false) == OK)
					return web_json_format_result(wp, 0, "OK");
			}
		}
		return web_json_format_result(wp, HTTP_CODE_INTERNAL_SERVER_ERROR, "can not get username or delete user faild");
	}
	web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
	#else
	web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
	#endif
	return ERROR;
}

static int web_admin_add_username(Webs *wp, char *username, char *password, char *authlevel)
{
	int ret = ERROR;
	web_assert(wp != NULL);

	if(web_app_username_add_api(username, password, authlevel) == OK &&
		vty_user_create(NULL, username, password, zpl_true, zpl_false) == OK)
	{
		if(strstr(authlevel, "mana"))
			ret = vty_user_setting_privilege(NULL, username, CMD_ADMIN_LEVEL);
		else if(strstr(authlevel, "user"))
			ret = vty_user_setting_privilege(NULL, username, CMD_CONFIG_LEVEL);
		else
			ret = vty_user_setting_privilege(NULL, username, CMD_ENABLE_LEVEL);
		if(ret == OK)
		{
			char encodedPassword[128];
			memset(encodedPassword, 0, sizeof(encodedPassword));
			if(web_app_gopass_api(username, password, "md5", "goahead.com", encodedPassword) == 0 )
			{
				websSetUserPassword(username, encodedPassword);
				web_app_auth_save_api();
				ret = OK;
			}
			else
				ret = ERROR;
		}	
	}
	else
	{
		if(WEB_IS_DEBUG(EVENT))
		{
			zlog_debug(MODULE_WEB, " Can not Create User for '%s'", username);
		}
		ret = ERROR;
	}
	return ret;
}


static int web_admin_change_password(Webs *wp, char *username, char *password, char *authlevel)
{
	//int ret = ERROR;
	char encodedPassword[128];
	memset(encodedPassword, 0, sizeof(encodedPassword));
	/*
	 * Password Encoded
	 */
	//webserver encoded cipher md5 realm goahead.com username admin password admin
	if(web_app_gopass_api(username, password, "md5", "goahead.com", encodedPassword) != 0 )
	{
		return ERROR;//web_textplain_result(wp, HTTP_CODE_BAD_REQUEST, "Can not Encoded password");
	}
	websSetUserPassword(username, encodedPassword);
	web_app_auth_save_api();
	return OK;//web_textplain_result(wp, OK, NULL);
}


static int web_username_one(struct vty_user *user, Webs *wp)
{
	#if ME_GOAHEAD_JSON	
	cJSON *obj = cJSON_CreateObject();
	if(obj)
	{
		cJSON_AddStringToObject(obj,"username", user->username);
		cJSON_AddStringToObject(obj,"level", (user->privilege == 4) ? "manage" :
				(user->privilege == 3) ? "user" : "view");
		cJSON_AddItemToArray(wp->pArgv, obj);
		wp->iValue++;
	}
	#endif
	return OK;
}

static int web_username_form(Webs *wp, char *path, char *query)
{
	if(websGetMethodCode(wp) == WEBS_METHOD_GET)
	{
		#if ME_GOAHEAD_JSON	
		cJSON *obj = cJSON_CreateObject();
		if(obj)
		{
			wp->iValue = 0;
			wp->pArgv = obj;
			vty_user_foreach (web_username_one, wp);
			wp->iValue = 0;
			wp->pArgv = NULL;
		}
		websResponseJson(wp, HTTP_CODE_OK, obj);
		return OK;
		#endif
	}
	else if(websGetMethodCode(wp) == WEBS_METHOD_POST)
	{
		int ret = ERROR;
		#if ME_GOAHEAD_JSON	
		cJSON *root = websGetJsonVar(wp);
		if(root)
		{
			char *username = cJSON_GetStringValue(root, "username");
			char *password = cJSON_GetStringValue(root, "password");
			char *authlevel = cJSON_GetStringValue(root, "authlevel");
			if(username && password)
			{
				if(web_app_username_lookup_api(username) == OK)
				{
					if(authlevel)
					{
						ret = web_admin_change_password(wp, username, password, authlevel);
					}
					else
						return web_json_format_result(wp, HTTP_CODE_INTERNAL_SERVER_ERROR, "can not get username auth level value");
					if(ret == OK)
						return web_json_format_result(wp, 0, "OK");
					else
						return web_json_format_result(wp, ret, "failed");	
				}
				else
				{
					if(authlevel)
					{
						ret = web_admin_add_username(wp, username, password, authlevel);
					}
					else
						return web_json_format_result(wp, HTTP_CODE_INTERNAL_SERVER_ERROR, "can not get username auth level value");
					if(ret == OK)
						return web_json_format_result(wp, 0, "OK");
					else
						return web_json_format_result(wp, ret, "failed");	
				}
			}
			else
				return web_json_format_result(wp, HTTP_CODE_INTERNAL_SERVER_ERROR, "can not get username and password value");
		}
		else
			return web_json_format_result(wp, HTTP_CODE_INTERNAL_SERVER_ERROR, "can not get json payload");
		#endif
	}
	return web_json_format_result(wp, HTTP_CODE_BAD_METHOD, "this url only support get and post mehtod");
}


int web_html_admin_init(void)
{
	websFormDefine("usertable", web_username_form);
	web_button_add_hook("usertable", "button-delete", web_admin_username_del, NULL);
	return 0;
}
