/*
 * nsm_qos.h
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */

#ifndef __NSM_QOS_H__
#define __NSM_QOS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NSM_QOS_PORT_QUEUE_NUM_4	4
#define NSM_QOS_PORT_QUEUE_NUM_8	8
#define NSM_QOS_PORT_QUEUE_NUM	NSM_QOS_PORT_QUEUE_NUM_4

//#define NSM_QOS_USERPRI_MAP_PRIORITY //用户优先级到内部优先级映射
//#define NSM_QOS_PRIORITY_MAP_USERPRI  //内部优先级到用户优先级映射

#define NSM_QOS_USERPRI_MAP_QUEUE //用户优先级到队列映射
//#define NSM_QOS_QUEUE_MAP_USERPRI  //队列到用户优先级映射

//#define NSM_QOS_PRIORITY_MAP_QUEUE //内部优先级到队列映射
//#define NSM_QOS_QUEUE_MAP_PRIORITY  //队列到内部优先级映射

//#define NSM_QOS_QUEUE_MAP_CLASS
/*
 * trust pri on input
 */
typedef enum
{
	NSM_QOS_TRUST_NONE = 0,
	NSM_QOS_TRUST_PORT,
	NSM_QOS_TRUST_EXP,
	NSM_QOS_TRUST_COS,
	NSM_QOS_TRUST_DSCP,
	NSM_QOS_TRUST_IP_PRE,
}nsm_qos_trust_e;

#ifdef NSM_QOS_QUEUE_MAP_CLASS
typedef enum
{
	NSM_QOS_CLASS_NONE = 0,
    NSM_QOS_CLASS_0 = 1,
    NSM_QOS_CLASS_1,
    NSM_QOS_CLASS_2,
    NSM_QOS_CLASS_3,

    NSM_QOS_CLASS_4,
    NSM_QOS_CLASS_5,
    NSM_QOS_CLASS_MAX,
}nsm_qos_class_e;
#endif

typedef enum
{
	NSM_QOS_QUEUE_NONE = 0,
    NSM_QOS_QUEUE_0 = 1,
    NSM_QOS_QUEUE_1,
    NSM_QOS_QUEUE_2,
    NSM_QOS_QUEUE_3,
#if NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_8
    NSM_QOS_QUEUE_4,
    NSM_QOS_QUEUE_5,
    NSM_QOS_QUEUE_6,
    NSM_QOS_QUEUE_7,
#endif
    NSM_QOS_QUEUE_MAX,
}nsm_qos_queue_e;


typedef enum
{
	NSM_QOS_PRI_NONE = 0,
    NSM_QOS_PRI_0 = 1,
    NSM_QOS_PRI_1,
    NSM_QOS_PRI_2,
    NSM_QOS_PRI_3,
    NSM_QOS_PRI_4,
    NSM_QOS_PRI_5,
    NSM_QOS_PRI_6,
    NSM_QOS_PRI_7,
    NSM_QOS_PRI_MAX,
}nsm_qos_priority_e;

#define NSM_QOS_DSCP_PRI_MAX	64
/*
*	pri:	0     1     2     3     4     5     6     7
*  dscp:   0-7  80-15 16-23 24-31 32-39 40-47 48-55  56-63
*
*/

typedef enum nsm_queue_sched_e
{
	NSM_QOS_MODE_NONE = 0,
    NSM_QOS_MODE_PQ = 1,
    NSM_QOS_MODE_SP = NSM_QOS_MODE_PQ,	//strict priority	
    NSM_QOS_MODE_WRR,//weighted round robin
	NSM_QOS_MODE_WRED,//random-detect
	NSM_QOS_MODE_DRRW,//drr-weight
    NSM_QOS_MODE_WFQ,
    NSM_QOS_MODE_FQ,
}nsm_queue_sched_t;

#ifdef NSM_QOS_QUEUE_MAP_CLASS
typedef enum nsm_class_sched_e {
    NSM_CLASS_SCHED_WRR = 0,//weighted round robin
	NSM_CLASS_SCHED_PQ, //strict priority
} nsm_class_sched_t;
#endif

typedef enum
{
	NSM_QOS_DIR_INBOUND = 1,
	NSM_QOS_DIR_OUTBOUND,
}nsm_qos_dir_e;


