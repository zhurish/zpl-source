/*
 * sdk_driver.h
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#ifndef __SDK_DRIVER_H__
#define __SDK_DRIVER_H__

#ifdef	__cplusplus
extern "C" {
#endif
#ifdef ZPL_SDK_USER
#include "auto_include.h"
#include <zplos_include.h>
#include "nsm_include.h"
#include "lib_include.h"
#include "vty_include.h"
#include "hal_include.h"
#include "bsp_include.h"
#include "bsp_driver.h"
#include "bsp_types.h"
#else
#include <linux/kernel.h>
#include <linux/phy.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/brcmphy.h>
#include <linux/rtnetlink.h>

#include "bsp_types.h"
#include "hal_ipcmsg.h"
#include "hal_ipccmd.h"
#include "hal_client.h"
#endif
#ifdef ZPL_SDK_USER
typedef zpl_uint8 u8;
typedef zpl_uint16 u16;
typedef zpl_uint32 u32;
typedef zpl_uint64 u64;
#define _SDK_CLI_DEBUG_EN
#endif



#ifdef ZPL_SDK_USER
#define sdk_err(format, ...) 				pl_zlog_err (__FILE__, __FUNCTION__, __LINE__, MODULE_SDK, format, ##__VA_ARGS__)
#define sdk_warn(format, ...) 				pl_zlog_warn (__FILE__, __FUNCTION__, __LINE__, MODULE_SDK, format, ##__VA_ARGS__)
#define sdk_info(format, ...) 				pl_zlog_info (__FILE__, __FUNCTION__, __LINE__, MODULE_SDK, format, ##__VA_ARGS__)
#define sdk_notice(format, ...) 			pl_zlog_notice (__FILE__, __FUNCTION__, __LINE__, MODULE_SDK, format, ##__VA_ARGS__)
#define sdk_debug(format, ...) 				pl_zlog_debug (__FILE__, __FUNCTION__, __LINE__, MODULE_SDK, format, ##__VA_ARGS__)
#define sdk_trap(format, ...) 				pl_zlog_trap (__FILE__, __FUNCTION__, __LINE__, MODULE_SDK, format, ##__VA_ARGS__)
#endif

/* Debug flags. */
#define SDK_DEBUG_EVENT 0x01
#define SDK_DEBUG_DETAIL 0x80

/* Debug related macro. */
#define sdk_debug_detail(dev, format, ...) {if(dev->debug&SDK_DEBUG_DETAIL) sdk_debug(format, ##__VA_ARGS__);}
#define sdk_debug_event(dev, format, ...) {if(dev->debug&SDK_DEBUG_EVENT) sdk_debug(format, ##__VA_ARGS__);}

#define sdk_handle_return(ret) 	{if(dev->debug&SDK_DEBUG_EVENT) sdk_debug("%s %s", __func__, (ret == OK)?"OK":"ERROR");}









typedef struct sdk_driver {

	zpl_phyport_t 	cpu_port;
	zpl_bool 	vlan_enabled;
	void 			*sdk_device;
#ifdef ZPL_SDK_KERNEL
	struct hal_client *hal_client;
#endif
	zpl_uint32 debug;
}sdk_driver_t;


extern sdk_driver_t *__msdkdriver;
extern zpl_uint64 sdk_ether_addr_u64(const zpl_uint8 *addr);
extern void sdk_u64_ether_addr(zpl_uint64 u, zpl_uint8 *addr);
extern bool sdk_is_multicast_ether_addr(const u8 *addr);



#ifdef ZPL_SDK_USER
extern int sdk_driver_init(struct bsp_driver *, zpl_void *);
extern int sdk_driver_start(struct bsp_driver *, zpl_void *);
extern int sdk_driver_stop(struct bsp_driver *, zpl_void *);
extern int sdk_driver_exit(struct bsp_driver *, zpl_void *);
#endif

 
#ifdef __cplusplus
}
#endif

#endif /* __SDK_DRIVER_H__ */
