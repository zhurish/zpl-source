/*
 * hal_8021x.c
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */


#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_8021x.h"


int hal_8021x_enable(zpl_bool enable)
{
	return OK;
}

int hal_8021x_interface_enable(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable?2:0);
	command = IPCCMD_SET(HAL_MODULE_8021X, HAL_MODULE_CMD_REQ, HAL_8021X_PORT_MODE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_8021x_interface_state(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_8021X, HAL_MODULE_CMD_REQ, HAL_8021X_PORT_STATE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_8021x_interface_mode(ifindex_t ifindex, zpl_uint32 value)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, value);
	command = IPCCMD_SET(HAL_MODULE_8021X, HAL_MODULE_CMD_REQ, HAL_8021X_PORT_MODE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_8021x_interface_dmac(ifindex_t ifindex, mac_t *mac)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_put(&ipcmsg, mac, NSM_MAC_MAX);
	command = IPCCMD_SET(HAL_MODULE_8021X, HAL_MODULE_CMD_REQ, HAL_8021X_PORT_MAC);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}



