/*
 * nsm_qos.c
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "if.h"

#include "prefix.h"
#include "vty.h"
#include "zmemory.h"
#include "template.h"
#include "algorithm.h"
#include "nsm_qos.h"
#include "nsm_interface.h"
#include "nsm_qos_acl.h"
#include "hal_include.h"

static Global_Qos_t mGlobalQos;

static int nsm_qos_interface_default(nsm_qos_t *intf);

#if NSM_QOS_PORT_QUEUE_NUM == NSM_QOS_PORT_QUEUE_NUM_4
#ifdef NSM_QOS_QUEUE_MAP_CLASS
static nsm_qos_class_e _class_queue_map_tbl[NSM_QOS_QUEUE_MAX] =
{
		NSM_QOS_CLASS_NONE,
		NSM_QOS_CLASS_0,
		NSM_QOS_CLASS_1,
		NSM_QOS_CLASS_2,
		NSM_QOS_CLASS_3,
		NSM_QOS_CLASS_MAX
};
#endif

static nsm_qos_queue_e _qos_cosipexp_map_tbl[NSM_QOS_PRI_MAX] =
	{
		NSM_QOS_QUEUE_NONE,
		NSM_QOS_QUEUE_0,
		NSM_QOS_QUEUE_0,
		NSM_QOS_QUEUE_1,
		NSM_QOS_QUEUE_1,
		NSM_QOS_QUEUE_2,
		NSM_QOS_QUEUE_2,
		NSM_QOS_QUEUE_3,
		NSM_QOS_QUEUE_3,
		NSM_QOS_QUEUE_NONE
	};
#ifdef NSM_QOS_QUEUE_MAP_PRIORITY
static nsm_qos_priority_e _qos_cosipexp_mapqueue_tbl[NSM_QOS_QUEUE_MAX] =
	{
		NSM_QOS_PRI_NONE,
		NSM_QOS_PRI_0,
		NSM_QOS_PRI_1,
		NSM_QOS_PRI_2,
		NSM_QOS_PRI_3,
		NSM_QOS_PRI_MAX
	};
#endif	
#elif NSM_QOS_PORT_QUEUE_NUM == NSM_QOS_PORT_QUEUE_NUM_8
#ifdef NSM_QOS_QUEUE_MAP_CLASS
static nsm_qos_class_e _class_queue_map_tbl[NSM_QOS_QUEUE_MAX] =
	{
		NSM_QOS_CLASS_NONE,
		NSM_QOS_CLASS_0,
		NSM_QOS_CLASS_0,
		NSM_QOS_CLASS_1,
		NSM_QOS_CLASS_1,
		NSM_QOS_CLASS_2,
		NSM_QOS_CLASS_2,
		NSM_QOS_CLASS_3,
		NSM_QOS_CLASS_3,
		NSM_QOS_CLASS_MAX
	};
#endif
static nsm_qos_priority_e _qos_cosipexp_mapqueue_tbl[NSM_QOS_QUEUE_MAX] =
	{
		NSM_QOS_PRI_NONE,
		NSM_QOS_PRI_0,
		NSM_QOS_PRI_1,
		NSM_QOS_PRI_2,
		NSM_QOS_PRI_3,
		NSM_QOS_PRI_4,
		NSM_QOS_PRI_5,
		NSM_QOS_PRI_6,
		NSM_QOS_PRI_7,
		NSM_QOS_PRI_MAX
	};
#endif

int nsm_qos_global_enable(zpl_bool enable)
{
	if(hal_qos_enable(enable) == OK)
	{
		mGlobalQos.qos_enable = enable;
		return OK;
	}
	return ERROR;
}

zpl_bool nsm_qos_global_get(void)
{
	return mGlobalQos.qos_enable;
}

int nsm_qos_shaping_global_enable(zpl_bool enable)
{
	if(hal_qos_ipg_enable(enable) == OK)
	{
		mGlobalQos.qos_shaping = enable;
		return OK;
	}
	return ERROR;	
}

zpl_bool nsm_qos_shaping_global_get(void)
{
	return mGlobalQos.qos_shaping;
}

int nsm_dscp_to_cos(zpl_uint32 dscp)
{
	return (dscp >> 3);
	//	内部DSCP值	0~7		8~15	16~23	24~31	32~39	40~47	48~55	56~63
	//	COS值		0		1		2		3		4		5		6		7
}

int nsm_cos_to_dscp(zpl_uint32 cos)
{
	if (cos == 0)
		return 0;
	return (cos * 8);
}

static nsm_qos_t *_nsm_qos_get(struct interface *ifp)
{
	if (ifp)
	{
		return (nsm_qos_t *)nsm_intf_module_data(ifp, NSM_INTF_QOS);
	}
	return NULL;
}


/*
 * port rate limit
 */
int nsm_qos_rate_set_api(struct interface *ifp, nsm_qos_dir_e qos_dir,
						 nsm_qos_limit_t *rate)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (qos_dir == NSM_QOS_DIR_INBOUND && rate)
		{
			if(hal_qos_ingress_rate_limit(ifp->ifindex, rate->qos_cir, rate->qos_cbs) == OK)
			{
				os_memcpy(&qos->qos_input_limit, rate, sizeof(nsm_qos_limit_t));
				IF_NSM_QOS_DATA_UNLOCK(qos);
				return OK;
			}
		}
		else if (qos_dir == NSM_QOS_DIR_OUTBOUND && rate)
		{
			if(hal_qos_egress_rate_limit(ifp->ifindex, rate->qos_cir, rate->qos_cbs) == OK)
			{
				os_memcpy(&qos->qos_output_limit, rate, sizeof(nsm_qos_limit_t));
				IF_NSM_QOS_DATA_UNLOCK(qos);
				return OK;
			}
		}
		IF_NSM_QOS_DATA_UNLOCK(qos);
	}
	return ERROR;
}

