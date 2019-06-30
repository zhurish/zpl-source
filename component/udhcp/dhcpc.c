/* vi: set sw=4 ts=4: */
/*
 * udhcp client
 * Russ Dill <Russ.Dill@asu.edu> July 2001
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
//applet:IF_UDHCPC(APPLET(udhcpc, BB_DIR_SBIN, BB_SUID_DROP))
//kbuild:lib-$(CONFIG_UDHCPC) += common.o packet.o signalpipe.o socket.o
//kbuild:lib-$(CONFIG_UDHCPC) += dhcpc.o
//kbuild:lib-$(CONFIG_FEATURE_UDHCPC_ARPING) += arpping.o
//kbuild:lib-$(CONFIG_FEATURE_UDHCP_RFC3397) += domain_codec.o
#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "prefix.h"
#include "sigevent.h"
#include "version.h"
#include "log.h"
#include "getopt.h"
#include "eloop.h"
#include "os_start.h"
#include "os_module.h"

/* Override ENABLE_FEATURE_PIDFILE - ifupdown needs our pidfile to always exist */
#define WANT_PIDFILE 1
#include "dhcp_def.h"
#include "dhcp_lease.h"
#include "dhcp_util.h"
#include "dhcp_main.h"
#include "dhcpc.h"

#include <netinet/if_ether.h>
#include <linux/filter.h>
#include <linux/if_packet.h>

/*
#ifndef PACKET_AUXDATA
# define PACKET_AUXDATA 8
struct tpacket_auxdata {
	uint32_t tp_status;
	uint32_t tp_len;
	uint32_t tp_snaplen;
	uint16_t tp_mac;
	uint16_t tp_net;
	uint16_t tp_vlan_tci;
	uint16_t tp_padding;
};
#endif
*/

static int udhcpc_read_thread(struct eloop *eloop);
static int udhcpc_discover_event(struct eloop *eloop);
static int udhcpc_client_socket(int ifindex);
static int udhcpc_state_mode_change(client_interface_t * ifter, int mode);

/* Call a script with a par file and env vars */
/*static void udhcp_run_script(struct dhcp_packet *packet, const char *name)
{
	return;
}*/

/*** Sending/receiving packets ***/

static uint32_t udhcpc_get_xid(void) {
	return rand();
}

/* Initialize the packet with the proper defaults */
static void udhcpc_packet_init(client_interface_t *inter, struct dhcp_packet *packet,
		char type) {
	uint16_t secs;

	/* Fill in: op, htype, hlen, cookie fields; message type option: */
	udhcp_header_init(packet, type);

	//packet->xid = udhcpc_get_xid();

	inter->last_secs = os_monotonic_time();
	if (inter->first_secs == 0)
		inter->first_secs = inter->last_secs;
	secs = inter->last_secs - inter->first_secs;
	packet->secs = htons(secs);

	memcpy(packet->chaddr, inter->client_mac, 6);
	//if (inter->clientid)
	//	udhcp_add_simple_option_value(packet, DHCP_CLIENT_ID, strlen(inter->clientid), inter->clientid);
		//udhcp_add_binary_option(packet, inter->clientid);
}


/*static uint8_t* alloc_dhcp_option(int code, const char *str, int extra) {
	uint8_t *storage = NULL;
	int len = 0;
	if (str)
		len = strnlen(str, 255);
	storage = malloc(len + extra + OPT_DATA);
	storage[OPT_CODE] = code;
	storage[OPT_LEN] = len + extra;
	if (str && len > 0)
		memcpy(storage + extra + OPT_DATA, str, len);
	return storage;
}*/


static void udhcpc_add_client_options(client_interface_t *inter,
		struct dhcp_packet *packet) {
	int i = 0, end = 0, len = 0;
	//udhcp_add_simple_option(packet, DHCP_MAX_SIZE, htons(IP_UDP_DHCP_SIZE));

	/* Add a "param req" option with the list of options we'd like to have
	 * from stubborn DHCP servers. Pull the data from the struct in common.c.
	 * No bounds checking because it goes towards the head of the packet. */
	dhcp_option_packet(inter->options, packet->options, sizeof(packet->options));

	end = udhcp_end_option(packet->options);
	len = 0;
	for (i = 1; i < DHCP_END; i++) {
		if (inter->opt_mask[i]) {
			packet->options[end + OPT_DATA + len] = i;
			len++;
		}
	}
	if (len) {
		packet->options[end + OPT_CODE] = DHCP_PARAM_REQ;
		packet->options[end + OPT_LEN] = len;
		packet->options[end + OPT_DATA + len] = DHCP_END;
	}

	//int dhcp_option_packet_set_value(char *data, int len, uint8_t code, uint32_t oplen, uint8_t *opt)

	/* Request broadcast replies if we have no IP addr */
	if (/*(option_mask32 & OPT_B) && */packet->ciaddr == 0)
		packet->flags |= htons(BROADCAST_FLAG);

	// This will be needed if we remove -V VENDOR_STR in favor of
	// -x vendor:VENDOR_STR
	//if (!udhcp_find_option(packet.options, DHCP_VENDOR))
	//	/* not set, set the default vendor ID */
	//	...add (DHCP_VENDOR, "udhcp "BB_VER) opt...
}

/* RFC 2131
 * 4.4.4 Use of broadcast and unicast
 *
 * The DHCP client broadcasts DHCPDISCOVER, DHCPREQUEST and DHCPINFORM
 * messages, unless the client knows the address of a DHCP server.
 * The client unicasts DHCPRELEASE messages to the server. Because
 * the client is declining the use of the IP address supplied by the server,
 * the client broadcasts DHCPDECLINE messages.
 *
 * When the DHCP client knows the address of a DHCP server, in either
 * INIT or REBOOTING state, the client may use that address
 * in the DHCPDISCOVER or DHCPREQUEST rather than the IP broadcast address.
 * The client may also use unicast to send DHCPINFORM messages
 * to a known DHCP server. If the client receives no response to DHCP
 * messages sent to the IP address of a known DHCP server, the DHCP
 * client reverts to using the IP broadcast address.
 */

static int udhcpc_raw_packet_bcast(int sock,
		struct dhcp_packet *packet, uint32_t src_nip, u_int32 ifindex) {
	struct udhcp_packet_cmd source;
	struct udhcp_packet_cmd dest;

	source.ip = src_nip;
	source.port = dhcp_global_config.client_port;//CLIENT_PORT;

	dest.ip = INADDR_BROADCAST;
	dest.port = dhcp_global_config.server_port;//SERVER_PORT;
	return udhcp_send_raw_packet(sock, packet, &source, &dest, DHCP_MAC_BCAST_ADDR, ifindex);
}

static int udhcpc_packet_bcast_ucast(int sock, struct dhcp_packet *packet, uint32_t ciaddr,
		uint32_t server, u_int32 ifindex) {
	if (server) {
		struct udhcp_packet_cmd source;
		struct udhcp_packet_cmd dest;

		source.ip = ciaddr;
		source.port = dhcp_global_config.client_port;//CLIENT_PORT;

		dest.ip = server;
		dest.port = dhcp_global_config.server_port;//SERVER_PORT;

		return udhcp_send_udp_packet(sock, packet, &source, &dest);
	}
	return udhcpc_raw_packet_bcast(sock, packet, ciaddr, ifindex);
}

/* Broadcast a DHCP discover packet to the network, with an optionally requested IP */
/* NOINLINE: limit stack usage in caller */
static int udhcpc_send_discover(client_interface_t *inter,
		uint32_t requested) {
	struct dhcp_packet packet;

	/* Fill in: op, htype, hlen, cookie, chaddr fields,
	 * random xid field (we override it below),
	 * client-id option (unless -C), message type option:
	 */
	udhcpc_packet_init(inter, &packet, DHCPDISCOVER);

	packet.xid = htonl(inter->state.xid);
	if (requested)
		udhcp_add_simple_option(&packet, DHCP_REQUESTED_IP, requested);

	/* Add options: maxsize,
	 * optionally: hostname, fqdn, vendorclass,
	 * "param req" option according to -O, options specified with -x
	 */
	udhcpc_add_client_options(inter, &packet);

	//if (msgs++ < 3)
	zlog_debug(ZLOG_DHCP, "sending %s", "discover");
	if(inter->state.mode == DHCP_RAW_MODE)
		return udhcpc_raw_packet_bcast(inter->sock, &packet,
			INADDR_ANY, inter->ifindex);
	return ERROR;
}

/* Broadcast a DHCP request message */
/* RFC 2131 3.1 paragraph 3:
 * "The client _broadcasts_ a DHCPREQUEST message..."
 */