typedef enum
{
	NSM_QOS_LIMIT_NONE = 0,
	NSM_QOS_LIMIT_BROADCAST = 1,
	NSM_QOS_LIMIT_UNICAST,
	NSM_QOS_LIMIT_MULTICAST,

}nsm_qos_limit_e;

typedef struct nsm_qos_limit_s
{
	zpl_uint32			qos_cir;//承诺信息速率
	zpl_uint32			qos_pir;//峰值信息速率
	//zpl_uint32			qos_bir;
	zpl_uint32			qos_cbs;//承诺突发尺寸突发尺寸
	//zpl_uint32			qos_pbs;//峰值突发尺寸
	//zpl_uint32			qos_ebs;//超出突发尺寸
}nsm_qos_limit_t;


typedef enum
{
	QOS_MAP_NONE = 0,
	QOS_MAP_PORT,
	QOS_MAP_COS,
	QOS_MAP_IP_PRE,
	QOS_MAP_DSCP,
	QOS_MAP_EXP,
} nsm_qos_map_e;


typedef struct 
{
	zpl_char	*service_policy;
}nsm_service_policy_t;

typedef struct nsm_qos_s
{
	ifindex_t			ifindex;
	// input
	//队列调度
	//队列调度模式（出方向应用）
	nsm_queue_sched_t		qos_queue_sched[NSM_QOS_QUEUE_MAX];//当前队列调度
	nsm_queue_sched_t		qos_queue_sched_default[NSM_QOS_QUEUE_MAX];//默认队列调度
	zpl_char				qos_queue_sched_weight[NSM_QOS_QUEUE_MAX];//调度权限
#ifdef NSM_QOS_QUEUE_MAP_CLASS
	//队列到class的映射
	zpl_bool			qos_class_enable;
	nsm_qos_class_e		qos_class[NSM_QOS_QUEUE_MAX]; //queue map to class
	nsm_qos_class_e		qos_class_default[NSM_QOS_QUEUE_MAX];
	nsm_class_sched_t	qos_class_sched[NSM_QOS_CLASS_MAX];
	nsm_class_sched_t	qos_class_sched_default[NSM_QOS_CLASS_MAX];
	nsm_class_sched_t	qos_class_sched_weight[NSM_QOS_CLASS_MAX];
#endif

	nsm_qos_trust_e 	qos_trust;//端口信任

	nsm_qos_queue_e		qos_port_input_queue;//端口到优先级的映射
	nsm_qos_queue_e		qos_port_input_queue_default;

#ifdef NSM_QOS_USERPRI_MAP_QUEUE
	//各类优先级到queue的映射
	nsm_qos_queue_e	cos_map_queue[NSM_QOS_PRI_MAX];
	nsm_qos_queue_e	cos_map_queue_default[NSM_QOS_PRI_MAX];
	nsm_qos_queue_e	ip_prec_map_queue[NSM_QOS_PRI_MAX];
	nsm_qos_queue_e	ip_prec_map_queue_default[NSM_QOS_PRI_MAX];
	nsm_qos_queue_e	mplsexp_map_queue[NSM_QOS_PRI_MAX];
	nsm_qos_queue_e	mplsexp_map_queue_default[NSM_QOS_PRI_MAX];
	nsm_qos_queue_e	dscp_map_queue[NSM_QOS_DSCP_PRI_MAX];
	nsm_qos_queue_e	dscp_map_queue_default[NSM_QOS_DSCP_PRI_MAX];
#endif

#ifdef NSM_QOS_PRIORITY_MAP_QUEUE
	//output 内部优先级到队列的映射
	nsm_qos_queue_e	qos_priority_map_queue[NSM_QOS_PRI_MAX]; //priority map to queue
	nsm_qos_queue_e qos_priority_map_queue_default[NSM_QOS_PRI_MAX];
#endif

#ifdef NSM_QOS_QUEUE_MAP_PRIORITY
	//input 用户优先级到内部优先级的映射（队列到内部优先级的映射）
	nsm_qos_priority_e	qos_queue_map_priority[NSM_QOS_QUEUE_MAX]; // priority map to priority
	nsm_qos_priority_e  qos_queue_map_priority_default[NSM_QOS_QUEUE_MAX];
#endif

#ifdef NSM_QOS_USERPRI_MAP_PRIORITY
	// user priority map to priority
	nsm_qos_priority_e	cos_map_priority[NSM_QOS_PRI_MAX];
	nsm_qos_priority_e	cos_map_priority_default[NSM_QOS_PRI_MAX];
	nsm_qos_priority_e	ip_prec_map_priority[NSM_QOS_PRI_MAX];
	nsm_qos_priority_e	ip_prec_map_priority_default[NSM_QOS_PRI_MAX];
	nsm_qos_priority_e	mplsexp_map_priority[NSM_QOS_PRI_MAX];
	nsm_qos_priority_e	mplsexp_map_priority_default[NSM_QOS_PRI_MAX];
	nsm_qos_priority_e	dscp_map_priority[NSM_QOS_DSCP_PRI_MAX];
	nsm_qos_priority_e	dscp_map_priority_default[NSM_QOS_DSCP_PRI_MAX];
#endif

	//端口优先级替换
	nsm_qos_priority_e	qos_cos_replace;
	nsm_qos_priority_e	qos_dscp_replace;
	//端口限速
	nsm_qos_limit_t		qos_input_limit;	//rate limit control
	nsm_qos_limit_t		qos_output_limit;

	//限速策略
	zpl_char	*qos_policer_input_acl;//policer-aggregate
	zpl_char	*qos_policer_output_acl;//policer-aggregate

	nsm_service_policy_t service_policy_output;
	nsm_service_policy_t service_policy_input;

	zpl_bool			qos_shaping;
	void        *mutex;
}nsm_qos_t;


