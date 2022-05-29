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

#ifndef __DHCPV6C_STATE_H__
#define __DHCPV6C_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif
#ifdef ZPL_DHCPV6C_MODULE
enum dhcpv6c_state {
	DHCPV6C_STATE_CLIENT_ID,
	DHCPV6C_STATE_SERVER_ID,
	DHCPV6C_STATE_SERVER_CAND,
	DHCPV6C_STATE_SERVER_ADDR,
	DHCPV6C_STATE_ORO,
	DHCPV6C_STATE_DNS,
	DHCPV6C_STATE_SEARCH,
	DHCPV6C_STATE_IA_NA,
	DHCPV6C_STATE_IA_PD,
	DHCPV6C_STATE_IA_PD_INIT,
	DHCPV6C_STATE_CUSTOM_OPTS,
	DHCPV6C_STATE_SNTP_IP,
	DHCPV6C_STATE_NTP_IP,
	DHCPV6C_STATE_NTP_FQDN,
	DHCPV6C_STATE_SIP_IP,
	DHCPV6C_STATE_SIP_FQDN,
	DHCPV6C_STATE_RA_ROUTE,
	DHCPV6C_STATE_RA_PREFIX,
	DHCPV6C_STATE_RA_DNS,
	DHCPV6C_STATE_RA_SEARCH,
	DHCPV6C_STATE_AFTR_NAME,
	DHCPV6C_STATE_OPTS,
	DHCPV6C_STATE_CER,
	DHCPV6C_STATE_S46_MAPT,
	DHCPV6C_STATE_S46_MAPE,
	DHCPV6C_STATE_S46_LW,
	DHCPV6C_STATE_PASSTHRU,
	DHCPV6C_STATE_MAX
};

typedef struct clientv6_state_s
{
	zpl_uint16		mode;	/* DHCP_RAW_MODE, DHCP_UDP_MODE*/
	zpl_uint16		state;
	zpl_uint16 	dis_timeout;
	zpl_uint16 	dis_retries;
	zpl_uint16 	dis_cnt;
	zpl_uint32		renew_timeout1;
	zpl_uint32		renew_timeout2;
	zpl_uint32 	xid;
	zpl_uint32 		read_bytes;
}clientv6_state_t;


typedef struct clientv6_lease
{
	struct ipstack_in6_addr 	lease_address;
	struct ipstack_in6_addr 	lease_netmask;
	struct ipstack_in6_addr 	lease_gateway;
	struct ipstack_in6_addr 	lease_broadcast;

	struct ipstack_in6_addr 	lease_dns1;
	struct ipstack_in6_addr 	lease_dns2;

	struct ipstack_in6_addr 	lease_ntp1;
	struct ipstack_in6_addr 	lease_ntp2;

	struct ipstack_in6_addr 	lease_sip1;
	struct ipstack_in6_addr 	lease_sip2;

	zpl_uint8 		lease_ttl;
	zpl_uint16 		lease_mtu;

	LIST 		static_route_list;

	struct ipstack_in6_addr 	server_address;
	struct ipstack_in6_addr 	gateway_address;


	uint8_t serv_duid_len;
	uint8_t serv_duid[130];
	uint32_t sol_max_rt;
	uint32_t inf_max_rt;
	uint8_t preference;
	uint8_t wants_reconfigure;

	char tz[64];
	char tz_name[64];
	char bootfile_url[64];
	char bootfile_param[64];
	char pxeconffile[64];
	char pxepathprefix[64];
	int16_t priority;
	uint32_t valid;
	uint32_t preferred;
	uint8_t prefix;
	uint32_t t1;
	uint32_t t2;
	uint32_t iaid;
	uint8_t auxtarget[];



}clientv6_lease_t;

// State manipulation
void dhcpv6c_state_clear(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state);
int dhcpv6c_state_add(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state, const void *data, size_t len);
void dhcpv6c_state_append(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state, const void *data, size_t len);
int dhcpv6c_state_insert(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state, size_t offset, const void *data, size_t len);
size_t dhcpv6c_state_remove(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state, size_t offset, size_t len);
void* dhcpv6c_state_move(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state, size_t *len);
void* dhcpv6c_state_get(struct dhcpv6c_interface *ifp, enum dhcpv6c_state state, size_t *len);
uint8_t *dhcpv6c_state_find_option(struct dhcpv6c_interface *ifp, const uint16_t code);

#endif /*ZPL_DHCPV6C_MODULE*/
#ifdef __cplusplus
}
#endif

#endif /* __DHCPV6C_STATE_H__ */