 

#if !defined (_OSPF_MAIN_H_)
#define _OSPF_MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif 
#include "ospf.h"
#include "ospf_table.h"
#include "ospf_policy.h"
/*task priority define,can be replaced using another compile define*/
#ifndef OSPF_TASK_PRIORITY
#define OSPF_TASK_PRIORITY 60 
#endif

/*task stack define,can be replaced using another compile define*/
#ifndef OSPF_TASK_STACK
#define OSPF_TASK_STACK (32*1024)    
#endif

#define OSPF_ROUTE_MSG_QUEUE_SIZE 4000
#define OSPF_IF_LINK_MSG_QUEUE_SIZE 100


#define		OSPF_RECV_OK			0
#define		OSPF_RECV_ERR			1
#define 	OSPF_MSG_SIZE			1512
#define 	OSPF_QUE_LEN 			10 /*OSPF que length*/

#define 	OSPF_REVL3_LEN 		14 /*OSPF REV L3SW length*/


#define OSPF_L3REDIS_MSG_NUM 1024
#define OSPF_DESCRIP_LEN 80

typedef enum
{
	OSPF_UPDATAIPADDR = 1,
	OSPF_UPDATAIFSTATUS,
	OSPF_UPDATAIFDELETE,
		
};

/*task name of ospf*/
#define OSPF_TASKNAME "tOSPF"

/*10ms==1tick*/
#define OSPF_MS_PER_TICK 10

/*100tick==1s*/
#define OSPF_TICK_PER_SECOND (1000/OSPF_MS_PER_TICK)

/*max tick get*/
#define OSPF_MAX_TICKS /*0x00ffffff*/0x1fffffff

/*how many ms used in one second:fixed 100ms,so there is 10ticks in one second*/
#define OSPF_MS_INTERVAL 100

/*max waiting time for socket,30s->300ticks*/
#define OSPF_MAX_WAIT_TIME 30*OSPF_TICK_PER_SECOND

#define OSPF_LSA_CHECK_TIME 600

#define OSPF_L3VPN_NUM_MAX  128
/*start timer in seconds*/
#define ospf_stimer_start(x,y) ospf_timer_start(x,(y)*OSPF_TICK_PER_SECOND);

/*start timer if not started*/
#define ospf_timer_try_start(x, y) do{if (!ospf_timer_active((x))) ospf_timer_start((x), y);}while(0)

/*check if timer is running*/
#define ospf_timer_active(x) ((x)->active == TRUE)

#define ospf_stimer_safe_start(x, y) do{\
    if (!ospf_timer_active(x)\
        || (y*OSPF_TICK_PER_SECOND) < ospf_timer_remain(x, ospf_sys_ticks()))\
    {\
        ospf_stimer_start(x, y);\
    }\
}while(0)
/*schedule a sync timer*/
#define ospf_sync_event_set(x) do{x = TRUE; if (!ospf.sync_check_timer.active)ospf_timer_start(&ospf.sync_check_timer, 1);}while(0)

/*timer struct*/
struct ospf_timer{
    /*node in work list*/
    struct list_head node;
    
    /*work is scheduled*/
    u_int active : 1;
    
    /*delay time in ticks*/
    u_int delay : 31;

    /*start time in ticks*/        
    u_int start_time;
    
    /*callback function of this work*/
    void (*func)(void *);

    void *arg;

    void *context;
};

/*struct used for lsa table,*/
struct ospf_lstable{
    /*node in parent lsa table,interface,area,process*/
    struct ospf_lstnode node;
 
    /*nm node*/
    struct ospf_lstnode nm_node;
 
    /*list in struct ospf*/
    struct ospf_lstnode global_node;
    
    /*standard lsa type*/
    u_int type;
    
    /*list containing lsa*/
    struct ospf_lst list;
 
    /*list containing confict lsa for type3,5,7,RFC2328附录E*/
    struct ospf_lst conflict_list;
 
    /*pointer to parent process*/
    struct ospf_process *p_process;
 
    /*pointer to parent area,only valid for lsa in special area*/
    struct ospf_area *p_area;
 
    /*pointer to parent area,only valid for lsa in special interface*/
    struct ospf_if *p_if;
};

/*statistics for sync msg io*/
struct ospf_syn_stat{
    /*send lsa add msg to backup*/
    u_int send_lsa_add;

    /*send lsa delete msg to backup*/
    u_int send_lsa_del;

    /*recv lsa add msg from master*/
    u_int rcv_lsa_add;

    /*recv lsa delete msg from master*/
    u_int rcv_lsa_del;

    /*recv lsa msg have error*/
    u_int rcv_lsa_error;

    /*send special type of msg to backup*/
    u_int send_msg[11];

    /*recv special type of msg from master*/
    u_int rcv_msg[11];

    /*total send packet to backup,a packet containing multiple msg*/
    u_int send_pkt_cnt;

    /*total recv packet from master,a packet containing multiple msg*/
    u_int rcv_pkt_cnt;

    /*total recvd seqnum mismach packet*/
    u_int seq_mismatch;

    /*the time rcv last seqnum mismatch packet*/
    u_int8 seq_mismatch_time[32];
};

struct ospf_rcv_rtm_stat{
    u_int ifaddr_add;
    
    u_int ifaddr_del;
    
    u_int ifinfo_cnt;
    
    /*total rcv RTM_ADD*/
    u_int rtm_add_cnt;

    /*total rcv RTM_DEL*/
    u_int rtm_del_cnt;

