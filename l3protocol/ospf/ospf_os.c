/* ospf_os.c - os related functions,such as socket api, sdk api, kernal ip/if operation */

#include <zebra.h>
#include "ospf.h"
#ifdef HAVE_BFD
#include "bfd_nm.h"
#endif
#include "ospf_main.h"
#include "interface.h"
#include "ospf_policy.h"
#include "ospf_api.h"
//#include "kernel_network.h"
#include "product.h"

u_int ospf_unblock = 0;
u_int8 g_ospf_buf[2000] =
{ 0 };
#ifdef USP_MULTIINSTANCE_WANTED
extern void in_len2mask(struct in_addr *mask, int len);
extern int in_mask2len(struct in_addr *mask);
#else
u_long ipAddrToIfAddr(u_long ipAddr);
#endif

//extern int inet_maskLen(unsigned int netmask);
extern int log_time_print(u_int8 *buf);

void ospf_rtsock_new_route_msg_input(u_int8 *buf);

u_int8* ospf_inet_ntoa(u_int8 *ip_str, u_int ipinput)
{
	u_int ipaddress = ipinput;
	unsigned char *ip;
	//static u_int8 ip_str[128];
	ipaddress = htonl(ipinput);
	ip = (unsigned char *) &ipaddress;
	sprintf(ip_str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	return ip_str;
}

#ifdef HAVE_BFD
void
ospf_bfd_log_add( BFD_MSG_T *pstBfdMsg)
{
	struct ospf_bfd_loginfo *p_stat = NULL;
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_nbr *p_next_nbr = NULL;
	if (pstBfdMsg == NULL)
	{
		ospf_logx(ospf_debug_bfd,"pstBfdMsg = NULL return");
		return;
	}
	if (OSPF_STAT_MAX_NUM <= ospf_lstcnt(&ospf.log_table.bfd_table))
	{
		p_stat = ospf_lstfirst(&ospf.log_table.bfd_table);
		ospf_lstdel_unsort(&ospf.log_table.bfd_table, p_stat);
	}
	else
	{
		p_stat = ospf_malloc(sizeof(struct ospf_bfd_loginfo), OSPF_MSTAT);
	}
	if (NULL == p_stat)
	{
		return;
	}
	memset(p_stat, 0, sizeof(*p_stat));
	for_each_node(&ospf.nm.nbr_table, p_nbr, p_next_nbr)
	{
		if((p_nbr->p_if->addr != pstBfdMsg->ulSrcIp) || (p_nbr->addr != pstBfdMsg->ulDstIp))
		{
			continue;
		}
		p_stat->nbr_addr = p_nbr->addr;
		p_stat->process_id = p_nbr->p_if->p_process->process_id;
	}
	p_stat->state = pstBfdMsg->ucState;
	// p_stat->diag = bfdmsg->diag;
	log_time_print(p_stat->time);
	ospf_lstadd_unsort(&ospf.log_table.bfd_table, p_stat);
	return;
}
#endif
/*disable socket,set rxbuf to min value 1*/
void ospf_sock_disable(int s)
{
	int i = 1;
	ip_setsockopt(s, SOL_SOCKET, SO_RCVBUF, (u_int8 *) &i, sizeof(i));
	return;
}

/*enable socket,set rxbuf to a large value*/
void ospf_sock_enable(int s)
{
	int i = 2030000;/*a lager value*/
	ip_setsockopt(s, SOL_SOCKET, SO_RCVBUF, (u_int8 *) &i, sizeof(i));
	ip_setsockopt(s, SOL_SOCKET, SO_SNDBUF, (u_int8 *) &i, sizeof(i));
	return;
}

/*control hardware to enable or disable ospf packet rx*/
void ospf_sys_packet_input_set(struct ospf_if *p_if, u_int enable)
{
	struct ospf_if *p_check_if;
	struct ospf_if start_if;
	u_int proto = USP_OSPF_PROTO;
	u_int if_unit = 0;
	if (p_if == NULL)
	{
		return;
	}
	if_unit = p_if->ifnet_uint;
	/*enable rx packet,set directly*/
	if (IP_ADD_MEMBERSHIP == enable)
	{
		ospf_logx(ospf_debug, "if addr %#x,if_unit %d\r\n", p_if->addr,
				if_unit);
#if 0
		if (OK != uspIfSetApi (if_unit, SYS_IF_PROTOENABLE, &proto))
		{
			ospf_logx(ospf_debug,"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^errono if addr %#x,if_unit %d\r\n",p_if->addr,if_unit);
			ospf_logx(ospf_debug, "set OSPF packet enable error:if_unit %d,errno:%d\r\n", if_unit,errnoGet());
			ospf.stat.pkt_tocpu_error++;
		}
#endif		
		return;
	}
	/*disable*/
	/*check if other interface with same ip index exist,if so,do nothing*/
	start_if.ifnet_index = p_if->ifnet_index;
	start_if.addr = 0;

	for_each_node_greater(&ospf.real_if_table, p_check_if, &start_if)
	{
		if (p_check_if->ifnet_index == p_if->ifnet_index)
		{
			if (p_if != p_check_if)
			{
				ospf_logx(ospf_debug, "%d\r\n", __LINE__);
				/*still have other interfce with this ifUnit*/
				return;
			}
		}
		else /*this table indexed by ipifindex at first,so if ifindex different,do not check anymore*/
		{
			break;
		}
	}

#if 0

	if (OK != uspIfSetApi (if_unit, SYS_IF_PROTODISABLE, &proto))
	{
		ospf_logx(ospf_debug, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^set OSPF packet disable error:if_unit %d,errno:%d\r\n", if_unit, errnoGet());
		ospf.stat.pkt_tocpu_error++;
	}
#endif
	return;
}

/*set system interface unit from a network*/
int ospf_sys_netmaskbingvrf(u_int processid, u_int ifnet, u_int mask)

{
	return 0;
}

#ifdef USP_MULTIINSTANCE_WANTED
/*get first system ip address for special vrf*/
STATUS ospf_sys_ifaddr_first(u_int vrid, u_int *p_unit, u_int *addr,
		u_int *mask)
{
	u_int out_addr[OSPF_MAX_IP_PER_IF];
	u_int out_mask[OSPF_MAX_IP_PER_IF];
	u_int if_vrid = 0;
	u_int iRet = ERROR;
	if ((p_unit == NULL) || (addr == NULL) || (mask == NULL))
	{
		return ERROR;
	}
	iRet = ospf_zebra_get_frist(0, p_unit); //zhurish
	if (OK == iRet)
	{
#ifdef OSPF_VPN //zhurish
		vrf_id_t vrf_id = 0;
		struct interface *ifp = if_lookup_by_index(*p_unit);
		if ((ifp && (OK != nsm_interface_vrf_get_api(ifp, &vrf_id)))
				|| if_vrid != vrid)
		{
			*addr = 0;
			*mask = 0;
			return OSPF_VRF_NOMATCH;
		}
#endif
		/*get ip address of this system interface,got the first address*/
		if (OK
				== ospf_ifunit_to_addrlist(*p_unit, 0, OSPF_MAX_IP_PER_IF,
						out_addr, out_mask))
		{
			*addr = ntohl(out_addr[0]);
			*mask = ntohl(out_mask[0]);
		}
		else
		{
			*addr = 0;
			*mask = 0;
		}
	}
	return iRet;
}

/*get next system ip address in special vrfid,input value also act as output value*/
STATUS ospf_sys_ifaddr_next(u_int vrid, u_int *p_unit, u_int *addr, u_int *mask)
{
	u_int out_addr[OSPF_MAX_IP_PER_IF];
	u_int out_mask[OSPF_MAX_IP_PER_IF];
	u_int i;
	u_int if_vrid = 0;
	int iRet = ERROR;

	if ((p_unit == NULL) || (addr == NULL) || (mask == NULL))
	{
		return ERROR;
	}
	iRet = ospf_zebra_get_next(0, p_unit); //zhurish
	if (OK == iRet)
	{
#ifdef OSPF_VPN
		vrf_id_t vrf_id = 0;
		struct interface *ifp = if_lookup_by_index(*p_unit); //zhurish
		if ((ifp && (OK != nsm_interface_vrf_get_api(ifp, &vrf_id)))
				|| if_vrid != vrid)
		{
			*addr = 0;
			*mask = 0;
			return OSPF_VRF_NOMATCH;
		}
#endif
		/*get ip address of this system interface,got the first address*/
		if (OK
				== ospf_ifunit_to_addrlist(*p_unit, 0, OSPF_MAX_IP_PER_IF,
						out_addr, out_mask))
		{
			*addr = ntohl(out_addr[0]);
			*mask = ntohl(out_mask[0]);
		}
		else
		{
			*addr = 0;
			*mask = 0;
		}
	}
	return iRet;
}

/*get link speed of system interface*/
STATUS ospf_sys_ifspeed_get(u_int if_unit, u_int ifaddr,/*not used in multiple instance*/
u_int *p_speed/*output speed in Mb/s*/)
{
	u_int uiRealSpeed = 0;
	if (p_speed == NULL)
	{
		return ERR;
	}
	/*output speed unit is Mb/s*/
	if ((OK != uspL3IfGetApi(if_unit, SYS_IF_HIGHSPEED, p_speed))
			|| (0 == *p_speed))
	{
		*p_speed = 100;/*if not implemented,use 100M rate*/
	}
	return OK;
}

STATUS ospf_sys_ifmtu_get(u_int if_unit, u_int ifaddr, u_int *p_mtu)
{
	u_int uiMtu = 0;
	if (p_mtu == NULL)
	{
		return ERR;
	}
	*p_mtu = 1500;
	return OK;
}

/*get link state flag of system interface*/
STATUS ospf_sys_ifflag_get(u_int if_unit, u_int ifaddr,/*not used in multiple instance*/
u_int *p_flag)
{
	int iRet = ERROR;

	if (p_flag == NULL)
	{
		return ERROR;
	}
	iRet = OK;
	*p_flag = OSPF_IFE_UP;
	return iRet;
}

/*get ip ifindex of system interface*/
STATUS ospf_sys_ifindex_get(u_int if_unit, u_int ifaddr,/*not used in multiple instance*/
u_int *p_index)
{
	return if_unit;
}

/*get system interface unit from ip address*/
STATUS ospf_sys_addr2ifunit(u_int vrid, u_int ifaddr, u_int *p_if_unit)
{
	struct in_addr addr;
	addr.s_addr = ifaddr;
	struct interface * ifp = if_lookup_exact_address_vrf(addr, vrid);
	if (!ifp)
		return ERROR;
	if (p_if_unit)
		*p_if_unit = ifp->ifindex;
	return OK;
}

/*get system interface unit from a network*/
STATUS ospf_sys_net2ifunit(u_int vrid, u_int ifnet, u_int *p_if_unit,
		u_int mask)

{
	struct in_addr addr;
	struct prefix prefix;
	struct interface * ifp = NULL;
	prefix.family = AF_INET;
	prefix.u.prefix4.s_addr = ifnet;
	addr.s_addr = mask;
	prefix.prefixlen = ip_masklen(addr);
	ifp = if_lookup_prefix_vrf(&prefix, vrid);
	if (!ifp)
		return ERROR;
	if (p_if_unit)
		*p_if_unit = ifp->ifindex;
	return OK;
}

/*get ip mask of system interface*/
STATUS ospf_sys_ifmask_get(u_int ifaddr, u_int if_unit, void *p_val)
{
	int iRet = ERROR;
	struct prefix stPrefixIp;
	iRet = uspL3IfGetApi(if_unit, SYS_IF_HIGHSPEED, &stPrefixIp);
	if (iRet == OK)
	{
		struct in_addr netmask;
		masklen2ip(stPrefixIp.prefixlen, &netmask);
		*((u_int*) p_val) = netmask.s_addr;
	}
	return iRet;
}

/*decide if an local ip address is from valid interface*/
u_int ospf_sys_is_local_addr(u_short vrid, u_int ifip)
{
	u_int8 ifname[32];
	/*no warning*/
	//vrid = vrid;
	/*ignore invalid loopback address*/
	if (0x7f000001 == ifip)
	{
		return FALSE;
	}
	return TRUE;
}

/*decide if an local network is from valid interface*/
u_int ospf_sys_is_local_network(u_short vrid, u_int network)
{
#if 0
	return ospf_sys_is_local_addr(vrid, ipAddrToIfAddr(network));
#endif
}

/*get system router id for special vrf*/
/*single instance system api,may be deleted in future*/
u_int uspIpRouterId(int vrid, u_int family, u_int8 *addr)
{
	struct prefix address;
	if (ospf_zebra_get_routeid(vrid, &address) == OK)
	{
		if (addr)
			memcpy(addr, &address.u.prefix4.s_addr, 4);
		return OK;
	}
	return ERROR;
}
u_int ospf_select_router_id(u_int vrid)
{
	u_int8 maxaddr[4];

	if (uspIpRouterId(0, AF_INET, maxaddr) == OK)
	{
		return ntohl(*(u_int*) maxaddr);
	}
	return 0;
}

/*get ip address list for a system interface*/
u_int ospf_ifunit_to_addrlist(u_int if_unit, u_int ifindex,/*not used in multiple instance*/
u_int max_count, u_int *ap_addr, u_int *ap_mask)
{
	u_int i;
	u_int count = 0;
	int iRet = ERROR;
	struct prefix_ipv4 stPrefixIp =
	{ 0 };
	if ((ap_addr == NULL) || (ap_mask == NULL))
	{
		return ERROR;
	}
	iRet = uspL3IfGetApi(if_unit, SYS_IF_IPV4ADDREX, &stPrefixIp);
	if (iRet == OK)
	{
		memcpy(ap_addr, &stPrefixIp.prefix, sizeof(struct in_addr));
		in_len2mask(ap_mask, stPrefixIp.prefixlen);
	}
	else
	{
		ospf_logx(ospf_debug, "ospf get ifunit = %d ip address failed\n",
				if_unit);
	}
	return iRet;
}
#endif

#if 0//def OSPF_REDISTRIBUTE zhurish
/*function used in kernal route walkup,add a kernal route to list*/
int
ospf_sys_route_get(
		void *p_sys,
		void *arg)
{
	M2_IPROUTETBL *p_m2route = p_sys;
	struct ospf_iproute *p_route = NULL;
	u_int flag = 0;
	u_int if_unit = 0;

	if ((p_sys == NULL)
			|| (arg == NULL))
	{
		return FALSE;
	}

	p_route = ospf_malloc2(OSPF_MIPROUTE);
	if (NULL == p_route)
	{
		return FALSE;
	}

	p_route->dest = p_m2route->ipRouteDest;
	p_route->mask = p_m2route->ipRouteMask;
	/* LOOPBACK*/
	if((0 != p_m2route->ipRouteDest)
			&& (0 == p_m2route->ipRouteMask))
	{
		p_route->mask = OSPF_HOST_MASK;
	}
	p_route->metric = p_m2route->ipRouteMetric1;
	p_route->fwdaddr = p_m2route->ipRouteNextHop;
	p_route->proto = p_m2route->ipRouteProto;

	/*flag*/
	if (OK != ospf_sys_net2ifunit(0, p_route->fwdaddr, &if_unit,p_route->mask))
	{
		ospf_logx(ospf_debug_rtm, "get ifunit from ifnet %x, dst%x, msk%x,vrid%dERR\r\n",p_route->fwdaddr,p_route->dest, p_route->mask, 0);
	}
	ospf_sys_ifflag_get(if_unit, p_route->fwdaddr, &flag);
	if(OSPF_IF_UP == flag)
	{
		p_route->flags = IFF_UP;
	}

	/*add to output table,may be changed to sorted table later*/
	ospf_lstadd_unsort((struct ospf_lst *)arg, p_route);
	return TRUE;
}
#endif

/*get all kernal route with special vrf id and protocol*/
void ospf_sys_route_list_get(u_int vrid,/*not used in multiple instance*/
u_int proto, struct ospf_lst *p_list/*must be inited before calling*/)
{
#ifdef USP_MULTIINSTANCE_WANTED
	if (OSPF_INVALID_VRID == vrid)
	{
		ospf_logx(ospf_debug, "sys route list get:vrid is invalid");
		return;
	}
	if (p_list == NULL)
	{
		return;
	}
#ifdef OSPF_REDISTRIBUTE
	//ipv4v6RouteTableWalk(AF_INET, proto, ospf_sys_route_get, p_list); zhurish
#endif
#else
	vrid = vrid;
	ipv4v6RouteTableWalk(AF_INET, proto, ospf_sys_route_get, p_list);
#endif    
	return;
}

/*a test flag*/
int ospf_route_flag = 1;

STATUS ospf_sys_route_add(struct ospf_iproute *p_route, u_int *no_rtmsg)
{

	struct ospf_process *p_process = p_route->p_process;
	STATUS rc = 0;
	int error = 0;
	u_int8 dstr[32];
	u_int8 mstr[32];
	u_int8 fstr[32];
	u_int uiRouteType = ZEBRA_ROUTE_OSPF;

	/*test control :no update to system*/
	if (0 == ospf_route_flag)
	{
		*no_rtmsg = TRUE;
		return OK;
	}

	ospf_logx(ospf_debug_route, "add kernal%sroute %s/%s/%s,cost %d",
			p_route->backup_route ? " backup " : " ",
			ospf_inet_ntoa(dstr, p_route->dest),
			ospf_inet_ntoa(mstr, p_route->mask),
			ospf_inet_ntoa(fstr, p_route->fwdaddr), p_route->metric);

#ifdef USP_MULTIINSTANCE_WANTED
	/*vrf id must valid*/
	if (OSPF_INVALID_VRID == p_process->vrid)
	{
		return OK;
	}
	if (p_route->path_type == OSPF_PATH_ASE
			|| p_route->path_type == OSPF_PATH_ASE2)
	{
		uiRouteType = ZEBRA_ROUTE_OSPF;
	}
	rc = ospf_add_route_to_system(p_process->vrid, uiRouteType, p_route->dest,
			p_route->fwdaddr, p_route->mask, 0, p_route->metric);
#endif

	/*check result,if sucessfully updated,do nothing more*/
	if (OK == rc)
	{
		if (TRUE == p_route->backup_route)
		{
			ospf.stat.backup_add_ok++;
		}
		else
		{
			ospf.stat.sys_add_ok++;
		}
		return OK;
	}
	/*update error*/
	if (TRUE == p_route->backup_route)
	{
		ospf.stat.backup_add_error++;
	}
	else
	{
		ospf.stat.sys_add_error++;
	}

	ospf_logx(ospf_debug_route, "add kernal route error %d", ospf_errnoGet());

	/*if error is no buffer,clear routesocket and wait a moment,retry to update*/
	error = ospf_errnoGet();
	ospf.stat.sys_errno = error;

	/*for other failed reason,act as if update success*/
	if ((ENOBUFS != error) && (ENOBUFS != error))
	{
		*no_rtmsg = TRUE;

		ospf.stat.kernal_other_errno = error;
		ospf.stat.kernal_other_errcnt++;
		ospf.stat.err_route.rtsock = ospf.rtsock;
		ospf.stat.err_route.vrid = p_process->vrid;
		ospf.stat.err_route.dest = p_route->dest;
		ospf.stat.err_route.mask = p_route->mask;
		ospf.stat.err_route.fwd = p_route->fwdaddr;
		ospf.stat.err_route.metric = p_route->metric;
		ospf.stat.err_route.rt_add = TRUE;
		return OK;
	}
	return ERR;

}

/*delete system route*/
STATUS ospf_sys_route_delete(struct ospf_iproute *p_route, u_int *no_rtmsg)
{

	struct ospf_process *p_process = p_route->p_process;
	STATUS rc = 0;
	int error = 0;
	u_int8 dstr[32];
	u_int8 mstr[32];
	u_int8 fstr[32];
	u_int uiRouteType = ZEBRA_ROUTE_OSPF;

	/*no export to ipkernal,only used for testing*/
	if (0 == ospf_route_flag)
	{
		*no_rtmsg = TRUE;
		return OK;
	}

	ospf_logx(ospf_debug_route, "delete kernal%sroute %s/%s/%s",
			p_route->backup_route ? " backup " : " ",
			ospf_inet_ntoa(dstr, p_route->dest),
			ospf_inet_ntoa(mstr, p_route->mask),
			ospf_inet_ntoa(fstr, p_route->fwdaddr));

#ifdef USP_MULTIINSTANCE_WANTED
	/*vrfid verify*/
	if (OSPF_INVALID_VRID == p_process->vrid)
	{
		return OK;
	}
	if (p_route->path_type == OSPF_PATH_ASE
			|| p_route->path_type == OSPF_PATH_ASE2)
	{
		uiRouteType = ZEBRA_ROUTE_OSPF;
	}
	rc = ospf_delete_route_to_system(p_process->vrid, uiRouteType,
			p_route->dest, p_route->fwdaddr, p_route->mask, 0, p_route->metric);
#endif
	if (OK == rc)
	{
		if (TRUE == p_route->backup_route)
		{
			ospf.stat.backup_del_ok++;
		}
		else
		{
			ospf.stat.sys_delete_ok++;
		}
		return OK;
	}

	if (TRUE == p_route->backup_route)
	{
		ospf.stat.backup_del_error++;
	}
	else
	{
		ospf.stat.sys_delete_error++;
	}

	ospf_logx(ospf_debug_route, "delete kernal route error %d",
			ospf_errnoGet());

	/*if error is no buffer,clear routesocket and wait a moment,retry to update*/
	error = ospf_errnoGet();

	ospf.stat.sys_errno = error;

	if ((ENOBUFS != error))
	{
		/*if error is no such route,still need delete hwroute*/
		if (ESRCH != error)
		{
			*no_rtmsg = TRUE;
		}

		ospf.stat.kernal_other_errno = error;
		ospf.stat.kernal_other_errcnt++;
		ospf.stat.err_route.rtsock = ospf.rtsock;
		ospf.stat.err_route.vrid = p_process->vrid;
		ospf.stat.err_route.dest = p_route->dest;
		ospf.stat.err_route.mask = p_route->mask;
		ospf.stat.err_route.fwd = p_route->fwdaddr;
		ospf.stat.err_route.metric = p_route->metric;
		ospf.stat.err_route.rt_add = FALSE;
		return OK;
	}
	return ERR;

}

/*add or drop an interface from a multicast group*/
void ospf_sys_mcast_set(struct ospf_if *p_if, int request)
{
	/*struct ip_mreq mreq;*/
//	struct ip_mreq mreq;
	struct ip_mreqn mreq;
	u_int uiVrf = p_if->p_process->vrid;
#ifndef USE_LINUX_OS
	int s = p_if->p_process->p_master->sock[uiVrf];
#else
	int s = p_if->p_process->p_master->sock[0];
#endif
	u_int8 on = 0;
	u_int uiLen = 0;

#ifndef WIN32
#ifdef USP_MULTIINSTANCE_WANTED
#if 2 == LINUX
	mreq.imr_address.s_addr = htonl(p_if->addr)/*INADDR_ANY htonl(p_if->addr)*/;
#else
	//mreq.imr_multiaddr.s_addr = mcast_addr;
	//mreq.imr_ifindex = p_if->ifnet_index;
	//mreq.imr_interface = p_if->ifnet_index;
#endif
#else
	mreq.imr_interface.s_addr = htonl(p_if->addr);
#endif
#else
	mreq.imr_interface.s_addr = htonl(p_if->addr);
#endif
	mreq.imr_ifindex = ifindex2ifkernel(p_if->ifnet_index); //zhurish
	uiLen = sizeof(struct ip_mreqn);
	mreq.imr_multiaddr.s_addr = htonl(OSPF_ADDR_ALLSPF);
	if (IP_ADD_MEMBERSHIP == request)
	{
		ospf.stat.mutigroup_jion_cnt++;
	}
	else
	{
		ospf.stat.mutigroup_drop_cnt++;
	}
	if (0 != ip_setsockopt(s, IPPROTO_IP, request, (u_int8 *) &mreq, uiLen))
	{
		ospf_logx(ospf_debug_if, "ospf set mcast addr of all spf error %d",
				ospf_errnoGet());
		ospf.stat.mutigroup_error++;
		ospf_logx(ospf_debug_if,
				"set all spf ifindex %x,unit %x,addr %x, mcaddr %x request %d",
				p_if->ifnet_index, p_if->ifnet_uint, p_if->addr,
				OSPF_ADDR_ALLSPF, request);
		if (IP_DROP_MEMBERSHIP == request)
		{
			p_if->stat->mcast_drop_error[OSPF_MCAST_ALLSPF]++;
		}
		else
		{
			p_if->stat->mcast_jion_error[OSPF_MCAST_ALLSPF]++;
		}
	}
	else
	{
		if (IP_DROP_MEMBERSHIP == request)
		{
			p_if->stat->mcast_drop_success[OSPF_MCAST_ALLSPF]++;
		}
		else
		{
			p_if->stat->mcast_jion_success[OSPF_MCAST_ALLSPF]++;
		}
	}

	mreq.imr_multiaddr.s_addr = htonl(OSPF_ADDR_ALLDR);
	if (0 != ip_setsockopt(s, IPPROTO_IP, request, (u_int8 *) &mreq, uiLen))
	{
		ospf_logx(ospf_debug_if, "ospf set mcast addr of all dr error %d",
				ospf_errnoGet());
		ospf_logx(ospf_debug_if,
				"set all dr ifindex %x,unit %x,addr %x, mcaddr %x request %d",
				p_if->ifnet_index, p_if->ifnet_uint, p_if->addr,
				OSPF_ADDR_ALLDR, request);
		if (IP_DROP_MEMBERSHIP == request)
		{
			p_if->stat->mcast_drop_error[OSPF_MCAST_ALLDR]++;
		}
		else
		{
			p_if->stat->mcast_jion_error[OSPF_MCAST_ALLDR]++;
		}
	}
	else
	{
		if (IP_DROP_MEMBERSHIP == request)
		{
			p_if->stat->mcast_drop_success[OSPF_MCAST_ALLDR]++;
		}
		else
		{
			p_if->stat->mcast_jion_success[OSPF_MCAST_ALLDR]++;
		}
	}

	if (0 != ip_setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, (u_int8 *) &on,
					sizeof(on)))
	{
		if (IP_DROP_MEMBERSHIP == request)
		{
			p_if->stat->mcast_drop_error[OSPF_MCAST_ALLLOOP]++;
		}
		else
		{
			p_if->stat->mcast_jion_error[OSPF_MCAST_ALLLOOP]++;
		}
	}
	else
	{
		if (IP_DROP_MEMBERSHIP == request)
		{
			p_if->stat->mcast_drop_success[OSPF_MCAST_ALLLOOP]++;
		}
		else
		{
			p_if->stat->mcast_jion_success[OSPF_MCAST_ALLLOOP]++;
		}
	}

	return;
}
#define OSPF_MAX_RX_RTMSG 128
void ospf_rtsock_workmode_change()
{
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_nxtinstance = NULL;
	u_int old_mode = ospf.work_mode;
	u_int new11;
	u_int old11;
	u_int old12;

	new11 = old11 % old12;
	ospf.work_mode = ospf_get_workmode(&ospf, old_mode);

	ospf_logx(ospf_debug_syn,
			"RTM card rolechange, get work mode %d(0:nomal,1:master,2:slave)",
			ospf.work_mode);

	if ((OSPF_MODE_SLAVE == old_mode) && (OSPF_MODE_SLAVE != ospf.work_mode))
	{
		for_each_ospf_process(p_process, p_nxtinstance)
		{
			if (p_process->proto_shutdown)
			{
				continue;
			}
			ospf_set_context(p_process);

			if (p_process->restart_enable)
			{
				ospf_restart_request(p_process, OSPF_RESTART_UNPLANNED);
			}
		}
	}
	return;
}

void ospf_rtsock_link_state_change(u_int ifindex, u_int cmd)
{
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_next_process = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_iproute route;
	u_int addr = 0;
	u_int mask = 0;
	u_int flags = 0;

	/*scan all process*/
	for_each_ospf_process(p_process, p_next_process)
	{
		/*ignore process to be deleted*/
		if (p_process->proto_shutdown)
		{
			continue;
		}
		p_if = ospf_if_lookup_by_ifindex(p_process, ifindex);
		if (!p_if)
		{
			continue;
		}
		memset(&route, 0, sizeof(route));
		route.metric |= OSPF_ASE_EBIT;
		route.proto = M2_ipRouteProto_local;
		route.dest = addr & mask;
		route.mask = mask;
		route.fwdaddr = addr;/*gateway is self*/
		route.flags = (flags & IFF_UP) ? RTF_UP : 0;

		/*ignored on slave card*/
		if (OSPF_MODE_SLAVE != p_process->p_master->work_mode)
		{
			/*update interface state*/
			if ((NULL != p_if) && (NULL != p_if->p_area))
			{
				/*interface down,stop restarting*/
				if ((!(flags & IFF_UP)) && (p_process->in_restart))
				{
					ospf_logx(ospf_debug_gr,
							"rcv ifstate msg,exit graceful restart");

					ospf_restart_finish(p_process,
							OSPF_RESTART_TOPLOGY_CHANGED);
				}
				ospf_if_state_update(p_if);
			}
			/*for safe,update all interface state in this process*/
			ospf_lstwalkup(&p_process->if_table, ospf_if_state_update);

		}
		/*check redistribute state for the connected route*/
		route.p_process = p_process;
		ospf_import_iproute((flags & IFF_UP) ? TRUE : FALSE, &route);
	}
#if 0
	/*scan all process*/
	for_each_ospf_process(p_process, p_next_process)
	{
		/*for safe,update all interface state in this process*/
		ospf_lstwalkup(&p_process->if_table, ospf_if_state_update);
	}
#endif
	return;
}

/*
 * delete interface
 */
void ospf_rtsock_dellink(u_int ifi_index)
{
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_next_process = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;

	/*scan all process*/
	for_each_ospf_process(p_process, p_next_process)
	{
		for_each_ospf_if(p_process, p_if, p_next_if)
		{
			if (ifi_index == p_if->ifnet_uint)
			{
				ospf_if_delete(p_if);
			}
		}
	}
	return;
}

#ifdef OSPF_RTSOCK_ENABLE
int ospf_rtsocket_init(void)
{
	struct sockaddr_nl addr;
	int error = 0, buff_len = 0;
	int nl_group = 0;
	int s = -1;
	int on;

	/*create route socket*/
	s = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
	if (0 >= s)
	{
		ospf_logx(ospf_debug_error, "Failed to create rt socket.");
		return 0;
	}

	buff_len = 1600;

	addr.nl_family = AF_NETLINK;
	addr.nl_pad = 0;
	addr.nl_pid = 0;
	addr.nl_groups = 0;

	if (0 > bind(s, (struct sockaddr *) &addr, sizeof(addr)))
	{
		ospf_logx(ospf_debug_error, "Failed to bind rt socket!");
		return 0;
	}
#if 0
	nl_group = RTNLGRP_IPV4_IFADDR;
	if (setsockopt(s , SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &nl_group, sizeof(nl_group)) != 0)
	{
		ospf_logx(ospf_debug_error,"Failed to setsockopt RTNLGRP_LINK.");
		close(s);
		return (-1);
	}

	nl_group = RTNLGRP_LINK;
	if (setsockopt(s , SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &nl_group, sizeof(nl_group)) != 0)
	{
		ospf_logx(ospf_debug_error,"Failed to setsockopt RTNLGRP_LINK.");
		close(s);
		return (-1);
	}

	nl_group = RTNLGRP_BFD;
	if (setsockopt(s , SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &nl_group, sizeof(nl_group)) != 0)
	{
		ospf_logx(ospf_debug_error,"Failed to setsockopt RTNLGRP_BFD.");
		close(s);
		return (-1);
	}

	nl_group = RTNLGRP_ROUTEPOLICY;
	if (setsockopt(s , SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &nl_group, sizeof(nl_group)) != 0)
	{
		ospf_logx(ospf_debug_error,"Failed to setsockopt RTNLGRP_ROUTEPOLICY.");
		close(s);
		return (-1);
	}

	nl_group = RTNLGRP_SUBSYSTEM;
	if (setsockopt(s , SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &nl_group, sizeof(nl_group)) != 0)
	{
		ospf_logx(ospf_debug_error,"Failed to setsockopt RTNLGRP_SUBSYSTEM.");
		close(s);
		return (-1);
	}

	nl_group = RTNLGRP_SLOT;
	if (setsockopt(s , SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &nl_group, sizeof(nl_group)) != 0)
	{
		ospf_logx(ospf_debug_error,"Failed to setsockopt RTNLGRP_SLOT.");
		close(s);
		return (-1);
	}

	nl_group = RTNLGRP_CARD;
	if (setsockopt(s , SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &nl_group, sizeof(nl_group)) != 0)
	{
		ospf_logx(ospf_debug_error,"Failed to setsockopt RTNLGRP_CARD.");
		close(s);
		return (-1);
	}

	nl_group = RTNLGRP_IPV4_ROUTE;
	if (setsockopt(s , SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &nl_group, sizeof(nl_group)) < 0)
	{
		ospf_logx(ospf_debug_error,"Failed to setsockopt RTNLGRP_IPV4_ROUTE.");
		close(s);
		return (-1);
	}
#endif

	if (ospf_unblock)
	{
		/* 设置非阻塞模式 */
		on = 1;
		if (ioctl(s, FIONBIO, (int) &on) < 0)
		{
			ospf_logx(ospf_debug_error, "setting unblock mode error!");
			close(s);
			return 0;
		}
	}

	return s;
}

/*receive message from routing socket,and deliver to special
 callback functions,just process one message in this function*/
void
ospf_rtsock_recv(int sock)
{
	struct sockaddr *p_addr_p = NULL;
	struct sockaddr_nl addr;
	struct nlmsghdr *p_msg;
	//struct rt_msghdr *p_rtm;
	socklen_t addr_len;
	u_int8 buf[OSPF_MAX_RTMSG_LEN*10];
	u_int cmd;
	u_int msgcount = 0;
	u_int buflen;

	addr.nl_family = AF_NETLINK;
	addr.nl_pad = 0;
	addr.nl_pid = 0;
	addr.nl_groups = 0;

	/*bind(sock , (struct sockaddr *)&addr, sizeof(addr));*/

	p_addr_p = (struct sockaddr *)&addr;
	addr_len = sizeof(addr);

	memset(buf, 0, sizeof(buf));
	p_msg = (struct nlmsghdr *)buf;

	buflen = sizeof(buf);
#if 1
	/*recv multiple msg*/
	if (ospf_unblock)
	{
		while (msgcount++ < OSPF_MAX_RX_RTMSG)
		{
			if (0 > recvfrom(sock, buf, buflen, 0, p_addr_p, &addr_len))
			{
				return;
			}

			/*there is no process configured,discard msg*/
			if (!ospf_lstcnt(&ospf.process_table))
			{
				return;
			}

			//p_rtm = (struct rt_msghdr *)nlmsg_data(nlh);
			/* extract and process the command from the routing socket message */
			cmd = p_msg->nlmsg_type;
			/*no process defined here,set running process to NULL*/
			ospf_set_context(NULL);

			ospf_logx(ospf_debug_rtm, "RTM rcv routeSock %d msg", cmd);

			switch (cmd)
			{
				case RTM_NEWADDR: /* Interface is getting a new address */
				case RTM_DELADDR: /* Address removed from interface */
				if (OSPF_MODE_SLAVE == ospf.work_mode)
				{
					break;
				}
				ospf_rtsock_ifaddr_msg_input(buf, cmd);
				break;

				case RTM_NEWLINK:
				if (OSPF_MODE_SLAVE == ospf.work_mode)
				{
					break;
				}
				ospf_rtsock_newlink_msg_input(buf, cmd);
				break;

				case RTM_DELLINK:
				if (OSPF_MODE_SLAVE == ospf.work_mode)
				{
					break;
				}
				ospf_rtsock_dellink_msg_input(buf, cmd);
				break;

				case RTM_NEWROUTE:
				/*slave rcv this msg for constructing redistribute route table*/
				ospf.stat.rtm.rtm_add_cnt++;
				ospf_logx(ospf_debug_rtm,"%s:%d %s\r\n",__FUNCTION__, __LINE__, (cmd == RTM_NEWROUTE) ? "RTM_NEWROUTE" : "unknow");
				/* route, connect route */
				ospf_rtsock_route_msg_input(buf, cmd);
				break;
				case RTM_DELROUTE:
				ospf.stat.rtm.rtm_del_cnt++;
				ospf_logx(ospf_debug_rtm,"%s:%d %s\r\n",__FUNCTION__, __LINE__, (cmd == RTM_DELROUTE) ? "RTM_DELROUTE" : "unknow");
				/* route, connect route */
				ospf_rtsock_route_msg_input(buf, cmd);
				break;

				case RTM_GETROUTE:
				ospf_logx(ospf_debug,"%s:%d %s\r\n",__FUNCTION__, __LINE__, "RTM_GETROUTE");
				/* route, connect route */
				//ospf_rtsock_route_msg_input(buf, cmd);
				break;
#if 0 /*caoyong 2017.9.16 TODO:vpn和主备倒换需要修改*/
				case RTM_SUBSYSTEM_UP:
				if (OSPF_MODE_SLAVE == ospf.work_mode)
				{
					break;
				}
				ospf_rtsock_subsys_msg_input(buf);
				break;

				case RTM_SLOT_UP:
				if (OSPF_MODE_SLAVE == ospf.work_mode)
				{
					break;
				}
				ospf_rtsock_slot_up_msg_input(buf);
				break;

				case RTM_CARD_UP:
				case RTM_CARD_DOWN:
				if (OSPF_MODE_SLAVE == ospf.work_mode)
				{
					break;
				}
				ospf_rtsock_card_msg_input(buf, cmd);
				break;

				case RTM_CARD_ROLECHANGE:
				/*SLAVE need process msg RTM_CARD_ROLECHANGE for gr*/
				ospf_rtsock_workmode_msg_input(buf, cmd);
				break;

				case RTM_BFD_SESSION:
				/*ignore up*/
#ifdef HAVE_BFD
				ospf_rtsock_bfd_msg_input((struct bfd_msghdr *) buf);
#endif
				break;
#endif

				default:
				break;
			}
		}
	}
	else
	{
		if (0 > recvfrom(sock, buf, buflen, 0, p_addr_p, &addr_len))
		{
			ospf_logx(ospf_debug_error,"ospf rtm recv error!");
			return;
		}

		/*there is no process configured,discard msg*/
		if (!ospf_lstcnt(&ospf.process_table))
		{
			return;
		}

		//p_rtm = (struct rt_msghdr *)nlmsg_data(nlh);
		/* extract and process the command from the routing socket message */
		cmd = p_msg->nlmsg_type;
		/*no process defined here,set running process to NULL*/
		ospf_set_context(NULL);

		ospf_logx(ospf_debug_rtm, "RTM rcv routeSock %d msg", cmd);

		switch (cmd)
		{
			default:
			break;

			case RTM_NEWADDR: /* Interface is getting a new address */
			case RTM_DELADDR: /* Address removed from interface */
			if (OSPF_MODE_SLAVE == ospf.work_mode)
			{
				break;
			}
			ospf_rtsock_ifaddr_msg_input(buf, cmd);
			break;

			case RTM_NEWLINK:
			if (OSPF_MODE_SLAVE == ospf.work_mode)
			{
				break;
			}
			ospf_rtsock_newlink_msg_input(buf, cmd);
			break;

			case RTM_DELLINK:
			if (OSPF_MODE_SLAVE == ospf.work_mode)
			{
				break;
			}
			ospf_rtsock_dellink_msg_input(buf, cmd);
			break;

			case RTM_NEWROUTE:
			/*slave rcv this msg for constructing redistribute route table*/
			ospf.stat.rtm.rtm_add_cnt++;
			ospf_logx(ospf_debug,"%s:%d %s\r\n",__FUNCTION__, __LINE__, (cmd == RTM_NEWADDR) ? "RTM_NEWADDR" : "RTM_DELADDR");
			/*static route, connect route */
			ospf_rtsock_route_msg_input(buf, cmd);
			break;
			case RTM_DELROUTE:
			ospf.stat.rtm.rtm_del_cnt++;
			ospf_logx(ospf_debug,"%s:%d %s\r\n",__FUNCTION__, __LINE__, (cmd == RTM_NEWADDR) ? "RTM_NEWADDR" : "RTM_DELADDR");
			/*static route, connect route */
			ospf_rtsock_route_msg_input(buf, cmd);
			break;

			case RTM_GETROUTE:
			ospf_logx(ospf_debug,"%s:%d %s\r\n",__FUNCTION__, __LINE__, "RTM_GETROUTE");
			/*static route, connect route */
			//ospf_rtsock_route_msg_input(buf, cmd);
			break;
			/*case RTM_NEWLINK:
			 ospf.stat.rtm.ifinfo_cnt++;
			 ospf_rtsock_if_msg_input(buf, cmd);
			 break;*/
#if 0
			case RTM_VPN_ADD:
			break;

			case RTM_VPN_DEL:
			if (OSPF_MODE_SLAVE == ospf.work_mode)
			{
				break;
			}
			ospf_rtsock_vpndel_msg_input(buf);
			break;

			case RTM_IF_BINDVPN:
			case RTM_IF_UNBINDVPN:
			break;

			case RTM_VPN_ROUTE_ADD:
			case RTM_VPN_ROUTE_DEL:
			break;

			case RTM_USERDEFINE:
			ospf_logx(ospf_debug_rtm, "RTM rcv routetyrp 255 msg");
			if (OSPF_MODE_SLAVE == ospf.work_mode)
			{
				break;
			}
			if (RTM_SUBTYPE_ROUTEPOLICY == p_genmsg->rtm_subtype )
			{
				ospf_rtsock_routepolicy_msg_input(buf, cmd);
			}
			break;

			case RTM_NEW_RTMSG:
			/*slave rcv this msg for constructing redistribute route table*/
			ospf.stat.rtm.rtmsg_cnt++;;
			ospf_rtsock_new_route_msg_input(buf);
			break;
			case RTM_SUBSYSTEM_UP:
			if (OSPF_MODE_SLAVE == ospf.work_mode)
			{
				break;
			}
			ospf_rtsock_subsys_msg_input(buf);
			break;

			case RTM_SLOT_UP:
			if (OSPF_MODE_SLAVE == ospf.work_mode)
			{
				break;
			}
			ospf_rtsock_slot_up_msg_input(buf);
			break;

			case RTM_CARD_UP:
			case RTM_CARD_DOWN:
			if (OSPF_MODE_SLAVE == ospf.work_mode)
			{
				break;
			}
			ospf_rtsock_card_msg_input(buf, cmd);
			break;

			case RTM_CARD_ROLECHANGE:
			/*SLAVE need process msg RTM_CARD_ROLECHANGE for gr*/
			ospf_rtsock_workmode_msg_input(buf, cmd);
			break;

			case RTM_BFD_SESSION:
			/*ignore up*/
#ifdef HAVE_BFD
			ospf_rtsock_bfd_msg_input((struct bfd_msghdr *) buf);
#endif
			break;
#endif
		}
	}
#endif
	return;
}
#endif

#ifdef HAVE_BFD
/*BFD notify msg*/
void
ospf_rtsock_bfd_msg_input(BFD_MSG_T *pstBfdMsg)
{

	struct ospf_nbr *p_nbr = NULL;
	struct ospf_nbr *p_next_nbr = NULL;
	/*test*/
	struct ospf_if *p_if = NULL;

	if (pstBfdMsg == NULL)
	{
		ospf_logx(ospf_debug_bfd," pstBfdMsg = NULL return");
		return;
	}

	ospf_logx(ospf_debug_bfd/*ospf_debug_rtm*/, "bfd  state%d\r\n", pstBfdMsg->ucState);

	ospf_bfd_log_add(pstBfdMsg);
	/*we only care about BFD down event*/
	if (BFD_SESS_STATE_DOWN != pstBfdMsg->ucState)
	{
		ospf_logx(ospf_debug_bfd,"pstBfdMsg->ucState %d return", pstBfdMsg->ucState);
		return;
	}
	/*scan all nbr,if nbr's bfd session is down,timeout nbr fastly*/
	for_each_node(&ospf.nm.nbr_table, p_nbr, p_next_nbr)
	{
		/*
		 if (p_nbr->bfd_discribe != bfdmsg->discr)
		 {
		 continue;
		 }*/
		if((p_nbr->p_if->addr != pstBfdMsg->ulSrcIp) || (p_nbr->addr != pstBfdMsg->ulDstIp))
		{
			continue;
		}

		p_nbr->p_if->stat->bfd_recv_msg_cnt++;

		/*管理断开，不断开邻居,底层bfd disable时，收到的通告:
		 本地是BFD_ADMINISTRATIVELY_DOWN，
		 对端是BFD_NEIGHBOR_SIGNALED_SESSION_DOWN*/

		if(BFD_SESS_STATE_DOWN == pstBfdMsg->ucState)
		{
			ospf_set_context(p_nbr->p_if->p_process);
			ospf_logx(ospf_debug_bfd, "recv bfd state%d, neighobr going down",pstBfdMsg->ucState);

			ospf_nsm_down(p_nbr);
			//ospf_nbr_inactive_timer(p_nbr);
			ospf_timer_start(&p_nbr->hold_timer, 0);

			/*方案二*/
			//Bfd_ospf_nbr_del_api(p_nbr);
			ospf.stat.bfd_nbr_down++;
		}
#if 0
		//if (BFD_ADMINISTRATIVELY_DOWN == bfdmsg->diag)
		if (3 == bfdmsg->locl_diag)
		{
			//ospf_unbind_bfd(p_nbr);
			// ospf_stimer_start(&p_nbr->bfd_timer, 5);
			ospf.stat.bfd_admin_down++;
			return;
		}

		//BFD_NEIGHBOR_SIGNALED_SESSION_DOWN
		else if(BFD_ADMINISTRATIVELY_DOWN == bfdmsg->locl_diag)
		{
			ospf_set_context(p_nbr->p_if->p_process);
			ospf_logx(ospf_debug_nbrchange, "recv bfd diag%d, state%d, neighobr going down", bfdmsg->locl_diag, bfdmsg->state);

			ospf_nbr_inactive_timer(p_nbr);

			/*方案二*/
			//Bfd_ospf_nbr_del_api(p_nbr);
			ospf.stat.bfd_nbr_down++;
		}
#endif
	}
	return;
#if 0
#ifdef HAVE_BFD
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_nbr *p_next_nbr = NULL;
	/*test*/
	struct ospf_if *p_if = NULL;

	ospf_logx(ospf_debug_nbrchange/*ospf_debug_rtm*/, "bfd diag%d, state%d, disc%x\r\n", bfdmsg->diag, bfdmsg->state, bfdmsg->discr);

	ospf_bfd_log_add(bfdmsg);
	/*we only care about BFD down event*/
	if (BFD_SESS_STATE_DOWN != bfdmsg->state)
	{
		return;
	}
	/*scan all nbr,if nbr's bfd session is down,timeout nbr fastly*/
	for_each_node(&ospf.nm.nbr_table, p_nbr, p_next_nbr)
	{
		/*
		 if (p_nbr->bfd_discribe != bfdmsg->discr)
		 {
		 continue;
		 }*/
		if((p_nbr->p_if->addr != bfdmsg->ip_src) || (p_nbr->addr != bfdmsg->ip_dst))
		{
			continue;
		}

		/*管理断开，不断开邻居,底层bfd disable时，收到的通告:
		 本地是BFD_ADMINISTRATIVELY_DOWN，
		 对端是BFD_NEIGHBOR_SIGNALED_SESSION_DOWN*/

		if(BFD_SESS_STATE_DOWN == bfdmsg->state)
		{
			ospf_set_context(p_nbr->p_if->p_process);
			ospf_logx(ospf_debug_nbrchange, "recv bfd diag%d, state%d, neighobr going down", bfdmsg->locl_diag, bfdmsg->state);

			ospf_nbr_inactive_timer(p_nbr);

			/*方案二*/
			//Bfd_ospf_nbr_del_api(p_nbr);
			ospf.stat.bfd_nbr_down++;
		}
#if 0
		//if (BFD_ADMINISTRATIVELY_DOWN == bfdmsg->diag)
		if (3 == bfdmsg->locl_diag)
		{
			//ospf_unbind_bfd(p_nbr);
			// ospf_stimer_start(&p_nbr->bfd_timer, 5);
			ospf.stat.bfd_admin_down++;
			return;
		}

		//BFD_NEIGHBOR_SIGNALED_SESSION_DOWN
		else if(BFD_ADMINISTRATIVELY_DOWN == bfdmsg->locl_diag)
		{
			ospf_set_context(p_nbr->p_if->p_process);
			ospf_logx(ospf_debug_nbrchange, "recv bfd diag%d, state%d, neighobr going down", bfdmsg->locl_diag, bfdmsg->state);

			ospf_nbr_inactive_timer(p_nbr);

			/*方案二*/
			//Bfd_ospf_nbr_del_api(p_nbr);
			ospf.stat.bfd_nbr_down++;
		}
#endif
	}
#endif/*HAVE_BFD*/
	return;
#endif

}
#endif/*HAVE_BFD*/


#ifdef OSPF_RTSOCK_ENABLE
/*get ifaddress from ifaddres message*/
void
ospf_rtsock_parse_ifaddr(
		u_int8 *p_buf,
		u_int *p_ipaddr,
		u_int *p_vrid)

{
	struct nlmsghdr *nlh = (struct nlmsghdr *)p_buf;
	struct nlattr *tb[IFA_MAX+1];
	struct ifaddrmsg *ifm;
	int prefixlen;
	int err;
	int attrlen, nlmsg_len;
	struct nlattr * attr = NULL;
	int value = 0;
	u_int8 *p;

	if ((p_buf == NULL)
			|| (p_ipaddr == NULL)
			|| (p_vrid == NULL))
	{
		return;
	}

#if 0
	u_int i;
	//u_int8 *p;
	p = p_buf;
	for(i = 0; i <100; i++)
	{
		if(i%16==0)
		ospf_logx(ospf_debug,"\r\nBuffer@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@:");

		ospf_logx(ospf_debug," %02x",p[i]);
	}
	g_ospf_buf = p_buf;

#endif
	ifm = NLMSG_DATA(nlh);
	nlmsg_len = NLMSG_ALIGN(sizeof(struct ifaddrmsg));
	attrlen = nlh->nlmsg_len - nlmsg_len;
	/*ospf_logx(ospf_debug,"\r\NLMSG_HDRLEN %d,nlmsg_len %d,attrlen %d\r\n",NLMSG_HDRLEN,nlmsg_len,attrlen);*/
	if(attrlen < 0)
	{
		return;
	}

	*p_vrid = 0;
	attr = (struct nlattr *)(((int8_t *)ifm)+nlmsg_len);

	for(;NLA_OK(attr, attrlen); attr = NLA_NEXT(attr, attrlen))
	{
		/*ospf_logx(ospf_debug,"attr->nla_type %x, attrlen %d\r\n", attr->nla_type, attrlen);*/

		switch(attr->nla_type)
		{
			case IFA_ADDRESS:
			/*p = NLA_DATA(attr);
			 ospf_logx(ospf_debug,"@@@@@@@@@@@@@@@@@@@@ip addr %x %x %x %x\r\n",p[0],p[1],p[2],p[3]);*/
			*p_ipaddr = ntohl(*(u_int *)NLA_DATA(attr));
			break;
#if 0
			case IFA_VRFID:
			/*p = NLA_DATA(attr);
			 ospf_log(ospf_debug,"@@@@@@@@@@@@@@@@@@@@@@@@@vrid %x %x %x %x\r\n",p[0],p[1],p[2],p[3]);*/
			*p_vrid = ntohl(*(u_int *)NLA_DATA(attr));
			/*ospf_log(ospf_debug,"vrid\r\n",*p_vrid);*/
			break;
#endif
			default:
			{
				break;
			}
		}
	}

	/*prefixlen = ifm->ifa_prefixlen;*/
	/*ospf_logx(ospf_debug,"addr %x,vrid %d\r\n",*p_ipaddr,*p_vrid);*/

	return;
}

/*get ip route from rtm message*/
STATUS
ospf_rtsock_parse_iproute(
		u_int8 *p_buf,
		struct ospf_iproute *p_iproute,
		u_int *p_vrid)
{
	struct nlmsghdr *nlh = (struct nlmsghdr *)p_buf;
	struct nlattr *attr;
	int err, remaining;
	struct rtmsg *rtm;
	u_int ifunit;
	u_int flag;
	u_int data;

	if ((p_buf == NULL)
			|| (p_iproute == NULL)
			|| (p_vrid == NULL))
	{
		return ERR;
	}

	rtm = NLMSG_DATA(nlh);

	for(attr = (struct nlattr *)(nlh + (int)NLMSG_ALIGN(sizeof(struct rtmsg))), remaining = sizeof(struct rtmsg); NLA_OK(attr, remaining); attr = NLA_NEXT(attr, remaining))
	{
		switch (NLA_TYPE(attr))
		{
			case RTA_DST:
			data = NLA_DATA(attr);
			p_iproute->dest = ntohl(data);
			break;

			case RTA_GATEWAY:
			data = NLA_DATA(attr);
			p_iproute->fwdaddr = ntohl(data);
			break;

			case RTA_METRICS:
			data = NLA_DATA(attr);
			p_iproute->metric = data;
			break;

			case RTA_OIF:
			ifunit = NLA_DATA(attr);
			break;

			case RTA_FLOW:
			*p_vrid = NLA_DATA(attr);
			break;
			default:
			break;
		}
	}

	ospf_sys_ifflag_get(ifunit,p_iproute->fwdaddr, &flag);
	p_iproute->flags = IFF_UP;
	p_iproute->proto =rtm->rtm_protocol;
	/**p_vrid = 0;*/

	return OK;
}

/*process address change message from rtsock*/
void
ospf_rtsock_ifaddr_msg_input(
		u_int8 *p_buf,
		u_int cmd)
{
	u_int ipaddr = 0;
	u_int mask = 0;
	u_int vrid;

	if (p_buf == NULL)
	{
		return;
	}

	if(RTM_NEWADDR == cmd)
	{
		ospf.stat.rtm.ifaddr_add++;
	}
	else
	{
		ospf.stat.rtm.ifaddr_del++;
	}
	/*get ip address from message*/
#ifdef OSPF_TEMP
	ospf_rtsock_parse_ifaddr(p_buf, &ipaddr, &vrid,&mask);
#else
	ospf_rtsock_parse_ifaddr(p_buf, &ipaddr, &vrid);
#endif

	/*ignore invalid interface for new address*/
	if ((RTM_NEWADDR != cmd) || (ospf_sys_is_local_addr(vrid, ipaddr)))
	{
#ifdef OSPF_TEMP
		ospf_if_addr_change(vrid, ipaddr, mask, (RTM_NEWADDR == cmd) ? TRUE : FALSE);
#else
		ospf_if_addr_change(vrid, ipaddr, (RTM_NEWADDR == cmd) ? TRUE : FALSE, 0);
#endif
	}

	return;
}

void
ospf_rtsock_route_msg_input(
		u_int8 *p_buf,
		u_int cmd)
{
	return;
}
/*interface state change*/
void
ospf_rtsock_if_msg_input(
		u_int8 *p_buf,
		u_int cmd)
{
	return;
}

/*sub system up notify,send init sync msg to it if self is master,and new system is control card*/
void
ospf_rtsock_subsys_msg_input(u_int8 *rtmmsg)
{
	return;
}

/*card on slot up notify,send init sync msg to it if self is master,and new system is control card*/
void
ospf_rtsock_slot_up_msg_input(u_int8 *rtmmsg)
{
	return;
}

/*card up notify,same as above*/
void
ospf_rtsock_card_msg_input(
		u_int8 *rtmmsg,
		u_int cmd)
{
	return;
}
#endif
#ifdef HAVE_BFD
/*do not support route socket in win32*/
void
ospf_bfd_session_set(struct ospf_process *p_process,u_int ulValue)
{
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_nbr *p_next_nbr = NULL;

	if (p_process == NULL)
	{
		ospf_logx(ospf_debug_bfd,"p_process = NULL return");
		return;
	}

	for_each_ospf_if(p_process, p_if, p_next_if)
	{
		if((p_if->link_up == FALSE)
				|| (p_if->bfd_enable == OSPF_BFD_BLOCK)
				|| (p_if->bfd_enable == OSPF_BFD_IF_ENABLE))
		{
			continue;
		}
		p_if->bfd_enable = ulValue;
		for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
		{
			if(ulValue == OSPF_BFD_GBL_ENABLE)
			{
				ospf_bind_bfd(p_nbr);
			}
			else if(ulValue == OSPF_BFD_DISABLE)
			{
				ospf_unbind_bfd(p_nbr);
			}
		}
	}

	return;
}
void
ospf_bfd_session_config_set(struct ospf_process *p_process,u_int mod_type,u_int uiValue)
{
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_nbr *p_next_nbr = NULL;

	if (p_process == NULL)
	{
		ospf_logx(ospf_debug_bfd,"p_process = NULL return");
		return;
	}

	for_each_ospf_if(p_process, p_if, p_next_if)
	{
#if 0
		if(mod_type == BFD_MOD_TYPE_MRI)
		{
			p_if->ulRxMinInterval = uiValue;
		}
		else if(mod_type == BFD_MOD_TYPE_MTI)
		{
			p_if->ulTxMinInterval = uiValue;
		}
		else if(mod_type == BFD_MOD_TYPE_DET_MULT)
		{
			p_if->ulDetMulti = uiValue;
		}
#endif
		for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
		{
			ospf_mod_bfd(p_nbr,mod_type);
		}
	}

	return;
}

void
ospf_mod_bfd(struct ospf_nbr *p_nbr,u_int mod_type)
{

#ifdef HAVE_BFD
	IPC_MSG_T stIpcMsg;
	BFD_MSG_T *msg_ospf_bfd = NULL;
	struct ospf_if *p_if = NULL;
	int ret = OK;
	llong llData = 0;
	u_long ulPortNum = 0;
	u_long ulBfdVar = 0;

	if (p_nbr == NULL)
	{
		ospf_logx(ospf_debug_bfd,"p_nbr = NULL return");
		return;
	}
	p_if = p_nbr->p_if;

	msg_ospf_bfd = (BFD_MSG_T *)MALLOC(MTYPE_IPC, sizeof(BFD_MSG_T));
	if(NULL == msg_ospf_bfd)
	{
		p_nbr->p_if->stat->bfd_error_cnt++;
		ospf_logx(ospf_debug_bfd," msg_ospf_bfd = NULL return");
		return;
	}
	/*发送绑定内容*/

	memset(msg_ospf_bfd, 0, sizeof(BFD_MSG_T));
	msg_ospf_bfd->emEvent = BFD_EVENT_MODIFY;
	msg_ospf_bfd->ulProType = BFD_PRO_TYPE_OSPF;
	msg_ospf_bfd->ulSrcIp = p_if->addr;
	msg_ospf_bfd->ulDstIp = p_nbr->addr;
	msg_ospf_bfd->ulIfIndex = p_if->ifnet_uint;
	msg_ospf_bfd->ulModType = mod_type;

	switch(mod_type)
	{
		case BFD_MOD_TYPE_MIN_RX:
		{
			ulBfdVar = ospf_if_bfd_var_get(p_if, BFD_MOD_TYPE_MIN_RX);
			msg_ospf_bfd->ulReqMinRxInterval = ulBfdVar;
			break;
		}
		case BFD_MOD_TYPE_MIN_TX:
		{
			ulBfdVar = ospf_if_bfd_var_get(p_if, BFD_MOD_TYPE_MIN_TX);
			msg_ospf_bfd->ulDesiredMinTxInterval = ulBfdVar;
			break;
		}
		case BFD_MOD_TYPE_DETECT_MULT:
		{
			ulBfdVar = ospf_if_bfd_var_get(p_if, BFD_MOD_TYPE_DETECT_MULT);
			msg_ospf_bfd->ulDetectMult = ulBfdVar;
			break;
		}
		default:
		{
			free(msg_ospf_bfd);
			msg_ospf_bfd = NULL;
			p_nbr->p_if->stat->bfd_error_cnt++;
			ospf_logx(ospf_debug_bfd,"switch default return");
			return;
		}
	}

	stIpcMsg.ulMsgId = IPC_MSG_QUE_OSPF;
	stIpcMsg.ulMsgType = MSG_TYPE_BFD_FOR_LINKAGE;
	stIpcMsg.ulMsgLen = 3*sizeof(u_int) + sizeof(BFD_MSG_T);
	stIpcMsg.pucBuf = msg_ospf_bfd;

	p_if->stat->bfd_send_msg_cnt[OSPF_BFD_MOD_MSG]++;
	ret = vos_msg_queue_send(IPC_MSG_QUE_BFD,&stIpcMsg,WAIT_FOREVER,MSG_PRI_NORMAL);

	if (ERR == ret)
	{
		free(msg_ospf_bfd);
		msg_ospf_bfd = NULL;
		p_nbr->p_if->stat->bfd_error_cnt++;
		ospf_logx(ospf_debug_bfd,"send msg to bfd error return");
		return;
	}

#endif
	return;
}

/*bfd binding*/
/*bind bfd session for neighbor*/
void
ospf_bind_bfd(struct ospf_nbr *p_nbr)
{

#ifdef HAVE_BFD
	IPC_MSG_T stIpcMsg;
	BFD_MSG_T *msg_ospf_bfd = NULL;
	u_long ulHwId = INVALID;
	u_long ulMinRx = 0;
	u_long ulMinTx = 0;
	u_long ulDetMulti = 0;
	struct ospf_if *p_if = NULL;
	int ret = OK;
	llong llData = 0;

	if (p_nbr == NULL)
	{
		ospf_logx(ospf_debug_bfd," p_nbr = NULL return");
		return;
	}
	p_if = p_nbr->p_if;

	msg_ospf_bfd = (BFD_MSG_T *)MALLOC(MTYPE_IPC, sizeof(BFD_MSG_T));
	if(NULL == msg_ospf_bfd)
	{
		p_nbr->p_if->stat->bfd_error_cnt++;
		ospf_logx(ospf_debug_bfd," msg_ospf_bfd = NULL return");
		return;
	}

	ulMinRx = ospf_if_bfd_var_get(p_if, BFD_MOD_TYPE_MIN_RX);
	ulMinTx = ospf_if_bfd_var_get(p_if, BFD_MOD_TYPE_MIN_TX);
	ulDetMulti = ospf_if_bfd_var_get(p_if, BFD_MOD_TYPE_DETECT_MULT);

	/*发送绑定内容*/
	memset(msg_ospf_bfd, 0, sizeof(BFD_MSG_T));
	msg_ospf_bfd->emEvent = BFD_EVENT_CREATE;
	msg_ospf_bfd->ulProType = BFD_PRO_TYPE_OSPF;
	msg_ospf_bfd->ulSrcIp = p_if->addr;
	msg_ospf_bfd->ulDstIp = p_nbr->addr;
	msg_ospf_bfd->ulIfIndex = p_if->ifnet_uint;
	msg_ospf_bfd->ulReqMinRxInterval = ulMinRx;
	msg_ospf_bfd->ulDesiredMinTxInterval = ulMinTx;
	msg_ospf_bfd->ulDetectMult = ulDetMulti;

	stIpcMsg.ulMsgId = IPC_MSG_QUE_OSPF;
	stIpcMsg.ulMsgType = MSG_TYPE_BFD_FOR_LINKAGE;
	stIpcMsg.ulMsgLen = 3*sizeof(u_int) + sizeof(BFD_MSG_T);
	stIpcMsg.pucBuf = msg_ospf_bfd;

	p_if->stat->bfd_send_msg_cnt[OSPF_BFD_BIND_MSG]++;
	ret = vos_msg_queue_send(IPC_MSG_QUE_BFD,&stIpcMsg,WAIT_FOREVER,MSG_PRI_NORMAL);
	if (ERR == ret)
	{
		free(msg_ospf_bfd);
		msg_ospf_bfd = NULL;
		p_nbr->p_if->stat->bfd_error_cnt++;
		ospf_logx(ospf_debug_bfd," send msg to bfd error return");
		return;
	}
	ospf_logx(ospf_debug_bfd, "ospf bind bfd nbr addr%x, disc %x", p_nbr->addr, p_nbr->bfd_discribe);

	/*判断是否有对应的会话下硬件，如果没有则继续发送通告消息*/
	if (FALSE == dyn_bfd_session_is_hw_by_srcip_and_destip(p_nbr->addr,p_if->addr,p_if->ifnet_uint,BFD_PRO_TYPE_OSPF))
	{
		/*start bfd checking timer*/
		ospf_stimer_start(&p_nbr->bfd_timer, 5);
	}
	else
	{
		/*stop bfd checking timer*/
		ospf_timer_stop(&p_nbr->bfd_timer);
		p_nbr->bfd_discribe = 1;
	}

#endif
	return;
}

/*unbind bfd session for neighbor*/
void
ospf_unbind_bfd(struct ospf_nbr *p_nbr)
{
#ifdef HAVE_BFD
	struct ospf_if *p_if = NULL;
	IPC_MSG_T stIpcMsg;
	int ret = OK;
	u_long ulMinRx = 0;
	u_long ulMinTx = 0;
	u_long ulDetMulti = 0;

	if (p_nbr == NULL)
	{
		ospf_logx(ospf_debug_bfd," p_nbr = NULL return");
		return;
	}
	p_if = p_nbr->p_if;

	ospf_logx(ospf_debug_bfd, "ospf unbind bfd nbr addr%x", p_nbr->addr);
	ospf_timer_stop(&p_nbr->bfd_timer);

	/*发送绑定内容*/
	BFD_MSG_T *msg_ospf_bfd = NULL;

	msg_ospf_bfd = (BFD_MSG_T *)MALLOC(MTYPE_IPC, sizeof(BFD_MSG_T));
	if(NULL == msg_ospf_bfd)
	{
		p_nbr->p_if->stat->bfd_error_cnt++;
		ospf_logx(ospf_debug_bfd," msg_ospf_bfd = NULL return");
		return;
	}

	ulMinRx = ospf_if_bfd_var_get(p_if, BFD_MOD_TYPE_MIN_RX);
	ulMinTx = ospf_if_bfd_var_get(p_if, BFD_MOD_TYPE_MIN_TX);
	ulDetMulti = ospf_if_bfd_var_get(p_if, BFD_MOD_TYPE_DETECT_MULT);

	memset(msg_ospf_bfd, 0, sizeof(BFD_MSG_T));
	msg_ospf_bfd->emEvent = BFD_EVENT_DELETE;
	msg_ospf_bfd->ulProType = BFD_PRO_TYPE_OSPF;
	msg_ospf_bfd->ulSrcIp = p_if->addr;
	msg_ospf_bfd->ulDstIp = p_nbr->addr;
	msg_ospf_bfd->ulIfIndex = p_if->ifnet_uint;
	msg_ospf_bfd->ulReqMinRxInterval = ulMinRx;
	msg_ospf_bfd->ulDesiredMinTxInterval = ulMinTx;
	msg_ospf_bfd->ulDetectMult = ulDetMulti;

	stIpcMsg.ulMsgId = IPC_MSG_QUE_OSPF;
	stIpcMsg.ulMsgType = MSG_TYPE_BFD_FOR_LINKAGE;
	stIpcMsg.ulMsgLen = 3*sizeof(u_int) + sizeof(BFD_MSG_T);
	stIpcMsg.pucBuf = msg_ospf_bfd;

	p_if->stat->bfd_send_msg_cnt[OSPF_BFD_UNBIND_MSG]++;
	ret = vos_msg_queue_send(IPC_MSG_QUE_BFD,&stIpcMsg,WAIT_FOREVER,MSG_PRI_NORMAL);
	if (ERR == ret)
	{
		free(msg_ospf_bfd);
		msg_ospf_bfd = NULL;
		p_nbr->p_if->stat->bfd_error_cnt++;
		ospf_logx(ospf_debug_bfd," send msg to bfd error return");
		return;
	}
	p_nbr->bfd_discribe = 0;
#endif
	return;
}

u_long ospf_if_bfd_var_get(struct ospf_if *pstIf, u_int mod_type)
{
	u_long ulBfdVar = 0;

	if(NULL == pstIf)
	{
		ospf_logx(ospf_debug_bfd," pstIf = NULL return");
		return OK;
	}

	if(pstIf->bfd_enable == OSPF_BFD_GBL_ENABLE)
	{
		switch(mod_type)
		{
			case BFD_MOD_TYPE_MIN_RX :
			{
				ulBfdVar = pstIf->p_process->bfd_minrx_interval;
				break;
			}
			case BFD_MOD_TYPE_MIN_TX :
			{
				ulBfdVar = pstIf->p_process->bfd_mintx_interval;
				break;
			}
			case BFD_MOD_TYPE_DETECT_MULT :
			{
				ulBfdVar = pstIf->p_process->bfd_detmul;
				break;
			}
			default :
			break;
		}
	}

	if(pstIf->bfd_enable == OSPF_BFD_IF_ENABLE)
	{
		switch(mod_type)
		{
			case BFD_MOD_TYPE_MIN_RX :
			{
				ulBfdVar = pstIf->ulRxMinInterval;
				break;
			}
			case BFD_MOD_TYPE_MIN_TX :
			{
				ulBfdVar = pstIf->ulTxMinInterval;
				break;
			}
			case BFD_MOD_TYPE_DETECT_MULT :
			{
				ulBfdVar = pstIf->ulDetMulti;
				break;
			}
			default :
			break;
		}
	}

	return ulBfdVar;
}
#endif

/*translate ip route into policy route,for policy checking*/
void ospf_iproute2policy(struct ospf_iproute *p_ip_route, void *var)
{
#ifdef HAVE_ROUTEPOLICY
	struct ospfPolicyEntry *p_policy_entry = var;

	if ((p_ip_route == NULL)
			|| (var == NULL))
	{
		return;
	}

	memset(p_policy_entry, 0, sizeof(struct ospfPolicyEntry));

	p_policy_entry->ospf_com.proto = RPOLICY_OSPF;
	p_policy_entry->ospf_com.family = AF_INET;
	p_policy_entry->ospf_com.prefixLen = in_mask2len((struct in_addr*)&p_ip_route->mask);
	p_policy_entry->ospf_com.dest[0] = p_ip_route->dest;
	p_policy_entry->ospf_com.nexthop[0] = p_ip_route->fwdaddr;
	p_policy_entry->ospf_com.metric = p_ip_route->metric;
	p_policy_entry->ospf_com.tag = p_ip_route->tag;
	switch (p_ip_route->path_type)
	{
		case OSPF_PATH_ASE:
		p_policy_entry->type = OSPF_EXTERNAL_TYPE1;
		break;

		case OSPF_PATH_ASE2:
		p_policy_entry->type = OSPF_EXTERNAL_TYPE2;
		break;

		case OSPF_PATH_INTER:
		case OSPF_PATH_INTRA:
		p_policy_entry->type = OSPF_INTERNAL;
		break;

		default:
		break;
	}
#endif
	return;
}

/*decide if route can be redistributed by policy control,if no policy
 configured,it can be redistributed
 return FALSE:  not advertise
 retur TRUE : advertise*/
u_int ospf_redistribute_policy_verify(struct ospf_iproute *p_ip_route)
{
	int iRet = OSPF_POLICY_PERMIT;

	if (p_ip_route == NULL)
	{
		return FALSE;
	}
#ifndef HAVE_ROUTEPOLICY
	iRet = ospf_import_policy_func(p_ip_route);
	if (iRet == OSPF_POLICY_PERMIT)
	{
		iRet = ospf_filter_policy_ex_func(p_ip_route);
	}
	/*end*/
#else
	struct ospf_process *p_process = p_ip_route->p_process;
	struct ospfPolicyEntry route_policy_entry;
	struct ospf_policy *p_policy = NULL;
	struct ospf_policy *p_next_policy = NULL;

	if (NULL == p_process->p_master->route_policy_apply)
	{
		return TRUE;
	}
	p_ip_route->path_type = ospf_ase2_metric(p_ip_route->metric) ? OSPF_PATH_ASE2 : OSPF_PATH_ASE;
	ospf_iproute2policy(p_ip_route, &route_policy_entry);

	for_each_node(&p_process->redis_policy_table, p_policy, p_next_policy)
	{
		/*protocol must same*/
		if ( p_policy->proto != p_ip_route->proto)
		{
			continue;
		}
		/*if route is rejected by this policy,do not redistribute it*/
		if (OSPF_POLICY_DENY == p_process->p_master->route_policy_apply(p_policy->policy_index, &route_policy_entry))
		{
			return FALSE;
		}
		/*update metric if configured*/
		if (APPLY_OSPF_TYPE2 == route_policy_entry.type)
		{
			p_ip_route->metric = (route_policy_entry.ospf_com.metric & 0x00ffffff) |0x80000000;
		}
		else
		{
			p_ip_route->metric = route_policy_entry.ospf_com.metric & 0x00ffffff;
		}

		/*if not set in route polcy, translate field is 0*/
		ospf_logx(ospf_debug_rtm, "redistribute route policy,translate %d(1-trans,2-not trans)", route_policy_entry.translate);

		if (APPLY_OSPF_TRANSLATE == route_policy_entry.translate)
		{
			p_ip_route->no_translate = 0;
		}
		else if (APPLY_OSPF_NOTRANSLATE == route_policy_entry.translate)
		{
			p_ip_route->no_translate = 1;
		}
	}
#endif
	if (iRet == OSPF_POLICY_PERMIT)
	{
		return TRUE;
	}
	return FALSE;
}

/* retur TRUE : route is filted by routepolicy,so can't add to ip route table*/
u_int ospf_filter_policy_verify(struct ospf_iproute *p_ip_route,
		struct ospf_policy *p_policy_exclude)
{
#ifndef HAVE_ROUTEPOLICY
	int iRet = OSPF_POLICY_PERMIT;
	iRet = ospf_filter_policy_im_func((void *) p_ip_route);
	if (iRet == OSPF_POLICY_PERMIT)
	{
		return FALSE;
	}
	/*end*/
#else
	struct ospf_process *p_process = p_ip_route->p_process;
	struct ospfPolicyEntry route_policy_entry;
	struct ospf_policy *p_policy = NULL;
	struct ospf_policy *p_next_policy = NULL;

	if (NULL == p_process->p_master->route_policy_apply)
	{
		ospf_logx(ospf_debug_route, "filter routepolicy apply is NULL");

		return FALSE;
	}
	ospf_iproute2policy(p_ip_route, &route_policy_entry);

	for_each_node(&p_process->filter_policy_table, p_policy, p_next_policy)
	{
		ospf_logx(ospf_debug_route, "check policy index=%d", p_policy->policy_index);

		if (p_policy_exclude == p_policy)
		{
			continue;
		}

		if (OSPF_POLICY_DENY == p_process->p_master->route_policy_apply(p_policy->policy_index, &route_policy_entry))
		{
			ospf_logx(ospf_debug_route, "route %x is filtered", p_ip_route->dest);

			return TRUE;
		}
	}
#endif
	return TRUE;
}


/*send route socket msg to system*/
#ifdef OSPF_RTSOCK_ENABLE
STATUS ospf_rtsock_route_msg_output(struct ospf_process *p_process)
{
	return OK;
}
#endif
STATUS ospf_rtsock_route_msg_insert(struct ospf_iproute *p_route)
{
	struct ospf_process *p_process = p_route->p_process;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;
	u_long ulPort = 0, ulIp = 0;
	int iRet = 0;
	//zhurish ARP_INFO_INDEX_T stIp = {0};
	//ARP_INFO_T *pstArp = NULL;
	u_int uiIfIndex = 0;
	u_char szMac[20];

//	ospf_logx(ospf_debug,"%s:%d\r\n",__FUNCTION__,__LINE__);
#ifdef USP_MULTIINSTANCE_WANTED
	if ((OSPF_INVALID_VRID == p_process->vrid))
	{
		p_process->p_master->stat.rt_cnt[p_route->active]++;
		return OK;
	}
#endif

	ospf_logx(ospf_debug,
			"111 dest=0x%x,metric=0x%x,fwdaddr=0x%x,flags=0x%x,path_type=%x,mask=0x%x,tag=0x%x,active=0x%x,if_unit=0x%x,\r\n",
			p_route->dest, p_route->metric, p_route->fwdaddr, p_route->flags,
			p_route->path_type, p_route->mask, p_route->tag, p_route->active,
			p_route->if_unit);

	if (p_route->fwdaddr == 0)
	{
		p_process->p_master->stat.rt_cnt[p_route->active]++;
		ospf_logx(ospf_debug,
				"ospf_rtsock_route_msg_insert,dest=0x%x,fwdaddr=0x%x,\r\n",
				p_route->dest, p_route->fwdaddr);
		zlog_info(ZLOG_OSPF,
				"ERR: ospf_rtsock_route_msg_insert,dest=0x%x,fwdaddr=0x%x,\r\n",
				p_route->dest, p_route->fwdaddr);
		return OK;
	}

	if (TRUE == p_route->active)
	{
#ifdef OSPF_DCN //zhurish
		if (p_process->process_id == OSPF_DCN_PROCESS)
		{
			if(ERROR == lldp_set_api(p_route->fwdaddr,LLDP_REMOTE_GET_MAC_BY_IP,szMac))
			{
				return ERR;
			}
			ospf_logx(ospf_debug,"ospf_rtsock_route_msg_insert,szMac=%02x,%02x,%02x,%02x,%02x,%02x\r\n",szMac[0],szMac[1],szMac[2],szMac[3],szMac[4],szMac[5]);
			if(OK != if_vport_ifindex_to_logic_port(p_route->if_unit,&ulPort))
			{
				return ERR;
			}
			iRet = ospf_get_rem_man_addr(p_route->if_unit, &ulIp);
			if((iRet == OK)&&(ulIp != 0))
			{
				p_route->fwdaddr = ulIp;

				//p_if = ospf_if_lookup(p_process, p_route->fwdaddr,p_route->if_unit);
				//	if(NULL  == p_if)
				ospf_logx(ospf_debug,"ospf_rtsock_route_msg_insert,addr:0x%x,ulPort:%d\r\n",p_route->fwdaddr, ulPort);
				//	arp_add_by_dcn(szMac, p_route->fwdaddr, ulPort);
			}
		}
#endif
#ifdef OSPF_DCN//zhurish
		/*若学到路由为邻居则不下发路由表*/
		if(p_process->process_id == OSPF_DCN_PROCESS)
		{
			//解决由于lldp邻居没有删除，导致直接return的问题，直接通过判断是否有arp去解决。
			stIp.uiVrf = p_route->if_unit;
			stIp.uiIp = p_route->dest;
			pstArp = arp_info_get(&stIp);

			//iRet = ospf_dcn_rem_man_addr_check(stRouteData.dest,&ulPort);
			/*过滤Overlay模型*/
			/*
			 if((OK == iRet)&&(OK != ospf_dcn_lookup_overlay_port(ulPort)))
			 {
			 zlog_info(ZLOG_OSPF,"active:%d dest=0x%x is lldp nbr!\r\n",p_route->active,stRouteData.dest);
			 return OK;
			 }
			 */
			if(pstArp &&(OK != ospf_dcn_lookup_overlay_port(ulPort)))
			{
				p_process->p_master->stat.rt_cnt[p_route->active]++;
				zlog_info(ZLOG_OSPF,"active:%d dest=0x%x is lldp nbr!\r\n",p_route->active,p_route->dest);
				return OK;
			}
		}
		//l3sw_setif_ipmac(stRouteData.if_unit,szMac);
#endif
	}
	else
	{
#ifdef OSPF_DCN
		/*若删除路由为邻居则删除lldp邻居，重新触发arp,防止l3table中无邻居*/
		if(p_process->process_id == OSPF_DCN_PROCESS)
		{

			iRet = ospf_dcn_rem_man_addr_check(p_route->dest,&ulPort);
			/*过滤Overlay模型和无效端口*/
			if((OK == iRet)
					/*&&(ulPort != INVALID)*/
					&&(OK != ospf_dcn_lookup_overlay_port(ulPort)))
			{
				if(ERROR == if_logic_port_to_index(ulPort,&uiIfIndex))
				{
					return ERR;
				}
				lldp_set_api(uiIfIndex,LLDP_REMOTE_DELETE_NBR_BY_LOGIC_PORT,&ulPort);
				zlog_info(ZLOG_OSPF,"active:%d dest=0x%x is lldp nbr,delete lldp nbr ulPort=%d!\r\n",p_route->active,p_route->dest,ulPort);
			}
		}
#endif
	}

	//p_route->fwdaddr = htonl(p_route->fwdaddr);
	//p_route->dest = htonl(p_route->dest);
	//p_route->mask = htonl(p_route->mask);
	ospf_logx(ospf_debug,
			"ospf_rtsock_route_msg_insert,dest=0x%x,fwdaddr=0x%x,\r\n",
			p_route->dest, p_route->fwdaddr);
//	printf("ospf_rtsock_route_msg_insert,dest=0x%x,fwdaddr=0x%x,\r\n",stRouteData.dest,stRouteData.fwdaddr);

	if (p_process->p_master->work_mode == OSPF_MODE_SLAVE)
	{
		/*ospf为slaver状态 不下路由*/
		p_process->p_master->stat.slaver_rt_cnt[p_route->active]++;
		ospf_logx(ospf_debug_route,
				"%d active%d work_mode is OSPF_MODE_SLAVE, return", __LINE__,
				p_route->active);
		//return OK;
	}
	p_process->p_master->stat.rt_cnt[p_route->active]++;

	if (p_route->active == TRUE)
	{
		iRet = ospf_add_route(p_route, p_process);
	}
	else
	{
		iRet = ospf_del_route(p_route, p_process);
	}
	/*end*/

//	printf("ospf_rtsock_route_msg_insert iRet:%d\n", iRet);
	return iRet;
}
#ifdef OSPF_RTSOCK_ENABLE
/*recv route socket msg using new msg format*/
void ospf_rtsock_new_route_msg_input(u_int8 *buf)
{
	return;
}
/*decide if route socket msg filter need change,multiple process using a single socket,so
 must check all process's setting*/
void ospf_rtsock_option_update(int sock)
{
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_next_process = NULL;
	u_int proto;
	u_int enable;

	if (0 >= sock)
	{
		return;
	}
	/*check all protocols*/
	for (proto = 0; proto < M2_ipRouteProto_max; proto++)
	{
		enable = 0;
		/*accept route in this protocol if any process permit*/
		for_each_ospf_process(p_process, p_next_process)
		{
			if (BIT_LST_TST(p_process->reditribute_flag, proto))
			{
				enable = 1;
			}
		}
		ospf_rtSockOptionSet(sock, proto, enable);
	}
	return;
}
#endif

int ospf_set_sock_noblock(int s, int on)
{
	if (s <= 0)
	{
		return ERROR;
	}
#if !defined(WIN32)
	return ioctl(s, FIONBIO, (void *) &on);
#else
	return ioctlsocket(s, FIONBIO, (int)&on);
#endif
}

/*init socket used by ospf*/
int ospf_socket_init(u_int uiVrf)
{
	int s = 0;
	u_int hincl = 1;
	int on = 1;

	/*raw socket for ip rx*/
	s = ip_socket(AF_INET, SOCK_RAW, OSPF_IP_ID);
	if (ERR == s)
	{
		ospf_logx(ospf_debug_error, "Failed to create input socket!");
		return 0;
	}
	/*include IP header when send packet*/
	if (0 != ip_setsockopt(s, IPPROTO_IP, IP_HDRINCL, (u_int8 *) &hincl,
					sizeof(hincl)))
	{
		vty_out_to_all_terminal(
				"setting IP_HDRINCL on OSPF socket error! sock = %d err = %d",
				s, ospf_errnoGet());
		ospf_logx(ospf_debug_error, "setting IP_HDRINCL on OSPF socket error!");
	}
	/*获取socket收包接口索引，解决双上行问题。*/
	if (0 != ip_setsockopt(s, IPPROTO_IP, IP_PKTINFO, &on, sizeof(on)))
	{
		vty_out_to_all_terminal(
				"setting IP_PKTINFO on OSPF socket error! sock = %d err = %d",
				s, ospf_errnoGet());
		ospf_logx(ospf_debug_error, "setting IP_PKTINFO on OSPF socket error!");
	}

	ospf_set_sock_noblock(s, on);

#ifndef USE_LINUX_OS
	if (uiVrf > 0)
	{
		ip_setsockopt(s, SOL_SOCKET, IP_SO_X_VR, &uiVrf, sizeof(uiVrf));
	}
#endif

	ospf_thread_add(uiVrf, s);
	return s;
}

int ospf_close_sock(u_int uiVrf)
{
	ospf_thread_del(uiVrf);
	ip_close(ospf.sock[uiVrf]);

	ospf.sock[uiVrf] = -1;

	return TRUE;
}

void ospf_socket_send(struct ospf_hdr *p_packet, struct ospf_if *p_if,
		u_int dest, u_int len)
{
	struct iphdr iph = { 0 };
	struct in_addr addr;
	u_int flags = 0;
	struct ip_mreqn mreq = { 0 };
	struct iovec iov[2];
	struct sockaddr_in sa;
	struct msghdr msg = { 0 };
	int i, iSockRtv = 0;
	int iRet = 0;
	u_int uiVrf = p_if->p_process->vrid;
#ifndef USE_LINUX_OS
	int s = p_if->p_process->p_master->sock[uiVrf];
#else
	int s = p_if->p_process->p_master->sock[0];
#endif

	if (OSPF_INVALID_VRID == p_if->p_process->vrid)
	{
		ospf_logx(ospf_debug, "send packet,invalid vrid\r\n");
		return;
	}

	/*mcast address*/
	if ((OSPF_ADDR_ALLSPF == dest) || (OSPF_ADDR_ALLDR == dest))
	{
#ifndef USE_LINUX_OS
		mreq.imr_ifindex = ifindex2ifkernel(p_if->ifnet_index); //zhurish
#else
		addr.s_addr = htonl(p_if->addr);
#endif
		/* call setsockopt to select an outgoing interface
		 for multicasting the packet */
#ifndef USE_LINUX_OS
		if (0 != ip_setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF,
						(u_int8 *) &mreq, sizeof(mreq)))
#else
		if (0 != setsockopt (s, IPPROTO_IP, IP_MULTICAST_IF, (u_int8 *)&addr, sizeof(addr)))
#endif
		{
			ospf_logx(ospf_debug,
					"setsockopt add muticast ERR! s= %d err = %d\r\n", s,
					ospf_errnoGet());
		}
		p_if->stat->tx_muti_pkt++;
	}
	else
	{
		p_if->stat->tx_unicast_pkt++;
	}

	if ((p_if->type != OSPF_IFT_VLINK) && (p_if->type != OSPF_IFT_SHAMLINK))
	{
		iRet = ospfGetPortLinkStates(p_if->ifnet_uint);
		if (ERR == iRet)
		{
			return;
		}
	}
	iph.ihl = sizeof(struct iphdr) >> 2;
	iph.version = IPVERSION;
	iph.tos = IPTOS_PREC_INTERNETCONTROL;
	iph.tot_len = htons((iph.ihl * 4) + len);
	iph.id = 0;
	iph.frag_off = 0;
	if (p_if->p_process->valid_hops)
	{
		iph.ttl = OSPF_MAX_TTL;
	}
	else
	{
		if ((OSPF_IFT_VLINK == p_if->type) || (OSPF_IFT_SHAMLINK == p_if->type))
		{
			iph.ttl = OSPF_VL_IP_TTL;
		}
		else
		{
			iph.ttl = OSPF_IP_TTL;
		}
	}
	/*修改TTL，使单播报文可发出*/
	if ((OSPF_ADDR_ALLSPF != dest) && (OSPF_ADDR_ALLDR != dest))
	{
		iph.ttl += 1;
	}
	iph.protocol = OSPF_IP_ID;
	iph.check = 0;
#ifdef OSPF_DCN //zhurish
	if(p_if->ulDcnflag == OSPF_DCN_FLAG)
	{
		iph.saddr = htonl(p_if->p_process->router_id);
	}
	else
	{
		iph.saddr = htonl(p_if->addr);
	}
#else
	iph.saddr = htonl(p_if->addr);
#endif
	iph.daddr = htonl(dest);

	/* send the packet using the raw socket */
	sa.sin_addr.s_addr = htonl(dest);
	sa.sin_family = AF_INET;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (u_int8 *) &sa;
	msg.msg_namelen = sizeof(sa);

	msg.msg_iov = iov;
	msg.msg_iovlen = 2;
	iov[0].iov_base = (u_int8*) &iph;
	iov[0].iov_len = iph.ihl * 4;
	iov[1].iov_base = (u_int8*) p_packet;
	iov[1].iov_len = len;

	/* Set DONTROUTE flag if dst is unicast. */
	if ((OSPF_IFT_VLINK != p_if->type) && (OSPF_IFT_SHAMLINK != p_if->type)
			&& (!IN_MULTICAST(htonl(dest))))
	{
		flags = MSG_DONTROUTE;
	}
	iSockRtv = sizeof(msg);

	iSockRtv = ip_sendmsg(s, &msg, flags | MSG_DONTWAIT);

	if (0 > iSockRtv)
	{
		ospf_logx(ospf_debug_error,
				"send packet failed iSockRtv = %d error = %d", iSockRtv,
				ospf_errnoGet());
	}
	return;
}

