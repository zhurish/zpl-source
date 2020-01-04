/*
 * dhcp_option.h
 *
 *  Created on: Apr 27, 2019
 *      Author: zhurish
 */

#ifndef __DHCP_OPTION_H__
#define __DHCP_OPTION_H__

#define DHCP_OPTION_MAX	256

#pragma pack(1)
typedef struct dhcp_option_set {
	uint8_t code;
	uint8_t len;
	uint8_t *data;
}dhcp_option_set_t;

typedef struct dhcp_option_hdr {
	uint8_t code;
	uint8_t len;
	union
	{
		unsigned char 	pval[1];
		unsigned char 	val8;
		unsigned short 	val16;
		unsigned int 	val32;
	}val;
}dhcp_option_hdr_t;
#pragma pack(0)


extern int dhcp_option_flags(uint8_t code);

/*
 * for dhcp pool option handle
 */
extern int dhcp_option_add(dhcp_option_set_t *option_tbl, uint8_t code, const uint8_t *opt, int len);
extern int dhcp_option_add_hex(dhcp_option_set_t *option_tbl, uint8_t code, const uint32_t value, int len);
extern int dhcp_option_del(dhcp_option_set_t *option_tbl, uint8_t code);
extern int dhcp_option_clean(dhcp_option_set_t *option_tbl);
extern int dhcp_option_lookup(dhcp_option_set_t *option_tbl, uint8_t code);
extern int dhcp_option_string_set(dhcp_option_set_t *option_tbl, uint8_t code,
		const char *const_str);

extern int dhcp_option_packet(dhcp_option_set_t *option_tbl, char *data, int len);


/*
 * for dhcp packet option handle
 */
extern int udhcp_end_option(uint8_t *optionptr);
extern int dhcp_option_get_length(char *data);
extern int dhcp_option_message_type(char *data, uint8_t code);
extern int dhcp_option_message_type_get(char *data, int len);
extern uint8_t * dhcp_option_get(char *data, int len, uint8_t code, uint8_t *optlen);
extern int dhcp_option_get_simple(const char *data, u_int32 *output, uint8_t code, uint8_t optlen);
extern uint8_t *udhcp_get_option(struct dhcp_packet *packet, int code, int *optlen);


extern int dhcp_option_packet_set_simple(char *data, int len, uint8_t code, uint32_t value);
extern int dhcp_option_packet_set_value(char *data, int len, uint8_t code, uint32_t oplen, uint8_t *opt);
extern int udhcp_add_simple_option(struct dhcp_packet *packet, uint8_t code, uint32_t value);
extern int udhcp_add_simple_option_value(struct dhcp_packet *packet, uint8_t code, uint32_t oplen, uint8_t *opt);

#define dhcp_option_add_8bit(t, c, v)		dhcp_option_add_hex(t, c, v, 1)
#define dhcp_option_add_16bit(t, c, v)		dhcp_option_add_hex(t, c, v, 2)
#define dhcp_option_add_32bit(t, c, v)		dhcp_option_add_hex(t, c, v, 4)
#define dhcp_option_add_address(t, c, v)	dhcp_option_add_hex(t, c, v, 4)
#define dhcp_option_add_string(t, c, v, l)	dhcp_option_add(t, c, v, l)

#define dhcp_option_get_8bit(d, c, v)		dhcp_option_get_simple(d, v, c, 1)
#define dhcp_option_get_16bit(d, c, v)		dhcp_option_get_simple(d, v, c, 2)
#define dhcp_option_get_32bit(d, c, v)		dhcp_option_get_simple(d, v, c, 4)
#define dhcp_option_get_address(d, c, v)	dhcp_option_get_simple(d, v, c, 4)


#endif /* __DHCP_OPTION_H__ */