    /*total rcv RTM_NEW_RTMSG*/
    u_int rtmsg_cnt;

    /*total route add in  RTM_NEW_RTMSG*/
    u_int rtmsg_rt_add;

    /*total route del in  RTM_NEW_RTMSG*/
    u_int rtmsg_rt_del;
};
struct ospf_error_route{
    u_int rtsock : 31;

    u_int rt_add : 1;
    
    u_int vrid;

    u_int dest;

    u_int mask;

    u_int fwd;

    u_int metric;
};
/*global statistic*/
struct ospf_statistics{
    /*count of api calling lstfirst*/
    u_int list_first;

    /*count of api calling lstlookup*/    
    u_int list_lookup;

    /*count of api calling lstadd*/    
    u_int list_add;

    /*count of api calling lstdel*/    
    u_int list_delete;

    /*sucessfully add route to system*/
    u_int sys_add_ok;

    /*failed to add route to system*/
    u_int sys_add_error;

    /*sucessfully remove route from system*/
    u_int sys_delete_ok;

    /*failed to remove route from system*/
    u_int sys_delete_error;

    /*sucessfully send routesock msg to system*/
    u_int sys_msg_ok;

    /*failed to send routesock msg to system*/
    u_int sys_msg_error;

    /*total send routesock msg to system*/
    u_int sys_msg_rt_count;

    /*add route total count in send routesock msg to system, maybe send failed*/
    u_int rt_msg_add_total;

    /*add route count failed in send routesock msg to system*/
    u_int rt_msg_add_err;

    /*del route tatal count in send routesock msg to system,, maybe send failed*/
    u_int rt_msg_del_total;

    /*del route count failed in send routesock msg to system*/
    u_int rt_msg_del_err;

    /*last error when try to add/delete route to/from system*/
    u_short sys_errno;

    /*last error when try to send routesock msg*/
    u_short sys_msg_errno;

    /*last error during system update,but error is not NOBUF*/
    u_short kernal_other_errno;

    /*total error during system update,but error is not NOBUF*/
    u_short kernal_other_errcnt;  

    /*sucessfully to add backup route to system*/
    u_short backup_add_ok;

    /*failed to add backup route to system*/
    u_short backup_add_error;

    /*sucessfully to remove backup route from system*/
    u_short backup_del_ok;

    /*failed to remove backup route from system*/
    u_short backup_del_error;   

    /*old backup route is same as current main route,do not delete it*/
    u_int backup_not_del;

    /*total discarded rxd packet for any reason*/
    u_int discard_packet;

    /*error when setting system filter to enable ospf packet*/
    u_int8 pkt_tocpu_error;

    /*error when add an interface to ospf group address*/
    u_int8 mutigroup_error;

    /*error number when add an interface to ospf group address*/
    u_short mutigroup_errno;

    /*number when add an interface to ospf group address*/
    u_short mutigroup_jion_cnt;

    /*number when delete an interface to ospf group address*/
    u_short mutigroup_drop_cnt;
#ifdef HAVE_BFD
    /*rxd system msg indicating bfd peer is admin shutdown,in this case,do not timeout ospf nbr*/
    u_short bfd_admin_down;

    /*rxd system msg indicating bfd peer is down,in this case,timeout ospf nbr direclty*/
    u_short bfd_nbr_down;
#endif
    
    /*router lsa originate count for lsa unexpected aging*/
    u_short check_router_lsa_count;

    /*network lsa originate count for lsa unexpected aging*/
    u_short check_network_lsa_count;
    
    struct ospf_syn_stat sync;

    struct ospf_rcv_rtm_stat rtm;

    /*nbr count in different state*/
    u_short nbr_count[12];

    /*count of local lsa construct*/
    u_int self_lsa_build[12];

    /*count of local lsa changed,if we try to originate a lsa,it's build count increased always.
       but change count increasee only lsa body changed*/    
    u_int self_lsa_change[12];

    /*total rxd lsa from update packet*/
    u_int rxd_lsa[12];

    /*total rxd more recent lsa from update packet*/
    u_int rxd_more_recent_lsa[12];
    
    /*total rxd less recent lsa from update packet*/
    u_int rxd_less_recent_lsa[12];
    
    /*total rxd same recent lsa from update packet*/
    u_int rxd_same_recent_lsa[12];    
    
    /*total rxd more recent lsa from update packet,and lsa body changed---
      will trigger spf calculation,do not include normal refreshed lsa*/
    u_int rxd_change_lsa[12];    

    u_int syn_chg_msg_cnt[3];
    char syn_work_msg_time[20];
    char syn_work_msg_time_old[20];
    char syn_bkp_msg_time[20];
    char syn_bkp_msg_time_old[20];
    char syn_other_msg_time[20];
    char syn_other_msg_time_old[20];

    u_int syn_done_msg_cnt;
    char syn_done_msg_time[20];
    char syn_done_msg_time_old[20];

    u_long rt_cnt[2];
    u_long slaver_rt_cnt[2];
    u_long rt_pre_cnt;

    /*record failed route when export to kernal route table*/
    struct ospf_error_route err_route;

};

/*policy struct for redistribute or route filter*/ 
struct ospf_policy{  
    /*node in process's table*/
    struct ospf_lstnode node;  

    /*node for nm access*/    
    struct ospf_lstnode nm_node;  
    
    /*parent process pointer*/
    struct ospf_process *p_process;
        
    /*system policy index*/
    u_int policy_index; 

