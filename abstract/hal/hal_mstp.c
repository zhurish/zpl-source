/*
 * hal_mstp.c
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"

//#include "hal_vlan.h"
#include "hal_mstp.h"


int hal_mstp_enable(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, enable);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_MSTP, HAL_MODULE_CMD_SET, HAL_MSTP);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mstp_age(zpl_uint32 age)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, age);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_MSTP, HAL_MODULE_CMD_SET, HAL_MSTP_AGE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mstp_bypass(zpl_bool enable, zpl_uint32 type)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, enable);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, type);
	hal_ipcmsg_putl(&ipcmsg, 0);

	command = IPCCMD_SET(HAL_MODULE_MSTP, HAL_MODULE_CMD_SET, HAL_MSTP_BYPASS);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mstp_state(ifindex_t ifindex, zpl_uint32 mstp, hal_port_stp_state_t state)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, mstp);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, state);
	command = IPCCMD_SET(HAL_MODULE_MSTP, HAL_MODULE_CMD_SET, HAL_MSTP_STATE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_stp_state(ifindex_t ifindex, hal_port_stp_state_t state)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, state);
	command = IPCCMD_SET(HAL_MODULE_STP, HAL_MODULE_CMD_SET, HAL_STP_STATE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
