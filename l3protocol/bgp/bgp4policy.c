

#include "bgp4peer.h"
#include "bgp4path.h"
#include "bgp4policy.h"

#ifdef NEW_BGP_WANTED
/*****************************************************************************
  bgp4policy.c :apply route policy func
******************************************************************************/

#include "bgp4com.h"

/*compare function for redistribute policy table
  AF,protocol,policy-id
 */
int
bgp4_redistribute_policy_lookup_cmp(
          tBGP4_REDISTRIBUTE_CONFIG *p1,
          tBGP4_REDISTRIBUTE_CONFIG *p2)
{
    if (p1->af != p2->af)
    {
        return (p1->af > p2->af) ? 1 : -1;
    }

    if (p1->proto != p2->proto)
    {
        return (p1->proto > p2->proto) ? 1 : -1;
    }

    if (p1->policy != p2->policy)
    {
        return (p1->policy > p2->policy) ? 1 : -1;
    }
    
    if(p1->processId != p2->processId)
    {
        return (p1->processId > p2->processId) ? 1 : -1;
    }
    
    return 0;
}

/*policy compare,af+direction+policy*/
int
bgp4_policy_lookup_cmp(
          tBGP4_POLICY_CONFIG *p1,
          tBGP4_POLICY_CONFIG *p2)
{
    if (p1->af != p2->af)
    {
        return (p1->af > p2->af) ? 1 : -1;
    }
    
    if (p1->direction != p2->direction)
    {
        return (p1->direction > p2->direction) ? 1 : -1;
    }
    
    if (p1->policy != p2->policy)
    {
        return (p1->policy > p2->policy) ? 1 : -1;
    }
    return 0;
}

tBGP4_POLICY_CONFIG *
bgp4_policy_create(
      avl_tree_t *p_table,
      u_int af,
      u_int direction,
      u_int id)
{
    tBGP4_POLICY_CONFIG* p_new = NULL;
    
    p_new = bgp4_malloc(sizeof(tBGP4_POLICY_CONFIG), MEM_BGP_POLICY_CONFIG);
    if (p_new == NULL)
    {
        return NULL;
    }
    p_new->af = af;
    p_new->direction = direction;
    p_new->policy = id;
    
    p_new->p_table = p_table;
    bgp4_avl_add(p_table, p_new);
    return p_new;
}

void 
bgp4_policy_delete(tBGP4_POLICY_CONFIG *p_policy)
{
    if (p_policy->policy && gbgp4.policy_ref_func)
    {
        gbgp4.policy_ref_func(p_policy->policy, 0);
    }

    bgp4_avl_delete(p_policy->p_table, p_policy);
    
    bgp4_free(p_policy, MEM_BGP_POLICY_CONFIG);    
    return;
}

void 
bgp4_policy_delete_all(avl_tree_t *p_table)
{
    bgp4_avl_walkup(p_table, bgp4_policy_delete);
    return;
}

/*translate BGP route into policy format*/
void 
bgp4_policy_fill(
        struct bgpPolicyEntry *p_policy,
        tBGP4_ROUTE* p_route)
{
    tBGP4_PATH *p_path = p_route->p_path;
    tBGP4_ASPATH *p_aspath = NULL;
    u_short *p_as = NULL;
    u_int i = 0;
    u_int afi = 0;
    u_int safi = 0;

    afi = bgp4_index_to_afi(p_route->dest.afi);
    safi = bgp4_index_to_safi(p_route->dest.afi);

    memset(p_policy,0,sizeof(struct bgpPolicyEntry));
  
    /*copy route policy item*/
    p_policy->bgp_com.proto = RPOLICY_BGP;
    p_policy->bgp_com.family = (afi == BGP4_AF_IP) ? AF_INET : AF_INET6;
    if (safi == BGP4_SAF_VLBL)
    {
        p_policy->bgp_com.prefixLen = p_route->dest.prefixlen - BGP4_VPN_RD_LEN*8;
        if (afi == BGP4_AF_IP)
        {
            memcpy(&p_policy->bgp_com.dest, p_route->dest.ip + BGP4_VPN_RD_LEN, 4);
        }
        else
        {
            memcpy(&p_policy->bgp_com.dest, p_route->dest.ip + BGP4_VPN_RD_LEN, 16);
        }
    }
    else
    {
        p_policy->bgp_com.prefixLen = p_route->dest.prefixlen;
        if( afi == BGP4_AF_IP)
        {
            memcpy(&p_policy->bgp_com.dest, p_route->dest.ip, 4);
        }
        else
        {
            memcpy(&p_policy->bgp_com.dest, p_route->dest.ip, 16);
        }
    }
    if (afi == BGP4_AF_IP)
    {
        memcpy(&p_policy->bgp_com.nexthop, p_path->nexthop.ip, 4);
        if (p_route->p_path->p_peer)
        {
            memcpy(&p_policy->bgp_com.src, p_path->p_peer->ip.ip, 4);
        }
    }
    else
    {
        memcpy(&p_policy->bgp_com.nexthop, p_path->nexthop.ip, 16);
        if (p_route->p_path->p_peer)
        {
            memcpy(&p_policy->bgp_com.src, p_path->p_peer->ip.ip, 16);
        }
    }
    bgp4_avl_for_each(&p_route->p_path->aspath_list, p_aspath)
    {
        p_as = (u_short *)p_aspath->as;
        
        for (i = 0 ; i < p_aspath->count; i++, p_as++)
        {
            if (p_policy->match_aspath_count >= 32)
            {
                break;
            }
            p_policy->match_aspath[p_policy->match_aspath_count] = *p_as;
            p_policy->match_aspath_count++;
        }
    }
    return;    
}

