/* ospf_nbr.h */

#if !defined (_OSPF_NBR_H_)
#define _OSPF_NBR_H_

#ifdef __cplusplus
extern "C" {
#endif 
#include "ospf.h"
/*NSM event according to RFC4750*/
enum{
    OSPF_NE_HELLO = 0,
    OSPF_NE_START,
    OSPF_NE_2WAY,
    OSPF_NE_NEGDONE,
    OSPF_NE_EXCHANGE_DONE,
    OSPF_NE_BAD_REQUEST,
    OSPF_NE_LOADDONE,
    OSPF_NE_ADJOK,
    OSPF_NE_SEQ_MISMATCH,
    OSPF_NE_1WAY,
    OSPF_NE_KILL,
    OSPF_NE_INACTIVITY_TIMER,
    OSPF_NE_LL_DOWN
};

/*exchange state*/
enum{
    OSPF_CLEAR_MODE = 0,
    OSPF_SLAVE,/*self is slave*/
    OSPF_MASTER,/*self is master*/
    OSPF_SLAVE_HOLD/* holding the last database summary delay */
};

/*to limit init DD pkt count */
#define OSPF_MAX_TWOWAY_NBR_COUNT 64

/*max count of nbr in exstart+exchange+loading state in a single process*/
#define OSPF_MAX_EXCHANGE_NBR 16/*8*/

/*max count of nbr in exstart+exchange+loading state in all process*/
#define OSPF_MAX_GLOBAL_EXSTART_NBR 256

/*max count of nbr in exchange+loading state in all process*/
#define OSPF_MAX_GLOBAL_EXCHANGE_NBR 24

#define OSPF_LSA_ERROR_MAX_CNT 100

/*neighbor struct*/
struct ospf_nbr{
    /*node in interface's nbr list,key is address*/
    struct ospf_lstnode node;

    /*node in instance's nbr list,key is local index*/
    struct ospf_lstnode index_node;

    /*node for nm access*/
    struct ospf_lstnode nm_node;
    
    /*interface which this neighbor belong to*/
    struct ospf_if *p_if;

    /*neighbor router id*/ 
    u_int id;

    /*unique internal index,used in retransmit operation*/ 
    u_int index;

    /*neighbor address*/
    u_int addr;

    /*standard state*/
    u_int8 state;

    /* Master or slave mode in db exchange*/
    u_int8 dd_state;   
    
    /* 0 means not eligible to become the Designated Router */
    u_short priority;  

    /*dr from neighbor's hello*/
    u_int dr;

    /*bdr from neighbor's hello*/
    u_int bdr;

    /*dd packet sequnce number*/
    int dd_seqnum;

    /* Used when Cryptographic Authentication is employed,one for each type of packet
       ,seqnum must increase for each type of packet in this case
    */
    u_int auth_seqnum[OSPF_PACKET_ACK + 1];  

    /*time when bdr state translations to full*/
    u_int full_time;

    /*dd retransmit timer*/
    struct ospf_timer dd_timer; 

    /*request retransmit timer*/
    struct ospf_timer request_timer; 

    /*lsa retransmit timer*/
    struct ospf_timer lsrxmt_timer; 
    
    /*hold/inactivity timer*/
    struct ospf_timer hold_timer; 
    
    /*slave hold timer*/
    struct ospf_timer slavehold_timer; 

    /*restart timer*/ 
    struct ospf_timer restart_timer; 

    /*lsa_error_cnt statistic time :1s*/ 
    struct ospf_timer lsa_error_timer; 

    /*in this period,reject establish nbr*/
    struct ospf_timer reject_nbr_timer;

    /*bfd senssion bind faied, retry*/
    struct ospf_timer bfd_timer;

    /*used for test center:self exit 3600 age lsa,rcv less recnet lsa cnt*/
    u_int lsa_error_cnt;
    
    /*pointer to last sent dd*/ 
    struct ospf_dd_msg *p_last_dd;

    /*store next lshdr to be sent in dd msg.lshdr length is 20*/
    u_int8 next_dd_hdr[20];
    
    /*last dd packet len,used to send packet*/
    u_short last_dd_len;

    /* Number of state changes */
    u_short events;  

    /*rest database node of nbr*/
    u_int dd_count;

    /*local I/M/MS bit flag during dd exchange*/ 
    u_int8 dd_flag;

    /*dd option copied from dd packet,used to check if nbr's option changed during 
       dd exchange*/
    u_int8 option;

    /*neighbor opaque capability from dd or hello*/        
    u_short opaque_enable : 1;

    /*is neighbor in graceful restarting*/ 
    u_short in_restart : 1;

    /*flag indicate if master recieve ack from slave for the end summary packet*/ 
    u_short empty_dd_need : 1;

    /*flag indicate if slave recieve summary with M=0 from master */ 
    u_short empty_dd_rcvd : 1;

    /*添加请求节点，重传节点时， 如果失败，则此ClearFlag为TRUE
      邻居超时检查，如果ClearFlag为TRUE，则视为超时*/
    u_short force_down : 1;      

    /*used for MASTER,need syn to SLAVE*/
    u_short syn_flag : 1;

    /*used in too many nbr,there exsit too many rxmt, delay rxmt is better*/
    u_short rxmt_timer_delay : 1;

    /*count of lsa will be retransmitted to this nbr*/
    u_int rxmt_count;

    /*count of lsa will be requested to this nbr*/
    u_int req_count;

    /*lsa count in last retransmit*/
    u_short lsa_rxmted; 

    /*acked lsa count for last retransmit*/
    u_short lsa_acked;

    /*lsa count in last request to nbr*/
    u_short lsa_requested;

    /*lsa replied from nbr as response of request*/
    u_short lsa_reply;

    /*record last restart finish reason*/  
    u_int restart_exitreason;

    /*bfd notify callback*/
    int bfd_discribe;

    /*record the last time rcv pkt,invoid start timer too many*/
    u_int rcv_pkt_time;
    u_int ulDdMtu;
};

/*clear requested lsa about special nbr*/         
#define ospf_nbr_request_list_flush(p_n) do\
{\
    struct ospf_request_node *p_req = NULL;\
    struct ospf_request_node *p_next = NULL;\
    for_each_node(&(p_n)->p_if->p_process->req_table, p_req, p_next) {\
        if (p_req->p_nbr == (p_n)) ospf_lstdel_free(&(p_n)->p_if->p_process->req_table, p_req, OSPF_MREQUEST);\
    }\
    (p_n)->req_count = 0;\
}while(0)

/*delete last send database packet of nbr*/
#define ospf_delete_last_send_dd(n) do{\
    if ((n)->p_last_dd)\
       ospf_mfree((n)->p_last_dd, OSPF_MDBD);\
    (n)->p_last_dd = NULL;\
  }while(0)

