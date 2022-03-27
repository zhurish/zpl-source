/*
 * web_app_jst.c
 *
 *  Created on: Apr 5, 2019
 *      Author: zhurish
 */

//#include "zpl_include.h"
#include "zpl_include.h"
#include "module.h"
#include "zmemory.h"
#include "zassert.h"
#include "command.h"
#include "prefix.h"
#include "host.h"
#include "log.h"
#include "vty.h"


#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

#ifdef ZPL_APP_MODULE
#include "application.h"
#endif




#ifndef FSHIFT
# define FSHIFT 16              /* nr of bits of precision */
#endif
#define FIXED_1      (1 << FSHIFT)     /* 1.0 as fixed-point */
#define LOAD_INT(x)  (unsigned)((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1 - 1)) * 100)
#if 0
static int jst_hostname(int eid, webs_t wp, int argc, char **argv)
{
    websWrite(wp, "%s", host_name_get());
    return 0;
}

static int jst_web_header(int eid, webs_t wp, int argc, char **argv)
{
#ifdef APP_X5BA_MODULE
	if(access("/etc/main_version", F_OK) == 0)
	{
		char tmp[128];
		memset(tmp, 0, sizeof(tmp));
		os_read_file("/etc/main_version", tmp, sizeof(tmp));
		if(strlen(tmp))
		{
			//char *brk = strstr(tmp, "-V");
			websWrite(wp, "%s", tmp);
			return 0;
		}
	}
	websWrite(wp, "%s", "TSLSmart-X5CM");
    //websWrite(wp, "%s", "TSLSmart-X5CM-V1.1.2.3");
#endif
#ifdef APP_X5BA_MODULE
	websWrite(wp, "%s", "TSLSmart-V9");
    //websWrite(wp, "%s", "TSLSmart-X5CM-V1.1.2.3");
#endif
    return 0;
}

static int jst_kernel_version(int eid, webs_t wp, int argc, char **argv)
{
	//Linux version 4.14.121
#if LINUX_VERSION_CODE
	zpl_uint32 mver = 0, sver = 0, lver = 0;
	mver = (LINUX_VERSION_CODE>>16) & 0xff;
	sver = (LINUX_VERSION_CODE>>8) & 0xff;
	lver = (LINUX_VERSION_CODE) & 0xff;
	websWrite(wp, "Linux Version %u.%u.%u", mver, sver, lver);
	return 0;
#else
	if(access("/proc/version", F_OK) == 0)
	{
		char tmp[128];
		memset(tmp, 0, sizeof(tmp));
		os_read_file("/proc/version", tmp, sizeof(tmp));
		if(strlen(tmp))
		{
			char *b = strstr(tmp, "(");
			if(b)
			{
				websWriteBlock(wp, tmp, tmp-b-1);
				//websWrite(wp, "%s", tmp);
				return 0;
			}
		}
	}
    websWrite(wp, "%s", "Linux version");
#endif
    return 0;
}

static int jst_serial_number(int eid, webs_t wp, int argc, char **argv)
{
	zpl_int8	serial[64];
	memset(serial, 0, sizeof(serial));
	host_config_get_api(API_GET_SERIAL_CMD, serial);
    websWrite(wp, "%s", serial);
    return 0;
}
#endif
/*
static int jst_localtime(int eid, webs_t wp, int argc, char **argv)
{
	websWrite(wp, "%s", os_time_fmt ("date", os_time(NULL)));
    return 0;
}
*/
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
/*
static int jst_uptime(int eid, webs_t wp, int argc, char **argv)
{
	char tmp[128];
	memset(tmp, 0, sizeof(tmp));
	websWrite(wp, "%s", _web_uptime(tmp));
	return 0;
}
*/
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
/*
static int jst_cpu_load(int eid, webs_t wp, int argc, char **argv)
{
	char tmp[128];
	memset(tmp, 0, sizeof(tmp));
	websWrite(wp, "%s", _web_cpu_load(tmp));
	return 0;
}
*/
#undef FSHIFT
#undef FIXED_1
#undef LOAD_INT
#undef LOAD_FRAC

#ifdef THEME_V9UI

static int web_firmware_version_get(char *argv, char *ver)
{
#ifdef ZPL_APP_MODULE
	if(strstr(argv, "VO"))
	{
		if(access("/etc/main_version", F_OK) == 0)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			os_read_file("/etc/main_version", tmp, sizeof(tmp));
			if(strlen(tmp))
			{
				strcpy(ver, tmp);
				return 0;
			}
		}
		strcpy(ver, "V0.0.0.1");
	}
