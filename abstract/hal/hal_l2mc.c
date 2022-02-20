/*
 * hal_l2mc.c
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */


#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_l2mc.h"

//添加二层组播
/* Add/Remove an entry in the multicast table. */
int hal_l2mcast_addr_add(
    mac_t * mac, 
    vlan_t vid)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_puts(&ipcmsg, vid);
	hal_ipcmsg_put(&ipcmsg, mac, 6);
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_REQ, HAL_L2MCAST_ADD);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
/* Add/Remove an entry in the multicast table. */
int hal_l2mcast_addr_remove(
    mac_t * mac, 
    vlan_t vid)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_puts(&ipcmsg, vid);
	hal_ipcmsg_put(&ipcmsg, mac, 6);

	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_REQ, HAL_L2MCAST_DEL);
	return hal_ipcmsg_send_message(IF_UNIT_ALL, 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

//端口加入二层组播	
/* Add a given port to the membership of a given multicast group. */
int hal_l2mcast_join(mac_t *mcMacAddr, 
    vlan_t vlanId, 
    ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_puts(&ipcmsg, vlanId);
	hal_ipcmsg_put(&ipcmsg, mcMacAddr, 6);
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_REQ, HAL_L2MCAST_JOIN);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
/* Remove a given port from the membership of a given multicast group. */
int hal_l2mcast_leave(mac_t *mcMacAddr, 
    vlan_t vlanId, 
    ifindex_t ifindex)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_puts(&ipcmsg, vlanId);
	hal_ipcmsg_put(&ipcmsg, mcMacAddr, 6);
	command = IPCCMD_SET(HAL_MODULE_MAC, HAL_MODULE_CMD_REQ, HAL_L2MCAST_LEAVE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