    /*system policy name*/
    /*add by wlt*/
    u_int policy_type;

    char policy_name[OSPF_POLICY_NAME_LEN]; 

    /*timer used to control state update,started when policy changed*/
    struct ospf_timer update_timer;

    /*timer used to control policy delete,started when policy is to be deleted*/
    struct ospf_timer delete_timer;
    
    /*stanard protocol*/
    u_short proto;

    /*stanard protocol process*/
    u_short proto_process;

    /*TRUE or FALSE*/
    u_short active;
};

#define OSPF_MAX_THREAD 64

#define OSPF_MAX_THREAD_HEAD 128

/*timer list*/
struct ospf_timer_block{
    /*link in global node*/
    struct ospf_lstnode node;
    
    /*include timer*/
    struct list_head list;

    /*is this list in use*/
    u_int active;
};

/*global nm list*/
struct ospf_nm_table{
    /*area table:process+areaid*/
    struct ospf_lst area_table;
 
    /*area range table*/
    struct ospf_lst range_table;
 
    /*interface table:key process+ipaddr*/
    struct ospf_lst if_table;
 
    /*vinterface table:key process+transit areaid+nbrid*/
    struct ospf_lst vif_table;
 
    /*shamlink*/
    struct ospf_lst shamlink_table;
 
    /*nbr--do not include virtual link*/
    struct ospf_lst nbr_table;
 
    /*lsa table,containing all lsa's table in area scope*/
    struct ospf_lst area_lsa_table;
 
    /*lsa table ,containing all lsa's table in as scope*/
    struct ospf_lst as_lsa_table;
 
    /*lsa table,containing all lsa's table in real if scope*/
    struct ospf_lst if_lsa_table;   
 
    /*lsa table,containing all lsa's table in virtual if scope*/
    struct ospf_lst vif_lsa_table;   
 
    /*network configured*/
    struct ospf_lst network_table;
 
    /*redistribute range configured*/
    struct ospf_lst redis_range_table;
 
    /*redistribute configured*/
    struct ospf_lst redistribute_table;
 
    /*redistribute policy configured*/
    struct ospf_lst redistribute_policy_table;   
 
    /*filter policy configured*/
    struct ospf_lst filter_policy_table;    

	/*asbr range table*/
    struct ospf_lst asbr_range_table;
};

struct ospf_log_table{
    /*nbr  state change log table*/
    struct ospf_lst nbr_table;

    /*lsa change log table*/
    struct ospf_lst lsa_table;

    /*spf calulate log table*/
    struct ospf_lst spf_table;

#ifdef HAVE_BFD
    /*bfd state change log table*/
    struct ospf_lst bfd_table;
#endif
};

/*te tunnel information*/
struct ospf_te_tunnel{
    /*node in global list*/
    struct ospf_lstnode node;

    /*process it belong to*/
    struct ospf_process *p_process;
    
    /*te tunnel head-end */
    u_int addr_in;
    
    /*te tunnel tail-end */
    u_int addr_out;

    /*te tunnel area*/
    u_int area;
    
    /*te interface unit*/
    u_int if_unit;
    
    /*te tunnel cost*/
    u_int cost;
    
    /*te tunnel state UP DOWN*/
    u_int active : 1;
    
    /*te shortcut ability*/
    u_int shortcut : 1;
    
    /*te FA ability*/
    u_int fa : 1;
    
};

/*global control information*/
struct ospf_global{
    /*mutex semaphore*/ 
	void	*master;
	void	*lock_sem;
	int		waittime;

    /*debug control*/
    u_int debug;

    /*debug control for context*/
    u_int process_debug;

    /*system thread in overload mode*/
    u_int8 thread_overload;

    /*some nbr's state changed,we need re-calculate nbr's count*/
    u_int8 nbr_changed : 1;

    /*if forbidden_debug is TRUE,not output the debug*/
    u_int8 forbidden_debug : 1;

    /*system working mode,master,slave,or single*/         
    u_short work_mode;

    /*system working state,master or slave*/         
    u_short syn_work_state;

    /* md5 seqnum for syn switch*/
    u_int seq_offset;

    /*uesd for chech syn packet lost*/
    u_int syn_seq_send;

    u_int syn_seq_rcv;
    
    /*socket for ospf packet*/
    int 	sock[OSPF_L3VPN_NUM_MAX + 1];
	void	*t_read[OSPF_L3VPN_NUM_MAX + 1];
	void	*t_write[OSPF_L3VPN_NUM_MAX + 1];
    /*route socket for system msg*/
    int rtsock;
    
    /*route socket for nm msg*/
    int cmdsock;
	int debugFd;
    /*currently running process*/
    struct ospf_process  *p_running_process;

    /*timer for update nbr state statistic*/
    struct ospf_timer nbr_state_update_timer;
    
    /* big packet*/
    /*this is 64K buffer for max packet length*/
    u_int8 *p_rxbuf;
            
    /*multiple running instance*/
    struct ospf_lst process_table;

    /*interface table,do not include virtual and shamlink
    used for mutiprocess fast search when rcv pkt*/
    struct ospf_lst real_if_table;

    /*buffer for all timer list*/
    struct ospf_timer_block *p_thread;

    /*include all running timer list*/
    struct ospf_lst thread_table;

    /*sync control table for all card*/  
    struct ospf_lst syn_control_table;

    /*include all lsa's table for syn lsa*/
    struct ospf_lst lsatable_table;

    /*workmode checking timer,read system work mode periodly*/
    struct ospf_timer workmode_timer;

