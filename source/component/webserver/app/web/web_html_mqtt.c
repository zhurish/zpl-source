/**
 * @file      : web_html_mqtt.c
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-02-05
 *
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 *
 */
#define HAS_BOOL 1
#include "goahead.h"
#include "webutil.h"
#include "web_api.h"
#include "web_app.h"


#ifdef ZPL_MQTT_MODULE
#include "mqtt_app_conf.h"
#include "mqtt_app_util.h"
#include "mqtt_app_api.h"
#endif

#ifdef ZPL_MQTT_MODULE
static int web_mqtt_form(Webs *wp, char *path, char *query)
{
	if (websGetMethodCode(wp) == WEBS_METHOD_GET)
	{
#if ME_GOAHEAD_JSON
		zpl_bool enable = zpl_false;
		zpl_int32 port = 0;
		char ipaddress[64];
		char path[256];
		cJSON *obj = cJSON_CreateObject();
		if (obj)
		{
			if (mqtt_config)
			{
				cJSON_AddBoolToObject(obj, "enable", mqtt_config->enable);
				cJSON_AddStringToObject(obj, "address", mqtt_config->host);
				cJSON_AddStringToObject(obj, "username", mqtt_config->username);
				cJSON_AddStringToObject(obj, "password", mqtt_config->password);

				cJSON_AddNumberToObject(obj, "port", mqtt_config->enable);
				cJSON_AddNumberToObject(obj, "qos", mqtt_config->qos);
				cJSON_AddNumberToObject(obj, "keepalive", mqtt_config->keepalive);
				cJSON_AddNumberToObject(obj, "version", mqtt_config->mqtt_version);
			}
		}
		websResponseJson(wp, HTTP_CODE_OK, obj);
		return OK;
#endif
	}
	else if (websGetMethodCode(wp) == WEBS_METHOD_POST)
	{
		int ret = ERROR;
#if ME_GOAHEAD_JSON
		cJSON *root = websGetJsonVar(wp);
		if (root)
		{
			zpl_bool enable = cJSON_GetBoolValue(root, "enable");
			int port = cJSON_GetIntValue(root, "port");
			int keepalive = cJSON_GetIntValue(root, "keepalive");
			int version = cJSON_GetIntValue(root, "version");
			int qos = cJSON_GetIntValue(root, "qos");
			char *address = cJSON_GetStringValue(root, "address");
			char *username = cJSON_GetStringValue(root, "username");
			char *password = cJSON_GetStringValue(root, "password");

			if (!enable)
			{
				if (mqtt_isenable_api(mqtt_config))
					ret = mqtt_enable_api(mqtt_config, zpl_false);
				else
					ret = OK;
				if (ret == OK)
					return web_json_format_result(wp, 0, "OK");
				else
					return web_json_format_result(wp, ret, "failed");
			}
			if (port)
				ret = mqtt_connect_port_api(mqtt_config, port);
			if (address)
				ret = mqtt_connect_host_api(mqtt_config, address);
			if (username)
				ret = mqtt_username_api(mqtt_config, username);
			if (password)
				ret = mqtt_password_api(mqtt_config, password);

			if (keepalive)
				ret = mqtt_keepalive_api(mqtt_config, keepalive);
			if (version)
				ret = mqtt_version_api(mqtt_config, version);
			if (qos > 0)
				ret = mqtt_qos_api(mqtt_config, qos);
			if (ret != OK)
				return web_json_format_result(wp, ret, "failed");

			ret = mqtt_enable_api(mqtt_config, zpl_true);
			if (ret == OK)
				return web_json_format_result(wp, 0, "OK");
			else
				return web_json_format_result(wp, ret, "failed");
		}
		web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
#endif
	}
	websResponse(wp, HTTP_CODE_BAD_METHOD, "not support method,just support post or get method");
	return 0;
}

static int web_mqtt_button(Webs *wp, void *pVoid)
{
#if ME_GOAHEAD_JSON
	int ret = ERROR;
	cJSON *root = websGetJsonVar(wp);
	if (root)
	{
		zpl_bool enable = cJSON_GetBoolValue(root, "enable");
		int port = cJSON_GetIntValue(root, "port");
		int keepalive = cJSON_GetIntValue(root, "keepalive");
		int version = cJSON_GetIntValue(root, "version");
		int qos = cJSON_GetIntValue(root, "qos");
		char *address = cJSON_GetStringValue(root, "address");
		char *username = cJSON_GetStringValue(root, "username");
		char *password = cJSON_GetStringValue(root, "password");

		if (!enable)
		{
			if (mqtt_isenable_api(mqtt_config))
				ret = mqtt_enable_api(mqtt_config, zpl_false);
			else
				ret = OK;
			if (ret == OK)
				return web_json_format_result(wp, 0, "OK");
			else
				return web_json_format_result(wp, ret, "failed");
		}
		if (port)
			ret = mqtt_connect_port_api(mqtt_config, port);
		if (address)
			ret = mqtt_connect_host_api(mqtt_config, address);
		if (username)
			ret = mqtt_username_api(mqtt_config, username);
		if (password)
			ret = mqtt_password_api(mqtt_config, password);

		if (keepalive)
			ret = mqtt_keepalive_api(mqtt_config, keepalive);
		if (version)
			ret = mqtt_version_api(mqtt_config, version);
		if (qos > 0)
			ret = mqtt_qos_api(mqtt_config, qos);
		if (ret != OK)
			return web_json_format_result(wp, ret, "failed");

		ret = mqtt_enable_api(mqtt_config, zpl_true);
		if (ret == OK)
			return web_json_format_result(wp, 0, "OK");
		else
			return web_json_format_result(wp, ret, "failed");
	}
	else
		return web_json_format_result(wp, ERROR, "can not get module value");
#else
	return web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
#endif
}


int web_html_mqtt_init(void)
{
	websFormDefine("mqtt", web_mqtt_form);
	web_button_add_hook("mqtt", "button-submit", web_mqtt_button, NULL);
	return 0;
}
#endif /*ZPL_MQTT_MODULE*/