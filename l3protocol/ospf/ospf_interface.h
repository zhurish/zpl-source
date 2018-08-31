/* ospf_iface.h  ospf interface*/

#if !defined (_OSPF_IF_H_)
#define _OSPF_IF_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "ospf_main.h"

/*record ism output string*/
#define ospf_log_ism(ifp, state, event) do\
{ \
   u_int8 *s_state[8] = {" ", "down", "loopback", "waiting", "PPP", "DR", "BDR", "DRother"};\
   u_int8 *s_event[7] = {"up", "waitTimer", "BackSeen", "NbrChange", "Loop", "Unloop", "Down"};\
   ospf_logx(ospf_debug, "if-fsm, if=%s,state=%s,event=%s", (ifp)->name, s_state[state],s_event[event]);\
}while(0)


/*set ifmask when interface is added*/
#define ospf_record_mask(ins, mask) do{\
    int i;\
    for (i = 0 ; i < 32; i++)\
    {\
        if ((ins)->ifmask[i] == 0)\
        {\
            (ins)->ifmask[i] = mask;\
            break;\
        }\
        else if ((ins)->ifmask[i] == mask)\
         break;\
    }\
}while(0)

/*ISM event define according to RFC2328*/
enum{
    OSPF_IFE_UP = 0x00,
    OSPF_IFE_WAIT_TIMER,
    OSPF_IFE_BACKUP_SEEN,
    OSPF_IFE_NBR_CHANGE,
    OSPF_IFE_LOOP,
    OSPF_IFE_UNLOOP,
    OSPF_IFE_DOWN  
};

enum{
    OSPF_MCAST_ALLSPF = 0,
    OSPF_MCAST_ALLDR,
    OSPF_MCAST_ALLLOOP
};


/*internal error*/
enum {
    OSPF_IFERR_SOURCE = 0,/*收到的源地址网段出错*/
    OSPF_IFERR_NONBR,/*收到的无法确定邻居的消息数目*/    
    OSPF_IFERR_AUTHTYPE,/*认证类型不匹配*/
    OSPF_IFERR_AUTH,/*认证错误消息数目*/
    OSPF_IFERR_MASK,/*掩码不匹配消息数目*/
    OSPF_IFERR_HELLOTIME,/*Hello间隔不匹配数目*/
    OSPF_IFERR_DEADTIME,/*邻居保持时间不匹配数目*/
    OSPF_IFERR_EBIT,/*E比特不匹配数目*/
    OSPF_IFERR_NBIT,/*NSSA不匹配数目*/
    OSPF_IFERR_MTU,/*MTU不匹配次数*/
    OSPF_IFERR_OWN,/*相同的router-id*/
    OSPF_IFERR_VERSION,/*错误的版本号*/
    OSPF_IFERR_DD_NBR,/*DD无效的邻居状态*/
    OSPF_IFERR_REQ_NBR,/*REQ无效的邻居状态*/
    OSPF_IFERR_REQ_INV,/*REQ无效的LSA*/
    OSPF_IFERR_REQ_EMP,/*REQ为空*/
    OSPF_IFERR_UPD_NBR,/*UPD无效的邻居状态*/
    OSPF_IFERR_ACK_NBR,/*ACK无效的邻居状态*/
    OSPF_IFERR_MAX
};

/*ack information used for ack msg tx*/
struct ospf_ackinfo{
    /*interface belong to*/
    struct ospf_if *p_if;
    
    /*message buffer*/
    struct ospf_hdr *p_msg;
};

/*statistics of interface*/
struct ospf_if_stat{
    /*interface statistic to support better debuging*/    
    /*total received packet,one for each type*/    
    u_int rx_packet[OSPF_PACKET_ACK + 1];

    /*total tx packet,one for each type*/
    u_int tx_packet[OSPF_PACKET_ACK + 1];

    /*total received byte,one for each type*/  
    u_int rx_byte[OSPF_PACKET_ACK + 1];

    /*total tx byte,one for each type*/
    u_int tx_byte[OSPF_PACKET_ACK + 1];