int nsm_qos_rate_get_api(struct interface *ifp, nsm_qos_dir_e qos_dir,
						 nsm_qos_limit_t *rate)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (qos_dir == NSM_QOS_DIR_INBOUND && rate)
		{
			os_memcpy(rate, &qos->qos_input_limit, sizeof(nsm_qos_limit_t));
			IF_NSM_QOS_DATA_UNLOCK(qos);
			return OK;
		}
		else if (qos_dir == NSM_QOS_DIR_OUTBOUND && rate)
		{
			os_memcpy(rate, &qos->qos_output_limit, sizeof(nsm_qos_limit_t));
			IF_NSM_QOS_DATA_UNLOCK(qos);
			return OK;
		}
		IF_NSM_QOS_DATA_UNLOCK(qos);
	}
	return ERROR;
}

/*
 * port trust
 */
int nsm_qos_trust_set_api(struct interface *ifp, nsm_qos_trust_e trust)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		qos->qos_trust = trust;
		IF_NSM_QOS_DATA_UNLOCK(qos);
		return OK;
	}
	return ERROR;
}

int nsm_qos_trust_get_api(struct interface *ifp, nsm_qos_trust_e *trust)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (trust)
			*trust = qos->qos_trust;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

int nsm_qos_queue_sched_mode_get_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_queue_sched_t *mode, int *sched_weight)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (mode)
			*mode = qos->qos_queue_sched[queue];
		if(sched_weight)
			*sched_weight = qos->qos_queue_sched_weight[queue];
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

int nsm_qos_queue_sched_mode_set_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_queue_sched_t mode, int sched_weight)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		qos->qos_queue_sched[queue] = mode;
		if(mode != NSM_QOS_MODE_PQ && mode != NSM_QOS_MODE_SP)
			qos->qos_queue_sched_weight[queue] = sched_weight;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

/*
 * queue map to class
 */
#ifdef NSM_QOS_QUEUE_MAP_CLASS
int nsm_qos_class_map_set_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_class_e class)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		nsm_qos_queue_e i = 0;
		qos->qos_class_enable = zpl_false;
		qos->qos_class[queue] = class;
		for (i = NSM_QOS_QUEUE_0; i < NSM_QOS_QUEUE_MAX; i++)
		{
			if (qos->qos_class[i])
				qos->qos_class_enable = zpl_true;
		}
		IF_NSM_QOS_DATA_UNLOCK(qos);
		return OK;
	}
	return ERROR;
}

int nsm_qos_class_map_get_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_class_e *class)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (class)
			*class = qos->qos_class[queue];
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

int nsm_qos_class_sched_set_api(struct interface *ifp, nsm_qos_class_e class, nsm_class_sched_t mode, int sched_weight)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		qos->qos_class_sched[class] = mode;
		if(mode == NSM_CLASS_SCHED_WRR)
			qos->qos_class_sched_weight[class] = sched_weight;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

int nsm_qos_class_sched_get_api(struct interface *ifp, nsm_qos_class_e class, nsm_class_sched_t *mode, int *sched_weight)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (mode)
			*mode = qos->qos_class_sched[class];
		if(sched_weight)
			*sched_weight = qos->qos_class_sched_weight[class];
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}
#endif

/*
 * flow shaping
 */
int nsm_qos_shaping_set_api(struct interface *ifp, zpl_bool enable)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		qos->qos_shaping = enable;
		IF_NSM_QOS_DATA_UNLOCK(qos);
		return OK;
	}
	return ERROR;
}

int nsm_qos_shaping_get_api(struct interface *ifp, zpl_bool *enable)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (enable)
			*enable = qos->qos_shaping;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

/* 端口到队列的映射 */
int nsm_qos_port_map_queue_set_api(struct interface *ifp, nsm_qos_queue_e queue)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (queue == NSM_QOS_QUEUE_NONE)
			qos->qos_port_input_queue = qos->qos_port_input_queue_default;
		else
			qos->qos_port_input_queue = queue;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

int nsm_qos_port_map_queue_get_api(struct interface *ifp, nsm_qos_queue_e *queue)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (queue)
			*queue = qos->qos_port_input_queue;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

#ifdef NSM_QOS_USERPRI_MAP_QUEUE
/* 报文优先级到队列的映射 */
static int _nsm_qos_user_priority_map_queue_get_api(struct interface *ifp, zpl_bool defaultmap, nsm_qos_map_e type,
													zpl_uint8 pri, nsm_qos_queue_e *queue)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		switch (type)
		{
		case QOS_MAP_PORT:
			// if (queue)
			//	*queue = qos->qos_input_queue[queue].port;
			break;
		case QOS_MAP_COS:
			if (!defaultmap)
			{
				if (queue)

					*queue = qos->cos_map_queue[pri];
			}
			else
			{
				if (queue)
					*queue = qos->cos_map_queue_default[pri];
			}
			break;
		case QOS_MAP_IP_PRE:
			if (!defaultmap)
			{
				if (queue)
					*queue = qos->ip_prec_map_queue[pri];
			}
			else
			{
				if (queue)
					*queue = qos->ip_prec_map_queue_default[pri];
			}
			break;
		case QOS_MAP_DSCP:
			if (!defaultmap)
			{
				if (queue)
					*queue = qos->dscp_map_queue[pri];
			}
			else
			{
				if (queue)
					*queue = qos->dscp_map_queue_default[pri];
			}
			break;
		case QOS_MAP_EXP:
			if (!defaultmap)
			{
				if (queue)
					*queue = qos->mplsexp_map_queue[pri];
			}
			else
			{
				if (queue)
					*queue = qos->mplsexp_map_queue_default[pri];
			}
			break;
		default:
			IF_NSM_QOS_DATA_UNLOCK(qos);
			return ERROR;
		}
		IF_NSM_QOS_DATA_UNLOCK(qos);
		return OK;
	}
	return ERROR;
}

