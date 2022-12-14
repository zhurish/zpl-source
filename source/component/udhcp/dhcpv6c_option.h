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

#ifndef __DHCPV6C_OPTION_H__
#define __DHCPV6C_OPTION_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_DHCPV6C_MODULE


enum dhcvp6_opt {
	DHCPV6C_OPT_CLIENTID = 1,
	DHCPV6C_OPT_SERVERID = 2,
	DHCPV6C_OPT_IA_NA = 3,//向server请求该ipv6地址
	DHCPV6C_OPT_IA_TA = 4,//client向server请求临时地址
	DHCPV6C_OPT_IA_ADDR = 5,
	DHCPV6C_OPT_ORO = 6,//Option Request Option(option 6) //封装需要请求的option的code
	DHCPV6C_OPT_PREF = 7,
	DHCPV6C_OPT_ELAPSED = 8,
	DHCPV6C_OPT_RELAY_MSG = 9,
	DHCPV6C_OPT_AUTH = 11,
	DHCPV6C_OPT_UNICAST = 12,
	DHCPV6C_OPT_STATUS = 13,
	DHCPV6C_OPT_RAPID_COMMIT = 14,
	DHCPV6C_OPT_USER_CLASS = 15,
	DHCPV6C_OPT_VENDOR_CLASS = 16,
	DHCPV6C_OPT_VENDOR_OPTS = 17,
	DHCPV6C_OPT_INTERFACE_ID = 18,
	DHCPV6C_OPT_RECONF_MESSAGE = 19,
	DHCPV6C_OPT_RECONF_ACCEPT = 20,
	DHCPV6C_OPT_SIP_SERVER_D = 21,
	DHCPV6C_OPT_SIP_SERVER_A = 22,
	DHCPV6C_OPT_DNS_SERVERS = 23,
	DHCPV6C_OPT_DNS_DOMAIN = 24,
	DHCPV6C_OPT_IA_PD = 25,
	DHCPV6C_OPT_IA_PREFIX = 26,//表示client请求server分配前缀信息
	DHCPV6C_OPT_SNTP_SERVERS = 31,
	DHCPV6C_OPT_INFO_REFRESH = 32,
	DHCPV6C_OPT_REMOTE_ID = 37,
	DHCPV6C_OPT_SUBSCRIBER_ID = 38,
	DHCPV6C_OPT_FQDN = 39,
	DHCPV6C_OPT_TZ_POSIX = 41,
	DHCPV6C_OPT_TZ_NAME = 42,	
	DHCPV6C_OPT_ERO = 43,
	DHCPV6C_OPT_LQ_QUERY = 44,
	DHCPV6C_OPT_CLIENT_DATA = 45,
	DHCPV6C_OPT_CLT_TIME = 46,
	DHCPV6C_OPT_LQ_RELAY_DATA = 47,
	DHCPV6C_OPT_LQ_CLIENT_LINK = 48,
	DHCPV6C_OPT_RELAY_ID = 53,
	DHCPV6C_OPT_NTP_SERVER = 56,
	DHCPV6C_OPT_BOOT_URL    =  59,
	DHCPV6C_OPT_BOOT_PARAM  =  60,	
	DHCPV6C_OPT_CLIENT_ARCH_TYPE = 61,
	DHCPV6C_OPT_AFTR_NAME = 64,
	DHCPV6C_OPT_RSOO = 66,
	DHCPV6C_OPT_PD_EXCLUDE = 67,
	DHCPV6C_OPT_VSS = 68,
	DHCPV6C_OPT_LINK_LAYER_ADDRESS = 79,
	DHCPV6C_OPT_LINK_ADDRESS = 80,
	DHCPV6C_OPT_RADIUS = 81,
	DHCPV6C_OPT_SOL_MAX_RT = 82,
	DHCPV6C_OPT_INF_MAX_RT = 83,
#ifdef EXT_CER_ID
	/* draft-donley-dhc-cer-id-option-03 */
	DHCPV6C_OPT_CER_ID = EXT_CER_ID,
#endif
	DHCPV6C_OPT_DHCPV4_MSG = 87,

