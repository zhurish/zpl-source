/*
 * web_firewall_jst.c
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

#include "nsm_firewalld.h"


#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"


#ifndef THEME_V9UI
static int web_firewall_rule_forwards_one(firewall_t *rule, Webs *wp)
{
	char id[16];
	char type[16];
	char action[16];
	char proto[16];
	char source[64];
	char destination[64];
	char s_port[16];
	char d_port[16];
	char s_ifindex[64];
	char d_ifindex[64];
	char s_mac[16];
	char d_mac[16];

	if (rule->class == wp->iValue1)
	{
		memset(id, 0, sizeof(id));
		memset(type, 0, sizeof(type));
		memset(action, 0, sizeof(action));
		memset(proto, 0, sizeof(proto));
		memset(source, 0, sizeof(source));
		memset(destination, 0, sizeof(destination));
		memset(s_port, 0, sizeof(s_port));
		memset(d_port, 0, sizeof(d_port));
		memset(s_ifindex, 0, sizeof(s_ifindex));
		memset(d_ifindex, 0, sizeof(d_ifindex));
		memset(s_mac, 0, sizeof(s_mac));
		memset(d_mac, 0, sizeof(d_mac));

		sprintf(id, "%d", rule->ID);
		sprintf(type, "%s", firewall_type_string(rule->type));
		sprintf(action, "%s", firewall_action_string(rule->action));
		sprintf(proto, "%s", firewall_proto_string(rule->proto));

		if (rule->source.family)
		{
			char tmp[64];
			union prefix46constptr pa;
			pa.p = &rule->source;
			//sprintf(source, "%s", prefix2str(pa, tmp, sizeof(tmp)));
			if(rule->source.prefixlen == 0 ||
					rule->source.prefixlen == IPV4_MAX_PREFIXLEN ||
					rule->source.prefixlen == IPV6_MAX_PREFIXLEN)
				sprintf(source, "%s", prefix_2_address_str(pa, tmp, sizeof(tmp)));
			else
				sprintf(source, "%s", prefix2str(pa, tmp, sizeof(tmp)));
		}
		if (rule->destination.family)
		{
			char tmp[64];
			union prefix46constptr pa;
			pa.p = &rule->destination;
			if(rule->destination.prefixlen == 0 ||
					rule->destination.prefixlen == IPV4_MAX_PREFIXLEN ||
					rule->destination.prefixlen == IPV6_MAX_PREFIXLEN)
				sprintf(destination, "%s", prefix_2_address_str(pa, tmp, sizeof(tmp)));
			else
				sprintf(destination, "%s", prefix2str(pa, tmp, sizeof(tmp)));
		}

		if (rule->s_port)
			sprintf(s_port, "%d", rule->s_port);
		if (rule->d_port)
			sprintf(d_port, "%d", rule->d_port);
		if (rule->s_ifindex)
			sprintf(s_ifindex, "%s", ifindex2ifname(rule->s_ifindex));
		if (rule->d_ifindex)
			sprintf(d_ifindex, "%s", ifindex2ifname(rule->d_ifindex));

		if (!str_isempty(rule->s_mac, sizeof(rule->s_mac)))
			sprintf(s_mac, "%s", inet_ethernet(rule->s_mac));

		if (!str_isempty(rule->d_mac, sizeof(rule->d_mac)))
			sprintf(d_mac, "%s", inet_ethernet(rule->d_mac));

		if(wp->iValue)
			websWrite(wp, ",");
		if(rule->class == FIREWALL_C_PORT)
		{
		websWrite(wp,
				"{\"ID\":\"%s\", \"name\":\"%s\", \"lanip\":\"%s\", \"lanport\":\"%s\","
			"\"wanip\":\"%s\", \"wanport\":\"%s\", \"proto\":\"%s\", \"action\":\"%s\", \"outif\":\"%s\","
			"\"inif\":\"%s\",\"smac\":\"%s\", \"dmac\":\"%s\"}",
				id, rule->name, destination, d_port, source, s_port, proto, action,
				s_ifindex, d_ifindex, s_mac, d_mac);
		}
		else if(rule->class == FIREWALL_C_FILTER)
		{
		websWrite(wp,
				"{\"ID\":\"%s\", \"name\":\"%s\", \"lanip\":\"%s\", \"lanport\":\"%s\","
			"\"wanip\":\"%s\", \"wanport\":\"%s\", \"proto\":\"%s\", \"action\":\"%s\", \"outif\":\"%s\","
			"\"inif\":\"%s\",\"smac\":\"%s\", \"dmac\":\"%s\"}",
				id, rule->name, destination, s_port, source, d_port, proto, action,
				s_ifindex, d_ifindex, s_mac, d_mac);
		}
		else if(rule->class == FIREWALL_C_SNAT)
		{
		websWrite(wp,
				"{\"ID\":\"%s\", \"name\":\"%s\", \"lanip\":\"%s\", \"lanport\":\"%s\","
			"\"wanip\":\"%s\", \"wanport\":\"%s\", \"proto\":\"%s\", \"action\":\"%s\", \"outif\":\"%s\","
			"\"inif\":\"%s\",\"smac\":\"%s\", \"dmac\":\"%s\"}",
				id, rule->name, source, s_port, destination, d_port, proto, action,
				d_ifindex, s_ifindex, s_mac, d_mac);
		}
		wp->iValue++;
	}
	return OK;
}


static int web_firewall_zone_foreach_one(firewall_zone_t *zone, Webs *wp)
{
	return firewall_rule_foreach_api(zone, web_firewall_rule_forwards_one, wp);
}

static int web_firewall_port_map_rule_tbl(Webs *wp, char *path, char *query)
{
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
	wp->iValue1 = FIREWALL_C_PORT;

	firewall_zone_foreach_api(web_firewall_zone_foreach_one, wp);
	wp->iValue = 0;
	wp->iValue1 = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	_WEB_DBG_TRAP("--------------%s-----------------\r\n", __func__);
	return 0;
}

static int web_firewall_port_filter_rule_tbl(Webs *wp, char *path, char *query)
{
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
	wp->iValue1 = FIREWALL_C_FILTER;
	firewall_zone_foreach_api(web_firewall_zone_foreach_one, wp);
	wp->iValue = 0;
	wp->iValue1 = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	_WEB_DBG_TRAP("--------------%s-----------------\r\n", __func__);
	return 0;
}


static int web_firewall_snat_rule_tbl(Webs *wp, char *path, char *query)
{
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
	wp->iValue1 = FIREWALL_C_SNAT;
	firewall_zone_foreach_api(web_firewall_zone_foreach_one, wp);
	wp->iValue = 0;
	wp->iValue1 = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	_WEB_DBG_TRAP("--------------%s-----------------\r\n", __func__);
	return 0;
}


static int web_firewall_port_map_rule_handle(Webs *wp, void *p)
{
	firewall_zone_t * zone = NULL;
	firewall_t rule;
	char *name = NULL;
	char *lanip = NULL;
	char *lanport = NULL;
	char *wanip = NULL;
	char *wanport = NULL;
	char *proto = NULL;
	char *strval = webs_get_var(wp, T("BTNID"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get BTNID Value");
		return ERROR;
	}
	if(strstr(strval, "delete"))
	{
		name = webs_get_var(wp, T("name"), T(""));
		lanip = webs_get_var(wp, T("lanip"), T(""));
		lanport = webs_get_var(wp, T("lanport"), T(""));
		wanip = webs_get_var(wp, T("wanip"), T(""));
		wanport = webs_get_var(wp, T("wanport"), T(""));

		proto = webs_get_var(wp, T("proto"), T(""));

		if(!name || !wanport /*|| !lanport || !lanip || !proto*/)
			return ERROR;

		zone = nsm_firewall_zone_lookup("zones");
		if(!zone)
			return ERROR;

		if(zone)
		{
			memset(&rule, 0, sizeof(firewall_t));

			rule.family = AF_INET;
			strcpy(rule.name, name);

			rule.class = FIREWALL_C_PORT;
			rule.type = FIREWALL_NAT_PREROUTING;
			rule.action = FIREWALL_A_DNAT;
			rule.proto = FIREWALL_P_ALL;
/*
			if(strstr(proto, "ALL"))
				rule.proto = FIREWALL_P_ALL;
			else if(strstr(proto, "TCP"))
				rule.proto = FIREWALL_P_TCP;
			else if(strstr(proto, "UDP"))
				rule.proto = FIREWALL_P_UDP;
*/

			if(wanip)
				str2prefix (wanip, &rule.source);

			if(lanip)
				str2prefix (lanip, &rule.destination);

			if(wanport)
				rule.s_port = atoi(wanport);
			if(lanport)
				rule.d_port = atoi(lanport);
			//iptables -t nat -A PREROUTING -p tcp --dport 80 -j REDIRECT --to-ports 8080

			if(firewall_rule_del_api(zone, &rule) == OK)
				return web_return_text_plain(wp, OK);
			else
				return ERROR;//

		}
	}
	else if(strstr(strval, "add"))
	{
		name = webs_get_var(wp, T("name"), T(""));
		lanip = webs_get_var(wp, T("lanip"), T(""));
		lanport = webs_get_var(wp, T("lanport"), T(""));
		wanip = webs_get_var(wp, T("wanip"), T(""));
		wanport = webs_get_var(wp, T("wanport"), T(""));
		proto = webs_get_var(wp, T("proto"), T(""));

		if(!name || !wanport || !lanport || !lanip || !proto)
			return ERROR;
		zone = nsm_firewall_zone_lookup("zones");
		if(!zone)
			zone = nsm_firewall_zone_add("zones");
		if(zone)
		{
			memset(&rule, 0, sizeof(firewall_t));

			rule.family = AF_INET;
			strcpy(rule.name, name);
			rule.class = FIREWALL_C_PORT;
			rule.type = FIREWALL_NAT_PREROUTING;
			rule.action = FIREWALL_A_DNAT;
			rule.proto = FIREWALL_P_ALL;
			if(strstr(proto, "ALL"))
				rule.proto = FIREWALL_P_ALL;
			else if(strstr(proto, "TCP"))
				rule.proto = FIREWALL_P_TCP;
			else if(strstr(proto, "UDP"))
				rule.proto = FIREWALL_P_UDP;

			if(wanip)
				str2prefix (wanip, &rule.source);

			if(lanip)
				str2prefix (lanip, &rule.destination);

			if(wanport)
				rule.s_port = atoi(wanport);
			if(lanport)
				rule.d_port = atoi(lanport);

			if(firewall_rule_lookup_api(zone, &rule))
				return ERROR;
			if(firewall_rule_add_api(zone, &rule) == OK)
				return web_return_text_plain(wp, OK);
			else
				return ERROR;//
		}
	}
	return ERROR;
}