static int _nsm_qos_user_priority_map_queue_set_api(struct interface *ifp, zpl_bool defaultmap, nsm_qos_map_e type,
													zpl_uint8 pri, nsm_qos_queue_e queue)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		switch (type)
		{
		case QOS_MAP_PORT:
			// if(dir == NSM_QOS_DIR_OUTBOUND)
			// qos->qos_input_queue[queue].port = map.port;
			break;
		case QOS_MAP_COS:
			if (!defaultmap)
			{
				if (queue == NSM_QOS_QUEUE_NONE)
					qos->cos_map_queue[pri] = qos->cos_map_queue_default[pri];
				else
					qos->cos_map_queue[pri] = queue;
			}
			else
				qos->cos_map_queue_default[pri] = queue;
			break;
		case QOS_MAP_IP_PRE:
			if (!defaultmap)
			{
				if (queue == NSM_QOS_QUEUE_NONE)
					qos->ip_prec_map_queue[pri] = qos->ip_prec_map_queue_default[pri];
				else
					qos->ip_prec_map_queue[pri] = queue;
			}
			else
				qos->ip_prec_map_queue_default[pri] = queue;
			break;
		case QOS_MAP_DSCP:
			if (!defaultmap)
			{
				if (queue == NSM_QOS_QUEUE_NONE)
					qos->dscp_map_queue[pri] = qos->dscp_map_queue_default[pri];
				else
					qos->dscp_map_queue[pri] = queue;
			}
			else
				qos->dscp_map_queue_default[pri] = queue;
			break;
		case QOS_MAP_EXP:
			if (!defaultmap)
			{
				if (queue == NSM_QOS_QUEUE_NONE)
					qos->mplsexp_map_queue[pri] = qos->mplsexp_map_queue_default[pri];
				else
					qos->mplsexp_map_queue[pri] = queue;
			}
			else
				qos->mplsexp_map_queue_default[pri] = queue;
			break;
		default:
			IF_NSM_QOS_DATA_UNLOCK(qos);
			return ERROR;
		}
		IF_NSM_QOS_DATA_UNLOCK(qos);
		return OK;
	}
	return ERROR;
}

int nsm_qos_cos_map_queue_set_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e queue)
{
	return _nsm_qos_user_priority_map_queue_set_api(ifp, defaultmap, QOS_MAP_COS, pri, queue);
}
int nsm_qos_cos_map_queue_get_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e *queue)
{
	return _nsm_qos_user_priority_map_queue_get_api(ifp, defaultmap, QOS_MAP_COS, pri, queue);
}

int nsm_qos_exp_map_queue_set_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e queue)
{
	return _nsm_qos_user_priority_map_queue_set_api(ifp, defaultmap, QOS_MAP_EXP, pri, queue);
}
int nsm_qos_exp_map_queue_get_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e *queue)
{
	return _nsm_qos_user_priority_map_queue_get_api(ifp, defaultmap, QOS_MAP_EXP, pri, queue);
}

int nsm_qos_ippre_map_queue_set_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e queue)
{
	return _nsm_qos_user_priority_map_queue_set_api(ifp, defaultmap, QOS_MAP_IP_PRE, pri, queue);
}
int nsm_qos_ippre_map_queue_get_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e *queue)
{
	return _nsm_qos_user_priority_map_queue_get_api(ifp, defaultmap, QOS_MAP_IP_PRE, pri, queue);
}

int nsm_qos_dscp_map_queue_set_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e queue)
{
	return _nsm_qos_user_priority_map_queue_set_api(ifp, defaultmap, QOS_MAP_DSCP, pri, queue);
}
int nsm_qos_dscp_map_queue_get_api(struct interface *ifp, zpl_bool defaultmap, zpl_uint8 pri, nsm_qos_queue_e *queue)
{
	return _nsm_qos_user_priority_map_queue_get_api(ifp, defaultmap, QOS_MAP_DSCP, pri, queue);
}
#endif

#ifdef NSM_QOS_PRIORITY_MAP_QUEUE
/*
 * priority map to queue
 */
int nsm_qos_priority_map_queue_set_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_queue_e queue)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (queue == NSM_QOS_QUEUE_NONE)
			qos->qos_priority_map_queue[pri] = qos->qos_priority_map_queue_default[pri];
		else
			qos->qos_priority_map_queue[pri] = queue;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

int nsm_qos_priority_map_queue_get_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_queue_e *queue)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (queue)
			*queue = qos->qos_priority_map_queue[pri];
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}
#endif

#ifdef NSM_QOS_QUEUE_MAP_PRIORITY
int nsm_qos_queue_map_priority_set_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_priority_e priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority == NSM_QOS_PRI_NONE)
			qos->qos_queue_map_priority[queue] = qos->qos_queue_map_priority_default[queue];
		else
			qos->qos_queue_map_priority[queue] = priority;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

int nsm_qos_queue_map_priority_get_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_priority_e *priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority)
			*priority = qos->qos_queue_map_priority[queue];
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}
#endif

