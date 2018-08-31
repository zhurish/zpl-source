#include "plateform.h"
#include "bgp4_api.h"
#include "bgp4dfs.h"
#ifdef NEW_BGP_WANTED
#ifndef  BGP4MAIN_H
#define  BGP4MAIN_H
#ifdef __cplusplus
 extern "C" {
#endif

#define BGP4_IPADDR_LEN 4
#define BGP4_IP6ADDR_LEN 16
#define BGP4_MAX_IPADDR_LEN 16

#define BGP4_MSG_SIZE 100
#define BGP4_MAX_MSG  100

#define BGP4_TASK_PRIORITY                100
#define BGP4_TASK_DEF_DEPTH              (1024 * 256)
#define BGP4_PROTOCOL_INVALID         0x10000000
#define BGP4_DEFAULT_LOCALPREF       100
#define BGP4_DEFAULT_MED       0
#define BGP4_DEFAULT_UPDATE_LIMIT 1400

#define BGP_MAX_PEER_NUM   256

/*common address struct for ipv4 and ipv6*/  
typedef struct {
    /*address family*/
    u_short afi;

    /*prefix length in bits*/
    u_short prefixlen;

    /*ip address.in network order*/
    u_char ip[24];
}tBGP4_ADDR;

/*format of vpls route dest */
typedef struct{
    u_short ve_id;
    
    u_short ve_block_size;
    
    u_short ve_block_offset;

    u_short rsvd;
    
    u_int label_base;
}tBGP4_VPLS_ADDR;

/*memory type define*/
enum {
    MEM_BGP_PEER, 
    MEM_BGP_ROUTE,  
    MEM_BGP_LINK,  
    MEM_BGP_INFO,
    MEM_BGP_ASPATH,
    MEM_BGP_BUF,
    MEM_BGP_COMMUNITY,
    MEM_BGP_NETWORK,
    MEM_BGP_POLICY_CONFIG,
    MEM_BGP_VPN_INSTANCE,
    MEM_BGP_RANGE,
    MEM_BGP_FLOOD,
    MEM_BGP_RXMSG,
    MEM_BGP_NEXTHOP,
    MEM_BGP_DAMP,
    MEM_BGP_RIB,
    MEM_BGP_ORF,
    MEM_BGP_EXTFLOOD,
    MEM_BGP_MAX
};

/*global statstics*/
struct bgp_sock_stat{
    u_int create;
        
    u_int create_fail;
    
    u_int bind;
    
    u_int bind_fail;
    
    u_int connect;
    
    u_int connect_fail;
    
    u_int tx;
    
    u_int tx_fail;
    
    u_int accept;
    
    u_int accept_fail;
    
    u_int rx;
    
    u_int rx_fail;
};

struct bgp4_sysroute_stat{
    u_int add;
    
    u_int fail;
    
    u_int add_error;

    /*last error without msg*/
    u_int add_nomsg_error;
    
    u_int delete;
    
    u_int delete_fail;
    
    u_int delete_error;

    /*last error without msg*/
    u_int delete_nomsg_error;

    /*send msg for route add*/
    u_int msg_add;

    /*send msg for route delete*/
    u_int msg_delete;

    u_int msg_send;

    /*failed to send msg*/
    u_int msg_fail;

    /*total length of msg send*/
    u_int msg_total_len;

    /*total route sucessfully send*/
    u_int msg_total_route;
};

struct bgp4_sync_stat{
    u_int tx;

    u_int rx;

    u_int fail;
};

struct bgp4_pdu_stat{
    u_int open;
    
    u_int update;
    
    u_int keepalive;
    
    u_int notify;

    u_int long_update;

    u_int bytes;
};

typedef struct {
    /*count of socket operation*/
    struct bgp_sock_stat sock;

    /*count of system ipv4 route operation*/
    struct bgp4_sysroute_stat sys_route;

    /*count of system ipv6 route operation*/
    struct bgp4_sysroute_stat sys_route6;

    /*count of system mpls vpn route*/
    struct bgp4_sysroute_stat mpls_route;

    /*count of sync msg operation*/
    struct bgp4_sync_stat sync;

    struct bgp4_pdu_stat msg_tx;

    struct bgp4_pdu_stat msg_rx;
    
    u_int vrf_not_exist;

    u_int duplicate_delete;
    
    u_int tcp_close;
    
    u_int import_filtered_route;
    
    u_int export_filtered_route;
    
    u_int vrf_add;
    
    u_int vrf_delete;
    
    u_int import_rt_add;
    
    u_int import_rt_delete;
    
    u_int export_rt_add;
    
    u_int export_rt_delete;
    
    u_int l2vpn_add;
    
    u_int l2vpn_delete;
    
    u_int l2vpn_import_rt_add;
    
    u_int l2vpn_import_rt_delete;
    
    u_int l2vpn_export_rt_add;
    
    u_int l2vpn_export_rt_del;

    u_int rib_in_discard;

    /*loop count during system update*/
    u_int system_loop;

    u_int rtsock_rx;

    u_int rtsock_rx_error;

    u_int rtsock_cmd;
}tBGP4_STAT;

#define BGP4_MAX_CONFEDPEER 8

#define BGP4_MAX_PEER_ID /*256*/BGP_MAX_PEER_NUM

#define BGP4_WORKMODE_INTERVAL 1

#define BGP4_PATH_CLEAR_INTERVAL 10

#define BGP4_IGP_SYNC_MAX_INTERVAL 30

#define BGP4_IGP_SYNC_MIN_INTERVAL 1

#define BGP4_ROUTE_CLEAR_INTERVAL 5

#define BGP4_PENALTY_HALF_TIME 900/*15 minitues*/

#define BGP4_PENALTY_UPDATE_INTERVAL 5

#define BGP4_PENALTY_REUSE 750

#define BGP4_PENALTY_SUPRESS 2000

#define BGP4_PENALTY_MAX 16000

#define BGP4_PENALTY_INCREASE 1000

#define BGP4_MAX_TIMER_TABLE 128

#define BGP4_ROUTE_MSG_QUEUE_SIZE  BGP4_DEFAULT_ROUTE_NUMBER

#define BGP4_IF_LINK_MSG_QUEUE_SIZE 100
/*Global BGP information struct,contain all bgp related global information*/
typedef struct s_bgpGlbInfo{
    pthread_mutex_t sem;
    
    int rtsock;/*interface of routesocket*/
               
    /*tcp sock*/
    int server_sock[L3_VPN_INSTANCE_NUM + 1];
    
    int server_sock6[L3_VPN_INSTANCE_NUM + 1];

    u_int server_port;

    u_int timer_table_id;
    
    /*list of timers*/
    tTimerLst timer_table[BGP4_MAX_TIMER_TABLE];

    /*routes selection deferral until all end of rib received or such timer expire*/
    tTimerNode gr_waiting_timer;
    
    /*timer for read system working mode*/  
    tTimerNode workmode_timer;
        
    /*timer for IGP sync checking*/  
    tTimerNode igp_sync_timer;
                
    /*timer for init update*/  
    tTimerNode init_update_timer;
    
    /*timer for init sync*/  
    tTimerNode init_sync_timer;

    /*timer for control update sending rate*/
    tTimerNode update_ratelimit_timer;
    
    /*ex-community attributes list sent by update msg*/
    avl_tree_t ecom_table;

    /*vpn instance data list,member type is tBGP4_VPN_INSTANCE*/
    avl_tree_t instance_table;
    
    /*array of confederate peer's as num*/    
    u_int confedration_as[BGP4_MAX_CONFEDPEER];

    /*bgp router ID*/
    u_int router_id;

    /*local as number*/
    u_int as;

    /*max update size*/
    u_int max_update_len;

    /*confederate as num seen by EBGP peers*/
    u_int confedration_id;

    /*cluter id,used to prevent loopback*/
    u_int cluster_id;

    /*community attribute value sent by update msg*/
    u_int community;

    /*default local pref*/
    u_int local_pref;

    /*default med*/
    u_int med;
    
    /*fdset containing socket to read*/
    fd_set *fd_read;

    /*fdset containing socket to write*/
    fd_set *fd_write;

    /*current max socket*/
    int max_sock;
    
    u_int tick_per_second;

    /*current send update size*/
    u_int update_tx_len;
    
    /*resource*/
    u_int max_peer;
    
    u_int max_path;
    
    u_int max_route;
    
    u_int max_vpn_instance;
    
    u_int max_l2pw_info;
    
    u_int max_l2vpn_instance;

    /*gr restart time of gr speaker that carried in open msg,
    *which must not longer than hold time*/
    u_int restart_period;

    u_int restart_wait_period;

    u_char *p_rxbuf;
    
    u_int debug;

    u_int sync_flag;

    u_int work_mode : 3; 
    
    /*bgp router admin state*/
    u_int enable : 1;
    
    u_int reflector_enable : 1;
    
    u_int igp_sync_enable : 1;
        
    u_int community_action : 2;
    
    u_int trap_enable: 1;
    
    u_int tcp_server_enable: 1;
    
    u_int tcp_client_enable: 1;
    
    u_int damp_enable : 1;
            
    /*if router support GR*/
    u_int restart_enable:1;
    
    /*if router execute gr*/
    u_int in_restarting:1;

    /*4bytes as support*/
    u_int as4_enable:1;
    
    /*if support 6PE*/
    u_int ipv6pe_enable : 1; 

    /*support fast backup route*/
    u_int backup_path_enable : 1; 
    
    /*damp params*/
    /*half lifetime of penalty decay*/
    u_int penalty_half_time;

    /*max value of penalty*/
    u_int penalty_max;

    /*suppressed value of penalty*/
    u_int penalty_supress;    
    
    /*reuse value of penalty*/
    u_int penalty_reuse;    

    /*pdu id for fragment in sync*/
    u_int sync_pdu_id;

    /*rxd fragment of a pdu*/
    void *sync_fragment_pdu[4];
    
    /*route policy apply func*/
    int (*policy_func)(int ,void *);

    /*route policy reference count func*/
    int (*policy_ref_func)(int, int);

    void *(*malloc)(u_int, u_int);/*memory init func*/
    
    void (*mfree)(void *);/*memory free func*/
    
    /*statistic data*/
    tBGP4_STAT stat;  
    u_int ipv4_vpnv4;
	
    int BgpRouteIdFlag;

    u_int uiPeerCount;

    u_char ucEbgpPre;

    u_char ucIbgpPre;

    u_char ucLocalPre;
}tBGP4_GLOBAL;

/*rx buffer,512Kbytes*/
#define BGP4_RXBUF_LEN /*(64*1024)*/512*1024

/*100ms interval for process input rib*/
#define BGP4_RIB_IN_CHECK_INTERVAL 100

/*1000 routes can be processed in a loop*/
#define BGP4_RIB_IN_CHECK_LIMIT 1000

/*60s---rib in check timer in slave card*/
#define BGP4_RIB_IN_SLAVE_CHECK_INTERVAL 60

/*different type of instance*/
enum{
    BGP4_INSTANCE_IP = 0,
    BGP4_INSTANCE_VPLS
};

/*bgp instance info*/
typedef struct s_bgpInstanceInfo{
    avl_node_t node; 

    /*vrf id,0 is public instance*/
    u_int vrf;

    /*vpn type*/
    u_int type;
    
    /*include all rib for each af*/
    struct bgp4_rib *rib[BGP4_PF_MAX];
    
    /*list of aggregated route range*/
    avl_tree_t range_table;

    /*list of peers belonged to this instance(CE list)*/
    avl_tree_t peer_table;

    /*list of peers belonged to this instance(CE list):use internal index as key*/
    avl_tree_t peer_index_table;

    /*list of network routes*/
    avl_tree_t network_table;

    /*route policy control,include export and import policy*/
    avl_tree_t policy_table;

    /*imported route policy config*/
    avl_tree_t redistribute_policy_table;

    /*timer for checking range's state*/
    tTimerNode range_update_timer;

    /*timer for delete*/
    tTimerNode delete_timer;
    
    /*rd for vpn*/
    u_char rd[BGP4_VPN_RD_LEN];

    /*VPLS attribute,only valid when instance is vpls type*/
    u_short mtu;
   
    u_char encapsulation;
 
    u_char control_word;

    /*local vpls information*/
    tBGP4_VPLS_ADDR local_nlri;

    /*upe peer count*/
    u_short upe_count;    

    /*6pe peer count*/
    u_short v6pe_count;
    u_int instance_id;

    u_int enable_state; /*caoyong add 2017 1121*/
}tBGP4_VPN_INSTANCE; 

/*damp information in special rib*/
typedef struct bgp4_damp_route{
    /*node in rib's damp table*/
    avl_node_t node;

    /*back pointer to rib*/
    struct bgp4_rib *p_rib;
    
    /*dest of network*/
    tBGP4_ADDR dest;

    /*peer address*/
    tBGP4_ADDR peer;

    /*penalty update timer,check for each 5 seconds*/
    tTimerNode timer;

    /*current penalty value*/
    int penalty;

    /*deceased value when timer expired*/
    u_int penalty_decrease;
}tBGP4_DAMP_ROUTE;

/*include all route related information*/
typedef struct bgp4_rib{
    /*address family type*/
    u_short af;
    
    /*flag,TRUE:we will scan rib for system update*/
    u_char system_check_need;

    /*TRUE:some path nexthop changed,we must delete
      old system route first,and add new nexthop */
    u_char nexthop_changed;
    
    /*instance it apply*/
    tBGP4_VPN_INSTANCE *p_instance;

    /*path of route*/
    avl_tree_t path_table;
    
    /*bgp rib table for all af*/   
    avl_tree_t rib_table; 
    
    /*store route learnt from peer and other protocol,
     wait for process.after processing,these route will
     be updated to above rib_table.*/   
    avl_tree_t rib_in_table; 

    /*withdraw to be send*/
    avl_tree_t out_withdraw_table; 

    /*feasible to be send*/
    avl_tree_t out_feasible_table; 

    /*damp route table*/
    avl_tree_t damp_table; 

    /*message to update hardware*/
    u_char *p_hwmsg;
    
    /*update system */
    tTimerNode system_timer;
    
    /*timer to clear unused path*/
    tTimerNode path_timer;

    /*timer to check input rib*/
    tTimerNode rib_in_timer;

    /*timer to clear uunsed route*/
    tTimerNode route_clear_timer;

    /*timer for check nexthop change*/  
    tTimerNode nexthop_timer;
    
    /*next route will be checked in walkup process,only for system update*/
    struct bgp4_route *p_next_walk;

    /*next route will be checked in nexthop change process*/
    struct bgp4_route *p_change_route;
}tBGP4_RIB;

#define bgp4_damp_table_flush(r) do{if (r) {bgp4_avl_walkup(&((r)->damp_table), bgp4_damp_route_delete);}}while(0)

#define BGP4_MAX(a,b) (((a) > (b)) ? (a) : (b))

/*wrapped timer api*/
/*init timer*/
#if 0
#define bgp4_timer_init(tmr, func, a1) do{\
    (tmr)->pFunc = (TIMERFUNCPTR)(func);\
    if(func == bgp4_path_clear_timeout)\
    {\
      printf("a1 = 0x%x\n",a1);\
    }\
    (tmr)->arg1 = (a1);\
    (tmr)->arg2 = NULL;\
    (tmr)->arg3 = NULL;}while(0)
