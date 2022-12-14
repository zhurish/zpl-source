/**
 * Copyright (C) 2012-2014 Steven Barth <steven@midlink.org>
 * Copyright (C) 2017-2018 Hans Dedecker <dedeckeh@gmail.com>
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
/*
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <signal.h>
#include <limits.h>
#include <resolv.h>
#include <string.h>
#include <unistd.h>*/
//#include <zlog_debug.h>
//#include <stdbool.h>
//#include <ctype.h>
//#include <sys/time.h>
//#include <sys/ioctl.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>
//#include <netinet/in.h>

//#include <net/if.h>
//#include <net/ethernet.h>

#include "auto_include.h"
#include "zplos_include.h"
#ifdef ZPL_DHCPV6C_MODULE
#include "prefix.h"
#include "libgl.h"
#include "resolv.h"

#include "dhcpv6c.h"
#include "dhcpv6c_api.h"
#include "dhcpv6c_option.h"
#include "dhcpv6c_state.h"
#include "dhcpv6c_packet.h"
#ifdef USE_LIBUBOX
#include <libubox/md5.h>
#else
#include "md5.h"
#endif



static bool dhcpv6c_response_is_valid(struct dhcpv6c_interface *ifp, const void *buf, ssize_t len,
		const uint8_t transaction[3], enum dhcpv6c_msg type,
		const struct ipstack_in6_addr *daddr);

static unsigned int dhcpv6c_parse_ia(struct dhcpv6c_interface *ifp, void *opt, void *end);

static unsigned int dhcpv6c_calc_refresh_timers(struct dhcpv6c_interface *ifp);
static void dhcpv6c_handle_status_code(struct dhcpv6c_interface *ifp, _unused const enum dhcpv6c_msg orig,
		const uint16_t code, const void *status_msg, const int len,
		int *ret);
static void dhcpv6c_handle_ia_status_code(struct dhcpv6c_interface *ifp, const enum dhcpv6c_msg orig,
		const struct dhcpv6c_ia_hdr *ia_hdr, const uint16_t code,
		const void *status_msg, const int len,
		bool handled_status_codes[_DHCPV6C_Status_Max],
		int *ret);
static void dhcpv6c_add_server_cand(struct dhcpv6c_interface *ifp, const struct dhcpv6c_server_cand *cand);
static void dhcpv6c_clear_server_cand(struct dhcpv6c_interface *ifp);

static dhcpv6c_reply_handler dhcpv6c_handle_reply;
static dhcpv6c_reply_handler dhcpv6c_handle_advert;
static dhcpv6c_reply_handler dhcpv6c_handle_rebind_reply;
static dhcpv6c_reply_handler dhcpv6c_handle_reconfigure;

static int dhcpv6c_commit_advert(struct dhcpv6c_interface *ifp);

// RFC 3315 - 5.5 Timeout and Delay values
static struct dhcpv6c_retx dhcpv6c_retx_default[_DHCPV6C_MSG_MAX] = {
	[DHCPV6C_MSG_UNKNOWN] = {false, 1, 120, 0, "<POLL>",
			dhcpv6c_handle_reconfigure, NULL},
	[DHCPV6C_MSG_SOLICIT] = {true, 1, DHCPV6C_SOL_MAX_RT, 0, "SOLICIT",
			dhcpv6c_handle_advert, dhcpv6c_commit_advert},
	[DHCPV6C_MSG_REQUEST] = {true, 1, DHCPV6C_REQ_MAX_RT, 10, "REQUEST",
			dhcpv6c_handle_reply, NULL},
	[DHCPV6C_MSG_RENEW] = {false, 10, DHCPV6C_REN_MAX_RT, 0, "RENEW",
			dhcpv6c_handle_reply, NULL},
	[DHCPV6C_MSG_REBIND] = {false, 10, DHCPV6C_REB_MAX_RT, 0, "REBIND",
			dhcpv6c_handle_rebind_reply, NULL},
	[DHCPV6C_MSG_RELEASE] = {false, 1, 0, 5, "RELEASE", NULL, NULL},
	[DHCPV6C_MSG_DECLINE] = {false, 1, 0, 5, "DECLINE", NULL, NULL},
	[DHCPV6C_MSG_INFO_REQ] = {true, 1, DHCPV6C_INF_MAX_RT, 0, "INFOREQ",
			dhcpv6c_handle_reply, NULL},
};

// Server unicast address
static struct ipstack_in6_addr server_addr = IN6ADDR_ANY_INIT;

uint32_t ntohl_unaligned(const uint8_t *data)
{
	uint32_t buf;

	memcpy(&buf, data, sizeof(buf));
	return ntohl(buf);
}

static char *dhcpv6c_msg_to_str(enum dhcpv6c_msg msg)
{
	switch (msg) {
	case DHCPV6C_MSG_SOLICIT:
		return "SOLICIT";

	case DHCPV6C_MSG_ADVERT:
		return "ADVERTISE";

	case DHCPV6C_MSG_REQUEST:
		return "REQUEST";

	case DHCPV6C_MSG_RENEW:
		return "RENEW";

	case DHCPV6C_MSG_REBIND:
		return "REBIND";

	case DHCPV6C_MSG_REPLY:
		return "REPLY";

	case DHCPV6C_MSG_RELEASE:
		return "RELEASE";

	case DHCPV6C_MSG_DECLINE:
		return "DECLINE";

	case DHCPV6C_MSG_RECONF:
		return "RECONFIGURE";

	case DHCPV6C_MSG_INFO_REQ:
		return "INFORMATION REQUEST";

	default:
		break;
	}

	return "UNKNOWN";
}

static char *dhcpv6c_status_code_to_str(uint16_t code)
{
	switch (code) {
	case DHCPV6C_Success:
		return "Success";

	case DHCPV6C_UnspecFail:
		return "Unspecified Failure";

	case DHCPV6C_NoAddrsAvail:
		return "No Address Available";

	case DHCPV6C_NoBinding:
		return "No Binding";

	case DHCPV6C_NotOnLink:
		return "Not On Link";

	case DHCPV6C_UseMulticast:
		return "Use Multicast";

	case DHCPV6C_NoPrefixAvail:
		return "No Prefix Available";

	default:
		break;
	}

	return "Unknown";
}


int dhcpv6c_set_ia_mode(struct dhcpv6c_interface *ifp, enum dhcpv6c_ia_mode na, enum dhcpv6c_ia_mode pd)
{
	int mode = DHCPV6C_UNKNOWN;

	ifp->na_mode = na;
	ifp->pd_mode = pd;

	if (ifp->na_mode == IA_MODE_NONE && ifp->pd_mode == IA_MODE_NONE)
		mode = DHCPV6C_STATELESS;
	else if (ifp->na_mode == IA_MODE_FORCE || ifp->pd_mode == IA_MODE_FORCE)
		mode = DHCPV6C_STATEFUL;

	return mode;
}

static int64_t dhcpv6c_rand_delay(int64_t time)
{
	int random;

	random = rand();
	return (time * ((int64_t)random % 1000LL)) / 10000LL;
}



