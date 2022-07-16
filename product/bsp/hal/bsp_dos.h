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