/*********************************************
 recv a packet from raw socket,afer selecting
 */
void ospf_socket_recv(int s)
{
	struct sockaddr_in from;
	struct msghdr msg;
	struct iovec iov;
	struct cmsghdr *p_cmsg = NULL;
	u_int ifindex = 0;
	u_int rx_count = 0;
	u_int8 adata[256];
	u_int8 *buf = ospf.p_rxbuf;
	struct in_pktinfo *pktinfo = NULL;
	int len;
	int opt_len;
	u_int8 s_src[16];
	u_int i;
	u_int8 *p;
	struct ifreq ifr;
	int ret = 0;
	int raw_sock_fd = 0;
	int fd = 0;
	u_int uiIfindex = 0;
	u_int uiVrf = 0;

	bzero((char *) &from, sizeof(from));
	bzero((char *) &msg, sizeof(msg));

	/* Fill in message and iovec. */
	msg.msg_name = (void *) &from;
	msg.msg_namelen = sizeof(struct sockaddr_in);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = (void *) adata;
	msg.msg_controllen = sizeof(adata);
	iov.iov_base = buf;
	iov.iov_len = OSPF_BIGPKT_BUF;

	/* If recvmsg fail return minus value. */
	len = ip_recvmsg(s, &msg, 0);
	if (len < 0)
	{
		return;
	}

	for (p_cmsg = CMSG_FIRSTHDR(&msg); p_cmsg != NULL;
			p_cmsg = CMSG_NXTHDR(&msg, p_cmsg))
	{
		// 忽略我们不需要的控制头（the control headers）
		if (p_cmsg->cmsg_level != IPPROTO_IP || p_cmsg->cmsg_type != IP_PKTINFO)
		{
			continue;
		}
		pktinfo = CMSG_DATA(p_cmsg);
#ifdef USE_LINUX_OS
		uiVrf = INVALID_VPN_ID;
#endif
		if (ERROR == ospf_sys_index_to_if_index(uiVrf, pktinfo->ipi_ifindex,
						&uiIfindex))
		{
			ospf_logx(ospf_debug, "if index change err %x\n",
					pktinfo->ipi_ifindex);
			return;
		}

	}
	if (OSPF_MODE_SLAVE == ospf.work_mode)
	{
		ospf_logx(ospf_debug, "do not recv packet in slave mode");
		/*continue;*/
		return;
	}

	//ospf_set_context(NULL);
	ospf_logx(ospf_debug_msg, "sock %d recv packet from %s,len %d, ifindex=%d",
			s, ospf_inet_ntoa(s_src, from.sin_addr.s_addr), len, uiIfindex);

	ospf_input(buf, len, uiIfindex);

	return;
}