void 
bgp4_path_update_by_policy(
      struct bgpPolicyEntry*p_policy,
      tBGP4_ROUTE *p_route,
      tBGP4_PATH *p_path)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_char community[2048];
    u_int community_len = 0;
    u_char *p_as = NULL;
    
    if (p_policy->bgp_com.set_flag == 0)
    {
        return;
    }
    /*apply cost*/
    if (BIT_LST_TST((unsigned char *)&p_policy->bgp_com.set_flag, ROUTRPOLICY_APPLY_COST))
    {
        p_path->med = p_policy->bgp_com.metric;
        p_path->policy_updated = TRUE;
    }

    /*apply pref value*/
    if (BIT_LST_TST((unsigned char *)&p_policy->bgp_com.set_flag, ROUTRPOLICY_APPLY_PREFVALUE))
    {
        p_route->preference = p_policy->pref_value;
        p_path->policy_updated = TRUE;
    }

    /*apply lp*/
    if (BIT_LST_TST((unsigned char *)&p_policy->bgp_com.set_flag, ROUTRPOLICY_APPLY_LOCALPREF))
    {
        p_path->localpref = p_policy->local_pref;
        p_path->policy_updated = TRUE;
    }

    /*apply origin*/
    if (BIT_LST_TST((unsigned char *)&p_policy->bgp_com.set_flag, ROUTRPOLICY_APPLY_ORIGINTYPE))
    {       
        p_path->policy_updated = TRUE;
        
        if (p_policy->origin_type == APPLY_BGP_EGP)
        {
            p_path->origin = BGP4_ORIGIN_EGP;
        }
        else if (p_policy->origin_type == APPLY_BGP_IGP)
        {
            p_path->origin = BGP4_ORIGIN_IGP;
        }
        else if (p_policy->origin_type == APPLY_BGP_INCOMPLETE)
        {
            p_path->origin = BGP4_ORIGIN_INCOMPLETE;
        }

        /*create as-path */
        if ((p_policy->origin_type == APPLY_BGP_EGP) 
            && p_policy->origin_value)
        {
            p_as = (u_char*)&p_policy->origin_value;
        
            bgp4_apath_list_free(p_path);

            /*include 1 as*/
            p_aspath = bgp4_malloc(sizeof(tBGP4_ASPATH) + 2, MEM_BGP_ASPATH);
            if (p_aspath == NULL) 
            {
                return;
            }
            memcpy(p_aspath->as, p_as + 2, 2);
            p_aspath->count = 1; 
            p_aspath->type = BGP_ASPATH_SEQ;
            
            bgp4_avl_add(&p_path->aspath_list, p_aspath);
        }
    }

    /*apply community*/
    if (BIT_LST_TST((unsigned char *)&p_policy->bgp_com.set_flag, ROUTRPOLICY_APPLY_COMMUNITY))
    {
        community_len = 0;
        /*clear all community*/
        if (p_policy->community_none)
        {
            if (p_path->p_community)
            {
                bgp4_free(p_path->p_community, MEM_BGP_BUF);
                p_path->p_community = NULL;
                p_path->community_len = 0;
            }
        }
        /*add or replace current community*/
        if (p_policy->community_count)
        {
            if (p_policy->community_additive)
            {
                community_len = p_path->community_len;

                /*save current community*/
                if (p_path->community_len && p_path->p_community)
                {
                    memcpy(community, p_path->p_community, p_path->community_len);    
                }
            }
            /*insert policy value*/
            memcpy(community + community_len, p_policy->set_community, p_policy->community_count * 4);
            community_len += p_policy->community_count * 4;

            bgp4_free(p_path->p_community, MEM_BGP_BUF);
            p_path->p_community = NULL;
            p_path->community_len = 0;

            p_path->p_community = bgp4_malloc(community_len, MEM_BGP_BUF);
            if (p_path->p_community)
            {
                memcpy(p_path->p_community, community, community_len);
                p_path->community_len = community_len;
            }
        }
    }
    
    /*apply extend community*/
    if (BIT_LST_TST((unsigned char *)&p_policy->bgp_com.set_flag, ROUTRPOLICY_APPLY_EXTCOMMUNITYRT))
    {
        community_len = 0;
      
        /*add or replace current community*/
        if (p_policy->excommunity_count)
        {
            if (p_policy->excommunity_additive)
            {
                community_len = p_path->excommunity_len;

                /*save current community*/
                if (p_path->excommunity_len && p_path->p_ecommunity)
                {
                    memcpy(community, p_path->p_ecommunity, p_path->excommunity_len);    
                }
            }
            /*insert policy value*/
            memcpy(community + community_len, p_policy->set_excommunity, p_policy->excommunity_count * 8);
            community_len += p_policy->excommunity_count * 8;

            bgp4_free(p_path->p_ecommunity, MEM_BGP_BUF);
            p_path->p_ecommunity = NULL;
            p_path->excommunity_len = 0;

            p_path->p_ecommunity = bgp4_malloc(community_len, MEM_BGP_BUF);
            if (p_path->p_ecommunity)
            {
                memcpy(p_path->p_ecommunity, community, community_len);
                p_path->excommunity_len = community_len;
            }
        }
    }
    return;
}

/*decide if we can send update to peer for special route,
  and may update path attribute of this update
*/
u_int
bgp4_export_route_policy_apply(
       tBGP4_ROUTE *p_route,
       tBGP4_PEER *p_peer,
       tBGP4_PATH *p_path/*include all modified params*/) 
{
    struct bgpPolicyEntry rt_policy;
    tBGP4_POLICY_CONFIG *p_policy = NULL;
    tBGP4_VPN_INSTANCE *p_instance = p_peer->p_instance;
    u_int rc = BGP4_ROUTE_PERMIT;

    if (gbgp4.policy_func == NULL)
    {
        return BGP4_ROUTE_PERMIT;
    }

    if (p_peer == NULL)
    {
        return BGP4_ROUTE_PERMIT;
    }
    
    /*first check global filter*/
    bgp4_avl_for_each(&p_instance->policy_table, p_policy)
    {
        if (p_policy->status != SNMP_ACTIVE)
        {
            continue;
        }

        if (/*(p_route->dest.afi != p_policy->af)
            || */(p_policy->direction != BGP4_POLICY_EXPORT_DIRECTION))
        {
            continue;
        }
        if (p_policy->policy == 0)
        {
            return BGP4_ROUTE_PERMIT;
        }
        
        bgp4_policy_fill(&rt_policy, p_route);
        
        /*call route policy apply func*/
        if (gbgp4.policy_func(p_policy->policy,
              &rt_policy) != RPOLICY_PERMIT)
        {
            return BGP4_ROUTE_DENY;
        }
        /*if permit,check if need modify*/
        bgp4_path_update_by_policy(&rt_policy, p_route, p_path);
    }

    /*check peer filter*/
    bgp4_avl_for_each(&p_peer->policy_table, p_policy)
    {
        if (p_policy->status != SNMP_ACTIVE)
        {
            continue;
        }
        
        if (/*(p_route->dest.afi != p_policy->af)
            || */(p_policy->direction != BGP4_POLICY_EXPORT_DIRECTION))
        {
            continue;
        }
        if (p_policy->policy == 0)
        {
            bgp4_log(BGP_DEBUG_EVT,"[line:%d]policy index is invalid value 0",__LINE__);
            return BGP4_ROUTE_PERMIT;
        }
        
        bgp4_policy_fill(&rt_policy, p_route);
        
        /*call route policy apply func*/
        if (gbgp4.policy_func(p_policy->policy,
            &rt_policy) != RPOLICY_PERMIT)
        {
            return BGP4_ROUTE_DENY;
        }
        /*if permit,check if need modify*/
        bgp4_path_update_by_policy(&rt_policy, p_route, p_path);
    }
    return rc;
}