#endif
/*start timer:seconds,must be called after inited*/
#define bgp4_timer_start(tmr, s) do{\
    u_int tid = gbgp4.timer_table_id & (BGP4_MAX_TIMER_TABLE - 1);\
    gbgp4.timer_table_id++;\
    bgp4_timer_stop(tmr);\
    timerStart(&gbgp4.timer_table[tid], (tmr),\
    (s)*gbgp4.tick_per_second,\
    (tmr)->pFunc, (tmr)->arg1, (tmr)->arg2, (tmr)->arg3);\
    }while(0)

/*start timer:ms,must be called after inited*/
#define bgp4_timer_start_ms(tmr, ms) do{\
    u_int tid = gbgp4.timer_table_id & (BGP4_MAX_TIMER_TABLE - 1);\
    gbgp4.timer_table_id++;\
    bgp4_timer_stop(tmr);\
    timerStart(&gbgp4.timer_table[tid], (tmr),\
    ((ms)*gbgp4.tick_per_second)/1000,\
    (tmr)->pFunc, (tmr)->arg1, (tmr)->arg2, (tmr)->arg3);\
    }while(0)

/*stop timer*/
#define bgp4_timer_stop(x) do{\
    u_int tid = 0;\
    for (tid = 0 ; tid < BGP4_MAX_TIMER_TABLE ; tid++)\
        timerStop(&gbgp4.timer_table[tid],(x));\
    }while(0)

