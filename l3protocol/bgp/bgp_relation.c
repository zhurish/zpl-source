/*caoyong add 2017.9.28*/
/*bgp模块中有很多依赖的接口暂时还没有实现，暂时先定义了一个空函数放在本文件*/

#include "bgp4com.h"
#include "mpls_api.h"
#include "mpls_l3vpn.h"
#include "bgp_relation.h"
#include "zebra_api.h"
#include "l3vpn.h"
#include "l3vpn_api.h"
#include "if.h"

STATUS rtSockOptionSet(int sockfd,int protoType,int opType)
{
    return VOS_OK;
}

int errnoGet()
{
    return errno;
}

int sendBgpBackwardTransitionTrap(int addrType,char* peerid, int state,char *LastError)
{
    return VOS_OK;
}

int uspGetIfUnitByIp(int vrId, int family, u_char *addr, u_int *ifIndex)
{
    u_int uiAddr = 0;
    ZEBRA_IP_INDEX_T stIpIndex = {0};
    int iRet = 0;
    octetstring stIfNameInfo = {0};
    u_char pucIfName[64] = {0};

    memset(&stIpIndex, 0, sizeof(ZEBRA_IP_INDEX_T));
    uiAddr = bgp_ip4(addr);
    uiAddr = htonl(uiAddr);
    memcpy(&stIpIndex.ucszIpAddr, &uiAddr, 4);
    stIpIndex.family = family;
    stIpIndex.uiMaskLen = 32;
    stIpIndex.uiVrfId = vrId;

    iRet = zebra_ip_vpn_get_api(&stIpIndex, ZEBRA_IF_GET_BY_VRF_IP, ifIndex);
    bgp4_log(BGP_DEBUG_EVT, "bgp get interface index by ip address %x index %d", uiAddr, *ifIndex);   
    
	memset(pucIfName, 0, 64);
	stIfNameInfo.len = 64;
	stIfNameInfo.pucBuf = pucIfName;
	iRet = zebra_if_get_api(*ifIndex, ZEBRA_IF_NAME, &stIfNameInfo);
	bgp4_log(BGP_DEBUG_EVT, "bgp get interface index by ip interface name %s", stIfNameInfo.pucBuf); 
	
    return VOS_OK;
}

int32_t uspSelfSysIndex()
{
    return 1;
}

int findMaxIfIpAddr()
{
    return 0xFEFEFEFE;
}

STATUS systemIPv4RouteAdd(u_int vrf, u_int proto, u_int dest, u_int nhop, u_int masklen, u_int flag, u_int distance, u_int cost)
{
    ZEBRA_ROUTE_INDEX_T stRouteIndex;
    ZEBRA_ROUTE_CONFIG_INFO_T stRouteInfo;
    int ret = VOS_ERR;
    u_int destAddr = 0;
    u_int nextHop = 0;

    memset(&stRouteIndex, 0, sizeof(ZEBRA_ROUTE_INDEX_T));
    memset(&stRouteInfo, 0, sizeof(ZEBRA_ROUTE_CONFIG_INFO_T));

    destAddr = ntohl(dest);
    nextHop = ntohl(nhop);
    
    stRouteInfo.ulVrfId = vrf;
    stRouteInfo.uiRouteProtocolType = route_protocol_translate_to_zebra(proto);
    stRouteInfo.stDstPrefixIpv4.family = AF_INET;
    stRouteInfo.stDstPrefixIpv4.prefixlen = masklen;
    memcpy(&stRouteInfo.stDstPrefixIpv4.prefix, &destAddr, 4);
    memcpy(&stRouteInfo.stNextHopIp, &nextHop, 4);
    stRouteInfo.ulDstIpMaskLen = masklen;
    stRouteInfo.ulZebraMessageFlag = flag;
    stRouteInfo.ucDistance = distance;
    stRouteInfo.ulMetric = cost;
    stRouteInfo.family = AF_INET;
    stRouteInfo.ucSafi = SAFI_UNICAST;

    bgp4_log(BGP_DEBUG_EVT,"vrf %d add proto %d dest %x mask %d flag %x\n",vrf,proto,dest,masklen,flag);
    ret = zebra_ip_route_set_api(&stRouteIndex, ZEBRA_DYNAMIC_ROUTE_ADD, &stRouteInfo);

    return ret;
}

STATUS systemIPv4RouteDelete(u_int vrf, u_int proto, u_int dest, u_int nhop, u_int masklen, u_int flag, u_int distance, u_int cost)
{
    ZEBRA_ROUTE_INDEX_T stRouteIndex;
    ZEBRA_ROUTE_CONFIG_INFO_T stRouteInfo;
    int ret = VOS_ERR;
    u_int destAddr = 0;
    u_int nextHop = 0;

    memset(&stRouteIndex, 0, sizeof(ZEBRA_ROUTE_INDEX_T));
    memset(&stRouteInfo, 0, sizeof(ZEBRA_ROUTE_CONFIG_INFO_T));

    destAddr = ntohl(dest);
    nextHop = ntohl(nhop);
    
    stRouteInfo.ulVrfId = vrf;
    stRouteInfo.uiRouteProtocolType = route_protocol_translate_to_zebra(proto);
    stRouteInfo.stDstPrefixIpv4.family = AF_INET;
    stRouteInfo.stDstPrefixIpv4.prefixlen = masklen;
    memcpy(&stRouteInfo.stDstPrefixIpv4.prefix, &destAddr, 4);
    memcpy(&stRouteInfo.stNextHopIp, &nextHop, 4);
    stRouteInfo.ulDstIpMaskLen = masklen;
    stRouteInfo.ulZebraMessageFlag = flag;
    stRouteInfo.ucDistance = distance;
    stRouteInfo.ulMetric = cost;
    stRouteInfo.family = AF_INET;
    stRouteInfo.ucSafi = SAFI_UNICAST;

    bgp4_log(BGP_DEBUG_EVT,"vrf %d delete proto %d dest %x mask %d flag %x\n",vrf,proto,dest,masklen,flag);
    ret = zebra_ip_route_set_api(&stRouteIndex, ZEBRA_DYNAMIC_ROUTE_DELETE, &stRouteInfo);

    return ret;
}

STATUS systemIPv6RouteAdd2(int s, int vrid, int proto, char * dest, char * nhop, int len, int flag, int cost)
{
    return VOS_OK;
}

