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
#endif

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
		printf("----------------------%s--------------------------\r\n",__func__);
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
	sntpTime.tv_sec = atoi(strval);

	printf( "-----%s------:%s\r\n", __func__, os_time_fmt ("date", sntpTime.tv_sec));

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
#endif
#endif
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
	websWrite(wp, "%s", os_time_fmt ("date", os_time(NULL)));
	websDone(wp);
	return 0;
}

int web_system_app(void)
{
	websFormDefine("system", web_system_handle);
	//websFormDefine("admin-password", web_admin_change_password);

	websFormDefine("localtime", web_localtime);
	web_button_add_hook("webtime", "syncwebtime", web_webtime_sync, NULL);

	return 0;
}
