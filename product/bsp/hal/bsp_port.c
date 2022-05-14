/*
 * bsp_port.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */


#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_include.h"

#include "hal_client.h"
#include "bsp_port.h"
#include "bsp_driver.h"

sdk_port_t sdk_port;

static int bsp_port_up(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_link_cb)
		ret = sdk_port.sdk_port_link_cb(driver, port->phyport, param->value);
	BSP_LEAVE_FUNC();
	return ret;
}



/*static int bsp_port_address_set(ifindex_t port->phyport, struct prefix *cp, static int secondry)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_set_address_cb)
		ret = sdk_port.sdk_port_set_address_cb(driver, port->phyport, cp, secondry);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_port_address_unset(ifindex_t port->phyport, struct prefix *cp, static int secondry)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_unset_address_cb)
		ret = sdk_port.sdk_port_unset_address_cb(driver, port->phyport, cp, secondry);
	BSP_LEAVE_FUNC();
	return ret;
}*/



/*static int bsp_port_metric_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_metric_cb)
		ret = sdk_port.sdk_port_metric_cb(driver, port->phyport,  value);
	BSP_LEAVE_FUNC();
	return ret;
}*/

static int bsp_port_vrf_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_vrf_cb)
		ret = sdk_port.sdk_port_vrf_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

/*static int bsp_port_multicast_set(ifindex_t port->phyport, static int value)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_multicast_cb)
		ret = sdk_port.sdk_port_multicast_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_port_bandwidth_set(ifindex_t port->phyport, static int value)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_bandwidth_cb)
		ret = sdk_port.sdk_port_bandwidth_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}*/

static int bsp_port_speed_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_speed_cb)
		ret = sdk_port.sdk_port_speed_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_port_duplex_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_duplex_cb)
		ret = sdk_port.sdk_port_duplex_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}



/*
static int bsp_port_linkdetect_set(ifindex_t port->phyport, static int value)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_linkdetect_cb)
		ret = sdk_port.sdk_port_linkdetect_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}
*/


static int bsp_port_loop_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_loop_cb)
		ret = sdk_port.sdk_port_loop_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}



static int bsp_port_jumbo_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_jumbo_cb)
		ret = sdk_port.sdk_port_jumbo_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_port_enable_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_enable_cb)
		ret = sdk_port.sdk_port_enable_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}


static int bsp_port_flow_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_flow_cb)
		ret = sdk_port.sdk_port_flow_cb(driver, port->phyport,  (param->value>>16), param->value&0xffff);
	BSP_LEAVE_FUNC();
	return ret;
}
static int bsp_port_pause_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_pause_cb)
		ret = sdk_port.sdk_port_pause_cb(driver, port->phyport,  (param->value>>16), param->value&0xffff);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_port_learning_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_learning_enable_cb)
		ret = sdk_port.sdk_port_learning_enable_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_port_software_learning_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_swlearning_enable_cb)
		ret = sdk_port.sdk_port_swlearning_enable_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_port_protected_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_protected_enable_cb)
		ret = sdk_port.sdk_port_protected_enable_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}


static int bsp_port_state_get(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_state_get_cb)
		ret = sdk_port.sdk_port_state_get_cb(driver, port->phyport);
	if(param)
		param->value	= ret;
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_port_speed_get(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_speed_get_cb)
		ret = sdk_port.sdk_port_speed_get_cb(driver, port->phyport);
	if(param)
		param->value	= ret;
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_port_duplex_get(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_duplex_get_cb)
		ret = sdk_port.sdk_port_duplex_get_cb(driver, port->phyport);
	if(param)
		param->value	= ret;
	BSP_LEAVE_FUNC();
	return ret;
}


static int bsp_port_mode_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_mode_cb)
		ret = sdk_port.sdk_port_mode_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_port_mtu_set(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_speed_cb)
		ret = sdk_port.sdk_port_speed_cb(driver, port->phyport,  param->value);
	BSP_LEAVE_FUNC();
	return ret;
}

