
#ifdef NEW_BGP_WANTED

#ifndef BGP4AGGR_H
#define BGP4AGGR_H

#ifdef __cplusplus
extern "C" {
#endif
#include "plateform.h"
#define BGP4_MAX_AS_COUNT 510 

typedef struct {
  u_int count ;
  u_short as[BGP4_MAX_AS_COUNT];
}tBGP4_ASVECTOR;

typedef struct BGP4_AGGR{
	avl_node_t node;
	              
	/*dest prefix*/
	tBGP4_ADDR dest;

    /*instance it belong to*/
    struct s_bgpInstanceInfo *p_instance;
    
    /*route for this range in rib*/
	tBGP4_ROUTE *p_route;
       
	/*aggreagte status.row status*/
	u_short row_status;
	
	/*aggreagte summaryonly or all.TRUE-only send summary route*/
	u_char summaryonly;

    /*some aggegrated route changed,we need update summary 
     route*/
    u_char update_need;

    /*matched route*/
    u_int matched_route;
}tBGP4_RANGE;

#define for_each_bgp_range(p_list,p_aggr) bgp4_avl_for_each(p_list, p_aggr)

tBGP4_RANGE *bgp4_range_match(tBGP4_VPN_INSTANCE*p_instance, tBGP4_ROUTE *p_route);
tBGP4_RANGE *bgp4_range_create(tBGP4_VPN_INSTANCE*p_instance,tBGP4_ADDR* p_aggr_rt);
tBGP4_RANGE *bgp4_range_lookup(tBGP4_VPN_INSTANCE*p_instance,tBGP4_ADDR * aggr_ip);
int bgp4_range_lookup_cmp(tBGP4_RANGE *p1, tBGP4_RANGE *p2);
void bgp4_path_merge(tBGP4_ROUTE *p_aggr_route, tBGP4_ROUTE *p_route);
void bgp4_range_delete(tBGP4_RANGE* p_range);
void bgp4_range_up(tBGP4_RANGE *p_range);
void bgp4_range_down(tBGP4_RANGE *p_range);
void bgp4_range_table_flush(tBGP4_VPN_INSTANCE* p_instance) ;
void bgp4_range_update(tBGP4_RANGE *p_range);
STATUS bgp4_feasible_flood_check_against_range(tBGP4_ROUTE *p_route);
STATUS bgp4_withdraw_flood_check_against_range(tBGP4_ROUTE *p_route);

#ifdef __cplusplus
}
#endif 
#endif

#else

#ifndef BGP4AGGR_H
#define BGP4AGGR_H

#ifdef __cplusplus
      extern "C" {
     #endif
#define BGP4_MAX_AS_COUNT 510 

typedef struct {
  u_int count ;
  u_short as[BGP4_MAX_AS_COUNT];
}tBGP4_ASVECTOR;

typedef struct BGP4_AGGR{
    tBGP4_LISTNODE node;
                  
    /*dest prefix*/
    tBGP4_ADDR dest;

    tBGP4_ROUTE* p_aggr_rt;
       
    /*aggreagte status*/
    u_int state : 4;
    
    /*aggreagte summaryonly or all*/
    u_int summaryonly : 1;

    u_int rsvd : 27;

}tBGP4_AGGR;

#define for_each_bgp_range(p_list,p_aggr) LST_LOOP(p_list, p_aggr, node, tBGP4_AGGR)

int  bgp4_rtlist_aggregate_enable(tBGP4_LIST *p_list) ;
void bgp4_aggregate_path(tBGP4_ROUTE *p_aggr_route, tBGP4_ROUTE *p_route);
tBGP4_AGGR *bgp4_aggr_match(tBGP4_VPN_INSTANCE*p_instance,u_int af, tBGP4_ROUTE *p_route);
tBGP4_AGGR *bgp4_add_aggr(tBGP4_LIST* p_aggr_lst,tBGP4_ADDR* p_aggr_rt);
tBGP4_AGGR* bgp4_aggregate_lookup(tBGP4_LIST* p_aggr_lst,tBGP4_ADDR * aggr_ip);
void bgp4_delete_aggr(tBGP4_LIST* p_aggr_lst,tBGP4_AGGR* p_range);
int bgp4_aggr_up(tBGP4_VPN_INSTANCE* p_instance,tBGP4_AGGR *p_range);
int bgp4_aggr_down(tBGP4_VPN_INSTANCE* p_instance,tBGP4_AGGR *p_range);
void bgp4_delete_all_aggregate(tBGP4_VPN_INSTANCE* p_instance) ;
#ifdef __cplusplus
     }
     #endif 
#endif

#endif
