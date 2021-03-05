/* vi: set sw=4 ts=4: */
/*
 * udhcp server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
//applet:IF_UDHCPD(APPLET(udhcpd, BB_DIR_USR_SBIN, BB_SUID_DROP))
//kbuild:lib-$(CONFIG_UDHCPD) += common.o packet.o signalpipe.o socket.o
//kbuild:lib-$(CONFIG_UDHCPD) += dhcpd.o arpping.o
//kbuild:lib-$(CONFIG_FEATURE_UDHCP_RFC3397) += domain_codec.o
//usage:#define udhcpd_trivial_usage
//usage:       "[-fS] [-I ADDR]" IF_FEATURE_UDHCP_PORT(" [-P N]") " [CONFFILE]"
//usage:#define udhcpd_full_usage "\n\n"
//usage:       "DHCP server\n"
//usage:     "\n	-f	Run in foreground"
//usage:     "\n	-S	Log to syslog too"
//usage:     "\n	-I ADDR	Local address"
//usage:     "\n	-a MSEC	Timeout for ARP ping (default 2000)"
//usage:	IF_FEATURE_UDHCP_PORT(
//usage:     "\n	-P N	Use port N (default 67)"
//usage:	)
#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "prefix.h"
#include "log.h"
#include "eloop.h"


#include "dhcp_def.h"
#include "dhcp_lease.h"
#include "dhcp_pool.h"
#include "dhcp_util.h"
#include "dhcpd.h"
#include "dhcp_main.h"


/************************************************************************************/
/**************************************************************/
static dhcpd_interface_t * dhcpd_pool_create_interface(ifindex_t ifindex) {
	dhcpd_interface_t *ifter = XMALLOC(MTYPE_DHCPS_INFO,
			sizeof(dhcpd_interface_t));
	if (ifter) {
		struct interface * ifp = if_lookup_by_index(ifindex);
		memset(ifter, 0, sizeof(dhcpd_interface_t));
		ifter->ifindex = ifindex;
		//ifter->port = SERVER_PORT;
		/*
		 if(ifp && strlen(ifp->k_name))
		 ifter->k_name = strdup(ifp->k_name);
		 if(ifter->k_name)
		 {
		 udhcp_read_interface(ifter->k_name, NULL, &ifter->ipaddr, ifter->server_mac);
		 zlog_debug(MODULE_DHCP, "===========%s", ifter->k_name);
		 }
		 */
		udhcp_interface_mac(ifindex, &ifter->ipaddr, ifter->server_mac);
		zlog_debug(MODULE_DHCP, "===========%s", ifp->k_name);
		return ifter;
		//ifter->port;
		//ifter->server_mac[6];          /* our MAC address (used only for ARP probing) */
	}
	return NULL;
}

ospl_uint32  dhcpd_lookup_address_on_interface(dhcp_pool_t*config, ifindex_t ifindex) {
	dhcpd_interface_t *pstNode = NULL;
	NODE index;
	if (!config)
		return 0;
	if (!lstCount(&config->interf))
		return 0;
	for (pstNode = (dhcpd_interface_t *) lstFirst(&config->interf);
			pstNode != NULL;
			pstNode = (dhcpd_interface_t *) lstNext((NODE*) &index)) {
		index = pstNode->node;
		if (pstNode->ifindex == ifindex) {
			return pstNode->ipaddr;
		}
	}
	return 0;
}

dhcpd_interface_t * dhcpd_lookup_interface(dhcp_pool_t*config, ifindex_t ifindex) {
	dhcpd_interface_t *pstNode = NULL;
	NODE index;
	if (!lstCount(&config->interf))
		return NULL;
	for (pstNode = (dhcpd_interface_t *) lstFirst(&config->interf);
			pstNode != NULL;
			pstNode = (dhcpd_interface_t *) lstNext((NODE*) &index)) {
		index = pstNode->node;
		if (pstNode->ifindex == ifindex) {
			return pstNode;
		}
	}
	return NULL;
}

int dhcpd_pool_add_interface(dhcp_pool_t*config, ifindex_t ifindex) {
	dhcpd_interface_t * ifter = dhcpd_pool_create_interface(ifindex);
	if (ifter) {
		ifter->pool = config;
		//ifter->pool = config;
		lstAdd(&config->interf, ifter);

		if(config->global->sock == 0)
			config->global->sock = udhcp_udp_socket(config->global->server_port);
		if(config->global->rawsock == 0)
			config->global->rawsock = udhcp_raw_socket();

		zlog_debug(MODULE_DHCP, "dhcpd_pool_add_interface -> udhcp_udp_socket udhcp_raw_socket");

		if (config->global->r_thread == NULL && config->global->sock > 0)
		{
			zlog_debug(MODULE_DHCP, "dhcpd_pool_add_interface");
			config->global->r_thread = eloop_add_read(
				config->global->eloop_master, udhcp_read_thread,
				config->global, config->global->sock);
		}
		return OK;
	}
	return ERROR;
}

int dhcpd_pool_del_interface(dhcp_pool_t*config, ifindex_t ifindex) {
	dhcpd_interface_t * ifter = dhcpd_lookup_interface(config, ifindex);
	if (ifter) {
		if(ifter->arp_thread)
			eloop_cancel(ifter->arp_thread);
		if(ifter->auto_thread)
			eloop_cancel(ifter->auto_thread);
		if(ifter->lease_thread)
			eloop_cancel(ifter->lease_thread);
		lstDelete(&config->interf, ifter);
		XFREE(MTYPE_DHCPS_INFO, ifter);
		return OK;
	}
	return ERROR;
}

int dhcpd_interface_clean(dhcp_pool_t*config)
{
	dhcpd_interface_t *pstNode = NULL;
	NODE index;
	if (!lstCount(&config->interf))
		return NULL;
	for (pstNode = (dhcpd_interface_t *) lstFirst(&config->interf);
			pstNode != NULL;
			pstNode = (dhcpd_interface_t *) lstNext((NODE*) &index))
	{
		index = pstNode->node;
		if (pstNode)
		{
			if(pstNode->arp_thread)
				eloop_cancel(pstNode->arp_thread);
			if(pstNode->auto_thread)
				eloop_cancel(pstNode->auto_thread);
			if(pstNode->lease_thread)
				eloop_cancel(pstNode->lease_thread);
			lstDelete(&config->interf, pstNode);
			XFREE(MTYPE_DHCPS_INFO, pstNode);
		}
	}
	return OK;
}



/************************************************************************************/
/* Clear out all leases with matching nonzero chaddr OR yiaddr.
 * If chaddr == NULL, this is a conflict lease.
 */
/*
 static void dhcpd_clear_leases(dhcp_pool_t *config, const ospl_uint8 *chaddr, ospl_uint32  yiaddr)
 {
 if (chaddr)
 {
 dhcp_lease_del_mac(&config->dhcp_lease_list, chaddr);
 }
 else if (yiaddr)
 dhcp_lease_del_address(&config->dhcp_lease_list, yiaddr);
 }
 */

/* True if a lease has expired */
static int dhcpd_lease_is_expired(struct dyn_lease *lease) {
	return (lease->ends < (leasetime_t) os_time(NULL));
}

/* Find the first lease that matches MAC, NULL if no match */
static struct dyn_lease *dhcpd_find_lease_by_mac(dhcp_pool_t *config,
		const ospl_uint8 *mac) {
	return dhcp_lease_lookup_by_lease_mac(&config->dhcp_lease_list,
			LEASE_DYNAMIC, mac);
}

/* Find the first lease that matches IP, NULL is no match */
static struct dyn_lease *dhcpd_find_lease_by_nip(dhcp_pool_t *config, ospl_uint32  nip) {
	return dhcp_lease_lookup_by_lease_address(&config->dhcp_lease_list,
			LEASE_DYNAMIC, nip);
}

static dyn_lease_t * dhcpd_is_nip_reserved(dhcp_pool_t *config, ospl_uint32  nip) {
	return dhcp_lease_lookup_by_lease_address(&config->dhcp_lease_list,
			LEASE_STATIC, nip);
}
static struct dyn_lease * dhcpd_find_static_lease_by_mac(dhcp_pool_t *config, const ospl_uint8 *mac)
{
	return dhcp_lease_lookup_by_lease_mac(&config->dhcp_lease_list,
			LEASE_STATIC, mac);
}

/* Check if the IP is taken; if it is, add it to the lease table */
static int dhcpd_icmp_echo_request(ospl_uint32  nip, const ospl_uint8 *safe_mac,
		unsigned arpping_ms, dhcpd_interface_t *ifter) {
	struct in_addr temp;
	int r;

	r = icmp_echo_request(nip, safe_mac, ifter->ipaddr, ifter->server_mac,
			ifkernelindex2kernelifname(ifindex2ifkernel(ifter->ifindex)),
			arpping_ms);
	if (r)
		return r;
	temp.s_addr = nip;
	zlog_err(MODULE_DHCP, "%s belongs to someone, reserving it for %u seconds",
			inet_ntoa(temp), (unsigned )ifter->pool->conflict_time);
	//add_lease(ifter->pool, NULL, nip, ifter->pool->conflict_time, NULL, 0);
	return 0;
}

