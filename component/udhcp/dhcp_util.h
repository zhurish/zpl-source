/*
 * util.h
 *
 *  Created on: Apr 20, 2019
 *      Author: zhurish
 */

#ifndef __UDHCP_UTIL_H__
#define __UDHCP_UTIL_H__


#ifdef __cplusplus
extern "C" {
#endif

extern const ospl_uint8 DHCP_MAC_BCAST_ADDR[6]; /* six all-ones */


#if ENABLE_FEATURE_UDHCP_RFC3397 || ENABLE_FEATURE_UDHCPC6_RFC3646 || ENABLE_FEATURE_UDHCPC6_RFC4704
char *dname_dec(const ospl_uint8 *cstr, ospl_uint32 clen, const char *pre);
ospl_uint8 *dname_enc(const ospl_uint8 *cstr, ospl_uint32 clen, const char *src, ospl_uint32 *retlen);
#endif

/* 2nd param is "ospl_uint32 *" */
int udhcp_str2nip(const char *str, void *arg);
/* 2nd param is "struct option_set**" */
/*
int udhcp_str2optset(const int opc, const char *str,
		void *arg,
		const struct dhcp_optflag *optflags,
		const char *option_strings);
*/

/* note: ip is a pointer to an IPv6 in network order, possibly misaliged */
int sprint_nip6(char *dest, /*const char *pre,*/ const ospl_uint8 *ip);


#if ENABLE_UDHCPC || ENABLE_UDHCPD
void udhcp_header_init(struct dhcp_packet *packet, char type);
#endif

int udhcp_recv_packet(struct dhcp_packet *packet, int fd, ospl_uint32 *ifindex);

//typedef struct dhcpd_interface_s dhcpd_interface_t;

int  udhcp_send_raw_packet(int fd, struct dhcp_packet *dhcp_pkt,
	struct udhcp_packet_cmd *source,
	struct udhcp_packet_cmd *dest, const ospl_uint8 *dest_arp,
	ifindex_t ifindex);

int udhcp_send_udp_packet(int fd, struct dhcp_packet *dhcp_pkt,
	struct udhcp_packet_cmd *source,
	struct udhcp_packet_cmd *dest);


/*
 * dhcp_util.c
*/
int udhcp_udp_socket(/*ospl_uint32  ip,*/ospl_uint16 port);
int udhcp_raw_socket(void);
int udhcp_client_socket_bind(int fd, ifindex_t ifindex);
int udhcp_client_socket_filter(int fd, ospl_uint16 port);

/*
 * dhcp_arpping.c
*/
/* Returns 1 if no reply received */
int icmp_echo_request(ospl_uint32  test_nip,
		const ospl_uint8 *safe_mac,
		ospl_uint32  from_ip,
		ospl_uint8 *from_mac,
		const char *interface,
		unsigned timeo);

int icmp_echo_request_mac(ospl_uint32  test_nip,
		ospl_uint32  from_ip,
		ospl_uint8 *from_mac,
		const char *interface,
		ospl_uint32 timeo,
		ospl_uint8 *safe_mac);

//ospl_uint16 inet_cksum(ospl_uint16 *addr, ospl_uint32 nleft);

char*  xasprintf(const char *format, ...);

ssize_t  safe_read(int fd, void *buf, ospl_uint32 count);
int  safe_poll(struct pollfd *ufds, nfds_t nfds, ospl_uint32 timeout);

//int  index_in_strings(const char *strings, const char *key);
char*  bin2hex(char *p, const char *cp, ospl_uint32 count);
char*  hex2bin(char *dst, const char *str, ospl_uint32 count);



int udhcp_interface_mac(ifindex_t ifindex, ospl_uint32  *nip, ospl_uint8 *mac);
int dhcp_client_lease_set(void *ifter);
int dhcp_client_lease_unset(void *ifter);
void udhcp_run_script(void *p, const char *name);
 
#ifdef __cplusplus
}
#endif
 
#endif /* __UDHCP_UTIL_H__ */
