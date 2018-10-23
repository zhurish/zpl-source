/*
 * nsm_qos.c
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "interface.h"
#include <log.h>

#include "os_list.h"

#include "nsm_qos.h"


int nsm_dscp_to_cos(int dscp)
{
	return (dscp >> 3);
//	内部DSCP值	0~7		8~15	16~23	24~31	32~39	40~47	48~55	56~63
//	COS值		0		1		2		3		4		5		6		7
}

int nsm_cos_to_dscp(int cos)
{
	if(cos == 0)
		return 0;
	return (cos * 8);
}

static nsm_qos_t * _nsm_qos_get(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(nsm)
		return (nsm_qos_t *)(nsm->nsm_client[NSM_QOS]);
	return NULL;
}

int nsm_qos_storm_enable_set_api(struct interface *ifp)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		qos->qos_storm_enable = TRUE;
		return OK;
	}
	return ERROR;
}

/*
 * port storm control
 */
BOOL nsm_qos_storm_enable_get_api(struct interface *ifp)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		return qos->qos_storm_enable;
	}
	return FALSE;
}

int nsm_qos_storm_set_api(struct interface *ifp, u_int qos_unicast,
		u_int qos_multicast, u_int qos_broadcast)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		qos->qos_storm.qos_unicast = qos_unicast;
		qos->qos_storm.qos_multicast = qos_multicast;
		qos->qos_storm.qos_broadcast = qos_broadcast;
		return OK;
	}
	return ERROR;
}

int nsm_qos_storm_get_api(struct interface *ifp, u_int *qos_unicast,
		u_int *qos_multicast, u_int *qos_broadcast)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		if(qos_unicast)
			*qos_unicast = qos->qos_storm.qos_unicast;
		if(qos_multicast)
			*qos_multicast = qos->qos_storm.qos_multicast;
		if(qos_broadcast)
			*qos_broadcast = qos->qos_storm.qos_broadcast;
		return OK;
	}
	return ERROR;
}
/*
 * port rate limit
 */
int nsm_qos_rate_set_api(struct interface *ifp, nsm_qos_dir_e qos_dir,
		nsm_qos_limit_t *rate)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		if(qos_dir == NSM_QOS_DIR_INBOUND && rate)
		{
			os_memcpy(&qos->qos_input_limit, rate, sizeof(nsm_qos_limit_t));
			return OK;
		}
		else if(qos_dir == NSM_QOS_DIR_OUTBOUND && rate)
		{
			os_memcpy(&qos->qos_output_limit, rate, sizeof(nsm_qos_limit_t));
			return OK;
		}
	}
	return ERROR;
}

int nsm_qos_rate_get_api(struct interface *ifp, nsm_qos_dir_e qos_dir,
		nsm_qos_limit_t *rate)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		if(qos_dir == NSM_QOS_DIR_INBOUND && rate)
		{
			os_memcpy(rate, &qos->qos_input_limit, sizeof(nsm_qos_limit_t));
			return OK;
		}
		else if(qos_dir == NSM_QOS_DIR_OUTBOUND && rate)
		{
			os_memcpy(rate, &qos->qos_output_limit, sizeof(nsm_qos_limit_t));
			return OK;
		}
	}
	return ERROR;
}

/*
 * port trust
 */
int nsm_qos_trust_set_api(struct interface *ifp, nsm_qos_trust_e trust)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		qos->qos_trust = trust;
		return OK;
	}
	return ERROR;
}

int nsm_qos_trust_get_api(struct interface *ifp, nsm_qos_trust_e *trust)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		if(trust)
			*trust = qos->qos_trust;
		return OK;
	}
	return ERROR;
}

/*
 * queue map to class
 */
int nsm_qos_class_set_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_class_e class)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		nsm_qos_queue_e i = 0;
		qos->qos_class_enable = FALSE;
		qos->qos_class[queue] = class;
		for(i = NSM_QOS_QUEUE_0; i <= NSM_QOS_QUEUE_7; i++)
		{
			if(qos->qos_class[i])
				qos->qos_class_enable = TRUE;
		}
		return OK;
	}
	return ERROR;
}

int nsm_qos_class_get_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_class_e *class)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		if(class)
			*class = qos->qos_class[queue];
		return OK;
	}
	return ERROR;
}



/*
 * priority map to queue
 */
int nsm_qos_priority_map_queue_set_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_priority_e pri)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		nsm_qos_queue_e i = 0;
		qos->qos_priority_enable = FALSE;
		qos->qos_queue[queue] = pri;
		for(i = NSM_QOS_QUEUE_0; i <= NSM_QOS_QUEUE_7; i++)
		{
			if(qos->qos_queue[i])
				qos->qos_priority_enable = TRUE;
		}
		return OK;
	}
	return ERROR;
}

int nsm_qos_priority_map_queue_get_api(struct interface *ifp, nsm_qos_queue_e queue, nsm_qos_priority_e *pri)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		if(pri)
			*pri = qos->qos_queue[queue];
		return OK;
	}
	return ERROR;
}



/*
 * USER priority map LOCAL priority
 */
int nsm_qos_map_type_set_api(struct interface *ifp, nsm_qos_map_e type)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		qos->qos_map_type = type;
		return OK;
	}
	return ERROR;
}

