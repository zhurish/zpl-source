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
#define XNLMSG_NEW(t, s)    nlmsg_new(s, GFP_KERNEL)

static inline void *XMALLOC(int type, int size)
{
    void *p = kmalloc(size, GFP_KERNEL);
    if(p)
    {
        memset(p, 0, size);
        return p;
    }
    return p;
}

static inline void *XREALLOC(int type, void *r, int size)
{
    void *p = krealloc(r, size, GFP_KERNEL);
    if(p && r == NULL)
    {
        memset(p, 0, size);
        return p;
    }
    return p;
}
//#define XMALLOC(t, s)       kmalloc(s, GFP_KERNEL)
//#define XREALLOC(t, p, s)       krealloc(p, s, GFP_KERNEL)


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

enum zpl_debug_cmd
{
    KLOG_LEVEL,
    NETPKT_DEBUG,
    HALCLIENT_DEBUG,
 	HAL_SDK_REG8,
	HAL_SDK_REG16,
	HAL_SDK_REG32,
	HAL_SDK_REG64,
    NETPKT_DEST,
    NETPKT_BIND,
    KLOG_DEST,
};

#define NO_SDK ERROR

#define ZPL_ARRAY_SIZE(x) (int)(sizeof(x) / sizeof(x[0]))


#ifndef NSM_MAC_MAX
#define NSM_MAC_MAX	6
#endif


#ifndef VLAN_TABLE_MAX
#define VLAN_TABLE_MAX 4096
#endif


#include "kbsp_types.h"
#include "hal_client.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BSP_TYPES_H__ */