/* Find a new usable (we think) address */
static ospl_uint32  dhcpd_find_free_lease(const ospl_uint8 *safe_mac,
		unsigned arpping_ms, dhcp_pool_t *config, dhcpd_interface_t *ifter) {
	ospl_uint32  addr = 0;
	struct dyn_lease *oldest_lease = NULL;
	ospl_uint32  server_nip = ifter->ipaddr;
#if ENABLE_FEATURE_UDHCPD_BASE_IP_ON_MAC
	ospl_uint32  lease_stop;
	unsigned i, hash;

	/* hash hwaddr: use the SDBM hashing algorithm.  Seems to give good
	 * dispersal even with similarly-valued "strings".
	 */
	hash = 0;
	for (i = 0; i < 6; i++)
	hash += safe_mac[i] + (hash << 6) + (hash << 16) - hash;

	/* pick a seed based on hwaddr then iterate until we find a free address. */
	addr = config->start_ip
	+ (hash % (1 + config->end_ip - config->start_ip));
	lease_stop = addr;
#else
	addr = config->start_ip;
	//#define lease_stop (config->end_ip + 1)
	ospl_uint32  lease_stop = config->end_ip + 1;
#endif
	do {
		ospl_uint32  nip;
		struct dyn_lease *lease;

		/* ie, 192.168.55.0 */
		if ((addr & 0xff) == 0)
			goto next_addr;
		/* ie, 192.168.55.255 */
		if ((addr & 0xff) == 0xff)
			goto next_addr;
		nip = htonl(addr);
		/* skip our own address */

		if (server_nip && server_nip == nip/* == server_config.server_nip*/)
			goto next_addr;
		/* is this a static lease addr? */
		if (dhcpd_is_nip_reserved(config, nip))
			goto next_addr;

		lease = dhcpd_find_lease_by_nip(config, nip);
		if (!lease) {
			//TODO: DHCP servers do not always sit on the same subnet as clients: should *ping*, not arp-ping!
			if (dhcpd_icmp_echo_request(nip, safe_mac, arpping_ms, ifter))
				return nip;
		} else {
			if (!oldest_lease || lease->expires < oldest_lease->expires)
				oldest_lease = lease;
		}

		next_addr: addr++;
#if ENABLE_FEATURE_UDHCPD_BASE_IP_ON_MAC
		if (addr > config->end_ip)
		addr = config->start_ip;
#endif
	} while (addr != lease_stop);

	if (oldest_lease && dhcpd_lease_is_expired(oldest_lease)
			&& dhcpd_icmp_echo_request(oldest_lease->lease_address, safe_mac,
					arpping_ms, ifter)) {
		return oldest_lease->lease_address;
	}

	return 0;
}

/* Add a lease into the table, clearing out any old ones.
 * If chaddr == NULL, this is a conflict lease.
 */
static struct dyn_lease *dhcpd_new_lease(dhcpd_interface_t *inter, ospl_uint8 *chaddr)
{
	//char *p_host_name = NULL;
	//int hostname_len = 0;
	dyn_lease_t lease;
	ospl_uint8 empty_chaddr[ETHER_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };
	memset(&lease, 0, sizeof(lease));
	if ((memcmp(chaddr, empty_chaddr, ETHER_ADDR_LEN) == 0))
		return NULL;

	memcpy(lease.lease_mac, chaddr, ETHER_ADDR_LEN);
	lease.lease_address = 0;//olddhcp_pkt->yiaddr;
	lease.ifindex = inter->ifindex;
	lease.poolid = inter->pool->poolid;
	lease.mode = LEASE_DYNAMIC;
	lease.starts = os_time(NULL);
	lease.expires = 0;
	lease.ends = lease.starts + 0;

	return dhcp_lease_add(&inter->pool->dhcp_lease_list, &lease);
}
static struct dyn_lease *dhcpd_update_lease(dyn_lease_t *lease,
		ospl_uint32  lease_address, struct dhcp_packet *olddhcp_pkt, ospl_uint32  offer_time)
{
	char *p_host_name = NULL;
	ospl_uint32 hostname_len = 0;

	lease->lease_address = lease_address;//olddhcp_pkt->yiaddr;
/*	lease->ifindex = inter->ifindex;
	lease->poolid = inter->pool->poolid;*/
	lease->mode = LEASE_DYNAMIC;
	lease->starts = os_time(NULL);
	lease->expires = offer_time;
	lease->ends = lease->starts + offer_time;

	p_host_name = (const char*)udhcp_get_option(olddhcp_pkt, DHCP_HOST_NAME, &hostname_len);
	if (p_host_name)
	{
		strncpy(lease->hostname, p_host_name, MIN(hostname_len, UDHCPD_HOSTNAME_MAX));
	}
	/*p_host_name = (const char*)udhcp_get_option(dhcp_pkt, DHCP_DOMAIN_NAME, &hostname_len);
	if (p_host_name)
	{
		strncpy(lease->domain_name, p_host_name, MIN(hostname_len, UDHCPD_HOSTNAME_MAX));
	}*/
/*	p_host_name = (const char*)udhcp_get_option(dhcp_pkt, DHCP_SUBNET, &hostname_len);
	if (p_host_name)
	{
		udhcp_str2nip(p_host_name, &lease->lease_netmask);
	}
	p_host_name = (const char*)udhcp_get_option(dhcp_pkt, DHCP_ROUTER, &hostname_len);
	if (p_host_name)
	{
		udhcp_str2nip(p_host_name, &lease->lease_gateway);
		if(hostname_len > 4)
			udhcp_str2nip(p_host_name + 4, &lease->lease_gateway2);
	}
	p_host_name = (const char*)udhcp_get_option(dhcp_pkt, DHCP_DNS_SERVER, &hostname_len);
	if (p_host_name)
	{
		udhcp_str2nip(p_host_name, &lease->lease_dns1);
		if(hostname_len > 4)
			udhcp_str2nip(p_host_name + 4, &lease->lease_dns2);
	}*/
	p_host_name = (const char*)udhcp_get_option(olddhcp_pkt, DHCP_VENDOR, &hostname_len);
	if (p_host_name)
	{
		strncpy(lease->vendor, p_host_name, MIN(hostname_len, UDHCPD_HOSTNAME_MAX));
	}

	p_host_name = (const char*)udhcp_get_option(olddhcp_pkt, DHCP_CLIENT_ID, &hostname_len);
	if (p_host_name)
	{
		strncpy(lease->client_id, p_host_name, MIN(hostname_len, UDHCPD_HOSTNAME_MAX));
	}
	//inter->lease = dhcp_lease_add(&inter->pool->dhcp_lease_list, &lease);
	return lease;
}
/************************************************************************************/
/* Send a packet to a specific mac address and ip address by creating our own ip packet */
static void dhcpd_packet_to_client(struct dhcp_packet *dhcp_pkt,
		int force_broadcast, dhcp_pool_t *config, dhcpd_interface_t *ifter) {
	const ospl_uint8 *chaddr;
	ospl_uint32  ciaddr;
	struct udhcp_packet_cmd source;
	struct udhcp_packet_cmd dest;
	//ospl_uint32  server_nip = ifter->ipaddr;
	// Was:
	//if (force_broadcast) { /* broadcast */ }
	//else if (dhcp_pkt->ciaddr) { /* unicast to dhcp_pkt->ciaddr */ }
	//else if (dhcp_pkt->flags & htons(BROADCAST_FLAG)) { /* broadcast */ }
	//else { /* unicast to dhcp_pkt->yiaddr */ }
	// But this is wrong: yiaddr is _our_ idea what client's IP is
	// (for example, from lease file). Client may not know that,
	// and may not have UDP socket listening on that IP!
	// We should never unicast to dhcp_pkt->yiaddr!
	// dhcp_pkt->ciaddr, OTOH, comes from client's request packet,
	// and can be used.

	if (force_broadcast || (dhcp_pkt->flags & htons(BROADCAST_FLAG))
			|| dhcp_pkt->ciaddr == 0) {
		zlog_err(MODULE_DHCP, "broadcasting packet to client");
		ciaddr = INADDR_BROADCAST;
		chaddr = DHCP_MAC_BCAST_ADDR;
	} else {
		zlog_err(MODULE_DHCP, "unicasting packet to client ciaddr");
		ciaddr = dhcp_pkt->ciaddr;
		chaddr = dhcp_pkt->chaddr;
	}
	source.ip = ifter->ipaddr;
	source.port = dhcp_global_config.server_port;

	dest.ip = ciaddr;
	dest.port = dhcp_global_config.client_port;
	udhcp_send_raw_packet(dhcp_global_config.rawsock, dhcp_pkt, &source,
	/*dst*/&dest, chaddr, ifter->ifindex);
}

