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
extern int dhcp_option_add(dhcp_option_set_t *option_tbl, zpl_uint8 code, const zpl_uint8 *opt, zpl_uint32 len);
extern int dhcp_option_add_hex(dhcp_option_set_t *option_tbl, zpl_uint8 code, const zpl_uint32  value, zpl_uint32 len);
extern int dhcp_option_del(dhcp_option_set_t *option_tbl, zpl_uint8 code);
extern int dhcp_option_clean(dhcp_option_set_t *option_tbl);
extern int dhcp_option_lookup(dhcp_option_set_t *option_tbl, zpl_uint8 code);
extern int dhcp_option_string_set(dhcp_option_set_t *option_tbl, zpl_uint8 code,
		const char *const_str);

extern int dhcp_option_packet(dhcp_option_set_t *option_tbl, char *data, zpl_uint32 len);


/*
 * for dhcp packet option handle
 */
extern int udhcp_end_option(zpl_uint8 *optionptr);
extern zpl_uint32 dhcp_option_get_length(char *data);
extern int dhcp_option_message_type(char *data, zpl_uint8 code);
extern int dhcp_option_message_type_get(char *data, zpl_uint32 len);
extern zpl_uint8 * dhcp_option_get(char *data, zpl_uint32 len, zpl_uint8 code, zpl_uint8 *optlen);
extern int dhcp_option_get_simple(const char *data, zpl_uint32 *output, zpl_uint8 code, zpl_uint8 optlen);
extern zpl_uint8 *udhcp_get_option(struct dhcp_packet *packet, zpl_uint8 code, zpl_uint32 *optlen);


extern int dhcp_option_packet_set_simple(char *data, zpl_uint32 len, zpl_uint8 code, zpl_uint32  value);
extern int dhcp_option_packet_set_value(char *data, zpl_uint32 len, zpl_uint8 code, zpl_uint32  oplen, zpl_uint8 *opt);
extern int udhcp_add_simple_option(struct dhcp_packet *packet, zpl_uint8 code, zpl_uint32  value);
extern int udhcp_add_simple_option_value(struct dhcp_packet *packet, zpl_uint8 code, zpl_uint32  oplen, zpl_uint8 *opt);

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
