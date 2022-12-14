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

#include "nsm_dos.h"



typedef struct hal_dos_param_s
{
	zpl_uint32 value;
}hal_dos_param_t;


int hal_dos_enable(zpl_bool enable, dos_type_en type);
int hal_dos_tcp_hdr_size(zpl_uint32 size);
int hal_dos_icmp_size(zpl_bool ipv6, zpl_uint32 size);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_DOS_H__ */
