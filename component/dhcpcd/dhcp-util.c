/*
 * dhcpcd - DHCP client daemon
 * Copyright (c) 2006-2018 Roy Marples <roy@marples.name>
 * All rights reserved

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <sys/stat.h>
#include <sys/utsname.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
//#include <arpa/nameser.h>  //after normal includes for sunos

#include "zebra.h"
#include "prefix.h"
#include "if.h"
#include "vty.h"
#include "dhcp-config.h"

#include "common.h"
#include "dhcp-common.h"
#include "dhcp.h"
#include "dhcpc-if.h"
#include "ipv6.h"
#include "logerr.h"
#include "dhcp-eloop.h"
#include "dhcpcd_api.h"

#define PL_DHCPC_MODULE

#include "nsm_dhcp.h"

int
if_carrier(struct dhcpc_interface *difp)
{
	int ret = 1;
	struct interface *ifp = if_lookup_by_kernel_index (difp->index);
	if(ifp)
	{
		ret = (ifp->flags & IFF_RUNNING) ? LINK_UP : LINK_DOWN;
	}
	ret = (difp->flags & IFF_RUNNING) ? LINK_UP : LINK_DOWN;
	return ret;
}

int
if_setflag(struct dhcpc_interface *difp, short flag)
{
	int ret = 1;
	struct interface *ifp = if_lookup_by_kernel_index (difp->index);
	if(ifp)
	{
		if (flag == 0 || (ifp->flags & flag) == flag)
			return 0;
		return pal_interface_update_flag(ifp,  flag);
	}
	return ERROR;
}

int
if_domtu(const struct dhcpc_interface *difp, short int mtu)
{
	struct interface *ifp = if_lookup_by_kernel_index (difp->index);
	if(ifp)
	{
		if(mtu)
		{
			if(nsm_interface_mtu_set_api(ifp, mtu) == OK)
				return mtu;
		}
		else
		{
			int omtu = 0;
			omtu = mtu;
			if(nsm_interface_mtu_get_api(ifp, &omtu) == OK)
				return omtu;
		}
	}
	return 1500;
}

unsigned short
if_vlanid(const struct dhcpc_interface *difp)
{
	int ret = 1;
	struct interface *ifp = if_lookup_by_kernel_index (difp->index);
	if(ifp)
	{
		return IF_VLAN_GET(ifp->ifindex);
	}
	return 0;
}

int dhcpcd_eloop_timeout_delete_chk_cb(void *arg)
{
	const struct dhcpc_interface *difp = arg;
	if(difp->ctx == &dhcpcd_ctx)
	{
		return 1;
	}
	return 1;
}


static struct dhcpc_interface * dhcpc_interface_alloc(struct dhcpcd_ctx *ctx, struct interface *ifp)
{
	struct dhcpc_interface *difp = NULL;
	difp = calloc(1, sizeof(struct dhcpc_interface));
	if (difp == NULL) {
		logerr(__func__);
		return NULL;
	}
	memset(difp, 0, sizeof(struct dhcpc_interface));
	difp->ctx = ctx;
	strcpy(difp->name, ifp->k_name);
	difp->flags = ifp->flags;
	difp->index = ifp->k_ifindex;
	difp->family = ARPHRD_ETHER;

	if(ifp->hw_addr_len)
	{
		memcpy(difp->hwaddr, ifp->hw_addr, ifp->hw_addr_len);
		difp->hwlen = ifp->hw_addr_len;
	}

	if (if_vimaster(ctx, difp->name) == 1) {
		logerrx("%s: is a Virtual Interface Master, skipping",
				difp->name);
		if_free(difp);
		return NULL;
	}
	difp->vlanid = if_vlanid(difp);

#ifdef SIOCGIFPRIORITY
	/* Respect the interface priority */
	memset(&ifr, 0, sizeof(ifr));
	strlcpy(ifr.ifr_name, difp->name, sizeof(ifr.ifr_name));
	if (ioctl(ctx->pf_inet_fd, SIOCGIFPRIORITY, &ifr) == 0)
		difp->metric = (unsigned int)ifr.ifr_metric;
	if_getssid(difp);
