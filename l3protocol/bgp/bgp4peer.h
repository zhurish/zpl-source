#include "plateform.h"
#include "bgp4msgh.h"
#include "bgp4main.h"
#ifdef NEW_BGP_WANTED
#ifndef BGP4PEER_H
#define BGP4PEER_H
#ifdef __cplusplus
extern "C" {
#endif
/* PEER Type Internally used by various modules */
enum {
    BGP4_EBGP = 0x01,
    BGP4_IBGP ,
    BGP4_CONFEDBGP 
};

/* PEER FSM STATES */
enum {
    BGP4_NS_NONE = 0, 
    BGP4_NS_IDLE, 
    BGP4_NS_CONNECT, 
    BGP4_NS_ACTIVE,  
    BGP4_NS_OPENSENT,
    BGP4_NS_OPENCONFIRM, 
    BGP4_NS_ESTABLISH, 
    BGP4_NS_MAX 
};

#define BGP4_MIN_HOLD_INTERVAL 3     /* Secs */ 

enum {
    BGP4_EVENT_START,
    BGP4_EVENT_STOP,
    BGP4_EVENT_RESTART,
    BGP4_EVENT_TCP_OPENED,
    BGP4_EVENT_TCP_CLOSED,
    BGP4_EVENT_CONNECT_FAIL,
    BGP4_EVENT_FATAL_ERROR,
    BGP4_EVENT_RETRY_TIMEOUT,
    BGP4_EVENT_HOLD_TMR_EXP,
    BGP4_EVENT_KEEPALIVE_TMR_EXP,
    BGP4_EVENT_GR_TMR_EXP,
    BGP4_EVENT_MAX 
};

typedef struct s_bgp_pkt{
   /*current buffer length,from starting*/
   u_int len ;
   
   /*read buffer*/
   u_char buf[BGP4_MAX_MSG_LEN];
}tBGP4_BUF;

#define BGP_PASSWORD_LEN  80

struct bgp4_peer_stat{
    u_int established_transitions;
    
    u_int uptime;
    
    u_int rx_updatetime;
    
    u_int rx_update;
    
    u_int tx_update;
    
    u_int rx_msg;
    
    u_int tx_msg;
    
    u_int rx_fea_route;
    
    u_int rx_with_route;
    
    u_int tx_feasible;
    
    u_int tx_withdraw;  
    
    u_int max_rx_updatetime;/*Max process time of input update*/   

    u_int max_start_updatetime;/*Max Process Time of start connection*/

    u_int max_end_updatetime;/*Max Process Time of end connection*/           

    u_short last_errcode;

    u_short last_subcode;

    u_int msg_len_err;

    u_int msg_marker_err;

    u_int open_rx;

    u_int open_tx;

    u_int open_err;

    u_int update_err;

    u_int notify_rx;

    u_int notify_tx;

    u_int keepalive_rx;

    u_int keepalive_tx;

    u_int discard_route;
    
    u_int event;  
};

/*notify information send to peer*/
typedef struct bgp4_peer_notify{
    u_char code;

    u_char sub_code;

    u_char len;

    /*if active closed peer*/
    u_char close_peer;
    
    u_char data[64];
}tBGP4_PEER_NOTIFY;

/*check adjout after 2seconds*/
#define BGP4_PEER_ADJOUT_INTERVAL 2

typedef struct bgp4_peer {
    avl_node_t node;

    /*node in bit index table*/
    avl_node_t bit_node;
    
    tBGP4_VPN_INSTANCE *p_instance;/*vpn instance pointer that the peer belongs to*/    

    tBGP4_ADDR update_source;/*connect interface ip*/
    
    /*CONNECTED IP ADDR*/ 
    tBGP4_ADDR ip;
    
    /*CONNECTED IP ADDR*/ 
    tBGP4_ADDR local_ip;

    /*muitlple type of timer*/
    tTimerNode connect_timer;
    
    tTimerNode retry_timer;    
    
    tTimerNode hold_timer;   
    
    tTimerNode keepalive_timer;       
    
    tTimerNode gr_timer;    

    /*timer is running if nbr is to be deleted,and there
      are rest route from nbr to be released...*/
    tTimerNode delete_timer;

    /*timer for control sending update of peer*/
    tTimerNode adj_out_timer;

    /*timer for checking bfd session*/
    tTimerNode bfd_timer;
    
    /*route policy control,include export and import policy*/
    avl_tree_t policy_table;

    /*orf table send to peer*/
    avl_tree_t orf_out_table;

    /*orf table recv from peer*/
    avl_tree_t orf_in_table;

    /*last orf table recv from peer*/
    avl_tree_t orf_old_in_table;
    
    /*store an output packet*/
    struct bgp4_update_packet *p_txupdate;
    
    /*recv msg buff*/
    u_int rx_len;

    u_char *p_rxbuf;

    /*notify to be send*/
    tBGP4_PEER_NOTIFY notify;
    
    u_int router_id;

    /*tcp socket id*/
    int sock;

    /*internal index in peer array,used for flooding*/
    u_int bit_index;

    /*Interface connected to peer,decide by system route table*/
    u_int if_unit;
    
    int bfd_discribe; 

    /*AS NUM*/
    u_int as;

    /*address family flags:learnt from peer*/
    u_int mpbgp_capability; 

    /*address family flags:local configured*/
    u_int local_mpbgp_capability;

    /*af support GR,learnt from peer*/
    u_int restart_mpbgp_capability;
    
    /*recv end-of-rib flag for each afi/safi*/
    u_int rxd_rib_end;
    
    /*send end-of-rib flag for each afi/safi*/
    u_int txd_rib_end;    
    
    /*tcp port*/
    u_short tcp_port; 

    u_short local_tcp_port; 

    /*setting of reset time,must < remote hold time in open*/
    u_short restart_time;

    /*static set tcp server port,if it is not configured,use default 179*/
    u_short fixed_tcp_port;

    /*FSM state*/
    u_char state;

    /*NM state*/
    u_char row_status;

    /*BGP version*/    
    u_char version;

    /*MD5 key len*/
    u_char md5key_len;

    /*MD5 key string*/
    u_char md5key[BGP_PASSWORD_LEN];

    /*connect retry configured*/
    u_int retry_interval;

    /*hold time configured*/
    u_int hold_interval;

    /*keeplive sending interval configured*/
    u_int keep_interval;

    u_short origin_interval; 
        
    u_char adv_interval; 

    /*FSM start interval configured*/
    u_char start_interval;

    /*neg result*/
    u_int neg_hold_interval;

    /*neg result*/
    u_int neg_keep_interval;
    
    /*max prefix num*/
    u_int max_prefix;

    /*current recv route num*/
    u_int current_prefix;

    /*TTL HOPS,can be set by nm*/
    u_int ttl_hops;
    
    /*rxd server socket from kernal*/
    int sync_server_sock;

    /*rxd client socket from kernal*/
    int sync_client_sock;
    
    /*fake as num 0:invalid,others:valid fake as*/
    u_int fake_as;

    /*allow local as num repeated times*/
    u_int allow_as_loop_times;

    /*af supporting sending orf*/
    u_int orf_send;

    /*af supporting recving orf*/
    u_int orf_recv;

    /*af supporting sending orf-learnt from peer*/
    u_int orf_remote_send;

    /*af supporting recving orf-leanrt from peer*/
    u_int orf_remote_recv;
    
    /*indicate if remote support 4bytes as*/
    u_int as4_enable : 1;

    /*if route reflector client*/
    u_int is_reflector_client : 1;

    /*if send community to*/
    u_int send_community : 1;

    /*if enable md5*/
    u_int md5_support : 1;

    /*if md5 option is set*/
    u_int md5_server_set:1;

    /*enable multihop ebgp neighbor-1*/    
    u_int mhop_ebgp_enable : 1;

    /*tcp connection not be established now,wait for a moment*/
    u_int connect_inprogress : 1;

    /*admin state,halted(1),running(2)*/    
    u_int admin_state:2;
    
    u_int cease_reason:4;

    /*has rxd notify msg from peer:txd open option is rejected*/
    u_int option_tx_fail:1;

    /*1-has unsupport cap,should not retry to establish peer,or otherwise*/
    u_int unsupport_capability:1;

    /*current role in gr,normal-0,restarting-1,recving-2*/    
    u_int restart_role : 4;

    /*if enable 6PE label*/
    u_int send_label : 1;

    /*0 can not send local ext_comm 1:send local ext_comm*/
    u_int ext_comm_enable : 1;

    /*if enable using local as num to substitute for peer's as num*/
    u_int as_substitute_enable : 1;

    /*if filter private AS num when routes sent out to EBGP peer*/
    u_int public_as_only : 1;

    /*if enable nexthop self*/
    u_int nexthop_self : 1;

    /*if enable bfd*/
    u_int bfd_enable :1;
    
    /*route refresh enable:learnt from peer*/
    u_int refresh_enable : 1; 
    
    u_int local_refresh_enable : 1; 
    
    /*reset bit,record GR's Rbit:learnt from peer.TRUE:peer is in restarting*/
    u_int in_restart : 1;
    
    /*support of gr capability,learnt from peer*/
    u_int restart_enable : 1;

    /*support UPE*/
    u_int upe_enable : 1;
    
    /*statistic*/
    struct bgp4_peer_stat stat;  
/*Interface connected to peer type*/
   u_char if_typename[64];
} tBGP4_PEER; 

#define bgp4_peer_rxmsg_reset(p) do{\
                      (p)->rx_len = 0;\
                       if ((p)->p_rxbuf) bgp4_free((p)->p_rxbuf, MEM_BGP_RXMSG);\
                       (p)->p_rxbuf = NULL;}while(0)
