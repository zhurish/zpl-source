#ifndef BGP_RELATION_H
#define BGP_RELATION_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "zebra_api.h"

enum 
{
    BGP4_GET_MPLS_LABEL = 0,
    BGP4_RELEASE_MPLS_LABLE = 1,
};

int errnoGet();
u_char* inet_ntoa_1(u_char *ip_str, u_int ipaddress);
int uspGetIfUnitByIp(int vrId, int family, u_char *addr, u_int *ifIndex);
//int inet_maskLen(unsigned int netmask);
int bgp_redistribute_route_change_process(ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *pstRouteAddMsg);
int bgp_if_link_change_process(ZEBRA_IF_MSG_REDISTRIBUTE_T *pstIfLinkMsg);
int bgp4_route_is_exist(u_int uiVrfId, u_int uiDest, u_int uiMaskLen);
int bgp_relate_vpn_instance_process(u_int uiInstanceId, u_int Mode);
u_int bgp_select_router_id(u_int vrid);
int bgp_label_handle(u_int uiCmd, void *pValue);

#ifdef __cplusplus
     } 
#endif

#endif

