/*
 * hal_qos.h
 *
 *  Created on: 2019年9月10日
 *      Author: DELL
 */

#ifndef __HAL_QOS_H__
#define __HAL_QOS_H__


#include "nsm_qos.h"


typedef struct sdk_qos_s
{
	//禁止使能qos
	int (*sdk_qos_enable_cb) (void *, BOOL);
	int (*sdk_qos_ipg_cb) (void *, BOOL tx, BOOL);


	//端口优先级模式
	int (*sdk_qos_base_cb) (void *, ifindex_t, nsm_qos_trust_e);


	int (*sdk_qos_8021q_enable_cb) (void *, ifindex_t, nsm_qos_trust_e);
	int (*sdk_qos_diffserv_enable_cb) (void *, ifindex_t, nsm_qos_trust_e);

	//端口优先级到queue映射
	int (*sdk_qos_port_map_queue_cb) (void *, ifindex_t, nsm_qos_priority_e, nsm_qos_queue_e);
	//差分服务到queue映射
	int (*sdk_qos_diffserv_map_queue_cb) (void *, ifindex_t, int, nsm_qos_queue_e);


	//队列到class的映射
	int (*sdk_qos_queue_map_class_cb) (void *, ifindex_t, nsm_qos_queue_e, nsm_qos_class_e);
	int (*sdk_qos_class_scheduling_cb) (void *,  nsm_qos_class_e, nsm_class_sched_t);
	int (*sdk_qos_class_weight_cb) (void *,  nsm_qos_class_e, int);

	//风暴
	int (*sdk_qos_storm_enable_cb) (void *, ifindex_t, BOOL, int);
	int (*sdk_qos_storm_rate_cb) (void *, ifindex_t, u_int ,
			u_int  );

	//端口限速
	int (*sdk_qos_port_egress_rate_cb) (void *, ifindex_t, u_int, u_int);
	int (*sdk_qos_port_ingress_rate_cb) (void *, ifindex_t, u_int, u_int);


	//CPU
	int (*sdk_qos_cpu_rate_cb) (void *, u_int, u_int);


	//remarking
	int (*sdk_qos_cfi_remarking_cb) (void *, ifindex_t, u_int, u_int);
	int (*sdk_qos_pcp_remarking_cb) (void *, ifindex_t, u_int, u_int);

	void *sdk_driver;
}sdk_qos_t;




int hal_qos_enable(BOOL enable);
int hal_qos_ipg_enable(BOOL enable);
int hal_qos_base_mode(ifindex_t ifindex, nsm_qos_trust_e enable);
int hal_qos_8021q_enable(ifindex_t ifindex, nsm_qos_trust_e enable);
int hal_qos_diffserv_enable(ifindex_t ifindex, nsm_qos_trust_e enable);
int hal_qos_port_map_queue(ifindex_t ifindex, nsm_qos_priority_e pri, nsm_qos_queue_e queue);
int hal_qos_diffserv_map_queue(ifindex_t ifindex, int diffserv, nsm_qos_queue_e queue);

//队列到class的映射
int hal_qos_queue_class(ifindex_t ifindex, nsm_qos_queue_e queue, nsm_qos_class_e class);
int hal_qos_queue_scheduling(nsm_qos_class_e class, nsm_class_sched_t type);
int hal_qos_queue_weight(nsm_qos_class_e class, int weight);

//风暴
int hal_qos_storm_mode(ifindex_t ifindex, BOOL enable, int mode);
int hal_qos_storm_rate_limit(ifindex_t ifindex, u_int limit, u_int burst_size);

//端口限速
int hal_qos_egress_rate_limit(ifindex_t ifindex, u_int limit, u_int burst_size);
int hal_qos_ingress_rate_limit(ifindex_t ifindex, u_int limit, u_int burst_size);
//CPU
int hal_qos_cpu_rate_limit(u_int limit, u_int burst_size);



#endif /* __HAL_QOS_H__ */
