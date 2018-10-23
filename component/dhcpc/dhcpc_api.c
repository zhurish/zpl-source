/*
 * dhcpc_api.c
 *
 *  Created on: Aug 28, 2018
 *      Author: zhurish
 */
#include <zebra.h>

#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "log.h"
#include "vrf.h"
#include "command.h"
#include "interface.h"

#include "arp.h"
#include "bind.h"
#include "dhcpc_config.h"
#include "dhcpc_common.h"
#include "configure.h"
#include "control.h"
#include "dhcpcd.h"
#include "dhcpc_eloop.h"
#include "if-options.h"
#include "ipv4ll.h"
#include "ipv6rs.h"
#include "net.h"
#include "dhcpc_util.h"
#include "signals.h"

#include "nsm_dhcp.h"

#include "os_ansync.h"
#include "nsm_client.h"

extern os_ansync_lst *dhcp_lstmaster;


int dhcpc_interface_enable_api(struct interface *ifp, BOOL enable)
{
	int ret = 0;
	dhcpc_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = enable ? DHCPC_CLIENT_ADD_IF:DHCPC_CLIENT_DEL_IF;
	//strcpy(head.name, ifp->k_name);
	head.kifindex = ifp->k_ifindex;
	ret = send_control(&head);
	if((ifp && ifp->k_ifindex && !if_is_wireless(ifp)) && enable)
	{
		os_msleep(10);
		memset(&head, 0, sizeof(head));
		head.action = enable ? DHCPC_CLIENT_START_IF:DHCPC_CLIENT_STOP_IF;
		head.kifindex = ifp->k_ifindex;
		ret = send_control(&head);
	}
	return (ret > 0)? 0:-1;
}

int dhcpc_interface_start_api(struct interface *ifp, BOOL enable)
{
	int ret = 0;
	dhcpc_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = enable ? DHCPC_CLIENT_START_IF:DHCPC_CLIENT_STOP_IF;
	//strcpy(head.name, ifp->k_name);
	head.kifindex = ifp->k_ifindex;
	ret = send_control(&head);
	return (ret > 0)? 0:-1;
}
/*int dhcpc_interface_metric_api(struct interface *ifp, BOOL set, int metric)
{
	dhcpc_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = DHCPC_CLIENT_METRIC_IF;
	strcpy(head.name, ifp->k_name);
	head.kifindex = ifp->k_ifindex;
	head.option = metric;
	return send_control(&head);
}*/

/*
 * dhcp client default-router distance 使用此命令为从 DHCP server 获得的路由设置默认的路由器的距离。
router 默认路由器选项 (3)
static-route 静态路由选项 (33)
classless-static-route 无类静态路由选项(121)
classless-static-route-ms Microsoft 无类静态路由选项 (249)
tftp-server-address TFTP 服务器 ip 地址选项 (150)
dns-nameserver DNS 服务器选项 (6)
domain-name 域名选项 (15)
netbios-nameserver Netbios 服务器选项 (44)
vendor-specific 厂商相关配置选项 (43)

default: (55)
subnet mask:	1
static route:	33
router:			3
domain name server:	6
domain name:	15
broadcast address:	28
lease:			51	[配置DHCP客户端的期望租期]
renewal:			58
rebinding time:	59

53 DHCP message type
61 client identifier [缺省情况下，DHCP客户端的标识是客户端的MAC地址]
50 request IP address
57 maximum DHCP message size
60 vendor class identifier [Option60的默认值与设备相关，表示为“huawei-设备名称”（设备名称由sysname命令的配置决定）]
12 hostname
55 parameter request list
 */
int dhcpc_interface_option_api(struct interface *ifp, BOOL set, int option, char *string)
{
	int ret = -1;
	dhcpc_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = set ? DHCPC_CLIENT_ADD_OPTION_IF:DHCPC_CLIENT_DEL_OPTION_IF;
	//strcpy(head.name, ifp->k_name);
	head.kifindex = ifp->k_ifindex;
	head.option = option;
	if(string)
		strcpy(head.value, string);
	ret = send_control(&head);
	return (ret > 0)? 0:-1;
}