    /*total lsa information in rxd packet,no means for hello*/
    u_int rx_lsainfo[OSPF_PACKET_ACK + 1];

    /*total lsa information in txd packet,no means for hello*/
    u_int tx_lsainfo[OSPF_PACKET_ACK + 1];

    /*nbr event on this interface*/
    u_int nbr_event[13];

    /*each if event count*/
    u_int if_event[OSPF_IFE_DOWN+1];

    /*processing error*/
    u_int error[OSPF_IFERR_MAX];
    
    /*related error data*/
    u_int error_data[OSPF_IFERR_MAX];

    /* Number of state changes */
    u_int state_change; 

    /*init dd msg retransmit count*/
    u_int init_dd_rxmt;

    /*normal dd msg retransmit count*/
    u_int dd_rxmt;

    /*multicast update msg send count*/
    u_int mcast_update;

    /*unicast update msg send count*/
    u_int ucast_update;

    /*request msg send count*/
    u_int req_rxmt;

    /*total rxd packet,dest is multicast address*/
    u_int rx_muti_pkt;

    /*total rxd packet,dest is unicast address*/
    u_int rx_unicast_pkt;

    /*total txd packet,dest is multicast address*/   
    u_int tx_muti_pkt;

    /*total txd packet,dest is unicast address*/
    u_int tx_unicast_pkt;    

    /*total mucast jion error count*/
    u_int mcast_jion_error[OSPF_MCAST_ALLLOOP+1];

    /*total mucast jion success count*/
    u_int mcast_jion_success[OSPF_MCAST_ALLLOOP+1];

    /*total mucast drop error count*/
    u_int mcast_drop_error[OSPF_MCAST_ALLLOOP+1];

    /*total mucast drop success count*/
    u_int mcast_drop_success[OSPF_MCAST_ALLLOOP+1];

    u_int linkup_msg_cnt;
    char linkup_msg_time[20];

    u_int linkdown_msg_cnt;
    char linkdown_msg_time[20];

    u_int ldp_msg_cnt[3];
    char ldp_err_msg_time[20];
    char ldp_init_msg_time[20];
    char ldp_up_msg_time[20];

    u_int bfd_error_cnt;
    u_int bfd_recv_msg_cnt;
    u_int bfd_send_msg_cnt[3];
};

/*bindwidth class type,according to OSPF TE*/
#define OSPF_MAX_BD_CLASS 8

#define OSPF_MAX_IFNAME_LEN 84

/*interface struct*/
struct ospf_if{
    /*node in index address*/
    struct ospf_lstnode list_node;

    /*node in different type interface list*/    
    struct ospf_lstnode type_node;

    /*node in index struct ospf_global,only used for real if*/    
    struct ospf_lstnode global_node;

    /*nm access*/
    struct ospf_lstnode nm_node;
    
    /*node in area*/
    struct ospf_lstnode area_node;
    
    /*process it belong to*/
    struct ospf_process *p_process;

    /*if_unit from system*/
    u_int ifnet_uint; 
    
    /*if_index from system*/
    u_int ifnet_index;
    /*u_short ifnet_index;   */

    /*rfc6549 muti instance*/
    u_short instance;
    
    u_short resved;

    u_int ulDcnflag;
	
	/*overlay module flag*/
    u_int uiOverlayflag;
	
    /*ip address of interface*/
    u_int addr;

    /*mask of interface*/
    u_int mask;

    /*name of interface,for normal interface, name is obtained from system
      for virtual interface,name is constructed from area-nbr
     */
    u_int8 name[OSPF_MAX_IFNAME_LEN];
    
    /* The area this interface is in,it is NULL when interface 
       created,interface can not work until area is set*/
    struct ospf_area *p_area;  

    /*transit area for virtual interface*/
    struct ospf_area *p_transit_area;

    /*only valid for virtual link and shamlink,*/
    /*20120724 shamlink*/
    /*for virtual link is nbr addr,for shamlink is nbr addr */
    u_int nbr;    
 
    /*ip mtu of interface,can be set according to NM*/
    u_short mtu;

