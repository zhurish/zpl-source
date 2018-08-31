#include "bgp4_api.h"
#include "bgp4com.h"
#ifdef NEW_BGP_WANTED
#ifndef BGP4PATH_H
#define BGP4PATH_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct ASPath {
    avl_node_t node;

    /*AS-SET,SEQENCE....*/
    u_short type;

    /*AS count*/
    u_short count;
    
    /*as buffer,length is decided by length field*/
    u_char as[0];
}tBGP4_ASPATH; 

#define BGP4_MAX_CLUSTERID 4
#define BGP4_MAX_COMMNUITY 4
#define BGP4_MAX_ECOMMNUITY 32   /*目前一个实例最多配置32*/

/************community attribute*****************/
enum {
   COM_ACTION_ADD = 1,
   COM_ACTION_REPLACE ,
   COM_ACTION_CLEAR 
};

/*description for attribute*/
typedef struct {
   /*flag for the attribute,same as attribute hdr*/
   u_char flag;
   
   /*unit length of this attribute,0 means no unit length*/
   u_char unitlen;

   /*is attribute is zero length,1-means has zero length*/
   u_char zerolen;

   /*fixed length of attribute,0 has no meaning*/
   u_char fixlen;
}tBGP4_ATTR_DESC;

typedef struct {
    u_char val[8];
}tBGP4_ECOMMUNITY;

struct bgp4_aggregator{
    u_int as;
    
    tBGP4_ADDR ip;
};

/*route's directly nexthop,include multiple nexthop address*/
#define BGP4_MAX_ECMP 8

typedef struct bgp4_nexthop_block{
    /*nexthop count in block*/
    u_int count;

    /*nexthop's ifunit array*/
    int ifunit[BGP4_MAX_ECMP];

    /*nexthop's address*/
    tBGP4_ADDR ip[BGP4_MAX_ECMP];
}tBGP4_NEXTHOP_BLOCK;

typedef struct bgp4_path{
    avl_node_t node;

    /*address family flag*/
    u_int af;

    /*path's vrf*/
    tBGP4_VPN_INSTANCE *p_instance;

    /*if path is imported from one vrf into other vrf,
      this field is set to original instance*/
    u_int origin_vrf;

    /*peer of this path*/
    tBGP4_PEER *p_peer; 

    /*all as path segment, 2bytes as*/
    avl_tree_t aspath_list;    

    /*all as path segment, 4bytes as*/
    avl_tree_t as4path_list;    
 
    /*all routes use this path*/
    avl_tree_t route_list;

    /*dynamic allocated extend community*/
    u_char *p_ecommunity;

    /*dynamic allocated cluster id*/
    u_char *p_cluster;

    /*dynamic allocated community*/
    u_char *p_community;

    u_char *p_unkown;

    /*dynamic allocated aggregator*/
    struct bgp4_aggregator *p_aggregator;

    struct bgp4_aggregator *p_new_aggregator;
        
    u_short cluster_len;
    
    u_short excommunity_len;

    u_short community_len;
    
    u_short origin;   
    
    u_int med;
    
    u_int localpref;
            
    u_int origin_id;
    
    u_int unknown_len;
    
    tBGP4_ADDR nexthop;

    /*link local nexthop for ipv6*/
    tBGP4_ADDR linklocal_nexthop;

    /*nexthop used by system update*/
    tBGP4_NEXTHOP_BLOCK *p_direct_nexthop;

    /*old nexthop used by system update,checked when nexthop changed*/
    tBGP4_NEXTHOP_BLOCK *p_old_nexthop;

    u_int med_exist : 1;

    /*have localpref*/
    u_int local_pref_exist : 1;

    /*rxd community not to external*/
    u_int community_no_export : 1;

    /*rxd community not adv*/
    u_int community_no_adv : 1; 

    /*rxd community not adv*/
    u_int community_no_subconfed : 1; 

    u_int atomic_exist : 1;

    /*this path is updated by policy*/
    u_int policy_updated : 1;
    u_int src_instance_id;
    u_int afi;
    u_int safi;
    u_int route_direction;
    u_int flags;
 }tBGP4_PATH; 

