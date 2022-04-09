/*
 * hal_vlan.c
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */


#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"

#include "hal_vlan.h"



int hal_vlan_enable(zpl_bool  enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, enable);
	hal_ipcmsg_putw(&ipcmsg, 0);
	hal_ipcmsg_putw(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN);
	return hal_ipcmsg_send_message(IF_UNIT_ALL,
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_vlan_create(vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, 1);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putw(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_CREATE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL,
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_vlan_destroy(vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putw(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_DELETE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL,
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_vlan_batch_create(vlan_t *vlan, int num)
{
	zpl_uint32 i = 0;
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, num);
	for(i = 0; i < num; i++)
		hal_ipcmsg_putw(&ipcmsg, vlan[i]);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_RANGE_CREATE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL,
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_vlan_batch_destroy(vlan_t *vlan, int num)
{
	zpl_uint32 i = 0;
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_putl(&ipcmsg, num);
	for(i = 0; i < num; i++)
		hal_ipcmsg_putw(&ipcmsg, vlan[i]);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_RANGE_DELETE);
	return hal_ipcmsg_send_message(IF_UNIT_ALL,
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_add_access_vlan(ifindex_t ifindex, vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 1);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putw(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_ACCESS);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_del_access_vlan(ifindex_t ifindex, vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putw(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_ACCESS);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_add_native_vlan(ifindex_t ifindex, vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 1);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putw(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_NATIVE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_del_native_vlan(ifindex_t ifindex, vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putw(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_NATIVE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_add_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 1);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putw(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_ALLOWE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_del_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putw(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_ALLOWE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_add_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end)
{
	vlan_t i = 0;
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 1);
	hal_ipcmsg_putl(&ipcmsg, end-start+1);
	for(i = start; i <= end; i++)
		hal_ipcmsg_putw(&ipcmsg, i);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_RANGE_ALLOWE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_del_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end)
{
	vlan_t i = 0;
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putl(&ipcmsg, end-start+1);
	for(i = start; i <= end; i++)
		hal_ipcmsg_putw(&ipcmsg, i);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_RANGE_ALLOWE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_set_vlan(ifindex_t ifindex, vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 1);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putw(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_PORT_BASE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_port_unset_vlan(ifindex_t ifindex, vlan_t vlan)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	HAL_ENTER_FUNC();
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, 0);
	hal_ipcmsg_putw(&ipcmsg, vlan);
	hal_ipcmsg_putw(&ipcmsg, 0);
	command = IPCCMD_SET(HAL_MODULE_VLAN, HAL_MODULE_CMD_REQ, HAL_VLAN_PORT_BASE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

#if 0
#if 0
/* vlan_mode_set            */  drv_vlan_mode_set,
/* vlan_mode_get            */  drv_vlan_mode_get,
/* port_vlan_pvid_set       */  drv_port_vlan_pvid_set,
/* port_vlan_pvid_get       */  drv_port_vlan_pvid_get,
/* port_vlan_set            */  drv_port_vlan_set,
/* port_vlan_get            */  drv_port_vlan_get,
/* vlan_property_set        */  drv_bcm53115_vlan_prop_set,
/* vlan_property_get        */  drv_bcm53115_vlan_prop_get,
/* vlan_prop_port_enable_set */ drv_bcm53115_vlan_prop_port_enable_set,
/* vlan_prop_port_enable_get */ drv_bcm53115_vlan_prop_port_enable_get,
/* vlan_vt_set              */  drv_bcm53115_vlan_vt_set,
/* vlan_vt_get              */  drv_bcm53115_vlan_vt_get,
/* vlan_vt_add              */  drv_bcm53115_vlan_vt_add,
/* vlan_vt_delete           */  drv_bcm53115_vlan_vt_delete,
/* vlan_vt_delete_all       */  drv_bcm53115_vlan_vt_delete_all,
#endif
int    (*sdk_port_set_native_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_unset_native_vlan)(ifindex_t, vlan_t);

int    (*sdk_port_set_access_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_unset_access_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_add_access_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_del_access_vlan)(ifindex_t, vlan_t);


int    (*sdk_port_set_trunk_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_unset_trunk_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_add_trunk_vlan)(ifindex_t, vlan_t);
int    (*sdk_port_del_trunk_vlan)(ifindex_t, vlan_t);
#endif