#else
	/* We reserve the 100 range for virtual interfaces, if and when
	 * we can work them out. */
	difp->metric = 900 + difp->index;
	if (if_getssid(difp) != -1) {
		difp->wireless = 1;
		difp->metric += 100;
	}
#endif
	difp->options = default_config(ctx);
	return difp;
}

static struct dhcpc_interface * dhcpc_interface_lookup(struct dhcpcd_ctx *ctx, int index)
{
	return if_findindex(ctx->ifaces, index);
}

static int dhcpc_interface_add(struct dhcpcd_ctx *ctx, struct dhcpc_interface *ifp)
{
	int i = 0;
	unsigned int active;
	for (i = 0; i < ctx->ifdc; i++)
		if (fnmatch(ctx->ifdv[i], ifp->name, 0) == 0)
		{
			if_free(ifp);
			return ERROR;
		}
	if (i < ctx->ifdc)
		active = IF_INACTIVE;
	for (i = 0; i < ctx->ifc; i++)
		if (fnmatch(ctx->ifv[i], ifp->name, 0) == 0)
		{
			if_free(ifp);
			return ERROR;
		}
	if (ctx->ifc && i == ctx->ifc)
		active = IF_INACTIVE;
	for (i = 0; i < ctx->ifac; i++)
		if (fnmatch(ctx->ifav[i], ifp->name, 0) == 0)
		{
			if_free(ifp);
			return ERROR;
		}
	if (ctx->ifac && i == ctx->ifac)
		active = IF_INACTIVE;

	/* Don't allow loopback or pointopoint unless explicit */
	if (ifp->flags & (IFF_LOOPBACK | IFF_POINTOPOINT))
	{
		if (ctx->ifac == 0 && !if_hasconf(ctx, ifp->name))
			active = IF_INACTIVE;
	}

	ifp->active = active;
	if (ifp->active)
		ifp->carrier = if_carrier(ifp);
	else
		ifp->carrier = LINK_UP;

	//ifp->active = IF_ACTIVE;
	//ifp->active = IF_ACTIVE_USER;
	//IF_SET_ACTIVE(ifp->active, IF_ACTIVE_USER);
	//IF_SET_V6ACTIVE(ifp->active, IF_ACTIVE_USER);

	logdebugx("%s:ifp->active=0x%x\n", __func__, ifp->active);

	//TAILQ_INSERT_TAIL(ctx->ifaces, ifp, next);
	//eloop_timeout_add_sec(ctx->eloop, 1, dhcpcd_prestartinterface, ifp);
	return OK;
}

/*
static int dhcpc_interface_del(struct dhcpcd_ctx *ctx, struct dhcpc_interface *ifp)
{
	//eloop_timeout_delete(ctx->eloop, dhcpcd_prestartinterface, ifp);
	dhcpcd_handleinterface(ctx, -1, ifp->name);
	TAILQ_REMOVE(ctx->ifaces, ifp, next);
	if_free(ifp);
	return OK;
}
*/