    /*timer for check if any sync msg to send*/
    struct ospf_timer sync_check_timer;

    /*timer for read packet from socket*/
    struct ospf_timer rx_timer;

    /*nm table*/
    struct ospf_nm_table nm;

    struct ospf_statistics stat;

    struct ospf_log_table log_table;
    
    int (*route_policy_apply)(u_int , void *);

    int (*route_policy_set)(u_int , u_int );

    void *(*malloc)(char *,u_int,u_int,u_int);
    
    void(* free)(int,void *);
};

extern struct ospf_global ospf;

/*set currently running process,record it's debug flag*/
#define ospf_set_context(process) do{\
    if((process)!= NULL)\
    {\
        ospf.p_running_process=process;\
        ospf.process_debug=((struct ospf_process *)(process))->debug;\
    }\
    else\
    {\
        ospf.p_running_process=NULL;\
        ospf.process_debug=0;\
    }\
}while(0)

 /*global debug flag*/
#define ospf_global_debug (ospf.debug & (1<<OSPF_DBG_MAIN))
#define ospf_global_debug_lsa (ospf.debug & (1<<OSPF_DBG_LSA))
#define ospf_global_debug_hello (ospf.debug & (1<<OSPF_DBG_HELLO))
#define ospf_global_debug_msg (ospf.debug & (1<<OSPF_DBG_PACKET))
#define ospf_global_debug_if (ospf.debug & (1<<OSPF_DBG_IF))
#define ospf_global_debug_nbr (ospf.debug & (1<<OSPF_DBG_NBR))
#define ospf_global_debug_route (ospf.debug & (1<<OSPF_DBG_ROUTE))
#define ospf_global_debug_spf (ospf.debug & (1<<OSPF_DBG_SPF))
#define ospf_global_debug_rtm (ospf.debug & (1<<OSPF_DBG_RTM)) 
#define ospf_global_debug_syn (ospf.debug & (1<<OSPF_DBG_SYN)) 
#define ospf_global_debug_gr (ospf.debug & (1<<OSPF_DBG_GR)) 
#define ospf_global_debug_nbrchange (ospf.debug & (1<<OSPF_DBG_NBRCHANGE))  
#define ospf_global_debug_frr (ospf.debug & (1<<OSPF_DBG_FRR)) 
#define ospf_global_debug_error (ospf.debug & (1<<OSPF_DBG_ERROR)) 
#define ospf_global_debug_syslog (ospf.debug & (1<<OSPF_DBG_SYSLOG))

/*logic OR for global debug and process debug*/
#define ospf_debug ((ospf.debug |ospf.process_debug)& (1<<OSPF_DBG_MAIN))
#define ospf_debug_lsa ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_LSA))
#define ospf_debug_hello ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_HELLO))
#define ospf_debug_msg ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_PACKET))
#define ospf_debug_if ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_IF))
#define ospf_debug_nbr ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_NBR))
#define ospf_debug_route ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_ROUTE))
#define ospf_debug_spf ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_SPF))
#define ospf_debug_rtm ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_RTM)) 
#define ospf_debug_syn ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_SYN)) 
#define ospf_debug_gr ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_GR)) 
#define ospf_debug_nbrchange ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_NBRCHANGE))  
#define ospf_debug_frr ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_FRR)) 
#define ospf_debug_error ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_ERROR)) 
#define ospf_debug_dcn ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_DCN)) 
#define ospf_debug_policy ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_POLICY)) 
#define ospf_debug_bfd ((ospf.debug |ospf.process_debug) & (1<<OSPF_DBG_BFD)) 

#define ospf_global_debug_set(type, val) do\
    {\
       if (val)\
           ospf.debug |= (1 << (type));\
       else\
           ospf.debug &= ~(1 << (type));\
    }while(0)

/*spf calculation log--for better debug*/
#define OSPF_STAT_MAX_NUM 100
struct ospf_spf_loginfo{
    /*node in global stat_table*/
    struct ospf_lstnode node;

    u_int process_id;

    /*spf calculate start time*/
    u_int8 spf_start_time[32];

    /*route calcute period in ticks*/
    u_int calculate_period;
    
    /*dijikstra running time in ticks*/
    u_short spf_period;

    /*full summary calculate time in ticks*/ 
    u_short ia_period;

    /*full external/nssa calculate time in ticks*/ 
    u_short ase_period;

    /*between last spf calculte */
    u_short spf_called;
    
    /*route count after this calculation*/
    u_int route_num;

    /*abr route count after this calculation*/    
    u_short abr_num;

    /*asbr route count after this calculation*/    
    u_short asbr_num;    
};

/*nbr event log,for better debug*/
struct ospf_nbr_loginfo{
    /*node in global stat_table*/
    struct ospf_lstnode node;

    u_int process_id;

    /*nbr ip address*/
    u_int nbr_addr;
    
    /*nbr state change's time string*/
    u_int8 time[32];

    /*old state before event appear*/
    u_int8 last_state;

    /*new state after event appear*/    
    u_int8 state;

    /*event trigger state change*/
    u_short event;
};

#ifdef HAVE_BFD
/*bfd log,for better debug*/
struct ospf_bfd_loginfo{
    /*node in global stat_table*/
    struct ospf_lstnode node;

    u_int process_id;

    /*nbr ip address*/
    u_int nbr_addr;

    /*bfd state change's time string*/
    u_int8 time[32];

    /*state when rcv rtm*/
    u_int8 state;

