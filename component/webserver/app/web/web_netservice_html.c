/*
 * web_netservice_html.c
 *
 *  Created on: 2020年4月24日
 *      Author: DELL
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
#ifdef PL_SERVICE_SYSLOG
#include "syslogcLib.h"
#endif
#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

#ifdef THEME_V9UI


static int v9_app_sntp_config_save(int sntp, char *ip, char *ip1)
{
#ifdef PL_OPENWRT_UCI
	int ret = 0;
	if(sntp == 0)
	{
		ret |= os_uci_set_integer("system.ntp.enabled", sntp);
		ret |= os_uci_set_integer("system.ntp.enable_server", sntp);
		ret |= os_uci_del("system", "ntp", "server", NULL);
		os_uci_save_config("system");
		super_system("/etc/init.d/sysntpd stop");
		return OK;
	}
	ret |= os_uci_set_integer("system.ntp.enabled", sntp);
	ret |= os_uci_set_integer("system.ntp.enable_server", sntp);
	ret |= os_uci_del("system", "ntp", "server", NULL);
	if(ip)
		ret |= os_uci_list_add("system.ntp.server", ip);
	if(ip1)
		ret |= os_uci_list_add("system.ntp.server", ip1);
	os_uci_save_config("system");
	super_system("/etc/init.d/sysntpd restart");
#endif
	return OK;
}

//ret |= os_uci_get_string("system.@system[0].zonename", tmp);
static int web_netservice_goform(Webs *wp, char *path, char *query)
{
	int	plat = 0;
	int sntpen = 0;
	char *sntp_addr[6] = {NULL, NULL,NULL,NULL,NULL,NULL};
	int sntp_cnt = 0;
	char *strval = NULL;
	char *sntp_address = NULL;
	char *sntp_address1 = NULL;
	char *syslog_address = NULL;
	char *plat_address = NULL;

	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		_WEB_DBG_TRAP("================%s=======================: can not get ACTION\r\n", __func__);
		return web_return_text_plain(wp, ERROR);
	}

	if (strstr(strval, "GET"))
	{
		char address[32];
		char plataddress[32];
		memset(address, 0, sizeof(address));
		memset(plataddress, 0, sizeof(plataddress));
#ifdef PL_OPENWRT_UCI
		os_uci_get_integer("system.ntp.enabled", &sntpen);
		os_uci_get_list("system.ntp.server", sntp_addr, &sntp_cnt);
		os_uci_get_integer("product.global.plat", &plat);
		os_uci_get_address("product.global.platip", plataddress);
#endif

#ifdef PL_SERVICE_SYSLOG
		syslogc_host_config_get(address, NULL, NULL);
#endif
		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);

		websWrite(wp,
				"{\"response\":\"%s\", \"plat\":%d, \"platip\":\"%s\", \"sntp\":\"%s\" , \"sntpip\":\"%s\", \"sntpip1\":\"%s\", \"syslog\":\"%s\" , \"syslogip\":\"%s\"}",
					"OK", plat, plataddress,
					sntpen?"true":"false", sntp_addr[0]?sntp_addr[0]:" ",
					sntp_addr[1]?sntp_addr[1]:" ",
#ifdef PL_SERVICE_SYSLOG
							syslogc_is_enable() ? "true" : "false",
#else
									"false",
#endif
									address);

		if(sntp_cnt)
		{
			for(plat = 0; plat < sntp_cnt; plat++)
			{
				if(sntp_addr[plat])
					free(sntp_addr[plat]);
			}
		}

		websDone(wp);
		return OK;
	}
	strval = webs_get_var(wp, T("plat"), T(""));
	if (NULL == strval)
	{
		_WEB_DBG_TRAP("================%s=======================: can not get plat=%s\r\n", __func__, strval);
		return web_return_text_plain(wp, ERROR);
	}
	plat = atoi(strval);
/*
	if(strncasecmp(strval, "IOT", 3))
		plat = 1;//atoi(strval);
*/

	plat_address = webs_get_var(wp, T("platip"), T(""));

	strval = webs_get_var(wp, T("sntp"), T(""));
	if (NULL == strval)
	{
		_WEB_DBG_TRAP("================%s=======================: can not get sntp=%s\r\n", __func__, strval);
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "true"))
	{
		sntpen = 1;
		sntp_address = webs_get_var(wp, T("sntpip"), T(""));
		if (NULL == sntp_address)
		{
			_WEB_DBG_TRAP("================%s=======================: can not get sntpip=%s\r\n", __func__, sntp_address);
			return web_return_text_plain(wp, ERROR);
		}
		sntp_address1 = webs_get_var(wp, T("sntpip1"), T(""));
	}

	strval = webs_get_var(wp, T("syslog"), T(""));
	if (NULL == strval)
	{
		_WEB_DBG_TRAP("================%s=======================: can not get syslog=%s\r\n", __func__, strval);
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "true"))
	{
		syslog_address = webs_get_var(wp, T("syslogip"), T(""));
		if (NULL == syslog_address)
		{
			_WEB_DBG_TRAP("================%s=======================: can not get syslogip=%s\r\n", __func__, syslog_address);
			return web_return_text_plain(wp, ERROR);
		}
#ifdef PL_SERVICE_SYSLOG
		if (!syslogc_is_enable())
			syslogc_enable(host.name);
		zlog_set_level(ZLOG_DEST_SYSLOG, LOG_WARNING);
		syslogc_mode_set(SYSLOG_UDP_MODE);
		syslogc_host_config_set(syslog_address, 0, 0);
#endif
	}
#ifdef PL_SERVICE_SYSLOG
	else
	{
		if (syslogc_is_enable())
		{
			syslogc_disable();
		}
		zlog_set_level(ZLOG_DEST_SYSLOG, ZLOG_DISABLED);
	}
#endif
	v9_app_sntp_config_save(sntpen, sntp_address, sntp_address1);
#ifdef PL_OPENWRT_UCI
	os_uci_set_integer("product.global.plat", plat);
	if(plat_address)
		os_uci_set_string("product.global.platip", plat_address);
	os_uci_save_config("product");
#endif
	return web_return_text_plain(wp, OK);
}
#endif /* THEME_V9UI */

int web_netservice_app(void)
{
#ifdef THEME_V9UI
	//websFormDefine("netsrvtbl", web_netservice_goform_get);
	websFormDefine("netservice", web_netservice_goform);
#endif /* THEME_V9UI */
	return 0;
}


