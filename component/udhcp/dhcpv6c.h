/**
 * Copyright (C) 2012-2014 Steven Barth <steven@midlink.org>
 * Copyright (C) 2018 Hans Dedecker <dedeckeh@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DHCPV6C_H__
#define __DHCPV6C_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_DHCPV6C_MODULE

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>



#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define ND_OPT_RECURSIVE_DNS 25
#define ND_OPT_DNSSL 31

#define DHCPV6C_SOL_MAX_RT 120
#define DHCPV6C_REQ_MAX_RT 30
#define DHCPV6C_CNF_MAX_RT 4
#define DHCPV6C_REN_MAX_RT 600
#define DHCPV6C_REB_MAX_RT 600
#define DHCPV6C_INF_MAX_RT 120

#define RA_MIN_ADV_INTERVAL 3   /* RFC 4861 paragraph 6.2.1 */


#define ALL_DHCPV6C_RELAYS {{{0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02}}}

#define DHCPV6C_CLIENT_PORT 546
#define DHCPV6C_SERVER_PORT 547
#define DHCPV6C_DUID_LLADDR 3
#define DHCPV6C_REQ_DELAY 1

#define DHCPV6C_SOL_MAX_RT_MIN 60
#define DHCPV6C_SOL_MAX_RT_MAX 86400
#define DHCPV6C_INF_MAX_RT_MIN 60
#define DHCPV6C_INF_MAX_RT_MAX 86400

enum dhcpv6c_opt_npt {
	NTP_SRV_ADDR = 1,
	NTP_MC_ADDR = 2,
	NTP_SRV_FQDN = 3
};

enum dhcpv6c_msg {
	DHCPV6C_MSG_UNKNOWN = 0,
	DHCPV6C_MSG_SOLICIT = 1,
	DHCPV6C_MSG_ADVERT = 2,
	DHCPV6C_MSG_REQUEST = 3,
	DHCPV6C_MSG_CONFIRM = 4,
	DHCPV6C_MSG_RENEW = 5,
	DHCPV6C_MSG_REBIND = 6,
	DHCPV6C_MSG_REPLY = 7,
	DHCPV6C_MSG_RELEASE = 8,
	DHCPV6C_MSG_DECLINE = 9,
	DHCPV6C_MSG_RECONF = 10,
	DHCPV6C_MSG_INFO_REQ = 11,
	DHCPV6C_MSG_RELAY_FORW = 12,
	DHCPV6C_MSG_RELAY_REPL= 13,	
	_DHCPV6C_MSG_MAX
};
/*
   Name         Code Description
   ----------   ---- -----------
   Success         0 Success.
   UnspecFail      1 Failure, reason unspecified; this
                     status code is sent by either a client
                     or a server to indicate a failure
                     not explicitly specified in this
                     document.
   NoAddrsAvail    2 Server has no addresses available to assign to
                     the IA(s).
   NoBinding       3 Client record (binding) unavailable.
   NotOnLink       4 The prefix for the address is not appropriate for
                     the link to which the client is attached.
   UseMulticast    5 Sent by a server to a client to force the
                     client to send messages to the server.
                     using the All_DHCP_Relay_Agents_and_Servers
                     address.
*/					 
enum dhcpv6c_status {
	DHCPV6C_Success = 0,
	DHCPV6C_UnspecFail = 1,
	DHCPV6C_NoAddrsAvail = 2,
	DHCPV6C_NoBinding = 3,
	DHCPV6C_NotOnLink = 4,
	DHCPV6C_UseMulticast = 5,
	DHCPV6C_NoPrefixAvail = 6,
	_DHCPV6C_Status_Max
};

enum dhcpv6c_config {
	DHCPV6C_STRICT_OPTIONS = 1,
	DHCPV6C_CLIENT_FQDN = 2,
	DHCPV6C_ACCEPT_RECONFIGURE = 4,
	DHCPV6C_IGNORE_OPT_UNICAST = 8,
};


enum dhcpv6c_mode {
	DHCPV6C_UNKNOWN = -1,
	DHCPV6C_STATELESS,
	DHCPV6C_STATEFUL
};

enum ra_config {
	RA_RDNSS_DEFAULT_LIFETIME = 1,
};

