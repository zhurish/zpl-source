/*caoyong add 2017.9.19*/
/*ospf模块中有很多依赖的接口暂时还没有实现，暂时先定义了一个空函数放在本文件*/

#include "ospf.h"

/*LLDP*/
#if 0
int lldp_set_api(u_int uiIfIndex,u_int uiCmd,void *pValue)
{
    
}

int lldp_get_api(u_int uiIfIndex,u_int uiCmd,void *pValue)
{
    
}

int lldp_remote_set_api(u_int uiRemoteIndex,u_int uiCmd,void *pValue)
{
    
}


int lldp_remote_get_api(u_int uiRemoteIndex,u_int uiCmd,void *pValue)
{
    
}

int lldp_remote_get_first(u_int *puiRemoteIndex)
{}


int lldp_remote_get_next(u_int uiRemoteIndex,u_int *puiNextRemoteIndex)
{}

int lldp_dcn_rem_nbr_del()
{
    return OK;
}

int lldp_rem_nbr_del_all()
{
    return OK;
}

int lldp_overlay_slave_clean_api(u_int uiIfIndex, u_int uiMask)
{
    return OK;
}

int lldp_rem_del_batch_by_if_index(u_int ulIfIndex)
{
    /*基于lag口的，暂时先不考虑*/
     return OK;
}
/*other*/
int ipv4_delet_rib(int iRouteType, int iPara, struct prefix_ipv4 *p, struct in_addr *pNextHop, u_int uiIfUint, u_int uiVrid, u_int uiSubAddrId)
{
    return 0;
}





int isis_get_api(u_int uiIfIndex,u_int uiCmd,void *pValue)
{

}

/*ARP_INFO_T *arp_info_get(ARP_INFO_INDEX_T *pArpIndex)
{
    return NULL;
}*/
#endif
int pcl_apply_set_api(char *pcPolicyName, u_int uiCmd, void *pValue)
{
}

BOOL ospf_get_cfg_state()
{
    return TRUE;
}
#if 0
int if_ifindex_to_sdk_net_ifindex(u_int uiIfIndex,u_int *pulSdkNetIndex)
{
	return OK;
}
#ifdef USE_LINUX_OS
int ospf_if_index_to_sys_index_dcn(u_int uiIp,u_int *puiSysIndex)
{
    int iRet = ERROR;
    struct ifaddrs *ifAddrStruct  = NULL;
    void  *tmpAddrPtr = NULL;
    u_int uiIp1 = 0;
    u_int uiIp2 = 0;
    
    memcpy(&uiIp1, &uiIp, 4);
    {
         getifaddrs(&ifAddrStruct);
         while (ifAddrStruct!=NULL) 
         {
            if(ifAddrStruct->ifa_addr->sa_family==AF_INET)
            { // check it is IP4 is a valid IP4 Address
                tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                memcpy(&uiIp2, &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr, 4);
                vty_out_to_all_terminal("%x %x",uiIp1,uiIp2);
                if(uiIp1 == uiIp2)
                {
                    *puiSysIndex = if_nametoindex(ifAddrStruct->ifa_name);
                    return OK;
                }
                //printf("%s IP Address %s\n", ifAddrStruct->ifa_name, addressBuffer); 
            } 
            ifAddrStruct=ifAddrStruct->ifa_next;
        } 
    }

    return ERROR;
}
#endif
#endif
int ospf_if_index_to_sys_index(u_int uiIfIndex,u_int *puiSysIndex)
{
	*puiSysIndex = ifindex2ifkernel(uiIfIndex);
    return OK;
}

int ospf_sys_index_to_if_index(u_int uiVrfid, ifindex_t uiSysIndex,u_int *puiIfIndex)
{
	*puiIfIndex = ifkernel2ifindex(uiSysIndex);
    return OK;
}   

#if 0
int CheckMaskAddr(u_int uiMask)
{
    return OK;
}

int arp_refresh_by_lldp()
{
    return OK;
}

int uspGetSlot()
{
    return 1;
}

int uspGetApi(u_int uiIndex, u_int uiCmd, void *pValue)
{
    return OK;
}

int uspVpIfToSubId(u_int uiIfIndex)
{
    return 1;
}

int mpls_port_set_api(u_int uiIndex, u_int uiCmd, void *pValue)
{
    return OK;
}

int ospf_dcn_route_check(u_int uiIp)
{
    return OK;
}

void ospf_msg_send(u_int uiMsgType, void *pBuf)
{
/*	IPC_MSG_T stIpcMsg = {0};
	stIpcMsg.pucBuf = (u_int *)pBuf;
	stIpcMsg.ulMsgId = IPC_MSG_QUE_OSPF;
	stIpcMsg.ulMsgType = uiMsgType;
	stIpcMsg.ulMsgLen = sizeof(IPC_MSG_T);
	if(OK != vos_msg_queue_send(IPC_MSG_QUE_OSPF,&stIpcMsg,NO_WAIT,MSG_PRI_NORMAL))
	{
	    XFREE(stIpcMsg.pucBuf, MTYPE_MSGQUEUE);
	    stIpcMsg.pucBuf = NULL;
	}
	*/
	return;	
}

