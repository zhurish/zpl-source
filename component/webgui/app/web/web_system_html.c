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
#endif/* PL_APP_MODULE */

static int web_system_handle(Webs *wp, char *path, char *query)
{
	int ret = 0;
	char *strval = NULL;
	struct ethaddr ether;
	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "GET"))
	{
		u_int8 sysmac[8];
		s_int8	serial[64];
		memset(sysmac, 0, sizeof(sysmac));
		memset(serial, 0, sizeof(serial));
		host_config_get_api(API_GET_SYSMAC_CMD, sysmac);
		host_config_get_api(API_GET_SERIAL_CMD, serial);

		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);
		websWrite(wp,
				"{\"response\":\"%s\", \"serial\":\"%s\", \"hostname\":\"%s\", \"mac\":\"%s\"}",
				"OK", serial, host_name_get(), inet_ethernet(sysmac));
		websDone(wp);
		_WEB_DBG_TRAP("----------------------%s--------------------------\r\n",__func__);
		return OK;
	}
	strval = webs_get_var(wp, T("hostname"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	ret |= host_config_set_api(API_SET_HOSTNAME_CMD, strval);

	strval = webs_get_var(wp, T("mac"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	ether_aton_r (strval, &ether);
	ret |= host_config_set_api(API_SET_SYSMAC_CMD, ether.octet);

	if(ret == OK)
	{
		u_int8 sysmac[8];
		s_int8	serial[64];
		memset(sysmac, 0, sizeof(sysmac));
		memset(serial, 0, sizeof(serial));
		host_config_get_api(API_GET_SYSMAC_CMD, sysmac);
		host_config_get_api(API_GET_SERIAL_CMD, serial);
		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);
		websWrite(wp,
				"{\"response\":\"%s\", \"serial\":\"%s\", \"hostname\":\"%s\", \"mac\":\"%s\"}",
				"OK", serial, host_name_get(), inet_ethernet(sysmac));
		websDone(wp);
		return OK;
	}

	return web_return_text_plain(wp, ERROR);
}

#if 0
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
#endif

static int web_webtime_sync(Webs *wp, void *p)
{
	char *strval = NULL;
	struct timespec sntpTime;	/* storage for retrieved time value */
	int value = 0;
	strval = webs_get_var(wp, T("timesp"), T(""));

	if (NULL == strval)
	{
		return ERROR;//web_return_text_plain(wp, ERROR);
	}
	sntpTime.tv_sec = os_timestamp_spilt(0,  strval);

	//sntpTime.tv_sec = atoi(strval);

	_WEB_DBG_TRAP( "-----%s------:%s\r\n", __func__, os_time_fmt ("date", sntpTime.tv_sec));

	value = 5;
	while(value)
	{
		errno = 0;
		if(clock_settime(CLOCK_REALTIME, &sntpTime)!= 0)//SET SYSTEM LOCAL TIME
		{
			value--;
		}
		else
		{
			break;
		}
	}
	if(value == 0)
	{
#ifdef PL_APP_MODULE
#ifdef APP_X5BA_MODULE
		if(x5b_app_mode_X5CM())
			x5b_app_sync_web_time(NULL, E_CMD_TO_C);
#endif/* APP_X5BA_MODULE */
#ifdef APP_V9_MODULE
		v9_cmd_sync_time_test();
#endif/* APP_V9_MODULE */
#endif/* PL_APP_MODULE */
		return web_return_text_plain(wp, OK);
	}
	else
		return ERROR;//web_return_text_plain(wp, ERROR);
}

static int web_localtime(Webs *wp, char *path, char *query)
{
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", os_time_fmt ("-", os_time(NULL)));
	websDone(wp);
	return 0;
}



static u_int8 web_reset_flag = 0;

static int web_system_app_action_job(void *a)
{
	os_sleep(1);
	if(web_reset_flag)
	{
		super_system("jffs2reset -y");
		super_system("reboot -f");
	}
	else
		super_system("reboot -f");
	return OK;
}

static int web_system_app_action(Webs *wp, void *p)
{
	int ret = 0;
	char *strval = NULL;
	strval = webs_get_var(wp, T("BTNID"), T(""));
	if (NULL == strval)
	{
		return ERROR;//;
	}
	if(strstr(strval, "reboot"))
	{
#ifdef APP_X5BA_MODULE
		ret = x5b_app_reboot_request(NULL, E_CMD_TO_A, FALSE);
		if(x5b_app_mgt && x5b_app_mode_X5CM())
			ret |= x5b_app_reboot_request(NULL, E_CMD_TO_C, FALSE);
#endif/* APP_X5BA_MODULE */
		if(ret == OK)
		{
			os_job_add(web_system_app_action_job, NULL);
			return web_return_text_plain(wp, OK);
		}
	}
	else if(strstr(strval, "reset"))
	{
		web_reset_flag = 1;
#ifdef APP_X5BA_MODULE
		ret = x5b_app_reboot_request(NULL, E_CMD_TO_A, TRUE);
		if(x5b_app_mgt && x5b_app_mode_X5CM())
			ret |= x5b_app_reboot_request(NULL, E_CMD_TO_C, TRUE);
#endif/* APP_X5BA_MODULE */
		if(ret == OK)
		{
			os_job_add(web_system_app_action_job, NULL);
			return web_return_text_plain(wp, OK);
		}
	}
	else if(strstr(strval, "websynctime"))
	{
#ifdef APP_X5BA_MODULE
		if(x5b_app_mgt && x5b_app_mode_X5CM())
			x5b_app_sync_web_time(x5b_app_mgt, E_CMD_TO_C);
#endif/* APP_X5BA_MODULE */
		return web_return_text_plain(wp, OK);
	}
	return ERROR;//web_return_text_plain(wp, ERROR);
}

int web_system_app(void)
{
	websFormDefine("system", web_system_handle);
	//websFormDefine("admin-password", web_admin_change_password);

	websFormDefine("localtime", web_localtime);
	web_button_add_hook("webtime", "syncwebtime", web_webtime_sync, NULL);

	web_button_add_hook("app", "websynctime", web_system_app_action, NULL);
	web_button_add_hook("app", "reboot", web_system_app_action, NULL);
	web_button_add_hook("app", "reset", web_system_app_action, NULL);
	return 0;
}
