/* ospf_frr.h */

#if !defined (_OSPF_FRR_H_)
#define _OSPF_FRR_H_

#ifdef __cplusplus
extern "C" {
#endif 

/*default frr calculate interval 5seconds*/
#define OSPF_FRR_INTERVAL 5
/*backup spf node struct*/
struct ospf_backup_spf_vertex{
     /*node in area's backup spf table*/
     struct ospf_lstnode node;

     /*dest node id*/
     u_int id;

     /*dest node type:abr,asbr,network*/
     u_int type;

     /*cost:from nexthop to dest*/
     u_int cost;

     /*cost:from nexthop to root*/
     u_int cost_root;     

     /*cost:from backup nexthop to main nexthop*/
     u_int cost_nexthop; 

     /*backup cost:from root to nexthop + nexthop to dest*/
     u_int cost_total;
     
     /*backup nexthop,no ecmp wanted,so just has one*/
     u_int ifunit;

     /*nexthop address*/
     u_int nexthop;

     /*main nexthop address*/
     u_int main_nhop;
};

/*backup path used in backup route*/
struct ospf_backup_path{
     u_int cost;
     
     /*nexthop's interface unit,we only consider one backup nexthop*/
     u_int ifunit;

     /*nexthop's ip address,we only consider one backup nexthop*/
     u_int nexthop;    
};
/*backup route */
struct ospf_backup_route{
     /*node in backup route table*/
     struct ospf_lstnode node;

     /*dest type:abr,asbr,network...*/
     u_int type;

     /*dest network*/     
     u_int dest;

     /*dest mask,not used for abr or asbr route*/ 
     u_int mask;
     
     /*old and new path for this route*/
     struct ospf_backup_path path[2];

     /*main nexthop address*/
     u_int nexthop;
};

void ospf_frr_timer_expired(struct ospf_process *p_process);
void ospf_backup_route_table_flush(struct ospf_lst * p_route_table);
void ospf_backup_spf_table_flush(struct ospf_area *p_area);
void ospf_backup_route_export(struct ospf_process *p_process);
int ospf_backup_spf_vertex_cmp(struct ospf_backup_spf_vertex * p1, struct ospf_backup_spf_vertex * p2);
int ospf_backup_route_cmp(struct ospf_backup_route * p1, struct ospf_backup_route * p2);
struct ospf_spf_vertex *ospf_spf_lookup(struct ospf_lst *p_table, u_int id, u_int type);

void ospf_backup_spf_examine_network_vertex(struct ospf_lst * p_spf_list, struct ospf_lst * p_candidate_list, struct ospf_spf_vertex * p_vertex);
void ospf_area_backup_spf_calculate(struct ospf_area *p_area);
void ospf_backup_route_calculate(struct ospf_process *p_process, struct ospf_route *p_route);
void ospf_backup_spf_calculate(struct ospf_area * p_area, struct ospf_spf_vertex * p_dest);
void ospf_backup_spf_examine_router_vertex(struct ospf_lst * p_spf_list, struct ospf_lst * p_candidate_list, struct ospf_spf_vertex * p_vertex);
void ospf_backup_spf_candidate_node_update(struct ospf_lst *p_spf_list, struct ospf_lst *p_candidate_list, struct ospf_spf_vertex *p_parent, u_int id, u_int type, u_int link_cost,u_int link_data);
void ospf_spf_parent_set(struct ospf_spf_vertex *p_parent, u_int ifaddr, struct ospf_spf_vertex *p_node);
void ospf_backup_spf_vertex_create(struct ospf_area *p_area, struct ospf_backup_spf_vertex *p_backup);
void ospf_backup_inter_route_calculate(struct ospf_process * p_process, struct ospf_route * p_route);
void *ospf_lsa_link_lookup (struct ospf_lsa *p_lsa, struct ospf_spf_vertex *p_parent);
u_int ospf_part_spf_calculate(struct ospf_area *p_area, struct ospf_spf_vertex *p_src, struct ospf_spf_vertex *p_dest);
int ospf_spf_lookup_cmp(struct ospf_spf_vertex *p1, struct ospf_spf_vertex *p2 );
int ospf_spf_cost_lookup_cmp(struct ospf_spf_vertex * p1, struct ospf_spf_vertex * p2);
u_int8* ospf_printf_area(u_int8 * string, u_int id);
#ifdef OSPF_FRR
int ospf_backup_route_switch(struct ospf_nbr* p_nbr);
#endif

#ifdef __cplusplus
}
#endif

#endif  

