/*
 * web_admin.c
 *
 *  Created on: Apr 13, 2019
 *      Author: zhurish
 */

/*
 * login.c
 *
 *  Created on: Mar 24, 2019
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



static int web_admin_add_username(Webs *wp, char *path, char *query)
{
	int ret = 0;
	char *strval = NULL;
	char username[64];
	char password[64];
	char authlevel[64];
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	memset(authlevel, 0, sizeof(authlevel));
	web_assert(wp != NULL);
	strval = webs_get_var(wp, T("username"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get username Value");
		return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "Can not Get username");
	}
	strcpy(username, strval);

	strval = webs_get_var(wp, T("password"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get password Value");
		return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "Can not Get password");
	}
	strcpy(password, strval);

	strval = webs_get_var(wp, T("user_level"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get user_level Value");
		return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "Can not Get User Private Level");
	}
	strcpy(authlevel, strval);
	if(WEB_IS_DEBUG(MSG) && WEB_IS_DEBUG(DETAIL))
	{
		zlog_debug(MODULE_WEB, " get action/adminuser username:%s password:%s authlevel:%s", username,password,authlevel);
	}
	if(vty_user_create(NULL, username, password, zpl_false , zpl_true ) == CMD_SUCCESS)
	{
		if(strstr(authlevel, "mana"))
			ret = vty_user_setting_privilege(NULL, username, CMD_ADMIN_LEVEL);
		else if(strstr(authlevel, "user"))
			ret = vty_user_setting_privilege(NULL, username, CMD_CONFIG_LEVEL);
		else
			ret = vty_user_setting_privilege(NULL, username, CMD_ENABLE_LEVEL);
	}
	else
	{
		if(WEB_IS_DEBUG(EVENT))
		{
			zlog_debug(MODULE_WEB, " Can not Create User for '%s'", username);
		}
		ret = ERROR;
	}
	websSetStatus(wp, 200);

	if(ret == ERROR)
		websWriteCache(wp, "%s", "ERROR");
	else
		websWriteCache(wp, "%s", "OK");

	websWriteHeaders (wp, websWriteCacheLen(wp), 0);
	websWriteHeader (wp, "Content-Type", "text/plain");
	websWriteEndHeaders (wp);	
	websWriteCacheFinsh(wp);

	websDone(wp);
	return OK;
}


static int web_admin_del_username(Webs *wp, void *p)
{
	char *strval = NULL;
	char username[128];
	memset(username, 0, sizeof(username));
	strval = webs_get_var(wp, T("username"), T(""));

	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get username Value");
		return ERROR;

	}
	strcpy(username, strval);

	if(vty_user_delete(NULL, wp->username, zpl_false , zpl_true ) == CMD_SUCCESS)
	{
		return web_return_text_plain(wp, OK, NULL);
	}
	if(WEB_IS_DEBUG(EVENT))
	{
		zlog_debug(MODULE_WEB, " Can not Delete User '%s'", username);
	}
	return ERROR;
}


static int web_username_one(struct vty_user *user, Webs *wp)
{
	if(wp->iValue)
		websWriteCache(wp, ",");

	websWriteCache(wp, "{\"username\":\"%s\", \"level\":\"%s\"}",
				user->username,
				(user->privilege == 4) ? "manage" :
				(user->privilege == 3) ? "user" : "view");
	wp->iValue++;

	return OK;
}

static int web_username_table(Webs *wp, char *path, char *query)
{
	websSetStatus(wp, 200);

	websWriteCache(wp, "%s", "[");
	wp->iValue = 0;
	vty_user_foreach (web_username_one, wp);

	websWriteCache(wp, "%s", "]");
	wp->iValue = 0;

	websWriteHeaders (wp, websWriteCacheLen(wp), 0);
	websWriteHeader (wp, "Content-Type", "text/plain");
	websWriteEndHeaders (wp);	
	websWriteCacheFinsh(wp);

	websDone(wp);
	return 0;
}


#if ME_GOAHEAD_AUTO_LOGIN
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

static int web_admin_change_password(Webs *wp, char *path, char *query)
{
	int ret = ERROR;
	char *username = NULL, *oldpassword = NULL;
	char *newpassword = NULL;
#if ME_GOAHEAD_AUTO_LOGIN
	char *olduser = NULL, *oldpass = NULL;
	char encodedPassword[128];
#endif
	newpassword = webs_get_var(wp, T("new-password"), T(""));

	if (NULL == newpassword)
	{
#if ME_GOAHEAD_AUTO_LOGIN
		if(olduser)
			wfree(olduser);
		if(oldpass)
			wfree(oldpass);
#endif
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get new password Value");
		return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "Can not Get new password");
	}

	username = webs_get_var(wp, T("username"), T(""));

	if (NULL == username)
	{
#if ME_GOAHEAD_AUTO_LOGIN
		if(olduser)
			wfree(olduser);
		if(oldpass)
			wfree(oldpass);
#endif
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get username Value");
		return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "Can not Get new username");
	}
	oldpassword = webs_get_var(wp, T("password"), T(""));

	if (NULL == oldpassword)
	{
#if ME_GOAHEAD_AUTO_LOGIN
		if(olduser)
			wfree(olduser);
		if(oldpass)
			wfree(oldpass);
#endif
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get old password Value");
		return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "Can not Get old password");
	}
#if ME_GOAHEAD_AUTO_LOGIN
	olduser = sclone(wp->username);
	oldpass = sclone(wp->password);

	ret = webs_authentication_verify(wp, username, oldpassword);
	if(ret == zpl_false)
	{
	    wfree(wp->username);
	    wp->username = sclone(olduser);
	    wfree(wp->password);
	    wp->password = sclone(oldpass);

		if(olduser)
			wfree(olduser);
		if(oldpass)
			wfree(oldpass);
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not verify password Value");

		return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "Can not verify password");
	}

	memset(encodedPassword, 0, sizeof(encodedPassword));
	/*
	 * Password Encoded
	 */
	//webserver encoded cipher md5 realm goahead.com username admin password admin
	if(web_app_gopass_api(username, newpassword, "md5", "goahead.com", encodedPassword) != 0 )
	{
	    wfree(wp->username);
	    wp->username = sclone(olduser);
	    wfree(wp->password);
	    wp->password = sclone(oldpass);

		if(olduser)
			wfree(olduser);
		if(oldpass)
			wfree(oldpass);
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Encoded password Value");

		return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "Can not Encoded password");
	}
	websSetUserPassword(username, encodedPassword);
	web_app_auth_save_api();
