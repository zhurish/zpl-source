/* vi: set sw=4 ts=4: */
/*
 * Port to ZPLSource Copyright (C) 2006 Jesse Dutton <jessedutton@gmail.com>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 *
 * DHCP Relay for 'DHCPv4 Configuration of IPSec Tunnel Mode' support
 * Copyright (C) 2002 Mario Strasser <mast@gmx.net>,
 *                   Zuercher Hochschule Winterthur,
 *                   Netbeat AG
 * Upstream has GPL v2 or later
 */
//applet:IF_DHCPRELAY(APPLET(dhcprelay, BB_DIR_USR_SBIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_DHCPRELAY) += dhcprelay.o

//usage:#define dhcprelay_trivial_usage
//usage:       "CLIENT_IFACE[,CLIENT_IFACE2]... SERVER_IFACE [SERVER_IP]"
//usage:#define dhcprelay_full_usage "\n\n"
//usage:       "Relay DHCP requests between clients and server"

#include "dhcp_def.h"
#include "dhcp_pool.h"
#include "dhcp_util.h"
#include "dhcp_lease.h"
#include "dhcp_main.h"
#include "dhcpd.h"
#include "dhcp_packet.h"
#include "dhcprelay.h"

/* lifetime of an xid entry in sec. */
#define DHCP_RELAY_MAX_LIFETIME   2*60
/* select timeout in sec. */
#define DHCP_RELAY_SELECT_TIMEOUT (DHCP_RELAY_MAX_LIFETIME / 8)



/************************************************************************************/
/* This list holds information about clients. The xid_* functions manipulate this list. */
struct dhcp_relay_xid_item {
	unsigned timestamp;
	int client;
	zpl_uint32  xid;
	struct ipstack_sockaddr_in ip;
	struct dhcp_relay_xid_item *next;
} ;

struct dhcp_relay_xid_item dhcprelay_xid_list;


static int udhcp_relay_read_thread(struct eloop *eloop);
static int udhcp_server_read_thread(struct eloop *eloop);
static void dhcp_relay_forward_client(dhcp_relay_t * ifter, struct dhcp_packet *p, int packet_len);
static void dhcp_relay_forward_server(dhcp_relay_t * ifter, struct dhcp_packet *p, int packet_len,
			struct ipstack_sockaddr_in *client_addr, struct ipstack_sockaddr_in *server_addr);
			

static struct dhcp_relay_xid_item *dhcp_relay_xid_add(zpl_uint32  xid, struct ipstack_sockaddr_in *ip, int client)
{
	struct dhcp_relay_xid_item *item;

	/* create new xid entry */
	item = malloc(sizeof(struct dhcp_relay_xid_item));

	/* add xid entry */
	item->ip = *ip;
	item->xid = xid;
	item->client = client;
	item->timestamp = os_monotonic_time();
	item->next = dhcprelay_xid_list.next;
	dhcprelay_xid_list.next = item;

	return item;
}

static void dhcp_relay_xid_expire(void)
{
	struct dhcp_relay_xid_item *item = dhcprelay_xid_list.next;
	struct dhcp_relay_xid_item *last = &dhcprelay_xid_list;
	unsigned current_time = os_monotonic_time();

	while (item != NULL) {
		if ((current_time - item->timestamp) > DHCP_RELAY_MAX_LIFETIME) {
			last->next = item->next;
			free(item);
			item = last->next;
		} else {
			last = item;
			item = item->next;
		}
	}
}

static struct dhcp_relay_xid_item *dhcp_relay_xid_find(zpl_uint32  xid)
{
	struct dhcp_relay_xid_item *item = dhcprelay_xid_list.next;
	while (item != NULL) {
		if (item->xid == xid) {
			break;
		}
		item = item->next;
	}
	return item;
}

static void dhcp_relay_xid_del(zpl_uint32  xid)
{
	struct dhcp_relay_xid_item *item = dhcprelay_xid_list.next;
	struct dhcp_relay_xid_item *last = &dhcprelay_xid_list;
	while (item != NULL) {
		if (item->xid == xid) {
			last->next = item->next;
			free(item);
			item = last->next;
		} else {
			last = item;
			item = item->next;
		}
	}
}