static int web_firewall_port_filter_handle(Webs *wp, void *p)
{
	firewall_zone_t * zone = NULL;
	firewall_t rule;
	char *name = NULL;
	char *lanip = NULL;
	char *lanport = NULL;
	char *wanip = NULL;
	char *wanport = NULL;
	char *proto = NULL;
	char *strval = webs_get_var(wp, T("BTNID"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get BTNID Value");
		return ERROR;
	}
	if(strstr(strval, "delete"))
	{
		name = webs_get_var(wp, T("name"), T(""));
		wanport = webs_get_var(wp, T("wanport"), T(""));

		lanip = webs_get_var(wp, T("lanip"), T(""));
		lanport = webs_get_var(wp, T("lanport"), T(""));
		wanip = webs_get_var(wp, T("wanip"), T(""));
		wanport = webs_get_var(wp, T("wanport"), T(""));
		proto = webs_get_var(wp, T("proto"), T(""));

		strval = webs_get_var(wp, T("handle"), T(""));

		if(!name || !wanport || !proto/*|| !lanport || !lanip */)
			return ERROR;

		zone = nsm_firewall_zone_lookup("zones");
		if(!zone)
			return ERROR;

		if(zone)
		{
			memset(&rule, 0, sizeof(firewall_t));

			rule.family = AF_INET;
			strcpy(rule.name, name);

			rule.class = FIREWALL_C_FILTER;
			rule.type = FIREWALL_FILTER_INPUT;
			if(strstr(strval, "disable"))
				rule.action = FIREWALL_A_DROP;
			else if(strstr(strval, "enable"))
				rule.action = FIREWALL_A_ACCEPT;

			//rule.action = FIREWALL_A_SNAT;
			rule.proto = FIREWALL_P_ALL;
			if(strstr(proto, "ALL"))
				rule.proto = FIREWALL_P_ALL;
			else if(strstr(proto, "TCP"))
				rule.proto = FIREWALL_P_TCP;
			else if(strstr(proto, "UDP"))
				rule.proto = FIREWALL_P_UDP;
/*
			if(strstr(proto, "ALL"))
				rule.proto = FIREWALL_P_ALL;
			else if(strstr(proto, "TCP"))
				rule.proto = FIREWALL_P_TCP;
			else if(strstr(proto, "UDP"))
				rule.proto = FIREWALL_P_UDP;
			if(lanip)
				str2prefix (lanip, &rule.destination);
*/
			if(wanip)
				str2prefix (wanip, &rule.source);
			if(wanport)
				rule.d_port = atoi(wanport);
			//if(lanport)
			//	rule.d_port = atoi(lanport);
			if(firewall_rule_del_api(zone, &rule) == OK)
				return web_return_text_plain(wp, OK);
			else
				return ERROR;//
			//return firewall_rule_del_api(zone, &rule);
		}
	}
	else if(strstr(strval, "add"))
	{
		name = webs_get_var(wp, T("name"), T(""));
		lanip = webs_get_var(wp, T("lanip"), T(""));
		lanport = webs_get_var(wp, T("lanport"), T(""));
		wanip = webs_get_var(wp, T("wanip"), T(""));
		wanport = webs_get_var(wp, T("wanport"), T(""));
		proto = webs_get_var(wp, T("proto"), T(""));

		strval = webs_get_var(wp, T("handle"), T(""));

		if(!name || !wanport || !strval/*|| !lanport || !lanip || !proto*/)
			return ERROR;
		zone = nsm_firewall_zone_lookup("zones");
		if(!zone)
			zone = nsm_firewall_zone_add("zones");
		if(zone)
		{
			memset(&rule, 0, sizeof(firewall_t));

			rule.family = AF_INET;
			strcpy(rule.name, name);
			rule.class = FIREWALL_C_FILTER;
			rule.type = FIREWALL_FILTER_INPUT;
			if(strstr(strval, "disable"))
				rule.action = FIREWALL_A_DROP;
			else if(strstr(strval, "enable"))
				rule.action = FIREWALL_A_ACCEPT;

			//rule.action = FIREWALL_A_SNAT;
			rule.proto = FIREWALL_P_ALL;
			if(strstr(proto, "ALL"))
				rule.proto = FIREWALL_P_ALL;
			else if(strstr(proto, "TCP"))
				rule.proto = FIREWALL_P_TCP;
			else if(strstr(proto, "UDP"))
				rule.proto = FIREWALL_P_UDP;

			if(wanip)
				str2prefix (wanip, &rule.source);
			/*
			if(lanip)
				str2prefix (lanip, &rule.destination);
*/

			if(wanport)
				rule.s_port = atoi(wanport);
			//if(lanport)
			//	rule.d_port = atoi(lanport);

			if(firewall_rule_lookup_api(zone, &rule))
				return ERROR;

			if(firewall_rule_add_api(zone, &rule) == OK)
				return web_return_text_plain(wp, OK);
			else
				return ERROR;//
			//return firewall_rule_add_api(zone, &rule);
		}
	}
	return ERROR;
}


static int web_firewall_snat_handle(Webs *wp, void *p)
{
	firewall_zone_t * zone = NULL;
	firewall_t rule;
	char *name = NULL;
	char *lanip = NULL;
	char *lanport = NULL;
	char *wanip = NULL;
	char *wanport = NULL;
	char *proto = NULL;
	char *strval = webs_get_var(wp, T("BTNID"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get BTNID Value");
		return ERROR;
	}
	if(strstr(strval, "delete"))
	{
		name = webs_get_var(wp, T("name"), T(""));
		wanport = webs_get_var(wp, T("wanport"), T(""));
/*
		lanip = webs_get_var(wp, T("lanip"), T(""));
		lanport = webs_get_var(wp, T("lanport"), T(""));
		wanip = webs_get_var(wp, T("wanip"), T(""));
		wanport = webs_get_var(wp, T("wanport"), T(""));
		proto = webs_get_var(wp, T("proto"), T(""));
*/

		if(!name /*|| !wanport || !lanport || !lanip || !proto*/)
			return ERROR;

		zone = nsm_firewall_zone_lookup("zones");
		if(!zone)
			return ERROR;

		if(zone)
		{
			memset(&rule, 0, sizeof(firewall_t));

			rule.family = AF_INET;
			strcpy(rule.name, name);

			rule.class = FIREWALL_C_SNAT;
			rule.type = FIREWALL_NAT_POSTROUTING;
			rule.action = FIREWALL_A_SNAT;
			rule.proto = FIREWALL_P_ALL;
/*
			if(strstr(proto, "ALL"))
				rule.proto = FIREWALL_P_ALL;
			else if(strstr(proto, "TCP"))
				rule.proto = FIREWALL_P_TCP;
			else if(strstr(proto, "UDP"))
				rule.proto = FIREWALL_P_UDP;
			if(wanip)
				str2prefix (wanip, &rule.source);
			if(lanip)
				str2prefix (lanip, &rule.destination);
*/

			if(wanport)
				rule.d_port = atoi(wanport);
			//if(lanport)
			//	rule.d_port = atoi(lanport);
			if(firewall_rule_del_api(zone, &rule) == OK)
				return web_return_text_plain(wp, OK);
			else
				return ERROR;//
			//return firewall_rule_del_api(zone, &rule);
		}
	}
	else if(strstr(strval, "add"))
	{
		char *outif = webs_get_var(wp, T("outif"), T(""));
		name = webs_get_var(wp, T("name"), T(""));
		lanip = webs_get_var(wp, T("lanip"), T(""));
		lanport = webs_get_var(wp, T("lanport"), T(""));
		wanip = webs_get_var(wp, T("wanip"), T(""));
		wanport = webs_get_var(wp, T("wanport"), T(""));
		proto = webs_get_var(wp, T("proto"), T(""));

		if(!name || !lanip/*|| !wanport || !lanport  || !proto*/)
			return ERROR;
		zone = nsm_firewall_zone_lookup("zones");
		if(!zone)
			zone = nsm_firewall_zone_add("zones");
		if(zone)
		{
			memset(&rule, 0, sizeof(firewall_t));

			rule.family = AF_INET;
			strcpy(rule.name, name);
			rule.class = FIREWALL_C_SNAT;
			rule.type = FIREWALL_NAT_POSTROUTING;
			rule.action = FIREWALL_A_SNAT;

			rule.proto = FIREWALL_P_ALL;
/*
			if(strstr(proto, "ALL"))
				rule.proto = FIREWALL_P_ALL;
			else if(strstr(proto, "TCP"))
				rule.proto = FIREWALL_P_TCP;
			else if(strstr(proto, "UDP"))
				rule.proto = FIREWALL_P_UDP;
*/
			if(outif)
				rule.d_ifindex = ifname2ifindex(outif);

			if(lanip )
				str2prefix (lanip, &rule.source);

			if(wanip)
				str2prefix (wanip, &rule.destination);


			if(wanport)
				rule.d_port = atoi(wanport);
			//if(lanport)
			//	rule.d_port = atoi(lanport);

			if(firewall_rule_lookup_api(zone, &rule))
				return ERROR;

			if(firewall_rule_add_api(zone, &rule) == OK)
				return web_return_text_plain(wp, OK);
			else
				return ERROR;//
			//return firewall_rule_add_api(zone, &rule);
		}
	}
	return ERROR;
}
#endif /* THEME_V9UI */

int web_firewall_jst_init(void)
{
#ifndef THEME_V9UI
	//websDefineJst("jst_port_connect", jst_port_connect);
	websFormDefine("port-map", web_firewall_port_map_rule_tbl);
	web_button_add_hook("firewall-map", "add", web_firewall_port_map_rule_handle, NULL);
	web_button_add_hook("firewall-map", "delete", web_firewall_port_map_rule_handle, NULL);


	websFormDefine("port-filter", web_firewall_port_filter_rule_tbl);
	web_button_add_hook("firewall-filter", "add", web_firewall_port_filter_handle, NULL);
	web_button_add_hook("firewall-filter", "delete", web_firewall_port_filter_handle, NULL);

	websFormDefine("snatget", web_firewall_snat_rule_tbl);
	web_button_add_hook("snat", "add", web_firewall_snat_handle, NULL);
	web_button_add_hook("snat", "delete", web_firewall_snat_handle, NULL);
#endif /* THEME_V9UI */
	return 0;
}