int nsm_qos_map_type_get_api(struct interface *ifp, nsm_qos_map_e *type)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		if(type)
			*type = qos->qos_map_type;
		return OK;
	}
	return ERROR;
}

int nsm_qos_user_pri_map_priority_set_api(struct interface *ifp, nsm_qos_priority_e priority, nsm_qos_map_t map)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		nsm_qos_queue_e i = 0;
		qos->qos_map_enable = FALSE;
		qos->qos_map[priority] = map;
		return OK;
	}
	return ERROR;
}

int nsm_qos_user_pri_map_priority_get_api(struct interface *ifp, nsm_qos_priority_e priority, nsm_qos_map_t *map)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos)
	{
		if(map)
			*map = qos->qos_map[priority];
		return OK;
	}
	return ERROR;
}







static int nsm_qos_add_interface(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(if_is_loop(ifp))
		return OK;
	nsm_qos_t *qos = nsm->nsm_client[NSM_QOS] = XMALLOC(MTYPE_QOS, sizeof(nsm_qos_t));
	os_memset(nsm->nsm_client[NSM_QOS], 0, sizeof(nsm_qos_t));
	qos->ifindex = ifp->ifindex;
	return OK;
}


static int nsm_qos_del_interface(struct interface *ifp)
{
	struct nsm_interface *nsm = ifp->info[MODULE_NSM];
	if(if_is_loop(ifp))
		return OK;
	if(nsm->nsm_client[NSM_QOS])
		XFREE(MTYPE_QOS, nsm->nsm_client[NSM_QOS]);
	nsm->nsm_client[NSM_QOS] = NULL;
	return OK;
}


static int nsm_qos_interface_config(struct vty *vty, struct interface *ifp)
{
	nsm_qos_t *qos = _nsm_qos_get(ifp);
	if(qos && !if_is_loop(ifp))
	{
		if(qos->qos_storm_enable)
		{
			//vty_out(vty, " storm enable%s", VTY_NEWLINE);
			vty_out(vty, " storm control unicast%s%d%s", (qos->qos_storm.qos_storm_type == STORM_PER) ? " percent ":" ",qos->qos_storm.qos_unicast, VTY_NEWLINE);
			vty_out(vty, " storm control multicast%s%d%s", (qos->qos_storm.qos_storm_type == STORM_PER) ? " percent ":" ",qos->qos_storm.qos_multicast, VTY_NEWLINE);
			vty_out(vty, " storm control broadcast%s%d%s", (qos->qos_storm.qos_storm_type == STORM_PER) ? " percent ":" ",qos->qos_storm.qos_broadcast, VTY_NEWLINE);
		}
		if(qos->qos_trust != NSM_QOS_TRUST_NONE)
		{
			switch(qos->qos_trust)
			{
			case NSM_QOS_TRUST_PORT:
				vty_out(vty, " qos trust port%s", VTY_NEWLINE);
				break;
			case NSM_QOS_TRUST_EXP:
				vty_out(vty, " qos trust exp%s", VTY_NEWLINE);
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
		//queue map to class
		if(qos->qos_class_enable)
		{
			nsm_qos_queue_e i = 0;
			for(i = NSM_QOS_QUEUE_0; i <= NSM_QOS_QUEUE_7; i++)
			{
				if(qos->qos_class[i])
					vty_out(vty, " qos queue %d class %d %s", i - NSM_QOS_QUEUE_0,
							qos->qos_class[i] - NSM_QOS_CLASS_0, VTY_NEWLINE);
			}
		}
		//priority map to queue
		if(qos->qos_priority_enable)
		{
			nsm_qos_queue_e i = 0;
			for(i = NSM_QOS_QUEUE_0; i <= NSM_QOS_QUEUE_7; i++)
			{
				if(qos->qos_queue[i])
					vty_out(vty, " qos priority %d queue %d %s",
							qos->qos_queue[i] - NSM_QOS_PRI_0, i - NSM_QOS_QUEUE_0, VTY_NEWLINE);
			}
		}
		//USER priority map to LOCAL priority
		if(qos->qos_map_enable)
		{
			nsm_qos_priority_e i = 0;
			char *map[] = {"none", "cos","ip-pre","dscp","exp","none"};
			if(qos->qos_map_type)
			{
				vty_out(vty, " qos priority map %s %s",
					map[qos->qos_map_type], VTY_NEWLINE);
			}
			for(i = NSM_QOS_PRI_0; i <= NSM_QOS_PRI_7; i++)
			{
				if(qos->qos_map[i].cos)
					vty_out(vty, " qos %s-priority %d map to %d %s",
							map[qos->qos_map_type], qos->qos_map[i].cos, i - NSM_QOS_PRI_0, VTY_NEWLINE);
			}
		}
	}
	return OK;
}


static int nsm_qos_client_init()
{
	struct nsm_client *nsm = nsm_client_new ();
	nsm->notify_add_cb = nsm_qos_add_interface;
	nsm->notify_delete_cb = nsm_qos_del_interface;
	nsm->interface_write_config_cb = nsm_qos_interface_config;
	nsm_client_install (nsm, NSM_QOS);
	return OK;
}

int nsm_qos_init()
{
	return nsm_qos_client_init();
}

int nsm_qos_exit()
{
	struct nsm_client *nsm = nsm_client_lookup (NSM_QOS);
	if(nsm)
		nsm_client_free (nsm);
	return OK;
}