STATUS systemIPv6RouteDelete2(int s, int vrid, int proto, char * dest, char * nhop, int len, int flag, int cost)
{
    return VOS_OK;
}

STATUS ipv4v6RouteLookup(u_int routeInstance,int addrFamily,char *dest, int mask,void *pRouteInfo)
{
    return VOS_OK;
}

int sysIndexTrans(void *pIndex, u_int cmd, void *pvalue)
{
    return VOS_OK;
}

int uspIpRouterId(u_int vrid ,u_int family ,u_int8 *addr)
{
    return VOS_OK;    
}

int bfdSessionBind(u_int uiProto, u_int uiIfUint, u_int uiIpType, u_int uiLen, u_char *pLocalIp, u_char* pIp, void *pValue)
{
    return VOS_OK;
}

int bfdSessionUnbind(u_int uiProto, int iDescribe, void *pValue)
{
    return VOS_OK;
}


int sendBgpEstablishedTrap(int addrType,char* peerid, int state, char *LastError)
{
    return VOS_OK;
}

STATUS ip_route_match(u_int routeInstance,uint32_t dest, void *pValue)
{
#if 0
    ZEBRA_ROUTE_MSG_T stRouteIndex;
    ZEBRA_ROUTE_MSG_T *pRouteInfo = NULL;
    u_int uiDest;
    u_int uiNextHop;

    pRouteInfo = (ZEBRA_ROUTE_MSG_T *)pValue;
    if(pRouteInfo == NULL)
    {
        return VOS_ERR;
    }
    
    memset(&stRouteIndex, 0, sizeof(ZEBRA_ROUTE_MSG_T));

    uiDest = ntohl(dest);
    stRouteIndex.family = AF_INET;
    stRouteIndex.uiIpMaskLen = 32;
    memcpy(&stRouteIndex.stDstIp, &uiDest, 4);
    
    if(VOS_OK != zebra_ipv4_route_get_api(&stRouteIndex, ZEBRA_MATCH_ROUTE, (void *)pRouteInfo))
    {
        return VOS_ERR;
    }

    memcpy(&uiDest, &pRouteInfo->stDstIp, 4);
    memcpy(&uiNextHop, &pRouteInfo->stNexthop, 4);
    
    printf("get match route %x %x %x\n", uiDest, uiNextHop, pRouteInfo->uiIpMaskLen);
#endif
    return VOS_OK;
}

STATUS ip6_route_match(u_int routeInstance,char *dest, M2_IPV6ROUTETBL *p_info)
{
    return VOS_OK;
}

int ipRouteRedisSet(int routeVrid, int addrFamily,int fromProtoType,int toProtoType,u_int processId)
{
    ZEBRA_ROUTE_MSG_T stRouteIndex;
    ZEBRA_ROUTE_MSG_T stRouteInfoNext; 
    tBGP4_PATH  path;
    tBGP4_ROUTE route;
    tBGP4_ADDR  dstAddr;
    tBGP4_VPN_INSTANCE *p_instance = NULL; 
    int index = 0;
    u_int destAddr = 0;
    u_int nextHop = 0;
    u_int  uiIfIndex = IFINDEX_GENERATE(0, IFINDEX_TYPE_LOOPBACK_IF, IN_LOOPBACK_INDEX); 

    memset(&stRouteIndex, 0, sizeof(ZEBRA_ROUTE_MSG_T));
    memset(&stRouteInfoNext, 0, sizeof(ZEBRA_ROUTE_MSG_T));

    stRouteIndex.family = AF_INET;
    stRouteIndex.uiVrfId = routeVrid;
    //stRouteIndex.uiType = route_protocol_translate_to_zebra(fromProtoType);
    if(VOS_OK != zebra_ip_route_get_api(&stRouteIndex, ZEBRA_ROUTE_NEXT, (void *)&stRouteInfoNext))
    {
        printf("route get null\n");
        return VOS_ERR;
    }

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, routeVrid);
    if (p_instance == NULL)
    {
        printf("instance get error\n");
        return VOS_ERR;
    }
    
    do
    {
        if(stRouteInfoNext.uiIfIndex != uiIfIndex && fromProtoType == route_protocol_zebra_translate(stRouteInfoNext.uiRouteProtocolType))
        {
            if(fromProtoType == M2_ipRouteProto_ospf && stRouteInfoNext.iProcessId != processId)
            {
                return VOS_ERR;
            }
            memcpy(&destAddr, &stRouteInfoNext.stDstIp, 4);
            memcpy(&nextHop, &stRouteInfoNext.stNexthop, 4);
            //printf("get %x %x %x\n", destAddr, stRouteInfoNext.uiIpMaskLen, nextHop);
        
            memset(&route, 0, sizeof(route));
            memset(&path, 0, sizeof(path));
            bgp4_path_init(&path);
            
            memset(&dstAddr, 0, sizeof(tBGP4_ADDR));
            dstAddr.afi = BGP4_PF_IPUCAST;
            dstAddr.prefixlen = stRouteInfoNext.uiIpMaskLen;
            ip_uint_to_char(htonl(destAddr), dstAddr.ip);        
            memcpy(&route.dest, &dstAddr, sizeof(tBGP4_ADDR));
            path.af = route.dest.afi;
            route.proto = fromProtoType;
            route.processId = processId;
            
            route.p_path = &path;
            path.p_instance = p_instance;
            path.origin_vrf = p_instance->vrf;
            path.nexthop.afi = BGP4_PF_IPUCAST;
            path.nexthop.prefixlen = 32;
            path.origin = BGP4_ORIGIN_INCOMPLETE;
            ip_uint_to_char(htonl(nextHop), path.nexthop.ip);
            /*local route has no nexthop*/
            if (route.proto == M2_ipRouteProto_local)
            {
                memset(path.nexthop.ip, 0, 4);
            }
            path.med = stRouteInfoNext.uiMetric;
            
            bgp4_link_path(&path, &route);

            bgp4_redistribute_route(p_instance, &route, TRUE);
        }
        
        memcpy(&stRouteIndex, &stRouteInfoNext, sizeof(ZEBRA_ROUTE_MSG_T));
        memset(&stRouteInfoNext, 0, sizeof(ZEBRA_ROUTE_MSG_T));      
    }
    while(VOS_OK == zebra_ip_route_get_api(&stRouteIndex, ZEBRA_ROUTE_NEXT, (void *)&stRouteInfoNext));
    
    return VOS_OK;
}

