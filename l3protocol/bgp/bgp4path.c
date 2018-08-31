

#include "bgp4peer.h"
#include "bgp4path.h"
#ifdef NEW_BGP_WANTED
#include "bgp4com.h"


u_short 
bgp4_as4path_fill(
      u_char *p_msg, 
      tBGP4_PATH *p_path,
      tBGP4_PEER *p_peer);

u_short 
bgp4_as4path_fill_2(
      u_char *p_msg, 
      tBGP4_PATH *p_path,
      tBGP4_PEER *p_peer);


/*create a path node*/
tBGP4_PATH *
bgp4_path_create(
          tBGP4_VPN_INSTANCE *p_instance, 
          u_int af)
{
    tBGP4_PATH *p_path = NULL;

    if (p_instance->rib[af] == NULL)
    {
        return NULL;
    }

    p_path = bgp4_malloc(sizeof(tBGP4_PATH), MEM_BGP_INFO);
    if (p_path == NULL)
    {
        return NULL;
    }
    bgp4_path_init(p_path);

    p_path->af = af;
    p_path->p_instance = p_instance;

    /*create direct nexthop*/
    p_path->p_direct_nexthop = bgp4_malloc(sizeof(tBGP4_NEXTHOP_BLOCK), MEM_BGP_NEXTHOP);
    
    /*insert path table direclty*/
    bgp4_avl_add(&p_instance->rib[af]->path_table, p_path);

    /*start timer*/
    bgp4_timer_start(&p_instance->rib[af]->path_timer, BGP4_PATH_CLEAR_INTERVAL);

    bgp4_timer_start(&p_instance->rib[af]->nexthop_timer, BGP4_IGP_SYNC_MAX_INTERVAL); 
    
    return p_path;
}

/*free a path*/
void 
bgp4_path_free(tBGP4_PATH *p_path)
{
    bgp4_path_clear(p_path);
    memset(p_path, 0, sizeof(*p_path));
    bgp4_free(p_path, MEM_BGP_INFO);
    return;
}

/*lookup same path*/
tBGP4_PATH *
bgp4_path_lookup(tBGP4_PATH *p_path)
{
    tBGP4_PATH *p_current = NULL;
    tBGP4_VPN_INSTANCE *p_instance = p_path->p_instance;
    u_int af = p_path->af;
    
    if ((af >= BGP4_PF_MAX) 
        || (p_instance == NULL))
    {
        return NULL;
    }
    if (p_instance->rib[af] == NULL)
    {
        return NULL;
    }
    bgp4_avl_for_each(&p_instance->rib[af]->path_table, p_current)
    {
        if (bgp4_path_same(p_current, p_path) == TRUE)
        {
            return p_current;
        }
    }
    return NULL;
}

/*check if any path has no route attached,delete these paths*/
void 
bgp4_path_garbage_collect(tBGP4_RIB *p_rib)
{
    tBGP4_PATH *p_path = NULL;
    tBGP4_PATH *p_next = NULL;

    bgp4_avl_for_each_safe(&p_rib->path_table, p_path, p_next)
    {
        if (!bgp4_avl_first(&p_path->route_list))
        {
            bgp4_avl_delete(&p_rib->path_table, p_path);
    
            bgp4_path_free(p_path);
        }
    }
    return;
}

/*copy path from source to dest*/
void 
bgp4_path_copy(
       tBGP4_PATH *p_dest,
       tBGP4_PATH *p_src)
{
    tBGP4_ASPATH *p_aspath = NULL;
    tBGP4_ASPATH *p_src_aspath = NULL;
    
    if ((p_src == NULL) || (p_dest == NULL))
    {
        return;
    }

    p_dest->med_exist = p_src->med_exist;

    p_dest->local_pref_exist = p_src->local_pref_exist;

    p_dest->community_no_adv = p_src->community_no_adv;
    p_dest->community_no_export = p_src->community_no_export;
    p_dest->community_no_subconfed = p_src->community_no_subconfed;
    
    p_dest->atomic_exist = p_src->atomic_exist; 
    p_dest->excommunity_len = p_src->excommunity_len;
    
    p_dest->p_peer = NULL;
    p_dest->origin = p_src->origin;
    memcpy(&p_dest->nexthop, &p_src->nexthop, sizeof(tBGP4_ADDR));
    memcpy(&p_dest->linklocal_nexthop, &p_src->linklocal_nexthop, sizeof(tBGP4_ADDR));
    p_dest->med = p_src->med; 
    p_dest->localpref = p_src->localpref; 

    bgp4_avl_for_each(&p_src->aspath_list, p_src_aspath)
    {
        p_aspath = bgp4_malloc(sizeof(tBGP4_ASPATH) + 
                    (p_src_aspath->count * 2), MEM_BGP_ASPATH);
        if (p_aspath == NULL)
        {
            continue;
        }
        p_aspath->type = p_src_aspath->type;
        p_aspath->count = p_src_aspath->count;
        memcpy(p_aspath->as, p_src_aspath->as, (p_src_aspath->count * 2));
        bgp4_avl_add(&p_dest->aspath_list, p_aspath);
    }

    /*copy 4bytes aspath*/
    bgp4_avl_for_each(&p_src->as4path_list, p_src_aspath)
    {
        p_aspath = bgp4_malloc(sizeof(tBGP4_ASPATH) + 
                    (p_src_aspath->count * 4), MEM_BGP_ASPATH);
        if (p_aspath == NULL)
        {
            continue;
        }
        p_aspath->type = p_src_aspath->type;
        p_aspath->count = p_src_aspath->count;
        memcpy(p_aspath->as, p_src_aspath->as, (p_src_aspath->count * 4));
        bgp4_avl_add(&p_dest->as4path_list, p_aspath);
    }
    
    /*copy comunity list*/
    if (p_src->p_community && p_src->community_len)
    {
        p_dest->p_community = bgp4_malloc(p_src->community_len, MEM_BGP_BUF);
        if (p_dest->p_community)
        {
            memcpy(p_dest->p_community, p_src->p_community, p_src->community_len);
            p_dest->community_len = p_src->community_len;
        }        
    }

    if (p_src->p_ecommunity && p_src->excommunity_len)
    {
        p_dest->p_ecommunity = bgp4_malloc(p_src->excommunity_len, MEM_BGP_BUF);
        if (p_dest->p_ecommunity)
        {
            memcpy(p_dest->p_ecommunity, p_src->p_ecommunity, p_src->excommunity_len);
            p_dest->excommunity_len = p_src->excommunity_len;
        }        
    }
    if (p_src->p_aggregator)
    {
        p_dest->p_aggregator = bgp4_malloc(sizeof(*p_dest->p_aggregator), MEM_BGP_BUF);
        if (p_dest->p_aggregator)
        {
            memcpy(p_dest->p_aggregator, p_src->p_aggregator, sizeof(*p_dest->p_aggregator));
        }
    }
    
    if (p_src->p_unkown && p_src->unknown_len)
    {
        p_dest->p_unkown = bgp4_malloc(p_src->unknown_len, MEM_BGP_BUF);
        if (p_dest->p_unkown)
        {
            memcpy(p_dest->p_unkown, p_src->p_unkown, p_src->unknown_len);
            p_dest->unknown_len = p_src->unknown_len;
        }
    }
    if (p_src->p_cluster && p_src->cluster_len)
    {
        p_dest->p_cluster = bgp4_malloc(p_src->cluster_len, MEM_BGP_BUF);
        if (p_dest->p_cluster)
        {
            memcpy(p_dest->p_cluster, p_src->p_cluster, p_src->cluster_len);
            p_dest->cluster_len = p_src->cluster_len;
        }
    }

    p_dest->origin_id = p_src->origin_id;
    p_dest->p_instance = p_src->p_instance;
    p_dest->af = p_src->af;
    p_dest->p_peer = p_src->p_peer;
    p_dest->origin_vrf = p_src->origin_vrf;    

    /*calculate new nexthop*/   
    bgp4_direct_nexthop_calculate(p_dest);

    return;
}

/*decide if two path is same*/
int  
bgp4_aspath_same(
        tBGP4_PATH *p1,
        tBGP4_PATH *p2)
{
    tBGP4_ASPATH *p_aspath1 = NULL;
    tBGP4_ASPATH *p_aspath2 = NULL;

    if ((p1 == NULL)||(p2 == NULL))
    {
        return FALSE;
    }

    if (bgp4_avl_count(&p1->aspath_list) 
        != bgp4_avl_count(&p2->aspath_list))
    {
        return FALSE;
    }

    p_aspath1 = bgp4_avl_first(&p1->aspath_list);
    p_aspath2 = bgp4_avl_first(&p2->aspath_list);

    while ((p_aspath1 != NULL) && (p_aspath2 != NULL))
    {
        if ((p_aspath1->type != p_aspath2->type) 
            || (p_aspath1->count != p_aspath2->count))
        {
            return FALSE;
        }
        if (memcmp(p_aspath1->as, p_aspath2->as, (p_aspath1->count * 2)) != 0)
        {
            return FALSE;
        }
        p_aspath1 = bgp4_avl_next(&p1->aspath_list, p_aspath1);
        p_aspath2 = bgp4_avl_next(&p2->aspath_list, p_aspath2);
    }

    /* Paths are identical. Entire ASPATH has been checked. */
    if ((p_aspath1 == NULL) && (p_aspath2 == NULL))
    {
        return TRUE;
    }
    return FALSE;
}

/*decide if two as4 path is same*/
int  
bgp4_as4path_same(
        tBGP4_PATH *p1,
        tBGP4_PATH *p2)
{
    tBGP4_ASPATH *p_aspath1 = NULL;
    tBGP4_ASPATH *p_aspath2 = NULL;

    if ((p1 == NULL)||(p2 == NULL))
    {
        return FALSE;
    }

    if (bgp4_avl_count(&p1->as4path_list) 
        != bgp4_avl_count(&p2->as4path_list))
    {
        return FALSE;
    }

    p_aspath1 = bgp4_avl_first(&p1->as4path_list);
    p_aspath2 = bgp4_avl_first(&p2->as4path_list);

    while ((p_aspath1 != NULL) && (p_aspath2 != NULL))
    {
        if ((p_aspath1->type != p_aspath2->type) 
            || (p_aspath1->count != p_aspath2->count))
        {
            return FALSE;
        }
        if (memcmp(p_aspath1->as, p_aspath2->as, (p_aspath1->count * 4)) != 0)
        {
            return FALSE;
        }
        p_aspath1 = bgp4_avl_next(&p1->as4path_list, p_aspath1);
        p_aspath2 = bgp4_avl_next(&p2->as4path_list, p_aspath2);
    }

    /* Paths are identical. Entire ASPATH has been checked. */
    if ((p_aspath1 == NULL) && (p_aspath2 == NULL))
    {
        return TRUE;
    }
    return FALSE;
}

/*calculate aspath attribute length*/
u_short 
bgp4_path_aspath_len(tBGP4_PATH *p_path)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_short len = 0;
    
    bgp4_avl_for_each(&p_path->aspath_list, p_aspath)
    {
        len += p_aspath->count;
    }
    return len;
}

/*decide if two path is same*/
int
bgp4_path_same(
        tBGP4_PATH *p1,
        tBGP4_PATH *p2)
{
    if ((p1 == NULL) || (p2 == NULL))
    {
        return FALSE;
    }
    if (p1->p_instance != p2->p_instance)
    {
        return FALSE;
    }
    if (p1->p_peer != p2->p_peer)
    {
        return FALSE;
    }

    if (p1->af != p2->af)
    {
        return FALSE;
    }

    if (p1->med_exist != p2->med_exist)
    {
        return FALSE;
    }
    if (p1->local_pref_exist != p2->local_pref_exist)
    {
        return FALSE;
    }
    if (p1->community_no_adv != p2->community_no_adv)
    {
        return FALSE;
    }
    if (p1->community_no_export != p2->community_no_export)
    {
        return FALSE;
    }
    if (p1->community_no_subconfed != p2->community_no_subconfed)
    {
        return FALSE;
    }

    if (p1->atomic_exist != p2->atomic_exist)
    {
        return FALSE;
    } 
    if (p1->excommunity_len != p2->excommunity_len)
    {
        return FALSE;
    }
    if (p1->community_len != p2->community_len)
    {
        return FALSE;
    }

    if (p1->origin != p2->origin)
    {
        return FALSE;
    }
    if (bgp4_aspath_same(p1, p2) == FALSE)
    {
        return FALSE;
    }
    if (bgp4_as4path_same(p1, p2) == FALSE)
    {
        return FALSE;
    }

    if (memcmp(p1->nexthop.ip, p2->nexthop.ip, 
          sizeof(p1->nexthop.ip)) != 0)
    {
        return FALSE;
    }
    if (p1->med != p2->med)
    {
        return FALSE;
    }
    if (p1->localpref != p2->localpref)
    {
        return FALSE;
    }  
    
    if (p1->p_aggregator && p2->p_aggregator)
    {
        if (p1->p_aggregator->as != p2->p_aggregator->as)
        {
            return FALSE;
        }
        if (bgp4_prefixcmp(&p1->p_aggregator->ip, &p2->p_aggregator->ip) != 0)
        {
            return FALSE;
        }
    }
    if (memcmp(p1->nexthop.ip, p2->nexthop.ip, sizeof(p1->nexthop.ip)) != 0)
    {
        return FALSE;
    }
    if (memcmp(p1->linklocal_nexthop.ip, p2->linklocal_nexthop.ip, sizeof(p1->linklocal_nexthop.ip)) != 0)
    {
        return FALSE;
    }

    if (p1->p_ecommunity || p2->p_ecommunity)
    {
        if ((p1->p_ecommunity == NULL) || (p2->p_ecommunity == NULL))
        {
            return FALSE;
        }
        if (memcmp(p1->p_ecommunity, p2->p_ecommunity, 
             p1->excommunity_len))
        {
            return FALSE;
        }
    }
    if (p1->p_community && p2->p_community)
    {
        if (memcmp(p1->p_community, p2->p_community, p1->community_len))
        {
            return FALSE;
        }
    }
    return TRUE;
}

/*merge same path for space reduction*/
tBGP4_PATH *
bgp4_path_add(tBGP4_PATH *p_path)
{
    tBGP4_PATH *p_new = NULL;
    if (p_path == NULL)
    {
        return NULL;
    }

    p_new = bgp4_path_lookup(p_path);
    if (p_new == NULL)
    {
        p_new = bgp4_path_create(p_path->p_instance, p_path->af);
        if (p_new == NULL)
        {
            return NULL;
        }
        bgp4_path_copy(p_new, p_path);
    }
    return p_new;
}

/*free aspath of path*/
void 
bgp4_apath_list_free(tBGP4_PATH *p_path)
{
    tBGP4_ASPATH *p_aspath = NULL;
    tBGP4_ASPATH *p_next = NULL;

    bgp4_avl_for_each_safe(&p_path->aspath_list, p_aspath, p_next)
    {
        bgp4_avl_delete(&p_path->aspath_list, p_aspath);
        bgp4_free(p_aspath, MEM_BGP_ASPATH);
    }

    bgp4_avl_for_each_safe(&p_path->as4path_list, p_aspath, p_next)
    {
        bgp4_avl_delete(&p_path->as4path_list, p_aspath);
        bgp4_free(p_aspath, MEM_BGP_ASPATH);
    }
    return;
}

/*substitute for a specific AS num in a aspath list*/
u_int 
bgp4_asnum_rebuild(
       tBGP4_ASPATH *p_aspath, 
       u_char *p_buf,
       tBGP4_PEER *p_peer,
       u_char remove_flag)
{
    u_short *p_as = NULL;
    u_char i = 0;
    u_int len = 0;

    if ((p_aspath == NULL) 
        || (p_buf == NULL)
        || (p_peer == NULL))
    {
        return 0;
    }

    /*point to first AS num*/
    p_as = (u_short *)p_aspath->as;

    for (i = 0 ; i < p_aspath->count ; i++, p_as++ )
    {
        /*if substitute dst peer's AS num*/
        if (p_peer->as_substitute_enable 
            && (ntohs(*p_as) == p_peer->as))
        {
            bgp4_put_2bytes(p_buf, gbgp4.as);
            len += 2;
        }
        /*filter private as num*/
        else if (remove_flag && ntohs(*p_as) >= BGP4_PRIVATE_AS_MIN)
        {
            continue;
        }
        else
        {
            memcpy(p_buf, p_as, 2);
            len += 2;
            p_buf += 2;
        }
    }
    return len;
}

/*lookup as in a aspath list*/
u_int 
bgp4_as_exist_in_aspath(
       avl_tree_t *p_list, 
       u_short as)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_short *p_as = NULL;
    u_char i;

    bgp4_avl_for_each(p_list, p_aspath)
    {
        p_as = (u_short *)p_aspath->as;
        for (i = 0 ; i < p_aspath->count ; i++, p_as++ )
        {
            if (ntohs(*p_as) == as)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

u_int 
bgp4_as_first_in_aspath(
       avl_tree_t *p_list, 
       u_short *as)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_short *p_as = NULL;
    u_char i;

    bgp4_avl_for_each(p_list, p_aspath)
    {
        p_as = (u_short *)p_aspath->as;
        if(p_aspath->type == BGP_ASPATH_SEQ)
        {
            for (i = 0 ; i < p_aspath->count ; i++, p_as++ )
            {
                *as = ntohs(*p_as);
            }
        }
    }
    
    return TRUE;
}

/*lookup as loop times in a aspath list*/
u_int 
bgp4_as_loop_times(
        avl_tree_t *p_list, 
        u_short as)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_short *p_as = NULL;
    u_char i = 0;
    u_int loop_times = 0;

    bgp4_avl_for_each(p_list, p_aspath)
    {
        p_as = (u_short *)p_aspath->as;
        for (i = 0 ; i < p_aspath->count ; i++, p_as++ )
        {
            if (ntohs(*p_as) == as)
            {
                loop_times++ ;
            }
        }
    }
    return loop_times;
}

/*lookup as in a aspath list*/
u_int 
bgp4_asseq_if_exist_pub_as(avl_tree_t *p_list)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_short *p_as = NULL;
    u_char i;

    bgp4_avl_for_each(p_list, p_aspath)
    {
        p_as = (u_short *)p_aspath->as;
        for (i = 0 ; i < p_aspath->count ; i++, p_as++ )
        {
            if (ntohs(*p_as)  < BGP4_PRIVATE_AS_MIN)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*decide new aspath length for sending*/
u_short  
bgp4_aspath_len(
        tBGP4_PATH *p_path, 
        u_int peertype)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_short len = 0;
    u_char exist = FALSE;

    if ((p_path != NULL) && (bgp4_avl_first(&p_path->aspath_list)))
    {
        if (peertype == BGP4_EBGP)
        {
            /*contain normal as set/seq*/
            bgp4_avl_for_each(&p_path->aspath_list,p_aspath)
            {
                if ((p_aspath->type == BGP_ASPATH_CONFED_SET)
                    || (p_aspath->type == BGP_ASPATH_CONFED_SEQ))
                {
                    continue;
                }

                if ((p_aspath->type == BGP_ASPATH_SEQ) && (exist == FALSE))
                {
                    exist = TRUE;
                }
                len += 1 + 1;
                len += (p_aspath->count * 2 );
            }

            /*just add an as-num,or add a as-seq part*/
            if (exist == TRUE)
            {
                len += 2;
            }
            else
            {
                len += 4;
            }
        }
        else
        {
            /*include all current as-path*/
            bgp4_avl_for_each(&p_path->aspath_list,p_aspath)
            {
                /* Seg. type and length field */
                len += 1 + 1;

                /* AS path length is in 2 bytes (int) format */
                len += (p_aspath->count * 2 );
            }

            /*confederation peer:add as-num or seq*/
            if (peertype == BGP4_CONFEDBGP)
            {
                p_aspath = bgp4_avl_first(&p_path->aspath_list);
                if (p_aspath->type == BGP_ASPATH_CONFED_SEQ)
                {
                    len += 2;
                }
                else
                {
                    len += 4;
                }
            }
        }
    }
    else if (peertype != BGP4_IBGP)
    {
        /* Needs to be filled by us */
        len += 4;
    }
    return len;
}

/*decide if loop in cluster id*/
STATUS  
bgp4_cluster_loop_check(tBGP4_PATH *p_path)
{
    u_int i ;
    u_int *p_id = (u_int *)p_path->p_cluster;
    
    if (p_id == NULL)
    {
        return VOS_OK;
    }

    if (gbgp4.cluster_id == 0)
    {
        return VOS_OK;
    }
    
    for (i = 0; i < p_path->cluster_len ; i += sizeof(*p_id))
    {
        if (gbgp4.cluster_id == ntohl(*p_id))
        {
            return VOS_ERR;
        }
    }
    return VOS_OK;
}

/*decide loop using origin*/
STATUS  
bgp4_originid_loop_check(tBGP4_PATH *p_path)
{
    if (gbgp4.reflector_enable == TRUE)
    {
        return (p_path->origin_id == gbgp4.router_id) ? VOS_ERR : VOS_OK;
    }
    return VOS_OK;
}

u_int bgp4_as_loop_check(avl_tree_t *p_list, u_short as)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_short *p_as = NULL;
    u_char i;

    bgp4_avl_for_each(p_list, p_aspath)
    {
        p_as = (u_short *)p_aspath->as;
        for (i = 0 ; i < p_aspath->count ; i++, p_as++ )
        {
            if (ntohs(*p_as) == as && p_aspath->count > 1)
            {
                return TRUE;
            }
        }
    }
    return FALSE;    
}

/*verify rxd path,check if loop detected and update some attribute*/
STATUS 
bgp4_path_loop_check(tBGP4_PATH *p_path)
{
    tBGP4_PEER *p_peer = p_path->p_peer;
    u_int local_as = 0;

    if (p_peer == NULL)
    {
        return VOS_OK;
    }
    local_as = (p_peer->fake_as != gbgp4.as) ? p_peer->fake_as : gbgp4.as;

    /*loop check by aspath*/
    if (p_peer->allow_as_loop_times 
        && bgp4_as_loop_times(&p_path->aspath_list, local_as) > p_peer->allow_as_loop_times)
    {
        return VOS_ERR;
    }
    else if (bgp4_as_loop_check(&p_path->aspath_list, local_as) == TRUE)
    {
        return VOS_ERR;
    }

    /*loop check by origin and cluster*/
    if (bgp4_originid_loop_check(p_path) != VOS_OK)
    {
        return VOS_ERR;
    }
    if (bgp4_cluster_loop_check(p_path) != VOS_OK)
    {
        return VOS_ERR;
    }
    return VOS_OK;
}

tBGP4_ATTR_DESC desc[256];
#define DESC_INIT(x, fl, u, z, f) do{desc[x].flag = (fl); desc[x].unitlen = (u); desc[x].zerolen = (z); desc[x].fixlen = (f);}while(0)
void 
bgp4_attribute_desc_init(void)
{
    memset(desc, 0, sizeof(desc));

    DESC_INIT(BGP4_ATTR_ORIGIN, BGP4_ATTR_FLAG_TRANSIT, 0, 0, BGP4_ORIGIN_LEN);
    
    DESC_INIT(BGP4_ATTR_PATH, BGP4_ATTR_FLAG_TRANSIT, 0, 0, 0);

    DESC_INIT(BGP4_ATTR_NEXT_HOP, BGP4_ATTR_FLAG_TRANSIT, 0, 0, BGP4_NEXTHOP_LEN);

    DESC_INIT(BGP4_ATTR_MED, BGP4_ATTR_FLAG_OPT, 0, 0, BGP4_MED_LEN);

    DESC_INIT(BGP4_ATTR_LOCAL_PREF, BGP4_ATTR_FLAG_TRANSIT, 0, 0, BGP4_LPREF_LEN);

    DESC_INIT(BGP4_ATTR_ATOMIC_AGGR, BGP4_ATTR_FLAG_TRANSIT, 0, 1, 0);

    DESC_INIT(BGP4_ATTR_AGGREGATOR, BGP4_ATTR_FLAG_TRANSIT|BGP4_ATTR_FLAG_OPT, 0, 0, BGP4_AGGR_LEN);

    DESC_INIT(BGP4_ATTR_COMMUNITY, BGP4_ATTR_FLAG_TRANSIT|BGP4_ATTR_FLAG_OPT, 4, 0, 0);

    DESC_INIT(BGP4_ATTR_ORIGINATOR, BGP4_ATTR_FLAG_OPT, 0, 0, BGP4_ORIGINATOR_LEN);

    DESC_INIT(BGP4_ATTR_CLUSTERLIST, BGP4_ATTR_FLAG_OPT, 4, 0, 0);

    DESC_INIT(BGP4_ATTR_MP_NLRI, BGP4_ATTR_FLAG_OPT, 0,0,0);

    DESC_INIT(BGP4_ATTR_MP_UNREACH_NLRI, BGP4_ATTR_FLAG_OPT,0,0,0);

    DESC_INIT(BGP4_ATTR_EXT_COMMUNITY,BGP4_ATTR_FLAG_TRANSIT|BGP4_ATTR_FLAG_OPT,4,0,0);

    DESC_INIT(BGP4_ATTR_NEW_PATH, BGP4_ATTR_FLAG_TRANSIT|BGP4_ATTR_FLAG_OPT, 0,0,0);

    DESC_INIT(BGP4_ATTR_NEW_AGGREGATOR, BGP4_ATTR_FLAG_TRANSIT|BGP4_ATTR_FLAG_OPT, 0,0,0);

    return;
}

