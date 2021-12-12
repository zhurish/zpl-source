
/*
 * web_dhcp_jst.c
 *
 *  Created on: 2019年8月3日
 *      Author: zhurish
 */
#define HAS_BOOL 1

#include <zpl_include.h>
#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "log.h"
#include "nsm_vrf.h"
#include "command.h"
#include "nsm_interface.h"
#include "if_name.h"
#include "nsm_dhcp.h"

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

#ifdef ZPL_DHCP_MODULE

#ifdef WEB_OPENWRT_PROCESS
//#else

#ifdef ZPL_DHCPD_MODULE

#ifdef ZPL_DHCP_MODULE
#include "dhcp_config.h"
#include "dhcpd.h"
#include "dhcp_lease.h"
#endif




static int web_dhcpd_get_config(Webs *wp, const char *poolname)
{
	nsm_dhcps_t * dhcps = nsm_dhcps_lookup_api(poolname);
	if(dhcps)
	{
		char startip[64];
		char endip[64];
		char netmask[64];
		char gateway[64];
		char gateway2[64];
		char dns[64];
		char dns2[64];
		struct prefix 	address;
		union prefix46constptr pu;

		memset(startip, 0, sizeof(startip));
		memset(endip, 0, sizeof(endip));
		memset(netmask, 0, sizeof(netmask));
		memset(dns, 0, sizeof(dns));
		memset(dns2, 0, sizeof(dns2));
		memset(gateway, 0, sizeof(gateway));
		memset(gateway2, 0, sizeof(gateway2));

		if(dhcps->address.prefixlen)
		{
			zpl_uint32 addr = 0;
			struct in_addr innetmask;
			memset(startip, 0, sizeof(startip));
			memcpy(&address, &dhcps->address, sizeof(address));
			addr = ntohl(address.u.prefix4.s_addr);
			if((ntohl(address.u.prefix4.s_addr) & 1) == 0)
			{
				address.u.prefix4.s_addr = htonl(addr | 1);
			}
			pu.p = &address;
			prefix_2_address_str (pu, startip, sizeof(startip));

			memset(endip, 0, sizeof(endip));
			address.u.prefix4.s_addr = htonl(addr | 254);
			prefix_2_address_str (pu, endip, sizeof(endip));

			masklen2ip (dhcps->address.prefixlen, &innetmask);

			memset(netmask, 0, sizeof(netmask));
			sprintf(netmask, "%s", ipstack_inet_ntoa(innetmask));
		}
		else
		{
			if(dhcps->start_address.prefixlen)
			{
				struct in_addr innetmask;

				memset(startip, 0, sizeof(startip));
				pu.p = &dhcps->start_address;
				prefix_2_address_str (pu, startip, sizeof(startip));

				memset(endip, 0, sizeof(endip));
				pu.p = &dhcps->end_address;
				prefix_2_address_str (pu, endip, sizeof(endip));

				masklen2ip (dhcps->start_address.prefixlen, &innetmask);
				memset(netmask, 0, sizeof(netmask));
				sprintf(netmask, "%s", ipstack_inet_ntoa(innetmask));
			}

		}
		if(dhcps->gateway.prefixlen)
		{
			pu.p = &dhcps->gateway;
			prefix_2_address_str (pu, gateway, sizeof(gateway));
		}
		if(dhcps->gateway_secondary.prefixlen)
		{
			pu.p = &dhcps->gateway_secondary;
			prefix_2_address_str (pu, gateway2, sizeof(gateway2));
		}
		if(dhcps->dns.prefixlen)
		{
			pu.p = &dhcps->dns;
			prefix_2_address_str (pu, dns, sizeof(dns));
		}
		if(dhcps->dns_secondary.prefixlen)
		{
			pu.p = &dhcps->dns_secondary;
			prefix_2_address_str (pu, dns2, sizeof(dns2));
		}

		websWrite(wp,
			"{\"response\":\"%s\", \"poolname\":\"%s\", \"startip\":\"%s\", \"endip\":\"%s\", \
			\"gateway\":\"%s\", \"gateway2\":\"%s\", \"dns\":\"%s\", \"dns2\":\"%s\",\
			 \"interface\":\"%s\"}",
			"OK", poolname, startip, endip, /*netmask,*/
			gateway, gateway2, dns, dns2, dhcps->ifp ? dhcps->ifp->name:" ");
		return OK;
	}
	return ERROR;
}


