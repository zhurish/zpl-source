/*
 * nsm_qos.h
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */

#ifndef __NSM_NSM_QOS_H_
#define __NSM_NSM_QOS_H_

#ifdef __cplusplus
extern "C" {
#endif


//#define
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

typedef enum
{
	NSM_QOS_QUEUE_NONE = 0,
    NSM_QOS_QUEUE_0 = 1,
    NSM_QOS_QUEUE_1,
    NSM_QOS_QUEUE_2,
    NSM_QOS_QUEUE_3,
    NSM_QOS_QUEUE_4,
    NSM_QOS_QUEUE_5,
    NSM_QOS_QUEUE_6,
    NSM_QOS_QUEUE_7,
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

typedef enum
{
	NSM_QOS_MODE_NONE = 0,
    NSM_QOS_MODE_PQ = 1,
    NSM_QOS_MODE_WFQ,
    NSM_QOS_MODE_FQ,
}nsm_qos_mode_e;

typedef enum nsm_class_sched_e {
    NSM_CLASS_SCHED_WEIGHT = 0,
	NSM_CLASS_SCHED_STRICT,
} nsm_class_sched_t;


typedef enum
{
	NSM_QOS_DIR_INBOUND = 1,
	NSM_QOS_DIR_OUTBOUND,
}nsm_qos_dir_e;

typedef enum
{
	NSM_QOS_REMARK_NONE = 0,
	NSM_QOS_REMARK_8021P = 1,
	NSM_QOS_REMARK_DSCP,
	NSM_QOS_REMARK_IP_PRI,
}nsm_qos_remark_e;

typedef enum
{
	NSM_QOS_ACTION_NONE = 0,
	NSM_QOS_PERIMIT = 1,
	NSM_QOS_DENY,
}nsm_qos_action_e;


typedef enum
{
	NSM_QOS_LIMIT_NONE = 0,
	NSM_QOS_LIMIT_BROADCAST = 1,
	NSM_QOS_LIMIT_UNICAST,
	NSM_QOS_LIMIT_MULTICAST,

}nsm_qos_limit_e;

typedef struct nsm_qos_limit_s
{
	ospl_uint32			qos_cir;
	ospl_uint32			qos_pir;
	ospl_uint32			qos_bir;
}nsm_qos_limit_t;


typedef struct nsm_qos_storm_s
{
	enum {STORM_RATE, STORM_PER} qos_storm_type;
	ospl_uint32			qos_unicast;
	ospl_uint32			qos_multicast;
	ospl_uint32			qos_broadcast;
}nsm_qos_storm_t;

typedef enum
{
	QOS_MAP_NONE = 0,
	QOS_MAP_PORT,
	QOS_MAP_COS,
	QOS_MAP_IP_PRE,
	QOS_MAP_DSCP,
	QOS_MAP_EXP,
} nsm_qos_map_e;

typedef union
{
	ospl_uint32			port;	//port
	ospl_uint32			cos;	//802.1q
	ospl_uint32			ip_pre;
	ospl_uint32			dscp;
	ospl_uint32			exp;
} nsm_qos_map_t;


typedef struct nsm_qos_s
{
	ifindex_t			ifindex;
	// input
	ospl_bool				qos_storm_enable;
	nsm_qos_storm_t		qos_storm;			//storm control
	nsm_qos_trust_e 	qos_trust;

	ospl_bool				qos_class_enable;
	nsm_qos_class_e		qos_class[NSM_QOS_QUEUE_MAX]; //queue map to class

	ospl_bool				qos_priority_enable;
	nsm_qos_priority_e	qos_queue[NSM_QOS_QUEUE_MAX]; //priority map to queue

	//USER priority map to LOCAL priority
	ospl_bool				qos_map_enable;
	nsm_qos_map_e		qos_map_type;
	//nsm_qos_priority_e	qos_map_priority[NSM_QOS_PRI_MAX];	//pri map to priority
	nsm_qos_map_t		qos_map[NSM_QOS_PRI_MAX];

	nsm_qos_limit_t		qos_input_limit;	//rate limit control
	nsm_qos_limit_t		qos_output_limit;
	//output
	nsm_qos_map_t		qos_output_queue;

	nsm_qos_dir_e		qos_dir;

	nsm_qos_mode_e		qos_mode;

	nsm_qos_remark_e	qos_remark;
	//nsm_qos_color_e		qos_color;
	nsm_qos_action_e	qos_action;

}nsm_qos_t;

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
extern int nsm_dscp_to_cos(ospl_uint32 dscp);
extern int nsm_cos_to_dscp(ospl_uint32 cos);

extern int nsm_qos_storm_enable_set_api(struct interface *ifp);
extern ospl_bool nsm_qos_storm_enable_get_api(struct interface *ifp);

extern int nsm_qos_storm_set_api(struct interface *ifp, ospl_uint32 qos_unicast,
		ospl_uint32 qos_multicast, ospl_uint32 qos_broadcast);
extern int nsm_qos_storm_get_api(struct interface *ifp, ospl_uint32 *qos_unicast,
		ospl_uint32 *qos_multicast, ospl_uint32 *qos_broadcast);
extern int nsm_qos_rate_set_api(struct interface *ifp, nsm_qos_dir_e qos_dir,
		nsm_qos_limit_t *rate);
extern int nsm_qos_rate_get_api(struct interface *ifp, nsm_qos_dir_e qos_dir,
		nsm_qos_limit_t *rate);
extern int nsm_qos_trust_set_api(struct interface *ifp, nsm_qos_trust_e trust);
extern int nsm_qos_trust_get_api(struct interface *ifp, nsm_qos_trust_e *trust);
extern int nsm_qos_class_set_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_class_e class);
extern int nsm_qos_class_get_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_class_e *class);
extern int nsm_qos_priority_map_queue_set_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_priority_e pri);
extern int nsm_qos_priority_map_queue_get_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_priority_e *pri);
extern int nsm_qos_map_type_set_api(struct interface *ifp, nsm_qos_map_e type);
extern int nsm_qos_map_type_get_api(struct interface *ifp, nsm_qos_map_e *type);
extern int nsm_qos_user_pri_map_priority_set_api(struct interface *ifp, nsm_qos_priority_e priority, nsm_qos_map_t map);
extern int nsm_qos_user_pri_map_priority_get_api(struct interface *ifp, nsm_qos_priority_e priority, nsm_qos_map_t *map);



extern int nsm_qos_init();
extern int nsm_qos_exit();

 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_NSM_QOS_H_ */