u_short checksum(u_short * pAddr, /* pointer to buffer  */
int len /* length of buffer   */
)
{
	int nLeft = len;
	int sum = 0;
	u_short * w = pAddr;
	u_short answer;

	while (nLeft > 1)
	{
		sum += *w++;
		nLeft -= 2;
	}

	if (nLeft == 1)
#if _BYTE_ORDER == _BIG_ENDIAN
		sum += 0 | ((*(u_char *) w) << 8);
#else
	sum += *(u_char *) w;
#endif

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = sum;

	return (~answer & 0xffff);
}


int ospf_errnoGet()
{
	return errno;
}



#if 0

void ospf_updataipaddr(u_int vrid, u_int ulIfIndex, u_int ipaddr, u_int cmd)
{
	u_long ulLbIfIndex = 0;

	ipaddr = htonl(ipaddr);
#ifdef OSPF_DCN //zhurish
	if(ERROR == if_loopback_id_to_index(OSPF_DCNLOOPBACK_IFINDEX,&ulLbIfIndex))
	{
		return;
	}
#endif
	if (ulIfIndex == ulLbIfIndex)
	{
		return;
	}
	ospf_logx(ospf_debug,
			"ospf_updataipaddr vrid=%d,ulIfIndex=0x%x,ipaddr=0x%x,cmd=%d!\n",
			vrid, ulIfIndex, ipaddr, cmd);
#if 0
	if((cmd == ZEBRA_REDISTRIBUTE_TYPE_IF_ADDR_ADD)&&(vrid == 0))
	{
		vrid = ospf_updatIfAdd(ipaddr);
	}
#endif
	ospf_if_addr_change(vrid, ipaddr, cmd, ulIfIndex);
}

