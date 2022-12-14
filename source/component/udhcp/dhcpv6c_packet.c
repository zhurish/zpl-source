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
#include "dhcp_def.h"

#ifdef ZPL_DHCPV6C_MODULE
#include "resolv.h"
#include "dhcpv6c.h"
#include "dhcpv6c_state.h"
#include "dhcpv6c_packet.h"
#include "md5.h"


int dhcpv6_packet_init(struct dhcpv6c_packet *packet, int msgtype)
{
	memset(packet, 0, sizeof(struct dhcpv6c_packet));
	packet->hdr.msg_type = msgtype;
	return sizeof(struct dhcpv6c_packet);
}



int dhcpv6_packet_option(dhcpv6_option_set_t *option_tbl, enum dhcpv6c_msg type, char *data, zpl_uint32 len)
{
	zpl_uint32 i = 0, offset = 0;
	struct dhcpv6c_fqdn fqdn;
	if(!option_tbl || !data)
		return 0;
	offset = dhcpv6_end_option(data);
	if(offset < 0)
		return 0;
	dhcpv6_option_hdr_t *msg = (dhcpv6_option_hdr_t *)(data + offset);
	for(i = 0; i <= DHCPV6C_OPT_MAX; i++)
	{
		if(option_tbl[i].code == DHCPV6C_OPT_CLIENTID)
			continue;

		if ((type != DHCPV6C_MSG_SOLICIT && type != DHCPV6C_MSG_REQUEST))
		{
			if(option_tbl[i].code == DHCPV6C_OPT_RECONF_ACCEPT)
				continue;
		}

		if(option_tbl[i].len && option_tbl[i].data)
		{
			msg = (dhcpv6_option_hdr_t *)(data + offset);
			if(option_tbl[i].code == DHCPV6C_OPT_FQDN)
			{
				size_t fqdn_len = 5 + dn_comp(option_tbl[i].data, fqdn.data, sizeof(fqdn.data), NULL, NULL);
				fqdn.type = htons(DHCPV6C_OPT_FQDN);
				fqdn.len = htons(fqdn_len - 4);
				fqdn.flags = 0;
				memcpy(msg->val.pval, &fqdn, fqdn_len);
				offset += fqdn_len;
			}
			else
			{
				msg->code = htons(option_tbl[i].code);
				msg->len = htons(option_tbl[i].len);
				if(option_tbl[i].len)
					memcpy(msg->val.pval, option_tbl[i].data, option_tbl[i].len);
				offset += DHCPV6_OPT_DATA + option_tbl[i].len;
			}
		}
		else
		{
			if(option_tbl[i].code)
			{
				msg->code = htons(option_tbl[i].code);
				msg->len = htons(option_tbl[i].len);
				if(option_tbl[i].len)
					memcpy(msg->val.pval, option_tbl[i].data, option_tbl[i].len);
				offset += DHCPV6_OPT_DATA + option_tbl[i].len;
			}
		}
	}
	dhcp_option_packet_end(data + offset, len - offset);
	return offset;
}

int dhcpv6_packet_build_header(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len)
{
	dhcpv6_option_set_t *opt = dhcpv6_option_find(option_tbl, DHCPV6C_OPT_CLIENTID);
	dhcpv6_option_hdr_t *msg = (dhcpv6_option_hdr_t *)(data);
	msg->code = htons(opt->code);
	msg->len = htons(opt->len);
	if(opt->len)
		memcpy(msg->val.pval, opt->data, opt->len);
	return (DHCPV6_OPT_DATA + opt->len);
}

int dhcpv6_packet_build(dhcpv6_option_set_t *option_tbl, zpl_uint16 code, char *data, zpl_uint32 len)
{
	dhcpv6_option_set_t *opt = dhcpv6_option_find(option_tbl, code);
	dhcpv6_option_hdr_t *msg = (dhcpv6_option_hdr_t *)(data);
	msg->code = htons(opt->code);
	msg->len = htons(opt->len);
	if(opt->len)
		memcpy(msg->val.pval, opt->data, opt->len);
	return (DHCPV6_OPT_DATA + opt->len);
}

static int dhcpv6_packet_build_option(zpl_uint16 code, char *data, zpl_uint32 len)
{
	dhcpv6_option_hdr_t *msg = (dhcpv6_option_hdr_t *)(data);
	msg->code = htons(code);
	msg->len = htons(len);
	if(len)
		memcpy(msg->val.pval, data, len);
	return (DHCPV6_OPT_DATA + len);
}

