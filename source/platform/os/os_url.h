/*
 * os_url.h
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#ifndef __OS_URL_H__
#define __OS_URL_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "zpl_type.h"

/*
 * URL
 */

typedef struct os_url_s
{
	zpl_char 		*proto;
	zpl_char 		*host;
	zpl_uint16		port;
	zpl_char 		*user;
	zpl_char 		*pass;
	zpl_char 		*path;
	zpl_char 		*filename;
    zpl_int32     	channel;
    zpl_int32     	level;
    zpl_char        url[128];
    zpl_uint16    	mode;
}os_url_t;

extern int os_url_split(const zpl_char * URL, os_url_t *spliurl);
extern int os_url_free(os_url_t *spliurl);

extern int os_url_test(void);

#ifdef __cplusplus
}
#endif

#endif /* __OS_URL_H__ */
