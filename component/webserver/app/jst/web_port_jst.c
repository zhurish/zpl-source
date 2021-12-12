/*
 * web_port_jst.c
 *
 *  Created on: 2019年8月3日
 *      Author: zhurish
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

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

#ifndef THEME_V9UI
static int jst_port_connect(int eid, webs_t wp, int argc, char **argv)
{
	if(argv[1])
	{
		return 1;
	}
	return 0;
}

#if 0
static int web_switch_port_tbl(Webs *wp, char *path, char *query)
{
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");

	websWrite(wp, "{\"port\":\"%s\", \"name\":\"%s\", \"vlan\":\"%s\", \"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"},",
			"1", "LAN1", "1","untagged","lan", "up");
	websWrite(wp, "{\"port\":\"%s\", \"name\":\"%s\", \"vlan\":\"%s\", \"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"},",
			"2", "LAN2", "1","untagged","lan", "up");
	websWrite(wp, "{\"port\":\"%s\", \"name\":\"%s\", \"vlan\":\"%s\", \"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"},",
			"3", "LAN3", "1","untagged","lan", "up");
	websWrite(wp, "{\"port\":\"%s\", \"name\":\"%s\", \"vlan\":\"%s\", \"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"},",
			"4", "LAN4", "2","untagged","wan", "up");
	websWrite(wp, "{\"port\":\"%s\", \"name\":\"%s\", \"vlan\":\"%s\", \"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"},",
			"5", "WWAN", "1","untagged","lan", "up");
	websWrite(wp, "{\"port\":\"%s\", \"name\":\"%s\", \"vlan\":\"%s\", \"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"}",
			"6", "CPU", "1","tagged","cpu", "up");

	websWrite(wp, "%s", "]");
	websDone(wp);
	return 0;
}

static int web_switch_port_add_tbl(Webs *wp, void *p)
{
/*	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "OK");
	websDone(wp);*/
	return ERROR;
}

static int web_switch_port_connect_tbl(Webs *wp, void *p)
{
/*	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "OK");
	websDone(wp);*/
	return ERROR;
}
#endif
#endif /* THEME_V9UI */

int web_port_jst_init(void)
{
#ifndef THEME_V9UI
	websDefineJst("jst_port_connect", jst_port_connect);
#endif /* THEME_V9UI */
#if 0
	websFormDefine("port-tbl", web_switch_port_tbl);
	web_button_add_hook("switch", "save", web_switch_port_add_tbl, NULL);
	web_button_add_hook("switch", "connect", web_switch_port_connect_tbl, NULL);
#endif
	return 0;
}