#ifdef NSM_QOS_USERPRI_MAP_PRIORITY
/*
 * user priority map to inside priority
 */
int nsm_qos_cos_map_priority_set_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority == NSM_QOS_PRI_NONE)
			qos->cos_map_priority[pri] = qos->cos_map_priority_default[pri];
		else
			qos->cos_map_priority[pri] = priority;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}
int nsm_qos_cos_map_priority_get_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e *priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority)
			*priority = qos->cos_map_priority[pri];
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}
int nsm_qos_ipprec_map_priority_set_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority == NSM_QOS_PRI_NONE)
			qos->ip_prec_map_priority[pri] = qos->ip_prec_map_priority_default[pri];
		else
			qos->ip_prec_map_priority[pri] = priority;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}
int nsm_qos_ipprec_map_priority_get_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e *priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority)
			*priority = qos->ip_prec_map_priority[pri];
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}
int nsm_qos_mplsexp_map_priority_set_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority == NSM_QOS_PRI_NONE)
			qos->mplsexp_map_priority[pri] = qos->mplsexp_map_priority_default[pri];
		else
			qos->mplsexp_map_priority[pri] = priority;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}
int nsm_qos_mplsexp_map_priority_get_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e *priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority)
			*priority = qos->mplsexp_map_priority[pri];
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}
int nsm_qos_dscp_map_priority_set_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority == NSM_QOS_PRI_NONE)
			qos->dscp_map_priority[pri] = qos->dscp_map_priority_default[pri];
		else
			qos->dscp_map_priority[pri] = priority;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}
int nsm_qos_dscp_map_priority_get_api(struct interface *ifp, nsm_qos_priority_e pri, nsm_qos_priority_e *priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority)
			*priority = qos->dscp_map_priority[pri];
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}
#endif


int nsm_qos_cos_replace_set_api(struct interface *ifp, nsm_qos_priority_e priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority == NSM_QOS_PRI_NONE)
			qos->qos_cos_replace = NSM_QOS_PRI_NONE;
		else
			qos->qos_cos_replace = priority;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

int nsm_qos_cos_replace_get_api(struct interface *ifp, nsm_qos_priority_e *priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority)
			*priority = qos->qos_cos_replace;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}


int nsm_qos_dscp_replace_set_api(struct interface *ifp, nsm_qos_priority_e priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority == NSM_QOS_PRI_NONE)
			qos->qos_dscp_replace = NSM_QOS_PRI_NONE;
		else
			qos->qos_dscp_replace = priority;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

int nsm_qos_dscp_replace_get_api(struct interface *ifp, nsm_qos_priority_e *priority)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (priority)
			*priority = qos->qos_dscp_replace;
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

int nsm_qos_service_policy_set_api(struct interface *ifp, int input, zpl_char * service_policy)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if(qos_service_policy_lookup(service_policy) == NULL)
		{
			IF_NSM_QOS_DATA_UNLOCK(qos);
			return ERROR;
		}
		if(input)
		{
			if (service_policy == NULL && qos->service_policy_input.service_policy)
			{
				qos_service_policy_reference(qos->service_policy_input.service_policy, zpl_false);
				free(qos->service_policy_input.service_policy);
				qos->service_policy_input.service_policy = NULL;
			}
			else if (service_policy)
			{
				if(qos->service_policy_input.service_policy)
					free(qos->service_policy_input.service_policy);
				qos_service_policy_reference(qos->service_policy_input.service_policy, zpl_false);
				qos->service_policy_input.service_policy = strdup(service_policy);
			}
		}
		else
		{
			if (service_policy == NULL && qos->service_policy_output.service_policy)
			{
				qos_service_policy_reference(qos->service_policy_output.service_policy, zpl_false);
				free(qos->service_policy_output.service_policy);
				qos->service_policy_output.service_policy = NULL;
			}
			else if (service_policy)
			{
				if(qos->service_policy_output.service_policy)
					free(qos->service_policy_output.service_policy);
				qos_service_policy_reference(qos->service_policy_output.service_policy, zpl_false);
				qos->service_policy_output.service_policy = strdup(service_policy);
			}
		}
		IF_NSM_QOS_DATA_UNLOCK(qos);
		return OK;
	}
	return ERROR;
}

int nsm_qos_service_policy_get_api(struct interface *ifp, int input, zpl_char *service_policy)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos)
	{
		IF_NSM_QOS_DATA_LOCK(qos);
		if (input && service_policy && qos->service_policy_input.service_policy)
			strcpy(service_policy, qos->service_policy_input.service_policy);
		else if (!input && service_policy && qos->service_policy_output.service_policy)
			strcpy(service_policy, qos->service_policy_output.service_policy);
		IF_NSM_QOS_DATA_UNLOCK(qos);	
		return OK;
	}
	return ERROR;
}

static int nsm_qos_interface_create_api(struct interface *ifp)
{
	nsm_qos_t *qos = NULL;
	if (if_is_loop(ifp))
		return OK;
	qos = nsm_intf_module_data(ifp, NSM_INTF_QOS);
	if(qos == NULL)
	{
		 qos = XMALLOC(MTYPE_QOS, sizeof(nsm_qos_t));
		os_memset(qos, 0, sizeof(nsm_qos_t));
		if (qos->mutex == NULL)
			qos->mutex = os_mutex_name_init(os_name_format("%s-qos_mutex", ifp->name));
		IF_NSM_QOS_DATA_LOCK(qos);	
		qos->ifindex = ifp->ifindex;
		nsm_intf_module_data_set(ifp, NSM_INTF_QOS, qos);
		nsm_qos_interface_default(qos);
		IF_NSM_QOS_DATA_UNLOCK(qos);
	}
	return OK;
}

