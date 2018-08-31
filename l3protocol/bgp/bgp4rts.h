#include "bgp4_api.h"
#include "bgp4main.h"
#include "bgp4path.h"

#ifdef NEW_BGP_WANTED
#ifndef BGP4RTS_H
#define BGP4RTS_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct LinkNode {
    avl_node_t node;
    
    tBGP4_ROUTE *p_route;
}tBGP4_LINK;

#define BGP4_BASE_PREFERENCE 1000

#define bgp4_link_path(pi, pf)   do{(pf)->p_path = (pi); bgp4_avl_add(&(pi)->route_list, (pf));}while(0)

#define is_bgp_route(x) ((x)->proto == M2_ipRouteProto_bgp)

/*add route to special rib table*/
#define bgp4_route_rib_table_add(ins, r) bgp4_route_table_add(&(p_instance)->rib[(r)->dest.afi]->rib_table, (r))

tBGP4_LINK *bgp4_rtlist_add(avl_tree_t *p_list,tBGP4_ROUTE *p_route);
tBGP4_ROUTE *bgp4_route_duplicate(tBGP4_ROUTE *p_route);
int  bgp4_update_output_verify(tBGP4_PEER * p_peer, tBGP4_ROUTE * p_route);
int bgp4_route_system_update(tBGP4_ROUTE  *p_route);
int32_t bgp4_damp_route_lookup_cmp(tBGP4_DAMP_ROUTE *p1, tBGP4_DAMP_ROUTE *p2);
u_int bgp4_route_flood_is_pending(tBGP4_ROUTE *p_route);
u_int bgp4_peer_route_exist(tBGP4_PEER *p_peer);
u_int bgp4_rib_in_new_route_check(tBGP4_ROUTE *p_route);
u_int bgp4_rib_in_withdraw_route_check(tBGP4_ROUTE *p_route);
u_int bgp4_system_update_finished(tBGP4_ROUTE *p_route);
u_int bgp4_damp_route_supressed(tBGP4_ROUTE *p_route, u_int feasible);
u_int bgp4_withdraw_flood_isset(tBGP4_ROUTE *p_route, tBGP4_PEER *p_peer);
u_int bgp4_feasible_flood_isset(tBGP4_ROUTE *p_route, tBGP4_PEER *p_peer);
void bgp4_schedule_init_update(u_int af, tBGP4_PEER *p_peer );
void bgp4_rib_system_update_check(tBGP4_RIB *p_rib);
void bgp4_withdraw_flood_set(tBGP4_ROUTE *p_route, tBGP4_PEER *p_peer);
void bgp4_feasible_flood_set(tBGP4_ROUTE *p_route, tBGP4_PEER *p_peer);
void bgp4_send_update(avl_tree_t* p_plist,avl_tree_t *p_flist, avl_tree_t *p_wlist);
void bgp4_redistribute_route(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_input, u_int action);
void bgp4_peer_route_clear (u_int af, tBGP4_PEER *p_peer);
void bgp4_send_init_update (u_int af, tBGP4_PEER *p_peer );
void bgp4_end_of_rib_output(tBGP4_PEER *p_peer,u_int af) ;
void bgp4_route_flood_clear(tBGP4_ROUTE *p_route);
void bgp4_rib_in_table_update(tBGP4_ROUTE *p_route);
void bgp4_route_withdraw_flood_set(tBGP4_ROUTE *p_route);
void bgp4_route_feasible_flood_set(tBGP4_ROUTE *p_new);
void bgp4_withdraw_flood_clear(tBGP4_ROUTE *p_route, tBGP4_PEER *p_peer);
void bgp4_feasible_flood_clear(tBGP4_ROUTE *p_route, tBGP4_PEER *p_peer);
void bgp4_schedule_system_add(tBGP4_ROUTE *p_route);
void bgp4_schedule_system_delete(tBGP4_ROUTE *p_route);
void bgp4_peer_stale_route_clear(u_int af, tBGP4_PEER *p_peer);
void bgp4_rib_in_check_timeout(tBGP4_RIB *p_rib);
void bgp4_route_release(tBGP4_ROUTE *p_route);
void bgp4_rtlist_clear (avl_tree_t *p_list);
void bgp4_unused_route_clear(tBGP4_RIB *p_rib);
void bgp4_peer_flood_clear(tBGP4_PEER *p_peer);
void bgp4_damp_route_update_timeout(tBGP4_DAMP_ROUTE *p_damp);
void bgp4_damp_route_delete(tBGP4_DAMP_ROUTE *p_damp);

