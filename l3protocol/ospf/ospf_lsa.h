/* ospf_lsa.h --- lsa control */

#if !defined (_OSPF_LSA_H_)
#define _OSPF_LSA_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "ospf.h"

/*generates a checksum for data*/
#define OSPF_MASK_FOR_UNEVEN_BITS 7
#define OSPF_INLINED_SHIFT 3
#define OSPF_FINAL_CHECKSUM_SHIFT 8
#define OSPF_LOG2_OF_NUMBER_OF_ITERATIONS 12
#define OSPF_MOD_MASK (4096 - 1)
#define OSPF_NUMBER_OF_INLINE_ITERATIONS (4096/8)
#define OSPF_MODULUS 255

/*stub router need not forward transit flow,so set a large cost*/
#define OSPF_STUB_ROUTER_DEFAULT_COST 65535

/*max metric value in lsa */
#define OSPF_METRIC_INFINITY  0xFFFFFF 

/*self originated lsa refresh interval*/
#define OSPF_LS_REFRESH_TIME  1800

/*max lsa age*/
#define OSPF_MAX_LSAGE 3600 

/*min lsa aging timer value 1 sec*/
#define OSPF_MIN_LSAGE_TIME 1

/*control lsa's rx rate,min rx interval is 1s->ticks*/
#define OSPF_MIN_LS_RX_INTERVAL OSPF_TICK_PER_SECOND

/*max lsa age difference for lsa compare,if age difference is less than this value, assume same*/
#define OSPF_MAX_LSAGE_DIFFERENCE 900

/*lsa length constant*/
/*standard 20bytes lsa header*/
#define OSPF_LSA_HLEN 20

/*routerlsa fixlen,include lsa head*/
#define OSPF_ROUTER_LSA_HLEN 24 

/*netlsa fixlen,include lsa head*/
#define OSPF_NETWORK_LSA_HLEN 24 

/*sumlsa fixlen,include lsa head*/
#define OSPF_SUMMARY_LSA_HLEN 28 

/*externlsa fixlen,include lsa head*/
#define OSPF_ASE_LSA_HLEN 36 

/*router lsa link length*/
#define OSPF_ROUTER_LINK_LEN 12 

/*extern metric unit in router lsa link*/
#define OSPF_ROUTER_LINK_METRIC_LEN 4 

/*network lsa link length*/
#define OSPF_NETWORK_LINK_LEN 4 

/*e bit in ase lsa's metric*/
#define OSPF_ASE_EBIT 0x80000000 

#define OSPF_LSA_MNG_EN 			1 //·�ɹ���ʹ�ܿ���
#define OSPF_LSA_MNG_DIS 			0 //·�ɹ���ʹ�ܽ�ֹ

/*router link type,according to rfc2328*/
enum{
    OSPF_RTRLINK_PPP = 0x1, 
    OSPF_RTRLINK_TRANSIT = 0x2, 
    OSPF_RTRLINK_STUB = 0x3,  
    OSPF_RTRLINK_VLINK = 0x4 
};

/*special seqnum of lsa*/
enum {
    OSPF_INVALID_LS_SEQNUM = 0x80000000L,  
    OSPF_INIT_LS_SEQNUM  = 0x80000001L,    
    OSPF_MAX_LS_SEQNUM  = 0x7fffffffL, 
};

/*check retransmit*/
#define ospf_nbr_rxmt_isset(x, y) ((x) &&((x)->rxmt_bits)&&(y)&&BIT_LST_TST((x)->rxmt_bits, (y)->index))

/*dd use same struct as retransmit*/
#define ospf_nbr_dd_isset(x, y) ((x) && (y)&&BIT_LST_TST((x)->dd_bits, (y)->index))

#define OSPF_LSVECTOR_LEN 128

/*vector contain lsa,can be used in multiple case*/
#pragma pack(1)

struct ospf_lsvector{
    /*vector count*/
    u_int count;
   
    /*contain lsdb pointer ,upto 128 entries*/
    struct ospf_lsa *p_lsa[OSPF_LSVECTOR_LEN];
}; 

