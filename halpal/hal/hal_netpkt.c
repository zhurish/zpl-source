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
#include "hal_netpkt.h"


int hal_netpkt_send(ifindex_t ifindex, zpl_vlan_t vlanid, 
	zpl_uint8 pri, zpl_uchar *data, zpl_uint32 len)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	command = IPCCMD_SET(HAL_MODULE_CPU, HAL_MODULE_CMD_DATA, 0);
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	//zpl_skb_data_t
	hal_ipcmsg_create_header(&ipcmsg, command);
	hal_ipcmsg_data_set(&ipcmsg, ifindex, vlanid, pri);
	return hal_ipcmsg_send(IF_UNIT_ALL, &ipcmsg);
}
