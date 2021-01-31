/*
 * web_interface_jst.c
 *
 *  Created on: 2019年8月3日
 *      Author: zhurish
 */

#include "zebra.h"
#include "zassert.h"
#include "vty.h"
#include "if.h"
#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "nsm_vrf.h"
#include "nsm_interface.h"
#ifdef PL_WIFI_MODULE
#include "iw_config.h"
#include "iw_ap.h"
#include "iw_client.h"
#include "iw_interface.h"
#endif

#include "nsm_dhcp.h"


#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

#ifndef THEME_V9UI
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




static int web_interface_name_list(struct interface *ifp, void *pVoid)
{
	webs_t wp = pVoid;
	if (if_is_loop (ifp))
		return OK;
	else if (if_is_tunnel (ifp))
		return OK;
	else if (if_is_lag_member (ifp))
		return OK;
	else if (if_is_brigde_member (ifp))
		return OK;

	if (web_type_get () == WEB_TYPE_HOME_WIFI)
	{
		if (ifp->ifindex != if_ifindex_make ("ethernet 0/0/2", NULL)
#ifdef APP_V9_MODULE
				&& ifp->ifindex != if_ifindex_make ("ethernet 0/0/3", NULL)
#endif
				&& ifp->ifindex != if_ifindex_make ("brigde 0/0/1", NULL))
			return OK;
	}
	if (web_type_get () == WEB_TYPE_HOME_WIFI)
	{
		if (ifp->ifindex == ifname2ifindex ("ethernet 0/0/2"))
			websWrite (wp, "<option value=\"%s\">%s</option>", "wan",
					   "wan");
#ifdef APP_V9_MODULE
		else if (ifp->ifindex == ifname2ifindex ("ethernet 0/0/3"))
			websWrite (wp, "<option value=\"%s\">%s</option>", "wan2",
					   "wan2");
#endif
#ifndef APP_V9_MODULE
		else if (ifp->ifindex == ifname2ifindex ("brigde 0/0/1"))
			websWrite (wp, "<option value=\"%s\">%s</option>", "lan",
					   "lan");
#endif
	}
	else
		websWrite (wp, "<option value=\"%s\">%s</option>", ifp->name,
				   ifp->name);
	return OK;
}

static int jst_interface_list(int eid, webs_t wp, int argc, char **argv)
{
	wp->iValue1 = 0;
	wp->iValue = 0;
	if_list_each(web_interface_name_list, wp);
	wp->iValue = 0;
	wp->iValue1 = 0;
	websDone(wp);
	return 0;
}


static int web_interface_name_tbl(struct interface *ifp, void *pVoid)
{
	webs_t wp = pVoid;
	if (if_is_loop (ifp))
		return OK;
	else if (if_is_tunnel (ifp))
		return OK;
	else if (if_is_lag_member (ifp))
		return OK;
	else if (if_is_brigde_member (ifp))
		return OK;

	if (web_type_get () == WEB_TYPE_HOME_WIFI)
	{
		if (ifp->ifindex == if_ifindex_make ("ethernet 0/0/2", NULL))
		{
			if (wp->iValue)
				websWrite (wp, "%s", ",\n");
			websWrite (wp, "{\"name\":\"%s\", \"state\":\"%s\"}", "wan",
					   "up");
			wp->iValue++;
		}
#ifdef APP_V9_MODULE
		else if (ifp->ifindex == if_ifindex_make ("ethernet 0/0/3", NULL))
		{
			if (wp->iValue)
				websWrite (wp, "%s", ",\n");
			websWrite (wp, "{\"name\":\"%s\", \"state\":\"%s\"}", "wan2",
					   "up");
			wp->iValue++;
		}
#endif
#ifndef APP_V9_MODULE
		else if (ifp->ifindex == if_ifindex_make ("brigde 0/0/1", NULL))
		{
			if (wp->iValue)
				websWrite (wp, "%s", ",\n");
			websWrite (wp, "{\"name\":\"%s\", \"state\":\"%s\"}", "lan",
					   "up");
			wp->iValue++;
		}
#endif
	}
	else
	{
		if (wp->iValue)
			websWrite (wp, "%s", ",\n");
		websWrite (wp, "{\"name\":\"%s\", \"state\":\"%s\"}", ifp->name,
				   "up");
		wp->iValue++;
	}
	return OK;
}

static int jst_interface_tbl(Webs *wp, char *path, char *query)
{
	wp->iValue1 = 0;
	wp->iValue = 0;
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	//websWriteHeader(wp, "Content-Type", "application/json"/*"text/plain"*/);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");

	if_list_each(web_interface_name_tbl, wp);
	websWrite(wp, "%s", "]");
	websDone(wp);
	wp->iValue = 0;
	wp->iValue1 = 0;
	return 0;
}
#endif /* THEME_V9UI */

