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

#ifndef __DHCPV6C_PACKET_H__
#define __DHCPV6C_PACKET_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_DHCPV6C_MODULE

#define _unused __attribute__((unused))
#define _packed __attribute__((packed))
#define _aligned(n) __attribute__((aligned(n)))


// DHCPv6 Protocol Headers
struct dhcpv6c_header {
	uint8_t msg_type;
	uint8_t tr_id[3];
} __attribute__((packed));

//DHCPV6C_OPT_IA_NA 头
//IA_NA sub-option有IA Address Option和Status Code Option
//IA_TA sub-option有IA Address Option和Status Code Option
struct dhcpv6c_ia_hdr {
	uint16_t type;
	uint16_t len;
	uint32_t iaid;
	uint32_t t1;
	uint32_t t2;
} _packed;

//IA Address Option(option 5)
struct dhcpv6c_ia_addr {
	uint16_t type;
	uint16_t len;
	struct in6_addr addr;
	uint32_t preferred;
	uint32_t valid;
} _packed;

struct dhcpv6c_ia_prefix {
	uint16_t type;
	uint16_t len;
	uint32_t preferred;
	uint32_t valid;
	uint8_t prefix;
	struct in6_addr addr;
} _packed;

enum dhcpv6_duid_mode {
	DUID_MODE_LLT = 1,
	DUID_MODE_EN,
	DUID_MODE_LL,
	DUID_MODE_NORMAL
};
//链路层地址(MAC地址)+时间(DUID_LLT)
struct dhcpv6_duid_llt  {
	uint16_t hardware_type;
	uint32_t timespec;
	uint8_t llmac[6];
} _packed;

//厂商的唯一ID标识(DUID_EN)
struct dhcpv6_duid_en  {
	uint32_t enterprise;
	uint8_t identifier[128];
} _packed;

//链路层地址(DUID_LL)
struct dhcpv6_duid_ll  {
	uint16_t hardware_type;
	uint8_t llmac[6];
} _packed;


struct dhcpv6_duid_normal  {
	uint8_t uuid[16];
} _packed;

struct dhcpv6_duid {
	uint16_t type;
	union 
	{
		struct dhcpv6_duid_llt duid_llt;
		struct dhcpv6_duid_en duid_en;
		struct dhcpv6_duid_ll duid_ll;
		struct dhcpv6_duid_normal normal;
	}duid;
} _packed;



struct dhcpv6c_auth_reconfigure {
	uint16_t type;
	uint16_t len;
	uint8_t protocol;
	uint8_t algorithm;
	uint8_t rdm;
	uint64_t replay;
	uint8_t reconf_type;
	uint8_t key[16];
} _packed;

struct dhcpv6c_cer_id {
	uint16_t type;
	uint16_t len;
	struct in6_addr addr;
} _packed;

struct dhcpv6c_s46_portparams {
	uint8_t offset;
	uint8_t psid_len;
	uint16_t psid;
} _packed;

struct dhcpv6c_s46_v4v6bind {
	struct in_addr ipv4_address;
	uint8_t bindprefix6_len;
	uint8_t bind_ipv6_prefix[];
} _packed;

struct dhcpv6c_s46_dmr {
	uint8_t dmr_prefix6_len;
	uint8_t dmr_ipv6_prefix[];
} _packed;

struct dhcpv6c_s46_rule {
	uint8_t flags;
	uint8_t ea_len;
	uint8_t prefix4_len;
	struct in_addr ipv4_prefix;
	uint8_t prefix6_len;
	uint8_t ipv6_prefix[];
} _packed;


struct dhcpv6c_request_prefix {
	uint32_t iaid;
	uint16_t length;
};

struct icmp6_opt {
	uint8_t type;
	uint8_t len;
	uint8_t data[6];
};

struct dhcpv6c_fqdn
{
	uint16_t type;
	uint16_t len;
	uint8_t flags;
	uint8_t data[256];
} ;

struct dhcpv6c_packet {
	struct dhcpv6c_header hdr;
};

#define dhcpv6c_for_each_option(start, end, otype, olen, odata)\
	for (_o = (uint8_t*)(start); _o + 4 <= (uint8_t*)(end) &&\
		((otype) = _o[0] << 8 | _o[1]) && ((odata) = (void*)&_o[4]) &&\
		((olen) = _o[2] << 8 | _o[3]) + (odata) <= (uint8_t*)(end); \
		_o += 4 + (_o[2] << 8 | _o[3]))

int dhcpv6_packet_init(struct dhcpv6c_packet *packet, int msgtype);
int dhcpv6_packet_option(dhcpv6_option_set_t *option_tbl, enum dhcpv6c_msg type, char *data, zpl_uint32 len);
int dhcpv6_packet_build_header(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len);
int dhcpv6_packet_build(dhcpv6_option_set_t *option_tbl, zpl_uint16 code, char *data, zpl_uint32 len);

int dhcpv6_packet_build_solicit(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len);
int dhcpv6_packet_build_request(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len);
int dhcpv6_packet_build_confirm(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len);
int dhcpv6_packet_build_renew(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len);
int dhcpv6_packet_build_rebind(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len);
int dhcpv6_packet_build_release(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len);
int dhcpv6_packet_build_decline(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len);
int dhcpv6_packet_build_info_req(dhcpv6_option_set_t *option_tbl, char *data, zpl_uint32 len);
/**************************************************************************************/

int dhcpv6_option_foreach_handle(struct dhcpv6c_interface *ifp, char *data, zpl_uint32 len);

int dhcpv6_packet_recv_advert(struct dhcpv6c_interface *ifp, char *data, zpl_uint32 len);
int dhcpv6_packet_recv_reply(struct dhcpv6c_interface *ifp, char *data, zpl_uint32 len);
int dhcpv6_packet_recv_reconf(struct dhcpv6c_interface *ifp, char *data, zpl_uint32 len);
/**************************************************************************************/
int dhcpv6c_packet_socket(struct dhcpv6c_interface *ifp);
/********************************************************************************************************/
int dhcpv6c_packet_send(struct dhcpv6c_interface *ifp, enum dhcpv6c_msg type, char *data, uint16_t len);
int dhcpv6c_packet_recv(struct dhcpv6c_interface *ifp, ifindex_t *ifindex, 
	struct ipstack_sockaddr_in6 *addr, char *data, uint16_t len);

#endif /*ZPL_DHCPV6C_MODULE*/

#ifdef __cplusplus
}
#endif

#endif /* __DHCPV6C_PACKET_H__ */