int dhcpc_interface_renew_api(struct interface *ifp)
{
	int ret = -1;
	dhcpc_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = DHCPC_CLIENT_RESTART_IF;
	//strcpy(head.name, ifp->k_name);
	head.kifindex = ifp->k_ifindex;
	ret = send_control(&head);
	return (ret > 0)? 0:-1;
}


int dhcpc_interface_config(struct interface *ifp, struct vty *vty)
{
	int i = 0;
	if(!ifp->k_ifindex)
		return OK;
	//os_ansync_lock(dhcp_lstmaster);
	struct dhcpc_interface * iface = nsm_dhcp_interface_get_pravite(ifp, DHCP_CLIENT);
	//struct dhcpc_interface * iface = dhcpc_interface_lookup(ifp->k_name, ifp->k_ifindex);
	if( (iface) && (iface->kifindex == ifp->k_ifindex) )
	{
		struct if_state *state = iface->state;
		struct if_options *ifo = NULL;
		if(!state)
		{
			return OK;
		}
		ifo = state->options;
		if(!ifo)
		{
			return OK;
		}
		if(ifo->options & DHCPCD_HOSTNAME)
			vty_out(vty, " dhcp client hostname %s%s", ifo->hostname, VTY_NEWLINE);
		if(ifo->vendorclassid[0] > 0)
			vty_out(vty, " dhcp client class-id %s%s", ifo->vendorclassid+1, VTY_NEWLINE);
		if(ifo->clientid[0] > 0 && (ifo->options & DHCPCD_CLIENTID))
			vty_out(vty, " dhcp client client-id %s%s", ifo->clientid + 1, VTY_NEWLINE);
		if(ifo->userclass[0] > 0)
			vty_out(vty, " dhcp client user-class %s%s", ifo->userclass + 1, VTY_NEWLINE);
		if(ifo->vendor[0] > 0 && (ifo->options & DHCPCD_VENDORRAW))
			vty_out(vty, " dhcp client vendor %s%s", ifo->vendor + 1, VTY_NEWLINE);

		if(/*has_option_mask(ifo->requestmask, DHO_LEASETIME) */
				ifo->leasetime && (ifo->options & DHCPCD_LASTLEASE))
			vty_out(vty, " dhcp client lease %d%s", ifo->leasetime, VTY_NEWLINE);

		if(ifo->metric)
			vty_out(vty, " dhcp client default-instance %d%s", ifo->metric, VTY_NEWLINE);

		if(!has_option_mask(ifo->requestmask, DHO_ROUTER))
			vty_out(vty, " no dhcp client request router%s", VTY_NEWLINE);

		if(has_option_mask(ifo->requestmask, DHO_STATICROUTE))
			vty_out(vty, " dhcp client request static-route%s", VTY_NEWLINE);

		if(has_option_mask(ifo->requestmask, DHO_CSR))
			vty_out(vty, " dhcp client request classless-static-route%s", VTY_NEWLINE);

		if(has_option_mask(ifo->requestmask, DHO_MSCSR))
			vty_out(vty, " dhcp client request classless-static-route-ms%s", VTY_NEWLINE);

		if(has_option_mask(ifo->requestmask, DHO_TFTPS))
			vty_out(vty, " dhcp client request tftp-server-address%s", VTY_NEWLINE);

		if(!has_option_mask(ifo->requestmask, DHO_DNSSERVER))
			vty_out(vty, " no dhcp client request dns-nameserver%s", VTY_NEWLINE);

		if(has_option_mask(ifo->requestmask, DHO_DNSDOMAIN))
			vty_out(vty, " no dhcp client request domain-name%s", VTY_NEWLINE);

		if(has_option_mask(ifo->requestmask, DHO_VENDOR))
			vty_out(vty, " dhcp client request vendor-specific%s", VTY_NEWLINE);

		if(ifo->fqdn == FQDN_NONE)
			vty_out(vty, " dhcp client fqdn none%s", VTY_NEWLINE);
		else if(ifo->fqdn == FQDN_PTR)
			vty_out(vty, " dhcp client fqdn ptr%s", VTY_NEWLINE);
		else if(ifo->fqdn == FQDN_BOTH)
			vty_out(vty, " dhcp client fqdn both%s", VTY_NEWLINE);
		else if(ifo->fqdn == FQDN_DISABLE)
			vty_out(vty, " dhcp client fqdn disable%s", VTY_NEWLINE);
	}
	//os_ansync_unlock(dhcp_lstmaster);
	return OK;
}

