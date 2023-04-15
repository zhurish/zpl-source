/*
 * dhcp_option.h
 *
 *  Created on: Apr 27, 2019
 *      Author: zhurish
 */

#ifndef __DHCP_OPTION_H__
#define __DHCP_OPTION_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DHCP_OPTION_MAX	256
/*** Options ***/


/* Offsets in option byte sequence */
#define DHCP_OPT_CODE                0
#define DHCP_OPT_LEN                 1
#define DHCP_OPT_DATA                2
/* Bits in "overload" option */
#define DHCP_OPTION_FIELD            0
#define DHCP_FILE_FIELD              1
#define DHCP_SNAME_FIELD             2



/* DHCP option codes (partial list). See RFC 2132 and
 * http://www.iana.org/assignments/bootp-dhcp-parameters/
 * Commented out options are handled by common option machinery,
 * uncommented ones have special cases (grep for them to see).
 */
#define DHCP_OPTION_PADDING            0x00
#define DHCP_OPTION_SUBNET             0x01
#define DHCP_OPTION_TIME_OFFSET      0x02 /* (localtime - UTC_time) in seconds. signed */
#define DHCP_OPTION_ROUTER           0x03
#define DHCP_OPTION_TIME_SERVER      0x04 /* RFC 868 time server (32-bit, 0 = 1.1.1900) */
#define DHCP_OPTION_NAME_SERVER      0x05 /* IEN 116 _really_ ancient kind of NS */
#define DHCP_OPTION_DNS_SERVER       0x06
#define DHCP_OPTION_LOG_SERVER       0x07 /* port 704 UDP log (not syslog)*/
#define DHCP_OPTION_COOKIE_SERVER    0x08 /* "quote of the day" server */
#define DHCP_OPTION_LPR_SERVER       0x09
#define DHCP_OPTION_HOST_NAME          0x0c /* either client informs server or server gives name to client */
#define DHCP_OPTION_BOOT_SIZE        0x0d
#define DHCP_OPTION_DOMAIN_NAME      0x0f /* server gives domain suffix */
#define DHCP_OPTION_SWAP_SERVER      0x10
#define DHCP_OPTION_ROOT_PATH        0x11
#define DHCP_OPTION_IP_TTL           0x17
#define DHCP_OPTION_MTU              0x1a
#define DHCP_OPTION_BROADCAST        0x1c
#define DHCP_OPTION_ROUTES           0x21
#define DHCP_OPTION_NIS_DOMAIN       0x28
#define DHCP_OPTION_NIS_SERVER       0x29
#define DHCP_OPTION_NTP_SERVER       0x2a
#define DHCP_OPTION_WINS_SERVER      0x2c
#define DHCP_OPTION_REQUESTED_IP       0x32 /* sent by client if specific IP is wanted */
#define DHCP_OPTION_LEASE_TIME         0x33
#define DHCP_OPTION_OVERLOAD    0x34
#define DHCP_OPTION_MESSAGE_TYPE       0x35
#define DHCP_OPTION_SERVER_ID          0x36 /* by default server's IP */
#define DHCP_OPTION_PARAM_REQ          0x37 /* list of options client wants */
#define DHCP_OPTION_ERR_MESSAGE      0x38 /* error message when sending NAK etc */
#define DHCP_OPTION_MAX_SIZE           0x39
#define DHCP_OPTION_RENEWAL_TIME         0x3a	/* renewal time */
#define DHCP_OPTION_REBINDING_TIME         0x3b	/* rebinding time */
#define DHCP_OPTION_VENDOR             0x3c /* client's vendor (a string) option 60*/
#define DHCP_OPTION_CLIENT_ID          0x3d /* by default client's MAC addr, but may be arbitrarily long */
#define DHCP_OPTION_TFTP_SERVER_NAME 0x42 /* same as 'sname' field */
#define DHCP_OPTION_BOOT_FILE        0x43 /* same as 'file' field */
#define DHCP_OPTION_USER_CLASS       0x4d /* RFC 3004. set of LASCII strings. "I am a printer" etc */
#define DHCP_OPTION_FQDN               0x51 /* client asks to update DNS to map its FQDN to its new IP */
#define DHCP_OPTION_OPT82            0x52 /* client asks to update DNS to map its FQDN to its new IP */
#define DHCP_OPTION_DOMAIN_SEARCH    0x77 /* RFC 3397. set of ASCIZ string, DNS-style compressed */
#define DHCP_OPTION_SIP_SERVERS      0x78 /* RFC 3361. flag byte, then: 0: domain names, 1: IP addrs */
#define DHCP_OPTION_STATIC_ROUTES    0x79 /* RFC 3442. (mask,ip,router) tuples */
#define DHCP_OPTION_VLAN_ID          0x84 /* 802.1P VLAN ID */
#define DHCP_OPTION_VLAN_PRIORITY    0x85 /* 802.1Q VLAN priority */
#define DHCP_OPTION_PXE_CONF_FILE    0xd1 /* RFC 5071 Configuration File */
#define DHCP_OPTION_PXE_PATH_PREFIX  0xd2 /* RFC 5071 Configuration File */
#define DHCP_OPTION_MS_STATIC_ROUTES 0xf9 /* Microsoft's pre-RFC 3442 code for 0x79? */
#define DHCP_OPTION_WPAD             0xfc /* MSIE's Web Proxy Autodiscovery Protocol */
#define DHCP_OPTION_END                0xff



