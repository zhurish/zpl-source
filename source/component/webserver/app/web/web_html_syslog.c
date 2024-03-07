/**
 * @file      : web_html_syslog.c
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-02-05
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
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


static int web_syslog_form(Webs *wp, char *path, char *query)
{
	if(websGetMethodCode(wp) == WEBS_METHOD_POST)
	{
		#if ME_GOAHEAD_JSON
		cJSON *root = websGetJsonVar(wp);
		if(root)
		{
			int enable = cJSON_GetBoolValue(root, "enable");
			char *address = cJSON_GetStringValue(root, "address");
			int port = cJSON_GetIntValue(root, "port");
			char* level = cJSON_GetStringValue(root, "level");
			if(address == NULL)
				enable = 0;
			if(enable)
			{
				zlog_set_level(ZLOG_DEST_SYSLOG, zlog_priority_match(level));
				syslogc_host_config_set(address, port, 0);
				syslogc_mode_set(SYSLOG_UDP_MODE);
			}
			else
			{
				if (syslogc_is_enable())
				{
					syslogc_disable();
				}
				zlog_set_level(ZLOG_DEST_SYSLOG, ZLOG_DISABLED);
				syslogc_host_config_set(NULL, 0, 0);
			}
			return web_json_format_result(wp, 0, "OK");
		}
		return web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
		#endif
	}
	else if(websGetMethodCode(wp) == WEBS_METHOD_GET)
	{
		#if ME_GOAHEAD_JSON
		int port = 0, log_level = 0;
		char address[32];
		cJSON *obj = NULL;
		syslogc_host_config_get(address, &port, NULL);
		zlog_get_level(ZLOG_DEST_SYSLOG, &log_level);
		obj = cJSON_CreateObject();
		if(obj)
		{
			cJSON_AddNumberToObject(obj,"port", port);
			cJSON_AddStringToObject(obj,"level", zlog_priority_name(log_level));
			cJSON_AddStringToObject(obj,"address", address);
			if(log_level <= ZLOG_DISABLED)
				cJSON_AddFalseToObject(obj,"enable");
			else	
				cJSON_AddTrueToObject(obj,"enable");
		}
		websResponseJson(wp, HTTP_CODE_OK, obj);
		#endif
		return OK;
	}
	websResponse(wp, HTTP_CODE_BAD_METHOD, "not support method,just support post or get method");
	return OK;
}

static int web_syslog_button(Webs *wp, void *pVoid)
{
	#if ME_GOAHEAD_JSON	
	cJSON *root = websGetJsonVar(wp);
	if(root)
	{
		int enable = cJSON_GetBoolValue(root, "enable");
		char *address = cJSON_GetStringValue(root, "address");
		int port = cJSON_GetIntValue(root, "port");
		char* level = cJSON_GetStringValue(root, "level");
		if(address == NULL)
			enable = 0;
		if(enable)
		{
			zlog_set_level(ZLOG_DEST_SYSLOG, zlog_priority_match(level));
			syslogc_host_config_set(address, port, 0);
			syslogc_mode_set(SYSLOG_UDP_MODE);
		}
		else
		{
			if (syslogc_is_enable())
			{
				syslogc_disable();
			}
			zlog_set_level(ZLOG_DEST_SYSLOG, ZLOG_DISABLED);
			syslogc_host_config_set(NULL, 0, 0);
		}
		return web_json_format_result(wp, 0, "OK");
	}
	return web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
	#endif
}

int web_html_syslog_init(void)
{
	websFormDefine("syslog", web_syslog_form);
	web_button_add_hook("syslog", "button-submit", web_syslog_button, NULL);
	return 0;
}

