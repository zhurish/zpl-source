/* ospf.h */

#if !defined (_OSPF_EXPORT_H_)
#define _OSPF_EXPORT_H_

#ifdef __cplusplus
extern "C" {
#endif 
#define M2_ipRouteProto_ospf				13


#define ospf_flush_filtered_route(ins) ospf_lstwalkup(&((ins)->filtered_route_table), ospf_filtered_route_delete)

struct ospf_policy *ospf_filter_policy_lookup(struct ospf_process *p_process, u_int policy_index);
struct ospf_policy *ospf_filter_policy_add(struct ospf_process *p_process, u_int policy_index);
struct ospf_iproute *ospf_filtered_route_add(struct ospf_process *p_process, struct ospf_iproute *p_route);
void ospf_network_route_change_check(struct ospf_process *p_process);
void ospf_filter_policy_table_init(struct ospf_process * p_process);
void ospf_filter_policy_delete(struct ospf_policy *p_policy);
void ospf_filtered_route_delete(struct ospf_iproute *p_route);
void ospf_policy_update_event_handler(struct ospf_policy *p_changed_policy);
void ospf_policy_delete_event_handler(struct ospf_policy *p_policy);
u_int ospf_sys_route_verify(struct ospf_iproute *p_route);
int ospf_iproute_cmp(struct ospf_iproute * p1, struct ospf_iproute * p2);
int ospf_filter_policy_nm_cmp(struct ospf_policy * p1, struct ospf_policy * p2);
//STATUS ospf_filtered_route_update(struct ospf_route * p_route, struct ospf_policy * p_changed_policy);

#ifdef __cplusplus
}
#endif 
#endif  
