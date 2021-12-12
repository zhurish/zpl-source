/*
 * voip_util.h
 *
 *  Created on: Dec 8, 2018
 *      Author: zhurish
 */

#ifndef __VOIP_UTIL_H__
#define __VOIP_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct voip_payload_s
{
	char 		payload_name[PJSIP_NUMBER_MAX*2];
	zpl_uint8 		active;
	char 		cmdname[PJSIP_NUMBER_MAX];
	zpl_uint8 		index;
}voip_payload_t;

int codec_payload_index(char *cmdname);
char * codec_payload_name(zpl_uint32 index);
char * codec_cmdname(zpl_uint32 index);


extern zpl_uint32 voip_get_address(zpl_uint32 ifindex);

#if 0
extern int phone_string_to_hex(char * room, zpl_uint8 *phone);
extern int phone_string_to_compress(char * room, zpl_uint8 *phone);
extern int phone_compress_to_uncompress(zpl_uint8 *phonetmp, int len, zpl_uint8 *phone);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VOIP_UTIL_H__ */