/*check if config router filter,return BGP4_ROUTE_PERMIT or BGP4_ROUTE_DENY */
u_int 
bgp4_import_route_policy_apply(tBGP4_ROUTE *p_route)
{
    struct bgpPolicyEntry rt_policy;
    tBGP4_POLICY_CONFIG *p_policy = NULL;
    tBGP4_PATH *p_path = p_route->p_path;
    tBGP4_PEER *p_peer = p_path->p_peer;
    tBGP4_VPN_INSTANCE *p_instance = p_path->p_instance;    
    u_int rc = BGP4_ROUTE_PERMIT;

    if (gbgp4.policy_func == NULL)
    {
        return BGP4_ROUTE_PERMIT;
    }

    if (p_peer == NULL)
    {
        return BGP4_ROUTE_PERMIT;
    }
    
    /*check some semantic for import routes only*/
    if (p_route->p_path->p_peer 
        && (p_route->dest.afi == BGP4_PF_IP6UCAST)
        && (memcmp(p_route->p_path->p_peer->local_ip.ip, p_route->p_path->nexthop.ip, 16)==0))
    {
        bgp4_peer_route_clear (p_route->dest.afi, p_peer);
        
        /*For the duration of the BGP session over which the Update message 
        was received, the speaker then SHOULD ignore all the subsequent routes 
        with that AFI/SAFI received over that session.
        */
        flag_clear(p_peer->mpbgp_capability, p_route->dest.afi);
        return BGP4_ROUTE_DENY;
    }
        
    /*if local capability or remote capability do not support that afi,we should
    not insert route */
    if (!flag_isset(p_peer->mpbgp_capability, p_route->dest.afi))
    {
        return BGP4_ROUTE_DENY;
    }
    if (!flag_isset(p_peer->local_mpbgp_capability, p_route->dest.afi))
    {    
        return BGP4_ROUTE_DENY;
    }
    
    /*multicast address && class E*/
    if (p_route->dest.afi == BGP4_PF_IPUCAST)
    {
         if (ntohl(bgp_ip4(p_route->dest.ip)) >= 0xE0000000)
         {
             return BGP4_ROUTE_DENY;
         }
         if (ntohl(bgp_ip4(p_route->dest.ip)) == 0x7F000001)
         {
             return BGP4_ROUTE_DENY;
         }
        /*ignore routes including invalid nexthop*/
        /*check if next hop is reachable*/
        if (bgp4_nexthop_is_reachable(p_instance->vrf, &p_route->p_path->nexthop) == VOS_ERR)
        {
            bgp4_log(BGP_DEBUG_EVT, "nexthop is unreachable");  
            return BGP4_ROUTE_DENY;
        }
    }
    else if (p_route->dest.afi == BGP4_PF_IP6UCAST)
    {
        /*ignore routes including invalid nexthop*/
        /*check if next hop is reachable*/
        if (bgp4_nexthop_is_reachable(p_instance->vrf, &p_route->p_path->nexthop) == VOS_ERR)
        {
            bgp4_log(BGP_DEBUG_EVT, "nexthop is unreachable");  
            return BGP4_ROUTE_DENY;
        }
    }
    
    /*first check global filter*/
    bgp4_avl_for_each(&p_instance->policy_table, p_policy)
    {
        if (p_policy->status != SNMP_ACTIVE)
        {
            continue;
        }

        if ((p_route->dest.afi != p_policy->af)
            || (p_policy->direction != BGP4_POLICY_IMPORT_DIRECTION))
        {
            continue;
        }
        if (p_policy->policy == 0)
        {
            return BGP4_ROUTE_PERMIT;
        }
        
        bgp4_policy_fill(&rt_policy, p_route);
        
        /*call route policy apply func*/
        if (gbgp4.policy_func(p_policy->policy, &rt_policy) != RPOLICY_PERMIT)
        {
            return BGP4_ROUTE_DENY;
        }
        /*if permit,check if need modify*/
        bgp4_path_update_by_policy(&rt_policy, p_route, p_path);
    }

    /*check peer filter*/
    bgp4_avl_for_each(&p_peer->policy_table, p_policy)
    {
        if (p_policy->status != SNMP_ACTIVE)
        {
            continue;
        }
        
        if ((p_route->dest.afi != p_policy->af)
            || (p_policy->direction != BGP4_POLICY_IMPORT_DIRECTION))
        {
            continue;
        }
        if (p_policy->policy == 0)
        {
            bgp4_log(BGP_DEBUG_EVT,"[line:%d]policy index is invalid value 0",__LINE__);
            return BGP4_ROUTE_PERMIT;
        }
        
        bgp4_policy_fill(&rt_policy, p_route);
        
        /*call route policy apply func*/
        if(gbgp4.policy_func(p_policy->policy, &rt_policy) != RPOLICY_PERMIT)
        {
            return BGP4_ROUTE_DENY;
        }
        /*if permit,check if need modify*/
        bgp4_path_update_by_policy(&rt_policy, p_route, p_path);
    }
    return rc;
}

/*check if config router filter,return BGP4_ROUTE_PERMIT or BGP4_ROUTE_DENY */
u_int 
bgp4_redistribute_policy_apply(
       tBGP4_VPN_INSTANCE *p_instance,
       tBGP4_ROUTE *p_route)
{
    tBGP4_REDISTRIBUTE_CONFIG *p_policy = NULL;
    struct bgpPolicyEntry rt_policy;
    u_int rc = BGP4_ROUTE_DENY;

    if (p_route->dest.afi == BGP4_PF_IPUCAST)
    {
         if (ntohl(bgp_ip4(p_route->dest.ip)) >= 0xE0000000)
         {
             return BGP4_ROUTE_DENY;
         }
         if (ntohl(bgp_ip4(p_route->dest.ip)) == 0x7F000001)
         {
             return BGP4_ROUTE_DENY;
         }
    }

    memset(&rt_policy, 0, sizeof(struct bgpPolicyEntry));

    /*first check global filter*/
    bgp4_avl_for_each(&p_instance->redistribute_policy_table, p_policy)
    {
        if (p_policy->status != SNMP_ACTIVE)
        {
            continue;
        }
        
        if ((p_policy->proto != p_route->proto)
            || (p_route->dest.afi != p_policy->af))
        {
            continue;
        } 
         if (p_policy->policy == 0)/*invalid policy,apply med if need*/
         {
             rc = BGP4_ROUTE_PERMIT;
             if (p_policy->med)
             {
                 p_route->p_path->med = p_policy->med;
             }
         }
         else
         {
             if (gbgp4.policy_func == NULL)
             {
                  return BGP4_ROUTE_DENY;
             }
 
             bgp4_policy_fill(&rt_policy, p_route);
             /*call route policy apply func*/
             if (gbgp4.policy_func(p_policy->policy, &rt_policy) != RPOLICY_PERMIT)
             {
                 return BGP4_ROUTE_DENY;
             }
             else/*if permit,check if need modify*/
             {
                 rc = BGP4_ROUTE_PERMIT;
                 bgp4_path_update_by_policy(&rt_policy, p_route, p_route->p_path);
             }
         }
    }
    return rc;
}

