/*
 * hal_qos.h
 *
 *  Created on: 2019年9月10日
 *      Author: DELL
 */

#ifndef __HAL_QOS_H__
#define __HAL_QOS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nsm_qos.h"





typedef struct hal_qos_param_s
{
	zpl_bool enable;
	zpl_uint32 value;
	zpl_uint32 limit;
	zpl_uint32 burst_size;
	zpl_uint32 mode;
	zpl_uint32 pri;
	zpl_uint32 diffserv;
	zpl_uint32 queue;
	zpl_uint32 class;
	zpl_uint32 type;
	zpl_uint32 weight;
}hal_qos_param_t;

int hal_qos_enable(zpl_bool enable);
int hal_qos_ipg_enable(zpl_bool enable);
int hal_qos_base_mode(ifindex_t ifindex, nsm_qos_trust_e enable);
int hal_qos_8021q_enable(ifindex_t ifindex, nsm_qos_trust_e enable);
int hal_qos_diffserv_enable(ifindex_t ifindex, nsm_qos_trust_e enable);
#ifdef NSM_QOS_CLASS_PRIORITY
int hal_qos_port_map_queue(ifindex_t ifindex, nsm_qos_priority_e pri, nsm_qos_queue_e queue);
#endif
int hal_qos_diffserv_map_queue(ifindex_t ifindex, zpl_uint32 diffserv, nsm_qos_queue_e queue);

#ifdef NSM_QOS_CLASS_PRIORITY
//队列到class的映射
int hal_qos_queue_class(ifindex_t ifindex, nsm_qos_queue_e queue, nsm_qos_class_e class);
int hal_qos_queue_scheduling(ifindex_t ifindex, nsm_qos_class_e class, nsm_class_sched_t type);
int hal_qos_queue_weight(ifindex_t ifindex, nsm_qos_class_e class, zpl_uint32 weight);
#endif


//端口限速
int hal_qos_egress_rate_limit(ifindex_t ifindex, zpl_uint32 limit, zpl_uint32 burst_size);
int hal_qos_ingress_rate_limit(ifindex_t ifindex, zpl_uint32 limit, zpl_uint32 burst_size);
//CPU
int hal_qos_cpu_rate_limit(zpl_uint32 limit, zpl_uint32 burst_size);



#ifdef __cplusplus
}
#endif

#endif /* __HAL_QOS_H__ */