static int nsm_qos_interface_del_api(struct interface *ifp)
{
	nsm_qos_t *qos = NULL;
	if (if_is_loop(ifp))
		return OK;
	qos = nsm_intf_module_data(ifp, NSM_INTF_QOS);
	if (qos)
	{
		if(qos->mutex)
		{
			os_mutex_exit(qos->mutex);
			qos->mutex = NULL;
		}
		XFREE(MTYPE_QOS, qos);
	}
	qos = NULL;
	nsm_intf_module_data_set(ifp, NSM_INTF_QOS, NULL);
	return OK;
}

#ifdef ZPL_SHELL_MODULE

int nsm_qos_interface_write_config(struct vty *vty, struct interface *ifp)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos && !if_is_loop(ifp) && nsm_qos_global_get())
	{
		int32_t i = 0;//, j = 0;//, n = 0;
		//zpl_uint32 tmpb[NSM_QOS_DSCP_PRI_MAX];
		zpl_char tmpstr[512];
		IF_NSM_QOS_DATA_LOCK(qos);
		if (qos->qos_input_limit.qos_cir)
		{
			memset(tmpstr, 0, sizeof(tmpstr));
			if(qos->qos_input_limit.qos_pir)
			{
				strcat(tmpstr, "pir ");
				strcat(tmpstr, itoa(qos->qos_input_limit.qos_pir, 10));
				strcat(tmpstr, " ");
			}
			if(qos->qos_input_limit.qos_cbs)
			{
				strcat(tmpstr, "cbs ");
				strcat(tmpstr, itoa(qos->qos_input_limit.qos_cbs, 10));
				strcat(tmpstr, " ");
			}
			vty_out(vty, " qos policer inbound cir %d %s%s", qos->qos_input_limit.qos_cir, strlen(tmpstr)?tmpstr:" ", VTY_NEWLINE);
		}
		if (qos->qos_output_limit.qos_cir)
		{
			memset(tmpstr, 0, sizeof(tmpstr));
			if(qos->qos_output_limit.qos_pir)
			{
				strcat(tmpstr, "pir ");
				strcat(tmpstr, itoa(qos->qos_output_limit.qos_pir, 10));
				strcat(tmpstr, " ");
			}
			if(qos->qos_output_limit.qos_cbs)
			{
				strcat(tmpstr, "cbs ");
				strcat(tmpstr, itoa(qos->qos_output_limit.qos_cbs, 10));
				strcat(tmpstr, " ");
			}
			vty_out(vty, " qos policer outbound cir %d %s%s", qos->qos_output_limit.qos_cir, strlen(tmpstr)?tmpstr:" ", VTY_NEWLINE);
		}
			if(qos->service_policy_output.service_policy)
			{
				vty_out(vty, " qos service-policy outbound %s%s", qos->service_policy_output.service_policy, VTY_NEWLINE);
			}
			if(qos->service_policy_input.service_policy)
			{
				vty_out(vty, " qos service-policy outbound %s%s", qos->service_policy_input.service_policy, VTY_NEWLINE);
			}
		if (qos->qos_trust != NSM_QOS_TRUST_NONE)
		{
			switch (qos->qos_trust)
			{
			case NSM_QOS_TRUST_PORT:
				vty_out(vty, " qos trust port%s", VTY_NEWLINE);
				break;
			case NSM_QOS_TRUST_EXP:
				vty_out(vty, " qos trust mplsexp%s", VTY_NEWLINE);
				break;
			case NSM_QOS_TRUST_COS:
				vty_out(vty, " qos trust cos%s", VTY_NEWLINE);
				break;
			case NSM_QOS_TRUST_DSCP:
				vty_out(vty, " qos trust dscp%s", VTY_NEWLINE);
				break;
			case NSM_QOS_TRUST_IP_PRE:
				vty_out(vty, " qos trust ip-prec%s", VTY_NEWLINE);
				break;
			}
		}
		for (i = NSM_QOS_QUEUE_0; i < NSM_QOS_QUEUE_MAX; i++)
		{
			if (qos->qos_queue_sched[i] == qos->qos_queue_sched_default[i])
			{
				if (qos->qos_queue_sched[i] == NSM_QOS_MODE_SP)
				{
					vty_out(vty, " qos queue %d sp%s", i - NSM_QOS_QUEUE_0, VTY_NEWLINE);
				}
				if (qos->qos_queue_sched[i] == NSM_QOS_MODE_WRR)
				{
					vty_out(vty, " qos queue %d wrr-weight%s", i - NSM_QOS_QUEUE_0, VTY_NEWLINE);
				}
			}
		}
		if (qos->qos_port_input_queue != qos->qos_port_input_queue_default)
		{
			vty_out(vty, " qos queue %d %s", qos->qos_port_input_queue - NSM_QOS_QUEUE_0, VTY_NEWLINE);
		}
		else
		{
#ifdef NSM_QOS_USERPRI_MAP_QUEUE
			for (i = NSM_QOS_PRI_0; i < NSM_QOS_PRI_MAX; i++)
			{
				if (qos->cos_map_queue[i] != qos->cos_map_queue_default[i])
					vty_out(vty, " qos cos-priority %d queue %d %s", i,
							qos->cos_map_queue[i] - NSM_QOS_QUEUE_0, VTY_NEWLINE);
			}
			for (i = NSM_QOS_PRI_0; i < NSM_QOS_PRI_MAX; i++)
			{
				if (qos->ip_prec_map_queue[i] != qos->ip_prec_map_queue_default[i])
					vty_out(vty, " qos ip-prec-priority %d queue %d %s", i,
							qos->ip_prec_map_queue[i] - NSM_QOS_QUEUE_0, VTY_NEWLINE);
			}
			for (i = NSM_QOS_PRI_0; i < NSM_QOS_PRI_MAX; i++)
			{
				if (qos->mplsexp_map_queue[i] != qos->mplsexp_map_queue_default[i])
					vty_out(vty, " qos mplsexp-priority %d queue %d %s", i,
							qos->mplsexp_map_queue[i] - NSM_QOS_QUEUE_0, VTY_NEWLINE);
			}
			for (i = 0; i < NSM_QOS_DSCP_PRI_MAX; i++)
			{
				if (qos->dscp_map_queue[i] != qos->dscp_map_queue_default[i])
					vty_out(vty, " qos dscp-priority %d queue %d %s", i,
							qos->dscp_map_queue[i] - NSM_QOS_QUEUE_0, VTY_NEWLINE);
			}
#endif
		}
#ifdef NSM_QOS_QUEUE_MAP_CLASS
		// queue map to class
		if (qos->qos_class_enable)
		{
			for (i = NSM_QOS_QUEUE_0; i < NSM_QOS_QUEUE_MAX; i++)
			{
				if (qos->qos_class[i] != qos->qos_class_default[i])
					vty_out(vty, " qos queue %d class %d %s", i - NSM_QOS_QUEUE_0,
							qos->qos_class[i] - NSM_QOS_CLASS_0, VTY_NEWLINE);
			}
			for (i = NSM_QOS_CLASS_0; i < NSM_QOS_CLASS_MAX; i++)
			{
				if (qos->qos_class_sched[i] != qos->qos_class_sched_default[i])
				{
					if((qos->qos_class_sched[i] == NSM_CLASS_SCHED_WRR))
						vty_out(vty, " qos class %d wrr %d %s", i - NSM_QOS_CLASS_0,
							qos->qos_class_sched_weight[i], VTY_NEWLINE);
					else
						vty_out(vty, " qos class %d sp %s", i - NSM_QOS_CLASS_0, VTY_NEWLINE);
				}
			}
		}
#endif
#ifdef NSM_QOS_PRIORITY_MAP_QUEUE
		// priority map to queue
		// if (qos->qos_priority_enable)
		{
			for (i = NSM_QOS_PRI_0; i <= NSM_QOS_PRI_7; i++)
			{
				if (qos->qos_priority_map_queue[i] != qos->qos_priority_map_queue_default[i])
					vty_out(vty, " qos priority %d queue %d %s",
							i - NSM_QOS_PRI_0, qos->qos_priority_map_queue[i] - NSM_QOS_QUEUE_0, VTY_NEWLINE);
			}
		}
#endif		
#ifdef NSM_QOS_QUEUE_MAP_PRIORITY
		// USER priority map to LOCAL priority
		// if (qos->qos_map_enable)
		{
			for (i = NSM_QOS_QUEUE_0; i < NSM_QOS_QUEUE_MAX; i++)
			{
				if (qos->qos_queue_map_priority[i] != qos->qos_queue_map_priority_default[i])
					vty_out(vty, " qos queue %d priority %d%s",
							i - NSM_QOS_QUEUE_0, qos->qos_queue_map_priority[i] - NSM_QOS_PRI_0, VTY_NEWLINE);
			}
		}
#endif
		if(qos->qos_shaping)
			vty_out(vty, " qos shaping%s", VTY_NEWLINE);
		IF_NSM_QOS_DATA_UNLOCK(qos);	
	}
	return OK;
}