/*struct for route flooding control*/
typedef struct bgp4_ext_flood_flag{
    /*min peer id in this flood flag*/
    u_short min_id;

    /*max peer id in this flood flag*/
    u_short max_id;
    
    /*flag,one bit per peer.we will send
      update to these peer*/
    u_char bits[60];
}tBGP4_EXT_FLOOD_FLAG;

/*struct for route flooding control*/
typedef struct bgp4_flood_flag{
    /*node in rib's withdraw table*/
    avl_node_t node;

    /*back pointer to route*/
    struct bgp4_route *p_route;
    
    /*flag,one bit per peer.we will send
      update to these peer*/
    u_char bits[32];

    /*extend flag for large peer number*/
    tBGP4_EXT_FLOOD_FLAG *p_extend[2];
}tBGP4_FLOOD_FLAG;

/*base flag support as most 255 peers*/
#define BGP4_MAX_BASE_PEER_ID 255

/*one extend flag contain 400 peers*/
#define BGP4_EXT_PEER_ID_COUNT 400

typedef struct bgp4_route {
   /*node used in rib table*/ 
   avl_node_t ribnode;
   
   /*node in path info route list*/
   avl_node_t node;
   
   tBGP4_PATH *p_path;

   /*routing table it belong to,if NULL,this route is not added
    to any table*/
   avl_tree_t *p_table;
   
   /*route protocol:ospf.local.static,bgp...*/
   u_short proto; 
   
   /*set by route policy interface,per route*/
   u_short preference;

   /*local assigned label*/
   u_int in_label;

   /*label learnt from peer,used as outgoing label when fwding*/
   u_int out_label;

   /*local assigned label for upe peer*/
   u_int upe_label;
   
   /*is summary route*/
   u_short summary_route : 1;
    
   /*route is summary,and have covered routes*/
   u_short active : 1;

   /*route is covered by summary route*/
   u_short summary_filtered : 1;
    
   /*indicate if route is stale in graceful-restart process*/
   u_short stale : 1;
   
   /*route will be deleted when all update operation finished*/
   u_short is_deleted : 1;

   /*this route is selected as kernal route*/
   u_short system_selected : 1;

   /*this route is filtered by policy,can not update to system*/
   u_short system_filtered : 1;

   /*no direct nexthop for route*/
   u_short system_no_nexthop : 1;
  
   /*route is in hardware and use mpls label forwarding*/
   u_short in_mpls_hardware : 1;

   /*if waiting for IGP sync*/
   u_short igp_sync_wait : 1;

   /*if route damp checked*/
   u_short damp_checked : 1;
   
   /*kernal flag for each nexthop*/
   u_char in_kernalx;

   /*hw flag for each nexthop*/
   u_char in_hardwarex;
   u_int vpn_label;
   
   /*new logic for kernal and update msg*/
   tBGP4_FLOOD_FLAG *p_withdraw;

   tBGP4_FLOOD_FLAG *p_feasible;   
   
   /*end of struct,may use avaliable length later*/
   tBGP4_ADDR dest;
   
    /*process id,such as ospf*/
   u_int processId;
}tBGP4_ROUTE;

/*vector,containing all routes with same prefix*/
typedef struct {
  /*real count in route node*/
  u_int count ;

  /*pointer to route*/
  tBGP4_ROUTE *p_route[BGP4_MAX_ECMP];
}tBGP4_ROUTE_VECTOR;

typedef struct BGP4_EXT_COMM{
    avl_node_t node;

    u_char main_type;
    
    u_char sub_type;
    
    u_short as;
    
    u_int address;
    
    u_int additive;
}tBGP4_EXT_COMM;

#define bgp4_path_init(x) do{\
        bgp4_unsort_avl_init(&((x)->aspath_list));\
        bgp4_unsort_avl_init(&((x)->as4path_list));\
        bgp4_avl_init2(&((x)->route_list), bgp4_avl_unsort_cmp, sizeof(avl_node_t));\
}while(0)