/*struct containing lsa header and body*/
/*normal lsa's head from protocol*/
struct ospf_lshdr{
    /*lsa's age,decrease from 3600 to 0*/ 
    u_short age;

    /*option flag*/
    u_int8 option;

    /*standard type:router,network.....*/
    u_int8 type;

    /*id of this lsa*/
    u_int id;      

    /*advertise router id*/
    u_int adv_id;

    /*seqnuce number*/
    int seqnum;

    /*standard checksum*/
    u_short checksum;

    /*total bytes,including header*/
    u_short len;   
} ;
#pragma pack()

/*flag in router lsa*/
#define ospf_router_flag_translator(x) (((x)&0x10) ? TRUE : FALSE)
#define ospf_router_flag_vlink(x)      (((x)&0x04) ? TRUE : FALSE)
#define ospf_router_flag_asbr(x)       (((x)&0x02) ? TRUE : FALSE)
#define ospf_router_flag_abr(x)        (((x)&0x01) ? TRUE : FALSE)

/*set flag in router lsa*/
#define ospf_set_router_flag_translator(x) do{(x) |= 0x10;}while(0)
#define ospf_set_router_flag_vlink(x) do{(x) |= 0x04;}while(0)
#define ospf_set_router_flag_asbr(x) do{(x) |= 0x02;}while(0)
#define ospf_set_router_flag_abr(x)  do{(x) |= 0x01;}while(0)


#pragma pack(1)

/*one of router link in router lsa,we only support default tos*/
struct ospf_router_link{
    /*link id*/
    u_int id;

    /*link data*/
    u_int data;

    /*link type :ppp,virtuallink,transit ,stub ...*/
    u_int8 type;

    /*tos count, we always set 0*/
    u_int8 tos_count;

    /*cost for default tos*/
    u_short tos0_metric;
};

/*fixed part for router lsa*/
struct ospf_router_lsa{
    /*header*/
    struct ospf_lshdr h;

    /*v,e,b flags*/
    u_int8 flag;

    u_int8 rsvd;
    
    /*link count*/
    u_short link_count;

    /*first router lsa link*/
    struct ospf_router_link link[0];
};

/*fixed part of network lsa*/
struct ospf_network_lsa{
    /*header*/
    struct ospf_lshdr h;

    /*mask of this network*/
    u_int mask;

    /*first attached router id*/
    u_int router[0];
} ;

/*summary link state,just contain one dest information,so it has fixed length*/
struct ospf_summary_lsa{
    /*head*/
    struct ospf_lshdr h;

    /*mask, for asbr summary,this field has no effect*/
    u_int mask;

    /*cost of this route*/
    u_int metric;
};

/*as external lsa,just support one tos*/
struct ospf_external_lsa{
    /*header*/
    struct ospf_lshdr h;

    /*mask of route*/
    u_int mask;

    /*cost of route*/
    u_int metric;

    /*forwarding address */
    u_int fwdaddr;

    /*tag */
    u_int tag;
};

/*nssa lsa,almost same as asexternal,except type*/
struct ospf_nssa_lsa{
    struct ospf_lshdr h;

    u_int mask;

    u_int metric;

    u_int fwdaddr;

    u_int tag;
};

/*opaque lsa type 9, 10, 11 lsa jkw,they are same in fact*/
struct ospf_opaque_lsa{
    struct ospf_lshdr h;

    /*opaque data buffer*/
    u_int8 data[128];
};
#pragma pack()


/*node containing lsa header,used in request table*/
struct ospf_request_node{
    /*node in process's request table*/
    struct ospf_lstnode node;

    /*nbr pointer for this requested lsa*/
    struct ospf_nbr *p_nbr;

    /*lsa table to store this lsa*/
    struct ospf_lstable *p_lstable;
    
    /*requested lsa's header*/
    struct ospf_lshdr ls_hdr;
}; 

/*for routes with same network and different mask,so there be conflict
  network,in this case,ospf will originated different lsa id for these network*/
struct ospf_conflict_network{
    struct ospf_lstnode node;

    /*common network*/
    u_int network;
};

/*max neighbor index currently,may be extended later*/
#define OSPF_MAX_NBR_INDEX /*192*/264

