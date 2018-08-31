
#include "bgp4_api.h"
#ifdef NEW_BGP_WANTED
#ifndef BGP4POLICY_H
#define BGP4POLICY_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "plateform.h"

enum{
       BGP4_ROUTE_PERMIT = 1,
       BGP4_ROUTE_DENY,
};

/*协议类型*/
enum{
    RPOLICY_BGP = 0,
    RPOLICY_PERMIT,
};

typedef enum
{
    APPLY_BGP_EGP = 0,
    APPLY_BGP_IGP,
    APPLY_BGP_INCOMPLETE,
}BGP_APPLY_E;



#define ROUTRPOLICY_APPLY_COMMUNITY 0x1

#define ROUTRPOLICY_APPLY_COST 0x2

#define ROUTRPOLICY_APPLY_PREFVALUE 0x4

#define ROUTRPOLICY_APPLY_LOCALPREF 0x8

#define ROUTRPOLICY_APPLY_ORIGINTYPE 0x10

#define ROUTRPOLICY_APPLY_EXTCOMMUNITYRT 0x20



typedef struct bgp4_redistribute_config{
    avl_node_t node;

    /*back pointer to instance*/
    tBGP4_VPN_INSTANCE *p_instance;
    
    u_char status;
    
    u_char af;   
    
    u_short proto;
    
    /*policy index in route map*/
    u_int policy;
    
    /*med configured*/
    u_int med;

    u_int processId;
}tBGP4_REDISTRIBUTE_CONFIG;


typedef struct bgp_com{
    /*ip address.in dest order*/
    u_char dest[24];
    /*ip address.in nexhop order*/
    u_char nexthop[24];
    /*ip address.in src order*/
    u_char src[24];
    u_short proto;
    /*address family*/
    u_short afi;
    /*prefix length in bits*/
    u_short prefixLen;
    u_int family;
    u_int metric;
    u_int set_flag;
}tBGP_COM;

typedef struct bgpPolicyEntry 
{  
    u_char match_aspath_count;
    u_char community_none;
    u_char aspath[2048];
    u_char set_excommunity[2048];
    u_char set_community[2048];
    u_short match_aspath[1024];
    /*set by route policy interface,per route*/
    u_short pref_value;
    u_short origin_type;
    u_int origin_value;
    u_int local_pref;
    u_int community_count;
    u_int community_additive;
    u_int excommunity_count;
    u_int excommunity_additive;
    tBGP_COM bgp_com;
}tBGP_POLICY_ENTRY;


typedef struct bgp4_policy_config
{
	avl_node_t node;

    /*back table it belong to*/
    avl_tree_t *p_table;
    
	u_char status;
	
	u_char af;	

    /*import or export*/
	u_short direction;

    /*policy index in route map*/
	u_int policy;
}tBGP4_POLICY_CONFIG; /*ROUTE POLICY CONFIG*/

/*orf entry stored*/
typedef struct bgp4_orf_entry{
    /*node in avl*/
    avl_node_t node;

    /*table it belong to*/
    avl_tree_t *p_table;

    /*unique seq*/
    u_int seqnum;

    /*match action*/
    u_char match : 4;

    /*NM state*/
    u_int rowstatus : 4;
        
    /*changed*/
    u_char changed : 1;

    /*to be deleted*/
    u_char wait_delete : 1;
    
    /*min prefixlength in bits*/
    u_char minlen;
    
    /*max prefixlength in bits*/
    u_char maxlen;

    /*dest:include af and prefix length*/
    tBGP4_ADDR prefix;
}tBGP4_ORF_ENTRY;

tBGP4_REDISTRIBUTE_CONFIG *bgp4_redistribute_policy_create(tBGP4_VPN_INSTANCE *p_instance,u_int af, u_int proto,u_int policy,u_int processId);
tBGP4_POLICY_CONFIG *bgp4_policy_create(avl_tree_t * p_head, u_int af, u_int proto, u_int id);
tBGP4_ORF_ENTRY *bgp4_orf_create(avl_tree_t *p_table, u_int af, u_int seq);
tBGP4_ORF_ENTRY *bgp4_orf_lookup(avl_tree_t *p_table, u_int af, u_int seq);
u_int bgp4_import_route_policy_apply(tBGP4_ROUTE * p_route);
u_int bgp4_redistribute_policy_apply(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE* p_route);
u_int bgp4_export_route_policy_apply(tBGP4_ROUTE *p_route, tBGP4_PEER *p_peer, tBGP4_PATH *p_path);
u_int bgp4_route_peer_orf_permitted(tBGP4_PEER * p_peer, avl_tree_t * p_table, tBGP4_ROUTE * p_route);
void bgp4_policy_delete_all(avl_tree_t *p_head);
void bgp4_policy_delete(tBGP4_POLICY_CONFIG* p_policy);
void bgp4_redistribute_policy_delete(tBGP4_REDISTRIBUTE_CONFIG *p_policy);
void bgp4_orf_delete(tBGP4_ORF_ENTRY *p_orf);
void bgp4_peer_orf_update(tBGP4_PEER *p_peer, u_int af, avl_tree_t *p_table);
int bgp4_policy_lookup_cmp(tBGP4_POLICY_CONFIG *p1, tBGP4_POLICY_CONFIG *p2);
int bgp4_redistribute_policy_lookup_cmp(tBGP4_REDISTRIBUTE_CONFIG *p1, tBGP4_REDISTRIBUTE_CONFIG *p2);
int bgp4_orf_lookup_cmp(tBGP4_ORF_ENTRY *p1, tBGP4_ORF_ENTRY *p2);

#ifdef __cplusplus
}
#endif     
#endif

#else
#ifndef BGP4POLICY_H
#define BGP4POLICY_H
#ifdef __cplusplus
      extern "C" {
     #endif
#include "routepolicy.h"
#include "routepolicy_nm.h"

enum{
       BGP4_ROUTE_PERMIT= 1,
       BGP4_ROUTE_DENY,
};

typedef struct bgp4_policy_config
{
    tBGP4_LISTNODE node;

    u_char status;
    
    u_char rsvd[3];
    
    u_char af;   
    
    u_char apply_direction;/*import or export*/
    
    u_short apply_protocol;/*route protoco to be applied*/
    
    u_int policy_index;/*policy index in route map*/

    u_int med;/*only apply to redistribution*/
    
}tBGP4_POLICY_CONFIG; /*ROUTE POLICY CONFIG*/

tBGP4_POLICY_CONFIG *  bgp4_policy_add(tBGP4_LIST *p_head);
void bgp4_policy_del(tBGP4_LIST *p_head,tBGP4_POLICY_CONFIG* p_policy);
tBGP4_POLICY_CONFIG*  bgp4_policy_lookup(tBGP4_LIST *p_head,u_int af,u_int policy_index,u_int proto);
u_int bgp4_check_route_policy(tBGP4_ROUTE*p_route,tBGP4_PEER*p_peer,u_int direction);
u_int bgp4_check_import_policy(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE* p_route);
void bgp4_delete_policy_list(tBGP4_LIST *p_head);
#ifdef __cplusplus
     }
     #endif     
#endif


#endif
