/*
 * bsp_driver.c
 *
 *  Created on: 2019年9月8日
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "if.h"
#include "vrf.h"
#include "vty.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"

#include "bsp_driver.h"
#include "bsp_include.h"

#ifdef ZPL_SDK_MODULE
#include "sdk_driver.h"
#endif


#ifndef ZPL_SDK_MODULE
sdk_driver_t *__msdkdriver = NULL;
#endif
bsp_driver_t bsp_driver;

struct module_list module_list_sdk = 
{ 
	.module=MODULE_SDK, 
	.name="SDK\0", 
	.module_init=bsp_module_init, 
	.module_exit=bsp_module_exit, 
	.module_task_init=bsp_module_task_init, 
	.module_task_exit=bsp_module_task_exit, 
	.module_cmd_init=NULL, 
	.taskid=0,
};


static hal_ipccmd_callback_t moduletable[] = {
    HAL_CALLBACK_ENTRY(HAL_MODULE_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_MODULE_MGT, NULL),
	HAL_CALLBACK_ENTRY(HAL_MODULE_GLOBAL, bsp_global_module_handle),
	HAL_CALLBACK_ENTRY(HAL_MODULE_SWITCH, NULL),
    HAL_CALLBACK_ENTRY(HAL_MODULE_CPU, bsp_cpu_module_handle),
	HAL_CALLBACK_ENTRY(HAL_MODULE_PORT, bsp_port_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_L3IF, bsp_l3if_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_ROUTE, bsp_route_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_STP, NULL),
#ifdef ZPL_NSM_MSTP
    HAL_CALLBACK_ENTRY(HAL_MODULE_MSTP, bsp_mstp_module_handle),
#else
	HAL_CALLBACK_ENTRY(HAL_MODULE_MSTP, NULL),
#endif
#ifdef ZPL_NSM_8021X	
    HAL_CALLBACK_ENTRY(HAL_MODULE_8021X, bsp_8021x_module_handle),
#else
	HAL_CALLBACK_ENTRY(HAL_MODULE_8021X, NULL),
#endif
#ifdef ZPL_NSM_IGMP
	HAL_CALLBACK_ENTRY(HAL_MODULE_IGMP, bsp_snooping_module_handle),
#else	
	HAL_CALLBACK_ENTRY(HAL_MODULE_IGMP, NULL),
#endif
#ifdef ZPL_NSM_DOS
    HAL_CALLBACK_ENTRY(HAL_MODULE_DOS, bsp_dos_module_handle),
#else
	HAL_CALLBACK_ENTRY(HAL_MODULE_DOS, NULL),
#endif
#ifdef ZPL_NSM_MAC
    HAL_CALLBACK_ENTRY(HAL_MODULE_MAC, bsp_mac_module_handle),
#else
	HAL_CALLBACK_ENTRY(HAL_MODULE_MAC, NULL),
#endif
#ifdef ZPL_NSM_MIRROR
    HAL_CALLBACK_ENTRY(HAL_MODULE_MIRROR, bsp_mirror_module_handle),
#else
	HAL_CALLBACK_ENTRY(HAL_MODULE_MIRROR, NULL),
#endif

#ifdef ZPL_NSM_VLAN
	HAL_CALLBACK_ENTRY(HAL_MODULE_QINQ, bsp_qinq_module_handle),
    HAL_CALLBACK_ENTRY(HAL_MODULE_VLAN, bsp_vlan_module_handle),
#else
	HAL_CALLBACK_ENTRY(HAL_MODULE_QINQ, NULL),
	HAL_CALLBACK_ENTRY(HAL_MODULE_VLAN, NULL),
#endif
#ifdef ZPL_NSM_QOS
    HAL_CALLBACK_ENTRY(HAL_MODULE_QOS, bsp_qos_module_handle),
#else
	HAL_CALLBACK_ENTRY(HAL_MODULE_QOS, NULL),
#endif
    HAL_CALLBACK_ENTRY(HAL_MODULE_ACL, NULL),
#ifdef ZPL_NSM_TRUNK	
    HAL_CALLBACK_ENTRY(HAL_MODULE_TRUNK, bsp_trunk_module_handle),
#else
	HAL_CALLBACK_ENTRY(HAL_MODULE_TRUNK, NULL),
#endif
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


int bsp_driver_msg_handle(struct hal_client *client, zpl_uint32 cmd, void *driver)
{
	int ret = 0;
	int module = IPCCMD_MODULE_GET(cmd);
	BSP_ENTER_FUNC();
	if(module > HAL_MODULE_NONE && module < HAL_MODULE_MAX && (moduletable[module].module_handle))
	{
		hal_ipccmd_callback_t * callback = hal_ipccmd_callback_get(moduletable, sizeof(moduletable)/sizeof(moduletable[0]), module);
		if(callback)
			ret = (callback->module_handle)(client, IPCCMD_CMD_GET(cmd), IPCCMD_SUBCMD_GET(cmd), driver);
		else
			ret = OS_NO_CALLBACK;	
	}
	BSP_LEAVE_FUNC();
	return ret;
}

int bsp_module_func(bsp_driver_t *bspdriver, bsp_sdk_func init_func, bsp_sdk_func start_func, 
	bsp_sdk_func stop_func, bsp_sdk_func exit_func)
{
    bspdriver->bsp_sdk_init = init_func;
    bspdriver->bsp_sdk_start = start_func;
    bspdriver->bsp_sdk_stop = stop_func;
    bspdriver->bsp_sdk_exit = exit_func;
	return OK;
}


static int bsp_driver_task(void *p)
{
  // int rc = 0;
  struct thread_master *master = (struct thread_master *)p;
  module_setup_task(master->module, os_task_id_self());
  // host_waitting_loadconfig();
  while (thread_mainloop(master))
    ;
  return OK;
}

int bsp_driver_init(bsp_driver_t *bspdriver)
{
  struct hal_client *bsp = hal_client_create(MODULE_BSP, zpl_false);
  if (bsp)
  {
    bsp->debug = 0xffff;
    bspdriver->hal_client = bsp;
    bsp->master = bspdriver->master = thread_master_module_create(MODULE_BSP);
  
    hal_client_start(bsp);
    return OK;
  }
  return ERROR;
}

int bsp_driver_exit(bsp_driver_t *bspdriver)
{
  hal_client_destroy(bspdriver->hal_client);
  if (bspdriver->master)
  {
    thread_master_free(bspdriver->master);
    bspdriver->master = NULL;
  }
  return OK;
}

int bsp_driver_task_init(bsp_driver_t *bspdriver)
{
  if (!bspdriver->master)
  {
    bspdriver->master = thread_master_module_create(MODULE_BSP);
  }
  if (bspdriver->taskid <= 0)
    bspdriver->taskid = os_task_create("bspTask", OS_TASK_DEFAULT_PRIORITY,
                                    0, bsp_driver_task, bspdriver->master, OS_TASK_DEFAULT_STACK * 4);
  if (bspdriver->taskid > 0)
  {
    module_setup_task(MODULE_BSP, bspdriver->taskid);
    return OK;
  }
  return ERROR;
}

int bsp_driver_task_exit(bsp_driver_t *bspdriver)
{
  if (bspdriver->taskid > 0)
    os_task_destroy(bspdriver->taskid);
  if (bspdriver->master)
  {
    thread_master_free(bspdriver->master);
    bspdriver->master = NULL;
  }
  return OK;
}


int bsp_module_init(void)
{
	memset(&bsp_driver, 0, sizeof(bsp_driver));
	bsp_driver_init(&bsp_driver);
//#ifdef ZPL_SDK_MODULE
	bsp_module_func(&bsp_driver, sdk_driver_init, sdk_driver_start, sdk_driver_stop, sdk_driver_exit);
//#endif	

	if(bsp_driver.bsp_sdk_init)
	{
		(bsp_driver.bsp_sdk_init)(&bsp_driver, bsp_driver.sdk_driver);
	}
	if(bsp_driver.sdk_driver)
	{
		hal_client_callback(bsp_driver.hal_client, bsp_driver_msg_handle, bsp_driver.sdk_driver);
	}
	return OK;
}

int bsp_module_task_init(void)
{
	bsp_driver_task_init(&bsp_driver);
	return OK;
}

int bsp_module_start(void)
{
	struct hal_ipcmsg_hwport porttbl[5];
	memset(porttbl, 0, sizeof(porttbl));

	porttbl[0].type = IF_ETHERNET;//lan1
	porttbl[0].port = 1;
  	porttbl[0].phyid = 4;

	porttbl[1].type = IF_ETHERNET;//lan2
	porttbl[1].port = 2;
  	porttbl[1].phyid = 0;

	porttbl[2].type = IF_ETHERNET;//lan3
	porttbl[2].port = 3;
  	porttbl[2].phyid = 1;

	porttbl[3].type = IF_ETHERNET;//lan4
	porttbl[3].port = 4;
  	porttbl[3].phyid = 2;

	porttbl[4].type = IF_ETHERNET;//wan
	porttbl[4].port = 5;
  	porttbl[4].phyid = 3;
  /*
				port0: port@0 {
					reg = <0>;
					label = "lan2";
				};

				port1: port@1 {
					reg = <1>;
					label = "lan3";
				};

				port2: port@2 {
					reg = <2>;
					label = "lan4";
				};

				port3: port@3 {
					reg = <3>;
					label = "wan";
				};

				port4: port@4 {
					reg = <4>;
					label = "lan1";
				};
        */
	os_sleep(1);
	zlog_debug(MODULE_BSP, "BSP Init");
  	hal_client_bsp_register(bsp_driver.hal_client, 0,
        0, 0, 5, "V0.0.0.1");

	os_sleep(1);
	zlog_debug(MODULE_BSP, "SDK Init, waitting...");

	if(bsp_driver.bsp_sdk_start)
	{
		(bsp_driver.bsp_sdk_start)(&bsp_driver, bsp_driver.sdk_driver);
	}
	zlog_debug(MODULE_BSP, "SDK Register Port Table Info.");
  	hal_client_bsp_hwport_register(bsp_driver.hal_client, 5, porttbl);

	hal_client_event(HAL_EVENT_REGISTER, bsp_driver.hal_client, 1);
	zlog_debug(MODULE_BSP, "SDK Init, Done.");
	return OK;
}

int bsp_module_exit(void)
{
	if(bsp_driver.bsp_sdk_stop)
		return (bsp_driver.bsp_sdk_stop)(&bsp_driver, bsp_driver.sdk_driver);
	bsp_driver_exit(&bsp_driver);
	return OK;
}

int bsp_module_task_exit(void)
{
	bsp_driver_task_exit(&bsp_driver);
	if(bsp_driver.bsp_sdk_exit)
		return (bsp_driver.bsp_sdk_exit)(&bsp_driver, bsp_driver.sdk_driver);
	return OK;
}

#ifndef ZPL_SDK_MODULE
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
	return OK;
}

int sdk_driver_stop(struct bsp_driver *bsp, zpl_void *p)
{
	sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
	return OK;
}

int sdk_driver_exit(struct bsp_driver *bsp, zpl_void *p)
{
	sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
	if(sdkdriver)
		sdk_driver_free(sdkdriver);
	return OK;
}

#endif