	DHCPV6C_OPT_PXE_CONF_FILE = 209,
	DHCPV6C_OPT_PXE_PATH_PREFIX = 210,
	/* draft-ietf-softwire-map-dhcp-08 */
	DHCPV6C_OPT_S46_RULE = 89,
	DHCPV6C_OPT_S46_BR = 90,
	DHCPV6C_OPT_S46_DMR = 91,
	DHCPV6C_OPT_S46_V4V6BIND = 92,
	DHCPV6C_OPT_S46_PORTPARAMS = 93,
	DHCPV6C_OPT_S46_CONT_MAPE = 94,
	DHCPV6C_OPT_S46_CONT_MAPT = 95,
	DHCPV6C_OPT_S46_CONT_LW = 96,
	DHCPV6C_OPT_LQ_BASE_TIME = 100,
	DHCPV6C_OPT_LQ_START_TIME = 101,
	DHCPV6C_OPT_LQ_END_TIME = 102,
	DHCPV6C_OPT_ANI_ATT = 105,
	DHCPV6C_OPT_ANI_NETWORK_NAME = 106,
	DHCPV6C_OPT_ANI_AP_NAME = 107,
	DHCPV6C_OPT_ANI_AP_BSSID = 108,
	DHCPV6C_OPT_ANI_OPERATOR_ID = 109,
	DHCPV6C_OPT_ANI_OPERATOR_REALM = 110,
	DHCPV6C_OPT_MUD_URL_V6 = 112,
	DHCPV6C_OPT_F_BINDING_STATUS = 114,
	DHCPV6C_OPT_F_CONNECT_FLAGS = 115,
	DHCPV6C_OPT_F_DNS_REMOVAL_INFO = 116,
	DHCPV6C_OPT_F_DNS_HOST_NAME = 117,
	DHCPV6C_OPT_F_DNS_ZONE_NAME = 118,
	DHCPV6C_OPT_F_DNS_FLAGS = 119,
	DHCPV6C_OPT_F_EXPIRATION_TIME = 120,
	DHCPV6C_OPT_F_MAX_UNACKED_BNDUPD = 121,
	DHCPV6C_OPT_F_MCLT = 122,
	DHCPV6C_OPT_F_PARTNER_LIFETIME = 123,
	DHCPV6C_OPT_F_PARTNER_LIFETIME_SENT = 124,
	DHCPV6C_OPT_F_PARTNER_DOWN_TIME = 125,
	DHCPV6C_OPT_F_PARTNER_RAW_CLT_TIME = 126,
	DHCPV6C_OPT_F_PROTOCOL_VERSION = 127,
	DHCPV6C_OPT_F_KEEPALIVE_TIME = 128,
	DHCPV6C_OPT_F_RECONFIGURE_DATA = 129,
	DHCPV6C_OPT_F_RELATIONSHIP_NAME = 130,
	DHCPV6C_OPT_F_SERVER_FLAGS = 131,
	DHCPV6C_OPT_F_SERVER_STATE = 132,
	DHCPV6C_OPT_F_START_TIME_OF_STATE = 133,
	DHCPV6C_OPT_F_STATE_EXPIRATION_TIME  = 134,
	DHCPV6C_OPT_RELAY_PORT = 135,

	DHCPV6C_OPT_MAX,
};


enum dhcpv6c_opt_flags {
	OPT_U8 = 0,
	OPT_IP6,
	OPT_STR,
	OPT_DNS_STR,
	OPT_USER_CLASS,
	OPT_MASK_SIZE = 0x0F,
	OPT_ARRAY = 0x10,
	OPT_INTERNAL = 0x20,
	OPT_NO_PASSTHRU = 0x40,
	OPT_ORO = 0x80,
	OPT_ORO_STATEFUL = 0x100,
	OPT_ORO_STATELESS = 0x200,
	OPT_ORO_SOLICIT = 0x400
};

#define DHCPV6_OPT_DATA 4

#pragma pack(1)
typedef struct dhcpv6_option_set {
	zpl_uint16 code;
	zpl_uint16 len;
	zpl_uint8 *data;
}dhcpv6_option_set_t;

typedef struct dhcpv6_option_hdr {
	zpl_uint16 code;
	zpl_uint16 len;
	union
	{
		zpl_uint8 	pval[1];
		zpl_uint8 	val8;
		zpl_uint16 	val16;
		zpl_uint32 	val32;
	}val;
}dhcpv6_option_hdr_t;
#pragma pack(0)

extern int dhcpv6_option_flags(zpl_uint16 code);

/*
 * for dhcp pool option handle
 */
extern int dhcpv6_option_default(dhcpv6_option_set_t *option_tbl);
extern int dhcpv6_option_add(dhcpv6_option_set_t *option_tbl, zpl_uint16 code, const zpl_uint8 *opt, zpl_uint32 len);
extern int dhcpv6_option_add_hex(dhcpv6_option_set_t *option_tbl, zpl_uint16 code, const zpl_uint32  value, zpl_uint32 len);
extern int dhcpv6_option_del(dhcpv6_option_set_t *option_tbl, zpl_uint16 code);
extern int dhcpv6_option_clean(dhcpv6_option_set_t *option_tbl);
extern int dhcpv6_option_lookup(dhcpv6_option_set_t *option_tbl, zpl_uint16 code);
extern int dhcpv6_option_string_set(dhcpv6_option_set_t *option_tbl, zpl_uint16 code,
		const char *const_str);

extern dhcpv6_option_set_t *dhcpv6_option_find(dhcpv6_option_set_t *option_tbl, const zpl_uint16 code);


extern zpl_uint32 dhcpv6_option_get_length(char *data);
extern int dhcpv6_end_option(zpl_uint8 *optionptr);

extern zpl_uint8 * dhcpv6_option_get(char *data, zpl_uint32 len, zpl_uint16 code, zpl_uint8 *optlen);
extern int dhcpv6_option_get_simple(const char *data, zpl_uint32 *output, zpl_uint16 code, zpl_uint8 optlen);

#define dhcpv6_option_add_8bit(t, c, v)			dhcpv6_option_add_hex(t, c, v, 1)
#define dhcpv6_option_add_16bit(t, c, v)		dhcpv6_option_add_hex(t, c, v, 2)
#define dhcpv6_option_add_32bit(t, c, v)		dhcpv6_option_add_hex(t, c, v, 4)
#define dhcpv6_option_add_address(t, c, v)		dhcpv6_option_add(t, c, v, 16)
#define dhcpv6_option_add_string(t, c, v, l)	dhcpv6_option_add(t, c, v, l)

#define dhcpv6_option_get_8bit(d, c, v)			dhcpv6_option_get_simple(d, v, c, 1)
#define dhcpv6_option_get_16bit(d, c, v)		dhcpv6_option_get_simple(d, v, c, 2)
#define dhcpv6_option_get_32bit(d, c, v)		dhcpv6_option_get_simple(d, v, c, 4)
#define dhcpv6_option_get_address(d, c, v)		dhcpv6_option_get_simple(d, v, c, 16)


#endif /*ZPL_DHCPV6C_MODULE*/

#ifdef __cplusplus
}
#endif

#endif /* __DHCPV6C_OPTION_H__ */