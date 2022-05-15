/*
 * hal_port.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */


#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_global.h"
#include "hal_port.h"
#include "hal_misc.h"


int hal_port_up(ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 1);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_LINK);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_down(ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_LINK);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}



int hal_port_speed_set(ifindex_t ifindex, nsm_speed_en value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_SPEED);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_duplex_set(ifindex_t ifindex, nsm_duplex_en value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_DUPLEX);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}







int hal_port_loop_set(ifindex_t ifindex, zpl_bool value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_LOOP);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_jumbo_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_JUMBO);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_enable_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_port_learning_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_LEARNING);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_flow_set(ifindex_t ifindex, zpl_bool tx, zpl_bool rx)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, (tx<<16|rx));
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_FLOW);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}



zpl_bool hal_port_state_get(ifindex_t ifindex)
{
	int ret = 0;
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	struct hal_ipcmsg_result getvalue;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_GET, HAL_PORT_LINK);
	ret = hal_ipcmsg_send_andget_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg), &getvalue);
	if(ret == OK)
		return 	getvalue.state;
	return ERROR;	
}

nsm_speed_en hal_port_speed_get(ifindex_t ifindex)
{
	int ret = 0;
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	struct hal_ipcmsg_result getvalue;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_GET, HAL_PORT_SPEED);
	ret = hal_ipcmsg_send_andget_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg), &getvalue);
	if(ret == OK)
		return 	getvalue.value;
	return ERROR;	
}

nsm_duplex_en hal_port_duplex_get(ifindex_t ifindex)
{
	int ret = 0;
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	struct hal_ipcmsg_result getvalue;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_GET, HAL_PORT_DUPLEX);
	ret = hal_ipcmsg_send_andget_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg), &getvalue);
	if(ret == OK)
		return 	getvalue.value;
	return ERROR;	
}

int hal_port_mode_set(ifindex_t ifindex, if_mode_t value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, (value));
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_MODE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_multicast_set (ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, (value));
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_MULTICAST);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_bandwidth_set (ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, (value));
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_BANDWIDTH);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

//风暴
int hal_port_stormcontrol_set(ifindex_t ifindex, zpl_uint32 mode, zpl_uint32 limit, zpl_uint32 type)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, mode);
	hal_ipcmsg_putl(&ipcmsg, limit);
	hal_ipcmsg_putl(&ipcmsg, type);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_STORM_RATELIMIT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
/*int hal_port_address_set(ifindex_t ifindex, struct prefix *cp, int secondry)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_set_address_cb)
		return hal_driver->port_tbl->sdk_port_set_address_cb(hal_driver->driver, ifindex, cp, secondry);
	return NO_SDK;
}

int hal_port_address_unset(ifindex_t ifindex, struct prefix *cp, int secondry)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_unset_address_cb)
		return hal_driver->port_tbl->sdk_port_unset_address_cb(hal_driver->driver, ifindex, cp, secondry);
	return NO_SDK;
}*/


int hal_port_pause_set(ifindex_t ifindex, zpl_bool pause_tx, zpl_bool pause_rx)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, (pause_tx<<16|pause_rx));
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_PAUSE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
/*int hal_port_multicast_set(ifindex_t ifindex, int value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_multicast_cb)
		return hal_driver->port_tbl->sdk_port_multicast_cb(hal_driver->driver, ifindex,  value);
	return NO_SDK;
}

int hal_port_bandwidth_set(ifindex_t ifindex, int value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_bandwidth_cb)
		return hal_driver->port_tbl->sdk_port_bandwidth_cb(hal_driver->driver, ifindex,  value);
	return NO_SDK;
}*/


/*
int hal_port_linkdetect_set(ifindex_t ifindex, int value)
{
	if(hal_driver && hal_driver->port_tbl && hal_driver->port_tbl->sdk_port_linkdetect_cb)
		return hal_driver->port_tbl->sdk_port_linkdetect_cb(hal_driver->driver, ifindex,  value);
	return NO_SDK;
}
*/
static int hal_port_stats_get_callback(zpl_uint8 *buf, zpl_uint32 len, void *p)
{
	struct if_stats *stats = (struct if_stats *)p;
	struct if_stats *get_stats = (struct if_stats *)buf;
	stats->rx_packets = ntohl(get_stats->rx_packets);	  /* total packets received       */
	stats->tx_packets = ntohl(get_stats->tx_packets);	  /* total packets transmitted    */
	stats->rx_bytes = ntohl(get_stats->rx_bytes);		  /* total bytes received         */
	stats->tx_bytes = ntohl(get_stats->tx_bytes);		  /* total bytes transmitted      */
	stats->rx_errors = ntohl(get_stats->rx_errors);		  /* bad packets received         */
	stats->tx_errors = ntohl(get_stats->tx_errors);		  /* packet transmit problems     */
	stats->rx_dropped = ntohl(get_stats->rx_dropped);	  /* no space in linux buffers    */
	stats->tx_dropped = ntohl(get_stats->tx_dropped);	  /* no space available in linux  */
	stats->rx_multicast = ntohl(get_stats->rx_multicast); /* multicast packets received   */
	stats->collisions = ntohl(get_stats->collisions);

	/* detailed rx_errors: */
	stats->rx_length_errors = ntohl(get_stats->rx_length_errors);
	stats->rx_over_errors = ntohl(get_stats->rx_over_errors);	  /* receiver ring buff overflow  */
	stats->rx_crc_errors = ntohl(get_stats->rx_crc_errors);		  /* recved pkt with crc error    */
	stats->rx_frame_errors = ntohl(get_stats->rx_frame_errors);	  /* recv'd frame alignment error */
	stats->rx_fifo_errors = ntohl(get_stats->rx_fifo_errors);	  /* recv'r fifo overrun          */
	stats->rx_missed_errors = ntohl(get_stats->rx_missed_errors); /* receiver missed packet     */
	/* detailed tx_errors */
	stats->tx_aborted_errors = ntohl(get_stats->tx_aborted_errors);
	stats->tx_carrier_errors = ntohl(get_stats->tx_carrier_errors);
	stats->tx_fifo_errors = ntohl(get_stats->tx_fifo_errors);
	stats->tx_heartbeat_errors = ntohl(get_stats->tx_heartbeat_errors);
	stats->tx_window_errors = ntohl(get_stats->tx_window_errors);
	/* for cslip etc */
	stats->rx_compressed = ntohl(get_stats->rx_compressed);
	stats->tx_compressed = ntohl(get_stats->tx_compressed);
	return 0;
}

int hal_port_stats_get(ifindex_t ifindex, struct if_stats *stats)
{
	int ret = 0;
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	struct hal_ipcmsg_result getvalue;
	struct hal_ipcmsg_callback callback;
	char buf[512];
	HAL_ENTER_FUNC();
	callback.ipcmsg_callback = hal_port_stats_get_callback;
	callback.pVoid = stats;
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_GET, HAL_PORT_STATS);

	ret = hal_ipcmsg_getmsg_callback(IF_IFINDEX_UNIT_GET(ifindex), command, buf, hal_ipcmsg_msglen_get(&ipcmsg), &getvalue, &callback);
	//ret = hal_ipcmsg_send_andget_message(IF_IFINDEX_UNIT_GET(ifindex), 
	//	command, buf, hal_ipcmsg_msglen_get(&ipcmsg), &getvalue);
	return ret;	
}


int hal_port_protected_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_PROTECTED);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_port_software_learning_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_SWLEARNING);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_port_vrf_set(ifindex_t ifindex, vrf_id_t value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_VRF);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_mtu_set(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_PORT, HAL_MODULE_CMD_REQ, HAL_PORT_MTU);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}