static unsigned int dhcpv6c_parse_ia(struct dhcpv6c_interface *ifp, void *opt, void *end)
{
	struct dhcpv6c_ia_hdr *ia_hdr = (struct dhcpv6c_ia_hdr *)opt;
	unsigned int updated_IAs = 0;
	uint32_t t1, t2;
	uint16_t otype, olen;
	uint8_t *odata, *_o = NULL;
	char buf[INET6_ADDRSTRLEN];

	t1 = ntohl(ia_hdr->t1);
	t2 = ntohl(ia_hdr->t2);

	if (t1 > t2)
		return 0;

	zlog_debug(MODULE_DHCP, "%s %04x T1 %d T2 %d", ntohs(ia_hdr->type) == DHCPV6C_OPT_IA_PD ? "IA_PD" : "IA_NA", ntohl(ia_hdr->iaid), t1, t2);

	// Update address IA
	dhcpv6c_for_each_option(&ia_hdr[1], end, otype, olen, odata) {
		struct dhcpv6c_entry entry = {IN6ADDR_ANY_INIT, 0, 0,
				IN6ADDR_ANY_INIT, 0, 0, 0, 0, 0, 0};

		entry.iaid = ia_hdr->iaid;

		if (otype == DHCPV6C_OPT_IA_PREFIX) {
			struct dhcpv6c_ia_prefix *prefix = (void*)&odata[-4];
			if (olen + 4U < sizeof(*prefix))
				continue;

			entry.valid = ntohl(prefix->valid);
			entry.preferred = ntohl(prefix->preferred);

			if (entry.preferred > entry.valid)
				continue;

			entry.t1 = (t1 ? t1 : (entry.preferred != UINT32_MAX ? 0.5 * entry.preferred : UINT32_MAX));
			entry.t2 = (t2 ? t2 : (entry.preferred != UINT32_MAX ? 0.8 * entry.preferred : UINT32_MAX));
			if (entry.t1 > entry.t2)
				entry.t1 = entry.t2;

			entry.length = prefix->prefix;
			entry.target = prefix->addr;
			uint16_t stype, slen;
			uint8_t *sdata;

			// Parse PD-exclude
			bool ok = true;
			dhcpv6c_for_each_option(odata + sizeof(*prefix) - 4U,
					odata + olen, stype, slen, sdata) {
				if (stype != DHCPV6C_OPT_PD_EXCLUDE || slen < 2)
					continue;

				uint8_t elen = sdata[0];
				if (elen > 64)
					elen = 64;

				if (entry.length < 32 || elen <= entry.length) {
					ok = false;
					continue;
				}

				uint8_t bytes = ((elen - entry.length - 1) / 8) + 1;
				if (slen <= bytes) {
					ok = false;
					continue;
				}

				uint32_t exclude = 0;
				do {
					exclude = exclude << 8 | sdata[bytes];
				} while (--bytes);

				exclude >>= 8 - ((elen - entry.length) % 8);
				exclude <<= 64 - elen;

				// Abusing router & priority fields for exclusion
				entry.router = entry.target;
				entry.router.s6_addr32[1] |= htonl(exclude);
				entry.priority = elen;
			}

			if (ok) {
				if (dhcpv6c_update_entry(ifp, DHCPV6C_STATE_IA_PD, &entry, 0, 0))
					updated_IAs++;

				zlog_debug(MODULE_DHCP, "%s/%d preferred %d valid %d",
				       inet_ntop(AF_INET6, &entry.target, buf, sizeof(buf)),
				       entry.length, entry.preferred , entry.valid);
			}

			entry.priority = 0;
			memset(&entry.router, 0, sizeof(entry.router));
		} else if (otype == DHCPV6C_OPT_IA_ADDR) {
			struct dhcpv6c_ia_addr *addr = (void*)&odata[-4];
			if (olen + 4U < sizeof(*addr))
				continue;

			entry.preferred = ntohl(addr->preferred);
			entry.valid = ntohl(addr->valid);

			if (entry.preferred > entry.valid)
				continue;

			entry.t1 = (t1 ? t1 : (entry.preferred != UINT32_MAX ? 0.5 * entry.preferred : UINT32_MAX));
			entry.t2 = (t2 ? t2 : (entry.preferred != UINT32_MAX ? 0.8 * entry.preferred : UINT32_MAX));
			if (entry.t1 > entry.t2)
				entry.t1 = entry.t2;

			entry.length = 128;
			entry.target = addr->addr;

			if (dhcpv6c_update_entry(ifp, DHCPV6C_STATE_IA_NA, &entry, 0, 0))
				updated_IAs++;

			zlog_debug(MODULE_DHCP, "%s preferred %d valid %d",
			       inet_ntop(AF_INET6, &entry.target, buf, sizeof(buf)),
			       entry.preferred , entry.valid);
		}
	}

	return updated_IAs;
}

static unsigned int dhcpv6c_calc_refresh_timers(struct dhcpv6c_interface *ifp)
{
	struct dhcpv6c_entry *e;
	size_t ia_na_entries, ia_pd_entries, i;
	size_t invalid_entries = 0;
	int64_t l_t1 = UINT32_MAX, l_t2 = UINT32_MAX, l_t3 = 0;

	e = dhcpv6c_state_get(ifp, DHCPV6C_STATE_IA_NA, &ia_na_entries);
	ia_na_entries /= sizeof(*e);

	for (i = 0; i < ia_na_entries; i++) {
		/* Exclude invalid IA_NA entries */
		if (!e[i].valid) {
			invalid_entries++;
			continue;
		}

		if (e[i].t1 < l_t1)
			l_t1 = e[i].t1;

		if (e[i].t2 < l_t2)
			l_t2 = e[i].t2;

		if (e[i].valid > l_t3)
			l_t3 = e[i].valid;
	}

	e = dhcpv6c_state_get(ifp, DHCPV6C_STATE_IA_PD, &ia_pd_entries);
	ia_pd_entries /= sizeof(*e);

	for (i = 0; i < ia_pd_entries; i++) {
		/* Exclude invalid IA_PD entries */
		if (!e[i].valid) {
			invalid_entries++;
			continue;
		}

		if (e[i].t1 < l_t1)
			l_t1 = e[i].t1;

		if (e[i].t2 < l_t2)
			l_t2 = e[i].t2;

		if (e[i].valid > l_t3)
			l_t3 = e[i].valid;
	}

	if (ia_pd_entries + ia_na_entries - invalid_entries) {
		ifp->gt1 = l_t1;
		ifp->gt2 = l_t2;
		ifp->gt3 = l_t3;

		zlog_debug(MODULE_DHCP, "T1 %"PRId64"s, T2 %"PRId64"s, T3 %"PRId64"s", ifp->gt1, ifp->gt2, ifp->gt3);
	}

	return (unsigned int)(ia_pd_entries + ia_na_entries);
}

static void dhcpv6c_log_status_code(const uint16_t code, const char *scope,
		const void *status_msg, int len)
{
	const char *src = status_msg;
	char buf[len + 3];
	char *dst = buf;

	if (len) {
		*dst++ = '(';
		while (len--) {
			*dst = isprint((unsigned char)*src) ? *src : '?';
			src++;
			dst++;
		}
		*dst++ = ')';
	}

	*dst = 0;

	zlog_debug(MODULE_DHCP, "Server returned %s status '%s %s'",
		scope, dhcpv6c_status_code_to_str(code), buf);
}

static void dhcpv6c_handle_status_code(struct dhcpv6c_interface *ifp, const enum dhcpv6c_msg orig,
		const uint16_t code, const void *status_msg, const int len,
		int *ret)
{
	dhcpv6c_log_status_code(code, "message", status_msg, len);

	switch (code) {
	case DHCPV6C_UnspecFail:
		// Generic failure
		*ret = 0;
		break;

	case DHCPV6C_UseMulticast:
		switch(orig) {
		case DHCPV6C_MSG_REQUEST:
		case DHCPV6C_MSG_RENEW:
		case DHCPV6C_MSG_RELEASE:
		case DHCPV6C_MSG_DECLINE:
			// Message needs to be retransmitted according to RFC3315 chapter 18.1.8
			server_addr = in6addr_any;
			*ret = 0;
			break;
		default:
			break;
		}
		break;

	case DHCPV6C_NoAddrsAvail:
	case DHCPV6C_NoPrefixAvail:
		if (orig == DHCPV6C_MSG_REQUEST)
			*ret = 0; // Failure
		break;

	default:
		break;
	}
}

static void dhcpv6c_handle_ia_status_code(struct dhcpv6c_interface *ifp, const enum dhcpv6c_msg orig,
		const struct dhcpv6c_ia_hdr *ia_hdr, const uint16_t code,
		const void *status_msg, const int len,
		bool handled_status_codes[_DHCPV6C_Status_Max], int *ret)
{
	dhcpv6c_log_status_code(code, ia_hdr->type == DHCPV6C_OPT_IA_NA ?
		"IA_NA" : "IA_PD", status_msg, len);

	switch (code) {
	case DHCPV6C_NoBinding:
		switch (orig) {
		case DHCPV6C_MSG_RENEW:
		case DHCPV6C_MSG_REBIND:
			if ((*ret > 0) && !handled_status_codes[code])
				*ret = dhcpv6c_request(ifp, DHCPV6C_MSG_REQUEST);
			break;

		default:
			break;
		}
		break;

	default:
		*ret = 0;
		break;
	}
}


