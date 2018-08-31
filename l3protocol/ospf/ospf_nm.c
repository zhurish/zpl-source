/* ospf_nm.c - Management Interface Helper routines for OSPFv2 MIB */
#include "ospf.h"
#include "ospf_table.h"
#include "ospf_nm.h"
#include "ospf_api.h"
//#include "errno.h"

#ifdef HAVE_BFD
#include "bfd_nm.h"
#endif
#include "ospf_policy.h"
#ifndef ERROR
#define ERROR -1
#endif

#define RANGE_DESTROY 2
#define RANGE_CREATE 1

extern BOOL ospf_get_cfg_state(void);

//u_int ulOspfRid = 0;
//u_int ospf_config_instance = 0;
#if 0
STATUS uspIfGetFirst(u_int *unit)
{
	long lAddr = 0;
	long lRet = 0;
	u_int ulIfIndex;
	u_int ulNexIfIndex;

	if(unit == NULL)
	{
		return ERR;
	}

	/*	lRet = zebra_if_get_api(0,ZEBRA_IF_GET_FIRST_IFINDEX,&ulNexIfIndex);
	 if(OK != lRet)
	 {
	 return ERR;
	 }*/
	*unit = ulNexIfIndex;

	return OK;

}

//逻辑端口和物理端口转换
STATUS uspIndexTrans(void *instanceRef, u_int cmd, void *var)
//int uspIndexTrans(void* instance ,uint32_t cmd,void *var)
{
	return 0;
}
#endif

STATUS uspL3IfGetApi(u_int instanceRef, u_int cmd, void *var)
{
	int lRtv = ERROR;
	struct interface *ifp = NULL;
	vrf_id_t ulData = 0;
	switch (cmd)
	{
	case SYS_IF_VRFID:
	{
		ifp = if_lookup_by_index(instanceRef);
		if (ifp)
			lRtv = nsm_interface_vrf_get_api(ifp, &ulData);
		*(vrf_id_t*) var = ulData;
		break;
	}
	case SYS_IF_IPV4ADDREX:
	{
		ifp = if_lookup_by_index(instanceRef);
		if (ifp)
		{
			struct connected *connected;
			struct listnode *node;
			struct prefix *p;
			for (ALL_LIST_ELEMENTS_RO(ifp->connected, node, connected))
			{
				p = connected->address;
				if (p)
				{
					if (var)
						os_memcpy(var, p, sizeof(struct prefix));
					lRtv = OK;
				}
			}
		}
		break;
	}
	case SYS_IF_OPER_STATUS: //״̬
	{
		ifp = if_lookup_by_index(instanceRef);
		if (ifp)
		{
			*(int*) var = if_is_running(ifp);
			lRtv = OK;
		}
		break;
	}
	case SYS_IF_HIGHSPEED: //接口速率
	{
		ifp = if_lookup_by_index(instanceRef);
		if (ifp)
			lRtv = nsm_interface_speed_get_api(ifp, var);
		break;
	}
	default:
	{
		break;
	}
	}
	return lRtv;
}

//根据当前接口，获取设备下一个接口
/*STATUS uspIfGetNext(u_int prev,u_int* next)
 {
 int iRtv = 0;
 //iRtv = zebra_if_get_api(prev,ZEBRA_IF_GET_NEXT_IFINDEX,next);
 return iRtv;
 }*/

STATUS ospfGetApi(u_int index, u_int cmd, void *var)
{
	struct ospf_route *p_route = NULL;
	struct ospf_route *p_nextroute = NULL;
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_nextarea = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_redistribute *pstRedisNode = NULL;
	struct ospf_redistribute *pstNextRedisNode = NULL;
	//u_int pid = 0;
	u_int i = 0;
	u_int *lval = (u_int*) var;
	int rc = OK;
	tlv_t *octet = (tlv_t *) var;
	u_int len = 0;
	tOSPF_DISTRIBUTE_PROCESS *pstDisProcess = (tOSPF_DISTRIBUTE_PROCESS *) var;
	ospf_semtake_try();
	//ospf_config_instance = (u_int)index;
	//pid = ospf_config_instance;
	p_process = ospf_process_lookup(&ospf, index);
	if ((NULL == p_process) && (OSPF_GBL_ADMIN != cmd))
	{
		rc = ERROR;
		goto FINISH;
	}
	switch (cmd)
	{
	case OSPF_GBL_ROUTERID:
		*lval = p_process->router_id;
		break;
	case OSPF_GBL_NEW_ROUTERID:
		*lval = p_process->new_routerid;
		break;
	case OSPF_GBL_ADMIN:
		*lval = ((NULL != p_process) && (FALSE == p_process->proto_shutdown)) ?
				ospfvaule_to_nm(TRUE) : ospfvaule_to_nm(FALSE);
		break;

	case OSPF_GBL_VER:
		*lval = OSPF_VERSION;
		break;

	case OSPF_GBL_ABRSTATE:
		*lval = ospfvaule_to_nm(p_process->abr);
		break;

	case OSPF_GBL_ASBRSTATE:
		*lval = ospfvaule_to_nm(p_process->asbr);
		break;

	case OSPF_GBL_EXTLSACOUNT:
		*lval = p_process->extlsa_count;
		break;

	case OSPF_GBL_EXTLSACHECKSUM:
		*lval = p_process->extlsa_checksum;
		break;

	case OSPF_GBL_TOSSUPPORT:
		*lval = ospfvaule_to_nm(p_process->tos_support);
		break;

	case OSPF_GBL_ORIGINNEWLSA:
		*lval = p_process->origin_lsa;
		break;

	case OSPF_GBL_RXDNEWLSA:
		*lval = p_process->rx_lsa;
		break;

	case OSPF_GBL_EXTLSDBLIMIT:
		*lval = p_process->overflow_limit;
		break;

	case OSPF_GBL_MCASTEXTERN:
		*lval = p_process->mcast_support;
		break;

	case OSPF_GBL_OVERFLOWINTERVAL:
		*lval = p_process->overflow_period;
		break;

	case OSPF_GBL_DEMANDEXTERN:
		*lval = ospfvaule_to_nm(p_process->dc_support);
		break;

	case OSPF_GBL_TRAP:
		*lval = p_process->trap_enable;
		break;

	case OSPF_GBL_TRAPERROR:
		*lval = p_process->trap_error;
		break;

	case OSPF_GBL_TRAPTYPE:
		*lval = p_process->trap_packet;
		break;

	case OSPF_GBL_TRAPSRC:
		*lval = p_process->trap_source;
		break;

	case OSPF_GBL_OPAQUE:
		*lval = ospfvaule_to_nm(p_process->opaque);
		break;

	case OSPF_GBL_REDISTRIBUTELOCAL:
		//*lval = ospfvaule_to_nm(BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_local));
		pstDisProcess->uiState =
				ospfvaule_to_nm(
						BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_local));
		break;

	case OSPF_GBL_REDISTRIBUTERIP:
		//*lval = ospfvaule_to_nm(BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_rip));
		pstDisProcess->uiState = FALSE;
		if (TRUE
				== ospfvaule_to_nm(
						BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_rip)))
		{
			for_each_node(&p_process->redistribute_config_table, pstRedisNode, pstNextRedisNode)
			{
				if (pstRedisNode->proto_process == pstDisProcess->uiProcessId)
				{
					pstDisProcess->uiState = TRUE;
					break;
				}
			}
		}
		break;

	case OSPF_GBL_REDISTRIBUTEBGP:
		//*lval = ospfvaule_to_nm(BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_bgp));
		pstDisProcess->uiState = ospfvaule_to_nm(
				BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_bgp));
		break;

	case OSPF_GBL_REDISTRIBUTESTATIC:
		//*lval = ospfvaule_to_nm(BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_netmgmt));
		pstDisProcess->uiState =
				ospfvaule_to_nm(
						BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_netmgmt));
		break;

	case OSPF_GBL_REDISTRIBUTEISIS:
		//*lval = ospfvaule_to_nm(BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_is_is));
		pstDisProcess->uiState = FALSE;
		if (TRUE
				== ospfvaule_to_nm(
						BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_is_is)))
		{
			for_each_node(&p_process->redistribute_config_table, pstRedisNode, pstNextRedisNode)
			{
				if (pstRedisNode->proto_process == pstDisProcess->uiProcessId)
				{
					pstDisProcess->uiState = TRUE;
					break;
				}
			}
		}
		break;

	case OSPF_GBL_REDISTRIBUTEOSPF:
		//*lval = ospfvaule_to_nm(BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_is_is));
		pstDisProcess->uiState = FALSE;
		if (TRUE
				== ospfvaule_to_nm(
						BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_ospf)))
		{
			for_each_node(&p_process->redistribute_config_table, pstRedisNode, pstNextRedisNode)
			{
				if (pstRedisNode->proto_process == pstDisProcess->uiProcessId)
				{
					pstDisProcess->uiState = TRUE;
					break;
				}
			}
		}
		break;

	case OSPF_GBL_REFRATE:
		*lval = p_process->reference_rate;
		break;

	case OSPF_GBL_SPFINTERVAL:
		*lval = p_process->spf_interval;/*unit of 100ms*/
		break;

	case OSPF_GBL_RFC1583COMPATIBILITY:
		*lval = ospfvaule_to_nm(p_process->rfc1583_compatibility);
		break;

	case OSPF_GBL_RESTARTSUPPORT:
		*lval = ospfvaule_to_nm(p_process->restart_enable);
		break;

	case OSPF_GBL_RESTARTINTERVAL:
		*lval = p_process->restart_period;
		break;

	case OSPF_GBL_RESTARTSTRICLSACHECKING:
		*lval = ospfvaule_to_nm(p_process->strictlsa_check);
		break;

	case OSPF_GBL_RESTARTSTATUS:
		*lval = p_process->restart_status;
		break;

	case OSPF_GBL_RESTARTAGE:
		*lval = 0;
		if (OSPF_RESTART_NO_RESTARTING != p_process->restart_status)
		{
			*lval = ospf_timer_remain(&p_process->restart_timer,
					ospf_sys_ticks()) / OSPF_TICK_PER_SECOND;
		}
		break;

	case OSPF_GBL_RESTARTEXITREASON:
		*lval = p_process->restart_exitreason;
		break;
	case OSPF_GBL_RESTARTREASON:
		*lval = p_process->restart_reason;
		break;
	case OSPF_GBL_RESTARTHELPLERENABLE:
		*lval = ospfvaule_to_nm(p_process->restart_helper);
		break;

	case OSPF_GBL_ASLSACOUNT:
		*lval = p_process->extlsa_count;
		break;

	case OSPF_GBL_ASLSACHECKSUM:
		*lval = p_process->extlsa_checksum;
		break;

	case OSPF_GBL_STUBROUTERSUPPORT:
		*lval = ospfvaule_to_nm(p_process->stub_support);
		break;

	case OSPF_GBL_STUBROUTERADVERTISEMENT:
		*lval = (p_process->stub_adv) ? OSPF_STUB_ADV : OSPF_STUB_NOADV;
		break;

	case OSPF_GBL_STUBROUTERADVERTISEMENT_TIME:
		*lval = p_process->stub_router_holdtime;
		break;

	case OSPF_GBL_DISCONTINUITYTIME:
		*lval = ospfvaule_to_nm(p_process->discontinuity_time);
		break;

	case OSPF_GBL_DEBUGGLOBAL:
		*lval = ospf_global_debug ? TRUE : FALSE;
		break;

	case OSPF_GBL_DEBUGPACKET:
		*lval = ospf_global_debug_msg ? TRUE : FALSE;
		break;

	case OSPF_GBL_DEBUGHELLO:
		*lval = ospf_global_debug_hello ? TRUE : FALSE;
		break;

	case OSPF_GBL_DEBUGIF:
		*lval = ospf_global_debug_if ? TRUE : FALSE;
		break;

	case OSPF_GBL_DEBUGNBR:
		*lval = ospf_global_debug_nbr ? TRUE : FALSE;
		break;

	case OSPF_GBL_DEBUGLSA:
		*lval = ospf_global_debug_lsa ? TRUE : FALSE;
		break;

	case OSPF_GBL_DEBUGIPROUTE:
		*lval = ospf_global_debug_route ? TRUE : FALSE;
		break;

	case OSPF_GBL_DEBUGROUTECHANGE:
		*lval = ospf_global_debug_rtm ? TRUE : FALSE;
		break;

	case OSPF_GBL_DEBUGSPF:
		*lval = ospf_global_debug_spf ? TRUE : FALSE;
		break;

	case OSPF_GBL_DEBUGSYN:
		*lval = ospf_global_debug_syn ? TRUE : FALSE;
		break;

	case OSPF_GBL_DEBUGGR:
		*lval = ospf_global_debug_gr ? TRUE : FALSE;
		break;

	case OSPF_GBL_DEBUGFRR:
		*lval = ospf_global_debug_frr ? TRUE : FALSE;
		break;

	case OSPF_GBL_DEBUGERROR:
		*lval = ospf_global_debug_error ? TRUE : FALSE;
		break;

	case OSPF_GBL_DEBUGALL:
		*lval = TRUE;
		for (i = OSPF_DBG_MAIN; OSPF_DBG_MAX > i; i++)
		{
			if (!(ospf.debug & (1 << i)))
			{
				*lval = FALSE;
				break;
			}
		}
		break;
	case OSPF_GBL_SYSLOG_DEBUG_ON:
		*lval = ospf_global_debug_syslog ? TRUE : FALSE;
		break;
	case OSPF_GBL_ROUTENUM:
		*lval = ospf_lstcnt(&p_process->route_table);
		break;

	case OSPF_GBL_NETWORKROUTECOUNT:
		*lval = ospf_lstcnt(&p_process->route_table);
		break;

	case OSPF_GBL_INTRAROUTECOUNT:
		lval[0] = 0;
		for_each_node(&p_process->route_table, p_route, p_nextroute)
		{
			if (OSPF_PATH_INTRA == p_route->path[p_process->current_route].type)
			{
				lval[0]++;
			}
		}
		break;

	case OSPF_GBL_INTERROUTECOUNT:
		lval[0] = 0;
		for_each_node(&p_process->route_table, p_route, p_nextroute)
		{
			if (OSPF_PATH_INTER == p_route->path[p_process->current_route].type)
			{
				lval[0]++;
			}
		}
		break;

	case OSPF_GBL_EXTERNALROUTECOUNT:
		lval[0] = 0;
		for_each_node(&p_process->route_table, p_route, p_nextroute)
		{
			if ((OSPF_PATH_ASE == p_route->path[p_process->current_route].type)
					|| (OSPF_PATH_ASE2
							== p_route->path[p_process->current_route].type))
			{
				lval[0]++;
			}
		}
		break;

	case OSPF_GBL_WILDCARDROUTECOUNT:
		break;

	case OSPF_GBL_ABRROUTECOUNT:
		lval[0] = 0;
		for_each_ospf_area(p_process, p_area, p_nextarea)
		{
			lval[0] += p_area->abr_count;
		}
		break;

	case OSPF_GBL_ASBRROUTECOUNT:
		lval[0] = 0;
		for_each_ospf_area(p_process, p_area, p_nextarea)
		{
			lval[0] += p_area->asbr_count;
		}
		break;

	case OSPF_GBL_NETWORKROUTEPATHCOUNT:
		lval[0] = 0;
		for_each_node(&p_process->route_table, p_route, p_nextroute)
		{
			if (p_route->path[p_process->current_route].p_nexthop)
			{
				lval[0] +=
						p_route->path[p_process->current_route].p_nexthop->count;
			}
		}
		break;

	case OSPF_GBL_INTRAROUTEPATHCOUNT:
		lval[0] = 0;
		for_each_node(&p_process->route_table, p_route, p_nextroute)
		{
			if (OSPF_PATH_INTRA == p_route->path[p_process->current_route].type)
			{
				if (p_route->path[p_process->current_route].p_nexthop)
				{
					lval[0] +=
							p_route->path[p_process->current_route].p_nexthop->count;
				}
			}
		}
		break;

	case OSPF_GBL_INTERROUTEPATHCOUNT:
		lval[0] = 0;
		for_each_node(&p_process->route_table, p_route, p_nextroute)
		{
			if (OSPF_PATH_INTER == p_route->path[p_process->current_route].type)
			{
				if (p_route->path[p_process->current_route].p_nexthop)
				{
					lval[0] +=
							p_route->path[p_process->current_route].p_nexthop->count;
				}
			}
		}
		break;

	case OSPF_GBL_EXTERNALROUTEPATHCOUNT:
		lval[0] = 0;
		for_each_node(&p_process->route_table, p_route, p_nextroute)
		{
			if ((OSPF_PATH_ASE == p_route->path[p_process->current_route].type)
					|| (OSPF_PATH_ASE2
							== p_route->path[p_process->current_route].type))
			{
				if (p_route->path[p_process->current_route].p_nexthop)
				{
					lval[0] +=
							p_route->path[p_process->current_route].p_nexthop->count;
				}
			}
		}
		break;

	case OSPF_GBL_EXTERNALROUTENSSACOUNT:
		lval[0] = 0;
		for_each_node(&p_process->route_table, p_route, p_nextroute)
		{
			if ((OSPF_PATH_ASE == p_route->path[p_process->current_route].type)
					|| (OSPF_PATH_ASE2
							== p_route->path[p_process->current_route].type))
			{
				if (p_route->path[p_process->current_route].p_area->is_nssa)
				{
					if (p_route->path[p_process->current_route].p_nexthop)
					{
						lval[0] +=
								p_route->path[p_process->current_route].p_nexthop->count;
					}
				}
			}
		}
		break;

	case OSPF_GBL_ABRROUTEPATHCOUNT:
		lval[0] = 0;
		for_each_ospf_area(p_process, p_area, p_nextarea)
		{
			for_each_node(&p_area->abr_table, p_route, p_nextroute)
			{
				if (p_route->path[p_process->current_route].p_nexthop)
				{
					lval[0] +=
							p_route->path[p_process->current_route].p_nexthop->count;
				}
			}
		}
		break;

	case OSPF_GBL_ASBRROUTEPATHCOUNT:
		lval[0] = 0;
		for_each_ospf_area(p_process, p_area, p_nextarea)
		{
			for_each_node(&p_area->asbr_table, p_route, p_nextroute)
			{
				if (p_route->path[p_process->current_route].p_nexthop)
				{
					lval[0] +=
							p_route->path[p_process->current_route].p_nexthop->count;
				}
			}
		}
		break;

	case OSPF_GBL_TYPE5COUNT:
		*lval = ospf_lstcnt(&p_process->t5_lstable.list);
		break;

	case OSPF_GBL_TYPE11COUNT:
		*lval = ospf_lstcnt(&p_process->t11_lstable.list);
		break;

	case OSPF_GBL_VRID:
		*lval = p_process->vrid;
		break;

	case OSPF_GBL_DOMAIN:
		rc = ERR;
		break;

	case OSPF_GBL_VALIDHOPS:
		*lval = p_process->valid_hops;
		break;

	case OSPF_GBL_ROUTETAG:
		*lval = p_process->route_tag;
		break;

	case OSPF_GBL_LOOPPREVENT:
		*lval = p_process->vpn_loop_prevent;
		break;

	case OSPF_GBL_DEFROUTEADV:
		*lval = p_process->def_route_adv ?
				OSPF_DEFROUTE_ADV : OSPF_DEFROUTE_NOADV;
		break;

	case OSPF_GBL_DEFROUTECOST:
		*lval = p_process->default_cost;
		break;

	case OSPF_GBL_DEFROUTECOSTTYPE:
		*lval = p_process->default_cost_type;
		break;
	case OSPF_GLB_SWVERSION:

		break;

#ifdef HAVE_BFD
		case OSPF_GBL_BFD:
		*lval = p_process->bfd_enable;
		break;

		case OSPF_GBL_BFD_MINRXINTR:
		*lval = p_process->bfd_minrx_interval;
		break;

		case OSPF_GBL_BFD_MINTXINTR:
		*lval = p_process->bfd_mintx_interval;
		break;

		case OSPF_GBL_BFD_DETMUL:
		*lval = p_process->bfd_detmul;
		break;
