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

enum hal_dos_cmd 
{
    HAL_DOS_CMD_NONE,
	HAL_DOS_CMD_IP_LAN_DRIP,
	HAL_DOS_CMD_TCP_BLAT_DROP,
	HAL_DOS_CMD_UDP_BLAT_DROP,
	HAL_DOS_CMD_TCP_NULLSCAN_DROP,
	HAL_DOS_CMD_TCP_XMASSCAN_DROP,
	HAL_DOS_CMD_TCP_SYNFINSCAN_DROP,
	HAL_DOS_CMD_TCP_SYNERROR_DROP,

	HAL_DOS_CMD_TCP_SHORTHDR_DROP,
	HAL_DOS_CMD_TCP_FRAGERROR_DROP,
	HAL_DOS_CMD_ICMPv4_FRAGMENT_DROP,
	HAL_DOS_CMD_ICMPv6_FRAGMENT_DROP,

	HAL_DOS_CMD_ICMPv4_LONGPING_DROP,
	HAL_DOS_CMD_ICMPv6_LONGPING_DROP,

	HAL_DOS_CMD_TCP_HDR_SIZE,
	HAL_DOS_CMD_ICMPv4_SIZE,
	HAL_DOS_CMD_ICMPv6_SIZE,
};

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