#ifdef APP_X5BA_MODULE
	else if(strstr(argv, "IO"))
	{
		if(x5b_app_mgt && strlen(x5b_app_mgt->app_a.ioversion))
		{
			strcpy(ver, x5b_app_mgt->app_a.ioversion);
			return 0;
		}
		strcpy(ver, " ");
	}
	else if(strstr(argv, "CO"))
	{
		if(x5b_app_mgt && strlen(x5b_app_mgt->app_a.version))
		{
			strcpy(ver, x5b_app_mgt->app_a.version);
			return 0;
		}
		strcpy(ver, " ");
	}
	else if(strstr(argv, "ZO"))
	{
		if(x5b_app_mgt && strlen(x5b_app_mgt->app_c.version))
		{
			strcpy(ver, x5b_app_mgt->app_c.version);
			return 0;
		}
		strcpy(ver, " ");
	}
#endif
#ifdef APP_V9_MODULE
	else if(strstr(argv, "JO"))
	{
		strcpy(ver, "V0.0.0.1");
	}
#endif

#else
	if(access("/etc/main_version", F_OK) == 0)
	{
		char tmp[128];
		memset(tmp, 0, sizeof(tmp));
		os_read_file("/etc/main_version", tmp, sizeof(tmp));
		if(strlen(tmp))
		{
			strcpy(ver, tmp);
			return 0;
		}
	}
	strcpy(ver, "V0.0.0.1");
#endif
	return 0;
}

static int jst_firmware_version(int eid, webs_t wp, int argc, char **argv)
{
#if 1
	char ver[128];
	memset(ver, 0, sizeof(ver));
	web_firmware_version_get(argv[0], ver);
	if(strlen(ver))
	{
		websWrite(wp, "%s", ver);
		return 0;
	}
#else
#ifdef ZPL_APP_MODULE
	if(strstr(argv[0], "VO"))
	{
		if(access("/etc/main_version", F_OK) == 0)
		{
			char tmp[128];
			memset(tmp, 0, sizeof(tmp));
			os_read_file("/etc/main_version", tmp, sizeof(tmp));
			if(strlen(tmp))
			{
				websWrite(wp, "%s", tmp);
				return 0;
			}
		}
		websWrite(wp, "%s", "V0.0.0.1");
	}
#ifdef APP_X5BA_MODULE
	else if(strstr(argv[0], "IO"))
	{
		if(x5b_app_mgt && strlen(x5b_app_mgt->app_a.ioversion))
		{
			websWrite(wp, "%s", x5b_app_mgt->app_a.ioversion);
			return 0;
		}
		websWrite(wp, "%s", " ");
	}
	else if(strstr(argv[0], "CO"))
	{
		if(x5b_app_mgt && strlen(x5b_app_mgt->app_a.version))
		{
			websWrite(wp, "%s", x5b_app_mgt->app_a.version);
			return 0;
		}
		websWrite(wp, "%s", " ");
	}
	else if(strstr(argv[0], "ZO"))
	{
		if(x5b_app_mgt && strlen(x5b_app_mgt->app_c.version))
		{
			websWrite(wp, "%s", x5b_app_mgt->app_c.version);
			return 0;
		}
		websWrite(wp, "%s", " ");
	}
#endif
#else
	if(access("/etc/main_version", F_OK) == 0)
	{
		char tmp[128];
		memset(tmp, 0, sizeof(tmp));
		os_read_file("/etc/main_version", tmp, sizeof(tmp));
		if(strlen(tmp))
		{
			websWrite(wp, "%s", tmp);
			return 0;
		}
	}
	websWrite(wp, "%s", "V0.0.0.1");
#endif
#endif
    return 0;
}

static int jst_arch(int eid, webs_t wp, int argc, char **argv)
{
	struct host_system host_system;
	memset(&host_system, 0, sizeof(struct host_system));
	host_system_information_get(&host_system);
#if defined(ZPL_BUILD_ARCH_X86)||defined(ZPL_BUILD_ARCH_X86_64)
	if(host_system.model_name)
	{
		websWrite(wp, "%s", host_system.model_name);
	}
#else
	if(host_system.cpu_model)
	{
	    websWrite(wp, "%s", host_system.cpu_model);
	}
#endif
    return 0;
}