/* Milticast a DHCPv6 Solicit packet to the network, with an optionally requested IP.
 *
 * RFC 3315 17.1.1. Creation of Solicit Messages
 *
 * The client MUST include a Client Identifier option to identify itself
 * to the server.  The client includes IA options for any IAs to which
 * it wants the server to assign addresses.  The client MAY include
 * addresses in the IAs as a hint to the server about addresses for
 * which the client has a preference. ...
 *
 * The client uses IA_NA options to request the assignment of non-
 * temporary addresses and uses IA_TA options to request the assignment
 * of temporary addresses.  Either IA_NA or IA_TA options, or a
 * combination of both, can be included in DHCP messages.
 *
 * The client SHOULD include an Option Request option (see section 22.7)
 * to indicate the options the client is interested in receiving.  The
 * client MAY additionally include instances of those options that are
 * identified in the Option Request option, with data values as hints to
 * the server about parameter values the client would like to have
 * returned.
 *
 * The client includes a Reconfigure Accept option (see section 22.20)
 * if the client is willing to accept Reconfigure messages from the
 * server.
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |        OPTION_CLIENTID        |          option-len           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      .                                                               .
      .                              DUID                             .
      .                        (variable length)                      .
      .                                                               .
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |          OPTION_IA_NA         |          option-len           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                        IAID (4 octets)                        |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                              T1                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                              T2                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      .                         IA_NA-options                         .
      .                                                               .
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |          OPTION_IAADDR        |          option-len           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      |                         IPv6 address                          |
      |                                                               |
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                      preferred-lifetime                       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                        valid-lifetime                         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      .                                                               .
      .                        IAaddr-options                         .
      .                                                               .
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           OPTION_ORO          |           option-len          |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |    requested-option-code-1    |    requested-option-code-2    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                              ...                              |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |     OPTION_RECONF_ACCEPT      |               0               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
int dhcpv6_packet_build_solicit(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len)
{
	int datalen = 0, i = 0;
	struct dhcpv6c_packet *packet = data;
	uint16_t req_oro[16];
	/*
	 = {
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
		*/
	if(dhcpv6_option_find(option_tbl, DHCPV6C_OPT_SIP_SERVER_D))
		req_oro[i++] = htons(DHCPV6C_OPT_SIP_SERVER_D);
	if(dhcpv6_option_find(option_tbl, DHCPV6C_OPT_SIP_SERVER_A))
		req_oro[i++] = htons(DHCPV6C_OPT_SIP_SERVER_A);
	if(dhcpv6_option_find(option_tbl, DHCPV6C_OPT_DNS_SERVERS))
		req_oro[i++] = htons(DHCPV6C_OPT_DNS_SERVERS);
	if(dhcpv6_option_find(option_tbl, DHCPV6C_OPT_DNS_DOMAIN))
		req_oro[i++] = htons(DHCPV6C_OPT_DNS_DOMAIN);
	if(dhcpv6_option_find(option_tbl, DHCPV6C_OPT_SNTP_SERVERS))
		req_oro[i++] = htons(DHCPV6C_OPT_SNTP_SERVERS);
	if(dhcpv6_option_find(option_tbl, DHCPV6C_OPT_NTP_SERVER))
		req_oro[i++] = htons(DHCPV6C_OPT_NTP_SERVER);

	if(dhcpv6_option_find(option_tbl, DHCPV6C_OPT_RADIUS))
		req_oro[i++] = htons(DHCPV6C_OPT_RADIUS);

	datalen = dhcpv6_packet_init(packet, DHCPV6C_MSG_SOLICIT);
	datalen += dhcpv6_packet_build_header(option_tbl, data + datalen, len - datalen);

	datalen += dhcpv6_packet_build_option(DHCPV6C_OPT_ORO, req_oro, sizeof(uint16_t)*i);

	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_RECONF_ACCEPT, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_FQDN, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_NA, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_ADDR, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_PD, data + datalen, len - datalen);

	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ELAPSED, data + datalen, len - datalen);

	return datalen;
}

int dhcpv6_packet_build_request(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len)
{
	int datalen = 0;
	struct dhcpv6c_packet *packet = data;
	datalen = dhcpv6_packet_init(packet, DHCPV6C_MSG_REQUEST);
	datalen += dhcpv6_packet_build_header(option_tbl, data + datalen, len - datalen);

	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ORO, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_SERVERID, data + datalen, len - datalen);
	//datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_RECONF_ACCEPT, data + datalen, len - datalen);
	//datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_FQDN, data + datalen, len - datalen);
	//datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_ADDR, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_NA, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_PD, data + datalen, len - datalen);

	//DHCPV6C_OPT_PREF
	//DHCPV6C_OPT_ELAPSED
	return datalen;
}