static int web_dhcpd_set_config(Webs *wp, const char *poolname, const char *start,
		const char *end, /*const char *netmask, */const char *gateway,
		const char *gateway2, const char *dns,
		const char *dns2, const char *ifpname)
{
	int ret = 0;
	struct prefix address;
	struct interface *ifp = NULL;
	nsm_dhcps_t * dhcps = nsm_dhcps_lookup_api(poolname);
	if(!dhcps)
	{
		if(nsm_dhcps_add_api(poolname) != OK)
			return ERROR;
		dhcps = nsm_dhcps_lookup_api(poolname);
	}
	if(dhcps)
	{
		prefix_zero(&address);
		if(str2prefix_ipv4 (start, (struct prefix_ipv4 *)&address) <= 0)
			return ERROR;
		ret |= nsm_dhcps_set_api(dhcps, DHCPS_CMD_NETWORK_START, &address);

		prefix_zero(&address);
		if(str2prefix_ipv4 (end, (struct prefix_ipv4 *)&address) <= 0)
			return ERROR;
		ret |= nsm_dhcps_set_api(dhcps, DHCPS_CMD_NETWORK_END, &address);

		prefix_zero(&address);
		if(str2prefix_ipv4 (gateway, (struct prefix_ipv4 *)&address) <= 0)
			return ERROR;
		ret |= nsm_dhcps_set_api(dhcps, DHCPS_CMD_GATEWAY, &address);

		if(gateway2)
		{
			prefix_zero(&address);
			if(str2prefix_ipv4 (gateway, (struct prefix_ipv4 *)&address) <= 0)
				return ERROR;
			ret |= nsm_dhcps_set_api(dhcps, DHCPS_CMD_GATEWAY_SECONDARY, &address);
		}
		if(dns)
		{
			prefix_zero(&address);
			if(str2prefix_ipv4 (dns, (struct prefix_ipv4 *)&address) <= 0)
				return ERROR;
			ret |= nsm_dhcps_set_api(dhcps, DHCPS_CMD_DNS, &address);
		}
		if(dns2)
		{
			prefix_zero(&address);
			if(str2prefix_ipv4 (dns2, (struct prefix_ipv4 *)&address) <= 0)
				return ERROR;
			ret |= nsm_dhcps_set_api(dhcps, DHCPS_CMD_DNS_SECONDARY, &address);
		}
		if(ifpname)
		{
			ifp = if_lookup_by_name(ifpname);
			if(ifp)
				ret |= nsm_interface_dhcp_mode_set_api(ifp, DHCP_SERVER, poolname);
		}
	}
	if(ret == OK)
	{
		websWrite(wp,
			"{\"response\":\"%s\", \"poolname\":\"%s\", \"startip\":\"%s\", \"endip\":\"%s\", \
			\"gateway\":\"%s\", \"gateway2\":\"%s\", \"dns\":\"%s\", \"dns2\":\"%s\",\
			 \"interface\":\"%s\"}",
			"OK", poolname, start, end,
			gateway, gateway2, dns, dns2, dhcps->ifp ? dhcps->ifp->name:" ");
		return OK;
	}
	return ERROR;
}

static int jst_dhcpd(int eid, webs_t wp, int argc, char **argv)
{
	websWrite(wp, "%s", "admin");
	return 0;
}

static int web_dhcpset_set(Webs *wp, char *path, char *query)
{
	char *strval = NULL;
	const char *poolname = NULL;
	const char *start = NULL;
	const char *end = NULL;
	/*const char *netmask, */
	const char *gateway = NULL;
	const char *gateway2 = NULL;
	const char *dns = NULL;
	const char *dns2 = NULL;
	const char *ifpname = NULL;

	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get ACTION Value");
		return web_return_text_plain(wp, ERROR);
		//return ERROR;
	}
	if(strstr(strval, "GET"))
	{
		poolname = webs_get_var(wp, T("poolname"), T(""));

		if(poolname == NULL)
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Can not Get poolname Value");
			return web_return_text_plain(wp, ERROR);
		}
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "web Get poolname=%s", poolname);
		if(web_dhcpd_get_config(wp, poolname) != OK)
		{
			return web_return_text_plain(wp, ERROR);
		}
		else
		{
			websSetStatus(wp, 200);
			websWriteHeaders(wp, -1, 0);
			websWriteHeader(wp, "Content-Type", "application/json");
			websWriteEndHeaders(wp);
		}
		websDone(wp);
		return OK;
	}
	poolname = webs_get_var(wp, T("poolname"), T(""));
	if (NULL == poolname)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get poolname Value");
		return web_return_text_plain(wp, ERROR);
	}
	ifpname = webs_get_var(wp, T("interface"), T(""));
	if (NULL == ifpname)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get interface Value");
		return web_return_text_plain(wp, ERROR);
	}

	start = webs_get_var(wp, T("startip"), T(""));
	if (NULL == start)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get start ip Value");
		return web_return_text_plain(wp, ERROR);
	}

	end = webs_get_var(wp, T("endip"), T(""));
	if (NULL == end)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get end ip Value");
		return web_return_text_plain(wp, ERROR);
	}

	gateway = webs_get_var(wp, T("gateway"), T(""));

	gateway2 = webs_get_var(wp, T("gateway2"), T(""));

