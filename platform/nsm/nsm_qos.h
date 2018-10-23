/*
 * nsm_qos.h
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */

#ifndef __NSM_NSM_QOS_H_
#define __NSM_NSM_QOS_H_


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

    //NSM_QOS_CLASS_4,
    //NSM_QOS_CLASS_5,
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
	u_int			qos_cir;
	u_int			qos_pir;
	u_int			qos_bir;
}nsm_qos_limit_t;


typedef struct nsm_qos_storm_s
{
	enum {STORM_RATE, STORM_PER} qos_storm_type;
	u_int			qos_unicast;
	u_int			qos_multicast;
	u_int			qos_broadcast;
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
	u_int			port;	//port
	u_int			cos;	//802.1q
	u_int			ip_pre;
	u_int			dscp;
	u_int			exp;
} nsm_qos_map_t;


typedef struct nsm_qos_s
{
	ifindex_t			ifindex;
	// input
	BOOL				qos_storm_enable;
	nsm_qos_storm_t		qos_storm;			//storm control
	nsm_qos_trust_e 	qos_trust;

	BOOL				qos_class_enable;
	nsm_qos_class_e		qos_class[NSM_QOS_QUEUE_MAX]; //queue map to class

	BOOL				qos_priority_enable;
	nsm_qos_priority_e	qos_queue[NSM_QOS_QUEUE_MAX]; //priority map to queue

	//USER priority map to LOCAL priority
	BOOL				qos_map_enable;
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
extern int nsm_dscp_to_cos(int dscp);
extern int nsm_cos_to_dscp(int cos);


extern int nsm_qos_init();
extern int nsm_qos_exit();


#endif /* __NSM_NSM_QOS_H_ */
