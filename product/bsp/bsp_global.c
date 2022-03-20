/*
 * bsp_global.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */



#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"
#include "bsp_global.h"

sdk_global_t sdk_global;
sdk_snooping_t sdk_snooping;

static int bsp_global_jumbo_size(void *driver, void *bsp, zpl_uint32 *size)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_global.sdk_jumbo_size_cb)
		ret = sdk_global.sdk_jumbo_size_cb(driver, *size);
	BSP_LEAVE_FUNC();
	return ret;
}


/*
 * Global
 */
static int bsp_switch_mode(void *driver, hal_global_header_t *global, zpl_bool *manage)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_global.sdk_switch_manege_cb)
		ret = sdk_global.sdk_switch_manege_cb(driver, *manage);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_switch_forward(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_global.sdk_switch_forward_cb)
		ret = sdk_global.sdk_switch_forward_cb(driver, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}


static int bsp_multicast_flood(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_global.sdk_multicast_flood_cb)
		ret = sdk_global.sdk_multicast_flood_cb(driver, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_unicast_flood(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_global.sdk_unicast_flood_cb)
		ret = sdk_global.sdk_unicast_flood_cb(driver, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}


static int bsp_multicast_learning(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_global.sdk_multicast_learning_cb)
		ret = sdk_global.sdk_multicast_learning_cb(driver, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}


static int bsp_global_bpdu_enable(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_global.sdk_bpdu_enable_cb)
		ret = sdk_global.sdk_bpdu_enable_cb(driver, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}


static int bsp_global_aging_time(void *driver, hal_global_header_t *global, zpl_uint32 *value)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_global.sdk_aging_time_cb)
		ret = sdk_global.sdk_aging_time_cb(driver, *value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_global_wan_port(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_global.sdk_wan_port_cb)
		ret = sdk_global.sdk_wan_port_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_GLOBAL_CMD_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_GLOBAL_CMD_JUMBO_SIZE, bsp_global_jumbo_size),
	HAL_CALLBACK_ENTRY(HAL_GLOBAL_MANEGE, bsp_switch_mode),
	HAL_CALLBACK_ENTRY(HAL_GLOBAL_FORWARD, bsp_switch_forward),
	HAL_CALLBACK_ENTRY(HAL_GLOBAL_MULTICAST_FLOOD, bsp_multicast_flood),
	HAL_CALLBACK_ENTRY(HAL_GLOBAL_UNICAST_FLOOD, bsp_unicast_flood),
	HAL_CALLBACK_ENTRY(HAL_GLOBAL_MULTICAST_LEARNING, bsp_multicast_learning),
	HAL_CALLBACK_ENTRY(HAL_GLOBAL_BPDU, bsp_global_bpdu_enable),
	HAL_CALLBACK_ENTRY(HAL_GLOBAL_AGINT, bsp_global_aging_time),
	HAL_CALLBACK_ENTRY(HAL_GLOBAL_WAN_PORT, bsp_global_wan_port),
	
};



int bsp_global_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	zpl_uint32 value = 0;
	BSP_ENTER_FUNC();
	ret = bsp_driver_module_check(subcmd_table, sizeof(subcmd_table)/sizeof(subcmd_table[0]), subcmd);
	if(ret == 0)
	{
		BSP_LEAVE_FUNC();
		return NO_SDK;
	}
	hal_ipcmsg_getl(&client->ipcmsg, &value);

	if(!(subcmd_table[subcmd].cmd_handle))
	{
		BSP_LEAVE_FUNC();
		return NO_SDK;
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (subcmd_table[subcmd].cmd_handle)(driver, NULL, &value);
	break;
	default:
		break;
	}
	BSP_LEAVE_FUNC();
	return ret;
}

/******************************************************************************/
static int bsp_igmp_snooping_set(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_snooping.sdk_igmp_snoop_cb)
		ret = sdk_snooping.sdk_igmp_snoop_cb(driver, global->value, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_igmpunknow_snooping_set(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_snooping.sdk_igmpunknow_snoop_cb)
		ret = sdk_snooping.sdk_igmpunknow_snoop_cb(driver, global->value, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_igmpqry_snooping_set(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_snooping.sdk_igmpqry_snoop_cb)
		ret = sdk_snooping.sdk_igmpqry_snoop_cb(driver, global->value, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mld_snooping_set(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_snooping.sdk_mld_snoop_cb)
		ret = sdk_snooping.sdk_mld_snoop_cb(driver, global->value, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_mldqry_snooping_set(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_snooping.sdk_mldqry_snoop_cb)
		ret = sdk_snooping.sdk_mldqry_snoop_cb(driver, global->value, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_igmp_ipcheck_enable(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_snooping.sdk_igmp_ipcheck_cb)
		ret = sdk_snooping.sdk_igmp_ipcheck_cb(driver, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_arp_snooping_set(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_snooping.sdk_arp_snoop_cb)
		ret = sdk_snooping.sdk_arp_snoop_cb(driver, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}
static int bsp_rarp_snooping_set(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_snooping.sdk_rarp_snoop_cb)
		ret = sdk_snooping.sdk_rarp_snoop_cb(driver, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_dhcp_snooping_set(void *driver, hal_global_header_t *global, zpl_bool *enable)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();	
	if(driver && sdk_snooping.sdk_dhcp_snoop_cb)
		ret = sdk_snooping.sdk_dhcp_snoop_cb(driver, *enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_snoop_table[] = {
	HAL_CALLBACK_ENTRY(HAL_IGMP_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_IGMP_IPCHECK, bsp_igmp_ipcheck_enable),
	HAL_CALLBACK_ENTRY(HAL_IGMP_SNOOPING, bsp_igmp_snooping_set),
	HAL_CALLBACK_ENTRY(HAL_IGMPQRY_SNOOPING, bsp_igmpqry_snooping_set),
	HAL_CALLBACK_ENTRY(HAL_IGMPUNKNOW_SNOOPING, bsp_igmpunknow_snooping_set),
	HAL_CALLBACK_ENTRY(HAL_MLD_SNOOPING, bsp_mld_snooping_set),
	HAL_CALLBACK_ENTRY(HAL_MLDQRY_SNOOPING, bsp_mldqry_snooping_set),
	HAL_CALLBACK_ENTRY(HAL_ARP_COPYTOCPU, bsp_arp_snooping_set),
	HAL_CALLBACK_ENTRY(HAL_RARP_COPYTOCPU, bsp_rarp_snooping_set),
	HAL_CALLBACK_ENTRY(HAL_DHCP_COPYTOCPU, bsp_dhcp_snooping_set),
};


int bsp_snooping_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	zpl_uint32 value = 0;
	BSP_ENTER_FUNC();
	ret = bsp_driver_module_check(subcmd_snoop_table, sizeof(subcmd_snoop_table)/sizeof(subcmd_snoop_table[0]), subcmd);
	if(ret == 0)
	{
		BSP_LEAVE_FUNC();
		return NO_SDK;
	}
	hal_ipcmsg_getl(&client->ipcmsg, &value);

	if(!(subcmd_snoop_table[subcmd].cmd_handle))
	{
		BSP_LEAVE_FUNC();
		return NO_SDK;
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (subcmd_snoop_table[subcmd].cmd_handle)(driver, NULL, &value);
	break;
	default:
		break;
	}
	BSP_LEAVE_FUNC();
	return ret;
}