    /*max length of packet txd on interface,calculated
     from interface's mtu and auth-key,
     if send pkt exceed maxlen,will send directly*/
    u_short maxlen;
    
    /*neighbor table on this interface*/
    struct ospf_lst nbr_table;

    /*opaque lsa table for type 9*/
    struct ospf_lstable opaque_lstable;
    
    /*20100910*/
    /*cost[0]used in ospf,cost[1]-cost[6] only save the config of MIB*/
    u_short cost[OSPF_MAX_TOS];   
    
    /*current dr address*/    
    u_int dr;

    /*current bdr address*/
    u_int bdr;

    /*hello msg sending interval,default 10s*/
    u_short hello_interval;     
    
    /*retransmit packet interval,for request and dbd*/   
    u_short rxmt_interval;
    
    /*neighbor dead interval in seconds,default 40s*/
    u_int dead_interval;  
    
    /*poll interval for NBMA interface,not used in most condition,default 120s*/
    u_int poll_interval;      
    
    /*hello timer, send hello when expired*/
    struct ospf_timer hello_timer;  

    /*poll timer,used in NBMA*/
    struct ospf_timer poll_timer;

    /*timer for waiting state, do DR election when expired*/
    struct ospf_timer wait_timer;

    /*delay ack timer*/
    struct ospf_timer ack_timer;

    /*lsa flood control timer*/
    struct ospf_timer flood_timer;

    /*ldp-sync hold-down timer*/
    struct ospf_timer hold_down_timer;

    /*ldp-sync  hold-max-cost timer*/
    struct ospf_timer hold_cost_timer;

    /*direct ack, use a tmp var to reduce malloc, 
     used point for function interface, */
    struct ospf_ackinfo *p_direct_ack;
    
    /*send delayed ack when ack_timer expired*/
    struct ospf_ackinfo delay_ack;  
    
    /*flooding buffer,just containg one update packet*/
    struct ospf_updateinfo update;
    
    /*DR priority for bcast and NBMA interface, if set to 0, be dr other */
    u_int8 priority; 

    /*interface type:ppp,bcast,nbma,p2mp,vlink,shamlink*/
    u_int8 type : 4;

    /*interface state:down,waiting,p2p,dr,bdr,dr_other*/
    u_int8 state : 4;

    /*auth type used:md5,simple,none*/
    u_int8 authtype : 3;

    /*auth display type used:cipher,plain,none*/
    u_int8 authdis;


    /*key length:for simple password, upto 8, for md5 upto 16*/
    u_int8 keylen : 5;

    /*key id for md5*/
    u_int8 md5id;

    /*auth key for simple and md5 key,4bytes padding not used now*/
    u_int8 key[OSPF_MD5_KEY_LEN + 4];

    /*cipher auth key for simple and md5 key,4bytes padding not used now*/
    u_int8 cipher_key[OSPF_MD5_KEY_LEN + 4];

    /*the interface in same flood_group will not flood lsa each other*/
    u_int flood_group;

    /*transmit delay in seconds*/
    u_short tx_delay;
    
    /*neighbor changed on this interface,use to schedule 
      interface event IFNBRCHANGE*/
    u_short nbrchange : 1;

    /*is passive interface:do not tx&rx any packet on this interface*/    
    u_short passive : 1;

    /*mcast lsa support ? default 3,only for NM*/
    u_short mcast : 1;

    /*demand circuit support ? default 3,only for NM*/    
    u_short demand : 1;

    /*indicate if has configured cost on this interface,
    default is 0,0-not config,1-has configured*/    
    u_short configcost : 1;

    /*if set TRUE,all packets are sent to mcast address*/
    u_short mcast_always : 1;

    /*2008.1.9 zm add for unicast hello*/
    /*if set TRUE,interface will send unicast hello on interface after neighbor learnt*/
    u_short unicast_hello : 1;

    /*is TE enabled*/
    u_short te_enable : 1;

#ifdef HAVE_BFD
    /*bfd enable used for nbr fast disconnection*/
    u_short bfd_enable;
    
