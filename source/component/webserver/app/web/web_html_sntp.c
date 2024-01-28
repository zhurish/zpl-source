/*
 * web_sntp_html.c
 *
 *  Created on: 2019年8月8日
 *      Author: DELL
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

#ifdef ZPL_SERVICE_SNTPC
#include "sntpcLib.h"

#include "web_api.h"

#include "web_app.h"


static int jst_sntp_timezone_list(int eid, webs_t wp, int argc, char **argv)
{
	int i = 0;
	websWrite(wp, "<option value=\"UTC\">UTC</option>");
	websWrite(wp, "<option value=\"GMT0\">GMT0</option>");
	for (i = 1; i <= 12; i++)
	{
		websWrite(wp, "<option value=\"GMT+%d\">GMT+%d</option>", i, i);
	}
	for (i = 1; i <= 12; i++)
	{
		if (i == 8)
			websWrite(wp,
					"<option selected=\"selected\" value=\"GMT-%d\">GMT-%d</option>",
					i, i);
		else
			websWrite(wp, "<option value=\"GMT-%d\">GMT-%d</option>", i, i);
	}
	return 0;
}

static int web_sntp_set(Webs *wp, char *path, char *query)
{
	zpl_bool	enable = zpl_false;
	char *strval = NULL;
	char *sntp_address = NULL;
	char *sntp_timezone = NULL;
	char *sntp_syncinterval = NULL;
	int	sntp_timezone_val = 0;
	strval = webs_get_var(wp, T("button-action"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}
	sntpc_client_get_api(NULL, API_SNTPC_GET_ENABLE, &enable);
	if (strstr(strval, "GET"))
	{
		zpl_uint32	address = 0, port = 0, interval = 0;
		zpl_int32 timezone = 0;
		char timezonestr[32];

		sntpc_client_get_api(NULL, API_SNTPC_GET_ADDRESS, &address);
		sntpc_client_get_api(NULL, API_SNTPC_GET_PORT, &port);
		sntpc_client_get_api(NULL, API_SNTPC_GET_INTERVAL, &interval);
		sntpc_client_get_api(NULL, API_SNTPC_GET_TIMEZONE, &timezone);

		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);
		//websWrite(wp, "%s", "[");

		memset(timezonestr, 0, sizeof(timezonestr));
		if(timezone == 'U')
			sprintf(timezonestr, "UTC");
		else if(timezone == 0)
			sprintf(timezonestr, "GTM0");
		else if(timezone > 0)
			sprintf(timezonestr, "GTM-%d", timezone);
		else if(timezone < 0)
			sprintf(timezonestr, "GTM+%d", abs(timezone));

		websWrite(wp,
				"{\"response\":\"%s\", \"sntp_enable\":%s, \"sntp_address\":\"%s\", \"sntp_timezone\":\"%s\" , \"sntp_syncinterval\":\"%s\"}",
				"OK", enable ? "true":"false", inet_address(address), timezonestr, itoa(interval, 10));

		websDone(wp);
		return OK;
	}
	strval = webs_get_var(wp, T("sntp_enable"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}
	if(strstr(strval, "false"))
	{
		_WEB_DBG_TRAP("%s: sntp_enable=%s\r\n", __func__, strval);
		sntpc_client_set_api(NULL, API_SNTPC_SET_ENABLE, NULL);
		return web_return_text_plain(wp, OK, NULL);
	}


	sntp_address = webs_get_var(wp, T("sntp_address"), T(""));
	if (NULL == sntp_address)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}
	sntpc_client_set_api(NULL, API_SNTPC_SET_ADDRESS, sntp_address);

	sntp_timezone = webs_get_var(wp, T("sntp_timezone"), T(""));
	if (NULL == sntp_timezone)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}
	if(strstr(sntp_timezone,"U"))
		sntp_timezone_val = 'U';
	else if(strstr(sntp_timezone,"0"))
		sntp_timezone_val = 0;
	else //if(atoi(sntp_timezone)>0)
		sntp_timezone_val = atoi(sntp_timezone);

	sntpc_client_set_api(NULL, API_SNTPC_SET_TIMEZONE, &sntp_timezone_val);


	sntp_syncinterval = webs_get_var(wp, T("sntp_syncinterval"), T(""));
	if (NULL == sntp_syncinterval)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}
	sntp_timezone_val = atoi(sntp_syncinterval);
	sntpc_client_set_api(NULL, API_SNTPC_SET_INTERVAL, &sntp_timezone_val);

	enable = zpl_true;
	sntpc_client_set_api(NULL, API_SNTPC_SET_ENABLE, &enable);

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);

	websWrite(wp,
			"{\"response\":\"%s\", \"sntp_enable\":%s, \"sntp_address\":\"%s\", \"sntp_timezone\":\"%s\" , \"sntp_syncinterval\":\"%s\"}",
			"OK", enable ? "true":"false", sntp_address, sntp_timezone, sntp_syncinterval);

	websDone(wp);
	return OK;
}



int web_html_sntp_init(void)
{

	websDefineJst("jst_timezone_list", jst_sntp_timezone_list);
	websFormDefine("setsntp", web_sntp_set);

	return 0;
}

#endif /* ZPL_SERVICE_SNTPC */
