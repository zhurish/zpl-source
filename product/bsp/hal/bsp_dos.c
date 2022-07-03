/*
 * bsp_dos.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "bsp_types.h"

#include "bsp_dos.h"


sdk_dos_t sdk_dos;

static int bsp_dos_enable(void *driver, zpl_uint32 *cmd, zpl_uint32 *value)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_dos.sdk_dos_enable_cb)
		ret = sdk_dos.sdk_dos_enable_cb(bspdev->sdk_driver, *cmd, *value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_dos_tcp_hdr_size(void *driver, zpl_uint32 *cmd, zpl_uint32 *value)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_dos.sdk_dos_tcp_hdr_size_cb)
		ret = sdk_dos.sdk_dos_tcp_hdr_size_cb(bspdev->sdk_driver, *cmd, *value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_dos_icmp_size(void *driver, zpl_uint32 *cmd, zpl_uint32 *value)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_dos.sdk_dos_icmp_size_cb)
		ret = sdk_dos.sdk_dos_icmp_size_cb(bspdev->sdk_driver, *cmd, *value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_dos_icmpv6_size(void *driver, zpl_uint32 *cmd, zpl_uint32 *value)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();	
	if(bspdev->sdk_driver && sdk_dos.sdk_dos_icmpv6_size_cb)
		ret = sdk_dos.sdk_dos_icmpv6_size_cb(bspdev->sdk_driver, *cmd, *value);
	BSP_LEAVE_FUNC();
	return ret;
}


static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_DOS_CMD_NONE, NULL),
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
	zpl_uint32 sdksubcmd = subcmd;
	int i = 0;
	hal_ipcsubcmd_callback_t * callback = NULL;
	BSP_ENTER_FUNC();	
	for(i = 0; i < ZPL_ARRAY_SIZE(subcmd_table); i++)
	{
		if(subcmd_table[i].subcmd == subcmd && subcmd_table[i].cmd_handle)
		{
            callback = &subcmd_table[i];
			break;
		}
	}
	if(callback == NULL)
	{
		zlog_warn(MODULE_HAL, "Can not Find this subcmd:%d ", subcmd);
		BSP_LEAVE_FUNC();
		return OS_NO_CALLBACK;
	}
	hal_ipcmsg_getl(&client->ipcmsg, &dos.value);
	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (callback->cmd_handle)(driver, &sdksubcmd, &dos.value);
	break;
	default:
		break;
	}
	BSP_LEAVE_FUNC();
	return ret;
}