/*bytes used for bitindex of nbr,a byte map to 8 nbrs*/
#define OSPF_RXMT_LEN (OSPF_MAX_NBR_INDEX/8)

/*database node,used for retransmit:lsa and database*/
struct ospf_retransmit{
    /*node in process's retransmit table*/ 
    struct ospf_lstnode node;

    /*pointer back to lsa*/
    struct ospf_lsa *p_lsa;

    /*retransmit neighbor count,0 means has no retransmit neighbor*/
    u_short rxmt_count;

    /*during nbr exchange process,we will build dd summary list for 
      nbr,we may try to establish multiple nbr currently,this field
      indicate how many nbrs this lsa is to be exchanged*/
    u_short dd_count;
    
    /*bit flag containing nbrs in this retransmit node,0 is not used*/
    u_int8 rxmt_bits[OSPF_RXMT_LEN];

    /*indicate detailed nbr index this lsa is to be exchanged during DD exchange*/
    u_int8 dd_bits[OSPF_RXMT_LEN];
}; 

/*database struct*/
struct ospf_lsa{
    /*node in special lsa table*/
    struct ospf_lstnode node;

    /*lsa table it belong to*/
    struct ospf_lstable *p_lstable;

    /*this lsa is self originated,and is
      rxd during self restarting*/
    u_int self_rx_in_restart : 1;

    /*if this is first timeout,this flag is used for route calculation triggered
       by lsa expiration and do flood*/
    u_int expired: 1;

    /*if in maxseq wait state*/
    u_int maxseq_wait : 1;

    /*used in syn,when slave become master*/
    u_int slave_rxmt : 1;
            
    /* for keeping age - stamped when arrived */
    u_int rx_time : 28; 

    /*time when age changed.*/
    u_int update_time;

	u_int ucFlag;/*�Ƿ��½�lsa*/
    
   /*pointer to retransmit node*/
    struct ospf_retransmit *p_rxmt;      
    
   /*contain linkstate buffer*/
    struct ospf_lshdr lshdr[1];

	 
};

typedef struct {
   u_int uiIfIndx;/*�ӿ�����*/
   u_int uiIp;
   u_int uiMask;
}tOSPF_LSA_NET;//·������

typedef struct {
   u_char ucEn;   				//����·��ʹ��
   u_int uiCnt;					//����·������
   tOSPF_LSA_NET stNet[100];//�����·��IP��ַ
}tOSPF_LSA_MANAGE;


#define OSPF_LSA_HDR_LEN_MAX 1024

/*decide if a metric is infinity*/
#define ospf_invalid_metric(x) ( ((x) & OSPF_METRIC_INFINITY) == OSPF_METRIC_INFINITY )

/*decide if a metric is type 2 external */
#define ospf_ase2_metric(x) (((x) & OSPF_ASE_EBIT) == OSPF_ASE_EBIT)

/*scan for router link in router lsa*/
#define ospf_router_link_len(_link) (OSPF_ROUTER_LINK_LEN + ((_link->tos_count) * OSPF_ROUTER_LINK_METRIC_LEN))           

/*first router link*/
#define ospf_first_router_link(p_lsa) ((p_lsa)->link_count ? (p_lsa)->link : NULL)

/*first network link*/
#define ospf_first_network_link(p_lsa) ((ntohs((p_lsa)->h.len) > OSPF_NETWORK_LSA_HLEN) ? (p_lsa)->router : NULL)

/*decide if lsa is self originated,first check advrouter,if not same ,check if network lsa'id is self interface*/
#define ospf_is_self_lsa(p_process, hdr) (ntohl((hdr)->adv_id) == (p_process)->router_id)

/*init a lsa's header*/
#define ospf_init_lshdr(p_hdr, t, i, router) do\
    {\
        (p_hdr)->age = (p_hdr)->len = 0;\
        (p_hdr)->type = t;\
        (p_hdr)->id = htonl(i);\
        (p_hdr)->adv_id = htonl(router);\
    }while(0)