int dhcpv6_packet_build_confirm(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len)
{
	int datalen = 0;
	struct dhcpv6c_packet *packet = data;
	datalen = dhcpv6_packet_init(packet, DHCPV6C_MSG_REQUEST);
	datalen += dhcpv6_packet_build_header(option_tbl, data + datalen, len - datalen);

	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ORO, data + datalen, len - datalen);
	//datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_SERVERID, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_RECONF_ACCEPT, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_FQDN, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_ADDR, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_NA, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_PD, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ORO, data + datalen, len - datalen);
	//DHCPV6C_OPT_ELAPSED
	return datalen;
}

int dhcpv6_packet_build_renew(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len)
{
	int datalen = 0;
	struct dhcpv6c_packet *packet = data;
	datalen = dhcpv6_packet_init(packet, DHCPV6C_MSG_RENEW);
	datalen += dhcpv6_packet_build_header(option_tbl, data + datalen, len - datalen);

	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ORO, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_SERVERID, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_RECONF_ACCEPT, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_FQDN, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_ADDR, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_NA, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_PD, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ORO, data + datalen, len - datalen);
	//DHCPV6C_OPT_ELAPSED
	return datalen;
}

int dhcpv6_packet_build_rebind(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len)
{
	int datalen = 0;
	struct dhcpv6c_packet *packet = data;
	datalen = dhcpv6_packet_init(packet, DHCPV6C_MSG_REBIND);
	datalen += dhcpv6_packet_build_header(option_tbl, data + datalen, len - datalen);

	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ORO, data + datalen, len - datalen);
	//datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_SERVERID, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_RECONF_ACCEPT, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_FQDN, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_ADDR, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_NA, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_PD, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ORO, data + datalen, len - datalen);
	//DHCPV6C_OPT_ELAPSED
	return datalen;
}


int dhcpv6_packet_build_release(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len)
{
	int datalen = 0;
	struct dhcpv6c_packet *packet = data;
	datalen = dhcpv6_packet_init(packet, DHCPV6C_MSG_RELEASE);
	datalen += dhcpv6_packet_build_header(option_tbl, data + datalen, len - datalen);

	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ORO, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_SERVERID, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_RECONF_ACCEPT, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_FQDN, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_ADDR, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_NA, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_PD, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ORO, data + datalen, len - datalen);
	//DHCPV6C_OPT_ELAPSED
	return datalen;
}

int dhcpv6_packet_build_decline(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len)
{
	int datalen = 0;
	struct dhcpv6c_packet *packet = data;
	datalen = dhcpv6_packet_init(packet, DHCPV6C_MSG_DECLINE);
	datalen += dhcpv6_packet_build_header(option_tbl, data + datalen, len - datalen);

	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ORO, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_SERVERID, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_RECONF_ACCEPT, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_FQDN, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_ADDR, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_NA, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_PD, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ORO, data + datalen, len - datalen);
	//DHCPV6C_OPT_ELAPSED
	return datalen;
}