/*get attribute from msg*/
int 
bgp4_origin_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_ORIGIN]; 
    u_char hlen = p_packet->option_hlen[BGP4_ATTR_ORIGIN]; 
    u_short len = p_packet->option_len[BGP4_ATTR_ORIGIN]; 
    u_char val = 0;
    
    if (p_buf == NULL)
    {
        return VOS_OK;
    }
    
    val = p_buf[hlen];
    if ((val != BGP4_ORIGIN_IGP) &&
        (val != BGP4_ORIGIN_EGP) &&
        (val != BGP4_ORIGIN_INCOMPLETE))
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "this orign value %d is invalid", val);

        p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
        p_path->p_peer->notify.sub_code = BGP4_INVALID_ORIGIN;
        p_path->p_peer->notify.len = (len+hlen)%64;
        memcpy(p_path->p_peer->notify.data, p_buf, p_path->p_peer->notify.len);
        return VOS_ERR;
    }
    p_path->origin = val;
    return VOS_OK;
}

int 
bgp4_aspath_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_PATH];
    u_char hlen = p_packet->option_hlen[BGP4_ATTR_PATH]; 
    u_short len = p_packet->option_len[BGP4_ATTR_PATH]; 
    tBGP4_ASPATH * p_aspath = NULL;
    u_char type = 0;
    u_char count = 0;
    u_short read_len = 0;
    u_short segment_len = 0;
    
    if (p_buf == NULL)
    {
        return VOS_OK;
    }
    /*skip option hdr*/
    p_buf += hlen;

    for (; read_len < len ; 
           read_len += segment_len, 
           p_buf += segment_len)
    {
        type = *p_buf;
        count = *(p_buf + 1);

        /*segment has 2bytes hdr,and a list of as,each as has 2bytes*/
        segment_len = 2 + (count * 2);

        /*check length*/
        if ((read_len + segment_len) > len)
        {
            p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
            p_path->p_peer->notify.sub_code = BGP4_MALFORMED_AS_PATH;
            return VOS_ERR;
        }
        
        if ((type != BGP_ASPATH_SET) 
            && (type != BGP_ASPATH_SEQ)
            && (type != BGP_ASPATH_CONFED_SEQ)
            && (type != BGP_ASPATH_CONFED_SET))
        {
            p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
            p_path->p_peer->notify.sub_code = BGP4_MALFORMED_AS_PATH;
            return VOS_ERR;
        }
               
        p_aspath = bgp4_malloc(sizeof(tBGP4_ASPATH) + segment_len, MEM_BGP_ASPATH);
        if (p_aspath == NULL)
        {
            bgp4_log(BGP_DEBUG_CMN|BGP_DEBUG_ERROR, "not enough memory to allocate !!");
            return VOS_ERR;
        }

        bgp4_avl_add(&p_path->aspath_list, p_aspath);

        p_aspath->type = type;
        p_aspath->count = count;

        if (count != 0)
        {
            memcpy(p_aspath->as, p_buf + 2, count*2);
        }
    }
    return VOS_OK;
}

int 
bgp4_as4path_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = NULL;
    u_char hlen = 0; 
    u_short len = 0; 
    tBGP4_ASPATH * p_aspath = NULL;
    u_char type = 0;
    u_char count = 0;
    u_short read_len = 0;
    u_short segment_len = 0;

    /*decide option to be checked*/
    /*if both local and peer support 4bytes as,check as_path,
      else check as4_path*/
    if ((gbgp4.as4_enable && p_path->p_peer->as4_enable) 
        && (gbgp4.work_mode != BGP4_MODE_SLAVE))
    {
        p_buf = p_packet->p_option[BGP4_ATTR_PATH];
        hlen = p_packet->option_hlen[BGP4_ATTR_PATH]; 
        len = p_packet->option_len[BGP4_ATTR_PATH]; 
    }
    else
    {
        p_buf = p_packet->p_option[BGP4_ATTR_NEW_PATH];
        hlen = p_packet->option_hlen[BGP4_ATTR_NEW_PATH]; 
        len = p_packet->option_len[BGP4_ATTR_NEW_PATH]; 
    }
    if (p_buf == NULL)
    {
        return VOS_OK;
    }
    /*skip option hdr*/
    p_buf += hlen;

    for (; read_len < len ; 
           read_len += segment_len, 
           p_buf += segment_len)
    {
        type = *p_buf;
        count = *(p_buf + 1);

        /*segment has 2bytes hdr,and a list of as,each as has 4bytes*/
        segment_len = 2 + (count * 4);

        /*check length*/
        if ((read_len + segment_len) > len)
        {
            p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
            p_path->p_peer->notify.sub_code = BGP4_MALFORMED_AS_PATH;
            return VOS_ERR;
        }
        
        if ((type != BGP_ASPATH_SET) 
            && (type != BGP_ASPATH_SEQ)
            && (type != BGP_ASPATH_CONFED_SEQ)
            && (type != BGP_ASPATH_CONFED_SET))
        {
            p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
            p_path->p_peer->notify.sub_code = BGP4_MALFORMED_AS_PATH;
            return VOS_ERR;
        }
               
        p_aspath = bgp4_malloc(sizeof(tBGP4_ASPATH) + segment_len, MEM_BGP_ASPATH);
        if (p_aspath == NULL)
        {
            bgp4_log(BGP_DEBUG_CMN|BGP_DEBUG_ERROR, "not enough memory for to allocate !!");
            return VOS_ERR;
        }

        bgp4_avl_add(&p_path->as4path_list, p_aspath);

        p_aspath->type = type;
        p_aspath->count = count;

        if (count != 0)
        {
            memcpy(p_aspath->as, p_buf + 2, count*4);
        }
    }
    return VOS_OK;
}

STATUS 
bgp4_path_nexthop_verify(u_int nexhop)
{
    if ((nexhop == 0) 
        || (nexhop == 0xffffffff) 
        || (nexhop == 0xffffff00))
    {
        return VOS_ERR;
    }

    if (((nexhop & 0xff000000) == 0x7f000000)
        || ((nexhop & 0x000000FF) == 0xff))
    {
        return VOS_ERR;
    }

    if ((nexhop & 0xf8000000) == 0xf0000000)
    {
        return VOS_ERR;
    }

    if ((nexhop & 0xf8000000) == 0xf8000000)
    {
        return VOS_ERR;
    }
    return VOS_OK;
}

int 
bgp4_nexthop_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_NEXT_HOP]; 
    u_char hlen = p_packet->option_hlen[BGP4_ATTR_NEXT_HOP]; 
    u_short len = p_packet->option_len[BGP4_ATTR_NEXT_HOP]; 
    tBGP4_PEER *p_peer = p_path->p_peer;
    ZEBRA_ROUTE_MSG_T m2route = {0};
    u_int vrf = 0;
    u_int code = BGP4_UPDATE_MSG_ERR;
    u_int subcode = BGP4_INVALID_NEXTHOP;
    u_int nexthop = 0;
    u_char addr[16];
    u_char addr2[16];
    
    if (p_buf == NULL)
    {
        return VOS_OK;
    }
    bgp4_get_4bytes(nexthop, (p_buf + hlen));

    /*internal struct always use network order*/
    bgp_ip4(p_path->nexthop.ip) = htonl(nexthop);

    if (bgp4_path_nexthop_verify(nexthop) != VOS_OK)
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "invalid nexthop %s", inet_ntoa_1(addr, *(u_int*)p_path->nexthop.ip));
        goto SEND_ERROR;
    }

    p_path->nexthop.afi = BGP4_PF_IPUCAST;

    /* Check for Semantic correctness - Sharing the same subnet */
    if (((bgp4_peer_type(p_peer)) == BGP4_EBGP)
        && (p_peer->mhop_ebgp_enable == FALSE))
    {        
        vrf = p_path->origin_vrf;
        if (bgp4_sys_ifunit_to_prefixlen(
               p_peer->if_unit, 
               p_peer->local_ip.afi) == 32)
        {
            if (ip_route_match(vrf, 
                bgp_ip4(p_path->nexthop.ip), &m2route) != VOS_OK)
            {
                goto SEND_ERROR;
            }
        }
        else if (bgp4_subnet_match(&p_path->nexthop,
                     &p_peer->local_ip, 
                     p_peer->if_unit) == FALSE)
        {
            bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "nexthop %s not in same subnet as %s",
               bgp4_printf_addr(&p_path->nexthop, addr),
               bgp4_printf_addr(&p_peer->local_ip, addr2));
            
            goto SEND_ERROR;
        }
    }

    if ((*(u_int*)p_path->nexthop.ip == *(u_int*)p_peer->local_ip.ip))
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "nexthop is local address");
        return VOS_ERR;
    }
    return VOS_OK;

    SEND_ERROR :
        
    p_peer->notify.code = code;
    p_peer->notify.sub_code = subcode;
    p_peer->notify.len = (len + hlen)%64;
    memcpy(p_peer->notify.data, p_buf, p_peer->notify.len);
    return VOS_ERR;
}

int 
bgp4_med_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_MED]; 
    u_char hlen = p_packet->option_hlen[BGP4_ATTR_MED];  
    if (p_buf == NULL)
    {
        return VOS_OK;
    }
    bgp4_get_4bytes(p_path->med, p_buf + hlen);
    p_path->med_exist = TRUE; 
    return VOS_OK;
}

int 
bgp4_lpref_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_LOCAL_PREF]; 
    u_char hlen = p_packet->option_hlen[BGP4_ATTR_LOCAL_PREF]; 
    if (p_buf == NULL)
    {
        return VOS_OK;
    }

    bgp4_get_4bytes(p_path->localpref, p_buf+hlen);
    p_path->local_pref_exist = TRUE; 
    return VOS_OK;
}

int 
bgp4_atomic_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_ATOMIC_AGGR]; 
    if (p_buf == NULL)
    {
        return VOS_OK;
    }

    p_path->atomic_exist = TRUE;
    return VOS_OK;
}

int 
bgp4_aggregator_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_AGGREGATOR]; 
    u_char hlen = p_packet->option_hlen[BGP4_ATTR_AGGREGATOR]; 
    u_short len = p_packet->option_len[BGP4_ATTR_AGGREGATOR]; 
    u_short expect = BGP4_AGGR_LEN;
    u_int ip = 0;
    
    if (p_buf == NULL)
    {
        return VOS_OK;
    }
    if (expect == BGP4_AGGR_LEN)
    {
        if (p_path->p_aggregator == NULL)
        {
            p_path->p_aggregator = 
                bgp4_malloc(sizeof(*p_path->p_aggregator), MEM_BGP_BUF);
            if (p_path->p_aggregator)
            {
                bgp4_get_2bytes(p_path->p_aggregator->as, (p_buf + hlen));
                bgp4_get_4bytes(bgp_ip4(p_path->p_aggregator->ip.ip), (p_buf + hlen + 2));
                ip = bgp_ip4(p_path->p_aggregator->ip.ip);
            }
        }
    }

    if ((ip == 0) || (ip == 0xFFFFFFFF) || (ip == 0x7F000001))
    {
        p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
        p_path->p_peer->notify.sub_code = BGP4_OPTIONAL_ATTR_ERR;
        p_path->p_peer->notify.len = (len + hlen)%64;
        memcpy(p_path->p_peer->notify.data, p_buf, p_path->p_peer->notify.len);
        return VOS_ERR;
    }
    return VOS_OK;
}

int 
bgp4_originator_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_ORIGINATOR]; 
    u_char hlen = p_packet->option_hlen[BGP4_ATTR_ORIGINATOR]; 
    u_short len = p_packet->option_len[BGP4_ATTR_ORIGINATOR]; 
    if (p_buf == NULL)
    {
        return VOS_OK;
    }

    bgp4_get_4bytes(p_path->origin_id, (p_buf + hlen));
    if (p_path->origin_id == 0)
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "invalid  originator id is 0");
        
        p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
        p_path->p_peer->notify.sub_code = BGP4_OPTIONAL_ATTR_ERR;
        p_path->p_peer->notify.len = (len + hlen)%64;
        memcpy(p_path->p_peer->notify.data, p_buf, p_path->p_peer->notify.len);
        return VOS_ERR;
    }
    return VOS_OK;
}

int 
bgp4_cluster_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_CLUSTERLIST]; 
    u_char hlen = p_packet->option_hlen[BGP4_ATTR_CLUSTERLIST]; 
    u_short len = p_packet->option_len[BGP4_ATTR_CLUSTERLIST]; 
    if (p_buf == NULL)
    {
        return VOS_OK;
    }

    if (p_path->p_cluster)
    {
        return VOS_OK;
    }
    
    p_path->p_cluster = bgp4_malloc(len, MEM_BGP_BUF);
    if (p_path->p_cluster)
    {
        memcpy(p_path->p_cluster, p_buf + hlen, len);
        p_path->cluster_len = len;
    }
    return VOS_OK;
}

int 
bgp4_unkown_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
#if 0
    u_char flag = 0;
    u_int oldlen = 0;
    u_char *p_str = NULL;
    if (p_buf == NULL)
    {
        return VOS_OK;
    }

    flag = *p_buf;

    if (!(flag & BGP4_ATTR_FLAG_OPT))
    {
        p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
        p_path->p_peer->notify.sub_code = BGP4_UNRECOGNISED_WELLKNOWN_ATTR;
        p_path->p_peer->notify.len = len%64;
        memcpy(p_path->p_peer->notify.data, p_buf, p_path->p_peer->notify.len);
        return VOS_ERR;
    }

    /*set partial flag*/
    if (!(flag & BGP4_ATTR_FLAG_TRANSIT))
    {
        return VOS_OK;
    }

    *p_buf |= BGP4_ATTR_FLAG_PARTIAL;

    if (p_path->p_unkown != NULL)
    {
        oldlen = p_path->unknown_len;
        p_str = bgp4_malloc(oldlen + len + hlen, MEM_BGP_BUF);
        if (p_str == NULL)
        {
            return VOS_OK;
        }

        memcpy(p_str, p_path->p_unkown, oldlen);

        bgp4_free(p_path->p_unkown, MEM_BGP_BUF);
        p_path->p_unkown = p_str;
    }
    else
    {
        oldlen = 0;
        p_path->p_unkown = bgp4_malloc(len, MEM_BGP_BUF);
        if (p_path->p_unkown == NULL)
        {
            return VOS_OK;
        }
    }

    memcpy((p_path->p_unkown + oldlen), p_buf, len + hlen);
    p_path->unknown_len = oldlen + len + hlen;
#endif    
    return VOS_OK;
}

int 
bgp4_community_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_COMMUNITY]; 
    u_short len = p_packet->option_len[BGP4_ATTR_COMMUNITY]; 
    u_char i;
    u_int *p_id = NULL;
    
    if (p_buf == NULL)
    {
        return VOS_OK;
    }
    if (p_path->p_community)
    {
        return VOS_OK;
    }
    
    p_path->p_community = bgp4_malloc(len, MEM_BGP_BUF);
    if (p_path->p_community)
    {
        memcpy(p_path->p_community, p_buf + 3, len);
        p_path->community_len = len;

        for (i = 0, p_id = (u_int *)p_path->p_community; 
             i < len ; i++, p_id++) 
        {
            if (ntohl(*p_id) == BGP4_COMMUNITY_NOEXPORT)
            {
                p_path->community_no_export = TRUE;
            }
            else if (ntohl(*p_id) == BGP4_COMMUNITY_NOADV)
            {
                p_path->community_no_adv = TRUE;
            }
            else if (ntohl(*p_id) == BGP4_COMMUNITY_NOEXPORT_SUSPEND)
            {
                p_path->community_no_subconfed = TRUE;
            }
        }
    }
    return VOS_OK;
}

int 
bgp4_mpnexthop_extract(
               u_char len,
               u_char *p_buf,
               u_short afi,
               u_char safi,
               tBGP4_PATH *p_path)
{
    u_char nexthop[64];
    u_int af_flag = bgp4_afi_to_index(afi, safi);

    /*get nexthop*/
    if ((afi == BGP4_AF_IP) || (afi == BGP4_AF_L2VPN))
    {
        if (safi == BGP4_SAF_VLBL)
        {
            if (len != 12)
            {
                bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "bgp4 mpnexthop,invalid nexthop length %d",len);
                return VOS_ERR;
            }

            bgp4_get_4bytes(bgp_ip4(p_path->nexthop.ip), (p_buf + 8));
            bgp_ip4(p_path->nexthop.ip) = htonl(bgp_ip4(p_path->nexthop.ip));

            /*if nexthop is zero,use neighbor address*/
            if (*(u_int*)p_path->nexthop.ip == 0)
            {
                *(u_int*)p_path->nexthop.ip = 
                    *(u_int*)p_path->p_peer->ip.ip ;
            }
        }
        else
        {
            if (len != 4)
            {
                bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "bgp mpnexthop,invalid nexthop length %d",len);
                return VOS_ERR;
            }
            p_path->nexthop.afi = BGP4_PF_IPUCAST;
            bgp4_get_4bytes(bgp_ip4(p_path->nexthop.ip), p_buf);
            bgp_ip4(p_path->nexthop.ip) = htonl(bgp_ip4(p_path->nexthop.ip));
            
            /*if nexthop is zero,use neighbor address*/
            if (*(u_int*)p_path->nexthop.ip == 0)
            {
                *(u_int*)p_path->nexthop.ip =
                    *(u_int*)p_path->p_peer->ip.ip;
            }
        }
        bgp4_printf_addr(&p_path->nexthop, nexthop);
        bgp4_log(BGP_DEBUG_UPDATE, "bgp mpnexthop:%s", nexthop);
    }
    else/*BGP4_AF_IP6*/
    {
        if (safi == BGP4_SAF_VLBL)
        {
            if ((len != 24) && (len != 48))
            {
                bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "bgp mpnexthop,invalid nexthop length %d",len);
                return VOS_ERR;
            }

            /*8 zero RD offset*/
            memcpy(p_path->nexthop.ip, p_buf + 8, 16);

            p_path->nexthop.afi = af_flag;

            if (len == 48)
            {
                memcpy(p_path->linklocal_nexthop.ip, p_buf + 24, 16);
                p_path->linklocal_nexthop.afi = af_flag;
            }
            bgp4_log(BGP_DEBUG_UPDATE, "bgp ipv6 vpn mpnexthop %s", bgp4_printf_addr(&p_path->nexthop, nexthop));
        }
        else
        {
            if ((len != 16) && (len != 32))
            {
                bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "bgp mpnexthop,invalid nexthop length %d",len);
                return VOS_ERR;
            }
            /*v6 tbi*/
            memcpy(p_path->nexthop.ip, p_buf ,16);

            p_path->nexthop.afi = af_flag;

            if (len == 32)
            {
                memcpy(p_path->linklocal_nexthop.ip, p_buf + 16, 16);
                p_path->linklocal_nexthop.afi = af_flag;
            }
        }
    }
    return VOS_OK;
}


/*
     VPLS sample nlri format
      +------------------------------------+
      |  Length (2 octets)                 |--00 01 == 17
      +------------------------------------+
      |  Route Distinguisher  (8 octets)   | == 00 00 00 64 00 00 00 01 ---RD
      +------------------------------------+
      |  VE ID (2 octets)                  | 00 02 ID为2
      +------------------------------------+
      |  VE Block Offset (2 octets)        |00 00 Offset为0
      +------------------------------------+
      |  VE Block Size (2 octets)          |00 0a BLOCK
      +------------------------------------+
      |  Label Base (3 octets)             |18 6a 11 ---和普通标记封装一样，都是增加了4比特偏移
      +------------------------------------+
*/

int 
bgp4_vpls_nlri_extract(
                    tBGP4_PATH *p_path,
                    u_char *p_buf,
                    short len,
                    avl_tree_t *p_list)
{
    tBGP4_ROUTE *p_route = NULL;
    u_short nlri_len = 0;

    bgp4_get_2bytes(nlri_len, p_buf);
    p_buf += 2;

    /*length must checking*/
    if (nlri_len != BGP4_VPLS_NLRI_LEN)
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "invalid vpls nlri len %d", nlri_len);
        return 0;
    }

    p_route = bgp4_malloc(sizeof(tBGP4_ROUTE), MEM_BGP_ROUTE);
    if (p_route == NULL)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "failed to allocate  memory for route ");
        return 0;
    }

    bgp4_link_path(p_path, p_route);

    /*protocol must be BGP4*/
    p_route->proto = M2_ipRouteProto_bgp;
    
    /*copy buffer directly,no parse here*/
    p_route->dest.afi = BGP4_PF_L2VPLS;
    
    /*no prefixlen need*/
    memcpy(p_route->dest.ip, p_buf, BGP4_VPLS_NLRI_LEN);

    bgp4_route_table_add(p_list, p_route);
    return (nlri_len + 2);/*include 2bytes length field*/
}

int 
bgp4_mp_nlri_extract(
                    tBGP4_PATH *p_path,
                    u_char *p_buf,
                    short len,
                    avl_tree_t *p_list)
{
    u_char plen = *(p_buf);
    u_char bytes = 0;
    u_char *p_nlri = (p_buf+1);
    u_int flag = 0;
    u_char label_len = 0;
    u_char addr_len = 0;
    tBGP4_ROUTE *p_route = NULL;
    u_int mask  = 0;

    /*vpls format is different....*/
    bytes = bgp4_bit2byte(plen);

    if (len < (bytes + 1))
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "invalid attribute length");
        return 0;
    }

    /*check length,if valid*/
    flag = p_path->af;
#ifdef BGP_VPLS_WANTED

    if (flag == BGP4_PF_L2VPLS)
    {
        return bgp4_vpls_nlri_extract(p_path, p_buf, len, p_list);
    }
#endif
    addr_len = (bgp4_index_to_afi(flag) == BGP4_AF_IP) ? 4 : 16;

    if ((bgp4_index_to_safi(flag) == BGP4_SAF_LBL) 
        || (bgp4_index_to_safi(flag) == BGP4_SAF_VLBL))
    {
        label_len = 3;
    }

    if (bgp4_index_to_safi(flag) == BGP4_SAF_VLBL)
    {
        addr_len += 8;
    }

    if ((bytes < (label_len )) ||
        (bytes > (label_len + addr_len)))
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR,
            "invalid mpbgp nlri prefix length %d", plen);
        return 0;
    }

    p_route = bgp4_malloc(sizeof(tBGP4_ROUTE), MEM_BGP_ROUTE);
    if (p_route == NULL)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "failed to allocate route ");
        return 0;
    }

    bgp4_link_path (p_path, p_route);
    
    /*protocol must be BGP4*/
    p_route->proto = M2_ipRouteProto_bgp;

    /*fill infor*/
    p_route->dest.afi = flag;
    
    if (label_len != 0)
    {
        p_route->out_label = bgp4_label_extract(p_nlri);
        p_nlri += label_len;
    }

    p_route->dest.prefixlen = plen - 8*label_len;  /*prefix include RD*/

    if (addr_len == 4)
    {
        bgp4_get_4bytes(*(u_int*)p_route->dest.ip, p_nlri);
        
        mask = bgp4_len2mask(p_route->dest.prefixlen);
        
        *(u_int*)p_route->dest.ip &= mask ;

        bgp4_log(BGP_DEBUG_UPDATE, "mpbgp nlri:%d.%d.%d.%d/%d,label %d",
                    p_route->dest.ip[0],
                    p_route->dest.ip[1],
                    p_route->dest.ip[2],
                    p_route->dest.ip[3],
                    p_route->dest.prefixlen,
                    p_route->out_label);
    }
    else if (addr_len == 16)/*ipv6 route get*/
    {
        memset(p_route->dest.ip, 0, 16);
        memcpy(p_route->dest.ip, p_nlri, bytes);
        bgp4_ip6_prefix_make(p_route->dest.ip, p_route->dest.prefixlen);
    }
    else if (addr_len == 12 || addr_len == 24)/*mpls vpn*/
    {
        /*The Prefix field contains IP address prefixes followed by
        enough trailing bits to make the end of the field fall on an
        octet boundary. Note that the value of trailing bits is irrelevant.rfc 1771*/
        u_int byte = bgp4_bit2byte(p_route->dest.prefixlen);
        memset(p_route->dest.ip, 0, sizeof(p_route->dest.ip));
        memcpy(p_route->dest.ip, p_nlri, byte);

        if (addr_len == 12)
        {
            bgp4_log(BGP_DEBUG_UPDATE, "mpbgp nlri:%d.%d.%d.%d/%d,af %d,label %d",
                        *(p_nlri+BGP4_VPN_RD_LEN),*(p_nlri +BGP4_VPN_RD_LEN+1),*(p_nlri + BGP4_VPN_RD_LEN+2),
                        *(p_nlri + BGP4_VPN_RD_LEN+3),p_route->dest.prefixlen,
                        p_route->dest.afi,
                        p_route->out_label);
        }
    }

    bgp4_route_table_add(p_list, p_route);
    return (bytes + 1);
}

/*parse path attribute of mp-reach option,do not include any route*/
int 
bgp4_mpreach_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_MP_NLRI]; 
    u_char hlen = p_packet->option_hlen[BGP4_ATTR_MP_NLRI]; 
    u_short len = p_packet->option_len[BGP4_ATTR_MP_NLRI]; 
    u_char nhop_len = 0;
    u_short afi = 0;
    u_char safi = 0;
    u_int flag = 0;

    if (p_buf == NULL)
    {
        return VOS_OK;
    }
    p_buf += hlen;
    bgp4_log(BGP_DEBUG_UPDATE, "get mpbgp reach nlri,len %d", len);

    /*must include AFI and SAFI*/
    if (len < 3)
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "invalid length");
        return VOS_ERR;
    }
    len -= 3;

    bgp4_get_2bytes(afi, p_buf);
    p_buf += 2;
    safi = *p_buf;
    p_buf++;

    /*check if neighbor support AFI,if not,only ignore the optional non-transitive attribute*/
    flag = bgp4_afi_to_index(afi, safi);
    if (!bgp4_af_support(p_path->p_peer, flag))
    {
        bgp4_log(BGP_DEBUG_UPDATE,
            "neighbor not support afi/safi %d/%d", afi, safi);
        return VOS_OK;
    }
    /*next hop*/
    if (len < 1)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "no memory for nexthop");
        goto MP_ERR;
    }

    nhop_len = *p_buf;

    if (len < nhop_len)/*nexthop length of buffer*/
    {
        bgp4_log(BGP_DEBUG_UPDATE,
                "len %d less than nhop len %d", len, nhop_len);
        goto MP_ERR;
    }
    len--;
    p_buf++;

    p_path->af = flag;
    /*check if the next hop is valid*/
    if (bgp4_mpnexthop_extract(nhop_len, p_buf, afi, safi, p_path) != VOS_OK)
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "mpBgp nexthop is invalid");

        goto MP_ERR;
    }

    return VOS_OK;