enum {
	DHCP_OPTION_TYPE_IP = 1,
	DHCP_OPTION_TYPE_IP_PAIR,
	DHCP_OPTION_TYPE_STRING,
	/* Opts of STRING_HOST type will be sanitized before they are passed
	 * to udhcpc script's environment: */
	DHCP_OPTION_TYPE_STRING_HOST,
	DHCP_OPTION_TYPE_BOOLEAN,
	DHCP_OPTION_TYPE_U8,
	DHCP_OPTION_TYPE_U16,
	DHCP_OPTION_TYPE_S16,
	DHCP_OPTION_TYPE_U32,
	DHCP_OPTION_TYPE_S32,
	DHCP_OPTION_TYPE_BIN,
	DHCP_OPTION_TYPE_STATIC_ROUTES,
	DHCP_OPTION_TYPE_6RD,
#if DHCP_ENABLE_RFC3397 || DHCP6_ENABLE_RFC3646 || DHCP6_ENABLE_RFC4704
	DHCP_OPTION_TYPE_DNS_STRING,  /* RFC1035 compressed domain name list */
#endif
#if DHCP_ENABLE_RFC3397
	DHCP_OPTION_TYPE_SIP_SERVERS,
#endif

	DHCP_OPTION_TYPE_MASK = 0x00ff,
	/* Client requests this option by default */
	DHCP_OPTION_TYPE_REQ  = 0x1000,
	/* There can be a list of 1 or more of these */
	DHCP_OPTION_TYPE_LIST = 0x2000,
};

#define DHCP_OPTION_61_MAC           1
#define DHCP_OPTION_61_UUID          0x02
#define DHCP_OPTION_61_IAID          0xFF
/*0x01代表后面是硬件地址，
    0xff表示IAID，
    0x0002表示后面是DUID*/

struct dhcp_optflag {
	zpl_uint32 flags;
	zpl_uint8 code;
};


#pragma pack(1)
typedef struct dhcp_option_set {
	zpl_uint8 code;
	zpl_uint8 len;
	zpl_uint8 *data;
}dhcp_option_set_t;

typedef struct dhcp_option_hdr {
	zpl_uint8 code;
	zpl_uint8 len;
	union
	{
		zpl_uint8 	pval[1];
		zpl_uint8 	val8;
		zpl_uint16 	val16;
		zpl_uint32 	val32;
	}val;
}dhcp_option_hdr_t;
#pragma pack(0)


extern int dhcp_option_flags(zpl_uint8 code);

/*
 * for dhcp pool option handle
 */
extern int dhcp_option_add(dhcp_option_set_t *option_tbl, zpl_uint16 code, const zpl_uint8 *opt, zpl_uint32 len);
extern int dhcp_option_add_hex(dhcp_option_set_t *option_tbl, zpl_uint16 code, const zpl_uint32  value, zpl_uint32 len);
extern int dhcp_option_del(dhcp_option_set_t *option_tbl, zpl_uint16 code);
extern int dhcp_option_clean(dhcp_option_set_t *option_tbl);
extern int dhcp_option_lookup(dhcp_option_set_t *option_tbl, zpl_uint16 code);
extern int dhcp_option_string_set(dhcp_option_set_t *option_tbl, zpl_uint16 code,
		const char *const_str);

extern int dhcp_option_packet(dhcp_option_set_t *option_tbl, char *data, zpl_uint32 len);


/*
 * for dhcp packet option handle
 */
extern int dhcp_end_option(zpl_uint8 *optionptr);
extern zpl_uint32 dhcp_option_get_length(char *data);
extern int dhcp_option_message_type(char *data, zpl_uint8 code);
extern int dhcp_option_message_type_get(char *data, zpl_uint32 len);
extern zpl_uint8 * dhcp_option_get(char *data, zpl_uint32 len, zpl_uint8 code, zpl_uint8 *optlen);
extern int dhcp_option_get_simple(const char *data, zpl_uint32 *output, zpl_uint8 code, zpl_uint8 optlen);
extern zpl_uint8 *udhcp_get_option(struct dhcp_packet *packet, zpl_uint8 code, zpl_uint32 *optlen);


extern int dhcp_option_packet_set_simple(char *data, zpl_uint32 len, zpl_uint8 code, zpl_uint32  value);
extern int dhcp_option_packet_set_value(char *data, zpl_uint32 len, zpl_uint8 code, zpl_uint32  oplen, zpl_uint8 *opt);
extern int dhcp_add_simple_option(struct dhcp_packet *packet, zpl_uint8 code, zpl_uint32  value);
extern int dhcp_add_simple_option_value(struct dhcp_packet *packet, zpl_uint8 code, zpl_uint32  oplen, zpl_uint8 *opt);

#define dhcp_option_add_8bit(t, c, v)		dhcp_option_add_hex(t, c, v, 1)
#define dhcp_option_add_16bit(t, c, v)		dhcp_option_add_hex(t, c, v, 2)
#define dhcp_option_add_32bit(t, c, v)		dhcp_option_add_hex(t, c, v, 4)
#define dhcp_option_add_address(t, c, v)	dhcp_option_add_hex(t, c, v, 4)
#define dhcp_option_add_string(t, c, v, l)	dhcp_option_add(t, c, v, l)

#define dhcp_option_get_8bit(d, c, v)		dhcp_option_get_simple(d, v, c, 1)
#define dhcp_option_get_16bit(d, c, v)		dhcp_option_get_simple(d, v, c, 2)
#define dhcp_option_get_32bit(d, c, v)		dhcp_option_get_simple(d, v, c, 4)
#define dhcp_option_get_address(d, c, v)	dhcp_option_get_simple(d, v, c, 4)

 
#ifdef __cplusplus
}
#endif
 
#endif /* __DHCP_OPTION_H__ */
