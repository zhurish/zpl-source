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

#ifdef ZPL_DHCPV6C_MODULE
#include "dhcpv6c.h"
#include "dhcpv6c_option.h"
#include "dhcpv6c_state.h"

struct dhcpv6c_option_flags {
	uint16_t code;
	uint16_t flags;
	const char *str;
};

static struct dhcpv6c_option_flags dhcpv6c_opt_flags[] = {
	{ .code = DHCPV6C_OPT_CLIENTID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_SERVERID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_IA_NA, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str= NULL },
	{ .code = DHCPV6C_OPT_IA_TA, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_IA_ADDR, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_ORO, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6C_OPT_PREF, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_ELAPSED, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6C_OPT_RELAY_MSG, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6C_OPT_AUTH, .flags = OPT_U8 | OPT_NO_PASSTHRU, .str = "authentication" },
	{ .code = DHCPV6C_OPT_UNICAST, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_STATUS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_RAPID_COMMIT, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6C_OPT_USER_CLASS, .flags = OPT_USER_CLASS | OPT_ARRAY, .str = "userclass" },
	{ .code = DHCPV6C_OPT_VENDOR_CLASS, .flags = OPT_U8, .str = "vendorclass" },
	{ .code = DHCPV6C_OPT_INTERFACE_ID, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6C_OPT_RECONF_MESSAGE, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6C_OPT_RECONF_ACCEPT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_SIP_SERVER_D, .flags = OPT_DNS_STR | OPT_ORO, .str = "sipserver_d" },
	{ .code = DHCPV6C_OPT_SIP_SERVER_A, .flags = OPT_IP6 | OPT_ARRAY | OPT_ORO, .str = "sipserver_a" },
	{ .code = DHCPV6C_OPT_DNS_SERVERS, .flags = OPT_IP6 | OPT_ARRAY | OPT_ORO, .str = "dns" },
	{ .code = DHCPV6C_OPT_DNS_DOMAIN, .flags = OPT_DNS_STR | OPT_ORO, .str = "search" },
	{ .code = DHCPV6C_OPT_IA_PD, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_IA_PREFIX, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6C_OPT_SNTP_SERVERS, .flags = OPT_IP6 | OPT_ARRAY | OPT_ORO, .str = "sntpservers" },
	{ .code = DHCPV6C_OPT_INFO_REFRESH, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO | OPT_ORO_STATELESS, .str = NULL },
	{ .code = DHCPV6C_OPT_REMOTE_ID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_SUBSCRIBER_ID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_FQDN, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO, .str = NULL },
	{ .code = DHCPV6C_OPT_TZ_POSIX, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_TZ_NAME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },	
	{ .code = DHCPV6C_OPT_ERO, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_LQ_QUERY, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_CLIENT_DATA, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_CLT_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_LQ_RELAY_DATA, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_LQ_CLIENT_LINK, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_RELAY_ID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_NTP_SERVER, .flags = OPT_U8 | OPT_ORO, .str = "ntpserver" },
	{ .code = DHCPV6C_OPT_BOOT_URL, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_BOOT_PARAM, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ARRAY, .str = NULL },
	{ .code = DHCPV6C_OPT_CLIENT_ARCH_TYPE, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_AFTR_NAME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO, .str = NULL },
	{ .code = DHCPV6C_OPT_RSOO, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_PD_EXCLUDE, .flags = OPT_INTERNAL | OPT_ORO | OPT_ORO_STATEFUL, .str = NULL },
	{ .code = DHCPV6C_OPT_VSS, .flags = OPT_U8, .str = "vss" },
	{ .code = DHCPV6C_OPT_LINK_LAYER_ADDRESS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_LINK_ADDRESS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_RADIUS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_SOL_MAX_RT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO | OPT_ORO_SOLICIT, .str = NULL },
	{ .code = DHCPV6C_OPT_INF_MAX_RT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO | OPT_ORO_STATELESS, .str = NULL },
#ifdef EXT_CER_ID
	{ .code = DHCPV6C_OPT_CER_ID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
#endif
	{ .code = DHCPV6C_OPT_DHCPV4_MSG, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_S46_RULE, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_S46_BR, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_S46_DMR, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_S46_V4V6BIND, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_S46_PORTPARAMS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_S46_CONT_MAPE, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO, .str = NULL },
	{ .code = DHCPV6C_OPT_S46_CONT_MAPT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO, .str = NULL },
	{ .code = DHCPV6C_OPT_S46_CONT_LW, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO, .str = NULL },
	{ .code = DHCPV6C_OPT_LQ_BASE_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_LQ_START_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_LQ_END_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_ANI_ATT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_ANI_NETWORK_NAME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_ANI_AP_NAME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_ANI_AP_BSSID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_ANI_OPERATOR_ID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_ANI_OPERATOR_REALM, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_MUD_URL_V6, .flags = OPT_STR | OPT_NO_PASSTHRU, .str = "mud_url_v6" },
	{ .code = DHCPV6C_OPT_F_BINDING_STATUS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_CONNECT_FLAGS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_DNS_REMOVAL_INFO, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_DNS_HOST_NAME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_DNS_ZONE_NAME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_DNS_FLAGS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_EXPIRATION_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_MAX_UNACKED_BNDUPD, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_MCLT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_PARTNER_LIFETIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_PARTNER_LIFETIME_SENT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_PARTNER_DOWN_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_PARTNER_RAW_CLT_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_PROTOCOL_VERSION, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_KEEPALIVE_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_RECONFIGURE_DATA, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_RELATIONSHIP_NAME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_SERVER_FLAGS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_SERVER_STATE, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_START_TIME_OF_STATE, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_F_STATE_EXPIRATION_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6C_OPT_RELAY_PORT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = 0, .flags = 0, .str = NULL },
};



static int dhcpv6_option_idx(zpl_uint16 code)
{
	zpl_uint32 i = 0;
	for (i = 0; i < sizeof(dhcpv6c_opt_flags) / sizeof(dhcpv6c_opt_flags[0]); i++)
	{
		if (dhcpv6c_opt_flags[i].code == code)
			return i;
	}
	return -1;
}

int dhcpv6_option_flags(zpl_uint16 code)
{
	zpl_uint32 i = 0;
	for (i = 0; i < sizeof(dhcpv6c_opt_flags) / sizeof(dhcpv6c_opt_flags[0]); i++)
	{
		if (dhcpv6c_opt_flags[i].code == code)
			return dhcpv6c_opt_flags[i].flags;
	}
	return 0;
}

static int dhcpv6_option_strget(const char *const_str, char *out_str)
{
	zpl_uint32 len = 0;
	char *p = const_str;
	if(!const_str)
		return 0;
	char *val = strstr(p, ",");
	if(val)
	{
		len = val - p;
		strncpy(out_str, p, len);
		return len;
	}
	else
	{
		len = strlen(p);
		strncpy(out_str, p, len);
		return len;
	}
	return 0;
}



int dhcpv6_option_string_set(dhcpv6_option_set_t *option_tbl, zpl_uint16 code,
		const char *const_str)
{
	struct dhcpv6c_option_flags *optflag = NULL;
	char str[254], *p = NULL;
	zpl_uint32 retval = 0, length = 0, optlen = 0, offset = 0;
	zpl_uint8 *val8 = NULL, buffer[512], optindex = 0;
	zpl_uint16 *val16 = NULL;
	zpl_uint32  *val32 = NULL;
	if(!option_tbl || !const_str)
		return 0;
	val8 = buffer;
	val16 = buffer;
	val32 = buffer;
	p = (char *) const_str;
	optindex = dhcpv6_option_idx(code);
	if (optindex == -1)
		return 0;
	optflag = &dhcpv6c_opt_flags[optindex];

	memset(buffer, 0, sizeof(buffer));
	do
	{
		memset(str, '\0', sizeof(str));
		if(p)
		{
			if(retval)
				p += retval + 1;
		}
		else
		{
			return dhcpv6_option_add(option_tbl, code, buffer, length);
		}
		if(p)
			retval = dhcpv6_option_strget(p, str);
		if(retval == 0)
		{
			return dhcpv6_option_add(option_tbl, code, buffer, length);
		}
		offset = length;

		switch (optflag->flags & OPT_MASK_SIZE)
		{
		case OPT_IP6:
			udhcp_str2nip(str, buffer + offset);
			length += 4;
			break;
		case OPT_STR:
		case OPT_USER_CLASS:
			optlen = strnlen(str, 254);
			strncpy(buffer + offset, str, optlen);
			length += optlen;
			break;
	#if DHCP_ENABLE_RFC3397
		case OPT_DNS_STR:
			p = dname_enc(NULL, 0, str, &optlen);
			if(p)
			{
				strncpy(buffer + offset, p, optlen);
				length += optlen;
			}
	#endif
			break;
		case OPT_U8:
			buffer[offset] = strtoul(str, NULL, 10);
			length += 1;
			break;
			/* htonX are macros in older libc's, using temp var
			 * in code below for safety */
			/* TODO: use bb_strtoX? */
#if 0			
		case DHCP_OPTION_TYPE_U16:
		case DHCP_OPTION_TYPE_S16:
		{
			val16 = buffer + offset;
			zpl_uint32  tmp = strtoul(str, NULL, 0);
			*val16 = htons(tmp);
			length += 2;
			break;
		}
		case DHCP_OPTION_TYPE_U32:
		case DHCP_OPTION_TYPE_S32:
		{
			val32 = buffer + offset;
			zpl_uint32  tmp = strtoul(str, NULL, 0);
			*val32 = htonl(tmp);
			length += 4;
			break;
		}
		case DHCP_OPTION_TYPE_BOOLEAN:
		{
			if(strstr(str, "no"))
				buffer[offset] = 0;
			else if(strstr(str, "yes"))
				buffer[offset] = 1;
			length += 1;
		}
		break;
		case DHCP_OPTION_TYPE_BIN: /* handled in attach_option() */
			buffer[offset] = str[0];
			length += 1;
			break;
		case DHCP_OPTION_TYPE_STATIC_ROUTES:
		{
			/* Input: "a.b.c.d a.b.c.d" */
			/* Output: mask(1 byte),pfx(0-4 bytes),gw(4 bytes) */
			//unsigned mask;
			char *slash = strchr(str, ' ');
			if (slash)
			{
				*slash = '\0';
				retval = udhcp_str2nip(str, buffer);
				retval = udhcp_str2nip(slash + 1, buffer + 4);
			}
			length += 8;
			break;
		}
		case DHCP_OPTION_TYPE_6RD: /* handled in attach_option() */
			strncpy(buffer + offset, str, optlen);
			length += 12;
			break;
	#if DHCP_ENABLE_RFC3397
		case DHCP_OPTION_TYPE_SIP_SERVERS:
			optlen = strnlen(str, 254);
			strncpy(buffer + offset, str, optlen);
			length += optlen;
			break;
	#endif
	#endif
		default:
			break;
		}
	}while(1);

	return 0;
}


int dhcpv6_option_add(dhcpv6_option_set_t *option_tbl, zpl_uint16 code, const zpl_uint8 *opt, zpl_uint32 len)
{
	zpl_uint32 inlen = len;
	zpl_uint32 type = 0;
	if(code > DHCPV6C_OPT_MAX)
		return ERROR;
	if(!option_tbl)
		return ERROR;
	if(code == DHCPV6C_OPT_CLIENTID)
	{
		if(len == 6)
		{
			inlen += 1;
			type = 1;//DHCP_OPTION_61_MAC;
		}
		else if(len > 6 && len <= 36)
		{
			inlen += 1;
			type = 2;//DHCP_OPTION_61_UUID;
		}
		else// if(len == 6)
		{
			inlen += 1;
			type = 0xff;//DHCP_OPTION_61_IAID;
		}
	}
	if(inlen && opt)
	{
		option_tbl[code].data = malloc(inlen);
		if(option_tbl[code].data)
		{
			memset(option_tbl[code].data, 0, inlen);
			if(code == DHCPV6C_OPT_CLIENTID)
			{
				memcpy(option_tbl[code].data + 1, opt, inlen - 1);
				option_tbl[code].data[0] = type;
			}
			else
				memcpy(option_tbl[code].data, opt, inlen);
			option_tbl[code].code = code;
			option_tbl[code].len = inlen;
			return OK;
		}
	}
	else
	{
		option_tbl[code].code = code;
		option_tbl[code].len = inlen;
		return OK;
	}
	return ERROR;
}

int dhcpv6_option_add_hex(dhcpv6_option_set_t *option_tbl, zpl_uint16 code, const zpl_uint32  value, zpl_uint32 len)
{
	zpl_uint32  invalue32 = value;
	zpl_uint16 invalue16 = value;
	zpl_uint8 invalue8 = value;
	if(code > DHCPV6C_OPT_MAX)
		return ERROR;
	if(!option_tbl)
		return ERROR;
	if(len)
	{	
		option_tbl[code].data = malloc(len);
		if(option_tbl[code].data)
		{
			memset(option_tbl[code].data, 0, len);
			if(len == 1)
				memcpy(option_tbl[code].data, &invalue8, len);
			else if(len == 2)
				memcpy(option_tbl[code].data, &invalue16, len);
			else if(len == 4)
				memcpy(option_tbl[code].data, &invalue32, len);
			option_tbl[code].code = code;
			option_tbl[code].len = len;
			return OK;
		}
	}
	else
	{
		option_tbl[code].code = code;
		option_tbl[code].len = len;
		return OK;
	}
	return ERROR;
}

int dhcpv6_option_del(dhcpv6_option_set_t *option_tbl, zpl_uint16 code)
{
	if(code > DHCPV6C_OPT_MAX)
		return ERROR;
	if(!option_tbl)
		return ERROR;
	if(option_tbl[code].len)
	{
		if(option_tbl[code].data)
			free(option_tbl[code].data);
		option_tbl[code].data = NULL;
		option_tbl[code].code = 0;
		option_tbl[code].len = 0;
		return OK;
	}
	return ERROR;
}

int dhcpv6_option_clean(dhcpv6_option_set_t *option_tbl)
{
	zpl_uint32 code = 0;
	if(!option_tbl)
		return ERROR;
	for(code = 0; code < DHCPV6C_OPT_MAX; code++)
	{
		if(option_tbl[code].data)
			free(option_tbl[code].data);
		option_tbl[code].data = NULL;
		option_tbl[code].code = 0;
		option_tbl[code].len = 0;
	}
	return OK;
}

int dhcpv6_option_lookup(dhcpv6_option_set_t *option_tbl, zpl_uint16 code)
{
	if(code > DHCPV6C_OPT_MAX)
		return ERROR;
	if(!option_tbl)
		return ERROR;
	if(option_tbl[code].len && option_tbl[code].data)
	{
		return OK;
	}
	if(option_tbl[code].code == code)
	{
		return OK;
	}
	return ERROR;
}

dhcpv6_option_set_t *dhcpv6_option_find(dhcpv6_option_set_t *option_tbl, const zpl_uint16 code)
{
	if(code > DHCPV6C_OPT_MAX)
		return NULL;
	if(!option_tbl)
		return NULL;
	if(option_tbl[code].len && option_tbl[code].data)
	{
		return &option_tbl[code];
	}
	return NULL;
}


int dhcpv6_option_default(dhcpv6_option_set_t *option_tbl)
{	
	dhcpv6_option_add(option_tbl, DHCPV6C_OPT_RECONF_ACCEPT, NULL, 0);
	dhcpv6_option_add(option_tbl, DHCPV6C_OPT_FQDN, NULL, 0);
	///if (type == DHCPV6C_MSG_SOLICIT)
	dhcpv6_option_add(option_tbl, DHCPV6C_OPT_IA_PD, NULL, 0);
	dhcpv6_option_add(option_tbl, DHCPV6C_OPT_IA_PREFIX, NULL, 0);

	dhcpv6_option_add(option_tbl, DHCPV6C_OPT_IA_NA, NULL, 0);
	dhcpv6_option_add(option_tbl, DHCPV6C_OPT_IA_ADDR, NULL, 0);
	dhcpv6_option_add(option_tbl, DHCPV6C_OPT_ELAPSED, NULL, 0);

	
	return OK;
}

/*********************************************************************************/
/*********************************************************************************/
zpl_uint32 dhcpv6_option_get_length(char *data)
{
	zpl_uint32 offset = 0;
	dhcpv6_option_hdr_t *msg = (dhcpv6_option_hdr_t *)(data + offset);	
	if(!msg)
		return 0;
	while (ntohs(msg->code) != 0xff) {
		if (ntohs(msg->code) != 0x00)
			offset += ntohs(msg->len) + DHCPV6_OPT_DATA;
		offset++;
		msg = (dhcpv6_option_hdr_t *)(data + offset);	
	}
	return offset;
}

/* Return the position of the 'end' option (no bounds checking) */
int dhcpv6_end_option(zpl_uint8 *optionptr)
{
	zpl_uint32 offset = 0;
	dhcpv6_option_hdr_t *msg = (dhcpv6_option_hdr_t *)(optionptr + offset);	
	if(!msg)
		return 0;
	while (ntohs(msg->code) != 0xff) {
		if (ntohs(msg->code) != 0x00)
			offset += ntohs(msg->len) + DHCPV6_OPT_DATA;
		offset++;
		msg = (dhcpv6_option_hdr_t *)(optionptr + offset);	
	}
	return offset;
}

static int dhcp_option_packet_end(char *data, zpl_uint32 len)
{
	if(!data)
		return 0;
	zpl_uint32 offset = 0;
	dhcpv6_option_hdr_t *msg = (dhcpv6_option_hdr_t *)(data + offset);
	msg->code = htons(0xff);
	return 1;
}


zpl_uint8 * dhcpv6_option_get(char *data, zpl_uint32 len, zpl_uint16 code, zpl_uint8 *optlen)
{
	zpl_uint32 /*i = 0, */offset = 0;
	if(!data)
		return NULL;
	dhcpv6_option_hdr_t *msg = (dhcpv6_option_hdr_t *)data;
	if(ntohs(msg->code) == code)
	{
		if(optlen)
			*optlen = ntohs(msg->len);
		return msg->val.pval;
	}
	offset = ntohs(msg->len) + DHCPV6_OPT_DATA;
	while(1)
	{
		msg = (dhcpv6_option_hdr_t *)(data + offset);
		if(ntohs(msg->code) == code)
		{
			if(optlen)
				*optlen = ntohs(msg->len);
			return msg->val.pval;//msg->data;
		}
		else if(ntohs(msg->code) == 0xff)
		{
			return NULL;
		}
		offset += ntohs(msg->len) + DHCPV6_OPT_DATA;

		if(offset >= len)
			return NULL;
	}
	return NULL;
}

int dhcpv6_option_get_simple(const char *data, zpl_uint32 *output, zpl_uint16 code, zpl_uint8 optlen)
{
	zpl_uint32 /*i = 0, */offset = 0;
	if(!data)
		return ERROR;
	zpl_uint32 len = dhcpv6_option_get_length(data);
	if(!data || len < 0)
		return ERROR;
	dhcpv6_option_hdr_t *msg = (dhcpv6_option_hdr_t *)data;

	if(ntohs(msg->code) == code)
	{
		if(optlen == 1 && output)
			*output = msg->val.val8;
		if(optlen == 2 && output)
			*output = msg->val.val16;
		if(optlen == 4 && output)
			*output = msg->val.val32;
		if(optlen == 16 && output)
			memcpy(output, msg->val.pval, 16);
		return OK;
	}
	offset = ntohs(msg->len) + DHCPV6_OPT_DATA;
	while(1)
	{
		msg = (dhcpv6_option_hdr_t *)(data + offset);
		if(ntohs(msg->code) == code)
		{
			if(optlen == 1 && output)
				*output = msg->val.val8;
			if(optlen == 2 && output)
				*output = msg->val.val16;
			if(optlen == 4 && output)
				*output = msg->val.val32;
			if(optlen == 16 && output)
				memcpy(output, msg->val.pval, 16);
			return OK;
		}
		else if(ntohs(msg->code) == 0xff)
		{
			return ERROR;
		}
		offset += ntohs(msg->len) + DHCPV6_OPT_DATA;
		if(offset >= len)
			return ERROR;
	}
	return ERROR;
}




#endif /*ZPL_DHCPV6C_MODULE*/