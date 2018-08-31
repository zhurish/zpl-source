 

#if !defined (_OSPF_SPF_H_)
#define _OSPF_SPF_H_

#ifdef __cplusplus
extern "C" {
#endif 
/*max spf calculating interval in ticks*/
#define OSPF_MAX_SPF_INTERVAL (30*OSPF_TICK_PER_SECOND)

/*fixed interval for routing table calculation in ticks*/
#define OSPF_SPF_INTERVAL 2


/*check fast spf interval,10ticks==1s*/
#define OSPF_FAST_SPF_INTERVAL 10

/*max count of fast spf in an interval*/
#define OSPF_FAST_SPF_LIMIT 5

/*spf node's parent,including parent node and link from parent to self*/
struct ospf_spf_parent {
    /*parent spf node*/
    struct ospf_spf_vertex *p_node;

    /*linkid connect parent and self,only valid for root connected node*/
    u_int link;
};

/*ECMP consider for spf parent*/
#define OSPF_MAX_SPF_PARENT 6

/*SPF node constructed during spf calculating*/
struct ospf_spf_vertex{
    /*node in spf tree*/
    struct ospf_lstnode node;

    /*node in cost table,ie.candidate table*/
    struct ospf_lstnode cost_node;

    /*area pointer*/ 
    struct ospf_area *p_area;

    /*related lsa entry*/
    struct ospf_lsa *p_lsa;

    /*parent pointer,support ecmp*/
    struct ospf_spf_parent parent[OSPF_MAX_SPF_PARENT];

    /*list of nexthop*/
    struct ospf_nexthop *p_nexthop;

    /*spf node type,also be lsa type */
    u_int type;

    /*lsid of lsa*/
    u_int id;

    /*cost of node*/
    u_int cost;
};

void ospf_spf_calculate(struct ospf_area *p_area);
#ifdef __cplusplus
}
#endif 
#endif  