int dhcpcd_handleargs(struct dhcpcd_ctx *ctx, struct fd_list *fd,
	   char *buf, int len)
{
	struct dhcpc_interface *dhcpifp = NULL;
	struct interface *ifp = NULL;
	dhcpcd_ctrl_head *head = (dhcpcd_ctrl_head *) buf;
	ifp = if_lookup_by_kernel_index(head->kifindex);
	if(!ifp || !ifp->k_name_hash)
		return -1;

	logdebugx("%s:\n", __func__);
	os_msleep(100);
	switch (head->action)
	{
	case DHCPC_CLIENT_ADD_IF:
		dhcpifp = dhcpc_interface_lookup(ctx, head->kifindex);
		if(!dhcpifp)
		{
			dhcpifp = dhcpc_interface_alloc(ctx, ifp);
			if(dhcpifp)
			{
				if(dhcpc_interface_add(ctx, dhcpifp) == OK)
				{
					nsm_dhcp_interface_set_pravite(ifp, DHCP_CLIENT, dhcpifp);
					dhcpcd_event_interface(dhcpifp, DHCP_EVENT_ADD, head->ipv6);
				}
			}
		}
		else
		{
			dhcpcd_event_interface(dhcpifp, DHCP_EVENT_ADD, head->ipv6);
		}
		break;
	case DHCPC_CLIENT_DEL_IF:
		dhcpifp = dhcpc_interface_lookup(ctx, head->kifindex);
		if(dhcpifp)
		{
			dhcpcd_event_interface(dhcpifp, DHCP_EVENT_DEL, head->ipv6);
		}
		break;
	case DHCPC_CLIENT_START_IF:
		dhcpifp = dhcpc_interface_lookup(ctx, head->kifindex);
		if(dhcpifp)
		{
			dhcpcd_event_interface(dhcpifp, DHCP_EVENT_START, head->ipv6);
		}
		break;
	case DHCPC_CLIENT_STOP_IF:
		dhcpifp = dhcpc_interface_lookup(ctx, head->kifindex);
		if(dhcpifp)
		{
			dhcpcd_event_interface(dhcpifp, DHCP_EVENT_STOP, head->ipv6);
		}
		break;
	case DHCPC_CLIENT_RESTART_IF:
		dhcpifp = dhcpc_interface_lookup(ctx, head->kifindex);
		if(dhcpifp)
		{
			dhcpcd_event_interface(dhcpifp, DHCP_EVENT_REBOOT, head->ipv6);
		}
		break;
	case DHCPC_CLIENT_RENEW_IF:
		dhcpifp = dhcpc_interface_lookup(ctx, head->kifindex);
		if(dhcpifp)
		{
			dhcpcd_event_interface(dhcpifp, DHCP_EVENT_RENEW, head->ipv6);
		}
		break;

	case DHCPC_CLIENT_FREE_IF:
		dhcpifp = dhcpc_interface_lookup(ctx, head->kifindex);
		if(dhcpifp)
		{
			dhcpcd_event_interface(dhcpifp, DHCP_EVENT_DEL, TRUE);
			dhcpcd_event_interface(dhcpifp, DHCP_EVENT_DEL, FALSE);
		}
		break;
	case DHCPC_CLIENT_ADD_OPTION_IF:
	case DHCPC_CLIENT_DEL_OPTION_IF:
		break;
	case DHCPC_CLIENT_METRIC_IF:
		dhcpifp = dhcpc_interface_lookup(ctx, head->kifindex);
		if(dhcpifp)
		{
			dhcpifp->metric = (unsigned int)head->value;
			dhcpcd_event_interface(dhcpifp, DHCP_EVENT_REBOOT, TRUE);
			dhcpcd_event_interface(dhcpifp, DHCP_EVENT_REBOOT, FALSE);
		}
		break;
	default:
		break;
	}
	return 0;
}

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
/*	case DHS_INIT_IPV4LL:
		return "INIT IPV4LL";
		break;*/
	case DHS_PROBE:
		return "PROBE";
		break;
	}
	return "UNKNOWN";
}