static int udhcp_relay_time_thread(struct eloop *eloop)
{
	dhcp_relay_t * ifter = NULL;
	dhcp_global_t *config = NULL;
	ifter = ELOOP_ARG(eloop);	
	config = ifter->global;
	ifter->t_thread = NULL;
	dhcp_relay_xid_expire();
	ifter->t_thread = eloop_add_timer(config->eloop_master, udhcp_relay_time_thread,
			ifter, DHCP_RELAY_SELECT_TIMEOUT);	
	return 0;		
}
/**************************************************************/
static dhcp_relay_t * dhcp_relay_create_interface(zpl_uint32 ifindex) {
	dhcp_relay_t *ifter = XMALLOC(MTYPE_DHCPR_INFO,
			sizeof(dhcp_relay_t));
	if (ifter) {
		struct interface * ifp = if_lookup_by_index(ifindex);
		memset(ifter, 0, sizeof(dhcp_relay_t));
		ifter->ifindex = ifindex;
		//ifter->port = SERVER_PORT;
		/*
		 if(ifp && strlen(ifp->ker_name))
		 ifter->ker_name = strdup(ifp->ker_name);
		 if(ifter->ker_name)
		 {
		 udhcp_read_interface(ifter->ker_name, NULL, &ifter->ipaddr, ifter->server_mac);
		 zlog_debug(MODULE_DHCP, "===========%s", ifter->ker_name);
		 }
		 */
		udhcp_interface_mac(ifindex, &ifter->ipaddr, ifter->mac);
		zlog_debug(MODULE_DHCP, "===========%s", ifp->ker_name);
		return ifter;
		//ifter->port;
		//ifter->server_mac[6];          /* our MAC address (used only for ARP probing) */
	}
	return NULL;
}


dhcp_relay_t * dhcp_relay_lookup_interface(dhcp_global_t*config, ifindex_t ifindex) {
	dhcp_relay_t *pstNode = NULL;
	NODE index;
	if (!lstCount(&config->relay_list))
		return NULL;
	for (pstNode = (dhcp_relay_t *) lstFirst(&config->relay_list);
			pstNode != NULL;
			pstNode = (dhcp_relay_t *) lstNext((NODE*) &index)) {
		index = pstNode->node;
		if (pstNode->ifindex == ifindex) {
			return pstNode;
		}
	}
	return NULL;
}

int dhcp_relay_add_interface(dhcp_global_t*config, ifindex_t ifindex) {
	dhcp_relay_t * ifter = dhcp_relay_create_interface(ifindex);
	if (ifter) {
		lstAdd(&config->relay_list, ifter);
		if(ipstack_invalid(config->sock))
			config->sock = udhcp_udp_socket(0, 0);
		if(ipstack_invalid(ifter->client_sock))
			ifter->client_sock = udhcp_udp_socket(config->server_port, ifindex);

		zlog_debug(MODULE_DHCP, "dhcp_relay_add_interface -> udhcp_udp_socket udhcp_raw_socket");

		if (ifter->cr_thread == NULL && !ipstack_invalid(ifter->client_sock))
		{
			zlog_debug(MODULE_DHCP, "dhcp_relay_add_interface");
			ifter->cr_thread = eloop_add_read(config->eloop_master, udhcp_relay_read_thread,
				ifter, ifter->client_sock);
		}
		if (ifter->sr_thread == NULL && !ipstack_invalid(config->sock))
		{
			zlog_debug(MODULE_DHCP, "dhcp_relay_add_interface");
			ifter->sr_thread = eloop_add_read(config->eloop_master, udhcp_server_read_thread,
				ifter, config->sock);
		}
		if (ifter->t_thread == NULL)
			ifter->t_thread = eloop_add_timer(config->eloop_master, udhcp_relay_time_thread,
				ifter, DHCP_RELAY_SELECT_TIMEOUT);	
		return OK;
	}
	return ERROR;
}