static int jst_platform(int eid, webs_t wp, int argc, char **argv)
{
	struct host_system host_system;
	memset(&host_system, 0, sizeof(struct host_system));
	host_system_information_get(&host_system);
#if defined(ZPL_BUILD_ARCH_X86)||defined(ZPL_BUILD_ARCH_X86_64)
	if(host_system.model_name)
	{
	    websWrite(wp, "%s", host_system.model_name);
	}
#else
	if(host_system.system_type && host_system.cpu_model)
	{
	    websWrite(wp, "%s %s", host_system.system_type, host_system.cpu_model);
	}
#endif
    return 0;
}


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

static int jst_auth_level(int eid, webs_t wp, int argc, char **argv)
{
	websWrite(wp, "%d", "1");
	//role name=manager abilities=view,edit,delete
    /*
     * manager:     view    edit    delete
     * root:        view    edit    delete
     * admin:       view    edit    delete
     * user:        view    edit    delete
	*/
	return 0;
}


static int jst_memory_progress_view(Webs *wp, void *p)
{
	char *strval = NULL;

	char tmp1[128],tmp2[128];
	memset(tmp1, 0, sizeof(tmp1));
	memset(tmp2, 0, sizeof(tmp2));
	struct host_system host_system;
	strval = websGetVar(wp, T("ID"), T(""));

	memset(&host_system, 0, sizeof(struct host_system));
	host_system_information_get(&host_system);

	host_system.mem_total = host_system.s_info.totalram >> 10;//               //total
	host_system.mem_uses = (host_system.s_info.totalram - host_system.s_info.freeram) >> 10; //used
	host_system.mem_free = host_system.s_info.freeram >> 10;                 //free

/*	websWrite(wp, "Total:%d MB, Free:%d MB, Uses:%d MB",
			host_system.mem_total>>10, host_system.mem_free>>10, host_system.mem_uses>>10);*/
	websSetCookie(wp, "webtype", "homewifi", "/", NULL, 0, 0);
	//websSetCookie(wp, "webapp", "tslv9", "/", NULL, 0, WEBS_COOKIE_SECURE);

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);

	websWrite(wp, "{\"response\":\"%s\", \"memuses\":\"%d%%\", \"localtime\":\"%s\", "
			"\"uptime\":\"%s\", \"cpu_load\":\"%s\"}",
		"OK", ((host_system.mem_uses*100)/host_system.mem_total),
		os_time_fmt ("date", os_time(NULL)), _web_uptime(tmp1), _web_cpu_load(tmp2));

	websDone(wp);
	return OK;
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
		if(web_app->webtype == WEB_OS_OPENWRT)
		{
			if(argv[0] && strstr(argv[0], "get"))
				websWrite(wp, "openwrt");
			return 0;
		}
		else if(web_app->webtype == ZPL_BUILD_LINUX)
		{
			if(argv[0] && strstr(argv[0], "get"))
				websWrite(wp, "linux");
			return 1;
		}
	}
	return 0;
}

#ifdef ME_DESCRIPTION
static int jst_web_description(int eid, webs_t wp, int argc, char **argv)
{
    websWrite(wp, "%s", ME_DESCRIPTION);
    return 0;
}
#endif

#ifdef ME_NAME
static int jst_web_name(int eid, webs_t wp, int argc, char **argv)
{
    websWrite(wp, "%s", ME_NAME);
    return 0;
}
#endif

#ifdef ME_TITLE
static int jst_web_title(int eid, webs_t wp, int argc, char **argv)
{
    websWrite(wp, "%s", ME_TITLE);
    return 0;
}
#endif
#ifdef ME_VERSION
static int jst_web_version(int eid, webs_t wp, int argc, char **argv)
{
    websWrite(wp, "%s", ME_VERSION);
    return 0;
}
#endif

#ifdef ME_SRC_PREFIX
static int jst_web_src_prefix(int eid, webs_t wp, int argc, char **argv)
{
    websWrite(wp, "%s", ME_SRC_PREFIX);
    return 0;
}
#endif