    /*ospf bfd session 接收间隔*/  
    u_long ulRxMinInterval;
    
    /*ospf bfd session 发送间隔*/
    u_long ulTxMinInterval;
    
    /*ospf bfd session 倍数*/
   	u_long ulDetMulti;
#endif
    /*ospf ldp-sync 使能*/
    u_int8 ucLdpSyncEn : 1;

    /*ospf ldp-sync hold-max-cost状态,当True:需发送max cost    hold down超时时配置True，hlod cost定时器启动时配置FALSE*/
    u_int8 ucHoldCostState : 1;

   	/*ospf ldp-sync hold-down 时间*/
   	u_long ulHoldDownInterval;

   	/*ospf ldp-sync hold-max-cost 时间*/
   	u_long ulHoldCostInterval;

    /*use fast dd exchange*/
    u_short fast_dd_enable : 1;

    /*used for MASTER,need syn to SLAVE*/
    u_short syn_flag : 1;

    /*used for rcv DD packet ignore mtu check*/
    u_short mtu_ignore : 1;

    /*if interface's link opertaional state is up? TRUE or FALSE,read from system*/
    u_short link_up : 1;
    
    /*TE instance id,used for originate te lsa lsid*/
    u_int te_instance;
   
    /*    表明接口的TE开销*/
    u_int te_cost;
   
    /*TE的管理组*/
    u_int te_group;   
   
    /*    最大带宽*/
    u_int max_bd;
   
    /*    最大可预留带宽*/
    u_int max_rsvdbd;
   
    /*8个级别的未预留带宽*/
    u_int unrsvdbd[OSPF_MAX_BD_CLASS];    

    /*interface statistics*/   
    struct ospf_if_stat *stat;
	
	/*interface cost status*/
    u_int costflag:1;

    /*interface ospf sync state*/
    u_long ulOspfSyncState;

    /*interface mpls ldp state*/
    u_long ulLdpState;

	/*p_if state*/
    u_int pif_enable;
};

/*start ack timer*/
#define ospf_delay_ack_timer_start(ifp) do{\
    if (10000 <= ospf_lstcnt(&(ifp)->p_process->t5_lstable.list))\
        ospf_timer_try_start(&(ifp)->ack_timer, 0);\
    else\
        ospf_timer_try_start(&(ifp)->ack_timer, OSPF_DELAY_ACK_INTERVAL);\
	}while(0)
	
/*macro for scan neighbor on interface*/
#define for_each_ospf_nbr(pif,nbr,nxt) for_each_node(&(pif)->nbr_table, nbr, nxt)

#define ospf_nbr_first(pif) (ospf_lstfirst(&(pif)->nbr_table))

/*scan interface*/
#define for_each_ospf_if(ins, pif, pn)  for_each_node(&((ins)->if_table), pif, pn)

#define ospf_if_mtu_len(p_if) ((OSPF_IFT_VLINK == (p_if)->type) ? OSPF_DEFAULT_IP_MTU : (p_if)->mtu)

/*for a given interface,get packet max length, exlcude ip part and auth part*/
/*for testcenter,ifmtu lenth include MacHdrLen,for safe not -18 */
#define ospf_if_max_len(x) ((ospf_if_mtu_len (x) - 20 - ospf_if_auth_len (x) -28))

struct ospf_if *ospf_real_if_create(struct ospf_process * p_process, u_int ifaddr, struct ospf_area * p_area);
struct ospf_if *ospf_real_if_create_by_ifindex(struct ospf_process *p_process, u_int uiIfIndex, struct ospf_area *p_area); 
struct ospf_if *ospf_virtual_if_create(struct ospf_process * p_process, u_int nbr_id, struct ospf_area * p_area);
struct ospf_if *ospf_shamlink_if_create(struct ospf_process * p_process, u_int if_addr, u_int nbr_addr);
struct ospf_if *ospf_if_lookup_by_network(struct ospf_process *p_process, u_int source) ;
#ifdef OSPF_DCN
struct ospf_if *ospf_if_lookup_unnumber(struct ospf_process * p_process, u_int addr, u_int unit);
struct ospf_if *ospf_if_lookup_dcn(struct ospf_process *p_process,u_int addr, u_int unit);
struct ospf_if *ospf_dcn_real_if_create(
                     struct ospf_process *p_process,
                     u_int unit,
                     u_int ifaddr,
                     u_int ifMask,
                     struct ospf_area *p_area);