/* NOINLINE: limit stack usage in caller */
static int udhcpc_send_request(client_interface_t *inter, uint32_t server,
		uint32_t requested) {
	struct dhcp_packet packet;
	struct in_addr temp_addr;

	/*
	 * RFC 2131 4.3.2 DHCPREQUEST message
	 * ...
	 * If the DHCPREQUEST message contains a 'server identifier'
	 * option, the message is in response to a DHCPOFFER message.
	 * Otherwise, the message is a request to verify or extend an
	 * existing lease. If the client uses a 'client identifier'
	 * in a DHCPREQUEST message, it MUST use that same 'client identifier'
	 * in all subsequent messages. If the client included a list
	 * of requested parameters in a DHCPDISCOVER message, it MUST
	 * include that list in all subsequent messages.
	 */
	/* Fill in: op, htype, hlen, cookie, chaddr fields,
	 * random xid field (we override it below),
	 * client-id option (unless -C), message type option:
	 */
	udhcpc_packet_init(inter, &packet, DHCPREQUEST);

	packet.xid = htonl(inter->state.xid);
	udhcp_add_simple_option(&packet, DHCP_REQUESTED_IP, requested);

	udhcp_add_simple_option(&packet, DHCP_SERVER_ID, server);

	/* Add options: maxsize,
	 * optionally: hostname, fqdn, vendorclass,
	 * "param req" option according to -O, and options specified with -x
	 */
	udhcpc_add_client_options(inter, &packet);

	temp_addr.s_addr = requested;
	zlog_debug(ZLOG_DHCP, "sending select for %s", inet_ntoa(temp_addr));
	if(inter->state.mode == DHCP_RAW_MODE)
		return udhcpc_raw_packet_bcast(inter->sock, &packet,
			INADDR_ANY, inter->ifindex);
	return ERROR;
}

/* Unicast or broadcast a DHCP renew message */
/* NOINLINE: limit stack usage in caller */
static int udhcpc_send_renew(client_interface_t *inter, uint32_t server,
		uint32_t ciaddr) {
	struct dhcp_packet packet;
	struct in_addr temp_addr;

	/*
	 * RFC 2131 4.3.2 DHCPREQUEST message
	 * ...
	 * DHCPREQUEST generated during RENEWING state:
	 *
	 * 'server identifier' MUST NOT be filled in, 'requested IP address'
	 * option MUST NOT be filled in, 'ciaddr' MUST be filled in with
	 * client's IP address. In this situation, the client is completely
	 * configured, and is trying to extend its lease. This message will
	 * be unicast, so no relay agents will be involved in its
	 * transmission.  Because 'giaddr' is therefore not filled in, the
	 * DHCP server will trust the value in 'ciaddr', and use it when
	 * replying to the client.
	 */
	/* Fill in: op, htype, hlen, cookie, chaddr fields,
	 * random xid field (we override it below),
	 * client-id option (unless -C), message type option:
	 */
	udhcpc_packet_init(inter, &packet, DHCPREQUEST);

	packet.xid = htonl(inter->state.xid);
	packet.ciaddr = ciaddr;

	udhcp_add_simple_option(&packet, DHCP_REQUESTED_IP, ciaddr);

	udhcp_add_simple_option(&packet, DHCP_SERVER_ID, server);
	/* Add options: maxsize,
	 * optionally: hostname, fqdn, vendorclass,
	 * "param req" option according to -O, and options specified with -x
	 */
	udhcpc_add_client_options(inter, &packet);

	temp_addr.s_addr = server;
	zlog_debug(ZLOG_DHCP, "sending renew to %s", inet_ntoa(temp_addr));
	//return udhcpc_packet_bcast_ucast(inter->sock, &packet, ciaddr, server, inter->ifindex);
	if(inter->state.mode == DHCP_RAW_MODE)
		return udhcpc_packet_bcast_ucast(inter->sock, &packet, ciaddr, server, inter->ifindex);
	else
		return udhcpc_packet_bcast_ucast(inter->udp_sock, &packet, ciaddr, server, inter->ifindex);
}

#if ENABLE_FEATURE_UDHCPC_ARPING
/* Broadcast a DHCP decline message */
/* NOINLINE: limit stack usage in caller */
static int udhcpc_send_decline(client_interface_t *inter, uint32_t server, uint32_t requested)
{
	struct dhcp_packet packet;

	/* Fill in: op, htype, hlen, cookie, chaddr, random xid fields,
	 * client-id option (unless -C), message type option:
	 */
	udhcpc_packet_init(inter, &packet, DHCPDECLINE);

	/* RFC 2131 says DHCPDECLINE's xid is randomly selected by client,
	 * but in case the server is buggy and wants DHCPDECLINE's xid
	 * to match the xid which started entire handshake,
	 * we use the same xid we used in initial DHCPDISCOVER:
	 */
	packet.xid = htonl(inter->state.xid);
	/* DHCPDECLINE uses "requested ip", not ciaddr, to store offered IP */
	udhcp_add_simple_option(&packet, DHCP_REQUESTED_IP, requested);

	udhcp_add_simple_option(&packet, DHCP_SERVER_ID, server);
	udhcpc_add_client_options(inter, &packet);
	zlog_debug(ZLOG_DHCP,"sending %s", "decline");
	if(inter->state.mode == DHCP_RAW_MODE)
		return udhcpc_packet_bcast_ucast(inter->sock, &packet, requested, server, inter->ifindex);
	else
		return udhcpc_packet_bcast_ucast(inter->udp_sock, &packet, requested, server, inter->ifindex);
	//return udhcpc_raw_packet_bcast(inter->sock, &packet, INADDR_ANY, inter->ifindex);
}
#endif

/* Unicast a DHCP release message */
static int udhcpc_send_release(client_interface_t *inter, uint32_t server,
		uint32_t ciaddr) {
	struct dhcp_packet packet;

	/* Fill in: op, htype, hlen, cookie, chaddr, random xid fields,
	 * client-id option (unless -C), message type option:
	 */
	udhcpc_packet_init(inter, &packet, DHCPRELEASE);

	/* DHCPRELEASE uses ciaddr, not "requested ip", to store IP being released */
	packet.ciaddr = ciaddr;
	packet.xid = htonl(inter->state.xid);
	udhcp_add_simple_option(&packet, DHCP_SERVER_ID, server);

	udhcpc_add_client_options(inter, &packet);

	zlog_debug(ZLOG_DHCP, "sending %s", "release");
	/* Note: normally we unicast here since "server" is not zero.
	 * However, there _are_ people who run "address-less" DHCP servers,
	 * and reportedly ISC dhcp client and Windows allow that.
	 */
	if(inter->state.mode == DHCP_RAW_MODE)
		return udhcpc_packet_bcast_ucast(inter->sock, &packet, ciaddr, server, inter->ifindex);
	else
		return udhcpc_packet_bcast_ucast(inter->udp_sock, &packet, ciaddr, server, inter->ifindex);
	//return udhcpc_packet_bcast_ucast(inter->sock, &packet, ciaddr, server, inter->ifindex);
}

static int udhcpc_send_inform(client_interface_t *inter, uint32_t server,
		uint32_t ciaddr) {
	struct dhcp_packet packet;

	/* Fill in: op, htype, hlen, cookie, chaddr, random xid fields,
	 * client-id option (unless -C), message type option:
	 */
	udhcpc_packet_init(inter, &packet, DHCPINFORM);

	/* DHCPRELEASE uses ciaddr, not "requested ip", to store IP being released */
	packet.ciaddr = ciaddr;
	packet.xid = htonl(inter->state.xid);
	udhcp_add_simple_option(&packet, DHCP_SERVER_ID, server);

	udhcpc_add_client_options(inter, &packet);

	zlog_debug(ZLOG_DHCP, "sending %s", "release");
	/* Note: normally we unicast here since "server" is not zero.
	 * However, there _are_ people who run "address-less" DHCP servers,
	 * and reportedly ISC dhcp client and Windows allow that.
	 */
	if(inter->state.mode == DHCP_RAW_MODE)
		return udhcpc_packet_bcast_ucast(inter->sock, &packet, ciaddr, server, inter->ifindex);
	else
		return udhcpc_packet_bcast_ucast(inter->udp_sock, &packet, ciaddr, server, inter->ifindex);
}
/************************************************************************************/
static int udhcpc_client_socket(int ifindex)
{
	int fd = 0;
	fd = udhcp_raw_socket();
	if(fd > 0)
	{
		//setsockopt_ifindex (AF_INET, fd, 1);
		if(udhcp_client_socket_bind( fd,  ifindex) == OK)
		{
			udhcp_client_socket_filter( fd,  68);
			zlog_err(ZLOG_DHCP, "created raw socket");
			return fd;
		}
		zlog_err(ZLOG_DHCP, "Can not bind raw socket(%s)", strerror(errno));
		close(fd);
		return 0;
	}
	return 0;
}

/************************************************************************************/
/************************************************************************************/
/* Called only on SIGUSR1 */

#if 0
static int perform_renew(client_interface_t * ifter) {
	zlog_err(ZLOG_DHCP, "performing DHCP renew");
	switch (ifter->state) {
	case BOUND:
		udhcp_client_socket_mode(ifter, LISTEN_KERNEL);
	case RENEWING:
	case REBINDING:
		ifter->state = RENEW_REQUESTED;
		break;
	case RENEW_REQUESTED: /* impatient are we? fine, square 1 */
	case REQUESTING:
	case RELEASED:
		udhcp_client_socket_mode(ifter, LISTEN_RAW);
		ifter->state = INIT_SELECTING;
		break;
	case INIT_SELECTING:
		break;
	}
	return OK;
}
#endif

