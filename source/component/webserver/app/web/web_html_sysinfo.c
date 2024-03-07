/**
 * @file      : web_html_sysinfo.c
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-02-05
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
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


#include "web_api.h"
#include "web_app.h"

#ifdef ZPL_APP_MODULE
#include "application.h"
#endif




#ifndef FSHIFT
# define FSHIFT 16              /* nr of bits of precision */
#endif
#define FIXED_1      (1 << FSHIFT)     /* 1.0 as fixed-point */
#define LOAD_INT(x)  (unsigned)((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1 - 1)) * 100)

static char *_web_kernel_version(char *tmp)
{
#if LINUX_VERSION_CODE
	zpl_uint32 mver = 0, sver = 0, lver = 0;
	mver = (LINUX_VERSION_CODE>>16) & 0xff;
	sver = (LINUX_VERSION_CODE>>8) & 0xff;
	lver = (LINUX_VERSION_CODE) & 0xff;
	sprintf(tmp, "Linux Version %u.%u.%u", mver, sver, lver);
	return tmp;
#else
	if(access("/proc/version", F_OK) == 0)
	{
		char tttmp[128];
		memset(tttmp, 0, sizeof(tttmp));
		os_read_file("/proc/version", tttmp, sizeof(tttmp));
		if(strlen(tttmp))
		{
			char *b = strstr(tttmp, "(");
			if(b)
			{
				memcpy(tmp, tttmp, tttmp-b-1);
				return tmp;
			}
		}
	}
	sprintf(tmp, "%s", "Linux version");
#endif
	return tmp;
}

static char *_web_serial_number(char *tmp)
{
	zpl_int8	serial[64];
	memset(serial, 0, sizeof(serial));
	host_config_get_api(API_GET_SERIAL_CMD, serial);
	sprintf(tmp, "%s", serial);
	return tmp;
}

static char *_web_localtime(char *tmp)
{
	sprintf(tmp, "%s", os_time_fmt ("date", os_time(NULL)));
	return tmp;
}
static char *_web_uptime(char *tmp)
{
	zpl_uint32 updays = 0, uphours = 0, upminutes = 0;
	struct sysinfo info;
	sysinfo(&info);
	updays = (unsigned) info.uptime / (unsigned)(60*60*24);
	if (updays)
		sprintf(tmp, "%u day%s, ", updays, (updays != 1) ? "s" : "");
	upminutes = (unsigned) info.uptime / (unsigned)60;
	uphours = (upminutes / (unsigned)60) % (unsigned)24;
	upminutes %= 60;
	if (uphours)
		sprintf(tmp + strlen(tmp), "%2u:%02u", uphours, upminutes);
	else
		sprintf(tmp + strlen(tmp), "%u min", upminutes);
	return tmp;
}
static char *_web_cpu_load(char *tmp)
{
	struct sysinfo info;
	sysinfo(&info);
	sprintf(tmp, " %u.%02u, %u.%02u, %u.%02u",
			LOAD_INT(info.loads[0]), LOAD_FRAC(info.loads[0]),
			LOAD_INT(info.loads[1]), LOAD_FRAC(info.loads[1]),
			LOAD_INT(info.loads[2]), LOAD_FRAC(info.loads[2]));
	return tmp;
}

#undef FSHIFT
#undef FIXED_1
#undef LOAD_INT
#undef LOAD_FRAC


/*
static int jst_username(int eid, webs_t wp, int argc, char **argv)
{
	if(wp->username)
		websWrite(wp, "%s", wp->username);
	else
	{
        char *username = (char*) websGetSessionVar(wp, WEBS_SESSION_USERNAME, 0);
        if(username)
        {
        	websWrite(wp, "%s", username);
        }
        else
        	websWrite(wp, "%s", "admin");
	}
    return 0;
}


static int jst_web_type(int eid, webs_t wp, int argc, char **argv)
{
	if(web_app)
	{
		if(web_app->webtype == WEB_TYPE_HOME_WIFI)
		{
			if(argv[0] && strstr(argv[0], "get"))
				websWrite(wp, "homewifi");
			return 0;
		}
		else if(web_app->webtype == WEB_TYPE_HOME_SWITCH)
		{
			if(argv[0] && strstr(argv[0], "get"))
				websWrite(wp, "switch");
			return 1;
		}
		else if(web_app->webtype == WEB_TYPE_HOME_ROUTE)
		{
			if(argv[0] && strstr(argv[0], "get"))
				websWrite(wp, "route");
			return 2;
		}
	}
	return 0;
}

static int jst_web_os(int eid, webs_t wp, int argc, char **argv)
{
	if(web_app)
	{
		if(web_app->webos == WEB_OS_OPENWRT)
		{
			if(argv[0] && strstr(argv[0], "get"))
				websWrite(wp, "openwrt");
			return 0;
		}
		else if(web_app->webos == WEB_OS_LINUX)
		{
			if(argv[0] && strstr(argv[0], "get"))
				websWrite(wp, "linux");
			return 1;
		}
	}
	return 0;
}
*/