int dhcpv6_packet_build_info_req(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len)
{
	int datalen = 0;
	struct dhcpv6c_packet *packet = data;
	datalen = dhcpv6_packet_init(packet, DHCPV6C_MSG_INFO_REQ);
	datalen += dhcpv6_packet_build_header(option_tbl, data + datalen, len - datalen);

	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ORO, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_SERVERID, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_RECONF_ACCEPT, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_FQDN, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_ADDR, data + datalen, len - datalen);
	//datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_NA, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_IA_PD, data + datalen, len - datalen);
	datalen += dhcpv6_packet_build(option_tbl, DHCPV6C_OPT_ORO, data + datalen, len - datalen);
	//DHCPV6C_OPT_ELAPSED
	return datalen;
}
/**************************************************************************************/
int dhcpv6_option_foreach_handle(struct dhcpv6c_interface *ifp, char *data, zpl_uint32 len)
{
	uint8_t *_o = NULL;
	uint8_t *end = ((uint8_t*)data) + len, *odata = NULL;
	uint16_t otype, olen = 0;
	dhcpv6c_for_each_option(data, end, otype, olen, odata)
	{
		switch(otype)
		{
			case DHCPV6C_OPT_SERVERID:
			if(olen)
			{
				memcpy(ifp->lease_v6.serv_duid, odata, olen);
				ifp->lease_v6.serv_duid_len = olen;
			}
			break;
			case DHCPV6C_OPT_STATUS:
			if(olen)
			{
				uint8_t *mdata = (olen > 2) ? &odata[2] : NULL;
				uint16_t mlen = (olen > 2) ? olen - 2 : 0;
				uint16_t code = ((int)odata[0]) << 8 | ((int)odata[1]);
				dhcpv6c_handle_status_code(ifp, ifp->current_state, code, mdata, mlen, &ret);
			}
			break;
			
			case DHCPV6C_OPT_IA_ADDR:
			if(olen)
			{
				struct dhcpv6c_ia_addr *iaaddr = (struct dhcpv6c_ia_addr *)(odata);
				memcpy(&ifp->lease_v6.lease_address, &iaaddr->addr, sizeof(struct ipstack_in6_addr));
				ifp->lease_v6.valid = ntohl(iaaddr->valid);
				ifp->lease_v6.preferred = ntohl(iaaddr->preferred);
			}
			break;
			case DHCPV6C_OPT_IA_PREFIX:
			if(olen)
			{
				struct dhcpv6c_ia_prefix *iaprefix = (struct dhcpv6c_ia_prefix *)(odata);
				memcpy(&ifp->lease_v6.lease_address, &iaprefix->addr, sizeof(struct ipstack_in6_addr));
				ifp->lease_v6.valid = ntohl(iaprefix->valid);
				ifp->lease_v6.preferred = ntohl(iaprefix->preferred);
				ifp->lease_v6.prefix = iaprefix->prefix;
			}
			break;
			case DHCPV6C_OPT_DNS_SERVERS:
			if(olen)
			{
				int addrs = olen>>4;
				struct ipstack_in6_addr *addr = (struct ipstack_in6_addr*)odata;
				if(addrs == 1)
				{
					memcpy(&ifp->lease_v6.lease_dns1, addr, sizeof(struct ipstack_in6_addr));
				}
				if(addrs == 2)
				{
					memcpy(&ifp->lease_v6.lease_dns1, addr, sizeof(struct ipstack_in6_addr));
					memcpy(&ifp->lease_v6.lease_dns2, addr, sizeof(struct ipstack_in6_addr));
				}
			}
			break;
			case DHCPV6C_OPT_DNS_DOMAIN:
			if(olen)
			{

			}	
			break;
			case DHCPV6C_OPT_PREF:
			if(olen)
			{
				ifp->lease_v6.preference = odata[0];
			}
			case DHCPV6C_OPT_UNICAST:
			if(olen)
			{
				struct ipstack_in6_addr *addr = (struct ipstack_in6_addr*)odata;
				memcpy(&ifp->lease_v6.server_address, addr, sizeof(struct ipstack_in6_addr));
			}
			case DHCPV6C_OPT_RECONF_ACCEPT:
			if(olen)
			{
				ifp->lease_v6.wants_reconfigure = true;
			}
			case DHCPV6C_OPT_SOL_MAX_RT:
			if(olen)
			{
				uint32_t *value = odata;
				ifp->lease_v6.sol_max_rt = ntohl(value);
			}
			case DHCPV6C_OPT_INF_MAX_RT:
			if(olen)
			{
				uint32_t *value = odata;
				ifp->lease_v6.inf_max_rt = ntohl(value);
			}
			case DHCPV6C_OPT_TZ_POSIX:
			if(olen)
			{
				memcpy(ifp->lease_v6.tz, odata, olen);
			}
			case DHCPV6C_OPT_TZ_NAME:
			if(olen)
			{
				memcpy(ifp->lease_v6.tz_name, odata, olen);
			}	
			case DHCPV6C_OPT_BOOT_URL:
			if(olen)
			{
				memcpy(ifp->lease_v6.bootfile_url, odata, olen);
			}	
			case DHCPV6C_OPT_BOOT_PARAM:
			if(olen)
			{
				memcpy(ifp->lease_v6.bootfile_param, odata, olen);
			}	
			case DHCPV6C_OPT_PXE_CONF_FILE:
			if(olen)
			{
				memcpy(ifp->lease_v6.pxeconffile, odata, olen);
			}	
			case DHCPV6C_OPT_PXE_PATH_PREFIX:
			if(olen)
			{
				memcpy(ifp->lease_v6.pxepathprefix, odata, olen);
			}	
		}
	}
	return 0;
}
/**************************************************************************************/
int dhcpv6_packet_recv_advert(struct dhcpv6c_interface *ifp, char *data, zpl_uint32 len)
{
	return dhcpv6_option_foreach_handle(ifp, data,  len);
}
/*
 * RFC 3315 18.1.8. Receipt of Reply Messages
 *
 * Upon the receipt of a valid Reply message in response to a Solicit
 * (with a Rapid Commit option), Request, Confirm, Renew, Rebind or
 * Information-request message, the client extracts the configuration
 * information contained in the Reply.  The client MAY choose to report
 * any status code or message from the status code option in the Reply
 * message.
 *
 * The client SHOULD perform duplicate address detection [17] on each of
 * the addresses in any IAs it receives in the Reply message before
 * using that address for traffic.  If any of the addresses are found to
 * be in use on the link, the client sends a Decline message to the
 * server as described in section 18.1.7.
 *
 * If the Reply was received in response to a Solicit (with a Rapid
 * Commit option), Request, Renew or Rebind message, the client updates
 * the information it has recorded about IAs from the IA options
 * contained in the Reply message:
 *
 * -  Record T1 and T2 times.
 *
 * -  Add any new addresses in the IA option to the IA as recorded by
 *    the client.
 *
 * -  Update lifetimes for any addresses in the IA option that the
 *    client already has recorded in the IA.
 *
 * -  Discard any addresses from the IA, as recorded by the client, that
 *    have a valid lifetime of 0 in the IA Address option.
 *
 * -  Leave unchanged any information about addresses the client has
 *    recorded in the IA but that were not included in the IA from the
 *    server.
 *
 * Management of the specific configuration information is detailed in
 * the definition of each option in section 22.
 *
 * If the client receives a Reply message with a Status Code containing
 * UnspecFail, the server is indicating that it was unable to process
 * the message due to an unspecified failure condition.  If the client
 * retransmits the original message to the same server to retry the
 * desired operation, the client MUST limit the rate at which it
 * retransmits the message and limit the duration of the time during
 * which it retransmits the message.
 *
 * When the client receives a Reply message with a Status Code option
 * with the value UseMulticast, the client records the receipt of the
 * message and sends subsequent messages to the server through the
 * interface on which the message was received using multicast.  The
 * client resends the original message using multicast.
 *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |          OPTION_IA_NA         |          option-len           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        IAID (4 octets)                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                              T1                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                              T2                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * .                         IA_NA-options                         .
 * .                                                               .
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |          OPTION_IAADDR        |          option-len           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * |                         IPv6 address                          |
 * |                                                               |
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      preferred-lifetime                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        valid-lifetime                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * .                                                               .
 * .                        IAaddr-options                         .
 * .                                                               .
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

int dhcpv6_packet_recv_reply(struct dhcpv6c_interface *ifp, char *data, zpl_uint32 len)
{
	return dhcpv6_option_foreach_handle(ifp, data,  len);
}


int dhcpv6_packet_recv_reconf(struct dhcpv6c_interface *ifp, char *data, zpl_uint32 len)
{
	return dhcpv6_option_foreach_handle(ifp, data,  len);
}

/**************************************************************************************/
int dhcpv6c_packet_socket(struct dhcpv6c_interface *ifp)
{
	ifindex_t kifindex = ifindex2ifkernel(ifp->ifindex);
	char *kifname = ifkernelindex2kernelifname(kifindex);
	ifp->sock = ipstack_socket(IPSTACK_IPCOM, IPSTACK_AF_INET6, IPSTACK_SOCK_DGRAM | SOCK_CLOEXEC, IPSTACK_IPPROTO_UDP);
	if (ipstack_invalid(ifp->sock))
		goto failure;

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
	if (sockopt_bindtodevice(ifp->sock, kifname) < 0)
		goto failure;

	struct ipstack_sockaddr_in6 client_addr = { .sin6_family = AF_INET6,
		.sin6_port = htons(ifp->client_port), .sin6_flowinfo = 0 };

	if (ipstack_bind(ifp->sock, (struct ipstack_sockaddr*)&client_addr, sizeof(client_addr)) < 0)
		goto failure;

	return 0;

failure:
	if (!ipstack_invalid(ifp->sock))
		ipstack_close(ifp->sock);

	return -1;
}
/********************************************************************************************************/
int dhcpv6c_packet_send(struct dhcpv6c_interface *ifp, enum dhcpv6c_msg type, char *data, uint16_t len)
{
	int ret = 0;
	struct ipstack_iovec iov[1] = {data, len};
	struct ipstack_sockaddr_in6 srv = {AF_INET6, htons(ifp->port),
		0, ALL_DHCPV6C_RELAYS, ifindex2ifkernel(ifp->ifindex)};
	struct ipstack_msghdr msg = {.msg_name = &srv, .msg_namelen = sizeof(srv),
			.msg_iov = iov, .msg_iovlen = 1};
	ret = ipstack_sendmsg(ifp->sock, &msg, 0);
	if (ret < 0) {
		char in6_str[INET6_ADDRSTRLEN];
		zlog_err(MODULE_DHCP, "Failed to send %s message to %s (%s)",
			dhcpv6c_msg_to_str(type),
			inet_ntop(AF_INET6, (const void *)&srv.sin6_addr,
				in6_str, sizeof(in6_str)), strerror(errno));
	}
	return ret;
}