int dhcp_relay_del_interface(dhcp_global_t*config, ifindex_t ifindex) {
	dhcp_relay_t * ifter = dhcp_relay_lookup_interface(config, ifindex);
	if (ifter) {
		if(ifter->cr_thread)
		{
			eloop_cancel(ifter->cr_thread);
		}
		if(ifter->sr_thread)
		{
			eloop_cancel(ifter->sr_thread);
		}
		if(!ipstack_invalid(ifter->client_sock))
		{
			ipstack_close(ifter->client_sock);
		}
		lstDelete(&config->relay_list, ifter);
		XFREE(MTYPE_DHCPR_INFO, ifter);
		return OK;
	}
	return ERROR;
}


static int udhcp_relay_read_thread(struct eloop *eloop)
{
	struct dhcp_packet packet;
	zpl_socket_t sock;
	zpl_uint32 ifindex = 0;
	dhcp_relay_t * ifter = NULL;
	dhcp_global_t *config = NULL;
	struct interface *ifp = NULL;
	int bytes;
	sock = ELOOP_FD(eloop);
	ifter = ELOOP_ARG(eloop);		
	config = ifter->global;	
	memset(&packet, 0, sizeof(struct dhcp_packet));
	bytes = udhcp_recv_packet(&packet, sock, &ifindex);
	if (bytes > 0) 
	{
		struct ipstack_sockaddr_in server_addr;
		struct ipstack_sockaddr_in client_addr;
		server_addr.sin_family = IPSTACK_AF_INET;
		server_addr.sin_addr.s_addr = htonl(IPSTACK_INADDR_BROADCAST);
		server_addr.sin_port = htons(DHCP_SERVER_PORT);
		ifp = if_lookup_by_index(ifindex);
		if(ifp)
		{
			struct prefix address;
			;
			if (nsm_interface_address_get_api(ifp, &address) == OK) 
			{
				packet.gateway_nip = address.u.prefix4.s_addr;
			}
			dhcp_relay_forward_server(ifter, &packet, bytes,
				&client_addr, &server_addr);
		}
	}
	ifter->sr_thread = eloop_add_read(config->eloop_master, udhcp_server_read_thread,
			ifter, config->sock);	
	return 0;
}

static int udhcp_server_read_thread(struct eloop *eloop)
{
	struct dhcp_packet packet;
	zpl_socket_t sock;
	zpl_uint32 ifindex = 0;
	dhcp_relay_t * ifter = NULL;
	dhcp_global_t *config = NULL;
	int bytes;
	sock = ELOOP_FD(eloop);
	ifter = ELOOP_ARG(eloop);	
	config = ifter->global;	
	ifter->sr_thread = NULL;
	memset(&packet, 0, sizeof(struct dhcp_packet));
	bytes = udhcp_recv_packet(&packet, sock, &ifindex);
	if (bytes > 0) 
	{
		dhcp_relay_forward_client(ifter, &packet, bytes);
	}
	ifter->sr_thread = eloop_add_read(config->eloop_master, udhcp_server_read_thread,
			ifter, config->sock);	
	return 0;		
}

/************************************************************************************/
/************************************************************************************/
/**
 * get_dhcp_packet_type - gets the message type of a dhcp packet
 * p - pointer to the dhcp packet
 * returns the message type on success, -1 otherwise
 */
static int dhcp_relay_get_packet_type(struct dhcp_packet *p)
{
	zpl_uint8 op = 0;
	/* it must be either a DHCP_BOOTREQUEST or a DHCP_BOOTREPLY */
	if (p->op != DHCP_BOOTREQUEST && p->op != DHCP_BOOTREPLY)
		return -1;
	/* get message type option */
	op = dhcp_option_message_type_get(p->options, sizeof(p->options));
	if (op != 0)
		return op;
	return -1;
}

static int dhcp_relay_forward(zpl_socket_t sock, const void *msg, int msg_len, struct ipstack_sockaddr_in *to)
{
	int err;
	ipstack_errno = 0;
	err = ipstack_sendto(sock, msg, msg_len, 0, (struct ipstack_sockaddr*) to, sizeof(*to));
	err -= msg_len;
	if (err)
		zlog_err(MODULE_DHCP,"sendto");
	return err;
}

/**
 * pass_to_server() - forwards dhcp packets from client to server
 * p - packet to send
 * client - number of the client
 */
