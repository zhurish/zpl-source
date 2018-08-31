/* ospf_area.h */

#if !defined (_OSPF_AREA_H_)
#define _OSPF_AREA_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "ospf_lsa.h"
/*backbone area id*/
#define OSPF_BACKBONE 0x00000000L

/*different length of key*/

/*simple password length*/
#define OSPF_KEY_LEN 8

/*md5 key length*/
#define OSPF_MD5_KEY_LEN 16

/*max key length stored in area or interface*/
#define OSPF_MAX_KEY_LEN OSPF_MD5_KEY_LEN

#define AREACOUNTMAX 512

/* list of networks associated with an area (section 6 of OSPF 
   Version 2 specification, dated June 1995) */
struct ospf_range{
    /*node in area's range table*/
    struct ospf_lstnode node;

    /*node in global nm access*/
    struct ospf_lstnode nm_node;

    /*area pointer it belong to*/
    struct ospf_area *p_area;

    /*range network*/
    u_int network;

    /*range mask*/
    u_int mask;

    /*configured cost of range,must be 32 bit,
      the highest bit indicate path_type--only for type 7
      0 means not configured,in this case,final cost is calculated
      from aggregated route/lsa's cost*/
    u_int cost;

    /*action for lsa matched,TRUE adervertise,FALSE not advertise*/
    u_short advertise;
    
    /*lsa type this range applied,3 or 7*/
    u_int8 lstype;
            
    /*range down flag,used for originate summary lsa filter range
      if this flag is set,this range can not used to match a route/lsa*/
    u_int8 isdown : 1;

    /*changed lsa need check range again,may exist loop between lsa chack 
    and range update*/
    u_int8 need_check : 1;

    /*count of matched routes*/
    u_int rtcount;       
};

/*area's tos cost for default summary lsa in stub/nssa area
  only tos 0 is used in fact,all other tos costs are used only for NM access*/
struct ospf_default_cost{
    /*cost of default lsa*/
    u_short cost;

    /*path type:intra/external1/external2, only for nm*/
    u_int8 type;

    /*set according to NM operation,value is same as RowStatus*/
    u_int8 status;
};

/*max tos currently supported*/
#define OSPF_MAX_TOS 8

/*area struct*/
struct ospf_area{
    /*node in process's area table*/
    struct ospf_lstnode node;

    /*node for nm access*/
    struct ospf_lstnode nm_node; 
    
    /*area id */
    u_int id;
    
    /*process pointer belong to*/
    struct ospf_process *p_process;
    
    /* area address ranges table,containing struct ospf_range*/
    struct ospf_lst range_table;

     /*spf tree:index:id+type*/
    struct ospf_lst spf_table;

     /*spf tree candidate:index:cost+id+type*/
    struct ospf_lst candidate_table; 
     
#ifdef OSPF_FRR/*frr test*/
    /*backup spf table for frr,no candidate need in frr*/
    struct ospf_lst backup_spf_table;
    
    /*backup abr route table*/
    struct ospf_lst backup_abr_table;

    /*backup asbr route table*/
    struct ospf_lst backup_asbr_table;
#endif

    /*abr route,asbr route*/
    struct ospf_lst abr_table;
    
    struct ospf_lst asbr_table;

    /*interface in this area,interface is added to 
      this table after setting area id for it,
      include all types of interface*/
    struct ospf_lst if_table;
            
    /*lsa table for each type in area scope*/ 
    struct ospf_lstable *ls_table[OSPF_LS_TYPE_10 + 1];

    /*asbr route count in area*/
    u_short asbr_count;

    /*abr route count in area*/
    u_short abr_count;

    /*cost for default summary lsa if stub configured,
      index1~7 is for tos routing*/
    struct ospf_default_cost stub_default_cost[OSPF_MAX_TOS];

    /*area auth type*/
    u_short authtype;

    /*area auth display*/
    u_short authdis;

    /*key id,only valid for md5*/
    u_int8 keyid;

    /*length of key*/
    u_int8 keylen; 
    
    /*key string,appended 4bytes,no used in fact*/
    u_int8 key[OSPF_MAX_KEY_LEN + 4];

    /*cipher key string*/
    u_int8 cipher_key[OSPF_MAX_KEY_LEN + 4];

    /*nssa translator waiting timer*/
    struct ospf_timer nssa_timer;

    /*area's delete schedule timer,started when area is to be deleted*/
    struct ospf_timer delete_timer;        

    /*range timer for transit area, area 0 range not arregate in transit area*/
    struct ospf_timer transit_range_timer;  
    
    /*    默认7类LSA的开销值，初始化为0*/
    u_int nssa_default_cost;

    /*    默认7类LSA的开销类型，初始化为0*/
    u_short nssa_cost_type;

    /*translator changed interval*/
    u_short nssa_wait_time;
             
    /*NSSA转换者角色 
      TRUE NSSA_TRANSLATOR_ALWAYS 1 
      FALSE NSSA_TRANSLATOR_CANDIDATE 0初始化为0*/
    u_int nssa_always_translate : 16;
      
    /*节点在此区域内的转换者状态。
      TRUE 作为转换者FALSE 不作为转换者*/
    u_int nssa_translator : 16;  

    /*nssa lsa's tag,not used now,only NM supported*/
    u_int nssa_tag;
    
    /*statistics*/
    /* times spf has been run for this area */
    u_int spf_run; 