enum dhcpv6c_ia_mode {
	IA_MODE_NONE,
	IA_MODE_TRY,
	IA_MODE_FORCE,
};



struct dhcpv6c_server_cand {
	bool has_noaddravail;
	bool wants_reconfigure;
	int16_t preference;
	uint8_t duid_len;
	uint8_t duid[130];
	struct in6_addr server_addr;
	uint32_t sol_max_rt;
	uint32_t inf_max_rt;
	void *ia_na;
	void *ia_pd;
	size_t ia_na_len;
	size_t ia_pd_len;
};


struct dhcpv6c_entry {
	struct in6_addr router;
	uint8_t auxlen;
	uint8_t length;
	struct in6_addr target;
	int16_t priority;
	uint32_t valid;
	uint32_t preferred;
	uint32_t t1;
	uint32_t t2;
	uint32_t iaid;
	uint8_t auxtarget[];
};

// Include padding after auxtarget to align the next entry
#define dhcpv6c_entry_size(entry) \
	(sizeof(struct dhcpv6c_entry) +	(((entry)->auxlen + 3) & ~3))

#define dhcpv6c_next_entry(entry) \
	((struct dhcpv6c_entry *)((uint8_t *)(entry) + dhcpv6c_entry_size(entry)))



struct dhcpv6c_interface;

typedef int(dhcpv6c_reply_handler)(struct dhcpv6c_interface *ifp, enum dhcpv6c_msg orig, const int rc,
		const void *opt, const void *end, const struct ipstack_sockaddr_in6 *from);

// retransmission strategy
struct dhcpv6c_retx {
	bool delay;
	uint8_t init_timeo;
	uint16_t max_timeo;
	uint8_t max_rc;
	char name[8];
	dhcpv6c_reply_handler *handler_reply;
	int(*handler_finish)(void);
};


#include "dhcpv6c_option.h"
#include "dhcpv6c_state.h"

struct dhcpv6c_interface 
{
	ifindex_t 			ifindex;
	zpl_socket_t		sock;
	zpl_socket_t		udp_sock;
	zpl_uint16			port;
	zpl_uint16			client_port;

	zpl_uint16			current_state;

	zpl_uint64 gt1, gt2, gt3;
	// IA states
	enum dhcpv6c_ia_mode na_mode;// = IA_MODE_NONE, pd_mode = IA_MODE_NONE;
	enum dhcpv6c_ia_mode pd_mode;
	enum dhcpv6c_mode 	ifpmode;//DHCPV6 模式，无状态，有状态
	
	bool 				accept_reconfig;
	// Reconfigure key
	zpl_uint8 			reconf_key[16];

	bool 				bound;
	zpl_uint32 			last_update;
	// RFC 3315 - 5.5 Timeout and Delay values
	struct dhcpv6c_retx dhcpv6c_retx[_DHCPV6C_MSG_MAX];

	void				*r_thread;	//read thread
	void				*t_thread;	//time thread,
	void				*d_thread;	//discover thread,

	zpl_uint8 			*state_data[256];
	zpl_uint32 			state_len[256];
	zpl_uint8			opt_mask[DHCPV6C_OPT_MAX];      /* Bitmask of options to send (-O option) */
	dhcpv6_option_set_t	options[DHCPV6C_OPT_MAX];
	clientv6_state_t	state_v6;
	clientv6_lease_t	lease_v6;
};



int init_dhcpv6c(struct dhcpv6c_interface *ifp, unsigned int client_options, int sol_timeout);
int dhcpv6c_set_ia_mode(struct dhcpv6c_interface *ifp, enum dhcpv6c_ia_mode na, enum dhcpv6c_ia_mode pd);
int dhcpv6c_request(struct dhcpv6c_interface *ifp, enum dhcpv6c_msg type);
int dhcpv6c_poll_reconfigure(struct dhcpv6c_interface *ifp);
int dhcpv6c_promote_server_cand(struct dhcpv6c_interface *ifp);
uint32_t ntohl_unaligned(const uint8_t *data);

#endif /*ZPL_DHCPV6C_MODULE*/

#ifdef __cplusplus
}
#endif

#endif /* __DHCPV6C_H__ */