static void dhcp_relay_forward_server(dhcp_relay_t * ifter, struct dhcp_packet *p, int packet_len,
			struct ipstack_sockaddr_in *client_addr, struct ipstack_sockaddr_in *server_addr)
{
	zpl_uint32 type;
	/* check packet_type */
	type = dhcp_relay_get_packet_type(p);
	if (type != DHCP_MESSAGE_DISCOVER && type != DHCP_MESSAGE_REQUEST
	 && type != DHCP_MESSAGE_DECLINE && type != DHCP_MESSAGE_RELEASE
	 && type != DHCP_MESSAGE_INFORM
	) 
	{
		return;
	}
	/* create new xid entry */
	dhcp_relay_xid_add(p->xid, client_addr, 1);

	/* forward request to server */
	/* note that we send from fds[0] which is bound to SERVER_PORT (67).
	 * IOW: we send _from_ SERVER_PORT! Although this may look strange,
	 * RFC 1542 not only allows, but prescribes this for BOOTP relays.
	 */
	dhcp_relay_forward(ifter->sock, p, packet_len, server_addr);
}

/**
 * pass_to_client() - forwards dhcp packets from server to client
 * p - packet to send
 */
static void dhcp_relay_forward_client(dhcp_relay_t * ifter, struct dhcp_packet *p, int packet_len)
{
	zpl_uint32 type;
	struct dhcp_relay_xid_item *item;

	/* check xid */
	item = dhcp_relay_xid_find(p->xid);
	if (!item) {
		return;
	}

	/* check packet type */
	type = dhcp_relay_get_packet_type(p);
	if (type != DHCP_MESSAGE_OFFER && type != DHCP_MESSAGE_ACK && type != DHCP_MESSAGE_NAK) {
		return;
	}

	//TODO: also do it if (p->flags & htons(DHCP_PACKET_FLAG_BROADCAST)) is set!
	if (item->ip.sin_addr.s_addr == htonl(IPSTACK_INADDR_ANY))
		item->ip.sin_addr.s_addr = htonl(IPSTACK_INADDR_BROADCAST);

	if (dhcp_relay_forward(ifter->client_sock, p, packet_len, &item->ip) != 0) {
		return; /* send error occurred */
	}

	/* remove xid entry */
	dhcp_relay_xid_del(p->xid);
}


#if 0
/************************************************************************************/
/**
 * make_iface_list - parses client/server interface names
 * returns array
 */
static char **dhcp_relay_make_iface_list(char **client_and_server_ifaces, int *client_number)
{
	char *s, **iface_list;
	int i, cn;

	/* get number of items */
	cn = 2; /* 1 server iface + at least 1 client one */
	s = client_and_server_ifaces[0]; /* list of client ifaces */
	while (*s) {
		if (*s == ',')
			cn++;
		s++;
	}
	*client_number = cn;

	/* create vector of pointers */
	iface_list = malloc(cn * sizeof(iface_list[0]));

	iface_list[0] = client_and_server_ifaces[1]; /* server iface */

	i = 1;
	s = strdup(client_and_server_ifaces[0]); /* list of client ifaces */
	goto store_client_iface_name;

	while (i < cn) {
		if (*s++ == ',') {
			s[-1] = '\0';
 store_client_iface_name:
			iface_list[i++] = s;
		}
	}

	return iface_list;
}

/* Creates listen sockets (in fds) bound to client and server ifaces,
 * and returns numerically max fd.
 */
static int dhcp_relay_init_sockets(char **iface_list, int num_clients, zpl_socket_t *fds)
{
	int i, n;

	n = 0;
	for (i = 0; i < num_clients; i++) {
		fds[i] = udhcp_udp_socket(/*IPSTACK_INADDR_ANY,*/ DHCP_SERVER_PORT, 0);
	}
	return n;
}

static int dhcp_relay_sendto_ip4(zpl_socket_t sock, const void *msg, int msg_len, struct ipstack_sockaddr_in *to)
{
	int err;

	ipstack_errno = 0;
	err = ipstack_sendto(sock, msg, msg_len, 0, (struct ipstack_sockaddr*) to, sizeof(*to));
	err -= msg_len;
	if (err)
		zlog_err(MODULE_DHCP,"sendto");
	return err;
}