#define bgp4_peer_for_each(ins, pe) bgp4_avl_for_each(&((ins)->peer_table), (pe))

#define bgp4_peer_for_each_safe(ins, pe, nxt) bgp4_avl_for_each_safe(&((ins)->peer_table), (pe), (nxt))

#define bgp4_af_support(peer, afflag) (flag_isset((peer)->local_mpbgp_capability, afflag) && flag_isset((peer)->mpbgp_capability, afflag))

#define bgp4_peer_start_time_backoff(x) do{\
    if ((x)->start_interval < BGP4_MAX_STARTINTERVAL)\
    (x)->start_interval *= 2;\
    else\
    (x)->start_interval = BGP4_DFLT_STARTINTERVAL;}while(0)

void
bgp4_peer_connect_timer_expired(tBGP4_PEER* p_peer);
void
bgp4_peer_keepalive_timer_expired(tBGP4_PEER* p_peer);
void
bgp4_peer_gr_expired(tBGP4_PEER* p_peer);
void
bgp4_peer_adjout_expire(tBGP4_PEER *p_peer);

void
bgp4_peer_retry_timer_expired(tBGP4_PEER* p_peer);

tBGP4_PEER *bgp4_peer_create(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR *ip);
tBGP4_PEER *bgp4_peer_lookup(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR *ip);
tBGP4_PEER *bgp4_peer_lookup_by_index(tBGP4_VPN_INSTANCE *p_instance, u_int id);
int bgp4_peer_lookup_cmp(tBGP4_PEER *p1, tBGP4_PEER *p2);
int bgp4_peer_index_lookup_cmp(tBGP4_PEER *p1, tBGP4_PEER *p2);
u_char bgp4_peer_type(tBGP4_PEER *p_peer) ;
void bgp4_peer_md5_authentication(tBGP4_PEER *p_peer,u_char action);
void bgp4_fsm_invalid(tBGP4_PEER *p_peer);
void bgp4_peer_sock_close(tBGP4_PEER *p_peer);
void bgp4_timer_event(tTimerNode* p_timer);
void bgp4_msg_input(tBGP4_PEER *p_peer,u_char *p_pdu);
void bgp4_peer_delete (tBGP4_PEER *p_peer);
void bgp4_fsm(tBGP4_PEER * p_peer, u_short event);
void bgp4_peer_reset_all(void);
void bgp4_peer_af_set(tBGP4_PEER *p_peer,u_int af_cmd);
void bgp4_peer_delete_expired(tBGP4_PEER* p_peer);
void bgp4_peer_holdtimer_expired(tBGP4_PEER* p_peer);

