/*
 * bsp_route.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"
#include "bsp_route.h"
#include "bsp_driver.h"


sdk_route_t sdk_route;


static int bsp_route_multipath_add(void *driver, hal_route_param_t *param, void *p)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_route.sdk_route_add_cb)
		ret = sdk_route.sdk_route_add_cb(driver, param);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_route_multipath_del(void *driver, hal_route_param_t *param, void *p)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_route.sdk_route_del_cb)
		ret = sdk_route.sdk_route_del_cb(driver, param);
	BSP_LEAVE_FUNC();
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_ROUTE_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_ROUTE_ADD, bsp_route_multipath_add),
	HAL_CALLBACK_ENTRY(HAL_ROUTE_DEL, bsp_route_multipath_del),
};

int bsp_route_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK, i = 0;
	hal_route_param_t	param;
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

	memset(&param, 0, sizeof(param));
	hal_ipcmsg_getc(&client->ipcmsg, &param.safi);
	hal_ipcmsg_getw(&client->ipcmsg, &param.vrf_id);
	hal_ipcmsg_getc(&client->ipcmsg, &param.family);
	hal_ipcmsg_getl(&client->ipcmsg, &param.table);
	hal_ipcmsg_getc(&client->ipcmsg, &param.prefixlen);
	if (param.family == IPSTACK_AF_INET)
	{
		hal_ipcmsg_getl(&client->ipcmsg, &param.destination.ipv4.s_addr);
		hal_ipcmsg_getl(&client->ipcmsg, &param.source.ipv4.s_addr);
	}
#ifdef ZPL_BUILD_IPV6
	else
	{
		hal_ipcmsg_get(&client->ipcmsg, (zpl_uchar *)&param.destination.ipv6, IPV6_MAX_BYTELEN);
		hal_ipcmsg_get(&client->ipcmsg, (zpl_uchar *)&param.source.ipv6, IPV6_MAX_BYTELEN);
	}
#endif

	hal_ipcmsg_getc(&client->ipcmsg, &param.nexthop_num);
	if(param.nexthop_num)
	{
		for(i = 0; i < param.nexthop_num; i++)
		{
			hal_ipcmsg_getl(&client->ipcmsg, &param.nexthop[i].kifindex);
			hal_ipcmsg_port_get(&client->ipcmsg, &param.nexthop[i].port);
			if (param.family == IPSTACK_AF_INET)
			{
				hal_ipcmsg_getl(&client->ipcmsg, &param.nexthop[i].gateway.ipv4.s_addr);
				//hal_ipcmsg_getl(&client->ipcmsg, &param.nexthop[i].source.ipv4.s_addr);
			}
#ifdef ZPL_BUILD_IPV6				
			else	
			{
				hal_ipcmsg_get(&client->ipcmsg, (zpl_uchar *)&param.nexthop[i].gateway.ipv6, IPV6_MAX_BYTELEN);
				//hal_ipcmsg_get(&client->ipcmsg, (zpl_uchar *)&param.nexthop[i].source.ipv6, IPV6_MAX_BYTELEN);
			}
#endif
		}
	}
	

	hal_ipcmsg_getc(&client->ipcmsg, &param.processid); // processid
	hal_ipcmsg_getc(&client->ipcmsg, &param.type);
	hal_ipcmsg_getc(&client->ipcmsg, &param.flags);
	hal_ipcmsg_getc(&client->ipcmsg, &param.distance);
	hal_ipcmsg_getl(&client->ipcmsg, &param.metric);
	hal_ipcmsg_getl(&client->ipcmsg, &param.tag);
	hal_ipcmsg_getl(&client->ipcmsg, &param.mtu);


	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (callback->cmd_handle)(driver, &param, NULL);
	break;
	default:
		break;
	}
	BSP_LEAVE_FUNC();
	return ret;
}