MP_ERR :
    /*Send Notify*/
    
    p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
    p_path->p_peer->notify.sub_code = BGP4_OPTIONAL_ATTR_ERR;
    p_path->p_peer->notify.len = len%64;
    memcpy(p_path->p_peer->notify.data, p_buf, p_path->p_peer->notify.len);
    return VOS_ERR;    
}

/*only parse nlri,do not update path*/
int 
bgp4_mpreach_nlri_extract(
         tBGP4_PATH *p_path,
         tBGP4_RXUPDATE_PACKET *p_packet,
         avl_tree_t *p_list)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_MP_NLRI]; 
    u_char hlen = p_packet->option_hlen[BGP4_ATTR_MP_NLRI]; 
    u_short len = p_packet->option_len[BGP4_ATTR_MP_NLRI]; 
    u_char nhop_len = 0;
    u_short afi = 0;
    u_char safi = 0;
    u_short nlri_len = 0;
    u_int flag = 0;

    if (p_buf == NULL)
    {
        return VOS_OK;
    }

    p_buf += hlen;
    bgp4_log(BGP_DEBUG_UPDATE, "get mpBgp reach nlri,len %d", len);

    /*must include AFI and SAFI*/
    if (len < 3)
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "invalid length");
        return VOS_ERR;
    }
    len -= 3;

    bgp4_get_2bytes(afi, p_buf);
    p_buf += 2;
    safi = *p_buf;
    p_buf++;

    /*check if neighbor support AFI,if not,only ignore the optional non-transitive attribute*/
    flag = bgp4_afi_to_index(afi, safi);
    if (!bgp4_af_support(p_path->p_peer, flag))
    {
        bgp4_log(BGP_DEBUG_UPDATE,
            "neighbor not support afi/safi %d/%d", afi, safi);
        return VOS_OK;
    }
    
    /*next hop*/
    if (len < 1)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "no memory for nexthop");
        goto MP_ERR;
    }

    nhop_len = *p_buf;

    if (len < nhop_len)/*nexthop length of buffer*/
    {
        bgp4_log(BGP_DEBUG_UPDATE,
                "len %d less than nhop len %d", len, nhop_len);
        goto MP_ERR;
    }
    len--;
    p_buf++;

    p_path->af = bgp4_afi_to_index(afi, safi);

    p_buf += nhop_len;
    len -= nhop_len;

    /*snpa must be 0*/
    if (*p_buf != 0)
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "invalid snpa count");
        return VOS_ERR;
    }

    len--;
    p_buf++;

    /*get route*/
    while (len > 0)
    {
        nlri_len = bgp4_mp_nlri_extract(p_path, p_buf, len, p_list);
        if (nlri_len == 0)
        {
            bgp4_log(BGP_DEBUG_UPDATE, "failed to get mpBgp routes");

            goto MP_ERR;
        }
        len -= nlri_len;
        p_buf += nlri_len;
    }
    return VOS_OK;

MP_ERR :
    /*Send Notify*/
    
    p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
    p_path->p_peer->notify.sub_code = BGP4_OPTIONAL_ATTR_ERR;
    p_path->p_peer->notify.len = len%64;
    memcpy(p_path->p_peer->notify.data, p_buf, p_path->p_peer->notify.len);
    return VOS_ERR;
}

/*no nexthop in mp unreach,only af need*/
int 
bgp4_mpunreach_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_MP_UNREACH_NLRI]; 
    u_char hlen = p_packet->option_hlen[BGP4_ATTR_MP_UNREACH_NLRI]; 
    u_short len = p_packet->option_len[BGP4_ATTR_MP_UNREACH_NLRI]; 
    u_short afi = 0;
    u_char safi = 0;
    u_int flag = 0;
    if (p_buf == NULL)
    {
        return VOS_OK;
    }
    
    p_buf += hlen;
    bgp4_log(BGP_DEBUG_UPDATE, "proc mpBgp unreachable nlri attribute,length %d", len);
    
    /*must include AFI and SAFI*/
    if (len < 3)
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "invalid attribute length");
        return VOS_ERR;
    }
    len -= 3;

    bgp4_get_2bytes(afi, p_buf);
    p_buf += 2;

    safi = *p_buf;
    p_buf++;

    /*check if neighbor support AFI,if not,only ignore the optional non-transitive attribute*/
    flag = bgp4_afi_to_index(afi, safi);
    if (!bgp4_af_support(p_path->p_peer, flag))
    {
        bgp4_log(BGP_DEBUG_UPDATE, "neighbor not support afi/safi %d/%d", afi, safi);
        return VOS_OK;
    }
    p_path->af = flag;
    return VOS_OK;
}

int 
bgp4_mpunreach_nlri_extract(
        tBGP4_PATH *p_path,
        tBGP4_RXUPDATE_PACKET *p_packet,
        avl_tree_t *p_list)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_MP_UNREACH_NLRI]; 
    u_char hlen = p_packet->option_hlen[BGP4_ATTR_MP_UNREACH_NLRI]; 
    u_short len = p_packet->option_len[BGP4_ATTR_MP_UNREACH_NLRI]; 
    u_short afi = 0;
    u_char safi = 0;
    u_short route_len = 0;
    u_int flag = 0;

    if (p_buf == NULL)
    {
        return VOS_OK;
    }

    p_buf += hlen;
    bgp4_log(BGP_DEBUG_UPDATE, "proc mpBgp unreachable nlri attribute,length %d", len);

    /*must include AFI and SAFI*/
    if (len < 3)
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "invalid attribute length");
        return VOS_ERR;
    }

    len -= 3;

    bgp4_get_2bytes(afi, p_buf);
    p_buf += 2;

    safi = *p_buf;
    p_buf++;

    /*check if neighbor support AFI,if not,only ignore the optional non-transitive attribute*/
    flag = bgp4_afi_to_index(afi, safi);
    if (!bgp4_af_support(p_path->p_peer, flag))
    {
        bgp4_log(BGP_DEBUG_UPDATE, "neighbor not support afi/safi %d/%d", afi, safi);
        return VOS_OK;
    }

    p_path->af = bgp4_afi_to_index(afi, safi);
    /*get route*/
    while (len > 0)
    {
        route_len = bgp4_mp_nlri_extract(p_path, p_buf, len, p_list);
        if (route_len == 0)
        {
            goto MP_ERR;
        }
        len -= route_len;
        p_buf += route_len;
    }
    return VOS_OK;

MP_ERR :
    /*Clear Flag*/
    flag_clear(p_path->p_peer->mpbgp_capability, flag);

    /*Send Notify*/
    p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
    p_path->p_peer->notify.sub_code = BGP4_OPTIONAL_ATTR_ERR;
    p_path->p_peer->notify.len = len%64;
    memcpy(p_path->p_peer->notify.data, p_buf, p_path->p_peer->notify.len);    
    return VOS_ERR;
}

int 
bgp4_ecommunity_extract(
       tBGP4_PATH *p_path, 
       tBGP4_RXUPDATE_PACKET *p_packet)
{
    u_char *p_buf = p_packet->p_option[BGP4_ATTR_EXT_COMMUNITY]; 
    u_char hlen = p_packet->option_hlen[BGP4_ATTR_EXT_COMMUNITY]; 
    u_short len = p_packet->option_len[BGP4_ATTR_EXT_COMMUNITY]; 
    u_char unit_len = sizeof(tBGP4_ECOMMUNITY);
    
    if (p_buf == NULL)
    {
        return VOS_OK;
    }
    p_buf += hlen;

    bgp4_log(BGP_DEBUG_UPDATE, "get extract community,length %d", len);

    /*length must be 8*n bytes*/
    if (((len % unit_len) != 0) || (len < unit_len))
    {
        return VOS_ERR;
    }

    if (p_path->p_ecommunity)
    {
        return VOS_OK;
    }
    
    p_path->p_ecommunity = bgp4_malloc(len, MEM_BGP_BUF);
    if (p_path->p_ecommunity)
    {
        memcpy(p_path->p_ecommunity, p_buf, len);
        p_path->excommunity_len = len;   
    }
    return VOS_OK;
}
#if 0
/*fill attribute,not include mpreach and unreach*/
u_short 
bgp4_path_fill (
        tBGP4_PEER *p_peer,
        tBGP4_ROUTE *p_route,
        u_char *p_msg)
{
    tBGP4_PATH *p_save_path = p_route->p_path;
    tBGP4_PATH policy_path;
    u_int af = p_route->dest.afi;
    u_long msg_start = (u_long)p_msg;

    /*build new path according to policy*/
    memset(&policy_path, 0, sizeof(policy_path));
    bgp4_path_init(&policy_path);
    bgp4_path_copy(&policy_path, p_save_path);

    bgp4_export_route_policy_apply(p_route, p_peer, &policy_path);

    /*set route's path for attribtue filling*/
    p_route->p_path = &policy_path;

    p_msg += bgp4_origin_fill(p_msg, p_route);

    /*fill aspath*/
    /*if local node do not support 4bytes as,fill normal aspath*/
    if (gbgp4.as4_enable == FALSE)
    {
        p_msg += bgp4_aspath_fill(p_msg, &policy_path, p_peer);

        /*if as4path attribute exist,fill it without change*/
        p_msg += bgp4_as4path_fill(p_msg, &policy_path, p_peer);
    }
    else if (p_peer->as4_enable == FALSE)
    {
        /*merge aspath and as4path into 2byte aspath,
          and create 4bytes aspath*/
        p_msg += bgp4_aspath_fill(p_msg, &policy_path, p_peer);

        p_msg += bgp4_as4path_fill_2(p_msg, &policy_path, p_peer);
    }
    else
    {
        /*merge aspath and as4path into 4byte aspath*/
       p_msg += bgp4_aspath_fill_2(p_msg, &policy_path, p_peer);
    }
    
    /*  Filling the Next Hop ,in MBGP,do not send nexthop*/
    if (af == BGP4_PF_IPUCAST)
    {
        p_msg += bgp4_nexthop_fill(p_msg, p_route, p_peer);
    }

    p_msg +=  bgp4_med_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_lpref_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_aggregate_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_community_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_originid_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_cluster_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_ecommunity_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_unkown_fill(p_msg, &policy_path);

    /*recover original path*/
    p_route->p_path = p_save_path;

    /*release policy path*/
    bgp4_path_clear(&policy_path);
    return (((u_long)p_msg) - msg_start);
}
#endif
u_short 
bgp4_nlri_fill(
       u_char *p_msg,
       tBGP4_ROUTE *p_route)
{
    u_char bytes = 0;
    u_short len = 0;
    u_int prefix = 0;

    /*first byte is prefix length*/
    bgp4_put_char(p_msg, p_route->dest.prefixlen);
    len++;

    if (p_route->dest.prefixlen == 0)
    {
        return len;
    }
    bytes = bgp4_bit2byte(p_route->dest.prefixlen);
    prefix = ntohl(bgp_ip4(p_route->dest.ip));
    /*copy the whole 4 bytes,but increase length matched to prefix len*/
    bgp4_put_4bytes(p_msg, prefix);
    len += bytes;
    return len;
}

u_short 
bgp4_mp_nexthop_fill(
        tBGP4_PEER *p_peer ,
        u_char *p_msg,
        tBGP4_ROUTE *p_route)
{
    u_char hoplen = 0;
    u_char *p_nexthop = (p_msg + 1);
    tBGP4_PATH *p_path = NULL;
    u_int nhop = 0;
    u_char v6nhop[32];
    
    p_path = p_route->p_path;
    
    memset(v6nhop, 0, sizeof(v6nhop));

    /*fill for different nexthop type*/
    switch (p_route->dest.afi){
        case BGP4_PF_IPUCAST:
        case BGP4_PF_IPMCAST:
        case BGP4_PF_IPLABEL:
        case BGP4_PF_L2VPLS:
        {
             hoplen = 4;
 
             /*if bgp has no nexthop,use self BGP Id or local address.*/
             if (*(u_int*)p_path->nexthop.ip)
             {
                 nhop = ntohl(*(u_int*)p_path->nexthop.ip);
             }
             else if (*(u_int*)p_peer->local_ip.ip)
             {
                 nhop = ntohl(*(u_int*)p_peer->local_ip.ip);
             }
             else
             {
                 nhop = gbgp4.router_id;
             }
             break;
        }
        case BGP4_PF_IPVPN:
        {
             hoplen = 12;
 
             /*8 bytes 0 for RD*/
             if (p_msg)
             {
                 memset(p_nexthop, 0, 8);
                 p_nexthop += 8;
             }

             if ((*(u_int*)p_path->nexthop.ip ) 
                && ((gbgp4.reflector_enable == TRUE) 
                && (p_peer->is_reflector_client == TRUE))
                && (p_path->p_instance->vrf == p_path->origin_vrf))
             {
                 nhop = ntohl(bgp_ip4(p_path->nexthop.ip));
             }
             else if (*(u_int*)p_peer->local_ip.ip)
             {
                 nhop = ntohl(*(u_int*)p_peer->local_ip.ip);
             }
             else
             {
                 nhop = gbgp4.router_id;
             }
             break;
        }
        case BGP4_PF_IP6VPN:/*mpls l3 vpn only use ipv4 backbone network,so ipv4 nexthop is valid*/
        /*|                80 位                 | 16 |      32 位          |
        +--------------------------------------+--------------------------+
        |0000..............................0000|FFFF|    IPv4 地址     |
        +--------------------------------------+----+---------------------+*/
        {
            /*8 bytes 0 for RD*/
            memset(v6nhop, 0, 8);

            memset(v6nhop + 8, 0, 10);
            memset(v6nhop + 18, 0xff, 2);

            if ((*(u_int*)p_path->nexthop.ip ) 
                && ((gbgp4.reflector_enable == TRUE) 
                    && (p_peer->is_reflector_client == TRUE)) )
            {
                memcpy(v6nhop + 20, p_path->nexthop.ip, 4);
            }
            else if (*(u_int*)p_peer->local_ip.ip)
            {
                memcpy(v6nhop + 20, p_peer->local_ip.ip, 4);
            }
            else
            {
                memcpy(v6nhop + 20, &gbgp4.router_id, 4);
            }
            hoplen = 24;
            break;
        }
        case BGP4_PF_IP6LABEL:
        /*|                80 位                 | 16 |      32 位          |
        +--------------------------------------+--------------------------+
        |0000..............................0000|FFFF|    IPv4 地址     |
        +--------------------------------------+----+---------------------+*/
        {
            memset(v6nhop, 0, 10);
            memset(v6nhop + 10, 0xff, 2);

            if ((*(u_int*)p_path->nexthop.ip ) 
                && ((gbgp4.reflector_enable == TRUE) 
                && (p_peer->is_reflector_client == TRUE)) )
            {
                memcpy(v6nhop + 12, p_path->nexthop.ip, 4);
            }
            else if (*(u_int*)p_peer->local_ip.ip)
            {
                memcpy(v6nhop + 12, p_peer->local_ip.ip, 4);
            }
            else
            {
                /*use network order*/
                bgp4_fill_4bytes((v6nhop + 12), gbgp4.router_id);
                memcpy(v6nhop + 12, &gbgp4.router_id, 4);
            }
            hoplen = 16;
            break;
        }
        case BGP4_PF_IP6UCAST:
        case BGP4_PF_IP6MCAST:
        {
            if ((p_route->summary_route) || (!is_bgp_route(p_route)))
            {
                memcpy(v6nhop , p_peer->local_ip.ip, 16);
                hoplen += 16;
            }
            else if (p_peer->nexthop_self)
            {
                memcpy(v6nhop , p_peer->local_ip.ip, 16);
                hoplen += 16;
            }
            else if (bgp4_peer_type(p_peer) != BGP4_EBGP)
            {
                if (memcmp(v6nhop, p_path->nexthop.ip, 16) != 0)
                {
                    hoplen += 16;
                    memcpy(v6nhop, p_path->nexthop.ip, 16);
                }
            }
            else
            {
                memcpy( v6nhop , p_peer->local_ip.ip, 16);
                hoplen += 16;
            }
            /*The link-local address shall be included in the Next Hop field if and
            only if the BGP speaker shares a common subnet with the entity
            identified by the global IPv6 address carried in the Network Address
            of Next Hop field and the peer the route is being advertised to*/
            if (bgp4_subnet_match(&p_path->nexthop, &p_peer->local_ip, p_peer->if_unit) == TRUE )
            {
                bgp4_ip6_linklocal_nexthop_get(p_peer->local_ip.ip, v6nhop + 16);
                if ((v6nhop[16] == 0xfe) && (v6nhop[17] == 0x80))
                {
                    hoplen += 16;
                }
            }
            break;
        }
        default :
            return 0;
    }

    if (p_msg)
    {
        if ((bgp4_index_to_afi(p_route->dest.afi) == BGP4_AF_IP)
            || (p_route->dest.afi == BGP4_PF_L2VPLS))
        {
            bgp4_fill_4bytes(p_nexthop, nhop);
        }
        else if (bgp4_index_to_afi(p_route->dest.afi) == BGP4_AF_IP6)
        {
            memcpy(p_nexthop, v6nhop, hoplen);
        }
        /*set nexthop length*/
        *(p_msg) = hoplen;
    }

    return (hoplen + 1);/*including 1byte of length*/
}
/*
     VPLS sample nlri format
      +------------------------------------+
      |  Length (2 octets)                 |--00 01 == 17
      +------------------------------------+
      |  Route Distinguisher  (8 octets)   | == 00 00 00 64 00 00 00 01 ---RD
      +------------------------------------+
      |  VE ID (2 octets)                  | 00 02 ID为2
      +------------------------------------+
      |  VE Block Offset (2 octets)        |00 00 Offset为0
      +------------------------------------+
      |  VE Block Size (2 octets)          |00 0a BLOCK
      +------------------------------------+
      |  Label Base (3 octets)             |18 6a 11 ---和普通标记封装一样，都是增加了4比特偏移
      +------------------------------------+
*/

u_short 
bgp4_vpls_nlri_fill(
        u_char *p_msg ,
        tBGP4_ROUTE *p_route)
{
    if (p_msg == NULL)
    {
        return (BGP4_VPLS_NLRI_LEN + 2);
    }

    /*fill length*/
    bgp4_fill_2bytes(p_msg, BGP4_VPLS_NLRI_LEN);
    p_msg += 2;

    /*fill nlri directly*/
    memcpy(p_msg, p_route->dest.ip, BGP4_VPLS_NLRI_LEN);
    
    return (BGP4_VPLS_NLRI_LEN + 2);
}

/*if input message is null,just return length*/
u_short 
bgp4_mp_nlri_fill(
        u_char *p_msg ,
        tBGP4_ROUTE *p_route)
{
    u_char byte = 0;
    u_short filled = 0;
    u_char label_len = 0;
    u_char rd_len = 0;
    u_char *p_buf = p_msg;
    u_int af = p_route->dest.afi;
    u_short safi = bgp4_index_to_safi(af);
    u_short afi = bgp4_index_to_afi(af);

    /*vpls nlri has different format*/
    if (af == BGP4_PF_L2VPLS)
    {
        return bgp4_vpls_nlri_fill(p_msg, p_route);
    }
    
    /*+---------------------------+
    | Length (1 octet) |
    +---------------------------+
    | Label (3 octets) |
    +---------------------------+
    .............................
    +---------------------------+
    | Prefix (variable) |
    +---------------------------+*/
    if (safi == BGP4_SAF_VLBL)
    {
        label_len = BGP4_VPN_LABEL_LEN;
        rd_len = BGP4_VPN_RD_LEN;
    }
    else if (safi == BGP4_SAF_LBL)
    {
        label_len = BGP4_VPN_LABEL_LEN;
    }

    /*just return length*/
    if (p_msg == NULL)
    {
        return (filled + 1);/*including 1byte of length*/
    }

    /*nlri length bytes,prefixlen (include rd len) + label len*/
    *(p_buf) = p_route->dest.prefixlen + label_len * 8;
    p_buf++;
    filled++;

    /*fill lable,3bytes*/
    if (label_len)
    {
        /*use UPE label for route send from upe peer*/
        if (p_route->p_path->p_peer
            && (p_route->p_path->p_peer->upe_enable == TRUE)
            && (p_route->upe_label != 0)
            && (p_route->dest.afi == BGP4_PF_IPVPN))
        {
            bgp4_label_fill(p_buf, p_route->upe_label);
        }
        else if (p_route->in_label)
        {
            bgp4_label_fill(p_buf, p_route->in_label);
        }
        else
        {
            bgp4_label_fill(p_buf, p_route->out_label);
        }    
        p_buf += BGP4_VPN_LABEL_LEN;
        filled += label_len;
    }

    /*fill rd*/
    if (rd_len)
    {
        memcpy(p_buf, p_route->dest.ip, rd_len);
        p_buf += BGP4_VPN_RD_LEN;
        filled += rd_len;
    }

    byte = bgp4_bit2byte(p_route->dest.prefixlen - rd_len*8);  /*prefix length bytes*/

    /*fill prefix*/
    if (afi == BGP4_AF_IP)
    {
        bgp4_fill_4bytes(p_buf, ntohl(bgp_ip4(p_route->dest.ip + rd_len)));
    }
    else if (afi == BGP4_AF_IP6)
    {
        memcpy(p_buf, p_route->dest.ip + rd_len, byte);
    }

    filled += byte;

    return filled;
}

/*fill mpunreach attribute*/
u_short 
bgp4_mp_unreach_fill(
      u_char *p_msg, 
      u_short af, 
      u_char *p_nlri, 
      u_short len)
{
    u_short hlen = 0;
    u_short afi = bgp4_index_to_afi(af);
    u_short safi = bgp4_index_to_safi(af);

    hlen = bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_MP_UNREACH_NLRI, len + 3);
    p_msg += hlen;

    /*afi,safi*/
    bgp4_fill_2bytes(p_msg, afi);
    p_msg += 2;

    *p_msg = safi;
    p_msg++;

    memcpy(p_msg, p_nlri, len);

    return (hlen + 3 + len);
}

/*fill mpreach nlri*/
u_short 
bgp4_mp_reach_fill(
     u_char *p_msg, 
     u_short af, 
     u_char *p_nexthop, 
     u_char nhoplen, 
     u_char *p_nlri, 
     u_short len)
{
    u_short hlen = 0;
    u_short afi = bgp4_index_to_afi(af);
    u_short safi = bgp4_index_to_safi(af);

    hlen = bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_MP_NLRI, len + nhoplen + 3 + 1);
    p_msg += hlen;

    /*afi,safi*/
    bgp4_fill_2bytes(p_msg, afi);
    p_msg += 2;
    *p_msg = safi;
    p_msg++;

    /*nhop*/
    memcpy(p_msg, p_nexthop, nhoplen);
    p_msg += nhoplen;

    *p_msg = 0;
    p_msg++;

    /*nlri*/
    memcpy(p_msg, p_nlri, len);

    return (hlen + 3 + nhoplen + 1 + len);
}

u_short 
bgp4_attribute_hdr_fill(
      u_char *p_msg, 
      u_char type,
      u_short len)
{
    u_short fill_len = 0;

    if (len > 0xFF)
    {
        /* Check for whether Ext. bit set is needed. */
        bgp4_put_char(p_msg,(desc[type].flag | BGP4_ATTR_FLAG_EXT));

        bgp4_put_char(p_msg, type);

        bgp4_put_2bytes(p_msg, len);

        fill_len += 4; /* Including the length field. */
    }
    else
    {
        bgp4_put_char(p_msg, desc[type].flag);

        bgp4_put_char(p_msg, type);

        bgp4_put_char(p_msg, len);

        fill_len += 3; /* Including the length field. */
    }

    return fill_len;
}

u_short 
bgp4_origin_fill(
     u_char *p_msg, 
     tBGP4_ROUTE *p_route)
{
    tBGP4_PATH *p_path = NULL;
    u_char origin = 0;
    
    p_path = p_route->p_path;   

    p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_ORIGIN, BGP4_ORIGIN_LEN);

    switch(p_route->proto) {
        case  M2_ipRouteProto_rip:
        case  M2_ipRouteProto_ospf:
        case  M2_ipRouteProto_is_is:
              origin = (u_char)BGP4_ORIGIN_IGP;
              break;

        case  M2_ipRouteProto_netmgmt:
        case  M2_ipRouteProto_other:
        case  M2_ipRouteProto_local:
              origin = (u_char)BGP4_ORIGIN_INCOMPLETE;
              break;

        case  M2_ipRouteProto_bgp:
              if (p_path)
              {
                  origin = p_path->origin;
              }
              break;

        default:
              origin = p_path->origin;
              break;
    }

    /*prevent peer sending notify.*/
    if (origin > BGP4_ORIGIN_INCOMPLETE)
    {
        origin = BGP4_ORIGIN_INCOMPLETE;
    }

    /*if set by policy*/
    if (p_path->origin)
    {
        origin = p_path->origin;
    }
    bgp4_put_char(p_msg, origin);
    return 4;
}