#ifdef __cplusplus
}
#endif     
#endif

#else
#ifndef BGP4PEER_H
#define BGP4PEER_H
#ifdef __cplusplus
      extern "C" {
     #endif

/* PEER Type Internally used by various modules */
enum {
    BGP4_EBGP = 0x01,
    BGP4_IBGP ,
    BGP4_CONFEDBGP 
};

/* PEER FSM STATES */
enum {
    BGP4_NS_NONE = 0, 
    BGP4_NS_IDLE, 
    BGP4_NS_CONNECT, 
    BGP4_NS_ACTIVE,  
    BGP4_NS_OPENSENT,
    BGP4_NS_OPENCONFIRM, 
    BGP4_NS_ESTABLISH, 
    BGP4_NS_MAX 
};

#define BGP4_MIN_HOLD_INTERVAL 3     /* Secs */ 

#define BGP4_BFD_DISABLE -1

/*peer timer*/
enum {
           BGP4_START_TIMER = 0,
           BGP4_CONNECTRETRY_TIMER,
           BGP4_HOLD_TIMER,
           BGP4_KEEPALIVE_TIMER,
           BGP4_MINASORIG_TIMER,
           BGP4_MINROUTEADV_TIMER,
           BGP4_RESET_TIMER,
           BGP4_MAX_TIMER
};

enum {
    BGP4_EVENT_START,
    BGP4_EVENT_STOP,
    BGP4_EVENT_RESTART,
    BGP4_EVENT_TCP_OPENED,
    BGP4_EVENT_TCP_CLOSED,
    BGP4_EVENT_CONNECT_FAIL,
    BGP4_EVENT_FATAL_ERROR,
    BGP4_EVENT_RETRY_TIMEOUT,
    BGP4_EVENT_HOLD_TMR_EXP,
    BGP4_EVENT_KEEPALIVE_TMR_EXP,
    BGP4_EVENT_GR_TMR_EXP,
    BGP4_EVENT_MAX 
};

typedef struct s_bgp_pkt{
   /*current buffer length,from starting*/
   u_int len ;
   
   /*read buffer*/
   u_char buf[BGP4_MAX_MSG_LEN];
}tBGP_PACKET;

#define BGP_PASSWORD_LEN  80
typedef struct bgp4_peer {
    tBGP4_LISTNODE    node;

    tBGP4_VPN_INSTANCE* p_instance;/*vpn instance pointer that the peer belongs to*/    

    u_int router_id;

    /*local and remote information*/
    struct {
        tBGP4_ADDR ip;/*CONNECTED IP ADDR*/ 
             
        u_int as;/*AS NUM*/
       
       
        u_int af; /*address family flags*/

    
        u_int reset_af;/*af support GR*/
       
        u_int update_time;/*update sent time*/
       
        u_int port; /*tcp port*/

        /*some reset flag*/
        u_short refresh : 1 ; /*route refresh enable*/
     
        u_short reset_bit : 1 ;/*reset bit,record GR's Rbit*/
       
        u_short reset_enable : 1 ;/*support of gr capability,default 0*/

        u_short rsvd:13;       
       
        u_short reset_time ;/*  setting of reset time,must < remote hold time in open*/
     
        u_int capability;/*cap advertisement*/
    }local, remote;  
 
    u_char state;/*FSM state*/

    u_char row_status;/*NM state*/
    
    u_char version;/*BGP version*/    
    
    u_char md5key_len;/*MD5 key len*/
    
    u_char md5key[BGP_PASSWORD_LEN];/*MD5 key string*/
    
    u_int retry_interval;/*connect retry*/

    u_int hold_interval;/*hold time*/

    u_int keep_interval;/*keeplive sending interval*/

    u_short origin_interval; 
        
    u_char adv_interval; 

    u_char start_interval;/*FSM start interval*/

    u_int  neg_hold_interval;/*neg result*/
    
    u_int  neg_keep_interval;/*neg result*/
           
    u_int outif; 

    u_int if_unit;/* Interface connected to peer */

    tBGP4_ADDR update_source;/*connect interface ip*/

    u_int event;  

    int sock;/*peer connect sock*/
    
    /*muitlple type of timer*/
    tTimerNode   timer[BGP4_MAX_TIMER];

    /*statistic*/
    u_int established_transitions;
    u_int uptime;
    u_int rx_updatetime;
    u_int rx_update;
    u_int tx_update;
    u_int rx_msg;
    u_int tx_msg;
    u_int rx_fea_route;
    u_int rx_with_route;
    u_int tx_feasible;
    u_int tx_withdraw;        
    u_int max_rx_updatetime;/*Max process time of input update*/   
    u_int max_start_updatetime;/*Max Process Time of start connection*/
    u_int max_end_updatetime;/*Max Process Time of end connection*/           
    u_short last_errcode;
    u_short last_subcode;
    u_int msg_len_err;
    u_int msg_marker_err;
    u_int open_rx;
    u_int open_tx;
    u_int open_err;
    u_int update_err;
    u_int notify_rx;
    u_int notify_tx;
    u_int keepalive_rx;
    u_int keepalive_tx;
    u_int discard_route;

    tBGP_PACKET  rxmsg;/*recv msg buff*/

    u_int rib_end_af;/*recv end-of-rib flag for each afi/safi*/

    u_int send_rib_end_af;/*send end-of-rib flag for each afi/safi*/

    u_int  max_prefix;/*max prefix num*/
    u_int  peer_route_number;/*current recv route num*/
    u_int  ttl_hops;/*TTL HOPS,can be set by nm*/
    u_int  send_capability;/*cap sent in latest open*/

    tBGP4_LIST rt_policy_import;/*peer import route policy list*/
    tBGP4_LIST rt_policy_export;/*peer export route policy list*/
    
    /*internal index in peer array*/
    u_int bit_index;

    u_int wcount ;

    u_int fcount ;

    u_int total_update ;
    
    /*withdraw route sent to peer*/
    tBGP4_LIST wlist;

    /*feasible route sent to peer*/
    tBGP4_LIST flist;   
    
    /*for two tcp connection sync*/ 
    u_int last_sync_fd;

    /*fake as num*/
    u_int fake_as;/*0:invalid,others:valid fake as*/

    /*allow local as num repeated times*/
    u_int allow_as_loop_times;
    /*some flags*/
    u_int send_rib_end : 1;/*indicate if send rib end */
    u_int as4_enable : 1;/*indicate if remote support 4bytes as*/
    u_int rr_client : 1;/*if route reflector client*/
    u_int send_community : 1;/*if send community to*/
    u_int md5_support : 1;/*if enable md5*/
    u_int md5_server_set:1;/*if md5 option is set*/
    u_int ebgp_mhop : 1;/*enable multihop ebgp neighbor-1*/
    u_int noactive_open : 1;/*0-can setup active connection using connect,1-only accept passive connection.*/          
    u_int connect_inprogress : 1;
    u_int admin:2;/*admin state,halted(1),running(2)*/  
    u_int cease_reason:4;
    u_int send_open_option:1;/*0-should not send open with option para,or otherwise*/
    u_int unsupport_capability:1;/*1-has unsupport cap,should not restart,or otherwise*/
    u_int reset_role : 4;/*current role in gr,normal-0,restarting-1,recving-2*/    
    u_int send_label : 1;/*if enable 6PE label*/
    u_int ext_comm_enable : 1;/*0 can not send local ext_comm 1:send local ext_comm*/
    u_int as_substitute_enable : 1;/*if enable using local as num to substitute for peer's as num*/
    u_int public_as_only : 1;/*if filter private AS num when routes sent out to EBGP peer*/
    u_int  nexthop_self : 1;/*if enable nexthop self*/
    u_int bfd_enable :1;/*if enable bfd*/
    u_int  rsvd:5;

#ifdef BGP_BFD_WANTED       
        int bfd_discribe;  
#endif

} tBGP4_PEER; 

/*UPDATE MSG DATA*/
typedef struct {
    
    u_char buf[BGP4_MAX_MSG_LEN];/*msg header*/
    
    u_char *p_msg;/*msg buffer*/
    
    u_short len;/*total length*/
    
    u_short with_len;/*withdraw length*/
    
    u_short attr_len;/*attribute length*/
    
    u_short with_count;/*withdraw length*/

    u_short fea_count ;/*feasible route cnt*/ 

    u_short rsvd;
}tBGP4_UPDATE_INFO;

#define bgp4_stop_peer_gr_timer(p_peer)  bgp4_stop_timer(&(p_peer)->timer[BGP4_RESET_TIMER])
#define bgp4_start_peer_gr_timer(p_peer)  bgp4_start_peer_timer(&(p_peer)->timer[BGP4_RESET_TIMER],(p_peer),(p_peer)->remote.reset_time,BGP4_EVENT_GR_TMR_EXP) 

#define bgp4_start_retry_timer(p_peer)       bgp4_start_peer_timer(&((p_peer)->timer[BGP4_CONNECTRETRY_TIMER]),p_peer,p_peer->retry_interval,BGP4_EVENT_RETRY_TIMEOUT)
#define bgp4_restart_retry_timer(p_peer)  do{bgp4_stop_retry_timer((p_peer));bgp4_start_retry_timer((p_peer));}while(0)
#define bgp4_stop_retry_timer(p_peer)        bgp4_stop_timer (&((p_peer)->timer[BGP4_CONNECTRETRY_TIMER]))

#define bgp4_start_start_timer(p_peer) bgp4_start_peer_timer (&((p_peer)->timer[BGP4_START_TIMER]), (p_peer),p_peer->start_interval,BGP4_EVENT_START) 
#define bgp4_stop_start_timer(p_peer)  bgp4_stop_timer  (&((p_peer)->timer[BGP4_START_TIMER]))

#define bgp4_start_keepalive_timer(p_peer) bgp4_start_peer_timer (&((p_peer)->timer[BGP4_KEEPALIVE_TIMER]), (p_peer),p_peer->neg_keep_interval,BGP4_EVENT_KEEPALIVE_TMR_EXP)
#define bgp4_stop_keepalive_timer(p_peer)  bgp4_stop_timer  (&((p_peer)->timer[BGP4_KEEPALIVE_TIMER]))

#define bgp4_start_hold_timer(p_peer)         bgp4_start_peer_timer(&((p_peer)->timer[BGP4_HOLD_TIMER]), p_peer, (p_peer)->neg_hold_interval,BGP4_EVENT_HOLD_TMR_EXP); 
#define bgp4_restart_hold_timer(p_peer)    do{bgp4_stop_hold_timer((p_peer));bgp4_start_hold_timer((p_peer));}while(0)
#define bgp4_stop_hold_timer(p_peer)               bgp4_stop_timer (&((p_peer)->timer[BGP4_HOLD_TIMER]))

#define bgp4_af_support(peer, afflag) (af_isset((peer)->local.af, afflag) && af_isset((peer)->remote.af, afflag))

#define is_ibgp_peer(peer) ((peer)->remote.as == gBgp4.asnum)

#define bgp4_update_start_time(x) do{\
    if ((x)->start_interval < BGP4_MAX_STARTINTERVAL)\
    (x)->start_interval *= 2;\
    else\
(x)->start_interval = BGP4_DFLT_STARTINTERVAL;}while(0)