int dhcpc_interface_config(struct interface *ifp, struct vty *vty)
{
	int i = 0;
	if(!ifp->k_ifindex)
		return OK;
	struct dhcpc_interface * iface = nsm_dhcp_interface_get_pravite(ifp, DHCP_CLIENT);
	if( (iface) && (iface->index == ifp->k_ifindex) )
	{
		struct if_options *ifo = NULL;
		ifo = iface->options;
		if(!iface->options)
		{
			return OK;
		}
		if((ifo->options & DHCPCD_HOSTNAME) && strlen(ifo->hostname))
			vty_out(vty, " dhcp client hostname %s%s", ifo->hostname, VTY_NEWLINE);
		if(ifo->vendorclassid[0] > 0 && strlen(ifo->vendorclassid))
			vty_out(vty, " dhcp client class-id %s%s", ifo->vendorclassid+1, VTY_NEWLINE);
		if(ifo->clientid[0] > 0 && strlen(ifo->clientid) && (ifo->options & DHCPCD_CLIENTID))
			vty_out(vty, " dhcp client client-id %s%s", ifo->clientid + 1, VTY_NEWLINE);
		if(ifo->userclass[0] > 0 && strlen(ifo->userclass))
			vty_out(vty, " dhcp client user-class %s%s", ifo->userclass + 1, VTY_NEWLINE);
		if(ifo->vendor[0] > 0 && strlen(ifo->vendor) && (ifo->options & DHCPCD_VENDORRAW))
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
	return OK;
}

int dhcpc_client_interface_show(struct interface *ifp, struct vty *vty)
{
	struct dhcpc_interface * iface = NULL;
	if(!ifp->k_ifindex)
		return OK;
	iface = nsm_dhcp_interface_get_pravite(ifp, DHCP_CLIENT);
	if(iface)
	{
		struct if_options *ifo = NULL;
		struct dhcp_state *state = D_STATE(iface);
		ifo = iface->options;
		if(!state || !iface->options)
		{
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
				inet_ntoa(state->lease.mask), VTY_NEWLINE);
		//if(!iface->state->server_routes)
			vty_out(vty, "  Gateway ip address             : %s%s",
				inet_ntoa(state->lease.server), VTY_NEWLINE);
/*		else
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
		}*/
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
				os_time_out("/", /*state->lease.leasedfrom + */state->lease.leasetime), VTY_NEWLINE);
		vty_out(vty, "  Lease renews at                : %s%s",
				os_time_out("/", /*state->lease.leasedfrom + */state->lease.renewaltime), VTY_NEWLINE);
		vty_out(vty, "  Lease rebinds at               : %s%s",
				os_time_out("/", /*state->lease.leasedfrom + */state->lease.rebindtime), VTY_NEWLINE);
/*		vty_out(vty, "  Lease from at                  : %s%s",
				os_time_out("/", state->lease.leasedfrom), VTY_NEWLINE);*/

/*		if(iface->state->server_routes)
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
		}*/
	}
	//os_ansync_unlock(dhcp_lstmaster);
	return OK;
}