/*
[Switch_1] display dhcp client
DHCP client lease information on interface Vlanif10 :
 Current machine state         : Bound
 Internet address assigned via : DHCP
 Physical address              : 0025-9efb-be55
 IP address                    : 192.168.1.254
 Subnet mask                   : 255.255.255.0
 Gateway ip address            : 192.168.1.126
 DHCP server                   : 192.168.1.1
 Lease obtained at             : 2014-09-10 20:30:39
 Lease expires at              : 2014-09-11 20:30:39
 Lease renews at               : 2014-09-11 08:30:39
 Lease rebinds at              : 2014-09-11 17:30:39
 DNS                           : 192.168.1.2
 */
static char * dhcpc_client_state(enum DHS state)
{
	switch(state)
	{
	case DHS_INIT:
		return "INIT";
		break;
	case DHS_DISCOVER:
		return "DISCOVER";
		break;
	case DHS_REQUEST:
		return "REQUEST";
		break;
	case DHS_BOUND:
		return "BOUND";
		break;
	case DHS_RENEW:
		return "RENEW";
		break;
	case DHS_REBIND:
		return "REBIND";
		break;
	case DHS_REBOOT:
		return "REBOOT";
		break;
	case DHS_INFORM:
		return "INFORM";
		break;
	case DHS_RENEW_REQUESTED:
		return "RENEW REQUESTED";
		break;
	case DHS_INIT_IPV4LL:
		return "INIT IPV4LL";
		break;
	case DHS_PROBE:
		return "PROBE";
		break;
	}
	return "UNKNOWN";
}

