/*
 * b53125_driver.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */
#ifdef ZPL_SDK_USER
#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "if.h"
#include "vrf.h"
#include "vty.h"
#include "nsm_include.h"
#include "hal_include.h"
#include "bsp_include.h"
#include "sdk_driver.h"
#include "b53_driver.h"
#else
#include "sdk_driver.h"
#include "b53_driver.h"
#include "hal_client.h"
#include "hal_netpkt.h"
#endif
/*************************************************************************/
/****************************    DOS   ***********************************/
sdk_driver_t *__msdkdriver = NULL;
#ifdef ZPL_SDK_USER
#if defined( _SDK_DEBUG_EN)

static const char *sdklog_priority[] = { "emerg", "alert", "crit", "err",
		"warning", "notice", "info", "debug", "trapping", "focetrap", NULL, };

void sdk_log(const char *file, const char *func, const zpl_uint32 line, zpl_uint32 module, zlog_level_t priority, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	time_print(stdout, ZLOG_TIMESTAMP_DATE);
	fprintf(stdout, "%-8s: ", sdklog_priority[priority]);
	zlog_depth_debug_detail(stdout, NULL, 2, file, func, line);
	vfprintf(stdout, format, args);
	fprintf(stdout, "\r\n");
	fflush(stdout);
	va_end(args);
}
#endif
#endif

bool sdk_is_multicast_ether_addr(const u8 *addr)
{
	return 0x01 & addr[0];
}
/**
 * ether_addr_to_u64 - Convert an Ethernet address into a u64 value.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return a u64 value of the address
 */
zpl_uint64 sdk_ether_addr_u64(const zpl_uint8 *addr)
{
	zpl_uint64 u = 0;
	zpl_uint32 i;

	for (i = 0; i < ETH_ALEN; i++)
		u = u << 8 | addr[i];
	if(u != 0)	
		_sdk_err( " sdk_ether_addr_u64:%llx", u);
	return u;
}

/**
 * u64_to_ether_addr - Convert a u64 to an Ethernet address.
 * @u: u64 to convert to an Ethernet MAC address
 * @addr: Pointer to a six-byte array to contain the Ethernet address
 */
void sdk_u64_ether_addr(zpl_uint64 u, zpl_uint8 *addr)
{
	//zpl_uint32 i;
	zpl_uint64 uv = u;
	zpl_uint8 u8val[8];
	memcpy(u8val, &uv, sizeof(zpl_uint64));
	if(u != 0)
		_sdk_err("sdk_u64_ether_addr:%llx -> %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 
			u, u8val[0], u8val[1], u8val[2], u8val[3], u8val[4], u8val[5], u8val[6],
			u8val[7]);
	addr[5] = u8val[0];
	addr[4] = u8val[1];
	addr[3] = u8val[2];
	addr[2] = u8val[3];
	addr[1] = u8val[4];
	addr[0] = u8val[5];
	//addr[0] = u8val[0];

	//for (i = ETH_ALEN - 1; i >= 0; i--) {
	//	addr[i] = u8val[i] & 0xff;
		//u = u >> 8;
	//}
}

int sdk_driver_mac_cache_add(sdk_driver_t *bsp_driver, zpl_uint8 port, zpl_uint8 *mac, vlan_t vid, zpl_uint8 isstatic, zpl_uint8 isage, zpl_uint8 vaild)
{
	int i = 0;
	for(i = 0; i < bsp_driver->mac_cache_max; i++)
	{
		if(bsp_driver->mac_cache_entry[i].use == 1 && (memcmp(bsp_driver->mac_cache_entry[i].mac, mac, ETH_ALEN) == 0))
		{
			bsp_driver->mac_cache_entry[i].port = port;
			bsp_driver->mac_cache_entry[i].vid = vid;
			bsp_driver->mac_cache_entry[i].is_valid = vaild;
			bsp_driver->mac_cache_entry[i].is_age = isage;
			bsp_driver->mac_cache_entry[i].is_static = isstatic;
			bsp_driver->mac_cache_entry[i].use = 1;
			return OK;
		}
	}
	for(i = 0; i < bsp_driver->mac_cache_max; i++)
	{
		if(bsp_driver->mac_cache_entry[i].use == 0)
		{
			bsp_driver->mac_cache_entry[i].port = port;
			bsp_driver->mac_cache_entry[i].vid = vid;
			bsp_driver->mac_cache_entry[i].is_valid = vaild;
			bsp_driver->mac_cache_entry[i].is_age = isage;
			bsp_driver->mac_cache_entry[i].is_static = isstatic;
			bsp_driver->mac_cache_entry[i].use = 1;
			memcpy(bsp_driver->mac_cache_entry[i].mac, mac, ETH_ALEN);
			bsp_driver->mac_cache_num++;
			return OK;
		}
	}
	return OK;
}


int sdk_driver_mac_cache_update(sdk_driver_t *bsp_driver, zpl_uint8 *mac, zpl_uint8 isage)
{
	int i = 0;
	for(i = 0; i < bsp_driver->mac_cache_max; i++)
	{
		if(bsp_driver->mac_cache_entry[i].use == 1 && (memcmp(bsp_driver->mac_cache_entry[i].mac, mac, ETH_ALEN) == 0))
		{
			bsp_driver->mac_cache_entry[i].use = isage?1:0;
			if(isage == 0)
				bsp_driver->mac_cache_num--;
			return OK;
		}
	}
	return OK;
}


