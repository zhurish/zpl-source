/*
 * pjsua_util.h
 *
 *  Created on: Jun 22, 2019
 *      Author: zhurish
 */

#ifndef __PJSIP_PJSUA_UTIL_H__
#define __PJSIP_PJSUA_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif


#define PJSIP_NUMBER_MAX		32
typedef struct voip_payload_s
{
	char 		payload_name[PJSIP_NUMBER_MAX*2];
	int 		active;
	char 		cmdname[PJSIP_NUMBER_MAX];
	int 		index;
}voip_payload_t;

int codec_payload_index(char *cmdname);
const char * codec_payload_name(int index);
const char * codec_cmdname(int index);


extern int voip_get_address(ifindex_t ifindex);

#ifdef __cplusplus
}
#endif

#endif /* __PJSIP_PJSUA_UTIL_H__ */