static int web_network_workmode(struct interface *ifp, char *workmode, char *ifname_str)
{
	if (if_is_wireless (ifp))
	{
#ifdef PL_WIFI_MODULE
		if(nsm_iw_mode(ifp) == IW_MODE_MANAGE)
		sprintf(workmode, "wireless-sta");
		else if(nsm_iw_mode(ifp) == IW_MODE_AP)
		sprintf(workmode, "wireless-ap");
#else
		sprintf (workmode, "wireless");
#endif
	}
	else if (if_is_serial (ifp))
	{
		sprintf (workmode, "serial");
	}
	else if (if_is_lag_member (ifp))
	{
		sprintf (workmode, "lag_member");
	}
	else if (if_is_lag (ifp))
	{
		sprintf (workmode, "lag");
	}
	else if (if_is_tunnel (ifp))
	{
		sprintf (workmode, "tunnel");
	}
	else if (if_is_vlan (ifp))
	{
		sprintf (workmode, "vlan");
	}
	else if (if_is_brigde (ifp))
	{
		sprintf (workmode, "brigde");
	}
	else if (if_is_brigde_member (ifp))
	{
		sprintf (workmode, "brigde_member");
	}
	else
		sprintf (workmode, "none");

	if (web_type_get () == WEB_TYPE_HOME_WIFI)
	{
		if (ifp->ifindex
				== if_ifindex_make ("ethernet 0/0/2", NULL))
		{
			//memset (ifname_str, 0, sizeof(ifname_str));
			sprintf (ifname_str, "%s", "wan");
		}
#ifdef APP_V9_MODULE
		else if (ifp->ifindex
				== if_ifindex_make ("ethernet 0/0/3", NULL))
		{
			//memset (ifname_str, 0, sizeof(ifname_str));
			sprintf (ifname_str, "%s", "wan2");
		}
#endif
#ifndef APP_V9_MODULE
		else if (ifp->ifindex
				== if_ifindex_make ("brigde 0/0/1", NULL))
		{
			//memset (ifname_str, 0, sizeof(ifname_str));
			sprintf (ifname_str, "%s", "lan");
		}
#endif
	}
	else
	{
		//memset (ifname_str, 0, sizeof(ifname_str));
		sprintf (ifname_str, "%s", ifp->name);
	}
	return OK;
}

static int web_network_address_netmask(struct interface *ifp, char *address_str, char *netmask_str)
{
	struct connected *ifc = NULL;
	struct prefix *p = NULL;
	struct in_addr netmask;
	ifc = (struct connected *)listnode_head(ifp->connected);
	if(ifc && ifc->address)
	{
		p = ifc->address;
	}
	else
		p = NULL;
	if(p)
	{
		sprintf (address_str, "%s", inet_address (ntohl (p->u.prefix4.s_addr)));
		masklen2ip (p->prefixlen, &netmask);
		sprintf (netmask_str, "%s",
				 inet_address (ntohl (netmask.s_addr)));
	}
	else
	{
		sprintf (address_str, "%s", " ");
		sprintf (netmask_str, "%s", " ");
	}
	return OK;
}

static int web_network_gateway_dns(struct interface *ifp, char *gateway_str, char *dns_str)
{
	u_int32 local_gateway;
#ifdef WEB_OPENWRT_PROCESS
	ifindex_t ifindex;
	u_int32 dns1 = 0, dns2 = 0;
	if (web_kernel_route_lookup_default (ifp->ifindex, &local_gateway) == OK)
#else
	struct prefix dns;
	if (web_route_lookup_default (ifp->ifindex, &local_gateway)
			== OK)
#endif
	{
		sprintf (gateway_str, "%s", inet_address (local_gateway));
	}
	else
		sprintf (gateway_str, "%s", " ");
#ifdef WEB_OPENWRT_PROCESS
	if(web_kernel_dns_lookup_default(&ifindex, &dns1, &dns2) == OK && ifindex == ifp->ifindex)
	{
		sprintf (dns_str, "%s", inet_address (dns1));
	}
	else
		sprintf (dns_str, "%s", " ");
#else
	//memset (dns_str, 0, sizeof(dns_str));
	if (nsm_ip_dns_get_api (ifp->ifindex, &dns, FALSE) == OK)
	{
		union prefix46constptr pu;
		pu.p = &dns;
		prefix_2_address_str (pu, dns_str, sizeof(dns_str));
	}
	else
		sprintf (dns_str, "%s", " ");
#endif
	return OK;
}



