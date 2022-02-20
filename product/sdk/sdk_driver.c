/*
 * b53125_driver.c
 *
 *  Created on: May 2, 2019
 *      Author: zhurish
 */

#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "bsp_include.h"

#include "sdk_driver.h"
#ifdef ZPL_SDK_BCM53125
#include "b53_driver.h"
#endif
/*************************************************************************/
/****************************    DOS   ***********************************/
#if defined( _SDK_DEBUG_EN)
sdk_driver_t *__msdkdriver;
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

bool is_multicast_ether_addr(const u8 *addr)
{
	return 0x01 & addr[0];
}
/**
 * ether_addr_to_u64 - Convert an Ethernet address into a u64 value.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return a u64 value of the address
 */
zpl_uint64 ether_addr_to_u64(const zpl_uint8 *addr)
{
	zpl_uint64 u = 0;
	zpl_uint32 i;

	for (i = 0; i < ETH_ALEN; i++)
		u = u << 8 | addr[i];

	return u;
}

/**
 * u64_to_ether_addr - Convert a u64 to an Ethernet address.
 * @u: u64 to convert to an Ethernet MAC address
 * @addr: Pointer to a six-byte array to contain the Ethernet address
 */
void u64_to_ether_addr(zpl_uint64 u, zpl_uint8 *addr)
{
	zpl_uint32 i;

	for (i = ETH_ALEN - 1; i >= 0; i--) {
		addr[i] = u & 0xff;
		u = u >> 8;
	}
}


static sdk_driver_t * sdk_driver_malloc(void)
{
	return (sdk_driver_t *)malloc(sizeof(sdk_driver_t));
}

static int sdk_driver_free(sdk_driver_t *sdkdriver)
{
	if(sdkdriver)
		free(sdkdriver);
	sdkdriver = NULL;
	return OK;
}


int sdk_driver_init(struct bsp_driver *bsp, sdk_driver_t *sdkdriver)
{
	bsp->sdk_driver = sdk_driver_malloc();
	if(bsp->sdk_driver)
		return OK;
	else
		return ERROR;	
}

int sdk_driver_start(struct bsp_driver *bsp, sdk_driver_t *sdkdriver)
{
#ifdef ZPL_SDK_BCM53125
	sdkdriver->sdk_device = b53125_device_probe();
	if(sdkdriver->sdk_device)
	{
		if(b53125_config_init(sdkdriver) == OK)
		{
#if defined( _SDK_DEBUG_EN)
			__msdkdriver = sdkdriver;
#endif
		}
	}
#endif
	//sdk_vlan_init();

	b53125_mac_address_read(sdkdriver, 0,  0, 0);
	return OK;
}

int sdk_driver_stop(struct bsp_driver *bsp, sdk_driver_t *sdkdriver)
{
	return OK;
}

int sdk_driver_exit(struct bsp_driver *bsp, sdk_driver_t *sdkdriver)
{
	if(sdkdriver)
		sdk_driver_free(sdkdriver);
	return OK;
}