/* Send a packet to gateway_nip using the kernel ip stack */
static void dhcpd_packet_to_relay(struct dhcp_packet *dhcp_pkt,
		dhcp_pool_t *config, dhcpd_interface_t *ifter) {
	zlog_err(MODULE_DHCP, "forwarding packet to relay");
	if (ifter && ifter->ipaddr) {
		struct udhcp_packet_cmd source;
		struct udhcp_packet_cmd dest;

		source.ip = ifter->ipaddr;
		source.port = dhcp_global_config.server_port;

		dest.ip = dhcp_pkt->gateway_nip;
		dest.port = dhcp_global_config.client_port;
		udhcp_send_udp_packet(dhcp_global_config.sock, dhcp_pkt, &source,
				&dest);
	}
	/*	else
	 udhcp_send_kernel_packet(dhcp_pkt,
	 server_config.server_nip, SERVER_PORT,
	 dhcp_pkt->gateway_nip, SERVER_PORT);*/
}

static void dhcpd_send_packet(struct dhcp_packet *dhcp_pkt, ospl_uint32 force_broadcast,
		dhcp_pool_t *config, dhcpd_interface_t *ifter) {
	if (dhcp_pkt->gateway_nip)
		dhcpd_packet_to_relay(dhcp_pkt, config, ifter);
	else
		dhcpd_packet_to_client(dhcp_pkt, force_broadcast, config, ifter);
}

static void dhcpd_packet_init(struct dhcp_packet *packet,
		struct dhcp_packet *oldpacket, char type, ospl_uint32 server_nip) {
	/* Sets op, htype, hlen, cookie fields
	 * and adds DHCP_MESSAGE_TYPE option */
	udhcp_header_init(packet, type);

	packet->xid = oldpacket->xid;
	memcpy(packet->chaddr, oldpacket->chaddr, sizeof(oldpacket->chaddr));
	packet->flags = oldpacket->flags;
	packet->gateway_nip = oldpacket->gateway_nip;
	packet->ciaddr = oldpacket->ciaddr;
	dhcp_option_packet_set_simple(packet->options, sizeof(packet->options), DHCP_SERVER_ID, server_nip);
	//udhcp_add_simple_option(packet, DHCP_SERVER_ID, server_nip);
}

/* Fill options field, siaddr_nip, and sname and boot_file fields.
 * TODO: teach this code to use overload option.
 */
static void dhcpd_build_options(dhcp_pool_t *config, struct dhcp_packet *packet, struct dhcp_packet *oldpacket) {
/*	struct option_set *curr = config->options;

	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			udhcp_add_binary_option(packet, curr->data);
		curr = curr->next;
	}*/

	dhcp_option_packet(config->options, packet->options, sizeof(packet->options));




	packet->siaddr_nip = config->siaddr_nip;

	if (config->boot_server_name)
		strncpy((char*) packet->sname, config->boot_server_name,
				sizeof(packet->sname) - 1);
	if (config->boot_file)
		strncpy((char*) packet->file, config->boot_file,
				sizeof(packet->file) - 1);
}

static ospl_uint32  dhcpd_select_lease_time(dhcp_pool_t *config,
		struct dhcp_packet *packet) {
	ospl_uint32  lease_time_sec = config->max_lease_sec;
	ospl_uint8 *lease_time_opt = udhcp_get_option(packet, DHCP_LEASE_TIME, NULL);
	if (lease_time_opt) {
		move_from_unaligned32(lease_time_sec, lease_time_opt);
		lease_time_sec = ntohl(lease_time_sec);
		if (lease_time_sec > config->max_lease_sec)
			lease_time_sec = config->max_lease_sec;
		if (lease_time_sec < config->min_lease_sec)
			lease_time_sec = config->min_lease_sec;
	}
	return lease_time_sec;
}


static int dhcpd_get_lease(struct dhcp_packet *oldpacket, dhcp_pool_t *config,
		dhcpd_interface_t *ifter)
{
	ospl_uint32 arpping_ms = 2000;
	ospl_uint32  requested_address = 0;
	ifter->state.requested_address = 0;
	//struct dyn_lease *cu_lease = dhcpd_find_lease_by_mac(config, oldpacket->chaddr);

	dhcp_option_get_address(oldpacket->options, DHCP_REQUESTED_IP, &ifter->state.requested_address);
	if(ifter->state.requested_address == 0)
	{
		//
		/* Otherwise, find a free IP */
		requested_address = dhcpd_find_free_lease(oldpacket->chaddr,
				arpping_ms, config, ifter);
	}
	else
	{
		if(ntohl(ifter->state.requested_address) >= config->start_ip
			&& ntohl(ifter->state.requested_address) <= config->end_ip)
		{
			struct dyn_lease *lease = dhcpd_find_lease_by_nip(config, ifter->state.requested_address);
			if( lease && dhcpd_lease_is_expired(lease))
			{
				requested_address = ifter->state.requested_address;
				//ifter->lease = lease;
			}
			else if(lease == NULL)
			{
				requested_address = ifter->state.requested_address;
/*				if(cu_lease)
					cu_lease->lease_address = requested_address;*/
				//ifter->lease = dhcpd_add_or_update_lease(ifter, requested_address, oldpacket, config->offer_time);;
			}
		}
	}

/*
	if(cu_lease && cu_lease->lease_address == 0)
		cu_lease->lease_address = requested_address;
*/

	//ifter->lease = cu_lease;
	return requested_address;
}
/* We got a DHCP DISCOVER. Send an OFFER. */
/* : limit stack usage in caller */
static void dhcpd_send_offer(struct dhcp_packet *oldpacket, dhcp_pool_t *config,
		dhcpd_interface_t *ifter, struct dyn_lease *lease)
{
	struct dhcp_packet packet;
	ospl_uint32  lease_time_sec;
	struct in_addr addr;
	ospl_uint8 optlen = 0, *opt_b = NULL;//, i = 0;
	//ospl_uint32 arpping_ms = 2000;
	if (!config)
		return;
	dhcpd_packet_init(&packet, oldpacket, DHCPOFFER, ifter->ipaddr/* server ip*/);

	if (lease->lease_address == 0)
	{
		lease->lease_address = dhcpd_get_lease(oldpacket, config, ifter);
	}
	if (lease)
	{
		packet.yiaddr = lease->lease_address;
	}
	if (!packet.yiaddr)
	{
		zlog_err(MODULE_DHCP, "no free IP addresses. OFFER abandoned");
		return;
	}
	packet.siaddr_nip = ifter->ipaddr;
	lease_time_sec = dhcpd_select_lease_time(config, oldpacket);

	dhcp_option_packet_set_simple(packet.options, sizeof(packet.options), DHCP_LEASE_TIME, htonl(lease_time_sec));
	//udhcp_add_simple_option(&packet, DHCP_LEASE_TIME, htonl(lease_time_sec));

	dhcp_option_packet_set_simple(packet.options, sizeof(packet.options), DHCP_RENEWAL_TIME, htonl(lease_time_sec>>DHCP_T1));
	lease_time_sec = (ospl_uint32 )((ospl_float)lease_time_sec * DHCP_T2);
	dhcp_option_packet_set_simple(packet.options, sizeof(packet.options), DHCP_REBINDING_TIME, htonl(lease_time_sec));

	dhcpd_build_options(config, &packet, oldpacket);

	opt_b = dhcp_option_get(oldpacket->options, dhcp_option_get_length(oldpacket->options), DHCP_PARAM_REQ, &optlen);

	if(opt_b && optlen)
	{
		memset(ifter->state.req_mask, 0, sizeof(ifter->state.req_mask));
		memcpy(ifter->state.req_mask, opt_b, optlen);
/*		for(i = 0; i < optlen; i++)
		{
			if(!dhcp_option_get(packet.options, dhcp_option_get_length(packet.options), ifter->state.req_mask[i], NULL))
			{
				dhcp_option_packet_set_value(packet.options, sizeof(packet.options), ifter->state.req_mask[i], 4, NULL);
			}
		}*/
	}

	addr.s_addr = packet.yiaddr;
	zlog_err(MODULE_DHCP, "sending OFFER of %s", inet_ntoa(addr));
	/* dhcpd_send_packet emits error message itself if it detects failure */
	dhcpd_send_packet(&packet, /*force_bcast:*/0, config, ifter);
}

/* : limit stack usage in caller */
static void dhcpd_send_nak(struct dhcp_packet *oldpacket, dhcp_pool_t *config,
		dhcpd_interface_t *ifter, struct dyn_lease *lease) {
	struct dhcp_packet packet;
	dhcpd_packet_init(&packet, oldpacket, DHCPNAK, ifter->ipaddr);
	dhcp_lease_del(&config->dhcp_lease_list, lease);
	lease = NULL;
	zlog_err(MODULE_DHCP, "sending %s", "NAK");
	dhcpd_send_packet(&packet, /*force_bcast:*/1, config, ifter);
}