typedef struct Global_Qos_s
{
	zpl_bool			qos_enable;
	zpl_bool			qos_shaping;
#ifdef NSM_QOS_QUEUE_MAP_CLASS
	//队列到class的映射
	zpl_bool			qos_class_enable;
	nsm_qos_class_e		qos_class[NSM_QOS_QUEUE_MAX]; //queue map to class
	nsm_class_sched_t	qos_class_sched[NSM_QOS_CLASS_MAX];
#endif

	//output 内部优先级到队列的映射
	nsm_qos_queue_e	qos_pri_map_queue[NSM_QOS_PRI_MAX]; //priority map to queue
	//input 用户优先级到内部优先级的映射（队列到内部优先级的映射）
	nsm_qos_priority_e	qos_queue_map_pri[NSM_QOS_QUEUE_MAX]; // queue map to priority
	//void        *mutex;
}Global_Qos_t;

#define IF_NSM_QOS_DATA_LOCK(qos)   if(qos && qos->mutex) os_mutex_lock(qos->mutex, OS_WAIT_FOREVER)
#define IF_NSM_QOS_DATA_UNLOCK(qos) if(qos && qos->mutex) os_mutex_unlock(qos->mutex)

extern int nsm_qos_global_enable(zpl_bool enable);
extern zpl_bool nsm_qos_global_get(void);
extern int nsm_qos_shaping_global_enable(zpl_bool enable);
extern zpl_bool nsm_qos_shaping_global_get(void);
/*
默认的映射关系表：
COS到内部DSCP的映射关系表
COS值	   0	1	2	3	4	5	6	7
内部DSCP值  0		8	16	24	32	40	48	56

IP优先级到内部DSCP的映射关系表
IP优先级     0	1	2	3	4	5	6	7
内部DSCP值	0	8	16	24	32	40	48	56

内部DSCP到COS的映射关系表
内部DSCP值	0~7		8~15	16~23	24~31	32~39	40~47	48~55	56~63
COS值		0		1		2		3		4		5		6		7
*/
extern int nsm_dscp_to_cos(zpl_uint32 dscp);
extern int nsm_cos_to_dscp(zpl_uint32 cos);


extern int nsm_qos_rate_set_api(struct interface *ifp, nsm_qos_dir_e qos_dir,
		nsm_qos_limit_t *rate);
extern int nsm_qos_rate_get_api(struct interface *ifp, nsm_qos_dir_e qos_dir,
		nsm_qos_limit_t *rate);

extern int nsm_qos_trust_set_api(struct interface *ifp, nsm_qos_trust_e trust);
extern int nsm_qos_trust_get_api(struct interface *ifp, nsm_qos_trust_e *trust);

