/*
 * web_route_jst.c
 *
 *  Created on: 2019年8月3日
 *      Author: zhurish
 */

#include "zplos_include.h"
#include "vty.h"
#include "if.h"
#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "zmemory.hh"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "nsm_ip_vrf.h"
#include "nsm_interface.h"
#include "nexthop.h"
#include "nsm_rib.h"
#include "nsm_zserv.h"
#include "nsm_rnh.h"


#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"


#ifdef WEB_OPENWRT_PROCESS
static int web_kernel_route_table_one(char *input, char *ifname, zpl_uint32 *dest,
									  zpl_uint32 *gateway, zpl_uint32 *mask, zpl_uint32 *metric)
{
	//zpl_uint32 flag = 0, ref = 0, use = 0;&flag, &ref, &use,
	sscanf(input, "%[^\t] %x %x %*x%*d%*d %d %x", ifname, dest, gateway, metric, mask);
	return 0;
}

int web_kernel_route_lookup_default(ifindex_t ifindex, zpl_uint32 *local_gateway)
{
	FILE *f;
	char buf[512];
	char ifname[32];
	zpl_uint32 dest = 0, gateway = 0, mask = 0, metric = 0;
	ifindex_t ifkindex = 0;
	f = fopen ("/proc/net/route", "r");
	if (f)
	{
		while (fgets (buf, sizeof(buf), f))
		{
			if (strstr (buf, "Destination"))
				continue;
			memset (ifname, 0, sizeof(ifname));
			dest = 0, gateway = 0, mask = 0, metric = 0;
			web_kernel_route_table_one (buf, ifname, &dest, &gateway, &mask,
										&metric);
			if(mask == 0)
			{
				ifkindex = ifname2kernelifindex(ifname);
				if(ifindex && ifindex == ifkernel2ifindex(ifkindex))
				{
					if(local_gateway)
						*local_gateway = ntohl(gateway);

					fclose (f);
					return OK;
				}
			}
			//printf ("%s %x %x %x %x %d\r\n", ifname, dest, gateway, mask,
			//		metric);
		}
		fclose (f);
	}
	return ERROR;
}

int web_kernel_dns_lookup_default(ifindex_t *ifindex, zpl_uint32 *dns1, zpl_uint32 *dns2)
{
	FILE *f;
	char buf[512];
	zpl_uint32 find = 0, in = 0;
	//ifindex_t ifkindex = 0;
	f = fopen ("/tmp/resolv.conf.auto", "r");
	if (f)
	{
		while (fgets (buf, sizeof(buf), f))
		{
#ifdef APP_V9_MODULE
			if (strstr (buf, "wan2"))
			{
				if(ifindex)
					*ifindex = if_ifindex_make ("ethernet 0/0/3", NULL);
				find = 1;
			}
			else if (strstr (buf, "wan"))
			{
				if(ifindex)
					*ifindex = if_ifindex_make ("ethernet 0/0/2", NULL);
				find = 1;
			}
#else
			if (strstr (buf, "wan"))
			{
				if(ifindex)
					*ifindex = if_ifindex_make ("ethernet 0/0/2", NULL);
				find = 1;
			}
#endif
			if (find == 1 && strstr (buf, "nameserver"))
			{
				if(in == 0)
				{
					in++;
					if(dns1)
						*dns1 = ntohl(ipstack_inet_addr(buf + strlen("nameserver ")));
				}
				if(in == 1)
				{
					in++;
					if(dns2)
						*dns2 = ntohl(ipstack_inet_addr(buf + strlen("nameserver ")));
				}
			}
			if(find == 1 && in != 0)
			{
				fclose (f);
				return OK;
			}
		}
		fclose (f);
	}
	return ERROR;
}
#endif

#ifndef THEME_V9UI
static int web_route_table_one(struct route_node *rn, struct rib *rib, ifindex_t ifindex, union g_addr *gate)
{
	struct nexthop *nexthop, *tnexthop;
	int recursing = 0;
	// Nexthop information.
	for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
	{
		if (nexthop == rib->nexthop)
		{
			if ( CHECK_FLAG(rib->flags, ZEBRA_FLAG_SELECTED) &&
				 CHECK_FLAG (rib->flags, NEXTHOP_FLAG_FIB) /*&&
				 rib->type == ZEBRA_ROUTE_DHCP*/)
			{
				if (nexthop->ifindex == ifindex /*&& rn->p.prefixlen == 0 */&&
						nexthop->gate.ipv4.s_addr)
				{
					if (gate)
						memcpy (gate, &nexthop->gate, sizeof(union g_addr));
					return OK;
				}
			}

		}
	}
	return ERROR;
}