tBGP4_PEER * bgp4_add_peer(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR *ip);
tBGP4_PEER *bgp4_peer_lookup(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR *ip);
void bgp4_delete_all_peer(tBGP4_LIST* p_peer_list) ;
void bgp4_delete_all_peer_route();
void bgp4_peer_md5_authentication(tBGP4_PEER *p_peer,u_char action);
u_char bgp4_peer_type(tBGP4_PEER *p_peer) ;
void bgp4_fsm_invalid(tBGP4_PEER *p_peer);
int bgp4_sock_close(tBGP4_PEER *p_peer);
void bgp4_send_keepalive(tBGP4_PEER *p_peer);
int bgp4_open_input(tBGP4_PEER *p_peer, u_char *p_msg, u_int len);
int bgp4_notify_input(tBGP4_PEER *p_peer, u_char *p_buf, u_int len);
int bgp4_update_input(tBGP4_PEER * p_peer, u_char * p_msg, u_int len, tBGP4_LIST * p_flist, tBGP4_LIST * p_wlist);
void bgp4_send_open(tBGP4_PEER * p_peer);
int bgp4_refresh_input(tBGP4_PEER *p_peer,u_char *p_buf,u_int msg_len);
int bgp4_send_refresh(tBGP4_PEER *p_peer,u_int af_flag);
void bgp4_timer_event(tTimerNode* p_timer);
void bgp4_msg_input(tBGP4_PEER *p_peer,u_char *p_pdu);
u_char * bgp4_printf_state(u_short state , u_char *p_str);
void bgp4_delete_peer (tBGP4_PEER *p_peer);
void bgp4_fsm(tBGP4_PEER * p_peer, u_short event);
void bgp4_reset_all_peers(u_int force_flag);
void bgp4_set_peer_af(tBGP4_PEER *p_peer,u_int af_cmd);
void bgp4_peer_if_addr_change(tBGP4_ADDR* p_if_addr,u_int if_unit);

extern  int sendBgpBackwardTransitionTrap(int addrType, char * peerid, int state, char * lastError);
void bgp4_peer_connect_timer_expired(tBGP4_PEER * p_peer);
void bgp4_peer_retry_timer_expired(tBGP4_PEER * p_peer);
void bgp4_peer_holdtimer_expired(tBGP4_PEER * p_peer);
void bgp4_peer_keepalive_timer_expired(tBGP4_PEER * p_peer);
void bgp4_peer_gr_expired(tBGP4_PEER* p_peer);
void bgp4_peer_adjout_expire(tBGP4_PEER* p_peer);
u_int bgp4_end_of_rib_check(tBGP4_PEER *p_peer, u_char *p_pdu, u_int len);
u_short bgp4_open_capablilty_fill(tBGP4_PEER* p_peer, u_char* p_option);
void bgp4_display_update_input(tBGP4_PEER *p_peer, avl_tree_t  *flist, avl_tree_t *wlist );

#ifdef __cplusplus
     }
     #endif     
#endif

#endif