u_short 
bgp4_aspath_fill(
      u_char *p_msg, 
      tBGP4_PATH *p_path,
      tBGP4_PEER *p_peer)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_char as_count = 0;
    u_short len = 0;
    u_short local_as = 0;
    u_char *p_start = p_msg ;
    u_char peer_type = bgp4_peer_type(p_peer);
    u_char private_as_flag = 0;

    len = bgp4_aspath_len(p_path, peer_type);

    p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_PATH, len);

    if ( (p_path != NULL) && (bgp4_avl_first(&p_path->aspath_list)))
    {
        if (peer_type == BGP4_EBGP)
        {
            /*ignore confed as path*/
            bgp4_avl_for_each(&p_path->aspath_list, p_aspath)
            {
                if ((p_aspath->type != BGP_ASPATH_CONFED_SET) 
                    && (p_aspath->type != BGP_ASPATH_CONFED_SEQ))
                {
                    break;
                }
            }

            /*path sequence,as the first*/
            bgp4_put_char(p_msg, BGP_ASPATH_SEQ);

            /*first as path is seq,add as num only;else
                add a new seq attribute*/
            if ((p_aspath != NULL) 
                 && (p_aspath->type == BGP_ASPATH_SEQ))
            {
                as_count = p_aspath->count;
            }
            else
            {
                as_count = 0;
                p_aspath = bgp4_avl_first(&p_path->aspath_list);
            }

            bgp4_put_char(p_msg, (as_count + 1));

            /*select local as*/
            if (p_peer->fake_as != gbgp4.as)
            {
                local_as = p_peer->fake_as;
            }
            else if (gbgp4.confedration_id != 0)
            {
                local_as = gbgp4.confedration_id;
            }
            else
            {
                local_as = gbgp4.as;
            }
            /*copy local as ,the first one*/
            bgp4_put_2bytes(p_msg, local_as);

            /*EBGP peer,should check if need to filter private AS num*/
            /*if pub as num exist in aspath list,do not del private as num to avoid data transfer error*/
            /*if peer as num exist in aspath list,do not del private as num to avoid loop*/
            if (p_peer->public_as_only 
                && (bgp4_asseq_if_exist_pub_as(&p_path->aspath_list) == FALSE)
                    && (bgp4_as_exist_in_aspath(&p_path->aspath_list, p_peer->as) == FALSE))
            {
                private_as_flag = 1;
            }

            while (p_aspath != NULL)
            {
                /*ignore confed as path*/
                if ((p_aspath->type != BGP_ASPATH_CONFED_SET)
                    && (p_aspath->type != BGP_ASPATH_CONFED_SEQ))
                {
                    /*count is 0,mean a new as-segment,*/
                    if (as_count == 0)
                    {
                        /*type*/
                        bgp4_put_char(p_msg, p_aspath->type);

                        /*as-count*/
                        as_count = p_aspath->count;
                        bgp4_put_char(p_msg, as_count);
                    }
                    p_msg += bgp4_asnum_rebuild(p_aspath, p_msg, p_peer, private_as_flag);
                }

                /*prepare next*/
                as_count = 0;
                p_aspath = bgp4_avl_next(&p_path->aspath_list, p_aspath);
            }
            return ((u_long)p_msg - (u_long)p_start);
        }
        else if (peer_type == BGP4_CONFEDBGP)
        {
            /*contain first confed seq*/
            if (gbgp4.confedration_id != 0)
            {
                bgp4_put_char(p_msg, BGP_ASPATH_CONFED_SEQ);

                p_aspath = bgp4_avl_first(&p_path->aspath_list);

                if ((p_aspath != NULL) 
                    && (p_aspath->type == BGP_ASPATH_CONFED_SEQ))
                {
                    as_count = p_aspath->count;
                }
                else
                {
                    as_count = 0;
                }
                bgp4_put_char(p_msg, (as_count + 1));

                local_as = gbgp4.as;

                bgp4_put_2bytes(p_msg, local_as);
            } 
        }

        bgp4_avl_for_each(&p_path->aspath_list, p_aspath)
        {
            if (as_count == 0)
            {
                /* AS SEQ has been copied. Seg. type and
                * length needs to be filled.
                */
                bgp4_put_char(p_msg, p_aspath->type);

                as_count = p_aspath->count;
                bgp4_put_char(p_msg, as_count);
            }
            bgp4_put_string(p_msg, p_aspath->as, (as_count * 2));
            as_count = 0;
        } 
    }
    else
    {
        /* Needs to be filled by us */
        if (peer_type == BGP4_EBGP)
        {
            bgp4_put_char(p_msg, BGP_ASPATH_SEQ);

            /* No. of ASes filled in the ASPATH */
            bgp4_put_char(p_msg, 1);

            /*select local as*/
            if (p_peer->fake_as != gbgp4.as)
            {
                local_as = p_peer->fake_as;
            }
            else if (gbgp4.confedration_id != 0)
            {
                local_as = gbgp4.confedration_id;
            }
            else
            {
                local_as = gbgp4.as;
            }

            bgp4_put_2bytes(p_msg, local_as);
        }
        else if (peer_type == BGP4_CONFEDBGP)
        {
            if (gbgp4.confedration_id != 0)
            {
                bgp4_put_char(p_msg, BGP_ASPATH_CONFED_SEQ);

                /* No. of ASes filled in the ASPATH */
                bgp4_put_char(p_msg, 1);

                local_as = gbgp4.as;

                bgp4_put_2bytes(p_msg, local_as);
            }
        }
    }
    return ((u_long)p_msg - (u_long)p_start);
}

u_short 
bgp4_aspath_fill_2(
      u_char *p_msg, 
      tBGP4_PATH *p_path,
      tBGP4_PEER *p_peer)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_char as_count = 0;
    u_short len = 0;
    u_int local_as = 0;
    u_char *p_start = p_msg ;
    u_char peer_type = bgp4_peer_type(p_peer);
    u_char buf[BGP4_MAX_MSG_LEN] = {0};
    u_char *p_opt = buf; 

    if (bgp4_avl_count(&p_path->aspath_list))
    {
        if (peer_type == BGP4_EBGP)
        {
            /*ignore confed as path*/
            bgp4_avl_for_each(&p_path->aspath_list, p_aspath)
            {
                if ((p_aspath->type != BGP_ASPATH_CONFED_SET) 
                    && (p_aspath->type != BGP_ASPATH_CONFED_SEQ))
                {
                    break;
                }
            }

            /*path sequence,as the first*/
            bgp4_put_char(p_opt, BGP_ASPATH_SEQ);

            /*first as path is seq,add as num only;else
                add a new seq attribute*/
            if ((p_aspath != NULL) 
                 && (p_aspath->type == BGP_ASPATH_SEQ))
            {
                as_count = p_aspath->count;
            }
            else
            {
                as_count = 0;
                p_aspath = bgp4_avl_first(&p_path->aspath_list);
            }

            bgp4_put_char(p_opt, (as_count + 1));

            /*select local as*/
            if (p_peer->fake_as != gbgp4.as)
            {
                local_as = p_peer->fake_as;
            }
            else if (gbgp4.confedration_id != 0)
            {
                local_as = gbgp4.confedration_id;
            }
            else
            {
                local_as = gbgp4.as;
            }
            /*copy local as ,the first one*/
            bgp4_put_4bytes(p_opt, local_as);
          
            while (p_aspath != NULL)
            {
                /*ignore confed as path*/
                if ((p_aspath->type != BGP_ASPATH_CONFED_SET)
                    && (p_aspath->type != BGP_ASPATH_CONFED_SEQ))
                {
                    /*count is 0,mean a new as-segment,*/
                    if (as_count == 0)
                    {
                        /*type*/
                        bgp4_put_char(p_opt, p_aspath->type);

                        /*as-count*/
                        as_count = p_aspath->count;
                        bgp4_put_char(p_opt, as_count);
                    }
                    bgp4_put_string(p_opt, p_aspath->as, as_count*4);
                }

                /*prepare next*/
                as_count = 0;
                p_aspath = bgp4_avl_next(&p_path->aspath_list, p_aspath);
            }
            bgp4_avl_for_each(&p_path->as4path_list, p_aspath)
            {
                /*ignore confed as path*/
                if ((p_aspath->type != BGP_ASPATH_CONFED_SET)
                    && (p_aspath->type != BGP_ASPATH_CONFED_SEQ))
                {
                     /*type*/
                    bgp4_put_char(p_opt, p_aspath->type);

                    /*as-count*/
                    as_count = p_aspath->count;
                    bgp4_put_char(p_opt, as_count);
                    bgp4_put_string(p_opt, p_aspath->as, as_count*4);
                }
            }
        }
        else if (peer_type == BGP4_CONFEDBGP)
        {
            /*contain first confed seq*/
            if (gbgp4.confedration_id != 0)
            {
                bgp4_put_char(p_opt, BGP_ASPATH_CONFED_SEQ);

                p_aspath = bgp4_avl_first(&p_path->aspath_list);

                if ((p_aspath != NULL) 
                    && (p_aspath->type == BGP_ASPATH_CONFED_SEQ))
                {
                    as_count = p_aspath->count;
                }
                else
                {
                    as_count = 0;
                }
                bgp4_put_char(p_opt, (as_count + 1));

                local_as = gbgp4.as;

                bgp4_put_4bytes(p_opt, local_as);
            } 
        }

        bgp4_avl_for_each(&p_path->aspath_list, p_aspath)
        {
            if (as_count == 0)
            {
                /* AS SEQ has been copied. Seg. type and
                * length needs to be filled.
                */
                bgp4_put_char(p_opt, p_aspath->type);

                as_count = p_aspath->count;
                bgp4_put_char(p_opt, as_count);
            }
            bgp4_put_string(p_opt, p_aspath->as, (as_count * 4));
            as_count = 0;
        } 
        bgp4_avl_for_each(&p_path->as4path_list, p_aspath)
        {
            /*ignore confed as path*/
            if ((p_aspath->type != BGP_ASPATH_CONFED_SET)
                && (p_aspath->type != BGP_ASPATH_CONFED_SEQ))
            {
                 /*type*/
                bgp4_put_char(p_opt, p_aspath->type);

                /*as-count*/
                as_count = p_aspath->count;
                bgp4_put_char(p_opt, as_count);
                bgp4_put_string(p_opt, p_aspath->as, as_count*4);
            }
        }
    }
    else if (bgp4_avl_count(&p_path->as4path_list))
    {
        if (peer_type == BGP4_EBGP)
        {
            /*ignore confed as path*/
            bgp4_avl_for_each(&p_path->as4path_list, p_aspath)
            {
                if ((p_aspath->type != BGP_ASPATH_CONFED_SET) 
                    && (p_aspath->type != BGP_ASPATH_CONFED_SEQ))
                {
                    break;
                }
            }

            /*path sequence,as the first*/
            bgp4_put_char(p_opt, BGP_ASPATH_SEQ);

            /*first as path is seq,add as num only;else
                add a new seq attribute*/
            if ((p_aspath != NULL) 
                 && (p_aspath->type == BGP_ASPATH_SEQ))
            {
                as_count = p_aspath->count;
            }
            else
            {
                as_count = 0;
                p_aspath = bgp4_avl_first(&p_path->as4path_list);
            }

            bgp4_put_char(p_opt, (as_count + 1));

            /*select local as*/
            if (p_peer->fake_as != gbgp4.as)
            {
                local_as = p_peer->fake_as;
            }
            else if (gbgp4.confedration_id != 0)
            {
                local_as = gbgp4.confedration_id;
            }
            else
            {
                local_as = gbgp4.as;
            }
            /*copy local as ,the first one*/
            bgp4_put_4bytes(p_opt, local_as);
          
            while (p_aspath != NULL)
            {
                /*ignore confed as path*/
                if ((p_aspath->type != BGP_ASPATH_CONFED_SET)
                    && (p_aspath->type != BGP_ASPATH_CONFED_SEQ))
                {
                    /*count is 0,mean a new as-segment,*/
                    if (as_count == 0)
                    {
                        /*type*/
                        bgp4_put_char(p_opt, p_aspath->type);

                        /*as-count*/
                        as_count = p_aspath->count;
                        bgp4_put_char(p_opt, as_count);
                    }
                    bgp4_put_string(p_opt, p_aspath->as, as_count*4);
                }

                /*prepare next*/
                as_count = 0;
                p_aspath = bgp4_avl_next(&p_path->as4path_list, p_aspath);
            }            
        }
        else if (peer_type == BGP4_CONFEDBGP)
        {
            /*contain first confed seq*/
            if (gbgp4.confedration_id != 0)
            {
                bgp4_put_char(p_opt, BGP_ASPATH_CONFED_SEQ);

                p_aspath = bgp4_avl_first(&p_path->aspath_list);

                if ((p_aspath != NULL) 
                    && (p_aspath->type == BGP_ASPATH_CONFED_SEQ))
                {
                    as_count = p_aspath->count;
                }
                else
                {
                    as_count = 0;
                }
                bgp4_put_char(p_opt, (as_count + 1));

                local_as = gbgp4.as;

                bgp4_put_4bytes(p_opt, local_as);
            } 
        }

        bgp4_avl_for_each(&p_path->as4path_list, p_aspath)
        {
            if (as_count == 0)
            {
                /* AS SEQ has been copied. Seg. type and
                * length needs to be filled.
                */
                bgp4_put_char(p_opt, p_aspath->type);

                as_count = p_aspath->count;
                bgp4_put_char(p_opt, as_count);
            }
            bgp4_put_string(p_opt, p_aspath->as, (as_count * 4));
            as_count = 0;
        } 
        bgp4_avl_for_each(&p_path->as4path_list, p_aspath)
        {
            /*ignore confed as path*/
            if ((p_aspath->type != BGP_ASPATH_CONFED_SET)
                && (p_aspath->type != BGP_ASPATH_CONFED_SEQ))
            {
                 /*type*/
                bgp4_put_char(p_opt, p_aspath->type);

                /*as-count*/
                as_count = p_aspath->count;
                bgp4_put_char(p_opt, as_count);
                bgp4_put_string(p_opt, p_aspath->as, as_count*4);
            }
        }
    }
    else
    {
       /* Needs to be filled by us */
        if (peer_type == BGP4_EBGP)
        {
            bgp4_put_char(p_opt, BGP_ASPATH_SEQ);

            /* No. of ASes filled in the ASPATH */
            bgp4_put_char(p_opt, 1);

            /*select local as*/
            if (p_peer->fake_as != gbgp4.as)
            {
                local_as = p_peer->fake_as;
            }
            else if (gbgp4.confedration_id != 0)
            {
                local_as = gbgp4.confedration_id;
            }
            else
            {
                local_as = gbgp4.as;
            }
            bgp4_put_4bytes(p_opt, local_as);
        }
    }
    
    len = (u_long)p_opt - (u_long)buf;
    if (len == 0)
    {
        return 0;
    }
    p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_NEW_PATH, len);
    bgp4_put_string(p_msg, buf, len);
    return ((u_long)p_msg - (u_long)p_start);
}


/*fill attribute,not include mpreach and unreach*/
u_short 
bgp4_path_fill (
        tBGP4_PEER *p_peer,
        tBGP4_ROUTE *p_route,
        u_char *p_msg)
{
    tBGP4_PATH *p_save_path = p_route->p_path;
    tBGP4_PATH policy_path;
    u_int af = p_route->dest.afi;
    u_long msg_start = (u_long)p_msg;

    /*build new path according to policy*/
    memset(&policy_path, 0, sizeof(policy_path));
    bgp4_path_init(&policy_path);
    bgp4_path_copy(&policy_path, p_save_path);

    bgp4_export_route_policy_apply(p_route, p_peer, &policy_path);

    /*set route's path for attribtue filling*/
    p_route->p_path = &policy_path;

    p_msg += bgp4_origin_fill(p_msg, p_route);

    /*fill aspath*/
    /*if local node do not support 4bytes as,fill normal aspath*/
    if (gbgp4.as4_enable == FALSE)
    {
        p_msg += bgp4_aspath_fill(p_msg, &policy_path, p_peer);

        /*if as4path attribute exist,fill it without change*/
        p_msg += bgp4_as4path_fill(p_msg, &policy_path, p_peer);
    }
    else if (p_peer->as4_enable == FALSE)
    {
        /*merge aspath and as4path into 2byte aspath,
          and create 4bytes aspath*/
        p_msg += bgp4_aspath_fill(p_msg, &policy_path, p_peer);

        p_msg += bgp4_as4path_fill_2(p_msg, &policy_path, p_peer);
    }
    else
    {
        /*merge aspath and as4path into 4byte aspath*/
       p_msg += bgp4_aspath_fill_2(p_msg, &policy_path, p_peer);
    }
    
    /*  Filling the Next Hop ,in MBGP,do not send nexthop*/
    if (af == BGP4_PF_IPUCAST)
    {
        p_msg += bgp4_nexthop_fill(p_msg, p_route, p_peer);
    }

    p_msg +=  bgp4_med_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_lpref_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_aggregate_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_community_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_originid_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_cluster_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_ecommunity_fill(p_msg, p_route, p_peer);

    p_msg +=  bgp4_unkown_fill(p_msg, &policy_path);

    /*recover original path*/
    p_route->p_path = p_save_path;

    /*release policy path*/
    bgp4_path_clear(&policy_path);
    return (((u_long)p_msg) - msg_start);
}

/*copy as4path attribute into msg*/
u_short 
bgp4_as4path_fill(
      u_char *p_msg, 
      tBGP4_PATH *p_path,
      tBGP4_PEER *p_peer)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_char buf[BGP4_MAX_MSG_LEN] = {0};
    u_char *p_opt = buf;
    u_char *p_start = p_msg;
    u_short len = 0;

    bgp4_avl_for_each(&p_path->as4path_list, p_aspath)
    {
        bgp4_put_char(p_opt, p_aspath->type);
        bgp4_put_char(p_opt, p_aspath->count);
        bgp4_put_string(p_opt, p_aspath->as, (p_aspath->count * 4));
    } 
    len = (u_long)p_opt - (u_long)buf;
    if (len == 0)
    {
        return 0;
    }
    p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_NEW_PATH, len);
    bgp4_put_string(p_msg, buf, len);
    return ((u_long)p_msg - (u_long)p_start);
}

u_short 
bgp4_as4path_fill_2(
      u_char *p_msg, 
      tBGP4_PATH *p_path,
      tBGP4_PEER *p_peer)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_char buf[BGP4_MAX_MSG_LEN] = {0};
    u_char *p_opt = buf;
    u_char *p_start = p_msg;
    u_char peer_type = bgp4_peer_type(p_peer);
    u_char as_count = 0;
    u_short len = 0;
    u_int local_as = 0;
    
    if ((p_path != NULL) && (bgp4_avl_first(&p_path->as4path_list)))
    {
        if (peer_type == BGP4_EBGP)
        {
            /*ignore confed as path*/
            bgp4_avl_for_each(&p_path->as4path_list, p_aspath)
            {
                if ((p_aspath->type != BGP_ASPATH_CONFED_SET) 
                    && (p_aspath->type != BGP_ASPATH_CONFED_SEQ))
                {
                    break;
                }
            }

            /*path sequence,as the first*/
            bgp4_put_char(p_opt, BGP_ASPATH_SEQ);

            /*first as path is seq,add as num only;else
                add a new seq attribute*/
            if ((p_aspath != NULL) 
                 && (p_aspath->type == BGP_ASPATH_SEQ))
            {
                as_count = p_aspath->count;
            }
            else
            {
                as_count = 0;
                p_aspath = bgp4_avl_first(&p_path->as4path_list);
            }

            bgp4_put_char(p_opt, (as_count + 1));

            /*select local as*/
            if (p_peer->fake_as != gbgp4.as)
            {
                local_as = p_peer->fake_as;
            }
            else if (gbgp4.confedration_id != 0)
            {
                local_as = gbgp4.confedration_id;
            }
            else
            {
                local_as = gbgp4.as;
            }
            /*copy local as ,the first one*/
            bgp4_put_4bytes(p_opt, local_as);

            while (p_aspath != NULL)
            {
                /*ignore confed as path*/
                if ((p_aspath->type != BGP_ASPATH_CONFED_SET)
                    && (p_aspath->type != BGP_ASPATH_CONFED_SEQ))
                {
                    /*count is 0,mean a new as-segment,*/
                    if (as_count == 0)
                    {
                        /*type*/
                        bgp4_put_char(p_opt, p_aspath->type);

                        /*as-count*/
                        as_count = p_aspath->count;
                        bgp4_put_char(p_opt, as_count);
                    }
                    bgp4_put_string(p_opt, p_aspath->as, (p_aspath->count * 4));
                }

                /*prepare next*/
                as_count = 0;
                p_aspath = bgp4_avl_next(&p_path->as4path_list, p_aspath);
            }
        }
        else if (peer_type == BGP4_CONFEDBGP)
        {
            /*contain first confed seq*/
            if (gbgp4.confedration_id != 0)
            {
                bgp4_put_char(p_msg, BGP_ASPATH_CONFED_SEQ);

                p_aspath = bgp4_avl_first(&p_path->as4path_list);

                if ((p_aspath != NULL) 
                    && (p_aspath->type == BGP_ASPATH_CONFED_SEQ))
                {
                    as_count = p_aspath->count;
                }
                else
                {
                    as_count = 0;
                }
                bgp4_put_char(p_msg, (as_count + 1));

                local_as = gbgp4.as;

                bgp4_put_4bytes(p_msg, local_as);
            } 
        }

        bgp4_avl_for_each(&p_path->as4path_list, p_aspath)
        {
            if (as_count == 0)
            {
                /* AS SEQ has been copied. Seg. type and
                * length needs to be filled.
                */
                bgp4_put_char(p_msg, p_aspath->type);

                as_count = p_aspath->count;
                bgp4_put_char(p_msg, as_count);
            }
            bgp4_put_string(p_msg, p_aspath->as, (as_count * 4));
            as_count = 0;
        } 
    }
    else
    {
        /* Needs to be filled by us */
        if (peer_type == BGP4_EBGP)
        {
            bgp4_put_char(p_msg, BGP_ASPATH_SEQ);

            /* No. of ASes filled in the ASPATH */
            bgp4_put_char(p_msg, 1);

            /*select local as*/
            if (p_peer->fake_as != gbgp4.as)
            {
                local_as = p_peer->fake_as;
            }
            else if (gbgp4.confedration_id != 0)
            {
                local_as = gbgp4.confedration_id;
            }
            else
            {
                local_as = gbgp4.as;
            }

            bgp4_put_4bytes(p_msg, local_as);
        }
        else if (peer_type == BGP4_CONFEDBGP)
        {
            if (gbgp4.confedration_id != 0)
            {
                bgp4_put_char(p_msg, BGP_ASPATH_CONFED_SEQ);

                /* No. of ASes filled in the ASPATH */
                bgp4_put_char(p_msg, 1);

                local_as = gbgp4.as;

                bgp4_put_4bytes(p_msg, local_as);
            }
        }
    }
    len = (u_long)p_opt - (u_long)buf;
    if (len == 0)
    {
        return 0;
    }
    p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_NEW_PATH, len);
    bgp4_put_string(p_msg, buf, len);
    return ((u_long)p_msg - (u_long)p_start);
}


u_short 
bgp4_nexthop_fill(
        u_char *p_msg,
        tBGP4_ROUTE *p_route,
        tBGP4_PEER *p_peer)
{
    tBGP4_PATH *p_path = NULL;
    u_int nexthop = 0;
    u_char *p_start = p_msg;

    p_path = p_route->p_path;   
    
    /*nexthop length must be filled into 1 byte*/
    p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_NEXT_HOP, BGP4_NEXTHOP_LEN);

    /* Protocol is Not BGP, or Self-Aggregated route */
    if ((p_route->summary_route) || (!is_bgp_route(p_route)))
    {
        nexthop = bgp_ip4(p_peer->local_ip.ip);
    }
    else if (p_peer->nexthop_self)
    {
        nexthop = bgp_ip4(p_peer->local_ip.ip);
    }    
    else if (bgp4_peer_type(p_peer) != BGP4_EBGP)
    {
        nexthop = bgp_ip4(p_path->nexthop.ip);
    }
    else
    {
        if ( bgp4_subnet_match(&p_path->nexthop, 
              &p_peer->local_ip, p_peer->if_unit) == TRUE )
        {
            nexthop = bgp_ip4(p_path->nexthop.ip);
        }
        else
        {
            nexthop = bgp_ip4(p_peer->local_ip.ip);
        }
    }

    /*special for vpn route exported,use local ip address*/
    if (p_path->p_instance->vrf && (p_path->origin_vrf == 0))
    {
        nexthop = bgp_ip4(p_peer->local_ip.ip);
    }
    
    /*translate nexthop into host order*/
    nexthop = ntohl(nexthop);
    
    /*fill local id*/
    bgp4_put_4bytes(p_msg, nexthop);

    return ((u_long)p_msg - (u_long)p_start);
}

u_short  
bgp4_med_fill(
       u_char *p_msg, 
       tBGP4_ROUTE *p_route,
       tBGP4_PEER *p_peer)
{
    u_int med = 0;
    u_char *p_start = p_msg;
    u_char peer_type = 0 ;
    tBGP4_PATH *p_path = NULL;

    p_path = p_route->p_path;   

    peer_type = bgp4_peer_type(p_peer);
    if (is_bgp_route(p_route))
    {
        if ((peer_type == BGP4_IBGP) 
            || bgp4_confedration_as_lookup(p_peer->as))
        {
            med = p_path->med;
        }
        else if (peer_type == BGP4_EBGP)
        {
            /*no med need*/
            med = 0;
            if(p_route->summary_route)
            {
                med = gbgp4.med;
            }
        }
    }
    else if (peer_type == BGP4_EBGP)
    {
        if (p_path->med)
        {
            med = p_path->med;/*import route*/
        }
        else 
        {
            med = gbgp4.med;
        }
    }
    
    if (med == 0)
    {
        return 0;
    }
    p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_MED,BGP4_MED_LEN);

    bgp4_put_4bytes(p_msg, med);

    return ((u_long)p_msg - (u_long)p_start);
}