int web_route_lookup_default(ifindex_t ifindex, zpl_uint32 *local_gateway)
{
	struct route_table *table;
	struct route_node *rn;
	struct rib *rib;
	union g_addr gate;
	table = nsm_vrf_table (AFI_IP, SAFI_UNICAST, 0);
	if (!table)
		return ERROR;
	for (rn = route_top (table); rn; rn = route_next (rn))
		RNODE_FOREACH_RIB (rn, rib)
		{
			if(rn && rib)
			{
				if(web_route_table_one(rn, rib, ifindex, &gate) == OK)
				{
					if(local_gateway)
						*local_gateway = ntohl(gate.ipv4.s_addr);
					return OK;
				}
			}
		}
	return ERROR;
}

static void web_route_one (Webs *wp, struct route_node *rn, struct rib *rib)
{
	struct nexthop *nexthop, *tnexthop;
	int recursing;
	//int len = 0;
	char buf[BUFSIZ];
	char dest[64],gateway[64],vpn[64], metric[64],proto[64],state[64],ifp[64];


	for (ALL_NEXTHOPS_RO(rib->nexthop, nexthop, tnexthop, recursing))
	{
		memset(buf, 0, sizeof(buf));

		memset(dest, 0, sizeof(dest));
		memset(gateway, 0, sizeof(gateway));
		memset(metric, 0, sizeof(metric));
		memset(proto, 0, sizeof(proto));
		memset(state, 0, sizeof(state));
		memset(vpn, 0, sizeof(vpn));
		memset(ifp, 0, sizeof(ifp));

		if (nexthop == rib->nexthop)
		{
			snprintf(dest, sizeof(dest), "%s",prefix2str(&rn->p, buf, sizeof buf));

			if (rib->type != ZEBRA_ROUTE_CONNECT
					&& rib->type != ZEBRA_ROUTE_KERNEL)
				snprintf(metric, sizeof(metric), "%d", rib->metric);
			else
				snprintf(metric, sizeof(metric), "%d", rib->metric);

			if (rib->vrf_id != VRF_DEFAULT)
				snprintf(vpn, sizeof(vpn), "%s", (ip_vrf_lookup(rib->vrf_id))->name);
		}
		switch (nexthop->type)
		{
		case NEXTHOP_TYPE_IPV4:
		case NEXTHOP_TYPE_IPV4_IFINDEX:
			snprintf(gateway, sizeof(gateway), "%s", ipstack_inet_ntoa(nexthop->gate.ipv4));
			if (nexthop->ifindex)
			{
				if(web_type_get() == WEB_TYPE_HOME_WIFI)
				{
					if(nexthop->ifindex ==ifname2ifindex("ethernet 0/0/2"))
					{
						memset(ifp, 0, sizeof(ifp));
						sprintf(ifp, "%s", "wan");
					}
#ifdef APP_V9_MODULE
					else if(nexthop->ifindex ==ifname2ifindex("ethernet 0/0/3"))
					{
						memset(ifp, 0, sizeof(ifp));
						sprintf(ifp, "%s", "wan2");
					}
#endif
					else if(nexthop->ifindex ==ifname2ifindex("brigde 0/0/1"))
					{
						memset(ifp, 0, sizeof(ifp));
						sprintf(ifp, "%s", "lan");
					}
				}
				else
					snprintf(ifp, sizeof(ifp), "%s", ifindex2ifname_vrf(nexthop->ifindex, rib->vrf_id));
			}

			break;
#ifdef ZPL_BUILD_IPV6
			case NEXTHOP_TYPE_IPV6:
			case NEXTHOP_TYPE_IPV6_IFINDEX:
			case NEXTHOP_TYPE_IPV6_IFNAME:
				snprintf(gateway, sizeof(gateway), "%s", ipstack_inet_ntop (IPSTACK_AF_INET6, &nexthop->gate.ipv6, buf, BUFSIZ));
			if (nexthop->type == NEXTHOP_TYPE_IPV6_IFNAME)
			{
				if(web_type_get() == WEB_TYPE_HOME_WIFI)
				{
					if(ifname2ifindex(nexthop->ifname) == ifname2ifindex("ethernet 0/0/2"))
					{
						memset(ifp, 0, sizeof(ifp));
						sprintf(ifp, "%s", "wan");
					}
#ifdef APP_V9_MODULE
					else if(ifname2ifindex(nexthop->ifname) == ifname2ifindex("ethernet 0/0/3"))
					{
						memset(ifp, 0, sizeof(ifp));
						sprintf(ifp, "%s", "wan2");
					}
#endif
					else if(ifname2ifindex(nexthop->ifname) == ifname2ifindex("brigde 0/0/1"))
					{
						memset(ifp, 0, sizeof(ifp));
						sprintf(ifp, "%s", "lan");
					}
				}
				else
					snprintf(ifp, sizeof(ifp), "%s", nexthop->ifname);
			}
			else if (nexthop->ifindex)
			{
				if(web_type_get() == WEB_TYPE_HOME_WIFI)
				{
					if(nexthop->ifindex ==ifname2ifindex("ethernet 0/0/2"))
					{
						memset(ifp, 0, sizeof(ifp));
						sprintf(ifp, "%s", "wan");
					}
#ifdef APP_V9_MODULE
					else if(nexthop->ifindex ==ifname2ifindex("ethernet 0/0/3"))
					{
						memset(ifp, 0, sizeof(ifp));
						sprintf(ifp, "%s", "wan2");
					}
#endif
					else if(nexthop->ifindex ==ifname2ifindex("brigde 0/0/1"))
					{
						memset(ifp, 0, sizeof(ifp));
						sprintf(ifp, "%s", "lan");
					}
				}
				else
					snprintf(ifp, sizeof(ifp), "%s", ifindex2ifname_vrf(nexthop->ifindex, rib->vrf_id));
			}
			break;
#endif
		case NEXTHOP_TYPE_IFINDEX:
			if(web_type_get() == WEB_TYPE_HOME_WIFI)
			{
				if(nexthop->ifindex ==ifname2ifindex("ethernet 0/0/2"))
				{
					memset(ifp, 0, sizeof(ifp));
					sprintf(ifp, "%s", "wan");
				}
#ifdef APP_V9_MODULE
				else if(nexthop->ifindex ==ifname2ifindex("ethernet 0/0/3"))
				{
					memset(ifp, 0, sizeof(ifp));
					sprintf(ifp, "%s", "wan2");
				}
#endif
				else if(nexthop->ifindex ==ifname2ifindex("brigde 0/0/1"))
				{
					memset(ifp, 0, sizeof(ifp));
					sprintf(ifp, "%s", "lan");
				}
			}
			else
				snprintf(ifp, sizeof(ifp), "%s", ifindex2ifname_vrf(nexthop->ifindex, rib->vrf_id));
			//vty_out(vty, " is directly connected, %s",
			//		ifindex2ifname_vrf(nexthop->ifindex, rib->vrf_id));
			break;
		case NEXTHOP_TYPE_IFNAME:
			if(web_type_get() == WEB_TYPE_HOME_WIFI)
			{
				if(ifname2ifindex(nexthop->ifname) == ifname2ifindex("ethernet 0/0/2"))
				{
					memset(ifp, 0, sizeof(ifp));
					sprintf(ifp, "%s", "wan");
				}
#ifdef APP_V9_MODULE
				else if(ifname2ifindex(nexthop->ifname) == ifname2ifindex("ethernet 0/0/3"))
				{
					memset(ifp, 0, sizeof(ifp));
					sprintf(ifp, "%s", "wan2");
				}
#endif
				else if(ifname2ifindex(nexthop->ifname) == ifname2ifindex("brigde 0/0/1"))
				{
					memset(ifp, 0, sizeof(ifp));
					sprintf(ifp, "%s", "lan");
				}
			}
			else
				snprintf(ifp, sizeof(ifp), "%s", nexthop->ifname);
			//vty_out(vty, " is directly connected, %s", nexthop->ifname);
			break;
		case NEXTHOP_TYPE_BLACKHOLE:
			//vty_out(vty, " is directly connected, Null0");
			break;
		default:
			break;
		}

		if (!CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
		{
			snprintf(state, sizeof(state), "%s", "inactive");
			//continue;
		}
		else
			snprintf(state, sizeof(state), "%s", "active");

		snprintf(proto, sizeof(proto), "%s", "inactive");
		if (rib->type == ZEBRA_ROUTE_RIP)
			snprintf(proto, sizeof(proto), "%s", "RIP");
		else if (rib->type == ZEBRA_ROUTE_RIPNG)
			snprintf(proto, sizeof(proto), "%s", "RIPNG");
		else if (rib->type == ZEBRA_ROUTE_OSPF)
			snprintf(proto, sizeof(proto), "%s", "OSPF");
		else if (rib->type == ZEBRA_ROUTE_OSPF6)
			snprintf(proto, sizeof(proto), "%s", "OSPF6");
		else if (rib->type == ZEBRA_ROUTE_BABEL)
			snprintf(proto, sizeof(proto), "%s", "BABEL");
		else if (rib->type == ZEBRA_ROUTE_ISIS)
			snprintf(proto, sizeof(proto), "%s", "ISIS");
		else if (rib->type == ZEBRA_ROUTE_BGP)
			snprintf(proto, sizeof(proto), "%s", "BGP");
		else if (rib->type == ZEBRA_ROUTE_KERNEL)
			snprintf(proto, sizeof(proto), "%s", "KERNEL");
		else if (rib->type == ZEBRA_ROUTE_CONNECT)
			snprintf(proto, sizeof(proto), "%s", "CONNECT");
		else if (rib->type == ZEBRA_ROUTE_STATIC)
			snprintf(proto, sizeof(proto), "%s", "STATIC");
		else if (rib->type == ZEBRA_ROUTE_PIM)
			snprintf(proto, sizeof(proto), "%s", "PIM");
		else if (rib->type == ZEBRA_ROUTE_HSLS)
			snprintf(proto, sizeof(proto), "%s", "HSLS");
		else if (rib->type == ZEBRA_ROUTE_OLSR)
			snprintf(proto, sizeof(proto), "%s", "OLSR");
		else if (rib->type == ZEBRA_ROUTE_NHRP)
			snprintf(proto, sizeof(proto), "%s", "NHRP");
		else if (rib->type == ZEBRA_ROUTE_VRRP)
			snprintf(proto, sizeof(proto), "%s", "VRRP");
		else if (rib->type == ZEBRA_ROUTE_FRP)
			snprintf(proto, sizeof(proto), "%s", "FRP");
		else if (rib->type == ZEBRA_ROUTE_LLDP)
			snprintf(proto, sizeof(proto), "%s", "LLDP");
		else if (rib->type == ZEBRA_ROUTE_LDP)
			snprintf(proto, sizeof(proto), "%s", "LDP");
		else if (rib->type == ZEBRA_ROUTE_DHCP)
			snprintf(proto, sizeof(proto), "%s", "DHCP");

		if(wp->iValue)
			websWrite(wp, "%s", ",");

		websWrite(wp,
				"{\"dest\":\"%s\", \"gateway\":\"%s\", \"metric\":\"%s\", \"proto\":\"%s\", \"state\":\"%s\", \"interface\":\"%s\", \"vpn\":\"%s\"}",
				dest, gateway, metric, proto, state, ifp, vpn);
		wp->iValue++;
	}
}

static int web_route_table(Webs *wp, safi_t safi, vrf_id_t vrf_id)
{
	struct route_table *table;
	struct route_node *rn;
	struct rib *rib;
	table = nsm_vrf_table(AFI_IP, safi, vrf_id);
	if (!table)
		return CMD_SUCCESS;
	/* Show all IPv4 routes. */
	for (rn = route_top(table); rn; rn = route_next(rn))
			RNODE_FOREACH_RIB (rn, rib)
	{
		web_route_one(wp, rn, rib);
	}
	return 0;
}

static int web_route_tbl(Webs *wp, char *path, char *query)
{
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	//websWriteHeader(wp, "Content-Type", "application/json"/*"text/plain"*/);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
	web_route_table(wp, SAFI_UNICAST, VRF_DEFAULT);
/*
	websWrite(wp,
			"{\"dest\":\"%s\", \"gateway\":\"%s\", \"metric\":\"%s\", \"proto\":\"%s\", \"state\":\"%s\", \"interface\":\"%s\"}",
			"1.1.1.1/24", "1.1.1.1", "1000", "static", "active", "eth0");
	websWrite(wp, "%s", ",");
	websWrite(wp,
			"{\"dest\":\"%s\", \"gateway\":\"%s\", \"metric\":\"%s\", \"proto\":\"%s\", \"state\":\"%s\", \"interface\":\"%s\"}",
			"1.1.2.1/24", "1.1.2.1", "1000", "static", "active", "eth0");
	websWrite(wp, "%s", ",");
	websWrite(wp,
			"{\"dest\":\"%s\", \"gateway\":\"%s\", \"metric\":\"%s\", \"proto\":\"%s\", \"state\":\"%s\", \"interface\":\"%s\"}",
			"1.1.3.1/24", "1.1.3.1", "1000", "static", "active", "eth0");
*/
	websWrite(wp, "%s", "]");
	websDone(wp);
	wp->iValue = 0;
	return OK;
}




int web_static_ipv4_safi (safi_t safi, int add_cmd,
			const char *dest_str, const char *mask_str,
			const char *gate_str, const char *flag_str,
			const char *tag_str, const char *distance_str,
			const char *vrf_id_str)
{
  int ret;
  zpl_uchar distance;
  struct prefix p;
  struct ipstack_in_addr gate;
  struct ipstack_in_addr mask;
  const char *ifname;
  //zpl_uchar flag = 0;
  route_tag_t tag = 0;
  vrf_id_t vrf_id = VRF_DEFAULT;

  ret = str2prefix (dest_str, &p);
  if (ret <= 0)
    {
      return CMD_WARNING;
    }

  /* Cisco like mask notation. */
  if (mask_str)
    {
      ret = ipstack_inet_aton (mask_str, &mask);
      if (ret == 0)
        {
          return CMD_WARNING;
        }
      p.prefixlen = ip_masklen (mask);
    }

  /* Apply mask for given prefix. */
  apply_mask (&p);

  /* Administrative distance. */
  if (distance_str)
    distance = atoi (distance_str);
  else
    distance = ZEBRA_STATIC_DISTANCE_DEFAULT;

  /* tag */
  if (tag_str)
    tag = atoi (tag_str);

  /* VRF id */
  if (vrf_id_str)
	  vrf_id = atoi(vrf_id_str);
    //VTY_GET_INTEGER ("VRF ID", vrf_id, vrf_id_str);

  /* tag */
  if (tag_str)
    tag = atoi(tag_str);

  /* Null0 static route.  */
  if ((gate_str != NULL) && (strncasecmp (gate_str, "Null0", strlen (gate_str)) == 0))
    {
      if (flag_str)
        {
          //vty_out (vty, "%% can not have flag %s with Null0%s", flag_str, VTY_NEWLINE);
          return CMD_WARNING;
        }
      if (add_cmd)
        static_add_ipv4_safi (safi, &p, NULL, NULL, ZEBRA_FLAG_BLACKHOLE, tag, distance, vrf_id);
      else
        static_delete_ipv4_safi (safi, &p, NULL, NULL, tag, distance, vrf_id);
      return CMD_SUCCESS;
    }


  if (gate_str == NULL)
  {
    if (add_cmd)
      static_add_ipv4_safi (safi, &p, NULL, NULL, flag, tag, distance, vrf_id);
    else
      static_delete_ipv4_safi (safi, &p, NULL, NULL, tag, distance, vrf_id);

    return CMD_SUCCESS;
  }

  /* When gateway is A.B.C.D format, gate is treated as nexthop
     address other case gate is treated as interface name. */
  ret = ipstack_inet_aton (gate_str, &gate);
  if (ret)
    ifname = NULL;
  else
    ifname = gate_str;

  if (add_cmd)
    static_add_ipv4_safi (safi, &p, ifname ? NULL : &gate, ifname, flag, tag, distance, vrf_id);
  else
    static_delete_ipv4_safi (safi, &p, ifname ? NULL : &gate, ifname, tag, distance, vrf_id);

  return CMD_SUCCESS;
}


static int web_route_tbl_handle(Webs *wp, char *path, char *query, zpl_uint32 type)
{

	int ret = 0, gate_null = 0;
	char *dest_str = NULL;
	char *mask_str = NULL;
	char *gate_str = NULL;
	char *ifp_str = NULL;
	/*	char *tag_str = NULL;
	char *distance_str = NULL;
	char *vrf_id_str = NULL;*/

	dest_str = webs_get_var(wp, T("destination"), T(""));
	if (NULL == dest_str)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get destination Value");
		ret = ERROR;
		goto err_out;
	}
	if(type)
	{
		mask_str = webs_get_var(wp, T("netmask"), T(""));
		if (NULL == mask_str)
		{
			if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
				zlog_debug(MODULE_WEB, "Can not Get netmask Value");
			ret = ERROR;
			goto err_out;
		}
	}
	gate_str = webs_get_var(wp, T("gateway"), T(""));
	if (NULL == gate_str)
	{
		gate_null = 1;
		ifp_str = webs_get_var(wp, T("interface"), T(""));
		if(ifp_str && web_type_get() == WEB_TYPE_HOME_WIFI)
		{
			struct interface *ifp = NULL;
#ifdef APP_V9_MODULE
			if(strstr(ifp_str, "wan2"))
				ifp = if_lookup_by_name("ethernet 0/0/3");
			else if(strstr(ifp_str, "wan"))
				ifp = if_lookup_by_name("ethernet 0/0/2");
#else
			if(strstr(ifp_str, "wan"))
				ifp = if_lookup_by_name("ethernet 0/0/2");
#endif
			else if(strstr(ifp_str, "lan"))
				ifp = if_lookup_by_name("brigde 0/0/2");
			if(ifp)
				ifp_str = ifp->name;
		}
		if (NULL == ifp_str)
		{
			ret = ERROR;
			goto err_out;
		}
	}


/*
	strval = webs_get_var(wp, T("metric"), T(""));
	if (NULL == strval)
	{
		ret = ERROR;
		goto err_out;
	}
	printf("%s: metric=%s\r\n", __func__, strval);
*/

	ifp_str = webs_get_var(wp, T("interface"), T(""));
	if(ifp_str && web_type_get() == WEB_TYPE_HOME_WIFI)
	{
		struct interface *ifp = NULL;
#ifdef APP_V9_MODULE
		if(strstr(ifp_str, "wan2"))
			ifp = if_lookup_by_name("ethernet 0/0/3");
		else if(strstr(ifp_str, "wan"))
			ifp = if_lookup_by_name("ethernet 0/0/2");
#else
		if(strstr(ifp_str, "wan"))
			ifp = if_lookup_by_name("ethernet 0/0/2");
#endif
		else if(strstr(ifp_str, "lan"))
			ifp = if_lookup_by_name("brigde 0/0/2");
		if(ifp)
			ifp_str = ifp->name;
	}
	if (NULL == ifp_str)
	{
		if(gate_null)
		{
			ret = ERROR;
			goto err_out;
		}
	}
/*	web_static_ipv4_safi (safi_t safi, int add_cmd,
				const char *dest_str, const char *mask_str,
				const char *gate_str, const char *flag_str,
				const char *tag_str, const char *distance_str,
				const char *vrf_id_str);*/
	if(type)
	  ret = web_static_ipv4_safi (SAFI_UNICAST, type, dest_str, mask_str,
			  gate_str ? gate_str:ifp_str, NULL, NULL, NULL, NULL);
	else
		  ret = web_static_ipv4_safi (SAFI_UNICAST, type, dest_str, NULL,
				  gate_str ? gate_str:ifp_str, NULL, NULL, NULL, NULL);

err_out:
	if(ret != OK)
		return ERROR;//
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	//websWriteHeader(wp, "Content-Type", "application/json"/*"text/plain"*/);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	if(ret == OK)
		websWrite(wp, "%s", "OK");
	else
		websWrite(wp, "%s", "ERROR");
	websDone(wp);
	return OK;
}

static int web_add_route_tbl(Webs *wp, char *path, char *query)
{
	return web_route_tbl_handle(wp, path, query, 1);
}

static int web_del_route_tbl(Webs *wp, void *p)
{
	return web_route_tbl_handle(wp, NULL, NULL, 0);
}
#endif /* THEME_V9UI */

int web_route_jst_init(void)
{
#ifndef THEME_V9UI
	websFormDefine("route-tbl", web_route_tbl);
	websFormDefine("addroute", web_add_route_tbl);
	web_button_add_hook("routetbl", "delete", web_del_route_tbl, NULL);
#endif /* THEME_V9UI */
	return 0;
}
