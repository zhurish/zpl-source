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
#if defined( _SDK_DEBUG_EN)
extern void sdk_log(const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, zlog_level_t priority, const char *format, ...);
#define _sdk_debug(format, ...) sdk_log (__FILE__, __FUNCTION__, __LINE__, MODULE_SDK, ZLOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define _sdk_warn(format, ...) 	sdk_log (__FILE__, __FUNCTION__, __LINE__, MODULE_SDK, ZLOG_LEVEL_WARNING, format, ##__VA_ARGS__)
#define _sdk_err(format, ...) 	sdk_log (__FILE__, __FUNCTION__, __LINE__, MODULE_SDK, ZLOG_LEVEL_ERR, format, ##__VA_ARGS__)
#define _sdk_info(format, ...)	sdk_log(__FILE__, __FUNCTION__, __LINE__, MODULE_SDK, ZLOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define _sdk_trap(format, ...)	sdk_log(__FILE__, __FUNCTION__, __LINE__, MODULE_SDK, ZLOG_LEVEL_TRAP, format, ##__VA_ARGS__)
#define _sdk_notice(format, ...) sdk_log(__FILE__, __FUNCTION__, __LINE__, MODULE_SDK, ZLOG_LEVEL_NOTICE, format, ##__VA_ARGS__)
#else
#define _sdk_debug(format, ...)	zlog_debug(MODULE_SDK,format, ##__VA_ARGS__)
#define _sdk_warn(format, ...)	zlog_warn(MODULE_SDK,format, ##__VA_ARGS__)
#define _sdk_err(format, ...)	zlog_err(MODULE_SDK,format, ##__VA_ARGS__)
#define _sdk_info(format, ...)	zlog_info(MODULE_SDK,format, ##__VA_ARGS__)
#define _sdk_trap(format, ...)	zlog_trap(MODULE_SDK,format, ##__VA_ARGS__)
#define _sdk_notice(format, ...) zlog_notice(MODULE_SDK,format, ##__VA_ARGS__)
#endif
#else
#define _sdk_debug(format, ...)	printk(format, ##__VA_ARGS__)
#define _sdk_warn(format, ...)	printk(format, ##__VA_ARGS__)
#define _sdk_err(format, ...)	printk(format, ##__VA_ARGS__)
#define _sdk_info(format, ...)	printk(format, ##__VA_ARGS__)
#define _sdk_trap(format, ...)	printk(format, ##__VA_ARGS__)
#define _sdk_notice(format, ...) printk(format, ##__VA_ARGS__)
#endif


#define ETH_ALEN	6

typedef struct sdk_driver {

	zpl_phyport_t 	cpu_port;
	zpl_uint32 		num_vlans;
	zpl_phyport_t 	num_ports;
	zpl_phyport_t	ports_table[8];
	void 			*sdk_device;
#ifndef ZPL_SDK_USER
	struct hal_client *hal_client;
#endif
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