tBGP4_REDISTRIBUTE_CONFIG *
bgp4_redistribute_policy_create(
                tBGP4_VPN_INSTANCE *p_instance,
                u_int af,
                u_int proto,
                u_int policy,
                u_int processId)
{
    tBGP4_REDISTRIBUTE_CONFIG *p_policy = NULL;
    
    p_policy = bgp4_malloc(sizeof(tBGP4_REDISTRIBUTE_CONFIG), MEM_BGP_POLICY_CONFIG);
    if (p_policy == NULL)
    {
        return NULL;
    }
    p_policy->med = gbgp4.med;/*set default med,used if not config redistribute med*/
    p_policy->af = af;
    p_policy->proto = proto;
    p_policy->policy = policy;
    p_policy->p_instance = p_instance;
    p_policy->processId = processId;
    bgp4_avl_add(&p_instance->redistribute_policy_table, p_policy);
    return p_policy;
}

void
bgp4_redistribute_policy_delete(tBGP4_REDISTRIBUTE_CONFIG *p_policy) 
{
    if (p_policy->policy && gbgp4.policy_ref_func)
    {
        gbgp4.policy_ref_func(p_policy->policy, 0);
    }
 
    bgp4_avl_delete(&p_policy->p_instance->redistribute_policy_table, p_policy);
    bgp4_free(p_policy, MEM_BGP_POLICY_CONFIG);
    return;
}

/*ORF related*/
int
bgp4_orf_lookup_cmp(
             tBGP4_ORF_ENTRY *p1,
             tBGP4_ORF_ENTRY *p2)
{
    /*AF*/
    if (p1->prefix.afi != p2->prefix.afi)
    {
        return (p1->prefix.afi > p2->prefix.afi) ? 1 : -1;
    }
    /*SEQ*/
    if (p1->seqnum != p2->seqnum)
    {
        return (p1->seqnum> p2->seqnum) ? 1 : -1;
    }
    return 0;
}

/*create orf,only key need*/
tBGP4_ORF_ENTRY *
bgp4_orf_create(
            avl_tree_t *p_table,
            u_int af,
            u_int seq)
{
    tBGP4_ORF_ENTRY *p_orf = bgp4_malloc(sizeof(tBGP4_ORF_ENTRY), MEM_BGP_ORF);
    if (p_orf)
    {
        p_orf->prefix.afi = af;
        p_orf->seqnum = seq;
        p_orf->p_table = p_table;
        bgp4_avl_add(p_table, p_orf);
    }
    return p_orf;
}

tBGP4_ORF_ENTRY *
bgp4_orf_lookup(
            avl_tree_t *p_table,
            u_int af,
            u_int seq)
{
    tBGP4_ORF_ENTRY orf;
    orf.prefix.afi = af;
    orf.seqnum = seq;
    return bgp4_avl_lookup(p_table, &orf);
}

void
bgp4_orf_delete(tBGP4_ORF_ENTRY *p_orf)
{
    if (p_orf->p_table)
    {
        bgp4_avl_delete(p_orf->p_table, p_orf);
    }
    bgp4_free(p_orf, MEM_BGP_ORF);
    return;
}

/*update peer's remote orf table according to msg*/
void
bgp4_peer_orf_update(
                 tBGP4_PEER *p_peer,
                 u_int af,
                 avl_tree_t *p_table)
{
    tBGP4_ORF_ENTRY *p_orf = NULL;
    tBGP4_ORF_ENTRY *p_current = NULL;
    tBGP4_ORF_ENTRY *p_next = NULL;

    /*copy current orf into old table*/
    bgp4_avl_walkup(&p_peer->orf_old_in_table, bgp4_orf_delete);
    bgp4_avl_for_each_safe(&p_peer->orf_in_table, p_current, p_next)
    {
        p_orf = bgp4_orf_create(&p_peer->orf_old_in_table, p_current->prefix.afi, p_current->seqnum);
        if (p_orf)
        {
            p_orf->match = p_current->match;
            p_orf->minlen = p_current->minlen;
            p_orf->maxlen = p_current->maxlen;
            memcpy(&p_orf->prefix, &p_current->prefix, sizeof(p_orf->prefix));
        }
    }

    /*update current orf table*/
    bgp4_avl_for_each(p_table, p_orf)
    {
        /*case remove all*/
        if (p_orf->seqnum == 0xffffffff)
        {
            bgp4_avl_for_each_safe(&p_peer->orf_in_table, p_current, p_next)
            {
                if (p_current->prefix.afi == af)
                {
                    bgp4_orf_delete(p_current);
                }
            }
            break;
        }
        /*search and update*/
        p_current = bgp4_avl_lookup(&p_peer->orf_in_table, p_orf);
        if (p_current)
        {
            bgp4_orf_delete(p_current);
        }
        if (p_orf->wait_delete == FALSE)
        {
            p_current = bgp4_orf_create(&p_peer->orf_in_table, af, p_orf->seqnum);
            if (p_current)
            {
                p_current->match = p_orf->match;
                p_current->minlen = p_orf->minlen;
                p_current->maxlen = p_orf->maxlen;
                memcpy(&p_current->prefix, &p_orf->prefix, sizeof(p_orf->prefix));
            }
        }
    }
    return;
}

/*decide if route is permit by orf*/
u_int
bgp4_route_peer_orf_permitted(
                    tBGP4_PEER *p_peer,
                    avl_tree_t *p_table,
                    tBGP4_ROUTE *p_route)
{
    tBGP4_ORF_ENTRY *p_orf = NULL;
    
    /*check each orf*/
    bgp4_avl_for_each(p_table, p_orf)
    {
        if (p_orf->prefix.afi != p_route->dest.afi)
        {
            continue;
        }
        /*prefix must match*/
        if ((p_route->dest.prefixlen < p_orf->prefix.prefixlen))
        {
            continue;
        }
        if (bgp4_prefixmatch(p_route->dest.ip, 
            p_orf->prefix.ip, p_orf->prefix.prefixlen) != TRUE)
        {
            continue;
        }
        /*check min and max length*/
        if (p_orf->minlen && (p_route->dest.prefixlen < p_orf->minlen))
        {
            continue;
        }
        if (p_orf->maxlen && (p_route->dest.prefixlen > p_orf->maxlen))
        {
            continue;
        }
        /*got it*/
        if (p_orf->match == BGP4_ORF_MATCH_PERMIT)
        {
            return TRUE;
        }
        return FALSE;
    }
    /*no matched orf exist,if peer support this orf,reject route*/
    if (flag_isset(p_peer->orf_recv, p_route->dest.afi)
        && flag_isset(p_peer->orf_remote_send, p_route->dest.afi))
    {
        return FALSE;
    }
    return TRUE;
}
#else
/*****************************************************************************
  bgp4policy.c :apply route policy func
******************************************************************************/

