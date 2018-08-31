#ifndef _OSPF_TE_INTERFACE_H
#define _OSPF_TE_INTERFACE_H
#ifdef __cplusplus
   extern "C" {
 #endif 
#include "ospf_table.h"

/*TE top tlv type */
enum{
    OSPF_TE_TLV_RTR_ADDR = 1,
    OSPF_TE_TLV_LINK
};

/*TE sub tlv type*/
enum{
    OSPF_TE_SUB_LINK_TYPE = 1,
    OSPF_TE_SUB_LINK_ID,
    OSPF_TE_SUB_LOCAL_ADDRESS,
    OSPF_TE_SUB_REMOTE_ADDRESS,
    OSPF_TE_SUB_METRIC,
    OSPF_TE_SUB_MAX_BANDWIDTH,
    OSPF_TE_SUB_MAX_RSVD_BANDWIDTH,
    OSPF_TE_SUB_UNRSVD_BANDWIDTH,
    OSPF_TE_SUB_RESOURCE_CLASS_COLOR 
};

#pragma pack(1)

/*TE tlv header*/
struct ospf_te_tlv{
    u_short type;

    u_short len;
};

/* Router Address TLV */
struct ospf_te_routeraddr_tlv{
    struct ospf_te_tlv hdr;

    u_int value;
};

/* Link TLV */
struct ospf_te_link_tlv{
    struct ospf_te_tlv hdr;
}_pack;

/* Link Type */
struct ospf_te_linktype_tlv{
    struct ospf_te_tlv hdr;

    u_int8 type;

    /*3bytes padding,do not include in length*/
    u_int8 padding[3];
};

/* Link ID */
struct ospf_te_linkid_tlv{
    struct ospf_te_tlv hdr;

    u_int value;
};

/* Local Interface IP Address */
struct ospf_te_localaddr_tlv{
    struct ospf_te_tlv hdr;

    /*we may insert multiple ip address*/
    u_int value[1];
};

/* Remote Interface IP Address */
struct ospf_te_remoteaddr_tlv{
    struct ospf_te_tlv hdr;

    /*we may insert multiple ip address*/
    u_int value[1];
};

/* Traffic Engineering Metric */ 
struct ospf_te_metric_tlv{
    struct ospf_te_tlv hdr;

    u_int value;
};

/* Maximum Bandwidth */ 
struct ospf_te_maxbd_tlv{
    struct ospf_te_tlv hdr;

    u_int value;
};

/* Maximum Reservable Bandwidth */ 
struct ospf_te_maxrsvddb_tlv{
    struct ospf_te_tlv hdr;

    u_int value;
};

/* Unreserved Bandwidth */
struct ospf_te_unrsvddb_tlv{
    struct ospf_te_tlv hdr;

    u_int value[8];
};

/* Resource Class/Color */ 
struct ospf_te_color_tlv{
    struct ospf_te_tlv hdr;

    u_int value;
};
#pragma pack()

#define MAX_ADDR_NUM 4
#define TE_MAX_PATH_HOP 16
 
struct ospf_te_param{
    /*1---ppp   2---multi*/
    u_int8 link_type;    

    /*for ppp,link_id= nbr routerId;  for multi, link_id= DR addr*/
    u_int link_id;       
    
    u_int local_addr[MAX_ADDR_NUM];

    u_int remoter_addr[MAX_ADDR_NUM];

    u_int cost;

    u_int max_band;

    u_int max_reserve_band;

    u_int unreserve_bind[8];
    
    u_int group;
	
    u_int lsa_id;
};

struct ospf_terouter{
    struct ospf_lstnode node;  
   
    /*protocal*/
    u_int proto;
    
    u_int area_id;
    
    u_int router_id;
    
    u_int addr;
    
    struct ospf_lst link_list;  
};

struct ospf_telink{
    struct ospf_lstnode node;
    
    /*the router which te_link belonged  to */
    struct ospf_terouter *p_router;   

    /*the another point of te_link ; the another point of remote_link must fill te_link*/
    struct ospf_telink *p_remote_link;
    
    struct ospf_te_param te_param;
};

void ospf_translate_type10_to_tedb(struct ospf_area * p_area);
void ospf_type10_lsa_to_tedb(struct ospf_area * p_area,struct ospf_lsa *p_lsa);

#ifdef __cplusplus
}
#endif

#endif