#ifdef ZPL_SDK_USER
static sdk_driver_t * sdk_driver_malloc(void)
{
	return (sdk_driver_t *)XMALLOC(MTYPE_SDK, sizeof(sdk_driver_t));
}

static int sdk_driver_free(sdk_driver_t *sdkdriver)
{
	if(sdkdriver)
		XFREE(MTYPE_SDK, sdkdriver);
	sdkdriver = NULL;
	return OK;
}


int sdk_driver_init(struct bsp_driver *bsp, zpl_void *p)
{
	sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
	bsp->sdk_driver = sdk_driver_malloc();
	if(bsp->sdk_driver)
		return OK;
	else
		return ERROR;	
}

int sdk_driver_start(struct bsp_driver *bsp, zpl_void *p)
{
	sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
#ifdef ZPL_SDK_USER
	sdkdriver->sdk_device = b53125_device_probe();
	if(sdkdriver->sdk_device)
	{
		if(b53125_config_start(sdkdriver) == OK)
		{
			__msdkdriver = sdkdriver;
		}
	}
#endif
	//sdk_vlan_init();

	//b53125_mac_address_read(sdkdriver, 0,  0, 0);
	return OK;
}

int sdk_driver_stop(struct bsp_driver *bsp, zpl_void *p)
{
	//sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
	return OK;
}

int sdk_driver_exit(struct bsp_driver *bsp, zpl_void *p)
{
	sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
	if(sdkdriver)
		sdk_driver_free(sdkdriver);
	return OK;
}

#else
static __init int sdk_driver_init(void)
{
	__msdkdriver = XMALLOC(MTYPE_SDK, sizeof(sdk_driver_t));
	if(__msdkdriver)
	{
		__msdkdriver->mac_cache_max = ETH_MAC_CACHE_MAX;
		__msdkdriver->mac_cache_num = 0;
		__msdkdriver->mac_cache_entry = XMALLOC(MTYPE_SDK_DATA, __msdkdriver->mac_cache_max*sizeof(hal_mac_cache_t));

		__msdkdriver->hal_client = hal_client_create(__msdkdriver);
		if(__msdkdriver->hal_client)
		{
			netpkt_netlink_init();
			klog_init();
			__msdkdriver->sdk_device = b53125_device_probe();
			if(__msdkdriver->sdk_device)
			{
				if(b53125_config_start(__msdkdriver) != OK)
				{
					printk("b53125_config_start ERROR\r\n");
					netpkt_netlink_exit();
					klog_exit();
					hal_client_destroy(__msdkdriver->hal_client);
					__msdkdriver->hal_client = NULL;
					if(__msdkdriver->mac_cache_entry)
					{
						XFREE(MTYPE_SDK_DATA, __msdkdriver->mac_cache_entry);
						__msdkdriver->mac_cache_entry = NULL;
					}
					XFREE(MTYPE_SDK, __msdkdriver);
					__msdkdriver = NULL;
					return ERROR;
				}
				return OK;
			}
			printk("b53125_device_probe ERROR\r\n");
			if(__msdkdriver->hal_client)
			{
				netpkt_netlink_exit();
				klog_exit();
				hal_client_destroy(__msdkdriver->hal_client);
				__msdkdriver->hal_client = NULL;
				if(__msdkdriver->mac_cache_entry)
				{
					XFREE(MTYPE_SDK_DATA, __msdkdriver->mac_cache_entry);
					__msdkdriver->mac_cache_entry = NULL;
				}
				XFREE(MTYPE_SDK, __msdkdriver);
				__msdkdriver = NULL;
				return ERROR;
			}
		}
		else
		{
			printk("hal_client_create ERROR\r\n");
			return ERROR;
		}	
	}
	return OK;	
}

static __exit void sdk_driver_exit(void)
{
	if(__msdkdriver)
	{
		if(__msdkdriver->sdk_device)
		{
			b53125_device_exit(__msdkdriver->sdk_device);
			__msdkdriver->sdk_device = NULL;
		}
		if(__msdkdriver->hal_client)
		{
			hal_client_destroy(__msdkdriver->hal_client);
			__msdkdriver->hal_client = NULL;
		}
		if(__msdkdriver->mac_cache_entry)
		{
			XFREE(MTYPE_SDK_DATA, __msdkdriver->mac_cache_entry);
			__msdkdriver->mac_cache_entry = NULL;
		}
		netpkt_netlink_exit();
		klog_exit();
		XFREE(MTYPE_SDK, __msdkdriver);
		__msdkdriver = NULL;
	}
	return ;
}


module_init(sdk_driver_init);
module_exit(sdk_driver_exit);

MODULE_AUTHOR("zhurish <zhurish@163.com>");
MODULE_DESCRIPTION("B53 switch sdk library");
MODULE_LICENSE("Dual BSD/GPL");
#endif