/* : limit stack usage in caller */
static void dhcpd_send_ack(struct dhcp_packet *oldpacket, dhcp_pool_t *config,
		dhcpd_interface_t *ifter, struct dyn_lease *lease) {
	struct dhcp_packet packet;
	ospl_uint32  lease_time_sec;
	struct in_addr addr;
	struct dyn_lease *cu_lease = NULL;
	ospl_uint8 *opt, optlen = 0;

	dhcpd_packet_init(&packet, oldpacket, DHCPACK, ifter->ipaddr);

	packet.yiaddr = lease->lease_address;
	packet.siaddr_nip = ifter->ipaddr;

	lease_time_sec = dhcpd_select_lease_time(config, oldpacket);
	//udhcp_add_simple_option(&packet, DHCP_LEASE_TIME, htonl(lease_time_sec));
	dhcp_option_packet_set_simple(packet.options, sizeof(packet.options), DHCP_LEASE_TIME, htonl(lease_time_sec));
	dhcp_option_packet_set_simple(packet.options, sizeof(packet.options), DHCP_RENEWAL_TIME, htonl(lease_time_sec>>DHCP_T1));
	lease_time_sec = (ospl_uint32 )((ospl_float)lease_time_sec * DHCP_T2);
	dhcp_option_packet_set_simple(packet.options, sizeof(packet.options), DHCP_REBINDING_TIME, htonl(lease_time_sec));

	dhcpd_build_options(config, &packet, oldpacket);

	addr.s_addr = lease->lease_address;
	zlog_err(MODULE_DHCP, "sending ACK to %s", inet_ntoa(addr));
	dhcpd_send_packet(&packet, /*force_bcast:*/0, config, ifter);

	cu_lease = dhcpd_update_lease(lease,
			lease->lease_address, oldpacket, lease_time_sec);

	dhcp_option_get_address(packet.options, DHCP_SUBNET, &cu_lease->lease_netmask);
	optlen = 0;
	opt = dhcp_option_get(packet.options, dhcp_option_get_length(packet.options), DHCP_ROUTER, &optlen);
	if(opt && optlen)
	{
		move_from_unaligned32(cu_lease->lease_gateway, opt);
		if(optlen > 4)
			move_from_unaligned32(cu_lease->lease_gateway2, opt + 4);
	}
	opt = dhcp_option_get(packet.options, dhcp_option_get_length(packet.options), DHCP_DNS_SERVER, &optlen);
	if(opt && optlen)
	{
		move_from_unaligned32(cu_lease->lease_dns1, opt);
		if(optlen > 4)
			move_from_unaligned32(cu_lease->lease_dns2, opt + 4);
	}

	if (cu_lease) {
		cu_lease->ifindex = ifter->ifindex;
	}
	if (ENABLE_FEATURE_UDHCPD_WRITE_LEASES_EARLY) {
		/* rewrite the file with leases at every new acceptance */
		dhcpd_lease_save();
	}
}

/* : limit stack usage in caller */
static void dhcpd_send_inform(struct dhcp_packet *oldpacket, dhcp_pool_t *config,
		dhcpd_interface_t *ifter, struct dyn_lease *lease) {

	/* "If a client has obtained a network address through some other means
	 * (e.g., manual configuration), it may use a DHCPINFORM request message
	 * to obtain other local configuration parameters.  Servers receiving a
	 * DHCPINFORM message construct a DHCPACK message with any local
	 * configuration parameters appropriate for the client without:
	 * allocating a new address, checking for an existing binding, filling
	 * in 'yiaddr' or including lease time parameters.  The servers SHOULD
	 * unicast the DHCPACK reply to the address given in the 'ciaddr' field
	 * of the DHCPINFORM message.
	 * ...
	 * The server responds to a DHCPINFORM message by sending a DHCPACK
	 * message directly to the address given in the 'ciaddr' field
	 * of the DHCPINFORM message.  The server MUST NOT send a lease
	 * expiration time to the client and SHOULD NOT fill in 'yiaddr'."
	 */
	//TODO: do a few sanity checks: is ciaddr set?
	//Better yet: is ciaddr == IP source addr?

	struct dhcp_packet packet;
	ospl_uint32  lease_time_sec;
	struct in_addr addr;
	struct dyn_lease *cu_lease = NULL;
	ospl_uint8 *opt, optlen = 0;

	dhcpd_packet_init(&packet, oldpacket, DHCPACK, ifter->ipaddr);

	packet.yiaddr = lease->lease_address;
	packet.siaddr_nip = ifter->ipaddr;

	lease_time_sec = config->max_lease_sec;
	//udhcp_add_simple_option(&packet, DHCP_LEASE_TIME, htonl(lease_time_sec));
	dhcp_option_packet_set_simple(packet.options, sizeof(packet.options), DHCP_LEASE_TIME, htonl(lease_time_sec));
	dhcp_option_packet_set_simple(packet.options, sizeof(packet.options), DHCP_RENEWAL_TIME, htonl(lease_time_sec>>DHCP_T1));
	lease_time_sec = (ospl_uint32 )((ospl_float)lease_time_sec * DHCP_T2);
	dhcp_option_packet_set_simple(packet.options, sizeof(packet.options), DHCP_REBINDING_TIME, htonl(lease_time_sec));

	//dhcp_option_packet_set_simple(packet.options, sizeof(packet.options), DHCP_OPT82, htonl(lease_time_sec));

	dhcpd_build_options(config, &packet, oldpacket);

	addr.s_addr = lease->lease_address;
	zlog_err(MODULE_DHCP, "sending ACK to %s", inet_ntoa(addr));
	dhcpd_send_packet(&packet, /*force_bcast:*/0, config, ifter);

	cu_lease = dhcpd_update_lease(lease,
			lease->lease_address, oldpacket, lease_time_sec);

	dhcp_option_get_address(packet.options, DHCP_SUBNET, &cu_lease->lease_netmask);
	optlen = 0;
	opt = dhcp_option_get(packet.options, dhcp_option_get_length(packet.options), DHCP_ROUTER, &optlen);
	if(opt && optlen)
	{
		move_from_unaligned32(cu_lease->lease_gateway, opt);
		if(optlen > 4)
			move_from_unaligned32(cu_lease->lease_gateway2, opt + 4);
	}
	opt = dhcp_option_get(packet.options, dhcp_option_get_length(packet.options), DHCP_DNS_SERVER, &optlen);
	if(opt && optlen)
	{
		move_from_unaligned32(cu_lease->lease_dns1, opt);
		if(optlen > 4)
			move_from_unaligned32(cu_lease->lease_dns2, opt + 4);
	}

	if (cu_lease) {
		cu_lease->ifindex = ifter->ifindex;
	}
	if (ENABLE_FEATURE_UDHCPD_WRITE_LEASES_EARLY) {
		/* rewrite the file with leases at every new acceptance */
		dhcpd_lease_save();
	}
}

static void dhcp_discover(dhcp_pool_t *pool, dhcpd_interface_t * ifter,
		struct dhcp_packet *packet)
{
	struct dyn_lease *lease = NULL;
	/* Look for a static/dynamic lease */
	lease = dhcpd_find_static_lease_by_mac(pool, packet->chaddr);
	if (lease == NULL)
	{
		lease = dhcpd_find_lease_by_mac(pool, packet->chaddr);
	}
	if(lease == NULL)
	{
		/* If we didn't find a lease, try to allocate one... */
		lease = dhcpd_new_lease(ifter, packet->chaddr);
	}
	if(lease == NULL)
	{
		zlog_warn(MODULE_DHCP, "dhcpd can not allocate new lease by '%s'", inet_ethernet(packet->chaddr));
		return ;
	}
	dhcpd_send_offer(packet, pool, ifter, lease);
}

static void dhcp_request(dhcp_pool_t *pool, dhcpd_interface_t * ifter,
		struct dhcp_packet *packet)
{
	struct dyn_lease *lease = NULL;
	/* Look for a static/dynamic lease */
	lease = dhcpd_find_static_lease_by_mac(pool, packet->chaddr);
	if (lease == NULL)
	{
		lease = dhcpd_find_lease_by_mac(pool, packet->chaddr);
	}
	if(lease == NULL)
	{
		zlog_warn(MODULE_DHCP, "Can not find lease by '%s'", inet_ethernet(packet->chaddr));
		return;
	}
	ifter->state.requested_address = 0;
	if(dhcp_option_get_address(packet->options, DHCP_REQUESTED_IP, &ifter->state.requested_address) != OK)
	{
		ifter->state.requested_address = packet->ciaddr;
	}
	if(ifter->state.requested_address == 0)
	{
		dhcpd_send_nak(packet, pool, ifter, lease);
		zlog_warn(MODULE_DHCP, "Can not get requested address for '%s', ignoring", inet_ethernet(packet->chaddr));
		return;
	}
	if (lease && ifter->state.requested_address == lease->lease_address)
	{
		dhcpd_send_ack(packet, pool, ifter, lease);
	}
	else
	{
		dhcpd_send_nak(packet, pool, ifter, lease);
	}
}

