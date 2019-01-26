/*
 * x5-b-ctl.c
 *
 *  Created on: 2018年12月28日
 *      Author: DELL
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

#include "x5_b_a.h"
#include "x5_b_ctl.h"
#include "voip_def.h"
#include "voip_sip.h"
#include "voip_ring.h"
#include "voip_app.h"
#include "voip_api.h"


static int x5b_app_local_address_set(char *address)
{
	int ret = 0;
	struct prefix cp;
	struct interface *ifp = if_lookup_by_name("eth 0/0/1");
	if(ifp)
	{
		ret = str2prefix_ipv4 (address, (struct prefix_ipv4 *)&cp);
		if (ret <= 0)
		{
			return ERROR;
		}
		cp.prefixlen = 24;
		ret = nsm_interface_address_set_api(ifp, &cp, FALSE);
		return  ret;
	}
	return ERROR;
}


int x5b_app_factory_set(x5_b_factory_data_t *data)
{
	//data->local_address;			//����IP��ַ��IP/DHCP��
	int ret = 0;
	ret |= voip_sip_server_set_api(ntohl(data->sip_server), ntohs(data->sip_port), FALSE);
	ret |= voip_sip_proxy_server_set_api(ntohl(data->sip_proxy_server), ntohs(data->sip_proxy_port), FALSE);
	ret |= voip_sip_enable(TRUE, ntohs(data->sip_local_port));
	ret |= voip_sip_local_number_set_api(data->phone_number);

	ret |= voip_local_rtp_set_api(0, ntohs(data->rtp_local_port));
//	ret |= voip_local_rtcp_set_api(0, ntohs(data->rtcp_local_port));

	ret |= x5b_app_local_address_set(inet_address(ntohl(data->local_address)));
	return ret;
}

int x5b_app_start_call(BOOL start, x5_b_room_position_t *room)
{
/*	event_node_t node;
	memset(&node, 0, sizeof(node));
	node.ev_cb = start ? voip_app_ev_start_call:voip_app_ev_stop_call;
	node.pVoid = NULL;
	if(room)
		memcpy(node.sbuf, room, sizeof(x5_b_room_position_t));
	event_node_add(&node);*/
	voip_event_node_register(start ? voip_app_ev_start_call:voip_app_ev_local_stop_call,
			NULL, room, sizeof(x5_b_room_position_t));
	return OK;
}

int x5b_app_call_result_api(int res)
{
	if(x5_b_a_mgt)
		return x5_b_a_call_result_api(x5_b_a_mgt, res);
	return ERROR;
}

int x5b_app_open_result_api(int res)
{
	if(x5_b_a_mgt)
		return x5_b_a_open_result_api(x5_b_a_mgt, res);
	return ERROR;
}


int x5b_app_open_door_api(int res)
{
	if(x5_b_a_mgt)
		return x5_b_a_open_door_api(x5_b_a_mgt, res);
	return ERROR;
}