#if 0
static int perform_release(client_interface_t * ifter, uint32_t server_addr,
		uint32_t requested_ip)
{
	char buffer[sizeof("255.255.255.255")];
	struct in_addr temp_addr;

	/* send release packet */
	if (ifter->state == BOUND || ifter->state == RENEWING
			|| ifter->state == REBINDING || ifter->state == RENEW_REQUESTED) {
		temp_addr.s_addr = server_addr;
		strcpy(buffer, inet_ntoa(temp_addr));
		temp_addr.s_addr = requested_ip;
		zlog_err(ZLOG_DHCP, "unicasting a release of %s to %s",
				inet_ntoa(temp_addr), buffer);
		udhcpc_send_release(ifter, server_addr, requested_ip); /* unicast */
	}
	zlog_err(ZLOG_DHCP, "entering released state");
	/*
	 * We can be here on: SIGUSR2,
	 * or on exit (SIGTERM) and -R "release on quit" is specified.
	 * Users requested to be notified in all cases, even if not in one
	 * of the states above.
	 */
	udhcp_run_script(NULL, "deconfig");

	ifter->state = RELEASED;
	return OK;
}
#endif

static int udhcp_client_explain_lease(struct dhcp_packet *packet,
		client_interface_t * ifter)
{
	int optlen = 0;
	u_int8 *temp = NULL;
	ifter->lease.lease_address = packet->yiaddr;
	dhcp_option_get_address(packet->options, DHCP_SERVER_ID, &ifter->lease.server_address);
	dhcp_option_get_address(packet->options, DHCP_SUBNET, &ifter->lease.lease_netmask);
	dhcp_option_get_address(packet->options, DHCP_BROADCAST, &ifter->lease.lease_broadcast);

	temp = (const char*)udhcp_get_option(packet, DHCP_DOMAIN_NAME, &optlen);
	if (temp)
	{
		if(optlen < sizeof(ifter->lease.domain_name))
		{
			memset(ifter->lease.domain_name, 0, sizeof(ifter->lease.domain_name));
			strncpy(ifter->lease.domain_name, temp, optlen);
		}
	}

	temp = (const char*)udhcp_get_option(packet, DHCP_ROUTER, &optlen);
	if (temp)
	{
		udhcp_str2nip(temp, &ifter->lease.lease_gateway);
		if(optlen > 4)
			udhcp_str2nip(temp + 4, &ifter->lease.lease_gateway2);
	}
	temp = (const char*)udhcp_get_option(packet, DHCP_DNS_SERVER, &optlen);
	if (temp)
	{
		udhcp_str2nip(temp, &ifter->lease.lease_dns1);
		if(optlen > 4)
			udhcp_str2nip(temp + 4, &ifter->lease.lease_dns2);
	}
	temp = udhcp_get_option(packet, DHCP_LEASE_TIME, NULL);
	if (!temp) {
		zlog_warn(ZLOG_DHCP, "no lease time with ACK, using 1 hour lease");
		ifter->lease.lease_gateway = 60 * 60;
	}
	else
	{
		move_from_unaligned32(ifter->lease.expires, temp);
		ifter->lease.expires = ntohl(ifter->lease.expires);
		if (ifter->lease.expires < 0x10)
			ifter->lease.expires = 0x10;
		if (ifter->lease.expires > 0x7fffffff / 1000)
			ifter->lease.expires = 0x7fffffff / 1000;
	}
	temp = (const char*)udhcp_get_option(packet, DHCP_TIME_SERVER, &optlen);
	if (temp)
	{
		udhcp_str2nip(temp, &ifter->lease.lease_timer1);
		if(optlen > 4)
			udhcp_str2nip(temp + 4, &ifter->lease.lease_timer2);
	}

	temp = (const char*)udhcp_get_option(packet, DHCP_LOG_SERVER, &optlen);
	if (temp)
	{
		udhcp_str2nip(temp, &ifter->lease.lease_log1);
		if(optlen > 4)
			udhcp_str2nip(temp + 4, &ifter->lease.lease_log2);
	}

	temp = (const char*)udhcp_get_option(packet, DHCP_NTP_SERVER, &optlen);
	if (temp)
	{
		udhcp_str2nip(temp, &ifter->lease.lease_ntp1);
		if(optlen > 4)
			udhcp_str2nip(temp + 4, &ifter->lease.lease_ntp2);
	}
	dhcp_option_get_8bit(packet->options, DHCP_IP_TTL, &ifter->lease.lease_ttl);
	dhcp_option_get_8bit(packet->options, DHCP_MTU, &ifter->lease.lease_mtu);

	temp = (const char*)udhcp_get_option(packet, DHCP_TFTP_SERVER_NAME, &optlen);
	if (temp)
	{
		if(optlen < sizeof(ifter->lease.tftp_srv_name))
		{
			memset(ifter->lease.tftp_srv_name, 0, sizeof(ifter->lease.tftp_srv_name));
			strncpy(ifter->lease.tftp_srv_name, temp, optlen);
		}
	}

	ifter->lease.starts = os_monotonic_time();;
	ifter->lease.ends = ifter->lease.starts + ifter->lease.expires;

	//ifter->lease.server_address = ifter->lease.server_address;
	ifter->lease.gateway_address = packet->siaddr_nip;
	ifter->lease.ciaddr = packet->ciaddr; /* client IP (if client is in BOUND, RENEW or REBINDING state) */
	/* IP address of next server to use in bootstrap, returned in DHCPOFFER, DHCPACK by server */
	ifter->lease.siaddr_nip = packet->siaddr_nip; /*若 client 需要透过网络开机，从 server 送出之 DHCP OFFER、DHCPACK、DHCPNACK封包中，
							此栏填写开机程序代码所在 server 之地址*/
	ifter->lease.gateway_nip = packet->gateway_nip; /* relay agent IP address */

	return OK;
}


static int udhcp_client_send_handle(client_interface_t * ifter, int event)
{
	switch (event)
	{
	case DHCP_INIT:
		/* broadcast */
		if(ifter->state.dis_cnt < ifter->state.dis_retries)
		{
			if (ifter->state.dis_cnt == 0)
				ifter->state.xid = udhcpc_get_xid();
			udhcpc_send_discover(ifter, ifter->lease.lease_address);
			ifter->state.dis_cnt++;
		}
		else
		{
			ifter->state.state = DHCP_INIT;
			ifter->state.dis_cnt = 0;
			ifter->state.renew_timeout1 = 0;
			ifter->state.renew_timeout2 = 0;
			ifter->state.xid = 0;
			ifter->state.read_bytes = 0;

			memset(&ifter->lease, 0, sizeof(ifter->lease));

			if(ifter->t_thread)
				eloop_cancel(ifter->t_thread);
			ifter->state.xid = udhcpc_get_xid();
			ifter->t_thread = eloop_add_timer(ifter->master, udhcpc_discover_event, ifter,
					ifter->state.dis_timeout + 5);
		}
		break;
	case DHCP_REQUESTING:
		/* send broadcast request packet */
		if(ifter->state.state <= DHCP_REQUESTING)
		{
			udhcpc_send_request(ifter, ifter->lease.server_address,
					ifter->lease.lease_address);
		}
/*		if(ifter->state.state >= DHCP_BOUND)
		{
			if(ifter->state.state == DHCP_BOUND)
				ifter->state.state = DHCP_RENEWING;
			else
				ifter->state = DHCP_REBINDING;
			udhcpc_send_renew(ifter, ifter->state.xid, ifter->lease.server_address,
								ifter->lease.lease_address);
		}*/
		break;

	case DHCP_BOUND:
		break;

	case DHCP_RENEWING:
		udhcpc_send_renew(ifter, ifter->lease.server_address,
							ifter->lease.lease_address);
		ifter->state.state = DHCP_RENEWING;
		break;

	case DHCP_REBINDING:
		udhcpc_send_renew(ifter, ifter->lease.server_address,
							ifter->lease.lease_address);
		ifter->state.state = DHCP_REBINDING;
		break;

	case DHCP_DECLINE:
		udhcpc_send_decline(ifter, ifter->lease.server_address, ifter->lease.lease_address);
		ifter->state.state = DHCP_DECLINE;
		break;
	case DHCP_RELEASE:
		udhcpc_send_release(ifter, ifter->lease.server_address,
				ifter->lease.lease_address);
		ifter->state.state = DHCP_RELEASE;
		break;
	case DHCP_INFORM:
		udhcpc_send_inform(ifter, ifter->lease.server_address, ifter->lease.lease_address);
		ifter->state.state = DHCP_INFORM;
		break;
	}
	return OK;
}

static int udhcpc_discover_event(struct eloop *eloop)
{
	client_interface_t * ifter = NULL;
	ifter = ELOOP_ARG(eloop);
	if(ifter == NULL)
		return OK;
	ifter->t_thread = NULL;
	udhcp_client_send_handle(ifter, DHCP_INIT);
	ifter->t_thread = eloop_add_timer(eloop->master, udhcpc_discover_event, ifter,
			ifter->state.dis_timeout);
	return OK;
}