/*macro to scan link list of a router lsa*/
#define for_each_router_link(lsa,link) for(link = ospf_first_router_link(lsa);\
                   link!=NULL;link = ospf_router_link_next(lsa,link))

/*macro to scan link of network lsa*/
#define for_each_network_link(lsa,link) for(link = ospf_first_network_link(lsa);\
                 link!=NULL;link = ospf_network_link_next(lsa,link))

/*lsa scan*/

#define for_each_t11_lsa(ins, a,next_a) for_each_node(&(ins)->t11_lstable.list, a, next_a)

#define for_each_external_lsa(ins, a,next_a) for_each_node(&(ins)->t5_lstable.list, a, next_a)

#define for_each_area_lsa(area,type,a,next_a) for_each_node(&((area)->ls_table[(type)]->list), a, next_a)

#define for_each_router_lsa(area,a,n) for_each_area_lsa(area, OSPF_LS_ROUTER, a, n)

#define for_each_nssa_lsa(area,a,n) for_each_area_lsa(area, OSPF_LS_TYPE_7, a, n)

/*struct translate,from lsa hdr to any other lsa format*/
#define ospf_lsa_body(x) ((void*)(x))

/*clear asexternal lsa for special dest*/
#define ospf_flush_external_lsa(ins, d,m) do\
  {\
     struct ospf_iproute iprt;\
     ospf_build_external_route(&iprt, d, m, OSPF_METRIC_INFINITY, 0);iprt.p_process = ins;\
     ospf_external_lsa_originate(&iprt);\
  }while(0)

/*originate default summary lsa for stub or nssa abr*/
#define ospf_originate_summary_default_lsa(p_area, flush) do\
{\
   struct ospf_summary_lsa summary;\
   struct ospf_route r;\
   u_int current = (p_area)->p_process->current_route;\
   memset(&r, 0, sizeof(r));\
   r.type = OSPF_ROUTE_NETWORK ;\
   r.path[current].cost = (p_area)->stub_default_cost[0].cost ;\
   memset(&summary, 0, sizeof(summary));\
   ospf_summary_lsa_build(p_area, &r, &summary, flush);\
   ospf_local_lsa_install (p_area->ls_table[summary.h.type], &summary.h);\
}while(0)

#define ospf_lsa_rxmt_exist(lsa) (((lsa)->p_rxmt && (lsa)->p_rxmt->rxmt_count) ? TRUE : FALSE)

/*build external metric*/
#define ospf_extmetric(a, b) ((1 == b) ? (a&0x00ffffff) : (0x80000000|(a&0x00ffffff)))

/*adjust lsa's aging when transmit,need increase according to delay*/
#define ospf_adjust_age(a, b) htons(((ntohs(a) + b) > OSPF_MAX_LSAGE ? OSPF_MAX_LSAGE : (ntohs(a) + b ))%(OSPF_MAX_LSAGE+1))

