/*
 * util.h
 *
 *  Created on: Apr 20, 2019
 *      Author: zhurish
 */

#ifndef __UDHCP_UTIL_H__
#define __UDHCP_UTIL_H__


extern const uint8_t DHCP_MAC_BCAST_ADDR[6]; /* six all-ones */


#if ENABLE_FEATURE_UDHCP_RFC3397 || ENABLE_FEATURE_UDHCPC6_RFC3646 || ENABLE_FEATURE_UDHCPC6_RFC4704
char *dname_dec(const uint8_t *cstr, int clen, const char *pre);
uint8_t *dname_enc(const uint8_t *cstr, int clen, const char *src, int *retlen);
#endif

/* 2nd param is "uint32_t*" */
int udhcp_str2nip(const char *str, void *arg);
/* 2nd param is "struct option_set**" */
/*
int udhcp_str2optset(const int opc, const char *str,
		void *arg,
		const struct dhcp_optflag *optflags,
		const char *option_strings);
*/

/* note: ip is a pointer to an IPv6 in network order, possibly misaliged */
int sprint_nip6(char *dest, /*const char *pre,*/ const uint8_t *ip);


#if ENABLE_UDHCPC || ENABLE_UDHCPD
void udhcp_header_init(struct dhcp_packet *packet, char type);
#endif

int udhcp_recv_packet(struct dhcp_packet *packet, int fd, u_int32 *ifindex);

//typedef struct dhcpd_interface_s dhcpd_interface_t;

int  udhcp_send_raw_packet(int fd, struct dhcp_packet *dhcp_pkt,
	struct udhcp_packet_cmd *source,
	struct udhcp_packet_cmd *dest, const uint8_t *dest_arp,
	int ifindex);

int udhcp_send_udp_packet(int fd, struct dhcp_packet *dhcp_pkt,
	struct udhcp_packet_cmd *source,
	struct udhcp_packet_cmd *dest);


/*
 * dhcp_util.c
*/
int udhcp_udp_socket(/*uint32_t ip,*/int port);
int udhcp_raw_socket(void);
int udhcp_client_socket_bind(int fd, int ifindex);
int udhcp_client_socket_filter(int fd, int port);

/*
 * dhcp_arpping.c
*/
/* Returns 1 if no reply received */
int icmp_echo_request(uint32_t test_nip,
		const uint8_t *safe_mac,
		uint32_t from_ip,
		uint8_t *from_mac,
		const char *interface,
		unsigned timeo);



//uint16_t inet_cksum(uint16_t *addr, int nleft);

char*  xasprintf(const char *format, ...);

ssize_t  safe_read(int fd, void *buf, size_t count);
int  safe_poll(struct pollfd *ufds, nfds_t nfds, int timeout);

//int  index_in_strings(const char *strings, const char *key);
char*  bin2hex(char *p, const char *cp, int count);
char*  hex2bin(char *dst, const char *str, int count);



int udhcp_interface_mac(int ifindex, uint32_t *nip, uint8_t *mac);
int dhcp_client_lease_set(void *ifter);
int dhcp_client_lease_unset(void *ifter);
void udhcp_run_script(void *p, const char *name);

#endif /* __UDHCP_UTIL_H__ */
