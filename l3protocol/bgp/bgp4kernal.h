#include "plateform.h"
#include "bgp4_api.h"
#ifdef NEW_BGP_WANTED
#ifndef  KERRNAL_H
#define KERRNAL_H
#ifdef __cplusplus
 extern "C" {
#endif

#define IPPROTO_BGP4 179



#define bgp_sem_take() vos_pthread_lock(&gbgp4.sem)
#define bgp_sem_take_timeout() ((0 == gbgp4.sem) ? VOS_OK : (vos_pthread_timedlock(&gbgp4.sem, 5*vos_get_clock_rate())))
#define bgp_sem_give() vos_pthread_unlock(&gbgp4.sem)

#define bgp4_get_max_ifipaddr() findMaxIfIpAddr()

STATUS bgp4_sys_msg_send(u_char*msg_buf);
STATUS bgp4_sys_msg_add(u_int vrf,u_char*msg_buf ,tBGP4_ROUTE *p_route, u_int hop_id);
STATUS bgp4_sys_ip6_msg_insert(u_int vrf,u_char*msg_buf ,tBGP4_ROUTE *p_route, u_int hop_id);
STATUS bgp4_sys_iproute_add(u_int vrf, tBGP4_ROUTE *p_route, u_int hop_id, u_int *p_hwneed);
STATUS bgp4_sys_ip6route_add(u_int vrf, tBGP4_ROUTE *p_route, u_int hop_id, u_int *p_hwneed);
STATUS bgp4_sys_ip6route_delete(u_int vrf, tBGP4_ROUTE *p_route, u_int hop_id,u_int *p_hwneed);
STATUS bgp4_sys_iproute_delete(u_int vrf, tBGP4_ROUTE *p_route, u_int hop_id,u_int *p_hwneed);
STATUS bgp4_sys_iproute_lookup(u_int vrf, tBGP4_ROUTE *p_route);
u_int bgp4_sys_ifunit_to_prefixlen(u_int if_unit,u_int addr_type);
int bgp4_sysroute_lookup(u_int vrf,tBGP4_ROUTE *p_route);
int bgp4_nexthop_metric_cmp(tBGP4_PATH* p_path1,tBGP4_PATH* p_path2);
int bgp4_nexthop_is_reachable(u_int instance_id,tBGP4_ADDR* p_nexthop);
int bgp4_direct_nexthop_calculate(tBGP4_PATH* p_path);
void bgp4_bind_bfd(tBGP4_PEER *p_peer);
void bgp4_unbind_bfd(tBGP4_PEER *p_peer);
void bgp4_nexthop_check_timout(tBGP4_RIB *p_rib);
void bgp4_rtsock_init(void);
void bgp4_rtsock_close(void);
void bgp4_set_router_id(u_int id);
void bgp4_rtsock_recv(void);
void bgp4_redistribute_set(tBGP4_VPN_INSTANCE* p_instance,u_char af,u_char proto, u_char action,u_int processId);
void bgp4_ip6_linklocal_nexthop_get(u_char* p_gaddr6,u_char* p_laddr6);
void bgp4_up_notify(void);
void bgp4_workmode_update_timeout(void);
void bgp4_rtsock_vpn_del_msg_input(u_char *p_buf);



extern STATUS ipv4v6RouteLookup(u_int routeInstance,int addrFamily,char *dest, int mask,void *pRouteInfo);
extern STATUS ip_route_match(u_int routeInstance,u_int dest, void *p_info);
extern STATUS ip6_route_match(u_int routeInstance,char *dest, M2_IPV6ROUTETBL *p_info);
extern int32_t uspIpRouterId(u_int vrId, u_int family, u_char *addr);
extern int32_t uspSelfSysIndex();
extern STATUS rtSockOptionSet(int sockfd,int protoType,int opType);
extern int findMaxIfIpAddr();
extern STATUS systemIPv4RouteAdd2(int,int,char ,int,int,int,int ,int);
extern STATUS systemIPv4RouteDelete2(int,int,char,int,int,int,int,int);
extern STATUS systemIPv6RouteAdd2(int s, int vrid, int proto, char * dest, char * nhop, int len, int flag, int cost);
extern STATUS systemIPv6RouteDelete2(int s, int vrid, int proto, char * dest, char * nhop, int len, int flag, int cost);
extern int ipRouteRedisSet(int routeVrid, int addrFamily,int fromProtoType,int toProtoType,u_int processId);
extern int ifIndexToIfunit(int ifIndex,int  *ifUnit);

void bgp4_rtsock_route_change(u_int vrf_id,tBGP4_ROUTE *p_route, u_int add);

#ifdef __cplusplus
}
#endif   

#endif

#else

#ifndef  KERRNAL_H
#define KERRNAL_H
#ifdef __cplusplus
      extern "C" {
     #endif
     
enum {
    BGP4_OS_IFNET_INDEX = 1,
    BGP4_OS_IFNET_FLAG,
    BGP4_OS_IFNET_MASK,
    BGP4_OS_IFNET_PHYSPEED 
};


void bgp4_get_max_ifipaddr(u_int *pu4_max_ip);
void bgp4_set_router_id(u_int id);
STATUS bgp4_rtsock_recv();
u_int bgp4_get_ifmask(u_int if_unit,u_int addr_type);
void bgp4_redistribute_ip_routes(tBGP4_VPN_INSTANCE* p_instance,u_char af,u_char proto, u_char action);
#ifdef  BGP_IPV6_WANTED
int bgp4_ipv6_get_local_addr(u_char* p_gaddr6,u_char* p_laddr6);
#endif
STATUS bgp4_send_routemsg(u_char*msg_buf);
STATUS bgp4_add_routemsg(u_int vrf_id,u_char*msg_buf ,tBGP4_ROUTE *p_route,tBGP4_ADDR* old_direct_nexthop,u_int old_if_unit);
STATUS bgp4_add_route6msg(u_int vrf_id,u_char*msg_buf ,tBGP4_ROUTE *p_route,tBGP4_ADDR* old_direct_nexthop,u_int old_if_unit);
int bgp4_add_sysroute(u_int vrf_id,tBGP4_ROUTE *p_route);
int bgp4_del_sysroute(u_int vrf_id,tBGP4_ROUTE *p_route);
int bgp4_sysroute_lookup(u_int vrf_id,tBGP4_ROUTE *p_route);
int bgp4_nexthop_metric_comp(tBGP4_PATH* p_path1,tBGP4_PATH* p_path2);
int bgp4_nexthop_is_reachable(u_int instance_id,tBGP4_ADDR* p_nexthop);
int bgp4_get_direct_nexthop(tBGP4_ROUTE* p_route,tBGP4_PATH* p_path);
void  bgp4_bind_bfd(tBGP4_PEER *p_peer);
void  bgp4_unbind_bfd(tBGP4_PEER *p_peer);
#ifdef __cplusplus
     }
     #endif   
#endif


#endif