void ospf_updatIfStatus(u_int uiIfindex, u_int uiLinkStatus)
{
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_next_process = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;

	for_each_ospf_process(p_process, p_next_process)
	{
		for_each_ospf_if(p_process, p_if, p_next_if)
		{
			if (uiIfindex == p_if->ifnet_uint)
			{
				ospf_if_state_update(p_if);
				/*for safe,update all interface state in this process*/
				// ospf_lstwalkup(&p_process->if_table, ospf_if_state_update);
				if (p_if->stat != NULL)
				{
					if (uiLinkStatus == OSPF_IFE_UP)
					{
						p_if->stat->linkup_msg_cnt++;
						log_time_print(p_if->stat->linkup_msg_time);
					}
					else
					{
						p_if->stat->linkdown_msg_cnt++;
						log_time_print(p_if->stat->linkdown_msg_time);
					}
				}
				p_if->link_up = (uiLinkStatus == OSPF_IFE_UP) ? TRUE : FALSE;
				if (p_if->link_up == FALSE)
				{
					p_if->ulOspfSyncState = OSPF_LDP_INIT;
					ospf_logx(ospf_debug_rtm,
							"ospf ldp sync change to OSPF_LDP_INIT %s %d\r\n",
							__FUNCTION__, __LINE__);
					ospf_if_hold_down_stop(p_if);
					p_if->ucHoldCostState = FALSE;
					if (ospf_timer_active(&p_if->hold_cost_timer))
					{
						ospf_timer_stop(&p_if->hold_cost_timer);
					}
				}
				ospf_logx(ospf_debug,
						"ospf_updatIfStatus ifindex=%x,link_up:%d\n", uiIfindex,
						p_if->link_up);
			}
		}
	}
}

