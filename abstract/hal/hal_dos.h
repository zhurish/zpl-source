/*
 * hal_dos.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_DOS_H__
#define __HAL_DOS_H__



typedef struct sdk_dos_s
{
	int (*sdk_dos_enable_cb) (BOOL, int);
	int (*sdk_dos_tcp_hdr_size_cb) (int);
	int (*sdk_dos_icmp_size_cb) (BOOL, int);
}sdk_dos_t;

int hal_dos_enable(BOOL enable, int type);
int hal_dos_tcp_hdr_size(int size);
int hal_dos_icmp_size(BOOL ipv6, int size);



#endif /* __HAL_DOS_H__ */