static int web_define_get(Webs *wp, char *path, char *query)
{
	//return web_return_text_plain(wp, ERROR);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	//websWriteHeader(wp, "Content-Type", "application/json");
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");

/*
    var webtype = "homewifi";
    var ostype = "openwrt";
    var platform = "TSLV9";
    var device = "TSLV9";
    var have_switch = true;
    var have_dns = false;
    var have_dhcp = true;
    var have_sip = true;
    var have_wireless = true;
*/

#ifdef APP_V9_MODULE
	websWrite(wp,"{\"response\":\"%s\", \"webtype\":\"%s\", \
			\"ostype\":\"%s\", \"platform\":\"%s\", \"device\":\"%s\", \
			\"switch\":%s, \"dns\":%s, \"dhcp\":%s, \"sip\":%s, \"wireless\":%s}",
			"OK", web_type_string(web_app), web_os_type_string(web_app), "TSLV9", "TSLV9",
			"false", "false", "false", "false", "false");
#else
	websWrite(wp,"{\"response\":\"%s\", \"webtype\":\"%s\", \
			\"ostype\":\"%s\", \"platform\":\"%s\", \"device\":\"%s\", \
			\"switch\":%s, \"dns\":%s, \"dhcp\":%s, \"sip\":%s, \"wireless\":%s}",
			"OK", web_type_string(web_app), web_os_type_string(web_app), "TSLX5", "TSLX5",
			"false", "false", "false", "true", "true");
#endif
	websWrite(wp, "%s", "]");
	websDone(wp);
	return OK;
}


static int jst_app_version(int eid, webs_t wp, int argc, char **argv)
{
#ifdef APP_V9_MODULE
	char ver[128];
	memset(ver, 0, sizeof(ver));
	web_firmware_version_get("JO", ver);
	if(strlen(ver))
	{
		websWrite(wp, "<tr>");
		websWrite(wp, "<td>计算模块版本</td>");
		websWrite(wp, "<td>%s</td>", ver);
		websWrite(wp, "</tr>");
	}
	memset(ver, 0, sizeof(ver));
	web_firmware_version_get("ZO", ver);
	if(strlen(ver))
	{
		websWrite(wp, "<tr>");
		websWrite(wp, "<td>主控模块版本</td>");
		websWrite(wp, "<td>%s</td>", ver);
		websWrite(wp, "</tr>");
	}
#endif
#ifdef APP_X5BA_MODULE
	char ver[128];
	memset(ver, 0, sizeof(ver));
	web_firmware_version_get("IO", ver);
	if(strlen(ver))
	{
		websWrite(wp, "<tr>");
		websWrite(wp, "<td>IO模块版本</td>");
		websWrite(wp, "<td>%s</td>", ver);
	    //<td><%jst_firmware_version("IO");%></td>
		websWrite(wp, "</tr>");
	}
	memset(ver, 0, sizeof(ver));
	web_firmware_version_get("CO", ver);
	if(strlen(ver))
	{
		websWrite(wp, "<tr>");
		websWrite(wp, "<td>刷卡模块版本</td>");
		websWrite(wp, "<td>%s</td>", ver);
		websWrite(wp, "</tr>");
	}
	memset(ver, 0, sizeof(ver));
	web_firmware_version_get("VO", ver);
	if(strlen(ver))
	{
		websWrite(wp, "<tr>");
		websWrite(wp, "<td>语音模块版本</td>");
		websWrite(wp, "<td>%s</td>", ver);
		websWrite(wp, "</tr>");
	}

	memset(ver, 0, sizeof(ver));
	web_firmware_version_get("ZO", ver);
	if(strlen(ver))
	{
		websWrite(wp, "<tr>");
		websWrite(wp, "<td>主控模块版本</td>");
		websWrite(wp, "<td>%s</td>", ver);
		websWrite(wp, "</tr>");
	}
#endif
	return 0;
}
#endif /* THEME_V9UI */

