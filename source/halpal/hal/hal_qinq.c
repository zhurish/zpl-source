/*
 * hal_qinq.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"

#include "hal_qinq.h"


int hal_qinq_enable(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_QINQ, HAL_MODULE_CMD_REQ, HAL_QINQ_CMD_ENABLE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_qinq_vlan_tpid(vlan_t tpid)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putw(&ipcmsg, tpid);
	//hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_QINQ, HAL_MODULE_CMD_REQ, HAL_QINQ_CMD_TPID);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_qinq_interface_enable(ifindex_t ifindex, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_QINQ, HAL_MODULE_CMD_REQ, HAL_QINQ_CMD_IF_ENABLE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
