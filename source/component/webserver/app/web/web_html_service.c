/**
 * @file      : web_html_service.c
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
#include "web_api.h"
#include "web_app.h"
#include "service.h"

#ifdef ZPL_SERVICE_TFTPD
#include "tftpdLib.h"
#endif
#ifdef ZPL_SERVICE_FTPD
#include "ftpdLib.h"
#endif

#ifdef ZPL_SERVICE_TFTPD
static int web_tftpd_form(Webs *wp, char *path, char *query)
{
	if(websGetMethodCode(wp) == WEBS_METHOD_GET)
	{
		#if ME_GOAHEAD_JSON	
		zpl_bool	enable = zpl_false;
		zpl_int32	port = 0;
		char ipaddress[64];
		char path[256];
		cJSON *obj = cJSON_CreateObject();
		if(obj)
		{
			memset(ipaddress, 0, sizeof(ipaddress));
			memset(path, 0, sizeof(path));
			tftpdCfgGet(&enable, &ipaddress, &port, path);
			cJSON_AddBoolToObject(obj,"enable", enable);
			cJSON_AddStringToObject(obj,"address", ipaddress);
			cJSON_AddStringToObject(obj,"path", path);	
			cJSON_AddNumberToObject(obj,"port", port);
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
			char *address = cJSON_GetStringValue(root, "address");
			char *path = cJSON_GetStringValue(root, "path");
			
			if(!enable)
			{
				if(tftpdIsEnable())
					ret = tftpdEnable(zpl_false, NULL, 0, NULL);
				else
					ret = OK;	
				if(ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			if(address && port)
			{
				ret = tftpdEnable(enable, address, port, path);
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
#endif/*ZPL_SERVICE_TFTPD*/

#ifdef ZPL_SERVICE_FTPD
static int web_ftpd_form(Webs *wp, char *path, char *query)
{
	if(websGetMethodCode(wp) == WEBS_METHOD_GET)
	{
		#if ME_GOAHEAD_JSON	
		zpl_bool	enable = zpl_false;
		zpl_int32	port = 0;
		char ipaddress[64];
		char path[256];
		cJSON *obj = cJSON_CreateObject();
		if(obj)
		{
			memset(ipaddress, 0, sizeof(ipaddress));
			memset(path, 0, sizeof(path));
			tpdCfgGet(&enable, &ipaddress, &port, path);
			cJSON_AddBoolToObject(obj,"enable", enable);
			cJSON_AddStringToObject(obj,"address", ipaddress);
			cJSON_AddStringToObject(obj,"path", path);	
			cJSON_AddNumberToObject(obj,"port", port);
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
			char *address = cJSON_GetStringValue(root, "address");
			char *path = cJSON_GetStringValue(root, "path");
			if(!enable)
			{
				if(ftpdIsEnable())
					ret = ftpdDisable();
				else
					ret = OK;	
				if(ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			if(port)
			{
				ret = ftpdEnable(address, port, path);
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
#endif/*ZPL_SERVICE_FTPD*/


static int web_service_button(Webs *wp, void *pVoid)
{
	#if ME_GOAHEAD_JSON	
	int ret = ERROR;
	cJSON *root = websGetJsonVar(wp);
	if(root)
	{
		zpl_bool enable = cJSON_GetBoolValue(root, "enable");
		int port = cJSON_GetIntValue(root, "port");
		char * path = cJSON_GetStringValue(root, "path");
		char *address = cJSON_GetStringValue(root, "address");
		char *module  = cJSON_GetStringValue(root, "module");
		if(module && os_memcmp(module, "tftpd", 5) == 0)
		{
			#ifdef ZPL_SERVICE_TFTPD
			if(!enable)
			{
				if(tftpdIsEnable())
					ret = tftpdEnable(zpl_false, NULL, 0, NULL);
				else
					ret = OK;	
				if(ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			if(port)
			{
				ret = tftpdEnable(enable, address, port, path);
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
		else if(module && os_memcmp(module, "ftpd", 5) == 0)
		{
			#ifdef ZPL_SERVICE_SNTPC
			if(!enable)
			{
				if(ftpdIsEnable())
					ret = ftpdDisable();
				else
					ret = OK;	
				if(ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			if(port)
			{
				ret = ftpdEnable(address, port, path);
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

int web_html_service_init(void)
{
	#ifdef ZPL_SERVICE_SNTPC
	websFormDefine("tftpd", web_tftpd_form);
	#endif
	#ifdef ZPL_SERVICE_SNTPS
	websFormDefine("ftpd", web_ftpd_form);
	#endif
	web_button_add_hook("service", "button-submit", web_service_button, NULL);
	return 0;
}