static int web_network_list(struct interface *ifp, void *pVoid)
{
	webs_t wp = pVoid;

	char workmode[64];
	char proto[32] = "static";

	char address_str[INET6_ADDRSTRLEN];
	char netmask_str[INET6_ADDRSTRLEN];
	char gateway_str[INET6_ADDRSTRLEN];
	char dns_str[INET6_ADDRSTRLEN];
	char ifname_str[INET6_ADDRSTRLEN];

	if (if_is_loop (ifp))
		return OK;
	else if (if_is_tunnel (ifp))
		return OK;
	else if (if_is_lag_member (ifp))
		return OK;
	else if (if_is_brigde_member (ifp))
		return OK;

	memset (workmode, 0, sizeof(workmode));
	memset (ifname_str, 0, sizeof(ifname_str));
	web_network_workmode(ifp, workmode, ifname_str);

	if (web_type_get () == WEB_TYPE_HOME_WIFI)
	{
#ifdef APP_V9_MODULE
		if (ifp->ifindex != if_ifindex_make ("ethernet 0/0/2", NULL)
				&& ifp->ifindex != if_ifindex_make ("ethernet 0/0/3", NULL)
				&& ifp->ifindex != if_ifindex_make ("brigde 0/0/1", NULL))
			return OK;
#endif
#ifdef APP_X5BA_MODULE
			if (ifp->ifindex != if_ifindex_make ("ethernet 0/0/2", NULL)
					&& ifp->ifindex != if_ifindex_make ("brigde 0/0/1", NULL))
				return OK;
#endif

	}

#ifdef WEB_OPENWRT_PROCESS
	if (web_type_get () == WEB_TYPE_HOME_WIFI)
	{
		memset (address_str, 0, sizeof(address_str));
		memset (netmask_str, 0, sizeof(netmask_str));
		memset (gateway_str, 0, sizeof(gateway_str));
		memset (proto, 0, sizeof(proto));
		memset (dns_str, 0, sizeof(dns_str));

		if (ifp->ifindex == if_ifindex_make ("ethernet 0/0/2", NULL))
		{
			os_uci_get_string ("network.wan.proto", proto);
			if (strlen (proto) && strstr (proto, "static"))
			{
				os_uci_get_string ("network.wan.ipaddr", address_str);
				os_uci_get_string ("network.wan.netmask", netmask_str);
				os_uci_get_string ("network.wan.gateway", gateway_str);
				os_uci_get_string ("network.wan.dns", dns_str);
			}
			else
			{
				web_network_address_netmask(ifp, address_str, netmask_str);
				web_network_gateway_dns(ifp, gateway_str, dns_str);
			}
		}
#ifdef APP_V9_MODULE
		else if (ifp->ifindex == if_ifindex_make ("ethernet 0/0/3", NULL))
		{
			os_uci_get_string ("network.wan2.proto", proto);
			if (strlen (proto) && strstr (proto, "static"))
			{
				os_uci_get_string ("network.wan2.ipaddr", address_str);
				os_uci_get_string ("network.wan2.netmask", netmask_str);
				os_uci_get_string ("network.wan2.gateway", gateway_str);
				os_uci_get_string ("network.wan2.dns", dns_str);
			}
			else
			{
				web_network_address_netmask(ifp, address_str, netmask_str);
				web_network_gateway_dns(ifp, gateway_str, dns_str);
			}
		}
#endif
#ifndef APP_V9_MODULE
		else if (ifp->ifindex == if_ifindex_make ("brigde 0/0/1", NULL))
		{
			os_uci_get_string ("network.lan.proto", proto);
			if (strlen (proto) && strstr (proto, "static"))
			{
				os_uci_get_string ("network.lan.ipaddr", address_str);
				os_uci_get_string ("network.lan.netmask", netmask_str);
				os_uci_get_string ("network.lan.gateway", gateway_str);
				os_uci_get_string ("network.lan.dns", dns_str);
			}
			else
			{
				web_network_address_netmask(ifp, address_str, netmask_str);
				web_network_gateway_dns(ifp, gateway_str, dns_str);
			}
		}
#endif
		if (wp->iValue)
			websWrite (wp, "%s", ",\n");
		websWrite (
				wp,
				"{\"name\":\"%s\", \"ip\":\"%s\", \"netmask\":\"%s\", \"gateway\":\"%s\", \
			\"dns\":\"%s\", \"state\":\"%s\", \"proto\":\"%s\" , \"mode\":\"%s\"}",
				ifname_str,
				address_str,
				netmask_str,
				gateway_str,
				dns_str,
				if_is_up (ifp) ? "up" : "down",
#ifdef PL_DHCP_MODULE
				(ifp->dhcp && nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)? "dhcp":"static",
#else
				proto,
#endif
				workmode);
		wp->iValue++;
		return OK;
	}
#endif

	memset (address_str, 0, sizeof(address_str));
	memset (netmask_str, 0, sizeof(netmask_str));
	memset (gateway_str, 0, sizeof(gateway_str));
	memset (proto, 0, sizeof(proto));
	memset (dns_str, 0, sizeof(dns_str));

	if (ifp->ifindex == if_ifindex_make ("ethernet 0/0/2", NULL))
	{
		web_network_address_netmask(ifp, address_str, netmask_str);
		web_network_gateway_dns(ifp, gateway_str, dns_str);
	}
#ifdef APP_V9_MODULE
	else if (ifp->ifindex == if_ifindex_make ("ethernet 0/0/3", NULL))
	{
		web_network_address_netmask(ifp, address_str, netmask_str);
		web_network_gateway_dns(ifp, gateway_str, dns_str);
	}
#endif
#ifndef APP_V9_MODULE
	else if (ifp->ifindex == if_ifindex_make ("brigde 0/0/1", NULL))
	{
		web_network_address_netmask(ifp, address_str, netmask_str);
		web_network_gateway_dns(ifp, gateway_str, dns_str);
	}
#endif
	if (wp->iValue)
		websWrite (wp, "%s", ",\n");

	websWrite (
			wp,
			"{\"name\":\"%s\", \"ip\":\"%s\", \"netmask\":\"%s\", \"gateway\":\"%s\", \
		\"dns\":\"%s\", \"state\":\"%s\", \"proto\":\"%s\" , \"mode\":\"%s\"}",
			ifname_str,
			address_str,
			netmask_str,
			gateway_str,
			dns_str,
			if_is_up (ifp) ? "up" : "down",
#ifdef PL_DHCP_MODULE
			(ifp->dhcp && nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)? "dhcp":"static",
#else
			proto,
#endif
			workmode);
	wp->iValue++;
	return OK;
}