#define bgp4_timerlist_checking() do{\
    u_int tid = 0;\
    for (tid = 0 ; tid < BGP4_MAX_TIMER_TABLE ; tid++)\
        timerListCheck(&gbgp4.timer_table[tid],50);\
    }while(0)

/*init timer list*/
#define bgp4_timerlist_init() do{\
    u_int tid = 0;\
    for (tid = 0 ; tid < BGP4_MAX_TIMER_TABLE ; tid++)\
        timerListInit(&gbgp4.timer_table[tid]);\
    }while(0)

/*is timer running*/
#define bgp4_timer_is_active(x) (((x)->ucActive == TIMER_ACTIVE) ? TRUE : FALSE)/*caoyong 修改,认为应该是等于*/

#define bgp4_instance_for_each(x) bgp4_avl_for_each(&gbgp4.instance_table, (x))
#define bgp4_instance_for_each_safe(x, nxt) bgp4_avl_for_each_safe(&gbgp4.instance_table, (x), (nxt))

extern tBGP4_GLOBAL gbgp4;

tBGP4_VPN_INSTANCE* bgp4_vpn_instance_lookup(u_int type, u_int vrf);
tBGP4_VPN_INSTANCE* bgp4_vpn_instance_create(u_int type, u_int vrf);
u_int *bgp4_confedration_as_lookup(u_short as);
u_int bgp4_6pe_peer_calculate(tBGP4_VPN_INSTANCE *p_instance);
int bgp4_global_init(u_int max_route, u_int max_path, u_int max_peer,u_int max_vpn_instance,
                        u_int max_l2vpn_instance,u_int ipv6pe_enable,void*func1,void* func2 );
