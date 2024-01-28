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

#include "web_app.h"

#ifdef ZPL_APP_MODULE
#include "application.h"
#endif/* ZPL_APP_MODULE */

static int web_system_handle(Webs *wp, char *path, char *query)
{
	int ret = 0;
	char *strval = NULL;
	struct ethaddr ether;
	web_assert(wp);
	strval = webs_get_var(wp, T("button-action"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}
	if(strstr(strval, "GET"))
	{
		zpl_uint8 sysmac[8];
		zpl_int8	serial[64];
		memset(sysmac, 0, sizeof(sysmac));
		memset(serial, 0, sizeof(serial));
		host_config_get_api(API_GET_SYSMAC_CMD, sysmac);
		host_config_get_api(API_GET_SERIAL_CMD, serial);

		websSetStatus(wp, 200);

		websWriteCache(wp,
				"{\"response\":\"%s\", \"serial\":\"%s\", \"hostname\":\"%s\", \"mac\":\"%s\"}",
				"OK", serial, host_name_get(), inet_ethernet(sysmac));

		websWriteHeaders (wp, websWriteCacheLen(wp), 0);
		websWriteHeader (wp, "Content-Type", "application/json");
		websWriteEndHeaders (wp);	
		websWriteCacheFinsh(wp);

		websDone(wp);
		_WEB_DBG_TRAP("----------------------%s--------------------------\r\n",__func__);
		return OK;
	}
	strval = webs_get_var(wp, T("hostname"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}
	ret |= host_config_set_api(API_SET_HOSTNAME_CMD, strval);

	strval = webs_get_var(wp, T("mac"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}
	ethaddr_aton_r (strval, &ether);
	ret |= host_config_set_api(API_SET_SYSMAC_CMD, ether.octet);

	if(ret == OK)
	{
		zpl_uint8 sysmac[8];
		zpl_int8	serial[64];
		memset(sysmac, 0, sizeof(sysmac));
		memset(serial, 0, sizeof(serial));
		host_config_get_api(API_GET_SYSMAC_CMD, sysmac);
		host_config_get_api(API_GET_SERIAL_CMD, serial);
		websSetStatus(wp, 200);

		websWriteCache(wp,
				"{\"response\":\"%s\", \"serial\":\"%s\", \"hostname\":\"%s\", \"mac\":\"%s\"}",
				"OK", serial, host_name_get(), inet_ethernet(sysmac));
		websWriteHeaders (wp, websWriteCacheLen(wp), 0);
		websWriteHeader (wp, "Content-Type", "application/json");
		websWriteEndHeaders (wp);	
		websWriteCacheFinsh(wp);
		websDone(wp);
		return OK;
	}

	return web_return_text_plain(wp, ERROR, NULL);
}

static int web_webtime_sync(Webs *wp, void *p)
{
	char *strval = NULL;
	struct timespec sntpTime;	/* storage for retrieved time value */
	int value = 0;
	web_assert(wp);
	strval = webs_get_var(wp, T("timesp"), T(""));

	if (NULL == strval)
	{
		return ERROR;//web_return_text_plain(wp, ERROR, NULL);
	}
	sntpTime.tv_sec = os_timestamp_spilt(0,  strval);

	//sntpTime.tv_sec = atoi(strval);

	_WEB_DBG_TRAP( "-----%s------:%s\r\n", __func__, os_time_fmt ("date", sntpTime.tv_sec));

	value = 5;
	while(value)
	{
		ipstack_errno = 0;
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
		return web_return_text_plain(wp, OK, NULL);
	}
	else
		return ERROR;//web_return_text_plain(wp, ERROR, NULL);
}

static int web_localtime(Webs *wp, char *path, char *query)
{
	web_assert(wp);
	websSetStatus(wp, 200);

	websWriteCache(wp, "%s", os_time_fmt ("-", os_time(NULL)));

	websWriteHeaders (wp, websWriteCacheLen(wp), 0);
	websWriteHeader (wp, "Content-Type", "text/plain");
	websWriteEndHeaders (wp);	
	websWriteCacheFinsh(wp);
	websDone(wp);
	return 0;
}


int web_html_system_init(void)
{
	websFormDefine("system", web_system_handle);
	websFormDefine("localtime", web_localtime);
	web_button_add_hook("webtime", "syncwebtime", web_webtime_sync, NULL);
	return 0;
}