u_short  
bgp4_lpref_fill(
       u_char *p_msg,
       tBGP4_ROUTE *p_route,
       tBGP4_PEER *p_peer)
{
    u_int lp = 0;
    u_int peer_type;
    u_char *p_start = p_msg;
    tBGP4_PATH  *p_path = NULL;

    p_path = p_route->p_path;   
   
    peer_type = bgp4_peer_type(p_peer);

    if ((peer_type == BGP4_IBGP) 
        || bgp4_confedration_as_lookup(p_peer->as))
    {
        lp = p_path->localpref;

        if ((gbgp4.local_pref == BGP4_DEFAULT_LOCALPREF) && (lp == 0))
        {
            lp = gbgp4.local_pref;
        }
        if ((gbgp4.local_pref != 0) && (gbgp4.local_pref != BGP4_DEFAULT_LOCALPREF))
        {
            lp = gbgp4.local_pref;
        }

        if (lp || (peer_type != BGP4_EBGP))
        {
            p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_LOCAL_PREF, BGP4_LPREF_LEN) ;

            /*reflect do not change lp value*/
            if (gbgp4.reflector_enable == TRUE)
            {
                if (p_path->localpref)
                {
                    lp = p_path->localpref;
                }
            }
            bgp4_put_4bytes(p_msg, lp);
        }
    }
    return ((u_long)p_msg - (u_long)p_start);
}

u_short  
bgp4_aggregate_fill(
      u_char *p_msg, 
      tBGP4_ROUTE *p_route,
      tBGP4_PEER* p_peer)
{
    u_short asnum = 0;
    tBGP4_PATH *p_path = NULL;
    u_int aggr = 0;
    u_char *p_start = p_msg;
    
    p_path = p_route->p_path;   
    
    /* Filling the Atomic Aggregate field  */
    if (((is_bgp_route(p_route)) 
        && (p_path->atomic_exist))
        || (p_route->summary_route))
    {
        if ((p_path->atomic_exist == TRUE)
            || (p_path->community_len == 0))
        {
            p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_ATOMIC_AGGR, BGP4_ATOMIC_LEN);
        }
    }

    /* Filling the Aggregator field  */
    if (((is_bgp_route(p_route)) 
        && (p_path->p_aggregator))
        || (p_route->summary_route))
    {
        p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_AGGREGATOR, BGP4_AGGR_LEN);
        if (p_route->summary_route)
        {
            if (bgp4_peer_type(p_peer) == BGP4_EBGP 
                && (p_peer->fake_as != gbgp4.as))
            {
                asnum = p_peer->fake_as;
            }
            else
            {
                asnum = gbgp4.as;
            }
            aggr = gbgp4.router_id;
        }
        else if (p_path->p_aggregator)
        {
            asnum = p_path->p_aggregator->as;
            aggr = bgp_ip4(p_path->p_aggregator->ip.ip);
        }
        bgp4_put_2bytes(p_msg, asnum);
        bgp4_put_4bytes(p_msg, aggr);
    }
    return ((u_long)p_msg - (u_long)p_start);
}

u_short 
bgp4_unkown_fill(
       u_char *p_msg, 
       tBGP4_PATH *p_path)
{
    u_short payload_len;
    u_short read_len = 0;
    u_short fill_len = 0;
    u_char hlen = 0;
    u_char flag = 0;
    u_char *p_option = NULL;
    
    if ((p_path == NULL) || (!p_path->p_unkown))
    {
        return 0;
    }

    /*split all option*/
    for (p_option = p_path->p_unkown; 
         read_len < p_path->unknown_len; 
         read_len += hlen + payload_len,
         p_option += hlen + payload_len)
    {
        flag = *p_option;
        if (flag & BGP4_ATTR_FLAG_EXT)
        {
            bgp4_get_2bytes(payload_len, (p_option + 2));
            hlen = 4;
        }
        else
        {
            payload_len = *(p_option + 2);
            hlen = 3;
        }
        /*ignore non-transive option*/
        if (!(flag & BGP4_ATTR_FLAG_TRANSIT))
        {
            continue;
        }
        /*copy into dest buffer*/
        memcpy(p_msg, p_option, hlen + payload_len);
        
        /*set Partial flag*/
        (*p_msg) = (flag | BGP4_ATTR_FLAG_PARTIAL);
        
        fill_len += hlen + payload_len;
        p_msg += hlen + payload_len;
    }
    return fill_len;
}

u_short  
bgp4_originid_fill(
     u_char *p_msg,
     tBGP4_ROUTE  *p_route,
     tBGP4_PEER *p_peer)
{
    u_int origin_id = 0;
    tBGP4_PATH * p_path = NULL;
    u_char *p_start = p_msg ;

    if ((bgp4_peer_type(p_peer) == BGP4_IBGP) 
        && (gbgp4.reflector_enable == TRUE))
    {
        p_path = p_route->p_path;
    
        if (p_path->origin_id)
        {
            origin_id = p_path->origin_id;
        }
        else if (p_path->p_peer != NULL)
        {
            origin_id = p_path->p_peer->router_id;
        }
        else
        {
            origin_id = gbgp4.router_id;
        }
        if (origin_id)
        {
            p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_ORIGINATOR, BGP4_ORIGINATOR_LEN);
            bgp4_put_4bytes(p_msg, origin_id);
        }
    }
    return ((u_long)p_msg - (u_long)p_start );
}

u_short  
bgp4_cluster_fill(
        u_char *p_msg,
        tBGP4_ROUTE *p_route,
        tBGP4_PEER *p_peer )
{
    tBGP4_PATH * p_path = NULL;
    u_char len = 0;
    u_char *p_start = p_msg;

    if ((bgp4_peer_type(p_peer) == BGP4_IBGP) 
        && (gbgp4.reflector_enable == TRUE))
    {
        p_path = p_route->p_path;

        /*cluster id len,add self*/
        len = p_path->cluster_len + 4;

        p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_CLUSTERLIST, len);
        bgp4_put_4bytes(p_msg, gbgp4.cluster_id);

        if (p_path->p_cluster)
        {
            bgp4_put_string(p_msg, p_path->p_cluster, p_path->cluster_len);
        }
    }
    return ((u_long)p_msg - (u_long)p_start );
}

u_short  
bgp4_community_fill(
        u_char *p_msg,
        tBGP4_ROUTE *p_route,
        tBGP4_PEER *p_peer )
{
    tBGP4_PATH *p_path = NULL;
    u_char len = 0;
    u_char *p_start = p_msg;
    
    if (p_peer->send_community != TRUE)
    {
        return 0;
    }
    
    p_path = p_route->p_path;   

    if (gbgp4.community_action == COM_ACTION_CLEAR)
    {
        return 0;
    }

    if (gbgp4.community_action == COM_ACTION_REPLACE)
    {
        len = 4;
        p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_COMMUNITY, len);

        bgp4_put_4bytes(p_msg, gbgp4.community);
        return ((u_long)p_msg - (u_long)p_start);
    }

    /*decide length*/
    if ((p_route->summary_route == FALSE) || (p_path->atomic_exist == FALSE))
    {
        len += p_path->community_len;
    }
    if ((gbgp4.community_action == COM_ACTION_ADD) && gbgp4.community)
    {
        len += 4;
    }

    if (len == 0)
    {
        return 0;
    }

    p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_COMMUNITY, len);

    if (p_path->p_community
       && ((p_route->summary_route == FALSE)
          || (p_path->atomic_exist == FALSE)))
    {
        bgp4_put_string(p_msg, p_path->p_community, p_path->community_len);
    }

    if ((gbgp4.community_action == COM_ACTION_ADD) && gbgp4.community)
    {
        bgp4_put_4bytes(p_msg, gbgp4.community);
    }

    return ((u_long)p_msg - (u_long)p_start);
}

u_short 
bgp4_ecommunity_fill(
         u_char *p_msg,
         tBGP4_ROUTE *p_route ,
         tBGP4_PEER *p_peer)
{
    tBGP4_PATH *p_path = NULL;
    u_char u1_send_flag = 0 ;
    u_char *p_start = p_msg ;
    u_char zero_buf[8];
    u_char ext_comm_buf[8192];
    u_char *p_buf_start = ext_comm_buf;
    u_int len=0;
    u_int i=0;
    u_short ext_as=0;
    u_int ext_address=0;
    tBGP4_EXT_COMM* p_ext=NULL;

    memset(zero_buf, 0, 8);
    memset(ext_comm_buf, 0, sizeof(ext_comm_buf));

    if ((p_msg == NULL) || (p_route == NULL) ||
        (p_peer == NULL)||
        (p_route->p_path== NULL))
    {
        return 0;
    }

    p_path = p_route->p_path;   
    
    /*if need to fill configured ex_comm in global ex_comm list*/
    if (p_path->p_peer && p_path->p_peer->ext_comm_enable)
    {
        bgp4_avl_for_each(&gbgp4.ecom_table, p_ext)
        {
            ext_as = 0;
            ext_address = 0;
            if (bgp4_peer_type(p_peer) != BGP4_IBGP)
            {
                if (p_ext->main_type & 0x40)
                {
                    /*if not configured Confederation,not add*/
                    if (gbgp4.confedration_id  == 0)
                    {
                        continue;
                    }

                    /*if remote as is not Confederation id ,and
                    is not Confederation member,not add*/
                    if (gbgp4.confedration_id != p_peer->as)
                    {
                        for (i = 0 ; i < BGP4_MAX_CONFEDPEER ; i++)
                        {
                            if (gbgp4.confedration_as[i] == 0)
                            {
                                break;
                            }

                            if ((p_path->p_peer != NULL)
                                &&(p_peer->as == gbgp4.confedration_as[i]))
                            {
                                u1_send_flag = 1;
                                break;
                            }
                        }
                        if (!u1_send_flag)
                        {
                            continue;
                        }
                    }
                }
            }
            /*add ext-community*/
            bgp4_put_char(p_buf_start,p_ext->main_type);
            bgp4_put_char(p_buf_start,p_ext->sub_type);

            if(p_ext->main_type==0x00||p_ext->main_type==0x40)
            {
                ext_as=htons(p_ext->as);
                ext_address=htonl(p_ext->address);
                bgp4_put_2bytes(p_buf_start, ext_as);
                bgp4_put_4bytes(p_buf_start, ext_address);
            }
            else if(p_ext->main_type&0x01)
            {
                ext_as=htons(p_ext->as);
                ext_address=htonl(p_ext->address);
                bgp4_put_4bytes(p_buf_start, ext_address);
                bgp4_put_2bytes(p_buf_start, ext_as);
            }
            else if(p_ext->main_type&0x03)
            {
                /*Opaque Extended Community*/
                p_buf_start+=6;
            }
            else
            {
                p_buf_start+=6;
            }
        }
    }

    /*fill ex_comm already in path*/
    if (p_path->excommunity_len && p_path->p_ecommunity)
    {
        bgp4_put_string(p_buf_start,  p_route->p_path->p_ecommunity, p_route->p_path->excommunity_len);
    }

     len = p_buf_start-ext_comm_buf;

    if (len > 0)
    {
        p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_EXT_COMMUNITY, len);
        bgp4_put_string(p_msg, ext_comm_buf, len);
    }

    return (((u_long)p_msg) - (u_long)p_start) ;
}

tBGP4_EXT_COMM *
bgp4_add_ext_comm(tBGP4_EXT_COMM *p_ext_comm)
{
    tBGP4_EXT_COMM *p_new_excomm = NULL;

    p_new_excomm = bgp4_malloc(sizeof(tBGP4_EXT_COMM), MEM_BGP_COMMUNITY);
    if (p_new_excomm == NULL)
    {
        return NULL;
    }
    p_new_excomm->main_type = p_ext_comm->main_type;
    p_new_excomm->sub_type = p_ext_comm->sub_type;
    p_new_excomm->address = p_ext_comm->address;
    p_new_excomm->as = p_ext_comm->as;

    bgp4_avl_add(&gbgp4.ecom_table, p_new_excomm);

    return p_new_excomm;
}

void 
bgp4_delete_ext_comm(tBGP4_EXT_COMM* p_ext_comm)
{
    bgp4_avl_delete(&gbgp4.ecom_table, p_ext_comm);
    bgp4_free(p_ext_comm, MEM_BGP_LINK);
    return;
}

void 
bgp4_delete_all_ext_comm(void)
{
    bgp4_avl_walkup(&gbgp4.ecom_table, bgp4_delete_ext_comm);
    return;
}

tBGP4_EXT_COMM * 
bgp4_ext_comm_lookup(tBGP4_EXT_COMM* p_ext_comm)
{
    tBGP4_EXT_COMM* p_ext=NULL;
    bgp4_avl_for_each(&gbgp4.ecom_table, p_ext)
    {
        if((p_ext->as==p_ext_comm->as)&&(p_ext->address==p_ext_comm->address))
        {
            return p_ext;
        }
    }
    return NULL;
}

/*get label value from buffer*/
u_int 
bgp4_label_extract(u_char *p_buf)
{
    u_int label = 0;
    u_char *p = (u_char *)&label;
    
    /*copy 3bytes label in network order*/
    memcpy(p + 1, p_buf, 3);

    /*translate into host order*/
    label = ntohl(label);

    /*right shift 4bits*/
    label = label >> 4;
    return label;
}

/*insert label value to buffer*/
void 
bgp4_label_fill(
              u_char *p_buf,
              u_int label)
{
    u_int rebuild = (label << 4) + 1;/*left shift 4bits,and insert BOS flag*/
    p_buf[0] = (rebuild & 0x00ff0000) >> 16;
    p_buf[1] = (rebuild & 0x0000ff00) >> 8;
    p_buf[2] = (rebuild & 0x000000ff);
    return;
}

#else
#include "bgp4com.h"


/*add path node to global list*/
void bgp4_path_add_to_list(tBGP4_PATH*p_path,u_char af)
{
    if(p_path == NULL)
    {
        return;
    }

    bgp4_lstadd(&gBgp4.attr_list[af],&p_path->node);
    bgp4_log(BGP_DEBUG_EVT,1,"add a af %d path %#x to path list,path count %d",
        af,p_path,bgp4_lstcnt(&gBgp4.attr_list[af]));

    return;

}

/*create a path node*/
tBGP4_PATH *bgp4_add_path(u_int af)
{
    tBGP4_PATH *p_path = NULL;

    p_path = (tBGP4_PATH*)bgp4_malloc(sizeof(tBGP4_PATH), MEM_BGP_INFO);
    if (p_path == NULL)
    {
        return (NULL);
    }

    memset(p_path,0,sizeof(tBGP4_PATH)) ;
    bgp4_lstinit(&p_path->aspath_list);
    bgp4_lstinit(&p_path->route_list);

    p_path->direct_nexthop_ifunit = -1;

    if(af >= BGP4_PF_MAX)
    {
        p_path->afi = BGP4_AF_IP;/*default*/
        p_path->safi = BGP4_SAF_UCAST;/*default*/
    }
    else
    {
        p_path->afi = bgp4_index_to_afi(af);
        p_path->safi = bgp4_index_to_safi(af);
        bgp4_path_add_to_list(p_path,af);
    }

    return p_path ;
}

/*free a path*/
void bgp4_path_free(tBGP4_PATH *p_path)
{
    /*as path list*/
    bgp4_apath_list_free(p_path);

    if (p_path->p_unkown != NULL)
    {
        bgp4_free((void*)p_path->p_unkown, MEM_BGP_BUF);
    }

    bgp4_free(p_path,MEM_BGP_INFO);
    return ;
}

/*lookup same path*/
tBGP4_PATH *bgp4_path_lookup(tBGP4_PATH *p_path)
{
    tBGP4_PATH *p_current = NULL ;
    u_int af = bgp4_afi_to_index(p_path->afi, p_path->safi);

    if(p_path == NULL)
    {
        return NULL;
    }

    if(af >= BGP4_PF_MAX)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"path af afi %d:safi %d index invalid!",p_path->afi, p_path->safi);
        return NULL;
    }

    LST_LOOP(&gBgp4.attr_list[af], p_current, node, tBGP4_PATH)
    {
        if (bgp4_same_path(p_current, p_path) == TRUE)
        {
            return p_current ;
        }
    }

    return NULL ;
}

/*clear unused path*/
void bgp4_clear_unused_path(void)
{
    tBGP4_PATH *p_path = NULL ;
    tBGP4_PATH *p_next = NULL ;
    u_int af = 0;

    for(af = 0;af < BGP4_PF_MAX;af++)
    {
        LST_LOOP_SAFE(&gBgp4.attr_list[af], p_path, p_next, node, tBGP4_PATH)
        {
            if (!bgp4_lstfirst(&p_path->route_list))
            {
                bgp4_log(BGP_DEBUG_EVT,1,"clear a af %d path %#x,bgp next path %#x,instance %d",
                    af,p_path,p_next,p_path->p_instance->instance_id);

                bgp4_lstdelete(&gBgp4.attr_list[af], &p_path->node);

                bgp4_path_free(p_path);

                bgp4_log(BGP_DEBUG_EVT,1,"after clear ,path count %d",
                    bgp4_lstcnt(&gBgp4.attr_list[af]));


            }
        }
    }



    return ;
}

/*copy path from source to dest*/
void bgp4_path_copy(tBGP4_PATH *p_dest ,tBGP4_PATH *p_src )
{
    tBGP4_ASPATH *p_aspath = NULL;
    tBGP4_ASPATH *p_src_aspath = NULL;

    if ((p_src == NULL) || (p_dest == NULL))
    {
        return ;
    }

    memcpy(&p_dest->flags, &p_src->flags, sizeof(p_dest->flags));
    p_dest->p_peer = NULL;
    p_dest->origin = p_src->origin;
    memcpy(&p_dest->nexthop,&p_src->nexthop,sizeof(tBGP4_ADDR));
    memcpy(&p_dest->nexthop_global,&p_src->nexthop_global,sizeof(tBGP4_ADDR));
    memcpy(&p_dest->nexthop_local,&p_src->nexthop_local,sizeof(tBGP4_ADDR));
    p_dest->rcvd_med = p_src->rcvd_med;
    p_dest->out_med = p_src->out_med;
    p_dest->rcvd_localpref = p_src->rcvd_localpref;
    p_dest->out_localpref = p_src->out_localpref;

    LST_LOOP(&p_src->aspath_list, p_src_aspath, node, tBGP4_ASPATH)
    {
        p_aspath = (tBGP4_ASPATH*)bgp4_malloc(sizeof(tBGP4_ASPATH),MEM_BGP_ASPATH);
        bgp4_lstnodeinit(&p_aspath->node);
        if (p_aspath == NULL)
        {
            continue ;
        }
        p_aspath->p_asseg = (u_char*)bgp4_malloc(p_src_aspath->len * 2, MEM_BGP_BUF);
        if (p_aspath->p_asseg == NULL)
        {
            bgp4_free(p_aspath, MEM_BGP_ASPATH);
            continue ;
        }
        p_aspath->type = p_src_aspath->type;
        p_aspath->len = p_src_aspath->len;
        memcpy (p_aspath->p_asseg, p_src_aspath->p_asseg, (p_src_aspath->len * 2));
        bgp4_lstadd(&p_dest->aspath_list, &p_aspath->node);
    }

    memcpy(p_dest->community, p_src->community, sizeof(p_src->community));
    memcpy(p_dest->ecommunity,p_src->ecommunity,sizeof(p_src->ecommunity));

    p_dest->p_instance = p_src->p_instance;

    return ;
}

/*decide if two path is same*/
int  bgp4_aspath_same (tBGP4_PATH *p_path1,
                                     tBGP4_PATH *p_path2)
{
    tBGP4_ASPATH *p_aspath1 = NULL;
    tBGP4_ASPATH *p_aspath2 = NULL;

    if ((p_path1 == NULL)||(p_path2 == NULL))
    {
        return FALSE;
    }
    p_aspath1 = (tBGP4_ASPATH *)bgp4_lstfirst(&p_path1->aspath_list);
    p_aspath2 = (tBGP4_ASPATH *)bgp4_lstfirst(&p_path2->aspath_list);

    while ( (p_aspath1 != NULL) && (p_aspath2 != NULL) )
    {
        if ((p_aspath1->type != p_aspath2->type) ||
            ( p_aspath1->len != p_aspath2->len))
        {
            return FALSE;
        }
        if ( memcmp(p_aspath1->p_asseg,
            p_aspath2->p_asseg,
            (p_aspath1->len * 2)) != 0)
        {
            return FALSE;
        }
        p_aspath1 = (tBGP4_ASPATH *)bgp4_lstnext(&p_path1->aspath_list, &p_aspath1->node);
        p_aspath2 = (tBGP4_ASPATH *)bgp4_lstnext(&p_path2->aspath_list, &p_aspath2->node);
    }

    /* Paths are identical. Entire ASPATH has been checked. */
    if ( (p_aspath1 == NULL) && (p_aspath2 == NULL))
    {
        return TRUE;
    }

    return FALSE;
}

/*calculate aspath attribute length*/
u_short bgp4_path_aspath_len(tBGP4_PATH *p_path)
{
    u_short len = 0;
    tBGP4_ASPATH  *p_aspath = NULL;

    LST_LOOP(&p_path->aspath_list, p_aspath, node, tBGP4_ASPATH)
    {
        len += p_aspath->len;
    }

    return len;
}

/*decide if two path is same*/
int  bgp4_same_path (tBGP4_PATH *p_path1,
                                       tBGP4_PATH *p_path2)
{
    if ((p_path1 == NULL) || (p_path2 == NULL))
    {
        bgp4_log(BGP_DEBUG_RT,1,"one of the path is null");
        return FALSE;
    }
    if(p_path1->p_instance != p_path2->p_instance)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path src instance mismatch");
        return FALSE;
    }
    if (p_path1->p_peer!= p_path2->p_peer)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path peer mismatch");
        return FALSE;
    }

    if(p_path1->afi != p_path2->afi ||
        p_path1->safi != p_path2->safi)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path afi mismatch");
        return FALSE;
    }
    if (memcmp(&p_path1->flags, &p_path2->flags, sizeof(p_path2->flags)))
    {
        bgp4_log(BGP_DEBUG_RT,1,"path flags mismatch");
        return FALSE;
    }
    if (p_path1->origin != p_path2->origin)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path origin mismatch");
        return FALSE;
    }
    if (bgp4_aspath_same (p_path1, p_path2)== FALSE)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path as path mismatch");
        return FALSE;
    }
    if (memcmp(p_path1->nexthop.ip,p_path2->nexthop.ip,sizeof(p_path2->nexthop.ip))!=0)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path nexthop mismatch");
        return FALSE;
    }
    if (p_path1->rcvd_med != p_path2->rcvd_med)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path received med mismatch");
        return FALSE;
    }
    if (p_path1->rcvd_localpref != p_path2->rcvd_localpref)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path received localpref mismatch");
        return FALSE;
    }
    if (p_path1->out_med != p_path2->out_med)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path out med mismatch");
        return FALSE;
    }
    if (p_path1->out_localpref != p_path2->out_localpref)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path out localpref mismatch");
        return FALSE;
    }
    if (p_path1->aggregator.asnum != p_path2->aggregator.asnum)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path aggregator.asnum mismatch");
        return FALSE;
    }
    if (bgp4_prefixcmp(&p_path1->aggregator.addr,&p_path2->aggregator.addr)!=0)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path aggregator.addr mismatch");
        return FALSE;
    }
    if(memcmp(p_path1->nexthop_global.ip,p_path2->nexthop_global.ip,sizeof(p_path2->nexthop_global.ip))!=0)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path nexthop_global mismatch");
        return FALSE;
    }
    if(memcmp(p_path1->nexthop_local.ip,p_path2->nexthop_local.ip,sizeof(p_path2->nexthop_local.ip))!=0)
    {
        bgp4_log(BGP_DEBUG_RT,1,"path nexthop_local mismatch");
        return FALSE;
    }
    if(memcmp(p_path1->community,p_path2->community,sizeof(p_path2->community)))
    {
        bgp4_log(BGP_DEBUG_RT,1,"path community mismatch");
        return FALSE;
    }
    if(memcmp(p_path1->ecommunity,p_path2->ecommunity,sizeof(tBGP4_ECOMMUNITY)*BGP4_MAX_ECOMMNUITY))
    {
        bgp4_log(BGP_DEBUG_RT,1,"path ecommunity mismatch");
        return FALSE;
    }

    return TRUE;
}

/*merge same path for space reduction*/
tBGP4_PATH *bgp4_path_merge(tBGP4_PATH *p_path)
{
    tBGP4_PATH *p_old_path = NULL;
    tBGP4_ROUTE* p_route = NULL;
    tBGP4_ROUTE* p_next = NULL;
    u_int af = 0;

    if(p_path == NULL)
    {
        return NULL;
    }

    p_old_path = bgp4_path_lookup(p_path);

    if(p_old_path &&
        p_path != p_old_path)
    {
        /*routes in path to be deleted should be linked to old path*/
        LST_LOOP_SAFE(&p_path->route_list, p_route,p_next,node, tBGP4_ROUTE)
        {
            bgp4_lstdelete(&p_path->route_list,&p_route->node);

            bgp4_link_path(p_old_path,p_route);
        }

        bgp4_path_free(p_path);

        return p_old_path;
    }
    else
    {
        /*add path to global list*/
        af = bgp4_afi_to_index(p_path->afi,p_path->safi);
        if(af < BGP4_PF_MAX)
        {
            bgp4_path_add_to_list(p_path,af);
            return p_path;

        }
        else
        {
            return NULL;
        }
    }
}