int init_dhcpv6c(struct dhcpv6c_interface *ifp, unsigned int options, int sol_timeout)
{
	ifp->client_options = options;
	dhcpv6c_retx_default[DHCPV6C_MSG_SOLICIT].max_timeo = sol_timeout;
	memcpy(ifp->dhcpv6c_retx, dhcpv6c_retx_default, sizeof(dhcpv6c_retx_default));
	ifp->sock = ipstack_socket(IPSTACK_IPCOM, IPSTACK_AF_INET6, IPSTACK_SOCK_DGRAM | SOCK_CLOEXEC, IPSTACK_IPPROTO_UDP);
	if (ipstack_invalid(ifp->sock))
		goto failure;

	// Detect interface

	// Create client DUID
	size_t client_id_len;
	dhcpv6c_state_get(ifp, DHCPV6C_STATE_CLIENT_ID, &client_id_len);
	if (client_id_len == 0) {
		uint8_t duid[14] = {0, DHCPV6C_OPT_CLIENTID, 0, 10, 0,
				DHCPV6C_DUID_LLADDR, 0, 1};
		/*
		if (ioctl(sock, SIOCGIFHWADDR, &ifr) >= 0)
			memcpy(&duid[8], ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);

		uint8_t zero[ETHER_ADDR_LEN] = {0, 0, 0, 0, 0, 0};
		struct ifreq ifs[100], *ifp, *ifend;
		struct ifconf ifc;
		ifc.ifc_req = ifs;
		ifc.ifc_len = sizeof(ifs);

		if (!memcmp(&duid[8], zero, ETHER_ADDR_LEN) &&
				ioctl(sock, SIOCGIFCONF, &ifc) >= 0) {
			// If our interface doesn't have an address...
			ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));
			for (ifp = ifc.ifc_req; ifp < ifend &&
					!memcmp(&duid[8], zero, ETHER_ADDR_LEN); ifp++) {
				memcpy(ifr.ifr_name, ifp->ifr_name,
						sizeof(ifr.ifr_name));
				if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
					continue;

				memcpy(&duid[8], ifr.ifr_hwaddr.sa_data,
						ETHER_ADDR_LEN);
			}
		}
		*/
		dhcpv6c_state_add(ifp, DHCPV6C_STATE_CLIENT_ID, duid, sizeof(duid));
	}

	// Create ORO
	if (!(ifp->client_options & DHCPV6C_STRICT_OPTIONS)) {
		uint16_t oro[] = {
			htons(DHCPV6C_OPT_SIP_SERVER_D),
			htons(DHCPV6C_OPT_SIP_SERVER_A),
			htons(DHCPV6C_OPT_DNS_SERVERS),
			htons(DHCPV6C_OPT_DNS_DOMAIN),
			htons(DHCPV6C_OPT_SNTP_SERVERS),
			htons(DHCPV6C_OPT_NTP_SERVER),
			htons(DHCPV6C_OPT_AFTR_NAME),
			htons(DHCPV6C_OPT_PD_EXCLUDE),
#ifdef EXT_CER_ID
			htons(DHCPV6C_OPT_CER_ID),
#endif
			htons(DHCPV6C_OPT_S46_CONT_MAPE),
			htons(DHCPV6C_OPT_S46_CONT_MAPT),
			htons(DHCPV6C_OPT_S46_CONT_LW),
		};
		dhcpv6c_state_add(ifp, DHCPV6C_STATE_ORO, oro, sizeof(oro));
	}
	// Required oro
	uint16_t req_oro[] = {
		htons(DHCPV6C_OPT_INF_MAX_RT),
		htons(DHCPV6C_OPT_SOL_MAX_RT),
		htons(DHCPV6C_OPT_INFO_REFRESH),
	};
	dhcpv6c_state_add(ifp, DHCPV6C_STATE_ORO, req_oro, sizeof(req_oro));

	// Configure IPv6-options
	int val = 1;
	if (ipstack_setsockopt(ifp->sock, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(val)) < 0)
		goto failure;

	if (ipstack_setsockopt(ifp->sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
		goto failure;

	if (ipstack_setsockopt(ifp->sock, IPPROTO_IPV6, IPV6_RECVPKTINFO, &val, sizeof(val)) < 0)
		goto failure;

	//if (ipstack_setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, ifname, strlen(ifname)) < 0)
	//	goto failure;

	struct ipstack_sockaddr_in6 client_addr = { .sin6_family = AF_INET6,
		.sin6_port = htons(DHCPV6C_CLIENT_PORT), .sin6_flowinfo = 0 };

	if (ipstack_bind(ifp->sock, (struct ipstack_sockaddr*)&client_addr, sizeof(client_addr)) < 0)
		goto failure;

	return 0;

failure:
	if (!ipstack_invalid(ifp->sock))
		ipstack_close(ifp->sock);

	return -1;
}

enum {
	IOV_HDR=0,
	IOV_ORO,
	IOV_CL_ID,
	IOV_SRV_ID,
	IOV_OPTS,
	IOV_RECONF_ACCEPT,
	IOV_FQDN,
	IOV_HDR_IA_NA,
	IOV_IA_NA,
	IOV_IA_PD,
	IOV_TOTAL
};


static void dhcpv6c_send(struct dhcpv6c_interface *ifp, enum dhcpv6c_msg type, uint8_t trid[3], uint32_t ecs)
{
	// Build FQDN
	size_t i = 0;
	char fqdn_buf[256];
	gethostname(fqdn_buf, sizeof(fqdn_buf));
	struct {
		uint16_t type;
		uint16_t len;
		uint8_t flags;
		uint8_t data[256];
	} fqdn;
	size_t fqdn_len = 5 + dn_comp(fqdn_buf, fqdn.data,
			sizeof(fqdn.data), NULL, NULL);
	fqdn.type = htons(DHCPV6C_OPT_FQDN);
	fqdn.len = htons(fqdn_len - 4);
	fqdn.flags = 0;

	// Build Client ID
	size_t cl_id_len;
	void *cl_id = dhcpv6c_state_get(ifp, DHCPV6C_STATE_CLIENT_ID, &cl_id_len);

	// Get Server ID
	size_t srv_id_len;
	void *srv_id = dhcpv6c_state_get(ifp, DHCPV6C_STATE_SERVER_ID, &srv_id_len);

	// Build IA_PDs
	size_t ia_pd_entries = 0, ia_pd_len = 0;
	uint8_t *ia_pd;

	if (type == DHCPV6C_MSG_SOLICIT) {
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_IA_PD);
		size_t n_prefixes;
		struct dhcpv6c_request_prefix *request_prefixes = dhcpv6c_state_get(ifp, DHCPV6C_STATE_IA_PD_INIT, &n_prefixes);
		n_prefixes /= sizeof(struct dhcpv6c_request_prefix);

		ia_pd = alloca(n_prefixes * (sizeof(struct dhcpv6c_ia_hdr) + sizeof(struct dhcpv6c_ia_prefix)));

		for (i = 0; i < n_prefixes; i++) {
			struct dhcpv6c_ia_hdr hdr_ia_pd = {
				htons(DHCPV6C_OPT_IA_PD),
				htons(sizeof(hdr_ia_pd) - 4 +
				      sizeof(struct dhcpv6c_ia_prefix) * !!request_prefixes[i].length),
				request_prefixes[i].iaid, 0, 0
			};
			struct dhcpv6c_ia_prefix pref = {
				.type = htons(DHCPV6C_OPT_IA_PREFIX),
				.len = htons(sizeof(pref) - 4),
				.prefix = request_prefixes[i].length
			};
			memcpy(ia_pd + ia_pd_len, &hdr_ia_pd, sizeof(hdr_ia_pd));
			ia_pd_len += sizeof(hdr_ia_pd);
			if (request_prefixes[i].length) {
				memcpy(ia_pd + ia_pd_len, &pref, sizeof(pref));
				ia_pd_len += sizeof(pref);
			}
		}
	} else {
		struct dhcpv6c_entry *e = dhcpv6c_state_get(ifp, DHCPV6C_STATE_IA_PD, &ia_pd_entries);
		ia_pd_entries /= sizeof(*e);

		// we're too lazy to count our distinct IAIDs,
		// so just allocate maximally needed space
		ia_pd = alloca(ia_pd_entries * (sizeof(struct dhcpv6c_ia_prefix) + 10 +
					sizeof(struct dhcpv6c_ia_hdr)));

		for ( i = 0; i < ia_pd_entries; ++i) {
			uint32_t iaid = e[i].iaid;

			// check if this is an unprocessed IAID and skip if not.
			int new_iaid = 1;
			for (int j = i-1; j >= 0; j--) {
				if (e[j].iaid == iaid) {
					new_iaid = 0;
					break;
				}
			}

			if (!new_iaid)
				continue;

			// construct header
			struct dhcpv6c_ia_hdr hdr_ia_pd = {
				htons(DHCPV6C_OPT_IA_PD),
				htons(sizeof(hdr_ia_pd) - 4),
				iaid, 0, 0
			};

			memcpy(ia_pd + ia_pd_len, &hdr_ia_pd, sizeof(hdr_ia_pd));
			struct dhcpv6c_ia_hdr *hdr = (struct dhcpv6c_ia_hdr *) (ia_pd + ia_pd_len);
			ia_pd_len += sizeof(hdr_ia_pd);

			for (size_t j = i; j < ia_pd_entries; j++) {
				if (e[j].iaid != iaid)
					continue;

				uint8_t ex_len = 0;
				if (e[j].priority > 0)
					ex_len = ((e[j].priority - e[j].length - 1) / 8) + 6;

				struct dhcpv6c_ia_prefix p = {
					.type = htons(DHCPV6C_OPT_IA_PREFIX),
					.len = htons(sizeof(p) - 4U + ex_len),
					.prefix = e[j].length,
					.addr = e[j].target
				};

				if (type == DHCPV6C_MSG_REQUEST) {
					p.preferred = htonl(e[j].preferred);
					p.valid = htonl(e[j].valid);
				}

				memcpy(ia_pd + ia_pd_len, &p, sizeof(p));
				ia_pd_len += sizeof(p);

				if (ex_len) {
					ia_pd[ia_pd_len++] = 0;
					ia_pd[ia_pd_len++] = DHCPV6C_OPT_PD_EXCLUDE;
					ia_pd[ia_pd_len++] = 0;
					ia_pd[ia_pd_len++] = ex_len - 4;
					ia_pd[ia_pd_len++] = e[j].priority;

					uint32_t excl = ntohl(e[j].router.s6_addr32[1]);
					excl >>= (64 - e[j].priority);
					excl <<= 8 - ((e[j].priority - e[j].length) % 8);

					for ( i = ex_len - 5; i > 0; --i, excl >>= 8)
						ia_pd[ia_pd_len + i] = excl & 0xff;
					ia_pd_len += ex_len - 5;
				}

				hdr->len = htons(ntohs(hdr->len) + ntohs(p.len) + 4U);
			}
		}
	}

	// Build IA_NAs
	size_t ia_na_entries, ia_na_len = 0;
	void *ia_na = NULL;
	struct dhcpv6c_entry *e = dhcpv6c_state_get(ifp, DHCPV6C_STATE_IA_NA, &ia_na_entries);
	ia_na_entries /= sizeof(*e);

	struct dhcpv6c_ia_hdr hdr_ia_na = {
		htons(DHCPV6C_OPT_IA_NA),
		htons(sizeof(hdr_ia_na) - 4),
		htonl(1), 0, 0
	};

	struct dhcpv6c_ia_addr pa[ia_na_entries];
	for ( i = 0; i < ia_na_entries; ++i) {
		pa[i].type = htons(DHCPV6C_OPT_IA_ADDR);
		pa[i].len = htons(sizeof(pa[i]) - 4U);
		pa[i].addr = e[i].target;

		if (type == DHCPV6C_MSG_REQUEST) {
			pa[i].preferred = htonl(e[i].preferred);
			pa[i].valid = htonl(e[i].valid);
		} else {
			pa[i].preferred = 0;
			pa[i].valid = 0;
		}
	}

	ia_na = pa;
	ia_na_len = sizeof(pa);
	hdr_ia_na.len = htons(ntohs(hdr_ia_na.len) + ia_na_len);

	// Reconfigure Accept
	struct {
		uint16_t type;
		uint16_t length;
	} reconf_accept = {htons(DHCPV6C_OPT_RECONF_ACCEPT), 0};

	// Option list
	size_t opts_len;
	void *opts = dhcpv6c_state_get(ifp, DHCPV6C_STATE_OPTS, &opts_len);

	// Option Request List
	size_t oro_entries, oro_len = 0;
	uint16_t *oro, *s_oro = dhcpv6c_state_get(ifp, DHCPV6C_STATE_ORO, &oro_entries);

	oro_entries /= sizeof(*s_oro);
	oro = alloca(oro_entries * sizeof(*oro));

	for ( i = 0; i < oro_entries; i++) {
		struct dhcpv6c_option *opt = dhcpv6c_option_find(ifp, htons(s_oro[i]));

		if (opt) {
			if (!(opt->flags & OPT_ORO))
				continue;

			if ((opt->flags & OPT_ORO_SOLICIT) && type != DHCPV6C_MSG_SOLICIT)
				continue;

			if ((opt->flags & OPT_ORO_STATELESS) && type != DHCPV6C_MSG_INFO_REQ)
				continue;

			if ((opt->flags & OPT_ORO_STATEFUL) && type == DHCPV6C_MSG_INFO_REQ)
				continue;
		}

		oro[oro_len++] = s_oro[i];
	}
	oro_len *= sizeof(*oro);

	// Prepare Header
	struct {
		uint8_t type;
		uint8_t trid[3];
		uint16_t elapsed_type;
		uint16_t elapsed_len;
		uint16_t elapsed_value;
		uint16_t oro_type;
		uint16_t oro_len;
	} hdr = {
		type, {trid[0], trid[1], trid[2]},
		htons(DHCPV6C_OPT_ELAPSED), htons(2),
			htons((ecs > 0xffff) ? 0xffff : ecs),
		htons(DHCPV6C_OPT_ORO), htons(oro_len),
	};

	struct iovec iov[IOV_TOTAL] = {
		[IOV_HDR] = {&hdr, sizeof(hdr)},
		[IOV_ORO] = {oro, oro_len},
		[IOV_CL_ID] = {cl_id, cl_id_len},
		[IOV_SRV_ID] = {srv_id, srv_id_len},
		[IOV_OPTS] = { opts, opts_len },
		[IOV_RECONF_ACCEPT] = {&reconf_accept, sizeof(reconf_accept)},
		[IOV_FQDN] = {&fqdn, fqdn_len},
		[IOV_HDR_IA_NA] = {&hdr_ia_na, sizeof(hdr_ia_na)},
		[IOV_IA_NA] = {ia_na, ia_na_len},
		[IOV_IA_PD] = {ia_pd, ia_pd_len},
	};

	size_t cnt = IOV_TOTAL;
	if (type == DHCPV6C_MSG_INFO_REQ)
		cnt = IOV_HDR_IA_NA;

	// Disable IAs if not used
	if (type != DHCPV6C_MSG_SOLICIT && ia_na_len == 0)
		iov[IOV_HDR_IA_NA].iov_len = 0;

	if (ifp->na_mode == IA_MODE_NONE)
		iov[IOV_HDR_IA_NA].iov_len = 0;

	if ((type != DHCPV6C_MSG_SOLICIT && type != DHCPV6C_MSG_REQUEST) ||
			!(ifp->client_options & DHCPV6C_ACCEPT_RECONFIGURE))
		iov[IOV_RECONF_ACCEPT].iov_len = 0;

	if (!(ifp->client_options & DHCPV6C_CLIENT_FQDN))
		iov[IOV_FQDN].iov_len = 0;

	struct ipstack_sockaddr_in6 srv = {AF_INET6, htons(DHCPV6C_SERVER_PORT),
		0, ALL_DHCPV6C_RELAYS, ifp->ifindex};
	struct ipstack_msghdr msg = {.msg_name = &srv, .msg_namelen = sizeof(srv),
			.msg_iov = iov, .msg_iovlen = cnt};

	switch (type) {
	case DHCPV6C_MSG_REQUEST:
	case DHCPV6C_MSG_RENEW:
	case DHCPV6C_MSG_RELEASE:
	case DHCPV6C_MSG_DECLINE:
		if (!IN6_IS_ADDR_UNSPECIFIED(&server_addr) &&
			dhcpv6c_addr_in_scope(ifp, &server_addr)) {
			srv.sin6_addr = server_addr;
			if (!IN6_IS_ADDR_LINKLOCAL(&server_addr))
				srv.sin6_scope_id = 0;
		}
		break;
	default:
		break;
	}

	if (ipstack_sendmsg(ifp->sock, &msg, 0) < 0) {
		char in6_str[INET6_ADDRSTRLEN];

		zlog_err(MODULE_DHCP, "Failed to send %s message to %s (%s)",
			dhcpv6c_msg_to_str(type),
			inet_ntop(AF_INET6, (const void *)&srv.sin6_addr,
				in6_str, sizeof(in6_str)), strerror(errno));
	}
}



int dhcpv6c_request(struct dhcpv6c_interface *ifp, enum dhcpv6c_msg type)
{
	uint8_t rc = 0;
	uint64_t timeout = UINT32_MAX;
	struct dhcpv6c_retx *retx = &ifp->dhcpv6c_retx[type];

	if (retx->delay) {
		struct timespec ts = {0, 0};
		ts.tv_nsec = (dhcpv6c_rand_delay((10000 * DHCPV6C_REQ_DELAY) / 2) + (1000 * DHCPV6C_REQ_DELAY) / 2) * 1000000;

		while (nanosleep(&ts, &ts) < 0 && errno == EINTR);
	}

	if (type == DHCPV6C_MSG_UNKNOWN)
		timeout = ifp->gt1;
	else if (type == DHCPV6C_MSG_RENEW)
		timeout = (ifp->gt2 > ifp->gt1) ? ifp->gt2 - ifp->gt1 : ((ifp->gt1 == UINT32_MAX) ? UINT32_MAX : 0);
	else if (type == DHCPV6C_MSG_REBIND)
		timeout = (ifp->gt3 > ifp->gt2) ? ifp->gt3 - ifp->gt2 : ((ifp->gt2 == UINT32_MAX) ? UINT32_MAX : 0);

	if (timeout == 0)
		return -1;

	zlog_debug(MODULE_DHCP, "Starting %s transaction (timeout %"PRIu64"s, max rc %d)",
			retx->name, timeout, retx->max_rc);

	uint64_t start = dhcpv6c_get_milli_time(), round_start = start, elapsed;

	// Generate transaction ID
	uint8_t trid[3] = {0, 0, 0};
	if (type != DHCPV6C_MSG_UNKNOWN)
		;//dhcpv6c_random(trid, sizeof(trid));

	ssize_t len = -1;
	int64_t rto = 0;

	do {
		if (rto == 0) {
			int64_t delay = dhcpv6c_rand_delay(retx->init_timeo * 1000);

			// First RT MUST be strictly greater than IRT for solicit messages (RFC3313 17.1.2)
			while (type == DHCPV6C_MSG_SOLICIT && delay <= 0)
				delay = dhcpv6c_rand_delay(retx->init_timeo * 1000);

			rto = (retx->init_timeo * 1000 + delay);
		} else
			rto = (2 * rto + dhcpv6c_rand_delay(rto));

		if (retx->max_timeo && (rto >= retx->max_timeo * 1000))
			rto = retx->max_timeo * 1000 +
				dhcpv6c_rand_delay(retx->max_timeo * 1000);

		// Calculate end for this round and elapsed time
		uint64_t round_end = round_start + rto;
		elapsed = round_start - start;

		// Don't wait too long if timeout differs from infinite
		if ((timeout != UINT32_MAX) && (round_end - start > timeout * 1000))
			round_end = timeout * 1000 + start;

		// Built and send package
		switch (type) {
		case DHCPV6C_MSG_UNKNOWN:
			break;
		default:
			zlog_debug(MODULE_DHCP, "Send %s message (elapsed %"PRIu64"ms, rc %d)",
					retx->name, elapsed, rc);
			// Fall through
		case DHCPV6C_MSG_SOLICIT:
		case DHCPV6C_MSG_INFO_REQ:
			dhcpv6c_send(ifp, type, trid, elapsed / 10);
			rc++;
		}

		// Receive rounds
		for (; len < 0 && (round_start < round_end);
				round_start = dhcpv6c_get_milli_time()) {
			uint8_t buf[1536];
			union {
				struct ipstack_cmsghdr hdr;
				uint8_t buf[CMSG_SPACE(sizeof(struct ipstack_in6_pktinfo))];
			} cmsg_buf;
			struct ipstack_iovec iov = {buf, sizeof(buf)};
			struct ipstack_sockaddr_in6 addr;
			struct ipstack_msghdr msg = {.msg_name = &addr, .msg_namelen = sizeof(addr),
					.msg_iov = &iov, .msg_iovlen = 1, .msg_control = cmsg_buf.buf,
					.msg_controllen = sizeof(cmsg_buf)};
			struct ipstack_in6_pktinfo *pktinfo = NULL;
			const struct dhcpv6c_header *hdr = (const struct dhcpv6c_header *)buf;

			// Check for pending signal
			//if (dhcpv6c_signal_process())
			//	return -1;

			// Set timeout for receiving
			uint64_t t = round_end - round_start;
			struct timeval tv = {t / 1000, (t % 1000) * 1000};
			if (ipstack_setsockopt(ifp->sock, SOL_SOCKET, SO_RCVTIMEO,
					&tv, sizeof(tv)) < 0)
				zlog_err(MODULE_DHCP, "setsockopt SO_RCVTIMEO failed (%s)",
						strerror(errno));

			// Receive cycle
			len = ipstack_recvmsg(ifp->sock, &msg, 0);
			if (len < 0)
				continue;

			for (struct ipstack_cmsghdr *ch = CMSG_FIRSTHDR(&msg); ch != NULL;
				ch = CMSG_NXTHDR(&msg, ch)) {
				if (ch->cmsg_level == SOL_IPV6 &&
					ch->cmsg_type == IPV6_PKTINFO) {
					pktinfo = (struct ipstack_in6_pktinfo *)CMSG_DATA(ch);
					break;
				}
			}

			if (pktinfo == NULL) {
				len = -1;
				continue;
			}

			if (!dhcpv6c_response_is_valid(ifp, buf, len, trid,
							type, &pktinfo->ipi6_addr)) {
				len = -1;
				continue;
			}

			uint8_t *opt = &buf[4];
			uint8_t *opt_end = opt + len - 4;

			round_start = dhcpv6c_get_milli_time();
			elapsed = round_start - start;
			zlog_debug(MODULE_DHCP, "Got a valid %s after %"PRIu64"ms",
			       dhcpv6c_msg_to_str(hdr->msg_type), elapsed);

			if (retx->handler_reply)
				len = retx->handler_reply(ifp, type, rc, opt, opt_end, &addr);

			if (len > 0 && round_end - round_start > 1000)
				round_end = 1000 + round_start;
		}

		// Allow
		if (retx->handler_finish)
			len = retx->handler_finish();
	} while (len < 0 && ((timeout == UINT32_MAX) || (elapsed / 1000 < timeout)) &&
			(!retx->max_rc || rc < retx->max_rc));
	return len;
}

// Message validation checks according to RFC3315 chapter 15
static bool dhcpv6c_response_is_valid(struct dhcpv6c_interface *ifp, const void *buf, ssize_t len,
		const uint8_t transaction[3], enum dhcpv6c_msg type,
		const struct ipstack_in6_addr *daddr)
{
	uint8_t *_o = NULL;
	const struct dhcpv6c_header *rep = buf;
	if (len < (ssize_t)sizeof(*rep) || memcmp(rep->tr_id,
			transaction, sizeof(rep->tr_id)))
		return false; // Invalid reply

	if (type == DHCPV6C_MSG_SOLICIT) {
		if (rep->msg_type != DHCPV6C_MSG_ADVERT &&
				rep->msg_type != DHCPV6C_MSG_REPLY)
			return false;

	} else if (type == DHCPV6C_MSG_UNKNOWN) {
		if (!ifp->accept_reconfig || rep->msg_type != DHCPV6C_MSG_RECONF)
			return false;

	} else if (rep->msg_type != DHCPV6C_MSG_REPLY)
		return false;

	uint8_t *end = ((uint8_t*)buf) + len, *odata = NULL,
		rcmsg = DHCPV6C_MSG_UNKNOWN;
	uint16_t otype, olen = UINT16_MAX;
	bool clientid_ok = false, serverid_ok = false, rcauth_ok = false,
		ia_present = false, options_valid = true;

	size_t client_id_len, server_id_len;
	void *client_id = dhcpv6c_state_get(ifp, DHCPV6C_STATE_CLIENT_ID, &client_id_len);
	void *server_id = dhcpv6c_state_get(ifp, DHCPV6C_STATE_SERVER_ID, &server_id_len);

	dhcpv6c_for_each_option(&rep[1], end, otype, olen, odata) {
		if (otype == DHCPV6C_OPT_CLIENTID) {
			clientid_ok = (olen + 4U == client_id_len) && !memcmp(
					&odata[-4], client_id, client_id_len);
		} else if (otype == DHCPV6C_OPT_SERVERID) {
			if (server_id_len)
				serverid_ok = (olen + 4U == server_id_len) && !memcmp(
						&odata[-4], server_id, server_id_len);
			else
				serverid_ok = true;
		} else if (otype == DHCPV6C_OPT_AUTH && olen == -4 +
				sizeof(struct dhcpv6c_auth_reconfigure)) {
			struct dhcpv6c_auth_reconfigure *r = (void*)&odata[-4];
			if (r->protocol != 3 || r->algorithm != 1 || r->reconf_type != 2)
				continue;

			os_md5_ctxt md5;
			uint8_t serverhash[16], secretbytes[64];
			uint32_t hash[4];
			memcpy(serverhash, r->key, sizeof(serverhash));
			memset(r->key, 0, sizeof(r->key));

			memset(secretbytes, 0, sizeof(secretbytes));
			memcpy(secretbytes, ifp->reconf_key, sizeof(ifp->reconf_key));

			for (size_t i = 0; i < sizeof(secretbytes); ++i)
				secretbytes[i] ^= 0x36;

			os_md5_init(&md5);
			os_md5_loop(&md5,secretbytes, sizeof(secretbytes));
			os_md5_loop(&md5, buf, len);
			os_md5_pad(&md5);
			os_md5_result(hash, &md5);
			for (size_t i = 0; i < sizeof(secretbytes); ++i) {
				secretbytes[i] ^= 0x36;
				secretbytes[i] ^= 0x5c;
			}
			os_md5_init(&md5);
			os_md5_loop(&md5,secretbytes, sizeof(secretbytes));
			os_md5_loop(&md5, buf, len);
			os_md5_pad(&md5);
			os_md5_result(hash, &md5);
			rcauth_ok = !memcmp(hash, serverhash, sizeof(hash));
		} else if (otype == DHCPV6C_OPT_RECONF_MESSAGE && olen == 1) {
			rcmsg = odata[0];
		} else if ((otype == DHCPV6C_OPT_IA_PD || otype == DHCPV6C_OPT_IA_NA)) {
			ia_present = true;
			if (olen < -4 + sizeof(struct dhcpv6c_ia_hdr))
				options_valid = false;
		} else if ((otype == DHCPV6C_OPT_IA_ADDR) || (otype == DHCPV6C_OPT_IA_PREFIX) ||
				(otype == DHCPV6C_OPT_PD_EXCLUDE))
			// Options are not allowed on global level
			options_valid = false;
	}

	if (!options_valid || ((odata + olen) > end))
		return false;

	if (type == DHCPV6C_MSG_INFO_REQ && ia_present)
		return false;

	if (rep->msg_type == DHCPV6C_MSG_RECONF) {
		if ((rcmsg != DHCPV6C_MSG_RENEW && rcmsg != DHCPV6C_MSG_REBIND && rcmsg != DHCPV6C_MSG_INFO_REQ) ||
			(rcmsg == DHCPV6C_MSG_INFO_REQ && ia_present) ||
			!rcauth_ok || IN6_IS_ADDR_MULTICAST(daddr))
			return false;
	}

	return clientid_ok && serverid_ok;
}

int dhcpv6c_poll_reconfigure(struct dhcpv6c_interface *ifp)
{
	int ret = dhcpv6c_request(ifp, DHCPV6C_MSG_UNKNOWN);

	switch (ret) {
	/*
	 * Only RENEW/REBIND/INFORMATION REQUEST
	 * message transmission can be requested
	 * by a RECONFIGURE
	 */
	case DHCPV6C_MSG_RENEW:
	case DHCPV6C_MSG_REBIND:
	case DHCPV6C_MSG_INFO_REQ:
		ret = dhcpv6c_request(ifp, ret);
		break;

	default:
		break;
	}

	return ret;
}

static int dhcpv6c_handle_reconfigure(struct dhcpv6c_interface *ifp, enum dhcpv6c_msg orig, const int rc,
		const void *opt, const void *end,  const struct ipstack_sockaddr_in6 *from)
{
	uint16_t otype, olen;
	uint8_t *odata, *_o = NULL;
	enum dhcpv6c_msg msg = DHCPV6C_MSG_UNKNOWN;

	dhcpv6c_for_each_option(opt, end, otype, olen, odata) {
		if (otype == DHCPV6C_OPT_RECONF_MESSAGE && olen == 1) {
			switch (odata[0]) {
			case DHCPV6C_MSG_REBIND:
				if (ifp->gt2 != UINT32_MAX)
					ifp->gt2 = 0;
			// Fall through
			case DHCPV6C_MSG_RENEW:
				if (ifp->gt1 != UINT32_MAX)
					ifp->gt1 = 0;
			// Fall through
			case DHCPV6C_MSG_INFO_REQ:
				msg = odata[0];
				zlog_debug(MODULE_DHCP, "Need to respond with %s in reply to %s",
				       dhcpv6c_msg_to_str(msg), dhcpv6c_msg_to_str(DHCPV6C_MSG_RECONF));
				break;

			default:
				break;
			}
		}
	}

	if (msg != DHCPV6C_MSG_UNKNOWN)
		dhcpv6c_handle_reply(ifp, orig, rc, NULL, NULL, NULL);

	return (msg == DHCPV6C_MSG_UNKNOWN? -1: (int)msg);
}

// Collect all advertised servers
static int dhcpv6c_handle_advert(struct dhcpv6c_interface *ifp, enum dhcpv6c_msg orig, const int rc,
		const void *opt, const void *end, _unused const struct ipstack_sockaddr_in6 *from)
{
	uint16_t olen, otype;
	uint8_t *odata, pref = 0, *_o = NULL;
	struct dhcpv6c_server_cand cand = {false, false, 0, 0, {0},
					IN6ADDR_ANY_INIT, DHCPV6C_SOL_MAX_RT,
					DHCPV6C_INF_MAX_RT, NULL, NULL, 0, 0};
	bool have_na = false;
	int have_pd = 0;

	dhcpv6c_for_each_option(opt, end, otype, olen, odata) {
		if (orig == DHCPV6C_MSG_SOLICIT &&
				((otype == DHCPV6C_OPT_IA_PD && ifp->pd_mode != IA_MODE_NONE) ||
				 (otype == DHCPV6C_OPT_IA_NA && ifp->na_mode != IA_MODE_NONE)) &&
				olen > -4 + sizeof(struct dhcpv6c_ia_hdr)) {
			struct dhcpv6c_ia_hdr *ia_hdr = (void*)(&odata[-4]);
			dhcpv6c_parse_ia(ifp, ia_hdr, odata + olen + sizeof(*ia_hdr));
		}

		if (otype == DHCPV6C_OPT_SERVERID && olen <= 130) {
			memcpy(cand.duid, odata, olen);
			cand.duid_len = olen;
		} else if (otype == DHCPV6C_OPT_PREF && olen >= 1 &&
				cand.preference >= 0) {
			cand.preference = pref = odata[0];
		} else if (otype == DHCPV6C_OPT_UNICAST && olen == sizeof(cand.server_addr)) {
			if (!(ifp->client_options & DHCPV6C_IGNORE_OPT_UNICAST))
				cand.server_addr = *(struct ipstack_in6_addr *)odata;

		} else if (otype == DHCPV6C_OPT_RECONF_ACCEPT) {
			cand.wants_reconfigure = true;
		} else if (otype == DHCPV6C_OPT_SOL_MAX_RT && olen == 4) {
			uint32_t sol_max_rt = ntohl_unaligned(odata);
			if (sol_max_rt >= DHCPV6C_SOL_MAX_RT_MIN &&
					sol_max_rt <= DHCPV6C_SOL_MAX_RT_MAX)
				cand.sol_max_rt = sol_max_rt;

		} else if (otype == DHCPV6C_OPT_INF_MAX_RT && olen == 4) {
			uint32_t inf_max_rt = ntohl_unaligned(odata);
			if (inf_max_rt >= DHCPV6C_INF_MAX_RT_MIN &&
					inf_max_rt <= DHCPV6C_INF_MAX_RT_MAX)
				cand.inf_max_rt = inf_max_rt;

		} else if (otype == DHCPV6C_OPT_IA_PD &&
					olen >= -4 + sizeof(struct dhcpv6c_ia_hdr)) {
			struct dhcpv6c_ia_hdr *h = (struct dhcpv6c_ia_hdr*)&odata[-4];
			uint8_t *oend = odata + olen, *d;
			dhcpv6c_for_each_option(&h[1], oend, otype, olen, d) {
				if (otype == DHCPV6C_OPT_IA_PREFIX &&
						olen >= -4 + sizeof(struct dhcpv6c_ia_prefix)) {
					struct dhcpv6c_ia_prefix *p = (struct dhcpv6c_ia_prefix*)&d[-4];
					have_pd = p->prefix;
				}
			}
		} else if (otype == DHCPV6C_OPT_IA_NA &&
					olen >= -4 + sizeof(struct dhcpv6c_ia_hdr)) {
			struct dhcpv6c_ia_hdr *h = (struct dhcpv6c_ia_hdr*)&odata[-4];
			uint8_t *oend = odata + olen, *d;

			dhcpv6c_for_each_option(&h[1], oend, otype, olen, d) {
				if (otype == DHCPV6C_OPT_IA_ADDR &&
						olen >= -4 + sizeof(struct dhcpv6c_ia_addr))
					have_na = true;
			}
		}
	}

	if ((!have_na && ifp->na_mode == IA_MODE_FORCE) ||
			(!have_pd && ifp->pd_mode == IA_MODE_FORCE)) {
		/*
		 * RFC7083 states to process the SOL_MAX_RT and
		 * INF_MAX_RT options even if the DHCPv6 server
		 * did not propose any IA_NA and/or IA_PD
		 */
		ifp->dhcpv6c_retx[DHCPV6C_MSG_SOLICIT].max_timeo = cand.sol_max_rt;
		ifp->dhcpv6c_retx[DHCPV6C_MSG_INFO_REQ].max_timeo = cand.inf_max_rt;
		return -1;
	}

	if (ifp->na_mode != IA_MODE_NONE && !have_na) {
		cand.has_noaddravail = true;
		cand.preference -= 1000;
	}

	if (ifp->pd_mode != IA_MODE_NONE) {
		if (have_pd)
			cand.preference += 2000 + (128 - have_pd);
		else
			cand.preference -= 2000;
	}

	if (cand.duid_len > 0) {
		cand.ia_na = dhcpv6c_state_move(ifp, DHCPV6C_STATE_IA_NA, &cand.ia_na_len);
		cand.ia_pd = dhcpv6c_state_move(ifp, DHCPV6C_STATE_IA_PD, &cand.ia_pd_len);
		dhcpv6c_add_server_cand(ifp, &cand);
	}

	return (rc > 1 || (pref == 255 && cand.preference > 0)) ? 1 : -1;
}

static int dhcpv6c_commit_advert(struct dhcpv6c_interface *ifp)
{
	return dhcpv6c_promote_server_cand(ifp);
}

static int dhcpv6c_handle_rebind_reply(struct dhcpv6c_interface *ifp, enum dhcpv6c_msg orig, const int rc,
		const void *opt, const void *end, const struct ipstack_sockaddr_in6 *from)
{
	dhcpv6c_handle_advert(ifp, orig, rc, opt, end, from);
	if (dhcpv6c_commit_advert(ifp) < 0)
		return -1;

	return dhcpv6c_handle_reply(ifp, orig, rc, opt, end, from);
}

static int dhcpv6c_handle_reply(struct dhcpv6c_interface *ifp, enum dhcpv6c_msg orig, _unused const int rc,
		const void *opt, const void *end, const struct ipstack_sockaddr_in6 *from)
{
	uint8_t *odata, *_o = NULL;
	uint16_t otype, olen;
	uint32_t refresh = 86400;
	int ret = 1;
	unsigned int state_IAs;
	unsigned int updated_IAs = 0;
	bool handled_status_codes[_DHCPV6C_Status_Max] = { false, };

	dhcpv6c_expire(ifp, true);

	if (orig == DHCPV6C_MSG_UNKNOWN) {
		static time_t last_update = 0;
		time_t now = dhcpv6c_get_milli_time() / 1000;

		uint32_t elapsed = (last_update > 0) ? now - last_update : 0;
		last_update = now;

		if (ifp->gt1 != UINT32_MAX)
			ifp->gt1 -= elapsed;

		if (ifp->gt2 != UINT32_MAX)
			ifp->gt2 -= elapsed;

		if (ifp->gt3 != UINT32_MAX)
			ifp->gt3 -= elapsed;

		if (ifp->gt1 < 0)
			ifp->gt1 = 0;

		if (ifp->gt2 < 0)
			ifp->gt2 = 0;

		if (ifp->gt3 < 0)
			ifp->gt3 = 0;
	}

	if (orig == DHCPV6C_MSG_REQUEST && !dhcpv6c_is_bound(ifp)) {
		// Delete NA and PD we have in the state from the Advert
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_IA_NA);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_IA_PD);
	}

	if (opt) {
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_DNS);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_SEARCH);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_SNTP_IP);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_NTP_IP);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_NTP_FQDN);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_SIP_IP);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_SIP_FQDN);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_AFTR_NAME);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_CER);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_S46_MAPT);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_S46_MAPE);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_S46_LW);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_PASSTHRU);
		dhcpv6c_state_clear(ifp, DHCPV6C_STATE_CUSTOM_OPTS);

		// Parse and find all matching IAs
		dhcpv6c_for_each_option(opt, end, otype, olen, odata) {
			struct dhcpv6c_option *dopt = dhcpv6c_option_find(ifp, otype);

			if ((otype == DHCPV6C_OPT_IA_PD || otype == DHCPV6C_OPT_IA_NA)
					&& olen > -4 + sizeof(struct dhcpv6c_ia_hdr)) {
				struct dhcpv6c_ia_hdr *ia_hdr = (void*)(&odata[-4]);

				if ((ifp->na_mode == IA_MODE_NONE && otype == DHCPV6C_OPT_IA_NA) ||
					(ifp->pd_mode == IA_MODE_NONE && otype == DHCPV6C_OPT_IA_PD))
					continue;

				// Test ID
				if (ia_hdr->iaid != htonl(1) && otype == DHCPV6C_OPT_IA_NA)
					continue;

				uint16_t code = DHCPV6C_Success;
				uint16_t stype, slen;
				uint8_t *sdata;
				// Get and handle status code
				dhcpv6c_for_each_option(&ia_hdr[1], odata + olen,
						stype, slen, sdata) {
					if (stype == DHCPV6C_OPT_STATUS && slen >= 2) {
						uint8_t *mdata = (slen > 2) ? &sdata[2] : NULL;
						uint16_t mlen = (slen > 2) ? slen - 2 : 0;

						code = ((int)sdata[0]) << 8 | ((int)sdata[1]);

						if (code == DHCPV6C_Success)
							continue;

						dhcpv6c_handle_ia_status_code(ifp, orig, ia_hdr,
							code, mdata, mlen, handled_status_codes, &ret);

						if (ret > 0)
							return ret;

						break;
					}
				}

				if (code != DHCPV6C_Success)
					continue;

				updated_IAs += dhcpv6c_parse_ia(ifp, ia_hdr, odata + olen);
			} else if (otype == DHCPV6C_OPT_UNICAST && olen == sizeof(server_addr)) {
				if (!(ifp->client_options & DHCPV6C_IGNORE_OPT_UNICAST))
					server_addr = *(struct ipstack_in6_addr *)odata;

			}
			else if (otype == DHCPV6C_OPT_STATUS && olen >= 2) {
				uint8_t *mdata = (olen > 2) ? &odata[2] : NULL;
				uint16_t mlen = (olen > 2) ? olen - 2 : 0;
				uint16_t code = ((int)odata[0]) << 8 | ((int)odata[1]);

				dhcpv6c_handle_status_code(ifp, orig, code, mdata, mlen, &ret);
			} else if (otype == DHCPV6C_OPT_DNS_SERVERS) {
				if (olen % 16 == 0)
					dhcpv6c_state_add(ifp, DHCPV6C_STATE_DNS, odata, olen);
			} else if (otype == DHCPV6C_OPT_DNS_DOMAIN)
				dhcpv6c_state_add(ifp, DHCPV6C_STATE_SEARCH, odata, olen);
			else if (otype == DHCPV6C_OPT_SNTP_SERVERS) {
				if (olen % 16 == 0)
					dhcpv6c_state_add(ifp, DHCPV6C_STATE_SNTP_IP, odata, olen);
			} else if (otype == DHCPV6C_OPT_NTP_SERVER) {
				uint16_t stype, slen;
				uint8_t *sdata;
				// Test status and bail if error
				dhcpv6c_for_each_option(odata, odata + olen,
						stype, slen, sdata) {
					if (slen == 16 && (stype == NTP_MC_ADDR ||
							stype == NTP_SRV_ADDR))
						dhcpv6c_state_add(ifp, DHCPV6C_STATE_NTP_IP,
								sdata, slen);
					else if (slen > 0 && stype == NTP_SRV_FQDN)
						dhcpv6c_state_add(ifp, DHCPV6C_STATE_NTP_FQDN,
								sdata, slen);
				}
			} else if (otype == DHCPV6C_OPT_SIP_SERVER_A) {
				if (olen == 16)
					dhcpv6c_state_add(ifp, DHCPV6C_STATE_SIP_IP, odata, olen);
			} else if (otype == DHCPV6C_OPT_SIP_SERVER_D)
				dhcpv6c_state_add(ifp, DHCPV6C_STATE_SIP_FQDN, odata, olen);
			else if (otype == DHCPV6C_OPT_INFO_REFRESH && olen >= 4) {
				refresh = ntohl_unaligned(odata);
			} else if (otype == DHCPV6C_OPT_AUTH) {
				if (olen == -4 + sizeof(struct dhcpv6c_auth_reconfigure)) {
					struct dhcpv6c_auth_reconfigure *r = (void*)&odata[-4];
					if (r->protocol == 3 && r->algorithm == 1 &&
							r->reconf_type == 1)
						memcpy(ifp->reconf_key, r->key, sizeof(r->key));
				}
			} else if (otype == DHCPV6C_OPT_AFTR_NAME && olen > 3) {
				size_t cur_len;
				dhcpv6c_state_get(ifp, DHCPV6C_STATE_AFTR_NAME, &cur_len);
				if (cur_len == 0)
					dhcpv6c_state_add(ifp, DHCPV6C_STATE_AFTR_NAME, odata, olen);
			} else if (otype == DHCPV6C_OPT_SOL_MAX_RT && olen == 4) {
				uint32_t sol_max_rt = ntohl_unaligned(odata);
				if (sol_max_rt >= DHCPV6C_SOL_MAX_RT_MIN &&
						sol_max_rt <= DHCPV6C_SOL_MAX_RT_MAX)
					ifp->dhcpv6c_retx[DHCPV6C_MSG_SOLICIT].max_timeo = sol_max_rt;
			} else if (otype == DHCPV6C_OPT_INF_MAX_RT && olen == 4) {
				uint32_t inf_max_rt = ntohl_unaligned(odata);
				if (inf_max_rt >= DHCPV6C_INF_MAX_RT_MIN &&
						inf_max_rt <= DHCPV6C_INF_MAX_RT_MAX)
					ifp->dhcpv6c_retx[DHCPV6C_MSG_INFO_REQ].max_timeo = inf_max_rt;
	#ifdef EXT_CER_ID
			} else if (otype == DHCPV6C_OPT_CER_ID && olen == -4 +
					sizeof(struct dhcpv6c_cer_id)) {
				struct dhcpv6c_cer_id *cer_id = (void*)&odata[-4];
				struct ipstack_in6_addr any = IN6ADDR_ANY_INIT;
				if (memcmp(&cer_id->addr, &any, sizeof(any)))
					dhcpv6c_state_add(ifp, DHCPV6C_STATE_CER, &cer_id->addr, sizeof(any));
	#endif
			} else if (otype == DHCPV6C_OPT_S46_CONT_MAPT) {
				dhcpv6c_state_add(ifp, DHCPV6C_STATE_S46_MAPT, odata, olen);
			} else if (otype == DHCPV6C_OPT_S46_CONT_MAPE) {
				size_t mape_len;
				dhcpv6c_state_get(ifp, DHCPV6C_STATE_S46_MAPE, &mape_len);
				if (mape_len == 0)
					dhcpv6c_state_add(ifp, DHCPV6C_STATE_S46_MAPE, odata, olen);
			} else if (otype == DHCPV6C_OPT_S46_CONT_LW) {
				dhcpv6c_state_add(ifp, DHCPV6C_STATE_S46_LW, odata, olen);
			} else
				dhcpv6c_state_add(ifp, DHCPV6C_STATE_CUSTOM_OPTS, &odata[-4], olen + 4);

			if (!dopt || !(dopt->flags & OPT_NO_PASSTHRU))
				dhcpv6c_state_add(ifp, DHCPV6C_STATE_PASSTHRU, &odata[-4], olen + 4);
		}
	}

	// Bail out if fatal status code was received
	if (ret <= 0)
		return ret;

	switch (orig) {
	case DHCPV6C_MSG_REQUEST:
	case DHCPV6C_MSG_REBIND:
	case DHCPV6C_MSG_RENEW:
		state_IAs = dhcpv6c_calc_refresh_timers(ifp);
		// In case there're no state IA entries
		// keep sending request/renew/rebind messages
		if (state_IAs == 0) {
			ret = 0;
			break;
		}

		if (orig == DHCPV6C_MSG_REQUEST) {
			// All server candidates can be cleared if not yet bound
			if (!dhcpv6c_is_bound(ifp))
				dhcpv6c_clear_server_cand(ifp);

			dhcpv6c_state_clear(ifp, DHCPV6C_STATE_SERVER_ADDR);
			dhcpv6c_state_add(ifp, DHCPV6C_STATE_SERVER_ADDR, &from->sin6_addr, 16);
		} else if (orig == DHCPV6C_MSG_RENEW) {
			// Send further renews if T1 is not set and if
			// there're IAs which were not in the Reply message
			if (!ifp->gt1 && state_IAs != updated_IAs) {
				if (updated_IAs)
					// Publish updates
					//script_call("updated", 0, false);

				/*
				 * RFC8415 states following in §18.2.10.1 :
				 * Sends a Renew/Rebind if any of the IAs are not in the Reply
				 * message, but as this likely indicates that the server that
				 * responded does not support that IA type, sending immediately is
				 * unlikely to produce a different result.  Therefore, the client
				 * MUST rate-limit its transmissions (see Section 14.1) and MAY just
				 * wait for the normal retransmission time (as if the Reply message
				 * had not been received).  The client continues to use other
				 * bindings for which the server did return information
				 */
				ret = -1;
			}
		} else if (orig == DHCPV6C_MSG_REBIND) {
			dhcpv6c_state_clear(ifp, DHCPV6C_STATE_SERVER_ADDR);
			dhcpv6c_state_add(ifp, DHCPV6C_STATE_SERVER_ADDR, &from->sin6_addr, 16);

			// Send further rebinds if T1 and T2 is not set and if
			// there're IAs which were not in the Reply message
			if (!ifp->gt1 && !ifp->gt2 && state_IAs != updated_IAs) {
				if (updated_IAs)
					// Publish updates
					//script_call("updated", 0, false);

				/*
				 * RFC8415 states following in §18.2.10.1 :
				 * Sends a Renew/Rebind if any of the IAs are not in the Reply
				 * message, but as this likely indicates that the server that
				 * responded does not support that IA type, sending immediately is
				 * unlikely to produce a different result.  Therefore, the client
				 * MUST rate-limit its transmissions (see Section 14.1) and MAY just
				 * wait for the normal retransmission time (as if the Reply message
				 * had not been received).  The client continues to use other
				 * bindings for which the server did return information
				 */
				ret = -1;
			}
		}
		break;

	case DHCPV6C_MSG_INFO_REQ:
		// All server candidates can be cleared if not yet bound
		if (!dhcpv6c_is_bound(ifp))
			dhcpv6c_clear_server_cand(ifp);

		ifp->gt1 = refresh;
		break;

	default:
		break;
	}

	return ret;
}