// Message validation checks according to RFC3315 chapter 15
static bool dhcpv6c_response_check(struct dhcpv6c_interface *ifp, const void *buf, ssize_t len,
		const uint8_t transaction[3], int current_state,
		const struct ipstack_in6_addr *daddr)
{
	const struct dhcpv6c_header *rep = buf;

	if (len < (ssize_t)sizeof(*rep) || memcmp(rep->tr_id,
			transaction, sizeof(rep->tr_id)))
		return false; // Invalid reply

	if (current_state == DHCPV6C_MSG_SOLICIT) {
		if (rep->msg_type != DHCPV6C_MSG_ADVERT &&
				rep->msg_type != DHCPV6C_MSG_REPLY)
			return false;

	} else if (current_state == DHCPV6C_MSG_UNKNOWN) {
		if (!ifp->accept_reconfig || rep->msg_type != DHCPV6C_MSG_RECONF)
			return false;

	} else if (rep->msg_type != DHCPV6C_MSG_REPLY)
		return false;

	if (rep->msg_type == DHCPV6C_MSG_RECONF) {
		if (IN6_IS_ADDR_MULTICAST(daddr))
			return false;
	}
	return true;
}

int dhcpv6c_packet_recv(struct dhcpv6c_interface *ifp, ifindex_t *ifindex, 
	struct ipstack_sockaddr_in6 *addr, char *data, uint16_t len)
{
	uint8_t rc = 0;
	zpl_uint32 d_ifindex = 0;
	const struct dhcpv6c_header *rep = data;
	struct ipstack_iovec iov;
	/* Header and data both require alignment. */
	uint8_t buff[CMSG_SPACE(sizeof(struct ipstack_in6_pktinfo))];
	struct ipstack_msghdr msgh;
	memset(&msgh, 0, sizeof(struct ipstack_msghdr));
	msgh.msg_name = addr;
	msgh.msg_namelen = sizeof(struct ipstack_sockaddr_in6);
	msgh.msg_iov = &iov;
	msgh.msg_iovlen = 1;
	msgh.msg_control = (caddr_t) buff;
	msgh.msg_controllen = sizeof(buff);
	iov.iov_base = data;
	iov.iov_len = len;

