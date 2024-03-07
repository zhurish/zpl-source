/**
 * @file      : web_html_sntp.c
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

#ifdef ZPL_SERVICE_SNTPC
#include "sntpcLib.h"
#endif
#ifdef ZPL_SERVICE_SNTPS
#include "sntpsLib.h"
#endif
#include "web_api.h"

#include "web_app.h"

#ifdef ZPL_SERVICE_SNTPC
static int web_sntpc_form(Webs *wp, char *path, char *query)
{
	if(websGetMethodCode(wp) == WEBS_METHOD_GET)
	{
		#if ME_GOAHEAD_JSON	
		zpl_bool	enable = zpl_false;
		zpl_uint32	address = 0, port = 0, interval = 0;
		zpl_int32 timezone = 0;
		char timezonestr[32];
		cJSON *obj = cJSON_CreateObject();
		sntpc_client_get_api(NULL, API_SNTPC_GET_ENABLE, &enable);
		sntpc_client_get_api(NULL, API_SNTPC_GET_ADDRESS, &address);
		sntpc_client_get_api(NULL, API_SNTPC_GET_PORT, &port);
		sntpc_client_get_api(NULL, API_SNTPC_GET_INTERVAL, &interval);
		sntpc_client_get_api(NULL, API_SNTPC_GET_TIMEZONE, &timezone);
		
		if(obj)
		{
			memset(timezonestr, 0, sizeof(timezonestr));
			if(timezone == 'U')
				sprintf(timezonestr, "UTC");
			else if(timezone == 0)
				sprintf(timezonestr, "GTM0");
			else if(timezone > 0)
				sprintf(timezonestr, "GTM-%d", timezone);
			else if(timezone < 0)
				sprintf(timezonestr, "GTM+%d", abs(timezone));

			cJSON_AddBoolToObject(obj,"enable", enable);
			cJSON_AddStringToObject(obj,"address", inet_address(address));
			cJSON_AddNumberToObject(obj,"port", port);	
			cJSON_AddNumberToObject(obj,"interval", interval);
			cJSON_AddStringToObject(obj,"timezone", timezonestr);	
		}
		websResponseJson(wp, HTTP_CODE_OK, obj);
		return OK;
		#endif
	}
	else if(websGetMethodCode(wp) == WEBS_METHOD_POST)
	{
		int ret = ERROR;
		#if ME_GOAHEAD_JSON	
		cJSON *root = websGetJsonVar(wp);
		if(root)
		{
			zpl_bool enable = cJSON_GetBoolValue(root, "enable");
			int port = cJSON_GetIntValue(root, "port");
			int interval = cJSON_GetIntValue(root, "interval");
			char *address = cJSON_GetStringValue(root, "address");
			char *timezonestr = cJSON_GetStringValue(root, "timezone");
			int timezone = -1;
			if(timezonestr)
			{
				if(strstr(timezonestr,"U"))
					timezone = 'U';
				else if(strstr(timezonestr,"0"))
					timezone = 0;
				else 
					timezone = atoi(timezonestr);
			}
			if(!enable)
			{
				ret = sntpc_client_set_api(NULL, API_SNTPC_SET_ENABLE, NULL);
				if(ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			if(address && port)
			{
				if(timezone >= 0)
					sntpc_client_set_api(NULL, API_SNTPC_SET_TIMEZONE, &timezone);
				if(interval)
					sntpc_client_set_api(NULL, API_SNTPC_SET_INTERVAL, &interval);

				if(port)
					sntpc_client_set_api(NULL, API_SNTPC_SET_PORT, &port);
				if(address)
					sntpc_client_set_api(NULL, API_SNTPC_SET_ADDRESS, address);

				ret = sntpc_client_set_api(NULL, API_SNTPC_SET_ENABLE, &enable);
				if(ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			return web_json_format_result(wp, HTTP_CODE_INTERNAL_SERVER_ERROR, "can not get address and port value");
		}
		web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
		#endif
	}
	websResponse(wp, HTTP_CODE_BAD_METHOD, "not support method,just support post or get method");
	return 0;
}
#endif
#ifdef ZPL_SERVICE_SNTPS
static int web_sntps_form(Webs *wp, char *path, char *query)
{
	if(websGetMethodCode(wp) == WEBS_METHOD_GET)
	{
		#if ME_GOAHEAD_JSON	
		zpl_bool	enable = zpl_false;
		zpl_uint32	version = 0, port = 0, interval = 0;
		zpl_int32 mode = 0;
		char modestr[32];
		cJSON *obj = cJSON_CreateObject();
		sntp_server_get_api(NULL, API_SNTPS_GET_ENABLE, &enable);
		sntp_server_get_api(NULL, API_SNTPS_GET_VERSION, &version);
		sntp_server_get_api(NULL, API_SNTPS_GET_LISTEN, &port);
		sntp_server_get_api(NULL, API_SNTPS_GET_INTERVAL, &interval);
		sntp_server_get_api(NULL, API_SNTPS_GET_MODE, &mode);
		if(obj)
		{
			memset(modestr, 0, sizeof(modestr));
			if(mode == SNTPS_BROADCAST)
				sprintf(modestr, "broadcast");
			else if(mode == SNTPS_UNICAST)
				sprintf(modestr, "unicast");
			else if(mode == SNTPS_MULTICAST)
				sprintf(modestr, "multicast");

			cJSON_AddBoolToObject(obj,"enable", enable);
			//cJSON_AddStringToObject(obj,"address", inet_address(address));
			cJSON_AddNumberToObject(obj,"port", port);	
			cJSON_AddNumberToObject(obj,"interval", interval);
			cJSON_AddStringToObject(obj,"mode", modestr);	
			cJSON_AddNumberToObject(obj,"version", version);
		}
		websResponseJson(wp, HTTP_CODE_OK, obj);
		return OK;
		#endif
	}
	else if(websGetMethodCode(wp) == WEBS_METHOD_POST)
	{
//extern int sntp_server_set_api(struct vty *, zpl_uint32 cmd, const char *value);
		int ret = ERROR;
		#if ME_GOAHEAD_JSON	
		cJSON *root = websGetJsonVar(wp);
		if(root)
		{
			zpl_bool enable = cJSON_GetBoolValue(root, "enable");
			int port = cJSON_GetIntValue(root, "port");
			int interval = cJSON_GetIntValue(root, "interval");
			//char *address = cJSON_GetStringValue(root, "address");
			int version = cJSON_GetIntValue(root, "version");
			char * modestr = cJSON_GetStringValue(root, "mode");

			int mode = -1;
			if(modestr && os_memcmp(modestr, "broadcast", 1) == 0)
			{
				//sntp_server->address.s_addr = ipstack_inet_addr("192.168.198.1");
				mode = SNTPS_BROADCAST;
			}
			else if(modestr && os_memcmp(modestr, "unicast", 1) == 0)
			{
				mode = SNTPS_UNICAST;
			}
			else if(modestr && os_memcmp(modestr, "multicast", 1) == 0)
			{
				//sntp_server->address.s_addr = ipstack_inet_addr(SNTP_MUTILCAST_ADDRESS);
				mode = SNTPS_MULTICAST;
			}
			if(!enable)
			{
				ret = sntp_server_set_api(NULL, API_SNTPS_SET_ENABLE, NULL);
				if(ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			if(modestr && port)
			{
				if(mode >= 0)
					sntp_server_set_api(NULL, API_SNTPS_SET_MODE, &mode);
				if(interval)
					sntp_server_set_api(NULL, API_SNTPS_SET_INTERVAL, &interval);
				if(version >= 0)
					sntp_server_set_api(NULL, API_SNTPS_SET_VERSION, &version);
				if(port >= 0)
					sntp_server_set_api(NULL, API_SNTPS_SET_LISTEN, &port);

				ret = sntp_server_set_api(NULL, API_SNTPS_SET_ENABLE, &enable);
				if(ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			return web_json_format_result(wp, HTTP_CODE_INTERNAL_SERVER_ERROR, "can not get mode and port value");
		}
		web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
		#endif
	}
	websResponse(wp, HTTP_CODE_BAD_METHOD, "not support method,just support post or get method");
	return 0;
}
#endif


static int web_sntp_button(Webs *wp, void *pVoid)
{
	#if ME_GOAHEAD_JSON	
	int ret = ERROR;
	cJSON *root = websGetJsonVar(wp);
	if(root)
	{
		zpl_bool enable = cJSON_GetBoolValue(root, "enable");
		int port = cJSON_GetIntValue(root, "port");
		int interval = cJSON_GetIntValue(root, "interval");
		char *address = cJSON_GetStringValue(root, "address");
		int version = cJSON_GetIntValue(root, "version");
		char * modestr = cJSON_GetStringValue(root, "mode");
		char *timezonestr = cJSON_GetStringValue(root, "timezone");
		char *module  = cJSON_GetStringValue(root, "module");
		int timezone = -1;
		int mode = -1;
		if(timezonestr)
		{
			if(strstr(timezonestr,"U"))
				timezone = 'U';
			else if(strstr(timezonestr,"0"))
				timezone = 0;
			else 
				timezone = atoi(timezonestr);
		}
		if(modestr)
		{
			if(os_memcmp(modestr, "broadcast", 1) == 0)
			{
				//sntp_server->address.s_addr = ipstack_inet_addr("192.168.198.1");
				mode = SNTPS_BROADCAST;
			}
			else if(os_memcmp(modestr, "unicast", 1) == 0)
			{
				mode = SNTPS_UNICAST;
			}
			else if(os_memcmp(modestr, "multicast", 1) == 0)
			{
				//sntp_server->address.s_addr = ipstack_inet_addr(SNTP_MUTILCAST_ADDRESS);
				mode = SNTPS_MULTICAST;
			}
		}
		if(module && os_memcmp(module, "sntps", 5) == 0)
		{
			#ifdef ZPL_SERVICE_SNTPS
			if(!enable)
			{
				ret = sntp_server_set_api(NULL, API_SNTPS_SET_ENABLE, NULL);
				if(ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			if(modestr && port)
			{
				if(mode >= 0)
					sntp_server_set_api(NULL, API_SNTPS_SET_MODE, &mode);
				if(interval)
					sntp_server_set_api(NULL, API_SNTPS_SET_INTERVAL, &interval);
				if(version >= 0)
					sntp_server_set_api(NULL, API_SNTPS_SET_VERSION, &version);
				if(port >= 0)
					sntp_server_set_api(NULL, API_SNTPS_SET_LISTEN, &port);

				ret = sntp_server_set_api(NULL, API_SNTPS_SET_ENABLE, &enable);
				if(ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			else
				return web_json_format_result(wp, HTTP_CODE_INTERNAL_SERVER_ERROR, "can not get mode and port value");
			#else
			return web_json_format_result(wp, ERROR, "sntp client is not support");
			#endif					
		}
		else if(module && os_memcmp(module, "sntpc", 5) == 0)
		{
			#ifdef ZPL_SERVICE_SNTPC
			if(!enable)
			{
				ret = sntpc_client_set_api(NULL, API_SNTPC_SET_ENABLE, NULL);
				if(ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			if(address && port)
			{
				if(timezone >= 0)
					sntpc_client_set_api(NULL, API_SNTPC_SET_TIMEZONE, &timezone);
				if(interval)
					sntpc_client_set_api(NULL, API_SNTPC_SET_INTERVAL, &interval);

				if(port)
					sntpc_client_set_api(NULL, API_SNTPC_SET_PORT, &port);
				if(address)
					sntpc_client_set_api(NULL, API_SNTPC_SET_ADDRESS, address);

				ret = sntpc_client_set_api(NULL, API_SNTPC_SET_ENABLE, &enable);
				if(ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			else
				return web_json_format_result(wp, HTTP_CODE_INTERNAL_SERVER_ERROR, "can not get address and port value");
			#else
			return web_json_format_result(wp, ERROR, "sntp client is not support");
			#endif	
		}
		else
			return web_json_format_result(wp, ERROR, "can not get module value");	
	}
	return web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
	#endif
}

int web_html_sntp_init(void)
{
	#ifdef ZPL_SERVICE_SNTPC
	websFormDefine("sntpc", web_sntpc_form);
	#endif
	#ifdef ZPL_SERVICE_SNTPS
	websFormDefine("sntps", web_sntps_form);
	#endif
	web_button_add_hook("sntp", "button-submit", web_sntp_button, NULL);
	return 0;
}

