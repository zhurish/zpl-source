/*
 * hal_igmp.c
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */


#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_global.h"

//jumbo
int hal_jumbo_size_set(zpl_uint32 size)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, size);
	command = IPCCMD_SET(HAL_MODULE_GLOBAL, HAL_MODULE_CMD_REQ, HAL_GLOBAL_CMD_JUMBO_SIZE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}




/*
 * Global
 */
int hal_switch_mode(zpl_bool manage)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	command = IPCCMD_SET(HAL_MODULE_GLOBAL, HAL_MODULE_CMD_REQ, HAL_GLOBAL_MANEGE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_switch_forward(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	command = IPCCMD_SET(HAL_MODULE_GLOBAL, HAL_MODULE_CMD_REQ, HAL_GLOBAL_FORWARD);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_multicast_flood(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	command = IPCCMD_SET(HAL_MODULE_GLOBAL, HAL_MODULE_CMD_REQ, HAL_GLOBAL_MULTICAST_FLOOD);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_unicast_flood(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	command = IPCCMD_SET(HAL_MODULE_GLOBAL, HAL_MODULE_CMD_REQ, HAL_GLOBAL_UNICAST_FLOOD);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_multicast_learning(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	command = IPCCMD_SET(HAL_MODULE_GLOBAL, HAL_MODULE_CMD_REQ, HAL_GLOBAL_MULTICAST_LEARNING);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_global_bpdu_enable(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	command = IPCCMD_SET(HAL_MODULE_GLOBAL, HAL_MODULE_CMD_REQ, HAL_GLOBAL_BPDU);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_global_aging_time(zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_GLOBAL, HAL_MODULE_CMD_REQ, HAL_GLOBAL_AGINT);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_port_wan_set(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_GLOBAL, HAL_MODULE_CMD_REQ, HAL_GLOBAL_WAN_PORT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

#ifndef ZPL_SDK_USER
int hal_bsp_client_set(zpl_bool enable, zpl_uint32 val)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, enable);
	hal_ipcmsg_putl(&ipcmsg, val);
	command = IPCCMD_SET(HAL_MODULE_DEBUG, HAL_MODULE_CMD_REQ, HAL_CLIENT_DEBUG);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
int hal_bsp_netpkt_debug_set(zpl_bool enable, zpl_uint32 val)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, enable);
	hal_ipcmsg_putl(&ipcmsg, val);
	command = IPCCMD_SET(HAL_MODULE_DEBUG, HAL_MODULE_CMD_REQ, HAL_NETPKT_DEBUG);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_bsp_loglevel_set(zpl_uint32 level)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, level);
	command = IPCCMD_SET(HAL_MODULE_DEBUG, HAL_MODULE_CMD_REQ, HAL_KLOG_LEVEL);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
#endif