static void dhcp_release(dhcp_pool_t *pool, dhcpd_interface_t * ifter,
		struct dhcp_packet *packet)
{
	struct dyn_lease *lease = NULL;
	/* Look for a static/dynamic lease */
	lease = dhcpd_find_static_lease_by_mac(pool, packet->chaddr);
	if (lease == NULL)
	{
		lease = dhcpd_find_lease_by_mac(pool, packet->chaddr);
	}
	if(lease == NULL)
	{
		zlog_warn(MODULE_DHCP, "Can not find lease by '%s'", inet_ethernet(packet->chaddr));
		return;
	}
/*	ifter->state.requested_address = 0;
	if(dhcp_option_get_address(packet->options, DHCP_REQUESTED_IP, &ifter->state.requested_address) != OK)
	{
		ifter->state.requested_address = packet->ciaddr;
	}
	if(ifter->state.requested_address == 0)
	{
		dhcpd_send_nak(packet, pool, ifter, lease);
		zlog_warn(MODULE_DHCP, "Can not get requested address for '%s', ignoring", inet_ethernet(packet->chaddr));
		return;
	}*/
	if (lease && lease->lease_address)
	{
		/*
		 * First, we ping this lease to see if it's still
		 * there. if it is, we don't release it. This avoids
		 * the problem of spoofed releases being used to liberate
		 * addresses from the server.
		 */
		int r = icmp_echo_request(lease->lease_address, lease->lease_mac, ifter->ipaddr, ifter->server_mac,
				ifkernelindex2kernelifname(ifindex2ifkernel(ifter->ifindex)),
				2000);
		if(r == 0)
			return;

		dhcp_lease_del(&pool->dhcp_lease_list, lease);
	}
/*	if (lease && ifter->state.requested_address == lease->lease_address)
	{
		dhcpd_send_ack(packet, pool, ifter, lease);
	}
	else
	{
		dhcpd_send_nak(packet, pool, ifter, lease);
	}*/
}

static void dhcp_decline(dhcp_pool_t *pool, dhcpd_interface_t * ifter,
		struct dhcp_packet *packet)
{
	struct dyn_lease *lease = NULL;
	/* Look for a static/dynamic lease */
	lease = dhcpd_find_static_lease_by_mac(pool, packet->chaddr);
	if (lease == NULL)
	{
		lease = dhcpd_find_lease_by_mac(pool, packet->chaddr);
	}
	if(lease == NULL)
	{
		zlog_warn(MODULE_DHCP, "Can not find lease by '%s'", inet_ethernet(packet->chaddr));
		return;
	}
	/* DHCPDECLINE must specify address. */
	ifter->state.requested_address = 0;
	if(dhcp_option_get_address(packet->options, DHCP_REQUESTED_IP, &ifter->state.requested_address) != OK)
	{
		return;
	}
	if(ifter->state.requested_address == 0)
	{
		return;
	}
	if (lease)
	{
		dhcp_lease_del(&pool->dhcp_lease_list, lease);
		//abandon_lease(&pool->dhcp_lease_list, lease);
	}
/*	if (lease && ifter->state.requested_address == lease->lease_address)
	{
		dhcpd_send_ack(packet, pool, ifter, lease);
	}
	else
	{
		dhcpd_send_nak(packet, pool, ifter, lease);
	}*/
}

static void dhcp_inform(dhcp_pool_t *pool, dhcpd_interface_t * ifter,
		struct dhcp_packet *packet)
{
	struct dyn_lease *lease = NULL;
	/* Look for a static/dynamic lease */
	lease = dhcpd_find_static_lease_by_mac(pool, packet->chaddr);
/*	if (lease == NULL)
	{
		lease = dhcpd_find_lease_by_mac(pool, packet->chaddr);
	}*/
	if(lease == NULL)
	{
		zlog_warn(MODULE_DHCP, "Can not find lease by '%s'", inet_ethernet(packet->chaddr));
		return;
	}
	/*
	 * ciaddr should be set to client's IP address but
	 * not all clients are standards compliant.
	 */
	ifter->state.requested_address = 0;
	ifter->state.requested_address = packet->ciaddr;
	if(dhcp_option_get_address(packet->options, DHCP_REQUESTED_IP, &ifter->state.requested_address) != OK)
	{
		return;
	}
	if(ifter->state.requested_address == 0)
	{
		return;
	}
/*
	if (lease)
	{
		//dhcp_lease_del(&pool->dhcp_lease_list, lease);
		//abandon_lease(&pool->dhcp_lease_list, lease);
	}
*/
	dhcpd_send_inform(packet, pool, ifter, lease);
}

int udhcp_server_handle_thread(dhcp_pool_t *pool, dhcpd_interface_t * ifter,
		struct dhcp_packet *packet) {
	//struct dyn_lease *lease = NULL;
	ospl_uint8 msg_type = 0;

	if (packet->hlen != 6) {
		zlog_err(MODULE_DHCP, "MAC length != 6, ignoring packet");
		return ERROR;
	}
	if (packet->op != BOOTREQUEST) {
		zlog_err(MODULE_DHCP, "not a REQUEST, ignoring packet");
		return ERROR;
	}

	msg_type = dhcp_option_message_type_get(packet->options, sizeof(packet->options));

	/* Get SERVER_ID if present */
	if(msg_type != DHCPDISCOVER)
	{
		ifter->state.server_identifier = 0;
		dhcp_option_get_address(packet->options, DHCP_SERVER_ID, &ifter->state.server_identifier);
		if (ifter->state.server_identifier != ifter->ipaddr) {
			zlog_err(MODULE_DHCP, "server ID doesn't match, ignoring 0x%x != 0x%x", ifter->state.server_identifier, ifter->ipaddr);
			//return ERROR;
		}
	}

	switch (msg_type) {

	case DHCPDISCOVER:
		zlog_err(MODULE_DHCP, "received %s", "DISCOVER");
		dhcp_discover(packet, pool, ifter);
		break;

	case DHCPREQUEST:
		zlog_debug(MODULE_DHCP, "received %s", "REQUEST");
		dhcp_request(packet, pool, ifter);
		break;

	case DHCPDECLINE:
		zlog_err(MODULE_DHCP, "received %s", "DECLINE");
		dhcp_decline(packet, pool, ifter);
/*		if (ifter->lease && packet->ciaddr == ifter->lease->lease_address)
		{
			memset(ifter->lease->lease_mac, 0, sizeof(ifter->lease->lease_mac));
			ifter->lease->starts = time(NULL);
			ifter->lease->expires = pool->decline_time;
			ifter->lease->ends = ifter->lease->starts + pool->decline_time;
		}*/
		break;

	case DHCPRELEASE:
		/* "Upon receipt of a DHCPRELEASE message, the server
		 * marks the network address as not allocated."
		 *
		 * SERVER_ID must be present,
		 * REQUESTED_IP must not be present (we do not check this),
		 * chaddr must be filled in,
		 * ciaddr must be filled in
		 */
		zlog_err(MODULE_DHCP, "received %s", "RELEASE");
		dhcp_release(packet, pool, ifter);
		break;

	case DHCPINFORM:
		zlog_err(MODULE_DHCP, "received %s", "INFORM");
		dhcp_inform(packet, pool, ifter);
		break;
	default:
		zlog_err(MODULE_DHCP, "not a REQUEST, ignoring packet msg_type=%d", msg_type);
		break;
	}
	return OK;
}