u_int ospf_updatIfAdd(u_int if_addr)
{
	struct ospf_network *p_network = NULL;
	struct ospf_network *p_next = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_next_process = NULL;
	u_int process_id = 0;
	u_int uiVrfId = 0;
	/*check if network configured,check all instance*/
	for_each_ospf_process(p_process, p_next_process)
	{
		/*ignore invalid process*/
		if (p_process->proto_shutdown)
		{
			continue;
		}
#ifdef OSPF_DCN //zhurish
		if(p_process->process_id == OSPF_DCN_PROCESS)
		{
			continue;
		}
#endif
		for_each_node(&p_process->network_table, p_network, p_next)
		{
			if (ospf_netmatch(p_network->dest, if_addr, p_network->mask))
			{
#ifndef OSPF_VPN
				ospf_sys_netmaskbingvrf(p_process->process_id,p_network->dest,p_network->mask);
#endif
				process_id = p_process->process_id;
				break;
			}
		}
	}
#ifndef OSPF_VPN
	zebra_if_ospf_get_api(process_id,ZEBRA_IF_GET_VRF_BY_OSPF_ID,&uiVrfId);
#endif

	return uiVrfId;
}

void ospf_updatIfDel(u_int ifindex)
{
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_next_process = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;
	u_long ulLbIfIndex = 0;
#ifdef OSPF_DCN //zhurish
	if(ERROR == if_loopback_id_to_index(OSPF_DCNLOOPBACK_IFINDEX,&ulLbIfIndex))
	{
		return;
	}
#endif
	if (ifindex == ulLbIfIndex)
	{
		return;
	}

	for_each_ospf_process(p_process, p_next_process)
	{
#ifdef OSPF_DCN //zhurish
		if(p_process->process_id == OSPF_DCN_PROCESS)
		{
			continue;
		}
#endif
		for_each_ospf_if(p_process, p_if, p_next_if)
		{
			if (ifindex == p_if->ifnet_uint)
			{
				ospf_logx(ospf_debug,
						"ospf_updatIfDel if_delete ifindex=0x%x.\r\n", ifindex);
				ospf_if_delete(p_if);
			}
		}
	}
}

