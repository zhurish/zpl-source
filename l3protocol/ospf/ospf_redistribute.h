 
#if !defined (_OSPF_REDISTRIBUTE_H_)
#define _OSPF_REDISTRIBUTE_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "ospf_relation.h"

/*fixed interval for redistribute checking*/
#define OSPF_IMPORT_INTERVAL 2  

/*default distribute info*/
#define OSPF_DISTRIBUTE_DEFAULT_COST    1
#define OSPF_DISTRIBUTE_DEFAULT_TYPE    2
#define OSPF_DISTRIBUTE_DEFAULT_TAG     1

/*redistribute control entry*/
struct ospf_redistribute{
    /*node in process's table*/
    struct ospf_lstnode node;
   
    /*node for nm access*/
    struct ospf_lstnode nm_node;
   
    /*parent pointer of process*/
    struct ospf_process *p_process;
    
    /*dest*/
    u_int dest;
   
    /*route mask*/
    u_int mask;
   
    /*metric for this route*/
    u_int metric;
   
    /*metric type for this route*/
    u_int8 type;
   
    /*action when matched,not-adv,adv*/
    u_int8 action : 1;
   
    /*for nssa use,Translate设置为0，NoTranslate设置为1，0为默认值*/
    u_int8 no_translate : 1;
   
    /*protocol*/
    u_int8 protocol : 5;

    /*引入路由的进程号*/
    u_int proto_process;

    u_int tag;

    /*for import use,未引入设置为0，引入过设置为1，0为默认值*/
    u_int proto_active : 1;
};   

/*redistribute range*/
struct ospf_redis_range{
    /*node in process's table*/
    struct ospf_lstnode node;
   
    /*node for nm access*/
    struct ospf_lstnode nm_node;     
    
    /*parent pointer of process*/
    struct ospf_process *p_process;
   
    /*dest*/
    u_int dest;
   
    /*route mask*/
    u_int mask;
   
    /*protocol id*/
    u_int protocol : 5;
   
    /*flag indicating some routes covered changed*/ 
    u_int update : 1;

    /*Translate设置为0，NoTranslate设置为1，0为默认值*/
    u_int8 no_translate : 1;
};


/*clear all import route*/
#define ospf_flush_import_route(ins) do{\
    struct ospf_iproute *p_iproute = NULL;\
    struct ospf_iproute *p_nextiproute = NULL;\
    for_each_node(&((ins)->import_table), p_iproute, p_nextiproute){\
        ospf_import_route_delete(p_iproute);\
    }\
}while(0)

int ospf_redis_is_enable (struct ospf_process *p_process, int type, int np);
void ospf_redistribute_table_init(struct ospf_process *);
void ospf_redistribute_delete(struct ospf_redistribute * p_redistribute);
void ospf_import_iproute(u_int enable, struct ospf_iproute * p_route);
void ospf_import_route_delete(struct ospf_iproute * p_route);
void ospf_import_route_lsa_originate(struct ospf_iproute * p_route);
void ospf_redistribute_set(struct ospf_process * p_process,int proto,int enable,u_int uiProcessId);
void ospf_import_route_add(struct ospf_process *p_process,struct ospf_iproute *p_notify);
void ospf_import_route_update_all(struct ospf_process * p_process);
void ospf_import_delete_by_nexthop(struct ospf_process * p_process, u_int nexthop);
void ospf_redis_policy_table_init(struct ospf_process *p_process);
void ospf_redis_policy_delete(struct ospf_policy *p_policy);
void ospf_redis_range_table_init(struct ospf_process *p_process);
void ospf_redis_range_up(struct ospf_redis_range *p_range);
void ospf_redis_range_down(struct ospf_redis_range *p_range);
void ospf_redis_range_update(struct ospf_redis_range *p_range);
void ospf_redis_range_delete(struct ospf_redis_range *p_redistribute);
void ospf_redis_range_update_timeout(struct ospf_process *p_process);
struct ospf_policy *ospf_redis_policy_lookup(struct ospf_process * p_process,u_int policy_index,u_int protocol,u_int proto_process);
struct ospf_policy *ospf_redis_policy_add(struct ospf_process * p_process,u_int policy_index,u_int potocol,u_int proto_process);
struct ospf_redis_range *ospf_redis_range_create(struct ospf_process * p_process, u_int protocol, u_int dest, u_int mask);
struct ospf_redis_range *ospf_redis_range_lookup(struct ospf_process * p_process, u_int protocol, u_int dest, u_int mask);
struct ospf_redistribute *ospf_redistribute_lookup(struct ospf_process * p_process,u_int protocol,u_int proto_process);
struct ospf_redistribute *ospf_redistribute_create(struct ospf_process * p_process,u_int protocol,u_int proto_process);
int ospf_redistribute_range_nm_cmp(struct ospf_redis_range *p1, struct ospf_redis_range *p2);
int ospf_redistribute_nm_cmp(struct ospf_redistribute * p1, struct ospf_redistribute * p2);
int ospf_redis_policy_nm_cmp(struct ospf_policy * p1, struct ospf_policy * p2);

int ospf_import_choose_proto2lsa(struct ospf_iproute *pstRoute);
int ospf_only_nssa_area_exist(struct ospf_process *pstProcess);


int ospf_import_route(ospf_import_route_t *pstImportRoute);

//int ospf_import_static_func(L3_IMPORT_ROUTE_INFO_T *pstImportRoute);
//void ospf_import_iproute_fun(ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *pstL3MsgAdd, struct ospf_process *pstProcess, u_char ucProto, u_long ulProcessId, u_long ulFlag);


#ifdef __cplusplus
}
#endif 
#endif  
