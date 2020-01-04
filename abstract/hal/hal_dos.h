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
	int (*sdk_dos_enable_cb) (void *, BOOL, dos_type_en);
	int (*sdk_dos_tcp_hdr_size_cb) (void *, int);
	int (*sdk_dos_icmp_size_cb) (void *, BOOL, int);
	void *sdk_driver;
}sdk_dos_t;

extern sdk_dos_t sdk_dos;


int hal_dos_enable(BOOL enable, dos_type_en type);
int hal_dos_tcp_hdr_size(int size);
int hal_dos_icmp_size(BOOL ipv6, int size);



#endif /* __HAL_DOS_H__ */
