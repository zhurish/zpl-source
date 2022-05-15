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

extern const zpl_uint8 DHCP_MAC_BCAST_ADDR[6]; /* six all-ones */


#if ENABLE_FEATURE_UDHCP_RFC3397 || ENABLE_FEATURE_UDHCPC6_RFC3646 || ENABLE_FEATURE_UDHCPC6_RFC4704
char *dname_dec(const zpl_uint8 *cstr, zpl_uint32 clen, const char *pre);
zpl_uint8 *dname_enc(const zpl_uint8 *cstr, zpl_uint32 clen, const char *src, zpl_uint32 *retlen);
#endif

/* 2nd param is "zpl_uint32 *" */
int udhcp_str2nip(const char *str, void *arg);
/* 2nd param is "struct option_set**" */
/*
int udhcp_str2optset(const int opc, const char *str,
		void *arg,
		const struct dhcp_optflag *optflags,
		const char *option_strings);
*/

/* note: ip is a pointer to an IPv6 in network order, possibly misaliged */
int sprint_nip6(char *dest, /*const char *pre,*/ const zpl_uint8 *ip);


#if ENABLE_UDHCPC || ENABLE_UDHCPD
void udhcp_header_init(struct dhcp_packet *packet, char type);
#endif

int udhcp_recv_packet(struct dhcp_packet *packet, zpl_socket_t fd, zpl_uint32 *ifindex);

//typedef struct dhcpd_interface_s dhcpd_interface_t;

int  udhcp_send_raw_packet(zpl_socket_t fd, struct dhcp_packet *dhcp_pkt,
	struct udhcp_packet_cmd *source,
	struct udhcp_packet_cmd *dest, const zpl_uint8 *dest_arp,
	ifindex_t ifindex);

int udhcp_send_udp_packet(zpl_socket_t fd, struct dhcp_packet *dhcp_pkt,
	struct udhcp_packet_cmd *source,
	struct udhcp_packet_cmd *dest);


/*
 * dhcp_util.c
*/
zpl_socket_t udhcp_udp_socket(zpl_uint16 port, ifindex_t ifindex);
zpl_socket_t udhcp_raw_socket(void);
int udhcp_client_socket_bind(zpl_socket_t fd, ifindex_t ifindex);
int udhcp_client_socket_filter(zpl_socket_t fd, zpl_uint16 port);

/*
 * dhcp_arpping.c
*/
/* Returns 1 if no reply received */
int icmp_echo_request(zpl_uint32  test_nip,
		const zpl_uint8 *safe_mac,
		zpl_uint32  from_ip,
		zpl_uint8 *from_mac,
		const char *interface,
		unsigned timeo);

int icmp_echo_request_mac(zpl_uint32  test_nip,
		zpl_uint32  from_ip,
		zpl_uint8 *from_mac,
		const char *interface,
		zpl_uint32 timeo,
		zpl_uint8 *safe_mac);

//zpl_uint16 inet_cksum(zpl_uint16 *addr, zpl_uint32 nleft);

char*  xasprintf(const char *format, ...);

ssize_t  safe_read(zpl_socket_t fd, void *buf, zpl_uint32 count);
int  safe_poll(zpl_socket_t ufds, int maxfd, zpl_uint32 timeout);
ssize_t safe_write(zpl_socket_t fd, const void *buf, size_t count);

//int  index_in_strings(const char *strings, const char *key);
char*  bin2hex(char *p, const char *cp, zpl_uint32 count);
char*  hex2bin(char *dst, const char *str, zpl_uint32 count);

int FAST_FUNC udhcp_read_interface(const char *interface, int *ifindex, uint32_t *nip, uint8_t *mac);

int udhcp_interface_mac(ifindex_t ifindex, zpl_uint32  *nip, zpl_uint8 *mac);
int dhcp_client_lease_set(void *ifter);
int dhcp_client_lease_unset(void *ifter);
void udhcp_run_script(void *p, const char *name);
 
#ifdef __cplusplus
}
#endif
 
#endif /* __UDHCP_UTIL_H__ */
