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
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>

#include <net/if.h>
#include <sys/syscall.h>
#include <arpa/inet.h>
#include <linux/if_addr.h>
*/
#include "auto_include.h"
#include <zplos_include.h>
#ifdef ZPL_DHCPV6C_MODULE
#include "resolv.h"
#include "dhcpv6c.h"
#include "dhcpv6c_api.h"
#include "dhcpv6c_state.h"
#include "dhcpv6c_option.h"
#include "odhcp6c.h"
#include "rtadv.h"


#define ARRAY_SEP " ,\t"

#if 1

static int parse_opt_data(const char *data, uint8_t **dst,
		const unsigned int type, const bool array);
static int parse_opt(struct dhcpv6c_interface *difp, const char *opt);


static int urandom_fd = -1, allow_slaac_only = 0;
static bool bound = false, release = true, ra = false;

static char *ifname = NULL;
struct dhcpv6c_interface *difp;

static unsigned int script_sync_delay = 10;
static unsigned int script_accu_delay = 1;

int dhcpv6_main(_unused int argc, char* const argv[])
{
	static struct ipstack_in6_addr ifid = IN6ADDR_ANY_INIT;
	// Allocate resources
	const char *pidfile = NULL;
	const char *script = "/usr/sbin/odhcp6c-update";
	ssize_t l;
	uint8_t buf[134], *o_data;
	char *optpos;
	uint16_t opttype;
	enum dhcpv6c_ia_mode ia_na_mode = IA_MODE_TRY;
	enum dhcpv6c_ia_mode ia_pd_mode = IA_MODE_NONE;
	struct dhcpv6c_option *opt;
	int ia_pd_iaid_index = 0;
	int sol_timeout = DHCPV6C_SOL_MAX_RT;
	int verbosity = 0;
	bool help = false, daemonize = false;
	int logopt = LOG_PID;
	int c, res;
	unsigned int client_options = DHCPV6C_CLIENT_FQDN | DHCPV6C_ACCEPT_RECONFIGURE;
	unsigned int ra_options = RA_RDNSS_DEFAULT_LIFETIME;
	unsigned int ra_holdoff_interval = RA_MIN_ADV_INTERVAL;

	while ((c = getopt(argc, argv, "S::N:V:P:FB:c:i:r:Ru:Ux:s:kt:m:Lhedp:fav")) != -1) {
		switch (c) {
		case 'S':
			allow_slaac_only = (optarg) ? atoi(optarg) : -1;
			break;

		case 'N':
			if (!strcmp(optarg, "force")) {
				ia_na_mode = IA_MODE_FORCE;
				allow_slaac_only = -1;
			} else if (!strcmp(optarg, "none"))
				ia_na_mode = IA_MODE_NONE;
			else if (!strcmp(optarg, "try"))
				ia_na_mode = IA_MODE_TRY;
			else
				help = true;
			break;

		case 'V':
			opt = dhcpv6c_option_find(difp, DHCPV6C_OPT_VENDOR_CLASS);
			if (!opt) {
				syslog(LOG_ERR, "Failed to set vendor-class option");
				return 1;
			}

			o_data = NULL;
			res = parse_opt_data(optarg, &o_data, opt->flags & OPT_MASK_SIZE,
						(opt->flags & OPT_ARRAY) == OPT_ARRAY);
			if (res > 0) {
				res = dhcpv6c_option_add(difp, opt->code, o_data, res);
				if (res) {
					if (res > 0)
						return 1;

					help = true;
				}
			} else
				help = true;

			free(o_data);
			break;

		case 'P':
			if (ia_pd_mode == IA_MODE_NONE)
				ia_pd_mode = IA_MODE_TRY;

			if (allow_slaac_only >= 0 && allow_slaac_only < 10)
				allow_slaac_only = 10;

			char *iaid_begin;
			int iaid_len = 0;
			int prefix_length = strtoul(optarg, &iaid_begin, 10);

			if (*iaid_begin != '\0' && *iaid_begin != ',' && *iaid_begin != ':') {
				syslog(LOG_ERR, "invalid argument: '%s'", optarg);
				return 1;
			}

			struct dhcpv6c_request_prefix prefix = { 0, prefix_length };

			if (*iaid_begin == ',' && (iaid_len = strlen(iaid_begin)) > 1)
				memcpy(&prefix.iaid, iaid_begin + 1, iaid_len > 4 ? 4 : iaid_len);
			else if (*iaid_begin == ':')
				prefix.iaid = htonl((uint32_t)strtoul(&iaid_begin[1], NULL, 16));
			else
				prefix.iaid = htonl(++ia_pd_iaid_index);

			if (dhcpv6c_state_add(difp, DHCPV6C_STATE_IA_PD_INIT, &prefix, sizeof(prefix))) {
				syslog(LOG_ERR, "Failed to set request IPv6-Prefix");
				return 1;
			}
			break;

		case 'F':
			allow_slaac_only = -1;
			ia_pd_mode = IA_MODE_FORCE;
			break;

		case 'c':
			l = script_unhexlify(&buf[4], sizeof(buf) - 4, optarg);
			if (l > 0) {
				buf[0] = 0;
				buf[1] = DHCPV6C_OPT_CLIENTID;
				buf[2] = 0;
				buf[3] = l;
				if (dhcpv6c_state_add(difp, DHCPV6C_STATE_CLIENT_ID, buf, l + 4)) {
					syslog(LOG_ERR, "Failed to override client-ID");
					return 1;
				}
			} else
				help = true;
			break;

		case 'i':
			if (inet_pton(AF_INET6, optarg, &ifid) != 1)
				help = true;
			break;

		case 'r':
			optpos = optarg;
			while (optpos[0]) {
				opttype = htons(strtoul(optarg, &optpos, 10));
				if (optpos == optarg)
					break;
				else if (optpos[0])
					optarg = &optpos[1];

				if (dhcpv6c_state_add(difp, DHCPV6C_STATE_ORO, &opttype, 2)) {
					syslog(LOG_ERR, "Failed to add requested option");
					return 1;
				}
			}
			break;

		case 'R':
			client_options |= DHCPV6C_STRICT_OPTIONS;
			break;

		case 'u':
			opt = dhcpv6c_option_find(difp, DHCPV6C_OPT_USER_CLASS);
			if (!opt) {
				syslog(LOG_ERR, "Failed to set user-class option");
				return 1;
			}

			o_data = NULL;
			res = parse_opt_data(optarg, &o_data, opt->flags & OPT_MASK_SIZE,
						(opt->flags & OPT_ARRAY) == OPT_ARRAY);
			if (res > 0) {
				res = dhcpv6c_option_add(difp, opt->code, o_data, res);
				if (res) {
					if (res > 0)
						return 1;

					help = true;
				}
			} else
				help = true;

			free(o_data);
			break;

		case 'U':
			client_options |= DHCPV6C_IGNORE_OPT_UNICAST;
			break;

		case 's':
			script = optarg;
			break;

		case 'k':
			release = false;
			break;

		case 't':
			sol_timeout = atoi(optarg);
			break;

		case 'm':
			ra_holdoff_interval = atoi(optarg);
			break;

		case 'L':
			ra_options &= ~RA_RDNSS_DEFAULT_LIFETIME;
			break;

		case 'e':
			logopt |= LOG_PERROR;
			break;

		case 'd':
			daemonize = true;
			break;

		case 'p':
			pidfile = optarg;
			break;

		case 'f':
			client_options &= ~DHCPV6C_CLIENT_FQDN;
			break;

		case 'a':
			client_options &= ~DHCPV6C_ACCEPT_RECONFIGURE;
			break;

		case 'v':
			++verbosity;
			break;

		case 'x':
			res = parse_opt(difp, optarg);
			if (res) {
				if (res > 0)
					return res;

				help = true;
			}
			break;

		default:
			help = true;
			break;
		}
	}

	if (allow_slaac_only > 0)
		script_sync_delay = allow_slaac_only;



	if ((urandom_fd = open("/dev/urandom", O_CLOEXEC | O_RDONLY)) < 0 ||
			init_dhcpv6c(difp, client_options, sol_timeout) ||
			ra_init(ifname, &ifid, ra_options, ra_holdoff_interval) ||
			script_init(script, ifname)) {
		syslog(LOG_ERR, "failed to initialize: %s", strerror(errno));
		return 3;
	}

	if (daemonize) {
		openlog("odhcp6c", LOG_PID, LOG_DAEMON); // Disable LOG_PERROR
		if (daemon(0, 0)) {
			syslog(LOG_ERR, "Failed to daemonize: %s",
					strerror(errno));
			return 4;
		}

		if (!pidfile) {
			snprintf((char*)buf, sizeof(buf), "/var/run/odhcp6c.%s.pid", ifname);
			pidfile = (char*)buf;
		}

		FILE *fp = fopen(pidfile, "w");
		if (fp) {
			fprintf(fp, "%i\n", getpid());
			fclose(fp);
		}
	}

	script_call("started", 0, false);

	while (1) { // Main logic
		dhcpv6c_state_clear(difp, DHCPV6C_STATE_SERVER_ID);
		dhcpv6c_state_clear(difp, DHCPV6C_STATE_SERVER_ADDR);
		dhcpv6c_state_clear(difp, DHCPV6C_STATE_IA_NA);
		dhcpv6c_state_clear(difp, DHCPV6C_STATE_IA_PD);
		dhcpv6c_state_clear(difp, DHCPV6C_STATE_SNTP_IP);
		dhcpv6c_state_clear(difp, DHCPV6C_STATE_NTP_IP);
		dhcpv6c_state_clear(difp, DHCPV6C_STATE_NTP_FQDN);
		dhcpv6c_state_clear(difp, DHCPV6C_STATE_SIP_IP);
		dhcpv6c_state_clear(difp, DHCPV6C_STATE_SIP_FQDN);
		bound = false;

		syslog(LOG_NOTICE, "(re)starting transaction on %s", ifname);


		int mode = dhcpv6c_set_ia_mode(difp, ia_na_mode, ia_pd_mode);
		if (mode != DHCPV6C_STATELESS)
			mode = dhcpv6c_request(difp, DHCPV6C_MSG_SOLICIT);



		if (mode < 0)
			continue;

		do {
			res = dhcpv6c_request(difp, mode == DHCPV6C_STATELESS ?
					DHCPV6C_MSG_INFO_REQ : DHCPV6C_MSG_REQUEST);
			bool signalled = dhcpv6c_signal_process();

			if (res > 0)
				break;
			else if (signalled) {
				mode = -1;
				break;
			}

			mode = dhcpv6c_promote_server_cand(difp);
		} while (mode > DHCPV6C_UNKNOWN);

		if (mode < 0)
			continue;

		switch (mode) {
		case DHCPV6C_STATELESS:
			bound = true;
			syslog(LOG_NOTICE, "entering stateless-mode on %s", ifname);

			while (1) {
	
				script_call("informed", script_sync_delay, true);

				res = dhcpv6c_poll_reconfigure(difp);
	

				if (res > 0)
					continue;

	

				res = dhcpv6c_request(difp, DHCPV6C_MSG_INFO_REQ);
			
			}
			break;

		case DHCPV6C_STATEFUL:
			bound = true;
			script_call("bound", script_sync_delay, true);
			syslog(LOG_NOTICE, "entering stateful-mode on %s", ifname);

			while (1) {
				// Renew Cycle
				// Wait for T1 to expire or until we get a reconfigure
				res = dhcpv6c_poll_reconfigure(difp);
				dhcpv6c_signal_process();
				if (res > 0) {
					script_call("updated", 0, false);
					continue;
				}


				// Send renew as T1 expired
				res = dhcpv6c_request(difp, DHCPV6C_MSG_RENEW);
				dhcpv6c_signal_process();

				if (res > 0) { // Renew was succesfull
					// Publish updates
					script_call("updated", 0, false);
					continue; // Renew was successful
				}

				dhcpv6c_state_clear(difp, DHCPV6C_STATE_SERVER_ID); // Remove binding
				dhcpv6c_state_clear(difp, DHCPV6C_STATE_SERVER_ADDR);

				size_t ia_pd_len, ia_na_len;
				dhcpv6c_state_get(difp, DHCPV6C_STATE_IA_PD, &ia_pd_len);
				dhcpv6c_state_get(difp, DHCPV6C_STATE_IA_NA, &ia_na_len);

				if (ia_pd_len == 0 && ia_na_len == 0)
					break;

				// If we have IAs, try rebind otherwise restart
				res = dhcpv6c_request(difp, DHCPV6C_MSG_REBIND);
				dhcpv6c_signal_process();

				if (res > 0)
					script_call("rebound", 0, true);
				else
					break;
			}
			break;

		default:
			break;
		}

		dhcpv6c_expire(difp, false);

		size_t ia_pd_len, ia_na_len, server_id_len;
		dhcpv6c_state_get(difp, DHCPV6C_STATE_IA_PD, &ia_pd_len);
		dhcpv6c_state_get(difp, DHCPV6C_STATE_IA_NA, &ia_na_len);
		dhcpv6c_state_get(difp, DHCPV6C_STATE_SERVER_ID, &server_id_len);

		// Add all prefixes to lost prefixes
		bound = false;
		script_call("unbound", 0, true);

		if (server_id_len > 0 && (ia_pd_len > 0 || ia_na_len > 0) && release)
			dhcpv6c_request(difp, DHCPV6C_MSG_RELEASE);

		dhcpv6c_state_clear(difp, DHCPV6C_STATE_IA_NA);
		dhcpv6c_state_clear(difp, DHCPV6C_STATE_IA_PD);
	}

	script_call("stopped", 0, true);

	return 0;
}






