/*
 * cmd_qos.c
 *
 *  Created on: Apr 20, 2018
 *      Author: zhurish
 */



#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"

#define QOS_STR		"Quality of Service\n"


#if NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_4
#define QUEUE_NUMSTR		"queue <0-3>"
#elif NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_8
#define QUEUE_NUMSTR		"queue <0-7>"
#endif


/*
 * global qos control
 */
DEFUN (qos_global_control,
		qos_global_control_cmd,
		"qos (enable|disable)" ,
		QOS_STR
		"Enable\n"
		"Disable\n")
{
	int ret = ERROR;
	if(strstr(argv[0], "enable"))
		ret = nsm_qos_global_enable(zpl_true);
	else
		ret = nsm_qos_global_enable(zpl_false);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_global_control,
		no_qos_global_control_cmd,
		"no qos enable",
		NO_STR
		QOS_STR
		"Enable\n")
{
	int ret = ERROR;
	ret = nsm_qos_global_enable(zpl_false);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (qos_global_shaping_control,
		qos_global_shaping_control_cmd,
		"qos ipg (enable|disable)",
		QOS_STR
		"Stream Shaping\n"
		"Enable\n"
		"Disable\n")
{
	int ret = ERROR;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	if(strstr(argv[0], "enable"))
		ret = nsm_qos_shaping_global_enable(zpl_true);
	else
		ret = nsm_qos_shaping_global_enable(zpl_false);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_global_shaping_control,
		no_qos_global_shaping_control_cmd,
		"no qos ipg enable",
		NO_STR
		QOS_STR
		"Stream Shaping\n"
		"Enable\n")
{
	int ret = ERROR;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	ret = nsm_qos_shaping_global_enable(zpl_false);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
/*
 * storm control
 */
DEFUN (qos_storm_control,
		qos_storm_control_cmd,
		"storm control (broadcast|multicast|unicast) <0-1000000000>" ,
		"Storm control\n"
		"Control\n"
		"broadcast\n"
		"multicast\n"
		"unicast\n"
		"value of bps\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_storm_enable_get_api(ifp))
	{
		ret = nsm_qos_storm_enable_set_api(ifp, zpl_true);
	}
	if(nsm_qos_storm_enable_get_api(ifp))
	{
		zpl_uint32 qos_unicast = 0;
		zpl_uint32 qos_multicast = 0;
		zpl_uint32 qos_broadcast = 0;

		if(strncmp(argv[0], "broadcast", 3) == 0)
		{
			qos_broadcast = atoi(argv[1]);
			ret = nsm_qos_storm_broadcast_set_api(ifp,  qos_broadcast, NSM_QOS_STORM_RATE);
		}
		else if(strncmp(argv[0], "multicast", 3) == 0)
		{
			qos_multicast = atoi(argv[1]);
			ret = nsm_qos_storm_multicast_set_api(ifp,  qos_multicast, NSM_QOS_STORM_RATE);
		}
		else if(strncmp(argv[0], "unicast", 3) == 0)
		{
			qos_unicast = atoi(argv[1]);
			ret = nsm_qos_storm_unicast_set_api(ifp,  qos_unicast, NSM_QOS_STORM_RATE);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (qos_storm_control_percent,
		qos_storm_control_percent_cmd,
		"storm control (broadcast|multicast|unicast) percent <0-100>" ,
		"Storm control\n"
		"Control\n"
		"broadcast\n"
		"multicast\n"
		"unicast\n"
		"percent\n"
		"value of percent\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_storm_enable_get_api(ifp))
	{
		ret = nsm_qos_storm_enable_set_api(ifp, zpl_true);
	}
	if(nsm_qos_storm_enable_get_api(ifp))
	{
		zpl_uint32 qos_unicast = 0;
		zpl_uint32 qos_multicast = 0;
		zpl_uint32 qos_broadcast = 0;
		
		if(strncmp(argv[0], "broadcast", 3) == 0)
		{
			qos_broadcast = atoi(argv[1]);
			ret = nsm_qos_storm_broadcast_set_api(ifp,  qos_broadcast, NSM_QOS_STORM_PERCENT);
		}
		else if(strncmp(argv[0], "multicast", 3) == 0)
		{
			qos_multicast = atoi(argv[1]);
			ret = nsm_qos_storm_multicast_set_api(ifp,  qos_multicast, NSM_QOS_STORM_PERCENT);
		}
		else if(strncmp(argv[0], "unicast", 3) == 0)
		{
			qos_unicast = atoi(argv[1]);
			ret = nsm_qos_storm_unicast_set_api(ifp,  qos_unicast, NSM_QOS_STORM_PERCENT);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (qos_storm_control_packet,
		qos_storm_control_packet_cmd,
		"storm control (broadcast|multicast|unicast) pps <0-1000000000>" ,
		"Storm control\n"
		"Control\n"
		"broadcast\n"
		"multicast\n"
		"unicast\n"
		"Pakcet\n"
		"value of Pakcet\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_storm_enable_get_api(ifp))
	{
		ret = nsm_qos_storm_enable_set_api(ifp, zpl_true);
	}
	if(nsm_qos_storm_enable_get_api(ifp))
	{
		zpl_uint32 qos_unicast = 0;
		zpl_uint32 qos_multicast = 0;
		zpl_uint32 qos_broadcast = 0;
		
		if(strncmp(argv[0], "broadcast", 3) == 0)
		{
			qos_broadcast = atoi(argv[1]);
			ret = nsm_qos_storm_broadcast_set_api(ifp,  qos_broadcast, NSM_QOS_STORM_PACKET);
		}
		else if(strncmp(argv[0], "multicast", 3) == 0)
		{
			qos_multicast = atoi(argv[1]);
			ret = nsm_qos_storm_multicast_set_api(ifp,  qos_multicast, NSM_QOS_STORM_PACKET);
		}
		else if(strncmp(argv[0], "unicast", 3) == 0)
		{
			qos_unicast = atoi(argv[1]);
			ret = nsm_qos_storm_unicast_set_api(ifp,  qos_unicast, NSM_QOS_STORM_PACKET);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_storm_control,
		no_qos_storm_control_cmd,
		"no storm control (broadcast|multicast|unicast)" ,
		NO_STR
		"Storm control\n"
		"Control\n"
		"broadcast\n"
		"multicast\n"
		"unicast\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(nsm_qos_storm_enable_get_api(ifp))
	{
		ret = nsm_qos_storm_enable_set_api(ifp, zpl_true);
	}
	if(nsm_qos_storm_enable_get_api(ifp))
	{
		zpl_uint32 qos_unicast = 0;
		zpl_uint32 qos_multicast = 0;
		zpl_uint32 qos_broadcast = 0;
		
		if(strncmp(argv[0], "broadcast", 3) == 0)
		{
			qos_broadcast = 0;
			ret = nsm_qos_storm_broadcast_set_api(ifp,  qos_broadcast, NSM_QOS_STORM_RATE);
		}
		else if(strncmp(argv[0], "multicast", 3) == 0)
		{
			qos_multicast = 0;
			ret = nsm_qos_storm_multicast_set_api(ifp,  qos_multicast, NSM_QOS_STORM_RATE);
		}
		else if(strncmp(argv[0], "unicast", 3) == 0)
		{
			qos_unicast = 0;
			ret = nsm_qos_storm_unicast_set_api(ifp,  qos_unicast, NSM_QOS_STORM_RATE);
		}
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * queue map to class
 */
DEFUN (qos_queue_map_class,
		qos_queue_map_class_cmd,
#if NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_4
		"qos queue <0-3> class <0-3>",
#elif NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_8
		"qos queue <0-7> class <0-3>",
#endif
		QOS_STR
		"queue\n"
		"Queue ID\n"
		"Class\n"
		"Class ID\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_queue_e queue = atoi(argv[0]) + NSM_QOS_QUEUE_0;
	nsm_qos_class_e class = atoi(argv[1]) + NSM_QOS_CLASS_0;
	ret = nsm_qos_class_map_set_api(ifp, queue, class);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_queue_map_class,
		no_qos_queue_map_class_cmd,
#if NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_4
		"no qos queue <0-3> class",
#elif NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_8
		"no qos queue <0-7> class",
#endif
		NO_STR
		QOS_STR
		"queue\n"
		"Queue ID\n"
		"Class\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_queue_e queue = atoi(argv[0]) + NSM_QOS_QUEUE_0;
	ret = nsm_qos_class_map_set_api(ifp, queue, NSM_QOS_CLASS_NONE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * class sched
 */
DEFUN (qos_queue_class_sched,
		qos_queue_class_sched_cmd,
		"qos class <0-3> sp",
		QOS_STR
		"Class\n"
		"Class ID\n"
		"weighted round robin\n"
		"Strict priority\n")
{
	int ret = ERROR, weight = 0;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_class_e class = atoi(argv[0]) + NSM_QOS_CLASS_0;
	ret = nsm_qos_class_sched_set_api(ifp, class, NSM_CLASS_SCHED_PQ, weight);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
DEFUN (qos_queue_class_sched_weight,
		qos_queue_class_sched_weight_cmd,
		"qos class <0-3> wrr <1-127>",
		QOS_STR
		"Class\n"
		"Class ID\n"
		"weighted round robin\n"
		"Strict priority\n")
{
	int ret = ERROR, weight = 0;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_class_e class = atoi(argv[0]) + NSM_QOS_CLASS_0;
	weight = atoi(argv[1]);
	ret = nsm_qos_class_sched_set_api(ifp, class, NSM_CLASS_SCHED_WRR, weight);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_queue_class_sched,
		no_qos_queue_class_sched_cmd,
		"no qos class <0-3> (wrr|sp)",
		NO_STR
		QOS_STR
		"Class\n"
		"Class ID\n"
		"weighted round robin\n"
		"Strict priority\n")
{
	int ret = ERROR, weight = 0;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_class_e class = atoi(argv[0]) + NSM_QOS_CLASS_0;
	ret = nsm_qos_class_sched_set_api(ifp, class, NSM_CLASS_SCHED_PQ, weight);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



/*
 * trust
 */
DEFUN (qos_trust_type,
		qos_trust_type_cmd,
		"qos trust (port|mplsexp|cos|dscp|ip-prec)" ,
		QOS_STR
		"enable the port trust\n"
		"port\n"
		"mpls-exp value\n"
		"cos value\n"
		"dscp value\n"
		"ip-precedence value\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_trust_e trust = NSM_QOS_TRUST_NONE;
	if(strncmp(argv[0], "port", 3) == 0)
		trust = NSM_QOS_TRUST_PORT;
	else if(strncmp(argv[0], "mplsexp", 3) == 0)
		trust = NSM_QOS_TRUST_EXP;
	else if(strncmp(argv[0], "cos", 3) == 0)
		trust = NSM_QOS_TRUST_COS;
	else if(strncmp(argv[0], "dscp", 3) == 0)
		trust = NSM_QOS_TRUST_DSCP;
	else if(strncmp(argv[0], "ip-prec", 3) == 0)
		trust = NSM_QOS_TRUST_IP_PRE;

	ret = nsm_qos_trust_set_api(ifp,  trust);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_trust_type,
		no_qos_trust_type_cmd,
		"no qos trust" ,
		NO_STR
		QOS_STR
		"enable the port trust\n\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_trust_e trust = NSM_QOS_TRUST_NONE;
	ret = nsm_qos_trust_set_api(ifp,  trust);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


/*
 * wrr-queue weight
 */
DEFUN (qos_queue_sched_mode_sp,
		qos_queue_sched_mode_sp_cmd,
#if NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_4
		"qos queue <0-3> sp",
#elif NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_8
		"qos queue <0-7> sp",
#endif
		QOS_STR
		"Queue\n"
		"Queue ID\n"
		"Strict priority\n")
{
	int ret = ERROR, weight = 0;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_queue_e queue = atoi(argv[0]) + NSM_QOS_QUEUE_0;
	ret = nsm_qos_queue_sched_mode_set_api(ifp, queue, NSM_QOS_MODE_SP, weight);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_queue_sched_mode_sp,
		no_qos_queue_sched_mode_sp_cmd,
#if NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_4
		"no qos queue <0-3> sp",
#elif NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_8
		"no qos queue <0-7> sp",
#endif
		NO_STR
		QOS_STR
		"Queue\n"
		"Queue ID\n"
		"Strict priority\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_queue_e queue = atoi(argv[0]) + NSM_QOS_QUEUE_0;
	ret = nsm_qos_queue_sched_mode_set_api(ifp, queue, NSM_QOS_MODE_SP, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (qos_queue_sched_mode_weight,
		qos_queue_sched_mode_weight_cmd,
#if NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_4
		"qos queue <0-3> wrr-weight <1-128>",
#elif NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_8
		"qos queue <0-7> wrr-weight <1-128>",
#endif
		QOS_STR
		"Queue\n"
		"Queue ID\n"
		"Weighted round robin\n"
		"weight\n")
{
	int ret = ERROR, weight = 0;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_queue_e queue = atoi(argv[0]) + NSM_QOS_QUEUE_0;
	weight = atoi(argv[1]);
	ret = nsm_qos_queue_sched_mode_set_api(ifp, queue, NSM_QOS_MODE_WRR, weight);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_queue_sched_mode_weight,
		no_qos_queue_sched_mode_weight_cmd,
#if NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_4
		"no qos queue <0-3> wrr-weight",
#elif NSM_QOS_PORT_QUEUE_NUM==NSM_QOS_PORT_QUEUE_NUM_8
		"no qos queue <0-7> wrr-weight",
#endif
		NO_STR
		QOS_STR
		"Queue\n"
		"Queue ID\n"
		"Weighted round robin\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_queue_e queue = atoi(argv[0]) + NSM_QOS_QUEUE_0;
	ret = nsm_qos_queue_sched_mode_set_api(ifp, queue, NSM_QOS_MODE_SP, 0);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



#ifdef NSM_QOS_USERPRI_MAP_QUEUE
/*用户优先级到队列的映射*/
DEFUN (qos_cos_priority_queue_map,
		qos_cos_priority_queue_map_cmd,
		"qos cos-priority <0-7> "QUEUE_NUMSTR ,
		QOS_STR
		"Cos Priority\n"
		"Cos Priority value\n"
		"Queue\n"
		"Queue ID\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_queue_e queue = atoi(argv[1]) + NSM_QOS_QUEUE_0;
	ret = nsm_qos_cos_map_queue_set_api(ifp, zpl_false,  pri,  queue);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_cos_priority_queue_map,
		no_qos_cos_priority_queue_map_cmd,
		"no qos cos-priority <0-7> queue" ,
		NO_STR
		QOS_STR
		"Cos Priority\n"
		"Cos Priority value\n"
		"Queue\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_queue_e queue = NSM_QOS_QUEUE_NONE;
	ret = nsm_qos_cos_map_queue_set_api(ifp, zpl_false,  pri,  queue);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (qos_exp_priority_queue_map,
		qos_exp_priority_queue_map_cmd,
		"qos mplsexp-priority <0-7> "QUEUE_NUMSTR ,
		QOS_STR
		"MPLS Exp Priority\n"
		"MPLS Exp Priority value\n"
		"Queue\n"
		"Queue ID\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_queue_e queue = atoi(argv[1]) + NSM_QOS_QUEUE_0;
	ret = nsm_qos_exp_map_queue_set_api(ifp, zpl_false,  pri,  queue);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_exp_priority_queue_map,
		no_qos_exp_priority_queue_map_cmd,
		"no qos mplsexp-priority <0-7> queue" ,
		NO_STR
		QOS_STR
		"MPLS Exp Priority\n"
		"MPLS Exp Priority value\n"
		"Queue\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_queue_e queue = NSM_QOS_QUEUE_NONE;
	ret = nsm_qos_exp_map_queue_set_api(ifp, zpl_false,  pri,  queue);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (qos_ippre_priority_queue_map,
		qos_ippre_priority_queue_map_cmd,
		"qos ip-prec-priority <0-7> "QUEUE_NUMSTR ,
		QOS_STR
		"Ip Pre Priority\n"
		"Ip Pre Priority value\n"
		"Queue\n"
		"Queue ID\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_queue_e queue = atoi(argv[1]) + NSM_QOS_QUEUE_0;
	ret = nsm_qos_ippre_map_queue_set_api(ifp, zpl_false,  pri,  queue);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_ippre_priority_queue_map,
		no_qos_ippre_priority_queue_map_cmd,
		"no qos ip-prec-priority <0-7> queue" ,
		NO_STR
		QOS_STR
		"Ip Pre Priority\n"
		"Ip Pre Priority value\n"
		"Queue\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_queue_e queue = NSM_QOS_QUEUE_NONE;
	ret = nsm_qos_ippre_map_queue_set_api(ifp, zpl_false,  pri,  queue);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (qos_dscp_priority_queue_map,
		qos_dscp_priority_queue_map_cmd,
		"qos dscp-priority <0-64> "QUEUE_NUMSTR ,
		QOS_STR
		"Dscp Priority\n"
		"Dscp Priority value\n"
		"Queue\n"
		"Queue ID\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	zpl_uint8 pri = atoi(argv[0]);
	nsm_qos_queue_e queue = atoi(argv[1]) + NSM_QOS_QUEUE_0;
	ret = nsm_qos_ippre_map_queue_set_api(ifp, zpl_false,  pri,  queue);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_dscp_priority_queue_map,
		no_qos_dscp_priority_queue_map_cmd,
		"no qos dscp-priority <0-64> queue" ,
		NO_STR
		QOS_STR
		"Dscp Priority\n"
		"Dscp Priority value\n"
		"Queue\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	zpl_uint8 pri = atoi(argv[0]);
	nsm_qos_queue_e queue = NSM_QOS_QUEUE_NONE;
	ret = nsm_qos_dscp_map_queue_set_api(ifp, zpl_false,  pri,  queue);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif




#ifdef NSM_QOS_PRIORITY_MAP_QUEUE
/*
 * inside priority map to queue
 */
DEFUN (qos_inside_priority_queue_map,
		qos_inside_priority_queue_map_cmd,
		"qos priority <0-7> "QUEUE_NUMSTR ,
		QOS_STR
		"Priority\n"
		"Priority value\n"
		"Queue\n"
		"Queue ID\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_queue_e queue = atoi(argv[1]) + NSM_QOS_QUEUE_0;
	ret = nsm_qos_priority_map_queue_set_api(ifp,  pri,  queue);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_inside_priority_queue_map,
		no_qos_inside_priority_queue_map_cmd,
		"no qos priority <0-7> queue" ,
		NO_STR
		QOS_STR
		"Priority\n"
		"Priority value\n"
		"Queue\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_queue_e queue = NSM_QOS_QUEUE_NONE;
	ret = nsm_qos_priority_map_queue_set_api(ifp, pri,  queue);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif

#ifdef NSM_QOS_QUEUE_MAP_PRIORITY
/*
 * queue map to inside priority
 */
DEFUN (qos_queue_inside_priority_map,
		qos_queue_inside_priority_map_cmd,
		"qos "QUEUE_NUMSTR" priority <0-7>" ,
		QOS_STR
		"Queue\n"
		"Queue ID\n"
		"Priority\n"
		"Priority value\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[1]) + NSM_QOS_PRI_0;
	nsm_qos_queue_e queue = atoi(argv[0]) + NSM_QOS_QUEUE_0;
	ret = nsm_qos_queue_map_priority_set_api(ifp, queue, pri);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_queue_inside_priority_map,
		no_qos_queue_inside_priority_map_cmd,
		"no qos "QUEUE_NUMSTR" priority" ,
		NO_STR
		QOS_STR
		"Queue\n"
		"Queue ID\n"
		"Priority\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_queue_e queue  = atoi(argv[0]) + NSM_QOS_QUEUE_0;
	ret = nsm_qos_queue_map_priority_set_api(ifp,  queue, NSM_QOS_PRI_NONE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif

#ifdef NSM_QOS_USERPRI_MAP_PRIORITY
/*
 * user priority map to inside priority
 */
DEFUN (qos_cos_priority_priority_map,
		qos_cos_priority_priority_map_cmd,
		"qos cos-priority <0-7> priority <0-7>" ,
		QOS_STR
		"Cos Priority\n"
		"Cos Priority value\n"
		"Priority\n"
		"Priority value\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_priority_e priority = atoi(argv[1]) + NSM_QOS_PRI_0;
	ret = nsm_qos_cos_map_priority_set_api(ifp,  pri,  priority);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_cos_priority_priority_map,
		no_qos_cos_priority_priority_map_cmd,
		"no qos cos-priority <0-7> priority" ,
		NO_STR
		QOS_STR
		"Cos Priority\n"
		"Cos Priority value\n"
		"Priority\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_priority_e priority = NSM_QOS_PRI_NONE;
	ret = nsm_qos_cos_map_priority_set_api(ifp,  pri,  priority);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (qos_exp_priority_priority_map,
		qos_exp_priority_priority_map_cmd,
		"qos mplsexp-priority <0-7> priority <0-7>" ,
		QOS_STR
		"MPLS Exp Priority\n"
		"MPLS Exp Priority value\n"
		"Priority\n"
		"Priority value\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_priority_e priority = atoi(argv[1]) + NSM_QOS_PRI_0;
	ret = nsm_qos_mplsexp_map_priority_set_api(ifp,  pri,  priority);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_exp_priority_priority_map,
		no_qos_exp_priority_priority_map_cmd,
		"no qos mplsexp-priority <0-7> priority" ,
		NO_STR
		QOS_STR
		"MPLS Exp Priority\n"
		"MPLS Exp Priority value\n"
		"Priority\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_priority_e priority = NSM_QOS_PRI_NONE;
	ret = nsm_qos_mplsexp_map_priority_set_api(ifp,  pri,  priority);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (qos_ippre_priority_priority_map,
		qos_ippre_priority_priority_map_cmd,
		"qos ip-prec-priority <0-7> priority <0-7>" ,
		QOS_STR
		"Ip Pre Priority\n"
		"Ip Pre Priority value\n"
		"Priority\n"
		"Priority value\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_priority_e priority = atoi(argv[1]) + NSM_QOS_PRI_0;
	ret = nsm_qos_ipprec_map_priority_set_api(ifp,   pri,  priority);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_ippre_priority_priority_map,
		no_qos_ippre_priority_priority_map_cmd,
		"no qos ip-prec-priority <0-7> priority" ,
		NO_STR
		QOS_STR
		"Ip Pre Priority\n"
		"Ip Pre Priority value\n"
		"Priority\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e pri = atoi(argv[0]) + NSM_QOS_PRI_0;
	nsm_qos_priority_e priority = NSM_QOS_PRI_NONE;
	ret = nsm_qos_ipprec_map_priority_set_api(ifp,   pri,  priority);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (qos_dscp_priority_priority_map,
		qos_dscp_priority_priority_map_cmd,
		"qos dscp-priority <0-64> priority <0-7>" ,
		QOS_STR
		"Dscp Priority\n"
		"Dscp Priority value\n"
		"Priority\n"
		"Priority value\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	zpl_uint8 pri = atoi(argv[0]);
	nsm_qos_priority_e priority = atoi(argv[1]) + NSM_QOS_PRI_0;
	ret = nsm_qos_dscp_map_priority_set_api(ifp,  pri,  priority);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_dscp_priority_priority_map,
		no_qos_dscp_priority_priority_map_cmd,
		"no qos dscp-priority <0-64> priority" ,
		NO_STR
		QOS_STR
		"Dscp Priority\n"
		"Dscp Priority value\n"
		"Priority\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	zpl_uint8 pri = atoi(argv[0]);
	nsm_qos_priority_e priority = NSM_QOS_PRI_NONE;
	ret = nsm_qos_dscp_map_priority_set_api(ifp,  pri,  priority);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif

DEFUN (qos_cos_priority_replace,
		qos_cos_priority_replace_cmd,
		"qos replace cos-priority <0-7>" ,
		QOS_STR
		"Replace\n"
		"Cos Priority\n"
		"Cos Priority value\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e priority = atoi(argv[0]) + NSM_QOS_PRI_0;
	ret = nsm_qos_cos_replace_set_api(ifp,  priority);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_cos_priority_replace,
		no_qos_cos_priority_replace_cmd,
		"no qos replace cos-priority" ,
		NO_STR
		QOS_STR
		"Replace\n"
		"Dscp Priority\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	ret = nsm_qos_cos_replace_set_api(ifp,  NSM_QOS_PRI_NONE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (qos_dscp_priority_replace,
		qos_dscp_priority_replace_cmd,
		"qos replace dscp-priority <0-64>" ,
		QOS_STR
		"Replace\n"
		"Dscp Priority\n"
		"Dscp Priority value\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	nsm_qos_priority_e priority = atoi(argv[0])+NSM_QOS_PRI_0;
	ret = nsm_qos_dscp_replace_set_api(ifp,  priority);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_dscp_priority_replace,
		no_qos_dscp_priority_replace_cmd,
		"no qos replace dscp-priority" ,
		NO_STR
		QOS_STR
		"Replace\n"
		"Dscp Priority\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	ret = nsm_qos_dscp_replace_set_api(ifp,  NSM_QOS_PRI_NONE);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (qos_service_policy,
		qos_service_policy_cmd,
		"qos service-policy (inbound|outbound) NAME" ,
		QOS_STR
		"Service Policy\n"
		"Inbound Direction\n"
		"Outbound Direction\n"
		"Policy Name\n")
{
	int ret = ERROR, input = 0;
	struct interface *ifp = vty->index;
	zpl_char service_policy[64];
	if(strncmp(argv[0], "inbound",4) == 0)
		input = 1;
	else if(strncmp(argv[0], "outbound",4) == 0)
		input = 0;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	if(qos_service_policy_lookup(service_policy) == NULL)
	{
		vty_out(vty, "the policy-map '%s' is not exist%s",argv[1], VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	memset(service_policy, 0, sizeof(service_policy));
	ret = nsm_qos_service_policy_get_api(ifp,  input, service_policy);
	if(strlen(service_policy))
	{
		vty_out(vty, "Service Policy is setup.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	ret = nsm_qos_service_policy_set_api(ifp,  input, argv[1]);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_service_policy,
		no_qos_service_policy_cmd,
		"no qos service-policy (inbound|outbound)" ,
		NO_STR
		QOS_STR
		"Service Policy\n"
		"Inbound Direction\n"
		"Outbound Direction\n")
{
	int ret = ERROR;
	int input = 0;
	struct interface *ifp = vty->index;
	zpl_char service_policy[64];
	if(strncmp(argv[0], "inbound",4) == 0)
		input = 1;
	else if(strncmp(argv[0], "outbound",4) == 0)
		input = 0;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	ret = nsm_qos_service_policy_get_api(ifp,  input, service_policy);
	if(strlen(service_policy))
	{
		ret = nsm_qos_service_policy_set_api(ifp,  input, NULL);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (qos_policer_cir,
		qos_policer_cir_cmd,
		"qos policer (inbound|outbound) cir <1-10000000>" ,
		QOS_STR
		"Policer Rate Limit\n"
		"Inbound Direction\n"
		"Outbound Direction\n"
		"Committed Information Rate\n"
		"Cir Value\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	nsm_qos_limit_t rate;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	memset(&rate, 0, sizeof(rate));
	if(strncmp(argv[0], "inbound", 5) == 0)
	{
		ret = nsm_qos_rate_get_api(ifp,  NSM_QOS_DIR_INBOUND, &rate);
	}
	else if(strncmp(argv[0], "outbound", 5) == 0)
	{
		ret = nsm_qos_rate_get_api(ifp,  NSM_QOS_DIR_OUTBOUND, &rate);	
	}
	if(argc == 2)
	{
		rate.qos_cir = atoi(argv[1]);
	}
	if(argc == 3)
	{
		rate.qos_cir = atoi(argv[1]);
		rate.qos_pir = atoi(argv[2]);
	}
	if(argc == 4)
	{
		rate.qos_cir = atoi(argv[1]);
		rate.qos_pir = atoi(argv[2]);
		rate.qos_cbs = atoi(argv[3]);
	}
	if(strncmp(argv[0], "inbound", 5) == 0)
	{
		ret = nsm_qos_rate_set_api(ifp,  NSM_QOS_DIR_INBOUND, &rate);
	}
	else if(strncmp(argv[0], "outbound", 5) == 0)
	{
		ret = nsm_qos_rate_set_api(ifp,  NSM_QOS_DIR_OUTBOUND, &rate);	
	}	
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS (qos_policer_cir,
		qos_policer_cir_pir_cmd,
		"qos policer (inbound|outbound) cir <1-10000000> pir <0-4000000>" ,
		QOS_STR
		"Policer Rate Limit\n"
		"Inbound Direction\n"
		"Outbound Direction\n"
		"Committed Information Rate\n"
		"Cir Value\n"
		"Peak Information Rate\n"
		"Pir Value\n")

ALIAS (qos_policer_cir,
		qos_policer_cir_pir_cbs_cmd,
		"qos policer (inbound|outbound) cir <1-10000000> pir <0-4000000> cbs <0-4000000>",
		QOS_STR
		"Policer Rate Limit\n"
		"Inbound Direction\n"
		"Outbound Direction\n"
		"Committed Information Rate\n"
		"Cir Value\n"
		"Peak Information Rate\n"
		"Pir Value\n"
		"Committed Burst Size\n"
		"Cbs Value\n")

DEFUN (no_qos_policer_cir,
		no_qos_policer_cir_cmd,
		"no qos policer (inbound|outbound)" ,
		NO_STR
		QOS_STR
		"Policer Rate Limit\n"
		"Inbound Direction\n"
		"Outbound Direction\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	nsm_qos_limit_t rate;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	memset(&rate, 0, sizeof(rate));
	if(strncmp(argv[0], "inbound", 5) == 0)
		ret = nsm_qos_rate_set_api(ifp,  NSM_QOS_DIR_INBOUND, &rate);
	else if(strncmp(argv[0], "outbound", 5) == 0)
		ret = nsm_qos_rate_set_api(ifp,  NSM_QOS_DIR_OUTBOUND, &rate);	
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



/*
 * trust
 */
DEFUN (qos_shaping,
		qos_shaping_cmd,
		"qos shaping" ,
		QOS_STR
		"Shaping Enable\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_shaping_global_get())
	{
		vty_out(vty, "Please Enable Qos Shaping frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	ret = nsm_qos_shaping_set_api(ifp,  zpl_true);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_shaping,
		no_qos_shaping_cmd,
		"no qos shaping" ,
		NO_STR
		QOS_STR
		"Shaping Enable\n")
{
	int ret = ERROR;
	struct interface *ifp = vty->index;
	if(!nsm_qos_shaping_global_get())
	{
		vty_out(vty, "Please Enable Qos Shaping frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	ret = nsm_qos_shaping_set_api(ifp,  zpl_false);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (show_qos_interface_info,
		show_qos_interface_info_cmd,
		"show qos interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		SHOW_STR
		QOS_STR
		"Select an interface to configure\n"
		CMD_IF_USPV_STR_HELP
		CMD_USP_STR_HELP)
{
	int ret = ERROR;
	struct interface *ifp = NULL;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Qos is Not Enable.%s",VTY_NEWLINE);
		return CMD_SUCCESS;
	}
	if (argv[0] && argv[1])
	{
		ifp = if_lookup_by_name (if_ifname_format(argv[0], argv[1]));
		if(ifp)
			ret = nsm_qos_interface_show(vty, ifp);
	}
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}





static int cmd_qos_intf_init(int node)
{

	install_element(node, CMD_CONFIG_LEVEL, &qos_storm_control_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &qos_storm_control_percent_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &qos_storm_control_packet_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_storm_control_cmd);	

	install_element(node, CMD_CONFIG_LEVEL, &qos_shaping_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_shaping_cmd);	

	install_element(node, CMD_CONFIG_LEVEL, &qos_queue_map_class_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_queue_map_class_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &qos_queue_class_sched_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &qos_queue_class_sched_weight_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_queue_class_sched_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &qos_trust_type_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_trust_type_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &qos_queue_sched_mode_sp_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_queue_sched_mode_sp_cmd);	
	install_element(node, CMD_CONFIG_LEVEL, &qos_queue_sched_mode_weight_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_queue_sched_mode_weight_cmd);

	#ifdef NSM_QOS_USERPRI_MAP_QUEUE
	install_element(node, CMD_CONFIG_LEVEL, &qos_cos_priority_queue_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_cos_priority_queue_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &qos_exp_priority_queue_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_exp_priority_queue_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &qos_ippre_priority_queue_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_ippre_priority_queue_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &qos_dscp_priority_queue_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_dscp_priority_queue_map_cmd);
	#endif
	#ifdef NSM_QOS_PRIORITY_MAP_QUEUE
	install_element(node, CMD_CONFIG_LEVEL, &qos_inside_priority_queue_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_inside_priority_queue_map_cmd);
	#endif
	#ifdef NSM_QOS_QUEUE_MAP_PRIORITY
	install_element(node, CMD_CONFIG_LEVEL, &qos_queue_inside_priority_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_queue_inside_priority_map_cmd);
	#endif
	#ifdef NSM_QOS_USERPRI_MAP_PRIORITY
	install_element(node, CMD_CONFIG_LEVEL, &qos_cos_priority_priority_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_cos_priority_priority_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &qos_exp_priority_priority_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_exp_priority_priority_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &qos_ippre_priority_priority_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_ippre_priority_priority_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &qos_dscp_priority_priority_map_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_dscp_priority_priority_map_cmd);
	#endif

	install_element(node, CMD_CONFIG_LEVEL, &qos_cos_priority_replace_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_cos_priority_replace_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &qos_dscp_priority_replace_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_dscp_priority_replace_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &qos_service_policy_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_qos_service_policy_cmd);
	return OK;
}

int cmd_qos_init()
{
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &qos_global_control_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_qos_global_control_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &qos_global_shaping_control_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_qos_global_shaping_control_cmd);
	
	cmd_qos_intf_init(INTERFACE_NODE);
	cmd_qos_intf_init(INTERFACE_L3_NODE);
	cmd_qos_intf_init(LAG_INTERFACE_NODE);
	cmd_qos_intf_init(LAG_INTERFACE_L3_NODE);
	cmd_qos_intf_init(WIRELESS_INTERFACE_NODE);
	cmd_qos_intf_init(TUNNEL_INTERFACE_NODE);
	cmd_qos_intf_init(BRIGDE_INTERFACE_NODE);

	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_qos_interface_info_cmd);
	cmd_qos_acl_init();
	return OK;
}