// Note this always takes ownership of cand->ia_na and cand->ia_pd
static void dhcpv6c_add_server_cand(struct dhcpv6c_interface *ifp, const struct dhcpv6c_server_cand *cand)
{
	size_t cand_len, i;
	struct dhcpv6c_server_cand *c = dhcpv6c_state_get(ifp, DHCPV6C_STATE_SERVER_CAND, &cand_len);

	// Remove identical duid server candidate
	for (i = 0; i < cand_len / sizeof(*c); ++i) {
		if (cand->duid_len == c[i].duid_len &&
				!memcmp(cand->duid, c[i].duid, cand->duid_len)) {
			free(c[i].ia_na);
			free(c[i].ia_pd);
			dhcpv6c_state_remove(ifp, DHCPV6C_STATE_SERVER_CAND, i * sizeof(*c), sizeof(*c));
			break;
		}
	}

	for (i = 0, c = dhcpv6c_state_get(ifp, DHCPV6C_STATE_SERVER_CAND, &cand_len);
		i < cand_len / sizeof(*c); ++i) {
		if (c[i].preference < cand->preference)
			break;
	}

	if (dhcpv6c_state_insert(ifp, DHCPV6C_STATE_SERVER_CAND, i * sizeof(*c), cand, sizeof(*cand))) {
		free(cand->ia_na);
		free(cand->ia_pd);
	}
}

