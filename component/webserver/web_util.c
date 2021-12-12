/*
 * web_util.c
 *
 *  Created on: 2019年8月23日
 *      Author: DELL
 */

#include "zpl_include.h"
#include "module.h"
#include "memory.h"
#include "zassert.h"
#include "command.h"
#include "prefix.h"
#include "host.h"
#include "log.h"
#include "vty.h"
#include "cJSON.h"


#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"


int _web_app_debug = 0;



const char *webs_get_var(Webs *wp, const char *var, const char *defaultGetValue)
{
	web_assert(wp);
	const char *value = websGetVar(wp, var, defaultGetValue);
	if(value != NULL)
	{
		if(strlen(value) <= 0)
		{
			return NULL;
		}
		if(all_space(value))
			return NULL;
		if(strcasecmp(value, "undefined") == 0)
			return NULL;

		return str_trim(value);
	}
	return NULL;
}

int webs_json_parse(Webs *wp)
{
	const char *value = websGetVar(wp, "json", NULL);
	if(value != NULL)
	{
		wp->json_item = cJSON_Parse(value);
		if(!wp->json_item)
		{
			wp->json_array_num = 0;
			wp->json_array_size = 0;
			wp->json_array = NULL;
			wp->json_item = NULL;
			return -1;
		}
		return 0;
	}
	return -1;
}

int webs_jsonarray_parse(Webs *wp)
{
	const char *value = websGetVar(wp, "jsonarray", NULL);
	if(value == NULL)
	{
		return -1;
	}
	wp->json_array = cJSON_Parse(value);
	if(!wp->json_array)
	{
		wp->json_array_num = 0;
		wp->json_array_size = 0;
		wp->json_array = NULL;
		wp->json_item = NULL;
		return -1;
	}
	wp->json_array_size = cJSON_GetArraySize(wp->json_array);
	wp->json_array_num = 0;
	wp->json_item = cJSON_GetArrayItem(wp->json_array, wp->json_array_num);
	if(!wp->json_item)
	{
		cJSON_Delete(wp->json_array);
		wp->json_array_num = 0;
		wp->json_array_size = 0;
		wp->json_array = NULL;
		wp->json_item = NULL;
		return -1;
	}
	wp->json_array_num++;
	return 0;
}

int webs_jsonarray_next(Webs *wp)
{
	if(wp->json_array && wp->json_item && (wp->json_array_num < wp->json_array_size))
	{
		wp->json_item = cJSON_GetArrayItem(wp->json_array, wp->json_array_num);
		if(!wp->json_item)
		{
			cJSON_Delete(wp->json_array);
			wp->json_array_num = 0;
			wp->json_array_size = 0;
			wp->json_array = NULL;
			wp->json_item = NULL;
			return -1;
		}
		wp->json_array_num++;
		return 0;
	}
	return -1;
}

int webs_json_finsh(Webs *wp)
{
	if(wp->json_item)
	{
		cJSON_Delete(wp->json_item);
		wp->json_item = NULL;
	}
	if(wp->json_array)
	{
		cJSON_Delete(wp->json_array);
		wp->json_array = NULL;
	}
	wp->json_array_num = 0;
	wp->json_array_size = 0;
	return 0;
}

int webs_json_get_var(Webs *wp, const char *var,
		char *valuestring, int *valueint, double *valuedouble)
{
	cJSON *json_tmp = cJSON_GetObjectItem (wp->json_item, var);
	if(json_tmp)
	{
		if(json_tmp->type == cJSON_False)
		{
			if(valueint)
				*valueint = 0;
			return OK;
		}
		else if(json_tmp->type == cJSON_True)
		{
			if(valueint)
				*valueint = 1;
			return OK;
		}
		else if(json_tmp->type == cJSON_NULL)
		{
			if(valuestring)
				strcpy(valuestring, "NULL");
			return OK;
		}
		else if(json_tmp->type == cJSON_Number)
		{
			if(valueint)
				*valueint = json_tmp->valueint;
			if(valuedouble)
				*valuedouble = json_tmp->valuedouble;
			return OK;
		}
		else if(json_tmp->type == cJSON_String)
		{
			if(valuestring && json_tmp->valuestring)
				strcpy(valuestring, json_tmp->valuestring);
			return OK;
		}
		else if(json_tmp->type == cJSON_Array)
		{
			if(valuestring && json_tmp->valuestring)
				strcpy(valuestring, json_tmp->valuestring);
			return OK;
		}
		else if(json_tmp->type == cJSON_Object)
		{
			if(valuestring && json_tmp->valuestring)
				strcpy(valuestring, json_tmp->valuestring);
			return OK;
		}
	}
	return ERROR;
}


/*
 *
 */