int dhcpc_client_interface_detail_show(struct interface *ifp, struct vty *vty)
{
	struct dhcpc_interface * iface = NULL;
	if(!ifp->k_ifindex)
		return OK;
	iface = nsm_dhcp_interface_get_pravite(ifp, DHCP_CLIENT);
	if(iface)
	{
		int i = 0;
		u_int8 head = 0;
		struct in_addr addr;
		struct if_options *ifo = NULL;
		struct dhcp_state *state = D_STATE(iface);
		ifo = iface->options;
		if(!state || !iface->options)
		{
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
				inet_ntoa(state->lease.mask), VTY_NEWLINE);
		//if(!iface->state->server_routes)
			vty_out(vty, "  Gateway ip address             : %s%s",
				inet_ntoa(state->lease.server), VTY_NEWLINE);
/*		else
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
		}*/
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
/*
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
*/

		//vty_out(vty, "  frominfo                       : %x%s",state->lease.frominfo, VTY_NEWLINE);
		vty_out(vty, "  cookie                         : %x%s",state->lease.cookie, VTY_NEWLINE);

		vty_out(vty, "  metric                         : %d%s", ifo->metric, VTY_NEWLINE);
		vty_out(vty, "  leasetime                      : %d%s", ifo->leasetime, VTY_NEWLINE);
		vty_out(vty, "  timeout                        : %d%s", ifo->timeout, VTY_NEWLINE);
		vty_out(vty, "  reboot                         : %d%s", ifo->reboot, VTY_NEWLINE);


		vty_out(vty, "  interval                       : %x%s",state->interval, VTY_NEWLINE);
		vty_out(vty, "  nakoff                         : %x%s",state->nakoff, VTY_NEWLINE);

		vty_out(vty, "  xid                            : %x%s",state->xid, VTY_NEWLINE);
		//vty_out(vty, "  probes                         : %x%s",state->probes, VTY_NEWLINE);

/*		vty_out(vty, "  claims                         : %x%s",state->claims, VTY_NEWLINE);
		vty_out(vty, "  conflicts                      : %x%s",state->conflicts, VTY_NEWLINE);
		vty_out(vty, "  defend                         : %x%s",state->defend, VTY_NEWLINE);*/
		vty_out(vty, "  arping_index                   : %x%s",state->arping_index, VTY_NEWLINE);
/*
		vty_out(vty, "  fail                           : %s%s",
				inet_ntoa(state->fail), VTY_NEWLINE);
*/

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

/*
		if(ifo->script[0] > 0)
			vty_out(vty, "  script                         : %s%s", ifo->script, VTY_NEWLINE);
*/

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

/*		if(ifo->routes)
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
		}*/
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
/*int dhcpc_interface_start_api(struct interface *ifp, BOOL enable)
{
	int ret = 0;
	dhcpcd_ctrl_head head;
	memset(&head, 0, sizeof(head));
	head.action = enable ? 4:5;

	if(ctlfd == 0)
		ctlfd = control_open("/var/run/dhcpcd.sock");

	head.kifindex = ifp->k_ifindex;
	ret = control_send(ctlfd, &head, sizeof(dhcpcd_ctrl_head));
	return (ret > 0)? 0:-1;
}*/


#if 0

struct if_head * if_discover(struct dhcpcd_ctx *ctx, char *ifname)
{
	struct ifaddrs *ifa;
	int i;
	unsigned int active;
	struct if_head *ifs;
	struct dhcpc_interface *ifp;
	struct if_spec spec;
#ifdef AF_LINK
	const struct sockaddr_dl *sdl;
#ifdef SIOCGIFPRIORITY
	struct ifreq ifr;
#endif
#ifdef IFLR_ACTIVE
	struct if_laddrreq iflr;
#endif

#ifdef IFLR_ACTIVE
	memset(&iflr, 0, sizeof(iflr));
#endif
#elif AF_PACKET
	const struct sockaddr_ll *sll;
#endif

	if ((ifs = malloc(sizeof(*ifs))) == NULL) {
		logerr(__func__);
		return NULL;
	}
	TAILQ_INIT(ifs);
	if (getifaddrs(ifaddrs) == -1) {
		logerr(__func__);
		goto out;
	}

	for (ifa = *ifaddrs; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr != NULL) {
#ifdef AF_LINK
			if (ifa->ifa_addr->sa_family != AF_LINK)
				continue;
#elif AF_PACKET
			if (ifa->ifa_addr->sa_family != AF_PACKET)
				continue;
#endif
		}
		if (if_nametospec(ifa->ifa_name, &spec) != 0)
			continue;

		/* It's possible for an interface to have >1 AF_LINK.
		 * For our purposes, we use the first one. */
		TAILQ_FOREACH(ifp, ifs, next) {
			if (strcmp(ifp->name, spec.devname) == 0)
				break;
		}
		if (ifp)
			continue;

		if (argc > 0) {
			for (i = 0; i < argc; i++) {
				if (strcmp(argv[i], spec.devname) == 0)
					break;
			}
			active = (i == argc) ? IF_INACTIVE : IF_ACTIVE_USER;
		} else {
			/* -1 means we're discovering against a specific
			 * interface, but we still need the below rules
			 * to apply. */
			if (argc == -1 && strcmp(argv[0], spec.devname) != 0)
				continue;
			active = ctx->options & DHCPCD_INACTIVE ?
			    IF_INACTIVE: IF_ACTIVE_USER;
		}

		for (i = 0; i < ctx->ifdc; i++)
			if (fnmatch(ctx->ifdv[i], spec.devname, 0) == 0)
				break;
		if (i < ctx->ifdc)
			active = IF_INACTIVE;
		for (i = 0; i < ctx->ifc; i++)
			if (fnmatch(ctx->ifv[i], spec.devname, 0) == 0)
				break;
		if (ctx->ifc && i == ctx->ifc)
			active = IF_INACTIVE;
		for (i = 0; i < ctx->ifac; i++)
			if (fnmatch(ctx->ifav[i], spec.devname, 0) == 0)
				break;
		if (ctx->ifac && i == ctx->ifac)
			active = IF_INACTIVE;

#ifdef PLUGIN_DEV
		/* Ensure that the interface name has settled */
		if (!dev_initialized(ctx, spec.devname))
			continue;
#endif

		/* Don't allow loopback or pointopoint unless explicit */
		if (ifa->ifa_flags & (IFF_LOOPBACK | IFF_POINTOPOINT)) {
			if ((argc == 0 || argc == -1) &&
			    ctx->ifac == 0 && !if_hasconf(ctx, spec.devname))
				active = IF_INACTIVE;
		}

		if (if_vimaster(ctx, spec.devname) == 1) {
			logfunc_t *logfunc = argc != 0 ? logerrx : logdebugx;
			logfunc("%s: is a Virtual Interface Master, skipping",
			    spec.devname);
			continue;
		}

		ifp = calloc(1, sizeof(*ifp));
		if (ifp == NULL) {
			logerr(__func__);
			break;
		}
		ifp->ctx = ctx;
		strlcpy(ifp->name, spec.devname, sizeof(ifp->name));
		ifp->flags = ifa->ifa_flags;

		if (ifa->ifa_addr != NULL) {
#ifdef AF_LINK
			sdl = (const void *)ifa->ifa_addr;

#ifdef IFLR_ACTIVE
			/* We need to check for active address */
			strlcpy(iflr.iflr_name, ifp->name,
			    sizeof(iflr.iflr_name));
			memcpy(&iflr.addr, ifa->ifa_addr,
			    MIN(ifa->ifa_addr->sa_len, sizeof(iflr.addr)));
			iflr.flags = IFLR_PREFIX;
			iflr.prefixlen = (unsigned int)sdl->sdl_alen * NBBY;
			if (ioctl(ctx->pf_link_fd, SIOCGLIFADDR, &iflr) == -1 ||
			    !(iflr.flags & IFLR_ACTIVE))
			{
				if_free(ifp);
				continue;
			}
#endif

			ifp->index = sdl->sdl_index;
			switch(sdl->sdl_type) {
#ifdef IFT_BRIDGE
			case IFT_BRIDGE: /* FALLTHROUGH */
#endif
#ifdef IFT_PPP
			case IFT_PPP: /* FALLTHROUGH */
#endif
#ifdef IFT_PROPVIRTUAL
			case IFT_PROPVIRTUAL: /* FALLTHROUGH */
#endif
#if defined(IFT_BRIDGE) || defined(IFT_PPP) || defined(IFT_PROPVIRTUAL)
				/* Don't allow unless explicit */
				if ((argc == 0 || argc == -1) &&
				    ctx->ifac == 0 && active &&
				    !if_hasconf(ctx, ifp->name))
				{
					logdebugx("%s: ignoring due to"
					    " interface type and"
					    " no config",
					    ifp->name);
					active = IF_INACTIVE;
				}
				/* FALLTHROUGH */
#endif
#ifdef IFT_L2VLAN
			case IFT_L2VLAN: /* FALLTHROUGH */
#endif
#ifdef IFT_L3IPVLAN
			case IFT_L3IPVLAN: /* FALLTHROUGH */
#endif
			case IFT_ETHER:
				ifp->family = ARPHRD_ETHER;
				break;
#ifdef IFT_IEEE1394
			case IFT_IEEE1394:
				ifp->family = ARPHRD_IEEE1394;
				break;
#endif
#ifdef IFT_INFINIBAND
			case IFT_INFINIBAND:
				ifp->family = ARPHRD_INFINIBAND;
				break;
#endif
			default:
				/* Don't allow unless explicit */
				if ((argc == 0 || argc == -1) &&
				    ctx->ifac == 0 &&
				    !if_hasconf(ctx, ifp->name))
					active = IF_INACTIVE;
				if (active)
					logwarnx("%s: unsupported"
					    " interface type %.2x",
					    ifp->name, sdl->sdl_type);
				/* Pretend it's ethernet */
				ifp->family = ARPHRD_ETHER;
				break;
			}
			ifp->hwlen = sdl->sdl_alen;
			memcpy(ifp->hwaddr, CLLADDR(sdl), ifp->hwlen);
#elif AF_PACKET
			sll = (const void *)ifa->ifa_addr;
			ifp->index = (unsigned int)sll->sll_ifindex;
			ifp->family = sll->sll_hatype;
			ifp->hwlen = sll->sll_halen;
			if (ifp->hwlen != 0)
				memcpy(ifp->hwaddr, sll->sll_addr, ifp->hwlen);
#endif
		}
#ifdef __linux__
		/* PPP addresses on Linux don't have hardware addresses */
		else
			ifp->index = if_nametoindex(ifp->name);
#endif

		/* Ensure hardware address is valid. */
		if (!if_valid_hwaddr(ifp->hwaddr, ifp->hwlen))
			ifp->hwlen = 0;

		/* We only work on ethernet by default */
		if (ifp->family != ARPHRD_ETHER) {
			if ((argc == 0 || argc == -1) &&
			    ctx->ifac == 0 && !if_hasconf(ctx, ifp->name))
				active = IF_INACTIVE;
			switch (ifp->family) {
			case ARPHRD_IEEE1394:
			case ARPHRD_INFINIBAND:
#ifdef ARPHRD_LOOPBACK
			case ARPHRD_LOOPBACK:
#endif
#ifdef ARPHRD_PPP
			case ARPHRD_PPP:
#endif
				/* We don't warn for supported families */
				break;

/* IFT already checked */
#ifndef AF_LINK
			default:
				if (active)
					logwarnx("%s: unsupported"
					    " interface family %.2x",
					    ifp->name, ifp->family);
				break;
#endif
			}
		}

		if (!(ctx->options & (DHCPCD_DUMPLEASE | DHCPCD_TEST))) {
			/* Handle any platform init for the interface */
			if (active != IF_INACTIVE && if_dhcpc_init(ifp) == -1) {
				logerr("%s: if_dhcpc_init", ifp->name);
				if_free(ifp);
				continue;
			}
		}

		ifp->vlanid = if_vlanid(ifp);

#ifdef SIOCGIFPRIORITY
		/* Respect the interface priority */
		memset(&ifr, 0, sizeof(ifr));
		strlcpy(ifr.ifr_name, ifp->name, sizeof(ifr.ifr_name));
		if (ioctl(ctx->pf_inet_fd, SIOCGIFPRIORITY, &ifr) == 0)
			ifp->metric = (unsigned int)ifr.ifr_metric;
		if_getssid(ifp);
#else
		/* We reserve the 100 range for virtual interfaces, if and when
		 * we can work them out. */
		ifp->metric = 200 + ifp->index;
		if (if_getssid(ifp) != -1) {
			ifp->wireless = 1;
			ifp->metric += 100;
		}
#endif

		ifp->active = active;
		if (ifp->active)
			ifp->carrier = if_carrier(ifp);
		else
			ifp->carrier = LINK_UNKNOWN;
		TAILQ_INSERT_TAIL(ifs, ifp, next);
	}

out:
	return ifs;
}
#endif
