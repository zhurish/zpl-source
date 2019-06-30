/*
 * web_app_jst.c
 *
 *  Created on: Apr 5, 2019
 *      Author: zhurish
 */

//#include "zebra.h"
#ifdef HAVE_CONFIG_H
#include "plconfig.h"
#endif /* HAVE_CONFIG_H */

#include "os_platform.h"
#include <sys/sysinfo.h>
#include <netinet/in.h>
#include "module.h"
#include "memory.h"
#include "vector.h"
#include "zassert.h"
#include "host.h"
#include "log.h"
#include "os_list.h"
#include "os_sem.h"
#include "os_task.h"

#include "goahead.h"
#include "webgui_app.h"

#ifndef FSHIFT
# define FSHIFT 16              /* nr of bits of precision */
#endif
#define FIXED_1      (1 << FSHIFT)     /* 1.0 as fixed-point */
#define LOAD_INT(x)  (unsigned)((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1 - 1)) * 100)

static int jst_hostname(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
    websWrite(wp, "%s", host_name_get());
    //gfree(date);
    return 0;
}

static int jst_localtime(int eid, webs_t wp, int argc, char **argv)
{
    char *date = websGetDateString(NULL);
    websWrite(wp, "%s", date);
    gfree(date);
    return 0;
}

static int jst_kernel_version(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
    websWrite(wp, "%s", "Linux version 3.10.0-957.10.1.el7.x86_64");
    //gfree(date);
    return 0;
}

static int jst_uptime(int eid, webs_t wp, int argc, char **argv)
{
	char tmp[128];
	unsigned int updays = 0, uphours = 0, upminutes = 0;
	struct sysinfo info;
	sysinfo(&info);
	memset(tmp, 0, sizeof(tmp));
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
    //char *date = websGetDateString(NULL);
    websWrite(wp, "%s", tmp);
    //gfree(date);
    return 0;
}

static int jst_cpu_load(int eid, webs_t wp, int argc, char **argv)
{
	char tmp[128];
	struct sysinfo info;
	sysinfo(&info);
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, " %u.%02u, %u.%02u, %u.%02u",
			LOAD_INT(info.loads[0]), LOAD_FRAC(info.loads[0]),
			LOAD_INT(info.loads[1]), LOAD_FRAC(info.loads[1]),
			LOAD_INT(info.loads[2]), LOAD_FRAC(info.loads[2]));

    //char *date = websGetDateString(NULL);
    websWrite(wp, "%s", tmp);
    //gfree(date);
    return 0;
}
#undef FSHIFT
#undef FIXED_1
#undef LOAD_INT
#undef LOAD_FRAC

static int jst_firmware_version(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
    websWrite(wp, "%s", "V0.1.0.1");
    //gfree(date);
    return 0;
}

static int jst_arch(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
    websWrite(wp, "%s", "MIPsel");
    //gfree(date);
    return 0;
}

static int jst_platform(int eid, webs_t wp, int argc, char **argv)
{
	struct host_system host_system;
	memset(&host_system, 0, sizeof(struct host_system));
	host_system_information_get(&host_system);
#ifdef BUILD_X86
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
    //char *date = websGetDateString(NULL);
    //websWrite(wp, "%s", "Switch Platform");
    //gfree(date);
    return 0;
}

/*static int jst_button(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
	websWrite(wp, "%s", "admin");
    return 0;
}*/

static int jst_web_header1(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
	websWrite(wp, "%s", "OpenWrt");
    return 0;
}

static int jst_web_header2(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
	websWrite(wp, "%s", "OpenWrt");
    return 0;
}



static int jst_web_title1(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
	websWrite(wp, "%s", "OpenWrt-web");
    return 0;
}

static int jst_web_title2(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
	websWrite(wp, "%s", "OpenWrt-web");
    return 0;
}

static int jst_web_title3(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
	websWrite(wp, "%s", "OpenWrt-web");
    return 0;
}

static int jst_web_title4(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
	websWrite(wp, "%s", "OpenWrt-web");
    return 0;
}

