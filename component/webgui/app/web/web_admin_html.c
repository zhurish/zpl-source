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



static int web_admin_user(Webs *wp, char *path, char *query)
{
	int ret = 0;
	char *strval = NULL;
	char username[64];
	char password[64];
	char authlevel[64];
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	memset(authlevel, 0, sizeof(authlevel));

	strval = webs_get_var(wp, T("username"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(username, strval);

	strval = webs_get_var(wp, T("password"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(password, strval);

	strval = webs_get_var(wp, T("user_level"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(authlevel, strval);
	if(vty_user_create(NULL, username, password, FALSE , TRUE ) == CMD_SUCCESS)
	{
		if(strstr(authlevel, "mana"))
			ret = vty_user_setting_privilege(NULL, username, ADMIN_LEVEL);
		else if(strstr(authlevel, "user"))
			ret = vty_user_setting_privilege(NULL, username, CONFIG_LEVEL);
		else //if(strstr(authlevel, "mana"))
			ret = vty_user_setting_privilege(NULL, username, ENABLE_LEVEL);
		//else
		//	ret = ERROR;
	}
	else
		ret = ERROR;

	//os_sleep(1);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	if(ret == ERROR)
		websWrite(wp, "%s", "ERROR");
	else
		websWrite(wp, "%s", "OK");
	websDone(wp);
	//websRedirect(wp, "/html/admin.html");
	return OK;
}

static int web_admin_change_password(Webs *wp, char *path, char *query)
{
	char *strval = NULL;
	char password[128];
	memset(password, 0, sizeof(password));
	strval = webs_get_var(wp, T("new_password"), T(""));

	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);

	}
	strcpy(password, strval);
	if(vty_user_create(NULL, wp->username, password, FALSE , TRUE ) == CMD_SUCCESS)
	{
		return web_return_text_plain(wp, OK);
	}
	return web_return_text_plain(wp, ERROR);
}

static int web_admin_deluser(Webs *wp, void *p)
{
	char *strval = NULL;
	char username[128];
	memset(username, 0, sizeof(username));
	strval = webs_get_var(wp, T("username"), T(""));

	if (NULL == strval)
	{
		return ERROR;//web_return_text_plain(wp, ERROR);

	}
	strcpy(username, strval);
	if(vty_user_delete(NULL, wp->username, FALSE , TRUE ) == CMD_SUCCESS)
	{
		return web_return_text_plain(wp, OK);
	}
	return ERROR;//web_return_text_plain(wp, ERROR);
}


static int web_username_one(struct vty_user *user, Webs *wp)
{
	if(wp->iValue)
		websWrite(wp, ",");

	websWrite(wp, "{\"username\":\"%s\", \"level\":\"%s\"}",
				user->username,
				(user->privilege == 4) ? "manage" :
				(user->privilege == 3) ? "user" : "view");
	wp->iValue++;
	/*
	 vty_out(vty,"username %s%s", user->username, VTY_NEWLINE);
	 else
	 {
	 if(user->encrypt && md5_encrypt_empty(user->password_encrypt))
	 vty_out(vty,"username %s password %s%s", user->username,
	 (user->password_encrypt), VTY_NEWLINE);
	 else if(user->password)
	 vty_out(vty,"username %s password %s%s", user->username,
	 user->password, VTY_NEWLINE);
	 }

	 if(user->privilege)
	 vty_out(vty,"username %s privilege %d%s", user->username, user->privilege, VTY_NEWLINE);
	 */
	return OK;
}

static int web_username_tbl(Webs *wp, char *path, char *query)
{
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
	vty_user_foreach (web_username_one, wp);
/*
	if (strlen(username))
	{
		websWrite(wp, "{\"username\":\"%s\", \"level\":\"%s\"},", username,
				authlevel);
	}
	websWrite(wp, "{\"username\":\"%s\", \"level\":\"%s\"},", "admin",
			"manage");
	websWrite(wp, "{\"username\":\"%s\", \"level\":\"%s\"},", "abcdef", "user");
	websWrite(wp, "{\"username\":\"%s\", \"level\":\"%s\"}", "useaa", "view");
*/

	websWrite(wp, "%s", "]");
	wp->iValue = 0;
	//websWrite(wp, "%s", "OK");
	websDone(wp);
	return 0;
}

int web_admin_app(void)
{
	websFormDefine("adminuser", web_admin_user);
	websFormDefine("admin-password", web_admin_change_password);
	//websFormDefine("admin-sshkeys", web_admin_ssh_keys);
	websFormDefine("username-tbl", web_username_tbl);

	web_button_add_hook("usertbl", "delete", web_admin_deluser, NULL);

	return 0;
}