void bgp_printf_rd_or_rt(u_char *pName, u_char *pInfo)
{
    bgp4_log(BGP_DEBUG_EVT,"%s:%x %x %x %x %x %x %x %x\n",pName,
           pInfo[0],pInfo[1],pInfo[2],pInfo[3],
           pInfo[4],pInfo[5],pInfo[6],pInfo[7]);
}

int bgp_translate_rd_from_l3vpn(u_int uiAdministator, u_int uiAssignedNumber, u_char ucRdType, u_char *rdInfo)
{
    u_int uiNetAdministator = 0;
    u_int uiNetAssignedNumber = 0;
    u_short usNetAssignedNumber = 0;

    uiNetAdministator = htonl(uiAdministator);
    uiNetAssignedNumber = htonl(uiAssignedNumber);
    
    if(NULL == rdInfo || ucRdType > L3VPN_PREFIX_TYPE_IP)
    {
        return VOS_ERR;
    }

    if(ucRdType == L3VPN_PREFIX_TYPE_DIGIT)
    {
        memcpy(rdInfo, &uiNetAdministator, 4);
        memcpy(rdInfo+4, &uiNetAssignedNumber, 4);
    }
    else if(ucRdType == L3VPN_PREFIX_TYPE_IP)
    {
        usNetAssignedNumber = htons((u_short)uiAssignedNumber);
        rdInfo[0] = 0;
        rdInfo[1] = 1;
        memcpy(rdInfo+2, &uiNetAdministator, 4);
        memcpy(rdInfo+6, &usNetAssignedNumber, 2);
    }

    return VOS_OK;
}

int bgp_translate_rt_from_l3vpn(u_int uiAdministator, u_int uiAssignedNumber, u_char ucRtType, u_char *rtInfo)
{
    u_int uiNetAdministator = 0;
    u_int uiNetAssignedNumber = 0;
    u_short usNetAssignedNumber = 0;

    uiNetAdministator = htonl(uiAdministator);
    uiNetAssignedNumber = htonl(uiAssignedNumber);
    
    if(NULL == rtInfo || ucRtType > L3VPN_PREFIX_TYPE_IP)
    {
        return VOS_ERR;
    }

    if(ucRtType == L3VPN_PREFIX_TYPE_DIGIT)
    {
        memcpy(rtInfo, &uiNetAdministator, 4);
        memcpy(rtInfo+4, &uiNetAssignedNumber, 4);
        rtInfo[0] = 0;
        rtInfo[1] = 2;
    }
    else if(ucRtType == L3VPN_PREFIX_TYPE_IP)
    {
        usNetAssignedNumber = htons((u_short)uiAssignedNumber);
        rtInfo[0] = 1;
        rtInfo[1] = 2;
        memcpy(rtInfo+2, &uiNetAdministator, 4);
        memcpy(rtInfo+6, &usNetAssignedNumber, 2);
    }

    return CMD_SUCCESS;
}