int ospf_msg_process(u_char *pBuf, u_long ulMsgLen)
{
	u_int ulCmd = 0;
	u_int vrid = 0;
	u_int ipaddr = 0;
	u_int ifindex = 0;
	u_int ulValue = 0;
	int i = 0;

	if (pBuf == NULL)
	{
		return ERR;
	}

	ospf_logx(ospf_debug, "ospf_msg_process ulMsgLen=%d.\r\n", ulMsgLen);

	if (ulMsgLen < OSPF_REVL3_LEN)
	{
		return ERR;
	}
#if 0
	for(i=0;i< ulMsgLen;i++)
	{
		ospf_logx(ospf_debug,"pBuf[%d]=%d.\r\n",i,pBuf[i]);
	}
#endif
	ulCmd = pBuf[0];

	memcpy(&vrid, &pBuf[1], 4);
	memcpy(&ipaddr, &pBuf[5], 4);
	memcpy(&ifindex, &pBuf[9], 4);
//	vrid 	= htonl(vrid);
	ipaddr = ntohl(ipaddr);
//	ifindex = ntohl(ifindex);
	if ((vrid == 0) || (ifindex == 0))
	{
		ospf_logx(ospf_debug, "ospf_msg_process bad value.\r\n");
		return ERR;
	}
	ulValue = pBuf[13];
	ospf_logx(ospf_debug,
			"ospf_msg_process ulValue=0x%x,ifindex=0x%x,ipaddr=0x%x,vrid=0x%x.\r\n",
			ulValue, ifindex, ipaddr, vrid);

	switch (ulCmd)
	{
	case OSPF_UPDATAIPADDR:
	{
		if (ipaddr == 0)
		{
			ospf_logx(ospf_debug,
					"ospf_msg_process bad value ipaddr == 0.\r\n");
			return ERR;
		}
		ospf_updataipaddr(vrid, ifindex, ipaddr, ulValue);
		break;
	}
	case OSPF_UPDATAIFSTATUS:
	{
		ospf_updatIfStatus(ifindex, 0);
		break;
	}
	case OSPF_UPDATAIFDELETE:
	{
		ospf_updatIfDel(ifindex);
		break;
	}
	default:
	{
		break;
	}
	}

	return OK;
}
#endif
#if 0
int ospf_set_loc_man_addr(u_long ulIfIndex, u_long ulIp)
{
	int ret = OK;
	u_long ulPort = 0;
	u_long ulIndex = 0;

	if(ERROR== if_vport_ifindex_to_logic_port(ulIfIndex,&ulPort))
	{
		return ERR;
	}

	if(ERROR== if_logic_port_to_index(ulPort,&ulIndex))
	{
		return ERR;
	}

	ret = lldp_set_api(ulIndex, LLDP_MANAGE_ADDR,&ulIp);

	return ret;
}

