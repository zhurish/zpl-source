/* ospf_route.h spf and routing table*/


#if !defined (_OSPF_ROUTE_H_)
#define _OSPF_ROUTE_H_

#ifdef __cplusplus
extern "C" {
#endif 

/*host network and mask*/
#define OSPF_HOST_NETWORK 0xffffffff
#define OSPF_HOST_MASK 0xffffffff

/*ECMP count supported*/
#define OSPF_ECMP_COUNT   32
#define OSPF_TE_TUNNEL_INTERVAL 3 

enum{
    OSPF_NEXTHOP_NORMAL,
    OSPF_NEXTHOP_SHAMLINK,
    OSPF_NEXTHOP_TETUNNEL,
#ifdef OSPF_DCN    
    OSPF_NEXTHOP_UNNUMBER
#endif

};

/*a nexthop node*/
struct ospf_nexthop_info{
    /*output interface unit*/
    u_int if_uint;

    /*nexthop ip address*/
    u_int addr;

    /*shamlink/te tunnel */
    u_int type;
};

/*nexthop struct,containing multiple nexthop address*/
struct ospf_nexthop{
    /*node in global nexthop*/
    struct ospf_lstnode node;
    
    /*TRUE:some route usd this nexthop;FALSE:no route use this nexthop*/
    u_short active;
    
    /*valid nexthop count*/
    u_short count;
    
    /*nexthop address:
      0:invalid nexthop 
      self addr:directly connected route,just have outif,nonexthop
      other:valid nexthop*/
    struct ospf_nexthop_info gateway[OSPF_ECMP_COUNT];       
};

/*path contained in route*/
struct ospf_path{
    /*area pointer which this route belong to*/
    struct ospf_area *p_area;      

    /*nexthop used*/
    struct ospf_nexthop *p_nexthop;

    /*cost*/
    u_int cost;

    /*cost if type2 used*/
    u_int cost2;

    /*this is ospf interface route,*/
    u_short local_route : 1;
    /*path type*/
    u_short type : 15;

    /*tag*/
    u_int tag;
    /*rtmsg send flag for each nexthop,bitset:this nexthop is updated to system*/
    u_int8 msg_send[2];

    u_int adv_id;

    u_int intra_type;
};

/*routing table node*/
struct ospf_route{
    /*node in abr/asbr/network route table*/
    struct ospf_lstnode node;

    /*dest network*/
    u_int dest;

    /*mask,no sense for abr/asbr */
    u_int mask;

    /*dest type:abr,asbr,network...*/
    u_short type;

    /*this is ospf interface route,*/
    /*u_short local_route : 1;*/

    /*path already compared*/
    u_short path_checked : 1;

    /*path different,only valid after checked*/
    u_short path_change : 1;

    /*summary lsa checked*/
    u_short summary_checked : 1;
    
    u_short rsvd : 12;

    /*store current and old paths*/
    struct ospf_path path[2];
};

#define for_each_ospf_import_route(ins, r, n) for_each_node(&(ins)->import_table, r, n)

/*search route with network type, do not consider any path and area*/
#define ospf_network_route_lookup(ins, d,m) ospf_route_lookup(&((ins)->route_table), OSPF_ROUTE_NETWORK, d, m)

/*search abr route,no mask ,path type used*/
#define ospf_abr_lookup(p_area,abr) ospf_route_lookup(&((p_area)->abr_table), OSPF_ROUTE_ABR, abr, 0)

/*macro to fill external route*/
#define ospf_build_external_route(ext,destx,maskx,cost,nhop) do{\
                              memset((ext),0,sizeof(struct ospf_iproute));\
                              (ext)->dest = destx;\
                              (ext)->mask = maskx;\
                              (ext)->metric = cost;\
                              (ext)->fwdaddr = nhop   ;   \
                              }while(0)

/*swap current routing table*/
#define ospf_save_old_routes(ins) do{(ins)->current_route = (ins)->old_route;(ins)->old_route = 1 - (ins)->current_route ;}while(0)

struct ospf_nexthop *ospf_nexthop_add(struct ospf_process *p_process,struct ospf_nexthop *p_nexthop);
struct ospf_route *ospf_route_add(struct ospf_process *p_process,struct ospf_lst *p_list, struct ospf_route *p_route);
struct ospf_route *ospf_asbr_lookup(struct ospf_process * p_process, struct ospf_area * p_area, u_int asbr);
struct ospf_route *ospf_route_lookup(struct ospf_lst * p_table, u_int type, u_int dest, u_int mask);
struct ospf_route *ospf_fwd_route_lookup (struct ospf_process *p_process, u_int fwd_addr, struct ospf_area *p_area);
void ospf_spf_table_flush(struct ospf_area *p_area);
void ospf_spf_node_delete(struct ospf_spf_vertex *p_node);
void ospf_route_calculate_full (struct ospf_process *p_process);
void ospf_spf_request(struct ospf_process * p_process);
void ospf_route_delete(struct  ospf_route * p_route);
void ospf_nexthop_merge(struct ospf_nexthop * p_current, struct ospf_nexthop * p_new);
void ospf_inter_route_calculate(struct ospf_lsa *);
void ospf_external_route_calculate(struct ospf_lsa * p_lsa);
void ospf_route_table_init(struct ospf_lst *p_list);
void ospf_summary_lsa_update(struct ospf_process * p_process, struct ospf_route * p_route);
void ospf_old_route_clear(struct ospf_process *p_process, struct ospf_lst *p_list,struct ospf_route *p_route);
void ospf_update_route_construct(struct ospf_route * p_route, u_int dest, u_int mask, u_int cost, u_int cost2, struct ospf_nexthop * p_nexthop, u_int current);
void ospf_fast_spf_timer_expired(struct ospf_process *process);
STATUS ospf_sys_route_update(struct ospf_process *p_process, struct ospf_route *p_route);
u_int ospf_nexthop_exist(struct ospf_nexthop * p_nhop, u_int gateway);
u_int ospf_nexhop_in_the_area(struct ospf_nexthop * p_nexthop, struct ospf_area * p_area);
u_int ospf_as_external_route_calculate_enable(struct ospf_process *p_process);
int ospf_path_cmp(struct ospf_path *p_path1, struct ospf_path *p_path2);
void ospf_ospf_route_print(struct ospf_route*p_route);
int  ospf_ecmp_nhop_check(struct ospf_nexthop *pstNexthop, struct ospf_nhop_weight *pstNhopw);
int ospf_max_ecmp_check(struct ospf_nexthop *pstNexthop , int iMaxecmp);
void ospf_clear_addr(struct ospf_nexthop_info *gateway);

#ifdef __cplusplus
}
#endif

#endif /* _OSPF_STRUCTURES_H_ */

