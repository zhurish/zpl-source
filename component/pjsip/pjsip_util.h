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

#ifdef __cplusplus
}
#endif

#endif /* __PJSIP_PJSUA_UTIL_H__ */