static int jst_systeminfo(Webs *wp, char *path, char *query)
{
	zpl_uint32 offset = 0;
	char buf[2048];
	char tmp[128];
	memset(buf, 0, sizeof(buf));
	struct host_system host_system;
	memset(&host_system, 0, sizeof(struct host_system));
	host_system_information_get(&host_system);
	wp->iValue1 = 0;
	wp->iValue = 0;
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	//websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	//websWrite(wp, "%s", "[");

	//websWrite(wp, "%s", host_name_get());
#if defined(ZPL_BUILD_ARCH_X86)||defined(ZPL_BUILD_ARCH_X86_64)
	if(host_system.model_name)
	{
		 sprintf (buf, "{\"devname\":\"%s\", \"platfrom\":\"%s\",", host_name_get(), host_system.model_name);
	}
#else
	if(host_system.system_type && host_system.cpu_model)
	{
		sprintf (buf, "{\"devname\":\"%s\", \"platfrom\":\"%s %s\",", host_name_get(),
				 host_system.system_type, host_system.cpu_model);
	}
#endif
	offset = strlen(buf);
#if LINUX_VERSION_CODE
	zpl_uint32 mver = 0, sver = 0, lver = 0;
	mver = (LINUX_VERSION_CODE>>16) & 0xff;
	sver = (LINUX_VERSION_CODE>>8) & 0xff;
	lver = (LINUX_VERSION_CODE) & 0xff;
	sprintf (buf + offset, "\"version\":\"Linux Version %u.%u.%u\",", mver, sver, lver);
#else
	if(access("/proc/version", F_OK) == 0)
	{
		memset(tmp, 0, sizeof(tmp));
		os_read_file("/proc/version", tmp, sizeof(tmp));
		if(strlen(tmp))
		{
			char *b = strstr(tmp, "(");
			if(b)
			{
				//websWriteBlock(wp, tmp, tmp-b-1);
				strcat (buf + offset, "\"version\":\"");
				offset = strlen(buf);
				strncat (buf + offset, tmp, tmp-b-1);
				offset = strlen(buf);
				strcat (buf + offset, "\",");
				offset = strlen(buf);
			}
		}
	}
#endif
	offset = strlen(buf);
	memset(tmp, 0, sizeof(tmp));
	sprintf (buf + offset, "\"appver\":\"%s\",", "V0.0.0.1");

	offset = strlen(buf);
	memset(tmp, 0, sizeof(tmp));
	sprintf (buf + offset, "\"uptime\":\"%s\",", _web_uptime(tmp));

	offset = strlen(buf);
	sprintf (buf + offset, "\"cpu_load\":\"%s\",", _web_cpu_load(tmp));

#ifdef APP_V9_MODULE
	offset = strlen(buf);
	v9_web_device_info(buf + offset);
#endif

	host_system.mem_total = host_system.s_info.totalram >> 10;//               //total
	host_system.mem_uses = (host_system.s_info.totalram - host_system.s_info.freeram) >> 10; //used
	host_system.mem_free = host_system.s_info.freeram >> 10;                 //free

	offset = strlen(buf);
	sprintf (buf + offset, "\"memload\":\"%d%%\"}", ((host_system.mem_uses*100)/host_system.mem_total));

	websWrite(wp, buf);
	//if_list_each(web_interface_name_tbl, wp);
	//websWrite(wp, "%s", "]");
	websDone(wp);
	wp->iValue = 0;
	wp->iValue1 = 0;
	return 0;
}

int web_system_jst_init(void)
{
#ifdef THEME_V9UI
#ifdef ME_DESCRIPTION
	websDefineJst("jst_web_description", jst_web_description);
#endif
#ifdef ME_NAME
	websDefineJst("jst_web_name", jst_web_name);
#endif
#ifdef ME_TITLE
	websDefineJst("jst_web_title", jst_web_title);
#endif
#ifdef ME_VERSION
	websDefineJst("jst_web_version", jst_web_version);
#endif
#ifdef ME_SRC_PREFIX
	websDefineJst("jst_web_src_prefix", jst_web_src_prefix);
#endif

	websDefineJst("jst_hostname", jst_hostname);
	websDefineJst("jst_kernel_version", jst_kernel_version);
	websDefineJst("jst_serial_number", jst_serial_number);

	websDefineJst("jst_firmware_version", jst_firmware_version);
	websDefineJst("jst_arch", jst_arch);
	websDefineJst("jst_platform", jst_platform);
	websDefineJst("jst_username", jst_username);
	websDefineJst("jst_auth_level", jst_auth_level);

	websDefineJst("jst_uptime", jst_uptime);
	websDefineJst("jst_cpu_load", jst_cpu_load);
	websDefineJst("jst_localtime", jst_localtime);

	websDefineJst("jst_app_version", jst_app_version);
	websDefineJst("jst_web_header", jst_web_header);
	websDefineJst("jst_web_type", jst_web_type);
	websDefineJst("jst_web_os", jst_web_os);

	websFormDefine("web-define", web_define_get);

	web_progress_view_add_hook("mem", "memtotal", jst_memory_progress_view, NULL);
#endif /* THEME_V9UI */
	websFormDefine("systeminfo", jst_systeminfo);
	return 0;
}
