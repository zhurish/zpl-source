/**
 * @file      : web_html_system.c
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-02-05
 *
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 *
 */
#include "goahead.h"
#include "host.h"
#include "os_time.h"
#include "web_api.h"
#include "web_app.h"

#ifdef ZPL_APP_MODULE
#include "application.h"
#endif /* ZPL_APP_MODULE */

static int web_system_handle(Webs *wp, char *path, char *query)
{
	int ret = 0;
	web_assert(wp);
	if (websGetMethodCode(wp) == WEBS_METHOD_POST)
	{
#if ME_GOAHEAD_JSON
		cJSON *root = websGetJsonVar(wp);
		if (root)
		{
			char *sysmac = cJSON_GetStringValue(root, "sysmac");
			char *devname = cJSON_GetStringValue(root, "devname");
			char *devtime = cJSON_GetStringValue(root, "devtime");
			struct ipstack_ethaddr ether;
			if (devname)
			{
				ret |= host_config_set_api(API_SET_HOSTNAME_CMD, devname);
			}
			if (sysmac)
			{
				ethaddr_aton_r(sysmac, &ether);
				ret |= host_config_set_api(API_SET_SYSMAC_CMD, ether.octet);
			}
			if (devtime)
			{
				int value = 0;
				struct timespec sntpTime;
				sntpTime.tv_sec = os_timestamp_spilt(0, devtime);
				value = 5;
				while (value)
				{
					ipstack_errno = 0;
					if (clock_settime(CLOCK_REALTIME, &sntpTime) != 0) // SET SYSTEM LOCAL TIME
					{
						value--;
					}
					else
					{
						break;
					}
				}
				if (value == 0)
				{
					return web_json_format_result(wp, 0, "OK");
				}
				else
					return web_json_format_result(wp, 0, "Can not set realtime");
			}
			return web_json_format_result(wp, 0, "OK");
		}
		return web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
#endif
	}
	else if (websGetMethodCode(wp) == WEBS_METHOD_GET)
	{
#if ME_GOAHEAD_JSON
		cJSON *obj = NULL;
		zpl_uint8 sysmac[8];
		zpl_int8 serial[64];
		memset(sysmac, 0, sizeof(sysmac));
		memset(serial, 0, sizeof(serial));
		host_config_get_api(API_GET_SYSMAC_CMD, sysmac);
		host_config_get_api(API_GET_SERIAL_CMD, serial);
		obj = cJSON_CreateObject();
		if (obj)
		{
			cJSON_AddStringToObject(obj, "serial", serial);
			cJSON_AddStringToObject(obj, "hostname", host_name_get());
			cJSON_AddStringToObject(obj, "sysmac", inet_ethernet(sysmac));
			cJSON_AddStringToObject(obj, "devname", host_name_get());
			cJSON_AddStringToObject(obj, "devtime", os_time_fmt("-", os_time(NULL)));
		}
		websResponseJson(wp, HTTP_CODE_OK, obj);
#endif
		return OK;
	}
	websResponse(wp, HTTP_CODE_BAD_METHOD, "not support method,just support post or get method");
	return OK;
}

static int web_webtime_sync(Webs *wp, void *p)
{
#if ME_GOAHEAD_JSON
	cJSON *root = websGetJsonVar(wp);
	if (root)
	{
		int ret = 0;
		char *sysmac = cJSON_GetStringValue(root, "sysmac");
		char *devname = cJSON_GetStringValue(root, "devname");
		char *devtime = cJSON_GetStringValue(root, "devtime");
		struct ipstack_ethaddr ether;
		if (devname)
		{
			ret |= host_config_set_api(API_SET_HOSTNAME_CMD, devname);
		}
		if (sysmac)
		{
			ethaddr_aton_r(sysmac, &ether);
			ret |= host_config_set_api(API_SET_SYSMAC_CMD, ether.octet);
		}
		if (devtime)
		{
			int value = 0;
			struct timespec sntpTime;
			sntpTime.tv_sec = os_timestamp_spilt(0, devtime);
			value = 5;
			while (value)
			{
				ipstack_errno = 0;
				if (clock_settime(CLOCK_REALTIME, &sntpTime) != 0) // SET SYSTEM LOCAL TIME
				{
					value--;
				}
				else
				{
					break;
				}
			}
			if (value == 0)
			{
				return web_json_format_result(wp, 0, "OK");
			}
			else
				return web_json_format_result(wp, 0, "Can not set realtime");
		}
		return web_json_format_result(wp, 0, "OK");
	}
	return web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
#else
	websResponse(wp, HTTP_CODE_BAD_METHOD, "not support method,just support post or get method");
	return OK;
#endif
}

int web_html_system_init(void)
{
	websFormDefine("device", web_system_handle);
	web_button_add_hook("device", "button-submit", web_webtime_sync, NULL);
	return 0;
}