//风暴
static int bsp_port_storm_rate_limit(void *driver, hal_port_header_t *port, hal_port_param_t *param)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if(driver && sdk_port.sdk_port_storm_rate_cb)
		ret = sdk_port.sdk_port_storm_rate_cb(driver, port->phyport, param->value, param->value1, param->value2);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_port_stats_get(void *driver, hal_port_header_t *port, struct if_stats *ifstats)
{
	int ret = NO_SDK;
	BSP_ENTER_FUNC();
	if (driver && sdk_port.sdk_port_stat_cb)
		ret = sdk_port.sdk_port_stat_cb(driver, port->phyport, ifstats);
	if (ret == OK)
	{
		ifstats->rx_packets = htonl(ifstats->rx_packets);	  /* total packets received       */
		ifstats->tx_packets = htonl(ifstats->tx_packets);	  /* total packets transmitted    */
		ifstats->rx_bytes = htonl(ifstats->rx_bytes);		  /* total bytes received         */
		ifstats->tx_bytes = htonl(ifstats->tx_bytes);		  /* total bytes transmitted      */
		ifstats->rx_errors = htonl(ifstats->rx_errors);		  /* bad packets received         */
		ifstats->tx_errors = htonl(ifstats->tx_errors);		  /* packet transmit problems     */
		ifstats->rx_dropped = htonl(ifstats->rx_dropped);	  /* no space in linux buffers    */
		ifstats->tx_dropped = htonl(ifstats->tx_dropped);	  /* no space available in linux  */
		ifstats->rx_multicast = htonl(ifstats->rx_multicast); /* multicast packets received   */
		ifstats->collisions = htonl(ifstats->collisions);

		/* detailed rx_errors: */
		ifstats->rx_length_errors = htonl(ifstats->rx_length_errors);
		ifstats->rx_over_errors = htonl(ifstats->rx_over_errors);	  /* receiver ring buff overflow  */
		ifstats->rx_crc_errors = htonl(ifstats->rx_crc_errors);		  /* recved pkt with crc error    */
		ifstats->rx_frame_errors = htonl(ifstats->rx_frame_errors);	  /* recv'd frame alignment error */
		ifstats->rx_fifo_errors = htonl(ifstats->rx_fifo_errors);	  /* recv'r fifo overrun          */
		ifstats->rx_missed_errors = htonl(ifstats->rx_missed_errors); /* receiver missed packet     */
		/* detailed tx_errors */
		ifstats->tx_aborted_errors = htonl(ifstats->tx_aborted_errors);
		ifstats->tx_carrier_errors = htonl(ifstats->tx_carrier_errors);
		ifstats->tx_fifo_errors = htonl(ifstats->tx_fifo_errors);
		ifstats->tx_heartbeat_errors = htonl(ifstats->tx_heartbeat_errors);
		ifstats->tx_window_errors = htonl(ifstats->tx_window_errors);
		/* for cslip etc */
		ifstats->rx_compressed = htonl(ifstats->rx_compressed);
		ifstats->tx_compressed = htonl(ifstats->tx_compressed);
	}
	BSP_LEAVE_FUNC();
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_PORT_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_PORT, bsp_port_enable_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_LINK, bsp_port_up),
	HAL_CALLBACK_ENTRY(HAL_PORT_SPEED, bsp_port_speed_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_DUPLEX, bsp_port_duplex_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_FLOW, bsp_port_flow_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_FLOW, bsp_port_pause_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_JUMBO, bsp_port_jumbo_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_LOOP, bsp_port_loop_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_LEARNING, bsp_port_learning_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_SWLEARNING, bsp_port_software_learning_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_PROTECTED, bsp_port_protected_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_VRF, bsp_port_vrf_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_MODE, bsp_port_mode_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_MTU, bsp_port_mtu_set),
	HAL_CALLBACK_ENTRY(HAL_PORT_MULTICAST, NULL),
	HAL_CALLBACK_ENTRY(HAL_PORT_BANDWIDTH, NULL),
	HAL_CALLBACK_ENTRY(HAL_PORT_STORM_RATELIMIT, bsp_port_storm_rate_limit),
	HAL_CALLBACK_ENTRY(HAL_PORT_STATS, bsp_port_stats_get),
};





int bsp_port_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_port_param_t	param;
	hal_port_header_t	bspport;
	struct hal_ipcmsg_result getvalue;
	struct if_stats ifstats;
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
	if(cmd == HAL_MODULE_CMD_REQ)
	{
		if(subcmd == HAL_PORT_STORM_RATELIMIT)
		{
			hal_ipcmsg_getc(&client->ipcmsg, &param.value);
			hal_ipcmsg_getl(&client->ipcmsg, &param.value1);
			hal_ipcmsg_getl(&client->ipcmsg, &param.value2);
		}
		else
			hal_ipcmsg_getl(&client->ipcmsg, &param.value);
	}
	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (callback->cmd_handle)(driver, &bspport, &param);
	break;
	case HAL_MODULE_CMD_GET:         //获取
	{
		memset(&getvalue, 0, sizeof(getvalue));
		switch(subcmd)
		{
			case HAL_PORT_LINK:
			ret = bsp_port_state_get(driver, &bspport, &param);
			getvalue.state = param.value;
			hal_client_send_result(client,  ret, &getvalue);
			break;
			case HAL_PORT_SPEED:
			ret = bsp_port_speed_get(driver, &bspport, &param);
			getvalue.value = param.value;
			hal_client_send_result(client,  ret, &getvalue);
			break;
			case HAL_PORT_DUPLEX:
			ret = bsp_port_duplex_get(driver, &bspport, &param);
			getvalue.value = param.value;
			hal_client_send_result(client,  ret, &getvalue);
			break;
			case HAL_PORT_STATS:
			ret = bsp_port_stats_get(driver, &bspport, &ifstats);
			getvalue.value = param.value;
			hal_client_send_result_msg(client,  ret, &getvalue, HAL_PORT_STATS, &ifstats, sizeof(struct if_stats));
			//hal_client_send_result(client,  ret, &getvalue);
			break;
			default:
			break;
		}
	}
	break;
	default:
		break;
	}
	BSP_LEAVE_FUNC();
	return ret;
}