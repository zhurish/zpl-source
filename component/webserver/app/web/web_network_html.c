/*
 * web_network_html.c
 *
 *  Created on: 2019年8月10日
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
#include "vrf.h"
#include "interface.h"
#include "nsm_dhcp.h"

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

#ifdef WEB_OPENWRT_PROCESS
static int web_networkset_set_active(void *a)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
#ifdef PL_WIFI_MODULE
	struct interface *ifp = NULL;
	ifp = if_lookup_by_name("wireless 0/0/1");
	if(ifp)
	{
		os_sleep(1);
		nsm_iw_enable_api(ifp, FALSE);
	}
#endif
#else
	os_sleep(1);
	super_system("reboot -f");
	//super_system("/etc/init.d/network restart");
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 14, 0)
#ifdef PL_WIFI_MODULE
	if(ifp)
	{
		os_sleep(2);
		nsm_iw_enable_api(ifp, TRUE);
	}
#endif
#endif	
	return OK;
}
#endif

static int web_networkset_set(Webs *wp, char *path, char *query)
{
	char *ifname = NULL;
	char *proto = NULL;
	//char *ifname = NULL;
	web_assert(wp);
	struct interface *ifp = NULL;
	ifname = webs_get_var(wp, T("ifname"), T(""));
	if (NULL == ifname)
	{
		_WEB_DBG_TRAP("================%s=======================:ifname:%s\r\n", __func__, websGetVar(wp, T("ifname"), NULL));
		//printf("================%s=======================:ifname\r\n", __func__);
		return web_return_text_plain(wp, ERROR);
	}
	if(web_type_get() == WEB_TYPE_HOME_WIFI)
	{
		_WEB_DBG_TRAP("================%s=======================:WEB_TYPE_HOME_WIFI\r\n", __func__);
#ifdef APP_V9_MODULE
		if(strstr(ifname, "wan2"))
			ifp = if_lookup_by_name("ethernet 0/0/3");
		else if(strstr(ifname, "wan"))
			ifp = if_lookup_by_name("ethernet 0/0/2");
#else
		if(strstr(ifname, "wan"))
			ifp = if_lookup_by_name("ethernet 0/0/2");
#endif
		else if(strstr(ifname, "lan"))
			ifp = if_lookup_by_name("brigde 0/0/1");
	}
	else
	{
		_WEB_DBG_TRAP("================%s=======================:%d\r\n", __func__, web_type_get());
		ifp = if_lookup_by_name(ifname);
	}
	if(!ifp)
	{
		_WEB_DBG_TRAP("================%s=======================:if_lookup_by_name\r\n", __func__);
		return web_return_text_plain(wp, ERROR);
	}
	proto = webs_get_var(wp, T("proto"), T(""));
	if (NULL == proto)
	{
		_WEB_DBG_TRAP("================%s=======================:proto\r\n", __func__);
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(proto, "dhcp"))
	{
#ifdef PL_DHCP_MODULE
		if(nsm_interface_dhcp_mode_get_api(ifp) == DHCP_CLIENT)
		{
			return web_return_text_plain(wp, OK);
		}
		if(nsm_interface_dhcp_mode_set_api(ifp, DHCP_CLIENT, NULL) == OK)
			return web_return_text_plain(wp, OK);
#endif
#ifdef WEB_OPENWRT_PROCESS
		if(ifp->ifindex ==ifname2ifindex("ethernet 0/0/2"))
		{
			os_uci_set_string("network.wan.proto", "dhcp");
			os_uci_del("network", "wan", "ipaddr", NULL);
			os_uci_del("network", "wan", "netmask", NULL);
			os_uci_del("network", "wan", "gateway", NULL);
			os_uci_del("network", "wan", "dns", NULL);
		}
#ifdef APP_V9_MODULE
		else if(ifp->ifindex ==ifname2ifindex("ethernet 0/0/3"))
		{
			os_uci_set_string("network.wan2.proto", "dhcp");
			os_uci_del("network", "wan2", "ipaddr", NULL);
			os_uci_del("network", "wan2", "netmask", NULL);
			os_uci_del("network", "wan2", "gateway", NULL);
			os_uci_del("network", "wan2", "dns", NULL);
		}
#endif
		else if(ifp->ifindex ==ifname2ifindex("brigde 0/0/1"))
		{
			os_uci_set_string("network.lan.proto", "dhcp");
			os_uci_del("network", "lan", "ipaddr", NULL);
			os_uci_del("network", "lan", "netmask", NULL);
			os_uci_del("network", "lan", "gateway", NULL);
			os_uci_del("network", "lan", "dns", NULL);
		}

		os_uci_save_config("network");
		os_job_add(web_networkset_set_active, NULL);
		//super_system("/etc/init.d/network restart");
		return web_return_text_plain(wp, OK);
#endif
		return web_return_text_plain(wp, ERROR);
	}
	else if(strstr(proto, "pppoe"))
	{
		char *username = NULL;
		char *password = NULL;
		username = webs_get_var(wp, T("username"), T(""));
		if (NULL == username)
		{
			return web_return_text_plain(wp, ERROR);
		}
		password = webs_get_var(wp, T("password"), T(""));
		if (NULL == password)
		{
			return web_return_text_plain(wp, ERROR);
		}
		return web_return_text_plain(wp, OK);
		//argv += "&username=" + document.getElementById("username").value +
		//	"&password=" + document.getElementById("password").value;
	}
	else if(strstr(proto, "static"))
	{
		char *ipaddress = NULL;
		char *netmask = NULL;
		char *gateway = NULL;
		char *dns = NULL;
		char *dns2 = NULL;
		struct prefix cp, ocp;
		char addressbuf[64];
		ipaddress = webs_get_var(wp, T("ipaddress"), T(""));
		if (NULL == ipaddress)
		{
			_WEB_DBG_TRAP("================%s=======================:ipaddress\r\n", __func__);
			return web_return_text_plain(wp, ERROR);
		}
		netmask = webs_get_var(wp, T("netmask"), T(""));
		if (NULL == netmask)
		{
			_WEB_DBG_TRAP("================%s=======================:netmask\r\n", __func__);
			return web_return_text_plain(wp, ERROR);
		}

		gateway = webs_get_var(wp, T("gateway"), T(""));
		dns = webs_get_var(wp, T("dns"), T(""));
		dns2 = webs_get_var(wp, T("dns2"), T(""));

		_WEB_DBG_TRAP("================ipaddress=======================:%s\r\n", ipaddress);
		_WEB_DBG_TRAP("================netmask=======================:%s\r\n", netmask);
		if(gateway)
		{
			_WEB_DBG_TRAP("================gateway=======================:%s\r\n", gateway);
		}
		if(dns)
		{
			_WEB_DBG_TRAP("================dns=======================:%s\r\n", dns);
		}

#ifdef WEB_OPENWRT_PROCESS
		if(ifp->ifindex ==ifname2ifindex("ethernet 0/0/2"))
		{
			os_uci_del("network", "wan", "gateway", NULL);
			os_uci_del("network", "wan", "dns", NULL);
			os_uci_set_string("network.wan.proto", "static");
			os_uci_set_string("network.wan.ipaddr", ipaddress);
			os_uci_set_string("network.wan.netmask", netmask);
			if(gateway)
				os_uci_set_string("network.wan.gateway", gateway);
			if(dns)
				os_uci_set_string("network.wan.dns", dns);
		}
#ifdef APP_V9_MODULE
		else if(ifp->ifindex ==ifname2ifindex("ethernet 0/0/3"))
		{
			os_uci_del("network", "wan2", "gateway", NULL);
			os_uci_del("network", "wan2", "dns", NULL);
			os_uci_set_string("network.wan2.proto", "static");
			os_uci_set_string("network.wan2.ipaddr", ipaddress);
			os_uci_set_string("network.wan2.netmask", netmask);
			if(gateway)
				os_uci_set_string("network.wan2.gateway", gateway);
			if(dns)
				os_uci_set_string("network.wan2.dns", dns);
		}
#endif
		else if(ifp->ifindex ==ifname2ifindex("brigde 0/0/1"))
		{
			os_uci_del("network", "lan", "gateway", NULL);
			os_uci_del("network", "lan", "dns", NULL);
			os_uci_set_string("network.lan.proto", "static");
			os_uci_set_string("network.lan.ipaddr", ipaddress);
			os_uci_set_string("network.lan.netmask", netmask);
			if(gateway)
				os_uci_set_string("network.lan.gateway", gateway);
			if(dns)
				os_uci_set_string("network.lan.dns", dns);

		}
		os_uci_save_config("network");
		os_job_add(web_networkset_set_active, NULL);
		//super_system("/etc/init.d/network restart");
		return web_return_text_plain(wp, OK);
#endif

		memset(addressbuf, 0, sizeof(addressbuf));
		//memset(addressbuf, "%s/%d", ipaddress);
		netmask_str2prefix_str(ipaddress, netmask, addressbuf);

		if(str2prefix_ipv4 (addressbuf, (struct prefix_ipv4 *)&cp) <= 0)
		{
			_WEB_DBG_TRAP("================%s=======================:str2prefix_ipv4:%s\r\n", __func__, addressbuf);
			return web_return_text_plain(wp, ERROR);
		}
		nsm_interface_address_get_api(ifp, &ocp);
		nsm_interface_address_unset_api(ifp, &ocp, FALSE);
		if(nsm_interface_address_set_api(ifp, &cp, FALSE) != OK)
		{
			_WEB_DBG_TRAP("================%s=======================:nsm_interface_address_set_api\r\n", __func__);
			return web_return_text_plain(wp, ERROR);
		}

		if(dns)
		{
			if (str2prefix (dns, &cp) <= 0)
			{
				_WEB_DBG_TRAP("================%s=======================:dns str2prefix:%s\r\n", __func__, dns);
				return web_return_text_plain(wp, ERROR);
			}
			if(nsm_ip_dns_add_api(&cp, FALSE) != OK)
			{
				_WEB_DBG_TRAP("================%s=======================:dns\r\n", __func__);
				return web_return_text_plain(wp, ERROR);
			}
		}
		if(dns2)
		{
			if (str2prefix (dns2, &cp) <= 0)
			{
				_WEB_DBG_TRAP("================%s=======================:dns2 str2prefix:%s\r\n", __func__,dns2);
				return web_return_text_plain(wp, ERROR);
			}
			if(nsm_ip_dns_add_api(&cp, TRUE) != OK)
			{
				_WEB_DBG_TRAP("================%s=======================:dns2\r\n", __func__);
				return web_return_text_plain(wp, ERROR);
			}
		}

		if(gateway)
		{
			_WEB_DBG_TRAP("================%s=======================:gateway=%s\r\n", __func__, gateway);
			if(web_static_ipv4_safi (SAFI_UNICAST, 1, "0.0.0.0", NULL,
				  gateway, NULL, NULL, NULL, NULL) != NULL)
			{
				_WEB_DBG_TRAP("================%s=======================:web_static_ipv4_safi\r\n", __func__);
				return web_return_text_plain(wp, ERROR);
			}
		}
		return web_return_text_plain(wp, OK);
	}
	else
		return web_return_text_plain(wp, ERROR);
}

int web_network_app(void)
{
	websFormDefine("networkset", web_networkset_set);
	return 0;
}

