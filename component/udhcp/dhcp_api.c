/*
 * dhcp_api.c
 *
 *  Created on: Jun 30, 2019
 *      Author: zhurish
 */

#include "zebra.h"
#include "if.h"
#include "memory.h"
#include "command.h"
#include "prefix.h"
#include "log.h"
#include "eloop.h"
#include "vty.h"

#include "dhcp_def.h"
#include "dhcpd.h"
#include "dhcp_lease.h"
#include "dhcp_option.h"
#include "dhcp_pool.h"
#include "dhcpc.h"
#include "dhcprelay.h"

#include "dhcp_api.h"



int dhcpc_interface_enable_api(struct interface *ifp, BOOL enable)
{
	if(dhcp_client_lookup_interface(&dhcp_global_config, ifp->ifindex))
	{
		if(enable)
			return OK;
		else
			return dhcp_client_del_interface(&dhcp_global_config, ifp->ifindex);
	}
	if(enable)
		return dhcp_client_add_interface(&dhcp_global_config, ifp->ifindex);
	else
		return ERROR;
}

int dhcpc_interface_start_api(struct interface *ifp, BOOL enable)
{
	if(dhcp_client_lookup_interface(&dhcp_global_config, ifp->ifindex))
	{
		if(enable)
			return OK;
	}
	return ERROR;
}

int dhcpc_interface_option_api(struct interface *ifp, BOOL enable, int index, char *option)
{
	client_interface_t * ifter = dhcp_client_lookup_interface(&dhcp_global_config, ifp->ifindex);
	if(ifter)
	{
		if(enable)
			return dhcp_client_interface_option_set(ifter, index, option, strlen(option));
		else
			return dhcp_option_del(ifter->options, index);
	}
	return ERROR;
}

int dhcpc_interface_config(struct interface *ifp, struct vty *vty)
{
	client_interface_t * ifter = dhcp_client_lookup_interface(&dhcp_global_config, ifp->ifindex);
	if(ifter)
	{
		int code = 0, i = 0;
		for(code = 0; code < DHCP_OPTION_MAX; code++)
		{
			if(ifter->options[code].len &&
					ifter->options[code].code == code &&
					ifter->options[code].data)
			{
				if(code == DHCP_HOST_NAME)
					vty_out(vty, " dhcp client hostname %s%s", ifter->options[code].data, VTY_NEWLINE);
				if(code == DHCP_LEASE_TIME)
					vty_out(vty, " dhcp client lease %s%s", ifter->options[code].data, VTY_NEWLINE);

				if(code == DHCP_VENDOR)
					vty_out(vty, " dhcp client class-id %s%s", ifter->options[code].data, VTY_NEWLINE);

				if(code == DHCP_CLIENT_ID)
					vty_out(vty, " dhcp client client-id %s%s", ifter->options[code].data, VTY_NEWLINE);
				
				for(i = 0; i < DHCP_OPTION_MAX; i++)
				{
					if(ifter->opt_mask[i])
					{
						if(i == DHCP_ROUTER)
							vty_out(vty, " dhcp client request router%s", VTY_NEWLINE);
						if(i == DHCP_ROUTES)
							vty_out(vty, " dhcp client request static-route%s", VTY_NEWLINE);
						if(i == DHCP_STATIC_ROUTES)
							vty_out(vty, " dhcp client request classless-static-route%s", VTY_NEWLINE);
						if(i == DHCP_MS_STATIC_ROUTES)
							vty_out(vty, " dhcp client request classless-static-route-ms%s", VTY_NEWLINE);
						if(i == DHCP_TFTP_SERVER_NAME)
							vty_out(vty, " dhcp client request tftp-server-address%s", VTY_NEWLINE);
						if(i == DHCP_DNS_SERVER)
							vty_out(vty, " dhcp client request dns-nameserver%s", VTY_NEWLINE);
						if(i == DHCP_DOMAIN_NAME)
							vty_out(vty, " dhcp client request domain-name%s", VTY_NEWLINE);
						if(i == DHCP_VENDOR)
							vty_out(vty, " dhcp client request vendor-specific%s", VTY_NEWLINE);
					}
				}
			}
		}
	}
	return OK;
}