static int web_define_get(Webs *wp, char *path, char *query)
{
	#if ME_GOAHEAD_JSON	
	cJSON *root = NULL;
	cJSON *obj = NULL;
	obj = cJSON_CreateObject();
	if(obj)
	{
		cJSON_AddStringToObject(obj,"webtype", web_type_string(web_app));
		cJSON_AddStringToObject(obj,"ostype", web_os_type_string(web_app));
		root = cJSON_CreateObject();
		if(root)
		{
			cJSON_AddItemToObject(root, "webSys", obj);
			websResponseJson(wp, HTTP_CODE_OK, root);
			return OK;
		}
		cJSON_Delete(obj);
	}
	#else
	websSetStatus(wp, 200);

	websWriteCache(wp, "%s", "[");

	websWriteCache(wp,"{\"response\":\"%s\", \"webtype\":\"%s\", \
			\"ostype\":\"%s\", \"platform\":\"%s\", \"device\":\"%s\", \
			\"switch\":%s, \"dns\":%s, \"dhcp\":%s, \"sip\":%s, \"wireless\":%s}",
			"OK", web_type_string(web_app), web_os_type_string(web_app), "TSLX5", "TSLX5",
			"false", "false", "false", "true", "true");

	websWriteCache(wp, "%s", "]");

	websWriteHeaders (wp, websWriteCacheLen(wp), 0);
	websWriteHeader (wp, "Content-Type", "text/plain");
	websWriteEndHeaders (wp);	
	websWriteCacheFinsh(wp);

	websDone(wp);
	#endif
	return OK;
}


static int web_sysinfo_get(Webs *wp, char *path, char *query)
{

#if ME_GOAHEAD_JSON	
	char tmp[128];
	struct host_system host_system;
	memset(&host_system, 0, sizeof(struct host_system));
	host_system_information_get(&host_system);
	cJSON *root = NULL;
	cJSON *obj = NULL;
	obj = cJSON_CreateObject();
	if(obj)
	{
		cJSON_AddStringToObject(obj,"devicename", host_name_get());
#if defined(ZPL_BUILD_ARCH_X86)||defined(ZPL_BUILD_ARCH_X86_64)
		if(host_system.model_name)
		{
			cJSON_AddStringToObject(obj,"platfrom", host_system.model_name);
		}
#else
		if(host_system.system_type && host_system.cpu_model)
		{
			memset(tmp, 0, sizeof(tmp));
			sprintf (tmp, "%s %s", host_system.system_type, host_system.cpu_model);
			cJSON_AddStringToObject(obj,"platfrom", tmp);
		}
#endif
		memset(tmp, 0, sizeof(tmp));
		cJSON_AddStringToObject(obj,"kernel", _web_kernel_version(tmp));

		memset(tmp, 0, sizeof(tmp));
		cJSON_AddStringToObject(obj,"version", "V0.1.2.11");

		cJSON_AddStringToObject(obj,"hardware", "V0.0.2.1");

		memset(tmp, 0, sizeof(tmp));
		cJSON_AddStringToObject(obj,"serialno", _web_serial_number(tmp));

		memset(tmp, 0, sizeof(tmp));
		cJSON_AddStringToObject(obj,"localtime", _web_localtime(tmp));

		memset(tmp, 0, sizeof(tmp));
		cJSON_AddStringToObject(obj,"uptime", _web_uptime(tmp));

		memset(tmp, 0, sizeof(tmp));
		cJSON_AddStringToObject(obj,"cpuload", _web_cpu_load(tmp));

		host_system.mem_total = host_system.s_info.totalram >> 10;//               //total
		host_system.mem_uses = (host_system.s_info.totalram - host_system.s_info.freeram) >> 10; //used
		host_system.mem_free = host_system.s_info.freeram >> 10;                 //free
		cJSON_AddNumberToObject(obj,"memoryload", ((host_system.mem_uses*100)/host_system.mem_total));

		cJSON_AddNumberToObject(obj,"diskload", ((host_system.mem_uses*100)/host_system.mem_total));
		root = cJSON_CreateObject();
		if(root)
		{
			cJSON_AddItemToObject(root, "sysinfo", obj);
			websResponseJson(wp, HTTP_CODE_OK, root);
			return OK;
		}
		websResponseJson(wp, HTTP_CODE_OK, NULL);
		cJSON_Delete(obj);
	}
	else
		websResponseJson(wp, HTTP_CODE_OK, NULL);
#else	
	web_json_format_result(wp, HTTP_CODE_UNSUPPORTED_MEDIA_TYPE, "webservice is not support json");
#endif
	return OK;
}

int web_html_sysinfo_init(void)
{
	websFormDefine("web-define", web_define_get);
	websFormDefine("sysinfo", web_sysinfo_get);
	return 0;
}