    /*bfd diag*/
    u_short diag;  
};
#endif
/*action for lsa log*/
enum {
   OSPF_LSA_STAT_CREATE,
   OSPF_LSA_STAT_DELETE,
   OSPF_LSA_STAT_CHANGE
};

/*lsa change log --for better debug*/
struct ospf_lsa_loginfo{
    /*node in global stat_table*/
    struct ospf_lstnode node;

    u_int process_id;

    /*timer string when event detected*/
    u_int8 time[32];

    /*struct ospf_lshdr*/
    u_int8 lsa_hdr[20];

    /*create/delete/change*/
    u_int action;
};

/*invalid vrf id*/
#define OSPF_INVALID_VRID 0xffffffff
#define OSPF_DCN_VRID       0
#define OSPF_NO_VRID       0    /*不启动vpn功能*/

/*packet send rate limit timer interval in seconds*/
#define OSPF_RATELIMIT_INTERVAL 60

/*limit retranmist msg in a period*/
#define OSPF_UPDATE_RXMT_LIMIT 60

/*default referace rate 10000kbit/s,100M/s*/
#define OSPF_DEFAULT_REF_RATE 100

#define OSPF_DEFAULT_COST_TYPE	2

#define OSPF_DEFAULT_COST		1

#ifdef HAVE_BFD
#define OSPF_DEFAULT_BFD_STATE 0
#endif

#define OSPF_DEFAULT_PACKET_BLOCK	0

#define OSPF_DEFAULT_DISTANCE       ZEBRA_OSPF_DISTANCE_DEFAULT
#define OSPF_DEFAULT_ASE_DISTANCE   150

#define OSPF_DEFAULT_MPLSID_COST   0

/*struct used for control route socket msg to be send*/
struct ospf_route_msg{
    /*a bit indcate a route's update state,1--wait for update,0--update finished*/
    u_int wait_update;
   
    /*1024 is max length of rtmsg now*/
    u_int8 buf[1024];
};

/*nexthop weight*/
struct ospf_nhop_weight{
    /*nexthop weight*/
    u_int weight;

    /*nexthop ip address*/
    u_int addr;

    /*nexthop weight falg,TRUE:nexthop weight is configed ,FALSE:not*/
    u_int flag:1;
};

/*ospf process struct*/
struct ospf_process{
    /*node in global table*/
    struct ospf_lstnode node;

    /*back pointer to global struct*/
    struct ospf_global *p_master;

    /*system vrf id bind to this process,it can be set only once*/
    u_int vrid;

    /*vpn route tag,not used now*/ 
    u_int route_tag;
    
    /*internal instance id*/
    u_int process_id;

    /*router id,it is normaly an local ip address*/        
    u_int router_id;   

    /*when set router id,save old_routerid in 60s*/
    u_int old_routerid;

	/*save new router id*/
    u_int new_routerid;

    u_int routerid_flg;

    /*debug flag*/
    u_int debug;

    /*backbone area pointer if exist*/
    struct ospf_area *p_backbone;    

    /*area list*/
    struct ospf_lst area_table;

    /*interface table,contain normal and vlink*/
    struct ospf_lst if_table;

     /*interface order by mask+addr,do not include virtual link*/
    struct ospf_lst normal_if_table;

    /*store interface mask appeared--for fast lookup according to network*/
    u_int ifmask[32]; 
            
    /*virtual interface table*/
    struct ospf_lst virtual_if_table;

    /*shamlink interface table*/
    struct ospf_lst shamlink_if_table;
    
    /*nexthop table*/
    struct ospf_lst nexthop_table;

    /*neighbor table*/
    struct ospf_lst nbr_table;
    
    /*list containing external routes from other protocol*/
    struct ospf_lst import_table;

    /*current routing table & last routing table*/
    struct ospf_lst route_table;

#ifdef OSPF_FRR
    struct ospf_lst backup_route_table;
#endif
    /*list for struct ospf_lstable*/
    struct ospf_lst lstable_table;

    /*type5 lsa table*/
    struct ospf_lstable t5_lstable;

    /*type11 lsa table*/
    struct ospf_lstable t11_lstable;

    /*network configure*/        
    struct ospf_lst network_table;

    /*request list*/
    struct ospf_lst req_table;

    /*global lsa retransmit list*/
    struct ospf_lst rxmt_table;
    
    /*te database interface*/
    struct ospf_lst te_router_table;

    /*list for redistribute route  policy */
    struct ospf_lst redis_policy_table;      

    /*list for filter route  policy */
    struct ospf_lst filter_policy_table;  

    /*list for filtered route by policy */
    struct ospf_lst filtered_route_table;  

    /*20121203 ospf redis range*/
    struct ospf_lst redis_range_table;

    /*list for te tunnel*/
    struct ospf_lst te_tunnel_table; 
    
#ifdef OSPF_DCN
    /*send dcnlsa rtmmsg to dcn manager*/
    struct ospf_lst dcn_rtm_tx_table; 
    /*timer for send dcnlsa rtmmsg*/
    struct ospf_timer dcn_rtm_tx_timer; 
    /*internal instance id*/
    u_int ulDcnFlag;
#endif

    /*timer for checking imported external route*/
    struct ospf_timer import_timer; 

    /*timer for checking overlay module*/
    struct ospf_timer overlay_timer; 


    /*timer for checking full route calculation*/
    struct ospf_timer spf_timer; 