struct ospf_if *ospf_unnumber_if_create(
                     struct ospf_process *p_process,
                     u_int ifunit,
                     struct ospf_area *p_area);
#endif
struct ospf_if *ospf_if_lookup(struct ospf_process * p_process, u_int addr);
struct ospf_if *ospf_vif_lookup(struct ospf_process * p_process, u_int area_id, u_int nbr_id);
struct ospf_if *ospf_shamlinkif_lookup(struct ospf_process * p_process, u_int if_addr, u_int nbr_addr);
#if OSPF_TEMP
void ospf_if_addr_change(u_int vrid, u_int if_addr, int msg_type, u_int mask);
#else
void ospf_if_addr_change(u_int vrid, u_int if_addr, int add, u_int ulIfIndex);

#endif
void ospf_if_table_init(struct ospf_process *p_process);
void ospf_if_state_update(struct ospf_if *p_if);
void ospf_ism(struct ospf_if * p_if, u_int event);
void ospf_vif_state_update (struct ospf_if *p_if);
void ospf_if_cost_update(struct ospf_if * p_if, u_int tos, u_int metrics);
void ospf_ism_wait_timer(struct ospf_if * p_if);
void ospf_ism_down(struct ospf_if * p_if);
void ospf_if_delete(struct ospf_if * p_if);
void ospf_if_hello_timeout(struct ospf_if *p_if);
void ospf_if_flood_timeout(struct ospf_if *p_if);
u_int ospf_if_auth_len(struct ospf_if * p_if);
int ospf_if_lookup_cmp(struct ospf_if * p_if1, struct ospf_if * p_if2);
int ospf_real_if_lookup_cmp(struct ospf_if *p1,struct ospf_if *p2);
int ospf_if_nm_cmp(struct ospf_if *p1, struct ospf_if *p2);
int ospf_vif_nm_cmp(struct ospf_if *p1, struct ospf_if *p2);
int ospf_shamlink_nm_cmp(struct ospf_if * p1, struct ospf_if * p2);
u_short ospf_if_cost_calculate(u_int rate, struct ospf_if *p_if);
void ospf_if_mcast_group_set(struct ospf_if * p_if, u_int join_flag);
void ospf_elect_bdr (struct ospf_if *p_if);
void ospf_elect_dr (struct ospf_if *p_if);
void ospf_dr_changed( struct ospf_if *p_if, u_int old_state, u_int old_dr_addr);
void ospf_nbr_table_init(struct ospf_if *p_if);
void ospf_if_hello_timeout(struct ospf_if *p_if);
void ospf_if_poll_timeout(struct ospf_if *p_if);
void ospf_if_send_delayack(struct ospf_if *p_if);
void ospf_if_wait_timeout(struct ospf_if *p_if);
void ospf_if_flood_timeout(struct ospf_if *p_if);
void ospf_if_set_packet_block(struct ospf_process *p_process,u_int uiValue);
void ospf_if_hold_cost_timeout(struct ospf_if * p_if);
void ospf_if_hold_cost_start(struct ospf_if *p_if, u_int ldpMsgType);
void ospf_if_hold_cost_stop(struct ospf_if *p_if);
void ospf_if_hold_down_timeout(struct ospf_if * p_if);
void ospf_if_hold_down_start(struct ospf_if *p_if);
void ospf_if_hold_down_stop(struct ospf_if *p_if);
struct ospf_if  *ospf_if_create(struct ospf_process *p_process);
struct ospf_if *ospf_if_lookup_by_ifindex(struct ospf_process *p_process, ifindex_t uiIfIndex);

struct ospf_process *ospf_process_lookup_by_ifindex(ifindex_t ifindex);


#ifdef __cplusplus
}
#endif

#endif  