int dhcpc_client_interface_show(struct interface *ifp, struct vty *vty)
{
	//os_ansync_lock(dhcp_lstmaster);
	struct dhcpc_interface * iface = NULL;
	if(!ifp->k_ifindex)
		return OK;
	iface = dhcpc_interface_lookup(NULL, ifp->k_ifindex);
	if(iface)
	{
		struct if_state *state = iface->state;
		struct if_options *ifo = NULL;
		if(!state)
		{
			//os_ansync_unlock(dhcp_lstmaster);
			return OK;
		}
		ifo = state->options;
		if(!ifo)
		{
			//os_ansync_unlock(dhcp_lstmaster);
			return OK;
		}

		vty_out(vty, " DHCP client lease information on interface %s :%s", ifp->name, VTY_NEWLINE);
		vty_out(vty, "  Current machine state          : %s%s",
				dhcpc_client_state(state->state), VTY_NEWLINE);
		vty_out(vty, "  Internet address assigned via  : DHCP%s", VTY_NEWLINE);
		vty_out(vty, "  Physical address               : %s%s",
				if_mac_out_format(iface->hwaddr, iface->hwlen), VTY_NEWLINE);

		vty_out(vty, "  IP address                     : %s%s",
				inet_ntoa(state->lease.addr), VTY_NEWLINE);
		vty_out(vty, "  Subnet address                 : %s%s",
				inet_ntoa(state->lease.net), VTY_NEWLINE);
		if(!iface->state->server_routes)
			vty_out(vty, "  Gateway ip address             : %s%s",
				inet_ntoa(state->lease.server), VTY_NEWLINE);
		else
		{
			struct rt *routes;
			for(routes = iface->state->server_routes; routes != NULL; routes = routes->next)
			{
				if(routes->dest.s_addr == 0 &&
						routes->net.s_addr == 0 &&
						routes->iface->kifindex == ifp->k_ifindex)
				{
					vty_out(vty, "  Gateway ip address             : %s%s",
						inet_ntoa(routes->gate), VTY_NEWLINE);
					break;
				}
			}
		}
		if(state->lease.dns1.s_addr)
		{
			vty_out(vty, "  DNS server                     : %s%s",
					inet_ntoa(state->lease.dns1), VTY_NEWLINE);
			if(state->lease.dns2.s_addr)
				vty_out(vty, "                                 : %s%s",
						inet_ntoa(state->lease.dns2), VTY_NEWLINE);
		}

		vty_out(vty, "  Domain name                    : %s%s",
				state->lease.domain, VTY_NEWLINE);

		vty_out(vty, "  DHCP server                    : %s%s",
				inet_ntoa(state->lease.server), VTY_NEWLINE);
		vty_out(vty, "  Lease time at                  : %s%s",
				os_time_out("/", state->lease.leasedfrom + state->lease.leasetime), VTY_NEWLINE);
		vty_out(vty, "  Lease renews at                : %s%s",
				os_time_out("/", state->lease.leasedfrom + state->lease.renewaltime), VTY_NEWLINE);
		vty_out(vty, "  Lease rebinds at               : %s%s",
				os_time_out("/", state->lease.leasedfrom + state->lease.rebindtime), VTY_NEWLINE);
		vty_out(vty, "  Lease from at                  : %s%s",
				os_time_out("/", state->lease.leasedfrom), VTY_NEWLINE);

		if(iface->state->server_routes)
		{
			struct rt *routes;
			char dest[64], net[64], gate[64], src[64];
			int prefixlen = 0;
			int flag = 0;
			for(routes = iface->state->server_routes; routes != NULL; routes = routes->next)
			{
				if(routes->dest.s_addr == 0 &&
						routes->net.s_addr == 0 &&
						routes->iface->kifindex == ifp->k_ifindex)
					continue;

				if(ntohl(routes->dest.s_addr) != 0 &&
						(ntohl(routes->dest.s_addr) & ntohl(routes->net.s_addr) ==
							ntohl(routes->src.s_addr) & ntohl(routes->net.s_addr)) &&
						routes->iface->kifindex == ifp->k_ifindex)
					continue;

				if(routes->gate.s_addr == 0 ||
						routes->dest.s_addr == 0)
					continue;

				os_memset(dest, 0, sizeof(dest));
				os_memset(net, 0, sizeof(net));
				os_memset(gate, 0, sizeof(gate));
				os_memset(src, 0, sizeof(src));

				prefixlen = ip_masklen(routes->net);

				os_snprintf(dest, sizeof(dest), "%s/%d", inet_ntoa(routes->dest), prefixlen);
				//os_snprintf(net, sizeof(net), "%s", inet_ntoa(routes->net));
				os_snprintf(gate, sizeof(gate), "%s", inet_ntoa(routes->gate));

				os_snprintf(src, sizeof(src), "%s", inet_ntoa(routes->src));
				if(flag == 0)
				{
					flag = 1;
					vty_out(vty, "  Route Table                    : %-16s %-16s %-16s %-16s %-8s %s",
							"destination", "gateway","source","interface","metric", VTY_NEWLINE);
				}
				vty_out(vty, "                                 : %-16s %-16s %-16s %-16s %d %s",
						dest, gate, src, routes->iface->name, routes->metric, VTY_NEWLINE);
			}
		}
	}
	//os_ansync_unlock(dhcp_lstmaster);
	return OK;
}