    /*mib object*/ 
    /*sum of lsa's checksum in this area,
      do not include opaque 10 lsa*/
    u_int cksum; 

    /*sum of opaque 10 lsa's checksum in this area*/
    u_int t10_cksum; 

    /*total count of lsa in this area,
      do not include opaque 10 lsa*/
    u_int lscount; 

    /*total count of opaque 10 lsa*/
    u_int t10_lscount; 

    /*last originate router lsa time,used for limit 
      router lsa's originate speed*/
    u_int last_routelsa_time;

    /*    TELSA中的实例号，创建TE时自动计算，
    此后将一直保持，直到TE被关闭  */
    u_int te_instance;

    /* This is a transit area,decide during spf calculate,
      if any of router lsa has virtual flag set,
      set area's transit flag*/
    u_int transit:1;                      

    /* This is a stub area (no external routes):set from NM*/
    u_int is_stub:1;                         

    /* This is a not so stubby area:set from NM,
       an area can not be both stub and nssa area*/
    u_int is_nssa:1;                         

    /* Inject default into this stub/nssa area:set from NM*/
    u_int nosummary:1;        
    
    /*表明区域的TE功能是否使能，默认为FALSE，
     可以通过配置改为TRUE*/
    u_int te_enable : 1;

   /*used for MASTER,need syn to SLAVE*/
    u_int syn_flag : 1;

    /*used for vlink area display flag*/
    u_int Vlinkcfg : 1;

    u_int VlinkAreaDis : 1;
    /*area description*/
    u_char description[OSPF_DESCRIP_LEN];
    
    u_int state;
};

/*scan of area's range*/ 
#define for_each_ospf_range(a,r, n) for_each_node(&(a)->range_table, r, n)

/*scan area*/
#define for_each_ospf_area(ins, a, n) for_each_node(&((ins)->area_table), a, n)

#define ospf_area_lsa_count(a, b) \
                  (((a) && (b <= OSPF_LS_TYPE_10) && (a)->ls_table[b]) ? ospf_lstcnt(&(a)->ls_table[b]->list) : 0)

#define ospf_flush_range(p_area) ospf_lstwalkup(&(p_area)->range_table, ospf_range_delete)

#define ospf_flush_network(ins) ospf_lstwalkup(&(ins)->network_table, ospf_network_delete)

#define ospf_flush_redistribute_list(ins) ospf_lstwalkup(&(ins)->redistribute_config_table, ospf_redistribute_delete)

#define ospf_flush_filter_policy(ins) ospf_lstwalkup(&(ins)->filter_policy_table, ospf_filter_policy_delete)

#define ospf_flush_redistribute_policy(ins) ospf_lstwalkup(&(ins)->redis_policy_table, ospf_redis_policy_delete)

#define ospf_flush_redistribute_range(ins) ospf_lstwalkup(&(ins)->redis_range_table, ospf_redis_range_delete)

struct ospf_range *ospf_range_create(struct ospf_area * p_area, u_int lstype, u_int dest, u_int mask);
struct ospf_range *ospf_range_match(struct ospf_area * p_area, u_int lstype, u_int dest, u_int mask, struct ospf_range * p_range_exclued);
struct ospf_range *ospf_range_lookup(struct ospf_area * p_area, u_int8 ls_type, u_int network, u_int mask);
struct ospf_area *ospf_area_create(struct ospf_process *p_process, u_int areaid );
struct ospf_area *ospf_area_lookup(struct ospf_process * p_process, u_int area_id);
void ospf_range_delete(struct ospf_range *p_range);
void ospf_area_up (struct ospf_area *p_area);
void ospf_area_down (struct ospf_area *p_area);
void ospf_range_up(struct ospf_range *p_range);
void ospf_range_down(struct ospf_range *p_range);
void ospf_range_update(struct ospf_range *p_range);
void ospf_nssa_translator_down(struct ospf_area *p_area);
void ospf_nssa_wait_timeout(struct ospf_area *p_area);
void ospf_nssa_range_up(struct ospf_range * p_range);
void ospf_nssa_range_down(struct ospf_range * p_range);
void ospf_nssa_range_update(struct ospf_range * p_range);
void ospf_nssa_translator_elect(struct ospf_area *p_area);
void ospf_area_delete(struct ospf_area *p_area );
void ospf_area_delete_event_handler(struct ospf_area *p_area);
void ospf_preferred_nssa_lsa_select(struct ospf_area *p_area, struct ospf_nssa_lsa *new_lsa, struct ospf_nssa_lsa *old_lsa);
void ospf_range_for_transit_area(struct ospf_area * p_transit_area);
u_int ospf_nssa_range_active(struct ospf_range *p_range);
u_int ospf_area_if_exist(struct ospf_area *p_area);
int ospf_area_nm_cmp(struct ospf_area *p1, struct ospf_area *p2);
int ospf_range_nm_cmp(struct ospf_range *p1, struct ospf_range *p2);
void ospf_originate_range_lsa(struct ospf_range *p_range,u_int need_flush);

int ospf_nssa_only_exist_exceptOwn(struct ospf_area *pstArea);

void ospf_backbone_status_update(struct ospf_process *p_process);

struct ospf_asbr_range *ospf_asbr_range_lookup(struct ospf_process *p_process, u_int8 ls_type, u_int dest, u_int mask);

u_int ospf_backbone_full_nbr_exsit(struct ospf_process *p_process);

#ifdef __cplusplus
}
#endif

#endif 