/*remove all dynamic resource for path*/
#define bgp4_path_clear(x) do{\
        bgp4_apath_list_free((x));\
        if ((x)->p_unkown != NULL)\
            bgp4_free((x)->p_unkown, MEM_BGP_BUF);\
        if ((x)->p_ecommunity != NULL)\
            bgp4_free((x)->p_ecommunity, MEM_BGP_BUF);\
        if ((x)->p_cluster != NULL)\
            bgp4_free((x)->p_cluster, MEM_BGP_BUF);\
        if ((x)->p_community != NULL)\
            bgp4_free((x)->p_community, MEM_BGP_BUF);\
        if ((x)->p_aggregator != NULL)\
            bgp4_free((x)->p_aggregator, MEM_BGP_BUF);\
        if ((x)->p_new_aggregator != NULL)\
            bgp4_free((x)->p_new_aggregator, MEM_BGP_BUF);\
        if ((x)->p_direct_nexthop)\
            bgp4_free((x)->p_direct_nexthop, MEM_BGP_NEXTHOP);\
        if ((x)->p_old_nexthop)\
            bgp4_free((x)->p_old_nexthop, MEM_BGP_NEXTHOP);\
}while(0)

tBGP4_PATH * bgp4_path_create(tBGP4_VPN_INSTANCE *p_instance, u_int af);
tBGP4_PATH *bgp4_path_add(tBGP4_PATH *p_path);
tBGP4_PATH *bgp4_path_lookup(tBGP4_PATH *p_path);
tBGP4_EXT_COMM* bgp4_add_ext_comm(tBGP4_EXT_COMM* p_ext_comm);
tBGP4_EXT_COMM* bgp4_ext_comm_lookup(tBGP4_EXT_COMM* p_ext_comm);
int bgp4_aspath_same (tBGP4_PATH *p_path1, tBGP4_PATH *p_path2) ;
int bgp4_as4path_same (tBGP4_PATH *p_path1, tBGP4_PATH *p_path2) ;
int bgp4_mpreach_nlri_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet, avl_tree_t * p_list);
int bgp4_mpunreach_nlri_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet, avl_tree_t * p_list);
int bgp4_cluster_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_originator_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_origin_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_aspath_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_as4path_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_nexthop_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_med_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_lpref_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_atomic_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_aggregator_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_unkown_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_community_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_ecommunity_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_mpunreach_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int bgp4_mpreach_extract(tBGP4_PATH * p_path, tBGP4_RXUPDATE_PACKET * p_packet);
int  bgp4_path_same(tBGP4_PATH * p_path1, tBGP4_PATH * p_path2);
u_short bgp4_attribute_hdr_fill(u_char * p_msg, u_char type,u_short len);
u_short bgp4_origin_fill (u_char *, tBGP4_ROUTE *);
u_short bgp4_aspath_fill (u_char *, tBGP4_PATH *, tBGP4_PEER *);
u_short bgp4_nexthop_fill (u_char *,tBGP4_ROUTE *, tBGP4_PEER *);
u_short bgp4_med_fill (u_char *, tBGP4_ROUTE *,tBGP4_PEER *);
u_short bgp4_lpref_fill (u_char *, tBGP4_ROUTE *, tBGP4_PEER *);
u_short bgp4_aggregate_fill (u_char *, tBGP4_ROUTE *,tBGP4_PEER*);
u_short bgp4_unkown_fill (u_char *, tBGP4_PATH *);
u_short bgp4_originid_fill(u_char  *,tBGP4_ROUTE  *,tBGP4_PEER *);
u_short bgp4_cluster_fill(u_char  *,tBGP4_ROUTE  *,tBGP4_PEER *);
u_short bgp4_community_fill(u_char  *,tBGP4_ROUTE  *,tBGP4_PEER *);
u_short bgp4_ecommunity_fill(u_char*, tBGP4_ROUTE *,tBGP4_PEER *);
u_short bgp4_mp_nexthop_fill(tBGP4_PEER *,u_char*,tBGP4_ROUTE *);
u_short bgp4_path_fill(tBGP4_PEER *,tBGP4_ROUTE *, u_char  *);    
u_short bgp4_nlri_fill(u_char *,tBGP4_ROUTE *);
u_short bgp4_mp_nlri_fill(u_char * p_msg, tBGP4_ROUTE * p_route);
u_short bgp4_vpls_nlri_fill(u_char *p_msg, tBGP4_ROUTE *p_route);
u_short bgp4_mp_unreach_fill(u_char * p_msg, u_short af, u_char *p_nlri, u_short len);
u_short bgp4_mp_reach_fill(u_char * p_msg, u_short af, u_char *p_nexthop, u_char nhoplen, u_char *p_nlri, u_short len);
u_short bgp4_path_aspath_len(tBGP4_PATH *p_path);
u_int bgp4_label_extract(u_char *p_buf);
u_int bgp4_as_exist_in_aspath(avl_tree_t *p_list, u_short as);
STATUS bgp4_path_loop_check(tBGP4_PATH *p_path);
STATUS bgp4_originid_loop_check(tBGP4_PATH *p_info );
STATUS bgp4_cluster_loop_check(tBGP4_PATH * p_info);
void bgp4_delete_all_ext_comm();
void bgp4_path_garbage_collect(tBGP4_RIB *p_rib);
void bgp4_apath_list_free(tBGP4_PATH *p_path);
void bgp4_path_copy(tBGP4_PATH *p_dest ,tBGP4_PATH *p_src );
void bgp4_delete_ext_comm(tBGP4_EXT_COMM* p_ext_comm);
void bgp4_path_free(tBGP4_PATH *p_path);
void bgp4_attribute_desc_init(void);
void bgp4_label_fill(u_char *p_buf, u_int label);