#include "bgp4com.h"

tBGP4_POLICY_CONFIG*  bgp4_policy_add(tBGP4_LIST *p_head)
{
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_POLICY_CONFIG* p_prev = NULL;
    tBGP4_POLICY_CONFIG* p_new = NULL;
    
    if(p_head == NULL)
    {
        return NULL;
    }
        
    p_new = bgp4_malloc(sizeof(tBGP4_POLICY_CONFIG),MEM_BGP_POLICY_CONFIG);
    if (p_new == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 policy add malloc policy config failed");
        return NULL;
    }
    memset(p_new,0,sizeof(tBGP4_POLICY_CONFIG));
    bgp4_lstnodeinit(&p_new->node);
    p_new->med = gBgp4.default_med;/*set default med,used if not config redistribute med*/

    /*sorted by policy_index*/
        LST_LOOP(p_head, p_policy, node, tBGP4_POLICY_CONFIG)
        {
            if (p_policy->policy_index > p_new->policy_index)
                    break;
            else
            p_prev = p_policy;
        }

        if (p_prev)
            bgp4_lstadd(&p_prev->node, &p_new->node);
    else
        bgp4_lstadd(p_head, &p_new->node);
    
        
        return p_new;
}

void bgp4_policy_del(tBGP4_LIST *p_head,tBGP4_POLICY_CONFIG* p_policy)
{
    if(p_policy == NULL || p_head == NULL)
        return;

    if(p_policy->policy_index && gBgp4.routePolicyRefCntFunc)
    {
        gBgp4.routePolicyRefCntFunc(p_policy->policy_index,0);
    }

    
    bgp4_lstdelete(p_head, &p_policy->node);
        bgp4_free(p_policy, MEM_BGP_POLICY_CONFIG);
    
    return;
}
tBGP4_POLICY_CONFIG*  bgp4_policy_lookup(tBGP4_LIST *p_head,u_int af,u_int policy_index,u_int proto)
{
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    u_char found = 0;
    
        if(p_head == NULL)
        {
        return NULL;
        }

    LST_LOOP(p_head,p_policy,node,tBGP4_POLICY_CONFIG)
    {
        if(af == p_policy->af &&
                policy_index == p_policy->policy_index &&
                proto == p_policy->apply_protocol)
        {
            found = 1;
            return p_policy;
        }
    }

    return NULL;
    

        
}

#if 0
tBGP4_POLICY_CONFIG* bgp4_policy_lookup_by_index(tBGP4_LIST *p_head,u_int index)
{
    tBGP4_POLICY_CONFIG* p_policy = NULL;

    LST_LOOP(p_head, p_policy, node, tBGP4_POLICY_CONFIG)
    {
        if(p_policy->policy_config_id == index)
        {
            return p_policy;
        }
    }
    
    return NULL;
}
#endif

void bgp4_delete_policy_list(tBGP4_LIST *p_head)
{
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_POLICY_CONFIG* p_next_policy = NULL;
    
    LST_LOOP_SAFE(p_head, p_policy, p_next_policy, node, tBGP4_POLICY_CONFIG) 
        {
            bgp4_policy_del(p_head,p_policy);
        }

}

void bgp4_fill_route_policy(struct bgpPolicyEntry* p_policy,tBGP4_ROUTE* p_route)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_short *p_asnos = NULL;
    u_int i = 0;
    u_int afi = 0;
    u_int safi = 0;

    afi = bgp4_index_to_afi(p_route->dest.afi);
    safi = bgp4_index_to_safi(p_route->dest.afi);

    memset(p_policy,0,sizeof(struct bgpPolicyEntry));
    
    /*copy route policy item*/
    p_policy->bgp_com.proto = RPOLICY_BGP;
    p_policy->bgp_com.family = (afi == BGP4_AF_IP) ? AF_INET : AF_INET6;
    if(safi == BGP4_SAF_VLBL)
    {
        p_policy->bgp_com.prefixLen = p_route->dest.prefixlen - BGP4_VPN_RD_LEN*8;
        if(afi == BGP4_AF_IP)
        {
            memcpy(&p_policy->bgp_com.dest,p_route->dest.ip+BGP4_VPN_RD_LEN,4);
        }
        else
        {
            memcpy(&p_policy->bgp_com.dest,p_route->dest.ip+BGP4_VPN_RD_LEN,16);
        }
    }
    else
    {
        p_policy->bgp_com.prefixLen = p_route->dest.prefixlen;
        if(afi == BGP4_AF_IP)
        {
            memcpy(&p_policy->bgp_com.dest,p_route->dest.ip,4);
        }
        else
        {
            memcpy(&p_policy->bgp_com.dest,p_route->dest.ip,16);
        }
    }
    if(afi == BGP4_AF_IP)
    {
        memcpy(&p_policy->bgp_com.nexthop,p_route->p_path->nexthop.ip,4);
        if(p_route->p_path->p_peer)
        {
            memcpy(&p_policy->bgp_com.src,p_route->p_path->p_peer->remote.ip.ip,4);
        }
    }
    else
    {
        memcpy(&p_policy->bgp_com.nexthop,p_route->p_path->nexthop_global.ip,16);
        if(p_route->p_path->p_peer)
        {
            memcpy(&p_policy->bgp_com.src,p_route->p_path->p_peer->remote.ip.ip,16);
        }
    }

#if 1/*fill aspath*/
    LST_LOOP(&p_route->p_path->aspath_list, p_aspath, node, tBGP4_ASPATH)
    {
        p_asnos = (u_short *)p_aspath->p_asseg;
        
        for (i = 0 ; i < p_aspath->len; i++, p_asnos++)
        {
            if (p_policy->match_aspath_count >= 32)
            {
                break;
            }
            p_policy->match_aspath[p_policy->match_aspath_count] = *p_asnos;
            p_policy->match_aspath_count++;
        }
    }
