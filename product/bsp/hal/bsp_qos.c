/*
 * bsp_qos.c
 *
 *  Created on: 2019年9月10日
 *      Author: DELL
 */
#include "bsp_types.h"
#include "hal_client.h"
#include "bsp_qos.h"


sdk_qos_t sdk_qos;

static int bsp_qos_enable(void *driver, hal_port_header_t *port, hal_qos_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_qos.sdk_qos_enable_cb)
		ret = sdk_qos.sdk_qos_enable_cb(bspdev->sdk_driver, param->enable);
	BSP_LEAVE_FUNC();	
	return ret;
}

static int bsp_qos_ipg_enable(void *driver, hal_port_header_t *port, hal_qos_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	//if(bspdev->sdk_driver && sdk_qos.sdk_qos_ipg_cb)
	//	ret = sdk_qos.sdk_qos_ipg_cb(bspdev->sdk_driver, zpl_true, param->enable);
	BSP_LEAVE_FUNC();
	return ret;
}


/*
static int bsp_qos_8021q_enable(void *driver, hal_port_header_t *port, hal_qos_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_qos.sdk_qos_8021q_enable_cb)
		ret = sdk_qos.sdk_qos_8021q_enable_cb(bspdev->sdk_driver, port->phyport, param->enable);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_qos_diffserv_enable(void *driver, hal_port_header_t *port, hal_qos_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_qos.sdk_qos_diffserv_enable_cb)
		ret = sdk_qos.sdk_qos_diffserv_enable_cb(bspdev->sdk_driver, port->phyport, param->enable);
	BSP_LEAVE_FUNC();
	return ret;
}


static int bsp_qos_diffserv_map_queue(void *driver, hal_port_header_t *port, hal_qos_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_qos.sdk_qos_diffserv_map_queue_cb)
		ret = sdk_qos.sdk_qos_diffserv_map_queue_cb(bspdev->sdk_driver, port->phyport, param->diffserv, param->queue);
	BSP_LEAVE_FUNC();
	return ret;
}
*/



//端口限速
static int bsp_qos_egress_rate_limit(void *driver, hal_port_header_t *port, hal_qos_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_qos.sdk_qos_port_egress_rate_cb)
		ret = sdk_qos.sdk_qos_port_egress_rate_cb(bspdev->sdk_driver, port->phyport, param->limit);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_qos_ingress_rate_limit(void *driver, hal_port_header_t *port, hal_qos_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_qos.sdk_qos_port_ingress_rate_cb)
		ret = sdk_qos.sdk_qos_port_ingress_rate_cb(bspdev->sdk_driver, port->phyport, param->limit);
	BSP_LEAVE_FUNC();
	return ret;
}

//CPU
static int bsp_qos_cpu_rate_limit(void *driver, hal_port_header_t *port, hal_qos_param_t *param)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
	BSP_ENTER_FUNC();
	if(bspdev->sdk_driver && sdk_qos.sdk_qos_cpu_rate_cb)
		ret = sdk_qos.sdk_qos_cpu_rate_cb(bspdev->sdk_driver, param->limit, param->burst_size);
	BSP_LEAVE_FUNC();
	return ret;
}



static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_QOS_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_QOS_EN, bsp_qos_enable),
	HAL_CALLBACK_ENTRY(HAL_QOS_IPG, bsp_qos_ipg_enable),
	//HAL_CALLBACK_ENTRY(HAL_QOS_BASE_TRUST, bsp_qos_base_mode),
	//HAL_CALLBACK_ENTRY(HAL_QOS_8021Q, bsp_qos_8021q_enable),
	//HAL_CALLBACK_ENTRY(HAL_QOS_DIFFSERV, bsp_qos_diffserv_enable),

	//CLASS
	//HAL_CALLBACK_ENTRY(HAL_QOS_QUEUE_MAP_CLASS, bsp_qos_queue_class),
	HAL_CALLBACK_ENTRY(HAL_QOS_CLASS_SCHED, NULL),
	HAL_CALLBACK_ENTRY(HAL_QOS_CLASS_WEIGHT, NULL),


	//INPUT
	HAL_CALLBACK_ENTRY(HAL_QOS_8021Q_MAP_QUEUE, NULL),
	//HAL_CALLBACK_ENTRY(HAL_QOS_DIFFSERV_MAP_QUEUE, bsp_qos_diffserv_map_queue),
	HAL_CALLBACK_ENTRY(HAL_QOS_IPPRE_MAP_QUEUE, NULL),
	HAL_CALLBACK_ENTRY(HAL_QOS_MPLSEXP_MAP_QUEUE, NULL),
	//HAL_CALLBACK_ENTRY(HAL_QOS_PORT_MAP_QUEUE, bsp_qos_port_map_queue),


	//HAL_CALLBACK_ENTRY(HAL_QOS_QUEUE_SCHED, bsp_qos_queue_scheduling),
	//HAL_CALLBACK_ENTRY(HAL_QOS_QUEUE_WEIGHT, bsp_qos_queue_weight),
	HAL_CALLBACK_ENTRY(HAL_QOS_QUEUE_RATELIMIT, NULL),
	HAL_CALLBACK_ENTRY(HAL_QOS_PRI_REMARK, NULL),


	HAL_CALLBACK_ENTRY(HAL_QOS_CPU_RATELIMIT, bsp_qos_cpu_rate_limit),
	HAL_CALLBACK_ENTRY(HAL_QOS_PORT_INRATELIMIT, bsp_qos_ingress_rate_limit),
	HAL_CALLBACK_ENTRY(HAL_QOS_PORT_OUTRATELIMIT, bsp_qos_egress_rate_limit),
};



int bsp_qos_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_qos_param_t	param;
	hal_port_header_t	bspport;
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
	hal_ipcmsg_port_get(&client->ipcmsg, &bspport);

	hal_ipcmsg_getl(&client->ipcmsg, &param.enable);
	hal_ipcmsg_getl(&client->ipcmsg, &param.value);
	//hal_ipcmsg_get(&client->ipcmsg, &param.mac, NSM_MAC_MAX);

	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (callback->cmd_handle)(driver, &bspport, &param);
	break;
	default:
		break;
	}
	BSP_LEAVE_FUNC();
	return ret;
}