static int dhcpc_interface_lease_show_one(struct vty *vty, client_interface_t *pstNode, BOOL detail)
{
	vty_out(vty, " Interface %s:%s", ifindex2ifname(pstNode->ifindex), VTY_NEWLINE);
	vty_out(vty, "  MAC            :%s%s", inet_ethernet(pstNode->client_mac), VTY_NEWLINE);
	vty_out(vty, "  address        :%s%s", inet_address(ntohl(pstNode->lease.lease_address)), VTY_NEWLINE);
	vty_out(vty, "  netmask        :%s%s", inet_address(ntohl(pstNode->lease.lease_netmask)), VTY_NEWLINE);
	vty_out(vty, "  broadcast      :%s%s", inet_address(ntohl(pstNode->lease.lease_broadcast)), VTY_NEWLINE);
	if(pstNode->lease.lease_gateway2)
		vty_out(vty, "  gateway        :%s %s%s", inet_address(ntohl(pstNode->lease.lease_gateway)),
				inet_address(ntohl(pstNode->lease.lease_gateway2)), VTY_NEWLINE);
	else
		vty_out(vty, "  gateway        :%s%s", inet_address(ntohl(pstNode->lease.lease_gateway)), VTY_NEWLINE);

	if(pstNode->lease.lease_dns2)
		vty_out(vty, "  dns            :%s %s%s", inet_address(ntohl(pstNode->lease.lease_dns1)),
				inet_address(ntohl(pstNode->lease.lease_dns2)), VTY_NEWLINE);
	else
		vty_out(vty, "  dns            :%s%s", inet_address(ntohl(pstNode->lease.lease_dns1)), VTY_NEWLINE);
	vty_out(vty, "  domain name    :%s%s", (pstNode->lease.domain_name), VTY_NEWLINE);

	vty_out(vty, "  server         :%s%s", inet_address(ntohl(pstNode->lease.server_address)), VTY_NEWLINE);

	if(detail)
	{
		vty_out(vty, "  gateway address:%s%s", inet_address(ntohl(pstNode->lease.gateway_address)), VTY_NEWLINE);
	}
	vty_out(vty, "  expires        :%s%s", os_time_string(pstNode->lease.expires), VTY_NEWLINE);
	if(detail)
	{
		vty_out(vty, "  starts         :%s%s", os_time_string(pstNode->lease.starts), VTY_NEWLINE);
		vty_out(vty, "  ends           :%s%s", os_time_string(pstNode->lease.starts), VTY_NEWLINE);
	}
	if(pstNode->lease.lease_timer1 && pstNode->lease.lease_timer2)
		vty_out(vty, "  timer srv      :%s %s%s", inet_address(ntohl(pstNode->lease.lease_timer1)),
				inet_address(ntohl(pstNode->lease.lease_timer2)), VTY_NEWLINE);
	else if(pstNode->lease.lease_timer1)
		vty_out(vty, "  timer srv      :%s%s", inet_address(ntohl(pstNode->lease.lease_timer1)), VTY_NEWLINE);

	if(pstNode->lease.lease_log1 && pstNode->lease.lease_log2)
		vty_out(vty, "  log srv        :%s %s%s", inet_address(ntohl(pstNode->lease.lease_log1)),
				inet_address(ntohl(pstNode->lease.lease_log2)), VTY_NEWLINE);
	else if(pstNode->lease.lease_log1)
		vty_out(vty, "  log srv        :%s%s", inet_address(ntohl(pstNode->lease.lease_log1)), VTY_NEWLINE);

	if(pstNode->lease.lease_ntp1 && pstNode->lease.lease_ntp2)
		vty_out(vty, "  ntp srv        :%s %s%s", inet_address(ntohl(pstNode->lease.lease_ntp1)),
				inet_address(ntohl(pstNode->lease.lease_ntp2)), VTY_NEWLINE);
	else if(pstNode->lease.lease_log1)
		vty_out(vty, "  ntp srv        :%s%s", inet_address(ntohl(pstNode->lease.lease_ntp1)), VTY_NEWLINE);

	if(pstNode->lease.lease_ttl)
		vty_out(vty, "  TTL            :%s%s", ((pstNode->lease.lease_ttl)), VTY_NEWLINE);

	if(pstNode->lease.lease_mtu)
		vty_out(vty, "  MTU            :%s%s", (ntohs(pstNode->lease.lease_mtu)), VTY_NEWLINE);

	if(strlen(pstNode->lease.tftp_srv_name))
		vty_out(vty, "  TFTP Server    :%s%s", ((pstNode->lease.tftp_srv_name)), VTY_NEWLINE);

	if(detail)
	{
		vty_out(vty, "  ciaddr         :%s%s", inet_address(ntohl(pstNode->lease.ciaddr)), VTY_NEWLINE);
		vty_out(vty, "  siaddr nip     :%s%s", inet_address(ntohl(pstNode->lease.siaddr_nip)), VTY_NEWLINE);
		vty_out(vty, "  relay address  :%s%s", inet_address(ntohl(pstNode->lease.gateway_nip)), VTY_NEWLINE);
	}

/*	uint8_t				opt_mask[256];
	dhcp_option_set_t	options[256];

	void				*master;
	void				*r_thread;	//read thread
	void				*t_thread;	//time thread,

	u_int32				first_secs;
	u_int32				last_secs;

	client_state_t		state;
	client_lease_t		lease;*/
	vty_out(vty, "%s", VTY_NEWLINE);
	return OK;
}

int dhcpc_interface_lease_show(struct vty *vty, struct interface *ifp, BOOL detail)
{
	client_interface_t *pstNode = NULL;
	NODE index;
	if (!lstCount(&dhcp_global_config.client_list))
		return OK;
	//vty_out(vty, " interface cnt = %d %s", lstCount(&dhcp_global_config.client_list), VTY_NEWLINE);
	for (pstNode = (client_interface_t *) lstFirst(&dhcp_global_config.client_list);
			pstNode != NULL;
			pstNode = (client_interface_t *) lstNext((NODE*) &index))
	{
		index = pstNode->node;
		if (ifp)
		{
			if(pstNode->ifindex == ifp->ifindex)
				return dhcpc_interface_lease_show_one(vty, pstNode,  detail);
			return OK;
		}
		else
		{
			dhcpc_interface_lease_show_one(vty, pstNode,  detail);
		}
	}
	return OK;
}
/************************************************************************************/

