/*
 * bsp_qos.h
 *
 *  Created on: 2019年9月10日
 *      Author: DELL
 */

#ifndef __BSP_QOS_H__
#define __BSP_QOS_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "nsm_qos.h"


typedef struct sdk_qos_s
{
	//禁止使能qos
	int (*sdk_qos_enable_cb) (void *, zpl_bool);
	int (*sdk_qos_ipg_cb) (void *, zpl_bool tx, zpl_bool);


	//端口优先级模式
	int (*sdk_qos_base_cb) (void *, zpl_phyport_t, nsm_qos_trust_e);


	int (*sdk_qos_8021q_enable_cb) (void *, zpl_phyport_t, nsm_qos_trust_e);
	int (*sdk_qos_diffserv_enable_cb) (void *, zpl_phyport_t, nsm_qos_trust_e);
#ifdef NSM_QOS_CLASS_PRIORITY
	//端口优先级到queue映射
	int (*sdk_qos_port_map_queue_cb) (void *, zpl_phyport_t, nsm_qos_priority_e, nsm_qos_queue_e);
#endif
	//差分服务到queue映射
	int (*sdk_qos_diffserv_map_queue_cb) (void *, zpl_phyport_t, zpl_uint32, nsm_qos_queue_e);

#ifdef NSM_QOS_CLASS_PRIORITY
	//队列到class的映射
	int (*sdk_qos_queue_map_class_cb) (void *, zpl_phyport_t, nsm_qos_queue_e, nsm_qos_class_e);
	int (*sdk_qos_class_scheduling_cb) (void *,  nsm_qos_class_e, nsm_class_sched_t);
	int (*sdk_qos_class_weight_cb) (void *,  nsm_qos_class_e, zpl_uint32);
#endif
	//风暴
	int (*sdk_qos_storm_rate_cb) (void *, zpl_phyport_t, zpl_uint32 ,
			zpl_uint32  );

	//端口限速
	int (*sdk_qos_port_egress_rate_cb) (void *, zpl_phyport_t, zpl_uint32);
	int (*sdk_qos_port_ingress_rate_cb) (void *, zpl_phyport_t, zpl_uint32);


	//CPU
	int (*sdk_qos_cpu_rate_cb) (void *, zpl_uint32, zpl_uint32);


	//remarking
	int (*sdk_qos_cfi_remarking_cb) (void *, zpl_phyport_t, zpl_uint32, zpl_uint32);
	int (*sdk_qos_pcp_remarking_cb) (void *, zpl_phyport_t, zpl_uint32, zpl_uint32);

}sdk_qos_t;





extern sdk_qos_t sdk_qos;
extern int bsp_qos_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_QOS_H__ */