int bgp4_global_initEx(u_int max_route, u_int max_path, u_int max_peer,u_int max_vpn_instance,
                        u_int max_l2vpn_instance,u_int ipv6pe_enable,void*func1,void* func2, void *(*mAlloc)(u_int, u_int), void (* mFree)(void *));
int bgp4_disable(void);
int bgp4_enable(u_int asnum);
void bgp4_vpn_instance_del(tBGP4_VPN_INSTANCE* p_instance);
void bgp4_confedration_as_flush(void);
int bgp4_confedration_as_add(u_short as);
void bgp4_confedration_as_delete(u_short as);
void bgp4_task_main(void);
void bgp4_rib_system_update_timeout(tBGP4_RIB *p_rib);
void bgp4_path_clear_timeout(tBGP4_RIB *p_rib);
void bgp4_range_update_timeout(tBGP4_VPN_INSTANCE * p_instance);
void bgp4_vpn_instance_delete_timeout(tBGP4_VPN_INSTANCE *p_instance);
void bgp4_damp_enable(void);
void bgp4_damp_disable(void);
extern STATUS rtSockOptionSet(int sockfd,int protoType,int opType);

void bgp4_update_limit_timeout(void);
void bgp4_igp_sync_timeout(void);
void bgp4_rib_system_update_timeout(tBGP4_RIB *p_ri);
void bgp4_init_update_timeout(void);
void bgp4_path_clear_timeout(tBGP4_RIB *p_rib);
void bgp4_range_update_timeout(tBGP4_VPN_INSTANCE *p_instance);
int bgp4_instance_lookup_cmp(tBGP4_VPN_INSTANCE *p1, tBGP4_VPN_INSTANCE *p2);
extern int bgp_display_routes(tBGP4_VPN_INSTANCE* p_instance,struct vty * vty,u_int vrf_id,u_int *flag);

