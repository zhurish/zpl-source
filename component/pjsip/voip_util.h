/*
 * voip_util.h
 *
 *  Created on: Dec 8, 2018
 *      Author: zhurish
 */

#ifndef __VOIP_UTIL_H__
#define __VOIP_UTIL_H__

typedef struct voip_payload_s
{
	char 		payload_name[PJSIP_NUMBER_MAX*2];
	u_int8 		active;
	char 		cmdname[PJSIP_NUMBER_MAX];
	u_int8 		index;
}voip_payload_t;

int codec_payload_index(char *cmdname);
char * codec_payload_name(int index);
char * codec_cmdname(int index);


extern u_int32 voip_get_address(u_int32 ifindex);

#if 0
extern int phone_string_to_hex(char * room, u_int8 *phone);
extern int phone_string_to_compress(char * room, u_int8 *phone);
extern int phone_compress_to_uncompress(u_int8 *phonetmp, int len, u_int8 *phone);
#endif

#endif /* __VOIP_UTIL_H__ */