static int jst_username(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
	if(wp->username)
		websWrite(wp, "%s", wp->username);
	else
		websWrite(wp, "%s", "admin");
    //gfree(date);
    return 0;
}

static int jst_memory(int eid, webs_t wp, int argc, char **argv)
{
	struct host_system host_system;
	memset(&host_system, 0, sizeof(struct host_system));
	host_system_information_get(&host_system);

	host_system.mem_total = host_system.s_info.totalram >> 10;//               //total
	host_system.mem_uses = (host_system.s_info.totalram - host_system.s_info.freeram) >> 10; //used
	host_system.mem_free = host_system.s_info.freeram >> 10;                 //free

	websWrite(wp, "Total:%d MB, Free:%d MB, Uses:%d MB",
			host_system.mem_total>>10, host_system.mem_free>>10, host_system.mem_uses>>10);
    //char *date = websGetDateString(NULL);
	//websWrite(wp, "%s", "admin");
    return 0;
}


/*
static bool can(Webs *wp, char *ability)
{
    assert(wp);
    assert(ability && *ability);

    if (wp->user && hashLookup(wp->user->abilities, ability)) {
        return 1;
    }
    return 0;
}
abilities=view,edit,delete,manage
*/

static int jst_auth_level(int eid, webs_t wp, int argc, char **argv)
{

    //char *date = websGetDateString(NULL);
    websWrite(wp, "%d", "1");
    //role name=manager abilities=view,edit,delete
    /*
     * manager:     view    edit    delete
     * root:        view    edit    delete
     * admin:       view    edit    delete
     * user:        view    edit    delete
     */
    //gfree(date);
    return 0;
}

static int jst_button(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
	websWrite(wp, "%s", "admin");
    return 0;
}

static int jst_flags(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
	websWrite(wp, "%s", "admin");
    return 0;
}

static int jst_ipv6(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
	websWrite(wp, "%s", "enable");
    return 0;
}

static int jst_dhcpd(int eid, webs_t wp, int argc, char **argv)
{
    //char *date = websGetDateString(NULL);
	websWrite(wp, "%s", "admin");
    return 0;
}


static int jst_syslog(int eid, webs_t wp, int argc, char **argv)
{
	int rows = 1;
	FILE *f;
	char buf[4096];

	f = fopen("/home/zhurish/Downloads/sip-test.txt", "r");
	if (f)
	{
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
			rows++;
		}
	}
	websWrite(wp, "<textarea style=\"font-size: 12px;\" readonly=\"readonly\" wrap=\"off\" rows=\"%d\" id=\"syslog\">", rows);
	fseek(f, 0, SEEK_SET);
	if (f)
	{
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
		    websWrite(wp, buf);
		    memset(buf, 0, sizeof(buf));
		}
		fclose(f);
	}
	websWrite(wp, "</textarea>");
    return 0;
}


int web_jst_init(void)
{
	websDefineJst("jst_hostname", jst_hostname);
	websDefineJst("jst_kernel_version", jst_kernel_version);
	websDefineJst("jst_uptime", jst_uptime);
	websDefineJst("jst_cpu_load", jst_cpu_load);
	websDefineJst("jst_localtime", jst_localtime);
	websDefineJst("jst_firmware_version", jst_firmware_version);
	websDefineJst("jst_arch", jst_arch);
	websDefineJst("jst_platform", jst_platform);
	websDefineJst("jst_username", jst_username);
	websDefineJst("jst_auth_level", jst_auth_level);
	websDefineJst("jst_memory", jst_memory);

	websDefineJst("jst_web_header1", jst_web_header1);
	websDefineJst("jst_web_header2", jst_web_header2);
	websDefineJst("jst_web_title1", jst_web_title1);
	websDefineJst("jst_web_title2", jst_web_title2);

	websDefineJst("jst_web_title3", jst_web_title3);
	websDefineJst("jst_web_title4", jst_web_title4);


	websDefineJst("jst_button", jst_button);
	websDefineJst("jst_flags", jst_flags);

	websDefineJst("jst_ipv6", jst_ipv6);
	websDefineJst("jst_dhcpd", jst_dhcpd);
	websDefineJst("jst_syslog", jst_syslog);
	return 0;
}
