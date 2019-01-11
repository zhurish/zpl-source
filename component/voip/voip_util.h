/*
 * voip_util.h
 *
 *  Created on: Dec 8, 2018
 *      Author: zhurish
 */

#ifndef __VOIP_UTIL_H__
#define __VOIP_UTIL_H__

#include "voip_def.h"

extern const char *inet_address(u_int32);

extern u_int8 atoascii(int a);

extern u_int32 string_to_hex(char * room);
extern char * hex_to_string(u_int32 hex);


extern int phone_string_to_hex(char * room, u_int8 *phone);
extern int phone_string_to_compress(char * room, u_int8 *phone);
extern int phone_compress_to_uncompress(u_int8 *phonetmp, int len, u_int8 *phone);

#endif /* __VOIP_UTIL_H__ */