	// Receive cycle
	len = ipstack_recvmsg(ifp->sock, &msgh, 0);
	if (len < 0)
	{
		zlog_err(MODULE_DHCP, "packet read error, ignoring");
		return len; /* returns -1 */
	}
	if (addr->sin6_family != IPSTACK_AF_INET6) {
		zlog_err(MODULE_DHCP, "received dhcpv6 message is not IPSTACK_AF_INET");
		return ERROR;
	}
	d_ifindex = getsockopt_ifindex(IPSTACK_AF_INET6, &msgh);
	if (ifindex)
		*ifindex = d_ifindex;
	
	/*
	if (len < offsetof(struct dhcp_packet, options)
			|| packet->cookie != htonl(DHCP_MAGIC))
	{
		zlog_err(MODULE_DHCP, "packet with bad magic, ignoring");
		return -2;
	}
	*/
	if (DHCPC_DEBUG_ISON(RECV))
	{
		if(DHCPC_DEBUG_ISON(DETAIL))
		{
			zlog_debug(MODULE_DHCP," dhcpv6 client received packet(%d byte) on interface %s", len, ifindex2ifname(d_ifindex));
			//dhcp_packet_dump(packet);
		}
		else
			zlog_debug(MODULE_DHCP," dhcpv6 client received packet(%d byte) on interface %s", len, ifindex2ifname(d_ifindex));
	}
	dhcpv6c_response_check(ifp, data,  len,
		rep->tr_id, ifp->current_state,
		addr);
	return len;
}




#endif /*ZPL_DHCPV6C_MODULE*/