int ospf_get_loc_man_addr(u_long ulIfIndex, u_long * ulIp)
{
	int ret = OK;
	u_long ulPort = 0;
	u_long ulIndex = 0;

	if(ERROR== if_vport_ifindex_to_logic_port(ulIfIndex,&ulPort))
	{
		return ERR;
	}

	if(ERROR== if_logic_port_to_index(ulPort,&ulIfIndex))
	{
		return ERR;
	}
	ret = lldp_get_api(ulIndex, LLDP_MANAGE_ADDR,ulIp);

	return ret;
}

int ospf_get_rem_man_addr(u_long ulIfIndex, u_long * pulIp)
{
	int ret = OK;
	u_long ulPort = 0,ulPortEn = 0;
	u_long ulIndex = 0;
	u_long ulRemNextIndex = 0;
	u_long ulLocIfIndex = 0;
	u_long ulRemIndex = 0;

	if(ERROR== if_vport_ifindex_to_logic_port(ulIfIndex,&ulPort))
	{
		return ERR;
	}

	if(ERROR== if_logic_port_to_index(ulPort,&ulIfIndex))
	{
		return ERR;
	}
	* pulIp = 0;
	ret = lldp_get_api(ulIndex,LLDP_PORT_EN,&ulPortEn);
	if((ret == OK)&&(ulPortEn == LLDP_PORT_DISABLE))
	{
		return ERR;
	}
	while(OK == lldp_remote_get_next(ulRemIndex,&ulRemNextIndex))
	{
		lldp_get_api(ulRemNextIndex,LLDP_REMOTE_PORT_NUM,&ulLocIfIndex);
		//	printf("%s %d  *****ulLocIfIndex  0x%x, ulIndex 0x%x******\n", __FUNCTION__,__LINE__,ulLocIfIndex,ulIndex);
		//   lldp_rem_get_api(ulRemNextIndex,LLDP_REMOTE_PORT_NUM,&ulRemIfIndex);
		if(ulIndex == ulLocIfIndex)
		{
			lldp_remote_get_api(ulRemNextIndex, LLDP_REMOTE_MANAGE_ADDR, pulIp);
			//		printf("%s %d  *****pulIp  0x%x******\n", __FUNCTION__,__LINE__,*pulIp);
			//ulRemIndex= ulRemNextIndex;
			return OK;
		}
		else
		{
			ulRemIndex= ulRemNextIndex;
			continue;
		}
	}

	return ret;
}