int dhcpc_client_interface_detail_show(struct interface *ifp, struct vty *vty)
{
	/*
Quagga(config)#
Quagga(config)# show dhcp client test
 DHCP client lease information on interface ethernet 0/0/1 :
  Current machine state          : BOUND
  Internet address assigned via  : DHCP
  Physical address               : 507b2-9de7-2949
  IP address                     : 192.16.1.100
  Subnet address                 : 255.255.255.0
  board ip address             : 192.16.1.255
  DHCP server                    : 192.16.1.253
  Lease time at                  : 1970/01/01 08:30:00
  Lease renews at                : 1970/01/01 08:15:00
  Lease rebinds at               : 1970/01/01 08:26:15
  Lease from at                  : 2018/09/19 18:37:23
  add                     : 192.16.1.100
  net             : 255.255.255.0
  dst                   : 0.0.0.0
  start_uptime                 : 1970/01/01 08:00:00
  boundtime                 : 1970/01/01 11:09:36
  frominfo                 : 0
  cookie                 : 63538263
  interval                 : 0
  nakoff                 : 1
  xid                 : 0
  probes                 : 0
  claims                 : 0
  conflicts                 : 0
  defend                 : 0
  arping_index                 : 0
  fail                   : 0.0.0.0
  metric                 : 1000
  leasetime                 : 0
  timeout                 : 30
  reboot                 : 5
  req_addr                   : 0.0.0.0
  req_mask                   : 0.0.0.0
 dhcp client hostname SWPlatformV0.0.3
 dhcp client class-id deth0/0/15.6
 dhcp client client-id
 dhcp client fqdn disable
 --------server----------
  rt dest                   : 0.0.0.0
  rt net                   : 0.0.0.0
  rt gate                   : 192.16.1.252
  rt src                   : 192.16.1.100
  rt metric                 : 1000
  rt interface              : enp0s25
 --------server----------
  rt dest                   : 192.16.1.0
  rt net                   : 255.255.255.0
  rt gate                   : 0.0.0.0
  rt src                   : 192.16.1.100
  rt metric                 : 1000
  rt interface              : enp0s25
 ---------------------

	 */
	//os_ansync_lock(dhcp_lstmaster);
	struct dhcpc_interface * iface = NULL;
	if(!ifp->k_ifindex)
		return OK;
	iface = dhcpc_interface_lookup(NULL, ifp->k_ifindex);
	if(iface)
	{
		int i = 0;
		u_int8 head = 0;
		struct in_addr addr;
		struct if_state *state = iface->state;
		struct if_options *ifo = NULL;
		if(!state)
		{
			//os_ansync_unlock(dhcp_lstmaster);
			return OK;
		}
		ifo = state->options;
		if(!ifo)
		{
			//os_ansync_unlock(dhcp_lstmaster);
			return OK;
		}

		vty_out(vty, " DHCP client lease information on interface %s :%s", ifp->name, VTY_NEWLINE);
		vty_out(vty, "  Current machine state          : %s%s",
				dhcpc_client_state(state->state), VTY_NEWLINE);
		vty_out(vty, "  Internet address assigned via  : DHCP%s", VTY_NEWLINE);
		vty_out(vty, "  Physical address               : %s%s",
				if_mac_out_format(iface->hwaddr, iface->hwlen), VTY_NEWLINE);

		vty_out(vty, "  IP address                     : %s%s",
				inet_ntoa(state->lease.addr), VTY_NEWLINE);
		vty_out(vty, "  Subnet address                 : %s%s",
				inet_ntoa(state->lease.net), VTY_NEWLINE);
		if(!iface->state->server_routes)
			vty_out(vty, "  Gateway ip address             : %s%s",
				inet_ntoa(state->lease.server), VTY_NEWLINE);
		else
		{
			struct rt *routes;
			for(routes = iface->state->server_routes; routes != NULL; routes = routes->next)
			{
				if(routes->dest.s_addr == 0 &&
						routes->net.s_addr == 0 &&
						routes->iface->kifindex == ifp->k_ifindex)
				{
					vty_out(vty, "  Gateway ip address             : %s%s",
						inet_ntoa(routes->gate), VTY_NEWLINE);
					break;
				}
			}
		}
		if(state->lease.dns1.s_addr)
		{
			vty_out(vty, "  DNS server                     : %s%s",
					inet_ntoa(state->lease.dns1), VTY_NEWLINE);
			if(state->lease.dns2.s_addr)
				vty_out(vty, "                                 : %s%s",
						inet_ntoa(state->lease.dns2), VTY_NEWLINE);
		}

		vty_out(vty, "  Domain name                    : %s%s",
				state->lease.domain, VTY_NEWLINE);

		vty_out(vty, "  DHCP server                    : %s%s",
				inet_ntoa(state->lease.server), VTY_NEWLINE);
		vty_out(vty, "  Lease time at                  : %s%s",
				os_time_out("/", state->lease.leasedfrom + state->lease.leasetime), VTY_NEWLINE);
		vty_out(vty, "  Lease renews at                : %s%s",
				os_time_out("/", state->lease.leasedfrom + state->lease.renewaltime), VTY_NEWLINE);
		vty_out(vty, "  Lease rebinds at               : %s%s",
				os_time_out("/", state->lease.leasedfrom + state->lease.rebindtime), VTY_NEWLINE);
		vty_out(vty, "  Lease from at                  : %s%s",
				os_time_out("/", state->lease.leasedfrom), VTY_NEWLINE);

		vty_out(vty, "  start_uptime                   : %s%s",
				os_time_out("/", state->lease.leasedfrom + iface->start_uptime), VTY_NEWLINE);

		vty_out(vty, "  boundtime                      : %s%s",
				os_time_out("/", state->lease.leasedfrom + state->lease.boundtime.tv_sec), VTY_NEWLINE);

		//vty_out(vty, "  frominfo                       : %x%s",state->lease.frominfo, VTY_NEWLINE);
		vty_out(vty, "  cookie                         : %x%s",state->lease.cookie, VTY_NEWLINE);

		vty_out(vty, "  metric                         : %d%s", ifo->metric, VTY_NEWLINE);
		vty_out(vty, "  leasetime                      : %d%s", ifo->leasetime, VTY_NEWLINE);
		vty_out(vty, "  timeout                        : %d%s", ifo->timeout, VTY_NEWLINE);
		vty_out(vty, "  reboot                         : %d%s", ifo->reboot, VTY_NEWLINE);


		vty_out(vty, "  interval                       : %x%s",state->interval, VTY_NEWLINE);
		vty_out(vty, "  nakoff                         : %x%s",state->nakoff, VTY_NEWLINE);

		vty_out(vty, "  xid                            : %x%s",state->xid, VTY_NEWLINE);
		vty_out(vty, "  probes                         : %x%s",state->probes, VTY_NEWLINE);

		vty_out(vty, "  claims                         : %x%s",state->claims, VTY_NEWLINE);
		vty_out(vty, "  conflicts                      : %x%s",state->conflicts, VTY_NEWLINE);
		vty_out(vty, "  defend                         : %x%s",state->defend, VTY_NEWLINE);
		vty_out(vty, "  arping_index                   : %x%s",state->arping_index, VTY_NEWLINE);
		vty_out(vty, "  fail                           : %s%s",
				inet_ntoa(state->fail), VTY_NEWLINE);

		vty_out(vty, "  req_addr                       : %s%s",
				inet_ntoa(ifo->req_addr), VTY_NEWLINE);
		vty_out(vty, "  req_mask                       : %s%s",
				inet_ntoa(ifo->req_mask), VTY_NEWLINE);

		if(ifo->hostname[0] > 0)
			vty_out(vty, "  hostname                       : %s%s", ifo->hostname, VTY_NEWLINE);
		if(ifo->vendorclassid[0] > 0)
			vty_out(vty, "  class-id                       : %s%s", ifo->vendorclassid+1, VTY_NEWLINE);
		if(ifo->clientid[0] > 0)
			vty_out(vty, "  client-id                      : %s%s", ifo->clientid + 1, VTY_NEWLINE);
		if(ifo->userclass[0] > 0)
			vty_out(vty, "  user-class                     : %s%s", ifo->userclass + 1, VTY_NEWLINE);
		if(ifo->vendor[0] > 0)
			vty_out(vty, "  vendor                         : %s%s", ifo->vendor + 1, VTY_NEWLINE);

		if(ifo->fqdn == FQDN_NONE)
			vty_out(vty, "  fqdn                           : none%s", VTY_NEWLINE);
		else if(ifo->fqdn == FQDN_PTR)
			vty_out(vty, "  fqdn                           : ptr%s", VTY_NEWLINE);
		else if(ifo->fqdn == FQDN_BOTH)
			vty_out(vty, "  fqdn                           : both%s", VTY_NEWLINE);
		else if(ifo->fqdn == FQDN_DISABLE)
			vty_out(vty, "  fqdn                           : disable%s", VTY_NEWLINE);

		if(ifo->script[0] > 0)
			vty_out(vty, "  script                         : %s%s", ifo->script, VTY_NEWLINE);

		head = 0;
		for(i = 0; i < 256; i++)
		{
			if(head == 0)
			{
				vty_out(vty, "  request option                 :%s", VTY_NEWLINE);
				head = 1;
			}
			if(has_option_mask(ifo->requestmask, i))
				vty_out(vty, "                                 : %d%s", i, VTY_NEWLINE);
		}
		head = 0;
		for(i = 0; i < 256 ; i++)
		{
			if(head == 0)
			{
				vty_out(vty, "  require option                 :%s", VTY_NEWLINE);
				head = 1;
			}
			if(has_option_mask(ifo->requiremask, i))
				vty_out(vty, "                                 : %d%s", i, VTY_NEWLINE);
		}
		head = 0;
		for(i = 0; i < 256 ; i++)
		{
			if(head == 0)
			{
				vty_out(vty, "  nomask option                  :%s", VTY_NEWLINE);
				head = 1;
			}
			if(has_option_mask(ifo->nomask, i))
				vty_out(vty, "                                 : %d%s", i, VTY_NEWLINE);
		}
		head = 0;
		for(i = 0; i < 256 ; i++)
		{
			if(head == 0)
			{
				vty_out(vty, "  nomask option                  :%s", VTY_NEWLINE);
				head = 1;
			}
			if(has_option_mask(ifo->dstmask, i))
				vty_out(vty, "                                 : %d%s", i, VTY_NEWLINE);
		}

		if(ifo->routes)
		{
			int prefixlen = 0;
			struct rt *routes;
			head = 0;
			for(routes = ifo->routes; routes != NULL; routes = routes->next)
			{
				if(head == 0)
				{
					vty_out(vty, "  route table                    :%s", VTY_NEWLINE);
					head = 1;
				}
				prefixlen = ip_masklen(routes->net);
				vty_out(vty, "  dhcp client fqdn               : disable%s", VTY_NEWLINE);
				vty_out(vty, "                                 : %s/%d ",
						inet_ntoa(routes->dest), prefixlen);

				vty_out(vty, "%s %s %d %s",
						inet_ntoa(routes->gate), routes->iface->name, routes->metric, VTY_NEWLINE);
			}
		}
		head = 0;
		if(ifo->blacklist_len && ifo->blacklist)
		{
			if(head == 0)
			{
				vty_out(vty, "  blacklist                      :%s", VTY_NEWLINE);
				head = 1;
			}
			for(i = 0; i < ifo->blacklist_len; i++)
			{
				addr.s_addr = ifo->blacklist[i];
				if(addr.s_addr)
					vty_out(vty, "                                 : %s%s",inet_ntoa(addr), VTY_NEWLINE);
			}
		}
		head = 0;
		if(ifo->whitelist_len && ifo->whitelist)
		{
			if(head == 0)
			{
				vty_out(vty, "  whitelist                      :%s", VTY_NEWLINE);
				head = 1;
			}
			for(i = 0; i < ifo->whitelist_len; i++)
			{
				addr.s_addr = ifo->whitelist[i];
				if(addr.s_addr)
					vty_out(vty, "                                 : %s%s",inet_ntoa(addr), VTY_NEWLINE);
			}
		}
	}
	//os_ansync_unlock(dhcp_lstmaster);
	return OK;
}
