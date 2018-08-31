 

#if !defined (_OSPF_SYNS_H_)
#define _OSPF_SYNS_H_

#ifdef __cplusplus
extern "C" {
#endif 

/*get workmode interval:5s*/
#define OSPF_WORKMODE_INTERVAL 5
#define OSPF_INIT_SYNC_DELAY_TIME 60
/*local define working mode*/
enum{
    OSPF_MODE_NORMAL,/*there is only one control card*/
    OSPF_MODE_MASTER,/*there are multiple control card,this card is master*/
    OSPF_MODE_SLAVE   /*there are multiple control card,this card is slave*/
};

/*local define lsa scope*/
enum{
    OSPF_LSA_SCOPE_INSTANCE,
    OSPF_LSA_SCOPE_AREA,
    OSPF_LSA_SCOPE_INTERFACE   
};

/*card sync msg node*/
struct ospf_card_sync{
    /*系统号，从1开始*/
    u_int sys_id;           
    
    /*插槽号，从1开始*/
    u_int slot_id;          
    
    /*卡类型,缺省为0*/
    int  type;    
};

/*syn cmd*/
enum{
    OSPF_SYN_TYPE_INSTANCE,
    OSPF_SYN_TYPE_AREA,
    OSPF_SYN_TYPE_IF,
    OSPF_SYN_TYPE_NBR,
    OSPF_SYN_TYPE_LSA,
    OSPF_SYN_TYPE_LSA_FRAGMENT /*20110709 big packet*/,
    OSPF_SYN_TYPE_LSA_RXMT
};

/*common sync msg header*/
struct ospf_syn_hdr{
    /*msg type as above*/
    u_int8 cmd;
   
    /*TRUE:add or update,FALSE:delete*/
    u_int8 add;
   
    /*not include struct ospf_syn_hdr*/
    u_short len; 
   
    /*check syn packet lost*/
    u_int seq_num;
};

#define OSPF_MAX_PROTOCOL 16

/*sync msg for process,most fields copied from ospf_process*/
struct ospf_syn_instance{
    /*internal instance id*/
    u_int process_id;
    
    u_int router_id;   

    /*overflow state and control*/
    u_int overflow_limit; 

    /*overflow state period*/
    u_int overflow_period;

    u_int reference_rate;
    
    /*spf running interval in ms*/
    u_short spf_interval;

    /*restart period in seconds*/
    u_short restart_period;

    u_int8 reditribute_flag[OSPF_MAX_PROTOCOL];

    /*enabled trap type*/
    u_int trap_enable;  /*must be u_int,used in bit*/
    
    /*is self asbr*/
    u_int8 asbr;
    
    /*is self abr*/
    u_int8 abr;
    
    /* TRUE means this router will be opaque-capable for opaque lsas */
    u_int8 opaque;

    /*in rout selection,use rule as rfc1583?*/
    u_int8 rfc1583_compatibility;

    u_int8 tos_support;

    u_int8 dc_support;

    u_int8 mcast_support;

    u_int8 stub_support;
   
    u_int8 stub_adv;
    
    /*reason*/
    u_int8 restart_reason;

    /* support unplanned restart*/
    u_int8 restart_enable;

    /* if can be restart hepler*/
    u_int8 restart_helper;

    u_int8 strictlsa_check;

    u_int8 padding[3];

    /*second*/
    u_int current_time;  
};

/*sync msg for area,most fields copied from ospf_area*/
struct ospf_syn_area{       
    /*area id */
    u_int id;

    /*instance belong to*/
    u_int process_id;

    /*cost for default summary lsa if stub configured,index1~7 is for tos routing*/
    struct ospf_default_cost stub_default_cost[OSPF_MAX_TOS];

    /*area auth type*/
    u_short authtype;

    /*key id,only valid for md5*/
    u_int8 keyid;

    u_int8 keylen; 
    
    /*key string,appended 4bytes*/
    u_int8 key[OSPF_MAX_KEY_LEN + 4];
          
    /*    默认7类LSA的开销值，初始化为0*/
    u_int nssa_default_cost;

    /*    默认7类LSA的开销类型，初始化为0*/
    u_short nssa_cost_type;

    /*translator changed interval*/
    u_short nssa_wait_time;

    /*    TELSA中的实例号，创建TE时自动计算，
    此后将一直保持，直到TE被关闭  */
    u_int te_instance;     
    
    u_int nssa_tag;    

    u_short up_if_count;   /*used to calculate route*/
    /* 表明NSSA转换者角色 
       TRUE NSSA_TRANSLATOR_ALWAYS 1 
       FALSE NSSA_TRANSLATOR_CANDIDATE 0初始化为0*/
    u_int8 nssa_always_translate;
      
    /* 节点在此区域内的转换者状态。
         TRUE 作为转换者FALSE 不作为转换者*/
    u_int8 nssa_translator;  

    /* This is a stub area (no external routes) */
    u_int8 is_stub;                         

    /* This is a not so stubby area */
    u_int8 is_nssa;                         

    /* Inject default into this stub area */
    u_int8 nosummary;        

   /*表明区域的TE功能是否使能，默认为FALSE，
   可以通过配置改为TRUE*/
    u_int8 te_enable;
};

/*sync msg for interface,most fields copied from ospf_if*/
struct ospf_syn_if{
    u_int process_id;
     
    /*ip address of interface*/
    u_int addr;

    /*nbr id,only valid for virtual link*/
    u_int nbrid;

    u_int transit_area_id;

     /*interface type*/
    u_int type;

    /*if_unit from system*/
    u_int ifnet_uint; 

    /* if_index from struct ifnet */
    u_short ifnet_index;   

    /*interface flag,deicide up/down*/
    u_int8 link_up;

    u_int8 rsvd;

    /*mask of interface*/
    u_int mask;

    u_int area_id;

    /*cost[0]used in ospf,cost[1]-cost[6] only save the config of MIB*/
    u_short cost[OSPF_MAX_TOS];   
    
    /*current dr address*/    
    u_int dr;

    /*current bdr address*/
    u_int bdr;

    /*ip mtu of interface*/
    u_short mtu;

    /* If set to 0, be dr other */
    u_short priority;       

    u_short hello_interval;   

    /*retransmit packet interval,for request and dbd*/   
    u_short rxmt_interval;

    /*neighbor dead interval in seconds*/
    u_int dead_interval;  

    u_int poll_interval;      

    /*transmit delay in seconds*/
    u_int8 tx_delay;

    /*interface state*/
    u_int8 state;

    /*auth type used*/
    u_int8 authtype;

    /*key id for md5*/
    u_int8 md5id;

    /*auth key for simple and md5 key*/
    u_int8 key[OSPF_MD5_KEY_LEN + 4];

    /*key length:for simple password, upto 8, for md5 upto 16*/
    u_int8 keylen;

    /*need build network lsa*/    
    /*is passive interface*/    
    u_int8 passive;

    /*mcast lsa support ? default 3*/
    u_int8 mcast;

    /*demand circuit support ? default 3*/    
    u_int8 demand;

    /*indicate if has configured cost on this interface,
    default is 0,0-not config,1-has configured*/    
    u_int8 configcost;

    /*is TE enabled*/
    u_int8 te_enable;

#ifdef HAVE_BFD
    u_int8 bfd_enable;
#endif

    /*use fast dd exchange*/
    u_int8 fast_dd_enable;
    
    /*TE instance id*/
    u_int te_instance;
   
    /*接口的TE开销*/
    u_int te_cost;
   
    /*TE的管理组*/
    u_int te_group;   
   
    /*最大带宽*/
    u_int max_bd;
   
    /*最大可预留带宽*/
    u_int max_rsvdbd;
   
    /*8个级别的未预留带宽*/
    u_int unrsvdbd[OSPF_MAX_BD_CLASS];       

   /*the interface in same foold_group will not flood lsa each other*/
    u_int flood_group;
};

/*sync msg for neighbor,most fields copied from ospf_nbr*/
struct ospf_syn_nbr{
    u_int  process_id;
 
    /*identify for if*/
    u_int iftype;
   
    u_int if_addr;

    u_int if_uint;
    
    u_int transit_area_id;
 
    u_int nbr_id;

    /*neighbor address*/
    u_int addr;

    /*dr from neighbor's hello*/
    u_int dr;

    /*bdr from neighbor's hello*/
    u_int bdr;
    
    /* 0 means not eligible to become the Designated Router */
    u_short priority;  
    
    u_int8 option;
    /*standard state*/
    u_int8  state;

    u_int ulDdMtu;
};

/*sync msg for lsa,containing scope and lsa body*/
struct ospf_syn_lsa{
    /*lsa's process id*/
    u_int process_id;

    /*lsa's area id,only used for area scope lsa*/    
    u_int area_id;

    /*lsa's if address,only used for iface scope lsa*/        
    u_int if_addr;

    /*lsa's transit area id,only used for vif scope lsa*/        
    u_int tansit_area_id;

    /*lsa's nbr id,only used for vif scope lsa*/
    u_int nbr_id;     

    /*interface type for if scope lsa*/
    u_short iftype;

    /*flag indicating if any retransmit not acked*/
    u_short retransmit_flag;
    
    /*contain linkstate buffer*/
    struct ospf_lshdr lshdr[1];
};

/*sync msg including part of lsa,some lsa's length exceed sync MTU,so we do fragment*/
struct ospf_syn_lsa_fragment{
    u_int process_id;    
    
    u_int area_id;    
    
    u_int if_addr;    
    
    u_int tansit_area_id;    
    
    u_int nbr_id;     
    
    u_int8 iftype;    
    
    u_int8 retransmit_flag;

    /*fragment start from 0*/
    u_int8 fragment;

    /*max fragment id*/
    u_int8 fragment_count;

    /*fragment length*/
    u_int buf_len;

    /*lsa's header*/
    struct ospf_lshdr lshdr;

    /*only conttaining lsa body*/
    u_int8 buf[4];
};

/*database list node for fragment lsa*/
struct ospf_lsa_fragment{
    struct ospf_lstnode node;

    struct ospf_process *p_process;
    
   /*socpe field, for area lsa ,contain area pointer,
     for type9 lsa, contain interface pointer;
     for ase and 11 lsa, contain instance pointer
    */
    void *p_scope;

    /*total fragment count*/
    u_int fragment_count;

    /*lsa header*/
    struct ospf_lshdr lshdr;

    /*first 4bytes contain fragment length in host order*/
    struct {
      u_int len;
      
      u_int8 *p_buf;
    }fragment[64];
};

/*sync packet header,one packet containing multuple msg*/
struct ospf_syn_pkt{
    /*check syn packet lost*/
    u_int seqnum;  

    /*used to check ospf pkt*/
    u_int8 cookie[4]; 

    /*start or msg*/
    u_int8 msg[0];
};

/*max buffer length of syn packet*/
#define OSPF_SYN_MAX_TXBUF 1400

/*sync packet header length*/
#define OSPF_SYN_PKY_HDR_LEN 8

/*sync control struct for new inited card,one control struct for each card
   ,and there is a common sync control struct used for all cards*/
struct ospf_syn_control{
    /*list in struct ospf_global*/
    struct ospf_lstnode node; 

    /*index,indicate one card. 0: syn for each card*/
    u_int synflag;  

    /*for initial syn: used to find lstable*/
    u_int process_id; 

    /*for initial syn: used to find lstable*/
    u_int area_id;  

    /*for initial syn: next need syn lsa*/
    u_int8 lshdr[20]; 
    
    /*sync message length*/
    u_short len;    
    
    /*for initial syn lsa*/
    u_short lsa_syn_need;
    
    /*used to send sync message*/
    struct ospf_syn_pkt *p_msg;

    /*containing msg buffer*/
    u_int8 buf[OSPF_SYN_MAX_TXBUF + OSPF_SYN_PKY_HDR_LEN];
};

struct ospf_syn_control *ospf_syn_control_lookup(u_int synflag);
struct ospf_syn_control *ospf_syn_control_create(u_int synflag);
u_int ospf_get_workmode(struct ospf_global * p_ospf,u_long ulWorkMode);
int ospf_lsa_fragment_lookup_cmp(struct ospf_lsa_fragment *p1,struct ospf_lsa_fragment *p2);
int ospf_syn_control_lookup_cmp(struct ospf_syn_control * p1, struct ospf_syn_control * p2);
void ospf_syn_process_send(struct ospf_process * p_process, u_int add,struct ospf_syn_control *p_control);
void ospf_syn_area_send(struct ospf_area * p_area, u_int add,struct ospf_syn_control *p_control);
void ospf_syn_if_send(struct ospf_if * p_if, u_int add,struct ospf_syn_control *p_control);
void ospf_syn_nbr_send(struct ospf_nbr * p_nbr, u_int add,struct ospf_syn_control *p_control);
void ospf_syn_lsa_send(struct ospf_lsa * p_lsa, u_int add,struct ospf_syn_control *p_control);
void ospf_syn_check_event(struct ospf_process * p_process); 
void ospf_syn_init_send(struct ospf_syn_control *p_control);
void ospf_syn_send(struct ospf_syn_control *p_control);
void ospf_syn_lsa_table_send_all(struct ospf_syn_control *p_control);
void ospf_syn_lsa_rxmt_send(struct ospf_lsa *p_lsa, u_int add, struct ospf_syn_control *p_control);
void ospf_lsa_fragment_delete(struct ospf_lsa_fragment *p_fragment);
void ospf_syn_control_delete(struct ospf_syn_control * p_control);
void ospf_syn_hdr_print(struct ospf_syn_hdr * p_hdr);
void ospf_syn_wokemode(void);

void ospf_syn_nbr_print(struct ospf_syn_nbr* p_nbr);
#ifdef OSPF_MASTER_SLAVE_SYNC
void ospf_slaver_card_up();
#endif

#ifdef __cplusplus
}
#endif 
#endif  
