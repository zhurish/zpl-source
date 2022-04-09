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
#include "auto_include.h"
#include <zplos_include.h>

/* Override ENABLE_FEATURE_PIDFILE - ifupdown needs our pidfile to always exist */
#define WANT_PIDFILE 1
#include "dhcp_def.h"
#include "dhcp_lease.h"
#include "dhcp_util.h"
#include "dhcp_main.h"
#include "dhcpc.h"
#include "checksum.h"
#include "host.h"
#include <netinet/if_ether.h>
#include <linux/filter.h>
#include <linux/if_packet.h>

/*
 #ifndef PACKET_AUXDATA
 # define PACKET_AUXDATA 8
 struct tpacket_auxdata {
 zpl_uint32  tp_status;
 zpl_uint32  tp_len;
 zpl_uint32  tp_snaplen;
 zpl_uint16 tp_mac;
 zpl_uint16 tp_net;
 zpl_uint16 tp_vlan_tci;
 zpl_uint16 tp_padding;
 };
 #endif
 */

static int udhcpc_raw_read_thread(struct eloop *eloop);
static int udhcpc_udp_read_thread(struct eloop *eloop);
static int udhcpc_discover_event(struct eloop *eloop);
static zpl_socket_t udhcpc_client_socket(int ifindex);
static int udhcpc_state_mode_change(client_interface_t * ifter, int mode);

/* Call a script with a par file and env vars */
/*static void udhcp_run_script(struct dhcp_packet *packet, const char *name)
 {
 return;
 }*/

/*** Sending/receiving packets ***/

static zpl_uint32  udhcpc_get_xid(void)
{
	return rand();
}

/* Initialize the packet with the proper defaults */
static void udhcpc_packet_init(client_interface_t *inter,
		struct dhcp_packet *packet, char type)
{
	zpl_uint16 secs;

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

/*static zpl_uint8* alloc_dhcp_option(int code, const char *str, int extra) {
 zpl_uint8 *storage = NULL;
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
		struct dhcp_packet *packet)
{
	int i = 0, end = 0, len = 0;
	//udhcp_add_simple_option(packet, DHCP_MAX_SIZE, htons(IP_UDP_DHCP_SIZE));

	/* Add a "param req" option with the list of options we'd like to have
	 * from stubborn DHCP servers. Pull the data from the struct in common.c.
	 * No bounds checking because it goes towards the head of the packet. */
	dhcp_option_packet(inter->options, packet->options,
			sizeof(packet->options));

	end = udhcp_end_option(packet->options);
	len = 0;
	for (i = 1; i < DHCP_END; i++)
	{
		if (inter->opt_mask[i])
		{
			packet->options[end + OPT_DATA + len] = i;
			len++;
		}
	}
	if (len)
	{
		packet->options[end + OPT_CODE] = DHCP_PARAM_REQ;
		packet->options[end + OPT_LEN] = len;
		packet->options[end + OPT_DATA + len] = DHCP_END;
	}

	//int dhcp_option_packet_set_value(char *data, int len, zpl_uint8 code, zpl_uint32  oplen, zpl_uint8 *opt)

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

static int udhcpc_raw_packet_bcast(zpl_socket_t sock, struct dhcp_packet *packet,
		zpl_uint32  src_nip, zpl_uint32 ifindex)
{
	struct udhcp_packet_cmd source;
	struct udhcp_packet_cmd dest;

	source.ip = src_nip;
	source.port = dhcp_global_config.client_port;

	dest.ip = IPSTACK_INADDR_BROADCAST;
	dest.port = dhcp_global_config.server_port;
	return udhcp_send_raw_packet(sock, packet, &source, &dest,
			DHCP_MAC_BCAST_ADDR, ifindex);
}

static int udhcpc_packet_bcast_ucast(zpl_socket_t sock, struct dhcp_packet *packet,
		zpl_uint32  ciaddr, zpl_uint32  server, zpl_uint32 ifindex)
{
	if (server)
	{
		struct udhcp_packet_cmd source;
		struct udhcp_packet_cmd dest;

		source.ip = ciaddr;
		source.port = dhcp_global_config.client_port;

		dest.ip = server;
		dest.port = dhcp_global_config.server_port;

		zlog_debug(MODULE_DHCP,
				"udhcpc_packet_bcast_ucast UDP %x:%d to %s:%d\r\n", ciaddr,
				source.port, inet_address(ntohl(server)), dest.port);

		return udhcp_send_udp_packet(sock, packet, &source, &dest);
	}
	return udhcpc_raw_packet_bcast(sock, packet, ciaddr, ifindex);
}

/* Broadcast a DHCP discover packet to the network, with an optionally requested IP */
/* NOINLINE: limit stack usage in caller */
static int udhcpc_send_discover(client_interface_t *inter, zpl_uint32  requested)
{
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

	if (DHCPC_DEBUG_ISON(EVENT))
	{
		zlog_debug(MODULE_DHCP, "DHCP Client sending DHCPDISCOVER packet on interface %s",
				ifindex2ifname(inter->ifindex));
	}
	if (inter->state.mode == DHCP_RAW_MODE)
		return udhcpc_raw_packet_bcast(inter->sock, &packet, IPSTACK_INADDR_ANY,
				inter->ifindex);
	return OK;
}

/* Broadcast a DHCP request message */
/* RFC 2131 3.1 paragraph 3:
 * "The client _broadcasts_ a DHCPREQUEST message..."
 */
/* NOINLINE: limit stack usage in caller */
static int udhcpc_send_request(client_interface_t *inter, zpl_uint32  server,
		zpl_uint32  requested)
{
	struct dhcp_packet packet;
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

	if (DHCPC_DEBUG_ISON(EVENT))
	{
		struct ipstack_in_addr temp_addr;
		temp_addr.s_addr = requested;
		zlog_debug(MODULE_DHCP, "DHCP Client sending DHCPREQUEST packet on interface %s",
				ifindex2ifname(inter->ifindex));
		zlog_debug(MODULE_DHCP, "DHCP Client  request ip address %s",
				ipstack_inet_ntoa(temp_addr));
	}
	if (inter->state.mode == DHCP_RAW_MODE)
		return udhcpc_raw_packet_bcast(inter->sock, &packet, IPSTACK_INADDR_ANY,
				inter->ifindex);
	return ERROR;
}

/* Unicast or broadcast a DHCP renew message */
/* NOINLINE: limit stack usage in caller */
static int udhcpc_send_renew(client_interface_t *inter, zpl_uint32  server,
		zpl_uint32  ciaddr)
{
	struct dhcp_packet packet;

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

	if (DHCPC_DEBUG_ISON(EVENT))
	{
		struct ipstack_in_addr temp_addr;
		temp_addr.s_addr = server;
		char tmpbug[64];
		memset(tmpbug, 0, sizeof(tmpbug));
		sprintf(tmpbug, "%s", inet_address(ciaddr));
		zlog_debug(MODULE_DHCP, "DHCP Client sending DHCPREQUEST packet on interface %s",
				ifindex2ifname(inter->ifindex));
		zlog_debug(MODULE_DHCP, "DHCP Client renew ip address %s from %s",
				tmpbug, ipstack_inet_ntoa(temp_addr));
	}
	//return udhcpc_packet_bcast_ucast(inter->sock, &packet, ciaddr, server, inter->ifindex);
	if (inter->state.mode == DHCP_RAW_MODE)
		return udhcpc_packet_bcast_ucast(inter->sock, &packet, ciaddr, server,
				inter->ifindex);
	else
		return udhcpc_packet_bcast_ucast(inter->udp_sock, &packet, ciaddr,
				server, inter->ifindex);
//	udhcp_run_script(&packet, state == REQUESTING ? "bound" : "renew");
}

#if ENABLE_FEATURE_UDHCPC_ARPING
/* Broadcast a DHCP decline message */
/* NOINLINE: limit stack usage in caller */
static int udhcpc_send_decline(client_interface_t *inter, zpl_uint32  server,
		zpl_uint32  requested)
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