int mplsLookupL3Vpn(tBgpLookupL3Vpn *pL3VpnInfo)
{
#if 1
    int ret = VOS_ERR;
    L3VPN_INSTANCE_T stVpnInstance;
    L3VPN_INSTANCE_T *pVpnInstance = NULL;
    L3VPN_TARGET_INDEX_T stVpnTargetIndex;
    L3VPN_TARGET stVpnTarget;
    L3VPN_TARGET *pVpnTargetNext = NULL;
    int index = 0;
    u_int uiAdministator = 0;
    u_int uiAssignedNumber = 0;

    memset(&stVpnInstance, 0, sizeof(L3VPN_INSTANCE_T));
    memset(&stVpnTargetIndex, 0, sizeof(L3VPN_TARGET_INDEX_T));
    memset(&stVpnTarget, 0, sizeof(L3VPN_TARGET));
    
    if(NULL == pL3VpnInfo)
    {
        return VOS_ERR;
    }
    
    switch(pL3VpnInfo->type)
    {
        case MPLSL3VPN_GET_LOCALVRF:
            break;
        case MPLSL3VPN_GET_REMOTEVRF:
            stVpnTargetIndex.ucRtDirection = L3VPN_INT_TARGET;
            stVpnTargetIndex.ucFamilyType = L3VPN_FAMILY_IPV4;
            if(pL3VpnInfo->inRT[0] == 0 && pL3VpnInfo->inRT[1] == 2)
            {
                pL3VpnInfo->inRT[1] = 0;
                memcpy(&uiAdministator, pL3VpnInfo->inRT, 4);
                memcpy(&uiAssignedNumber, &pL3VpnInfo->inRT[4], 4);
            }
            else if(pL3VpnInfo->inRT[0] == 1 && pL3VpnInfo->inRT[1] == 2)
            {
                memcpy(&uiAdministator, &pL3VpnInfo->inRT[2], 4);
                memcpy(&uiAssignedNumber, &pL3VpnInfo->inRT[4], 4);
                /*低两个字节没有用*/
                uiAssignedNumber = uiAssignedNumber & 0xFFFF0000;
            }

            stVpnTargetIndex.uiVpnAdministrator = ntohl(uiAdministator);
            stVpnTargetIndex.uiAssignedNumber = ntohl(uiAssignedNumber);
            bgp4_log(BGP_DEBUG_EVT, "mpls l3vpn get remote vrf rt  %u:%u",stVpnTargetIndex.uiVpnAdministrator, stVpnTargetIndex.uiAssignedNumber);
            ret = l3vpn_get_api(&stVpnTargetIndex, L3VPN_GET_FIRST_BY_TARGET, &stVpnInstance);

            while(ret == VOS_OK)
            {
                if(stVpnInstance.uiInstanceId != 0)
                {
                    bgp4_log(BGP_DEBUG_EVT, "mpls l3vpn get remote vrf instanceId %d vpn name  %s", stVpnInstance.uiInstanceId, stVpnInstance.ucszL3VpnName);
                    pL3VpnInfo->outData[index].vrfId = stVpnInstance.uiInstanceId;
                    index++;
                }
                
                ret = l3vpn_get_api(&stVpnTargetIndex, L3VPN_GET_NEXT_BY_TARGET, &stVpnInstance);
            }

            pL3VpnInfo->count = index;
            ret = VOS_OK;
            break;
        case MPLSL3VPN_GET_RD_BY_VRFID:
            pVpnInstance = l3vpn_get_instance_by_id(pL3VpnInfo->inVrfId);
            if(pVpnInstance != NULL)
            {
                pL3VpnInfo->outData[0].vrfId = pVpnInstance->uiInstanceId;
                uiAdministator = ntohl(pVpnInstance->stVpnFamilyEntry[0].uiVpnAdministrator);
                uiAssignedNumber = ntohl(pVpnInstance->stVpnFamilyEntry[0].uiAssignedNumber);
                memcpy(pL3VpnInfo->outData[0].vrfRT, &uiAdministator, 4);
                memcpy(&pL3VpnInfo->outData[0].vrfRT[4], &uiAssignedNumber, 4);
                bgp4_log(BGP_DEBUG_EVT,"mpls l3vpn get route-distinguisher by vrf id %d rd %u:%u",
                         pVpnInstance->uiInstanceId,
                         pVpnInstance->stVpnFamilyEntry[0].uiVpnAdministrator,
                         pVpnInstance->stVpnFamilyEntry[0].uiAssignedNumber);
                bgp_translate_rd_from_l3vpn(pVpnInstance->stVpnFamilyEntry[0].uiVpnAdministrator,
                                            pVpnInstance->stVpnFamilyEntry[0].uiAssignedNumber,
                                            pVpnInstance->stVpnFamilyEntry[0].ucRdType,
                                            pL3VpnInfo->outData[0].vrfRT);
                bgp_printf_rd_or_rt("RD", pL3VpnInfo->outData[0].vrfRT);
                pL3VpnInfo->count = 1;
                ret = VOS_OK;
            }
            break;
        case MPLSL3VPN_GET_VIDANDRD:
            ret = l3vpn_get_api(NULL, L3VPN_GET_FIRST, &stVpnInstance);
            if(ret == VOS_OK)
            {
                pL3VpnInfo->outData[0].vrfId = stVpnInstance.uiInstanceId;
                bgp_translate_rd_from_l3vpn(stVpnInstance.stVpnFamilyEntry[0].uiVpnAdministrator,
                                            stVpnInstance.stVpnFamilyEntry[0].uiAssignedNumber,
                                            stVpnInstance.stVpnFamilyEntry[0].ucRdType,
                                            pL3VpnInfo->outData[0].vrfRT);
                bgp4_log(BGP_DEBUG_EVT,"mpls l3vpn get  vrf %d RD %u:%u",
                         stVpnInstance.uiInstanceId,
                         stVpnInstance.stVpnFamilyEntry[0].uiVpnAdministrator, 
                         stVpnInstance.stVpnFamilyEntry[0].uiAssignedNumber);
                bgp_printf_rd_or_rt("RD", pL3VpnInfo->outData[0].vrfRT);
                index = 1;
            }

            while((ret == VOS_OK) && (VOS_OK == l3vpn_get_api(NULL, L3VPN_GET_NEXT, &stVpnInstance)) && index < BGP4_MAX_VRF_ID)
            {   
                pL3VpnInfo->outData[index].vrfId = stVpnInstance.uiInstanceId;
                bgp4_log(BGP_DEBUG_EVT,"mpls l3vpn get  vrf %d RD %u:%u",
                         stVpnInstance.uiInstanceId,
                         stVpnInstance.stVpnFamilyEntry[0].uiVpnAdministrator, 
                         stVpnInstance.stVpnFamilyEntry[0].uiAssignedNumber);
                bgp_translate_rd_from_l3vpn(stVpnInstance.stVpnFamilyEntry[0].uiVpnAdministrator,
                                            stVpnInstance.stVpnFamilyEntry[0].uiAssignedNumber,
                                            stVpnInstance.stVpnFamilyEntry[0].ucRdType,
                                            pL3VpnInfo->outData[index].vrfRT);
                bgp_printf_rd_or_rt("RD", pL3VpnInfo->outData[index].vrfRT);
                index++;
            }

            pL3VpnInfo->count = index;
            break;
        case MPLSL3VPN_GET_EXPORTRT:
            pVpnInstance = l3vpn_get_instance_by_id(pL3VpnInfo->inVrfId);
            
            if(pVpnInstance != NULL)
            {
                stVpnTargetIndex.ucFamilyType = L3VPN_FAMILY_IPV4;
                stVpnTargetIndex.ucRtDirection = L3VPN_OUT_TARGET;
                memcpy(stVpnTargetIndex.ucszL3VpnName, pVpnInstance->ucszL3VpnName, L3VPN_NAME_LEN);
                bgp4_log(BGP_DEBUG_EVT,"mpls l3vpn get vpn-target export-extcommunity vpn name %s",stVpnTargetIndex.ucszL3VpnName);
                ret = l3vpn_target_entry_get_api(&stVpnTargetIndex, TARGET_GET_FIRST, &stVpnTarget);
                
                if(ret == VOS_OK)
                {
                    bgp4_log(BGP_DEBUG_EVT,"mpls l3vpn get vpn-target export-extcommunity,rt %u:%u",
                             stVpnTarget.stL3VpnTargetIndex.uiVpnAdministrator,
                             stVpnTarget.stL3VpnTargetIndex.uiAssignedNumber);
                    bgp_translate_rt_from_l3vpn(stVpnTarget.stL3VpnTargetIndex.uiVpnAdministrator,
                                                stVpnTarget.stL3VpnTargetIndex.uiAssignedNumber,
                                                stVpnTarget.ucRtType, pL3VpnInfo->outData[0].vrfRT);
                    bgp_printf_rd_or_rt("RT", pL3VpnInfo->outData[0].vrfRT);
                    index = 1;
                }

                while((ret == VOS_OK) 
                       && (VOS_OK == l3vpn_target_entry_get_api(&stVpnTargetIndex, TARGET_GET_NEXT, &stVpnTarget)) 
                       && index < BGP4_MAX_ECOMMNUITY)
                {
                    bgp4_log(BGP_DEBUG_EVT,"mpls l3vpn get vpn-target export-extcommunity, rt %u:%u",
                             stVpnTarget.stL3VpnTargetIndex.uiVpnAdministrator,
                             stVpnTarget.stL3VpnTargetIndex.uiAssignedNumber);
                    bgp_translate_rt_from_l3vpn(stVpnTarget.stL3VpnTargetIndex.uiVpnAdministrator,
                                                stVpnTarget.stL3VpnTargetIndex.uiAssignedNumber,
                                                stVpnTarget.ucRtType, pL3VpnInfo->outData[index].vrfRT);
                    bgp_printf_rd_or_rt("RT", pL3VpnInfo->outData[index].vrfRT);
                    index++;
                }

                pL3VpnInfo->count = index;
            }
            break;
        default:
            break;
    }

    return ret;
#endif    
}