/*	strval = webs_get_var(wp, T("netmask"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}*/

	dns = webs_get_var(wp, T("dns"), T(""));

	dns2 = webs_get_var(wp, T("dns2"), T(""));



	if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
	{
		zlog_debug(MODULE_WEB, "web Get poolname=%s interface=%s start=%s end=%s gateway=%s gateway2=%s dns=%s dns2=%s",
				   poolname, ifpname, start, end, gateway?gateway:" ", gateway2?gateway2:" ", dns?dns:" ", dns2?dns2:" ");
	}
	if(web_dhcpd_set_config(wp, poolname, start,
			end, /*const char *netmask, */gateway,
			gateway2, dns,
			dns2, ifpname) != OK)
	{
		return web_return_text_plain(wp, ERROR);
	}
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);
/*
	websWrite(wp,
			"{\"response\":\"%s\", \"poolname\":\"%s\", \"startip\":\"%s\", \"endip\":\"%s\", \"netmask\":\"%s\", \
			\"dns\":\"%s\", \"dns2\":\"%s\", \"interface\":\"%s\"}",
			"OK", "test", "192.168.2.2", "192.168.2.200", "255.255.255.0", "192.168.2.3", "", "ethernet 0/0/1");
*/

	//websWrite(wp, "%s", "]");
	//websWrite(wp, "%s", "OK");
	websDone(wp);
	return OK;
}

#ifdef ZPL_DHCP_MODULE
static int web_dhcp_lease_cb(dyn_lease_t *lease, Webs *wp)
{
	//char *lease_type[3] = {"Unknow", "dynamic", "static"};
	if(wp->iValue)
		websWrite(wp, ",");

	websWrite(wp, "{\"name\":\"%s\", \"mac\":\"%s\", \"ip\":\"%s\", \"expires\":\"%s\",\
			 \"interface\":\"%s\", \"poolname\":\"%s\"}",
		strlen(lease->hostname)?lease->hostname:" ",
		inet_ethernet(lease->lease_mac),
		inet_address(ntohl(lease->lease_address)),
		(lease->mode==1)?itoa(lease->expires, 10):"static",
		ifindex2ifname(lease->ifindex),
		dhcpd_pool_poolid2name(lease->poolid));

	return 0;
}
#endif


static int web_dhcp_client_tbl(Webs *wp, char *path, char *query)
{
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
#ifdef ZPL_DHCP_MODULE
	nsm_dhcps_lease_foreach(web_dhcp_lease_cb, wp);
#endif
/*
	websWrite(wp, "{\"name\":\"%s\", \"mac\":\"%s\", \"ip\":\"%s\", \"expires\":\"%s\"},",
			"hostdhcp1", "00:02:02:02:02:05", "191.2.2.3","3600");
	websWrite(wp, "{\"name\":\"%s\", \"mac\":\"%s\", \"ip\":\"%s\", \"expires\":\"%s\"},",
			"hostdhcp2", "00:02:02:02:02:06", "191.2.3.3","static");
	websWrite(wp, "{\"name\":\"%s\", \"mac\":\"%s\", \"ip\":\"%s\", \"expires\":\"%s\"},",
			"hostdhcp3", "00:02:02:02:02:07", "191.2.4.3","static");
	websWrite(wp, "{\"name\":\"%s\", \"mac\":\"%s\", \"ip\":\"%s\", \"expires\":\"%s\"}",
			"hostdhcp4", "00:02:02:02:02:08", "191.2.5.3","3600");
*/

	websWrite(wp, "%s", "]");
	wp->iValue = 0;
	websDone(wp);
	//printf("--------------%s-----------------\r\n", __func__);
	return 0;
}