	if (DHCPC_DEBUG_ISON(EVENT))
	{
		struct ipstack_in_addr temp_addr;
		temp_addr.s_addr = server;
		char tmpbug[64];
		memset(tmpbug, 0, sizeof(tmpbug));
		sprintf(tmpbug, "%s", inet_address(requested));
		zlog_debug(MODULE_DHCP, "DHCP Client sending DHCPDECLINE packet on interface %s",
				ifindex2ifname(inter->ifindex));
		zlog_debug(MODULE_DHCP, "DHCP Client decline ip address %s from %s",
				tmpbug, ipstack_inet_ntoa(temp_addr));
	}
	if (inter->state.mode == DHCP_RAW_MODE)
		return udhcpc_packet_bcast_ucast(inter->sock, &packet, requested,
				server, inter->ifindex);
	else
		return udhcpc_packet_bcast_ucast(inter->udp_sock, &packet, requested,
				server, inter->ifindex);
	//return udhcpc_raw_packet_bcast(inter->sock, &packet, IPSTACK_INADDR_ANY, inter->ifindex);
}
#endif

/* Unicast a DHCP release message */
static int udhcpc_send_release(client_interface_t *inter, zpl_uint32  server,
		zpl_uint32  ciaddr)
{
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
	if (DHCPC_DEBUG_ISON(EVENT))
	{
		struct ipstack_in_addr temp_addr;
		temp_addr.s_addr = server;
		char tmpbug[64];
		memset(tmpbug, 0, sizeof(tmpbug));
		sprintf(tmpbug, "%s", inet_address(ciaddr));
		zlog_debug(MODULE_DHCP, "DHCP Client sending DHCPRELEASE packet on interface %s",
				ifindex2ifname(inter->ifindex));
		zlog_debug(MODULE_DHCP, "DHCP Client release ip address %s from %s",
				tmpbug, ipstack_inet_ntoa(temp_addr));
	}
	/* Note: normally we unicast here since "server" is not zero.
	 * However, there _are_ people who run "address-less" DHCP servers,
	 * and reportedly ISC dhcp client and Windows allow that.
	 */
	if (inter->state.mode == DHCP_RAW_MODE)
		return udhcpc_packet_bcast_ucast(inter->sock, &packet, ciaddr, server,
				inter->ifindex);
	else
		return udhcpc_packet_bcast_ucast(inter->udp_sock, &packet, ciaddr,
				server, inter->ifindex);
	//return udhcpc_packet_bcast_ucast(inter->sock, &packet, ciaddr, server, inter->ifindex);
	//udhcp_run_script(NULL, "deconfig");
}

static int udhcpc_send_inform(client_interface_t *inter, zpl_uint32  server,
		zpl_uint32  ciaddr)
{
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

	if (DHCPC_DEBUG_ISON(EVENT))
	{
		struct ipstack_in_addr temp_addr;
		temp_addr.s_addr = server;
		char tmpbug[64];
		memset(tmpbug, 0, sizeof(tmpbug));
		sprintf(tmpbug, "%s", inet_address(ciaddr));
		zlog_debug(MODULE_DHCP, "DHCP Client sending DHCPINFORM packet on interface %s",
				ifindex2ifname(inter->ifindex));
		zlog_debug(MODULE_DHCP, "DHCP Client inform ip address %s from %s",
				tmpbug, ipstack_inet_ntoa(temp_addr));
	}
	/* Note: normally we unicast here since "server" is not zero.
	 * However, there _are_ people who run "address-less" DHCP servers,
	 * and reportedly ISC dhcp client and Windows allow that.
	 */
	if (inter->state.mode == DHCP_RAW_MODE)
		return udhcpc_packet_bcast_ucast(inter->sock, &packet, ciaddr, server,
				inter->ifindex);
	else
	{
		return udhcpc_packet_bcast_ucast(inter->udp_sock, &packet, ciaddr,
				server, inter->ifindex);
	}
}
/************************************************************************************/
static zpl_socket_t udhcpc_client_socket(int ifindex)
{
	zpl_socket_t fd;
	fd = udhcp_raw_socket();
	if (!ipstack_invalid(fd))
	{
		//setsockopt_ifindex (IPSTACK_AF_INET, fd, 1);
		if (udhcp_client_socket_bind(fd, ifindex) == OK)
		{
			udhcp_client_socket_filter(fd, 68);
			//zlog_debug(MODULE_DHCP, "created raw socket");
			return fd;
		}
		zlog_err(MODULE_DHCP, "Can not bind raw socket(%s)", strerror(ipstack_errno));
		ipstack_close(fd);
		return fd;
	}
	return fd;
}