    /*fast protection*/
    struct ospf_timer fast_spf_timer;
    
#ifdef OSPF_FRR
    struct ospf_timer frr_timer;  
#endif

    /*database exchange overload timer,start when 
      dbd exchange failed,when timer is running,do not accpet new
       exchange.*/
    struct ospf_timer db_overload_timer;
    
    /*when modify router id, start routerid_timer*/
    struct ospf_timer routeid_timer;

    /*lsa aging timer for all lsa table*/
    struct ospf_timer lsa_aging_timer;

    /* ip update timer*/
    struct ospf_timer ipupdate_timer;
    
#ifdef OSPF_FRR
    /*ip update timer for backup route*/
    struct ospf_timer backup_ipupdate_timer; 
#endif

    /*rate limit,if too many retransmit 
      detected,we will decrease packet sending rate 
      by calling taskdelay before sending*/
    struct ospf_timer ratelimit_timer;

    /* lsa retransmit limit timer*/
    struct ospf_timer rxmtlimit_timer;

    /*timer for process delete*/
    struct ospf_timer delete_timer;        

    /*timer for router id reset*/
    struct ospf_timer id_reset_timer;        

    /*timer for check area's range*/
    struct ospf_timer range_update_timer; 

	struct ospf_timer redis_range_update_timer;

    /*schedule for build te router lsa*/
    struct ospf_timer te_tunnel_timer;

    /*lsa check timer for check self originate lsa*/
    struct ospf_timer lsa_check_timer;

    /*route socket msg to be send*/         
    struct ospf_route_msg *p_rtmsg;

    /*overflow state and control*/
    u_int overflow_limit; 

    /*overflow state period*/
    u_int overflow_period;

    /*spf running interval in ms*/
    u_short spf_interval;

    /*used to adjust spf request timer interval*/
    u_short spf_delay_interval;

    /*decide how many retransmit update msg to be send*/
    int16_t lsa_rxmt_limit;

    u_short rsvd;

    /*for calculate as-route to fast find the best forward route*/
    /*default value 0xffffffff*/
    u_int min_mask;

     /*default value 0*/
    u_int max_mask;

    /*redistribute information,one bit for each protocol*/
    u_int8 reditribute_flag[4];

    /*redistribute control policy table*/
    struct ospf_lst redistribute_config_table;

   
    /*containing list of fragment lsa*/
    struct ospf_lst fragment_table;

    /*reference rate,used to calculate interface cost,
     unit is Mbits/s---must change to kbits/s when calculation,
     cost is calculated as (reference_rate/if_speed).
     if val is 0,do not use this rate*/ 
    u_int reference_rate;

    /*trap config*/
    /*enabled trap type, The right-most bit (least significant) represents trap 0*/
    u_int trap_enable;
    
    /*error type*/
    u_int trap_error;

    /*packet type*/
    u_int trap_packet;

    /*source of error*/
    u_int trap_source;
    
    /*control flags*/
    /*is protocol enabled*/
    u_int enable : 1;
    
    /*is self asbr*/
    u_int asbr : 1;
    
    /*is self abr*/
    u_int abr : 1;
    
    /* TRUE means this router will be opaque-capable for opaque lsas */
    u_int opaque : 1;   

    /*in rout selection,use rule as rfc1583?*/
    u_int rfc1583_compatibility : 1;

    /*tos capability,used to NM*/  
    u_int tos_support : 1;

    /*demand circuit capability,used to NM*/  
    u_int dc_support : 1;

    /*mcast routing capability,used to NM*/  
    u_int mcast_support : 1;

    /*current routing table index,0 or 1*/
    u_int current_route : 1;

    /*old routing table index,must be 1-current_route*/
    u_int old_route : 1;

#ifdef OSPF_FRR
    /*backup route :current routing table index,0 or 1*/
    u_int backup_current_route : 1;

    /*old routing table index must be 1-backup_current_route*/
    u_int backup_old_route : 1;
#endif        
    /*stub router capablity:TRUE or FALSE*/
    u_int stub_support : 1;

    /*send stub router advertisement*/       
    u_int stub_adv : 1;

    /*send stub route advertisement hold time*/
    u_int stub_router_holdtime;

    /*stub-router timer*/
    struct ospf_timer stub_router_timer;
    
    /*is any vlink exist*/       
    u_int vlink_configured : 1;

    /*used for MASTER,need syn to SLAVE*/
    u_int syn_flag : 1;

    /*originate default type5 lsa*/
    u_int def_route_adv : 1;

    /*flag indicating some route socket msg to be send*/
    u_int send_routemsg : 1;

    /*set to TRUE when process is admin stoppped,can not do any protocol operation
      when this flag is TRUE*/
    u_int proto_shutdown : 1;

    /*total nbr count before restarting,used to control restart period*/                
    u_int gr_nbr_count : 8;

#ifdef OSPF_FRR
    u_int frr : 1;

    u_int frr_enable : 1;
#endif

    /*PE check DN bit to prevent loop, MCE needn't check*/
    u_int vpn_loop_prevent : 1;

    /*redistribute need update for all route*/
    u_int import_update : 1;
    
    /*default type5 lsa cost type*/
    u_int default_cost_type : 2;
    
    /*default type5 lsa cost*/
    u_int default_cost;

    /*accoring rfc4750 mib,not used in protocol*/
    u_int discontinuity_time;

    /* fast protection*/
    u_int fast_spf_count;

    /*调度SPF计算的总次数。每一次调度ospf_spf_sched函数加1*/
    u_int spf_called_count;