#endif

	if(WEB_IS_DEBUG(MSG) && WEB_IS_DEBUG(DETAIL))
	{
		zlog_debug(MODULE_WEB, " get action/admin-password username:%s password:%s", wp->username, newpassword);
	}

#if (ME_GOAHEAD_AUTO_LOGIN==0)
	if(user_authentication(username, oldpassword) != 0)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not verify password Value");

		return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "Can not verify password");
	}
	if(vty_user_create(NULL, wp->username, newpassword, zpl_false , zpl_true ) != CMD_SUCCESS)
	{
		if(WEB_IS_DEBUG(EVENT))
		{
			zlog_debug(MODULE_WEB, " Can not Change User Password for '%s'", wp->username);
		}
		return web_return_text_fmt(wp, HTTP_CODE_BAD_REQUEST, "Can not Change User Password for '%s'", wp->username);
	}
#endif
#if ME_GOAHEAD_AUTO_LOGIN
	if(olduser)
		wfree(olduser);
	if(oldpass)
		wfree(oldpass);
#endif
	return web_return_text_plain(wp, OK, NULL);
}


int web_admin_app(void)
{
	websFormDefine("addusername", web_admin_add_username);
	websFormDefine("changepassword", web_admin_change_password);
	websFormDefine("username-table", web_username_table);
	web_button_add_hook("username-table", "delete", web_admin_del_username, NULL);

	return 0;
}