#ifdef __cplusplus
}
#endif

#endif

#else
#ifndef  BGP4MAIN_H
#define  BGP4MAIN_H
#ifdef __cplusplus
      extern "C" {
     #endif
  
typedef avl_tree_t  tBGP4_RIB ; 
typedef avl_node_t tBGP4_RIBNODE;

typedef struct Addr{
    u_short  afi ;
    u_short  prefixlen;
    u_char  ip[24];
}tBGP4_ADDR;

typedef struct {
    u_int add ;
    u_int fail ;
    u_int del;
}tBGP4_MEMSTAT;

enum {
    MEM_BGP_COMMON = 0,
    MEM_BGP_PEER, 
    MEM_BGP_ROUTE,  
    MEM_BGP_LINK,  
    MEM_BGP_INFO,
    MEM_BGP_ASPATH,
    MEM_BGP_BUF,
    MEM_BGP_TREE, 
    MEM_BGP_CLUSTER,  
    MEM_BGP_COMMUNITY,
    MEM_BGP_CONFED,  
    MEM_BGP_NETWORK,
    MEM_BGP_POLICY_CONFIG,
    MEM_BGP_VPN_INSTANCE,
    MEM_BGP_L2LABEL,
    MEM_BGP_L2VPN_INSTANCE,
    MEM_BGP_MAX
};

/*global statstics*/
typedef struct {
    tBGP4_MEMSTAT   mem[MEM_BGP_MAX];
    u_int   sock;
    u_int   sockfail;
    u_int   bind;
    u_int   bindfail;
    u_int   connect;
    u_int   connectfail;
    u_int   tx;
    u_int   txfail;
    u_int   accept;
    u_int   acceptfail;
    u_int   rcv;
    u_int   rcvfail;
    u_int   iprtadd;
    u_int   iprtfail;
    int        iprtadderror;
    u_int   iprtdel;
    u_int   iprtdelfail;
    int        iprtdelerror;
    u_int   ip6rtadd;
    int        ip6rtadderror;
    u_int   ip6rtfail;
    u_int   ip6rtdel;
    int        ip6rtdelerror;
    u_int   ip6rtdelfail;
    u_int   ipmsgsend;
    u_int   ipmsgfail;
    u_int   ipmsglen;
    u_int   rr_client;
    u_int   sync_msg_send;
    u_int   sync_msg_recv;
    u_int   sync_msg_fail;
    u_int   rt_msg_send;
    u_int   rt_msg_fail;
    u_int   rt6_msg_fail;
    u_int   rt_msg_rt_add;
    u_int   rt_msg_rt_del;
    u_int   rt6_msg_rt_add;
    u_int   rt6_msg_rt_del;
    u_int   tcp_open;
    u_int   tcp_update;
    u_int   tcp_keepalive;
    u_int   tcp_notify;

    u_int   rx_open;
    u_int   rx_update;
    u_int   rx_keepalive;
    u_int   rx_notify;

    u_int   mpls_add_notify;
    u_int   mpls_add_notify_failed;
    int        mpls_add_notify_error;

    u_int   mpls_del_notify;
    u_int   mpls_del_notify_failed;
    int        mpls_del_notify_error;

    u_int   vpn_remote_vrf_no_find_count;
    u_int   rx_big_update;
   
}tBGP4_STAT ;

#define BGP4_MAX_CONFEDPEER 8

#define BGP4_MAX_PEER_ID 256

/*Global BGP information struct,contain all bgp related global information*/
typedef struct s_bgpGlbInfo{

    tTimerLst*  p_timerlist;/*list of timers*/
    
    u_int   confedpeer[BGP4_MAX_CONFEDPEER];/*array of confederate peer's as num*/
    
    u_char   admin;/*bgp router admin state*/
    
    u_char   aggr_apply; /*application range of routes aggregation,default BGP4_EBGP*/
    
    u_short   max_len ;/*max update size*/
    
    u_int   router_id;/*bgp router ID*/
    
    u_short   asnum;/*local as number*/
    
    u_short   confed_id;/*confederate as num seen by EBGP peers*/
    
    u_int   cluster_id;/*cluter id,used to prevent loopback*/
    
    u_int   community;/*community attribute value sent by update msg*/
    
    u_int   default_lpref;/*default local pref*/
    
    u_int   default_med;/*default med*/
    
    tBGP4_LIST  ext_community_list;/*ex-community attributes list sent by update msg*/

    /*global flags*/
    u_int   is_reflector : 1;
    u_int   sync_enable : 1;
    u_int   cease_enable : 1;
    u_int   community_action : 2;
    u_int   trap_enable: 1;
    u_int   server_open: 1;
    u_int   active_open: 1;
    u_int   damp_enable : 1;
    u_int   gr_last_update:1;
    u_int   gr_current_update:1;
    
    /*TRUE: walkup all ribs in main task*/
    u_int   rib_walkup_need : 1;

    /*if router support GR*/
    u_int   gr_enable:1;

    /*if router execute gr*/
    u_int   gr_restart_flag:1;

    /*if set vpn4 target matching*/
    u_int   rt_matching_vpnv4:1;
    
    /*if set vpn6 target matching*/
    u_int   rt_matching_vpnv6:1;

    u_int   rib_update_wait : 1;

    /*if need look up direct nexthop again for nexthop route changing*/
    u_int nexthop_lookup_need : 1;
    
    /*if exists direct_nexthop*/
    u_int direct_nexthop_exist : 1;

    /*if support 6PE*/
    u_int   if_support_6pe : 1;
    
    u_int   rsvd : 12;

    /*contain all peer's pointer*/
    void *  peer_array[BGP4_MAX_PEER_ID];

    pthread_mutex_t  sem;

    u_int   stacksize ;

    u_int   priority ;
    
    int        rtsock;/*interface of routesocket*/

    u_int   timerate;
    
    u_short   server_port;
        
    u_short   client_port;

    /*tcp sock*/
    int        server_sock;
    int        server_sock6;

    /*resource*/
    u_int   max_peer ;
    u_int   max_path ;    
    u_int   max_route ;
    u_int   max_vpn_instance ;
    u_int   max_l2pw_info ;
    u_int      max_l2vpn_instance;

    /*gr restart time of gr speaker that carried in open msg,
    *which must not longer than hold time*/
    u_int   gr_restart_time;
    
    /*routes selection deferral until all end of rib received or such timer expire*/
    tTimerNode  gr_selection_deferral_timer;

    u_int   gr_selection_deferral_time;
    
    u_int   dbg_flag;

    u_int   work_mode;/*word mode*/

    u_int   slave_up;

    u_int   sync_tick;

    u_int      sync_flag;

    u_char      have_slave;
    
    tBGP4_STAT  stat;/*statistic data*/

    int (*routePolicyFunc)(int ,void *);/*route policy apply func*/
    int (*routePolicyRefCntFunc) (int, int);/*route policy reference count func*/

    u_char   ip_msg_buf[1200];

    u_char   ip6_msg_buf[1200];


    /*all input message process time,in ticks*/
    u_int   input_time; 

    /*all output message process time,in ticks*/
    u_int   output_time;

    /*all ip update process time,in ticks*/
    u_int   ipupdate_time;

    /*total wait time in select*/
    u_int   select_time;

    /*next rib to be processed in BGP update*/
    void *  p_rib_update;

    tBGP4_LIST  vpn_instance_list;/*vpn instance data list,member type is tBGP4_VPN_INSTANCE*/

    tBGP4_LIST    l2vpn_instance_list;/*l2vpn instance data list,member type is tBGP4_VS_INSTANCE*/

    tBGP4_LIST  attr_list[BGP4_PF_MAX];/*path attr list,containing tBGP4_PATH*/

    tBGP4_LIST  delay_update_list;     /*add delay update list for nexthop unreachable*/

    u_int   add_tcp_msg_count;

    u_int   add_check_tcp_count;

    u_int   del_tcp_msg_count;

    u_int   del_check_tcp_count;

    u_int   kernel_routes_count;

    u_int   hw_routes_count;

    u_int   duplicate_deleted_route_count;

    u_int   tcp_connection_closed_times;

    u_int   direct_nexthop_changed_times;

    u_int   import_filtered_route_count;
    
    u_int   export_filtered_route_count;

    u_int   vpn_instance_add_count;

    u_int   vpn_instance_del_count;

    u_int   vpn_import_rt_add_count;

    u_int   vpn_import_rt_del_count;

    u_int   vpn_export_rt_add_count;

    u_int   vpn_export_rt_del_count;

    u_int   l2vpn_instance_add_count;

    u_int   l2vpn_instance_del_count;

    u_int   l2vpn_import_rt_add_count;

    u_int   l2vpn_import_rt_del_count;

    u_int   l2vpn_export_rt_add_count;

    u_int   l2vpn_export_rt_del_count;

    int (*memAllocFunc)(int, int);/*memory init func*/
    int (*memFreeFunc) (void *);/*memory free func*/
    
}tBGP4_GLOBAL;

/*bgp instance info*/
typedef struct s_bgpInstanceInfo{

    tBGP4_LISTNODE node; 

    u_int instance_id;

    tBGP4_LIST  aggr_list;/*list of aggregated route range*/
    
    tBGP4_RIB rib; /*bgp rib table*/   

    tBGP4_LIST peer_list;/*list of peers belonged to this instance(CE list)*/
     
    tBGP4_LIST network ;/*list of network routes*/
    
    /*tBGP4_LIST  attr_list;path attr list,containing tBGP4_PATH*/

    tBGP4_LIST route_policy_import;/*global route filter config*/

    tBGP4_LIST route_policy_export;/*global route filter config*/

    tBGP4_LIST import_policy;/*imported route policy config*/

    /*next rib to be processed in ip update*/
    void *p_route_update;

    u_int if_unit;

    u_char rd[BGP4_VPN_RD_LEN];

}tBGP4_VPN_INSTANCE; 

typedef struct s_bgpL2InstanceInfo{

    tBGP4_LISTNODE node; 

    tBGP4_LIST remote_pw;/*list of label information*/

    u_int vpn_vrid;

    u_char l2vpn_rd[8];

    u_short mtu;
   
    u_char encapType;
   
    u_char ctrlWord;

    u_short  local_ve_id;

    u_short local_ve_block_size;

    u_short local_ve_block_offset;

    u_int local_label_base;

}tBGP4_L2VPN_INSTANCE; 


#define bgp4_stop_deferral_timer()  bgp4_stop_timer(&gBgp4.gr_selection_deferral_timer)
#define bgp4_start_deferral_timer() bgp4_deferral_timer_start(&gBgp4.gr_selection_deferral_timer,gBgp4.gr_selection_deferral_time) 

extern tBGP4_GLOBAL gBgp4;

int bgp4_global_init(u_int max_route, u_int max_path, u_int max_peer,u_int max_vpn_instance,
                        u_int max_l2vpn_instance,u_int if_support_6pe,void*func1,void* func2 );
int bgp4_global_initEx(u_int max_route, u_int max_path, u_int max_peer,u_int max_vpn_instance,
                        u_int max_l2vpn_instance,u_int if_support_6pe,void*func1,void* func2, void *(*mAlloc)(u_int,u_int), void (* mFree)(void *));
int bgp4_disable();
int bgp4_enable(u_int asnum) ;
extern int bgp4_shutdown_timer();
INT1 bgp4_check_router_end();
void bgp4_task_main();
tBGP4_VPN_INSTANCE* bgp4_vpn_instance_lookup(u_int instance_id);
tBGP4_VPN_INSTANCE* bgp4_vpn_instance_create(u_int instance_id);
void bgp4_vpn_instance_release(tBGP4_VPN_INSTANCE* p_instance);
void bgp4_vpn_instance_del(tBGP4_VPN_INSTANCE* p_instance);
STATUS bgp4_up_notify(u_int vr_id);
int bgp4_timer_init(tTimerNode *tmr, void *func, u_long arg);


#ifdef __cplusplus
     }
     #endif     
#endif


#endif