/*free aspath of path*/
void bgp4_apath_list_free(tBGP4_PATH *p_path)
{
    tBGP4_ASPATH *p_aspath = NULL;
    tBGP4_ASPATH *p_next = NULL;

    LST_LOOP_SAFE(&p_path->aspath_list, p_aspath, p_next, node, tBGP4_ASPATH)
    {
        bgp4_lstdelete(&p_path->aspath_list, &p_aspath->node);
        bgp4_free(p_aspath->p_asseg, MEM_BGP_BUF);
        bgp4_free(p_aspath, MEM_BGP_ASPATH);
    }

    bgp4_lstinit(&p_path->aspath_list);

    return ;
}
#if 0
/*substitute for a specific AS num in a aspath list*/
u_int bgp4_asseq_substitution(tBGP4_LIST *p_list, u_short as)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_short *p_as = NULL;
    u_char i;

    LST_LOOP(p_list, p_aspath, node, tBGP4_ASPATH)
    {
        p_as = (u_short *)p_aspath->p_asseg;
        for (i = 0 ; i < p_aspath->len ; i++, p_as++ )
        {
            if (ntohs(*p_as) == as)
            {
                *p_as = htons(gBgp4.asnum) ;
            }
        }
    }

    return FALSE;
}
#else
/*substitute for a specific AS num in a aspath list*/
u_int bgp4_asnum_rebuild(tBGP4_ASPATH *p_aspath, u_char*p_dst_buf,tBGP4_PEER*p_dst_peer,u_char remove_flag)
{
    u_short *p_as = NULL;
    u_char i = 0;
    u_int len = 0;

    if(p_aspath == NULL || p_dst_buf == NULL || p_dst_peer == NULL)
    {
        return 0;
    }

    /*point to first AS num*/
    p_as = (u_short *)p_aspath->p_asseg;

    for (i = 0 ; i < p_aspath->len ; i++, p_as++ )
    {
        /*if substitute dst peer's AS num*/
        if(p_dst_peer->as_substitute_enable &&
                ntohs(*p_as) == p_dst_peer->remote.as)
        {
            bgp4_put_2bytes(p_dst_buf,gBgp4.asnum);
            len +=2;
        }
        /*filter private as num*/
        else if(remove_flag &&
                ntohs(*p_as) >= BGP4_PRIVATE_AS_MIN)
        {
            continue;
        }
        else
        {
            memcpy(p_dst_buf,p_as,2);
            len += 2;
            p_dst_buf += 2;
        }
    }

    return len;
}
#endif

/*lookup as in a aspath list*/
u_int bgp4_asseq_exist(tBGP4_LIST *p_list, u_short as)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_short *p_as = NULL;
    u_char i;

    LST_LOOP(p_list, p_aspath, node, tBGP4_ASPATH)
    {
#if 0/*conformance test,AS_SET type should also be checked*/
        if (p_aspath->type != BGP_ASPATH_SEQ)
        {
            continue ;
        }
#endif
        p_as = (u_short *)p_aspath->p_asseg;
        for (i = 0 ; i < p_aspath->len ; i++, p_as++ )
        {
            if (ntohs(*p_as) == as)
            {
                return TRUE ;
            }
        }
    }

    return FALSE;
}

/*lookup as loop times in a aspath list*/
u_int bgp4_as_loop_times(tBGP4_LIST *p_list, u_short as)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_short *p_as = NULL;
    u_char i = 0;
    u_int loop_times = 0;

    LST_LOOP(p_list, p_aspath, node, tBGP4_ASPATH)
    {
        p_as = (u_short *)p_aspath->p_asseg;
        for (i = 0 ; i < p_aspath->len ; i++, p_as++ )
        {
            if (ntohs(*p_as) == as)
            {
                loop_times++ ;
            }
        }
    }

    return loop_times;
}

/*lookup as in a aspath list*/
u_int bgp4_asseq_if_exist_pub_as(tBGP4_LIST *p_list)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_short *p_as = NULL;
    u_char i;

    LST_LOOP(p_list, p_aspath, node, tBGP4_ASPATH)
    {
        p_as = (u_short *)p_aspath->p_asseg;
        for (i = 0 ; i < p_aspath->len ; i++, p_as++ )
        {
            if (ntohs(*p_as)  < BGP4_PRIVATE_AS_MIN)
            {
                return TRUE ;
            }
        }
    }

    return FALSE;
}


/*decide new aspath length for sending*/
u_short  bgp4_aspath_len(tBGP4_PATH *p_path, u_int peertype)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_short   len = 0;
    u_char seq_exist = FALSE;

    if ((p_path != NULL) && (bgp4_lstfirst(&p_path->aspath_list)))
    {
        if (peertype == BGP4_EBGP)
        {
            /*contain normal as set/seq*/
            LST_LOOP(&p_path->aspath_list,p_aspath, node, tBGP4_ASPATH)
            {
                if ((p_aspath->type == BGP_ASPATH_CONFED_SET)||
                    (p_aspath->type == BGP_ASPATH_CONFED_SEQ))
                {
                    continue;
                }

                if (p_aspath->type == BGP_ASPATH_SEQ&&
                    (seq_exist == FALSE))
                {
                    seq_exist = TRUE;
                }

                len += 1 + 1;
                len += (p_aspath->len * 2 );

            }

            /*just add an as-num,or add a as-seq part*/
            if (seq_exist == TRUE)
            {
                len += 2;
            }
            else
            {
                len += 4;
            }
        }
        else
        {
            /*include all current as-path*/
            LST_LOOP(&p_path->aspath_list,p_aspath, node, tBGP4_ASPATH)
            {
                /* Seg. type and length field */
                len += 1 + 1;

                /* AS path length is in 2 bytes (int) format */
                len += (p_aspath->len * 2 );
            }

            /*confederation peer:add as-num or seq*/
            if (peertype == BGP4_CONFEDBGP)
            {
                p_aspath = (tBGP4_ASPATH *)bgp4_lstfirst(&p_path->aspath_list);
                if (p_aspath->type == BGP_ASPATH_CONFED_SEQ)
                {
                    len += 2;
                }
                else
                {
                    len += 4;
                }
            }
        }
    }
    else  if (peertype !=BGP4_IBGP)
    {
        /* Needs to be filled by us */
        len += 4;
    }

    return len;
}

/*decide if loop in cluster id*/
STATUS  bgp4_check_clusterlist(tBGP4_PATH *p_info )
{
    u_int i ;

    for (i = 0 ; i < BGP4_MAX_COMMNUITY ; i++)
    {
        if (gBgp4.cluster_id && (gBgp4.cluster_id == p_info->cluster[i]))
        {
            return VOS_ERR;
        }
    }

    return VOS_OK;
}

/*decide loop using origin*/
STATUS  bgp4_check_originid(tBGP4_PATH *p_info )
{
    if (gBgp4.is_reflector == TRUE)
    {
        return (p_info->origin_id == gBgp4.router_id) ? VOS_ERR : VOS_OK;
    }

    return VOS_OK;
}


/*verify rxd path,check if loop detected and update some attribute*/
STATUS bgp4_verify_path(tBGP4_PEER *p_peer, tBGP4_PATH *p_path)
{
    u_int local_as = (p_peer->fake_as!=gBgp4.asnum) ? p_peer->fake_as : gBgp4.asnum;

    /*loop check by aspath*/
    if(p_peer->allow_as_loop_times &&
            bgp4_as_loop_times(&p_path->aspath_list, local_as) != p_peer->allow_as_loop_times)
    {
        return VOS_ERR;
    }
    else if (bgp4_asseq_exist(&p_path->aspath_list, local_as) == TRUE)
    {
        return VOS_ERR;
    }

    /*loop check by origin and cluster*/
    if (bgp4_check_originid(p_path) != VOS_OK)
    {
        return VOS_ERR;
    }
    if (bgp4_check_clusterlist(p_path) != VOS_OK)
    {
        return VOS_ERR;
    }
#if 0
    /*decide direct nexthop*/
    /*add by chengyq to enable IBGP,2002-01-16*/
    if((is_ibgp_peer(p_peer))&&(!gBgp4.sync_enable  == TRUE))
    {
        bgp_ip4(p_path->direct_nexthop.ip)  = bgp_ip4(p_peer->remote.ip.ip);
    }
#endif
    return VOS_OK;
}

static tBGP4_ATTR_DESC desc[256];
#define DESC_INIT(x, fl, u, z, f) do{desc[x].flag = (fl); desc[x].unitlen = (u); desc[x].zerolen = (z); desc[x].fixlen = (f);}while(0)
void bgp4_attribute_desc_init()
{
    memset(desc, 0, sizeof(desc));

    DESC_INIT(BGP4_ATTR_ORIGIN, BGP4_ATTR_FLAG_TRANSIT, 0, 0, BGP4_ORIGIN_LEN);
    DESC_INIT(BGP4_ATTR_PATH, BGP4_ATTR_FLAG_TRANSIT, 0, 0, 0);
    DESC_INIT(BGP4_ATTR_NEXT_HOP, BGP4_ATTR_FLAG_TRANSIT, 0, 0, BGP4_NEXTHOP_LEN);
    DESC_INIT(BGP4_ATTR_MED, BGP4_ATTR_FLAG_OPT, 0, 0, BGP4_MED_LEN);
    DESC_INIT(BGP4_ATTR_LOCAL_PREF, BGP4_ATTR_FLAG_TRANSIT, 0, 0, BGP4_LPREF_LEN);
    DESC_INIT(BGP4_ATTR_ATOMIC_AGGR, BGP4_ATTR_FLAG_TRANSIT, 0, 1, 0);
    DESC_INIT(BGP4_ATTR_AGGREGATOR, BGP4_ATTR_FLAG_TRANSIT|BGP4_ATTR_FLAG_OPT, 0, 0, BGP4_AGGR_LEN);
    DESC_INIT(BGP4_ATTR_COMMUNITY, BGP4_ATTR_FLAG_TRANSIT|BGP4_ATTR_FLAG_OPT, 4, 0, 0);
    DESC_INIT(BGP4_ATTR_ORIGINATOR, BGP4_ATTR_FLAG_OPT, 0, 0, BGP4_ORIGINATOR_LEN);
    DESC_INIT(BGP4_ATTR_CLUSTERLIST, BGP4_ATTR_FLAG_OPT, 4, 0, 0);
    DESC_INIT(BGP4_ATTR_MP_NLRI, BGP4_ATTR_FLAG_OPT, 0,0,0);
    DESC_INIT(BGP4_ATTR_MP_UNREACH_NLRI, BGP4_ATTR_FLAG_OPT,0,0,0);
    DESC_INIT(BGP4_ATTR_EXT_COMMUNITY,BGP4_ATTR_FLAG_TRANSIT|BGP4_ATTR_FLAG_OPT,4,0,0);

    return ;
}

int bgp4_extract_path_attribute(
                   tBGP4_PATH * p_path,
                   u_char  *p_buf,
                   int len,
                   tBGP4_LIST *p_flist ,
                   tBGP4_LIST *p_wlist)
{
    u_int subcode = 0 ;
    u_int payload_len;
    u_char hlen = 0;
    u_char flag = 0;
    u_char type = 0;
    u_char seen[256] = {0};
    u_char *p_err = NULL ;
    u_int err_len = 0 ;
    STATUS ret = 0;

    /*check each attribute*/
    while (len > 0)
    {
        flag = *p_buf;
        if (flag & BGP4_ATTR_FLAG_EXT)
        {
            bgp4_get_2bytes(payload_len, (p_buf+2));
            hlen = 4;
        }
        else
        {
            payload_len = *(p_buf + 2);
            hlen = 3;
        }

        type = *(p_buf + 1);
        /*can appear once*/
        if (seen[type])
        {
            bgp4_log(BGP_DEBUG_UPDATE,1,"attribute appear multiple ,attribute type %d",type);

            subcode = BGP4_MALFORMED_ATTR_LIST ;

            goto SEND_ERROR ;
        }
        seen[type] = 1;

        /*decide flag*/
        if (desc[type].flag
            && (desc[type].flag != (flag & (BGP4_ATTR_FLAG_OPT | BGP4_ATTR_FLAG_TRANSIT))))
        {
            bgp4_log(BGP_DEBUG_UPDATE,1,"invalid attribute flag,attribute type %d,flag %02x",type, flag);

            subcode = BGP4_ATTR_FLAG_ERR ;
            p_err = p_buf ;
            err_len = payload_len + hlen;
            goto SEND_ERROR ;
        }
        /*check length*/
        if ((desc[type].zerolen && payload_len) ||
            (desc[type].fixlen && (payload_len != desc[type].fixlen)) ||
            (desc[type].unitlen && (payload_len%desc[type].unitlen)))
        {
            bgp4_log(BGP_DEBUG_UPDATE,1,"invalid attribute len,attribute type %d,payload len %d",type, payload_len);

            subcode = BGP4_ATTR_LEN_ERR;
            p_err = p_buf ;
            err_len = payload_len + hlen;
            goto SEND_ERROR ;
        }

        switch (type) {
            case BGP4_ATTR_ORIGIN       :
                ret = bgp4_extract_origin(p_path, p_buf, hlen, payload_len);
                break;

            case BGP4_ATTR_PATH         :
                ret = bgp4_extract_aspath(p_path, p_buf, hlen, payload_len);
                break;

            case BGP4_ATTR_NEXT_HOP     :
                ret = bgp4_extract_nexthop(p_path, p_buf, hlen, payload_len);
                break;

            case BGP4_ATTR_MED          :
                ret = bgp4_extract_med(p_path, p_buf, hlen, payload_len);
                break;

            case BGP4_ATTR_LOCAL_PREF   :
                ret = bgp4_extract_lpref(p_path, p_buf, hlen, payload_len);
                break;

            case BGP4_ATTR_ATOMIC_AGGR  :
                ret = bgp4_extract_atomic(p_path, p_buf, hlen, payload_len);
                break;

            case BGP4_ATTR_AGGREGATOR   :
                ret = bgp4_extract_aggr(p_path, p_buf, hlen, payload_len);
                break;

            case BGP4_ATTR_COMMUNITY:
                ret = bgp4_extract_com(p_path, p_buf, hlen, payload_len);
                break;

            case BGP4_ATTR_ORIGINATOR:
                ret = bgp4_extract_originator(p_path, p_buf, hlen, payload_len);
                break;

            case BGP4_ATTR_CLUSTERLIST:
                ret = bgp4_extract_clusterlist(p_path, p_buf, hlen, payload_len);
                break;

            case BGP4_ATTR_EXT_COMMUNITY :
                ret = bgp4_extract_extcom(p_path, p_buf, hlen, payload_len);
                break ;

            case BGP4_ATTR_MP_NLRI:
                ret = bgp4_extract_mpreach_nlri(p_path, p_buf, hlen, payload_len, p_flist);
                break ;

            case BGP4_ATTR_MP_UNREACH_NLRI :
                ret = bgp4_extract_mpunreach_nlri(p_path, p_buf, hlen, payload_len,p_wlist);
                break ;

            default :
                ret = bgp4_extract_unkown(p_path,p_buf, hlen, payload_len);
                break;
        }
        if (ret == VOS_ERR)
        {
            return FALSE;
        }
        /*move buffer to next*/
        len -= (hlen + payload_len);
        p_buf += (hlen + payload_len);
    }

    type = 0;
    /*some attribute must appear*/

    /*in cisco,when mpbgp update sent,do not include nexthop attribute!,so in this case
    do not return failed.*/
    if (!seen[BGP4_ATTR_ORIGIN] && !seen[BGP4_ATTR_MP_UNREACH_NLRI])
    {
        type = BGP4_ATTR_ORIGIN;
    }

    if (!type && !seen[BGP4_ATTR_PATH] && !seen[BGP4_ATTR_MP_UNREACH_NLRI])
    {
        type = BGP4_ATTR_PATH;
    }

    if (!type && !seen[BGP4_ATTR_NEXT_HOP] &&
        !seen[BGP4_ATTR_MP_UNREACH_NLRI] &&
        !seen[BGP4_ATTR_MP_NLRI])
    {
        type = BGP4_ATTR_NEXT_HOP;
    }

    if (type)
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"missing wellknown attribute type %d",type);

        subcode = BGP4_MISSING_WELLKNOWN_ATTR ;
        p_err = &type ;
        err_len = 1 ;
        goto SEND_ERROR ;
    }

    /* check lp attribute*/
    if (!seen[BGP4_ATTR_LOCAL_PREF]
        && is_ibgp_peer(p_path->p_peer)
        && !seen[BGP4_ATTR_MP_UNREACH_NLRI])
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"missing wellknown attribute type %d",type);

        type = BGP4_ATTR_LOCAL_PREF;
        subcode = BGP4_MISSING_WELLKNOWN_ATTR ;
        p_err = &type ;
        err_len = 1 ;
        goto SEND_ERROR ;
    }

    return (TRUE);

    SEND_ERROR :
    bgp4_send_notify(p_path->p_peer, BGP4_UPDATE_MSG_ERR, subcode , p_err, err_len);
    return FALSE ;
}


/*get attribute from msg*/
int bgp4_extract_origin(tBGP4_PATH *p_path, u_char *p_buf, u_char hlen, u_short len)
{
    p_path->origin = p_buf[hlen];
    if ((p_path->origin != BGP4_ORIGIN_IGP) &&
        (p_path->origin != BGP4_ORIGIN_EGP) &&
        (p_path->origin != BGP4_ORIGIN_INCOMPLETE))
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"invalid origin value %d",p_path->origin);

        bgp4_send_notify(p_path->p_peer, BGP4_UPDATE_MSG_ERR, BGP4_INVALID_ORIGIN, p_buf, len+hlen);

        return VOS_ERR;
    }

    return VOS_OK;
}

int bgp4_extract_aspath(tBGP4_PATH *p_path, u_char *p_buf, u_char hlen, short len)
{
    tBGP4_ASPATH * p_aspath = NULL;
    INT1 aslen = 2;/*as length,default 2 bytes*/

    p_buf += hlen;
    while (len > 0)
    {
        p_aspath = (tBGP4_ASPATH*)bgp4_malloc(sizeof(tBGP4_ASPATH),MEM_BGP_ASPATH);
        if (p_aspath == NULL)
        {
            bgp4_log(BGP_DEBUG_CMN,1,"not enough memory for feasiable list!!");
            return VOS_ERR;
        }

        bgp4_lstnodeinit(&p_aspath->node);
        bgp4_lstadd_tail(&p_path->aspath_list, &p_aspath->node);

        if ((*p_buf  != BGP_ASPATH_SET) &&
            (*p_buf != BGP_ASPATH_SEQ)&&
            (*p_buf != BGP_ASPATH_CONFED_SEQ)&&
            (*p_buf != BGP_ASPATH_CONFED_SET))
        {
            bgp4_send_notify(p_path->p_peer,BGP4_UPDATE_MSG_ERR,
                    BGP4_MALFORMED_AS_PATH,NULL,0);

            return VOS_ERR;
        }

        p_aspath->type =  *p_buf;
        p_buf++;

        p_aspath->len = *p_buf;
        p_buf++;

        if (p_aspath->len != 0)
        {
            p_aspath->p_asseg = (u_char*)bgp4_malloc(p_aspath->len*aslen, MEM_BGP_BUF);
            if (p_aspath->p_asseg)
            {
                memcpy(p_aspath->p_asseg, p_buf, (p_aspath->len)*aslen);
            }
            else
                bgp4_log(BGP_DEBUG_CMN,1,"bgp4 malloc as path failed!");
        }

        /*update length and buffer*/
        len -= (p_aspath->len*aslen + 2);
        p_buf += (p_aspath->len*aslen);
    }

    return VOS_OK;
}

STATUS bgp4_verify_nexthop(u_int nexhop)
{
    u_char addrstr[16];

    bgp4_log(BGP_DEBUG_UPDATE,1,"verify nexthop %s",inet_ntoa_1(addrstr,nexhop));

    if ((nexhop == 0) || (nexhop == 0xffffffff) || (nexhop == 0xffffff00))
    {
        return VOS_ERR;
    }

    if (((nexhop&0xff000000) == 0x7f000000)|| ((nexhop&0x000000FF)==0xff))
    {
        return VOS_ERR;
    }

    if ((nexhop&0xf8000000) == 0xf0000000)
    {
        return VOS_ERR;
    }

    if ((nexhop&0xf8000000) ==0xf8000000)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}

int bgp4_extract_nexthop(tBGP4_PATH *p_path, u_char *p_buf, u_char hlen, short len)
{
    u_int code = BGP4_UPDATE_MSG_ERR ;
    u_int subcode = BGP4_INVALID_NEXTHOP ;
    u_char addr[16];

    bgp4_get_4bytes(bgp_ip4(p_path->nexthop.ip), (p_buf+hlen));

    if (bgp4_verify_nexthop(*(u_int*)p_path->nexthop.ip) != VOS_OK)
    {
        bgp4_log(BGP_DEBUG_UPDATE, 1, "invalid nexthop %s",inet_ntoa_1(addr, *(u_int*)p_path->nexthop.ip));
        goto SEND_ERROR ;
    }

    p_path->nexthop.afi = BGP4_PF_IPUCAST;

    /* Check for Semantic correctness - Sharing the same subnet */
    if ((bgp4_peer_type(p_path->p_peer))  == BGP4_EBGP )
    {
        u_int vrf_id = 0;
        M2_IPROUTETBL nexthop_metric = {0};
        
        vrf_id = p_path->src_instance_id;
        if(bgp4_get_ifmask(p_path->p_peer->if_unit,p_path->p_peer->local.ip.afi) == 32)
        {
            if(ip_route_match(vrf_id, bgp_ip4(p_path->nexthop.ip), nexthop_metric)!= VOS_OK)
                goto SEND_ERROR;
        }
            else if (bgp4_subnet_match(&p_path->nexthop,
                    &p_path->p_peer->local.ip, p_path->p_peer->if_unit) == FALSE)
        {
            bgp4_log(BGP_DEBUG_UPDATE, 1,"nexthop is not on our subnet,peer unit %d",p_path->p_peer->if_unit);
            goto SEND_ERROR ;
        }
    }

    /* Check for semantic correctness - Same Subnet - Apr/02 */
    if (gBgp4.server_port == gBgp4.client_port &&/*more than one bgp entity using the same IP ADDR for testing*/
        (*(u_int*)p_path->nexthop.ip ==*(u_int*)p_path->p_peer->local.ip.ip))
    {
        bgp4_log(BGP_DEBUG_UPDATE, 1,"nexthop is local ip address,INVALID");
        return VOS_ERR;
    }
    return VOS_OK;

    SEND_ERROR :
    bgp4_send_notify(p_path->p_peer, code, subcode, p_buf, len + hlen);
    return VOS_ERR;
}

int bgp4_extract_med(tBGP4_PATH *p_path, u_char *p_buf, u_char hlen, short len)
{
    bgp4_get_4bytes(p_path->rcvd_med, p_buf+hlen);
    p_path->flags.med = TRUE ;

    /*default action:do not modify rxd lpref and med*/
    p_path->out_med = p_path->rcvd_med;

    return VOS_OK;
}

int bgp4_extract_lpref(tBGP4_PATH *p_path, u_char *p_buf, u_char hlen, short len)
{
    bgp4_get_4bytes(p_path->rcvd_localpref, p_buf+hlen);
    p_path->flags.lp = TRUE ;

    if(p_path->p_peer)
    {
        /*LP value of EBGP should be ignored*/
        if(bgp4_peer_type(p_path->p_peer) != BGP4_EBGP)
        {
            /*default action:do not modify rxd lpref and med*/
            p_path->out_localpref = p_path->rcvd_localpref;
        }
    }

    return VOS_OK;
}

int bgp4_extract_atomic(tBGP4_PATH *p_path, u_char *p_buf, u_char hlen, short len)
{
    p_path->flags.atomic = TRUE;
    return VOS_OK;
}

int bgp4_extract_aggr(tBGP4_PATH *p_path, u_char *p_buf, u_char hlen, short len)
{
    u_short expect = BGP4_AGGR_LEN;

    if (expect == BGP4_AGGR_LEN)
    {
        bgp4_get_2bytes(p_path->aggregator.asnum, (p_buf+hlen));
        bgp4_get_4bytes(bgp_ip4(p_path->aggregator.addr.ip), (p_buf+hlen+2));
    }

    if ( (*(u_int*)p_path->aggregator.addr.ip == 0)
            || (*(u_int*)p_path->aggregator.addr.ip == 0xFFFFFFFF)
            || (*(u_int*)p_path->aggregator.addr.ip == 0x7F000001))
    {
        bgp4_send_notify(p_path->p_peer,BGP4_UPDATE_MSG_ERR,
                BGP4_OPTIONAL_ATTR_ERR,p_buf,len+hlen);
        return VOS_ERR;
    }
    p_path->flags.aggr = TRUE ;

    return VOS_OK;
}

int bgp4_extract_originator(tBGP4_PATH *p_path, u_char *p_buf, u_char hlen, short len)
{
    bgp4_get_4bytes(p_path->origin_id, (p_buf+hlen));
    if (p_path->origin_id == 0)
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"invalid null originator id ");
        bgp4_send_notify(p_path->p_peer,BGP4_UPDATE_MSG_ERR,
                BGP4_OPTIONAL_ATTR_ERR,p_buf,len+hlen);

        return VOS_ERR;
    }

    return VOS_OK;
}

int bgp4_extract_clusterlist(tBGP4_PATH *p_path, u_char *p_buf, u_char hlen, short len)
{
    u_char total = len/4;
    if (total > BGP4_MAX_CLUSTERID)
    {
        total = BGP4_MAX_CLUSTERID;
    }
    memcpy(p_path->cluster, p_buf+hlen, total*4);

    return VOS_OK;
}