int nsm_qos_interface_show(struct vty *vty, struct interface *ifp)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if (qos && !if_is_loop(ifp) && nsm_qos_global_get())
	{
		int32_t i = 0;//, j = 0, n = 0;
		//zpl_uint32 tmpb[NSM_QOS_DSCP_PRI_MAX];
		zpl_char tmpstr[512], tmpstr1[512], tmpstr2[512];
		memset(tmpstr, 0, sizeof(tmpstr));
		memset(tmpstr1, 0, sizeof(tmpstr1));
		memset(tmpstr2, 0, sizeof(tmpstr2));
		IF_NSM_QOS_DATA_LOCK(qos);
		vty_out(vty, "%sInterface  : %s %s", VTY_NEWLINE, ifp->name, VTY_NEWLINE);
		vty_out(vty, " Qos Shaping  : %s %s", qos->qos_shaping?"Enable":"Disable", VTY_NEWLINE);
		for (i = NSM_QOS_QUEUE_0; i < NSM_QOS_QUEUE_MAX; i++)
		{
			if (qos->qos_queue_sched[i] == NSM_QOS_MODE_PQ)
			{
				strcat(tmpstr, itoa(i - NSM_QOS_QUEUE_0, 10));
				strcat(tmpstr, " ");
			}
			if (qos->qos_queue_sched[i] == NSM_QOS_MODE_WFQ)
			{
				strcat(tmpstr1, itoa(i - NSM_QOS_QUEUE_0, 10));
				strcat(tmpstr1, " ");
			}
			if (qos->qos_queue_sched[i] == NSM_QOS_MODE_FQ)
			{
				strcat(tmpstr2, itoa(i - NSM_QOS_QUEUE_0, 10));
				strcat(tmpstr2, " ");
			}
		}

		if (strlen(tmpstr))
			vty_out(vty, " Strict Priority Queue      : %s %s", tmpstr, VTY_NEWLINE);
		if (strlen(tmpstr1))
			vty_out(vty, " Weighted Round Robin Queue : %s %s", tmpstr1, VTY_NEWLINE);
		if (strlen(tmpstr2))
			vty_out(vty, " Random-detect Queue        ：%s %s", tmpstr2, VTY_NEWLINE);

		memset(tmpstr, 0, sizeof(tmpstr));
		memset(tmpstr1, 0, sizeof(tmpstr1));
		memset(tmpstr2, 0, sizeof(tmpstr2));
#ifdef NSM_QOS_PRIORITY_MAP_QUEUE
		// priority map to queue
		vty_out(vty, " Priority Map Queue: ");
		// if (qos->qos_priority_enable)
		for (i = NSM_QOS_PRI_0; i <= NSM_QOS_PRI_7; i++)
		{
			vty_out(vty, "  %d(%d)", i - NSM_QOS_PRI_0, qos->qos_priority_map_queue[i] - NSM_QOS_QUEUE_0);
		}
		vty_out(vty, " %s", VTY_NEWLINE);
#endif

// USER priority map to LOCAL priority
// if (qos->qos_map_enable)
#ifdef NSM_QOS_QUEUE_MAP_PRIORITY
		vty_out(vty, " Queue Map Priority: ");
		for (i = NSM_QOS_QUEUE_0; i < NSM_QOS_QUEUE_MAX; i++)
		{
			vty_out(vty, "  %d(%d)",
					i - NSM_QOS_QUEUE_0, qos->qos_queue_map_priority[i] - NSM_QOS_PRI_0);
		}
		vty_out(vty, " %s", VTY_NEWLINE);
#endif
		vty_out(vty, " Queue Map Table :%s", VTY_NEWLINE);
		// queue map to class
		vty_out(vty, " Class Map Queue :");
#ifdef NSM_QOS_QUEUE_MAP_CLASS
		for (i = NSM_QOS_QUEUE_0; i < NSM_QOS_QUEUE_MAX; i++)
		{
			vty_out(vty, "  %d(%d)", qos->qos_class[i] - NSM_QOS_CLASS_0, i - NSM_QOS_QUEUE_0);
		}
		vty_out(vty, " %s", VTY_NEWLINE);
#endif

#ifdef NSM_QOS_USERPRI_MAP_QUEUE
		vty_out(vty, " Cos Map Queue   :");
		for (i = NSM_QOS_PRI_0; i < NSM_QOS_PRI_MAX; i++)
		{
			vty_out(vty, "  %d(%d)", i - NSM_QOS_PRI_0, qos->cos_map_queue[i] - NSM_QOS_QUEUE_0);
		}
		vty_out(vty, " %s", VTY_NEWLINE);

		vty_out(vty, " IPPrec Map Queue:");
		for (i = NSM_QOS_PRI_0; i < NSM_QOS_PRI_MAX; i++)
		{
			vty_out(vty, "  %d(%d)", i - NSM_QOS_PRI_0, qos->ip_prec_map_queue[i] - NSM_QOS_QUEUE_0);
		}
		vty_out(vty, " %s", VTY_NEWLINE);

		vty_out(vty, " Exp Map Queue   :");
		for (i = NSM_QOS_PRI_0; i < NSM_QOS_PRI_MAX; i++)
		{
			vty_out(vty, "  %d(%d)", i - NSM_QOS_PRI_0, qos->mplsexp_map_queue[i] - NSM_QOS_QUEUE_0);
		}
		vty_out(vty, " %s", VTY_NEWLINE);

		vty_out(vty, " DSCP Map Queue  :");
		for (i = 0; i < NSM_QOS_DSCP_PRI_MAX; i++)
		{
			vty_out(vty, "  %d(%d)", i, qos->dscp_map_queue[i] - NSM_QOS_QUEUE_0);
			if ((i + 1) % 8 == 0 && i != (NSM_QOS_DSCP_PRI_MAX-1))
			{
				vty_out(vty, "%s                 :", VTY_NEWLINE);
			}
		}
		vty_out(vty, " %s", VTY_NEWLINE);
#endif
		IF_NSM_QOS_DATA_UNLOCK(qos);
	}
	return OK;
}
#endif