    u_int backup_spf_called_count;

    u_int last_spf_called_count;
  
   /*real spf running times*/
    u_int spf_running_count;
   
   /*real spf running times*/
    u_int backup_spf_running_count;

   /*SPF调度启动时刻，Tick计数，在调度
     ospf_spf_sched时，如果当前的计算标志为0，则更新此字段。*/
    u_int spf_start_time;      

   /*max routing table building time*/
    u_int max_spf_time; 

    /*global mib object,see rfc4750*/
    u_int extlsa_count; 

    u_int extlsa_checksum;

    u_int origin_lsa; 

    u_int rx_lsa;

    u_int origin_opaque_lsa;  

    u_int rx_opaque_lsa; 

    u_int t9lsa_count; 

    u_int t9lsa_checksum; 

    u_int t11lsa_count; 

    u_int t11lsa_checksum;

    /*restart information*/

    /*restart timer*/
    struct ospf_timer restart_timer;

    /*wait all gr-lsa acked before peform restarting*/  
    struct ospf_timer restart_wait_timer;
    
    /*restart period in seconds*/
    u_short restart_period;

    /*reason*/
    u_short restart_reason : 4;

    /* support unplanned restart*/
    u_short restart_enable : 1;       

    /*instance is in restarting*/      
    u_short in_restart : 1;
    
    /* if can be restart hepler*/
    u_short restart_helper : 1;

    /*刚刚完成GR restart的任务，设置该标志用来在计算路由时，检查summary lsa*/
    u_short restart_route_finish : 1;

    /*is lsa checking strictly during restarting*/
    u_short strictlsa_check : 1;

    /*flag indicate some network routes waiting for update*/
    u_short wait_export : 1;
    
#ifdef OSPF_FRR
    
    /*flag indicate some backup routes waiting for update*/
    u_short backup_wait_export : 1;
#endif

    
    /*restrat 退出的原因*/
    u_short restart_exitreason;

    /*NO_RESTARTING, planed, unplaned*/
    u_short restart_status;

    /*last exported route dest*/
    u_int last_export_network;

    /*last exported route mask*/
    u_int last_export_mask;
#ifdef OSPF_FRR
    /*last exported backup route*/
    u_int backup_last_export_network;

    u_int backup_last_export_mask;
#endif
     
    /*last kernal error detected*/        
    u_short kernal_errno;

    /*GTSM checking*/ 
    u_short valid_hops;

    /*nbr count in different state*/
    u_short nbr_count[12];  
	
#ifdef HAVE_BFD
	/*bfd enable used for nbr fast disconnection*/
	u_short bfd_enable;
    
    /*ospf bfd session 接收间隔*/  
    u_long bfd_minrx_interval;
    
    /*ospf bfd session 发送间隔*/
    u_long bfd_mintx_interval;
    
    /*ospf bfd session 倍数*/
   	u_long bfd_detmul;
#endif
	
	/*maximum load-balancing*/
    u_short max_ecmp;

    /*nexthop weight*/
    struct ospf_nhop_weight nhopw[64];      

	/*ospf packet block*/
	u_short packet_block;
	
	/*preference*/
    u_short preference;

    /*ase preference*/
    u_short preference_ase;

    /*policy preference*/
    u_short preference_policy;

    /*policy ase preference*/
    u_short preference_ase_policy;
		
    u_char overlay_update;
	
	u_int uiOverlayNet;

    /*process description*/
	u_char description[OSPF_DESCRIP_LEN];

	/*mpls lsr id advertise flag*/
    u_char mpls_flag;

    /*mpls lsr id */
    u_int mpls_lsrid;

    /*mpls global te state FALSE:disable,TRUE:enable*/
    u_int mpls_te;

    /*mpls global te state FALSE:disable,TRUE:enable*/
    u_int mpls_cost;

    /*asbr range*/
	struct ospf_lst asbr_range_table;

	/*asbr range update timer*/
	struct ospf_timer asbr_range_update_timer; 

	u_int uiDcnIfCnt;
	u_int uiDcnIfErrCnt;
	u_int uiDcnLsaCnt;
	u_int uiDcnLsaErrCnt;
	u_int uiDcnNeidCnt;
	u_int uiDcnNeidErrCnt;

} ;
#define for_each_ospf_process(a, b) for_each_node(&ospf.process_table, (a), (b))

#define ospf_is_vpn_process(p) ((((p)->vrid != 0) && ((p)->vrid != OSPF_INVALID_VRID)) ? TRUE : FALSE)

/*asbr range struct*/
struct ospf_asbr_range{
    /*node in process's range table*/
    struct ospf_lstnode node;

    /*node in global nm access*/
    struct ospf_lstnode nm_node;

    /*process pointer it belong to*/
    struct ospf_process *p_process;

    /*range network*/
    u_int network;

    /*range mask*/
    u_int mask;

    /*dest*/
    u_int dest;

    /*action for lsa matched,TRUE adervertise,FALSE not advertise*/
    u_short advertise;
    
    /*lsa type this range applied,3 or 7*/
    u_int8 lstype;

    /*configured cost of range,must be 32 bit,
      the highest bit indicate path_type--only for type 7
      0 means not configured,in this case,final cost is calculated
      from aggregated route/lsa's cost*/
    u_int cost;
            
    /*range down flag,used for originate summary lsa filter range
      if this flag is set,this range can not used to match a route/lsa*/
    u_int8 isdown : 1;

