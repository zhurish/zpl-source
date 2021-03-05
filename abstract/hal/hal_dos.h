/*
 * hal_dos.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_DOS_H__
#define __HAL_DOS_H__
#ifdef __cplusplus
extern "C" {
#endif


typedef struct sdk_dos_s
{
	int (*sdk_dos_enable_cb) (void *, ospl_bool, dos_type_en);
	int (*sdk_dos_tcp_hdr_size_cb) (void *, ospl_uint32);
	int (*sdk_dos_icmp_size_cb) (void *, ospl_bool, ospl_uint32);
	void *sdk_driver;
}sdk_dos_t;

extern sdk_dos_t sdk_dos;


int hal_dos_enable(ospl_bool enable, dos_type_en type);
int hal_dos_tcp_hdr_size(ospl_uint32 size);
int hal_dos_icmp_size(ospl_bool ipv6, ospl_uint32 size);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_DOS_H__ */
