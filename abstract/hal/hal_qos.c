/*
 * hal_qos.c
 *
 *  Created on: 2019年9月10日
 *      Author: DELL
 */
#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "nsm_interface.h"
#include <log.h>
#include "os_list.h"

#include "nsm_client.h"

#include "hal_qinq.h"
#include "hal_driver.h"



int hal_qos_enable(ospl_bool enable)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_enable_cb)
		return hal_driver->qos_tbl->sdk_qos_enable_cb(hal_driver->driver, enable);
	return ERROR;
}

int hal_qos_ipg_enable(ospl_bool enable)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_ipg_cb)
		return hal_driver->qos_tbl->sdk_qos_ipg_cb(hal_driver->driver, ospl_true, enable);
	return ERROR;
}

int hal_qos_base_mode(ifindex_t ifindex, nsm_qos_trust_e enable)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_base_cb)
		return hal_driver->qos_tbl->sdk_qos_base_cb(hal_driver->driver, ifindex, enable);
	return ERROR;
}

int hal_qos_8021q_enable(ifindex_t ifindex, nsm_qos_trust_e enable)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_8021q_enable_cb)
		return hal_driver->qos_tbl->sdk_qos_8021q_enable_cb(hal_driver->driver, ifindex, enable);
	return ERROR;
}

int hal_qos_diffserv_enable(ifindex_t ifindex, nsm_qos_trust_e enable)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_diffserv_enable_cb)
		return hal_driver->qos_tbl->sdk_qos_diffserv_enable_cb(hal_driver->driver, ifindex, enable);
	return ERROR;
}


int hal_qos_port_map_queue(ifindex_t ifindex, nsm_qos_priority_e pri, nsm_qos_queue_e queue)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_port_map_queue_cb)
		return hal_driver->qos_tbl->sdk_qos_port_map_queue_cb(hal_driver->driver, ifindex, pri, queue);
	return ERROR;
}

int hal_qos_diffserv_map_queue(ifindex_t ifindex, ospl_uint32 diffserv, nsm_qos_queue_e queue)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_diffserv_map_queue_cb)
		return hal_driver->qos_tbl->sdk_qos_diffserv_map_queue_cb(hal_driver->driver, ifindex, diffserv, queue);
	return ERROR;
}


//队列到class的映射
int hal_qos_queue_class(ifindex_t ifindex, nsm_qos_queue_e queue, nsm_qos_class_e class)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_queue_map_class_cb)
		return hal_driver->qos_tbl->sdk_qos_queue_map_class_cb(hal_driver->driver, ifindex, queue, class);
	return ERROR;
}

int hal_qos_queue_scheduling(nsm_qos_class_e class, nsm_class_sched_t type)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_class_scheduling_cb)
		return hal_driver->qos_tbl->sdk_qos_class_scheduling_cb(hal_driver->driver, class, type);
	return ERROR;
}

int hal_qos_queue_weight(nsm_qos_class_e class, ospl_uint32 weight)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_class_weight_cb)
		return hal_driver->qos_tbl->sdk_qos_class_weight_cb(hal_driver->driver, class, weight);
	return ERROR;
}

//风暴
int hal_qos_storm_mode(ifindex_t ifindex, ospl_bool enable, ospl_uint32 mode)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_storm_enable_cb)
		return hal_driver->qos_tbl->sdk_qos_storm_enable_cb(hal_driver->driver, ifindex, enable, mode);
	return ERROR;
}

int hal_qos_storm_rate_limit(ifindex_t ifindex, ospl_uint32 limit, ospl_uint32 burst_size)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_storm_rate_cb)
		return hal_driver->qos_tbl->sdk_qos_storm_rate_cb(hal_driver->driver, ifindex, limit, burst_size);
	return ERROR;
}


//端口限速

int hal_qos_egress_rate_limit(ifindex_t ifindex, ospl_uint32 limit, ospl_uint32 burst_size)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_port_egress_rate_cb)
		return hal_driver->qos_tbl->sdk_qos_port_egress_rate_cb(hal_driver->driver, ifindex, limit, burst_size);
	return ERROR;
}

int hal_qos_ingress_rate_limit(ifindex_t ifindex, ospl_uint32 limit, ospl_uint32 burst_size)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_port_ingress_rate_cb)
		return hal_driver->qos_tbl->sdk_qos_port_ingress_rate_cb(hal_driver->driver, ifindex, limit, burst_size);
	return ERROR;
}

//CPU
int hal_qos_cpu_rate_limit(ospl_uint32 limit, ospl_uint32 burst_size)
{
	if(hal_driver && hal_driver->qos_tbl && hal_driver->qos_tbl->sdk_qos_cpu_rate_cb)
		return hal_driver->qos_tbl->sdk_qos_cpu_rate_cb(hal_driver->driver, limit, burst_size);
	return ERROR;
}