#endif

	case OSPF_GBL_MAXECMP:
		*lval = p_process->max_ecmp;
		break;
	case OSPF_GBL_NEXTHOPWEIGHT:
		rc = ospf_nexthopweight_get(p_process, octet);
		break;
	case OSPF_GBL_PACKET_BLOCK:
		*lval = p_process->packet_block;
		break;
	case OSPF_GLB_PREFERENCE:
		*lval = p_process->preference;
		break;
	case OSPF_GLB_ASE_PREFERENCE:
		*lval = p_process->preference_ase;
		break;
	case OSPF_GBL_DESCRIPTION:
	{
		u_char zero[OSPF_DESCRIP_LEN];
		bzero(zero, OSPF_DESCRIP_LEN);
		if (memcmp(p_process->description, zero, OSPF_DESCRIP_LEN) == 0)
		{
			rc = ERROR;
			return rc;
		}
		if (octet->len > strlen(p_process->description))
		{
			octet->len = strlen(p_process->description);
			memcpy(octet->data, p_process->description, octet->len);
		}
	}
		break;
	case OSPF_GLB_MPLS_ID_ADV:
		*lval = p_process->mpls_flag;
		break;
	case OSPF_GLB_MPLS_ID:
		*lval = p_process->mpls_lsrid;
		break;
	case OSPF_GLB_MPLS_ID_COST:
		*lval = p_process->mpls_cost;
		break;
	case OSPF_GLB_MPLS_TE:
		*lval = p_process->mpls_te;
		break;
	case OSPF_GBL_ROUTERID_SETFLG:
		*lval = p_process->routerid_flg;
		break;
	case OSPF_GBL_AREACOUNT:
		*lval = ospf_lstcnt(&p_process->area_table);
		;
		break;
	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS _ospfSetApi(u_int index, u_int cmd, void *var, u_int flag)
{

	struct ospf_process *p_process = NULL;
	struct ospf_iproute route;
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_nextarea = NULL;
	tlv_t *octet = (tlv_t *) var;
	u_int lval = (*(u_int*) var);
	//u_int pid = 0;
	STATUS rc = OK;
	int ret = 0;
	tOSPF_DISTRIBUTE_PROCESS *pstDisProcess = (tOSPF_DISTRIBUTE_PROCESS *) var;
	BOOL cfg_flg = ospf_get_cfg_state();/*上电完成标志*/
	u_int *pBuf = NULL;
	u_int uiPreRouteId = 0;

	/*debug not need take sem*/
	switch (cmd)
	{
	case OSPF_GBL_DEBUGGLOBAL:
		ospf_global_debug_set(OSPF_DBG_MAIN, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGPACKET:
		ospf_global_debug_set(OSPF_DBG_PACKET, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGHELLO:
		ospf_global_debug_set(OSPF_DBG_HELLO, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGIF:
		ospf_global_debug_set(OSPF_DBG_IF, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGNBR:
		ospf_global_debug_set(OSPF_DBG_NBR, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGLSA:
		ospf_global_debug_set(OSPF_DBG_LSA, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGIPROUTE:
		ospf_global_debug_set(OSPF_DBG_ROUTE, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGROUTECHANGE:
		ospf_global_debug_set(OSPF_DBG_RTM, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGSPF:
		ospf_global_debug_set(OSPF_DBG_SPF, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGSYN:
		ospf_global_debug_set(OSPF_DBG_SYN, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGGR:
		ospf_global_debug_set(OSPF_DBG_GR, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGNBRCHANGE:
		ospf_global_debug_set(OSPF_DBG_NBRCHANGE, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGFRR:
		ospf_global_debug_set(OSPF_DBG_FRR, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGERROR:
		ospf_global_debug_set(OSPF_DBG_ERROR, nmvaule_to_ospf(lval));
		return OK;
		break;

	case OSPF_GBL_DEBUGALL:
		ospf.debug = nmvaule_to_ospf(lval) ? 0xffffffff : 0;
		return OK;
		break;
	case OSPF_GLB_DEBUG_ON:
		ospf.debugFd = lval;
		return OK;
	case OSPF_GLB_DEBUG_CLR:
		if (ospf.debugFd == lval)
		{
			ospf.debugFd = -1;
		}
		return OK;
	case OSPF_GBL_DEBUGPOLICY:
		ospf_global_debug_set(OSPF_DBG_POLICY, nmvaule_to_ospf(lval));
		return OK;
	case OSPF_GBL_DEBUGDCN:
		ospf_global_debug_set(OSPF_DBG_DCN, nmvaule_to_ospf(lval));
		return OK;
		break;
	case OSPF_GBL_SYSLOG_DEBUG_ON:
		ospf_global_debug_set(OSPF_DBG_SYSLOG, nmvaule_to_ospf(lval));
		return OK;
	default:
		break;

	}

	ospf_semtake_try();

	//ospf_config_instance = (u_int)index;

	//pid = ospf_config_instance;
	p_process = ospf_process_lookup(&ospf, index);

	if ((NULL == p_process) && (OSPF_GBL_ADMIN != cmd)
			&& (OSPF_GBL_SYNPACKET != cmd))
	{

		rc = ERROR;
		goto FINISH;
	}

	if (p_process && p_process->proto_shutdown)
	{
		ospf_log(ospf_debug, "process is being shutdown");
		rc = ERR;
		goto FINISH;
	}

	switch (cmd)
	{

	case OSPF_GBL_ROUTERID:
		/*new id must not zero*/
		if ((0 == lval) || (p_process->router_id == lval))
		{
			break;
		}

		uiPreRouteId = p_process->router_id;
		/*first set, do not rebuild ospf state*/
		if (0 == p_process->router_id)
		{
			if (OSPF_NO_SET_VAL == lval)
			{
				p_process->router_id = ospf_select_router_id(p_process->vrid);
			}
			else
			{
				p_process->router_id = lval;
				p_process->new_routerid = lval;
			}
			break;
		}
		/*reset process*/
		p_process->old_routerid = p_process->router_id;
		p_process->new_routerid = lval;
#if 0 /*caoyong delete 2017.9.20*/
		ret = ospf_dcn_get_init();
#endif
		//上电加载配置文件或DCN进程,修改route id需重启进程
		if ((uiPreRouteId != 0) && (uiPreRouteId != p_process->router_id)
#ifdef OSPF_DCN
				&& (p_process->process_id == OSPF_DCN_PROCESS)
#endif
				)
		{
			ospf_timer_start(&p_process->id_reset_timer, 1);
		}

		break;

	case OSPF_GBL_ADMIN:
		/*start up or shutdown*/

		if ((NULL == p_process) && (TRUE == lval))
		{
			if (0 == ospf_lstcnt(&ospf.process_table))
			{
				ospf_init_memory();
				//ospf_logx(ospf_debug,"##############################################################%s:%d\r\n",__FUNCTION__,__LINE__);
				/*turn on socket*/
				ospf_sock_enable(ospf.sock[0]);
				//ospf_logx(ospf_debug,"##############################################################%s:%d\r\n",__FUNCTION__,__LINE__);
				//ospf_sock_enable(ospf.rtsock);
				//ospf_set_mcast_to_cpu(1);
				//ospf_logx(ospf_debug,"##############################################################%s:%d\r\n",__FUNCTION__,__LINE__);
			}
#if 1
			/*Limit the process to 100.*/
			if (ospf_lstcnt(&ospf.process_table) >= OSPF_PROCESS_COUNT)
			{
				vty_out_to_all_terminal("Limit the process to 100.");
				rc = ERROR;
				ospf_semgive();
				return rc;
			}
#endif
			p_process = ospf_process_add(&ospf, index);
		}
		else if ((NULL != p_process) && (FALSE == lval))
		{
			p_process->proto_shutdown = TRUE;
			ospf_timer_start(&p_process->delete_timer, 1);

		}

		break;

	case OSPF_GBL_ASBRSTATE:
		p_process->asbr = nmvaule_to_ospf(lval);
		ospf_lstwalkup(&p_process->area_table, ospf_router_lsa_originate);
		break;

	case OSPF_GBL_TOSSUPPORT:
		p_process->tos_support = nmvaule_to_ospf(lval);
		break;

	case OSPF_GBL_EXTLSDBLIMIT:
		p_process->overflow_limit = lval;
		break;

	case OSPF_GBL_OVERFLOWINTERVAL:
		p_process->overflow_period = lval;
		break;

	case OSPF_GBL_MCASTEXTERN:
		p_process->mcast_support = lval;
		break;

	case OSPF_GBL_DEMANDEXTERN:
		p_process->dc_support = nmvaule_to_ospf(lval);
		break;

	case OSPF_GBL_TRAP:
		p_process->trap_enable = lval;
		break;

	case OSPF_GBL_TRAPERROR:
		p_process->trap_error = lval;
		break;

	case OSPF_GBL_TRAPTYPE:
		p_process->trap_packet = lval;
		break;

	case OSPF_GBL_TRAPSRC:
		p_process->trap_source = lval;
		break;

	case OSPF_GBL_OPAQUE:
		p_process->opaque = nmvaule_to_ospf(lval);
		/*修改opaque使能后需重启进程.*/
		ospf_timer_start(&p_process->id_reset_timer, 1);
		break;

	case OSPF_GBL_REDISTRIBUTELOCAL:
		//ospf_redistribute_set(p_process, M2_ipRouteProto_local, nmvaule_to_ospf(lval));
		ospf_redistribute_set(p_process, M2_ipRouteProto_local,
				nmvaule_to_ospf(pstDisProcess->uiState),
				pstDisProcess->uiProcessId);
		break;

	case OSPF_GBL_REDISTRIBUTERIP:
		//ospf_redistribute_set(p_process, M2_ipRouteProto_rip, nmvaule_to_ospf(lval));
		ospf_redistribute_set(p_process, M2_ipRouteProto_rip,
				nmvaule_to_ospf(pstDisProcess->uiState),
				pstDisProcess->uiProcessId);
		break;

	case OSPF_GBL_REDISTRIBUTEBGP:
		//ospf_redistribute_set(p_process, M2_ipRouteProto_bgp, nmvaule_to_ospf(lval));
		ospf_redistribute_set(p_process, M2_ipRouteProto_bgp,
				nmvaule_to_ospf(pstDisProcess->uiState),
				pstDisProcess->uiProcessId);
		break;

	case OSPF_GBL_REDISTRIBUTESTATIC:
		//ospf_redistribute_set(p_process, M2_ipRouteProto_netmgmt, nmvaule_to_ospf(lval));
		ospf_redistribute_set(p_process, M2_ipRouteProto_netmgmt,
				nmvaule_to_ospf(pstDisProcess->uiState),
				pstDisProcess->uiProcessId);
		break;

	case OSPF_GBL_REDISTRIBUTEISIS:
		//ospf_redistribute_set(p_process, M2_ipRouteProto_is_is, nmvaule_to_ospf(lval));
		ospf_redistribute_set(p_process, M2_ipRouteProto_is_is,
				nmvaule_to_ospf(pstDisProcess->uiState),
				pstDisProcess->uiProcessId);
		break;

	case OSPF_GBL_REDISTRIBUTEOSPF:
		ospf_redistribute_set(p_process, M2_ipRouteProto_ospf,
				nmvaule_to_ospf(pstDisProcess->uiState),
				pstDisProcess->uiProcessId);
		break;

	case OSPF_GBL_REFRATE:
		p_process->reference_rate = lval;
		ospf_lstwalkup(&p_process->if_table, ospf_if_state_update);
		break;

	case OSPF_GBL_SPFINTERVAL:
		/*in 100ms*/
		p_process->spf_interval = (!lval) ? OSPF_SPF_INTERVAL : lval;
		break;

	case OSPF_GBL_RESTARTSUPPORT:
		/*reset graceful restart capability*/
		p_process->restart_enable = nmvaule_to_ospf(lval);
		if ((FALSE == p_process->restart_enable) && p_process->in_restart)
		{
			/*if GR is running,exit it*/
			ospf_logx(ospf_debug_gr, "nm set gr,exit graceful restart");

			ospf_restart_finish(p_process, OSPF_RESTART_TOPLOGY_CHANGED);
		}
		break;

	case OSPF_GBL_RESTARTHELPLERENABLE:
		if (TRUE == lval)
		{
			p_process->restart_helper = TRUE;
		}
		else
		{
			p_process->restart_helper = FALSE;
			ospf_restart_helper_finish_all(p_process, OSPF_RESTART_NONE);
		}
		break;

	case OSPF_GBL_RESTARTINTERVAL:
		p_process->restart_period = lval;
		break;

	case OSPF_GBL_RESTARTBEGIN:
		ospf_restart_request(p_process, OSPF_RESTART_PLANNED);
		break;

	case OSPF_GBL_RESTARTREASON:
		p_process->restart_reason = lval;
		break;

	case OSPF_GBL_RFC1583COMPATIBILITY:
		p_process->rfc1583_compatibility = nmvaule_to_ospf(lval);
		if (ospf_lstcnt(&p_process->nexthop_table) != 0)
		{
			ospf_route_calculate_full(p_process);
		}
		break;

	case OSPF_GBL_RESTARTSTRICLSACHECKING:
		p_process->strictlsa_check = nmvaule_to_ospf(lval);
		break;

	case OSPF_GBL_STUBROUTERADVERTISEMENT:
		if (p_process->stub_adv == ((OSPF_STUB_ADV == lval) ? TRUE : FALSE))
		{
			break;
		}
		p_process->stub_adv = (OSPF_STUB_ADV == lval) ? TRUE : FALSE;
		ospf_stub_router_lsa_originate(p_process);
		break;

	case OSPF_GBL_STUBROUTERADVERTISEMENT_TIME:
		if (p_process->stub_router_holdtime == lval)
		{
			break;
		}
		p_process->stub_router_holdtime = lval;
		if (p_process->stub_router_holdtime != 0)
		{
			if ((cfg_flg == FALSE)
					|| (p_process->stub_router_timer.active == FALSE))
			{
				p_process->stub_adv = TRUE;
				ospf_stub_router_lsa_originate(p_process);
				ospf_stimer_start(&p_process->stub_router_timer,
						p_process->stub_router_holdtime);
			}
		}
		else
		{
			p_process->stub_adv = FALSE;
			if (p_process->stub_router_timer.active == TRUE)
			{
				ospf_stub_router_lsa_originate(p_process);
				ospf_timer_stop(&p_process->stub_router_timer);
			}
		}
		break;

	case OSPF_GBL_SPFRUN:
		ospf_spf_request(p_process);
		break;

	case OSPF_GBL_RESET:
		p_process->old_routerid = p_process->router_id;
		if ((p_process->routerid_flg != TRUE)
		/*&& (p_process->process_id != OSPF_DCN_PROCESS)*/)
		{
			p_process->new_routerid = ospf_select_router_id(p_process->vrid);
		}
		ospf_timer_start(&p_process->id_reset_timer, 1);
		break;

	case OSPF_GBL_CLRSTATISTIC:
		ospf_clear_statistic(p_process);
		break;

	case OSPF_GBL_CLRIFSTATISTIC:
		ospf_clear_interface_statistic(lval);
		break;

	case OSPF_GBL_VRID:
#ifdef OSPF_VPN
		if (p_process->vrid != lval)
#else
		if (OSPF_INVALID_VRID == p_process->vrid)
#endif
		{
			p_process->vrid = lval;
			if (p_process->vrid > 0)
			{
				p_process->abr = TRUE;
#ifndef USE_LINUX_OS
				if (ospf.sock[p_process->vrid] <= 0)
				{
					ospf.sock[p_process->vrid] = ospf_socket_init(
							p_process->vrid);
				}
#endif
			}
			//p_process->router_id = ospf_select_router_id(p_process->vrid);
			/*router id should syn to Slave*/
			//ospf_sync_event_set(p_process->syn_flag);
			rc = OK;

		}
#ifdef OSPF_VPN
		else
		{
			rc = OK;                     //ENOTSUP;
		}
#else
		else if (p_process->vrid != lval)
		{
			rc = ERR;
		}
#endif
		break;

	case OSPF_GBL_VALIDHOPS:
		p_process->valid_hops = lval;
		break;

	case OSPF_GBL_ROUTETAG:
		if (!ospf_is_vpn_process(p_process))
		{
			ospf_logx(ospf_debug, "can't set route tag in public network");
			rc = ERR;
			break;
		}

		if (p_process->route_tag != lval)
		{
			p_process->route_tag = lval;
			p_process->import_update = TRUE;
			ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
			ospf_spf_request(p_process);
		}

		break;

	case OSPF_GBL_DOMAIN:
		rc = ERR;
		break;

	case OSPF_GBL_LOOPPREVENT:
		if (!ospf_is_vpn_process(p_process))
		{
			ospf_logx(ospf_debug, "can't set loop prevent in public network");
			rc = ERR;
			break;
		}
		if (p_process->vpn_loop_prevent != nmvaule_to_ospf(lval))
		{
			p_process->vpn_loop_prevent = nmvaule_to_ospf(lval);
			ospf_spf_request(p_process);
		}
		break;

	case OSPF_GBL_DEFROUTEADV:
		ospf_build_external_route(&route, 0, 0, 1, 0);
		if (OSPF_DEFROUTE_ADV == lval)
		{
			p_process->def_route_adv = TRUE;

			route.active = TRUE;
			route.metric = ospf_extmetric(p_process->default_cost,
					p_process->default_cost_type);
			route.p_process = p_process;
			ospf_external_lsa_originate(&route);
		}
		else
		{
			p_process->def_route_adv = FALSE;
			if (NULL == ospf_lstlookup(&p_process->import_table, &route))
			{
				ospf_flush_external_lsa(p_process, 0, 0);
			}
		}
		/*
		 when set defult route adv,reoriginate router lsa for asbr flag changed*/
		ospf_lstwalkup(&p_process->area_table, ospf_router_lsa_originate);

		break;

	case OSPF_GBL_DEFROUTECOST:
		p_process->default_cost = lval;

		break;

	case OSPF_GBL_DEFROUTECOSTTYPE:
		p_process->default_cost_type = lval;
		break;
#ifdef OSPF_DCN
		case OSPF_GBL_DCNLSA:
		p_process->ulDcnFlag = OSPF_DCN_FLAG;
		ospf_dcn_lsa_originate(p_process, octet->data, octet->len);
		flag |= USP_SYNC_OCTETDATA;
		break;
#endif
	case OSPF_GBL_SYNPACKET:
		/*rx sync packet from master*/
		//zhurish flag |= (USP_SYNC_OCTETDATA /*| USP_SYNC_NONBLOCK*/);
		ospf_syn_recv(var);
		break;

#ifdef HAVE_BFD
		case OSPF_GBL_BFD:
		{
			if(p_process->bfd_enable == lval)
			{
				break;
			}
			p_process->bfd_enable = lval;
			ospf_bfd_session_set(p_process,lval);
			break;
		}

		case OSPF_GBL_BFD_MINRXINTR:
		{
			if(p_process->bfd_minrx_interval == lval)
			{
				break;
			}
			p_process->bfd_minrx_interval = lval;
			ospf_bfd_session_config_set(p_process,BFD_MOD_TYPE_MIN_RX,lval);
			break;
		}

		case OSPF_GBL_BFD_MINTXINTR:
		{
			if(p_process->bfd_mintx_interval == lval)
			{
				break;
			}
			p_process->bfd_mintx_interval = lval;
			ospf_bfd_session_config_set(p_process,BFD_MOD_TYPE_MIN_TX,lval);
			break;
		}

		case OSPF_GBL_BFD_DETMUL:
		{
			if(p_process->bfd_detmul == lval)
			{
				break;
			}
			p_process->bfd_detmul = lval;
			ospf_bfd_session_config_set(p_process,BFD_MOD_TYPE_DETECT_MULT,lval);
			break;
		}
#endif
	case OSPF_GBL_MAXECMP:
		p_process->max_ecmp = lval;
		if (ospf_lstcnt(&p_process->nexthop_table) != 0)
		{
			ospf_route_calculate_full(p_process);
			ospf_network_route_change_check(p_process);

		}
		break;

	case OSPF_GBL_NEXTHOPWEIGHT:
		ospf_nexthopweight_add(p_process, octet);
		if (ospf_lstcnt(&p_process->nexthop_table) != 0)
		{
			ospf_route_calculate_full(p_process);
			ospf_network_route_change_check(p_process);
		}
		break;

	case OSPF_GBL_PACKET_BLOCK:
		p_process->packet_block = lval;
		ospf_if_set_packet_block(p_process, lval);
		break;

	case OSPF_GLB_PREFERENCE:
		/* zhurish           pBuf = XMALLOC(MTYPE_MSGQUEUE, 2*sizeof(int));
		 if(pBuf != NULL)
		 {
		 p_process->preference = lval;
		 if(p_process->p_master->work_mode != OSPF_MODE_SLAVE)
		 {
		 pBuf[0] = pid;
		 pBuf[1] = OSPF_GLB_PREFERENCE;
		 ospf_msg_send(MSG_TYPE_IP_CHG, pBuf);
		 }
		 }
		 else*/
	{
		rc = ERR;
	}
		break;
	case OSPF_GLB_ASE_PREFERENCE:
		/* zhurish           pBuf = XMALLOC(MTYPE_MSGQUEUE, 2*sizeof(int));
		 if(pBuf != NULL)
		 {
		 p_process->preference_ase = lval;
		 if(p_process->p_master->work_mode != OSPF_MODE_SLAVE)
		 {
		 pBuf[0] = pid;
		 pBuf[1] = OSPF_GLB_ASE_PREFERENCE;
		 ospf_msg_send(MSG_TYPE_IP_CHG, pBuf);
		 }
		 }
		 else*/
	{
		rc = ERR;
	}
		break;
	case OSPF_GBL_DESCRIPTION:
		if (octet->len > OSPF_DESCRIP_LEN)
		{
			rc = ERROR;
			break;
		}
		memset(p_process->description, 0, OSPF_DESCRIP_LEN);
		memcpy(p_process->description, octet->data, octet->len);
		break;
	case OSPF_GLB_MPLS_ID_ADV:
		p_process->mpls_flag = lval;
#if 0/*因无mpls te命令，暂时使用mpls rsvp-te使能状态代替*/
		iRet = rsvpGetApi(NULL, RSVP_GLOBAL_STATUS, &ulValue);
		if(iRet == OK)
		{
			if (ulValue == TRUE)
			{
				ulValue = TRUE;
			}
			else
			{
				ulValue = FALSE;
			}
			p_process->mpls_te = ulValue;
		}
#endif
		p_process->mpls_te = TRUE;/*暂时取消与mpls rsvp-te的关联*/
		for_each_node(&p_process->area_table, p_area, p_nextarea)
		{
			ospf_router_lsa_originate(p_area);
		}
		break;

	case OSPF_GLB_MPLS_ID:
		p_process->mpls_lsrid = lval;
		break;

	case OSPF_GLB_MPLS_ID_COST:
		p_process->mpls_cost = lval;
		break;

	case OSPF_GLB_MPLS_TE:
		p_process->mpls_te = lval;
		break;

	case OSPF_GBL_ROUTERID_SETFLG:
		p_process->routerid_flg = lval;
		break;

	case OSPF_GBL_ORIGINNEWLSA:
		p_process->origin_lsa = lval;
		break;

	case OSPF_GBL_RXDNEWLSA:
		p_process->rx_lsa = lval;
		break;

	default:
		rc = ERR;
		break;
	}
	FINISH:
	ospf_semgive();
	if (OK == rc)
	{
#if 0
#ifdef USP_MULTIINSTANCE_WANTED
#if OS == VXWORKS || OS == VXWORKS_M
		uspHwOspfProcessSync((void*)index, HW_OSPF_PROCESS_CMDSTART + cmd, var, flag);
#endif
#else
		gHwApiFunction.HwScalarSync(NULL, HW_SYS_OSPFCMDSTART + cmd, var, flag);
#endif
#endif
	}

	return rc;
}

STATUS ospfSetApi(u_int index, u_int cmd, void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iFlg = 0;
	int ret = 0;
	if (var == NULL)
	{
		return ERR;
	}
	if ((cmd == OSPF_GBL_DCNLSA) || (cmd == OSPF_GBL_SYNPACKET) || (cmd == OSPF_GBL_DESCRIPTION) || (cmd == OSPF_GBL_NEXTHOPWEIGHT))
	{
		iFlg |= USP_SYNC_OCTETDATA;
	}

	if((index != OSPF_DCN_PROCESS)&&(index != 0))
	{

	}
#endif
	return _ospfSetApi(index, cmd, var, (USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfSyncApi(u_int index, u_int cmd, void *var)
{
	return _ospfSetApi(index, cmd, var, USP_SYNC_LOCAL);
}

STATUS ospfInstanceGetFirst(u_int *p_id)
{
	struct ospf_process *p_process = NULL;
	STATUS rc = ERROR;
	ospf_semtake_try();
	p_process = ospf_lstfirst(&ospf.process_table);
	if (NULL != p_process)
	{
		rc = OK;
		*p_id = p_process->process_id;
	}
	ospf_semgive();
	return rc;
}

STATUS ospfInstanceGetNext(u_int instanceid, u_int *p_nextid)
{
	struct ospf_process search;
	struct ospf_process *p_process = NULL;
	STATUS rc = ERROR;
	if (instanceid == 0)
	{
		*p_nextid = 0;
		return rc;
	}
	ospf_semtake_try();
	search.process_id = instanceid;
	p_process = ospf_lstgreater(&ospf.process_table, &search);
	if (NULL != p_process)
	{
		rc = OK;
		*p_nextid = p_process->process_id;
	}
	ospf_semgive();
	return rc;
}

STATUS ospfInstanceSetApi(u_int id, u_int cmd, void *var)
{
	struct ospf_process *p_process = NULL;
	STATUS rc = OK;
	u_int lval = *(u_int*) var;
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iFlg = 0;
	int ret = 0;
#endif
	if (OSPF_PROCESS_DEBUG == cmd)
	{
		ospf.forbidden_debug = TRUE;
	}
	ospf_semtake_try();
	p_process = ospf_process_lookup(&ospf, id);
	if (NULL == p_process)
	{
		rc = ERROR;
		goto FINISH;
	}
	if (var == NULL)
	{
		return ERROR;
	}
	switch (cmd)
	{
	case OSPF_PROCESS_DEBUG:
		p_process->debug = *(u_int*) var;
		ospf.process_debug = p_process->debug;
		break;
#ifdef OSPF_FRR
	case OSPF_PROCESS_FRR:
		p_process->frr = nmvaule_to_ospf(lval);
		break;

	case OSPF_PROCESS_FRR_NOLOOP:
		p_process->frr_enable = nmvaule_to_ospf(lval);
		ospf_stimer_start(&p_process->frr_timer, OSPF_FRR_INTERVAL)
		;
		break;
#endif
	default:
		break;
	}
	FINISH:
	ospf_semgive();
	if (OSPF_PROCESS_DEBUG == cmd)
	{
		ospf.forbidden_debug = FALSE;
	}
	return rc;
}

STATUS ospfInstanceGetApi(u_int id, u_int cmd, void *var)
{
	struct ospf_process *p_process = NULL;
	STATUS rc = OK;
	/*before cli set debug,it must get debug first,so here must not delete */
	if (OSPF_PROCESS_DEBUG == cmd)
	{
		ospf.forbidden_debug = TRUE;
	}
	ospf_semtake_try();
	p_process = ospf_process_lookup(&ospf, id);
	if (NULL == p_process)
	{
		rc = ERR;
		goto FINISH;
	}
	switch (cmd)
	{
	case OSPF_PROCESS_DEBUG:
		*(u_int*) var = p_process->debug;
		break;
#ifdef OSPF_FRR
	case OSPF_PROCESS_FRR:
		*(u_int*) var = ospfvaule_to_nm(p_process->frr);
		break;

	case OSPF_PROCESS_FRR_NOLOOP:
		*(u_int*) var = ospfvaule_to_nm(p_process->frr_enable);
		break;
#endif
	default:
		break;
	}
	FINISH:
	ospf_semgive();
	if (OSPF_PROCESS_DEBUG == cmd)
	{
		ospf.forbidden_debug = FALSE;
	}
	return rc;
}

STATUS ospfAreaGetFirst(void *index)
{
	struct ospf_area *p_area = NULL;
	tOSPF_AREA_INDEX *p_index = index;
	STATUS rc = ERROR;
	ospf_semtake_try();
	p_area = ospf_lstfirst(&ospf.nm.area_table);
	if (p_area)
	{
		p_index->process_id = p_area->p_process->process_id;
		p_index->area_id = p_area->id;
		rc = OK;
	}
	ospf_semgive();
	return rc;
}

STATUS ospfAreaGetNext(void *index, void *p_nextindex)
{
	struct ospf_area *p_area = NULL;
	struct ospf_area search;
	struct ospf_process start_process;
	tOSPF_AREA_INDEX *p_index = index;
	tOSPF_AREA_INDEX *p_nextareaindex = p_nextindex;
	STATUS rc = ERROR;
	ospf_semtake_try();
	search.id = p_index->area_id;
	search.p_process = &start_process;
	start_process.process_id = p_index->process_id;
	p_area = ospf_lstgreater(&ospf.nm.area_table, &search);
	if (p_area)
	{
		rc = OK;
		p_nextareaindex->process_id = p_area->p_process->process_id;
		p_nextareaindex->area_id = p_area->id;
	}
	ospf_semgive();
	return rc;
}

static void ospf_nm_extract_area_index(void *index, struct ospf_area *p_area,
		struct ospf_process *p_process)
{
	tOSPF_AREA_INDEX *p_areaindex = (tOSPF_AREA_INDEX *) index;
	p_area->id = p_areaindex->area_id;
	p_process->process_id = p_areaindex->process_id;
	return;
}

STATUS ospfAreaGetApi(void *index, u_int cmd, void *var)
{
	struct ospf_area *p_area = NULL;
	struct ospf_area search;
	struct ospf_process start_process;
	tlv_t *octet = (tlv_t *) var;
	u_int *lval = (u_int*) var;
	STATUS rc = OK;
	u_int i = 0;
	ospf_semtake_try();
	search.p_process = &start_process;
	ospf_nm_extract_area_index(index, &search, &start_process);
	p_area = ospf_lstlookup(&ospf.nm.area_table, &search);
	if (NULL == p_area)
	{
		rc = ERROR;
		goto FINISH;
	}
	switch (cmd)
	{
	case OSPF_AREA_ID:
		*lval = p_area->id;
		break;

	case OSPF_AREA_AUTHTYPE:
		*lval = p_area->authtype;
		break;

	case OSPF_AREA_AUTHDIS:
		*lval = p_area->authdis;
		break;

	case OSPF_AREA_IMPORTASEXTERNAL:
		if (p_area->is_nssa)
		{
			*lval = OSPF_AREA_IMPORT_NSSA;
		}
		else if (p_area->is_stub)
		{
			*lval = OSPF_AREA_IMPORT_STUB;
		}
		else
		{
			*lval = OSPF_AREA_IMPORT_EXTERNAL;
		}
		break;
	case OSPF_AREA_SPFRUNNING:
		*lval = p_area->spf_run;
		break;

	case OSPF_AREA_ABRCOUNT:
		*lval = p_area->abr_count;
		break;

	case OSPF_AREA_ASBRCOUNT:
		*lval = p_area->asbr_count;
		break;

	case OSPF_AREA_LSACOUNT:
		*lval = p_area->lscount;
		break;

	case OSPF_AREA_LSACHECKSUM:
		*lval = p_area->cksum;
		break;

	case OSPF_AREA_SUMMARY:
		*lval = p_area->nosummary ? OSPF_NO_AREASUMMARY : OSPF_SEND_AREASUMMARY;
		break;

	case OSPF_AREA_STATUS:
		*lval = p_area->state;
		break;

	case OSPF_AREA_TRANSLATORROLE:
		*lval = (
				(p_area->nssa_always_translate) ?
						OSPF_TRANSLATOR_ALWAYS : OSPF_TRANSLATOR_CANDIDATE);
		break;

	case OSPF_AREA_TRANSLATORSTATE:
		/*enabled (1),elected (2),disabled (3)*/
		if (p_area->nssa_always_translate)
		{
			*lval = OSPF_TRANSLATOR_STATE_ENABLE;
		}
		else
		{
			*lval = (
					(p_area->nssa_translator) ?
							OSPF_TRANSLATOR_STATE_ELECTED :
							OSPF_TRANSLATOR_STATE_DISABLE);
		}
		break;

	case OSPF_AREA_NSSADEFAULTCOST:
		*lval = p_area->nssa_default_cost;
		break;

	case OSPF_AREA_TRANSLATORSTABLEINTERVAL:
		*lval = p_area->nssa_wait_time;
		break;

	case OSPF_AREA_TRANSLATOREVENT:
		break;

	case OSPF_AREA_TEENABLE:
		*lval = ospfvaule_to_nm(p_area->te_enable);
		break;

	case OSPF_AREA_AUTHKEYID:
		*lval = p_area->keyid;
		break;

	case OSPF_AREA_AUTHKEY:
		octet->len = p_area->keylen;
		if (0 != p_area->keylen)
		{
			memcpy(octet->data, p_area->key, octet->len);
		}
		break;

	case OSPF_AREA_EACHLSACOUNT:
		octet->len = 0;
		lval = (u_int *) var;
		for (i = OSPF_LS_ROUTER; i < (OSPF_LS_TYPE_10 + 1); i++)
		{
			if (p_area->ls_table[i])
			{
				lval[i] = ospf_lstcnt(&p_area->ls_table[i]->list);
				octet->len += 4;
			}
		}
		break;

	case OSPF_AREA_DESCRIPTION:
		if(octet->len < OSPF_DESCRIP_LEN)
		{
			rc = ERROR;
		}
		u_char zero[OSPF_DESCRIP_LEN];
		bzero(zero, OSPF_DESCRIP_LEN);

		if(memcmp(p_area->description, zero, OSPF_DESCRIP_LEN) == 0)
		{
			rc = ERROR;
		}
		else
		{
			octet->len = strlen(p_area->description);
			octet->len = MIN(OSPF_DESCRIP_LEN, octet->len);
			memcpy(octet->data, p_area->description,
						MIN(OSPF_DESCRIP_LEN, octet->len));
		}
		break;

	case OSPF_AREA_CIPHERKEY:
		octet->len = strlen(p_area->cipher_key);
		octet->len = MIN(OSPF_DESCRIP_LEN, octet->len);
		if (0 != octet->len)
		{
			memcpy(octet->data, p_area->cipher_key, octet->len);
		}
		break;
	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS _ospfAreaSetApi(void *index, u_int cmd, void *var, u_int flag)
{
	struct ospf_area *p_area = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;
	struct ospf_iproute route;
	struct ospf_process *p_process = NULL;
	struct ospf_area search;
	struct ospf_process start_process;
	tlv_t *octet = (tlv_t *) var;
	u_int summary_flag = FALSE;
	u_int lval = 0;
	u_int cost = 0;
	u_int cost_type = 0;
	STATUS rc = OK;
	u_int uiLoopbackId = 0;

	ospf_semtake_try();
	ospf_nm_extract_area_index(index, &search, &start_process);
	p_process = ospf_process_lookup(&ospf, start_process.process_id);
	if (NULL == p_process)
	{
		rc = ERROR;
		goto FINISH;
	}
	p_area = ospf_area_lookup(p_process, search.id);
	if ((NULL == p_area) && (OSPF_AREA_STATUS != cmd))
	{
		rc = ERROR;
		goto FINISH;
	}
	if (p_area && ospf_timer_active(&p_area->delete_timer))
	{
		rc = ERROR;
		goto FINISH;
	}
	lval = *(u_int *) var;
	switch (cmd)
	{
	case OSPF_AREA_AUTHTYPE:
		if (lval == p_area->authtype)
		{
			break;
		}
		p_area->authtype = lval;
		p_area->keyid = 0;
		p_area->keylen = 0;
		memset(p_area->key, 0, OSPF_MAX_KEY_LEN);
		/*update all interface's max length*/
		for_each_node(&p_area->if_table, p_if, p_next_if)
		{
			p_if->maxlen = ospf_if_max_len(p_if);
		}
		break;
	case OSPF_AREA_AUTHDIS:
		if (lval == p_area->authdis)
		{
			break;
		}
		p_area->authdis = lval;
		break;

	case OSPF_AREA_IMPORTASEXTERNAL:
		switch (lval)
		{
		case OSPF_AREA_IMPORT_EXTERNAL:
			/*area is normal now,do nothing*/
			if (p_area->is_stub || p_area->is_nssa)
			{
				/*will generate interface down*/
				ospf_area_down(p_area);
				/*clear nssa and stub flag*/
				p_area->is_nssa = FALSE;
				p_area->is_stub = FALSE;
				p_area->nosummary = FALSE;
				p_area->stub_default_cost[0].cost = 0;
				/*will check interface up */
				ospf_area_up(p_area);
			}
			break;

		case OSPF_AREA_IMPORT_STUB:
			if (p_area->is_stub)
			{
				break;
			}
			/*Ignore backbone area and nssa area*/
			if ((p_area->is_nssa) || (OSPF_BACKBONE == p_area->id))
			{
				rc = ERROR;
				ospf_logx(ospf_debug,
						"OSPF Area is nssa or backbone,do not set stub flag");
				break;
			}
			/*will generate interface down*/
			ospf_area_down(p_area);
			p_area->is_stub = TRUE;
			/*will check interface up */
			ospf_area_up(p_area);
			break;

		case OSPF_AREA_IMPORT_NSSA:
			if (p_area->is_nssa)
			{
				break;
			}
			/*backbone or stub area,ignored*/
			if (p_area->is_stub || (OSPF_BACKBONE == p_area->id))
			{
				rc = ERROR;
				ospf_logx(ospf_debug,
						"OSPF Area is stub or backbone,do not set nssa flag");
				break;
			}
			/*will generate interface down*/
			ospf_area_down(p_area);
			p_area->is_nssa = TRUE;
			/*will check interface up */
			ospf_area_up(p_area);
			break;
		default:
			break;
		}
		break;

	case OSPF_AREA_SUMMARY:
		summary_flag = (OSPF_NO_AREASUMMARY == lval) ? TRUE : FALSE;
		if (p_area->nosummary == summary_flag)
		{
			break;
		}
		p_area->nosummary = summary_flag;
		/*for stub area,change to normal area ,and reinit to stub*/
		if (p_area->is_stub || p_area->is_nssa)
		{
			ospf_area_down(p_area);
			/*will check interface up */
			ospf_area_up(p_area);
		}
		break;

	case OSPF_AREA_STATUS:
		switch (lval)
		{
		case SNMP_NOTINSERVICE:
		case SNMP_DESTROY:
			if (NULL != p_area)
			{
				if (FALSE == ospf_area_if_exist(p_area))
				{
					p_area->state = lval;
					ospf_timer_start(&p_area->delete_timer, 2);
				}
				else
				{
					rc = ERROR;
				}
			}
			break;

		case SNMP_ACTIVE:
		case SNMP_CREATEANDGO:
		case SNMP_CREATEANDWAIT:
			if (NULL == p_area)
			{
				p_area = ospf_area_create(p_process, search.id);
				if (NULL == p_area)
				{
					rc = ERROR;
					ospf_semgive();
					return rc;
				}
				p_area->state = lval;
			}
			break;

		default:
			break;
		}
		break;

	case OSPF_AREA_TRANSLATORROLE:
		lval = (OSPF_TRANSLATOR_ROLE_ALWAYS == lval) ? TRUE : FALSE;
		if (lval == p_area->nssa_always_translate)
		{
			break;
		}
		p_area->nssa_always_translate = lval;
		if (p_area->is_nssa)
		{
			/*elect translator in   this area*/
			ospf_nssa_translator_elect(p_area);

			/*regenerate route lsa for each area*/
			ospf_lstwalkup(&p_process->area_table, ospf_router_lsa_originate);
		}
		break;

	case OSPF_AREA_TEENABLE:
		if (p_area->te_enable == lval)
		{
			break;
		}
		p_area->te_enable = lval;

		/*regenerate lsa*/
		ospf_router_te_lsa_originate(p_area);

		for_each_node(&p_area->if_table, p_if, p_next_if)
		{
			/*过滤环回口*/
			if (!ospf_if_is_loopback(p_if->ifnet_uint))
			{
				p_if->te_enable = lval;
			}
			if (p_area->te_enable)
			{
				ospf_link_te_lsa_originate(p_if);
			}
		}
		break;

	case OSPF_AREA_NSSADEFAULTCOST:
		/*get cost and type from input value*/
		cost = lval;
		cost_type = (cost >> 16);
		cost &= 0x0000ffff;

		p_area->nssa_cost_type = cost_type;
		p_area->nssa_default_cost = cost;

		/*originate default nssa lsa if necessary*/
		if (p_process->abr && p_area->is_nssa && (!p_area->nosummary))
		{
			memset(&route, 0, sizeof(route));
			route.metric = ospf_extmetric(cost, cost_type);
			route.active = TRUE;

			ospf_nssa_lsa_originate(p_area, &route);
		}
		break;

	case OSPF_AREA_AUTHKEYID:
		/*only valid for md5 auth*/
		if (OSPF_AUTH_MD5 == p_area->authtype)
		{
			p_area->keyid = lval;
		}
		break;

	case OSPF_AREA_AUTHKEY:
		/*if no auth need,error it*/
		if (OSPF_AUTH_NONE == p_area->authtype)
		{
			rc = ERROR;
			break;
		}

		/*validate length*/
		p_area->keylen = octet->len;
		if (OSPF_AUTH_SIMPLE == p_area->authtype)
		{
			if (OSPF_KEY_LEN < p_area->keylen)
			{
				p_area->keylen = OSPF_KEY_LEN;
			}
		}
		else if (OSPF_MAX_KEY_LEN < p_area->keylen)
		{
			p_area->keylen = OSPF_MAX_KEY_LEN;
		}
		memset(p_area->key, 0, OSPF_MAX_KEY_LEN);
		memcpy(p_area->key, octet->data, octet->len);
		flag |= USP_SYNC_OCTETDATA;
		break;

	case OSPF_AREA_TRANSLATORSTABLEINTERVAL:
		p_area->nssa_wait_time = lval;
		break;

	case OSPF_AREA_DESCRIPTION:
		if (octet->len > OSPF_DESCRIP_LEN)
		{
			rc = ERROR;
			break;
		}
		memset(p_area->description, 0, OSPF_DESCRIP_LEN);
		memcpy(p_area->description, octet->data, octet->len);
		break;

	case OSPF_AREA_CIPHERKEY:
		memset(p_area->cipher_key, 0, OSPF_MAX_KEY_LEN + 4);
		memcpy(p_area->cipher_key, octet->data, octet->len);
		break;

	default:
		rc = ERROR;
		break;
	}

	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		// uspHwOspfAreaSync(index, HW_OSPF_AREA_CMDSTART + cmd, var,  flag);
	}
	return rc;
}

STATUS ospfAreaSetApi(void *p_index, u_int cmd, void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iFlg = 0;
	int ret = 0;
	if((p_index == NULL) || (var == NULL))
	{
		return ERR;
	}

	if((cmd == OSPF_AREA_AUTHKEY)
			|| (cmd == OSPF_AREA_DESCRIPTION)
			|| (cmd == OSPF_AREA_CIPHERKEY))
	{
		iFlg |= USP_SYNC_OCTETDATA;
	}

#endif
	return _ospfAreaSetApi(p_index, cmd, var,
			(USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfAreaSyncApi(void *p_index, u_int cmd, void *var)
{
	return _ospfAreaSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

STATUS ospfStubAreaGetFirst(tOSPF_STUBAREA_INDEX *p_index)
{
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_next = NULL;
	STATUS rc = ERROR;
	ospf_semtake_try();
	for_each_node(&ospf.nm.area_table, p_area, p_next)
	{
		ospf_nm_process_check(p_area->p_process);
		if (p_area->is_stub || p_area->is_nssa)
		{
			rc = OK;
			p_index->process_id = p_area->p_process->process_id;
			p_index->area = p_area->id;
			p_index->tos = 0;
			break;
		}
	}
	ospf_semgive();
	return rc;
}

STATUS ospfStubAreaGetNext(tOSPF_STUBAREA_INDEX *p_index,
		tOSPF_STUBAREA_INDEX *p_nextindex)
{
	struct ospf_area *p_area = NULL;
	struct ospf_area search_area;
	struct ospf_process start_process;
	u_int tos = 0;
	STATUS rc = ERROR;
	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.p_process = &start_process;
	search_area.id = p_index->area;

	p_area = ospf_lstlookup(&ospf.nm.area_table, &search_area);
	if (p_area)
	{
		for (tos = p_index->tos + 1; OSPF_MAX_TOS > tos; tos++)
		{
			if (p_area->stub_default_cost[tos].cost)
			{
				rc = OK;
				goto FINISH;
			}
		}
	}

	for_each_node_greater(&ospf.nm.area_table, p_area, &search_area)
	{
		ospf_nm_process_check(p_area->p_process);

		if (p_area->is_stub)
		{
			tos = 0;
			rc = OK;
			goto FINISH;
		}
	}

	FINISH: if (OK == rc)
	{
		p_nextindex->process_id = p_area->p_process->process_id;
		p_nextindex->area = p_area->id;
		p_nextindex->tos = tos;
	}
	ospf_semgive();
	return rc;
}

STATUS ospfStubAreaGetApi(tOSPF_STUBAREA_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_area *p_area = NULL;
	struct ospf_area search_area;
	struct ospf_process start_process;
	struct ospf_default_cost *p_cost = NULL;
	STATUS rc = OK;
	u_int *lval = (u_int*) var;

	if (OSPF_MAX_TOS <= p_index->tos)
	{
		return ERROR;
	}
	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.p_process = &start_process;
	search_area.id = p_index->area;
	p_area = ospf_lstlookup(&ospf.nm.area_table, &search_area);
	if (NULL == p_area)
	{
		rc = ERROR;
		goto FINISH;
	}
	if (!(p_area->is_stub || p_area->is_nssa))
	{
		printf(
				"Error: Configuring default cost in neither STUB area or NSSA area is prohibited.\n");
		rc = ERROR;
		goto FINISH;
	}
	p_cost = &p_area->stub_default_cost[p_index->tos];
	switch (cmd)
	{
	case OSPF_STUBAREA_ID:
		*lval = p_index->area;
		break;

	case OSPF_STUBAREA_TOS:
		*lval = p_index->tos;
		break;

	case OSPF_STUBAREA_METRICS:
		*lval = p_cost->cost;
		break;

	case OSPF_STUBAREA_STATUS:
		*lval = p_cost->status;
		break;

	case OSPF_STUBAREA_METRICTYPE:
		*lval = p_cost->type;
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS _ospfStubAreaSetApi(tOSPF_STUBAREA_INDEX *p_index, u_int cmd, void *var,
		u_int flag)
{
	struct ospf_area *p_area = NULL;
	struct ospf_area search_area;
	struct ospf_process start_process;
	struct ospf_default_cost *p_cost = NULL;
	STATUS rc = OK;
	u_int lval = *(u_int*) var;
	if (OSPF_MAX_TOS <= p_index->tos)
	{
		return ERROR;
	}
	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.p_process = &start_process;
	search_area.id = p_index->area;
	p_area = ospf_lstlookup(&ospf.nm.area_table, &search_area);
	if (NULL == p_area)
	{
		rc = ERROR;
		goto FINISH;
	}
	if (ospf_timer_active(&p_area->delete_timer))
	{
		rc = ERROR;
		goto FINISH;
	}
	if (!(p_area->is_stub || p_area->is_nssa))
	{
		printf("Error: Configuring default cost in neither STUB area or NSSA area is prohibited.\n");
		rc = ERROR;
		goto FINISH;
	}
	p_cost = &p_area->stub_default_cost[p_index->tos];
	switch (cmd)
	{
	case OSPF_STUBAREA_METRICS:
		if (p_cost->cost != lval)
		{
			p_cost->cost = lval;
#if 0
			if (!p_index->tos && p_area->p_process->abr)
			{
				if (p_area->is_stub || (p_area->is_nssa && p_area->nosummary))
				{
					ospf_originate_summary_default_lsa(p_area, FALSE);
				}
			}
#endif
			if (!p_index->tos && p_area->p_process->abr && p_area->nosummary)
			{
				ospf_originate_summary_default_lsa(p_area, FALSE);
			}

		}
		p_cost->status = (0 != lval) ? SNMP_ACTIVE : SNMP_NOTINSERVICE;
		break;

	case OSPF_STUBAREA_STATUS:
		switch (lval)
		{
		case SNMP_NOTINSERVICE:
		case SNMP_DESTROY:
			p_cost->cost = 1;
			p_cost->status = SNMP_NOTINSERVICE;
			break;

		case SNMP_ACTIVE:
		case SNMP_CREATEANDGO:
		case SNMP_CREATEANDWAIT:
			p_cost->cost = 1;
			p_cost->status = SNMP_ACTIVE;
			break;

		default:
			break;
		}
		break;

	case OSPF_STUBAREA_METRICTYPE:
		p_cost->type = lval;
		break;

	default:
		rc = ERROR;
		break;
	}

	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		//uspHwOspfAreaStubSync(p_index, HW_OSPF_STUBAREA_CMDSTART + cmd, var,  flag);
	}
	return rc;
}

STATUS ospfStubAreaSetApi(tOSPF_STUBAREA_INDEX * p_index, u_int cmd, void * var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iFlg = 0;
	int ret = 0;
	if((p_index == NULL) || (var == NULL))
	{
		return ERR;
	}

#endif
	return _ospfStubAreaSetApi(p_index, cmd, var,
			(USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfStubAreaSyncApi(tOSPF_STUBAREA_INDEX * p_index, u_int cmd,
		void * var)
{
	return _ospfStubAreaSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

STATUS ospfLsdbGetFirst(tOSPF_LSDB_INDEX *p_index)
{
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_lstable *p_table = NULL;
	struct ospf_lstable *p_next = NULL;
	STATUS rc = ERROR;
	ospf_semtake_try();
	for_each_node(&ospf.nm.area_lsa_table, p_table, p_next)
	{
		ospf_nm_process_check(p_table->p_process);
		/*get the first*/
		p_lsa = ospf_lstfirst(&p_table->list);
		if (NULL != p_lsa)
		{
			p_index->process_id = p_table->p_process->process_id;
			p_index->area = p_table->p_area->id;
			p_index->type = p_table->type;
			p_index->id = ntohl(p_lsa->lshdr->id);
			p_index->adv = ntohl(p_lsa->lshdr->adv_id);
			rc = OK;
			break;
		}
	}
	ospf_semgive();
	return rc;
}

STATUS ospfLsdbGetNext(tOSPF_LSDB_INDEX *p_index, tOSPF_LSDB_INDEX *p_nextindex)
{
	struct ospf_area search_area;
	struct ospf_lsa search_lsa;
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_process start_process;
	struct ospf_lstable *p_table = NULL;
	struct ospf_lstable search_table;
	STATUS rc = ERROR;
	ospf_semtake_try();
	memset(&search_table, 0, sizeof(search_table));
	search_table.type = p_index->type;
	search_table.p_area = &search_area;
	search_area.id = p_index->area;
	search_table.p_process = &start_process;
	start_process.process_id = ospf_nm_process_id(p_index);
	search_lsa.lshdr->type = p_index->type;
	search_lsa.lshdr->id = htonl(p_index->id);
	search_lsa.lshdr->adv_id = htonl(p_index->adv);
	/*get expected lsa table*/
	p_table = ospf_lstlookup(&ospf.nm.area_lsa_table, &search_table);
	if (p_table)
	{
		p_lsa = ospf_lstgreater(&p_table->list, &search_lsa);
		if (p_lsa)
		{
			rc = OK;
			goto FINISH;
		}
	}
	/*check next table*/
	for_each_node_greater(&ospf.nm.area_lsa_table, p_table, &search_table)
	{
		ospf_nm_process_check(p_table->p_process);
		p_lsa = ospf_lstfirst(&p_table->list);
		if (p_lsa)
		{
			rc = OK;
			goto FINISH;
		}
	}
	FINISH: if (OK == rc)
	{
		p_nextindex->process_id = p_table->p_process->process_id;
		p_nextindex->area = p_table->p_area->id;
		p_nextindex->type = p_lsa->lshdr->type;
		p_nextindex->id = ntohl(p_lsa->lshdr->id);
		p_nextindex->adv = ntohl(p_lsa->lshdr->adv_id);
	}
	ospf_semgive();
	return rc;
}

STATUS ospfLsdbGetApi(tOSPF_LSDB_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_lstable *p_table = NULL;
	struct ospf_area search_area;
	struct ospf_lsa search_lsa;
	struct ospf_process search_process;
	struct ospf_lstable search_table;
	tlv_t *octet = (tlv_t *) var;
	u_int *lval = (u_int *) var;
	STATUS rc = OK;

	ospf_semtake_try();
	memset(&search_table, 0, sizeof(search_table));
	search_table.type = p_index->type;
	search_table.p_area = &search_area;
	search_area.id = p_index->area;

	search_table.p_process = &search_process;
	search_process.process_id = ospf_nm_process_id(p_index);

	search_lsa.lshdr->type = p_index->type;
	search_lsa.lshdr->id = htonl(p_index->id);
	search_lsa.lshdr->adv_id = htonl(p_index->adv);

	if ((p_index->type == 5) || (p_index->type == 11))
	{
		p_table = ospf_lstlookup(&ospf.nm.as_lsa_table, &search_table);
	}
	else
	{
		p_table = ospf_lstlookup(&ospf.nm.area_lsa_table, &search_table);

	}

	if (p_table)
	{
		p_lsa = ospf_lstlookup(&p_table->list, &search_lsa);
	}
	if (NULL == p_lsa)
	{
		ospf_logx(ospf_debug, "%d\r\n", __LINE__);
		rc = ERROR;
		goto FINISH;
	}
	switch (cmd)
	{
	case OSPF_LSDB_AREAID:
		*lval = p_index->area;
		break;

	case OSPF_LSDB_TYPE:
		*lval = p_index->type;
		break;

	case OSPF_LSDB_LSID:
		*lval = p_index->id;
		break;

	case OSPF_LSDB_ROUTERID:
		*lval = p_index->adv;
		break;

	case OSPF_LSDB_SEQUENCE:
		*lval = (unsigned int) (ntohl(p_lsa->lshdr->seqnum));
		break;

	case OSPF_LSDB_AGE:
		ospf_lsa_age_update(p_lsa);
		*lval = (unsigned int) ntohs(p_lsa->lshdr->age);
		break;

	case OSPF_LSDB_CHECKSUM:
		*lval = (unsigned int) (ntohs(p_lsa->lshdr->checksum));
		break;

	case OSPF_LSDB_ADVERTISEMENT:
		octet->len = (ntohs(p_lsa->lshdr->len));
		memcpy(octet->data, p_lsa->lshdr, octet->len);
		break;

	case OSPF_LSDB_LEN:
		*lval = (unsigned int) (ntohs(p_lsa->lshdr->len));
		break;

	case OSPF_LSDB_METRIC:
		if (p_index->type == OSPF_LS_ROUTER)
		{
			struct ospf_router_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohs(p_router->link[0].tos0_metric);
		}
		else if ((p_index->type == OSPF_LS_SUMMARY_NETWORK)
				|| (p_index->type == OSPF_LS_SUMMARY_ASBR))
		{
			struct ospf_summary_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->metric) & 0X00FFFFFF;
		}
		else if (p_index->type == OSPF_LS_TYPE_7)
		{
			struct ospf_nssa_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->metric) & 0X00FFFFFF;
		}
		else
		{
			*lval = 0;
		}
		break;

	case OSPF_LSDB_OPTIONS:
		*lval = p_lsa->lshdr->option;
		break;

	case OSPF_LSDB_LINKID:
		if (p_index->type == OSPF_LS_ROUTER)
		{
			struct ospf_router_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = htonl(p_router->link[0].id);
		}
		else
		{
			*lval = 0;
		}
		break;

	case OSPF_LSDB_ADVID:
		if (p_index->type == OSPF_LS_ROUTER)
		{
			struct ospf_router_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = htonl(p_router->link[0].data);
		}
		else
		{
			*lval = 0;
		}
		break;

	case OSPF_LSDB_LINKTYPE:
		if (p_index->type == OSPF_LS_ROUTER)
		{
			struct ospf_router_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = p_router->link[0].type;
		}
		else
		{
			*lval = 0;
		}
		break;

	case OSPF_LSDB_LINKCOUNT:
		if (p_index->type == OSPF_LS_ROUTER)
		{
			struct ospf_router_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohs(p_router->link_count);
		}
		else
		{
			*lval = 0;
		}
		break;

	case OSPF_LSDB_NETROUTER:
		if (p_index->type == OSPF_LS_NETWORK)
		{
			struct ospf_network_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->router[1]);
		}
		else
		{
			*lval = 0;
		}
		break;

	case OSPF_LSDB_TOS:
		if ((p_index->type == OSPF_LS_SUMMARY_NETWORK)
				|| (p_index->type == OSPF_LS_SUMMARY_ASBR))
		{
			struct ospf_summary_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->metric) & 0XFF000000;
		}
		else if (p_index->type == OSPF_LS_TYPE_7)
		{
			struct ospf_nssa_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->metric) & 0X7F000000;
		}
		break;

	case OSPF_LSDB_TAG:
		if (p_index->type == OSPF_LS_TYPE_7)
		{
			struct ospf_nssa_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->tag);
		}
		else
		{
			*lval = 0;
		}
		break;

	case OSPF_LSDB_FADADDR:
		if (p_index->type == OSPF_LS_TYPE_7)
		{
			struct ospf_nssa_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->fwdaddr);
		}
		else
		{
			*lval = 0;
		}
		break;

	case OSPF_LSDB_MASK:
		if (p_index->type == OSPF_LS_TYPE_7)
		{
			struct ospf_nssa_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->mask);
		}
		else if (p_index->type == OSPF_LS_NETWORK)
		{
			struct ospf_network_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->mask);
		}
		else if (p_index->type == OSPF_LS_SUMMARY_NETWORK
				|| p_index->type == OSPF_LS_SUMMARY_NETWORK)
		{
			struct ospf_summary_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->mask);
		}
		else
		{
			*lval = 0;
		}
		break;

	case OSPF_LSDB_ETYPE:
		if (p_index->type == OSPF_LS_TYPE_7)
		{
			struct ospf_nssa_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->metric) & 0x10000000;
		}
		else
		{
			*lval = 0;
		}
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS ospfExtLsdbGetFirst(tOSPF_LSDB_INDEX *p_index)
{
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_lstable *p_lstable = NULL;
	struct ospf_lstable *p_nextlstable = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();

	for_each_node(&ospf.nm.as_lsa_table, p_lstable, p_nextlstable)
	{
		ospf_nm_process_check(p_lstable->p_process);

		if (OSPF_LS_AS_EXTERNAL != p_lstable->type)
		{
			continue;
		}

		/*get the first*/
		p_lsa = ospf_lstfirst(&p_lstable->list);
		if (NULL != p_lsa)
		{
			p_index->process_id = p_lstable->p_process->process_id;
			p_index->type = p_lstable->type;
			p_index->id = ntohl(p_lsa->lshdr->id);
			p_index->adv = ntohl(p_lsa->lshdr->adv_id);
			rc = OK;
			break;
		}
	}
	ospf_semgive();
	return rc;
}

STATUS ospfExtLsdbGetNext(tOSPF_LSDB_INDEX *p_index,
		tOSPF_LSDB_INDEX *p_nextindex)
{
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_lsa search_lsa;
	struct ospf_process start_process;
	struct ospf_lstable lstable;
	struct ospf_lstable *p_table;
	STATUS rc = ERROR;

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);
	lstable.p_process = &start_process;
	lstable.type = OSPF_LS_AS_EXTERNAL;

	p_table = ospf_lstlookup(&ospf.nm.as_lsa_table, &lstable);
	if (p_table)
	{
		search_lsa.lshdr->type = OSPF_LS_AS_EXTERNAL;
		search_lsa.lshdr->id = htonl(p_index->id);
		search_lsa.lshdr->adv_id = htonl(p_index->adv);
		p_lsa = ospf_lstgreater(&p_table->list, &search_lsa);
		if (p_lsa)
		{
			rc = OK;
			goto FINISH;
		}
	}

	/*start from next table*/
	for_each_node_greater(&ospf.nm.as_lsa_table, p_table, &lstable)
	{
		ospf_nm_process_check(p_table->p_process);
		if (OSPF_LS_AS_EXTERNAL != p_table->type)
		{
			continue;
		}

		/*get the first*/
		p_lsa = ospf_lstfirst(&p_table->list);
		if (NULL != p_lsa)
		{
			rc = OK;
			goto FINISH;
		}
	}

	FINISH: if (OK == rc)
	{
		p_nextindex->process_id = p_table->p_process->process_id;
		p_nextindex->type = p_table->type;
		p_nextindex->id = ntohl(p_lsa->lshdr->id);
		p_nextindex->adv = ntohl(p_lsa->lshdr->adv_id);
	}

	ospf_semgive();
	return rc;
}

STATUS ospfExtLsdbGetApi(tOSPF_LSDB_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_lstable *p_table;
	struct ospf_lsa search_lsa;
	struct ospf_process search_process;
	struct ospf_lstable lstable;
	tlv_t *octet = (tlv_t *) var;
	u_int *lval = (u_int *) var;
	STATUS rc = OK;

	ospf_semtake_try();
	search_process.process_id = ospf_nm_process_id(p_index);
	lstable.p_process = &search_process;
	lstable.type = OSPF_LS_AS_EXTERNAL;
	p_table = ospf_lstlookup(&ospf.nm.as_lsa_table, &lstable);
	if (p_table)
	{
		search_lsa.lshdr->type = OSPF_LS_AS_EXTERNAL;
		search_lsa.lshdr->id = htonl(p_index->id);
		search_lsa.lshdr->adv_id = htonl(p_index->adv);
		p_lsa = ospf_lstlookup(&p_table->list, &search_lsa);
	}
	if (NULL == p_lsa)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_EXTLSDB_TYPE:
		*lval = OSPF_LS_AS_EXTERNAL;
		break;

	case OSPF_EXTLSDB_LSID:
		*lval = p_index->id;
		break;

	case OSPF_EXTLSDB_ROUTERID:
		*lval = p_index->adv;
		break;

	case OSPF_EXTLSDB_SEQUENCE:
		*lval = (unsigned int) (ntohl(p_lsa->lshdr->seqnum));
		break;

	case OSPF_EXTLSDB_AGE:
		ospf_lsa_age_update(p_lsa);
		*lval = (unsigned int) ntohs(p_lsa->lshdr->age);
		break;

	case OSPF_EXTLSDB_CHECKSUM:
		*lval = (unsigned int) (ntohs(p_lsa->lshdr->checksum));
		break;

	case OSPF_EXTLSDB_ADVERTISEMENT:
		if (octet->len > ntohs(p_lsa->lshdr->len))
		{
			octet->len = (ntohs(p_lsa->lshdr->len));
		}
		memcpy(octet->data, p_lsa->lshdr, octet->len);
		break;

	case OSPF_EXTLSDB_METRIC:
		if (p_index->type == OSPF_LS_AS_EXTERNAL)
		{
			struct ospf_external_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->metric) & 0X00FFFFFF;
		}
		else
		{
			*lval = 0;
		}
		break;

	case OSPF_EXTLSDB_OPTIONS:
		*lval = p_lsa->lshdr->option;
		break;

	case OSPF_EXTLSDB_NETMASK:
		if (p_index->type == OSPF_LS_AS_EXTERNAL)
		{
			struct ospf_external_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->mask);
		}
		else
		{
			*lval = 0;
		}
		break;

	case OSPF_EXTLSDB_TOS:
		if (p_index->type == OSPF_LS_AS_EXTERNAL)
		{
			struct ospf_external_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->metric) & 0x7F000000;
		}
		else
		{
			*lval = 0;
		}
		break;

	case OSPF_EXTLSDB_TAG:
		if (p_index->type == OSPF_LS_AS_EXTERNAL)
		{
			struct ospf_external_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->tag);
		}
		else
		{
			*lval = 0;
		}
		break;

	case OSPF_EXTLSDB_FWADDR:
		if (p_index->type == OSPF_LS_AS_EXTERNAL)
		{
			struct ospf_external_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->fwdaddr);
		}
		else
		{
			*lval = 0;
		}
		break;
	case OSPF_EXTLSDB_ETYPE:
		if (p_index->type == OSPF_LS_AS_EXTERNAL)
		{
			struct ospf_external_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
			*lval = ntohl(p_router->metric) & 0x10000000;
		}
		else
		{
			*lval = 0;
		}
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

/*OSPF Link State Database, AS-scope*/
STATUS ospfASLsdbGetFirst(tOSPF_ASLSDB_INDEX *p_index)
{
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_lstable *p_lstable = NULL;
	struct ospf_lstable *p_next_table = NULL;
	STATUS rc = ERROR;
	ospf_semtake_try();
	for_each_node(&ospf.nm.as_lsa_table, p_lstable, p_next_table)
	{
		ospf_nm_process_check(p_lstable->p_process);
		/*get the first*/
		p_lsa = ospf_lstfirst(&p_lstable->list);
		if (NULL != p_lsa)
		{
			p_index->process_id = p_lstable->p_process->process_id;
			p_index->type = p_lstable->type;
			p_index->id = ntohl(p_lsa->lshdr->id);
			p_index->adv = ntohl(p_lsa->lshdr->adv_id);
			rc = OK;
			break;
		}
	}
	ospf_semgive();
	return rc;
}

STATUS ospfASLsdbGetNext(tOSPF_ASLSDB_INDEX *p_index,
		tOSPF_ASLSDB_INDEX *p_nextindex)
{
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_lsa search_lsa;
	struct ospf_process start_process;
	struct ospf_lstable *p_table = NULL;
	struct ospf_lstable lstable;
	STATUS rc = ERROR;

	search_lsa.lshdr->type = p_index->type;
	search_lsa.lshdr->id = htonl(p_index->id);
	search_lsa.lshdr->adv_id = htonl(p_index->adv);

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	lstable.p_process = &start_process;
	lstable.type = p_index->type;
	p_table = ospf_lstlookup(&ospf.nm.as_lsa_table, &lstable);
	if (p_table)
	{
		p_lsa = ospf_lstgreater(&p_table->list, &search_lsa);
		if (p_lsa)
		{
			rc = OK;
			goto FINISH;
		}
	}
	/*start from next table*/
	for_each_node_greater(&ospf.nm.as_lsa_table, p_table, &lstable)
	{
		ospf_nm_process_check(p_table->p_process);

		/*get the first*/
		p_lsa = ospf_lstfirst(&p_table->list);
		if (NULL != p_lsa)
		{
			rc = OK;
			goto FINISH;
		}
	}

	FINISH: if (OK == rc)
	{
		p_nextindex->process_id = p_table->p_process->process_id;
		p_nextindex->id = ntohl(p_lsa->lshdr->id);
		p_nextindex->adv = ntohl(p_lsa->lshdr->adv_id);
		p_nextindex->type = p_lsa->lshdr->type;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfASLsdbGetApi(tOSPF_ASLSDB_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_lsa search_lsa;
	struct ospf_process start_process;
	struct ospf_lstable *p_table = NULL;
	struct ospf_lstable lstable;
	tlv_t *octet = (tlv_t *) var;
	u_int *lval = (u_int *) var;
	STATUS rc = OK;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	lstable.p_process = &start_process;
	lstable.type = p_index->type;
	p_table = ospf_lstlookup(&ospf.nm.as_lsa_table, &lstable);
	if (p_table)
	{
		search_lsa.lshdr->type = p_index->type;
		search_lsa.lshdr->id = htonl(p_index->id);
		search_lsa.lshdr->adv_id = htonl(p_index->adv);
		p_lsa = ospf_lstlookup(&p_table->list, &search_lsa);
	}
	if (NULL == p_lsa)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_ASLSDB_TYPE:
		*lval = p_index->type;
		break;

	case OSPF_ASLSDB_LSID:
		*lval = p_index->id;
		break;

	case OSPF_ASLSDB_ROUTERID:
		*lval = p_index->adv;
		break;

	case OSPF_ASLSDB_SEQUENCE:
		*lval = (unsigned int) (ntohl(p_lsa->lshdr->seqnum));
		break;

	case OSPF_ASLSDB_AGE:
		ospf_lsa_age_update(p_lsa);
		*lval = (unsigned int) ntohs(p_lsa->lshdr->age);
		break;

	case OSPF_ASLSDB_CHECKSUM:
		*lval = (unsigned int) (ntohs(p_lsa->lshdr->checksum));
		break;

	case OSPF_ASLSDB_ADVERTISEMENT:
		if (octet->len > ntohs(p_lsa->lshdr->len))
		{
			octet->len = (ntohs(p_lsa->lshdr->len));
		}
		memcpy(octet->data, p_lsa->lshdr, octet->len);
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

/*OSPF Link State Database, link-local for non-virtual links*/
STATUS ospfIfLsdbGetFirst(tOSPF_IFLSDB_INDEX *p_index)
{
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_lstable *p_table = NULL;
	struct ospf_lstable *p_next_table = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.if_lsa_table, p_table, p_next_table)
	{
		ospf_nm_process_check(p_table->p_process);
		p_lsa = ospf_lstfirst(&p_table->list);
		if (NULL != p_lsa)
		{
			p_index->process_id = p_table->p_process->process_id;
			p_index->type = p_table->type;
			p_index->id = ntohl(p_lsa->lshdr->id);
			p_index->adv = ntohl(p_lsa->lshdr->adv_id);
			p_index->ifaddr = p_lsa->p_lstable->p_if->addr;
			p_index->addrlessif = 0;
			rc = OK;
			break;
		}
	}
	ospf_semgive();
	return rc;
}

STATUS ospfIfLsdbGetNext(tOSPF_IFLSDB_INDEX *p_index,
		tOSPF_IFLSDB_INDEX *p_nextindex)
{
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_lsa search_lsa;
	struct ospf_if search_if;
	struct ospf_process start_process;
	struct ospf_lstable *p_table = NULL;
	struct ospf_lstable search_table;
	STATUS rc = ERROR;

	ospf_semtake_try();
	memset(&search_table, 0, sizeof(search_table));
	start_process.process_id = ospf_nm_process_id(p_index);
	search_table.p_process = &start_process;

	search_table.type = p_index->type;
	search_table.p_if = &search_if;

	search_if.type = OSPF_IFT_BCAST;
	search_if.addr = p_index->ifaddr;

	search_lsa.lshdr->type = OSPF_LS_TYPE_9;
	search_lsa.lshdr->id = htonl(p_index->id);
	search_lsa.lshdr->adv_id = htonl(p_index->adv);

	/*search table*/
	p_table = ospf_lstlookup(&ospf.nm.if_lsa_table, &search_table);
	if (p_table)
	{
		p_lsa = ospf_lstgreater(&p_table->list, &search_lsa);
		if (p_lsa)
		{
			rc = OK;
			goto FINISH;
		}
	}

	/*next table*/
	for_each_node_greater(&ospf.nm.if_lsa_table, p_table, &search_table)
	{
		ospf_nm_process_check(p_table->p_process);

		p_lsa = ospf_lstfirst(&p_table->list);
		if (p_lsa)
		{
			rc = OK;
			goto FINISH;
		}
	}

	FINISH: if (OK == rc)
	{
		p_nextindex->process_id = p_table->p_process->process_id;
		p_nextindex->type = OSPF_LS_TYPE_9;
		p_nextindex->id = ntohl(p_lsa->lshdr->id);
		p_nextindex->adv = ntohl(p_lsa->lshdr->adv_id);
		p_nextindex->ifaddr = p_table->p_if->addr;
		p_nextindex->addrlessif = 0;
	}
	ospf_semgive();
	return rc;
}

STATUS ospfIfLsdbGetApi(tOSPF_IFLSDB_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_lsa search_lsa;
	struct ospf_if search_if;
	struct ospf_process start_process;
	struct ospf_lstable search_table;
	struct ospf_lstable *p_table = NULL;
	struct ospf_lsa *p_lsa = NULL;
	tlv_t *octet = (tlv_t *) var;
	STATUS rc = OK;
	u_int *lval = (u_int *) var;

	ospf_semtake_try();

	memset(&search_table, 0, sizeof(search_table));
	start_process.process_id = ospf_nm_process_id(p_index);
	search_table.p_process = &start_process;

	search_table.type = p_index->type;
	search_table.p_if = &search_if;

	search_if.type = OSPF_IFT_BCAST;
	search_if.addr = p_index->ifaddr;
	p_table = ospf_lstlookup(&ospf.nm.if_lsa_table, &search_table);
	if (p_table)
	{
		search_lsa.lshdr->type = p_index->type;
		search_lsa.lshdr->id = htonl(p_index->id);
		search_lsa.lshdr->adv_id = htonl(p_index->adv);
		p_lsa = ospf_lstlookup(&p_table->list, &search_lsa);
	}
	if (NULL == p_lsa)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_IFLSDB_IPADDR:
		*lval = p_index->ifaddr;
		break;

	case OSPF_IFLSDB_IPADDRLESSIF:
		*lval = p_index->addrlessif;
		break;

	case OSPF_IFLSDB_TYPE:
		*lval = p_index->type;
		break;

	case OSPF_IFLSDB_LSID:
		*lval = p_index->id;
		break;

	case OSPF_IFLSDB_ROUTERID:
		*lval = p_index->adv;
		break;

	case OSPF_IFLSDB_SEQUENCE:
		*lval = (unsigned int) (ntohl(p_lsa->lshdr->seqnum));
		break;

	case OSPF_IFLSDB_AGE:
		ospf_lsa_age_update(p_lsa);
		*lval = (unsigned int) ntohs(p_lsa->lshdr->age);
		break;

	case OSPF_IFLSDB_CHECKSUM:
		*lval = (unsigned int) (ntohs(p_lsa->lshdr->checksum));
		break;

	case OSPF_IFLSDB_ADVERTISEMENT:
		if (octet->len > ntohs(p_lsa->lshdr->len))
		{
			octet->len = (ntohs(p_lsa->lshdr->len));
		}
		memcpy(octet->data, p_lsa->lshdr, octet->len);
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

/*OSPF Link State Database, link-local for virtual Links*/
STATUS ospfVIfLsdbGetFirst(tOSPF_VIFLSDB_INDEX *p_index)
{
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_lstable *p_table = NULL;
	struct ospf_lstable *p_next_table = NULL;
	STATUS rc = ERR;
	ospf_semtake_try();
	for_each_node(&ospf.nm.vif_lsa_table, p_table, p_next_table)
	{
		ospf_nm_process_check(p_table->p_process);
		p_lsa = ospf_lstfirst(&p_table->list);
		if (p_lsa)
		{
			p_index->process_id = p_table->p_process->process_id;
			p_index->nbr = p_table->p_if->nbr;
			p_index->area = p_table->p_if->p_transit_area->id;
			p_index->type = p_lsa->lshdr->type;
			p_index->id = ntohl(p_lsa->lshdr->id);
			p_index->adv = ntohl(p_lsa->lshdr->adv_id);
			rc = OK;
			break;
		}
	}

	ospf_semgive();

	return rc;
}

STATUS ospfVIfLsdbGetNext(tOSPF_VIFLSDB_INDEX *p_index,
		tOSPF_VIFLSDB_INDEX *p_nextindex)
{
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_lsa search_lsa;
	struct ospf_if search_if;
	struct ospf_area transit;
	struct ospf_process start_process;
	struct ospf_lstable *p_table = NULL;
	struct ospf_lstable search_table;
	STATUS rc = ERR;

	ospf_semtake_try();
	memset(&search_table, 0, sizeof(search_table));

	search_table.type = p_index->type;

	search_table.p_if = &search_if;
	search_if.type = OSPF_IFT_VLINK;
	search_if.nbr = p_index->nbr;
	search_if.p_transit_area = &transit;
	transit.id = p_index->area;

	search_table.p_process = &start_process;
	start_process.process_id = ospf_nm_process_id(p_index);

	search_lsa.lshdr->type = OSPF_LS_TYPE_9;
	search_lsa.lshdr->id = htonl(p_index->id);
	search_lsa.lshdr->adv_id = htonl(p_index->adv);

	p_table = ospf_lstlookup(&ospf.nm.vif_lsa_table, &search_table);
	if (p_table)
	{
		p_lsa = ospf_lstgreater(&p_table->list, &search_lsa);
		if (p_lsa)
		{
			rc = OK;
			goto FINISH;
		}
	}

	/*next table*/
	for_each_node_greater(&ospf.nm.vif_lsa_table, p_table, &search_table)
	{
		ospf_nm_process_check(p_table->p_process);
		p_lsa = ospf_lstfirst(&p_table->list);
		if (p_lsa)
		{
			rc = OK;
			goto FINISH;
		}
	}
	FINISH: if (OK == rc)
	{
		p_nextindex->process_id = p_table->p_process->process_id;
		p_nextindex->nbr = p_table->p_if->nbr;
		p_nextindex->type = p_lsa->lshdr->type;
		p_nextindex->id = ntohl(p_lsa->lshdr->id);
		p_nextindex->adv = ntohl(p_lsa->lshdr->adv_id);
		p_nextindex->area = p_table->p_if->p_transit_area->id;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfVIfLsdbGetApi(tOSPF_VIFLSDB_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_lsa *p_lsa = NULL;
	struct ospf_lsa search_lsa;
	struct ospf_if search_if;
	struct ospf_area transit;
	struct ospf_process start_process;
	struct ospf_lstable search_table;
	struct ospf_lstable *p_table = NULL;
	tlv_t *octet = (tlv_t *) var;
	STATUS rc = OK;
	u_int *lval = (u_int *) var;

	ospf_semtake_try();

	memset(&search_table, 0, sizeof(search_table));

	search_table.type = p_index->type;

	search_table.p_if = &search_if;
	search_if.type = OSPF_IFT_VLINK;
	search_if.nbr = p_index->nbr;
	search_if.p_transit_area = &transit;
	transit.id = p_index->area;

	search_table.p_process = &start_process;
	start_process.process_id = ospf_nm_process_id(p_index);

	p_table = ospf_lstlookup(&ospf.nm.vif_lsa_table, &search_table);
	if (p_table)
	{
		search_lsa.lshdr->type = OSPF_LS_TYPE_9;
		search_lsa.lshdr->id = htonl(p_index->id);
		search_lsa.lshdr->adv_id = htonl(p_index->adv);

		p_lsa = ospf_lstlookup(&p_table->list, &search_lsa);
	}
	if (NULL == p_lsa)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_VIFLSDB_TRANSITAREA:
		*lval = p_index->area;
		break;

	case OSPF_VIFLSDB_NEIGHBOR:
		*lval = p_index->nbr;
		break;

	case OSPF_VIFLSDB_TYPE:
		*lval = p_index->type;
		break;

	case OSPF_VIFLSDB_LSID:
		*lval = p_index->id;
		break;

	case OSPF_VIFLSDB_ROUTERID:
		*lval = p_index->adv;
		break;

	case OSPF_VIFLSDB_SEQUENCE:
		*lval = (unsigned int) (ntohl(p_lsa->lshdr->seqnum));
		break;

	case OSPF_VIFLSDB_AGE:
		ospf_lsa_age_update(p_lsa);
		*lval = (unsigned int) ntohs(p_lsa->lshdr->age);
		break;

	case OSPF_VIFLSDB_CHECKSUM:
		*lval = (unsigned int) (ntohs(p_lsa->lshdr->checksum));
		break;

	case OSPF_VIFLSDB_ADVERTISEMENT:
		if (octet->len > ntohs(p_lsa->lshdr->len))
		{
			octet->len = (ntohs(p_lsa->lshdr->len));
		}
		memcpy(octet->data, p_lsa->lshdr, octet->len);
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

/*OSPF Area LSA Counter Table*/
STATUS ospfAreaLsaCountGetFirst(tOSPF_LSDBCOUNT_INDEX *p_index)
{
	struct ospf_lstable *p_table = NULL;
	struct ospf_lstable *p_ntable = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.area_lsa_table, p_table, p_ntable)
	{
		ospf_nm_process_check(p_table->p_process);

		p_index->process_id = p_table->p_process->process_id;
		p_index->area = p_table->p_area->id;
		p_index->type = p_table->type;
		rc = OK;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfAreaLsaCountGetNext(tOSPF_LSDBCOUNT_INDEX *p_index,
		tOSPF_LSDBCOUNT_INDEX *p_nextindex)
{
	struct ospf_area search_area;
	struct ospf_process start_process;
	struct ospf_lstable search_table;
	struct ospf_lstable *p_table = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();

	search_table.type = p_index->type;
	search_table.p_process = &start_process;
	start_process.process_id = ospf_nm_process_id(p_index);

	search_table.p_area = &search_area;
	search_area.p_process = &start_process;
	search_area.id = p_index->area;

	for_each_node_greater(&ospf.nm.area_lsa_table, p_table, &search_table)
	{
		ospf_nm_process_check(p_table->p_process);
		rc = OK;
		p_nextindex->process_id = p_table->p_process->process_id;
		p_nextindex->area = p_table->p_area->id;
		p_nextindex->type = p_table->type;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfAreaLsaCountGetApi(tOSPF_LSDBCOUNT_INDEX *p_index, u_int cmd,
		void *var)
{
	struct ospf_lstable *p_table = NULL;
	struct ospf_area search_area;
	struct ospf_process start_process;
	struct ospf_lstable search_table;
	u_int *lval = (u_int *) var;
	STATUS rc = OK;

	ospf_semtake_try();

	search_table.type = p_index->type;
	search_table.p_process = &start_process;
	start_process.process_id = ospf_nm_process_id(p_index);

	search_table.p_area = &search_area;
	search_area.p_process = &start_process;
	search_area.id = p_index->area;

	p_table = ospf_lstlookup(&ospf.nm.area_lsa_table, &search_table);
	if (NULL == p_table)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_AREALSACOUNT_AREAID:
		*lval = p_index->area;
		break;

	case OSPF_AREALSACOUNT_LSTYPE:
		*lval = p_index->type;
		break;

	case OSPF_AREALSACOUNT_NUMBER:
		*lval = ospf_lstcnt(&p_table->list);
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:

	ospf_semgive();
	return rc;
}

STATUS ospfAreaRangeGetFirst(tOSPF_RANGE_INDEX *p_index)
{
	struct ospf_range *p_range = NULL;
	struct ospf_range *p_next = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();

	for_each_node(&ospf.nm.range_table, p_range, p_next)
	{
		ospf_nm_process_check(p_range->p_area->p_process);
		if (OSPF_LS_SUMMARY_NETWORK != p_range->lstype)
		{
			continue;
		}
		p_index->process_id = p_range->p_area->p_process->process_id;
		p_index->area = p_range->p_area->id;
		p_index->dest = p_range->network;
		p_index->mask = p_range->mask;
		rc = OK;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfAreaRangeGetNext(tOSPF_RANGE_INDEX *p_index,
		tOSPF_RANGE_INDEX *p_nextindex)
{
	struct ospf_range *p_range = NULL;
	struct ospf_range search_range;
	struct ospf_area search_area;
	struct ospf_process start_process;
	STATUS rc = ERROR;

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);

	search_area.p_process = &start_process;
	search_area.id = p_index->area;
	search_range.p_area = &search_area;
	search_range.network = p_index->dest;
	search_range.mask = p_index->mask;
	search_range.lstype = OSPF_LS_SUMMARY_NETWORK;

	for_each_node_greater(&ospf.nm.range_table, p_range, &search_range)
	{
		ospf_nm_process_check(p_range->p_area->p_process);
		if (OSPF_LS_SUMMARY_NETWORK == p_range->lstype)
		{
			rc = OK;
			p_nextindex->process_id = p_range->p_area->p_process->process_id;
			p_nextindex->area = p_range->p_area->id;
			p_nextindex->dest = p_range->network;
			p_nextindex->mask = p_range->mask;
			break;
		}
	}

	ospf_semgive();
	return rc;
}

STATUS _ospfAreaRangeSetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var,
		u_int flag)
{
	struct ospf_area *p_area = NULL;
	struct ospf_range *p_range = NULL;
	struct ospf_area search_area;
	struct ospf_process start_process;
	u_int effect = TRUE;
	u_int *lval = (u_int *) var;
	STATUS rc = OK;

	ospf_semtake_try();

	search_area.p_process = &start_process;
	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.id = p_index->area;
	p_area = ospf_lstlookup(&ospf.nm.area_table, &search_area);
	if (NULL == p_area)
	{
		rc = ERROR;
		goto FINISH;
	}
	p_range = ospf_range_lookup(p_area, OSPF_LS_SUMMARY_NETWORK, p_index->dest,
			p_index->mask);
	if ((NULL == p_range) && (OSPF_ADDRRANGE_STATUS != cmd))
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_ADDRRANGE_STATUS:
		switch (*lval)
		{
		case SNMP_ACTIVE:
		case SNMP_CREATEANDGO:
		case SNMP_CREATEANDWAIT:
			if (NULL == p_range)
			{
				p_range = ospf_range_create(p_area, OSPF_LS_SUMMARY_NETWORK,
						p_index->dest, p_index->mask);
				if (NULL == p_range)
				{
					rc = ERROR;
					break;
				}
			}
			ospf_range_up(p_range);
			break;

		case SNMP_NOTINSERVICE:
		case SNMP_NOTREADY:
			break;

		case SNMP_DESTROY:
			if (NULL == p_range)
			{
				rc = ERROR;
				break;
			}
			ospf_range_down(p_range);

			ospf_range_delete(p_range);
			break;

		default:
			break;
		}
		break;

	case OSPF_ADDRRANGE_EFFECT:
		effect = (OSPF_MATCH_ADV == (*(u_int*) var)) ? TRUE : FALSE;
		p_range->advertise = effect;

		if (p_range->advertise)
		{
			ospf_range_update(p_range);
		}
		else
		{
			ospf_originate_range_lsa(p_range, TRUE);
		}
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		// uspHwOspfAreaRangeSync(p_index, HW_OSPF_ADDRRANGE_CMDSTART + cmd, var, flag);
	}
	return rc;
}

STATUS ospfAreaRangeSetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iFlg = 0;
	int ret = 0;
	if((p_index == NULL) || (var == NULL))
	{
		return ERR;
	}

#endif
	return _ospfAreaRangeSetApi(p_index, cmd, var,
			(USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfAreaRangeSyncApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var)
{
	return _ospfAreaRangeSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

STATUS ospfAreaRangeGetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_range *p_range = NULL;
	struct ospf_range search_range;
	struct ospf_area search_area;
	struct ospf_process start_process;
	u_int *lval = (u_int *) var;
	STATUS rc = OK;

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.p_process = &start_process;
	search_area.id = p_index->area;
	search_range.p_area = &search_area;
	search_range.network = p_index->dest;
	search_range.mask = p_index->mask;
	search_range.lstype = OSPF_LS_SUMMARY_NETWORK;

	p_range = ospf_lstlookup(&ospf.nm.range_table, &search_range);
	if (NULL == p_range)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_ADDRRANGE_AREAID:
		*lval = p_index->area;
		break;

	case OSPF_ADDRRANGE_NET:
		*lval = p_index->dest;
		break;

	case OSPF_ADDRRANGE_MASK:
		*lval = p_index->mask;
		break;

	case OSPF_ADDRRANGE_STATUS:
		*lval = SNMP_ACTIVE;
		break;

	case OSPF_ADDRRANGE_EFFECT:
		*lval = (p_range->advertise) ? OSPF_MATCH_ADV : OSPF_MATCH_NOADV;
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

/*ospfAreaAggregateTable*/
STATUS ospfAreaAggregateGetFirst(tOSPF_AREAAGGR_INDEX *p_index)
{
	struct ospf_range *p_range = NULL;
	struct ospf_range *p_next_range = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();

	for_each_node(&ospf.nm.range_table, p_range, p_next_range)
	{
		ospf_nm_process_check(p_range->p_area->p_process);

		p_index->process_id = p_range->p_area->p_process->process_id;
		p_index->area = p_range->p_area->id;
		p_index->lstype = p_range->lstype;
		p_index->net = p_range->network;
		p_index->mask = p_range->mask;
		rc = OK;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfAreaAggregateGetNext(tOSPF_AREAAGGR_INDEX *p_index,
		tOSPF_AREAAGGR_INDEX *p_nextindex)
{
	struct ospf_range *p_range = NULL;
	struct ospf_range search_range;
	struct ospf_area search_area;
	struct ospf_process start_process;
	STATUS rc = ERROR;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.p_process = &start_process;
	search_area.id = p_index->area;
	search_range.p_area = &search_area;
	search_range.network = p_index->net;
	search_range.mask = p_index->mask;
	search_range.lstype = p_index->lstype;
	for_each_node_greater(&ospf.nm.range_table, p_range, &search_range)
	{
		ospf_nm_process_check(p_range->p_area->p_process);
		rc = OK;
		p_nextindex->process_id = p_range->p_area->p_process->process_id;
		p_nextindex->area = p_range->p_area->id;
		p_nextindex->net = p_range->network;
		p_nextindex->mask = p_range->mask;
		p_nextindex->lstype = p_range->lstype;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfAreaAggregateGetApi(tOSPF_AREAAGGR_INDEX *p_index, u_int cmd,
		void *var)
{
	struct ospf_range *p_range = NULL;
	struct ospf_range search_range;
	struct ospf_area search_area;
	struct ospf_process start_process;
	u_int *lval = (u_int *) var;
	STATUS rc = OK;

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.p_process = &start_process;
	search_area.id = p_index->area;
	search_range.p_area = &search_area;
	search_range.network = p_index->net;
	search_range.mask = p_index->mask;
	search_range.lstype = p_index->lstype;

	p_range = ospf_lstlookup(&ospf.nm.range_table, &search_range);
	if (NULL == p_range)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_AREAAGGR_AREAID:
		*lval = p_index->area;
		break;

	case OSPF_AREAAGGR_LSDBTYPE:
		*lval = p_index->lstype;
		break;

	case OSPF_AREAAGGR_NET:
		*lval = p_index->net;
		break;

	case OSPF_AREAAGGR_MASK:
		*lval = p_index->mask;
		break;

	case OSPF_AREAAGGR_STATUS:
		*lval = SNMP_ACTIVE;
		break;

	case OSPF_AREAAGGR_EFFECT:
		*lval = p_range->advertise ? OSPF_MATCH_ADV : OSPF_MATCH_NOADV;
		break;

	case OSPF_AREAAGGR_EXTROUTETAG:
		*lval = 0;
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS _ospfAreaAggregateSetApi(tOSPF_AREAAGGR_INDEX *p_index, u_int cmd,
		void *var, u_int flag)
{
	tOSPF_RANGE_INDEX range;
	struct ospf_process *p_process = NULL;
	STATUS rc = OK;
	ospf_semtake_try();
	p_process = ospf_get_nm_process(p_index);
	if (NULL == p_process)
	{
		rc = ERROR;
		goto FINISH;
	}
	range.area = p_index->area;
	range.dest = p_index->net;
	range.mask = p_index->mask;

	switch (cmd)
	{
	case OSPF_AREAAGGR_STATUS:
		if (OSPF_LS_SUMMARY_NETWORK == p_index->lstype)
		{
			rc = ospfAreaRangeSetApi(&range, OSPF_ADDRRANGE_STATUS, var);
		}
		else
		{
			rc = ospfNssaAreaRangeSetApi(&range, OSPF_NSSARANGE_STATUS, var);
		}
		break;

	case OSPF_AREAAGGR_EFFECT:
		if (OSPF_LS_SUMMARY_NETWORK == p_index->lstype)
		{
			rc = ospfAreaRangeSetApi(&range, OSPF_ADDRRANGE_EFFECT, var);
		}
		else
		{
			rc = ospfNssaAreaRangeSetApi(&range, OSPF_NSSARANGE_EFFECT, var);
		}
		break;

	case OSPF_AREAAGGR_EXTROUTETAG:
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		//  uspHwOspfAreaAggrSync(p_index, HW_OSPF_AREAAGGR_CMDSTART + cmd, var,  flag);
	}
	return rc;
}

STATUS ospfAreaAggregateSetApi(tOSPF_AREAAGGR_INDEX *p_index, u_int cmd,
		void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iFlg = 0;
	int ret = 0;
	if((p_index == NULL) || (var == NULL))
	{
		return ERR;
	}

#endif
	return _ospfAreaAggregateSetApi(p_index, cmd, var,
			(USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfAreaAggregateSyncApi(tOSPF_AREAAGGR_INDEX *p_index, u_int cmd,
		void *var)
{
	return _ospfAreaAggregateSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

STATUS ospfNssaAreaRangeGetFirst(tOSPF_RANGE_INDEX *p_index)
{
	struct ospf_range *p_range = NULL;
	struct ospf_range *p_next_range = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.range_table, p_range, p_next_range)
	{
		ospf_nm_process_check(p_range->p_area->p_process);
		if (OSPF_LS_TYPE_7 != p_range->lstype)
		{
			continue;
		}
		p_index->process_id = p_range->p_area->p_process->process_id;
		p_index->area = p_range->p_area->id;
		p_index->dest = p_range->network;
		p_index->mask = p_range->mask;
		rc = OK;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfNssaAreaRangeGetNext(tOSPF_RANGE_INDEX *p_index,
		tOSPF_RANGE_INDEX *p_nextindex)
{
	struct ospf_range *p_range = NULL;
	struct ospf_range search_range;
	struct ospf_area search_area;
	struct ospf_process start_process;
	STATUS rc = ERROR;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.p_process = &start_process;
	search_area.id = p_index->area;
	search_range.p_area = &search_area;
	search_range.network = p_index->dest;
	search_range.mask = p_index->mask;
	search_range.lstype = OSPF_LS_TYPE_7;

	for_each_node_greater(&ospf.nm.range_table, p_range, &search_range)
	{
		ospf_nm_process_check(p_range->p_area->p_process);
		if (OSPF_LS_TYPE_7 == p_range->lstype)
		{
			rc = OK;
			p_nextindex->process_id = p_range->p_area->p_process->process_id;
			p_nextindex->area = p_range->p_area->id;
			p_nextindex->dest = p_range->network;
			p_nextindex->mask = p_range->mask;
			break;
		}
	}

	ospf_semgive();
	return rc;
}

STATUS _ospfNssaAreaRangeSetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd,
		void *var, u_int flag)
{
	struct ospf_area search_area;
	struct ospf_process start_process;
	struct ospf_area *p_area = NULL;
	struct ospf_range *p_range = NULL;
	STATUS rc = OK;
	u_int *lval = (u_int *) var;

	ospf_semtake_try();
	search_area.p_process = &start_process;
	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.id = p_index->area;
	p_area = ospf_lstlookup(&ospf.nm.area_table, &search_area);
	if (NULL == p_area)
	{
		rc = ERROR;
		goto FINISH;
	}
	p_range = ospf_range_lookup(p_area, OSPF_LS_TYPE_7, p_index->dest,
			p_index->mask);
	if ((NULL == p_range) && (OSPF_NSSARANGE_STATUS != cmd))
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_NSSARANGE_STATUS:
		switch (*lval)
		{
		case SNMP_ACTIVE:
		case SNMP_CREATEANDGO:
		case SNMP_CREATEANDWAIT:
			if (NULL == p_range)
			{
				p_range = ospf_range_create(p_area, OSPF_LS_TYPE_7,
						p_index->dest, p_index->mask);
				if (NULL == p_range)
				{
					rc = ERROR;
					break;
				}
			}
			ospf_nssa_range_up(p_range);
			break;

		case SNMP_NOTINSERVICE:
		case SNMP_NOTREADY:
			break;

		case SNMP_DESTROY:
			if (NULL == p_range)
			{
				rc = ERROR;
				break;
			}
			p_range->isdown = TRUE;
			ospf_nssa_range_down(p_range);
			p_range->isdown = FALSE;
			ospf_range_delete(p_range);
			break;

		default:
			break;
		}
		break;

	case OSPF_NSSARANGE_COST:
		p_range->cost = *lval;
		break;

	case OSPF_NSSARANGE_EFFECT:
		p_range->advertise = (OSPF_MATCH_ADV == *lval) ? TRUE : FALSE;
		if (p_range->advertise)
		{
			ospf_nssa_range_update(p_range);
		}
		else
		{
			ospf_originate_range_lsa(p_range, TRUE);
		}
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		// uspHwOspfNssaAreaRangeSync(p_index, HW_OSPF_NSSARANGE_CMDSTART + cmd, var,  flag);
	}
	return rc;
}

STATUS ospfNssaAreaRangeSetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iFlg = 0;
	int ret = 0;
	if((p_index == NULL) || (var == NULL))
	{
		return ERR;
	}

#endif
	return _ospfNssaAreaRangeSetApi(p_index, cmd, var,
			(USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfNssaAreaRangeSyncApi(tOSPF_RANGE_INDEX *p_index, u_int cmd,
		void *var)
{
	return _ospfNssaAreaRangeSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

STATUS ospfNssaAreaRangeGetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_range search_range;
	struct ospf_area search_area;
	struct ospf_process start_process;
	struct ospf_range *p_range = NULL;
	u_int *lval = (u_int *) var;
	STATUS rc = OK;

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.p_process = &start_process;
	search_area.id = p_index->area;
	search_range.p_area = &search_area;
	search_range.network = p_index->dest;
	search_range.mask = p_index->mask;
	search_range.lstype = OSPF_LS_TYPE_7;

	p_range = ospf_lstlookup(&ospf.nm.range_table, &search_range);
	if (NULL == p_range)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_NSSARANGE_AREAID:
		*lval = p_index->area;
		break;

	case OSPF_NSSARANGE_DEST:
		*lval = p_index->dest;
		break;

	case OSPF_NSSARANGE_MASK:
		*lval = p_index->mask;
		break;

	case OSPF_NSSARANGE_STATUS:
		*lval = SNMP_ACTIVE;
		break;

	case OSPF_NSSARANGE_COST:
		*lval = p_range->cost;
		break;

	case OSPF_NSSARANGE_EFFECT:
		*lval = p_range->advertise ? OSPF_MATCH_ADV : OSPF_MATCH_NOADV;
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

/*host,not support*/
STATUS ospfHostGetFirst(u_int *p_id)
{
	return ERROR;
}

STATUS ospfHostGetNext(u_int id, u_int *p_nextid)
{
	return ERROR;
}

STATUS ospHostGetApi(u_int id, u_int cmd, void *var)
{
	return ERROR;
}

STATUS ospHostSetApi(u_int id, u_int cmd, void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iFlg = 0;
	int ret = 0;
	if(var == NULL)
	{
		return ERR;
	}

#endif
	return ERROR;
}

/*normal interface*/
STATUS ospfIfGetFirst(tOSPF_IFINDEX *p_index)
{
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.if_table, p_if, p_next_if)
	{
#ifdef OSPF_DCN
		if(OSPF_DCN_PROCESS == p_if->p_process->process_id)
		{
			continue;
		}
#endif
		ospf_nm_process_check(p_if->p_process);
		p_index->process_id = p_if->p_process->process_id;
		p_index->ipaddr = p_if->addr;
		p_index->addrlessif = p_if->ifnet_uint;
		rc = OK;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfIfGetNext(tOSPF_IFINDEX *p_index, tOSPF_IFINDEX *p_next_index)
{
	struct ospf_if search_if =
	{ 0 };
	struct ospf_if *p_if = NULL;
	struct ospf_process start_process =
	{ 0 };
	STATUS rc = ERROR;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search_if.p_process = &start_process;
	search_if.addr = p_index->ipaddr;
	search_if.ifnet_uint = p_index->addrlessif;

	for_each_node_greater(&ospf.nm.if_table, p_if, &search_if)
	{
#ifdef OSPF_DCN
		if(OSPF_DCN_PROCESS == p_if->p_process->process_id)
		{
			continue;
		}
#endif
		ospf_nm_process_check(p_if->p_process);
		rc = OK;
		p_next_index->process_id = p_if->p_process->process_id;
		p_next_index->ipaddr = p_if->addr;
		p_next_index->addrlessif = p_if->ifnet_uint;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfIfGetApi(tOSPF_IFINDEX *p_index, u_int cmd, void *var)
{
	struct ospf_if *p_if = NULL;
	struct ospf_nbr *p_nbr = NULL;
	STATUS rc = OK;
	u_int *lval = (u_int*) var;
	tlv_t *octet = (tlv_t *) var;
	struct ospf_process *pProcess = NULL;

	ospf_semtake_try();

	pProcess = ospf_get_nm_process(p_index);
	if (NULL == pProcess)
	{
		rc = ERROR;
		goto FINISH;
	}

	if (p_index->ipaddr != 0
#ifdef OSPF_DCN
			&& pProcess->process_id != OSPF_DCN_PROCESS
#endif
			)
	{
		p_if = ospf_if_lookup(pProcess, p_index->ipaddr);
	}
	else
	{
		p_if = ospf_if_lookup_by_ifindex(pProcess, p_index->addrlessif);
	}
	if (NULL == p_if)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_IF_IPADDR:
		*lval = p_if->addr;
		break;

	case OSPF_IF_IPADDRLESSIF:
		*lval = 0;
		break;

	case OSPF_IF_AREAID:
		if (NULL != p_if->p_area)
		{
			*lval = p_if->p_area->id;
		}
		break;

	case OSPF_IF_TYPE:
		*lval = p_if->type;
		break;

	case OSPF_IF_ADMINSTATUS:
		*lval = p_if->pif_enable;
		break;

	case OSPF_IF_PRIORITY:
		*lval = p_if->priority;
		break;

	case OSPF_IF_TRANSITDELAY:
		*lval = p_if->tx_delay;
		break;

	case OSPF_IF_RETRANSMITINTERVAL:
		*lval = p_if->rxmt_interval;
		break;

	case OSPF_IF_HELLOINTERVAL:
		*lval = p_if->hello_interval;
		break;

	case OSPF_IF_DEADINTERVAL:
		*lval = p_if->dead_interval;
		break;

	case OSPF_IF_POLLINTERVAL:
		*lval = p_if->poll_interval;
		break;

	case OSPF_IF_STATE:
		*lval = p_if->state;
		break;

	case OSPF_IF_DRADDR:
		*lval = p_if->dr;
		break;

	case OSPF_IF_BDRADDR:
		*lval = p_if->bdr;
		break;

	case OSPF_IF_EVENT:
		*lval = p_if->stat->state_change;
		break;

	case OSPF_IF_AUTHKEY:
	{
		if (octet->len > p_if->keylen)
		{
			octet->len = p_if->keylen;
		}

		if (0 != octet->len)
		{
			memcpy(octet->data, p_if->key, octet->len);
		}
		break;
	}

	case OSPF_IF_AUTHKEYID:
		*lval = p_if->md5id;
		break;

	case OSPF_IF_STATUS:
		*lval = SNMP_ACTIVE;
		break;

	case OSPF_IF_MCASTFWDING:
		*lval = p_if->mcast;
		break;

	case OSPF_IF_DEMAND:
		*lval = p_if->demand;
		break;

	case OSPF_IF_AUTHTYPE:
		*lval = p_if->authtype;
		break;

	case OSPF_IF_AUTHDIS:
		*lval = p_if->authdis;
		break;

	case OSPF_IF_PASSIVE:
		*lval = p_if->passive;
		break;

	case OSPF_IF_MTU:
		*lval = p_if->mtu;
		break;

	case OSPF_IF_TEADMINGROUP:
	{
		u_int i;
		u_int group;

		if (0 == p_if->te_group)
		{
			*lval = 0;
			break;
		}

		for (i = 0; i < 32; i++)
		{
			group = p_if->te_group >> i;
			if (group & 0x00000001)
			{
				*lval = i;
				break;
			}
		}
	}
		break;

	case OSPF_IF_TECOST:
		*lval = p_if->te_cost;
		break;

	case OSPF_IF_TEENABLE:
		*lval = ospfvaule_to_nm(p_if->te_enable);
		break;

	case OSPF_IF_TEMAXBINDWIDTH:
		*lval = p_if->max_bd;
		break;

	case OSPF_IF_TEMAXRSVDBINDWIDTH:
		*lval = p_if->max_rsvdbd;
		break;

	case OSPF_IF_STATICCOST:
		*lval = p_if->configcost;
		break;

	case OSPF_IF_LSCOUNT:
		*lval = ospf_lstcnt(&p_if->opaque_lstable.list);
		break;

	case OSPF_IF_LSCHECKSUM:
		*lval = p_if->p_process->t9lsa_checksum;
		break;

	case OSPF_IF_DRID:
		*lval = 0;
		if (p_if->dr == p_if->addr)
		{
			*lval = p_if->p_process->router_id;
		}
		else
		{
			p_nbr = ospf_nbr_lookup(p_if, p_if->dr);
			if (NULL != p_nbr)
			{
				*lval = p_nbr->id;
			}
		}
		break;

	case OSPF_IF_BDRID:
		*lval = 0;
		if (p_if->bdr == p_if->addr)
		{
			*lval = p_if->p_process->router_id;
		}
		else
		{
			p_nbr = ospf_nbr_lookup(p_if, p_if->bdr);
			if (NULL != p_nbr)
			{
				*lval = p_nbr->id;
			}
		}
		break;

#ifdef HAVE_BFD
		case OSPF_IF_BFD:
		*lval = p_if->bfd_enable;
		break;

		case OSPF_IF_BFD_MIN_RX_INTERVAL:
		*lval = p_if->ulRxMinInterval;
		break;

		case OSPF_IF_BFD_MIN_TX_INTERVAL:
		*lval = p_if->ulTxMinInterval;
		break;

		case OSPF_IF_BFD_DETECT_MUL:
		*lval = p_if->ulDetMulti;
		break;
		case OSPF_IF_BFD_COUNT:
		*lval = p_if->stat->bfd_error_cnt;
		break;
#endif
	case OSPF_IF_FLOODGROUP:
		*lval = p_if->flood_group;
		break;

	case OSPF_IF_FASTDDEXCHANGE:
		*lval = ospfvaule_to_nm(p_if->fast_dd_enable);
		break;

	case OSPF_IF_UNIT:
		*lval = p_if->ifnet_uint;
		break;

	case OSPF_IF_MTUIGNORE:
		*lval = ospfvaule_to_nm(p_if->mtu_ignore);
		break;

	case OSPF_IF_CIPHERKEY:
		octet->len = strlen(p_if->cipher_key);
		if (0 != octet->len)
		{
			memcpy(octet->data, p_if->cipher_key, octet->len);
		}
		break;

	case OSPF_IF_LDP_SYNC:
		*lval = ospfvaule_to_nm(p_if->ucLdpSyncEn);
		break;

	case OSPF_IF_VRID:
		*lval = p_if->p_process->vrid;
		break;

	case OSPF_IF_HOLD_DOWN:
		*lval = p_if->ulHoldDownInterval;
		break;

	case OSPF_IF_HOLD_COST:
		*lval = p_if->ulHoldCostInterval;
		break;

	case OSPF_IF_LDP_SYNC_STATE:
		*lval = p_if->ulOspfSyncState;
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS _ospfIfSetApi(tOSPF_IFINDEX *p_index, u_int cmd, void *var, u_int flag)
{
	struct ospf_area *p_area = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_process start_process;
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_nbr *p_next_nbr = NULL;
	u_int lval = 0;
	u_int te_build = FALSE;
	STATUS rc = OK;
	struct ospf_network *p_network = NULL;
	struct ospf_network *p_next = NULL;
	u_int uiMinMask = 0;
	u_int uiIfAddr = 0;
	u_int uiMask = 0;
	u_int uiLoopBackIndex = 0;
	struct prefix_ipv4 stPrefixIp =
	{ 0 };

	ospf_logx(ospf_debug, "_ospfIfSetApi:  strat\r\n");

	ospf_logx(ospf_debug, "%d\r\n", __LINE__);

	ospf_semtake_try();
	ospf_logx(ospf_debug, "%d\r\n", __LINE__);
	p_process = ospf_get_nm_process(p_index);
	if (NULL == p_process)
	{
		rc = ERROR;

		ospf_logx(ospf_debug, "_ospfIfSetApip_index1 0x%x.\n", p_index);

		goto FINISH;
	}

#if 0 //def OSPF_DCN
	p_if = ospf_if_lookup_unnumber(p_process, p_index->ipaddr, p_index->addrlessif);
#else
	if (p_index->ipaddr != 0
#ifdef OSPF_DCN
			&& p_process->process_id != OSPF_DCN_PROCESS
#endif
			)
	{
		p_if = ospf_if_lookup(p_process, p_index->ipaddr);
	}
	else
	{
		p_if = ospf_if_lookup_by_ifindex(p_process, p_index->addrlessif);
	}
#endif
	if ((NULL == p_if) && (OSPF_IF_AREAID != cmd) && (OSPF_IF_STATUS != cmd)
			&& OSPF_IF_ADMINSTATUS != cmd && OSPF_IF_DCN_ADD != cmd
			&& OSPF_IF_DCN_DELETE != cmd)
	{
		rc = ERROR;
		ospf_logx(ospf_debug, "_ospfIfSetApip_index:22 0x%x.\n",
				p_index->ipaddr);
		goto FINISH;
	}

	lval = *(u_int*) var;
	tlv_t *octet = (tlv_t *) var;
	switch (cmd)
	{
	case OSPF_IF_AREAID:
		p_area = ospf_area_lookup(p_process, lval);
		if (NULL == p_area)
		{
			rc = ERROR;
			break;
		}

		/*if interface exist,and interface has area configured,do nothing*/
		if (NULL != p_if)
		{
			if (NULL == p_if->p_area)
			{
				p_if->p_area = p_area;
				p_if->state = OSPF_IFS_DOWN;
				ospf_lstadd(&p_area->if_table, p_if);
				ospf_lstwalkup(&p_process->if_table, ospf_if_state_update);
				p_if->opaque_lstable.p_area = p_area;
			}
			else
			{
				ospf_ism(p_if, OSPF_IFE_DOWN);
				ospf_lstdel(&p_if->p_area->if_table, p_if);
				p_if->p_area = p_area;
				ospf_lstadd(&p_area->if_table, p_if);
				ospf_lstwalkup(&p_process->if_table, ospf_if_state_update);
				p_if->opaque_lstable.p_area = p_area;
			}
		}
		else
		{
			/*create interface with area*/
			if (p_index->ipaddr != 0)
			{
				ospf_real_if_create(p_process, p_index->ipaddr, p_area);
			}
#ifdef OSPF_DCN
			else if(p_index->process_id != OSPF_DCN_PROCESS)
			{
				ospf_logx(ospf_debug,"##ospf if set ifunit=%x",p_index->addrlessif);

				ospf_real_if_create_by_ifindex(p_process, p_index->addrlessif, p_area);
			}
#endif
		}
		ospf_logx(ospf_debug, "%d\r\n", __LINE__);

		break;

	case OSPF_IF_TYPE:
		if ((OSPF_IFT_VLINK == lval) || (OSPF_IFT_SHAMLINK == lval))
		{
			rc = ERROR;
			ospf_logx(ospf_debug, "%d\r\n", __LINE__);
			ospf_logx(ospf_debug, "OSPF_IF_TYPE1:lval=5d\n", lval);
			break;
		}
		if (p_if->type != lval)
		{
			if (NULL != p_if->p_area)
			{
				ospf_ism(p_if, OSPF_IFE_DOWN);
			}
			ospf_logx(ospf_debug, "OSPF_IF_TYPE2:lval=5d\n", lval);
			if (OSPF_IFT_NBMA == p_if->type)
			{
				for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
				{
					ospf_nsm(p_nbr, OSPF_NE_LL_DOWN);
					ospf_nbr_delete(p_nbr);
				}
			}
			p_if->type = lval;
			ospf_lstwalkup(&p_process->if_table, ospf_if_state_update);
			ospf_logx(ospf_debug, "%d\r\n", __LINE__);
			ospf_logx(ospf_debug, "OSPF_IF_TYPE3:lval=5d\n", lval);
		}
		break;

	case OSPF_IF_ADMINSTATUS:
		if (NULL != p_if)
		{
			p_if->pif_enable = lval;
		}
		if (TRUE == lval)
		{
			if (NULL == p_if)
			{
				if (p_index->ipaddr != 0)
				{
					ospf_real_if_create(p_process, p_index->ipaddr, NULL);
				}
#ifdef OSPF_DCN
				else if(p_index->process_id != OSPF_DCN_PROCESS)
				{
					ospf_real_if_create_by_ifindex(p_process, p_index->addrlessif, p_area);

				}
#endif
			}
		}
		else if (NULL != p_if)
		{
			for_each_node(&p_process->network_table, p_network, p_next)
			{
				uiMinMask =
						(p_network->mask < p_if->mask) ?
								p_network->mask : p_if->mask;
				if (ospf_netmatch(p_network->dest, p_if->addr, uiMinMask))
				{
					if (p_if->p_area->id != p_network->area_id)
					{
						ospf_ism(p_if, OSPF_IFE_DOWN);
						p_area = ospf_area_lookup(p_process,
								p_network->area_id);
						ospf_lstdel(&p_if->p_area->if_table, p_if);
						p_if->p_area = p_area;
						//p_if->state = OSPF_IFS_DOWN;
						if (p_area)
						{
							ospf_lstadd(&p_area->if_table, p_if);
						}
						ospf_lstwalkup(&p_process->if_table,
								ospf_if_state_update);
						p_if->opaque_lstable.p_area = p_area;
					}
					break;
				}
			}
			if (p_network == NULL)
			{
				ospf_if_delete(p_if);
			}
		}
		break;

	case OSPF_IF_PRIORITY:
		p_if->priority = lval;
		break;

	case OSPF_IF_TRANSITDELAY:
		p_if->tx_delay = lval;
		break;

	case OSPF_IF_RETRANSMITINTERVAL:
		p_if->rxmt_interval = lval;
		for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
		{
			if (ospf_timer_active(&p_nbr->lsrxmt_timer))
			{
				ospf_stimer_start(&p_nbr->lsrxmt_timer, p_if->rxmt_interval + 1);
			}
			if (ospf_timer_active(&p_nbr->request_timer))
			{
				ospf_stimer_start(&p_nbr->request_timer,
						p_if->rxmt_interval + 1);
			}
		}
		break;

	case OSPF_IF_HELLOINTERVAL:
		if (p_if->hello_interval * 4 == p_if->dead_interval) /*��Ϊdead��Ĭ��ֵ������4*/
		{
			p_if->dead_interval = lval * 4;
			for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
			{
				ospf_stimer_start(&p_nbr->hold_timer, p_if->dead_interval);
			}
		}
		p_if->hello_interval = lval;
		ospf_stimer_start(&p_if->hello_timer, p_if->hello_interval)
		;
#if 0
		if(p_if->dead_interval == OSPF_DEFAULT_ROUTER_DEAD_INTERVAL)
		{
			for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
			{
				ospf_stimer_start(&p_nbr->hold_timer, p_if->hello_interval*4);
			}
		}
#endif
		break;

	case OSPF_IF_DEADINTERVAL:
		p_if->dead_interval = lval;
		for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
		{
			ospf_stimer_start(&p_nbr->hold_timer, p_if->dead_interval);
		}
		break;

	case OSPF_IF_POLLINTERVAL:
		p_if->poll_interval = lval;
		ospf_stimer_start(&p_if->poll_timer, p_if->poll_interval)
		;
		break;

	case OSPF_IF_AUTHKEY:/*add 4bytes for ngpon,may be deleted*/
	{
		u_int len =
				(octet->len > (OSPF_MD5_KEY_LEN + 4)) ?
						(OSPF_MD5_KEY_LEN + 4) : octet->len;
		/*md5 key use the max 16bytes,it cover simple password*/
		bzero(p_if->key, OSPF_MD5_KEY_LEN + 4);
		bcopy(octet->data, p_if->key, len);
		p_if->keylen = len;
		flag |= USP_SYNC_OCTETDATA;
	}
		break;

	case OSPF_IF_AUTHKEYID:
		p_if->md5id = lval;
		break;

	case OSPF_IF_AUTHDIS:
		p_if->authdis = lval;
		break;

	case OSPF_IF_STATUS:
		switch (lval)
		{
		case SNMP_ACTIVE:
		case SNMP_CREATEANDGO:
		case SNMP_CREATEANDWAIT:
			if (NULL == p_if)
			{
				ospf_real_if_create(p_process, p_index->ipaddr, NULL);
			}
			break;

		default:
			if (NULL != p_if)
			{
				ospf_if_delete(p_if);
			}
			break;
		}
		break;

	case OSPF_IF_MCASTFWDING:
		p_if->mcast = lval;
		break;

	case OSPF_IF_DEMAND:
		p_if->demand = lval;
		break;

	case OSPF_IF_AUTHTYPE:
		p_if->authtype = lval;

		p_if->maxlen = ospf_if_max_len(p_if);

		if (OSPF_AUTH_NONE == p_if->authtype)
		{
			memset(p_if->key, 0, sizeof(p_if->key));
			p_if->md5id = 0;
		}
		break;

	case OSPF_IF_PASSIVE:
		p_if->passive = lval;
		break;

	case OSPF_IF_MTU:
		/*limit range*/
		if ((0 == lval) || (OSPF_MAX_IP_MTU < lval))
		{
			lval = OSPF_DEFAULT_IP_MTU;
		}
		p_if->mtu = lval;
		p_if->maxlen = ospf_if_max_len(p_if);
		ospf_if_to_nbr(p_if);
		break;

	case OSPF_IF_TEMAXBINDWIDTH:
		if (lval != p_if->max_bd)
		{
			p_if->max_bd = lval;
			if (p_if->te_enable)
			{
				te_build = TRUE;
			}
		}
		break;

	case OSPF_IF_TEENABLE:
		if (lval != p_if->te_enable)
		{
			p_if->te_enable = lval;

			if (p_if->p_area->te_enable)
			{
				te_build = TRUE;
			}
		}
		break;

	case OSPF_IF_TEMAXRSVDBINDWIDTH:
		if (lval != p_if->max_rsvdbd)
		{
			p_if->max_rsvdbd = lval;

			if (p_if->te_enable)
			{
				te_build = TRUE;
			}
		}
		break;

	case OSPF_IF_TEADMINGROUP:
		if (31 < lval)
		{
			p_if->te_group = 0;
		}
		else
		{
			p_if->te_group |= (0x00000001 << lval);
		}

		if (p_if->te_enable)
		{
			te_build = TRUE;
		}
		break;

	case OSPF_IF_TECOST:
		if (lval != p_if->te_cost)
		{
			p_if->te_cost = lval;

			if (p_if->te_enable)
			{
				te_build = TRUE;
			}
		}
		break;

#ifdef HAVE_BFD
		case OSPF_IF_BFD:
		if (p_if->bfd_enable != lval)
		{
			p_if->bfd_enable = lval;
			if ((p_if->bfd_enable == OSPF_BFD_IF_TRUE)
					|| (p_if->bfd_enable == OSPF_BFD_GBL_TRUE))
			{
				//ospf_lstwalkup(&p_if->nbr_table, ospf_bind_bfd);
				ospf_if_bind_bfd_enable(p_if);
			}
			else if((p_if->bfd_enable == OSPF_BFD_DISABLE)
					|| (p_if->bfd_enable == OSPF_BFD_BLOCK))
			{
				ospf_lstwalkup(&p_if->nbr_table, ospf_unbind_bfd);
			}
		}
		break;

		case OSPF_IF_BFD_MIN_RX_INTERVAL:
		{
			if(p_if->ulRxMinInterval == lval)
			{
				break;
			}
			p_if->ulRxMinInterval = lval;
			if(p_if->bfd_enable == OSPF_BFD_IF_TRUE)
			{
				for_each_node(&p_if->nbr_table,p_nbr,p_next_nbr)
				{
					ospf_mod_bfd(p_nbr,BFD_MOD_TYPE_MIN_RX);
				}
			}
			break;
		}

		case OSPF_IF_BFD_MIN_TX_INTERVAL:
		{
			if(p_if->ulTxMinInterval == lval)
			{
				break;
			}
			p_if->ulTxMinInterval = lval;
			if(p_if->bfd_enable == OSPF_BFD_IF_TRUE)
			{
				for_each_node(&p_if->nbr_table,p_nbr,p_next_nbr)
				{
					ospf_mod_bfd(p_nbr,BFD_MOD_TYPE_MIN_TX);
				}
			}
			break;
		}

		case OSPF_IF_BFD_DETECT_MUL:
		{
			if(p_if->ulDetMulti == lval)
			{
				break;
			}
			p_if->ulDetMulti = lval;
			if(p_if->bfd_enable == OSPF_BFD_IF_TRUE)
			{
				for_each_node(&p_if->nbr_table,p_nbr,p_next_nbr)
				{
					ospf_mod_bfd(p_nbr,BFD_MOD_TYPE_DETECT_MULT);
				}
			}
			break;
		}
		case OSPF_IF_BFD_COUNT:
		{
			p_if->stat->bfd_error_cnt = lval;
			break;
		}
#endif
	case OSPF_IF_FLOODGROUP:
		p_if->flood_group = lval;
		break;

	case OSPF_IF_FASTDDEXCHANGE:
		p_if->fast_dd_enable = nmvaule_to_ospf(lval);
		break;

	case OSPF_IF_MTUIGNORE:
		if (p_if->mtu_ignore == nmvaule_to_ospf(lval))
		{
			break;
		}
		p_if->mtu_ignore = nmvaule_to_ospf(lval);
		/*修改mtu使能后需重启进程*/
		ospf_timer_start(&p_process->id_reset_timer, 1);
		ospf_hello_output(p_if, TRUE, FALSE);
		break;

	case OSPF_IF_CIPHERKEY:
		bzero(p_if->cipher_key, OSPF_MD5_KEY_LEN + 4);
		memcpy(p_if->cipher_key, octet->data, OSPF_MD5_KEY_LEN + 4);
		break;

	case OSPF_IF_LDP_SYNC:
		if (p_if->ucLdpSyncEn == nmvaule_to_ospf(lval))
		{
			break;
		}
		p_if->ucLdpSyncEn = nmvaule_to_ospf(lval);

		if (p_if->ucLdpSyncEn == TRUE)
		{
			p_if->ulOspfSyncState = OSPF_LDP_INIT;
			if (p_if->link_up == TRUE)
			{
				//ospf_ldp_timer_start(p_if);
			}
		}
		else
		{
			ospf_if_hold_down_stop(p_if);
			ospf_if_hold_cost_stop(p_if);
		}
#if 0
		if (p_if->ucLdpSyncEn)
		{
			/*ospf ldp-sync enable*/
			p_if->ulOspfSyncState = OSPF_LDP_INIT;
			if(p_if->link_up == TRUE)
			{
				ospf_stimer_start(&p_if->hold_down_timer, p_if->ulHoldDownInterval);
			}
		}
		else
		{
			ospf_if_hold_down_stop(p_if);
			ospf_if_hold_cost_stop(p_if);
		}
#endif

		break;

	case OSPF_IF_HOLD_DOWN:
		if (p_if->ulHoldDownInterval == lval)
		{
			break;
		}
		p_if->ulHoldDownInterval = lval;
		if (ospf_timer_active(&p_if->hold_down_timer) == TRUE)
		{
			/*��ʱ����ʱǰ���������ö�ʱ��ʱ�䣬��ʱ���������ó�ʱʱ��*/
			ospf_stimer_start(&p_if->hold_down_timer, p_if->ulHoldDownInterval);
		}
		break;

	case OSPF_IF_HOLD_COST:
		if (p_if->ulHoldCostInterval == lval)
		{
			break;
		}
		p_if->ulHoldCostInterval = lval;
		if (ospf_timer_active(&p_if->hold_cost_timer) == TRUE)
		{
			ospf_stimer_start(&p_if->hold_cost_timer, p_if->ulHoldCostInterval);
		}
		break;
#ifdef OSPF_DCN
		case OSPF_IF_DCN_ADD:
		if(OK != if_loopback_id_to_index(OSPF_DCN_LOOPBACK_INDEX, &uiLoopBackIndex))
		{
			rc = ERROR;
			break;
		}

		if(OK != zebra_if_get_api(uiLoopBackIndex, ZEBRA_IF_IPADRESS_GET, &stPrefixIp))
		{
			rc = ERROR;
			break;
		}

		memcpy(&uiIfAddr, &stPrefixIp.prefix, 4);
		uiIfAddr = htonl(uiIfAddr);
		uiMask = ospf_len2mask(stPrefixIp.prefixlen);

		p_area = ospf_area_lookup(p_process, 0);
		if(NULL == p_area)
		{
			rc = ERROR;
			break;
		}

		ospf_Dcn_real_if_create(p_process, p_index->addrlessif, uiIfAddr, uiMask, p_area);
		break;
		case OSPF_IF_DCN_DELETE:
		if(OK != if_loopback_id_to_index(OSPF_DCN_LOOPBACK_INDEX, &uiLoopBackIndex))
		{
			rc = ERROR;
			break;
		}

		if(OK != zebra_if_get_api(uiLoopBackIndex, ZEBRA_IF_IPADRESS_GET, &stPrefixIp))
		{
			rc = ERROR;
			break;
		}
		memcpy(&uiIfAddr, &stPrefixIp.prefix, 4);
		uiIfAddr = htonl(uiIfAddr);
		uiMask = ospf_len2mask(stPrefixIp.prefixlen);
		p_if = ospf_if_lookup_forDcnCreat(p_process, uiIfAddr, p_index->addrlessif);
		if(p_if == NULL)
		{
			printf("get p_if is null\n");
		}
		else
		{
			ospf_if_delete(p_if);
		}
		break;
#endif
	default:
		rc = ERROR;
		break;
	}

	if (te_build)
	{
		ospf_link_te_lsa_originate(p_if);
	}
	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		//  uspHwOspfIfSync(p_index, HW_OSPF_IF_CMDSTART + cmd, var, flag);
	}
	return rc;
}

STATUS ospfIfSetApi(tOSPF_IFINDEX *p_index, u_int cmd, void *var)

{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iFlg = 0;
	int ret = 0;
	if((p_index == NULL) || (var == NULL))
	{
		return ERR;
	}

	if ((cmd == OSPF_IF_AUTHKEY)
			|| (cmd == OSPF_IF_CIPHERKEY))
	{
		iFlg |= USP_SYNC_OCTETDATA;
	}

#endif
	return _ospfIfSetApi(p_index, cmd, var, (USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfIfSynApi(tOSPF_IFINDEX *p_index, u_int cmd, void *var)

{
	return _ospfIfSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

STATUS ospfIfMetricGetFirst(tOSPF_IFMETRIC_INDEX *p_index)
{
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.if_table, p_if, p_next_if)
	{
		ospf_nm_process_check(p_if->p_process);
		p_index->process_id = p_if->p_process->process_id;
		p_index->ifip = p_if->addr;
		p_index->ifindex = p_if->ifnet_uint;
		p_index->tos = 0;
		rc = OK;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfIfMetricGetNext(tOSPF_IFMETRIC_INDEX *p_index,
		tOSPF_IFMETRIC_INDEX *p_nextindex)
{
	struct ospf_if search_if =
	{ 0 };
	struct ospf_if *p_if = NULL;
	struct ospf_process start_process =
	{ 0 };
	u_int tos = 0;
	STATUS rc = ERROR;

	if (OSPF_MAX_TOS <= p_index->tos)
	{
		return ERROR;
	}

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);
	search_if.p_process = &start_process;
	search_if.addr = p_index->ifip;
	search_if.ifnet_uint = p_index->ifindex;

	p_if = ospf_lstlookup(&ospf.nm.if_table, &search_if);
	if (p_if)
	{
		for (tos = p_index->tos + 1; tos < OSPF_MAX_TOS; tos++)
		{
			if (0 != p_if->cost[tos])
			{
				rc = OK;
				goto FINISH;
			}
		}
	}
	for_each_node_greater(&ospf.nm.if_table, p_if, &search_if)
	{
		ospf_nm_process_check(p_if->p_process);
		tos = 0;
		rc = OK;
		goto FINISH;
	}
	FINISH: if (OK == rc)
	{
		p_nextindex->process_id = p_if->p_process->process_id;
		p_nextindex->ifip = p_if->addr;
		p_nextindex->ifindex = p_if->ifnet_uint;
		p_nextindex->tos = tos;
	}

	ospf_semgive();

	return rc;
}

STATUS ospfIfMetricGetApi(tOSPF_IFMETRIC_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_if search_if =
	{ 0 };
	struct ospf_process start_process =
	{ 0 };
	struct ospf_if *p_if = NULL;
	u_int *lval = (u_int*) var;
	STATUS rc = OK;

	if (OSPF_MAX_TOS <= p_index->tos)
	{
		return ERROR;
	}

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);
	search_if.p_process = &start_process;
	search_if.addr = p_index->ifip;
	search_if.ifnet_uint = p_index->ifindex;

	p_if = ospf_lstlookup(&ospf.nm.if_table, &search_if);
	if (NULL == p_if)
	{
		rc = ERROR;
		goto FINISH;
	}
	switch (cmd)
	{
	case OSPF_IFMETRIC_IPADDR:
		*lval = p_index->ifip;
		break;

	case OSPF_IFMETRIC_IPADDRLESSIF:
		*lval = 0;
		break;

	case OSPF_IFMETRIC_TOS:
		*lval = p_index->tos;
		break;

	case OSPF_IFMETRIC_VALUE:
		*lval = p_if->cost[p_index->tos];
		break;

	case OSPF_IFMETRIC_STATUS:
		*lval = (0 != p_if->cost[p_index->tos]) ? SNMP_ACTIVE : SNMP_DESTROY;
		break;

	case OSPF_IFMERTRIC_COSTSTATUS:
		*lval = p_if->costflag;
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS _ospfIfMetricSetApi(tOSPF_IFMETRIC_INDEX *p_index, u_int cmd, void *var,
		u_int flag)
{
	struct ospf_if *p_if = NULL;
	struct ospf_if search_if =
	{ 0 };
	struct ospf_process start_process =
	{ 0 };
	u_int lval = *(u_int*) var;
	STATUS rc = OK;

	if (OSPF_MAX_TOS <= p_index->tos)
	{
		return ERROR;
	}

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);
	search_if.p_process = &start_process;
	search_if.addr = p_index->ifip;
#ifdef OSPF_DCN
	search_if.ifnet_uint = p_index->ifindex;
#endif
	p_if = ospf_lstlookup(&ospf.nm.if_table, &search_if);
	if (NULL == p_if)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_IFMETRIC_VALUE:
		//p_if->cost[p_index->tos] = lval;
		ospf_if_cost_update(p_if, p_index->tos, lval);
		break;

	case OSPF_IFMETRIC_STATUS:
		p_if->costflag = ospf_status(lval);
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		//  uspHwOspfIfMetricSync(p_index, HW_OSPF_IFMETRIC_CMDSTART + cmd, var,  flag);
	}
	return rc;
}

STATUS ospfIfMetricSetApi(tOSPF_IFMETRIC_INDEX *p_index, u_int cmd, void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iFlg = 0;
	int ret = 0;
	if((p_index == NULL) || (var == NULL))
	{
		return ERR;
	}

#endif
	return _ospfIfMetricSetApi(p_index, cmd, var,
			(USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfIfMetricSyncApi(tOSPF_IFMETRIC_INDEX *p_index, u_int cmd, void *var)
{
	return _ospfIfMetricSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

/*virtual interface*/
STATUS ospfVifGetFirst(tOSPF_VIF_INDEX *p_index)
{
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.vif_table, p_if, p_next_if)
	{
		ospf_nm_process_check(p_if->p_process);
		p_index->process_id = p_if->p_process->process_id;
		p_index->area = p_if->p_transit_area->id;
		p_index->nbr = p_if->nbr;
		rc = OK;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfVifGetNext(tOSPF_VIF_INDEX *p_index, tOSPF_VIF_INDEX *p_nextindex)
{
	struct ospf_area transit;
	struct ospf_if search_if;
	struct ospf_if *p_if = NULL;
	struct ospf_process start_process;
	STATUS rc = ERROR;

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);
	search_if.p_process = &start_process;
	search_if.nbr = p_index->nbr;
	search_if.p_transit_area = &transit;
	transit.id = p_index->area;
	for_each_node_greater(&ospf.nm.vif_table, p_if, &search_if)
	{
		ospf_nm_process_check(p_if->p_process);
		rc = OK;
		p_nextindex->process_id = p_if->p_process->process_id;
		p_nextindex->area = p_if->p_transit_area->id;
		p_nextindex->nbr = p_if->nbr;
		break;
	}

	ospf_semgive();

	return rc;
}

STATUS ospfVifGetApi(tOSPF_VIF_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_area transit;
	struct ospf_if search_if;
	struct ospf_process start_process;
	struct ospf_if *p_if = NULL;
	u_int *lval = (u_int *) var;
	tlv_t *oct = (tlv_t *) var;
	STATUS rc = OK;

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);
	search_if.p_process = &start_process;
	search_if.nbr = p_index->nbr;
	search_if.p_transit_area = &transit;
	transit.id = p_index->area;
	p_if = ospf_lstlookup(&ospf.nm.vif_table, &search_if);
	if (NULL == p_if)
	{
		rc = ERROR;
		goto FINISH;
	}
	switch (cmd)
	{
	case OSPF_VIF_AREAID:
		*lval = p_index->area;
		break;

	case OSPF_VIF_NEIGHBOR:
		*lval = p_index->nbr;
		break;

	case OSPF_VIF_TRANSIT_DELAY:
		*lval = p_if->tx_delay;
		break;

	case OSPF_VIF_RETRANSMITINTERVAL:
		*lval = p_if->rxmt_interval;
		break;

	case OSPF_VIF_HELLOINTERVAL:
		*lval = p_if->hello_interval;
		break;

	case OSPF_VIF_DEADINTERVAL:
		*lval = p_if->dead_interval;
		break;

	case OSPF_VIF_STATE:
		*lval = p_if->state;
		break;

	case OSPF_VIF_EVENT:
		*lval = p_if->stat->state_change;
		break;

	case OSPF_VIF_AUTHKEY:
	{
		tlv_t *octet = (tlv_t *) var;
		if (octet->len > p_if->keylen)
		{
			octet->len = p_if->keylen;
		}

		if (0 != octet->len)
		{
			memcpy(octet->data, p_if->key, octet->len);
		}
		break;
	}

	case OSPF_VIF_AUTHKEYID:
		*lval = p_if->md5id;
		break;

	case OSPF_VIF_STATUS:
		*lval = SNMP_ACTIVE;
		break;

	case OSPF_VIF_AUTHTYPE:
		*lval = p_if->authtype;
		break;

	case OSPF_VIF_LSACOUNT:
		break;

	case OSPF_VIF_LSACHECKSUM:
		break;
	case OSPF_VIF_NBRSTATE:
		if (ospf_vif_nbr_state_get(p_if, p_index->nbr, lval) != 0)
		{
			rc = ERROR;
		}
		break;

	case OSPF_VIF_TYPE:
		*lval = p_if->type;
		break;

	case OSPF_VIF_ADDR:
		*lval = p_if->addr;
		break;

	case OSPF_VIF_NAME:
		if (oct->len < OSPF_MAX_IFNAME_LEN)
		{
			rc = ERROR;
			break;
		}
		//uspIfNameGetByIndex(p_if->ifnet_uint,oct->pBuf);
		memcpy(oct->data, p_if->name, strlen(p_if->name));
		oct->len = strlen(p_if->name);
		break;

	case OSPF_VIF_COST:
		*lval = p_if->cost[0];
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS _ospfVifSetApi(tOSPF_VIF_INDEX *p_index, u_int cmd, void *var,
		u_int flag)
{
	struct ospf_process *p_process = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_nbr *p_nbr = NULL;
	u_int lval = 0;
	STATUS rc = OK;

	ospf_semtake_try();

	p_process = ospf_get_nm_process(p_index);
	if (NULL == p_process)
	{
		rc = ERROR;
		goto FINISH;
	}

	p_if = ospf_vif_lookup(p_process, p_index->area, p_index->nbr);
	if ((NULL == p_if) && (OSPF_VIF_STATUS != cmd))
	{
		rc = ERROR;
		goto FINISH;
	}

	lval = *(u_int *) var;

	switch (cmd)
	{
	case OSPF_VIF_TRANSIT_DELAY:
		p_if->tx_delay = lval;
		break;

	case OSPF_VIF_RETRANSMITINTERVAL:
		p_if->rxmt_interval = lval;
		p_nbr = ospf_lstfirst(&p_if->nbr_table);
		if (p_nbr)
		{
			if (ospf_timer_active(&p_nbr->lsrxmt_timer))
			{
				ospf_stimer_start(&p_nbr->lsrxmt_timer, p_if->rxmt_interval + 1);
			}
			if (ospf_timer_active(&p_nbr->request_timer))
			{
				ospf_stimer_start(&p_nbr->request_timer,
						p_if->rxmt_interval + 1);
			}
		}
		break;

	case OSPF_VIF_HELLOINTERVAL:
		p_if->hello_interval = lval;
		ospf_stimer_start(&p_if->hello_timer, p_if->hello_interval)
		;
		break;

	case OSPF_VIF_DEADINTERVAL:
		p_if->dead_interval = lval;
		p_nbr = ospf_lstfirst(&p_if->nbr_table);
		if (p_nbr)
		{
			ospf_stimer_start(&p_nbr->hold_timer, p_if->dead_interval);
		}
		break;

	case OSPF_VIF_AUTHKEY:
	{
		tlv_t *octet = (tlv_t *) var;
		u_int len =
				(octet->len > OSPF_MD5_KEY_LEN + 4) ?
						(OSPF_MD5_KEY_LEN + 4) : octet->len;
		/*md5 key use the max 16bytes,it cover simple password*/
		bzero(p_if->key, OSPF_MD5_KEY_LEN + 4);
		bcopy(octet->data, p_if->key, len);
		p_if->keylen = len;
		flag |= USP_SYNC_OCTETDATA;
		break;
	}

	case OSPF_VIF_AUTHKEYID:
		p_if->md5id = lval;
		break;

	case OSPF_VIF_STATUS:
		switch (lval)
		{
		case SNMP_ACTIVE:
		case SNMP_CREATEANDGO:
		case SNMP_CREATEANDWAIT:
			if (NULL == p_if)
			{
				ospf_virtual_if_create(p_process, p_index->nbr,
						ospf_area_lookup(p_process, p_index->area));
			}
			break;

		default:
			if (NULL != p_if)
			{
				ospf_if_delete(p_if);
			}
			break;
		}
		break;

	case OSPF_VIF_AUTHTYPE:
		p_if->authtype = lval;
		p_if->maxlen = ospf_if_max_len(p_if);

		if (OSPF_AUTH_NONE == p_if->authtype)
		{
			memset(p_if->key, 0, sizeof(p_if->key));
			p_if->md5id = 0;
		}
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		// uspHwOspfVifSync(p_index, HW_OSPF_VIF_CMDSTART + cmd, var, flag);
	}
	return rc;
}

STATUS ospfVifSetApi(tOSPF_VIF_INDEX *p_index, u_int cmd, void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iRet = 0;
	int iFlg = 0;

	if(cmd == OSPF_VIF_AUTHKEY)
	{
		iFlg |= USP_SYNC_OCTETDATA;
	}

	if((NULL == p_index) || (NULL == var))
	{
		return ERR;
	}

#endif
	return _ospfVifSetApi(p_index, cmd, var, (USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfVifSyncApi(tOSPF_VIF_INDEX *p_index, u_int cmd, void *var)
{
	return _ospfVifSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

/*nbr*/
STATUS ospfNbrGetFirst(tOSPF_NBR_INDEX *p_index)
{
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_nbr *p_next_nbr = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();

	for_each_node(&ospf.nm.nbr_table, p_nbr, p_next_nbr)
	{
		if ((p_nbr->p_if == NULL) || (p_nbr->p_if->p_process == NULL))
		{
			break;
		} ospf_nm_process_check(p_nbr->p_if->p_process);
		p_index->process_id = p_nbr->p_if->p_process->process_id;
		p_index->ipaddr = p_nbr->addr;
		p_index->addrlessif = 0;
		rc = OK;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfNbrGetNext(tOSPF_NBR_INDEX *p_index, tOSPF_NBR_INDEX *p_next_index)
{
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_process start_process;
	struct ospf_if search_if;
	struct ospf_nbr search_nbr;
	STATUS rc = ERROR;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search_if.p_process = &start_process;
	search_nbr.p_if = &search_if;
	search_nbr.addr = p_index->ipaddr;
	for_each_node_greater(&ospf.nm.nbr_table, p_nbr, &search_nbr)
	{
		if ((p_nbr->p_if == NULL) || (p_nbr->p_if->p_process == NULL))
		{
			break;
		} ospf_nm_process_check(p_nbr->p_if->p_process);
		rc = OK;
		p_next_index->process_id = p_nbr->p_if->p_process->process_id;
		p_next_index->ipaddr = p_nbr->addr;
		p_next_index->addrlessif = 0;
		break;
	}
	ospf_semgive();
	return rc;
}

STATUS ospfNbrGetSelect(tOSPF_NBR_INDEX *p_index, tOSPF_NBR_INDEX *p_next_index)
{
	STATUS rc = OK;
	if (p_index == NULL)
	{
		rc = ospfNbrGetFirst(p_next_index);
	}
	else
	{
		rc = ospfNbrGetNext(p_index, p_next_index);
	}
	return rc;
}

STATUS _ospfNbrSetApi(tOSPF_NBR_INDEX *p_index, u_int cmd, void *var,
		u_int flag)
{
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_if *p_if = NULL;
	u_int lval = *(u_int *) var;
	STATUS rc = OK;

	ospf_semtake_try();

	p_process = ospf_get_nm_process(p_index);
	if (NULL == p_process)
	{
		rc = ERROR;
		goto FINISH;
	}
	/*decide interface*/
	p_if = ospf_if_lookup_by_network(p_process, p_index->ipaddr);
	if ((NULL == p_if) || (OSPF_IFT_NBMA != p_if->type))
	{
		rc = ERROR;
		goto FINISH;
	}

	p_nbr = ospf_nbr_lookup(p_if, p_index->ipaddr);
	if ((NULL == p_nbr) && (OSPF_NBR_STATUS != cmd))
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_NBR_STATUS:
		switch (lval)
		{
		case SNMP_ACTIVE:
		case SNMP_CREATEANDGO:
		case SNMP_CREATEANDWAIT:
			if (NULL == p_nbr)
			{
				p_nbr = ospf_nbr_create(p_if, p_index->ipaddr);
			}
			break;

		case SNMP_NOTINSERVICE:
		case SNMP_DESTROY:
			if (NULL != p_nbr)
			{
				ospf_nbr_delete(p_nbr);
			}
			break;

		default:
			break;
		}
		break;

	case OSPF_NBR_PRIORITY:
		p_nbr->priority = lval;
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		// uspHwOspfNbrSync(p_index, HW_OSPF_NBR_CMDSTART + cmd, var, flag);
	}
	return rc;
}

STATUS ospfNbrSetApi(tOSPF_NBR_INDEX *p_index, u_int cmd, void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iRet = 0;
	int iFlg = 0;

	if((NULL == p_index) || (NULL == var))
	{
		return ERR;
	}

#endif
	return _ospfNbrSetApi(p_index, cmd, var, (USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfNbrSyncApi(tOSPF_NBR_INDEX *p_index, u_int cmd, void *var)
{
	return _ospfNbrSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

STATUS ospfNbrGetApi(tOSPF_NBR_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_process start_process;
	struct ospf_if search_if;
	struct ospf_nbr search_nbr;
	u_int *lval = (u_int *) var;
	u_char *chval = (char *) var;
	STATUS rc = OK;

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);
	search_if.p_process = &start_process;
	search_nbr.p_if = &search_if;
	search_nbr.addr = p_index->ipaddr;

	p_nbr = ospf_lstlookup(&ospf.nm.nbr_table, &search_nbr);
	if (NULL == p_nbr)
	{
		rc = ERROR;
		goto FINISH;
	}
	switch (cmd)
	{
	case OSPF_NBR_IPADDR:
		*lval = p_nbr->addr;
		break;

	case OSPF_NBR_IPADDRLESSINDEX:
		*lval = 0;
		break;

	case OSPF_NBR_ROUTERID:
		*lval = p_nbr->id;
		break;

	case OSPF_NBR_OPTION:
		*lval = p_nbr->option;
		break;

	case OSPF_NBR_PRIORITY:
		*lval = p_nbr->priority;
		break;

	case OSPF_NBR_STATE:
		*lval = p_nbr->state;
		break;

	case OSPF_NBR_EVENT:
		*lval = p_nbr->events;
		break;

	case OSPF_NBR_RETRANSQLEN:
		*lval = p_nbr->rxmt_count;
		break;

	case OSPF_NBR_STATUS:
		*lval = SNMP_ACTIVE;
		break;

	case OSPF_NBR_NBMAPERMANENCE:
		*lval = (OSPF_IFT_NBMA == p_nbr->p_if->type) ?
				OSPF_NBMA_PERMANENT : OSPF_NBMA_DYNMAIC;
		break;

	case OSPF_NBR_HELLOSUPRESS:
		*lval = FALSE;
		break;

	case OSPF_NBR_RESTARTHELPERSTATUS:
		*lval = p_nbr->in_restart ?
				OSPF_RESTART_HELPING : OSPF_RESTART_NOT_HELPING;
		break;

	case OSPF_NBR_RESTARTHELPERAGE:
		*lval = 0;
		if (p_nbr->in_restart)
		{
			*lval = ospf_timer_remain(&p_nbr->restart_timer,
					ospf_sys_ticks())/OSPF_TICK_PER_SECOND;
		}
		break;

	case OSPF_NBR_RESTARTHELPEREXITREASON:
		*lval = p_nbr->restart_exitreason;
		break;

	case OSPF_NBR_DEADTIMER:
		*lval = ospf_timer_remain(&p_nbr->hold_timer,
				ospf_sys_ticks())/OSPF_TICK_PER_SECOND;
		break;

	case OSPF_NBR_UPTIME:
		if (OSPF_NS_FULL != p_nbr->state)
		{
			*lval = 0;
			break;
		}
		*lval = os_system_tick() - p_nbr->full_time;
		break;

#ifdef HAVE_BFD
		case OSPF_NBR_BFDSESSION:
		*lval = p_nbr->bfd_discribe;
		break;
#endif
	case OSPF_NBR_IFUNIT:
		*lval = p_nbr->p_if->ifnet_uint;
		break;
	case OSPF_NBR_IFNAME:
		/*              if(zebra_if_get_api(p_nbr->p_if->ifnet_uint,ZEBRA_IF_NAME,chval) == ERROR)
		 {
		 break;
		 }
		 */
		//memcpy(chval,p_nbr->p_if->name,strlen(p_nbr->p_if->name));
		break;
	case OSPF_NBR_DD_STATE:
		*lval = p_nbr->dd_state;
		break;
	case OSPF_NBR_DR:
		*lval = p_nbr->dr;
		break;
	case OSPF_NBR_BDR:
		*lval = p_nbr->bdr;
		break;
	case OSPF_NBR_AREAID:
		*lval = p_nbr->p_if->p_area->id;
		break;
	case OSPF_NBR_IFIPADDR:
		*lval = p_nbr->p_if->addr;
		break;
	case OSPF_NBR_RXMTINTERVAL:
		*lval = p_nbr->p_if->rxmt_interval;
		break;
	case OSPF_NBR_AUTHSEQNUM:
		memcpy(chval, p_nbr->auth_seqnum, strlen(p_nbr->auth_seqnum));
		break;
	case OSPF_NBR_MTU:
		*lval = p_nbr->p_if->mtu;
		break;
	case OSPF_NBR_RESTARTSTATUS:
		*lval = p_nbr->in_restart;
		break;
	default:
		rc = ERROR;
		break;
	}
	FINISH:

	ospf_semgive();
	return rc;
}

STATUS ospfVnbrGetFirst(tOSPF_VIF_INDEX *p_index)
{
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;
	struct ospf_nbr *p_nbr = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.vif_table, p_if, p_next_if)
	{
		ospf_nm_process_check(p_if->p_process);
		p_nbr = ospf_lstfirst(&p_if->nbr_table);
		if (p_nbr)
		{
			p_index->process_id = p_if->p_process->process_id;
			p_index->area = p_if->p_transit_area->id;
			p_index->nbr = p_nbr->id;

			rc = OK;
			break;
		}
	}

	ospf_semgive();
	return rc;
}

STATUS ospfVnbrGetNext(tOSPF_VIF_INDEX *p_index, tOSPF_VIF_INDEX *p_nextindex)
{
	struct ospf_if *p_if = NULL;
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_if search_if;
	struct ospf_area transit;
	struct ospf_process start_process;
	STATUS rc = ERROR;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search_if.p_process = &start_process;
	search_if.nbr = p_index->nbr;
	search_if.p_transit_area = &transit;
	transit.id = p_index->area;
	for_each_node_greater(&ospf.nm.vif_table, p_if, &search_if)
	{
		ospf_nm_process_check(p_if->p_process);
		p_nbr = ospf_lstfirst(&p_if->nbr_table);
		if (p_nbr)
		{
			rc = OK;
			p_nextindex->process_id = p_if->p_process->process_id;
			p_nextindex->area = p_if->p_transit_area->id;
			p_nextindex->nbr = p_if->nbr;
			break;
		}
	}

	ospf_semgive();

	return rc;
}

STATUS ospfVnbrGetApi(tOSPF_VIF_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_area transit;
	struct ospf_if search_if;
	struct ospf_process start_process;
	struct ospf_if *p_if = NULL;
	struct ospf_nbr *p_nbr = NULL;
	u_int *lval = (u_int *) var;
	STATUS rc = OK;

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);
	search_if.p_process = &start_process;
	search_if.nbr = p_index->nbr;
	search_if.p_transit_area = &transit;
	transit.id = p_index->area;
	p_if = ospf_lstlookup(&ospf.nm.vif_table, &search_if);
	if (NULL == p_if)
	{
		rc = ERROR;
		goto FINISH;
	}
	p_nbr = ospf_nbr_first(p_if);
	if (NULL == p_nbr)
	{
		//rc = EOSPF_NONEIGH;
		goto FINISH;
	}
	switch (cmd)
	{
	case OSPF_VNBR_AREAID:
		*lval = p_index->area;
		break;

	case OSPF_VNBR_ROUTERID:
		*lval = p_index->nbr;
		break;

	case OSPF_VNBR_IPADDR:
		*lval = p_nbr->addr;
		break;

	case OSPF_VNBR_OPTION:
		*lval = p_nbr->option;
		break;

	case OSPF_VNBR_STATE:
		*lval = p_nbr->state;
		break;

	case OSPF_VNBR_EVENT:
		*lval = p_nbr->events;
		break;

	case OSPF_VNBR_RETRANSQLEN:
		*lval = p_nbr->rxmt_count;
		break;

	case OSPF_VNBR_HELLOSUPRESS:
		*lval = FALSE;
		break;

	case OSPF_VNBR_RESTARTHELPERSTATUS:
		break;

	case OSPF_VNBR_RESTARTHELPERAGE:
		break;

	case OSPF_VNBR_RESTARTHELPEREXITREASON:
		break;

	default:
		rc = ERROR;
		break;
	}

	FINISH:
	ospf_semgive();
	return rc;
}

STATUS ospfNetworkGetFirst(tOSPF_NETWORK_INDEX *p_index)
{
	struct ospf_network *p_net = NULL;
	struct ospf_network *p_next = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.network_table, p_net, p_next)
	{
		ospf_nm_process_check(p_net->p_process);
		p_index->process_id = p_net->p_process->process_id;
		p_index->area = p_net->area_id;
		p_index->network = p_net->dest;
		p_index->mask = p_net->mask;
		rc = OK;
		break;
	}
	ospf_semgive();
	return rc;
}

STATUS ospfNetworkGetNext(tOSPF_NETWORK_INDEX *p_index,
		tOSPF_NETWORK_INDEX *p_nextindex)
{
	struct ospf_network search;
	struct ospf_network *p_network = NULL;
	struct ospf_process start_process;
	STATUS rc = ERROR;

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);
	search.p_process = &start_process;
	search.area_id = p_index->area;
	search.dest = p_index->network;
	search.mask = p_index->mask;
	for_each_node_greater(&ospf.nm.network_table, p_network, &search)
	{
		ospf_nm_process_check(p_network->p_process);
		rc = OK;
		p_nextindex->process_id = p_network->p_process->process_id;
		p_nextindex->area = p_network->area_id;
		p_nextindex->network = p_network->dest;
		p_nextindex->mask = p_network->mask;
		break;
	}
	ospf_semgive();
	return rc;
}

#define MAX_IFADDR_COUNT 40

/*network setting api*/
STATUS _ospfNetworkSetApi(tOSPF_NETWORK_INDEX *p_index, u_int cmd, void *var,
		u_int flag)
{
	struct ospf_area *p_area = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;
	struct ospf_network *p_network = NULL;
	struct ospf_network *p_next = NULL;
	STATUS rc = OK;
	STATUS ifrc = OK;
	u_int if_unit = 0;
	u_int addr = 0;
	u_int mask = 0;
	u_int uiLoopBackIndex = 0;
	struct prefix_ipv4 stPrefixIp =
	{ 0 };

	ospf_semtake_try();

	p_process = ospf_get_nm_process(p_index);

	if (NULL == p_process)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_NETWORK_STATUS:
		if (TRUE == *(u_int*) var)
		{
			ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS \r\n");
			if (NULL
					== ospf_network_add(p_process, p_index->network,
							p_index->mask, p_index->area))
			{
				ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS NULL == ospf\r\n");
				ospf_semgive();
				rc = ERROR;
				break;
			}

			p_area = ospf_area_lookup(p_process, p_index->area);
			if (NULL == p_area)
			{
				p_area = ospf_area_create(p_process, p_index->area);
				if (p_area == NULL)
				{
					ospf_semgive();
					rc = ERROR;
				}
			}
#ifdef OSPF_VPN
			ospf_sys_netmaskbingvrf(p_process->process_id, p_index->network,
					p_index->mask);
#endif

			//����ϵͳ��ַ��ƥ��
			for (ifrc = ospf_sys_ifaddr_first(p_process->vrid, &if_unit, &addr,
					&mask);
			ERROR != ifrc;
					ifrc = ospf_sys_ifaddr_next(p_process->vrid, &if_unit,
							&addr, &mask))
			{
				/*ignore invalid interfaces*/
				if (!ospf_netmatch(addr, p_index->network,
						p_index->mask) || ifrc == OSPF_VRF_NOMATCH)
				{
					ospf_logx(ospf_debug,
							"OSPF_NETWORK_STATUS NULL addr=0x%x,net=0x%x,mask=0x%x.\r\n",
							addr, p_index->network, p_index->mask);
					continue;
				}
				if (addr == 0
#ifdef OSPF_DCN
						&& p_process->process_id != OSPF_DCN_PROCESS
#endif
						)
				{
					continue;
				}
#ifdef OSPF_DCN
				if(p_process->process_id == OSPF_DCN_PROCESS)
				{
					if(ospf_if_is_loopback(if_unit))
					{
						continue;
					}
					if(addr == 0)
					{
						//dcnGetApi(&if_unit, DCN_NE_IP, &addr);
						//dcnGetApi(&if_unit, DCN_NE_IP_MASK, &mask);
						if(OK != if_loopback_id_to_index(OSPF_DCN_LOOPBACK_INDEX, &uiLoopBackIndex))
						{
							break;
						}

						if(OK != zebra_if_get_api(uiLoopBackIndex, ZEBRA_IF_IPADRESS_GET, &stPrefixIp))
						{
							break;
						}

						memcpy(&addr, &stPrefixIp.prefix, 4);
						addr = htonl(addr);
						mask = ospf_len2mask(stPrefixIp.prefixlen);
					}
				}
#endif
#if 0//def OSPF_DCN
				//p_if = ospf_if_lookup_by_addr(p_process, addr);
				p_if = ospf_if_lookup_unnumber(p_process, addr, if_unit);
#else
				//  	printf("%s %d  ****ospf_if_lookup*******\n", __FUNCTION__,__LINE__);
#ifdef OSPF_DCN
				if(p_process->process_id != OSPF_DCN_PROCESS)
				{
					p_if = ospf_if_lookup_forDcnCreat(p_process, addr, if_unit);

				}
				else
#endif
				{
					p_if = ospf_if_lookup(p_process, addr);
				}
#endif
				ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS  p_if=%p.\r\n",
						p_if);
				/*if interface exist,and interface has area configured,do nothing*/
				if (NULL != p_if)
				{
					if (NULL == p_if->p_area)
					{
						ospf_logx(ospf_debug,
								"OSPF_NETWORK_STATUS NULL == p_if->p_area.\r\n");
						p_if->p_area = p_area;
						p_if->state = OSPF_IFS_DOWN;
						//ospf_lstadd_unsort(&p_area->if_table, p_if);
						ospf_lstadd(&p_area->if_table, p_if);
					}
				}
				else
				{
					/*create interface with area*/
					ospf_logx(ospf_debug,
							"OSPF_NETWORK_STATUS real_if_cr.\r\n");
#ifdef OSPF_DCN
					if(p_process->process_id == OSPF_DCN_PROCESS)
					{
						ospf_Dcn_real_if_create(p_process, if_unit, addr, mask, p_area);
						ospf_real_if_create(p_process, addr, p_area);
					}
					else
#endif
					{
						ospf_real_if_create(p_process, addr, p_area);
					}
				}
			}
		}
		else
		{
			ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS del\r\n");
			p_network = ospf_network_lookup(p_process, p_index->network,
					p_index->mask);
			if ((NULL == p_network) || (p_network->area_id != p_index->area))
			{
				ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS area=%d.\r\n",
						p_index->area);
				rc = ERROR;
				break;
			}
			ospf_network_delete(p_network);
			ospf_logx(ospf_debug,
					"OSPF_NETWORK_STATUS network_delete area=%d.\r\n",
					p_index->area);

			/*clear related interface */
			for_each_ospf_if(p_process, p_if, p_next_if)
			{
				/*area  must match*/
				if ((NULL != p_if->p_area)
						&& (p_if->p_area->id == p_index->area)
						&& ospf_netmatch(p_if->addr, p_index->network,
								p_index->mask))
				{
					for_each_node(&p_process->network_table, p_network, p_next)
					{
						if (ospf_netmatch(p_if->addr, p_network->dest,
								p_network->mask)
								&& p_if->p_area->id == p_network->area_id)
						{
							break;
						}
					}
					if (p_network == NULL && p_if->pif_enable != TRUE)
					{
						ospf_if_delete(p_if);
					}
				}
			}
#if 0
			/* if no more network using this area,try to delete this area*/
			if (ospf_network_match_area(p_process, p_index->area) == ERR)
			{
				p_area = ospf_area_lookup(p_process, p_index->area);
				if (p_area && (FALSE == ospf_area_if_exist(p_area)))
				{
					ospf_timer_start(&p_area->delete_timer, 2);
				}
			}
#endif
		}
		break;
	case OSPF_NETWORK_AREA_STATUS:
		if (TRUE == *(u_int*) var)
		{
			ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS \r\n");
			if (NULL
					== ospf_network_add(p_process, p_index->network,
							p_index->mask, p_index->area))
			{
				ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS NULL == ospf\r\n");
				//rc = EOSPF_NONETORAREANOMATCH;
				break;
			}

			p_area = ospf_area_lookup(p_process, p_index->area);
			if (NULL == p_area)
			{
				p_area = ospf_area_create(p_process, p_index->area);
				if (p_area == NULL)
				{
					ospf_semgive();
					rc = ERROR;
				}
			}

#ifndef OSPF_VPN
			ospf_sys_netmaskbingvrf(p_process->process_id,p_index->network,p_index->mask);
#endif

			//����ϵͳ��ַ��ƥ��
			for (ifrc = ospf_sys_ifaddr_first(p_process->vrid, &if_unit, &addr,
					&mask);
			ERROR != ifrc;
					ifrc = ospf_sys_ifaddr_next(p_process->vrid, &if_unit,
							&addr, &mask))
			{
				/*ignore invalid interfaces*/
				if (!ospf_netmatch(addr, p_index->network,
						p_index->mask) || ifrc == OSPF_VRF_NOMATCH)
				{
					ospf_logx(ospf_debug,
							"OSPF_NETWORK_STATUS NULL addr=0x%x,net=0x%x,mask=0x%x.\r\n",
							addr, p_index->network, p_index->mask);
					continue;
				}

#if 0//def OSPF_DCN
				//p_if = ospf_if_lookup_by_addr(p_process, addr);
				p_if = ospf_if_lookup_unnumber(p_process, addr, if_unit);
#else
				//  	printf("%s %d  ****ospf_if_lookup*******\n", __FUNCTION__,__LINE__);
				p_if = ospf_if_lookup(p_process, addr);
#endif
				ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS  p_if=%p.\r\n",
						p_if);
				/*if interface exist,and interface has area configured,do nothing*/
				if (NULL != p_if)
				{
					if (NULL == p_if->p_area)
					{
						ospf_logx(ospf_debug,
								"OSPF_NETWORK_STATUS NULL == p_if->p_area.\r\n");
						p_if->p_area = p_area;
						p_if->state = OSPF_IFS_DOWN;
						//ospf_lstadd_unsort(&p_area->if_table, p_if);
						ospf_lstadd(&p_area->if_table, p_if);
					}
				}
				else
				{
					/*create interface with area*/
					ospf_logx(ospf_debug,
							"OSPF_NETWORK_STATUS real_if_cr.\r\n");
					ospf_real_if_create(p_process, addr, p_area);
				}
			}
		}
		else
		{
			ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS del\r\n");
			p_network = ospf_network_lookup(p_process, p_index->network,
					p_index->mask);
			if ((NULL == p_network) || (p_network->area_id != p_index->area))
			{
				ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS area=%d.\r\n",
						p_index->area);
				//rc = EOSPF_NONETORAREANOMATCH;
				break;
			}
			ospf_network_delete(p_network);
			ospf_logx(ospf_debug,
					"OSPF_NETWORK_STATUS network_delete area=%d.\r\n",
					p_index->area);

			/*clear related interface */
			for_each_ospf_if(p_process, p_if, p_next_if)
			{
				/*area  must match*/
				if ((NULL != p_if->p_area)
						&& (p_if->p_area->id == p_index->area)
						&& ospf_netmatch(p_if->addr, p_index->network,
								p_index->mask))
				{
					ospf_if_delete(p_if);
				}
			}

		}
		break;
	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		// uspHwOspfNetworkSync(p_index, HW_OSPF_NETWORK_CMDSTART + cmd, var,  flag);
	}
	return rc;
}

STATUS ospfNetworkSetApi(tOSPF_NETWORK_INDEX *p_index, u_int cmd, void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iRet = 0;
	int iFlg = 0;

	if((NULL == p_index) || (NULL == var))
	{
		return ERR;
	}

	if(0 )
	{
		iFlg |= USP_SYNC_OCTETDATA;
	}
	if(p_index->process_id != OSPF_DCN_PROCESS)
	{

	}
#endif
	return _ospfNetworkSetApi(p_index, cmd, var,
			(USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfNetworkSyncApi(tOSPF_NETWORK_INDEX *p_index, u_int cmd, void *var)
{
	return _ospfNetworkSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

STATUS ospfNetworkGetApi(tOSPF_NETWORK_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_network search;
	struct ospf_process start_process;
	struct ospf_network *p_network = NULL;
	u_int *lval = (u_int*) var;
	STATUS rc = OK;

	ospf_semtake_try();

	start_process.process_id = ospf_nm_process_id(p_index);
	search.p_process = &start_process;
	search.area_id = p_index->area;
	search.dest = p_index->network;
	search.mask = p_index->mask;

	p_network = ospf_lstlookup(&ospf.nm.network_table, &search);
	if (NULL == p_network)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_NETWORK_AREAID:
		*lval = p_index->area;
		break;

	case OSPF_NETWORK_DEST:
		*lval = p_index->network;
		break;

	case OSPF_NETWORK_MASK:
		*lval = p_index->mask;
		break;

	case OSPF_NETWORK_STATUS:
		*lval = (NULL != p_network) ? SNMP_ACTIVE : SNMP_DESTROY;
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS ospfRedisRangeGetFirst(tOSPF_DISTRIBUTE_INDEX *p_index)
{
	struct ospf_redis_range *p_redis;
	struct ospf_redis_range *p_next;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.redis_range_table, p_redis, p_next)
	{
		ospf_nm_process_check(p_redis->p_process);

		p_index->process_id = p_redis->p_process->process_id;
		p_index->proto = p_redis->protocol;
		p_index->dest = p_redis->dest;
		p_index->mask = p_redis->mask;
		rc = OK;
		break;
	}

	ospf_semgive();

	return rc;
}

STATUS ospfRedisRangeGetNext(tOSPF_DISTRIBUTE_INDEX *p_index,
		tOSPF_DISTRIBUTE_INDEX *p_nextindex)
{
	struct ospf_redis_range *p_redis = NULL;
	struct ospf_redis_range search;
	struct ospf_process start_process;
	STATUS rc = ERROR;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);

	search.p_process = &start_process;
	search.dest = p_index->dest;
	search.mask = p_index->mask;
	search.protocol = p_index->proto;
	for_each_node_greater(&ospf.nm.redis_range_table, p_redis, &search)
	{
		ospf_nm_process_check(p_redis->p_process);
		rc = OK;
		p_nextindex->process_id = p_redis->p_process->process_id;
		p_nextindex->proto = p_redis->protocol;
		p_nextindex->dest = p_redis->dest;
		p_nextindex->mask = p_redis->mask;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS _ospfRedisRangeSetApi(tOSPF_DISTRIBUTE_INDEX *p_index, u_int cmd,
		void *var, u_int flag)
{
	struct ospf_redis_range *p_range = NULL;
	struct ospf_process *p_process = NULL;
	u_int lval = *(u_int*) var;
	STATUS rc = OK;

	ospf_semtake_try();
	p_process = ospf_get_nm_process(p_index);
	if (NULL == p_process)
	{
		rc = ERROR;
		goto FINISH;
	}
	p_range = ospf_redis_range_lookup(p_process, p_index->proto, p_index->dest,
			p_index->mask);

	switch (cmd)
	{
	case OSPF_DISTRIBUTERANGE_STATUS:
	{
		switch (lval)
		{
		case SNMP_CREATEANDWAIT:
			if (NULL == p_range)
			{
				p_range = ospf_redis_range_create(p_process, p_index->proto,
						p_index->dest, p_index->mask);
				if (NULL == p_range)
				{
					rc = ERROR;
					goto FINISH;
				}
			}
			break;
		case SNMP_CREATEANDGO:
			if (NULL == p_range)
			{
				p_range = ospf_redis_range_create(p_process, p_index->proto,
						p_index->dest, p_index->mask);
				if (NULL == p_range)
				{
					rc = ERROR;
					goto FINISH;
				}
			}
			ospf_redis_range_up(p_range);
			break;
		case SNMP_ACTIVE:
			if (NULL == p_range)
			{
				rc = ERROR;
				goto FINISH;
			}
			ospf_redis_range_up(p_range);
			break;
		case SNMP_NOTINSERVICE:
		case SNMP_NOTREADY:
			break;

		case SNMP_DESTROY:
			if (p_range)
			{
				ospf_redis_range_down(p_range);
				ospf_redis_range_delete(p_range);
			}
			break;

		default:
			break;
		}
		break;
	}
	case OSPF_DISTRIBUTERANGE_TRANSLATE:
	{
		if (NULL == p_range)
		{
			rc = ERROR;
			goto FINISH;
		}
		if (p_range->no_translate == ((OSPF_MATCH_ADV == lval) ? FALSE : TRUE))
		{
			break;
		}
		p_range->no_translate = (OSPF_MATCH_ADV == lval) ? FALSE : TRUE;
		break;
	}

	default:
		rc = ERROR;
		break;
	}

	FINISH:
	//if (OK == rc)
	{
		//  uspHwOspfRedistributeRangeSync(p_index, HW_OSPF_REDISTRIBUTERANGECMDSTART + cmd, var,  flag);
	}
	ospf_semgive();
	return rc;
}

STATUS ospfRedisRangeSetApi(tOSPF_DISTRIBUTE_INDEX *p_index, u_int cmd,
		void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iRet = 0;
	int iFlg = 0;

	if((NULL == p_index) || (NULL == var))
	{
		return ERR;
	}

#endif
	return _ospfRedisRangeSetApi(p_index, cmd, var,
			(USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfRedisRangeSynApi(tOSPF_DISTRIBUTE_INDEX *p_index, u_int cmd,
		void *var)
{
	return _ospfRedisRangeSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}
STATUS ospfRedisRangeGetApi(tOSPF_DISTRIBUTE_INDEX *p_index, u_int cmd,
		void *var)
{
	struct ospf_redis_range *p_range = NULL;
	struct ospf_redis_range search;
	struct ospf_process start_process;
	u_int *lval = (u_int*) var;
	STATUS rc = OK;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search.p_process = &start_process;
	search.dest = p_index->dest;
	search.mask = p_index->mask;
	search.protocol = p_index->proto;

	p_range = ospf_lstlookup(&ospf.nm.redis_range_table, &search);
	if (NULL == p_range)
	{
		rc = ERROR;
		goto FINISH;
	}
	switch (cmd)
	{
	case OSPF_DISTRIBUTERANGE_PROTO:
		*lval = p_index->proto;
		break;

	case OSPF_DISTRIBUTERANGE_DEST:
		*lval = p_index->dest;
		break;

	case OSPF_DISTRIBUTERANGE_MASK:
		*lval = p_index->mask;
		break;

	case OSPF_DISTRIBUTERANGE_STATUS:
		*lval = SNMP_ACTIVE;
		break;
	case OSPF_DISTRIBUTERANGE_TRANSLATE:
		*lval = p_range->no_translate ? OSPF_MATCH_NOADV : OSPF_MATCH_ADV;
		break;

	default:
		rc = ERROR;
		break;
	}

	FINISH:
	ospf_semgive();
	return rc;
}
STATUS ospfDistributeGetFirst(tOSPF_DISTRIBUTE_INDEX *p_index)
{
	struct ospf_redistribute *p_redis;
	struct ospf_redistribute *p_next;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.redistribute_table, p_redis, p_next)
	{
		ospf_nm_process_check(p_redis->p_process);
		p_index->process_id = p_redis->p_process->process_id;
		p_index->proto = p_redis->protocol;
		p_index->proto_process = p_redis->proto_process;
		rc = OK;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfDistributeGetNext(tOSPF_DISTRIBUTE_INDEX *p_index,
		tOSPF_DISTRIBUTE_INDEX *p_nextindex)
{
	struct ospf_redistribute *p_redis = NULL;
	struct ospf_redistribute search;
	struct ospf_process start_process;
	STATUS rc = ERROR;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search.p_process = &start_process;
	search.protocol = p_index->proto;
	search.proto_process = p_index->proto_process;

	for_each_node_greater(&ospf.nm.redistribute_table, p_redis, &search)
	{
		ospf_nm_process_check(p_redis->p_process);
		rc = OK;
		p_nextindex->process_id = p_redis->p_process->process_id;
		p_nextindex->proto = p_redis->protocol;
		p_nextindex->proto_process = p_redis->proto_process;
		break;
	}

	ospf_semgive();

	return rc;
}

STATUS _ospfDistributeSetApi(tOSPF_DISTRIBUTE_INDEX *p_index, u_int cmd,
		void *var, u_int flag)
{
	struct ospf_redistribute *p_redistribute = NULL;
	struct ospf_process *p_process = NULL;
	u_int lval = 0;
	STATUS rc = OK;
	lval = *(u_int*) var;

	ospf_semtake_try();
	p_process = ospf_get_nm_process(p_index);
	if (NULL == p_process)
	{
		rc = ERROR;
		goto FINISH;
	}
	p_redistribute = ospf_redistribute_lookup(p_process, p_index->proto,
			p_index->proto_process);
	if ((NULL == p_redistribute) && (OSPF_DISTRIBUTE_STATUS != cmd))
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_DISTRIBUTE_COST:
		p_redistribute->metric = lval;
		break;

	case OSPF_DISTRIBUTE_COSTTYPE:
		p_redistribute->type = lval;
		break;

	case OSPF_DISTRIBUTE_ADVERTISE:
		p_redistribute->action = (OSPF_MATCH_ADV == lval) ? TRUE : FALSE;
		break;

	case OSPF_DISTRIBUTE_STATUS:
		switch (lval)
		{
		case SNMP_ACTIVE:
		case SNMP_CREATEANDGO:
		case SNMP_CREATEANDWAIT:
			if (NULL == p_redistribute)
			{
				ospf_redistribute_create(p_process, p_index->proto,
						p_index->proto_process);
			}
			break;

		case SNMP_NOTINSERVICE:
		case SNMP_NOTREADY:
			break;

		case SNMP_DESTROY:
			if (NULL != p_redistribute)
			{
				ospf_redistribute_delete(p_redistribute);
			}
			break;

		default:
			break;
		}
		break;

	case OSPF_DISTRIBUTE_TRANSLATE:
		p_redistribute->no_translate = lval;
		break;

	case OSPF_DISTRIBUTE_TAG:
		p_redistribute->tag = lval;
		break;

	default:
		rc = ERROR;
		break;
	}
	/*prepare redistribute*/
	ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
	p_process->import_update = TRUE;
	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		//  uspHwOspfDistributeSync(p_index, HW_OSPF_DISTRIBUTE_CMDSTART + cmd, var,  flag);
	}
	return rc;
}

STATUS ospfDistributeSetApi(tOSPF_DISTRIBUTE_INDEX *p_index, u_int cmd,
		void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iRet = 0;
	int iFlg = 0;

	if((NULL == p_index) || (NULL == var))
	{
		return ERR;
	}

#endif
	return _ospfDistributeSetApi(p_index, cmd, var,
			(USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfDistributeSyncApi(tOSPF_DISTRIBUTE_INDEX *p_index, u_int cmd,
		void *var)
{
	return _ospfDistributeSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

STATUS ospfDistributeGetApi(tOSPF_DISTRIBUTE_INDEX *p_index, u_int cmd,
		void *var)
{
	struct ospf_redistribute *p_redistribute = NULL;
	struct ospf_redistribute search;
	struct ospf_process start_process;
	u_int *lval = (u_int *) var;
	STATUS rc = OK;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search.p_process = &start_process;
	search.protocol = p_index->proto;
	search.proto_process = p_index->proto_process;

	p_redistribute = ospf_lstlookup(&ospf.nm.redistribute_table, &search);
	if (NULL == p_redistribute)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_DISTRIBUTE_PROTO:
		*lval = p_index->proto;
		break;

	case OSPF_DISTRIBUTE_DEST:
		*lval = p_index->dest;
		break;

	case OSPF_DISTRIBUTE_MASK:
		*lval = p_index->mask;
		break;

	case OSPF_DISTRIBUTE_COST:
		*lval = p_redistribute->metric;
		break;

	case OSPF_DISTRIBUTE_COSTTYPE:
		*lval = p_redistribute->type;
		break;

	case OSPF_DISTRIBUTE_ADVERTISE:
		*lval = ospfvaule_to_nm(p_redistribute->action);
		break;

	case OSPF_DISTRIBUTE_STATUS:
		*lval = SNMP_ACTIVE;
		break;

	case OSPF_DISTRIBUTE_TRANSLATE:
		*lval = p_redistribute->no_translate;
		break;

	case OSPF_DISTRIBUTE_TAG:
		*lval = p_redistribute->tag;
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS ospfNetwrokRouteGetFirst(tOSPF_ROUTE_INDEX *p_index)
{
	struct ospf_route *p_route = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_next = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_ospf_process(p_process, p_next)
	{
		ospf_nm_process_check(p_process);
		p_route = ospf_lstfirst(&p_process->route_table);
		if (NULL != p_route)
		{
			p_index->process_id = p_process->process_id;
			p_index->type = OSPF_ROUTE_NETWORK;
			p_index->dest = p_route->dest;
			p_index->mask = p_route->mask;
			rc = OK;
			break;
		}
	}

	ospf_semgive();

	return rc;
}

STATUS ospfNetwrokRouteGetNext(tOSPF_ROUTE_INDEX *p_index,
		tOSPF_ROUTE_INDEX *p_nextindex)
{
	struct ospf_route *p_route = NULL;
	struct ospf_route search;
	struct ospf_process *p_process = NULL;
	struct ospf_process start_process;
	STATUS rc = ERROR;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	for_each_node_noless(&ospf.process_table, p_process, &start_process)
	{
		ospf_nm_process_check(p_process);

		if (p_process->process_id == p_index->process_id)
		{
			search.type = OSPF_ROUTE_NETWORK;
			search.dest = p_index->dest;
			search.mask = p_index->mask;

			p_route = ospf_lstgreater(&p_process->route_table, &search);
			if (NULL != p_route)
			{
				rc = OK;
				goto FINISH;
			}
		}
		else
		{
			p_route = ospf_lstfirst(&p_process->route_table);
			if (NULL != p_route)
			{
				rc = OK;
				goto FINISH;
			}
		}
	}

	FINISH: if (OK == rc)
	{
		p_nextindex->process_id = p_process->process_id;
		p_nextindex->type = OSPF_ROUTE_NETWORK;
		p_nextindex->mask = p_route->mask;
		p_nextindex->dest = p_route->dest;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfNetwrokRouteGetApi(tOSPF_ROUTE_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_route *p_route = NULL;
	struct ospf_process *p_process = NULL;
	tlv_t *octet = (tlv_t *) var;
	u_int current = 0;
	u_int *lval = (u_int*) var;
	STATUS rc = OK;
	u_int i = 0;
	u_int uiLen = 0;

	ospf_semtake_try();
	p_process = ospf_get_nm_process(p_index);
	if (NULL == p_process)
	{
		rc = ERROR;
		goto FINISH;
	}
	p_route = ospf_route_lookup(&p_process->route_table, p_index->type,
			p_index->dest, p_index->mask);
	if (NULL == p_route)
	{
		rc = ERROR;
		goto FINISH;
	}
	current = p_process->current_route;
	switch (cmd)
	{
	case OSPF_ROUTE_TYPE:
		*lval = p_route->type;
		break;

	case OSPF_ROUTE_MASK:
		*lval = p_route->mask;
		break;

	case OSPF_ROUTE_DEST:
		*lval = p_route->dest;
		break;

	case OSPF_ROUTE_NEXTHOP:
		memset(octet->data, 0, octet->len);
		uiLen = octet->len;

		octet->len = 0;
		if (NULL != p_route->path[current].p_nexthop)
		{
			for (i = 0; i < p_route->path[current].p_nexthop->count; i++)
			{
				if (p_route->path[current].p_nexthop->gateway[i].addr
						&& octet->len + 4 <= uiLen)
				{
					memcpy(octet->data + octet->len,
							&p_route->path[current].p_nexthop->gateway[i].addr,
							4);
					octet->len += 4;
				}
			}
		}
		break;
	case OSPF_ROUTE_NEXTHOP_IFINDEX:
		memset(octet->data, 0, octet->len);
		uiLen = octet->len;
		octet->len = 0;
		if (NULL != p_route->path[current].p_nexthop)
		{
			for (i = 0; i < p_route->path[current].p_nexthop->count; i++)
			{
				if (p_route->path[current].p_nexthop->gateway[i].addr
						&& octet->len + 4 <= uiLen)
				{
					memcpy(octet->data + octet->len,
							&p_route->path[current].p_nexthop->gateway[i].if_uint,
							4);
					octet->len += 4;
				}
			}
		}
		break;

	case OSPF_ROUTE_PATHTYPE:
		*lval = p_route->path[current].type;
		break;

	case OSPF_ROUTE_INTRA:
		if (p_route->path[current].type == OSPF_PATH_INTRA)
		{
			*lval = p_route->path[current].intra_type;
		}
		break;

	case OSPF_ROUTE_BACKUPNEXTHOP:
	{
#ifdef OSPF_FRR
		struct ospf_backup_route *p_broute = NULL;
		struct ospf_backup_route broute;

		broute.type = OSPF_ROUTE_NETWORK;
		broute.dest = p_route->dest;
		broute.mask = p_route->mask;
		broute.nexthop = p_index->nexthop;
		p_broute = ospf_lstlookup(&p_process->backup_route_table, &broute);
		if (p_broute)
		{
			*lval = p_broute->path[p_process->backup_current_route].nexthop;
		}
		else
		{
			*lval = 0;
		}
#else
		*lval = 0;
#endif
	}
		break;

	case OSPF_ROUTE_COST:
		*lval = p_route->path[current].cost;
		break;

	case OSPF_ROUTE_COST2:
		*lval = p_route->path[current].cost2;
		break;

	case OSPF_ROUTE_EXT_TYPE:
		*lval = p_route->path[current].type;
		break;

	case OSPF_ROUTE_TAG:
		*lval = p_route->path[current].tag;
		break;
	case OSPF_ROUTE_ADV_ID:
		*lval = p_route->path[current].adv_id;
		break;
	case OSPF_ROUTE_AREA:
		*lval = p_route->path[current].p_area->id;
		break;

	case OSPF_ROUTE_INTERFACENAME:
		memset(octet->data, 0, octet->len);
		octet->len = 0;
		if (NULL != p_route->path[current].p_nexthop)
		{
			for (i = 0; i < p_route->path[current].p_nexthop->count; i++)
			{
				if (p_route->path[current].p_nexthop->gateway[i].if_uint)
				{
					memcpy(octet->data + octet->len,
							&p_route->path[current].p_nexthop->gateway[i].if_uint,
							4);
					octet->len += 4;
				}
			}
		}
		break;

	case OSPF_ROUTE_IMPORTASEXTERNAL:
		if (p_route->path[current].p_area->is_nssa)
		{
			*lval = OSPF_AREA_IMPORT_NSSA;
		}
		else
		{
			*lval = 0;
		}

		break;
	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS ospAbrRouteGetFirst(tOSPF_ROUTE_INDEX *p_index)
{
	struct ospf_route *p_route = NULL;
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_next_area = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.area_table, p_area, p_next_area)
	{
		ospf_nm_process_check(p_area->p_process);
		p_route = ospf_lstfirst(&p_area->abr_table);
		if (NULL != p_route)
		{
			p_index->process_id = p_area->p_process->process_id;
			p_index->type = OSPF_ROUTE_ABR;
			p_index->dest = p_route->dest;
			p_index->mask = p_route->mask;
			p_index->areaid = p_area->id;
			rc = OK;
			break;
		}
	}

	ospf_semgive();

	return rc;
}

STATUS ospfAbrRouteGetNext(tOSPF_ROUTE_INDEX *p_index,
		tOSPF_ROUTE_INDEX *p_nextindex)
{
	struct ospf_route *p_route = NULL;
	struct ospf_route search;
	struct ospf_area search_area;
	struct ospf_area *p_area = NULL;
	struct ospf_process start_process;
	STATUS rc = ERROR;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.p_process = &start_process;
	search_area.id = p_index->areaid;

	search.type = OSPF_ROUTE_ABR;
	search.dest = p_index->dest;
	search.mask = p_index->mask;

	p_area = ospf_lstlookup(&ospf.nm.area_table, &search_area);
	if (p_area)
	{
		p_route = ospf_lstgreater(&p_area->abr_table, &search);
		if (p_route)
		{
			rc = OK;
			goto FINISH;
		}
	}

	for_each_node_greater(&ospf.nm.area_table, p_area, &search_area)
	{
		ospf_nm_process_check(p_area->p_process);
		p_route = ospf_lstfirst(&p_area->abr_table);
		if (NULL != p_route)
		{
			rc = OK;
			goto FINISH;
		}
	}

	FINISH: if (OK == rc)
	{
		p_nextindex->process_id = p_area->p_process->process_id;
		p_nextindex->type = OSPF_ROUTE_ABR;
		p_nextindex->mask = p_route->mask;
		p_nextindex->dest = p_route->dest;
		p_nextindex->areaid = p_area->id;
	}

	ospf_semgive();

	return rc;
}

STATUS ospfAbrRouteGetApi(tOSPF_ROUTE_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_route *p_route = NULL;
	struct ospf_area *p_area = NULL;
	struct ospf_area search_area;
	struct ospf_process start_process;
	tlv_t *octet = (tlv_t *) var;
	STATUS rc = OK;
	u_int *lval = (u_int*) var;
	u_int current;
	u_int i = 0;
	u_int uiLen = 0;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.p_process = &start_process;
	search_area.id = p_index->areaid;
	p_area = ospf_lstlookup(&ospf.nm.area_table, &search_area);
	if (NULL == p_area)
	{
		rc = ERROR;
		goto FINISH;
	}

	current = p_area->p_process->current_route;
	p_route = ospf_abr_lookup(p_area, p_index->dest);
	if (NULL == p_route)
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_ROUTE_TYPE:
		*lval = p_route->type;
		break;

	case OSPF_ROUTE_MASK:
		*lval = p_route->mask;
		break;

	case OSPF_ROUTE_DEST:
		*lval = p_route->dest;
		break;

	case OSPF_ROUTE_NEXTHOP:
		memset(octet->data, 0, octet->len);
		uiLen = octet->len;

		octet->len = 0;
		if (NULL != p_route->path[current].p_nexthop)
		{
			for (i = 0; i < p_route->path[current].p_nexthop->count; i++)
			{
				if (p_route->path[current].p_nexthop->gateway[i].addr
						&& octet->len + 4 <= uiLen)
				{
					memcpy(octet->data + octet->len,
							&p_route->path[current].p_nexthop->gateway[i].addr,
							4);
					octet->len += 4;
				}
			}
		}
		break;
	case OSPF_ROUTE_NEXTHOP_IFINDEX:
		memset(octet->data, 0, octet->len);
		uiLen = octet->len;

		octet->len = 0;
		if (NULL != p_route->path[current].p_nexthop)
		{
			for (i = 0; i < p_route->path[current].p_nexthop->count; i++)
			{
				if (p_route->path[current].p_nexthop->gateway[i].addr
						&& octet->len + 4 <= uiLen)
				{
					memcpy(octet->data + octet->len,
							&p_route->path[current].p_nexthop->gateway[i].if_uint,
							4);
					octet->len += 4;
				}
			}
		}
		break;
	case OSPF_ROUTE_PATHTYPE:
		*lval = p_route->path[current].type;
		//*lval &= 0x7FFF;
		break;

	case OSPF_ROUTE_INTRA:
		if (p_route->path[current].type == OSPF_PATH_INTRA)
		{
			*lval = p_route->path[current].intra_type;
		}
		break;

	case OSPF_ROUTE_BACKUPNEXTHOP:
	{
#ifdef OSPF_FRR
		struct ospf_backup_route *p_broute = NULL;
		struct ospf_backup_route broute;

		broute.type = OSPF_ROUTE_ABR;
		broute.dest = p_route->dest;
		broute.mask = OSPF_HOST_MASK;
		broute.nexthop = p_index->nexthop;
		p_broute = ospf_lstlookup(&p_area->backup_abr_table, &broute);
		if (p_broute)
		{
			*lval =
					p_broute->path[p_area->p_process->backup_current_route].nexthop;
		}
		else
		{
			*lval = 0;
		}
#else
		*lval = 0;
#endif
	}
		break;

	case OSPF_ROUTE_COST:
		*lval = p_route->path[current].cost;
		break;

	case OSPF_ROUTE_COST2:
		*lval = p_route->path[current].cost2;
		break;

	case OSPF_ROUTE_EXT_TYPE:
		*lval = p_route->path[current].type;
		break;

	case OSPF_ROUTE_TAG:
		*lval = p_route->path[current].tag;
		break;

	case OSPF_ROUTE_ADV_ID:
		*lval = p_route->path[current].adv_id;
		break;
	case OSPF_ROUTE_AREA:
		*lval = p_route->path[current].p_area->id;
		break;
	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS ospAsbrRouteGetFirst(tOSPF_ROUTE_INDEX *p_index)
{
	struct ospf_route *p_route = NULL;
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_next_area = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.area_table, p_area, p_next_area)
	{
		ospf_nm_process_check(p_area->p_process);
		p_route = ospf_lstfirst(&p_area->asbr_table);
		if (NULL != p_route)
		{
			p_index->process_id = p_area->p_process->process_id;
			p_index->type = OSPF_ROUTE_ASBR;
			p_index->dest = p_route->dest;
			p_index->mask = p_route->mask;
			p_index->areaid = p_area->id;
			rc = OK;
			break;
		}
	}

	ospf_semgive();
	return rc;
}

STATUS ospfAsbrRouteGetNext(tOSPF_ROUTE_INDEX *p_index,
		tOSPF_ROUTE_INDEX *p_nextindex)
{
	struct ospf_route *p_route = NULL;
	struct ospf_route search;
	struct ospf_area search_area;
	struct ospf_area *p_area = NULL;
	struct ospf_process start_process;
	STATUS rc = ERROR;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.p_process = &start_process;
	search_area.id = p_index->areaid;

	search.type = OSPF_ROUTE_ASBR;
	search.dest = p_index->dest;
	search.mask = p_index->mask;

	p_area = ospf_lstlookup(&ospf.nm.area_table, &search_area);
	if (p_area)
	{
		p_route = ospf_lstgreater(&p_area->asbr_table, &search);
		if (p_route)
		{
			rc = OK;
			goto FINISH;
		}
	}

	for_each_node_greater(&ospf.nm.area_table, p_area, &search_area)
	{
		ospf_nm_process_check(p_area->p_process);
		p_route = ospf_lstfirst(&p_area->asbr_table);
		if (NULL != p_route)
		{
			rc = OK;
			goto FINISH;
		}
	}

	FINISH: if (OK == rc)
	{
		p_nextindex->process_id = p_area->p_process->process_id;
		p_nextindex->type = OSPF_ROUTE_ASBR;
		p_nextindex->mask = p_route->mask;
		p_nextindex->dest = p_route->dest;
		p_nextindex->areaid = p_area->id;
	}

	ospf_semgive();

	return rc;
}

STATUS ospfAsbrRouteGetApi(tOSPF_ROUTE_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_route *p_route = NULL;
	struct ospf_area *p_area = NULL;
	struct ospf_area search_area;
	struct ospf_process start_process;
	tlv_t *octet = (tlv_t *) var;
	STATUS rc = OK;
	u_int *lval = (u_int*) var;
	u_int current;
	u_int i = 0;
	u_int uiLen = 0;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search_area.p_process = &start_process;
	search_area.id = p_index->areaid;
	p_area = ospf_lstlookup(&ospf.nm.area_table, &search_area);
	if (NULL == p_area)
	{
		rc = ERROR;
		goto FINISH;
	}

	p_route = ospf_asbr_lookup(p_area->p_process, p_area, p_index->dest);

	if (NULL == p_route)
	{
		rc = ERROR;
		goto FINISH;
	}
	current = p_area->p_process->current_route;
	switch (cmd)
	{
	case OSPF_ROUTE_TYPE:
		*lval = p_route->type;
		break;

	case OSPF_ROUTE_MASK:
		*lval = p_route->mask;
		break;

	case OSPF_ROUTE_DEST:
		*lval = p_route->dest;
		break;

	case OSPF_ROUTE_NEXTHOP:
		memset(octet->data, 0, octet->len);
		uiLen = octet->len;
		octet->len = 0;
		if (NULL != p_route->path[current].p_nexthop)
		{
			for (i = 0; i < p_route->path[current].p_nexthop->count; i++)
			{
				if (p_route->path[current].p_nexthop->gateway[i].addr
						&& octet->len + 4 <= uiLen)
				{
					memcpy(octet->data + octet->len,
							&p_route->path[current].p_nexthop->gateway[i].addr,
							4);
					octet->len += 4;
				}
			}
		}
		break;
	case OSPF_ROUTE_NEXTHOP_IFINDEX:
		memset(octet->data, 0, octet->len);
		uiLen = octet->len;
		octet->len = 0;
		if (NULL != p_route->path[current].p_nexthop)
		{
			for (i = 0; i < p_route->path[current].p_nexthop->count; i++)
			{
				if (p_route->path[current].p_nexthop->gateway[i].addr
						&& octet->len + 4 <= uiLen)
				{
					memcpy(octet->data + octet->len,
							&p_route->path[current].p_nexthop->gateway[i].if_uint,
							4);
					octet->len += 4;
				}
			}
		}
		break;

	case OSPF_ROUTE_PATHTYPE:
		*lval = p_route->path[current].type;
		// *lval &= 0x7FFF;
		break;

	case OSPF_ROUTE_INTRA:
		if (p_route->path[current].type == OSPF_PATH_INTRA)
		{
			*lval = p_route->path[current].intra_type;
		}
		break;

	case OSPF_ROUTE_BACKUPNEXTHOP:
	{
#ifdef OSPF_FRR
		struct ospf_backup_route *p_broute = NULL;
		struct ospf_backup_route broute;

		broute.type = OSPF_ROUTE_ASBR;
		broute.dest = p_route->dest;
		broute.mask = OSPF_HOST_MASK;
		broute.nexthop = p_index->nexthop;
		p_broute = ospf_lstlookup(&p_area->backup_asbr_table, &broute);
		if (p_broute)
		{
			*lval =
					p_broute->path[p_area->p_process->backup_current_route].nexthop;
		}
		else
		{
			*lval = 0;
		}
#else
		*lval = 0;
#endif
	}
		break;
	case OSPF_ROUTE_COST:
		*lval = p_route->path[current].cost;
		break;

	case OSPF_ROUTE_COST2:
		*lval = p_route->path[current].cost2;
		break;

	case OSPF_ROUTE_EXT_TYPE:
		*lval = p_route->path[current].type;
		break;

	case OSPF_ROUTE_TAG:
		*lval = p_route->path[current].tag;
		break;
	case OSPF_ROUTE_ADV_ID:
		*lval = p_route->path[current].adv_id;
		break;
	case OSPF_ROUTE_AREA:
		*lval = p_route->path[current].p_area->id;
		break;
	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS ospfRedisPolicyFirst(tOSPF_REDISPOLICY_INDEX *p_index)
{
	struct ospf_policy *p_policy;
	struct ospf_policy *p_next;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.redistribute_policy_table, p_policy, p_next)
	{
		ospf_nm_process_check(p_policy->p_process);
		p_index->process_id = p_policy->p_process->process_id;
		p_index->proto = p_policy->proto;
		p_index->policy_index = p_policy->policy_index;
		rc = OK;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfRedisPolicyGetNext(tOSPF_REDISPOLICY_INDEX *p_index,
		tOSPF_REDISPOLICY_INDEX *p_nextindex)
{
	STATUS rc = ERROR;
	struct ospf_process start_process;
	struct ospf_policy search;
	struct ospf_policy *p_policy = NULL;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search.p_process = &start_process;
	search.proto = p_index->proto;
	search.policy_index = p_index->policy_index;

	for_each_node_greater(&ospf.nm.redistribute_policy_table, p_policy, &search)
	{
		ospf_nm_process_check(p_policy->p_process);
		rc = OK;
		p_nextindex->process_id = p_policy->p_process->process_id;
		p_nextindex->proto = p_policy->proto;
		p_nextindex->policy_index = p_policy->policy_index;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS _ospfRedisPolicySetApi(tOSPF_REDISPOLICY_INDEX *p_index, u_int cmd,
		void *var, u_int flag)
{
	struct ospf_policy *p_policy = NULL;
	struct ospf_policy *p_tmp_policy = NULL;
	struct ospf_policy *p_next_policy = NULL;
	struct ospf_process *p_process = NULL;
	u_int lval = *(u_int *) var;
	char *pPolicyName = (char *) var;
	u_int is_set = TRUE;
	STATUS rc = OK;

	ospf_semtake_try();
	p_process = ospf_get_nm_process(p_index);
	if (NULL == p_process)
	{
		rc = ERROR;
		goto FINISH;
	}
	p_policy = ospf_redis_policy_lookup(p_process, p_index->policy_index,
			p_index->proto, p_index->proto_process);

	switch (cmd)
	{
	case OSPF_ROUTEPOLICY_STATUS:
		switch (lval)
		{
		case SNMP_NOTINSERVICE:
		case SNMP_DESTROY:
			if (NULL != p_policy)
			{
				p_policy->active = FALSE;
				ospf_redis_policy_delete(p_policy);
				ospf_stimer_start(&p_process->import_timer,
						OSPF_IMPORT_INTERVAL);
				p_process->import_update = TRUE;

#if 0
				if (p_index->proto == M2_ipRouteProto_ospf)
				{
					for_each_node(&p_process->redis_policy_table, p_tmp_policy, p_next_policy)
					{
						if (p_tmp_policy->policy_index == p_index->policy_index)
						{
							continue;
						}
						if (p_tmp_policy->proto == M2_ipRouteProto_ospf)
						{
							is_set = FALSE;
						}
					}
					if (TRUE == is_set)
					{
						ospf_redistribute_set(p_process, M2_ipRouteProto_ospf, FALSE);
					}
				}
#endif
			}
			break;

		case SNMP_ACTIVE:
			if (NULL == p_policy)
			{
				break;
			}
			p_policy->active = TRUE;
			p_policy->policy_type = OSPF_TYPE_RMAP;
			ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL)
			;
			p_process->import_update = TRUE;

#if 0
			if (p_index->proto == M2_ipRouteProto_ospf)
			{
				ospf_redistribute_set(p_process, M2_ipRouteProto_ospf, TRUE);
			}
#endif
			break;

		case SNMP_CREATEANDGO:
		case SNMP_CREATEANDWAIT:
			if (NULL == p_policy)
			{
				ospf_redis_policy_add(p_process, p_index->policy_index,
						p_index->proto, p_index->proto_process);
			}
			break;

		default:
			break;
		}
		break;

	case OSPF_ROUTEPOLICY_MAPNAME:
		if (NULL == p_policy)
		{
			break;
		}
		memcpy(p_policy->policy_name, pPolicyName, OSPF_POLICY_NAME_LEN);
		break;

	default:
		rc = ERROR;
		break;
	}

	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		//  uspHwOspfRedistributePolicySync(p_index, HW_OSPF_REDISTRIBUTEPOLICY_CMDSTART+cmd, var,  flag);
	}
	return rc;
}

STATUS ospfRedisPolicySetApi(tOSPF_REDISPOLICY_INDEX *p_index, u_int cmd,
		void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iRet = 0;
	int iFlg = 0;

	if((NULL == p_index) || (NULL == var))
	{
		return ERR;
	}

#endif
	return _ospfRedisPolicySetApi(p_index, cmd, var,
			(USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfRedisPolicySyncApi(tOSPF_REDISPOLICY_INDEX *p_index, u_int cmd,
		void *var)
{
	return _ospfRedisPolicySetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

STATUS ospfRedisPolicyGetApi(tOSPF_REDISPOLICY_INDEX *p_index, u_int cmd,
		void *var)
{
	struct ospf_policy *p_policy = NULL;
	struct ospf_process start_process;
	struct ospf_policy search;
	STATUS rc = OK;
	u_int *lval = (u_int *) var;
	char *pPolicyName = (char *) var;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	search.p_process = &start_process;
	search.proto = p_index->proto;
	search.policy_index = p_index->policy_index;
	search.proto_process = p_index->proto_process;
	p_policy = ospf_lstlookup(&ospf.nm.redistribute_policy_table, &search);
	if (NULL == p_policy)
	{
		rc = ERROR;
		goto FINISH;
	}
	switch (cmd)
	{
	case OSPF_ROUTEPOLICY_STATUS:
		if (p_policy->active == TRUE)
		{
			*lval = SNMP_ACTIVE;
		}
		else
		{
			*lval = SNMP_CREATEANDGO;
		}
		break;

	case OSPF_ROUTEPOLICY_MAPNAME:
		memcpy(pPolicyName, p_policy->policy_name, OSPF_POLICY_NAME_LEN);
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS ospfFilterPolicyFirst(void *index)
{
	tOSPF_FILTERPOLICY_INDEX *p_policy_index = index;
	struct ospf_policy *p_policy;
	STATUS rc = ERROR;

	ospf_semtake_try();
	p_policy = ospf_lstfirst(&ospf.nm.filter_policy_table);
	if (p_policy)
	{
		p_policy_index->process_id = p_policy->p_process->process_id;
		p_policy_index->policy_index = p_policy->policy_index;
		rc = OK;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfFilterPolicyGetNext(void *index, void *p_nextindex)
{
	struct ospf_policy search;
	struct ospf_policy *p_policy = NULL;
	struct ospf_process start_process;
	STATUS rc = ERROR;
	tOSPF_FILTERPOLICY_INDEX *p_filterindex = index;
	tOSPF_FILTERPOLICY_INDEX *p_nextfilterindex = p_nextindex;

	ospf_semtake_try();

	search.policy_index = p_filterindex->policy_index;
	start_process.process_id = p_filterindex->process_id;
	search.p_process = &start_process;
	p_policy = ospf_lstgreater(&ospf.nm.filter_policy_table, &search);
	if (p_policy)
	{
		rc = OK;
		p_nextfilterindex->process_id = p_policy->p_process->process_id;
		p_nextfilterindex->policy_index = p_policy->policy_index;
	}

	ospf_semgive();

	return rc;
}

static void ospf_nm_extract_policy_index(void *index,
		struct ospf_policy *p_policy, struct ospf_process *p_process)
{
	tOSPF_FILTERPOLICY_INDEX *p_policyindex = index;

	if ((index == NULL) || (p_policy == NULL) || (p_process == NULL))
	{
		return;
	}

	p_process->process_id = p_policyindex->process_id;
	p_policy->policy_index = p_policyindex->policy_index;

	return;
}

STATUS _ospfFilterPolicySetApi(void *index, u_int cmd, void *var, u_int flag)
{
	struct ospf_policy *p_policy = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_policy policy;
	struct ospf_process process;
	u_int lval = *(u_int *) var;
	char *pPolicyName = (char *) var;
	STATUS rc = OK;

	ospf_semtake_try();
	ospf_nm_extract_policy_index(index, &policy, &process);

	p_process = ospf_process_lookup(&ospf, process.process_id);
	if (NULL == p_process)
	{
		rc = ERROR;
		goto FINISH;
	}
	p_policy = ospf_filter_policy_lookup(p_process, policy.policy_index);

	switch (cmd)
	{
	case OSPF_ROUTEPOLICY_STATUS:
		switch (lval)
		{
		case SNMP_NOTINSERVICE:
		case SNMP_DESTROY:
			if (NULL == p_policy)
			{
				break;
			}
			p_policy->active = FALSE;
			if (policy.policy_index == OSPF_IM_POLICY)
			{
				ospf_timer_start(&p_policy->delete_timer, 5);
			}
			else if (policy.policy_index == OSPF_EX_POLICY)
			{
				ospf_filter_policy_delete(p_policy);
				ospf_stimer_start(&p_process->import_timer,
						OSPF_IMPORT_INTERVAL);
				p_process->import_update = TRUE;
			}
			else if (policy.policy_index == OSPF_PRE_POLICY)
			{
				ospf_filter_policy_delete(p_policy);
				p_process->preference_policy = 0;
			}
			else if (policy.policy_index == OSPF_ASE_PRE_POLICY)
			{
				ospf_filter_policy_delete(p_policy);
				p_process->preference_ase_policy = 0;
			}
#if 0
			if (TRUE == p_policy->active)
			{
				ospf_timer_start(&p_policy->delete_timer, 5);
			}
			else
			{
				ospf_filter_policy_delete(p_policy);
			}
#endif
			/*end*/
			break;

		case SNMP_ACTIVE:
			if (NULL == p_policy)
			{
				break;
			}
			p_policy->active = TRUE;
			if (policy.policy_index == OSPF_IM_POLICY)
			{
				ospf_timer_start(&p_policy->update_timer, 5);
			}
			else if (policy.policy_index == OSPF_EX_POLICY)
			{
				ospf_stimer_start(&p_process->import_timer,
						OSPF_IMPORT_INTERVAL);
				p_process->import_update = TRUE;
			}
			//ospf_timer_start(&p_policy->update_timer, 5);
			break;

		case SNMP_CREATEANDGO:
		case SNMP_CREATEANDWAIT:
			if (NULL == p_policy)
			{
				ospf_filter_policy_add(p_process, policy.policy_index);
			}
			break;

		default:
			break;
		}
		break;
	case OSPF_ROUTEPOLICY_TYPE:
		if (NULL == p_policy)
		{
			break;
		}
		p_policy->policy_type = lval;
		break;

	case OSPF_ROUTEPOLICY_MAPNAME:
		if (NULL == p_policy)
		{
			break;
		}
		memcpy(p_policy->policy_name, pPolicyName, OSPF_POLICY_NAME_LEN);
		break;

	default:
		rc = ERROR;
		break;
	}

	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		//  uspHwOspfFilterPolicySync(index, HW_OSPF_FILTERPOLICY_CMDSTART + cmd, var,  flag);
	}
	return rc;
}

STATUS ospfFilterPolicySetApi(void *index, u_int cmd, void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iRet = 0;
	int iFlg = 0;

	if((NULL == index) || (NULL == var))
	{
		return ERR;
	}

#endif
	return _ospfFilterPolicySetApi(index, cmd, var,
			(USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfFilterPolicySyncApi(void *index, u_int cmd, void *var)
{
	return _ospfFilterPolicySetApi(index, cmd, var, USP_SYNC_LOCAL);
}

STATUS ospfFilterPolicyGetApi(void *index, u_int cmd, void *var)
{
	struct ospf_policy *p_policy = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_policy policy;
	struct ospf_process process;
	STATUS rc = OK;
	u_int *lval = (u_int *) var;
	char *pPolicyName = (char *) var;

	ospf_semtake_try();
	ospf_nm_extract_policy_index(index, &policy, &process);

	p_process = ospf_process_lookup(&ospf, process.process_id);
	if (NULL == p_process)
	{
		rc = ERROR;
		goto FINISH;
	}
	p_policy = ospf_filter_policy_lookup(p_process, policy.policy_index);

	switch (cmd)
	{
	case OSPF_ROUTEPOLICY_STATUS:
		*lval = (NULL != p_policy) ? SNMP_ACTIVE : SNMP_DESTROY;
		break;
	case OSPF_ROUTEPOLICY_TYPE:
		if (NULL == p_policy)
		{
			break;
		}
		*lval = p_policy->policy_type;
		break;
	case OSPF_ROUTEPOLICY_MAPNAME:
		if (NULL == p_policy)
		{
			break;
		}
		memcpy(pPolicyName, p_policy->policy_name, OSPF_POLICY_NAME_LEN);
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS ospfShamlinkGetFirst(tOSPF_SHAMLINK_INDEX *p_index)
{
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();
	for_each_node(&ospf.nm.shamlink_table, p_if, p_next)
	{
		ospf_nm_process_check(p_if->p_process);
		p_index->process_id = p_if->p_process->process_id;
		p_index->if_addr = p_if->addr;
		p_index->nbr_addr = p_if->nbr;
		rc = OK;
		break;
	}
	ospf_semgive();
	return rc;
}

STATUS ospfShamlinkGetNext(tOSPF_SHAMLINK_INDEX *p_index,
		tOSPF_SHAMLINK_INDEX *p_nextindex)
{
	struct ospf_if *p_if = NULL;
	struct ospf_if if_search;
	struct ospf_process start_process;
	STATUS rc = ERROR;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	if_search.p_process = &start_process;
	if_search.type = OSPF_IFT_SHAMLINK;
	if_search.addr = p_index->if_addr;
	if_search.nbr = p_index->nbr_addr;

	for_each_node_greater(&ospf.nm.shamlink_table, p_if, &if_search)
	{
		ospf_nm_process_check(p_if->p_process);
		rc = OK;
		p_nextindex->process_id = p_if->p_process->process_id;
		p_nextindex->if_addr = p_if->addr;
		p_nextindex->nbr_addr = p_if->nbr;
		break;
	}

	ospf_semgive();
	return rc;
}

STATUS ospfShamlinkGetApi(tOSPF_SHAMLINK_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_if if_search;
	struct ospf_process start_process;
	struct ospf_if *p_if = NULL;
	STATUS rc = OK;
	u_int *lval = (u_int *) var;

	ospf_semtake_try();
	start_process.process_id = ospf_nm_process_id(p_index);
	if_search.p_process = &start_process;
	if_search.type = OSPF_IFT_SHAMLINK;
	if_search.addr = p_index->if_addr;
	if_search.nbr = p_index->nbr_addr;
	p_if = ospf_lstlookup(&ospf.nm.shamlink_table, &if_search);
	if (NULL == p_if)
	{
		rc = ERROR;
		goto FINISH;
	}
	switch (cmd)
	{
	case OSPF_SHAMLINK_STATUS:
		*lval = SNMP_ACTIVE;
		break;

	case OSPF_SHAMLINK_TRANSIT_DELAY:
		*lval = p_if->tx_delay;
		break;

	case OSPF_SHAMLINK_RETRANSMITINTERVAL:
		*lval = p_if->rxmt_interval;
		break;

	case OSPF_SHAMLINK_HELLOINTERVAL:
		*lval = p_if->hello_interval;
		break;

	case OSPF_SHAMLINK_DEADINTERVAL:
		*lval = p_if->dead_interval;
		break;

	case OSPF_SHAMLINK_AUTHKEY:
		break;

	case OSPF_SHAMLINK_AUTHTYPE:
		*lval = p_if->authtype;
		break;

	case OSPF_SHAMLINK_AUTHKEYID:
		break;

	case OSPF_SHAMLINK_COST:
		*lval = p_if->cost[0];
		break;

	case OSPF_SHAMLINK_AREAID:
		*lval = p_if->p_area->id;
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS _ospfShamlinkSetApi(tOSPF_SHAMLINK_INDEX *p_index, u_int cmd, void *var,
		u_int flag)
{
	struct ospf_process *p_process = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_area *p_area = NULL;
	u_int lval = *(u_int *) var;
	STATUS rc = OK;

	ospf_semtake_try();
	p_process = ospf_get_nm_process(p_index);
	if (NULL == p_process)
	{
		rc = ERROR;
		goto FINISH;
	}
	p_if = ospf_shamlinkif_lookup(p_process, p_index->if_addr,
			p_index->nbr_addr);
	if ((NULL == p_if) && (OSPF_SHAMLINK_STATUS != cmd))
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_SHAMLINK_STATUS:
		switch (lval)
		{
		case SNMP_NOTINSERVICE:
		case SNMP_DESTROY:
			if (NULL != p_if)
			{
				ospf_if_delete(p_if);
			}
			break;

		case SNMP_ACTIVE:
		case SNMP_CREATEANDGO:
		case SNMP_CREATEANDWAIT:
			if (NULL == p_if)
			{
				ospf_shamlink_if_create(p_process, p_index->if_addr,
						p_index->nbr_addr);
			}
			break;

		default:
			break;
		}
		break;

	case OSPF_SHAMLINK_AREAID:
		/*area id can only set once*/
#if 0
		if (NULL != p_if->p_area)
		{
			rc = ERROR;
			goto FINISH;
		}
#endif
		p_area = ospf_area_lookup(p_process, lval);
		if (NULL == p_area)
		{
			rc = ERROR;
			goto FINISH;
		}

		p_if->p_area = p_area;
		p_if->opaque_lstable.p_area = p_area;
		//ospf_lstadd_unsort(&p_if->p_area->if_table, p_if);
		ospf_lstadd(&p_if->p_area->if_table, p_if);

		ospf_ism(p_if, OSPF_IFE_UP);
		break;

	case OSPF_SHAMLINK_TRANSIT_DELAY:
		p_if->tx_delay = lval;
		break;

	case OSPF_SHAMLINK_RETRANSMITINTERVAL:
		p_if->rxmt_interval = lval;
		break;

	case OSPF_SHAMLINK_HELLOINTERVAL:
		p_if->hello_interval = lval;
		break;

	case OSPF_SHAMLINK_DEADINTERVAL:
		p_if->dead_interval = lval;
		break;

	case OSPF_SHAMLINK_AUTHKEY:
		break;

	case OSPF_SHAMLINK_AUTHTYPE:
		p_if->authtype = lval;
		break;

	case OSPF_SHAMLINK_AUTHKEYID:
		break;

	case OSPF_SHAMLINK_COST:
		p_if->cost[0] = lval;
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}

STATUS ospfShamlinkSetApi(tOSPF_SHAMLINK_INDEX *p_index, u_int cmd, void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iRet = 0;
	int iFlg = 0;

	if((NULL == p_index) || (NULL == var))
	{
		return ERR;
	}

#endif
	return _ospfShamlinkSetApi(p_index, cmd, var,
			(USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfShamlinkSyncApi(tOSPF_SHAMLINK_INDEX *p_index, u_int cmd, void *var)
{
	return _ospfShamlinkSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

#if 1
STATUS _ospfUtilSetApi(u_int id, u_int cmd, void *var, u_int flag)
{
	switch (cmd)
	{
	case OSPF_UTIL_GLOBAL:
		ospf_display_global(NULL);

		ospf_display_interface(NULL, 0);
		break;

	default:
		break;
	}

	return;

}

STATUS ospfUtilSetApi(u_int id, u_int cmd, void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iRet = 0;
	int iFlg = 0;

	if(NULL == var)
	{
		return ERR;
	}

#endif
	return _ospfUtilSetApi(id, cmd, var, (USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

#endif

STATUS ospfAsbrSummeryGetFirst(tOSPF_RANGE_INDEX *p_index)
{
	struct ospf_asbr_range *p_range = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();

	p_range = ospf_lstfirst(&ospf.nm.asbr_range_table);
	if (p_range == NULL)
	{
		rc = ERROR;
		ospf_semgive();
		return rc;
	}
	p_index->process_id = p_range->p_process->process_id;
	p_index->dest = p_range->dest;
	p_index->mask = p_range->mask;
	rc = OK;

	ospf_semgive();
	return rc;
}

STATUS ospfAsbrSummeryGetNext(tOSPF_RANGE_INDEX *p_index,
		tOSPF_RANGE_INDEX *p_nextindex)
{
	struct ospf_asbr_range *p_range = NULL;
	struct ospf_asbr_range *p_rangenext = NULL;
	struct ospf_asbr_range search_range;
	struct ospf_process *p_process = NULL;
	STATUS rc = ERROR;

	ospf_semtake_try();

	p_process = ospf_process_lookup(&ospf, p_index->process_id);
	if (p_process == NULL)
	{
		rc = ERROR;
		ospf_semgive();
		return rc;
	}
	search_range.p_process = p_process;
	search_range.dest = p_index->dest;
	search_range.mask = p_index->mask;
	//search_range.lstype = OSPF_LS_SUMMARY_NETWORK;

	p_range = ospf_lstlookup(&ospf.nm.asbr_range_table, &search_range);
	if (p_range == NULL)
	{
		rc = ERROR;
		ospf_semgive();
		return rc;
	}
	p_rangenext = ospf_lstnext(&ospf.nm.asbr_range_table, p_range);
	if (p_rangenext == NULL)
	{
		rc = ERROR;
		ospf_semgive();
		return rc;
	}
	//ospf_nm_process_check(p_range->p_area->p_process);
	//if (OSPF_LS_SUMMARY_NETWORK == p_range->lstype)
	{
		rc = OK;
		p_nextindex->process_id = p_rangenext->p_process->process_id;
		p_nextindex->dest = p_rangenext->dest;
		p_nextindex->mask = p_rangenext->mask;

	}

	ospf_semgive();
	return rc;
}

STATUS ospfAsbrSummeryGetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var)
{
	struct ospf_process *p_process = NULL;
	struct ospf_asbr_range *p_asbr_range = NULL;
	struct ospf_asbr_range stSearch;
	u_int *lval = (u_int *) var;
	STATUS rc = OK;

	ospf_semtake_try();

	p_process = ospf_process_lookup(&ospf, p_index->process_id);
	if (p_process == NULL)
	{
		rc = ERROR;
		goto FINISH;
	}

	stSearch.dest = p_index->dest;
	stSearch.mask = p_index->mask;
	stSearch.p_process = p_process;
	p_asbr_range = ospf_lstlookup(&p_process->asbr_range_table, &stSearch);
	if ((NULL == p_asbr_range) && (OSPF_ASBRRANGE_STATUS != cmd))
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_ADDRRANGE_NET:
		*lval = p_index->dest;
		break;

	case OSPF_ASBRRANGE_MASK:
		*lval = p_index->mask;
		break;

	case OSPF_ASBRRANGE_STATUS:
		*lval = SNMP_ACTIVE;
		break;

	case OSPF_ASBRRANGE_EFFECT:
		*lval = (p_asbr_range->advertise) ? OSPF_MATCH_ADV : OSPF_MATCH_NOADV;
		break;

	case OSPF_ASBRRANGE_COST:
		*lval = p_asbr_range->cost;
		break;

	case OSPF_ASBRRANGE_MATCH:
		*lval = p_asbr_range->rtcount;
		break;

	case OSPF_ASBRRANGE_COUNT:
		*lval = ospf_lstcnt(&p_process->asbr_range_table);
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();
	return rc;
}
/*asbr 的聚合*/
#if 1
STATUS _ospfAsbrSummerySetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var,
		u_int flag)
{
	struct ospf_range *p_rangeT7 = NULL;
	struct ospf_asbr_range *p_rangeT5 = NULL;
	struct ospf_process *p_process = NULL;
	STATUS rc = OK;
	u_int *lval = (u_int *) var;

	ospf_semtake_try();
	p_process = ospf_process_lookup(&ospf, ospf_nm_process_id(p_index));
	if (p_process == NULL)
	{
		rc = ERROR;
		goto FINISH;
	}
	/*asbr type-5 range是否存在*/
	p_rangeT5 = ospf_asbr_range_lookup(p_process, OSPF_LS_AS_EXTERNAL,
			p_index->dest, p_index->mask);

	if ((NULL == p_rangeT5) && (OSPF_ASBRRANGE_STATUS != cmd))
	{
		rc = ERROR;
		goto FINISH;
	}

	switch (cmd)
	{
	case OSPF_ASBRRANGE_STATUS:
		switch (*lval)
		{
		case SNMP_ACTIVE:
		case SNMP_CREATEANDGO:
		case SNMP_CREATEANDWAIT:
			/*asbr type-5 range不存在则创建一条type-5 range*/
			if (p_rangeT5 == NULL)
			{
				p_rangeT5 = ospf_asbr_range_create(p_process, p_index->dest,
						p_index->mask);
				if (NULL == p_rangeT5)
				{
					rc = ERROR;
					break;
				}

			}
			//ospf_asbr_nssa_verify(p_rangeT5, RANGE_CREATE);

			break;

		case SNMP_NOTINSERVICE:
		case SNMP_NOTREADY:
			break;

		case SNMP_DESTROY:
#if 1
			/*asbr type-5 rage 的删除*/
			if (NULL == p_rangeT5)
			{
				rc = ERROR;
				break;
			}
			if (NULL != p_rangeT5)
			{
				/*如果是nssa区域，则要删除区域下type-7的range*/
				ospf_asbr_nssa_verify(p_rangeT5);
				/*删除进程下type-5的range*/
				ospf_asbr_T5_range_down(p_rangeT5);
				ospf_T5_range_delete(p_rangeT5);
				/*启动定时器，计算lsa*/
				ospf_stimer_start(&p_process->import_timer,
						OSPF_IMPORT_INTERVAL);
				//ospf_import_route_update_all
			}

			break;
#endif
		default:
			break;
		}
		break;

	case OSPF_ASBRRANGE_COST:
#if 1
		p_rangeT5->cost = *lval;
		break;
#endif
	case OSPF_ASBRRANGE_EFFECT:
		/*如果是不宣告的方式，需要把type-5的lsa down掉*/
		ospf_advertise_verify(p_rangeT5);
		p_rangeT5->advertise = (OSPF_MATCH_ADV == *lval) ? TRUE : FALSE;
		/*update type-5 lsa*/
		ospf_range_update_asbr_T5(p_rangeT5);
		/*update type-7 lsa*/
		ospf_asbr_nssa_range_update_verify(p_rangeT5);
		break;

	default:
		rc = ERROR;
		break;
	}
	FINISH:
	ospf_semgive();

	//if (OK == rc)
	{
		// uspHwOspfNssaAreaRangeSync(p_index, HW_OSPF_NSSARANGE_CMDSTART + cmd, var,  flag);
	}
	return rc;
}

STATUS ospfAsbrSummerySetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
	int iRet = 0;
	int iFlg = 0;

	if((NULL == p_index) || (NULL == var))
	{
		return ERROR;
	}

#endif
	return _ospfAsbrSummerySetApi(p_index, cmd, var,
			(USP_SYNC_LOCAL | USP_SYNC_REMOTE));
}

STATUS ospfospfAsbrSummerySyncApi(tOSPF_RANGE_INDEX *p_index, u_int cmd,
		void *var)
{
	return _ospfAsbrSummerySetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}
#endif
/* 通过接口索引获取实例ID 和IP*/
int ospf_get_Process_and_If(u_int ulIfunit, u_int *pulProcessId, u_int *pulAddr)
{
	/*process filter*/
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_nprocess = NULL;
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_nextarea = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_nextif = NULL;

	if ((pulAddr == NULL) || (pulProcessId == NULL))
	{
		return ERR;
	}

	for_each_node(&ospf.process_table, p_process, p_nprocess)
	{
		//printf("process_id = %d\n",p_process->process_id);
		//for_each_node(&p_process->area_table, p_area, p_nextarea)
		{
			for_each_ospf_if(p_process, p_if, p_nextif)
			//	for_each_node(&p_area->if_table, p_if, p_nextif)
			{
				//printf("ifunit = %d\n", p_if->ifnet_uint);
				if (ulIfunit == p_if->ifnet_uint)
				{
#ifdef OSPF_DCN
					if(p_process->process_id == OSPF_DCN_PROCESS)
					{
						return ERR;
					}
#endif
					*pulProcessId = p_process->process_id;
					*pulAddr = p_if->addr;
					return OK;

				}
			}
		}
	}
	return ERR;
}

/* 通过接口索引获取实例ID 和区域ID  */
int ospf_get_process_and_area_by_ifaddr(u_int ulIfAddr, u_int *pulProcessId,
		u_int *pulAreaId)
{
	/*process filter*/
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_nprocess = NULL;
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_nextarea = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_nextif = NULL;

	if ((pulProcessId == NULL) || (pulAreaId == NULL))
	{
		return ERR;
	}

	for_each_node(&ospf.process_table, p_process, p_nprocess)
	{
		for_each_node(&p_process->area_table, p_area, p_nextarea)
		{
			for_each_ospf_if(p_process, p_if, p_nextif)
				for_each_node(&p_area->if_table, p_if, p_nextif)
				{
					if (ulIfAddr == p_if->ifnet_uint)
					{
						*pulProcessId = p_process->process_id;
						*pulAreaId = p_area->id;
						return OK;

					}
				}
		}
	}
	return ERR;
}
/*获取已配置下一跳优先级的个数*/
int ospf_nexthop_count(struct ospf_nhop_weight *pnhopw)
{
	int len = 0;

	if (pnhopw == NULL)
	{
		return len;
	}
	while (pnhopw[len].flag)
	{
		len++;
	}
	return len;
}
/*添加下一跳优先级到ospf_nhop_weight结构体数组*/
void ospf_nexthopweight_add(struct ospf_process *p_process, tlv_t *octet)
{
	int i = 0;
	u_long addr = 0;
	u_int weight = 0;

	if (p_process == NULL)
	{
		return;
	}

	memcpy(&addr, octet->data, 4);
	memcpy(&weight, octet->data + 4, 4);
	if ((p_process->nhopw[63].flag == TRUE) && (weight != 0))
	{
		vty_out_to_all_terminal(
				"Error: The number of next hops is above the threshold.(64)");
		return;
	}
	for (i = 0; i < OSPF_NHOP_MAXCOUNT; i++)
	{
		if (p_process->nhopw[i].flag)
		{
			if (addr == p_process->nhopw[i].addr)
			{
				if (weight == 0)
				{
					p_process->nhopw[i].addr = 0;
					p_process->nhopw[i].flag = FALSE;
					p_process->nhopw[i].weight = weight;
					if (i < OSPF_NHOP_MAXCOUNT - 1)
					{
						memcpy(&p_process->nhopw[i], &p_process->nhopw[i + 1],
								sizeof(p_process->nhopw[i + 1])
										* (OSPF_NHOP_MAXCOUNT - i - 1));
					}
					memset(&p_process->nhopw[OSPF_NHOP_MAXCOUNT - 1], 0,
							sizeof(p_process->nhopw[OSPF_NHOP_MAXCOUNT - 1]));
				}
				else
				{
					p_process->nhopw[i].weight = weight;
				}
				break;
			}
		}
		else
		{
			if (weight == 0)
			{
				break;
			}
			p_process->nhopw[i].addr = addr;
			p_process->nhopw[i].weight = weight;
			p_process->nhopw[i].flag = TRUE;
			break;
		}
	}
}
/*获取ospf_nhop_weight结构体数组中所有的下一跳优先级*/
int ospf_nexthopweight_get(struct ospf_process *p_process, tlv_t *octet)
{
	int i = 0;

	if ((p_process == NULL) || (octet == NULL) || (octet->data == NULL)
			|| (octet->len < (ospf_nexthop_count(p_process->nhopw) * 8)))
	{
		return -1;
	}
	for (i = 0; i < OSPF_NHOP_MAXCOUNT; i++)
	{
		if (!p_process->nhopw[i].flag)
		{
			break;
		}
		memcpy(octet->data + (8 * i), &p_process->nhopw[i].addr, 4);
		memcpy(octet->data + (8 * i + 4), &p_process->nhopw[i].weight, 4);
	}
	octet->len = 8 * i;
	return 0;
}

/*if advertise is changed from no-advertise*/
void ospf_advertise_verify(struct ospf_asbr_range *p_rangeT5)
{
	if (p_rangeT5 && (!p_rangeT5->advertise))
	{
		ospf_asbr_T5_range_down(p_rangeT5);
		ospf_asbr_nssa_verify(p_rangeT5);
	}
	return;
}
/*asbr-summary range down for each nssa area */
void ospf_asbr_nssa_verify(struct ospf_asbr_range *p_rangeT5)
{
	struct ospf_process *p_process = NULL;
	struct ospf_area *p_area_nssa = NULL;
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_areanext = NULL;
	struct ospf_range *p_rangeT7 = NULL;
	u_int mask = 0;
	u_int dest = 0;

	if (p_rangeT5 == NULL)
	{
		return;
	}
	p_process = p_rangeT5->p_process;
	mask = p_rangeT5->mask;
	dest = p_rangeT5->dest;

	for_each_node(&p_process->area_table, p_area_nssa, p_areanext)
	{
		if (p_area_nssa->is_nssa)
		{
			ospf_asbr_T7_range_down(p_area_nssa, p_rangeT5);

		}
	}
}
/*asbr-summary range update for each nssa area*/
void ospf_asbr_nssa_range_update_verify(struct ospf_asbr_range *p_rangeT5)
{
	struct ospf_process *p_process = NULL;
	struct ospf_area *p_area_nssa = NULL;
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_areanext = NULL;
	struct ospf_range *p_rangeT7 = NULL;
	u_int mask = 0;
	u_int dest = 0;

	if (p_rangeT5 == NULL)
	{
		return;
	}
	p_process = p_rangeT5->p_process;
	mask = p_rangeT5->mask;
	dest = p_rangeT5->dest;

	for_each_node(&p_process->area_table, p_area_nssa, p_areanext)
	{
		if (p_area_nssa->is_nssa)
		{
			/*update nssa区域的type-7 lsa*/
			ospf_asbr_range_nssa_update(p_area_nssa, p_rangeT5);

		}
	}
}

#ifdef OSPF_MASTER_SLAVE_SYNC
int ospf_master_slave_sync_recv(int iModid, u_int ulCmd, void *var, u_int uiIdxLen, u_int uiStrLen, u_int uiFlg)
{
	int iRet = OK;
	int iMytype = 0;
	int iSubModId = 0;
	u_char *pcTmpVar = NULL;
	u_int uiIndexId = 0;
	u_long aulIndex[64] =
	{	0};
	tOSPF_AREA_INDEX stOspfAreaIndex =
	{	0};
	tOSPF_STUBAREA_INDEX stOspfStubAreaIndex =
	{	0};
	tOSPF_RANGE_INDEX stOspfRangeIndex =
	{	0};
	tOSPF_AREAAGGR_INDEX stOspfAreaAggrIndex =
	{	0};
	tOSPF_IFINDEX stOspfIfIndex =
	{	0};
	tOSPF_IFMETRIC_INDEX stOspfIfMetriclIndex =
	{	0};

	tOSPF_VIF_INDEX tVifIndex =
	{	0};
	tOSPF_NBR_INDEX tNbrIndex =
	{	0};
	tOSPF_NETWORK_INDEX tNetworkIndex =
	{	0};
	tOSPF_DISTRIBUTE_INDEX tDistributeIndex =
	{	0};
	tOSPF_REDISPOLICY_INDEX tRedispolicyIndex =
	{	0};
	tOSPF_FILTERPOLICY_INDEX tFilterpolicyIndex =
	{	0};
	tOSPF_SHAMLINK_INDEX tShamlinkInex =
	{	0};
	tOSPF_AREAAGGR_INDEX tAsbrrangeIndex =
	{	0};
	u_int uId;
	tlv_t stOctet =
	{	0};
	tlv_t *pstOctValue;
	char acValueStr[SYN_CFG_DEF_DATE_LEN] =
	{	0};

	if(NULL == var)
	{
		return ERR;
	}

	iMytype = (iModid >> 8);
	iSubModId = (iModid & 0xff);

	/*从buf中获取index, uiIdxLen为0代表无需索引，无需解析*/
	pcTmpVar = var;
	if(iMytype != MTYPE_OSPF)
	{
		OSPF_LOG_ERR("%s  master slave syn recv index analysis err!",__FUNCTION__);
		return ERR;
	}

	if (uiIdxLen != 0)
	{
		switch (iSubModId)
		{
			case MTYPE_OSPF_SYNC_SETAPI_SUB_MODID:
			{
				//  memcpy(&uiIndexId, (char *)var, uiIdxLen);
				uiIndexId = *(u_int *)var;

				if(uiFlg)
				{
					memcpy(acValueStr, (char *)(var + uiIdxLen), uiStrLen);
					stOctet.data = acValueStr;
					stOctet.len = uiStrLen;

					iRet = ospfSetApi(uiIndexId, ulCmd, &stOctet);
					break;
				}
				else
				{
					pcTmpVar = (char *)(var + uiIdxLen);
					iRet = ospfSetApi(uiIndexId, ulCmd, pcTmpVar);
					break;
				}
			}
			case MTYPE_OSPF_SYNC_SYNCAPI_SUB_MODID:
			{
				//memcpy(&uiIndexId, (char *)var, uiIdxLen);
				uiIndexId = *(u_int *)var;

				if(uiFlg)
				{
					memcpy(acValueStr, (char *)(var + uiIdxLen), uiStrLen);
					stOctet.data = acValueStr;
					stOctet.len = uiStrLen;

					iRet = ospfSyncApi(uiIndexId, ulCmd, &stOctet);
					break;
				}
				else
				{
					pcTmpVar = (char *)(var + uiIdxLen);
					iRet = ospfSyncApi(uiIndexId, ulCmd, pcTmpVar);
					break;
				}
			}
			case MTYPE_OSPF_SYNC_INSTANCE_SETAPI_SUB_MODID:
			{
				memcpy(&uiIndexId, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfInstanceSetApi(uiIndexId, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_AREA_SETAPI_SUB_MODID:
			{
				memcpy(aulIndex, (char *)var, uiIdxLen);

				if(uiFlg)
				{
					memcpy(acValueStr, (char *)(var + uiIdxLen), uiStrLen);
					stOctet.data = acValueStr;
					stOctet.len = uiStrLen;
					iRet = ospfAreaSetApi(aulIndex, ulCmd, &stOctet);
					break;
				}
				else
				{
					pcTmpVar = (char *)(var + uiIdxLen);
					iRet = ospfAreaSetApi(aulIndex, ulCmd, pcTmpVar);
					break;
				}
			}
			case MTYPE_OSPF_SYNC_AREA_SYNCAPI_SUB_MODID:
			{
				memcpy(aulIndex, (char *)var, uiIdxLen);

				if(uiFlg)
				{
					memcpy(acValueStr, (char *)(var + uiIdxLen), uiStrLen);
					stOctet.data = acValueStr;
					stOctet.len = uiStrLen;
					iRet = ospfAreaSyncApi(aulIndex, ulCmd, &stOctet);
					break;
				}
				else
				{
					pcTmpVar = (char *)(var + uiIdxLen);
					iRet = ospfAreaSyncApi(aulIndex, ulCmd, pcTmpVar);
					break;
				}
			}
			case MTYPE_OSPF_SYNC_STUBAREA_SETAPI_SUB_MODID:
			{
				memcpy(&stOspfStubAreaIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfStubAreaSetApi(&stOspfStubAreaIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_STUBAREA_SYNCAPI_SUB_MODID:
			{
				memcpy(&stOspfStubAreaIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfStubAreaSyncApi(&stOspfStubAreaIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_AREARANGE_SETAPI_SUB_MODID:
			{
				memcpy(&stOspfRangeIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfAreaRangeSetApi(&stOspfRangeIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_AREARANGE_SYNCAPI_SUB_MODID:
			{
				memcpy(&stOspfRangeIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfAreaRangeSyncApi(&stOspfRangeIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_AREAAGGREGATE_SETAPI_SUB_MODID:
			{
				memcpy(&stOspfAreaAggrIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfAreaAggregateSetApi(&stOspfAreaAggrIndex, ulCmd, pcTmpVar);
				break;
			}

			case MTYPE_OSPF_SYNC_AREAAGGREGATE_SYNCAPI_SUB_MODID:
			{
				memcpy(&stOspfAreaAggrIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfAreaAggregateSyncApi(&stOspfAreaAggrIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_NSSAAREARANGE_SETAPI_SUB_MODID:
			{
				memcpy(&stOspfRangeIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfNssaAreaRangeSetApi(&stOspfRangeIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_NSSAAREARANGE_SYNCAPI_SUB_MODID:
			{
				memcpy(&stOspfRangeIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfNssaAreaRangeSyncApi(&stOspfRangeIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_HOST_SETAPI_SUB_MODID:
			{
				memcpy(&uiIndexId, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospHostSetApi(uiIndexId, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_IF_SETAPI_SUB_MODID:
			{
				memcpy(&stOspfIfIndex, (char *)var, uiIdxLen);
				if(uiFlg)
				{
					memcpy(acValueStr, (char *)(var + uiIdxLen), uiStrLen);
					stOctet.data = acValueStr;
					stOctet.len = uiStrLen;
					iRet = ospfIfSetApi(&stOspfIfIndex, ulCmd, &stOctet);
					break;
				}
				else
				{
					pcTmpVar = (char *)(var + uiIdxLen);
					iRet = ospfIfSetApi(&stOspfIfIndex, ulCmd, pcTmpVar);
				}
				break;
			}
			case MTYPE_OSPF_SYNC_IF_SYNCAPI_SUB_MODID:
			{
				memcpy(&stOspfIfIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfIfSynApi(&stOspfIfIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_IFMETRIC_SETAPI_SUB_MODID:
			{
				memcpy(&stOspfIfMetriclIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfIfMetricSetApi(&stOspfIfMetriclIndex, ulCmd, pcTmpVar);
				break;
			}
			/*18*/case MTYPE_OSPF_SYNC_IFMETRIC_SYNCAPI_SUB_MODID:
			{
				memcpy(&stOspfIfMetriclIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfIfMetricSyncApi(&stOspfIfMetriclIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_VIFSET_SUB_MODID:
			{
				memcpy(&tVifIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfVifSetApi(&tVifIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_VIFSYNC_SUB_MODID:
			{
				memcpy(&tVifIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfVifSyncApi(&tVifIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_NBRSET_SUB_MODID:
			{
				memcpy(&tNbrIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfNbrSetApi(&tNbrIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_NBRSYNC_SUB_MODID:
			{
				memcpy(&tNbrIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfNbrSyncApi(&tNbrIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_NETWORKSET_SUB_MODID:
			{
				memcpy(&tNetworkIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfNetworkSetApi(&tNetworkIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_NETWORKSYNC_SUB_MODID:
			{
				memcpy(&tNetworkIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfNetworkSyncApi(&tNetworkIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_REDISRANGSET_SUB_MODID:
			{
				memcpy(&tDistributeIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfRedisRangeSetApi(&tDistributeIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_REDISRANGSYNC_SUB_MODID:
			{
				memcpy(&tDistributeIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfRedisRangeSynApi(&tDistributeIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_DISTRIBUTESET_SUB_MODID:
			{
				memcpy(&tDistributeIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfDistributeSetApi(&tDistributeIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_DISTRIBUTESYNC_SUB_MODID:
			{
				memcpy(&tDistributeIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfDistributeSyncApi(&tDistributeIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_REDISPOLICYSET_SUB_MODID:
			{
				memcpy(&tRedispolicyIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfRedisPolicySetApi(&tRedispolicyIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_REDISPOLICYSYNC_SUB_MODID:
			{
				memcpy(&tRedispolicyIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfRedisPolicySyncApi(&tRedispolicyIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_FILTERPOLICYSET_SUB_MODID:
			{
				memcpy(&tFilterpolicyIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfFilterPolicySetApi(&tFilterpolicyIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_FILTERPOLICYSYNC_SUB_MODID:
			{
				memcpy(&tFilterpolicyIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfFilterPolicySyncApi(&tFilterpolicyIndex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_SHAMLINKSET_SUB_MODID:
			{
				memcpy(&tShamlinkInex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfShamlinkSetApi(&tShamlinkInex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_SHAMLINKSYNC_SUB_MODID:
			{
				memcpy(&tShamlinkInex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfShamlinkSyncApi(&tShamlinkInex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_UTILSET_SUB_MODID :
			{
				memcpy(&uId, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfUtilSetApi(uId,ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_ASBRSUMMARY_SUB_MODID :
			{
				memcpy(&tAsbrrangeIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfShamlinkSyncApi(&tShamlinkInex, ulCmd, pcTmpVar);
				break;
			}
			case MTYPE_OSPF_SYNC_ASBRSUMMARYSYNC_SUB_MODID :
			{
				memcpy(&tAsbrrangeIndex, (char *)var, uiIdxLen);
				pcTmpVar = (char *)(var + uiIdxLen);
				iRet = ospfShamlinkSyncApi(&tShamlinkInex, ulCmd, pcTmpVar);
				break;
			}
			default:
			{
				iRet = ERR;
				break;
			}
		}
	}

	return iRet;
}
#endif

#if 0
void ospf_ldp_timer_start(struct ospf_if *p_if)
{
	int iRet = 0;
	tLDP_PEER_INDEX stIndex =
	{	0};
	tLDP_PEER_INDEX stNextIndex =
	{	0};
	u_int uiLdpState = 0;
	u_int uiIfunit = 0;

	if (p_if == NULL)
	{
		return;
	}
#if 0
	stLdpIfIndex.process_id = 0;
	stLdpIfIndex.if_unit = p_if->ifnet_uint;

	iRet = LdpInterfaceGetApi(&stLdpIfIndex, LDP_IF_SESSION_STATE, &ulLdpState);
#endif
	iRet = ldpPeerGetFirst(&stIndex);
	while(iRet == OK)
	{
		ldpPeerGetApi(&stIndex, LDP_PEER_IFUNIT, &uiIfunit);
		if(p_if->p_process->vrid == stIndex.process_id && p_if->ifnet_uint == uiIfunit)
		{
			ldpSessionGetApi(&stIndex, LDP_SESSION_STATE, &uiLdpState);
			break;
		}

		iRet = ldpPeerGetNext(&stIndex, &stNextIndex);
		memcpy(&stIndex, &stNextIndex, sizeof(tLDP_PEER_INDEX));
	}

	if(uiLdpState != LDP_SNMP_SESSION_STATE_OPERATIONAL)
	{
		uiLdpState = LDP_MSG_SESSION_STATE_DOWN;
	}
	else
	{
		uiLdpState = LDP_MSG_SESSION_STATE_UP;
	}

	if(uiLdpState == LDP_MSG_SESSION_STATE_UP)
	{
		p_if->ulOspfSyncState = OSPF_LDP_SYNC_ACHIEVED;
	}
	else if(ospf_nbr_search(p_if) == 0)
	{
		p_if->ulOspfSyncState = OSPF_LDP_HOLD_DOWN;
		ospf_stimer_start(&p_if->hold_down_timer, p_if->ulHoldDownInterval);
	}
	else
	{
		p_if->ulOspfSyncState = OSPF_LDP_HOLD_MAX_COST;
		ospf_stimer_start(&p_if->hold_cost_timer, p_if->ulHoldCostInterval);
	}
}
#endif
void ospf_stub_router_lsa_originate(struct ospf_process *p_process)
{
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_next = NULL;

	if (p_process == NULL)
	{
		return;
	}

	if (ospf_lstcnt(&p_process->nbr_table) == 0)
	{
		return;
	}
	for_each_node(&p_process->area_table, p_area, p_next)
	{
		ospf_router_lsa_originate(p_area);
	}

}

/**/
void ospf_refresh_stub_router_all(void)
{
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_area_next = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_process_next = NULL;

	for_each_node(&ospf.process_table,p_process,p_process_next)
	{
		if (p_process->stub_router_holdtime != 0)
		{
			p_process->stub_adv = TRUE;
			ospf_stimer_start(&p_process->stub_router_timer,
					p_process->stub_router_holdtime);
		}

	}
}

#ifdef HAVE_BFD
/*ospf bfd 静态使能判断*/
BOOL ospf_if_bfd_for_static_Enable(u_long ulIfIndex)
{
	struct ospf_if *p_if;
	struct ospf_if *p_next_if;

	for_each_node(&ospf.nm.if_table,p_if,p_next_if)
	{
		if(p_if->ifnet_uint != ulIfIndex)
		{
			continue;
		}
		if(p_if->bfd_enable == OSPF_BFD_STATIC)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void ospf_if_bind_bfd_enable(struct ospf_if *pstIf)
{
	struct ospf_nbr *p_nbr;
	struct ospf_nbr *p_next_nbr;

	if(pstIf == NULL)
	{
		ospf_logx(ospf_debug_bfd,"%s pstIf = NULL",__FUNCTION__);
		return;
	}

	for_each_node(&pstIf->nbr_table, p_nbr, p_next_nbr)
	{
		if(p_nbr->bfd_discribe == 1)
		{
			/*在使能全局bfd时标志位已置1，然后使能接口bfd需要更新为接口参数*/
			ospf_mod_bfd(p_nbr,BFD_MOD_TYPE_MIN_RX);
			ospf_mod_bfd(p_nbr,BFD_MOD_TYPE_MIN_TX);
			ospf_mod_bfd(p_nbr,BFD_MOD_TYPE_DETECT_MULT);
		}
		else
		{
			ospf_bind_bfd(p_nbr);
		}
	}

	return;
}
#endif

int ospf_ifindex_to_process(u_int ulIfunit, u_int *pulProcessId, u_int *pulAddr)
{
	int iRet = OK;

	ospf_semtake_try();

	iRet = ospf_get_Process_and_If(ulIfunit, pulProcessId, pulAddr);

	ospf_semgive();

	return iRet;
}

int ospf_if_ldp_enable_lookup(u_long ulIfindex)
{
	struct ospf_if *pstIf = NULL;
	struct ospf_if *pstIfNext = NULL;

	ospf_semtake_try();

	for_each_node(&ospf.nm.if_table, pstIf, pstIfNext)
	{
		if (pstIf->ifnet_uint == ulIfindex)
		{
			if (pstIf->ucLdpSyncEn == TRUE)
			{
				ospf_semgive();
				return OK;
			}
		}
	}

	ospf_semgive();

	return ERR;
}