static int parse_opt_u8(const char *src, uint8_t **dst)
{
	int len = strlen(src);

	*dst = realloc(*dst, len/2);
	if (!*dst)
		return -1;

	return script_unhexlify(*dst, len, src);
}

static int parse_opt_string(const char *src, uint8_t **dst, const bool array)
{
	int o_len = 0;
	char *sep = strpbrk(src, ARRAY_SEP);

	if (sep && !array)
		return -1;

	do {
		if (sep) {
			*sep = 0;
			sep++;
		}

		int len = strlen(src);

		*dst = realloc(*dst, o_len + len);
		if (!*dst)
			return -1;

		memcpy(&((*dst)[o_len]), src, len);

		o_len += len;
		src = sep;

		if (sep)
			sep = strpbrk(src, ARRAY_SEP);
	} while (src);

	return o_len;
}

static int parse_opt_dns_string(const char *src, uint8_t **dst, const bool array)
{
	int o_len = 0;
	char *sep = strpbrk(src, ARRAY_SEP);

	if (sep && !array)
		return -1;

	do {
		uint8_t tmp[256];

		if (sep) {
			*sep = 0;
			sep++;
		}

		int len = dn_comp(src, tmp, sizeof(tmp), NULL, NULL);
		if (len < 0)
			return -1;

		*dst = realloc(*dst, o_len + len);
		if (!*dst)
			return -1;

		memcpy(&((*dst)[o_len]), tmp, len);

		o_len += len;
		src = sep;

		if (sep)
			sep = strpbrk(src, ARRAY_SEP);
	} while (src);

	return o_len;
}