static int web_dhcp_static_lease(zpl_uint32 type, char *devname, char *devip, zpl_uint8 *devmac, char *expires)
{
	nsm_dhcps_host_t *dhcphost = NULL;
	nsm_dhcps_t * dhcps = nsm_dhcps_lookup_api("lantest");
	if(!dhcps)
		return ERROR;
	if(type == 1)
	{
		dhcphost = nsm_dhcps_host_lookup_api(dhcps, devip, devmac);
		if(dhcphost)
		{
			memset(dhcphost->hostname, 0, sizeof(dhcphost->hostname));
			if(devname)
			{
				strcpy(dhcphost->hostname, devname);
			}
			return OK;
		}
		else
		{
			if(nsm_dhcps_add_host_api(dhcps, devip, devmac) == OK)
			{
				dhcphost = nsm_dhcps_host_lookup_api(dhcps, devip, devmac);
				if(dhcphost)
				{
					memset(dhcphost->hostname, 0, sizeof(dhcphost->hostname));
					if(devname)
					{
						strcpy(dhcphost->hostname, devname);
					}
					return OK;
				}
			}
		}
	}
	else
	{
		dhcphost = nsm_dhcps_host_lookup_api(dhcps, devip, devmac);
		if(dhcphost)
		{
			if(nsm_dhcps_del_host_api(dhcps, devip, devmac) == OK)
				return OK;
			return ERROR;
		}
		else
		{
			return OK;
		}
	}
	return ERROR;
}

static int web_dhcp_static_handle(Webs *wp, void *p)
{
	char *strval = webs_get_var(wp, T("BTNID"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get BTNID Value");
		return ERROR;//web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "delete"))
	{
		char devmac[128];
		char devip[128];
		strval = webs_get_var(wp, T("devmac"), T(""));
		if(strval == NULL || all_space(strval))
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Can not Get devmac Value");
			return ERROR;//web_return_text_plain(wp, ERROR);
		}
		memset(devmac, 0, sizeof(devmac));
		strcpy(devmac, strval);

		strval = webs_get_var(wp, T("devip"), T(""));
		if(strval == NULL || all_space(strval))
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Can not Get devip Value");
			return ERROR;//web_return_text_plain(wp, ERROR);
		}
		memset(devip, 0, sizeof(devip));
		strcpy(devip, strval);
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
		{
			zlog_debug(MODULE_WEB, "web Get devip=%s devmac=%s",
					    devip, devmac);
		}
		return (web_dhcp_static_lease(0, NULL, devip, devmac, NULL) == OK) ? web_return_text_plain(wp, OK):ERROR;//;
	}
	else if(strstr(strval, "dhcpadd"))
	{
		char devname[128];
		char devip[128];
		char devmac[128];
		char expires[128];
		strval = webs_get_var(wp, T("devname"), T(""));
		if(strval == NULL || all_space(strval))
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Can not Get devname Value");
			return ERROR;//web_return_text_plain(wp, ERROR);
		}
		memset(devname, 0, sizeof(devname));
		strcpy(devname, strval);

		strval = webs_get_var(wp, T("devip"), T(""));
		if(strval == NULL || all_space(strval))
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Can not Get devip Value");
			return ERROR;//web_return_text_plain(wp, ERROR);
		}
		memset(devip, 0, sizeof(devip));
		strcpy(devip, strval);
		strval = webs_get_var(wp, T("devmac"), T(""));
		if(strval == NULL || all_space(strval))
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Can not Get devmac Value");
			return ERROR;//web_return_text_plain(wp, ERROR);
		}
		memset(devmac, 0, sizeof(devmac));
		strcpy(devmac, strval);

		strval = webs_get_var(wp, T("expires"), T(""));
		if(strval == NULL || all_space(strval))
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Can not Get expires Value");
			return web_return_text_plain(wp, ERROR);
		}
		memset(expires, 0, sizeof(expires));
		strcpy(expires, strval);
		
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
		{
			zlog_debug(MODULE_WEB, "web Get devname=%s devip=%s devmac=%s expires=%s",
					   devname, devip, devmac, expires);
		}
		//memset(expires, 0, sizeof(expires));
		//strcpy(expires, strval);
		return (web_dhcp_static_lease(0, NULL, devip, devmac, NULL) == OK) ? web_return_text_plain(wp, OK):ERROR;//
		//return web_dhcp_static_lease(1, devname, devip, devmac, NULL);
/*		strval = webs_get_var(wp, T("devname"), T(""));
		strval = webs_get_var(wp, T("devip"), T(""));
		strval = webs_get_var(wp, T("devmac"), T(""));
		strval = webs_get_var(wp, T("expires"), T(""));*/
	}
/*	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "OK");
	websDone(wp);*/
	return ERROR;
}
#endif
#endif



int web_dhcp_jst_init(void)
{
#ifdef ZPL_DHCPD_MODULE
	websDefineJst("jst_dhcpd", jst_dhcpd);
	websFormDefine("setdhcp", web_dhcpset_set);
	websFormDefine("dhcp-client", web_dhcp_client_tbl);
	web_button_add_hook("dhcp", "dhcpadd", web_dhcp_static_handle, NULL);
	web_button_add_hook("dhcp", "delete", web_dhcp_static_handle, NULL);
#endif
	return 0;
}
#endif
