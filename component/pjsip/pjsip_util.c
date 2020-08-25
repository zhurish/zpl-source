/*
 * pjsua_app_cb.c
 *
 *  Created on: Jun 22, 2019
 *      Author: zhurish
 */

#include "pjsua_app_common.h"
#include "pjsip_util.h"


PJ_IDEF(pj_str_t) * pj_str_alloc(char *str)
{
    pj_str_t *dst;
    dst = malloc(sizeof(pj_str_t));
    if(dst)
    {
    	dst->ptr = str ? strdup(str):NULL;
    	dst->slen = str ? pj_ansi_strlen(str) : 0;
    	return dst;
    }
    return NULL;
}

PJ_IDEF(void) pj_str_free(pj_str_t *str)
{
    if(str)
    {
    	if(str->ptr)
    		free(str->ptr);
    	str->slen = 0;
    	free(str);
    	return ;
    }
    return ;
}

PJ_IDEF(void) pj_str_set(pj_str_t *dst, char *str)
{
    if(dst)
    {
    	dst->ptr = str ? strdup(str):NULL;
    	dst->slen = str ? pj_ansi_strlen(str) : 0;
    	return ;
    }
    return ;
}

PJ_IDEF(void) pj_str_clr(pj_str_t *str)
{
    if(str)
    {
    	if(str->ptr)
    		free(str->ptr);
    	str->slen = 0;
    	return ;
    }
    return ;
}
