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

	//风暴
	int (*sdk_qos_storm_rate_cb) (void *, zpl_phyport_t, zpl_uint32 ,
			zpl_uint32  );

	//端口限速
	int (*sdk_qos_port_egress_rate_cb) (void *, zpl_phyport_t, zpl_uint32);
	int (*sdk_qos_port_ingress_rate_cb) (void *, zpl_phyport_t, zpl_uint32);

	//CPU
	int (*sdk_qos_cpu_rate_cb) (void *, zpl_uint32, zpl_uint32);
	/***********************************************************************/
	/***********************************************************************/
	//端口信任那类优先级
	int (*sdk_qos_trust_enable_cb) (void *, zpl_phyport_t, nsm_qos_trust_e, zpl_bool enable);

	//8021P优先级到内部优先级映射
	int (*sdk_qos_8021p_map_priority_cb) (void *, zpl_phyport_t port, int q8021p, int priority);
	//diffserv优先级到内部优先级映射
	int (*sdk_qos_diffserv_map_priority_cb) (void *, zpl_phyport_t, int diffserv, int priority);
	//内部优先级到queue队列的映射
	int (*sdk_qos_priority_map_queue_cb) (void *, zpl_phyport_t, int priority, int queue);
	//设置queue调度方式
	int (*sdk_qos_queue_scheduling_cb) (void *, zpl_phyport_t, int mode);
	//设置queue调度权限
	int (*sdk_qos_queue_weight_cb) (void *, zpl_phyport_t, int queue, int weight);
	//8021P,diffserv优先级映射恢复默认
	int (*sdk_qos_map_default_cb) (void *, zpl_phyport_t port, zpl_uint32 dif);

	int (*sdk_qos_remarking_map_cb) (void *, zpl_phyport_t port, zpl_uint32 ipri, zpl_uint32 outpri);
	int (*sdk_qos_remarking_map_default_cb) (void *, zpl_phyport_t port);
}sdk_qos_t;





extern sdk_qos_t sdk_qos;
extern int bsp_qos_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_QOS_H__ */