static int jst_network_tbl(Webs *wp, char *path, char *query)
{
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	//websWriteHeader(wp, "Content-Type", "application/json"/*"text/plain"*/);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
	wp->iValue1 = 0;
	if_list_each(web_network_list, wp);

	websWrite(wp, "%s", "]");
	wp->iValue = 0;
	wp->iValue1 = 0;
	//websWrite(wp, "%s", "OK");
	websDone(wp);
	return 0;
}

#ifndef THEME_V9UI
static int web_network_connect(Webs *wp, void *p)
{
	char *strval = NULL;
	struct interface *ifp = NULL;
	strval = websGetVar(wp, T("ifname"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get ifname Value");
		return ERROR;//web_return_text_plain(wp, ERROR);
	}
	if (strval)
	{
		if(web_type_get() == WEB_TYPE_HOME_WIFI)
		{
			if(strstr(strval, "wan"))
				ifp = if_lookup_by_name("ethernet 0/0/2");
			else if(strstr(strval, "lan"))
				ifp = if_lookup_by_name("brigde 0/0/2");
#ifdef APP_V9_MODULE
			else if(strstr(strval, "wan2"))
				ifp = if_lookup_by_name("ethernet 0/0/3");
#endif

		}
		else
		{
			ifp = if_lookup_by_name(strval);
		}
	}
	if(!ifp)
		return ERROR;//web_return_text_plain(wp, ERROR);
	strval = websGetVar(wp, T("state"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get state Value");
		return ERROR;//web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "up"))
	{
		if(if_up(ifp) == OK)
			return web_return_text_plain(wp, OK);
	}
	else if(strstr(strval, "down"))
	{
		if(if_down(ifp) == OK)
			return web_return_text_plain(wp, OK);
	}
	return ERROR;//web_return_text_plain(wp, ERROR);
}
#endif /* THEME_V9UI */

int web_interface_jst_init(void)
{
#ifndef THEME_V9UI
	websDefineJst("jst_flags", jst_flags);

	websDefineJst("jst_ipv6", jst_ipv6);

	websDefineJst("jst_interface", jst_interface_list);

	websFormDefine("interface-tbl", jst_interface_tbl);
#endif /* THEME_V9UI */

	websFormDefine("network-tbl", jst_network_tbl);
#ifndef THEME_V9UI
	web_button_add_hook("network", "connect", web_network_connect, NULL);
#endif /* THEME_V9UI */
	return 0;
}