int bgp4_import_route_process(tBGP4_VPN_INSTANCE *p_instance, tBGP4_ADDR *p_destAddr, tBGP4_ADDR *p_nextHop, u_int proto, u_int cost, u_int new_route)
{
    tBGP4_PATH  path;
    tBGP4_ROUTE route;

    memset(&route, 0, sizeof(route));
    memset(&path, 0, sizeof(path));
    bgp4_path_init(&path);

    memcpy(&route.dest, p_destAddr, sizeof(tBGP4_ADDR));
    path.af = route.dest.afi;
    route.proto = proto;
    route.p_path = &path;
    path.p_instance = p_instance;
    path.origin_vrf = p_instance->vrf;
    path.med = cost;
    path.origin = BGP4_ORIGIN_INCOMPLETE;
    memcpy(&path.nexthop, p_nextHop, sizeof(tBGP4_ADDR));
    
    bgp4_link_path(&path, &route);
    
    bgp4_redistribute_route(p_instance, &route, new_route);
    
    return VOS_OK;
}

int bgp_redistribute_route_change_process(ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *pstRouteAddMsg)
{
    int ret = VOS_ERR;
    tBGP4_ADDR  dstAddr;
    tBGP4_ADDR  nextHopAddr;
    tBGP4_VPN_INSTANCE *p_instance = NULL; 
    u_int destAddr = 0;
    u_int nextHop = 0;
    u_int proto = 0;
    tBGP4_REDISTRIBUTE_CONFIG* pRetPolicy = NULL;
    tBGP4_REDISTRIBUTE_CONFIG stRetConfig = {0};
    u_int  uiIfIndex = IFINDEX_GENERATE(0, IFINDEX_TYPE_LOOPBACK_IF, IN_LOOPBACK_INDEX);
    
    if(NULL == pstRouteAddMsg)
    {
        return ret;
    }

    bgp_sem_take();
    if(!gbgp4.enable)
    {
        bgp_sem_give();
        return VOS_OK;
    }
    
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, pstRouteAddMsg->stRouteMsg.uiVrfId);
    if (p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT, "route change process get vpn instance vrfId %d  error",pstRouteAddMsg->stRouteMsg.uiVrfId);
        bgp_sem_give();
        return VOS_ERR;
    }
    #if 1
    if(uiIfIndex == pstRouteAddMsg->ulIfIndex)
    {
        bgp_sem_give();
        return VOS_ERR;
    }
    #endif
    proto = route_protocol_zebra_translate(pstRouteAddMsg->ulMsgType);
    
    memcpy(&destAddr, &pstRouteAddMsg->stRouteMsg.stDstIp, 4);
    memcpy(&nextHop, &pstRouteAddMsg->stRouteMsg.stNexthop, 4);
    //printf("get %x %x %x\n", destAddr, pstRouteAddMsg->stRouteMsg.uiIpMaskLen, nextHop);

    memset(&dstAddr, 0, sizeof(tBGP4_ADDR));
    dstAddr.afi = BGP4_PF_IPUCAST;
    dstAddr.prefixlen = pstRouteAddMsg->stRouteMsg.uiIpMaskLen;
    ip_uint_to_char(htonl(destAddr), dstAddr.ip);

    memset(&nextHopAddr, 0, sizeof(tBGP4_ADDR));
    nextHopAddr.afi = BGP4_PF_IPUCAST;
    nextHopAddr.prefixlen = 32;
    ip_uint_to_char(htonl(nextHop), nextHopAddr.ip);
    /*local route has no nexthop*/
    if (proto == M2_ipRouteProto_local)
    {
        memset(nextHopAddr.ip, 0, 4);
    }

    stRetConfig.af = BGP4_PF_IPUCAST;
    stRetConfig.policy = 0;
    stRetConfig.proto = proto;
    if(proto == M2_ipRouteProto_ospf)
    {
        stRetConfig.processId = pstRouteAddMsg->stRouteMsg.iProcessId;
    }
    pRetPolicy = bgp4_avl_lookup(&p_instance->redistribute_policy_table, &stRetConfig);
    if(pRetPolicy == NULL || pRetPolicy->status != SNMP_ACTIVE)
    {
        bgp4_log(BGP_DEBUG_EVT, "There is no redistribute vrf %d proto %d active",
                                pstRouteAddMsg->stRouteMsg.uiVrfId,proto);
        //printf("There is no redistribute vrf %d proto %d active\n",pstRouteAddMsg->stRouteMsg.uiVrfId,proto);
        bgp_sem_give();
        return VOS_ERR;
    }
    
    switch(pstRouteAddMsg->ulSubCode)
    {
        case ZEBRA_REDISTRIBUTE_TYPE_ROUTE_ADD:
            ret = bgp4_import_route_process(p_instance, &dstAddr, &nextHopAddr, proto, pstRouteAddMsg->stRouteMsg.uiMetric, TRUE);
            break;
        case ZEBRA_REDISTRIBUTE_TYPE_ROUTE_DELETE:
            ret = bgp4_import_route_process(p_instance, &dstAddr, &nextHopAddr, proto, pstRouteAddMsg->stRouteMsg.uiMetric, FALSE);
            break;
        default:
            break;
    }
    bgp_sem_give();
    
    bgp4_log(BGP_DEBUG_EVT, "bgp redistribute route change ");
    //printf("bgp_redistribute_route_change_process\n");
    
    return ret;
}