int bgp4_extract_unkown(tBGP4_PATH *p_path, u_char *p_buf, u_char hlen, short len)
{
    u_char flag = 0;
    u_int  oldlen = 0;
    u_char *p_str = NULL;

    flag = *p_buf;

    if (!(flag & BGP4_ATTR_FLAG_OPT))
    {
        bgp4_send_notify(p_path->p_peer,BGP4_UPDATE_MSG_ERR,
                BGP4_UNRECOGNISED_WELLKNOWN_ATTR, p_buf,len);
        return VOS_ERR;
    }

    /*set partial flag*/
    if (!(flag & BGP4_ATTR_FLAG_TRANSIT))
    {
        return VOS_OK;
    }

    *p_buf |= BGP4_ATTR_FLAG_PARTIAL;

    if (p_path->p_unkown != NULL)
    {
        oldlen = p_path->unknown_len ;

        p_str = (u_char*)bgp4_malloc(oldlen + len + hlen, MEM_BGP_BUF);
        if (p_str == NULL)
        {
            return VOS_OK;
        }

        memcpy(p_str, p_path->p_unkown, oldlen);

        bgp4_free(p_path->p_unkown, MEM_BGP_BUF);
        p_path->p_unkown = p_str;
    }
    else
    {
        oldlen = 0;
        p_path->p_unkown = (u_char*)bgp4_malloc(len, MEM_BGP_BUF);
        if (p_path->p_unkown == NULL)
        {
            return VOS_OK;
        }
    }

    memcpy ((p_path->p_unkown + oldlen), p_buf, len + hlen);
    p_path->unknown_len=oldlen+len+hlen;

    return VOS_OK;
}

int bgp4_extract_com(tBGP4_PATH *p_path, u_char *p_buf, u_char hlen, short len)
{
    u_char total = len/4;
    u_char i;

    if (total > BGP4_MAX_COMMNUITY)
    {
        total = BGP4_MAX_COMMNUITY;
    }

    memcpy(p_path->community, p_buf + 3, total*4);
    for (i = 0 ; i < total ; i++)
    {
        if (p_path->community[i] == BGP4_COMMUNITY_NOEXPORT)
        {
            p_path->flags.notto_external = TRUE;
        }
        else if (p_path->community[i] == BGP4_COMMUNITY_NOADV)
        {
            p_path->flags.notto_external = TRUE ;
            p_path->flags.notto_internal = TRUE ;
        }
        else if (p_path->community[i] == BGP4_COMMUNITY_NOEXPORT_SUSPEND)
        {
            p_path->flags.notto_internal = TRUE ;
        }
    }

    return VOS_OK;
}

int bgp4_extract_mpnexthop(
               u_char len,
               u_char *p_buf,
               u_short afi,
               u_char safi,
               tBGP4_PATH *p_path)
{
    u_char nexthop[64];
    u_int bgp4_afi;

    bgp4_afi=bgp4_afi_to_index(afi,safi);

    /*get nexthop*/
    if ((afi == BGP4_AF_IP) || (afi == BGP4_AF_L2VPN))
    {
        if (safi == BGP4_SAF_VLBL )
        {
            if (len != 12)
            {
                bgp4_log(BGP_DEBUG_UPDATE,1,"mpbgp nexthop,invalid nexthop length %d",len);
                return FALSE ;
            }

            bgp4_get_4bytes(bgp_ip4(p_path->nexthop.ip), (p_buf + 8));
            /*if nexthop is zero,use neighbor address*/
            if (*(u_int*)p_path->nexthop.ip == 0)
            {
                *(u_int*)p_path->nexthop.ip  = *(u_int*)p_path->p_peer->remote.ip.ip ;
            }
        }
        else
        {
            if (len != 4)
            {
                bgp4_log(BGP_DEBUG_UPDATE,1,"mpbgp nexthop,invalid nexthop length %d",len);
                return FALSE ;
            }
            p_path->nexthop.afi=BGP4_PF_IPUCAST;
            bgp4_get_4bytes(bgp_ip4(p_path->nexthop.ip), p_buf);
            /*if nexthop is zero,use neighbor address*/
            if (*(u_int*)p_path->nexthop.ip  == 0)
            {
                *(u_int*)p_path->nexthop.ip  = *(u_int*)p_path->p_peer->remote.ip.ip;
            }
        }
        bgp4_log(BGP_DEBUG_UPDATE,1,"mpbgp nexthop:%s",inet_ntoa_1(nexthop,bgp_ip4(p_path->nexthop.ip)));
    }
    else/*BGP4_AF_IP6*/
    {
        if (safi == BGP4_SAF_VLBL)
        {
             if ((len != 24) && (len != 48))
            {
                bgp4_log(BGP_DEBUG_UPDATE,1,"mpbgp nexthop,invalid nexthop length %d",len);
                return FALSE ;
            }

            /*8 zero RD offset*/
            memcpy(p_path->nexthop_global.ip,p_buf + 8,16);

            p_path->nexthop_global.afi = bgp4_afi;

            if(len==48)
            {
                memcpy(p_path->nexthop_local.ip,p_buf+24,16);
                p_path->nexthop_local.afi = bgp4_afi;
            }
            {
                u_char nexthop_addr[64];
                bgp4_log(BGP_DEBUG_UPDATE,1,"mpbgp ipv6 vpn nexthop %s",bgp4_printf_addr(&p_path->nexthop_global,nexthop_addr));
            }

        }
        else
        {
            if ((len != 16) && (len != 32))
            {
                bgp4_log(BGP_DEBUG_UPDATE,1, "mpbgp nexthop,invalid nexthop length %d",len);
                return FALSE ;
            }
            /*v6 tbi*/
            memcpy(p_path->nexthop_global.ip,p_buf ,16);

            p_path->nexthop_global.afi=bgp4_afi;

            if(len==32)
            {
                memcpy(p_path->nexthop_local.ip,p_buf+16,16);
                p_path->nexthop_local.afi=bgp4_afi;
            }
        }
    }

    return TRUE;
}

int bgp4_extract_mp_nlri(
                    tBGP4_PATH *p_path,
                    u_char *p_buf,
                    short len,
                    tBGP4_LIST *p_list)
{
    u_char plen = *(p_buf);
    u_char bytes = 0;
    u_char *p_nlri = (p_buf+1);
    u_int flag = 0;
    u_char label_len = 0 ;
    u_char addr_len = 0 ;
    tBGP4_ROUTE *p_route = NULL;
    u_int mask  = 0;

    bytes = bgp4_bit2byte(plen) ;

    if (len < (bytes + 1))
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"invalid attribute length");
        return 0;
    }

    /*check length,if valid*/
    flag = bgp4_afi_to_index(p_path->afi, p_path->safi) ;

    addr_len = (p_path->afi == BGP4_AF_IP) ? 4 : 16;

    if ((p_path->safi == BGP4_SAF_LBL) || (p_path->safi == BGP4_SAF_VLBL))
    {
        label_len = 3 ;
    }

    if (p_path->safi == BGP4_SAF_VLBL)
    {
        addr_len += 8;
    }

    if ((bytes < (label_len )) ||
        (bytes > (label_len  + addr_len)))
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,
            "invalid mpbgp nlri prefix length %d",plen);
        return 0 ;
    }

    p_route = bgp4_creat_route();
    if (p_route == NULL)
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"failed to allocate route ");
        return (0);
    }

    bgp4_link_path (p_path, p_route);
    /*protocol must be BGP4*/
    p_route->proto = M2_ipRouteProto_bgp ;

    /*fill infor*/
    p_route->dest.afi = flag ;

    if (label_len != 0)
    {
        memcpy(((u_char*)&p_route->vpn_label)+1,p_nlri,label_len);
        p_nlri += label_len ;
    }

    p_route->dest.prefixlen = plen - 8*label_len;  /*prefix include RD*/

    if (addr_len == 4)
    {
        bgp4_get_4bytes(*(u_int*)p_route->dest.ip, p_nlri);
        mask = bgp4_len2mask(p_route->dest.prefixlen);
        *(u_int*)p_route->dest.ip &= mask ;

        bgp4_log(BGP_DEBUG_UPDATE,1,"mpbgp nlri:%d.%d.%d.%d/%d",
                    *p_nlri,*(p_nlri +1),*(p_nlri + 2),
                    *(p_nlri + 3),p_route->dest.prefixlen);
    }
#ifdef BGP_IPV6_WANTED
    else if(addr_len==16)/*ipv6 route get*/
    {
        memset(p_route->dest.ip,0,16);
        memcpy(p_route->dest.ip,p_nlri,bytes);
        bgp4_ip6_prefix_make(p_route->dest.ip,p_route->dest.prefixlen);
    }
#endif
    else if(addr_len == 12 || addr_len == 24)/*mpls vpn*/
    {
        /*The Prefix field contains IP address prefixes followed by
        enough trailing bits to make the end of the field fall on an
        octet boundary. Note that the value of trailing bits is irrelevant.rfc 1771*/
        u_int byte = bgp4_bit2byte(p_route->dest.prefixlen);
        memset(p_route->dest.ip,0,sizeof(p_route->dest.ip));
        memcpy(p_route->dest.ip,p_nlri,byte);

        if(addr_len == 12)
        {
            bgp4_log(BGP_DEBUG_UPDATE,1,"mpbgp nlri:%d.%d.%d.%d/%d,af %d",
                        *(p_nlri+BGP4_VPN_RD_LEN),*(p_nlri +BGP4_VPN_RD_LEN+1),*(p_nlri + BGP4_VPN_RD_LEN+2),
                        *(p_nlri + BGP4_VPN_RD_LEN+3),p_route->dest.prefixlen,
                        p_route->dest.afi);
        }

    }

    bgp4_rtlist_add(p_list, p_route);

    return (bytes + 1);
}

int bgp4_extract_mpreach_nlri(
                                         tBGP4_PATH      *p_path,
                                         u_char *p_buf,
                                         u_char hlen,
                                         short len,
                                         tBGP4_LIST *p_list)
{
    u_char nhop_len = 0;
    u_short afi = 0;
    u_char safi = 0;
    u_short nlri_len = 0;
    u_int flag = 0;

    p_buf += hlen;
    bgp4_log(BGP_DEBUG_UPDATE,1,"get mpbgp reach nlri,length %d",len);

    /*must include AFI and SAFI*/
    if (len < 3)
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"invalid length");
        return VOS_ERR;
    }
    len -= 3 ;

    bgp4_get_2bytes(afi, p_buf);
    p_buf += 2;
    safi = *p_buf ;
    p_buf ++ ;

    /*check if neighbor support AFI,if not,only ignore the optional non-transitive attribute*/
    flag = bgp4_afi_to_index(afi, safi);
    if (!bgp4_af_support(p_path->p_peer, flag))
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,
            "neighbor not support afi/safi %d/%d",afi,safi);

        return VOS_OK;
    }
    /*next hop*/
    if (len < 1)
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"no memory for nexthop");
        goto MP_ERR;
    }

    nhop_len = *p_buf ;

    if (len < nhop_len)/*nexthop length of buffer*/
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,
                "length %d less than nexthop length %d",len,nhop_len);
        goto MP_ERR ;
    }
#if 0/*consider in bgp4_extract_mpnexthop later*/
    if(nhop_len!=16&&nhop_len!=32)
    {
        goto MP_ERR ;
    }
#endif
    len -- ;
    p_buf ++ ;

    p_path->afi = afi ;
    p_path->safi = safi ;

    /*check if the next hop is valid*/
    if (bgp4_extract_mpnexthop(nhop_len, p_buf, afi, safi, p_path) == FALSE)
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"MPBGP nexthop is invalid");

        goto MP_ERR ;
    }

    p_buf += nhop_len ;
    len -= nhop_len ;

    /*snpa must be 0*/
    if (*p_buf != 0)
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"invalid snpa count");
        return VOS_ERR;
    }

    len -- ;
    p_buf ++ ;

    /*get route*/
    while (len > 0)
    {
        nlri_len = bgp4_extract_mp_nlri(p_path, p_buf, len, p_list);
        if (nlri_len == 0)
        {
            bgp4_log(BGP_DEBUG_UPDATE,1,"failed to get mpbgp routes");

            goto MP_ERR ;
        }
        len -= nlri_len ;
        p_buf += nlri_len ;
    }

    return VOS_OK ;

MP_ERR :
    /*End Rib*/
    /*bgp4_end_peer_route(flag, p_path->p_peer,0,1);*/
    /*Clear Flag*/
    /*af_clear(p_path->p_peer->remote.af, flag);*/

    /*Send Notify*/
    bgp4_send_notify(p_path->p_peer,BGP4_UPDATE_MSG_ERR,
            BGP4_OPTIONAL_ATTR_ERR, p_buf, len );
    /*bgp4_fsm_invalid(p_path->p_peer);*/

    return VOS_ERR;
}

int bgp4_extract_mpunreach_nlri(
                            tBGP4_PATH *p_path,
                            u_char *p_buf,
                            u_char hlen,
                            short len,
                            tBGP4_LIST *p_list)
{
    u_short afi  = 0;
    u_char safi = 0;
    u_short route_len  = 0;
    u_int flag = 0;

    p_buf += hlen;
    bgp4_log(BGP_DEBUG_UPDATE,1,"process mpbgp unreachable nlri attribute,length %d", len);


    /*must include AFI and SAFI*/
    if (len < 3)
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"invalid attribute length");
        return VOS_ERR;
    }

    len -= 3 ;

    bgp4_get_2bytes(afi, p_buf);
    p_buf += 2;

    safi = *p_buf ;
    p_buf ++ ;

    /*check if neighbor support AFI,if not,only ignore the optional non-transitive attribute*/
    flag = bgp4_afi_to_index(afi, safi) ;
    if (!bgp4_af_support(p_path->p_peer, flag))
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"neighbor not support afi/safi %d/%d",afi,safi);
        return VOS_OK;
    }

    p_path->afi = afi ;
    p_path->safi = safi ;
    /*get route*/
    while (len > 0)
    {
        route_len = bgp4_extract_mp_nlri(p_path, p_buf, len, p_list);
        if (route_len == 0)
        {
            goto MP_ERR;
        }
        len -= route_len ;
        p_buf += route_len ;
    }

    return VOS_OK ;

MP_ERR :
#if 0
    /*End Rib*/
    bgp4_end_connection(flag, p_path->p_peer);
#endif/*bgp4_end_connection is called by bgp4_fsm_invalid,duplicate*/
    /*Clear Flag*/
    af_clear(p_path->p_peer->remote.af, flag);

    /*Send Notify*/
    bgp4_send_notify(p_path->p_peer,BGP4_UPDATE_MSG_ERR,
        BGP4_OPTIONAL_ATTR_ERR,p_buf,len);

    return VOS_ERR;
}

int bgp4_extract_extcom(
                        tBGP4_PATH *p_path,
                        u_char *p_buf,
                        u_char hlen,
                        short len)
{
    u_short count = 0;

    p_buf += hlen;

    bgp4_log(BGP_DEBUG_UPDATE,1,"get ext community,length %d",len);

    /*length must be 8*n bytes*/
    if (((len%8) != 0) || (len < 8))
    {
        return VOS_ERR ;
    }

    count = len/8 ;
    if (count > BGP4_MAX_ECOMMNUITY)
    {
        count = BGP4_MAX_ECOMMNUITY;
    }
    memcpy(p_path->ecommunity, p_buf, count * sizeof(tBGP4_ECOMMUNITY));
    p_path->flags.excommunity_count = count;

    return VOS_OK ;
}

/*fill attribute,not include mpreach and unreach*/
u_short bgp4_fill_attr (
                        u_int af,
                        tBGP4_PEER *p_peer,
                        tBGP4_ROUTE   *p_route,
                        u_char *p_msg)
{
    tBGP4_PATH  *p_path = NULL;
    u_long msg_start = (u_long)p_msg;

    if ((p_peer == NULL)||(p_route == NULL)||
        (p_msg == NULL))
    {
        return 0;
    }

    if(p_route->p_out_path)
    {
        p_path = p_route->p_out_path;
    }
    else
    {
        p_path = p_route->p_path;
    }
    
    p_msg += bgp4_fill_origin(p_msg, p_route);

    p_msg += bgp4_fill_aspath(p_msg, p_path, p_peer);

    /*  Filling the Next Hop ,in MBGP,do not send nexthop*/
    if (af == BGP4_PF_IPUCAST)
    {
        p_msg += bgp4_fill_nexthop(p_msg, p_route, p_peer);
    }

    p_msg +=  bgp4_fill_med(p_msg, p_route,p_peer);

    p_msg +=  bgp4_fill_localpref(p_msg, p_route,p_peer);

    p_msg +=  bgp4_fill_aggregate(p_msg, p_route,p_peer);

    p_msg +=  bgp4_fill_community(p_msg, p_route,p_peer);

    p_msg +=  bgp4_fill_originid(p_msg, p_route,p_peer);

    p_msg +=  bgp4_fill_cluster_list(p_msg, p_route,p_peer);

    p_msg +=  bgp4_fill_ext_community(p_msg, p_route, p_peer);

    p_msg +=  bgp4_fill_unkown(p_msg, p_path);
    
    return (((u_long)p_msg) - msg_start);
}

u_short bgp4_fill_nlri( u_char *p_msg ,tBGP4_ROUTE *p_route)
{
    u_char bytes = 0;
    u_short len = 0;
    u_int  prefix = 0;
    u_char addrstr[64] = {0};

    /*first byte is prefix length*/
    bgp4_put_char(p_msg, p_route->dest.prefixlen);
    len ++;

    if (p_route->dest.prefixlen == 0)
    {
        return len;
    }
    bytes = bgp4_bit2byte(p_route->dest.prefixlen);
    prefix = bgp_ip4(p_route->dest.ip);

    bgp4_log(BGP_DEBUG_UPDATE,1,"construct nlri ,%s",
                    bgp4_printf_route(p_route,addrstr));

    /*copy the whole 4 bytes,but increase length matched to prefix len*/
    bgp4_put_4bytes(p_msg, prefix);
    len += bytes;

    return len;
}

u_short bgp4_fill_mp_nexthop(
                            u_char af,
                            tBGP4_PEER *p_peer ,
                            u_char *p_msg,
                            tBGP4_ROUTE *p_route)
{
    u_char  hoplen = 0;
    u_char *p_nexthop = (p_msg + 1);
    tBGP4_PATH *p_path = NULL ;
    u_int nhop = 0 ;
    u_char ip6_nhop[32];

    if(p_route->p_out_path)
    {
        p_path = p_route->p_out_path;
    }
    else
    {
        p_path = p_route->p_path;
    }   

    memset(ip6_nhop, 0,32);

    /*fill for different nexthop type*/
    switch (af){
        case BGP4_PF_IPUCAST :
        case BGP4_PF_IPMCAST:
        case BGP4_PF_IPLABEL:
        {
            hoplen = 4 ;

            /*if bgp has no nexthop,use self BGP Id or local address.*/
            if (*(u_int*)p_path->nexthop.ip )
            {
                nhop = *(u_int*)p_path->nexthop.ip  ;
            }
            else if (*(u_int*)p_peer->local.ip.ip)
            {
                nhop = *(u_int*)p_peer->local.ip.ip;
            }
            else
            {
                nhop = gBgp4.router_id;
            }
            break ;
        }
        case BGP4_PF_IPVPN:
        {
            hoplen = 12 ;

            /*8 bytes 0 for RD*/
            if (p_msg)
            {
                memset(p_nexthop, 0, 8);
                p_nexthop += 8 ;
            }
            if ((*(u_int*)p_path->nexthop.ip ) &&
                ((gBgp4.is_reflector == TRUE) &&
                (p_peer->rr_client == TRUE)) )
            {
                nhop = bgp_ip4(p_path->nexthop.ip) ;
            }
            else if (*(u_int*)p_peer->local.ip.ip)
            {
                nhop = *(u_int*)p_peer->local.ip.ip;
            }
            else
            {
                nhop = gBgp4.router_id;
            }
            break ;
        }
        case BGP4_PF_IP6VPN:/*mpls l3 vpn only use ipv4 backbone network,so ipv4 nexthop is valid*/
        /*|                80 位                 | 16 |      32 位          |
        +--------------------------------------+--------------------------+
        |0000..............................0000|FFFF|    IPv4 地址     |
        +--------------------------------------+----+---------------------+*/
        {
            /*8 bytes 0 for RD*/
            memset(ip6_nhop, 0, 8);

            memset(ip6_nhop+8, 0, 10);
            memset(ip6_nhop+18, 0xff, 2);

            if ((*(u_int*)p_path->nexthop.ip ) &&
                ((gBgp4.is_reflector == TRUE) &&
                (p_peer->rr_client == TRUE)) )
            {
                memcpy(ip6_nhop+20,p_path->nexthop.ip,4);
            }
            else if (*(u_int*)p_peer->local.ip.ip)
            {
                memcpy(ip6_nhop+20,p_peer->local.ip.ip,4);
            }
            else
            {
                memcpy(ip6_nhop+20,&gBgp4.router_id,4);
            }

            hoplen = 24 ;

            break ;
        }
        case BGP4_PF_IP6LABEL:
        /*|                80 位                 | 16 |      32 位          |
        +--------------------------------------+--------------------------+
        |0000..............................0000|FFFF|    IPv4 地址     |
        +--------------------------------------+----+---------------------+*/
        {
            memset(ip6_nhop, 0, 10);
            memset(ip6_nhop+10, 0xff, 2);

            if ((*(u_int*)p_path->nexthop.ip ) &&
                ((gBgp4.is_reflector == TRUE) &&
                (p_peer->rr_client == TRUE)) )
            {
                memcpy(ip6_nhop+12,p_path->nexthop.ip,4);
            }
            else if (*(u_int*)p_peer->local.ip.ip)
            {
                memcpy(ip6_nhop+12,p_peer->local.ip.ip,4);
            }
            else
            {
                memcpy(ip6_nhop+12,&gBgp4.router_id,4);
            }

            hoplen = 16 ;

            break ;
        }
        case BGP4_PF_IP6UCAST:
        case BGP4_PF_IP6MCAST:
        {
            if ((p_route->is_summary) || (!is_bgp_route(p_route)))
            {
                memcpy( ip6_nhop , p_peer->local.ip.ip,16) ;
                hoplen += 16;
            }
            else if(p_peer->nexthop_self)
            {
                memcpy( ip6_nhop , p_peer->local.ip.ip,16) ;
                hoplen += 16;
            }
            else if (bgp4_peer_type(p_peer) != BGP4_EBGP)
            {
                if(memcmp(ip6_nhop, p_path->nexthop_global.ip,16)!=0)
                {
                    hoplen += 16;
                    memcpy(ip6_nhop, p_path->nexthop_global.ip,16);
                }
            }
            else
            {
                memcpy( ip6_nhop , p_peer->local.ip.ip,16) ;
                hoplen += 16;
            }
#ifdef  BGP_IPV6_WANTED
            /*The link-local address shall be included in the Next Hop field if and
            only if the BGP speaker shares a common subnet with the entity
            identified by the global IPv6 address carried in the Network Address
            of Next Hop field and the peer the route is being advertised to*/
            if ( bgp4_subnet_match(&p_path->nexthop_global,&p_peer->local.ip,p_peer->if_unit) == TRUE )
            {
                bgp4_ipv6_get_local_addr(p_peer->local.ip.ip,ip6_nhop+16);
                hoplen += 16;
            }
#endif
            break;
        }
        default :
            return 0 ;
    }

    if (p_msg)
    {
        if(bgp4_index_to_afi(p_route->dest.afi) == BGP4_AF_IP)
        {
            bgp4_fill_4bytes(p_nexthop, nhop);
        }
        else if(bgp4_index_to_afi(p_route->dest.afi) ==BGP4_AF_IP6)
        {
            memcpy(p_nexthop,ip6_nhop,hoplen);
        }
        /*set nexthop length*/
        *(p_msg) = hoplen ;
    }

    return (hoplen+1);/*including 1byte of length*/
}

/*if input message is null,just return length*/
u_short bgp4_fill_mp_nlri(
                        u_char af,
                        u_char *p_msg ,
                        tBGP4_ROUTE *p_route)
{
    u_char byte = 0;
    u_short filled = 0;
    u_char label_len = 0 ;
    u_char rd_len = 0;
    u_char *p_buf = p_msg ;
    u_short safi = bgp4_index_to_safi(af);
    u_short afi = bgp4_index_to_afi(af);

    /*+---------------------------+
    | Length (1 octet) |
    +---------------------------+
    | Label (3 octets) |
    +---------------------------+
    .............................
    +---------------------------+
    | Prefix (variable) |
    +---------------------------+*/


    if(safi == BGP4_SAF_VLBL)
    {
        label_len = BGP4_VPN_LABEL_LEN;
        rd_len = BGP4_VPN_RD_LEN;
    }
    else if(safi == BGP4_SAF_LBL)
    {
        label_len = BGP4_VPN_LABEL_LEN;
    }

    /*just return length*/
    if (p_msg == NULL)
    {
        return (filled + 1);/*including 1byte of length*/
    }

    /*nlri length bytes,prefixlen (include rd len) + label len*/
    *(p_buf) = p_route->dest.prefixlen + label_len*8;
    p_buf ++ ;
    filled++;

    /*fill lable*/
    if(label_len)
    {
        memcpy(p_buf,((u_char*)&p_route->vpn_label)+1,label_len);
        p_buf += BGP4_VPN_LABEL_LEN;
        filled += label_len;
    }

    /*fill rd*/
    if(rd_len)
    {
        memcpy(p_buf,p_route->dest.ip,rd_len);
        p_buf += BGP4_VPN_RD_LEN;
        filled += rd_len;
    }

    byte = bgp4_bit2byte(p_route->dest.prefixlen - rd_len*8);  /*prefix length bytes*/

    /*fill prefix*/
    if (afi == BGP4_AF_IP)
    {
        bgp4_fill_4bytes(p_buf, bgp_ip4(p_route->dest.ip +rd_len));
    }
    else if (afi == BGP4_AF_IP6)
    {
        memcpy(p_buf,  p_route->dest.ip+rd_len,byte);
    }

    filled+=byte;

#if 0
    if(p_route->dest.afi == BGP4_PF_IP6LABEL)/*recover 6pe route's afi in rib if need*/
    {
        p_route->dest.afi = BGP4_PF_IP6UCAST;
    }
    else if(p_route->dest.afi == BGP4_PF_IPVPN)/*recover vpn route's afi in rib if need*/
    {
        p_route->dest.afi = BGP4_PF_IPUCAST;
    }
    else if(p_route->dest.afi == BGP4_PF_IP6VPN)/*recover vpn route's afi in rib if need*/
    {
        p_route->dest.afi = BGP4_PF_IP6UCAST;
    }
#endif
    return (filled);
}