    /*changed lsa need check range again,may exist loop between lsa chack 
    and range update*/
    u_int8 need_check : 1;

    /*count of matched routes*/
    u_int rtcount;  
          
};


    
/*network command node*/
struct ospf_network{
    /*node in process*/
    struct ospf_lstnode node;
   
    /*node for nm access*/
    struct ospf_lstnode nm_node;
    
    struct ospf_process *p_process;
   
    /*if network*/
    u_int dest;
   
    /*mask*/
    u_int mask;
   
    /*area id which network belong to*/
    u_int area_id;
};

/*flush log of spf*/
#define ospf_spf_log_clear(x) do{\
   struct ospf_spf_loginfo *p_log = NULL;\
   struct ospf_spf_loginfo *p_nlog = NULL;\
   for_each_node(&ospf.log_table.spf_table, p_log, p_nlog){\
     ospf_lstdel_unsort(&ospf.log_table.spf_table, p_log);\
     ospf_mfree(p_log, OSPF_MSTAT);\
   }\
}while(0)

/*flush log of lsa*/
#define ospf_lsa_log_clear(x) do{\
   struct ospf_lsa_loginfo *p_log = NULL;\
   struct ospf_lsa_loginfo *p_nlog = NULL;\
   for_each_node(&ospf.log_table.lsa_table, p_log, p_nlog){\
     ospf_lstdel_unsort(&ospf.log_table.lsa_table, p_log);\
     ospf_mfree(p_log, OSPF_MSTAT);\
   }\
}while(0)

/*flush log of nbr*/
#define ospf_nbr_log_clear(x) do{\
   struct ospf_nbr_loginfo *p_log = NULL;\
   struct ospf_nbr_loginfo *p_nlog = NULL;\
   for_each_node(&ospf.log_table.nbr_table, p_log, p_nlog){\
     ospf_lstdel_unsort(&ospf.log_table.nbr_table, p_log);\
     ospf_mfree(p_log, OSPF_MSTAT);\
   }\
}while(0)

#ifdef HAVE_BFD
/*flush log of bfd*/
#define ospf_bfd_log_clear(x) do{\
   struct ospf_bfd_loginfo *p_log = NULL;\
   struct ospf_bfd_loginfo *p_nlog = NULL;\
   for_each_node(&ospf.log_table.bfd_table, p_log, p_nlog){\
     ospf_lstdel_unsort(&ospf.log_table.bfd_table, p_log);\
     ospf_mfree(p_log, OSPF_MSTAT);\
   }\
}while(0)
#endif

struct ospf_network *ospf_network_add(struct ospf_process *p_process, u_int network,u_int mask,u_int area_id);
struct ospf_network *ospf_network_lookup(struct ospf_process *p_process, u_int dest, u_int mask);
struct ospf_process *ospf_process_lookup(struct ospf_global * p_master, u_int id);
struct ospf_process *ospf_process_lookup_by_id(u_int id);
struct ospf_process *ospf_process_add(struct ospf_global * p_master, u_int id);
struct ospf_te_tunnel *ospf_te_tunnel_lookup(struct ospf_process * p_process, u_int addr_out);

void ospf_network_delete(struct ospf_network * net_cfg);
void ospf_timer_start(struct ospf_timer * p_thread, u_int delay);
void ospf_timer_init(struct ospf_timer * p_thread, void * arg, void * func, void *context);
void ospf_timer_stop(struct ospf_timer * p_thread);
void ospf_process_delete(struct ospf_process *p_process);
void ospf_reboot_event_handler(struct ospf_process *p_process);    
int ospf_network_match_area(struct ospf_process *p_process, u_int area_id);
u_int ospf_timer_remain(struct ospf_timer * p_thread, u_int now);
void ospf_timer_table_init(void);

struct ospf_process *ospf_process_add(struct ospf_global *p_master, u_int id);
int ospf_process_lookup_cmp(struct ospf_process *p1,struct ospf_process *p2);
u_int ospf_next_expired_time(struct ospf_process *p_process);
int ospf_network_nm_cmp(struct ospf_network *p1, struct ospf_network *p2);
int ospf_global_lstable_lookup_cmp(struct ospf_lstable *p1,struct ospf_lstable *p2);
int ospf_te_tunnel_lookup_cmp(struct ospf_te_tunnel * p1, struct ospf_te_tunnel * p2);

void ospf_process_up(struct ospf_process *p_process);
void ospf_area_table_init(struct ospf_process *p_process);
void ospf_te_router_table_init(struct ospf_process *p_process);
void ospf_import_table_init(struct ospf_process *p_process);
void ospf_network_table_init(struct ospf_process *p_process);
void ospf_nexthop_table_init(struct ospf_process * p_process);
void ospf_reboot_event_handler(struct ospf_process * p_process);
void ospf_range_update_timeout(struct ospf_process *p_process);
void ospf_te_tunnel_timer_expired(struct ospf_process *p_process);
int ospf_asbr_range_lookup_cmp(struct ospf_asbr_range *p1,struct ospf_asbr_range *p2);
void ospf_stub_router_timeout(struct ospf_process *p_process);
void ospf_te_tunnel_delete(struct ospf_te_tunnel *p_te_tunnel);



void ospf_main_task(void);
void ospf_thread_timer();
void ospf_update_min_timer();

void ospf_sync_check_timeout(void *arg);



int ospf_data_init();

void ospf_set_mcast_to_cpu(u_int uiEnable);

	

#ifdef __cplusplus
}
#endif 
#endif  