#if 0
int udhcpd_main_a(void *p)
{
	int server_socket = -1, retval;
	ospl_uint8 *state;
	unsigned timeout_end;
	unsigned num_ips;
	//unsigned opt;
	struct option_set *option;
	char *str_I = str_I;
	const char *str_a = "2000";
	unsigned arpping_ms;
	//char *str_P;
	ospl_uint32 ifindex = 0;

	dhcpd_config_init(&server_config);
	//setup_common_bufsiz();
	dhcpd_pool_add(&server_config, NULL, "192.168.3.2", "192.168.3.254");
	dhcpd_pool_leases(&server_config, "235", "60");
	dhcpd_pool_autotime(&server_config, "7200");
	dhcpd_pool_decline_time(&server_config, "3600");
	dhcpd_pool_conflict_time(&server_config, "3600");
	dhcpd_pool_offer_time(&server_config, "60");
	//dhcpd_pool_add_interface(&server_config, "vmnet8");
	//dhcpd_pool_add_interface(&server_config, "enp0s25");
	dhcpd_pool_set_siaddr(&server_config, "0.0.0.0");
	config->lease_file = strdup(LEASES_FILE);
	/*	dhcpd_pool_set_notify_file(&server_config, char *str);
	 dhcpd_pool_set_sname(&server_config, char *str);
	 dhcpd_pool_set_boot_file(&server_config, char *str);
	 //option	subnet	255.255.255.0
	 dhcpd_pool_set_option(&server_config, char *str);
	 //00:60:08:11:CE:4E 192.168.0.54
	 dhcpd_pool_set_static_lease(&server_config, char *str);*/

	/*
	 SERVER_PORT = 67;
	 CLIENT_PORT = 68;
	 */

	arpping_ms = atoi(str_a);

	zlog_err(MODULE_DHCP, "started, v"BB_VER);

	option = udhcp_find_option(config->options, DHCP_LEASE_TIME);
	config->max_lease_sec = DEFAULT_LEASE_TIME;
	if (option)
	{
		move_from_unaligned32(config->max_lease_sec,
				option->data + OPT_DATA);
		config->max_lease_sec = ntohl(config->max_lease_sec);
	}

	/* Sanity check */
	num_ips = config->end_ip - config->start_ip + 1;
	if (config->max_leases > num_ips)
	{
		zlog_err(MODULE_DHCP, "max_leases=%u is too big, setting to %u",
				(unsigned )config->max_leases, num_ips);
		config->max_leases = num_ips;
	}

	config->g_leases = malloc(
			config->max_leases * sizeof(struct dyn_lease));
	read_leases(config->lease_file);

	/*
	 if (udhcp_read_interface(config->interface,
	 &config->ifindex,
	 (config->server_nip == 0 ? &config->server_nip : NULL),
	 config->server_mac)
	 ) {
	 retval = 1;
	 goto ret;
	 }
	 */

	/* Setup the signal pipe */
	//udhcp_sp_setup();
	continue_with_autotime: timeout_end = monotonic_sec()
	+ config->auto_time;
	while (1)
	{ /* loop until universe collapses */
		struct pollfd pfds[1];
		struct dhcp_packet packet;
		int bytes;
		int tv;
		ospl_uint8 *server_id_opt;
		ospl_uint8 *requested_ip_opt;
		ospl_uint32  requested_nip = requested_nip; /* for compiler */
		ospl_uint32  static_lease_nip;
		struct dyn_lease *lease, fake_lease;

		if (server_socket < 0)
		{
			server_socket = udhcp_listen_socket(/*INADDR_ANY,*/SERVER_PORT,
					NULL/*config->interface*/);
		}

		udhcp_sp_fd_set(pfds, server_socket);

		new_tv: tv = -1;
		if (config->auto_time)
		{
			tv = timeout_end - monotonic_sec();
			if (tv <= 0)
			{
				write_leases: write_leases();
				goto continue_with_autotime;
			}
			tv *= 1000;
		}

		/* Block here waiting for either signal or packet */
		retval = poll(pfds, 1, tv);

		if (retval <= 0)
		{
			if (retval == 0)
			goto write_leases;
			if (errno == EINTR)
			goto new_tv;
			/* < 0 and not EINTR: should not happen */
			//bb_perror_msg_and_die("poll");
		}

		/*		if (pfds[0].revents) switch (udhcp_sp_read()) {
		 case SIGUSR1:
		 zlog_err(MODULE_DHCP,"received %s", "SIGUSR1");
		 write_leases();
		 goto continue_with_autotime;
		 case SIGTERM:
		 zlog_err(MODULE_DHCP,"received %s", "SIGTERM");
		 write_leases();
		 goto ret0;
		 }*/

		/* Is it a packet? */
		if (!pfds[0].revents)
		continue; /* no */

		/* Note: we do not block here, we block on poll() instead.
		 * Blocking here would prevent SIGTERM from working:
		 * socket read inside this call is restarted on caught signals.
		 */
		bytes = udhcp_recv_kernel_packet(&packet, server_socket, &ifindex);
		if (bytes < 0)
		{
			/* bytes can also be -2 ("bad packet data") */
			if (bytes == -1 && errno != EINTR)
			{
				zlog_err(MODULE_DHCP,
						"read error: "STRERROR_FMT", reopening socket" STRERROR_ERRNO);
				close(server_socket);
				server_socket = -1;
			}
			continue;
		}
		if (dhcpd_lookup_interface(&server_config, ifindex))
		{

		}
		if (packet.hlen != 6)
		{
			zlog_err(MODULE_DHCP, "MAC length != 6, ignoring packet");
			continue;
		}
		if (packet.op != BOOTREQUEST)
		{
			zlog_err(MODULE_DHCP, "not a REQUEST, ignoring packet");
			continue;
		}
		state = udhcp_get_option(&packet, DHCP_MESSAGE_TYPE);
		if (state == NULL || state[0] < DHCP_MINTYPE || state[0] > DHCP_MAXTYPE)
		{
			zlog_err(MODULE_DHCP,
					"no or bad message type option, ignoring packet");
			continue;
		}

		/* Get SERVER_ID if present */
		server_id_opt = udhcp_get_option(&packet, DHCP_SERVER_ID);
		if (server_id_opt)
		{
			ospl_uint32  server_id_network_order;
			move_from_unaligned32(server_id_network_order, server_id_opt);
			ospl_uint32  server_nip = dhcpd_lookup_address_on_interface(
					&server_config, ifindex);
			if (server_id_network_order
					!= server_nip/*config->server_nip*/)
			{
				/* client talks to somebody else */
				zlog_err(MODULE_DHCP, "server ID doesn't match, ignoring");
				continue;
			}
		}

		/* Look for a static/dynamic lease */
		static_lease_nip = dhcpd_get_static_nip_by_mac(config->static_leases,
				&packet.chaddr);
		if (static_lease_nip)
		{
			zlog_err(MODULE_DHCP, "found static lease: %x", static_lease_nip);
			memcpy(&fake_lease.lease_mac, &packet.chaddr, 6);
			fake_lease.lease_nip = static_lease_nip;
			fake_lease.expires = 0;
			lease = &fake_lease;
		}
		else
		{
			lease = dhcpd_find_lease_by_mac(packet.chaddr);
		}

		/* Get REQUESTED_IP if present */
		requested_ip_opt = udhcp_get_option(&packet, DHCP_REQUESTED_IP);
		if (requested_ip_opt)
		{
			move_from_unaligned32(requested_nip, requested_ip_opt);
		}

		switch (state[0])
		{

			case DHCPDISCOVER:
			zlog_err(MODULE_DHCP, "received %s", "DISCOVER");

			dhcpd_send_offer(&packet, static_lease_nip, lease, requested_ip_opt,
					arpping_ms, ifindex);
			break;

			case DHCPREQUEST:
			zlog_err(MODULE_DHCP, "received %s", "REQUEST");
			/* RFC 2131:

			 o DHCPREQUEST generated during SELECTING state:

			 Client inserts the address of the selected server in 'server
			 identifier', 'ciaddr' MUST be zero, 'requested IP address' MUST be
			 filled in with the yiaddr value from the chosen DHCPOFFER.

			 Note that the client may choose to collect several DHCPOFFER
			 messages and select the "best" offer.  The client indicates its
			 selection by identifying the offering server in the DHCPREQUEST
			 message.  If the client receives no acceptable offers, the client
			 may choose to try another DHCPDISCOVER message.  Therefore, the
			 servers may not receive a specific DHCPREQUEST from which they can
			 decide whether or not the client has accepted the offer.

			 o DHCPREQUEST generated during INIT-REBOOT state:

			 'server identifier' MUST NOT be filled in, 'requested IP address'
			 option MUST be filled in with client's notion of its previously
			 assigned address. 'ciaddr' MUST be zero. The client is seeking to
			 verify a previously allocated, cached configuration. Server SHOULD
			 send a DHCPNAK message to the client if the 'requested IP address'
			 is incorrect, or is on the wrong network.

			 Determining whether a client in the INIT-REBOOT state is on the
			 correct network is done by examining the contents of 'giaddr', the
			 'requested IP address' option, and a database lookup. If the DHCP
			 server detects that the client is on the wrong net (i.e., the
			 result of applying the local subnet mask or remote subnet mask (if
			 'giaddr' is not zero) to 'requested IP address' option value
			 doesn't match reality), then the server SHOULD send a DHCPNAK
			 message to the client.

			 If the network is correct, then the DHCP server should check if
			 the client's notion of its IP address is correct. If not, then the
			 server SHOULD send a DHCPNAK message to the client. If the DHCP
			 server has no record of this client, then it MUST remain silent,
			 and MAY output a warning to the network administrator. This
			 behavior is necessary for peaceful coexistence of non-
			 communicating DHCP servers on the same wire.

			 If 'giaddr' is 0x0 in the DHCPREQUEST message, the client is on
			 the same subnet as the server.  The server MUST broadcast the
			 DHCPNAK message to the 0xffffffff broadcast address because the
			 client may not have a correct network address or subnet mask, and
			 the client may not be answering ARP requests.

			 If 'giaddr' is set in the DHCPREQUEST message, the client is on a
			 different subnet.  The server MUST set the broadcast bit in the
			 DHCPNAK, so that the relay agent will broadcast the DHCPNAK to the
			 client, because the client may not have a correct network address
			 or subnet mask, and the client may not be answering ARP requests.

			 o DHCPREQUEST generated during RENEWING state:

			 'server identifier' MUST NOT be filled in, 'requested IP address'
			 option MUST NOT be filled in, 'ciaddr' MUST be filled in with
			 client's IP address. In this situation, the client is completely
			 configured, and is trying to extend its lease. This message will
			 be unicast, so no relay agents will be involved in its
			 transmission.  Because 'giaddr' is therefore not filled in, the
			 DHCP server will trust the value in 'ciaddr', and use it when
			 replying to the client.

			 A client MAY choose to renew or extend its lease prior to T1.  The
			 server may choose not to extend the lease (as a policy decision by
			 the network administrator), but should return a DHCPACK message
			 regardless.

			 o DHCPREQUEST generated during REBINDING state:

			 'server identifier' MUST NOT be filled in, 'requested IP address'
			 option MUST NOT be filled in, 'ciaddr' MUST be filled in with
			 client's IP address. In this situation, the client is completely
			 configured, and is trying to extend its lease. This message MUST
			 be broadcast to the 0xffffffff IP broadcast address.  The DHCP
			 server SHOULD check 'ciaddr' for correctness before replying to
			 the DHCPREQUEST.

			 The DHCPREQUEST from a REBINDING client is intended to accommodate
			 sites that have multiple DHCP servers and a mechanism for
			 maintaining consistency among leases managed by multiple servers.
			 A DHCP server MAY extend a client's lease only if it has local
			 administrative authority to do so.
			 */
			if (!requested_ip_opt)
			{
				requested_nip = packet.ciaddr;
				if (requested_nip == 0)
				{
					zlog_err(MODULE_DHCP,
							"no requested IP and no ciaddr, ignoring");
					break;
				}
			}
			if (lease && requested_nip == lease->lease_nip)
			{
				/* client requested or configured IP matches the lease.
				 * ACK it, and bump lease expiration time. */
				dhcpd_send_ack(&packet, lease->lease_nip, ifindex);
				break;
			}
			/* No lease for this MAC, or lease IP != requested IP */

			if (server_id_opt /* client is in SELECTING state */
					|| requested_ip_opt /* client is in INIT-REBOOT state */
			)
			{
				/* "No, we don't have this IP for you" */
				dhcpd_send_nak(&packet, ifindex);
			} /* else: client is in RENEWING or REBINDING, do not answer */

			break;

			case DHCPDECLINE:
			/* RFC 2131:
			 * "If the server receives a DHCPDECLINE message,
			 * the client has discovered through some other means
			 * that the suggested network address is already
			 * in use. The server MUST mark the network address
			 * as not available and SHOULD notify the local
			 * sysadmin of a possible configuration problem."
			 *
			 * SERVER_ID must be present,
			 * REQUESTED_IP must be present,
			 * chaddr must be filled in,
			 * ciaddr must be 0 (we do not check this)
			 */
			zlog_err(MODULE_DHCP, "received %s", "DECLINE");
			if (server_id_opt && requested_ip_opt && lease /* chaddr matches this lease */
					&& requested_nip == lease->lease_nip)
			{
				memset(lease->lease_mac, 0, sizeof(lease->lease_mac));
				lease->expires = time(NULL) + config->decline_time;
			}
			break;

			case DHCPRELEASE:
			/* "Upon receipt of a DHCPRELEASE message, the server
			 * marks the network address as not allocated."
			 *
			 * SERVER_ID must be present,
			 * REQUESTED_IP must not be present (we do not check this),
			 * chaddr must be filled in,
			 * ciaddr must be filled in
			 */
			zlog_err(MODULE_DHCP, "received %s", "RELEASE");
			if (server_id_opt && lease /* chaddr matches this lease */
					&& packet.ciaddr == lease->lease_nip)
			{
				lease->expires = time(NULL);
			}
			break;

			case DHCPINFORM:
			zlog_err(MODULE_DHCP, "received %s", "INFORM");
			dhcpd_send_inform(&packet, ifindex);
			break;
		}
	}
	ret0: retval = 0;
	ret:
	/*if (config->pidfile) - config->pidfile is never NULL */
	//remove_pidfile(config->pidfile);
	return retval;
}

