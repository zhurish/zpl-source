/*
 * bsp_dos.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */


#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"

#include "bsp_dos.h"


sdk_dos_t sdk_dos;

static int bsp_dos_enable(void *driver, hal_global_header_t *global, zpl_uint32 *value)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();	
	if(driver && sdk_dos.sdk_dos_enable_cb)
		ret = sdk_dos.sdk_dos_enable_cb(driver, global->value, *value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_dos_tcp_hdr_size(void *driver, hal_global_header_t *global, zpl_uint32 *value)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();	
	if(driver && sdk_dos.sdk_dos_tcp_hdr_size_cb)
		ret = sdk_dos.sdk_dos_tcp_hdr_size_cb(driver, global->value, *value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_dos_icmp_size(void *driver, hal_global_header_t *global, zpl_uint32 *value)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();	
	if(driver && sdk_dos.sdk_dos_icmp_size_cb)
		ret = sdk_dos.sdk_dos_icmp_size_cb(driver, global->value, *value);
	SDK_LEAVE_FUNC();
	return ret;
}

static int bsp_dos_icmpv6_size(void *driver, hal_global_header_t *global, zpl_uint32 *value)
{
	int ret = NO_SDK;
	SDK_ENTER_FUNC();	
	if(driver && sdk_dos.sdk_dos_icmpv6_size_cb)
		ret = sdk_dos.sdk_dos_icmpv6_size_cb(driver, global->value, *value);
	SDK_LEAVE_FUNC();
	return ret;
}


static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_IP_LAN_DRIP, bsp_dos_enable),
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_TCP_BLAT_DROP, bsp_dos_enable),
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_UDP_BLAT_DROP, bsp_dos_enable),
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_TCP_NULLSCAN_DROP, bsp_dos_enable),

	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_TCP_XMASSCAN_DROP, bsp_dos_enable),
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_TCP_SYNFINSCAN_DROP, bsp_dos_enable),
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_TCP_SYNERROR_DROP, bsp_dos_enable),
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_TCP_SHORTHDR_DROP, bsp_dos_enable),
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_TCP_FRAGERROR_DROP, bsp_dos_enable),
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_ICMPv4_FRAGMENT_DROP, bsp_dos_enable),
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_ICMPv6_FRAGMENT_DROP, bsp_dos_enable),
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_ICMPv4_LONGPING_DROP, bsp_dos_enable),
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_ICMPv6_LONGPING_DROP, bsp_dos_enable),

	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_TCP_HDR_SIZE, bsp_dos_tcp_hdr_size),
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_ICMPv4_SIZE, bsp_dos_icmp_size),
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_ICMPv6_SIZE, bsp_dos_icmpv6_size),
};


int bsp_dos_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_dos_param_t	dos;
	hal_global_header_t	global;
	SDK_ENTER_FUNC();
	hal_ipcmsg_global_get(&client->ipcmsg, &global);
	hal_ipcmsg_getl(&client->ipcmsg, &dos.value);
	if(!(subcmd_table[subcmd].cmd_handle))
	{
		SDK_LEAVE_FUNC();
		return NO_SDK;
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (subcmd_table[subcmd].cmd_handle)(driver, &global, &dos.value);
	break;
	default:
		break;
	}
	SDK_LEAVE_FUNC();
	return ret;
}