#ifdef __cplusplus
}
#endif  
#endif

#else
#ifndef BGP4PATH_H
#define BGP4PATH_H
#ifdef __cplusplus
      extern "C" {
     #endif
typedef struct ASPath {
    tBGP4_LISTNODE node;
    u_char  type;
    u_char  len;
    u_short  rsvd;
    u_char *p_asseg;
}tBGP4_ASPATH; 

#define BGP4_MAX_CLUSTERID 4
#define BGP4_MAX_COMMNUITY 4
#define BGP4_MAX_ECOMMNUITY 8

/************community attribute*****************/
enum {
           COM_ACTION_ADD = 1,
           COM_ACTION_REPLACE ,
           COM_ACTION_CLEAR 
};

/*description for attribute*/
typedef struct {
   /*flag for the attribute*/
   u_char flag;
   
   /*unit length of this attribute,0 means no unit length*/
   u_char unitlen;

   /*is attribute is zero length,1-means has zero length*/
   u_char zerolen;

   /*fixed length of attribute,0 has no meaning*/
   u_char fixlen;
}tBGP4_ATTR_DESC;

typedef struct {
    u_char val[8];
}tBGP4_ECOMMUNITY;

typedef struct bgp4_path{
    tBGP4_LISTNODE node ;
    
    tBGP4_PEER * p_peer; 

    tBGP4_LIST aspath_list;    

    /*contain route node*/
    tBGP4_LIST      route_list;

    tBGP4_ECOMMUNITY ecommunity[BGP4_MAX_ECOMMNUITY];

    tBGP4_VPN_INSTANCE* p_instance;/*for imported routes use*/

    u_int src_instance_id;

    u_int cluster[BGP4_MAX_CLUSTERID];

    u_int community[BGP4_MAX_COMMNUITY];
    
    struct {
        u_int asnum;
        tBGP4_ADDR addr;
    }aggregator,new_aggregator; 

    u_short         afi ;
    u_char         safi ;
    u_char         origin;   
    u_int         rcvd_med;
    u_int         rcvd_localpref;
    u_int         out_med;
    u_int         out_localpref;
    u_int           origin_id;
    u_char         *p_unkown ;
    u_int          unknown_len ;
    struct {
        /*have aggregator*/
        u_int aggr : 1 ;

        /*have med*/
        u_int med : 1 ;

        /*have localpref*/
        u_int lp : 1 ;
        
        /*rxd community not to external*/
        u_int notto_external : 1 ;
        
        /*rxd community not to intternal*/
        u_int notto_internal : 1 ;  
        
        u_int atomic : 1 ;

     u_int excommunity_count:8;
#if 0
     u_int sync_flag:1;
#endif
     u_int set_origin:1;/*set by policy*/

     u_int rsvd:17;
    }flags;    
        
    tBGP4_ADDR  nexthop;
    tBGP4_ADDR  direct_nexthop;
    int            direct_nexthop_ifunit;
    tBGP4_ADDR  nexthop_global;
    tBGP4_ADDR  nexthop_local;
 }tBGP4_PATH; 

#if 1 /* new logic for kernal and update msg*/
 /*IP action of a route*/
enum {
    BGP4_IP_ACTION_NONE = 0,/*nothing*/
    BGP4_IP_ACTION_ADD,/*add to ip*/
    BGP4_IP_ACTION_DELETE/*delete from ip*/
};

#endif

typedef struct bgp4_route {
   /*node used in rib table*/ 
   tBGP4_RIBNODE ribnode;
   
   /*node in path info route list*/
   tBGP4_LISTNODE node ;
   u_int vpn_label;


   tBGP4_PATH *p_path;

   tBGP4_PATH *p_out_path;

   u_short proto ; 
   
   short   refer;    

   u_short preference;/*set by route policy interface,per route*/

   u_short rsvd2; 

   /*is summary route*/
   u_int is_summary : 1;
    
   /*route is summary,and have covered routes*/
   u_int summary_active : 1;

   /*route is covered by summary route*/
   u_int filter_by_range : 1;
    
   /*be filtered by other policy*/
   u_int is_filtered : 1;

   /*indicate if route is stale in graceful-restart process*/
   u_int stale : 1;

   /*indicate if route is deferral in graceful-restart process*/
   u_int deferral:1;

   u_int rib_add:1;
   
   /*route will be deleted when all update operation finished*/
   u_int is_deleted : 1;

   /*ip update action BGP4_IP_ACTION_XXX*/
   u_int ip_action : 2 ;
   
   /*is ip update finished*/
   u_int ip_finish : 1;

   /*is rtmsg send finished*/
   u_int rtmsg_finish : 1 ;

   /*is mplsvpn route notify finished*/
   u_int mpls_notify_finish : 1 ;

   /*mpls vpn route direction*/
   u_int route_direction : 8;

   /*if waiting for IGP sync*/
   u_int igp_sync_wait : 1;

   /*if exist in ip table,for ECMP*/
   u_int ip_table : 1;

   u_int rsvd:9;

   /* new logic for kernal and update msg*/
   /*withdraw flag,one bit per peer*/
   u_char withdraw_bits[BGP4_MAX_PEER_ID/8];

   /*update flag,one bit per peer*/
   u_char update_bits[BGP4_MAX_PEER_ID/8];

   /*end of struct,may use avaliable length later*/
   tBGP4_ADDR dest;
}tBGP4_ROUTE;

#define BGP4_MAX_ECMP 8
/*vector,containing all routes with same prefix*/
typedef struct {
  /*real count in route node*/
  u_int count ;

  /*pointer to route*/
  tBGP4_ROUTE *p_route[BGP4_MAX_ECMP];
}tBGP4_ROUTE_VECTOR;

typedef struct BGP4_EXT_COMM{
    tBGP4_LISTNODE node;

    u_char main_type;
    u_char sub_type;
    u_short as;
    u_int address;
    u_int additive ;
}tBGP4_EXT_COMM;

INT1 bgp4_is_confed_peer(u_short as) ;

u_int bgp4_asseq_exist(tBGP4_LIST *p_list, u_short as);
void bgp4_apath_list_free(tBGP4_PATH *p_path);
tBGP4_PATH * bgp4_add_path(u_int af);
void bgp4_path_copy(tBGP4_PATH *p_dest ,tBGP4_PATH *p_src );
int  bgp4_aspath_same (tBGP4_PATH *p_path1, tBGP4_PATH *p_path2) ;
int bgp4_delete_all_confed_peer() ;
void bgp4_clear_unused_path();
int bgp4_send_notify(tBGP4_PEER *p_peer, u_char code, u_char sub_code,u_char *p_data, u_int errlen);
int bgp4_extract_mpunreach_nlri(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, short len, tBGP4_LIST * p_list);
int bgp4_extract_clusterlist(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, short len);
int bgp4_extract_originator(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, short len);

int bgp4_extract_origin(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, u_short len);
int bgp4_extract_aspath(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, short len);
int bgp4_extract_nexthop(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, short len);
int bgp4_extract_med(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, short len);
int bgp4_extract_lpref(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, short len);
int bgp4_extract_atomic(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, short len);
int bgp4_extract_aggr(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, short len);
int bgp4_extract_unkown(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, short len);
int bgp4_extract_com(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, short len);
int bgp4_extract_mpreach_nlri(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, short len, tBGP4_LIST * p_list);
int bgp4_extract_extcom(tBGP4_PATH * p_path, u_char * p_buf, u_char hlen, short len);

u_short bgp4_fill_attr_hdr(u_char * p_msg, u_char type,u_short len);
u_short  bgp4_fill_origin (u_char *, tBGP4_ROUTE *);
u_short  bgp4_fill_aspath (u_char *, tBGP4_PATH *, tBGP4_PEER *);
u_short  bgp4_fill_nexthop (u_char *,tBGP4_ROUTE *, tBGP4_PEER *);
u_short  bgp4_fill_med (u_char *, tBGP4_ROUTE *,tBGP4_PEER *);
u_short  bgp4_fill_localpref (u_char *, tBGP4_ROUTE *, tBGP4_PEER *);
u_short  bgp4_fill_aggregate (u_char *, tBGP4_ROUTE *,tBGP4_PEER*);
u_short  bgp4_fill_unkown (u_char *, tBGP4_PATH *);
u_short  bgp4_fill_originid(u_char  *,tBGP4_ROUTE  *,tBGP4_PEER *);
u_short  bgp4_fill_cluster_list(u_char  *,tBGP4_ROUTE  *,tBGP4_PEER *);
u_short  bgp4_fill_community(u_char  *,tBGP4_ROUTE  *,tBGP4_PEER *);
u_short  bgp4_fill_ext_community(u_char*, tBGP4_ROUTE *,tBGP4_PEER *);
u_short  bgp4_fill_mp_nexthop( u_char, tBGP4_PEER *,u_char*,tBGP4_ROUTE *);
u_short  bgp4_fill_attr(u_int, tBGP4_PEER *,tBGP4_ROUTE *, u_char  *); 
u_short  bgp4_fill_nlri(u_char *,tBGP4_ROUTE *);
u_short  bgp4_fill_mp_nlri(u_char,u_char*,tBGP4_ROUTE *);
void bgp4_path_free(tBGP4_PATH *p_path);
INT1 bgp4_add_confed_peer(u_short as);
INT1 bgp4_delete_confed_peer(u_short as);
int  bgp4_same_path(tBGP4_PATH * p_path1, tBGP4_PATH * p_path2);
STATUS  bgp4_check_originid(tBGP4_PATH *p_info );
STATUS bgp4_check_clusterlist(tBGP4_PATH * p_info);
tBGP4_PATH *bgp4_path_lookup(tBGP4_PATH *p_path);
void bgp4_attribute_desc_init();
int bgp4_extract_path_attribute(tBGP4_PATH * p_path, u_char * p_buf, int len, tBGP4_LIST * p_flist, tBGP4_LIST * p_wlist);
u_short bgp4_fill_mp_unreach(u_char * p_msg, u_short af, u_char *p_nlri, u_short len);
u_short bgp4_fill_mp_reach(u_char * p_msg, u_short af, u_char *p_nexthop, u_char nhoplen, u_char *p_nlri, u_short len);
STATUS bgp4_verify_path(tBGP4_PEER *p_peer, tBGP4_PATH *p_path);
u_short bgp4_path_aspath_len(tBGP4_PATH *p_path);
void bgp4_path_add_to_list(tBGP4_PATH*p_path,u_char af);
tBGP4_PATH *bgp4_path_merge(tBGP4_PATH *p_path);

tBGP4_EXT_COMM* bgp4_add_ext_comm(tBGP4_EXT_COMM* p_ext_comm);
void bgp4_delete_ext_comm(tBGP4_EXT_COMM* p_ext_comm);
tBGP4_EXT_COMM* bgp4_ext_comm_lookup(tBGP4_EXT_COMM* p_ext_comm);
extern STATUS ip_route_match(u_int routeInstance,u_int dest, M2_IPROUTETBL *p_info);

u_short bgp4_as4path_fill(u_char *p_msg, tBGP4_PATH *p_path, tBGP4_PEER *p_peer);
u_short bgp4_as4path_fill_2(u_char *p_msg, tBGP4_PATH *p_path, tBGP4_PEER *p_peer);
u_short bgp4_aspath_fill_2(u_char *p_msg, tBGP4_PATH *p_path, tBGP4_PEER *p_peer);
u_int bgp4_as_first_in_aspath(avl_tree_t *p_list, u_short *as);


#ifdef __cplusplus
     }
     #endif  
#endif


#endif