int bgp_if_link_change_process(ZEBRA_IF_MSG_REDISTRIBUTE_T *pstIfLinkMsg)
{
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_ADDR stIfAddr = {0};

    if(pstIfLinkMsg == NULL)
    {
        printf("pstIfLinkMsg is NULL\n");
        return VOS_ERR;
    }
    
    bgp4_log(BGP_DEBUG_EVT, "receive interface change, interface index %d, flag %x", 
        pstIfLinkMsg->ulIfIndex, pstIfLinkMsg->ulSubCode);
        
    bgp_sem_take();
    if(!gbgp4.enable)
    {
        bgp_sem_give();
        return VOS_OK;
    }
    
    switch(pstIfLinkMsg->ulSubCode)
    {
        case ZEBRA_REDISTRIBUTE_TYPE_IF_DOWN:
            bgp4_avl_for_each(&gbgp4.instance_table, p_instance)
            {
                bgp4_avl_for_each(&p_instance->peer_table, p_peer)
                {
                    if ((p_peer->if_unit == pstIfLinkMsg->ulIfIndex)
                        && (p_peer->state == BGP4_NS_ESTABLISH))
                    {
                        bgp4_timer_stop(&p_peer->hold_timer);
                        bgp4_peer_holdtimer_expired(p_peer);
                    }
                }
            }
            break;
        case ZEBRA_REDISTRIBUTE_TYPE_IF_ADDR_DEL:
            stIfAddr.afi = BGP4_PF_IPUCAST;
            stIfAddr.prefixlen = 4;
            memcpy(stIfAddr.ip, &pstIfLinkMsg->stIpPrefix.u, 4);
            bgp4_instance_for_each(p_instance)
            {
                bgp4_peer_for_each(p_instance, p_peer)
                {
                    if ((bgp4_prefixcmp(&stIfAddr, &p_peer->local_ip) == 0)
                        && (p_peer->if_unit == pstIfLinkMsg->ulIfIndex))
                    {
                        /*if both of peers support GR,then go to GR process,otherwise,go to normal restart process*/
                        if (bgp4_local_grace_restart(p_peer) != VOS_OK)
                        {
                            /*if still in GR process,should quit*/
                            if (gbgp4.restart_enable)
                            {
                                bgp4_peer_restart_finish(p_peer);
                            }
                             p_peer->notify.code = BGP4_CEASE;
                             p_peer->notify.sub_code = BGP4_ADMINISTRATIVE_RESET;
                            /*common peer reset*/
                            bgp4_fsm_invalid(p_peer);
                        }
                    }
                }
            }
            break;
        default:
            break;
    }
    bgp_sem_give();

    return VOS_OK;
}

int bgp4_route_is_exist(u_int uiVrfId, u_int uiDest, u_int uiMaskLen)
{
    ZEBRA_ROUTE_MSG_T stRouteIndex = {0};
    ZEBRA_ROUTE_MSG_T stRouteInfo = {0};
    int iRet = VOS_ERR;

    uiDest = ntohl(uiDest);
    stRouteIndex.uiVrfId = uiVrfId;
    stRouteIndex.family = AF_INET;
    stRouteIndex.uiIpMaskLen = uiMaskLen;
    memcpy(&stRouteIndex.stDstIp, &uiDest, 4);
    //printf("dest = %x masklen = %d\n",uiDest,uiMaskLen);
    if(zebra_ip_route_get_api(&stRouteIndex, ZEBRA_MATCH_ROUTE, &stRouteInfo) != VOS_OK)
    {
        return VOS_ERR;
    }

    iRet = (stRouteInfo.uiIpMaskLen == uiMaskLen) ? VOS_OK : VOS_ERR;
    
    return iRet;
}

int bgp_relate_vpn_instance_process(u_int uiInstanceId, u_int Mode)
{
	u_int TypeNode = BGP_E;
	int ret = 0;
	L3VPN_INSTANCE_T* pstL3VpnInstance = NULL;
	
	if(uiInstanceId == 0)
	{
		return VOS_ERR;
	}
	pstL3VpnInstance = l3vpn_get_instance_by_id(uiInstanceId);
	if (NULL == pstL3VpnInstance)
	{
             return VOS_ERR;
	}
                    
	ret = l3vpn_set_api(pstL3VpnInstance->ucszL3VpnName, Mode, &TypeNode);
	
	return ret;	
	
}

int bgp_modify_system_route_distance(u_char ucEbgpPre, u_char ucIbgpPre, u_char ucLocalPre)
{
    ZEBRA_ROUTE_MSG_T stRouteIndex;
    ZEBRA_ROUTE_MSG_T stRouteInfoNext; 
    ZEBRA_ROUTE_CONFIG_INFO_T stRouteInfo;
    tBGP4_VPN_INSTANCE *p_instance = NULL; 
    int index = 0;
    u_int uiDistance = 0;

    for(index = 0; index <= MAX_VRF_NUM; index ++)
    {
        p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, index);
        if(p_instance == NULL)
        {
            continue;
        }
        
        memset(&stRouteIndex, 0, sizeof(ZEBRA_ROUTE_MSG_T));
        memset(&stRouteInfoNext, 0, sizeof(ZEBRA_ROUTE_MSG_T));
        memset(&stRouteInfo, 0, sizeof(ZEBRA_ROUTE_CONFIG_INFO_T));
        
        stRouteIndex.family = AF_INET;
        stRouteIndex.uiVrfId = index;

        while(VOS_OK == zebra_ip_route_get_api(&stRouteIndex, ZEBRA_ROUTE_NEXT, (void *)&stRouteInfoNext))
        {
            if(stRouteInfoNext.uiRouteProtocolType == ZEBRA_ROUTE_PROTOCOL_BGP)
            {
                if(CHECK_FLAG(stRouteInfoNext.uiFlag, ZEBRA_FLAG_IBGP))
                {
                    uiDistance = (stRouteInfoNext.uiDistance != ucIbgpPre) ? ucIbgpPre : 0;
                }
                else if(stRouteInfoNext.uiDistance != ucEbgpPre)
                {
                    uiDistance = ucEbgpPre;
                }
                
                if(CHECK_FLAG(stRouteInfoNext.uiFlag, ZEBRA_FLAG_SUMMARY))
                {
                    uiDistance = (stRouteInfoNext.uiDistance != ucLocalPre) ? ucLocalPre : 0;
                }

                if(uiDistance != 0)
                {
                    stRouteInfo.ulVrfId = index;
                    stRouteInfo.uiRouteProtocolType = stRouteInfoNext.uiRouteProtocolType;
                    stRouteInfo.stDstPrefixIpv4.family = AF_INET;
                    stRouteInfo.stDstPrefixIpv4.prefixlen = stRouteInfoNext.uiIpMaskLen;
                    memcpy(&stRouteInfo.stDstPrefixIpv4.prefix, &stRouteInfoNext.stDstIp, 4);
                    memcpy(&stRouteInfo.stNextHopIp, &stRouteInfoNext.stNexthop, 4);
                    stRouteInfo.ulDstIpMaskLen = stRouteInfoNext.uiIpMaskLen;
                    stRouteInfo.ulZebraMessageFlag = stRouteInfoNext.uiFlag;
                    stRouteInfo.ucDistance = uiDistance;
                    stRouteInfo.family = AF_INET;
                    stRouteInfo.ucSafi = SAFI_UNICAST;

                    zebra_ip_route_set_api(&stRouteIndex, ZEBRA_DYNAMIC_ROUTE_ADD, &stRouteInfo); 
                }
            }

            memcpy(&stRouteIndex, &stRouteInfoNext, sizeof(ZEBRA_ROUTE_MSG_T));
            memset(&stRouteInfoNext, 0, sizeof(ZEBRA_ROUTE_MSG_T));
        }
    }
    
    return VOS_OK;
}

