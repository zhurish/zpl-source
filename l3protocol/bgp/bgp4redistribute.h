
#include "bgp4_api.h"
#include "bgp4main.h"
#include "bgp4path.h"

#ifdef NEW_BGP_WANTED
#ifndef BGP4_REDISTRIBUTE_H
#define BGP4_REDISTRIBUTE_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct bgp4_network
{
	avl_node_t node;

    tBGP4_VPN_INSTANCE *p_instance;
        
	tBGP4_ADDR net;
    
	u_int status;
}tBGP4_NETWORK;

tBGP4_NETWORK *bgp4_network_lookup(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR* p_net);
tBGP4_NETWORK *bgp4_network_add(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR* p_net);
int bgp4_redistribute_is_permitted(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_input);
int bgp4_network_lookup_cmp(tBGP4_NETWORK *p1, tBGP4_NETWORK *p2);
void bgp4_network_down(tBGP4_NETWORK * p_network);
void bgp4_network_up(tBGP4_NETWORK * p_network);
void bgp4_network_delete(tBGP4_NETWORK * p_network);
void bgp4_network_delete_all(tBGP4_VPN_INSTANCE* p_instance);

#ifdef __cplusplus
}
#endif  
#endif

#else

#ifndef BGP4_REDISTRIBUTE_H
#define BGP4_REDISTRIBUTE_H
#ifdef __cplusplus
      extern "C" {
     #endif
typedef struct bgp4_network
{
    tBGP4_LISTNODE node;
    tBGP4_ADDR          net;
    u_int status;
}tBGP4_NETWORK;

int   bgp4_redistribute_check_route(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_input);
int bgp4_init_rtsock() ;
int bgp4_close_rtsock();
void bgp4_delete_all_network(tBGP4_VPN_INSTANCE* p_instance);
tBGP4_NETWORK *bgp4_network_lookup(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR* p_net);
INT1 bgp4_network_up(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR* p_net);
tBGP4_NETWORK *bgp4_add_network(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR* p_net);
INT1 bgp4_network_down(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR* p_net);
void bgp4_delete_network(tBGP4_VPN_INSTANCE* p_instance,tBGP4_NETWORK * p_network);
void bgp4_rtsock_route_change(u_int vrf_id,tBGP4_ROUTE *p_route, u_int add);
#ifdef __cplusplus
     }
     #endif  
#endif

#endif