static int udhcpc_stop(client_interface_t * ifter)
{
	ifter->state.state = DHCP_INIT;
	//ifter->state.dis_timeout;
	//ifter->state.dis_retries;
	ifter->state.dis_cnt = 0;
	ifter->state.renew_timeout1 = 0;
	ifter->state.renew_timeout2 = 0;
	ifter->state.xid = 0;
	ifter->state.read_bytes = 0;

	memset(&ifter->lease, 0, sizeof(ifter->lease));
	return OK;
}

static int udhcpc_timeout_event(struct eloop *eloop)
{
	client_interface_t * ifter = NULL;
	ifter = ELOOP_ARG(eloop);
	if(ifter == NULL)
		return OK;
	ifter->t_thread = NULL;

	udhcpc_stop(ifter);

	udhcpc_state_mode_change(ifter, DHCP_RAW_MODE);

	ifter->t_thread = eloop_add_timer(eloop->master, udhcpc_discover_event, ifter,
			ifter->state.dis_timeout + 5);
	return OK;
}


static int udhcpc_rebinding_event(struct eloop *eloop)
{
	client_interface_t * ifter = NULL;
	ifter = ELOOP_ARG(eloop);
	if(ifter == NULL)
		return OK;
	ifter->t_thread = NULL;
	udhcp_client_send_handle(ifter, DHCP_REBINDING);

	ifter->t_thread = eloop_add_timer(eloop->master, udhcpc_timeout_event, ifter,
		ifter->lease.expires - ifter->state.renew_timeout2);

	return OK;
}


static int udhcpc_renew_event(struct eloop *eloop)
{
	client_interface_t * ifter = NULL;
	ifter = ELOOP_ARG(eloop);
	if(ifter == NULL)
		return OK;
	ifter->t_thread = NULL;
	udhcp_client_send_handle(ifter, DHCP_RENEWING);

	ifter->t_thread = eloop_add_timer(eloop->master, udhcpc_rebinding_event, ifter,
		ifter->state.renew_timeout2 - ifter->state.renew_timeout1);

	return OK;
}

static int udhcpc_time_cancel(client_interface_t * ifter)
{
	if(ifter->t_thread)
	{
		eloop_cancel(ifter->t_thread);
		ifter->t_thread = NULL;
	}
	return OK;
}

static int udhcpc_state_mode_change(client_interface_t * ifter, int mode)
{
	return OK;
	if(ifter->r_thread)
	{
		eloop_cancel(ifter->r_thread);
		ifter->r_thread = NULL;
	}
	if(DHCP_RAW_MODE == mode && ifter->state.mode != DHCP_RAW_MODE)
	{
		if(ifter->udp_sock)
			ifter->udp_sock = 0;
		if(ifter->sock == 0)
		{
			ifter->sock = udhcpc_client_socket(ifter->ifindex);
			if (ifter->sock > 0)
			{
				if(ifter->r_thread == NULL)
					ifter->r_thread = eloop_add_read(dhcp_global_config.eloop_master, udhcpc_read_thread,
							ifter, ifter->sock);
			}
		}
	}
	else if(DHCP_UDP_MODE == mode && ifter->state.mode != DHCP_UDP_MODE)
	{
		if(ifter->sock)
			close(ifter->sock);
		ifter->sock = 0;
		if(dhcp_global_config.client_sock == 0)
			dhcp_global_config.client_sock = udhcp_udp_socket(dhcp_global_config.client_port);

		ifter->udp_sock = dhcp_global_config.client_sock;
		zlog_debug(ZLOG_DHCP,"==========%d", ifter->udp_sock);
		if(ifter->r_thread == NULL)
			ifter->r_thread = eloop_add_read(dhcp_global_config.eloop_master, udhcpc_read_thread,
					ifter, ifter->udp_sock);
	}
	ifter->state.mode = mode;
	return OK;
}

static int udhcp_client_recv_handle(struct dhcp_packet *packet,
		client_interface_t * ifter) {
	uint8_t message = 0;
	//uint8_t *temp = NULL;

	if (ntohl(packet->xid) != ifter->state.xid)
	{
		zlog_err(ZLOG_DHCP, "xid %x (our is %x), ignoring packet",
			(unsigned)ntohl(packet->xid), (unsigned)ifter->state.xid);
		return ERROR;
	}

	/* Ignore packets that aren't for us */
	if (packet->hlen != 6
			|| memcmp(packet->chaddr, ifter->client_mac, 6) != 0) {
		//FIXME: need to also check that last 10 bytes are zero
		zlog_err(ZLOG_DHCP, "chaddr does not match, ignoring packet"); // log2?
		return ERROR;
	}

	message = dhcp_option_message_type_get(packet->options, ifter->state.read_bytes);

	switch (message)
	{
	case DHCPOFFER:
		udhcp_client_explain_lease(packet, ifter);
		udhcpc_time_cancel(ifter);
		ifter->state.state = DHCP_REQUESTING;
		udhcp_client_send_handle(ifter, DHCP_REQUESTING/*DHCPREQUEST*/);
		break;

	case DHCPACK:
		udhcp_client_explain_lease(packet, ifter);
		udhcpc_time_cancel(ifter);
		if( (ifter->state.state == DHCP_REQUESTING) )
		{
			ifter->state.renew_timeout1 = ifter->lease.expires >> DHCP_T1;
			ifter->state.renew_timeout2 = (u_int32)((float)ifter->lease.expires * DHCP_T2);
			ifter->state.state = DHCP_BOUND;
			ifter->t_thread = eloop_add_timer(ifter->master, udhcpc_renew_event, ifter,
				ifter->lease.expires - ifter->state.renew_timeout1);
		}
		else if(ifter->state.state == DHCP_RENEWING)
		{
			ifter->state.renew_timeout1 = ifter->lease.expires >> DHCP_T1;
			ifter->state.renew_timeout2 = (u_int32)((float)ifter->lease.expires * DHCP_T2);
			ifter->state.state = DHCP_BOUND;
			ifter->t_thread = eloop_add_timer(ifter->master, udhcpc_renew_event, ifter,
				ifter->lease.expires - ifter->state.renew_timeout1);
		}
		else if(ifter->state.state == DHCP_REBINDING)
		{
			ifter->state.renew_timeout1 = ifter->lease.expires >> DHCP_T1;
			ifter->state.renew_timeout2 = (u_int32)((float)ifter->lease.expires * DHCP_T2);
			ifter->state.state = DHCP_BOUND;
			ifter->t_thread = eloop_add_timer(ifter->master, udhcpc_renew_event, ifter,
				ifter->lease.expires - ifter->state.renew_timeout1);
		}
		udhcpc_state_mode_change(ifter, DHCP_UDP_MODE);
		break;

	case DHCPNAK:
		{
			zlog_err(ZLOG_DHCP, "received %s", "DHCP NAK");
/*			udhcpc_time_cancel(ifter);
			ifter->state.state = DHCP_INIT;
			ifter->lease.lease_address = 0;
			ifter->state.dis_cnt = 0;
			udhcpc_state_mode_change(ifter, DHCP_RAW_MODE);
			*/
			udhcpc_stop(ifter);

			udhcpc_state_mode_change(ifter, DHCP_RAW_MODE);

			ifter->t_thread = eloop_add_timer(ifter->master, udhcpc_discover_event, ifter,
					ifter->state.dis_timeout + 5);
		}
		break;
	default:
		break;
	}
	return OK;
}
/* Returns -1 on errors that are fatal for the socket, -2 for those that aren't */
/* NOINLINE: limit stack usage in caller */
static int udhcp_recv_raw_packet(struct dhcp_packet *dhcp_pkt, int fd,
		u_int32 *ifindex) {
	int bytes;
	struct ip_udp_dhcp_packet packet;
	uint16_t check;
	unsigned char cmsgbuf[CMSG_SPACE(SOPT_SIZE_CMSG_IFINDEX_IPV4())];
	struct iovec iov;
	struct msghdr msg;
	//struct cmsghdr *cmsg;

	/* used to use just safe_read(fd, &packet, sizeof(packet))
	 * but we need to check for TP_STATUS_CSUMNOTREADY :(
	 */
	iov.iov_base = &packet;
	iov.iov_len = sizeof(packet);
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof(cmsgbuf);
	for (;;) {
		bytes = recvmsg(fd, &msg, 0);
		if (bytes < 0) {
			if (errno == EINTR)
				continue;
			zlog_err(ZLOG_DHCP, "packet read error, ignoring");
			/* NB: possible down interface, etc. Caller should pause. */
			return bytes; /* returns -1 */
		}
		break;
	}
	if (bytes < (int) (sizeof(packet.ip) + sizeof(packet.udp))) {
		zlog_err(ZLOG_DHCP, "packet is too short, ignoring");
		return -2;
	}

	if (bytes < ntohs(packet.ip.tot_len)) {
		/* packet is bigger than sizeof(packet), we did partial read */
		zlog_err(ZLOG_DHCP, "oversized packet, ignoring");
		return -2;
	}

	/* ignore any extra garbage bytes */
	bytes = ntohs(packet.ip.tot_len);

	/* make sure its the right packet for us, and that it passes sanity checks */
	if (packet.ip.protocol != IPPROTO_UDP || packet.ip.version != IPVERSION
			|| packet.ip.ihl != (sizeof(packet.ip) >> 2)
			|| packet.udp.dest != htons(CLIENT_PORT)
			/* || bytes > (int) sizeof(packet) - can't happen */
			|| ntohs(packet.udp.len) != (uint16_t)(bytes - sizeof(packet.ip))) {
		zlog_err(ZLOG_DHCP, "unrelated/bogus packet, ignoring");
		return -2;
	}

	/* verify IP checksum */
	check = packet.ip.check;
	packet.ip.check = 0;
	if (check != in_cksum((uint16_t *) &packet.ip, sizeof(packet.ip))) {
		zlog_err(ZLOG_DHCP, "bad IP header checksum, ignoring");
		return -2;
	}

	if (ifindex)
		*ifindex = getsockopt_ifindex(AF_INET, &msg);

	/* verify UDP checksum. IP header has to be modified for this */
	memset(&packet.ip, 0, offsetof(struct iphdr, protocol));
	/* ip.xx fields which are not memset: protocol, check, saddr, daddr */
	packet.ip.tot_len = packet.udp.len; /* yes, this is needed */
	check = packet.udp.check;
	packet.udp.check = 0;
	if (check && check != in_cksum((uint16_t *) &packet, bytes)) {
		zlog_err(ZLOG_DHCP, "packet with bad UDP checksum received, ignoring");
		return -2;
	}
	if (packet.data.cookie != htonl(DHCP_MAGIC)) {
		zlog_err(ZLOG_DHCP, "packet with bad magic, ignoring");
		return -2;
	}

	zlog_err(ZLOG_DHCP, "received %s", "a packet");
	udhcp_dump_packet(&packet.data);

	bytes -= sizeof(packet.ip) + sizeof(packet.udp);
	memcpy(dhcp_pkt, &packet.data, bytes);
	return bytes;
}