int bgp_get_exist_if_ip(u_int uiIndex, u_int uiVrf, u_int *pIpAddr)
{
    int iRet = VOS_ERR;
    u_int uiIfVrf = 0;
    struct prefix_ipv4 stPrefixIp = {0};
    
    if(VOS_OK != zebra_if_get_api(uiIndex, ZEBRA_IF_VRF_ID_GET, &uiIfVrf) || uiVrf != uiIfVrf)
    {
        return VOS_ERR;
    }

    iRet = zebra_if_get_api(uiIndex, ZEBRA_IF_IPADRESS_GET, &stPrefixIp);
    
    memcpy(pIpAddr, &stPrefixIp.prefix, 4);

    return iRet;
}

/*get system router id for special vrf*/
u_int 
bgp_select_router_id(u_int vrid)
{
    u_int uiIfIndex = 0;
    u_int i = 0;
    u_long ulRtv = 0;
	ZEBRA_IF_INDEX_T stIndx;
	u_char ucAdmin = 0;
    u_int uiMaxAddr = 0;
    u_int uiTempAddr = 0;
	struct prefix stPrefx;
	ZEBRA_ROUTE_ID_T stRouteIdIndex = {0};
	u_int uiRouteId = 0;
	u_int iRet = 0;
	struct interface stInterface = {0};
	
	memset(&stPrefx,0,sizeof(struct prefix));

    if(vrid == 0 && zebra_get_api(&stRouteIdIndex,ZEBRA_ROUTE_ID_GET,&stRouteIdIndex) == VOS_OK)
    {
        memcpy(&uiRouteId, &stRouteIdIndex.stRouteId, 4);
        if(uiRouteId != 0)
        {
            uiRouteId = htonl(uiRouteId);
            return uiRouteId;
        }
    }
    
    for (i = 0; i < MAX_LBIF_NUM; i++)
    {
    	if(VOS_ERR == if_loopback_id_to_index(i,&uiIfIndex))
        {
            continue;
        }

        if(VOS_OK == bgp_get_exist_if_ip(uiIfIndex, vrid, &uiTempAddr))
        {
            uiTempAddr = htonl(uiTempAddr);
            if(uiTempAddr > uiMaxAddr)
            {
                uiMaxAddr = uiTempAddr;
            }
        }
    }

    if (uiMaxAddr != 0)
    {
        return uiMaxAddr;
    }

    iRet = zebra_if_get_api(uiIfIndex, ZEBRA_IF_GET_FIRST_IFINDEX, &uiIfIndex);
    while(iRet == VOS_OK)
    {
        zebra_if_get_api(uiIfIndex, ZEBRA_IF_ADMIN_STATE, &ucAdmin);
        if(ucAdmin != IF_SHUTDOWN_OFF || IFINDEX_TYPE_LOOPBACK_IF == IFINDEX_TO_TYPE(uiIfIndex))
        {
            iRet = zebra_if_get_api(uiIfIndex, ZEBRA_IF_GET_NEXT_IFINDEX, &uiIfIndex);
            continue;
        }

        if(VOS_OK == bgp_get_exist_if_ip(uiIfIndex, vrid, &uiTempAddr))
        {
            uiTempAddr = htonl(uiTempAddr);
            if(uiTempAddr > uiMaxAddr)
            {
                uiMaxAddr = uiTempAddr;
            }
        }

        iRet = zebra_if_get_api(uiIfIndex, ZEBRA_IF_GET_NEXT_IFINDEX, &uiIfIndex);
    }
    
    return uiMaxAddr;
}

int bgp_label_handle(u_int uiCmd, void *pValue)
{
    int iRet = VOS_ERR;
    u_int *puiLabel = (u_int *)pValue;

    if(NULL == pValue)
    {
        return iRet;
    }
    
    switch(uiCmd)
    {
        case BGP4_GET_MPLS_LABEL:      
            iRet = mpls_label_get_api(LABEL_DYNAMIC_INGRESS_ALLOC_CMD, DYNAMIC_LABEL, puiLabel);
            bgp4_log(BGP_DEBUG_EVT, "bgp get new label %x %d",*puiLabel,*puiLabel);
            break;
        case BGP4_RELEASE_MPLS_LABLE:
            iRet = mpls_label_set_api(LABEL_DYNAMIC_INGRESS_FREE_CMD, *puiLabel, puiLabel);
            bgp4_log(BGP_DEBUG_EVT, "bgp delete label %x %d ret = %d",*puiLabel,*puiLabel,iRet);
            break;
        default:
            break;
    }

    return iRet;
}

