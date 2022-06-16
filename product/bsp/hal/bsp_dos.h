/*
 * bsp_dos.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_DOS_H__
#define __BSP_DOS_H__
#ifdef __cplusplus
extern "C" {
#endif
#ifndef ZPL_SDK_USER

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
#endif

typedef struct sdk_dos_s
{
	int (*sdk_dos_enable_cb) (void *, zpl_uint32, zpl_bool);
	int (*sdk_dos_tcp_hdr_size_cb) (void *, zpl_uint32, zpl_uint32);
	int (*sdk_dos_icmp_size_cb) (void *, zpl_uint32, zpl_uint32);
	int (*sdk_dos_icmpv6_size_cb) (void *, zpl_uint32, zpl_uint32);
}sdk_dos_t;

extern sdk_dos_t sdk_dos;

extern int bsp_dos_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_DOS_H__ */