static int parse_opt_ip6(const char *src, uint8_t **dst, const bool array)
{
	int o_len = 0;
	char *sep = strpbrk(src, ARRAY_SEP);

	if (sep && !array)
		return -1;

	do {
		int len = sizeof(struct ipstack_in6_addr);

		if (sep) {
			*sep = 0;
			sep++;
		}

		*dst = realloc(*dst, o_len + len);
		if (!*dst)
			return -1;

		if (inet_pton(AF_INET6, src, &((*dst)[o_len])) < 1)
			return -1;

		o_len += len;
		src = sep;

		if (sep)
			sep = strpbrk(src, ARRAY_SEP);
	} while (src);

	return o_len;
}

static int parse_opt_user_class(const char *src, uint8_t **dst, const bool array)
{
	int o_len = 0;
	char *sep = strpbrk(src, ARRAY_SEP);

	if (sep && !array)
		return -1;

	do {
		if (sep) {
			*sep = 0;
			sep++;
		}
		uint16_t str_len = strlen(src);

		*dst = realloc(*dst, o_len + str_len + 2);
		if (!*dst)
			return -1;

		struct user_class {
			uint16_t len;
			uint8_t data[];
		} *e = (struct user_class *)&((*dst)[o_len]);

		e->len = ntohs(str_len);
		memcpy(e->data, src, str_len);

		o_len += str_len + 2;
		src = sep;

		if (sep)
			sep = strpbrk(src, ARRAY_SEP);
	} while (src);

	return o_len;
}