/*delete all dd for neighbor*/
void ospf_nbr_state_update_timeout(void) ;
void ospf_nbr_dd_list_flush(struct ospf_nbr * p_nbr);
void ospf_nbr_dd_delete (struct ospf_nbr *p_nbr, struct ospf_retransmit *p_rxmt);
void ospf_nbr_inactive_timer(struct ospf_nbr * p_nbr);
void ospf_nbr_lsa_flush(struct ospf_process *p_process);
void ospf_nbr_rxmt_delete(struct ospf_nbr * p_nbr, struct ospf_retransmit * p_rxmt);
void ospf_nsm(struct ospf_nbr * p_nbr, u_int event);
void ospf_request_add(struct ospf_nbr * p_nbr, struct ospf_lshdr * p_lshdr);
void ospf_request_delete(struct ospf_request_node * p_req);
void ospf_nbr_delete(struct ospf_nbr * p_nbr);
void ospf_process_nbr_table_init(struct ospf_process *p_process);
void ospf_nbr_bfd_timeout(struct ospf_nbr *p_nbr);
u_int ospf_nbr_count_in_state(struct ospf_process *p_process, u_int8 *state_mask);
u_int ospf_if_nbr_count_in_state(struct ospf_if * p_check_if, u_int8 * state_mask);
u_int ospf_area_full_nbr_exist(struct ospf_area *p_area);
int ospf_request_lookup_cmp(struct ospf_request_node * p_hdr1, struct ospf_request_node * p_hdr2);
int ospf_nbr_nm_cmp(struct ospf_nbr * p1, struct ospf_nbr * p2);
struct ospf_nbr *ospf_nbr_create(struct ospf_if *p_if, u_int source);
struct ospf_nbr *ospf_nbr_lookup(struct ospf_if *p_if, u_int ip_address);
struct ospf_retransmit *ospf_nbr_rxmt_add(struct ospf_nbr * p_nbr, struct ospf_lsa * p_lsdb);
struct ospf_retransmit *ospf_nbr_dd_add (struct ospf_nbr *p_nbr,struct ospf_lsa *p_lsa);
struct ospf_request_node *ospf_request_lookup(struct ospf_lstable * p_table, struct ospf_lshdr * p_lshdr);

u_int ospf_vif_nbr_state_get(struct ospf_if *p_if, u_int nbr, u_int *lval);


#ifdef __cplusplus
}
#endif

#endif  

