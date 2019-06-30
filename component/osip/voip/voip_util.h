/*
 * voip_util.h
 *
 *  Created on: Dec 8, 2018
 *      Author: zhurish
 */

#ifndef __VOIP_UTIL_H__
#define __VOIP_UTIL_H__


extern u_int32 voip_get_address(u_int32 ifindex);


extern int phone_string_to_hex(char * room, u_int8 *phone);
extern int phone_string_to_compress(char * room, u_int8 *phone);
extern int phone_compress_to_uncompress(u_int8 *phonetmp, int len, u_int8 *phone);


extern int usb_event_socket_handle(int timeout);
#endif /* __VOIP_UTIL_H__ */
