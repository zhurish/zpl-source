/*
 * hal_qos.c
 *
 *  Created on: 2019年9月10日
 *      Author: DELL
 */
#include "zpl_include.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "nsm_interface.h"
#include <log.h>
#include "os_list.h"

#include "hal_qos.h"

#include "hal_qinq.h"
#include "hal_driver.h"



int hal_qos_enable(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	//hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_QOS, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_QOS_EN);
	return hal_ipcmsg_send_message(-1,//IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_qos_ipg_enable(zpl_bool enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	//hal_ipcmsg_port_set(&ipcmsg, ifindex);
	command = IPCCMD_SET(HAL_MODULE_QOS, enable?HAL_MODULE_CMD_ENABLE:HAL_MODULE_CMD_DISABLE, HAL_QOS_IPG);
	return hal_ipcmsg_send_message(-1,//IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_qos_base_mode(ifindex_t ifindex, nsm_qos_trust_e enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putw(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_QOS, HAL_MODULE_CMD_SET, HAL_QOS_BASE_TRUST);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_qos_8021q_enable(ifindex_t ifindex, nsm_qos_trust_e enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putw(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_QOS, HAL_MODULE_CMD_SET, HAL_QOS_8021Q);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_qos_diffserv_enable(ifindex_t ifindex, nsm_qos_trust_e enable)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putw(&ipcmsg, enable);
	command = IPCCMD_SET(HAL_MODULE_QOS, HAL_MODULE_CMD_SET, HAL_QOS_DIFFSERV);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

#ifdef NSM_QOS_CLASS_PRIORITY
int hal_qos_port_map_queue(ifindex_t ifindex, nsm_qos_priority_e pri, nsm_qos_queue_e queue)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, pri);
	hal_ipcmsg_putc(&ipcmsg, queue);
	command = IPCCMD_SET(HAL_MODULE_QOS, HAL_MODULE_CMD_SET, HAL_QOS_PORT_MAP_QUEUE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
#endif
int hal_qos_diffserv_map_queue(ifindex_t ifindex, zpl_uint32 diffserv, nsm_qos_queue_e queue)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, diffserv);
	hal_ipcmsg_putc(&ipcmsg, queue);
	command = IPCCMD_SET(HAL_MODULE_QOS, HAL_MODULE_CMD_SET, HAL_QOS_DIFFSERV_MAP_QUEUE);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

#ifdef NSM_QOS_CLASS_PRIORITY
//队列到class的映射
int hal_qos_queue_class(ifindex_t ifindex, nsm_qos_queue_e queue, nsm_qos_class_e class)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, queue);
	hal_ipcmsg_putc(&ipcmsg, class);
	command = IPCCMD_SET(HAL_MODULE_QOS, HAL_MODULE_CMD_SET, HAL_QOS_QUEUE_MAP_CLASS);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_qos_queue_scheduling(ifindex_t ifindex,nsm_qos_class_e class, nsm_class_sched_t type)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, class);
	hal_ipcmsg_putc(&ipcmsg, type);
	command = IPCCMD_SET(HAL_MODULE_QOS, HAL_MODULE_CMD_SET, HAL_QOS_CLASS_SCHED);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_qos_queue_weight(ifindex_t ifindex,nsm_qos_class_e class, zpl_uint32 weight)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, class);
	hal_ipcmsg_putc(&ipcmsg, weight);
	command = IPCCMD_SET(HAL_MODULE_QOS, HAL_MODULE_CMD_SET, HAL_QOS_CLASS_WEIGHT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}
#endif

//风暴
int hal_qos_storm_rate_limit(ifindex_t ifindex, zpl_uint32 mode, zpl_uint32 limit, zpl_uint32 burst_size)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putc(&ipcmsg, mode);
	hal_ipcmsg_putl(&ipcmsg, limit);
	hal_ipcmsg_putl(&ipcmsg, burst_size);
	command = IPCCMD_SET(HAL_MODULE_QOS, HAL_MODULE_CMD_SET, HAL_QOS_STORM_RATELIMIT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}


//端口限速

int hal_qos_egress_rate_limit(ifindex_t ifindex, zpl_uint32 limit, zpl_uint32 burst_size)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, limit);
	hal_ipcmsg_putl(&ipcmsg, burst_size);
	command = IPCCMD_SET(HAL_MODULE_QOS, HAL_MODULE_CMD_SET, HAL_QOS_PORT_OUTRATELIMIT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

int hal_qos_ingress_rate_limit(ifindex_t ifindex, zpl_uint32 limit, zpl_uint32 burst_size)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, limit);
	hal_ipcmsg_putl(&ipcmsg, burst_size);
	command = IPCCMD_SET(HAL_MODULE_QOS, HAL_MODULE_CMD_SET, HAL_QOS_PORT_INRATELIMIT);
	return hal_ipcmsg_send_message(IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}

//CPU
int hal_qos_cpu_rate_limit(zpl_uint32 limit, zpl_uint32 burst_size)
{
	zpl_uint32 command = 0;
	struct hal_ipcmsg ipcmsg;
	char buf[512];
	hal_ipcmsg_msg_init(&ipcmsg, buf, sizeof(buf));
	//hal_ipcmsg_port_set(&ipcmsg, ifindex);
	hal_ipcmsg_putl(&ipcmsg, limit);
	hal_ipcmsg_putl(&ipcmsg, burst_size);
	command = IPCCMD_SET(HAL_MODULE_QOS, HAL_MODULE_CMD_SET, HAL_QOS_CPU_RATELIMIT);
	return hal_ipcmsg_send_message(-1,//IF_IFINDEX_UNIT_GET(ifindex), 
		command, buf, hal_ipcmsg_msglen_get(&ipcmsg));
}