int udhcpd_main(int argc UNUSED_PARAM, char **argv)
{
	int server_socket = -1, retval;
	ospl_uint8 *state;
	unsigned timeout_end;
	unsigned num_ips;
	unsigned opt;
	struct option_set *option;
	char *str_I = str_I;
	const char *str_a = "2000";
	unsigned arpping_ms;
	char *str_P;

	//setup_common_bufsiz();

	SERVER_PORT = 67;
	CLIENT_PORT = 68;
#if 0
	opt = getopt32(argv, "^"
			"fSI:va:"IF_FEATURE_UDHCP_PORT("P:")
			"\0"
#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
			"vv"
#endif
			, &str_I
			, &str_a
			IF_FEATURE_UDHCP_PORT(, &str_P)
			IF_UDHCP_VERBOSE(, &dhcp_verbose)
	);
	if (!(opt & 1))
	{ /* no -f */
		bb_daemonize_or_rexec(0, argv);
		logmode = LOGMODE_NONE;
	}
#endif
	/* update argv after the possible vfork+exec in daemonize */
	argv += optind;
#if 0
	if (opt & 2)
	{ /* -S */
		openlog(applet_name, LOG_PID, LOG_DAEMON);
		logmode |= LOGMODE_SYSLOG;
	}
	if (opt & 4)
	{ /* -I */
		len_and_sockaddr *lsa = xhost_and_af2sockaddr(str_I, 0, AF_INET);
		server_config.server_nip = lsa->u.sin.sin_addr.s_addr;
		free(lsa);
	}
#endif
#if ENABLE_FEATURE_UDHCP_PORT
	if (opt & 32)
	{ /* -P */
		SERVER_PORT = xatou16(str_P);
		CLIENT_PORT = SERVER_PORT + 1;
	}
#endif
	arpping_ms = atoi(str_a);

	/* Would rather not do read_config before daemonization -
	 * otherwise NOMMU machines will parse config twice */
	read_config(argv[0] ? argv[0] : DHCPD_CONF_FILE);
	/* prevent poll timeout overflow */
	if (server_config.auto_time > INT_MAX / 1000)
	server_config.auto_time = INT_MAX / 1000;

	/* Make sure fd 0,1,2 are open */
	//bb_sanitize_stdio();
	/* Equivalent of doing a fflush after every \n */
	//setlinebuf(stdout);
	/* Create pidfile */
	//write_pidfile(server_config.pidfile);
	/* if (!..) bb_perror_msg("can't create pidfile %s", pidfile); */

	zlog_err(MODULE_DHCP,"started, v"BB_VER);

	option = udhcp_find_option(server_config.options, DHCP_LEASE_TIME);
	server_config.max_lease_sec = DEFAULT_LEASE_TIME;
	if (option)
	{
		move_from_unaligned32(server_config.max_lease_sec, option->data + OPT_DATA);
		server_config.max_lease_sec = ntohl(server_config.max_lease_sec);
	}

	/* Sanity check */
	num_ips = server_config.end_ip - server_config.start_ip + 1;
	if (server_config.max_leases > num_ips)
	{
		zlog_err(MODULE_DHCP,"max_leases=%u is too big, setting to %u",
				(unsigned)server_config.max_leases, num_ips);
		server_config.max_leases = num_ips;
	}

	g_leases = malloc(server_config.max_leases * sizeof(g_leases[0]));
	read_leases(server_config.lease_file);

	if (udhcp_read_interface(server_config.interface,
					&server_config.ifindex,
					(server_config.server_nip == 0 ? &server_config.server_nip : NULL),
					server_config.server_mac)
	)
	{
		retval = 1;
		goto ret;
	}

	/* Setup the signal pipe */
	//udhcp_sp_setup();
	continue_with_autotime:
	timeout_end = monotonic_sec() + server_config.auto_time;
	while (1)
	{ /* loop until universe collapses */
		struct pollfd pfds[1];
		struct dhcp_packet packet;
		int bytes;
		int tv;
		ospl_uint8 *server_id_opt;
		ospl_uint8 *requested_ip_opt;
		ospl_uint32  requested_nip = requested_nip; /* for compiler */
		ospl_uint32  static_lease_nip;
		struct dyn_lease *lease, fake_lease;

		if (server_socket < 0)
		{
			server_socket = udhcp_listen_socket(/*INADDR_ANY,*/SERVER_PORT,
					server_config.interface);
		}

		udhcp_sp_fd_set(pfds, server_socket);

		new_tv:
		tv = -1;
		if (server_config.auto_time)
		{
			tv = timeout_end - monotonic_sec();
			if (tv <= 0)
			{
				write_leases:
				write_leases();
				goto continue_with_autotime;
			}
			tv *= 1000;
		}

		/* Block here waiting for either signal or packet */
		retval = poll(pfds, 1, tv);
		if (retval <= 0)
		{
			if (retval == 0)
			goto write_leases;
			if (errno == EINTR)
			goto new_tv;
			/* < 0 and not EINTR: should not happen */
			//bb_perror_msg_and_die("poll");
		}

		/*		if (pfds[0].revents) switch (udhcp_sp_read()) {
		 case SIGUSR1:
		 zlog_err(MODULE_DHCP,"received %s", "SIGUSR1");
		 write_leases();
		 goto continue_with_autotime;
		 case SIGTERM:
		 zlog_err(MODULE_DHCP,"received %s", "SIGTERM");
		 write_leases();
		 goto ret0;
		 }*/

		/* Is it a packet? */
		if (!pfds[0].revents)
		continue; /* no */

		/* Note: we do not block here, we block on poll() instead.
		 * Blocking here would prevent SIGTERM from working:
		 * socket read inside this call is restarted on caught signals.
		 */
		bytes = udhcp_recv_kernel_packet(&packet, server_socket);
		if (bytes < 0)
		{
			/* bytes can also be -2 ("bad packet data") */
			if (bytes == -1 && errno != EINTR)
			{
				zlog_err(MODULE_DHCP,"read error: "STRERROR_FMT", reopening socket" STRERROR_ERRNO);
				close(server_socket);
				server_socket = -1;
			}
			continue;
		}
		if (packet.hlen != 6)
		{
			zlog_err(MODULE_DHCP,"MAC length != 6, ignoring packet");
			continue;
		}
		if (packet.op != BOOTREQUEST)
		{
			zlog_err(MODULE_DHCP,"not a REQUEST, ignoring packet");
			continue;
		}
		state = udhcp_get_option(&packet, DHCP_MESSAGE_TYPE);
		if (state == NULL || state[0] < DHCP_MINTYPE || state[0] > DHCP_MAXTYPE)
		{
			zlog_err(MODULE_DHCP,"no or bad message type option, ignoring packet");
			continue;
		}

		/* Get SERVER_ID if present */
		server_id_opt = udhcp_get_option(&packet, DHCP_SERVER_ID);
		if (server_id_opt)
		{
			ospl_uint32  server_id_network_order;
			move_from_unaligned32(server_id_network_order, server_id_opt);
			if (server_id_network_order != server_config.server_nip)
			{
				/* client talks to somebody else */
				zlog_err(MODULE_DHCP,"server ID doesn't match, ignoring");
				continue;
			}
		}

		/* Look for a static/dynamic lease */
		static_lease_nip = dhcpd_get_static_nip_by_mac(server_config.static_leases, &packet.chaddr);
		if (static_lease_nip)
		{
			zlog_err(MODULE_DHCP,"found static lease: %x", static_lease_nip);
			memcpy(&fake_lease.lease_mac, &packet.chaddr, 6);
			fake_lease.lease_nip = static_lease_nip;
			fake_lease.expires = 0;
			lease = &fake_lease;
		}
		else
		{
			lease = dhcpd_find_lease_by_mac(packet.chaddr);
		}

		/* Get REQUESTED_IP if present */
		requested_ip_opt = udhcp_get_option(&packet, DHCP_REQUESTED_IP);
		if (requested_ip_opt)
		{
			move_from_unaligned32(requested_nip, requested_ip_opt);
		}

		switch (state[0])
		{

			case DHCPDISCOVER:
			zlog_err(MODULE_DHCP,"received %s", "DISCOVER");

			dhcpd_send_offer(&packet, static_lease_nip, lease, requested_ip_opt, arpping_ms);
			break;

			case DHCPREQUEST:
			zlog_err(MODULE_DHCP,"received %s", "REQUEST");
			/* RFC 2131:

			 o DHCPREQUEST generated during SELECTING state:

			 Client inserts the address of the selected server in 'server
			 identifier', 'ciaddr' MUST be zero, 'requested IP address' MUST be
			 filled in with the yiaddr value from the chosen DHCPOFFER.

			 Note that the client may choose to collect several DHCPOFFER
			 messages and select the "best" offer.  The client indicates its
			 selection by identifying the offering server in the DHCPREQUEST
			 message.  If the client receives no acceptable offers, the client
			 may choose to try another DHCPDISCOVER message.  Therefore, the
			 servers may not receive a specific DHCPREQUEST from which they can
			 decide whether or not the client has accepted the offer.

			 o DHCPREQUEST generated during INIT-REBOOT state:

			 'server identifier' MUST NOT be filled in, 'requested IP address'
			 option MUST be filled in with client's notion of its previously
			 assigned address. 'ciaddr' MUST be zero. The client is seeking to
			 verify a previously allocated, cached configuration. Server SHOULD
			 send a DHCPNAK message to the client if the 'requested IP address'
			 is incorrect, or is on the wrong network.

			 Determining whether a client in the INIT-REBOOT state is on the
			 correct network is done by examining the contents of 'giaddr', the
			 'requested IP address' option, and a database lookup. If the DHCP
			 server detects that the client is on the wrong net (i.e., the
			 result of applying the local subnet mask or remote subnet mask (if
			 'giaddr' is not zero) to 'requested IP address' option value
			 doesn't match reality), then the server SHOULD send a DHCPNAK
			 message to the client.

			 If the network is correct, then the DHCP server should check if
			 the client's notion of its IP address is correct. If not, then the
			 server SHOULD send a DHCPNAK message to the client. If the DHCP
			 server has no record of this client, then it MUST remain silent,
			 and MAY output a warning to the network administrator. This
			 behavior is necessary for peaceful coexistence of non-
			 communicating DHCP servers on the same wire.

			 If 'giaddr' is 0x0 in the DHCPREQUEST message, the client is on
			 the same subnet as the server.  The server MUST broadcast the
			 DHCPNAK message to the 0xffffffff broadcast address because the
			 client may not have a correct network address or subnet mask, and
			 the client may not be answering ARP requests.

			 If 'giaddr' is set in the DHCPREQUEST message, the client is on a
			 different subnet.  The server MUST set the broadcast bit in the
			 DHCPNAK, so that the relay agent will broadcast the DHCPNAK to the
			 client, because the client may not have a correct network address
			 or subnet mask, and the client may not be answering ARP requests.

			 o DHCPREQUEST generated during RENEWING state:

			 'server identifier' MUST NOT be filled in, 'requested IP address'
			 option MUST NOT be filled in, 'ciaddr' MUST be filled in with
			 client's IP address. In this situation, the client is completely
			 configured, and is trying to extend its lease. This message will
			 be unicast, so no relay agents will be involved in its
			 transmission.  Because 'giaddr' is therefore not filled in, the
			 DHCP server will trust the value in 'ciaddr', and use it when
			 replying to the client.

			 A client MAY choose to renew or extend its lease prior to T1.  The
			 server may choose not to extend the lease (as a policy decision by
			 the network administrator), but should return a DHCPACK message
			 regardless.

			 o DHCPREQUEST generated during REBINDING state:

			 'server identifier' MUST NOT be filled in, 'requested IP address'
			 option MUST NOT be filled in, 'ciaddr' MUST be filled in with
			 client's IP address. In this situation, the client is completely
			 configured, and is trying to extend its lease. This message MUST
			 be broadcast to the 0xffffffff IP broadcast address.  The DHCP
			 server SHOULD check 'ciaddr' for correctness before replying to
			 the DHCPREQUEST.

			 The DHCPREQUEST from a REBINDING client is intended to accommodate
			 sites that have multiple DHCP servers and a mechanism for
			 maintaining consistency among leases managed by multiple servers.
			 A DHCP server MAY extend a client's lease only if it has local
			 administrative authority to do so.
			 */
			if (!requested_ip_opt)
			{
				requested_nip = packet.ciaddr;
				if (requested_nip == 0)
				{
					zlog_err(MODULE_DHCP,"no requested IP and no ciaddr, ignoring");
					break;
				}
			}
			if (lease && requested_nip == lease->lease_nip)
			{
				/* client requested or configured IP matches the lease.
				 * ACK it, and bump lease expiration time. */
				dhcpd_send_ack(&packet, lease->lease_nip);
				break;
			}
			/* No lease for this MAC, or lease IP != requested IP */

			if (server_id_opt /* client is in SELECTING state */
					|| requested_ip_opt /* client is in INIT-REBOOT state */
			)
			{
				/* "No, we don't have this IP for you" */
				dhcpd_send_nak(&packet);
			} /* else: client is in RENEWING or REBINDING, do not answer */

			break;

			case DHCPDECLINE:
			/* RFC 2131:
			 * "If the server receives a DHCPDECLINE message,
			 * the client has discovered through some other means
			 * that the suggested network address is already
			 * in use. The server MUST mark the network address
			 * as not available and SHOULD notify the local
			 * sysadmin of a possible configuration problem."
			 *
			 * SERVER_ID must be present,
			 * REQUESTED_IP must be present,
			 * chaddr must be filled in,
			 * ciaddr must be 0 (we do not check this)
			 */
			zlog_err(MODULE_DHCP,"received %s", "DECLINE");
			if (server_id_opt
					&& requested_ip_opt
					&& lease /* chaddr matches this lease */
					&& requested_nip == lease->lease_nip
			)
			{
				memset(lease->lease_mac, 0, sizeof(lease->lease_mac));
				lease->expires = time(NULL) + server_config.decline_time;
			}
			break;

			case DHCPRELEASE:
			/* "Upon receipt of a DHCPRELEASE message, the server
			 * marks the network address as not allocated."
			 *
			 * SERVER_ID must be present,
			 * REQUESTED_IP must not be present (we do not check this),
			 * chaddr must be filled in,
			 * ciaddr must be filled in
			 */
			zlog_err(MODULE_DHCP,"received %s", "RELEASE");
			if (server_id_opt
					&& lease /* chaddr matches this lease */
					&& packet.ciaddr == lease->lease_nip
			)
			{
				lease->expires = time(NULL);
			}
			break;

			case DHCPINFORM:
			zlog_err(MODULE_DHCP,"received %s", "INFORM");
			dhcpd_send_inform(&packet);
			break;
		}
	}
	ret0:
	retval = 0;
	ret:
	/*if (server_config.pidfile) - server_config.pidfile is never NULL */
	//remove_pidfile(server_config.pidfile);
	return retval;
}

#endif

