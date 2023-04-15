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

PJ_IDEF(pj_str_t) * pj_str_alloc(char *str);
PJ_IDEF(void) pj_str_free(pj_str_t *str);
PJ_IDEF(void) pj_str_set(pj_str_t *dst, char *str);
PJ_IDEF(void) pj_str_clr(pj_str_t *str);


typedef struct voip_payload_s
{
	char 		payload_name[PJSIP_NUMBER_MAX*2];
	zpl_int8 		active;
	char 		cmdname[PJSIP_NUMBER_MAX];
	zpl_uint8 		index;
}voip_payload_t;

int codec_payload_index(char *cmdname);
const char * codec_payload_name(zpl_uint32 index);
const char * codec_cmdname(zpl_uint32 index);


extern zpl_uint32 voip_get_address(ifindex_t ifindex);

#ifdef __cplusplus
}
#endif

#endif /* __PJSIP_PJSUA_UTIL_H__ */