#endif

    bgp4_log(BGP_DEBUG_RT,1,"bgp4 fill route policy:fill route dest %#x/%d,nexthop %#x,source ip %#x",
                p_policy->bgp_com.dest[0],
                p_policy->bgp_com.prefixLen,
                p_policy->bgp_com.nexthop[0],
                p_policy->bgp_com.src[0]);
    
    
}
tBGP4_PATH* bgp4_policy_path_create(tBGP4_ROUTE* p_route,u_int direction)
{
    tBGP4_PATH* p_old_path = NULL;
    tBGP4_PATH* p_new_path = NULL;
    tBGP4_PATH* p_temp_path = NULL;
    u_int af =0;

    p_old_path = p_route->p_path;

    if(p_old_path == NULL)
    {
        return NULL;
    }
    
    if(bgp4_lstcnt(&p_old_path->route_list) <= 1)
    {
        return p_old_path;
    }   
    else if(direction == BGP4_POLICY_EXPORT_DIRECTION)
    {       
        af = p_route->dest.afi;
        p_temp_path = bgp4_malloc(sizeof(tBGP4_PATH), MEM_BGP_INFO);
        if(p_temp_path)
        {
            memset(p_temp_path,0,sizeof(tBGP4_PATH)) ;
            bgp4_lstinit(&p_temp_path->aspath_list);
            bgp4_lstinit(&p_temp_path->route_list);
            p_temp_path->afi = bgp4_index_to_afi(af);
            p_temp_path->safi = bgp4_index_to_safi(af);
            
            p_route->p_out_path = p_temp_path;
            bgp4_path_copy(p_temp_path,p_old_path);
            return p_temp_path;
        }
        else
        {
            bgp4_log(BGP_DEBUG_CMN,1,"bgp4 policy path create bgp4 malloc new path failed!");
            return NULL;
        }
    }
    else 
    {
        bgp4_lstdelete(&p_old_path->route_list,&p_route->node);
        p_new_path = bgp4_add_path(p_route->dest.afi);
        if(p_new_path)
        {
            bgp4_link_path(p_new_path,p_route);         
            bgp4_path_copy(p_new_path,p_old_path);
            p_new_path->p_peer = p_old_path->p_peer;
            return p_new_path;
        }
        else
        {
            bgp4_log(BGP_DEBUG_CMN,1,"bgp4 policy path create bgp4 malloc new path failed!");
            return NULL;
        }
    }
}

void bgp4_check_route_change_by_policy(struct bgpPolicyEntry*p_policy,
                            tBGP4_ROUTE* p_route,u_int direction)
{
    tBGP4_PATH* p_apply_path = NULL;
    tBGP4_PATH* p_true_path = NULL;
    u_char apply_path_flag = 0;
    
    if(p_policy == NULL || p_route == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route change by policy input para error!");
        return;
    }
    
    if(p_policy->bgp_com.set_flag == 0)
    {
        return;
    }

    if(direction == BGP4_POLICY_EXPORT_DIRECTION)
    {
        /*apply cost*/
        if(BIT_LST_TST(&p_policy->bgp_com.set_flag,ROUTRPOLICY_APPLY_COST))
        {
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route change by policy ROUTRPOLICY_APPLY_COST %d",p_policy->bgp_com.metric);  
            p_apply_path = bgp4_policy_path_create(p_route,BGP4_POLICY_EXPORT_DIRECTION);
            if(p_apply_path)
            {
                apply_path_flag = 1;
                p_apply_path->out_med = p_policy->bgp_com.metric;
            }
        }

        /*apply pref value*/
        if(BIT_LST_TST(&p_policy->bgp_com.set_flag,ROUTRPOLICY_APPLY_PREFVALUE))
        {
            p_route->preference= p_policy->pref_value;
        }

        /*apply lp*/
        if(BIT_LST_TST(&p_policy->bgp_com.set_flag,ROUTRPOLICY_APPLY_LOCALPREF))
        {
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route change by policy ROUTRPOLICY_APPLY_LOCALPREF %d",p_policy->local_pref);
            if(apply_path_flag = 0)
            {
                p_apply_path = bgp4_policy_path_create(p_route,BGP4_POLICY_EXPORT_DIRECTION);
                if(p_apply_path)
                {
                    apply_path_flag = 1;                
                }
            }
            p_apply_path->out_localpref = p_policy->local_pref;
        }

        /*apply origin*/
        if(BIT_LST_TST(&p_policy->bgp_com.set_flag,ROUTRPOLICY_APPLY_ORIGINTYPE))
        {
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route change by policy ROUTRPOLICY_APPLY_ORIGINTYPE type %d,as %d",
                    p_policy->origin_type,p_policy->origin_value);
            if(apply_path_flag == 0)
            {
                p_apply_path = bgp4_policy_path_create(p_route,BGP4_POLICY_EXPORT_DIRECTION);
                if(p_apply_path)
                {
                    apply_path_flag = 1;
                }
            }
            if(p_apply_path)
            {
                p_route->p_path->flags.set_origin = 1;
            
                if(p_policy->origin_type == APPLY_BGP_EGP)
                {
                    p_route->p_path->origin = BGP4_ORIGIN_EGP;
                    if(p_policy->origin_value)/*add as num*/
                    {
                        tBGP4_ASPATH * p_aspath = NULL;
                        u_char* p_as = (u_char*)&p_policy->origin_value;
                    
                        bgp4_apath_list_free(p_apply_path);
                    
                        p_aspath = (tBGP4_ASPATH*)bgp4_malloc(sizeof(tBGP4_ASPATH),MEM_BGP_ASPATH);
                        if (p_aspath == NULL) 
                        {
                                bgp4_log(BGP_DEBUG_CMN,1,"bgp4 check route change by policy not enough memory for feasiable list!!");
                                return ;
                        }
                        p_aspath->p_asseg = (u_char*)bgp4_malloc(2, MEM_BGP_BUF);            
                        if (p_aspath->p_asseg)
                        {
                                memcpy(p_aspath->p_asseg, p_as+2,2);
                            p_aspath->len = 1;/*as count*/
                            p_aspath->type = BGP_ASPATH_SEQ;
                        }
                        else
                        {
                            bgp4_log(BGP_DEBUG_CMN,1,"bgp4 check route change by policy bgp4 malloc as path failed!");
                            bgp4_free(p_aspath, MEM_BGP_ASPATH);
                            return ;
                        }
                    
                        bgp4_lstadd(&p_apply_path->aspath_list, &p_aspath->node);

                    }
                }
                else if (p_policy->origin_type == APPLY_BGP_IGP)
                {
                    p_apply_path->origin = BGP4_ORIGIN_IGP;
                }
                else if (p_policy->origin_type == APPLY_BGP_INCOMPLETE)
                {
                    p_apply_path->origin = BGP4_ORIGIN_INCOMPLETE;
                }
            }
        }
        if(apply_path_flag == 0)
        {
            p_route->p_out_path = NULL;
        }
    }
    else
    {
        /*apply cost*/
        if(BIT_LST_TST(&p_policy->bgp_com.set_flag,ROUTRPOLICY_APPLY_COST))
        {
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route change by policy ROUTRPOLICY_APPLY_COST %d",p_policy->bgp_com.metric);  
            p_apply_path = bgp4_policy_path_create(p_route,BGP4_POLICY_IMPORT_DIRECTION);
            if(p_apply_path)
            {
                apply_path_flag = 1;
                p_apply_path->out_med = p_policy->bgp_com.metric;
            }
        }

        /*apply pref value*/
        if(BIT_LST_TST(&p_policy->bgp_com.set_flag,ROUTRPOLICY_APPLY_PREFVALUE))
        {
            p_route->preference= p_policy->pref_value;
        }

        /*apply lp*/
        if(BIT_LST_TST(&p_policy->bgp_com.set_flag,ROUTRPOLICY_APPLY_LOCALPREF))
        {
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route change by policy ROUTRPOLICY_APPLY_LOCALPREF %d",p_policy->local_pref);
            if(apply_path_flag = 0)
            {
                p_apply_path = bgp4_policy_path_create(p_route,BGP4_POLICY_IMPORT_DIRECTION);
                if(p_apply_path)
                {
                    apply_path_flag = 1;                
                }
            }
            p_apply_path->out_localpref = p_policy->local_pref;
        }

        /*apply origin*/
        if(BIT_LST_TST(&p_policy->bgp_com.set_flag,ROUTRPOLICY_APPLY_ORIGINTYPE))
        {
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route change by policy ROUTRPOLICY_APPLY_ORIGINTYPE type %d,as %d",
                        p_policy->origin_type,p_policy->origin_value);
            if(apply_path_flag == 0)
            {
                p_apply_path = bgp4_policy_path_create(p_route,BGP4_POLICY_IMPORT_DIRECTION);
                if(p_apply_path)
                {
                    apply_path_flag = 1;
                }
            }
            if(p_apply_path)
            {
                p_route->p_path->flags.set_origin = 1;
            
                if(p_policy->origin_type == APPLY_BGP_EGP)
                {
                    p_route->p_path->origin = BGP4_ORIGIN_EGP;
                    if(p_policy->origin_value)/*add as num*/
                    {
                        tBGP4_ASPATH * p_aspath = NULL;
                        u_char* p_as = (u_char*)&p_policy->origin_value;
                    
                        bgp4_apath_list_free(p_apply_path);
                    
                        p_aspath = (tBGP4_ASPATH*)bgp4_malloc(sizeof(tBGP4_ASPATH),MEM_BGP_ASPATH);
                        if (p_aspath == NULL) 
                        {
                                bgp4_log(BGP_DEBUG_CMN,1,"bgp4 check route change by policy not enough memory for feasiable list!!");
                                return ;
                        }
                        p_aspath->p_asseg = (u_char*)bgp4_malloc(2, MEM_BGP_BUF);            
                        if (p_aspath->p_asseg)
                        {
                                memcpy(p_aspath->p_asseg, p_as+2,2);
                            p_aspath->len = 1;/*as count*/
                            p_aspath->type = BGP_ASPATH_SEQ;
                        }
                        else
                        {
                            bgp4_log(BGP_DEBUG_CMN,1,"bgp4 check route change by policy bgp4 malloc as path failed!");
                            bgp4_free(p_aspath, MEM_BGP_ASPATH);
                            return ;
                        }
                    
                        bgp4_lstadd(&p_apply_path->aspath_list, &p_aspath->node);

                    }
                }
                else if (p_policy->origin_type == APPLY_BGP_IGP)
                {
                    p_apply_path->origin = BGP4_ORIGIN_IGP;
                }
                else if (p_policy->origin_type == APPLY_BGP_INCOMPLETE)
                {
                    p_apply_path->origin = BGP4_ORIGIN_INCOMPLETE;
                }
            }
        }
    }

    if(apply_path_flag == 1)
    {
        p_true_path = bgp4_path_lookup(p_apply_path);
        if((p_true_path)&&(p_apply_path != p_true_path))
        {
            bgp4_lstdelete(&p_apply_path->route_list,&p_route->node);
            bgp4_link_path(p_true_path,p_route);
            bgp4_path_free(p_apply_path);
        }
    }
    
    
    return;
}

