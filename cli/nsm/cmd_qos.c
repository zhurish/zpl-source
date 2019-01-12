/*
 * cmd_qos.c
 *
 *  Created on: Apr 20, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "vrf.h"
#include "interface.h"
#include "nsm_mac.h"
#include "nsm_arp.h"
#include "nsm_qos.h"

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

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * trust
 */
DEFUN (qos_trust_type,
		qos_trust_type_cmd,
		"qos trust (port|mpls-exp|cos|dscp|ip-precedence)" ,
		"qos\n"
		"enable the port trust\n"
		"port\n"
		"mpls-exp value\n"
		"cos value\n"
		"dscp value\n"
		"ip-precedence value\n")
{
	int ret = ERROR;

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_trust_type,
		no_qos_trust_type_cmd,
		"no qos trust" ,
		NO_STR
		"qos\n"
		"enable the port trust\n\n")
{
	int ret = ERROR;

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * cos default
 */
DEFUN (qos_cos,
		qos_cos_cmd,
		"qos cos <0-7>" ,
		"qos\n"
		"enable the port cos\n"
		"cos value\n")
{
	int ret = ERROR;

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_cos,
		no_qos_cos_cmd,
		"no qos cos" ,
		NO_STR
		"qos\n"
		"enable the port cos\n\n")
{
	int ret = ERROR;

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * queue map to class
 */
DEFUN (qos_queue_class,
		qos_queue_class_cmd,
		"queue <0-7> class <1-3>" ,
		"queue\n"
		"Queue ID\n"
		"Class\n"
		"Class ID\n")
{
	int ret = ERROR;

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_queue_class,
		no_qos_queue_class_cmd,
		"no queue <0-7> class" ,
		NO_STR
		"queue\n"
		"Queue ID\n"
		"Class\n")
{
	int ret = ERROR;

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * priority map to queue
 */
DEFUN (qos_priority_queue,
		qos_priority_queue_cmd,
		"priority <0-7> queue <0-7>" ,
		"priority\n"
		"priority value\n"
		"queue\n"
		"Queue ID\n")
{
	int ret = ERROR;

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_priority_queue,
		no_qos_priority_queue_cmd,
		"no priority <0-7> queue" ,
		NO_STR
		"priority\n"
		"priority value\n"
		"queue\n")
{
	int ret = ERROR;

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


/*
 * user priority map to local priority
 */
DEFUN (qos_priority_cos_map,
		qos_priority_cos_map_cmd,
		"qos (cos-priority|ip-pre-priority|exp-priority) <0-7> priority <0-7>" ,
		"qos\n"
		"cos priority\n"
		"ip precedence priority\n"
		"dscp priority\n"
		"mpls exp priority\n"
		"priority value\n"
		"priority\n"
		"priority value\n")
{
	int ret = ERROR;

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_priority_cos_map,
		no_qos_priority_cos_map_cmd,
		"no qos (cos-priority|ip-pre-priority|exp-priority) <0-7> priority" ,
		NO_STR
		"qos\n"
		"cos priority\n"
		"ip precedence priority\n"
		"dscp priority\n"
		"mpls exp priority\n"
		"priority value\n"
		"priority\n")
{
	int ret = ERROR;

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
/*
 * dscp map priority
 */
DEFUN (qos_dscp_priority_cos_map,
		qos_dscp_priority_cos_map_cmd,
		"qos dscp-priority <0-63> priority <0-7>" ,
		"qos\n"
		"cos priority\n"
		"ip precedence priority\n"
		"dscp priority\n"
		"mpls exp priority\n"
		"priority value\n"
		"priority\n"
		"priority value\n")
{
	int ret = ERROR;

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_dscp_priority_cos_map,
		no_qos_dscp_priority_cos_map_cmd,
		"no qos dscp-priority <0-63> priority" ,
		NO_STR
		"qos\n"
		"cos priority\n"
		"ip precedence priority\n"
		"dscp priority\n"
		"mpls exp priority\n"
		"priority value\n"
		"priority\n")
{
	int ret = ERROR;

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


/*
 * wrr-queue weight
 */
DEFUN (qos_wrr_queue_weight,
		qos_wrr_queue_weight_cmd,
		"wrr-queue <0-7> weight <1-100>" ,
		"wrr-queue\n"
		"Queue ID\n"
		"weight\n"
		"value of percent\n")
{
	int ret = ERROR;
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_qos_wrr_queue_weight,
		no_qos_wrr_queue_weight_cmd,
		"no wrr-queue <0-7> weight" ,
		NO_STR
		"wrr-queue\n"
		"Queue ID\n"
		"weight\n")
{
	int ret = ERROR;

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