static void dhcpv6c_clear_server_cand(struct dhcpv6c_interface *ifp)
{
	size_t cand_len, i;
	struct dhcpv6c_server_cand *c = dhcpv6c_state_get(ifp, DHCPV6C_STATE_SERVER_CAND, &cand_len);

	// Server candidates need deep delete for IA_NA/IA_PD
	for (i = 0; i < cand_len / sizeof(*c); ++i) {
		free(c[i].ia_na);
		free(c[i].ia_pd);
	}
	dhcpv6c_state_clear(ifp, DHCPV6C_STATE_SERVER_CAND);
}

int dhcpv6c_promote_server_cand(struct dhcpv6c_interface *ifp)
{
	size_t cand_len;
	struct dhcpv6c_server_cand *cand = dhcpv6c_state_get(ifp, DHCPV6C_STATE_SERVER_CAND, &cand_len);
	uint16_t hdr[2];
	int ret = DHCPV6C_STATELESS;

	// Clear lingering candidate state info
	dhcpv6c_state_clear(ifp, DHCPV6C_STATE_SERVER_ID);
	dhcpv6c_state_clear(ifp, DHCPV6C_STATE_IA_NA);
	dhcpv6c_state_clear(ifp, DHCPV6C_STATE_IA_PD);

	if (!cand_len)
		return -1;

	if (cand->has_noaddravail && ifp->na_mode == IA_MODE_TRY) {
		ifp->na_mode = IA_MODE_NONE;

		ifp->dhcpv6c_retx[DHCPV6C_MSG_SOLICIT].max_timeo = cand->sol_max_rt;
		ifp->dhcpv6c_retx[DHCPV6C_MSG_INFO_REQ].max_timeo = cand->inf_max_rt;

		return dhcpv6c_request(ifp, DHCPV6C_MSG_SOLICIT);
	}

	hdr[0] = htons(DHCPV6C_OPT_SERVERID);
	hdr[1] = htons(cand->duid_len);
	dhcpv6c_state_add(ifp, DHCPV6C_STATE_SERVER_ID, hdr, sizeof(hdr));
	dhcpv6c_state_add(ifp, DHCPV6C_STATE_SERVER_ID, cand->duid, cand->duid_len);
	ifp->accept_reconfig = cand->wants_reconfigure;

	if (cand->ia_na_len) {
		dhcpv6c_state_add(ifp, DHCPV6C_STATE_IA_NA, cand->ia_na, cand->ia_na_len);
		free(cand->ia_na);
		if (ifp->na_mode != IA_MODE_NONE)
			ret = DHCPV6C_STATEFUL;
	}

	if (cand->ia_pd_len) {
		dhcpv6c_state_add(ifp, DHCPV6C_STATE_IA_PD, cand->ia_pd, cand->ia_pd_len);
		free(cand->ia_pd);
		if (ifp->pd_mode != IA_MODE_NONE)
			ret = DHCPV6C_STATEFUL;
	}

	ifp->dhcpv6c_retx[DHCPV6C_MSG_SOLICIT].max_timeo = cand->sol_max_rt;
	ifp->dhcpv6c_retx[DHCPV6C_MSG_INFO_REQ].max_timeo = cand->inf_max_rt;

	dhcpv6c_state_remove(ifp, DHCPV6C_STATE_SERVER_CAND, 0, sizeof(*cand));

	return ret;
}

#endif /*ZPL_DHCPV6C_MODULE*/