static int parse_opt_data(const char *data, uint8_t **dst, const unsigned int type,
		const bool array)
{
	int ret = 0;

	switch (type) {
	case OPT_U8:
		ret = parse_opt_u8(data, dst);
		break;

	case OPT_STR:
		ret = parse_opt_string(data, dst, array);
		break;

	case OPT_DNS_STR:
		ret = parse_opt_dns_string(data, dst, array);
		break;

	case OPT_IP6:
		ret = parse_opt_ip6(data, dst, array);
		break;

	case OPT_USER_CLASS:
		ret = parse_opt_user_class(data, dst, array);
		break;

	default:
		ret = -1;
		break;
	}

	return ret;
}

static int parse_opt(struct dhcpv6c_interface *ifp, const char *opt)
{
	uint32_t optn;
	char *data;
	uint8_t *payload = NULL;
	int payload_len;
	unsigned int type = OPT_U8;
	bool array = false;
	struct dhcpv6c_option *dopt = NULL;
	int ret = -1;

	data = strpbrk(opt, ":");
	if (!data)
		return -1;

	*data = '\0';
	data++;

	if (strlen(opt) == 0 || strlen(data) == 0)
		return -1;

	dopt = dhcpv6c_option_find_by_name(ifp, opt);
	if (!dopt) {
		char *e;
		optn = strtoul(opt, &e, 0);
		if (*e || e == opt || optn > USHRT_MAX)
			return -1;

		dopt = dhcpv6c_option_find(ifp, optn);
	} else
		optn = dopt->code;

	/* Check if the type for the content is well-known */
	if (dopt) {
		/* Refuse internal options */
		if (dopt->flags & OPT_INTERNAL)
			return -1;

		type = dopt->flags & OPT_MASK_SIZE;
		array = ((dopt->flags & OPT_ARRAY) == OPT_ARRAY) ? true : false;
	} else if (data[0] == '"' || data[0] == '\'') {
		char *end = strrchr(data + 1, data[0]);

		if (end && (end == (data + strlen(data) - 1))) {
			/* Raw option is specified as a string */
			type = OPT_STR;
			data++;
			*end = '\0';
		}

	}

	payload_len = parse_opt_data(data, &payload, type, array);
	if (payload_len > 0)
		ret = dhcpv6c_option_add(ifp, optn, payload, payload_len);

	free(payload);

	return ret;
}

#endif 

#endif /*ZPL_DHCPV6C_MODULE*/