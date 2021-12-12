/*
	onvif_util.h
*/
#ifndef __ONVIF_UTIL_H__
#define __ONVIF_UTIL_H__

#define ONVIF_MULTICAST_GROUP   "239.255.255.250" /* use a group IP such as "225.0.0.37" */
#define ONVIF_BIND_PORT         3702

#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"

#define MAX_4_LEN 			4
#define MAX_8_LEN 			8
#define MAX_16_LEN 			16
#define MAX_32_LEN 			32
#define MAX_64_LEN 			64
#define MAX_128_LEN 		128
#define MAX_256_LEN 		256
#define MAX_512_LEN 		512
#define MAX_1024_LEN 		1024
#define MAX_2048_LEN 		2048

#define _ONVIF_DEBUG
#ifdef _ONVIF_DEBUG
#define ONVIF_DEBUG_MSG(fmt,...)     zlog_force_trap (MODULE_ONVIF, fmt, ##__VA_ARGS__)
#define ONVIF_HWDEBUG_MSG(fmt,...)   zlog_force_trap (MODULE_ONVIF, fmt, ##__VA_ARGS__)
#else
#define ONVIF_DEBUG_MSG(fmt...)
#define ONVIF_HWDEBUG_MSG(fmt...) 
#endif
	
#ifdef ZPL_ONVIF_SSL
zpl_bool soap_access_control(struct soap* soap);
#endif
int _hw_onvif_get_uuid(zpl_uint8 *buf);
int _hw_onvif_get_IPport(zpl_uint8 *buf, zpl_uint16 *port);



#endif /* __ONVIF_UTIL_H__ */
