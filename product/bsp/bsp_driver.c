/*
 * bsp_driver.c
 *
 *  Created on: 2019年9月8日
 *      Author: zhurish
 */

#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"

#include "bsp_driver.h"
#include "bsp_include.h"

#ifdef ZPL_SDK_MODULE
#include "sdk_driver.h"
#endif


bsp_driver_t *bsp_driver = NULL;

struct module_list module_list_sdk = 
{ 
	.module=MODULE_SDK, 
	.name="SDK", 
	.module_init=bsp_module_init, 
	.module_exit=NULL, 
	.module_task_init=NULL, 
	.module_task_exit=NULL, 
	.module_cmd_init=NULL, 
	.module_write_config=NULL, 
	.module_show_config=NULL,
	.module_show_debug=NULL, 
	.taskid=0,
};


static hal_ipccmd_callback_t moduletable[] = {
    HAL_CALLBACK_ENTRY(HAL_MODULE_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_MODULE_MGT, NULL),
	HAL_CALLBACK_ENTRY(HAL_MODULE_GLOBAL, bsp_global_module_handle),
	HAL_CALLBACK_ENTRY(HAL_MODULE_SWITCH, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_CPU, bsp_cpu_module_handle),
	HAL_CALLBACK_ENTRY(HAL_MODULE_PORT, bsp_port_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_IFADDR, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_ROUTE, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_STP, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_MSTP, bsp_mstp_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_8021X, bsp_8021x_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_DOS, bsp_dos_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_MAC, bsp_mac_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_MIRROR, bsp_mirror_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_QINQ, bsp_qinq_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_VLAN, bsp_vlan_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_QOS, bsp_qos_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_ACL, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_TRUNK, bsp_trunk_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_ARP, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_BRIDGE, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_PPP, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_SECURITY, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_SNMP, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_VRF, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_MPLS, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_STATISTICS, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_EVENT, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_STATUS, NULL),
};


int bsp_module_init()
{

	if(bsp_driver)
		return OK;
	bsp_driver = malloc(sizeof(bsp_driver_t));
	if(bsp_driver == NULL)
	{
		zlog_debug(MODULE_HAL, " Can not malloc bsp_driver_t");
		return ERROR;
	}
	memset(bsp_driver, 0, sizeof(bsp_driver_t));

#ifdef ZPL_SDK_MODULE
	sdk_module_init(bsp_driver);
#endif

	return OK;
}




int bsp_client_msg_handle(struct hal_client *client, zpl_uint32 cmd, void *driver)
{
	int ret = 0;
	int module = IPCCMD_MODULE_GET(cmd);
	if(module > HAL_MODULE_NONE && module < HAL_MODULE_MAX && (moduletable[module].module_handle))
	{
		ret = (moduletable[module].module_handle)(client, IPCCMD_CMD_GET(cmd), IPCCMD_SUBCMD_GET(cmd), driver);
	}
	return ret;
}