/************************************************************************************/
/************************************************************************************/
/* Called only on SIGUSR1 */

#if 0
static int perform_renew(client_interface_t * ifter)
{
	zlog_err(MODULE_DHCP, "performing DHCP renew");
	switch (ifter->state)
	{
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


static int udhcp_client_release(client_interface_t * ifter, zpl_uint32  server_addr,
		zpl_uint32  requested_ip)
{
	/* send release packet */
	if (ifter->state.state == DHCP_BOUND || ifter->state.state == DHCP_RENEWING
			|| ifter->state.state == DHCP_REBINDING || ifter->state.state == DHCP_REBINDING)
	{
		if (DHCPC_DEBUG_ISON(EVENT))
		{
			struct ipstack_in_addr temp_addr;
			temp_addr.s_addr = server_addr;
			char tmpbug[64];
			memset(tmpbug, 0, sizeof(tmpbug));
			sprintf(tmpbug, "%s", inet_address(ntohl(requested_ip)));
			zlog_debug(MODULE_DHCP, "DHCP Client sending DHCPRELEASE packet on interface %s",
					ifindex2ifname(ifter->ifindex));
			zlog_debug(MODULE_DHCP, "DHCP Client release ip address %s from %s",
					tmpbug, ipstack_inet_ntoa(temp_addr));
		}
		udhcpc_send_release(ifter, server_addr, requested_ip); /* unicast */
	}
	if (DHCPC_DEBUG_ISON(EVENT))
		zlog_debug(MODULE_DHCP, "DHCP Client state change to DHCP_RELEASE");
	/*
	 * We can be here on: SIGUSR2,
	 * or on exit (SIGTERM) and -R "release on quit" is specified.
	 * Users requested to be notified in all cases, even if not in one
	 * of the states above.
	 */
	dhcp_client_lease_unset(ifter);
	//udhcp_run_script(NULL, "deconfig");

	ifter->state.state = DHCP_RELEASE;
	return OK;
}


static int udhcp_client_explain_lease(struct dhcp_packet *packet,
		client_interface_t * ifter)
{
	int optlen = 0;
	zpl_uint8 *temp = NULL;
	ifter->lease.lease_address = packet->yiaddr;
	dhcp_option_get_address(packet->options, DHCP_SERVER_ID,
			&ifter->lease.server_address);
	dhcp_option_get_address(packet->options, DHCP_SUBNET,
			&ifter->lease.lease_netmask);
	dhcp_option_get_address(packet->options, DHCP_BROADCAST,
			&ifter->lease.lease_broadcast);

	temp = (const char*) udhcp_get_option(packet, DHCP_DOMAIN_NAME, &optlen);
	if (temp)
	{
		if (optlen < sizeof(ifter->lease.domain_name))
		{
			memset(ifter->lease.domain_name, 0,
					sizeof(ifter->lease.domain_name));
			strncpy(ifter->lease.domain_name, temp, optlen);
		}
	}

	temp = (const char*) udhcp_get_option(packet, DHCP_ROUTER, &optlen);
	if (temp)
	{
		move_get_unaligned32(temp, ifter->lease.lease_gateway);
		if (optlen > 4)
			move_get_unaligned32(temp + 4, ifter->lease.lease_gateway2);
	}
	temp = (const char*) udhcp_get_option(packet, DHCP_DNS_SERVER, &optlen);
	if (temp)
	{
		move_get_unaligned32(temp, ifter->lease.lease_dns1);
		if (optlen > 4)
			move_get_unaligned32(temp + 4, ifter->lease.lease_dns2);
		/*		udhcp_str2nip(temp, &ifter->lease.lease_dns1);
		 if(optlen > 4)
		 udhcp_str2nip(temp + 4, &ifter->lease.lease_dns2);*/
	}
	temp = udhcp_get_option(packet, DHCP_LEASE_TIME, NULL);
	if (!temp)
	{
		zlog_warn(MODULE_DHCP, "no lease time with ACK, using 1 hour lease");
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
	temp = (const char*) udhcp_get_option(packet, DHCP_TIME_SERVER, &optlen);
	if (temp)
	{
		move_get_unaligned32(temp, ifter->lease.lease_timer1);
		if (optlen > 4)
			move_get_unaligned32(temp + 4, ifter->lease.lease_timer2);
		/*		udhcp_str2nip(temp, &ifter->lease.lease_timer1);
		 if(optlen > 4)
		 udhcp_str2nip(temp + 4, &ifter->lease.lease_timer2);*/
	}

	temp = (const char*) udhcp_get_option(packet, DHCP_LOG_SERVER, &optlen);
	if (temp)
	{
		move_get_unaligned32(temp, ifter->lease.lease_log1);
		if (optlen > 4)
			move_get_unaligned32(temp + 4, ifter->lease.lease_log2);
		/*		udhcp_str2nip(temp, &ifter->lease.lease_log1);
		 if(optlen > 4)
		 udhcp_str2nip(temp + 4, &ifter->lease.lease_log2);*/
	}

	temp = (const char*) udhcp_get_option(packet, DHCP_NTP_SERVER, &optlen);
	if (temp)
	{
		move_get_unaligned32(temp, ifter->lease.lease_ntp1);
		if (optlen > 4)
			move_get_unaligned32(temp + 4, ifter->lease.lease_ntp2);
		/*		udhcp_str2nip(temp, &ifter->lease.lease_ntp1);
		 if(optlen > 4)
		 udhcp_str2nip(temp + 4, &ifter->lease.lease_ntp2);*/
	}
	dhcp_option_get_8bit(packet->options, DHCP_IP_TTL, &ifter->lease.lease_ttl);
	dhcp_option_get_8bit(packet->options, DHCP_MTU, &ifter->lease.lease_mtu);

	temp = (const char*) udhcp_get_option(packet, DHCP_TFTP_SERVER_NAME,
			&optlen);
	if (temp)
	{
		if (optlen < sizeof(ifter->lease.tftp_srv_name))
		{
			memset(ifter->lease.tftp_srv_name, 0,
					sizeof(ifter->lease.tftp_srv_name));
			strncpy(ifter->lease.tftp_srv_name, temp, optlen);
		}
	}

	ifter->lease.starts = os_monotonic_time();
	;
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
		if (ifter->state.dis_cnt < ifter->state.dis_retries)
		{
			if (ifter->state.dis_cnt == 0)
				ifter->state.xid = udhcpc_get_xid();
			udhcpc_send_discover(ifter, ifter->lease.lease_address);
			ifter->state.dis_cnt++;
		}
		else
		{
			if (DHCPC_DEBUG_ISON(STATE))
			{
				zlog_debug(MODULE_DHCP, "DHCP Client state change to INIT");
			}
			ifter->state.state = DHCP_INIT;
			ifter->state.dis_cnt = 0;
			ifter->state.renew_timeout1 = 0;
			ifter->state.renew_timeout2 = 0;
			ifter->state.xid = 0;
			ifter->state.read_bytes = 0;

			memset(&ifter->lease, 0, sizeof(ifter->lease));

			if (ifter->t_thread)
				eloop_cancel(ifter->t_thread);
			ifter->state.xid = udhcpc_get_xid();
			ifter->d_thread = eloop_add_timer(ifter->master,
					udhcpc_discover_event, ifter, ifter->state.dis_timeout + 5);
		}
		break;
	case DHCP_REQUESTING:
		/* send broadcast request packet */
		if (ifter->state.state <= DHCP_REQUESTING)
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
		if (DHCPC_DEBUG_ISON(STATE))
		{
			zlog_debug(MODULE_DHCP, "DHCP Client state change to RENEWING");
		}
		break;

	case DHCP_REBINDING:
		udhcpc_send_renew(ifter, ifter->lease.server_address,
				ifter->lease.lease_address);
		ifter->state.state = DHCP_REBINDING;
		if (DHCPC_DEBUG_ISON(STATE))
		{
			zlog_debug(MODULE_DHCP, "DHCP Client state change to REBINDING");
		}
		break;

	case DHCP_DECLINE:
		udhcpc_send_decline(ifter, ifter->lease.server_address,
				ifter->lease.lease_address);
		ifter->state.state = DHCP_DECLINE;
		if (DHCPC_DEBUG_ISON(STATE))
		{
			zlog_debug(MODULE_DHCP, "DHCP Client state change to DECLINE");
		}
		break;
	case DHCP_RELEASE:
		udhcpc_send_release(ifter, ifter->lease.server_address,
				ifter->lease.lease_address);
		ifter->state.state = DHCP_RELEASE;
		if (DHCPC_DEBUG_ISON(STATE))
		{
			zlog_debug(MODULE_DHCP, "DHCP Client state change to RELEASE");
		}
		break;
	case DHCP_INFORM:
		udhcpc_send_inform(ifter, ifter->lease.server_address,
				ifter->lease.lease_address);
		ifter->state.state = DHCP_INFORM;
		if (DHCPC_DEBUG_ISON(STATE))
		{
			zlog_debug(MODULE_DHCP, "DHCP Client state change to INFORM");
		}
		break;
	}
	return OK;
}

static int udhcpc_discover_event(struct eloop *eloop)
{
	client_interface_t * ifter = NULL;
	ifter = ELOOP_ARG(eloop);
	if (ifter == NULL)
		return OK;
	ifter->d_thread = NULL;

	udhcp_client_send_handle(ifter, DHCP_INIT);
	if (ifter->state.state <= DHCP_INIT)
	{
		if (ifter->master && !ifter->d_thread)
			ifter->d_thread = eloop_add_timer(ifter->master,
					udhcpc_discover_event, ifter, ifter->state.dis_timeout);
	}
	/*
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
	 ifter->d_thread = eloop_add_timer(ifter->master, udhcpc_discover_event, ifter,
	 ifter->state.dis_timeout + 5);
	 }*/
	return OK;
}

static int udhcpc_stop(client_interface_t * ifter)
{
	if (DHCPC_DEBUG_ISON(STATE))
	{
		zlog_debug(MODULE_DHCP, "DHCP Client state change to INIT");
	}
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

/*static int udhcpc_timeout_event(struct eloop *eloop)
 {
 client_interface_t * ifter = NULL;
 ifter = ELOOP_ARG(eloop);
 if(ifter == NULL)
 return OK;
 ifter->t_thread = NULL;

 dhcp_client_lease_unset(ifter);
 udhcpc_stop(ifter);

 udhcpc_state_mode_change(ifter, DHCP_RAW_MODE);
 if(ifter->master)
 ifter->t_thread = eloop_add_timer(ifter->master, udhcpc_discover_event, ifter,
 ifter->state.dis_timeout + 5);
 return OK;
 }*/

static int udhcpc_rebinding_event(struct eloop *eloop)
{
	client_interface_t * ifter = NULL;
	ifter = ELOOP_ARG(eloop);
	if (ifter == NULL)
		return OK;
	ifter->t_thread = NULL;
	udhcp_client_send_handle(ifter, DHCP_REBINDING);
	/*
	 if(ifter->master && !ifter->d_thread)
	 ifter->d_thread = eloop_add_timer(ifter->master, udhcpc_discover_event, ifter,
	 ifter->lease.expires - ifter->state.renew_timeout2);
	 */

	return OK;
}

static int udhcpc_renew_event(struct eloop *eloop)
{
	client_interface_t * ifter = NULL;
	ifter = ELOOP_ARG(eloop);
	if (ifter == NULL)
		return OK;
	ifter->t_thread = NULL;
	udhcp_client_send_handle(ifter, DHCP_RENEWING);
	if (ifter->master)
		ifter->t_thread = eloop_add_timer(ifter->master, udhcpc_rebinding_event,
				ifter,
				ifter->state.renew_timeout2 - ifter->state.renew_timeout1);

	return OK;
}

static int udhcpc_time_cancel(client_interface_t * ifter)
{
	if (ifter->t_thread)
	{
		eloop_cancel(ifter->t_thread);
		ifter->t_thread = NULL;
	}
	return OK;
}

static int udhcpc_state_mode_change(client_interface_t * ifter, int mode)
{
	if (ifter->r_thread)
	{
		eloop_cancel(ifter->r_thread);
		ifter->r_thread = NULL;
	}
	if (DHCP_RAW_MODE == mode)
	{
		zlog_debug(MODULE_DHCP, "udhcpc_state_mode_change DHCP_RAW_MODE\r\n");
		if (ifter->state.mode != DHCP_RAW_MODE)
		{
			zlog_debug(MODULE_DHCP,
					"udhcpc_state_mode_change DHCP_RAW_MODE close udp socket=%d\r\n",
					ifter->udp_sock);

			if (DHCPC_DEBUG_ISON(STATE))
			{
				zlog_debug(MODULE_DHCP, "DHCP Client change to RAW MODE");
			}

			if (!ipstack_invalid(ifter->udp_sock))
				ipstack_close(ifter->udp_sock);
			if (dhcp_global_config.client_cnt)
				dhcp_global_config.client_cnt--;
			if (dhcp_global_config.client_cnt == 0)
			{
				ipstack_close(dhcp_global_config.client_sock);
				//dhcp_global_config.client_sock = 0;
			}
		}
		else
		{
			ipstack_close(ifter->sock);
			//ifter->sock = 0;
		}
		if (ipstack_invalid(ifter->sock))
		{
			ifter->sock = udhcpc_client_socket(ifter->ifindex);
			if (!ipstack_invalid(ifter->sock))
			{
				zlog_debug(MODULE_DHCP,
						"udhcpc_state_mode_change DHCP_RAW_MODE open raw socket=%d\r\n",
						ifter->sock._fd);
				if (ifter->r_thread == NULL && !ipstack_invalid(ifter->sock))
					ifter->r_thread = eloop_add_read(
							dhcp_global_config.eloop_master, udhcpc_raw_read_thread,
							ifter, ifter->sock);
			}
		}
	}
	else if (DHCP_UDP_MODE == mode/* && ifter->state.mode != DHCP_UDP_MODE*/)
	{
		if (ifter->state.mode != DHCP_UDP_MODE)
		{
			zlog_debug(MODULE_DHCP,
					"udhcpc_state_mode_change DHCP_UDP_MODE close raw socket=%d\r\n",
					ifter->sock._fd);
			if (DHCPC_DEBUG_ISON(STATE))
			{
				zlog_debug(MODULE_DHCP, "DHCP Client change to UDP MODE");
			}
			if (!ipstack_invalid(ifter->sock))
				ipstack_close(ifter->sock);
			//ifter->sock = 0;
		}
		else
		{
			ipstack_close(dhcp_global_config.client_sock);
			//dhcp_global_config.client_sock = 0;
		}
		if (ipstack_invalid(dhcp_global_config.client_sock))
			dhcp_global_config.client_sock = udhcp_udp_socket(
					dhcp_global_config.client_port);

		ifter->udp_sock = dhcp_global_config.client_sock;
		dhcp_global_config.client_cnt++;

		zlog_debug(MODULE_DHCP,
				"udhcpc_state_mode_change DHCP_UDP_MODE open udp socket=%d\r\n",
				ifter->udp_sock._fd);

		if (ifter->r_thread == NULL && !ipstack_invalid(ifter->udp_sock))
			ifter->r_thread = eloop_add_read(dhcp_global_config.eloop_master,
					udhcpc_udp_read_thread, NULL, ifter->udp_sock);
	}
	ifter->state.mode = mode;
	return OK;
}

static int udhcp_client_recv_handle(struct dhcp_packet *packet,
		client_interface_t * ifter)
{
	zpl_uint8 message = 0;

	if (ntohl(packet->xid) != ifter->state.xid)
	{
		zlog_err(MODULE_DHCP, "dhcp transaction id 0x%x (our is 0x%x), ignoring packet",
				(unsigned )ntohl(packet->xid), (unsigned )ifter->state.xid);
		return ERROR;
	}

	/* Ignore packets that aren't for us */
	if (packet->hlen != 6 || memcmp(packet->chaddr, ifter->client_mac, 6) != 0)
	{
		//FIXME: need to also check that last 10 bytes are zero
		zlog_err(MODULE_DHCP, "client hardware address does not match, ignoring packet"); // log2?
		return ERROR;
	}

	message = dhcp_option_message_type_get(packet->options,
			ifter->state.read_bytes);

	switch (message)
	{
	case DHCPOFFER:

		if (ifter->d_thread)
		{
			eloop_cancel(ifter->d_thread);
			ifter->d_thread = NULL;
		}
		//udhcpc_time_cancel(ifter);
		udhcp_client_explain_lease(packet, ifter);
		ifter->state.state = DHCP_REQUESTING;
		if (DHCPC_DEBUG_ISON(STATE))
		{
			zlog_debug(MODULE_DHCP, "DHCP Client state change to REQUESTING");
		}
		udhcp_client_send_handle(ifter, DHCP_REQUESTING/*DHCPREQUEST*/);
		break;

	case DHCPACK:
		udhcp_client_explain_lease(packet, ifter);
		udhcpc_time_cancel(ifter);
		if ((ifter->state.state == DHCP_REQUESTING))
		{
			ifter->state.renew_timeout1 = ifter->lease.expires >> DHCP_T1;
			ifter->state.renew_timeout2 =
					(zpl_uint32) ((zpl_float) ifter->lease.expires * DHCP_T2);
			ifter->state.state = DHCP_BOUND;
			if (DHCPC_DEBUG_ISON(STATE))
			{
				zlog_debug(MODULE_DHCP, "DHCP Client state change to BOUND");
			}
			ifter->t_thread = eloop_add_timer(ifter->master, udhcpc_renew_event,
					ifter, ifter->lease.expires - ifter->state.renew_timeout1);
			zlog_debug(MODULE_DHCP,
					"udhcp_client_recv_handle DHCP_REQUESTING ACK and dhcp_client_lease_set");
			dhcp_client_lease_set(ifter);
		}
		else if (ifter->state.state == DHCP_RENEWING)
		{
			ifter->state.renew_timeout1 = ifter->lease.expires >> DHCP_T1;
			ifter->state.renew_timeout2 =
					(zpl_uint32) ((zpl_float) ifter->lease.expires * DHCP_T2);
			ifter->state.state = DHCP_BOUND;
			if (DHCPC_DEBUG_ISON(STATE))
			{
				zlog_debug(MODULE_DHCP, "DHCP Client state change to BOUND");
			}
			ifter->t_thread = eloop_add_timer(ifter->master, udhcpc_renew_event,
					ifter, ifter->lease.expires - ifter->state.renew_timeout1);
			zlog_debug(MODULE_DHCP,
					"udhcp_client_recv_handle DHCP_RENEWING ACK and dhcp_client_lease_set");
			dhcp_client_lease_set(ifter);
		}
		else if (ifter->state.state == DHCP_REBINDING)
		{
			ifter->state.renew_timeout1 = ifter->lease.expires >> DHCP_T1;
			ifter->state.renew_timeout2 =
					(zpl_uint32) ((zpl_float) ifter->lease.expires * DHCP_T2);
			ifter->state.state = DHCP_BOUND;
			if (DHCPC_DEBUG_ISON(STATE))
			{
				zlog_debug(MODULE_DHCP, "DHCP Client state change to BOUND");
			}
			ifter->t_thread = eloop_add_timer(ifter->master, udhcpc_renew_event,
					ifter, ifter->lease.expires - ifter->state.renew_timeout1);
			zlog_debug(MODULE_DHCP,
					"udhcp_client_recv_handle DHCP_REBINDING ACK and dhcp_client_lease_set");
			dhcp_client_lease_set(ifter);
		}
		if (ifter->d_thread)
		{
			eloop_cancel(ifter->d_thread);
			ifter->d_thread = NULL;
		}
		//udhcp_run_script(&packet, state == REQUESTING ? "bound" : "renew");
		udhcpc_state_mode_change(ifter, DHCP_UDP_MODE);
		break;

	case DHCPNAK:
	{

		/*			udhcpc_time_cancel(ifter);
		 ifter->state.state = DHCP_INIT;
		 ifter->lease.lease_address = 0;
		 ifter->state.dis_cnt = 0;
		 udhcpc_state_mode_change(ifter, DHCP_RAW_MODE);
		 */

		dhcp_client_lease_unset(ifter);
		udhcpc_stop(ifter);

		udhcpc_state_mode_change(ifter, DHCP_RAW_MODE);

		ifter->d_thread = eloop_add_timer(ifter->master, udhcpc_discover_event,
				ifter, ifter->state.dis_timeout + 5);
	}
		break;
	default:
		break;
	}
	return OK;
}
/* Returns -1 on errors that are fatal for the socket, -2 for those that aren't */
/* NOINLINE: limit stack usage in caller */
static int udhcp_recv_raw_packet(struct dhcp_packet *dhcp_pkt, zpl_socket_t fd,
		zpl_uint32 *ifindex)
{
	int bytes;
	struct ip_udp_dhcp_packet packet;
	zpl_uint16 check;
	zpl_uint8 cmsgbuf[IPSTACK_CMSG_SPACE(SOPT_SIZE_CMSG_IFINDEX_IPV4())];
	struct ipstack_iovec iov;
	struct ipstack_msghdr msg;

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
	for (;;)
	{
		bytes = ipstack_recvmsg(fd, &msg, 0);
		if (bytes < 0)
		{
			if (ipstack_errno == EINTR)
				continue;
			zlog_err(MODULE_DHCP, "packet read error, ignoring");
			/* NB: possible down interface, etc. Caller should pause. */
			return bytes; /* returns -1 */
		}
		break;
	}
	if (bytes < (int) (sizeof(packet.ip) + sizeof(packet.udp)))
	{
		zlog_err(MODULE_DHCP, "packet is too zpl_int16, ignoring");
		return -2;
	}

	if (bytes < ntohs(packet.ip.tot_len))
	{
		/* packet is bigger than sizeof(packet), we did partial read */
		zlog_err(MODULE_DHCP, "oversized packet, ignoring");
		return -2;
	}

	/* ignore any extra garbage bytes */
	bytes = ntohs(packet.ip.tot_len);

	/* make sure its the right packet for us, and that it passes sanity checks */
	if (packet.ip.protocol != IPSTACK_IPPROTO_UDP || packet.ip.version != IPVERSION
			|| packet.ip.ihl != (sizeof(packet.ip) >> 2)
			|| packet.udp.dest != htons(DHCP_CLIENT_PORT)
			/* || bytes > (int) sizeof(packet) - can't happen */
			|| ntohs(packet.udp.len) != (zpl_uint16) (bytes - sizeof(packet.ip)))
	{
		zlog_err(MODULE_DHCP, "unrelated/bogus packet, ignoring");
		return -2;
	}

	/* verify IP checksum */
	check = packet.ip.check;
	packet.ip.check = 0;
	if (check != in_cksum((zpl_uint16 *) &packet.ip, sizeof(packet.ip)))
	{
		zlog_err(MODULE_DHCP, "bad IP header checksum, ignoring");
		return -2;
	}

	if (ifindex)
		*ifindex = getsockopt_ifindex(IPSTACK_AF_INET, &msg);

	/* verify UDP checksum. IP header has to be modified for this */
	memset(&packet.ip, 0, offsetof(struct iphdr, protocol));
	/* ip.xx fields which are not memset: protocol, check, saddr, daddr */
	packet.ip.tot_len = packet.udp.len; /* yes, this is needed */
	check = packet.udp.check;
	packet.udp.check = 0;
	if (check && check != in_cksum((zpl_uint16 *) &packet, bytes))
	{
		zlog_err(MODULE_DHCP, "packet with bad UDP checksum received, ignoring");
		return -2;
	}
	if (packet.data.cookie != htonl(DHCP_MAGIC))
	{
		zlog_err(MODULE_DHCP, "packet with bad magic, ignoring");
		return -2;
	}

	bytes -= sizeof(packet.ip) + sizeof(packet.udp);
	memcpy(dhcp_pkt, &packet.data, bytes);
	return bytes;
}

static int udhcpc_raw_read_thread(struct eloop *eloop)
{
	struct dhcp_packet packet;
	int bytes = 0;
	zpl_socket_t sock;
	client_interface_t * ifter = NULL;
	sock = ELOOP_FD(eloop);
	ifter = ELOOP_ARG(eloop);
	if(!ifter)
	{
		dhcp_client_lease_unset(ifter);
		udhcpc_stop(ifter);
		//ifter->state.mode = DHCP_UDP_MODE;
		udhcpc_state_mode_change(ifter, DHCP_RAW_MODE);

		ifter->d_thread = eloop_add_timer(ifter->master,
				udhcpc_discover_event, ifter,
				ifter->state.dis_timeout + 5);

		return ERROR;
	}
	ifter->r_thread = NULL;

	if (ifter->state.mode == DHCP_RAW_MODE)
	{
		bytes = udhcp_recv_raw_packet(&packet, sock, NULL);
		if (bytes < 0)
		{
			/* bytes can also be -2 ("bad packet data") */
			if (bytes == -1 && ipstack_errno != EINTR)
			{
				zlog_err(MODULE_DHCP, "received error %s",strerror(ipstack_errno));
				dhcp_client_lease_unset(ifter);
				udhcpc_stop(ifter);
				//ifter->state.mode = DHCP_UDP_MODE;
				udhcpc_state_mode_change(ifter, DHCP_RAW_MODE);

				ifter->d_thread = eloop_add_timer(ifter->master,
						udhcpc_discover_event, ifter,
						ifter->state.dis_timeout + 5);

				return ERROR;
			}
		}
	}
	ifter->state.read_bytes = bytes;
	udhcp_client_recv_handle(&packet, ifter);
	if(!ifter->r_thread)
		ifter->r_thread = eloop_add_read(eloop->master, udhcpc_raw_read_thread, ifter, sock);
	return OK;
}

static int udhcpc_udp_read_thread(struct eloop *eloop)
{
	struct dhcp_packet packet;
	int bytes = 0;
	zpl_socket_t sock;
	zpl_uint32 ifindex = 0;
	client_interface_t * ifter = NULL;
	sock = ELOOP_FD(eloop);

	/*	ifter = ELOOP_ARG(eloop);
	 ifter->r_thread = NULL;*/

	bytes = udhcp_recv_packet(&packet, sock, &ifindex);
	if (bytes < 0)
	{
		/* bytes can also be -2 ("bad packet data") */
		if (bytes == -1 && ipstack_errno != EINTR)
		{
			//ifter->r_thread = eloop_add_read(eloop->master, udhcpc_udp_read_thread, ifter, sock);
			return ERROR;
		}
	}
	ifter = dhcp_client_lookup_interface(&dhcp_global_config, ifindex);
	if (ifter == NULL)
	{
		zlog_err(MODULE_DHCP, " this Interface is not allow DHCP");
		return ERROR;
	}
	if (ifter->state.mode == DHCP_RAW_MODE)
	{
		ifter->r_thread = eloop_add_read(eloop->master, udhcpc_udp_read_thread,
				NULL, sock);
		zlog_err(MODULE_DHCP, " this Interface is running in RAW MODE");
		return ERROR;
	}
	ifter->r_thread = NULL;

	ifter->state.read_bytes = bytes;
	udhcp_client_recv_handle(&packet, ifter);

	if (!ifter->r_thread)
		ifter->r_thread = eloop_add_read(eloop->master, udhcpc_udp_read_thread,
				NULL, sock);
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

	dhcp_option_add(ifter->options, DHCP_CLIENT_ID, ifter->client_mac,
			ETHER_ADDR_LEN);
	if (host_name_get())
		dhcp_option_add(ifter->options, DHCP_HOST_NAME, host_name_get(),
				strlen(host_name_get()));

	if (IF_VLAN_GET(ifter->ifindex) > 0)
		dhcp_option_add_32bit(ifter->options, DHCP_VLAN_ID,
				IF_VLAN_GET(ifter->ifindex));

	return OK;
}
/************************************************************************************/
static client_interface_t * dhcp_client_create_interface(zpl_uint32 ifindex)
{
	client_interface_t *ifter = XMALLOC(MTYPE_DHCPC_INFO,
			sizeof(client_interface_t));
	if (ifter)
	{
		//struct interface * ifp = if_lookup_by_index(ifindex);
		memset(ifter, 0, sizeof(client_interface_t));

		ifter->ifindex = ifindex;
		ifter->port = DHCP_CLIENT_PORT;

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

		//zlog_debug(MODULE_DHCP, "===========%s", ifp->k_name);
		return ifter;
	}
	return NULL;
}

client_interface_t * dhcp_client_lookup_interface(dhcp_global_t*config,
		ifindex_t ifindex)
{
	client_interface_t *pstNode = NULL;
	NODE index;
	if (!lstCount(&config->client_list))
		return NULL;
	for (pstNode = (client_interface_t *) lstFirst(&config->client_list);
			pstNode != NULL;
			pstNode = (client_interface_t *) lstNext((NODE*) &index))
	{
		index = pstNode->node;
		if (pstNode->ifindex == ifindex)
		{
			return pstNode;
		}
	}
	return NULL;
}

int dhcp_client_add_interface(dhcp_global_t*config, ifindex_t ifindex)
{
	client_interface_t * ifter = dhcp_client_create_interface(ifindex);
	if (ifter)
	{
		ifter->master = config->eloop_master;
		ifter->sock = udhcpc_client_socket(ifter->ifindex);
		if (!ipstack_invalid(ifter->sock))
		{
			lstAdd(&config->client_list, ifter);
			ifter->state.mode = DHCP_RAW_MODE;
			if (ifter->d_thread == NULL)
				ifter->d_thread = eloop_add_timer(ifter->master,
						udhcpc_discover_event, ifter, 2);

			if (ifter->r_thread == NULL)
				ifter->r_thread = eloop_add_read(ifter->master,
						udhcpc_raw_read_thread, ifter, ifter->sock);
			return OK;
		}
	}
	return ERROR;
}

int dhcp_client_del_interface(dhcp_global_t*config, ifindex_t ifindex)
{
	client_interface_t * ifter = dhcp_client_lookup_interface(config, ifindex);
	if (ifter)
	{
		udhcp_client_release(ifter, ifter->lease.server_address,
				 ifter->lease.lease_address);

		udhcpc_time_cancel(ifter);

		if (ifter->r_thread)
		{
			eloop_cancel(ifter->r_thread);
			ifter->r_thread = NULL;
		}
		if (!ipstack_invalid(ifter->sock))
		{
			ipstack_close(ifter->sock);
			//ifter->sock = 0;
		}

		//dhcp_client_lease_unset(ifter);
		lstDelete(&config->client_list, ifter);
		XFREE(MTYPE_DHCPC_INFO, ifter);
		return OK;
	}
	return ERROR;
}

int dhcp_client_interface_option_set(client_interface_t * ifter, zpl_uint16 code,
		zpl_uint8 *str, zpl_uint32 len)
{
	if (code > DHCP_OPTION_MAX)
	{
		if (code == 255 + 'm')
		{
			ifter->instance = atoi(str);
			return OK;
		}
		return ERROR;
	}
	dhcp_option_add(ifter->options, (zpl_uint8) code, str, len);
	return OK;
}

int dhcp_client_interface_request_set(client_interface_t * ifter, zpl_uint16 code,
		zpl_bool enable)
{
	ifter->opt_mask[code] = enable;
	return OK;
}

int dhcp_client_interface_clean(void)
{
	client_interface_t *pstNode = NULL;
	NODE index;
	if (!lstCount(&dhcp_global_config.client_list))
		return OK;
	for (pstNode = (client_interface_t *) lstFirst(
			&dhcp_global_config.client_list); pstNode != NULL; pstNode =
			(client_interface_t *) lstNext((NODE*) &index))
	{
		index = pstNode->node;
		if (pstNode)
		{
			udhcp_client_release(pstNode, pstNode->lease.server_address,
					pstNode->lease.lease_address);

			lstDelete(&dhcp_global_config.client_list, (NODE*) pstNode);
			udhcpc_time_cancel(pstNode);
			if (pstNode->r_thread)
			{
				eloop_cancel(pstNode->r_thread);
				pstNode->r_thread = NULL;
			}
			if (!ipstack_invalid(pstNode->sock))
			{
				ipstack_close(pstNode->sock);
				//pstNode->sock = 0;
			}

			dhcp_client_lease_unset(pstNode);
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


/*
 *
报文类型：

1）DHCPDISCOVER（0x01），此为Client开始DHCP过程的第一个报文

2）DHCPOFFER（0x02），此为Server对DHCPDISCOVER报文的响应

3）DHCPREQUEST（0x03），此报文是Client开始DHCP过程中对server的DHCPOFFER报文的回应，或者是Client续延IP地址租期时发出的报文

4）DHCPDECLINE（0x04），当Client发现Server分配给它的IP地址无法使用，如IP地址冲突时，将发出此报文，通知Server禁止使用IP地址

5）DHCPACK（0x05），Server对Client的DHCPREQUEST报文的确认响应报文，Client收到此报文后，才真正获得了IP地址和相关的配置信息。

6）DHCPNAK（0x06），Server对Client的DHCPREQUEST报文的拒绝响应报文，Client收到此报文后，一般会重新开始新的DHCP过程。

7）DHCPRELEASE（0x07），Client主动释放server分配给它的IP地址的报文，当Server收到此报文后，就可以回收这个IP地址，能够分配给其他的Client。

8）DHCPINFORM（0x08），Client已经获得了IP地址，发送此报文，只是为了从DHCP SERVER处获取其他的一些网络配置信息，如route ip，DNS Ip等，这种报文的应用非常少见。
 *
 */
