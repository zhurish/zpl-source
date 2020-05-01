/*
 * login.c
 *
 *  Created on: Mar 24, 2019
 *      Author: zhurish
 */

#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

char * web_encoded_password(char *username, char *password, char *realm,
		char *cipher)
{
	char *encodedPassword = websMD5(
			sfmt("%s:%s:%s", username, realm, password));

	if (smatch(cipher, "md5"))
	{ //blowfish md5
		encodedPassword = websMD5(sfmt("%s:%s:%s", username, realm, password));
	}
	else
	{
		/* This uses the more secure blowfish cipher */
		encodedPassword = websMakePassword(
				sfmt("%s:%s:%s", username, realm, password), 16, 128);
	}
	return encodedPassword;
}

#if !ME_GOAHEAD_AUTO_LOGIN
static int web_login_auth(Webs *wp, char *path, char *query)
{
	char *strval = NULL;
	char g_test_name[32];
	char g_test_pass[32];
	memset(g_test_name, 0, sizeof(g_test_name));
	memset(g_test_pass, 0, sizeof(g_test_pass));
	strval = webs_get_var(wp, T("username"), T(""));
	/* webs_get_var()可以从页面取出文本框内的数据 */
	if (NULL != strval)
	{
		/* 将从页面上取出的数据存入全局变量中 */
		strcpy(g_test_name, strval);
	}

	strval = webs_get_var(wp, T("password"), T(""));
	if (NULL != strval)
	{
		strcpy(g_test_pass, strval);
	}
	//printf("%s: %s %s\r\n", __func__, g_test_name, g_test_pass);
	if(user_authentication (g_test_name, g_test_pass) == OK)
	{
#ifdef THEME_V9UI
		return web_return_text_plain(wp, OK);
#else
		if(web_app && web_app->web_main)
			websRedirect(wp, web_app->web_main);
		else
			websRedirect(wp, "/html/main.html");
#endif
	}
	else
	{
	    websRemoveSessionVar(wp, WEBS_SESSION_USERNAME);
	    websDestroySession(wp);
#ifdef THEME_V9UI
	    return web_return_text_plain(wp, ERROR);
#else
		if(web_app && web_app->web_login)
			websRedirect(wp, web_app->web_login);
		else
			websRedirect(wp, "/html/login.html");
#endif
	}
	return 0;
}


static int web_system_logout(Webs *wp, void *p)
{
    web_assert(wp);
    websRemoveSessionVar(wp, WEBS_SESSION_USERNAME);
    websDestroySession(wp);
    if (smatch(wp->authType, "basic") || smatch(wp->authType, "digest")) {
        websError(wp, HTTP_CODE_UNAUTHORIZED, "Logged out.");
        return OK;
    }
#ifdef THEME_V9UI
	return web_return_text_plain(wp, OK);
#else
	if(web_app && web_app->web_logout)
		websRedirect(wp, web_app->web_logout);
	else
		websRedirect(wp, "/html/login.html");
#endif
	return OK;
}
#endif

int web_login_app(void)
{
#if !ME_GOAHEAD_AUTO_LOGIN
	websFormDefine("login", web_login_auth);
	websFormDefine("logout", web_system_logout);
#endif
	return 0;
}
