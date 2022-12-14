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

#ifndef ZPL_SDK_KERNEL
#include "bsp_include.h"
#endif
#ifdef ZPL_SDK_USER
#include "sdk_driver.h"
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


int bsp_driver_mac_cache_add(bsp_driver_t *sdkdriver, zpl_uint8 port, zpl_uint8 *mac, vlan_t vid, zpl_uint8 isstatic, zpl_uint8 isage, zpl_uint8 vaild)
{
	zpl_uint32 i = 0;
	for(i = 0; i < sdkdriver->mac_cache_max; i++)
	{
		if(sdkdriver->mac_cache_entry[i].use == 1 && 
			sdkdriver->mac_cache_entry[i].vid == vid && 
			(memcmp(sdkdriver->mac_cache_entry[i].mac, mac, ETH_ALEN) == 0))
		{
			sdkdriver->mac_cache_entry[i].port = port;
			sdkdriver->mac_cache_entry[i].vid = vid;
			sdkdriver->mac_cache_entry[i].is_valid = vaild;
			sdkdriver->mac_cache_entry[i].is_age = isage;
			sdkdriver->mac_cache_entry[i].is_static = isstatic;
			sdkdriver->mac_cache_entry[i].use = 1;
			return OK;
		}
	}
	for(i = 0; i < sdkdriver->mac_cache_max; i++)
	{
		if(sdkdriver->mac_cache_entry[i].use == 0)
		{
			sdkdriver->mac_cache_entry[i].port = port;
			sdkdriver->mac_cache_entry[i].vid = vid;
			sdkdriver->mac_cache_entry[i].is_valid = vaild;
			sdkdriver->mac_cache_entry[i].is_age = isage;
			sdkdriver->mac_cache_entry[i].is_static = isstatic;
			sdkdriver->mac_cache_entry[i].use = 1;
			memcpy(sdkdriver->mac_cache_entry[i].mac, mac, ETH_ALEN);
			sdkdriver->mac_cache_num++;
			return OK;
		}
	}
	return OK;
}


int bsp_driver_mac_cache_update(bsp_driver_t *sdkdriver, zpl_uint8 *mac, zpl_uint8 isage)
{
	zpl_uint32 i = 0;
	for(i = 0; i < sdkdriver->mac_cache_max; i++)
	{
		if(sdkdriver->mac_cache_entry[i].use == 1 && (memcmp(sdkdriver->mac_cache_entry[i].mac, mac, ETH_ALEN) == 0))
		{
			sdkdriver->mac_cache_entry[i].use = isage?1:0;
			if(isage == 0)
				sdkdriver->mac_cache_num--;
			return OK;
		}
	}
	return OK;
}