extern int nsm_qos_queue_sched_mode_get_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_queue_sched_t *mode, int *sched_weight);
extern int nsm_qos_queue_sched_mode_set_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_queue_sched_t mode, int sched_weight);

extern int nsm_qos_port_map_queue_set_api(struct interface *ifp, nsm_qos_queue_e queue);
extern int nsm_qos_port_map_queue_get_api(struct interface *ifp, nsm_qos_queue_e *queue);


#ifdef NSM_QOS_USERPRI_MAP_QUEUE
extern int nsm_qos_cos_map_queue_set_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e queue);
extern int nsm_qos_cos_map_queue_get_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e *queue);

extern int nsm_qos_exp_map_queue_set_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e queue);
extern int nsm_qos_exp_map_queue_get_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e *queue);

extern int nsm_qos_ippre_map_queue_set_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e queue);
extern int nsm_qos_ippre_map_queue_get_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e *queue);

extern int nsm_qos_dscp_map_queue_set_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e queue);
extern int nsm_qos_dscp_map_queue_get_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e *queue);
#endif

#ifdef NSM_QOS_QUEUE_MAP_CLASS
/*
 * queue map to class
 */
extern int nsm_qos_class_map_set_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_class_e class);
extern int nsm_qos_class_map_get_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_class_e *class);

/*
 * class sched
 */
extern int nsm_qos_class_sched_set_api(struct interface *ifp, nsm_qos_class_e class, nsm_class_sched_t mode, int sched_weight);
extern int nsm_qos_class_sched_get_api(struct interface *ifp, nsm_qos_class_e class, nsm_class_sched_t *mode, int *sched_weight);
#endif

/*
 * flow shaping
 */
extern int nsm_qos_shaping_set_api(struct interface *ifp, zpl_bool enable);
extern int nsm_qos_shaping_get_api(struct interface *ifp, zpl_bool *enable);



#ifdef NSM_QOS_PRIORITY_MAP_QUEUE
/*
 * inside priority map to queue
 */
extern int nsm_qos_priority_map_queue_set_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_queue_e queue);
extern int nsm_qos_priority_map_queue_get_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_queue_e *queue);
#endif
#ifdef NSM_QOS_QUEUE_MAP_PRIORITY
/*
 * queue map to inside priority
 */
extern int nsm_qos_queue_map_priority_set_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_priority_e priority);
extern int nsm_qos_queue_map_priority_get_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_priority_e *priority);
#endif

#ifdef NSM_QOS_USERPRI_MAP_PRIORITY
/*
 * user priority map to inside priority
 */
extern int nsm_qos_cos_map_priority_set_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e priority);
extern int nsm_qos_cos_map_priority_get_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e *priority);
extern int nsm_qos_ipprec_map_priority_set_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e priority);
extern int nsm_qos_ipprec_map_priority_get_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e *priority);
extern int nsm_qos_mplsexp_map_priority_set_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e priority);
extern int nsm_qos_mplsexp_map_priority_get_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e *priority);
extern int nsm_qos_dscp_map_priority_set_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e priority);
extern int nsm_qos_dscp_map_priority_get_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e *priority);
#endif

extern int nsm_qos_cos_replace_set_api(struct interface *ifp, nsm_qos_priority_e priority);
extern int nsm_qos_cos_replace_get_api(struct interface *ifp, nsm_qos_priority_e *priority);

extern int nsm_qos_dscp_replace_set_api(struct interface *ifp, nsm_qos_priority_e priority);
extern int nsm_qos_dscp_replace_get_api(struct interface *ifp, nsm_qos_priority_e *priority);

extern int nsm_qos_service_policy_set_api(struct interface *ifp, int input, zpl_char * service_policy);
extern int nsm_qos_service_policy_get_api(struct interface *ifp, int input, zpl_char *service_policy);


extern int nsm_qos_init(void);
extern int nsm_qos_exit(void);
#ifdef ZPL_SHELL_MODULE
extern int cmd_qos_init(void);
extern int nsm_qos_interface_write_config(struct vty *vty, struct interface *ifp);
extern int nsm_qos_interface_show(struct vty *vty, struct interface *ifp);
#endif 


#ifdef __cplusplus
}
#endif

#endif /* __NSM_QOS_H__ */