static int udhcpc_read_thread(struct eloop *eloop)
{
	struct dhcp_packet packet;
	int bytes = 0;
	int sock = -1;
	//u_int32 ifindex = 0;
	client_interface_t * ifter = NULL;//, *iflook = NULL;

	sock = ELOOP_FD(eloop);
	ifter = ELOOP_ARG(eloop);
	ifter->r_thread = NULL;

	zlog_debug(ZLOG_DHCP,"==========sock=%d ifter->sock=%d ifter->udp_sock=%d", sock, ifter->sock, ifter->udp_sock);

	if(ifter->state.mode == DHCP_RAW_MODE)
	{
		bytes = udhcp_recv_raw_packet(&packet, sock, NULL);
		if (bytes < 0) {
			/* bytes can also be -2 ("bad packet data") */
			if (bytes == -1 && errno != EINTR) {

				udhcpc_stop(ifter);
				ifter->state.mode = DHCP_UDP_MODE;
				close(sock);
				ifter->sock = 0;
				udhcpc_state_mode_change(ifter, DHCP_RAW_MODE);

				ifter->t_thread = eloop_add_timer(ifter->master, udhcpc_discover_event, ifter,
						ifter->state.dis_timeout + 5);

				return ERROR;
			}
		}
	}
	else
	{
		bytes = udhcp_recv_packet(&packet, sock, NULL);
		if (bytes < 0) {
			/* bytes can also be -2 ("bad packet data") */
			if (bytes == -1 && errno != EINTR) {

				close(sock);
				dhcp_global_config.client_sock = ifter->udp_sock = 0;
				ifter->udp_sock = dhcp_global_config.client_sock = udhcp_udp_socket(dhcp_global_config.client_port);
				//udhcpc_state_mode_change(ifter, DHCP_RAW_MODE);
				ifter->t_thread = eloop_add_timer(ifter->master, udhcpc_discover_event, ifter,
						ifter->state.dis_timeout + 5);

				return ERROR;
			}
		}
	}
/*	iflook = dhcp_client_lookup_interface(&dhcp_global_config, ifindex);
	if (iflook == NULL || ifter != iflook)
	{
		ifter->r_thread = eloop_add_read(eloop->master,
			udhcpc_read_thread, ifter, sock);
		zlog_err(ZLOG_DHCP, " this Interface is not allow DHCP");
		return ERROR;
	}*/
	ifter->state.read_bytes = bytes;
	udhcp_client_recv_handle(&packet, ifter);

	ifter->r_thread = eloop_add_read(eloop->master, udhcpc_read_thread,
		ifter, sock);
	return OK;
}

/************************************************************************************/
static int dhcp_client_interface_option_default(client_interface_t *ifter)
{
	ifter->opt_mask[DHCP_SUBNET] = 1;
	ifter->opt_mask[DHCP_ROUTER] = 1;
	ifter->opt_mask[DHCP_DNS_SERVER] = 1;
	ifter->opt_mask[DHCP_BROADCAST] = 1;
	ifter->opt_mask[DHCP_LEASE_TIME] = 1;
	ifter->opt_mask[DHCP_DOMAIN_NAME] = 1;

	dhcp_option_add(ifter->options,  DHCP_CLIENT_ID, ifter->client_mac,  ETHER_ADDR_LEN);
	if(host_name_get())
		dhcp_option_add(ifter->options,  DHCP_HOST_NAME, host_name_get(),  strlen(host_name_get()));

	return OK;
}
/************************************************************************************/
static client_interface_t * dhcp_client_create_interface(u_int32 ifindex) {
	client_interface_t *ifter = XMALLOC(MTYPE_DHCPC_INFO,
			sizeof(client_interface_t));
	if (ifter) {
		struct interface * ifp = if_lookup_by_index(ifindex);
		memset(ifter, 0, sizeof(client_interface_t));

		ifter->ifindex = ifindex;
		ifter->port = CLIENT_PORT;

		ifter->state.state = DHCP_INIT;
		ifter->state.dis_timeout = DHCP_DEFAULT_TIMEOUT;
		ifter->state.dis_retries = DHCP_DEFAULT_RETRIES;
		ifter->state.dis_cnt = 0;
		ifter->state.renew_timeout1 = 0;
		ifter->state.renew_timeout2 = 0;
		ifter->state.xid = 0;
		ifter->state.read_bytes = 0;
		ifter->state.mode = DHCP_RAW_MODE;

		memset(&ifter->lease, 0, sizeof(ifter->lease));
		udhcp_interface_mac(ifindex, NULL, ifter->client_mac);

		dhcp_client_interface_option_default(ifter);


		zlog_debug(ZLOG_DHCP, "===========%s", ifp->k_name);
		return ifter;
	}
	return NULL;
}

client_interface_t * dhcp_client_lookup_interface(dhcp_global_t*config,
		u_int32 ifindex) {
	client_interface_t *pstNode = NULL;
	NODE index;
	if (!lstCount(&config->client_list))
		return NULL;
	for (pstNode = (client_interface_t *) lstFirst(&config->client_list);
			pstNode != NULL;
			pstNode = (client_interface_t *) lstNext((NODE*) &index)) {
		index = pstNode->node;
		if (pstNode->ifindex == ifindex) {
			return pstNode;
		}
	}
	return NULL;
}

int dhcp_client_add_interface(dhcp_global_t*config, u_int32 ifindex)
{
	client_interface_t * ifter = dhcp_client_create_interface(ifindex);
	if (ifter) {
		ifter->master = config->eloop_master;
		ifter->sock = udhcpc_client_socket(ifter->ifindex);
		if (ifter->sock > 0)
		{
			lstAdd(&config->client_list, ifter);
			ifter->state.mode = DHCP_RAW_MODE;
			if(ifter->t_thread == NULL)
				ifter->t_thread = eloop_add_timer(ifter->master, udhcpc_discover_event, ifter, 2);

			if(ifter->r_thread == NULL)
				ifter->r_thread = eloop_add_read(dhcp_global_config.eloop_master, udhcpc_read_thread,
						ifter, ifter->sock);
			return OK;
		}
	}
	return ERROR;
}

int dhcp_client_del_interface(dhcp_global_t*config, u_int32 ifindex) {
	client_interface_t * ifter = dhcp_client_lookup_interface(config, ifindex);
	if (ifter) {
		udhcpc_time_cancel(ifter);
		if(ifter->r_thread)
		{
			eloop_cancel(ifter->r_thread);
			ifter->r_thread = NULL;
		}
		if(ifter->sock)
		{
			close(ifter->sock);
			ifter->sock = 0;
		}
		lstDelete(&config->client_list, ifter);
		XFREE(MTYPE_DHCPC_INFO, ifter);
		return OK;
	}
	return ERROR;
}

int dhcp_client_interface_option_set(client_interface_t * ifter, uint8_t code, uint8_t *str, int len)
{
	dhcp_option_add(ifter->options,  code, str,  len);
	return OK;
}

int dhcp_client_interface_request_set(client_interface_t * ifter, uint8_t code, BOOL enable)
{
	ifter->opt_mask[code] = enable;
	return OK;
}