struct ospf_lstable *ospf_lsa_scope_to_lstable(struct ospf_process * p_process, struct ospf_area * p_area, struct ospf_if * p_if, u_int lstype);
struct ospf_lstable *ospf_lstable_lookup(struct ospf_if *p_if,u_int8 type);
struct ospf_lsa *ospf_lsa_install(struct ospf_lstable * p_table, struct ospf_lsa * p_old, struct ospf_lshdr * p_lshdr);
struct ospf_lsa *ospf_lsa_lookup(struct ospf_lstable * p_table, struct ospf_lshdr * p_lshdr);
struct ospf_lsa *ospf_best_lsa_for_translate(u_int dest, u_int mask, struct ospf_lsvector * p_vector);
struct ospf_router_link *ospf_router_link_next(struct ospf_router_lsa * p_lsa, struct ospf_router_link * p_link);
void ospf_lsa_maxage_set(struct ospf_lsa *p_lsa) ;
void ospf_lsa_option_build(struct ospf_area *p_area, struct ospf_lshdr *p_lshdr);
void ospf_local_lsa_install(struct ospf_lstable *p_table, struct ospf_lshdr * p_lshdr);
void ospf_lsa_age_update(struct ospf_lsa * p_lsdb);
//void ospf_summary_lsa_build(struct ospf_area * p_area, struct ospf_route * p_route, struct ospf_summary_lsa * p_summary, u_int need_flush);
void ospf_get_nssa_lsa_with_id(struct ospf_process *p_process, u_int dest, u_int mask, struct ospf_lsvector * p_vector);
//void ospf_originate_range_lsa(struct ospf_range *p_range,u_int need_flush);
void ospf_nssa_lsa_originate(struct ospf_area * p_area, struct ospf_iproute * p_route);
void ospf_router_te_lsa_originate(struct ospf_area * p_area);
void ospf_lsa_delete(struct ospf_lsa * p_lsa);
void ospf_external_lsa_originate(struct ospf_iproute * p_route);
//void ospf_summary_lsa_originate(struct ospf_route * p_route, struct ospf_area * p_area, u_int need_flush);
void ospf_nssa_lsa_translate(struct ospf_lsa * p_lsa);
void ospf_link_te_lsa_originate(struct ospf_if * p_if);
void ospf_lsa_rxmt_clear(struct ospf_lsa * p_lsdb);
void ospf_router_lsa_originate(struct ospf_area * p_area);
void ospf_summary_lsa_originate_for_area(struct ospf_area * p_area, u_int need_flush);
void ospf_lsa_table_timer_expired(struct ospf_process * p_process);
void ospf_network_lsa_originate(struct ospf_if * p_if);
void ospf_nssa_lsa_originate_for_route(struct ospf_iproute * p_route);
void ospf_lsa_lookup_by_id(struct ospf_lst * p_table, u_int type, u_int id, u_int mask,struct ospf_lsvector * p_vector);
void ospf_lsa_refresh( struct ospf_lsa * p_lsdb);
void ospf_lsa_table_init(struct ospf_lstable * p_table, u_int type, struct ospf_process * p_process, struct ospf_area * p_area, struct ospf_if * p_if);
void ospf_lsa_table_shutdown(struct ospf_lstable *p_table);
void ospf_lsa_flush_all(struct ospf_process * p_process);
void ospf_lsa_force_refresh(struct ospf_process *p_process);
void ospf_lsa_dd_clear(struct ospf_lsa * p_lsa);
void ospf_delete_lsa_check_conflict(void * p_scope, struct ospf_lshdr * p_lshdr);
void ospf_lsa_table_flush(struct ospf_lstable *p_table);
u_short ospf_lsa_checksum_calculate(struct ospf_lshdr * p_lsa);
u_int ospf_lsa_flood(  struct ospf_lsa *p_lsdb , struct ospf_if * p_rx_if, struct ospf_nbr * p_rx_nbr);
u_int ospf_lstype_is_valid(struct ospf_if * p_if, u_int8 lstype);
u_int ospf_lsa_changed(struct ospf_lshdr * p_lshdr, struct ospf_lshdr * p_lshdr2);
u_int ospf_lshdr_is_valid(struct ospf_lshdr *p_lshdr);
int ospf_lshdr_cmp(struct ospf_lshdr * p_new, struct ospf_lshdr * p_old);
int ospf_conflict_network_cmp(struct ospf_conflict_network * p_network1, struct ospf_conflict_network * p_network2);
u_int ospf_add_lsa_check_conflict(void * p_scope, struct ospf_lshdr * p_lshdr);
int ospf_area_lstable_nm_cmp(struct ospf_lstable *p1, struct ospf_lstable *p2);
int ospf_as_lstable_nm_cmp(struct ospf_lstable *p1, struct ospf_lstable *p2);
int ospf_if_lstable_nm_cmp(struct ospf_lstable *p1, struct ospf_lstable *p2);
int ospf_vif_lstable_nm_cmp(struct ospf_lstable * p1, struct ospf_lstable * p2);
u_int *ospf_network_link_next(struct ospf_network_lsa * p_lsa, u_int * p_link);
void ospf_lsa_check_timer_expired(struct ospf_process *p_process);
void ospf_vpn_summary_lsa_originate_all(struct ospf_process * p_process, struct ospf_iproute * p_route, u_int need_flush);


#ifdef __cplusplus
}
#endif

#endif 