u_char bgp4_if_route_match_policy(tBGP4_POLICY_CONFIG* p_policy,tBGP4_ROUTE* p_route,u_int direction)
{   
    if(p_policy->status == SNMP_ACTIVE &&
        (p_policy->apply_protocol == 0 || p_route->proto == p_policy->apply_protocol) &&/*[p_policy->apply_protocol == 0] means match protocols all*/ 
        (p_route->dest.afi == p_policy->af))
    {
        return 1;
    }

    return 0;
}

/*check if config router filter,return BGP4_ROUTE_PERMIT or BGP4_ROUTE_DENY */
u_int bgp4_check_route_policy(tBGP4_ROUTE* p_route,
                    tBGP4_PEER* p_peer,u_int direction)
{
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    struct bgpPolicyEntry rt_policy;
    u_int rc = BGP4_ROUTE_PERMIT;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_LIST* p_global_rt_list = NULL;
    tBGP4_LIST* p_peer_rt_list = NULL;

    if(p_route == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy input route null");
        return BGP4_ROUTE_DENY;
    }

    if(p_route->p_path == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy input route PATH null");
        return BGP4_ROUTE_PERMIT;
    }

    if(gBgp4.routePolicyFunc == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy routePolicyFunc is NULL");
        return BGP4_ROUTE_PERMIT;
    }

    if(p_peer == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy peer is NULL");
        return BGP4_ROUTE_PERMIT;
    }
    
    p_instance = p_peer->p_instance;
    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy instance is NULL");
        return BGP4_ROUTE_PERMIT;
    }


    if(direction == BGP4_POLICY_IMPORT_DIRECTION)
    {
        /*check some semantic for import routes only*/
        if((p_route->p_path->p_peer) &&
            p_route->dest.afi == BGP4_PF_IP6UCAST &&
            (memcmp(p_route->p_path->p_peer->local.ip.ip, p_route->p_path->nexthop_global.ip, 16)==0))
        {
            u_char buf[64];
            u_char buf1[64];
            memset(buf,0,64);
            memset(buf,0,64);

            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy ipv6 route next hop invalid,peer local ip %s,nexthop global ip %s!",
                        bgp4_printf_addr(&p_route->p_path->p_peer->local.ip,buf),
                        bgp4_printf_addr(&p_route->p_path->nexthop_global,buf1));

            bgp4_end_peer_route (p_route->dest.afi, p_peer,0,1);
            
            /*For the duration of the BGP session over which the Update message 
            was received, the speaker then SHOULD ignore all the subsequent routes 
            with that AFI/SAFI received over that session.
            */
            af_clear(p_peer->remote.capability,p_route->dest.afi);
            
            return BGP4_ROUTE_DENY;
        }
            
        /*if local capability or remote capability do not support that afi,we should
        not insert route */
        if(!af_isset(p_peer->remote.capability,p_route->dest.afi))
        {
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy peer remote afi do not support such afi %d(route)/%d(peer),deny directly!",
                p_route->dest.afi,p_peer->remote.capability);
            return BGP4_ROUTE_DENY;
        }
        if(!af_isset(p_peer->local.capability,p_route->dest.afi))
        {   
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy peer local afi do not support such afi %d(route)/%d(peer),deny directly!",
                p_route->dest.afi,p_peer->local.capability);
            return BGP4_ROUTE_DENY;
        }
        
        /*multicast address && class E*/
        if(p_route->dest.afi == BGP4_PF_IPUCAST)
        {
             
             if(bgp_ip4(p_route->dest.ip) >= 0xE0000000)
             {
                bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy multicast address and class E,deny directly!");
                return BGP4_ROUTE_DENY;
             }
             if(bgp_ip4(p_route->dest.ip) == 0x7F000001)
             {
                bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy loopback address should be filtered directly!");
                return BGP4_ROUTE_DENY;
             }
            /*ignore routes including invalid nexthop*/
            /*check if next hop is reachable*/
            if(direction == BGP4_POLICY_IMPORT_DIRECTION &&
                    bgp4_nexthop_is_reachable(p_instance->instance_id,&p_route->p_path->nexthop) == VOS_ERR)
            {
                bgp4_log(BGP_DEBUG_EVT, 1,"nexthop is unreachable");  
                return BGP4_ROUTE_DENY;
            }
        }
        else if(p_route->dest.afi == BGP4_PF_IP6UCAST)
        {
            /*ignore routes including invalid nexthop*/
            /*check if next hop is reachable*/
            if(direction == BGP4_POLICY_IMPORT_DIRECTION &&
                bgp4_nexthop_is_reachable(p_instance->instance_id,&p_route->p_path->nexthop_global) == VOS_ERR)
            {
                bgp4_log(BGP_DEBUG_EVT, 1,"nexthop is unreachable");  
                return BGP4_ROUTE_DENY;
            }
        }
        
        p_global_rt_list = &p_instance->route_policy_import;
        p_peer_rt_list = &p_peer->rt_policy_import;
    }
    else
    {
        p_global_rt_list = &p_instance->route_policy_export;
        p_peer_rt_list = &p_peer->rt_policy_export;
    }
    
    /*first check global filter*/
    LST_LOOP(p_global_rt_list, p_policy, node, tBGP4_POLICY_CONFIG)
    {
        if(bgp4_if_route_match_policy(p_policy,p_route,direction))
        {
            if(p_policy->policy_index == 0)
            {
                bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy policy index is 0,invalid");
                return BGP4_ROUTE_PERMIT;
            }
            
            
            bgp4_fill_route_policy(&rt_policy,p_route);
            
            /*call route policy apply func*/
            if(gBgp4.routePolicyFunc(p_policy->policy_index,&rt_policy) != RPOLICY_PERMIT)
            {
                rc = BGP4_ROUTE_DENY;
                bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy route is denied by policy %d",
                    p_policy->policy_index);
                return rc;
            }
            else/*if permit,check if need modify*/
            {
                bgp4_check_route_change_by_policy(&rt_policy,p_route,direction);
            }
        }
    }

    /*check peer filter*/
    p_policy = NULL;
    LST_LOOP(p_peer_rt_list, p_policy, node, tBGP4_POLICY_CONFIG)
    {
        if(bgp4_if_route_match_policy(p_policy,p_route,direction))
        {
            if(p_policy->policy_index == 0)
            {
                bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy policy index is 0,invalid");
                return BGP4_ROUTE_PERMIT;
            }
            
            bgp4_fill_route_policy(&rt_policy,p_route);
            
            /*call route policy apply func*/
            if(gBgp4.routePolicyFunc(p_policy->policy_index,&rt_policy) != RPOLICY_PERMIT)
            {
                rc = BGP4_ROUTE_DENY;
                bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check route policy route is denied by policy %d",
                    p_policy->policy_index);
                return rc;
            }
            else/*if permit,check if need modify*/
            {
                bgp4_check_route_change_by_policy(&rt_policy,p_route,direction);
            }
        }
    }

    return rc;
    
}


