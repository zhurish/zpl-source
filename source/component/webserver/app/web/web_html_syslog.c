/*
 * web_syslog_html.c
 *
 *  Created on: 2019年8月9日
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

#include "syslogcLib.h"

#include "web_api.h"
#include "web_app.h"


static int web_syslog_set(Webs *wp, char *path, char *query)
{
	char *strval = NULL;
	char *syslog_address = NULL;
	char *syslog_level = NULL;
	char *syslog_port = NULL;
	char *syslog_proto = NULL;
	strval = webs_get_var(wp, T("button-action"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}
	if (strstr(strval, "GET"))
	{
		int port = 0, mode = 0, log_level = 0;
		char address[32];
		syslogc_host_config_get(address, &port, NULL);
		syslogc_mode_get(&mode);

		zlog_get_level(ZLOG_DEST_SYSLOG, &log_level);
		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);

		websWrite(wp,
				"{\"response\":\"%s\", \"syslog_enable\":%s, \"syslog_address\":\"%s\", \
        		\"syslog_level\":\"%s\" , \"syslog_port\":\"%s\", \"syslog_proto\":\"%s\"}",
				"OK", syslogc_is_enable() ? "true" : "false", address,
				(log_level >= ZLOG_LEVEL_ERR) ?
						zlog_priority_name(log_level) : "warnings",
				itoa(port, 10), (mode == SYSLOG_UDP_MODE) ? "UDP" : "TCP");
		websDone(wp);
		return OK;
	}
	strval = webs_get_var(wp, T("syslog_enable"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}
	if(strstr(strval, "false"))
	{
		if (syslogc_is_enable())
		{
			syslogc_disable();
		}
		zlog_set_level(ZLOG_DEST_SYSLOG, ZLOG_DISABLED);
		return web_return_text_plain(wp, OK, NULL);
	}
	//if (!syslogc_is_enable())
	//	syslogc_enable(host.name);

	syslog_address = webs_get_var(wp, T("syslog_address"), T(""));
	if (NULL == syslog_address)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}
	_WEB_DBG_TRAP("%s: syslog_address=%s\r\n", __func__, syslog_address);

	syslog_level = webs_get_var(wp, T("syslog_level"), T(""));
	if (NULL == syslog_level)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}
	zlog_set_level(ZLOG_DEST_SYSLOG, zlog_priority_match(syslog_level));

	syslog_port = webs_get_var(wp, T("syslog_port"), T(""));
	if (NULL == syslog_port)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}

	syslogc_host_config_set(syslog_address, atoi(syslog_port), 0);

	syslog_proto = webs_get_var(wp, T("syslog_proto"), T(""));
	if (NULL == syslog_proto)
	{
		return web_return_text_plain(wp, ERROR, NULL);
	}
	if (strstr(syslog_proto, "TCP"))
		syslogc_mode_set(SYSLOG_TCP_MODE);
	else
		syslogc_mode_set(SYSLOG_UDP_MODE);

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);

	websWrite(wp,
			"{\"response\":\"%s\", \"syslog_enable\":%s, \"syslog_address\":\"%s\", \
    		\"syslog_level\":\"%s\" , \"syslog_port\":\"%s\", \"syslog_proto\":\"%s\"}",
			"OK", syslogc_is_enable() ? "true" : "false", syslog_address,
			syslog_level, syslog_port, syslog_proto);

	websDone(wp);
	return OK;
}



static int jst_syslog(int eid, webs_t wp, int argc, char **argv)
{
	int rows = 1;
	FILE *f = NULL;
	char buf[4096];
	if(argv[0])
	{
		f = fopen(argv[0], "r");
	}
	else
	{
		f = fopen(WEB_SYSTEM_LOG, "r");
	}
	if (f)
	{
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
			rows++;
		}
	}
	//class="form-control"
	//websWrite(wp, "<textarea class=\"form-control\" style=\"font-size: 12px;\" readonly=\"readonly\" wrap=\"off\" rows=\"%d\" id=\"syslog\">", rows);
	websWrite(wp, "<textarea class=\"form-control\" disabled rows=\"%d\" id=\"syslog\">", rows > 35 ? 35:rows);
/*
	if (f)
	{
		fseek(f, 0, SEEK_SET);
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
		    websWrite(wp, buf);
		    memset(buf, 0, sizeof(buf));
		}
		fclose(f);
	}*/
	websWrite(wp, "ddddddddddddddddddddddddddddddddddddd======================================");
	websWrite(wp, "</textarea>");
	return 0;
}


int web_html_syslog_init(void)
{
	websDefineJst("jst_syslog", jst_syslog);
	websFormDefine("setsyslog", web_syslog_set);
	return 0;
}

