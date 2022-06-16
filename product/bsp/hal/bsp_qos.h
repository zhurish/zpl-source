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
#ifndef ZPL_SDK_USER

enum hal_qos_cmd 
{
    HAL_QOS_NONE,
	HAL_QOS_EN,
	HAL_QOS_IPG,
	HAL_QOS_BASE_TRUST,
	HAL_QOS_8021Q,
	HAL_QOS_DIFFSERV,

	//CLASS
	HAL_QOS_QUEUE_MAP_CLASS,
	HAL_QOS_CLASS_SCHED,
	HAL_QOS_CLASS_WEIGHT,

	//INPUT
	HAL_QOS_8021Q_MAP_QUEUE,
	HAL_QOS_DIFFSERV_MAP_QUEUE,
	HAL_QOS_IPPRE_MAP_QUEUE,
    HAL_QOS_MPLSEXP_MAP_QUEUE,		//
	HAL_QOS_PORT_MAP_QUEUE,

	//OUTPUT
	HAL_QOS_QUEUE_SCHED,
	HAL_QOS_QUEUE_WEIGHT,
	HAL_QOS_QUEUE_RATELIMIT,
	HAL_QOS_PRI_REMARK,

	//CPU
	HAL_QOS_CPU_RATELIMIT,

	HAL_QOS_PORT_INRATELIMIT,
	HAL_QOS_PORT_OUTRATELIMIT,
};



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

typedef enum
{
	NSM_QOS_TRUST_NONE = 0,
	NSM_QOS_TRUST_PORT,
	NSM_QOS_TRUST_EXP,
	NSM_QOS_TRUST_COS,
	NSM_QOS_TRUST_DSCP,
	NSM_QOS_TRUST_IP_PRE,
}nsm_qos_trust_e;

#endif
typedef struct sdk_qos_s
{
	//禁止使能qos
	int (*sdk_qos_enable_cb) (void *, zpl_bool);

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