int dhcp_client_interface_clean(void)
{
	client_interface_t *pstNode = NULL;
	NODE index;
	if (!lstCount(&dhcp_global_config.client_list))
		return NULL;
	for (pstNode = (client_interface_t *) lstFirst(&dhcp_global_config.client_list);
			pstNode != NULL;
			pstNode = (client_interface_t *) lstNext((NODE*) &index))
	{
		index = pstNode->node;
		if (pstNode)
		{
			lstDelete(&dhcp_global_config.client_list, (NODE*)pstNode);
			udhcpc_time_cancel(pstNode);
			if(pstNode->r_thread)
			{
				eloop_cancel(pstNode->r_thread);
				pstNode->r_thread = NULL;
			}
			if(pstNode->sock)
			{
				close(pstNode->sock);
				pstNode->sock = 0;
			}
			dhcp_option_clean(pstNode->options);
			XFREE(MTYPE_DHCPC_INFO, pstNode);
		}
	}
	return OK;
}
/************************************************************************************/
//usage:#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
//usage:# define IF_UDHCP_VERBOSE(...) __VA_ARGS__
//usage:#else
//usage:# define IF_UDHCP_VERBOSE(...)
//usage:#endif
//usage:#define udhcpc_trivial_usage
//usage:       "[-fbq"IF_UDHCP_VERBOSE("v")"RB]"IF_FEATURE_UDHCPC_ARPING(" [-a[MSEC]]")" [-t N] [-T SEC] [-A SEC/-n]\n"
//usage:       "	[-i IFACE]"IF_FEATURE_UDHCP_PORT(" [-P PORT]")" [-s PROG] [-p PIDFILE]\n"
//usage:       "	[-oC] [-r IP] [-V VENDOR] [-F NAME] [-x OPT:VAL]... [-O OPT]..."
//usage:#define udhcpc_full_usage "\n"
//usage:     "\n	-i IFACE	Interface to use (default eth0)"
//usage:	IF_FEATURE_UDHCP_PORT(
//usage:     "\n	-P PORT		Use PORT (default 68)"
//usage:	)
//usage:     "\n	-s PROG		Run PROG at DHCP events (default "CONFIG_UDHCPC_DEFAULT_SCRIPT")"
//usage:     "\n	-p FILE		Create pidfile"
//usage:     "\n	-B		Request broadcast replies"
//usage:     "\n	-t N		Send up to N discover packets (default 3)"
//usage:     "\n	-T SEC		Pause between packets (default 3)"
//usage:     "\n	-A SEC		Wait if lease is not obtained (default 20)"
//usage:     "\n	-n		Exit if lease is not obtained"
//usage:     "\n	-q		Exit after obtaining lease"
//usage:     "\n	-R		Release IP on exit"
//usage:     "\n	-f		Run in foreground"
//usage:	USE_FOR_MMU(
//usage:     "\n	-b		Background if lease is not obtained"
//usage:	)
//usage:     "\n	-S		Log to syslog too"
//usage:	IF_FEATURE_UDHCPC_ARPING(
//usage:     "\n	-a[MSEC]	Validate offered address with ARP ping"
//usage:	)
//usage:     "\n	-r IP		Request this IP address"
//usage:     "\n	-o		Don't request any options (unless -O is given)"
//usage:     "\n	-O OPT		Request option OPT from server (cumulative)"
//usage:     "\n	-x OPT:VAL	Include option OPT in sent packets (cumulative)"
//usage:     "\n			Examples of string, numeric, and hex byte opts:"
//usage:     "\n			-x hostname:bbox - option 12"
//usage:     "\n			-x lease:3600 - option 51 (lease time)"
//usage:     "\n			-x 0x3d:0100BEEFC0FFEE - option 61 (client id)"
//usage:     "\n	-F NAME		Ask server to update DNS mapping for NAME"
//usage:     "\n	-V VENDOR	Vendor identifier (default 'udhcp VERSION')"
//usage:     "\n	-C		Don't send MAC as client identifier"
//usage:	IF_UDHCP_VERBOSE(
//usage:     "\n	-v		Verbose"
//usage:	)
//usage:     "\nSignals:"
//usage:     "\n	USR1	Renew lease"
//usage:     "\n	USR2	Release lease"

#if 0
struct client_config_t client_config;