static int web_return_message_format(Webs *wp, char * header, zpl_uint32 type, int ret, char * msg, char * json)
{
	web_assert(wp);
	web_assert(header);
	websSetStatus (wp, 200);
	websWriteHeaders (wp, -1, 0);
	websWriteHeader (wp, "Content-Type", header);
	websWriteEndHeaders (wp);

	if (type == 0)
	{
		if (ret == ERROR)
			websWrite (wp, "%s", "ERROR");
		else if (ret == OK)
			websWrite (wp, "%s", "OK");
	}
	else if (type == 1)
	{
		web_assert(msg);
		if (json)
			websWrite (
					wp,
					"{\"response\":\"%s\", \"msg\":\"%s\", \"data\":{\"%s\"}",
					(ret == ERROR) ? "ERROR" : "OK", msg, json);
		else
			websWrite (wp,
					   "{\"response\":\"%s\", \"msg\":\"%s\", \"data\":null}",
					   (ret == ERROR) ? "ERROR" : "OK", msg);
	}
	else if (type == 2)
	{
		web_assert(json);
		if(json)
			websWrite(wp, json);
	}
	else if (type == 3)
	{
		if (msg)
			websWrite (wp,
				   "{\"result\":\"%s\", \"reason\":\"%s\"}",
				   (ret == ERROR) ? "ERROR" : "OK", msg);
		else
			websWrite (wp,
				   "{\"result\":\"%s\", \"reason\":null}",
				   (ret == ERROR) ? "ERROR" : "OK");
	}
	websDone (wp);
	return ret;
}

int web_return_text_plain(Webs *wp, int ret)
{
/*	web_assert(wp);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	if(ret == ERROR)
		websWrite(wp, "%s", "ERROR");
	else if(ret == OK)
		websWrite(wp, "%s", "OK");
	else if(ret == '[')
		websWrite(wp, "%s", "[]");
	websDone(wp);
	return ret;*/
	return web_return_message_format(wp, "text/plain", 0,  ret, NULL, NULL);
}



int web_return_application_json_result(Webs *wp, int ret, char * result)
{
/*	web_assert(wp);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, json);
	websDone(wp);
	return OK;*/
	return web_return_message_format(wp, "application/json", 3,  0, result, NULL);
}


int web_return_application_json(Webs *wp, char * json)
{
/*	web_assert(wp);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);
	websWrite(wp, json);
	websDone(wp);
	return OK;*/
	return web_return_message_format(wp, "application/json", 2,  0, NULL, json);
}

int web_return_application_json_array(Webs *wp, int ret, char * msg, char * json)
{
/*	web_assert(wp);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);
	if(state == ERROR)
	{
		if(json)
			websWrite (wp, "{\"response\":\"ERROR\", \"msg\":\"%s\", \"data\":{\"%s\"}", msg, json);
		else
			websWrite (wp, "{\"response\":\"ERROR\", \"msg\":\"%s\", \"data\":null}", msg);
	}
	else if(state == OK)
	{
		if(json)
			websWrite (wp, "{\"response\":\"OK\", \"msg\":\"%s\", \"data\":{\"%s\"}", msg, json);
		else
			websWrite (wp, "{\"response\":\"OK\", \"msg\":\"%s\", \"data\":null}", msg);
	}
	websDone(wp);
	return state;*/
	return web_return_message_format(wp, "application/json", 1,  ret, msg, json);
}





/*
 *
 */
const char * web_type_string(web_app_t *wp)
{
	web_assert(wp);
	if(wp)
	{
		if(wp->webtype == WEB_TYPE_HOME_WIFI)
		{
			return "homewifi";
		}
		else if(wp->webtype == WEB_TYPE_HOME_SWITCH)
		{
			return "switch";
		}
		else if(wp->webtype == WEB_TYPE_HOME_ROUTE)
		{
			return "route";
		}
	}
	return "unknow";
}


const char * web_os_type_string(web_app_t *wp)
{
	web_assert(wp);
	if(wp)
	{
		if(wp->webos == WEB_OS_OPENWRT)
		{
			return "openwrt";
		}
		else if(wp->webos == WEB_OS_LINUX)
		{
			return "linux";
		}
	}
	return "unknow";
}


web_type web_type_get()
{
	if(web_app)
	{
		return web_app->webtype;
	}
	return WEB_TYPE_HOME_WIFI;
}

web_os web_os_get()
{
	if(web_app)
	{
		return web_app->webos;
	}
	return WEB_OS_OPENWRT;
}


int webs_username_password_update(void *pwp, char *username, char *password)
{
#if ME_GOAHEAD_AUTO_LOGIN
	char encodedPassword[128];
	memset(encodedPassword, 0, sizeof(encodedPassword));
	/*
	 * Password Encoded
	 */
	//webserver encoded cipher md5 realm goahead.com username admin password admin
	if(web_app_gopass_api(username, password, "md5", "goahead.com", encodedPassword) != 0 )
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Encoded password Value");
		return ERROR;
	}
	websSetUserPassword(username, encodedPassword);
	web_app_auth_save_api();
#endif

#if !ME_GOAHEAD_AUTO_LOGIN
	if(vty_user_create(NULL, username, password, zpl_false , zpl_true ) != CMD_SUCCESS)
	{
		if(WEB_IS_DEBUG(EVENT))
		{
			zlog_debug(MODULE_WEB, " Can not Change User Password for '%s'", username);
		}
		return ERROR;
	}
#endif
	return OK;
}