/**
 * pass_to_server() - forwards dhcp packets from client to server
 * p - packet to send
 * client - number of the client
 */
static void dhcp_relay_pass_to_server(struct dhcp_packet *p, int packet_len, int client, zpl_socket_t *fds,
			struct ipstack_sockaddr_in *client_addr, struct ipstack_sockaddr_in *server_addr)
{
	zpl_uint32 type;

	/* check packet_type */
	type = dhcp_relay_get_packet_type(p);
	if (type != DHCP_MESSAGE_DISCOVER && type != DHCP_MESSAGE_REQUEST
	 && type != DHCP_MESSAGE_DECLINE && type != DHCP_MESSAGE_RELEASE
	 && type != DHCP_MESSAGE_INFORM
	) {
		return;
	}

	/* create new xid entry */
	dhcp_relay_xid_add(p->xid, client_addr, client);

	/* forward request to server */
	/* note that we send from fds[0] which is bound to SERVER_PORT (67).
	 * IOW: we send _from_ SERVER_PORT! Although this may look strange,
	 * RFC 1542 not only allows, but prescribes this for BOOTP relays.
	 */
	dhcp_relay_sendto_ip4(fds[0], p, packet_len, server_addr);
}

/**
 * pass_to_client() - forwards dhcp packets from server to client
 * p - packet to send
 */
static void dhcp_relay_pass_to_client(struct dhcp_packet *p, int packet_len, zpl_socket_t *fds)
{
	zpl_uint32 type;
	struct dhcp_relay_xid_item *item;

	/* check xid */
	item = dhcp_relay_xid_find(p->xid);
	if (!item) {
		return;
	}

	/* check packet type */
	type = dhcp_relay_get_packet_type(p);
	if (type != DHCP_MESSAGE_OFFER && type != DHCP_MESSAGE_ACK && type != DHCP_MESSAGE_NAK) {
		return;
	}

//TODO: also do it if (p->flags & htons(DHCP_PACKET_FLAG_BROADCAST)) is set!
	if (item->ip.sin_addr.s_addr == htonl(IPSTACK_INADDR_ANY))
		item->ip.sin_addr.s_addr = htonl(IPSTACK_INADDR_BROADCAST);

	if (dhcp_relay_sendto_ip4(fds[item->client], p, packet_len, &item->ip) != 0) {
		return; /* send error occurred */
	}

	/* remove xid entry */
	dhcp_relay_xid_del(p->xid);
}