int bgp_module_l3vpn_test()
{
	mpls_l3vpn_t pstNode;
	struct in_addr mask;
	u_int uiAddr1,uiAddr2 = 0;
	pstNode.uiL3vpnId = 1;	
	pstNode.uiVrfId = 0;	

	pstNode.dest.u.prefix4.s_addr = ntohl(inet_addr("1.1.1.0"));//uni 
	pstNode.dest.prefixlen = 24;
	pstNode.nexthop.u.prefix4.s_addr = ntohl(inet_addr("192.168.10.105"));//tunnel destination ip address
	pstNode.uiEgressLabel = 99;
	pstNode.uiVrfId = 1;
	
	if_name_to_index((u_char *)"gigabit-ethernet", (const u_char *)"0/1/7", (u_int *)&pstNode.uiAcIfindex);
	
	//masklen2ip (pstNode.dest.prefixlen,  &mask);
	//pstNode.dest.u.prefix4.s_addr = pstNode.dest.u.prefix4.s_addr & ntohl(mask.s_addr);
	//printf("%s: mask=%x\r\n", __func__, ntohl(mask.s_addr));
/*
	pstNode.uiTunnelifIndex;
	pstNode.uiRouterIndex;
	pstNode.uiOutIfindex;		       //out interface ifindex
	pstNode.usOutVlanId;			//vlan id; =0 no vlan
	pstNode.uiOutPhyport;			//out interface phy port
	pstNode.ucDst_mac[MAC_ADDR_LEN];//destination mac address
	pstNode.enActive;
*/
/*
      case ZEBRA_IF_IPADRESS_GET:

            iRet = VOS_ERR;
            
            pstPrefixIp = (struct prefix_ipv4 *)pValue;
			
*/
	mpls_l3vpn_set_api(1, 0, &pstNode);	

    memcpy(&uiAddr1, &pstNode.dest.u.prefix4.s_addr, 4);
    memcpy(&uiAddr2, &pstNode.nexthop.u.prefix4.s_addr, 4);
    bgp4_log(BGP_DEBUG_EVT,"bgp module test l3vpn  uiAddr1 = %x uiAddr2 = %x\n",uiAddr1,uiAddr2);
}

int bgp4_l3vpn_label_set(tMplsVpnRouteEntry stVpnInfo, u_char ucAction)
{
    mpls_l3vpn_t pstNode;
    u_int uiCmd = 0;
    int iRet = VOS_OK;
    u_int uiAddr1,uiAddr2 = 0;

    memset(&pstNode, 0, sizeof(mpls_l3vpn_t));

    pstNode.uiL3vpnId = stVpnInfo.vrf_id;
    pstNode.uiVrfId = stVpnInfo.vrf_id;

    uiAddr1 = ntohl(bgp_ip4(stVpnInfo.dest));
    memcpy(&pstNode.dest.u.prefix4.s_addr, &uiAddr1, 4);
    pstNode.dest.prefixlen = bgp4_mask2len(stVpnInfo.mask);//zhurish edit
	
    uiAddr2 = ntohl(bgp_ip4(stVpnInfo.next_hop));
    memcpy(&pstNode.nexthop.u.prefix4.s_addr, &uiAddr2, 4);
    //zhurish edit	
    if(stVpnInfo.route_direction == MPLSL3VPN_ROUTE_REMOTE)
    	pstNode.enDirection = MPLS_L3VPN_ME_TO_REMOTE;
    else if(stVpnInfo.route_direction == MPLSL3VPN_ROUTE_LOCAL)
    	pstNode.enDirection = MPLS_L3VPN_REMOTE_TO_ME;	
	
    uiAddr2 = ntohl(bgp_ip4(stVpnInfo.tunnel_source));
    memcpy(&pstNode.tunnel_source.u.prefix4.s_addr, &uiAddr2, 4);
    //end zhurish edit	
    pstNode.uiEgressLabel = stVpnInfo.vc_label;
    pstNode.uiIngressLabel = stVpnInfo.uiInLabel;

    pstNode.uiAcIfindex = stVpnInfo.uiIndex;

    uiCmd = (MPLSL3VPN_ADD_ROUTE == ucAction) ? MPLS_L3VPN_ADD_CMD : MPLS_L3VPN_DEL_CMD;

    iRet = mpls_l3vpn_set_api(uiCmd, 0, &pstNode);  
    
    return iRet;
}

int bgp4_route_in_label_set_to_sys(tBGP4_ROUTE *p_route, tBGP4_PEER *p_peer, u_char ucAction)
{
    int iRet = VOS_OK;
    tMplsVpnRouteEntry stVpnRoute = {0};
    u_int uiCmd = 0;
    L3VPN_INSTANCE_T stVpnInstance = {0};
    u_char pucRd[BGP4_VPN_RD_LEN] = {0};

    memset(&stVpnRoute,0,sizeof(tMplsVpnRouteEntry));

    if(p_route == NULL || p_peer == NULL)
    {
        return VOS_ERR;
    }
    
    stVpnRoute.dest_type = AF_INET;
    stVpnRoute.next_hop_type = AF_INET;
    memcpy(stVpnRoute.dest, p_route->dest.ip + BGP4_VPN_RD_LEN, 4);
    stVpnRoute.mask = bgp4_len2mask(p_route->dest.prefixlen - 64);
    stVpnRoute.route_direction = MPLSL3VPN_ROUTE_REMOTE;
    stVpnRoute.uiInLabel = p_route->in_label;
    memcpy(stVpnRoute.next_hop, p_peer->ip.ip, 4);
    memcpy(stVpnRoute.tunnel_source, p_peer->local_ip.ip,4);
    stVpnRoute.uiIndex =  p_peer->if_unit; 
    
    iRet = l3vpn_get_api(NULL, L3VPN_GET_FIRST, &stVpnInstance);
    while(iRet == VOS_OK)
    {
         bgp_translate_rd_from_l3vpn(stVpnInstance.stVpnFamilyEntry[0].uiVpnAdministrator,
                                     stVpnInstance.stVpnFamilyEntry[0].uiAssignedNumber,
                                     stVpnInstance.stVpnFamilyEntry[0].ucRdType,pucRd);
         bgp_printf_rd_or_rt("RD1", p_route->dest.ip);
         bgp_printf_rd_or_rt("RD2", pucRd);
         if(!memcmp(p_route->dest.ip,pucRd,BGP4_VPN_RD_LEN))
         {
            stVpnRoute.vrf_id = stVpnInstance.uiInstanceId;
            break;
         }

         iRet = l3vpn_get_api(NULL, L3VPN_GET_NEXT, &stVpnInstance);
    }
    
    bgp4_log(BGP_DEBUG_MPLS_VPN,
                    "vpn route vrf %d,dest %#x, nexthop %#x, tunnel source %#x in label %d action %d interface index %x",
                    stVpnRoute.vrf_id,bgp_ip4(stVpnRoute.dest),
                    bgp_ip4(stVpnRoute.next_hop),
                    bgp_ip4(stVpnRoute.tunnel_source),
                    stVpnRoute.uiInLabel,ucAction, stVpnRoute.uiIndex);
    
    iRet = bgp4_l3vpn_label_set(stVpnRoute, ucAction);
    
    return iRet;
}