int udhcpc_main(int argc UNUSED_PARAM, char **argv)
{
uint8_t *message;
const char *str_V, *str_h, *str_F, *str_r;
const char *str_a = "2000";
char *str_P;
void *clientid_mac_ptr;
llist_t *list_O = NULL;
llist_t *list_x = NULL;
int tryagain_timeout = 20;
int discover_timeout = 3;
int discover_retries = 3;
uint32_t server_addr = server_addr; /* for compiler */
uint32_t requested_ip = 0;
uint32_t xid = xid; /* for compiler */
int packet_num;
int timeout; /* must be signed */
unsigned already_waited_sec;
unsigned opt;
unsigned arpping_ms;
int retval;

//setup_common_bufsiz();

/* Default options */
SERVER_PORT = 67;
CLIENT_PORT = 68;
client_config.interface = "eth0";
client_config.script = CONFIG_UDHCPC_DEFAULT_SCRIPT;
str_V = "udhcp "BB_VER;
#if 0
/* Parse command line */
opt = getopt32long(argv, "^"
		/* O,x: list; -T,-t,-A take numeric param */
		"CV:H:h:F:i:np:qRr:s:T:+t:+SA:+O:*ox:*fB"
		USE_FOR_MMU("b")
		IF_FEATURE_UDHCPC_ARPING("a::")
		IF_FEATURE_UDHCP_PORT("P:")
		"v"
		"\0" IF_UDHCP_VERBOSE("vv") /* -v is a counter */
		, udhcpc_longopts
		, &str_V, &str_h, &str_h, &str_F
		, &client_config.interface, &client_config.pidfile /* i,p */
		, &str_r /* r */
		, &client_config.script /* s */
		, &discover_timeout, &discover_retries, &tryagain_timeout /* T,t,A */
		, &list_O
		, &list_x
		IF_FEATURE_UDHCPC_ARPING(, &str_a)
		IF_FEATURE_UDHCP_PORT(, &str_P)
		IF_UDHCP_VERBOSE(, &dhcp_verbose)
);
#endif
if (opt & (OPT_h|OPT_H)) {
	//msg added 2011-11
	zlog_err(ZLOG_DHCP,"option -h NAME is deprecated, use -x hostname:NAME");
	client_config.hostname = alloc_dhcp_option(DHCP_HOST_NAME, str_h, 0);
}
if (opt & OPT_F) {
	/* FQDN option format: [0x51][len][flags][0][0]<fqdn> */
	client_config.fqdn = alloc_dhcp_option(DHCP_FQDN, str_F, 3);
	/* Flag bits: 0000NEOS
	 * S: 1 = Client requests server to update A RR in DNS as well as PTR
	 * O: 1 = Server indicates to client that DNS has been updated regardless
	 * E: 1 = Name is in DNS format, i.e. <4>host<6>domain<3>com<0>,
	 *    not "host.domain.com". Format 0 is obsolete.
	 * N: 1 = Client requests server to not update DNS (S must be 0 then)
	 * Two [0] bytes which follow are deprecated and must be 0.
	 */
	client_config.fqdn[OPT_DATA + 0] = 0x1;
	/*client_config.fqdn[OPT_DATA + 1] = 0; - malloc did it */
	/*client_config.fqdn[OPT_DATA + 2] = 0; */
}
if (opt & OPT_r)
requested_ip = inet_addr(str_r);
#if ENABLE_FEATURE_UDHCP_PORT
if (opt & OPT_P) {
	CLIENT_PORT = xatou16(str_P);
	SERVER_PORT = CLIENT_PORT - 1;
}
#endif
arpping_ms = atoi(str_a);
while (list_O) {
	char *optstr = llist_pop(&list_O);
	unsigned n = strtoul(optstr, NULL, 0);
	if (errno || n > 254) {
		n = udhcp_option_idx(optstr, dhcp_option_strings);
		n = dhcp_optflags[n].code;
	}
	client_config.opt_mask[n >> 3] |= 1 << (n & 7);
}
if (!(opt & OPT_o)) {
	unsigned i, n;
	for (i = 0; (n = dhcp_optflags[i].code) != 0; i++) {
		if (dhcp_optflags[i].flags & OPTION_REQ) {
			client_config.opt_mask[n >> 3] |= 1 << (n & 7);
		}
	}
}
while (list_x) {
	char *optstr = llist_pop(&list_x);
	char *colon = strchr(optstr, ':');
	if (colon)
	*colon = ' ';
	/* now it looks similar to udhcpd's config file line:
	 * "optname optval", using the common routine: */
	udhcp_str2optset(optstr, &client_config.options, dhcp_optflags, dhcp_option_strings);
	if (colon)
	*colon = ':'; /* restore it for NOMMU reexec */
}

if (udhcp_read_interface(client_config.interface,
				&client_config.ifindex,
				NULL,
				client_config.client_mac)
) {
	return 1;
}

clientid_mac_ptr = NULL;
if (!(opt & OPT_C) && !udhcp_find_option(client_config.options, DHCP_CLIENT_ID)) {
	/* not suppressed and not set, set the default client ID */
	client_config.clientid = alloc_dhcp_option(DHCP_CLIENT_ID, "", 7);
	client_config.clientid[OPT_DATA] = 1; /* type: ethernet */
	clientid_mac_ptr = client_config.clientid + OPT_DATA+1;
	memcpy(clientid_mac_ptr, client_config.client_mac, 6);
}
if (str_V[0] != '\0') {
	// can drop -V, str_V, client_config.vendorclass,
	// but need to add "vendor" to the list of recognized
	// string opts for this to work;
	// and need to tweak udhcpc_add_client_options() too...
	// ...so the question is, should we?
	//bb_error_msg("option -V VENDOR is deprecated, use -x vendor:VENDOR");
	client_config.vendorclass = alloc_dhcp_option(DHCP_VENDOR, str_V, 0);
}

#if 0//!BB_MMU
/* on NOMMU reexec (i.e., background) early */
if (!(opt & OPT_f)) {
	bb_daemonize_or_rexec(0 /* flags */, argv);
	logmode = LOGMODE_NONE;
}
#endif
/*	if (opt & OPT_S) {
 openlog(applet_name, LOG_PID, LOG_DAEMON);
 logmode |= LOGMODE_SYSLOG;
 }*/

/* Make sure fd 0,1,2 are open */
//bb_sanitize_stdio();
/* Equivalent of doing a fflush after every \n */
//setlinebuf(stdout);
/* Create pidfile */
//write_pidfile(client_config.pidfile);
/* Goes to stdout (unless NOMMU) and possibly syslog */
zlog_err(ZLOG_DHCP,"started, v"BB_VER);
/* Set up the signal pipe */
//udhcp_sp_setup();
/* We want udhcpc_get_xid to be random... */
//srand(monotonic_us());
state = INIT_SELECTING;
udhcp_run_script(NULL, "deconfig");
change_listen_mode(LISTEN_RAW);
packet_num = 0;
timeout = 0;
already_waited_sec = 0;

/* Main event loop. select() waits on signal pipe and possibly
 * on sockfd.
 * "continue" statements in code below jump to the top of the loop.
 */
for (;;) {
	int tv;
	struct pollfd pfds[2];
	struct dhcp_packet packet;
	/* silence "uninitialized!" warning */
	unsigned timestamp_before_wait = timestamp_before_wait;

	/* When running on a bridge, the ifindex may have changed (e.g. if
	 * member interfaces were added/removed or if the status of the
	 * bridge changed).
	 * Workaround: refresh it here before processing the next packet */
	udhcp_read_interface(client_config.interface, &client_config.ifindex, NULL, client_config.client_mac);

	//bb_error_msg("sockfd:%d, listen_mode:%d", sockfd, listen_mode);

	/* Was opening raw or udp socket here
	 * if (listen_mode != LISTEN_NONE && sockfd < 0),
	 * but on fast network renew responses return faster
	 * than we open sockets. Thus this code is moved
	 * to change_listen_mode(). Thus we open listen socket
	 * BEFORE we send renew request (see "case BOUND:"). */

	//udhcp_sp_fd_set(pfds, sockfd);
	tv = timeout - already_waited_sec;
	retval = 0;
	/* If we already timed out, fall through with retval = 0, else... */
	if (tv > 0) {
		zlog_err(ZLOG_DHCP,"waiting %u seconds", tv);
		timestamp_before_wait = (unsigned)os_monotonic_time();
		retval = poll(pfds, 2, tv < INT_MAX/1000 ? tv * 1000 : INT_MAX);
		if (retval < 0) {
			/* EINTR? A signal was caught, don't panic */
			if (errno == EINTR) {
				already_waited_sec += (unsigned)os_monotonic_time() - timestamp_before_wait;
				continue;
			}
			/* Else: an error occurred, panic! */
			zlog_err(ZLOG_DHCP,"poll");
		}
	}

	/* If timeout dropped to zero, time to become active:
	 * resend discover/renew/whatever
	 */
	if (retval == 0) {
		/* When running on a bridge, the ifindex may have changed
		 * (e.g. if member interfaces were added/removed
		 * or if the status of the bridge changed).
		 * Refresh ifindex and client_mac:
		 */
		if (udhcp_read_interface(client_config.interface,
						&client_config.ifindex,
						NULL,
						client_config.client_mac)
		) {
			goto ret0; /* iface is gone? */
		}
		if (clientid_mac_ptr)
		memcpy(clientid_mac_ptr, client_config.client_mac, 6);

		/* We will restart the wait in any case */
		already_waited_sec = 0;

		switch (state) {
			case INIT_SELECTING:
			if (!discover_retries || packet_num < discover_retries) {
				if (packet_num == 0)
				xid = udhcpc_get_xid();
				/* broadcast */
				udhcpc_send_discover(xid, requested_ip);
				timeout = discover_timeout;
				packet_num++;
				continue;
			}
			leasefail:
			udhcp_run_script(NULL, "leasefail");
#if BB_MMU /* -b is not supported on NOMMU */
			if (opt & OPT_b) { /* background if no lease */
				zlog_err(ZLOG_DHCP,"no lease, forking to background");
				client_background();
				/* do not background again! */
				opt = ((opt & ~OPT_b) | OPT_f);
			} else
#endif
			if (opt & OPT_n) { /* abort if no lease */
				zlog_err(ZLOG_DHCP,"no lease, failing");
				retval = 1;
				goto ret;
			}
			/* wait before trying again */
			timeout = tryagain_timeout;
			packet_num = 0;
			continue;
			case REQUESTING:
			if (packet_num < 3) {
				/* send broadcast select packet */
				udhcpc_send_request(xid, server_addr, requested_ip);
				timeout = discover_timeout;
				packet_num++;
				continue;
			}
			/* Timed out, go back to init state.
			 * "discover...select...discover..." loops
			 * were seen in the wild. Treat them similarly
			 * to "no response to discover" case */
			change_listen_mode(LISTEN_RAW);
			state = INIT_SELECTING;
			goto leasefail;
			case BOUND:
			/* 1/2 lease passed, enter renewing state */
			state = RENEWING;
			client_config.first_secs = 0; /* make secs field count from 0 */
			change_listen_mode(LISTEN_KERNEL);
			zlog_err(ZLOG_DHCP,"entering renew state");
			/* fall right through */
			case RENEW_REQUESTED: /* manual (SIGUSR1) renew */
			case_RENEW_REQUESTED:
			case RENEWING:
			if (timeout > 60) {
				/* send an unicast renew request */
				/* Sometimes observed to fail (EADDRNOTAVAIL) to bind
				 * a new UDP socket for sending inside udhcpc_send_renew.
				 * I hazard to guess existing listening socket
				 * is somehow conflicting with it, but why is it
				 * not deterministic then?! Strange.
				 * Anyway, it does recover by eventually failing through
				 * into INIT_SELECTING state.
				 */
				if (udhcpc_send_renew(xid, server_addr, requested_ip) >= 0) {
					timeout >>= 1;
//TODO: the timeout to receive an answer for our renew should not be selected
//with "timeout = lease_seconds / 2; ...; timeout = timeout / 2": it is often huge.
//Waiting e.g. 4*3600 seconds for a reply does not make sense
//(if reply isn't coming, we keep an open socket for hours),
//it should be something like 10 seconds.
//Also, it's probably best to try sending renew in kernel mode a few (3-5) times
//and fall back to raw mode if it does not work.
					continue;
				}
				/* else: error sending.
				 * example: ENETUNREACH seen with server
				 * which gave us bogus server ID 1.1.1.1
				 * which wasn't reachable (and probably did not exist).
				 */
			}
			/* Timed out or error, enter rebinding state */
			zlog_err(ZLOG_DHCP,"entering rebinding state");
			state = REBINDING;
			/* fall right through */
			case REBINDING:
			/* Switch to bcast receive */
			change_listen_mode(LISTEN_RAW);
			/* Lease is *really* about to run out,
			 * try to find DHCP server using broadcast */
			if (timeout > 0) {
				/* send a broadcast renew request */
				udhcpc_send_renew(xid, 0 /*INADDR_ANY*/, requested_ip);
				timeout >>= 1;
				continue;
			}
			/* Timed out, enter init state */
			zlog_err(ZLOG_DHCP,"lease lost, entering init state");
			udhcp_run_script(NULL, "deconfig");
			state = INIT_SELECTING;
			client_config.first_secs = 0; /* make secs field count from 0 */
			/*timeout = 0; - already is */
			packet_num = 0;
			continue;
			/* case RELEASED: */
		}
		/* yah, I know, *you* say it would never happen */
		timeout = INT_MAX;
		continue; /* back to main loop */
	} /* if poll timed out */

	/* poll() didn't timeout, something happened */

	/* Is it a signal? */
	switch (udhcp_sp_read()) {
		case SIGUSR1:
		client_config.first_secs = 0; /* make secs field count from 0 */
		already_waited_sec = 0;
		perform_renew();
		if (state == RENEW_REQUESTED) {
			/* We might be either on the same network
			 * (in which case renew might work),
			 * or we might be on a completely different one
			 * (in which case renew won't ever succeed).
			 * For the second case, must make sure timeout
			 * is not too big, or else we can send
			 * futile renew requests for hours.
			 * (Ab)use -A TIMEOUT value (usually 20 sec)
			 * as a cap on the timeout.
			 */
			if (timeout > tryagain_timeout)
			timeout = tryagain_timeout;
			goto case_RENEW_REQUESTED;
		}
		/* Start things over */
		packet_num = 0;
		/* Kill any timeouts, user wants this to hurry along */
		timeout = 0;
		continue;
		case SIGUSR2:
		perform_release(server_addr, requested_ip);
		timeout = INT_MAX;
		continue;
		case SIGTERM:
		zlog_err(ZLOG_DHCP,"received %s", "SIGTERM");
		goto ret0;
	}

	/* Is it a packet? */
	if (!pfds[1].revents)
	continue; /* no */

	{
		int len;

		/* A packet is ready, read it */
		if (listen_mode == LISTEN_KERNEL)
		len = udhcp_recv_kernel_packet(&packet, sockfd);
		else
		len = udhcp_recv_raw_packet(&packet, sockfd);
		if (len == -1) {
			/* Error is severe, reopen socket */
			zlog_err(ZLOG_DHCP,"read error: "STRERROR_FMT", reopening socket" STRERROR_ERRNO);
			sleep(discover_timeout); /* 3 seconds by default */
			change_listen_mode(listen_mode); /* just close and reopen */
		}
		/* If this packet will turn out to be unrelated/bogus,
		 * we will go back and wait for next one.
		 * Be sure timeout is properly decreased. */
		already_waited_sec += (unsigned)os_monotonic_time() - timestamp_before_wait;
		if (len < 0)
		continue;
	}

	if (packet.xid != xid) {
		zlog_err(ZLOG_DHCP,"xid %x (our is %x), ignoring packet",
				(unsigned)packet.xid, (unsigned)xid);
		continue;
	}

	/* Ignore packets that aren't for us */
	if (packet.hlen != 6
			|| memcmp(packet.chaddr, client_config.client_mac, 6) != 0
	) {
		//FIXME: need to also check that last 10 bytes are zero
		zlog_err(ZLOG_DHCP,"chaddr does not match, ignoring packet");// log2?
		continue;
	}

	message = udhcp_get_option(&packet, DHCP_MESSAGE_TYPE);
	if (message == NULL) {
		zlog_err(ZLOG_DHCP,"no message type option, ignoring packet");
		continue;
	}

	switch (state) {
		case INIT_SELECTING:
		/* Must be a DHCPOFFER */
		if (*message == DHCPOFFER) {
			uint8_t *temp;

			/* What exactly is server's IP? There are several values.
			 * Example DHCP offer captured with tchdump:
			 *
			 * 10.34.25.254:67 > 10.34.25.202:68 // IP header's src
			 * BOOTP fields:
			 * Your-IP 10.34.25.202
			 * Server-IP 10.34.32.125   // "next server" IP
			 * Gateway-IP 10.34.25.254  // relay's address (if DHCP relays are in use)
			 * DHCP options:
			 * DHCP-Message Option 53, length 1: Offer
			 * Server-ID Option 54, length 4: 10.34.255.7       // "server ID"
			 * Default-Gateway Option 3, length 4: 10.34.25.254 // router
			 *
			 * We think that real server IP (one to use in renew/release)
			 * is one in Server-ID option. But I am not 100% sure.
			 * IP header's src and Gateway-IP (same in this example)
			 * might work too.
			 * "Next server" and router are definitely wrong ones to use, though...
			 */
			/* We used to ignore pcakets without DHCP_SERVER_ID.
			 * I've got user reports from people who run "address-less" servers.
			 * They either supply DHCP_SERVER_ID of 0.0.0.0 or don't supply it at all.
			 * They say ISC DHCP client supports this case.
			 */
			server_addr = 0;
			temp = udhcp_get_option(&packet, DHCP_SERVER_ID);
			if (!temp) {
				zlog_err(ZLOG_DHCP,"no server ID, using 0.0.0.0");
			} else {
				/* it IS unaligned sometimes, don't "optimize" */
				move_from_unaligned32(server_addr, temp);
			}
			/*xid = packet.xid; - already is */
			requested_ip = packet.yiaddr;

			/* enter requesting state */
			state = REQUESTING;
			timeout = 0;
			packet_num = 0;
			already_waited_sec = 0;
		}
		continue;
		case REQUESTING:
		case RENEWING:
		case RENEW_REQUESTED:
		case REBINDING:
		if (*message == DHCPACK) {
			unsigned start;
			uint32_t lease_seconds;
			struct in_addr temp_addr;
			uint8_t *temp;

			temp = udhcp_get_option(&packet, DHCP_LEASE_TIME);
			if (!temp) {
				zlog_err(ZLOG_DHCP,"no lease time with ACK, using 1 hour lease");
				lease_seconds = 60 * 60;
			} else {
				/* it IS unaligned sometimes, don't "optimize" */
				move_from_unaligned32(lease_seconds, temp);
				lease_seconds = ntohl(lease_seconds);
				/* paranoia: must not be too small and not prone to overflows */
				if (lease_seconds < 0x10)
				lease_seconds = 0x10;
				if (lease_seconds > 0x7fffffff / 1000)
				lease_seconds = 0x7fffffff / 1000;
			}
#if ENABLE_FEATURE_UDHCPC_ARPING
			if (opt & OPT_a) {
				/* RFC 2131 3.1 paragraph 5:
				 * "The client receives the DHCPACK message with configuration
				 * parameters. The client SHOULD perform a final check on the
				 * parameters (e.g., ARP for allocated network address), and notes
				 * the duration of the lease specified in the DHCPACK message. At this
				 * point, the client is configured. If the client detects that the
				 * address is already in use (e.g., through the use of ARP),
				 * the client MUST send a DHCPDECLINE message to the server and restarts
				 * the configuration process..." */
				if (!arpping(packet.yiaddr,
								NULL,
								(uint32_t) 0,
								client_config.client_mac,
								client_config.interface,
								arpping_ms)
				) {
					zlog_err(ZLOG_DHCP,"offered address is in use "
							"(got ARP reply), declining");
					udhcpc_send_decline(/*xid,*/server_addr, packet.yiaddr);

					if (state != REQUESTING)
					udhcp_run_script(NULL, "deconfig");
					change_listen_mode(LISTEN_RAW);
					state = INIT_SELECTING;
					client_config.first_secs = 0; /* make secs field count from 0 */
					requested_ip = 0;
					timeout = tryagain_timeout;
					packet_num = 0;
					already_waited_sec = 0;
					continue; /* back to main loop */
				}
			}
#endif
			/* enter bound state */
			temp_addr.s_addr = packet.yiaddr;
			zlog_err(ZLOG_DHCP,"lease of %s obtained, lease time %u",
					inet_ntoa(temp_addr), (unsigned)lease_seconds);
			requested_ip = packet.yiaddr;

			start = os_monotonic_time();
			udhcp_run_script(&packet, state == REQUESTING ? "bound" : "renew");
			already_waited_sec = (unsigned)os_monotonic_time() - start;
			timeout = lease_seconds / 2;
			if ((unsigned)timeout < already_waited_sec) {
				/* Something went wrong. Back to discover state */
				timeout = already_waited_sec = 0;
			}

			state = BOUND;
			change_listen_mode(LISTEN_NONE);
			if (opt & OPT_q) { /* quit after lease */
				goto ret0;
			}
			/* future renew failures should not exit (JM) */
			opt &= ~OPT_n;
#if BB_MMU /* NOMMU case backgrounded earlier */
			if (!(opt & OPT_f)) {
				client_background();
				/* do not background again! */
				opt = ((opt & ~OPT_b) | OPT_f);
			}
#endif
			/* make future renew packets use different xid */
			/* xid = udhcpc_get_xid(); ...but why bother? */

			continue; /* back to main loop */
		}
		if (*message == DHCPNAK) {
			/* If network has more than one DHCP server,
			 * "wrong" server can reply first, with a NAK.
			 * Do not interpret it as a NAK from "our" server.
			 */
			if (server_addr != 0) {
				uint32_t svid;
				uint8_t *temp;

				temp = udhcp_get_option(&packet, DHCP_SERVER_ID);
				if (!temp) {
					non_matching_svid:
					zlog_err(ZLOG_DHCP,"received DHCP NAK with wrong"
							" server ID, ignoring packet");
					continue;
				}
				move_from_unaligned32(svid, temp);
				if (svid != server_addr)
				goto non_matching_svid;
			}
			/* return to init state */
			zlog_err(ZLOG_DHCP,"received %s", "DHCP NAK");
			udhcp_run_script(&packet, "nak");
			if (state != REQUESTING)
			udhcp_run_script(NULL, "deconfig");
			change_listen_mode(LISTEN_RAW);
			sleep(3); /* avoid excessive network traffic */
			state = INIT_SELECTING;
			client_config.first_secs = 0; /* make secs field count from 0 */
			requested_ip = 0;
			timeout = 0;
			packet_num = 0;
			already_waited_sec = 0;
		}
		continue;
		/* case BOUND: - ignore all packets */
		/* case RELEASED: - ignore all packets */
	}
	/* back to main loop */
} /* for (;;) - main loop ends */

ret0:
if (opt & OPT_R) /* release on quit */
perform_release(server_addr, requested_ip);
retval = 0;
ret:
/*if (client_config.pidfile) - remove_pidfile has its own check */
//	remove_pidfile(client_config.pidfile);
return retval;
}
#endif


int dhcpc_enable_test()
{
	return ERROR;
}

