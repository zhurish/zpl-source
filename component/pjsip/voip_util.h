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
	ospl_uint8 		active;
	char 		cmdname[PJSIP_NUMBER_MAX];
	ospl_uint8 		index;
}voip_payload_t;

int codec_payload_index(char *cmdname);
char * codec_payload_name(ospl_uint32 index);
char * codec_cmdname(ospl_uint32 index);


extern ospl_uint32 voip_get_address(ospl_uint32 ifindex);

#if 0
extern int phone_string_to_hex(char * room, ospl_uint8 *phone);
extern int phone_string_to_compress(char * room, ospl_uint8 *phone);
extern int phone_compress_to_uncompress(ospl_uint8 *phonetmp, int len, ospl_uint8 *phone);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VOIP_UTIL_H__ */