/*dcn 轮询检测邻居ip地址是否存在,存在则返回OK*/
int ospf_dcn_rem_man_addr_check(u_long ulIp,u_long *pulPortNum)
{
	int ret = OK;
	u_long ulRemNextIndex = 0;
	u_long ulRemIndex = 0,ulRemIp = 0;
	u_long ulIfindx = 0;

	if(pulPortNum == NULL)
	{
		return ERR;
	}
	if(OK != lldp_remote_get_first(&ulRemIndex))
	{
		return ERR;
	}
	ret = lldp_remote_get_api(ulRemIndex, LLDP_REMOTE_MANAGE_ADDR,&ulRemIp);/*获取远端ip*/
	if((ulRemIp == ulIp)&&(OK == ret))
	{
		ret = lldp_remote_get_api(ulRemIndex, LLDP_REMOTE_PORT_NUM,&ulIfindx);/*获取本端索引*/
		if(ERROR == ifindex_to_logic_port(ulIfindx, pulPortNum))
		{
			return ERR;
		}
		return OK;
	}

	while(OK == lldp_remote_get_next(ulRemIndex,&ulRemNextIndex))
	{
		ret = lldp_remote_get_api(ulRemNextIndex, LLDP_REMOTE_MANAGE_ADDR,&ulRemIp);/*获取远端ip*/
		if((ulRemIp == ulIp)&&(OK == ret))
		{
			ret = lldp_remote_get_api(ulRemNextIndex, LLDP_REMOTE_PORT_NUM,&ulIfindx);/*获取本端索引*/

			if(ERROR == ifindex_to_logic_port(ulIfindx, pulPortNum))
			{
				return ERR;
			}

			return OK;
		}
		else
		{
			ulRemIndex= ulRemNextIndex;
			continue;
		}
	}

	return ERR;
}

/*dcn 轮询检测指定接口邻居ip地址是否存在,存在则返回OK*/
int ospf_dcn_rem_man_ifindex_addr_check(u_long ulIfindx,u_long ulIp)
{
	int ret = OK;
	u_long ulRemNextIndex = 0,ulLocIfIndex = 0;
	u_long ulRemIndex = 0,ulRemIp = 0;
	u_long ulPort = 0,ulIndex = 0;

	/*子接口索引转换为接口索引*/

	if(ERROR== if_vport_ifindex_to_logic_port(ulIfindx,&ulPort))
	{
		return ERR;
	}

	if(ERROR== if_logic_port_to_index(ulPort,&ulIndex))
	{
		return ERR;
	}

	if(OK != lldp_remote_get_first(&ulRemIndex))
	{
		return ERR;
	}
	lldp_remote_get_api(ulRemIndex,LLDP_REMOTE_PORT_NUM,&ulLocIfIndex); /*获取本端接口索引*/
	lldp_remote_get_api(ulRemIndex, LLDP_REMOTE_MANAGE_ADDR,&ulRemIp); /*获取远端ip*/

	if((ulLocIfIndex == ulIndex)&&(ulRemIp == ulIp))
	{
		//     printf("ospf_dcn_rem_man_ifindex_addr_check : OK ulIndex 0x%x ulRemIp 0x%x\n", ulIndex, ulRemIp);
		return OK;
	}

	while(OK == lldp_remote_get_next(ulRemIndex,&ulRemNextIndex))
	{
		lldp_remote_get_api(ulRemNextIndex,LLDP_REMOTE_PORT_NUM,&ulLocIfIndex); /*获取本端接口索引*/
		lldp_remote_get_api(ulRemNextIndex, LLDP_REMOTE_MANAGE_ADDR,&ulRemIp);/*获取远端ip*/

		if((ulLocIfIndex == ulIndex)&&(ulRemIp == ulIp))
		{
			//    printf("ospf_dcn_rem_man_ifindex_addr_check : OK ulIndex 0x%x ulRemIp 0x%x\n", ulIndex, ulRemIp);
			return OK;
		}
		ulRemIndex= ulRemNextIndex;

	}

	return ERR;
}
#endif

int ospf_ip_route_brief(struct rib *rib, struct ospf_routetokernal *pstOspf)
{
	struct nexthop *nexthop;

	for (nexthop = rib->nexthop; nexthop; nexthop = nexthop->next)
	{
		if (!CHECK_FLAG(nexthop->flags, NEXTHOP_FLAG_ACTIVE))
		{
			continue;
		}
		if (nexthop->gate.ipv4.s_addr == (in_addr_t) pstOspf->fwdaddr)
		{
			return ERR;
		}
	}

	return OK;
}

int ospf_add_route(struct ospf_iproute *pstOspf,
		struct ospf_process *pstProcess)
{
	ospf_zebra_add(pstOspf, pstProcess);
	return OK;
}

#define ERR_RTNOEXIST  0

int ospf_del_route(struct ospf_iproute *pstOspf,
		struct ospf_process *pstProcess)
{
	ospf_zebra_delete(pstOspf, pstProcess);

	return OK;
}

#if 0
/*LDP-OSPF联动 */
void ospf_ldp_up_msg(struct ospf_if * pstOspfIf)
{
	pstOspfIf->ucHoldCostState = FALSE;
	ospf_if_hold_cost_stop(pstOspfIf);
	/*mpls ldp is up*/
	pstOspfIf->ulOspfSyncState = OSPF_LDP_SYNC_ACHIEVED;
	ospf_logx(ospf_debug_rtm,
			"ospf ldp sync change to OSPF_LDP_SYNC_ACHIEVED %s %d\r\n",
			__FUNCTION__, __LINE__);
}

void ospf_ldp_init_msg(struct ospf_if * pstOspfIf)
{
	ospf_if_hold_down_stop(pstOspfIf);
	pstOspfIf->ucHoldCostState = TRUE;
	ospf_if_hold_cost_start(pstOspfIf, OSPF_LDP_INIT_MSG);
	//ospf_if_hold_cost_stop(pstOspfIf);

}

void ospf_ldp_error_msg(struct ospf_if * pstOspfIf)
{
	pstOspfIf->ucHoldCostState = TRUE;
	ospf_if_hold_cost_start(pstOspfIf, OSPF_LDP_ERR_MSG);
}

void ospf_ldp_control(u_int uiIfIndex, u_int uiType)
{
	struct ospf_if *p_if = NULL;
	struct ospf_if start_if;

	start_if.ifnet_index = 0;
	start_if.addr = 0;

	for_each_node_noless(&ospf.real_if_table, p_if, &start_if)
	{
		/*interface index match or do not care*/
		if ((p_if->ifnet_uint == uiIfIndex) && (p_if->link_up == TRUE))
		{
			if (p_if->ucLdpSyncEn == FALSE)
			{
				continue;
			}

			switch (uiType)
			{
			case OSPF_LDP_INIT_MSG:
			{
				if (p_if->stat)
				{
					p_if->stat->ldp_msg_cnt[OSPF_LDP_INIT_MSG]++;
					log_time_print(p_if->stat->ldp_init_msg_time);
				}
				ospf_ldp_init_msg(p_if);
				break;
			}
			case OSPF_LDP_ERR_MSG:
			{
				if (p_if->stat)
				{
					p_if->stat->ldp_msg_cnt[OSPF_LDP_ERR_MSG]++;
					log_time_print(p_if->stat->ldp_err_msg_time);
				}
				ospf_ldp_error_msg(p_if);
				break;
			}
			case OSPF_LDP_UP_MSG:
			{
				if (p_if->stat)
				{
					p_if->stat->ldp_msg_cnt[OSPF_LDP_UP_MSG]++;
					log_time_print(p_if->stat->ldp_up_msg_time);
				}
				ospf_ldp_up_msg(p_if);
				break;
			}
			default:
			{
				break;
			}
			}
		}
	}
}
#endif