u_int ospf_zebra_route_translate_to_m2_route(u_int uiZebraRouteType)
{
    switch(uiZebraRouteType)
    {
        case ZEBRA_ROUTE_CONNECT:
        {
            return M2_ipRouteProto_local;
        }
        case ZEBRA_ROUTE_STATIC:
        {
            return M2_ipRouteProto_netmgmt;
        }
        case ZEBRA_ROUTE_RIP:
        {
            return M2_ipRouteProto_rip;
        }
        case ZEBRA_ROUTE_OSPF:
        //case ZEBRA_ROUTE_PROTOCOL_OSPF_ASE:
        {
            return M2_ipRouteProto_ospf;
        }
        case ZEBRA_ROUTE_ISIS:
        {
            return M2_ipRouteProto_is_is;
        }
        case ZEBRA_ROUTE_BGP:
        {
            return M2_ipRouteProto_bgp;
        }
        default:
        {
            return M2_ipRouteProto_max;
        }
    }  

}

u_int ospf_m2_route_translate_to_zebra_route(u_int uiM2RouteType)
{
    switch(uiM2RouteType)
    {
        case M2_ipRouteProto_local:
        {
            return ZEBRA_ROUTE_CONNECT;
        }
        case M2_ipRouteProto_netmgmt:
        {
            return ZEBRA_ROUTE_STATIC;
        }
        case M2_ipRouteProto_rip:
        {
            return ZEBRA_ROUTE_RIP;
        }
        case M2_ipRouteProto_ospf:
        {
            return ZEBRA_ROUTE_OSPF;
        }
        case M2_ipRouteProto_is_is:
        {
            return ZEBRA_ROUTE_ISIS;
        }
        case M2_ipRouteProto_bgp:
        {
            return ZEBRA_ROUTE_BGP;
        }
        default:
        {
            return ZEBRA_ROUTE_MAX;
        }
    }  

}
#endif
// rc = ospf_add_route_to_system(p_process->vrid,uiRouteType,p_route->dest, p_route->fwdaddr,p_route->mask,0,p_route->metric);
int ospf_add_route_to_system(u_int16 vrfid, u_int8 proto, u_int dest, u_int nexthop, u_int mask, u_int a, u_int metric)
{

    return OK;
}

int ospf_delete_route_to_system(u_int16 vrfid, u_int8 proto, u_int dest, u_int nexthop, u_int mask, u_int a, u_int metric)
{
	return OK;
}

#if 0
int ospf_add_route_to_system(ZEBRA_ROUTE_CONFIG_INFO_T *pstRouteInfo)
{
    ZEBRA_ROUTE_INDEX_T stRouteIndex;
    int ret = ERROR;

    memset(&stRouteIndex, 0, sizeof(ZEBRA_ROUTE_INDEX_T));
    
    ret = zebra_ip_route_set_api(&stRouteIndex, ZEBRA_DYNAMIC_ROUTE_ADD, pstRouteInfo);

    return ret;
}

int ospf_delete_route_to_system(ZEBRA_ROUTE_CONFIG_INFO_T *pstRouteInfo)
{
    ZEBRA_ROUTE_INDEX_T stRouteIndex;
    int ret = ERROR;

    memset(&stRouteIndex, 0, sizeof(ZEBRA_ROUTE_INDEX_T));

    ret = zebra_ip_route_set_api(&stRouteIndex, ZEBRA_DYNAMIC_ROUTE_DELETE, pstRouteInfo);

    return ret;
}

int ospf_get_exist_if_ip(u_int uiIndex, u_int uiVrf, u_int *pIpAddr)
{
    int iRet = ERROR;
    u_int uiIfVrf = 0;
    struct prefix_ipv4 stPrefixIp = {0};
    
    if(OK != zebra_if_get_api(uiIndex, ZEBRA_IF_VRF_ID_GET, &uiIfVrf) || uiVrf != uiIfVrf)
    {
        return ERROR;
    }

    iRet = zebra_if_get_api(uiIndex, ZEBRA_IF_IPADRESS_GET, &stPrefixIp);
    
    memcpy(pIpAddr, &stPrefixIp.prefix, 4);

    return iRet;
}

u_int ospf_get_ldp_session_state_by_ifindex(u_int uiVrfId, u_int uiIfIndex)
{
    int iRet = 0;
    tLDP_PEER_INDEX stIndex = {0};
    tLDP_PEER_INDEX stNextIndex = {0};
    u_int uiLdpState = 0;
    u_int uiIfunit = 0;
    
    iRet = ldpPeerGetFirst(&stIndex);
    while(iRet == OK)
    {
        ldpPeerGetApi(&stIndex, LDP_PEER_IFUNIT, &uiIfunit);
        if(uiVrfId == stIndex.process_id && uiIfIndex == uiIfunit)
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

    return uiLdpState;
}
#endif
