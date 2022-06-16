/*
 * bsp_types.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef __BSP_TYPES_H__
#define __BSP_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_SDK_USER
#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_include.h"
#include "hal_client.h"
#include "bsp_driver.h"
#include "bsp_include.h"

#define msleep  os_msleep
#define usleep_range(a,b)   os_usleep((a+b)/2)
#else
#include "plconfig.h"
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/brcmphy.h>
#include <linux/rtnetlink.h>
#include <net/netlink.h>
#include <net/sock.h>


#define IPSTACK_AF_INET     AF_INET


#define XFREE(t, p)         kfree(p)
#define XMALLOC(t, s)       kmalloc(s, GFP_KERNEL)
#define XREALLOC(t, p, s)       krealloc(p, s, GFP_KERNEL)
#define XNLMSG_NEW(t, s)    nlmsg_new(s, GFP_KERNEL)

enum zpl_memory_type
{
    MTYPE_HALIPCSRV,
    MTYPE_HALIPCCLIENT,
    MTYPE_HALIPCMSG,
    MTYPE_IPCBC,
    MTYPE_IPCBCCLIENT,
    MTYPE_BSP,
    MTYPE_BSP_CLIENT,
    MTYPE_BSP_SERV,
    MTYPE_BSP_DATA,
    MTYPE_SDK,
    MTYPE_SDK_CLIENT,
    MTYPE_SDK_SERV,
    MTYPE_SDK_DATA,
};


/* MODULE */
enum zpl_module
{
    MODULE_HAL = 0,
    MODULE_BSP = 1,
    MODULE_SDK = 2,
};

/* LOG MODULE */
#define kzlog_emerg(m, fmt, ...) { printk(KERN_EMERG fmt, ##__VA_ARGS__);printk("\r\n");}
#define kzlog_err(m, fmt, ...) { printk(KERN_ERR fmt, ##__VA_ARGS__);printk("\r\n");}
#define kzlog_warn(m, fmt, ...) { printk(KERN_WARNING fmt, ##__VA_ARGS__);printk("\r\n");}
#define kzlog_notice(m, fmt, ...) { printk(KERN_NOTICE fmt, ##__VA_ARGS__);printk("\r\n");}
#define kzlog_info(m, fmt, ...) { printk(KERN_INFO fmt, ##__VA_ARGS__);printk("\r\n");}
#define kzlog_debug(m, fmt, ...) { printk(KERN_DEBUG fmt, ##__VA_ARGS__);printk("\r\n");}

#define zlog_emerg kzlog_emerg
#define zlog_err kzlog_err
#define zlog_warn kzlog_warn
#define zlog_notice kzlog_notice
#define zlog_info kzlog_info
#define zlog_debug kzlog_debug

#include "kbsp_types.h"
#include "hal_client.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BSP_TYPES_H__ */