/*check if config router filter,return BGP4_ROUTE_PERMIT or BGP4_ROUTE_DENY */
u_int bgp4_check_import_policy(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE* p_route)
{
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    struct bgpPolicyEntry rt_policy;
    u_int rc = BGP4_ROUTE_DENY;

    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check import policy input instance null");
        return BGP4_ROUTE_DENY;
    }

    if(p_route == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check import policy input route null");
        return BGP4_ROUTE_DENY;
    }
    
    if(p_route->p_path == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check import policy input route path null");
        return BGP4_ROUTE_DENY;
    }
#if 0/*need?*/
    if(p_route->proto == M2_ipRouteProto_bgp)
    {
        return BGP4_ROUTE_PERMIT;
    }
#endif

    if(p_route->dest.afi == BGP4_PF_IPUCAST)
    {
         
         if(bgp_ip4(p_route->dest.ip) >= 0xE0000000)
         {
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check import policy multicast address and class E,deny directly!");
            return BGP4_ROUTE_DENY;
         }
         if(bgp_ip4(p_route->dest.ip) == 0x7F000001)
         {
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check import policy loopback address should be filtered directly!");
            return BGP4_ROUTE_DENY;
         }
    }

    memset(&rt_policy,0,sizeof(struct bgpPolicyEntry));

    /*first check global filter*/
    LST_LOOP(&p_instance->import_policy, p_policy, node, tBGP4_POLICY_CONFIG)
    {
        if(bgp4_if_route_match_policy(p_policy,p_route,0))
        {
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check import policy find a match import route policy,police index %d",p_policy->policy_index);
            if(p_policy->policy_index == 0)/*invalid policy,apply med if need*/
            {
                rc = BGP4_ROUTE_PERMIT;
                if(p_policy->med)
                {
                    p_route->p_path->out_med = p_policy->med;
                }
                
            }
            else
            {
                if(gBgp4.routePolicyFunc == NULL)
                {
                    bgp4_log(BGP_DEBUG_EVT,1,"bgp4 check import policy routePolicyFunc is NULL");
                    return BGP4_ROUTE_DENY;
                }

                bgp4_fill_route_policy(&rt_policy,p_route);
                /*call route policy apply func*/
                if(gBgp4.routePolicyFunc(p_policy->policy_index,&rt_policy) != RPOLICY_PERMIT)
                {
                    rc = BGP4_ROUTE_DENY;
                    return rc;
                }
                else/*if permit,check if need modify*/
                {
                    rc = BGP4_ROUTE_PERMIT;
                    bgp4_check_route_change_by_policy(&rt_policy,p_route,NULL);
                }
            }
            
        }
    }

    return rc;
    
}




#endif
