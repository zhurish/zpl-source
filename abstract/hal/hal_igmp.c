/*
 * hal_igmp.c
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */


#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_igmp.h"


int hal_igmp_snooping_set(zpl_uint32 type, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, type);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_IGMP, HAL_MODULE_CMD_REQ, HAL_IGMP_SNOOPING);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_igmpunknow_snooping_set(zpl_uint32 type, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, type);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_IGMP, HAL_MODULE_CMD_REQ, HAL_IGMPUNKNOW_SNOOPING);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_igmpqry_snooping_set(zpl_uint32 type, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, type);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_IGMP, HAL_MODULE_CMD_REQ, HAL_IGMPQRY_SNOOPING);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_mldqry_snooping_set(zpl_uint32 type, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, type);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_IGMP, HAL_MODULE_CMD_REQ, HAL_MLDQRY_SNOOPING);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


int hal_mld_snooping_set(zpl_uint32 type, zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, type);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_IGMP, HAL_MODULE_CMD_REQ, HAL_MLD_SNOOPING);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_igmp_ipcheck_enable(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_IGMP, HAL_MODULE_CMD_REQ, HAL_IGMP_IPCHECK);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_arp_snooping_set(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_IGMP, HAL_MODULE_CMD_REQ, HAL_ARP_COPYTOCPU);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_rarp_snooping_set(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_IGMP, HAL_MODULE_CMD_REQ, HAL_RARP_COPYTOCPU);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_dhcp_snooping_set(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_IGMP, HAL_MODULE_CMD_REQ, HAL_DHCP_COPYTOCPU);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}