#if defined(ZPL_SDK_USER) || defined(ZPL_SDK_NONE)
static const hal_ipccmd_callback_t moduletable[] = {
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
#endif

int bsp_driver_msg_handle(struct hal_client *client, zpl_uint32 cmd, void *driver)
{
	int ret = OS_NO_CALLBACK;
	int module = IPCCMD_MODULE_GET(cmd);
	hal_ipccmd_callback_t * callback = NULL;
	BSP_ENTER_FUNC();
#if defined(ZPL_SDK_USER) || defined(ZPL_SDK_NONE)
	if(module > HAL_MODULE_NONE && module < HAL_MODULE_MAX)
	{
		int i = 0;
		for(i = 0; i < ZPL_ARRAY_SIZE(moduletable); i++)
		{
			//zlog_warn(MODULE_HAL, "=== this module:%d  %d", moduletable[i].module, module);
			if(moduletable[i].module == module && moduletable[i].module_handle)
			{
				callback = &moduletable[i];
				//ret = (moduletable[i].module_handle)(client, IPCCMD_CMD_GET(cmd), IPCCMD_SUBCMD_GET(cmd), driver);
				break;
			}
		}
		if(callback)
			ret = (callback->module_handle)(client, IPCCMD_CMD_GET(cmd), IPCCMD_SUBCMD_GET(cmd), driver);
		else
		{
			zlog_warn(MODULE_HAL, "Can not Find this module:%d ", module);
			ret = OS_NO_CALLBACK;
		}		
	}
#elif defined(ZPL_SDK_MODULE) && defined(ZPL_SDK_KERNEL)
	if(module > HAL_MODULE_NONE && module < HAL_MODULE_MAX)
	{
		if(IPCCMD_CMD_GET(cmd) != HAL_MODULE_CMD_ACK)
			ret = bsp_driver_kernel_netlink_proxy(client->bsp_driver, client->ipcmsg.buf, client->ipcmsg.setp, client);
		else
		{
			zlog_warn(MODULE_HAL, "=============HAL_MODULE_CMD_ACK:%d ", module);
			ret = OS_NO_CALLBACK;
		}	
	}
#endif	
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

int bsp_driver_report(bsp_driver_t *bspdriver, char *data, int len)
{
	if(bspdriver && bspdriver->hal_client)
		return hal_client_send_report(bspdriver->hal_client, data, len);
	return ERROR;	
}

static int bsp_driver_task(void *p)
{
  struct thread_master *master = (struct thread_master *)p;
  module_setup_task(master->module, os_task_id_self());
  while (thread_mainloop(master))
    ;
  return OK;
}

int bsp_driver_init(bsp_driver_t *bspdriver)
{
  struct hal_client *bsp = hal_client_create(MODULE_BSP, 0, 0);
  if (bsp)
  {
	bspdriver->mac_cache_max = ETH_MAC_CACHE_MAX;
	bspdriver->mac_cache_num = 0;
	bspdriver->mac_cache_entry = XMALLOC(MTYPE_MAC, sizeof(hal_mac_cache_t)*bspdriver->mac_cache_max);
	if(bspdriver->mac_cache_entry == NULL)
	{
		hal_client_destroy(bsp);
		return ERROR;
	}

    bsp->debug = 0xffff;
    bsp->master = bspdriver->master = thread_master_module_create(MODULE_BSP);
    bspdriver->hal_client = bsp;
    hal_client_start(bsp, HAL_IPCMSG_CMD_PATH, -1/*HAL_IPCMSG_CMD_PORT*/, 0);
    return OK;
  }
  return ERROR;
}

int bsp_driver_exit(bsp_driver_t *bspdriver)
{
	if(bspdriver->mac_cache_entry)
	{
		XFREE(MTYPE_MAC, bspdriver->mac_cache_entry);
		bspdriver->mac_cache_entry = NULL;
		bspdriver->mac_cache_max = 0;
		bspdriver->mac_cache_num = 0;
	}		
	if(bspdriver->hal_client)
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
	bsp_module_func(&bsp_driver, sdk_driver_init, sdk_driver_start, sdk_driver_stop, sdk_driver_exit);

	memset(bsp_driver.phyports, 0, sizeof(bsp_driver.phyports));
	bsp_driver.phyports[0].phyport = 0;
	bsp_driver.phyports[1].phyport = 1;
	bsp_driver.phyports[2].phyport = 2;
	bsp_driver.phyports[3].phyport = 3;
	bsp_driver.phyports[4].phyport = 4;
	bsp_driver.phyports[5].phyport = -1;
	bsp_driver.phyports[6].phyport = -1;
	bsp_driver.phyports[7].phyport = 8;// cpu port

	if(bsp_driver.bsp_sdk_init)
	{
		(bsp_driver.bsp_sdk_init)(&bsp_driver, bsp_driver.sdk_driver);
	}
	if(bsp_driver.sdk_driver && bsp_driver.hal_client)
	{
		hal_client_callback(bsp_driver.hal_client, bsp_driver_msg_handle, &bsp_driver);
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

	if(bsp_driver.bsp_sdk_start)
	{
		(bsp_driver.bsp_sdk_start)(&bsp_driver, bsp_driver.sdk_driver);
	}
	bsp_driver.cpu_port = ((sdk_driver_t*)bsp_driver.sdk_driver)->cpu_port;

	porttbl[0].type = IF_ETHERNET;//lan1
	porttbl[0].lport = 1;
  	porttbl[0].phyid = 4;

	porttbl[1].type = IF_ETHERNET;//lan2
	porttbl[1].lport = 2;
  	porttbl[1].phyid = 0;

	porttbl[2].type = IF_ETHERNET;//lan3
	porttbl[2].lport = 3;
  	porttbl[2].phyid = 1;

	porttbl[3].type = IF_ETHERNET;//lan4
	porttbl[3].lport = 4;
  	porttbl[3].phyid = 2;

	porttbl[4].type = IF_ETHERNET;//wan
	porttbl[4].lport = 5;
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
	if(bsp_driver.hal_client)
  	hal_client_bsp_register(bsp_driver.hal_client, 0,
        0, 0, 5, "V0.0.0.1");

	os_sleep(1);
	zlog_debug(MODULE_BSP, "SDK Init, waitting...");


	zlog_debug(MODULE_BSP, "SDK Register Port Table Info.");
	if(bsp_driver.hal_client)
  	hal_client_bsp_hwport_register(bsp_driver.hal_client, 5, porttbl);

	if(bsp_driver.hal_client)
		hal_client_event(HAL_EVENT_REGISTER, bsp_driver.hal_client, 1, 0);
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

#ifdef ZPL_SDK_NONE
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
	//sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
	bsp->sdk_driver = sdk_driver_malloc();
	if(bsp->sdk_driver)
		return OK;
	else
		return ERROR;	
}

int sdk_driver_start(struct bsp_driver *bsp, zpl_void *p)
{
	//sdk_driver_t *sdkdriver = (sdk_driver_t *)p;
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
#elif defined(ZPL_SDK_MODULE) && defined(ZPL_SDK_KERNEL)
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
	bsp->sdk_driver = sdk_driver_malloc();
	if(bsp->sdk_driver)
	{
		return OK;
	}
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
	{
		sdk_driver_free(sdkdriver);
	}
	return OK;
}
#endif