/*fill mpunreach attribute*/
u_short bgp4_fill_mp_unreach(u_char * p_msg, u_short af, u_char *p_nlri, u_short len)
{
    u_short hlen = 0 ;
    u_short afi = bgp4_index_to_afi(af);
    u_short safi = bgp4_index_to_safi(af);

    hlen = bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_MP_UNREACH_NLRI, len+3);
    p_msg += hlen;

    /*afi,safi*/
    bgp4_fill_2bytes(p_msg, afi);
    p_msg += 2;

    *p_msg = safi;
    p_msg++;

    memcpy(p_msg, p_nlri, len);

    return (hlen + 3 + len);
}

/*fill mpreach nlri*/
u_short bgp4_fill_mp_reach(u_char * p_msg, u_short af, u_char *p_nexthop, u_char nhoplen, u_char *p_nlri, u_short len)
{
    u_short hlen = 0 ;

    u_short afi = bgp4_index_to_afi(af);
    u_short safi = bgp4_index_to_safi(af);

    hlen = bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_MP_NLRI, len+nhoplen+3+1);
    p_msg += hlen;

    /*afi,safi*/
    bgp4_fill_2bytes(p_msg, afi);
    p_msg += 2;
    *p_msg = safi;
    p_msg++;

    /*nhop*/
    memcpy(p_msg, p_nexthop, nhoplen);
    p_msg += nhoplen ;

    *p_msg=0;
    p_msg++;

    /*nlri*/
    memcpy(p_msg, p_nlri, len);

    return (hlen + 3 + nhoplen + 1 +len);
}

u_short bgp4_fill_attr_hdr(u_char * p_msg, u_char type,u_short len)
{
    u_short fill_len =  0;

    if (len > 0xFF)
    {
        /* Check for whether Ext. bit set is needed. */
        bgp4_put_char(p_msg,(desc[type].flag | BGP4_ATTR_FLAG_EXT));

        bgp4_put_char(p_msg, type);

        bgp4_put_2bytes(p_msg, len);

        fill_len += 4; /* Including the length field. */
    }
    else
    {
        bgp4_put_char(p_msg, desc[type].flag);

        bgp4_put_char(p_msg, type);

        bgp4_put_char(p_msg, len);

        fill_len += 3; /* Including the length field. */
    }

    return fill_len;
}

u_short bgp4_fill_origin(u_char *p_msg, tBGP4_ROUTE *p_route)
{
    tBGP4_PATH  *p_path = NULL;
    u_char origin = 0;

    if(p_route->p_out_path)
    {
        p_path = p_route->p_out_path;
    }
    else
    {
        p_path = p_route->p_path;
    }

    p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_ORIGIN, BGP4_ORIGIN_LEN);

    switch(p_route->proto) {
        case  M2_ipRouteProto_rip:
        case  M2_ipRouteProto_ospf:
        case  M2_ipRouteProto_is_is:
            origin = (u_char)BGP4_ORIGIN_IGP;
            break;

        case  M2_ipRouteProto_netmgmt:
        case  M2_ipRouteProto_other:
        case  M2_ipRouteProto_local:
            origin = (u_char)BGP4_ORIGIN_INCOMPLETE;
            break;

        case  M2_ipRouteProto_bgp:
            if (p_path)
            {
                origin = p_path->origin;
            }
            break;

        default:
            origin = p_path->origin;
            break;
    }

    /*prevent peer sending notify.*/
    if (origin > BGP4_ORIGIN_INCOMPLETE)
    {
        origin = BGP4_ORIGIN_INCOMPLETE ;
    }

    /*if set by policy*/
    if(p_path->flags.set_origin)
    {
        origin = p_path->origin;
    }

    bgp4_put_char(p_msg,origin);

    return(4);
}

u_short bgp4_fill_aspath(u_char *p_msg, tBGP4_PATH *p_path,
      tBGP4_PEER* p_peer)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_char as_count = 0;
    u_short len = 0;
    u_short local_as = 0;
    u_char *p_start = p_msg ;
    u_char peer_type = bgp4_peer_type(p_peer);
    u_char private_as_flag = 0;

    len = bgp4_aspath_len(p_path, peer_type);

    p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_PATH, len);

    if ( (p_path != NULL) && (bgp4_lstfirst(&p_path->aspath_list)) )
    {

        if (peer_type == BGP4_EBGP)
        {
            /*ignore confed as path*/
            LST_LOOP(&p_path->aspath_list,p_aspath, node, tBGP4_ASPATH)
            {
                if ((p_aspath->type != BGP_ASPATH_CONFED_SET) &&
                    (p_aspath->type != BGP_ASPATH_CONFED_SEQ))
                {
                    break;
                }
            }

            /*path sequence,as the first*/
            bgp4_put_char(p_msg,BGP_ASPATH_SEQ);

            /*first as path is seq,add as num only;else
                add a new seq attribute*/
            if ((p_aspath != NULL) &&
                (p_aspath->type == BGP_ASPATH_SEQ))
            {
                as_count = p_aspath->len;
            }
            else
            {
                as_count  = 0;
                p_aspath = (tBGP4_ASPATH*)bgp4_lstfirst(&p_path->aspath_list);
            }

            bgp4_put_char(p_msg,(as_count+1));

            /*select local as*/
            if(p_peer->fake_as != gBgp4.asnum)
            {
                local_as = p_peer->fake_as;
            }
            else if (gBgp4.confed_id != 0)
            {
                local_as = (gBgp4.confed_id);
            }
            else
            {
                local_as = (gBgp4.asnum);
            }
            /*copy local as ,the first one*/
            bgp4_put_2bytes(p_msg,local_as);

            /*EBGP peer,should check if need to filter private AS num*/
            /*if pub as num exist in aspath list,do not del private as num to avoid data transfer error*/
            /*if peer as num exist in aspath list,do not del private as num to avoid loop*/
            if(p_peer->public_as_only &&
                (bgp4_asseq_if_exist_pub_as(&p_path->aspath_list)  == FALSE)&&
                    (bgp4_asseq_exist(&p_path->aspath_list,p_peer->remote.as) == FALSE))
            {
                private_as_flag = 1;
            }

            while (p_aspath != NULL)
            {
                /*ignore confed as path*/
                if(p_aspath->type != BGP_ASPATH_CONFED_SET &&
                    p_aspath->type != BGP_ASPATH_CONFED_SEQ)
                {
                    /*count is 0,mean a new as-segment,*/
                    if (as_count == 0)
                    {
                        /*type*/
                        bgp4_put_char(p_msg, p_aspath->type);

                        /*as-count*/
                        as_count = p_aspath->len;
                        bgp4_put_char(p_msg, as_count);
                    }
#if 0
                    /*as-part,one as contain 2 bytes*/
                    bgp4_put_string(p_msg, p_aspath->p_asseg, (as_count * 2));
#else
                    p_msg += bgp4_asnum_rebuild(p_aspath,p_msg,p_peer,private_as_flag);
#endif
                }

                /*prepare next*/
                as_count = 0;
                p_aspath = (tBGP4_ASPATH *)bgp4_lstnext(&p_path->aspath_list, &p_aspath->node);
            }

            return ((u_int)p_msg - (u_int)p_start);

        }
        else if (peer_type == BGP4_CONFEDBGP)
        {
            /*contain first confed seq*/
            if (gBgp4.confed_id != 0)
            {
                bgp4_put_char(p_msg,BGP_ASPATH_CONFED_SEQ);

                p_aspath = (tBGP4_ASPATH *)bgp4_lstfirst(&p_path->aspath_list);

                if ((p_aspath != NULL) &&
                    (p_aspath->type == BGP_ASPATH_CONFED_SEQ) )
                {
                    as_count = p_aspath->len;
                }
                else
                {
                    as_count = 0;
                }
                bgp4_put_char(p_msg,(as_count + 1));

                local_as = (gBgp4.asnum);

                bgp4_put_2bytes(p_msg, local_as);
            }/*gBgp4.confed_id*/
        }/*TMO_SLL_Scan*/

        LST_LOOP(&p_path->aspath_list,p_aspath, node, tBGP4_ASPATH)
        {
            if (as_count == 0)
            {
                /* AS SEQ has been copied. Seg. type and
                * length needs to be filled.
                */
                bgp4_put_char(p_msg,p_aspath->type);

                as_count = p_aspath->len;
                bgp4_put_char(p_msg,as_count);
            }
            bgp4_put_string(p_msg, p_aspath->p_asseg, (as_count * 2));
            as_count = 0;
        }/*(u4_peer_type == BGP4_CONFEDBGP)*/
    }
    else
    {
        /* Needs to be filled by us */
        if (peer_type == BGP4_EBGP)
        {
            bgp4_put_char(p_msg,BGP_ASPATH_SEQ);

            /* No. of ASes filled in the ASPATH */
            bgp4_put_char(p_msg,1);

            /*select local as*/
            if(p_peer->fake_as!=gBgp4.asnum)
            {
                local_as = p_peer->fake_as;
            }
            else if (gBgp4.confed_id != 0)
            {
                local_as = (gBgp4.confed_id);
            }
            else
            {
                local_as = (gBgp4.asnum);
            }

            bgp4_put_2bytes(p_msg, local_as);
        }
        else if (peer_type == BGP4_CONFEDBGP)
        {
            if (gBgp4.confed_id != 0)
            {
                bgp4_put_char(p_msg, BGP_ASPATH_CONFED_SEQ);

                /* No. of ASes filled in the ASPATH */
                bgp4_put_char(p_msg,1);

                local_as =  gBgp4.asnum;

                bgp4_put_2bytes(p_msg, local_as);
            }
        }/*(u4_peer_type == BGP4_CONFEDBGP)*/
        else
        {
            /* Empty ASPATH attribute - no need to be fill anything. */
        }
    }

    return ((u_int)p_msg - (u_int)p_start);
}

u_short bgp4_fill_nexthop(u_char *p_msg,
                        tBGP4_ROUTE   *p_route,
                        tBGP4_PEER *p_peer)
{
    tBGP4_PATH  *p_path = NULL;
    u_int nexthop = 0;
    u_char *p_start = p_msg ;

    if(p_route->p_out_path)
    {
        p_path = p_route->p_out_path;
    }
    else
    {
        p_path = p_route->p_path;
    }

    /*nexthop length must be filled into 1 byte*/
    p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_NEXT_HOP, BGP4_NEXTHOP_LEN);

    /* Protocol is Not BGP, or Self-Aggregated route */
    if ((p_route->is_summary) || (!is_bgp_route(p_route)))
    {
        nexthop = bgp_ip4(p_peer->local.ip.ip);
    }
    else if(p_peer->nexthop_self)
        {
            nexthop = bgp_ip4(p_peer->local.ip.ip);
        }    
    else if (bgp4_peer_type(p_peer) != BGP4_EBGP)
    {
        nexthop = bgp_ip4(p_path->nexthop.ip) ;
    }
    else
    {
        if ( bgp4_subnet_match(&p_path->nexthop,&p_peer->local.ip,p_peer->if_unit) == TRUE )
        {
            nexthop = bgp_ip4(p_path->nexthop.ip) ;
        }
        else
        {
            nexthop =bgp_ip4(p_peer->local.ip.ip);
        }
    }

    /*fill local id*/
    bgp4_put_4bytes(p_msg, nexthop);

    return ((u_int)p_msg - (u_int)p_start);
}

u_short  bgp4_fill_med(u_char *p_msg, tBGP4_ROUTE *p_route,
                     tBGP4_PEER *p_peer )
{
    u_int med = 0;
    u_char *p_start = p_msg ;
    u_char peer_type = 0 ;
    tBGP4_PATH  *p_path = NULL;

    if(p_route->p_out_path)
    {
        p_path = p_route->p_out_path;
    }
    else
    {
        p_path = p_route->p_path;
    }

    peer_type = bgp4_peer_type(p_peer);
    if (is_bgp_route(p_route))
    {
        if ((peer_type == BGP4_IBGP) || bgp4_is_confed_peer(p_peer->remote.as))
        {
            med = p_path->out_med;

        }
        else if (peer_type == BGP4_EBGP)
        {
            /*decide med value*/
            med = 0;
        }
    }
    else if (p_path->flags.aggr)
    {
        med = gBgp4.default_med;
    }
    else 
    {
        med = p_path->out_med;/*import route*/
    }
    
    if (med == 0)
    {
        return 0 ;
    }
    p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_MED,BGP4_MED_LEN) ;

    bgp4_put_4bytes(p_msg, med);

    return ((u_int)p_msg - (u_int)p_start);
}

u_short  bgp4_fill_localpref(u_char  *p_msg,tBGP4_ROUTE  *p_route,
                           tBGP4_PEER *p_peer)
{
    u_int lp = 0;
    u_int peer_type;
    u_char *p_start = p_msg ;
    tBGP4_PATH  *p_path = NULL;

    if(p_route->p_out_path)
    {
        p_path = p_route->p_out_path;
    }
    else
    {
        p_path = p_route->p_path;
    }

    peer_type = bgp4_peer_type(p_peer);

    if ((peer_type == BGP4_IBGP) || bgp4_is_confed_peer(p_peer->remote.as))
    {
        lp = p_path->out_localpref;
        if(lp==0)
        {
            lp=p_path->rcvd_localpref;

            if(gBgp4.default_lpref!=0)
                lp=gBgp4.default_lpref;
        }

        if (lp || (peer_type != BGP4_EBGP))
        {
            p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_LOCAL_PREF,BGP4_LPREF_LEN) ;

            /*reflect do not change lp value*/
            if ((gBgp4.is_reflector == TRUE) && p_route->p_out_path)
            {
                lp = p_path->out_localpref;

            }

            bgp4_put_4bytes(p_msg, lp);
        }

    }

    return ((u_int)p_msg - (u_int)p_start);
}

u_short  bgp4_fill_aggregate(u_char *p_msg, tBGP4_ROUTE *p_route,tBGP4_PEER* p_peer)
{
    u_short asnum = 0;
    tBGP4_PATH *p_path = NULL;
    u_int aggr = 0;
    u_char *p_start = p_msg ;

    if(p_route->p_out_path)
    {
        p_path = p_route->p_out_path;
    }
    else
    {
        p_path = p_route->p_path;
    }

    /* Filling the Atomic Aggregate field  */
    if (((is_bgp_route(p_route)) && (p_path->flags.atomic))
        || (p_route->is_summary) )
    {
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_ATOMIC_AGGR,BGP4_ATOMIC_LEN);
    }

    /* Filling the Aggregator field  */
    if (((is_bgp_route(p_route)) && (p_path->flags.aggr))
        || (p_route->is_summary) )
    {
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_AGGREGATOR,BGP4_AGGR_LEN);
        if (p_route->is_summary)
        {
            if(bgp4_peer_type(p_peer) == BGP4_EBGP && (p_peer->fake_as != gBgp4.asnum))
            {
                asnum = p_peer->fake_as;
            }
            else
            {
                asnum = gBgp4.asnum;
            }

            aggr = gBgp4.router_id;
        }
        else
        {
            asnum = p_path->aggregator.asnum;
            aggr = bgp_ip4(p_path->aggregator.addr.ip);
        }
        bgp4_put_2bytes(p_msg, asnum);
        bgp4_put_4bytes(p_msg, aggr);
    }

    return ((u_int)p_msg - (u_int)p_start);
}

u_short bgp4_fill_unkown(u_char *p_msg, tBGP4_PATH *p_path)
{
    if ((p_path == NULL) || (!p_path->p_unkown))
    {
        return 0 ;
    }
    memcpy(p_msg, p_path->p_unkown, p_path->unknown_len);

    return p_path->unknown_len;
}

u_short  bgp4_fill_originid(u_char  *p_msg,
                          tBGP4_ROUTE  *p_route,
                          tBGP4_PEER *p_peer)
{
    u_int origin_id = 0;
    tBGP4_PATH * p_path = NULL;
    u_char *p_start = p_msg ;

    if ((bgp4_peer_type(p_peer) == BGP4_IBGP) &&
            (gBgp4.is_reflector == TRUE))
    {
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_ORIGINATOR, BGP4_ORIGINATOR_LEN);

        p_path = p_route->p_path;
    
        if (p_path->origin_id)
        {
            origin_id = p_path->origin_id;
        }
        else if (p_path->p_peer != NULL)
        {
            origin_id = p_path->p_peer->router_id;
        }
        else
        {
            origin_id = gBgp4.router_id;
        }
        bgp4_put_4bytes(p_msg, origin_id);
    }

    return ((u_int)p_msg - (u_int)p_start );
}

u_short  bgp4_fill_cluster_list(u_char  *p_msg,
                              tBGP4_ROUTE  *p_route,
                              tBGP4_PEER *p_peer )
{
    tBGP4_PATH * p_path = NULL;
    u_char len = 0;
    u_char i = 0 ;
    u_char *p_start = p_msg ;

    if ((bgp4_peer_type(p_peer) == BGP4_IBGP) &&
        (gBgp4.is_reflector == TRUE))
    {
        p_path = p_route->p_path;

        /*cluster id len,add self*/
        for (i = 0 ; i < BGP4_MAX_CLUSTERID ; i++)
        {
            if (p_path->cluster[i])
                len += 4;
        }
        len += 4;

        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_CLUSTERLIST, len);
        bgp4_put_4bytes(p_msg, gBgp4.cluster_id);

        for (i = 0 ; i < BGP4_MAX_CLUSTERID ; i++)
        {
            if (p_path->cluster[i])
            {
                bgp4_put_4bytes(p_msg, p_path->cluster[i]);
            }
        }
    }

    return ((u_int)p_msg - (u_int)p_start );
}

u_short  bgp4_fill_community(u_char  *p_msg,
                           tBGP4_ROUTE  *p_route,
                           tBGP4_PEER *p_peer )
{
    tBGP4_PATH * p_path = NULL;
    u_char i = 0 ;
    u_char len = 0 ;
    u_char *p_start = p_msg ;

    if (p_peer->send_community != TRUE)
    {
        return 0;
    }

    if(p_route->p_out_path)
    {
        p_path = p_route->p_out_path;
    }
    else
    {
        p_path = p_route->p_path;
    }

    if (gBgp4.community_action == COM_ACTION_CLEAR)
    {
        return 0;
    }

    if (gBgp4.community_action == COM_ACTION_REPLACE)
    {
        len = 4;
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_COMMUNITY, len);

        bgp4_put_4bytes(p_msg, gBgp4.community);

        return ((u_int)p_msg-(u_int)p_start);
    }

    /*decide length*/
    for (i = 0 ; i < BGP4_MAX_COMMNUITY ; i++)
    {
        if (p_path->community[i])
        {
            len += 4;
        }
    }

    if ((gBgp4.community_action == COM_ACTION_ADD) && gBgp4.community)
    {
        len += 4;
    }

    if (len==0)
    {
        return 0;
    }

    p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_COMMUNITY, len);

    for (i = 0 ; i < BGP4_MAX_COMMNUITY ; i++)
    {
        if (p_path->community[i])
        {
            bgp4_put_4bytes(p_msg, p_path->community[i]);
        }
    }

    if ((gBgp4.community_action == COM_ACTION_ADD) &&
        gBgp4.community)
    {
        bgp4_put_4bytes(p_msg, gBgp4.community);
    }

    return ((u_int)p_msg-(u_int)p_start);
}

u_short bgp4_fill_ext_community(u_char  *p_msg,
                              tBGP4_ROUTE  *p_route ,
                              tBGP4_PEER *p_peer)
{
    tBGP4_PATH* p_bgp_info = NULL;
    tBGP4_PEER *p_path_peer = NULL;/*origin peer*/
    u_char u1_send_flag = 0 ;

    u_char *p_start = p_msg ;
    u_char zero_buf[8];
    u_char ext_comm_buf[1024];
    u_char *p_buf_start=ext_comm_buf;
    u_int len=0;
    u_int i=0;

    u_short ext_as=0;
    u_int ext_address=0;
    tBGP4_EXT_COMM* p_ext=NULL;

    memset(zero_buf,0,8);
    memset(ext_comm_buf,0,1024);

    if ((p_msg == NULL) || (p_route == NULL) ||
        (p_peer == NULL)||
        (p_route->p_path== NULL))
    {
        return 0;
    }

    if(p_route->p_out_path)
    {
        p_bgp_info = p_route->p_out_path;
    }
    else
    {
        p_bgp_info = p_route->p_path;
    }

    p_path_peer = p_bgp_info->p_peer;

    /*if need to fill configured ex_comm in global ex_comm list*/
    if(p_path_peer && p_path_peer->ext_comm_enable)
    {
        LST_LOOP(&gBgp4.ext_community_list, p_ext, node, tBGP4_EXT_COMM)
        {
            ext_as=0;
            ext_address=0;
            if (!is_ibgp_peer(p_peer))
            {
                if (p_ext->main_type& 0x40)
                {
                    /*if not configured Confederation,not add*/
                    if (gBgp4.confed_id  == 0)
                    {
                        continue;
                    }

                    /*if remote as is not Confederation id ,and
                    is not Confederation member,not add*/
                    if (gBgp4.confed_id != p_peer->remote.as)
                    {
                        for (i = 0 ; i < BGP4_MAX_CONFEDPEER ; i++)
                        {
                            if (gBgp4.confedpeer[i] == 0)
                            {
                                break;
                            }

                            if ((p_path_peer != NULL)
                                &&(p_peer->remote.as ==gBgp4.confedpeer[i]))
                            {
                                u1_send_flag = 1;
                                break;
                            }
                        }
                        if (!u1_send_flag)
                        {
                            continue;
                        }
                    }
                }
            }
            /*add ext-community*/
            bgp4_put_char(p_buf_start,p_ext->main_type);
            bgp4_put_char(p_buf_start,p_ext->sub_type);

            if(p_ext->main_type==0x00||p_ext->main_type==0x40)
            {
                ext_as=htons(p_ext->as);
                ext_address=htonl(p_ext->address);
                bgp4_put_2bytes(p_buf_start, ext_as);
                bgp4_put_4bytes(p_buf_start, ext_address);
            }
            else if(p_ext->main_type&0x01)
            {
                ext_as=htons(p_ext->as);
                ext_address=htonl(p_ext->address);
                bgp4_put_4bytes(p_buf_start, ext_address);
                bgp4_put_2bytes(p_buf_start, ext_as);
            }
            else if(p_ext->main_type&0x03)
            {
                /*Opaque Extended Community*/
                p_buf_start+=6;
            }
            else
            {
                p_buf_start+=6;
            }
        }
    }

    /*fill ex_comm already in path*/
    if(p_bgp_info->flags.excommunity_count != 0)
    {
        bgp4_put_string(p_buf_start,  p_route->p_path->ecommunity[0].val, 8*p_bgp_info->flags.excommunity_count);
    }

    len = p_buf_start-ext_comm_buf;

    if(len>0)
    {
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_EXT_COMMUNITY, len);
        bgp4_put_string(p_msg, ext_comm_buf, len);
    }

    return (((u_int)p_msg) - (u_int)p_start) ;
}

tBGP4_EXT_COMM *bgp4_add_ext_comm(tBGP4_EXT_COMM* p_ext_comm)
{
    tBGP4_EXT_COMM *p_new_excomm = NULL;

    p_new_excomm = (tBGP4_EXT_COMM*)bgp4_malloc(sizeof(tBGP4_EXT_COMM), MEM_BGP_COMMUNITY);
    if (p_new_excomm == NULL)
    {
        return NULL;
    }

    memset(p_new_excomm, 0, sizeof(tBGP4_EXT_COMM));

    bgp4_lstnodeinit(&p_new_excomm->node);

    p_new_excomm->main_type=p_ext_comm->main_type;
    p_new_excomm->sub_type=p_ext_comm->sub_type;
    p_new_excomm->address=p_ext_comm->address;
    p_new_excomm->as=p_ext_comm->as;

    bgp4_lstadd_tail(&gBgp4.ext_community_list, &p_new_excomm->node);

    return p_new_excomm;
}

void bgp4_delete_ext_comm(tBGP4_EXT_COMM* p_ext_comm)
{
    bgp4_lstdelete(&gBgp4.ext_community_list, &p_ext_comm->node);
    bgp4_free(p_ext_comm, MEM_BGP_LINK);
    return;
}

void bgp4_delete_all_ext_comm()
{
    tBGP4_EXT_COMM* p_ext=NULL;
    tBGP4_EXT_COMM* p_next_ext=NULL;
    LST_LOOP_SAFE(&gBgp4.ext_community_list, p_ext,p_next_ext, node, tBGP4_EXT_COMM)
    {
        bgp4_delete_ext_comm(p_ext);
    }
    return;
}

tBGP4_EXT_COMM* bgp4_ext_comm_lookup(tBGP4_EXT_COMM* p_ext_comm)
{
    tBGP4_EXT_COMM* p_ext=NULL;
    LST_LOOP(&gBgp4.ext_community_list, p_ext, node, tBGP4_EXT_COMM)
    {
        if((p_ext->as==p_ext_comm->as)&&(p_ext->address==p_ext_comm->address))
        {
            return p_ext;
        }
    }
    return NULL;
}




#endif