#ifdef __cplusplus
}
#endif    
#endif /* BGP4RTS_H */

#else

#ifndef BGP4RTS_H
#define BGP4RTS_H
#ifdef __cplusplus
      extern "C" {
     #endif
typedef struct LinkNode {
    tBGP4_LISTNODE    node;
    tBGP4_ROUTE *  p_route;
}tBGP4_LINK;

#define  bgp4_link_path(pi, pf)   do{pf->p_path = pi; bgp4_lstadd(&pi->route_list, &pf->node);}while(0)
#define bgp4_link_delete(x) do{\
    if (x != NULL)\
    {\
        bgp4_release_route((x)->p_route);\
        bgp4_free((x), MEM_BGP_LINK);\
    }}while(0)

#define bgp4_rtlist_delete(x, y) do {\
     bgp4_lstdelete((x), &(y)->node);bgp4_link_delete ((y));}while(0)

#define is_bgp_route(x) ((x)->proto == M2_ipRouteProto_bgp)

tBGP4_ROUTE* bgp4_creat_route();
tBGP4_LINK *bgp4_rtlist_lookup(tBGP4_TREE *p_list, tBGP4_ROUTE *p_route);
void bgp4_release_route(tBGP4_ROUTE *p_route);
tBGP4_LINK *bgp4_rtlist_add(tBGP4_TREE *p_list,tBGP4_ROUTE *p_route);
tBGP4_LINK *bgp4_rtlist_add_tail(tBGP4_TREE *p_list,tBGP4_ROUTE *p_route);
void bgp4_rtlist_clear (tBGP4_TREE *p_list);

void bgp4_schedule_init_update(u_int af, tBGP4_PEER *p_peer );
void bgp4_schedule_ip_update(tBGP4_ROUTE *p_route, u_int action);
void bgp4_rib_walkup(u_int wait_finish);
void bgp4_init_next_ip_update_route(tBGP4_VPN_INSTANCE* p_instance);
void bgp4_init_next_rib_update_route(void);
 void bgp4_schedule_rib_update(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_old,tBGP4_ROUTE *p_new,tBGP4_PEER *p_target_peer);
void bgp4_schedule_rib_update_with_range( tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_old,tBGP4_ROUTE *p_new);

void  bgp4_send_update(tBGP4_TREE* p_plist,tBGP4_TREE *p_flist, tBGP4_TREE *p_wlist);
int   bgp4_import_route(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_input, u_int action);
void bgp4_end_peer_route (u_int af, tBGP4_PEER *p_peer,u_char stale_flag,u_char send_flag);
void bgp4_send_init_update (u_int af, tBGP4_PEER *p_peer );
void bgp4_send_update_to_peer(tBGP4_PEER * p_peer, u_int af, tBGP4_TREE * p_flist, tBGP4_TREE * p_wlist);
void bgp4_send_rib_end(tBGP4_PEER *p_peer,u_int af) ;
int bgp4_verify_nlri_afi(tBGP4_TREE *p_list);
int  bgp4_send_check(tBGP4_PEER * p_peer, tBGP4_ROUTE * p_route);
void bgp4_update_rib(tBGP4_PEER * p_peer, tBGP4_TREE * p_flist, tBGP4_TREE * p_wlist);

void bgp4_update_new_route(tBGP4_PEER *p_peer,tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_route);
void bgp4_update_withdraw_route(tBGP4_PEER *p_peer,tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_route);

#ifdef __cplusplus
     }
     #endif    
#endif /* BGP4RTS_H */


#endif