static int nsm_qos_interface_default(nsm_qos_t *intf)
{
	int32_t i = 0;
	//队列调度
	//队列调度模式（出方向应用）
	for (i = NSM_QOS_QUEUE_0; i < NSM_QOS_QUEUE_MAX; i++)
	{
		intf->qos_queue_sched[i] = intf->qos_queue_sched_default[i] = NSM_QOS_MODE_PQ;
	}
#ifdef NSM_QOS_QUEUE_MAP_CLASS
	//队列到class的映射
	intf->qos_class_enable = zpl_true;
	// queue map to class
	for (i = NSM_QOS_QUEUE_0; i < NSM_QOS_QUEUE_MAX; i++)
	{
		intf->qos_class[i] = intf->qos_class_default[i] = _class_queue_map_tbl[i];
	}
	for (i = NSM_QOS_CLASS_0; i <= NSM_QOS_CLASS_4; i++)
	{
		intf->qos_class_sched[i] = intf->qos_class_sched_default[i] = NSM_CLASS_SCHED_PQ;
	}
#endif	
#ifdef NSM_QOS_USERPRI_MAP_QUEUE
	//各类优先级到queue的映射
	for (i = NSM_QOS_PRI_0; i <= NSM_QOS_PRI_7; i++)
	{
		intf->cos_map_queue[i] = _qos_cosipexp_map_tbl[i];
		intf->ip_prec_map_queue[i] = _qos_cosipexp_map_tbl[i];
		intf->mplsexp_map_queue[i] = _qos_cosipexp_map_tbl[i];

		intf->cos_map_queue_default[i] = _qos_cosipexp_map_tbl[i];
		intf->ip_prec_map_queue_default[i] = _qos_cosipexp_map_tbl[i];
		intf->mplsexp_map_queue_default[i] = _qos_cosipexp_map_tbl[i];
	}
#if NSM_QOS_PORT_QUEUE_NUM == NSM_QOS_PORT_QUEUE_NUM_4
	for (i = 0; i < NSM_QOS_DSCP_PRI_MAX; i++)
	{
		if (i < 16)
		{
			intf->dscp_map_queue[i] = NSM_QOS_QUEUE_0;
			intf->dscp_map_queue_default[i] = NSM_QOS_QUEUE_0;
		}
		else if (i < 32)
		{
			intf->dscp_map_queue[i] = NSM_QOS_QUEUE_1;
			intf->dscp_map_queue_default[i] = NSM_QOS_QUEUE_1;
		}
		else if (i < 48)
		{
			intf->dscp_map_queue[i] = NSM_QOS_QUEUE_2;
			intf->dscp_map_queue_default[i] = NSM_QOS_QUEUE_2;
		}
		else if (i < 64)
		{
			intf->dscp_map_queue[i] = NSM_QOS_QUEUE_3;
			intf->dscp_map_queue_default[i] = NSM_QOS_QUEUE_3;
		}
	}
#elif NSM_QOS_PORT_QUEUE_NUM == NSM_QOS_PORT_QUEUE_NUM_8
	for (i = 0; i < NSM_QOS_DSCP_PRI_MAX; i++)
	{
		if (i < 8)
		{
			intf->dscp_map_queue[i] = NSM_QOS_QUEUE_0;
			intf->dscp_map_queue_default[i] = NSM_QOS_QUEUE_0;
		}
		else if (i < 16)
		{
			intf->dscp_map_queue[i] = NSM_QOS_QUEUE_1;
			intf->dscp_map_queue_default[i] = NSM_QOS_QUEUE_1;
		}
		else if (i < 24)
		{
			intf->dscp_map_queue[i] = NSM_QOS_QUEUE_2;
			intf->dscp_map_queue_default[i] = NSM_QOS_QUEUE_2;
		}
		else if (i < 32)
		{
			intf->dscp_map_queue[i] = NSM_QOS_QUEUE_3;
			intf->dscp_map_queue_default[i] = NSM_QOS_QUEUE_3;
		}
		else if (i < 40)
		{
			intf->dscp_map_queue[i] = NSM_QOS_QUEUE_4;
			intf->dscp_map_queue_default[i] = NSM_QOS_QUEUE_4;
		}
		else if (i < 48)
		{
			intf->dscp_map_queue[i] = NSM_QOS_QUEUE_5;
			intf->dscp_map_queue_default[i] = NSM_QOS_QUEUE_5;
		}
		else if (i < 56)
		{
			intf->dscp_map_queue[i] = NSM_QOS_QUEUE_6;
			intf->dscp_map_queue_default[i] = NSM_QOS_QUEUE_6;
		}
		else if (i < 64)
		{
			intf->dscp_map_queue[i] = NSM_QOS_QUEUE_7;
			intf->dscp_map_queue_default[i] = NSM_QOS_QUEUE_7;
		}
	}
#endif /* NSM_QOS_PORT_QUEUE_NUM */
#endif /* NSM_QOS_USERPRI_MAP_QUEUE */

#ifdef NSM_QOS_USERPRI_MAP_PRIORITY
	// user priority map to priority
	for (i = NSM_QOS_PRI_0; i <= NSM_QOS_PRI_7; i++)
	{
		intf->cos_map_priority[i] = i;
		intf->cos_map_priority_default[i] = i;
		intf->ip_prec_map_priority[i] = i;
		intf->ip_prec_map_priority_default[i] = i;
		intf->mplsexp_map_priority[i] = i;
		intf->mplsexp_map_priority_default[i] = i;
		intf->dscp_map_priority[i] = (i - 1) / 8;
		intf->dscp_map_priority_default[i] = (i - 1) / 8;
	}
#endif
#ifdef NSM_QOS_PRIORITY_MAP_QUEUE
	// output 内部优先级到队列的映射
	// priority map to queue
	for (i = NSM_QOS_PRI_0; i <= NSM_QOS_PRI_7; i++)
	{
		intf->qos_priority_map_queue[i] = intf->qos_priority_map_queue_default[i] = _qos_cosipexp_map_tbl[i];
	}
#endif

#ifdef NSM_QOS_QUEUE_MAP_PRIORITY
	// input 用户优先级到内部优先级的映射（队列到内部优先级的映射）
	// queue map to priority
	for (i = NSM_QOS_QUEUE_0; i < NSM_QOS_QUEUE_MAX; i++)
	{
		intf->qos_queue_map_priority[i] = intf->qos_queue_map_priority_default[i] = _qos_cosipexp_mapqueue_tbl[i];
	}
#endif
	return OK;
}

int nsm_qos_init(void)
{
	memset(&mGlobalQos, 0, sizeof(mGlobalQos));
	nsm_interface_hook_add(NSM_INTF_QOS, nsm_qos_interface_create_api, nsm_qos_interface_del_api);
	nsm_interface_write_hook_add(NSM_INTF_QOS, nsm_qos_interface_write_config);
	qos_access_list_init();
	return OK;
}

int nsm_qos_exit(void)
{
	return OK;
}