int dhcprelay_main(int argc, char **argv)
{
	struct ipstack_sockaddr_in server_addr;
	char **iface_list;
	zpl_socket_t *fds;
	int num_sockets, max_socket;
	zpl_uint32  our_nip;

	//INIT_G();

	server_addr.sin_family = IPSTACK_AF_INET;
	server_addr.sin_addr.s_addr = htonl(IPSTACK_INADDR_BROADCAST);
	server_addr.sin_port = htons(DHCP_SERVER_PORT);

	/* dhcprelay CLIENT_IFACE1[,CLIENT_IFACE2...] SERVER_IFACE [SERVER_IP] */
	if (argc == 4) {
		if (!ipstack_inet_aton(argv[3], &server_addr.sin_addr))
			;//bb_perror_msg_and_die("bad server IP");
	} else if (argc != 3) {
		//bb_show_usage();
	}

	iface_list = dhcp_relay_make_iface_list(argv + 1, &num_sockets);

	fds = malloc(num_sockets * sizeof(zpl_socket_t));

	/* Create sockets and bind one to every iface */
	max_socket = dhcp_relay_init_sockets(iface_list, num_sockets, fds);

	/* Get our IP on server_iface */
	if (udhcp_read_interface(argv[2], NULL, &our_nip, NULL))
		return 1;

	/* Main loop */
	while (1) {
		// reinit stuff from time to time? go back to make_iface_list
		// every N minutes?
		fd_set rfds;
		struct timeval tv;
		int i;

		IPSTACK_FD_ZERO(&rfds);
		for (i = 0; i < num_sockets; i++)
			IPSTACK_FD_SET(ipstack_fd(fds[i]), &rfds);
		tv.tv_sec = DHCP_RELAY_SELECT_TIMEOUT;
		tv.tv_usec = 0;
		if (ipstack_socketselect(IPSTACK_IPCOM, max_socket + 1, &rfds, NULL, NULL, &tv) > 0) {
			int packlen;
			struct dhcp_packet dhcp_msg;

			/* server */
			if (IPSTACK_FD_ISSET(ipstack_fd(fds[0]), &rfds)) {
				packlen = udhcp_recv_packet(&dhcp_msg, fds[0], NULL);//udhcp_recv_kernel_packet(&dhcp_msg, fds[0]);
				if (packlen > 0) {
					dhcp_relay_pass_to_client(&dhcp_msg, packlen, fds);
				}
			}

			/* clients */
			for (i = 1; i < num_sockets; i++) {
				struct ipstack_sockaddr_in client_addr;
				socklen_t addr_size;

				if (!IPSTACK_FD_ISSET(ipstack_fd(fds[i]), &rfds))
					continue;

				addr_size = sizeof(client_addr);
				packlen = ipstack_recvfrom(fds[i], &dhcp_msg, sizeof(dhcp_msg), 0,
						(struct ipstack_sockaddr *)(&client_addr), &addr_size);
				if (packlen <= 0)
					continue;

				/* Get our IP on corresponding client_iface */
// RFC 1542
// 4.1 General BOOTP Processing for Relay Agents
// 4.1.1 DHCP_BOOTREQUEST Messages
//   If the relay agent does decide to relay the request, it MUST examine
//   the 'giaddr' ("gateway" IP address) field.  If this field is zero,
//   the relay agent MUST fill this field with the IP address of the
//   interface on which the request was received.  If the interface has
//   more than one IP address logically associated with it, the relay
//   agent SHOULD choose one IP address associated with that interface and
//   use it consistently for all BOOTP messages it relays.  If the
//   'giaddr' field contains some non-zero value, the 'giaddr' field MUST
//   NOT be modified.  The relay agent MUST NOT, under any circumstances,
//   fill the 'giaddr' field with a broadcast address as is suggested in
//   [1] (Section 8, sixth paragraph).

// but why? what if server can't route such IP? Client ifaces may be, say, NATed!

// 4.1.2 DHCP_BOOTREPLY Messages
//   BOOTP relay agents relay DHCP_BOOTREPLY messages only to BOOTP clients.
//   It is the responsibility of BOOTP servers to send DHCP_BOOTREPLY messages
//   directly to the relay agent identified in the 'giaddr' field.
// (yeah right, unless it is impossible... see comment above)
//   Therefore, a relay agent may assume that all DHCP_BOOTREPLY messages it
//   receives are intended for BOOTP clients on its directly-connected
//   networks.
//
//   When a relay agent receives a DHCP_BOOTREPLY message, it should examine
//   the BOOTP 'giaddr', 'yiaddr', 'chaddr', 'htype', and 'hlen' fields.
//   These fields should provide adequate information for the relay agent
//   to deliver the DHCP_BOOTREPLY message to the client.
//
//   The 'giaddr' field can be used to identify the logical interface from
//   which the reply must be sent (i.e., the host or router interface
//   connected to the same network as the BOOTP client).  If the content
//   of the 'giaddr' field does not match one of the relay agent's
//   directly-connected logical interfaces, the DHCP_BOOTREPLY message MUST be
//   silently discarded.
				if (udhcp_read_interface(iface_list[i], NULL, &dhcp_msg.gateway_nip, NULL)) {
					/* Fall back to our IP on server iface */
// this makes more sense!
					dhcp_msg.gateway_nip = our_nip;
				}
// maybe dhcp_msg.hops++? drop packets with too many hops (RFC 1542 says 4 or 16)?
				dhcp_relay_pass_to_server(&dhcp_msg, packlen, i, fds, &client_addr, &server_addr);
			}
		}
		dhcp_relay_xid_expire();
	} /* while (1) */

	/* return 0; - not reached */
}

#endif