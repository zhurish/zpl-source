#include "bgp4_api.h"
#ifdef NEW_BGP_WANTED
/*****************************************************************************
  bgp4aggr.c :contain aggregated route processing
******************************************************************************/
#include "bgp4com.h"
#include "bgp4aggr.h"
int
bgp4_range_lookup_cmp(
            tBGP4_RANGE *p1,
            tBGP4_RANGE *p2)
{
    /*compare address*/
    return bgp4_prefixcmp(&p1->dest, &p2->dest);
}

tBGP4_RANGE *
bgp4_range_create(
           tBGP4_VPN_INSTANCE *p_instance,
           tBGP4_ADDR *p_addr)
{
    tBGP4_RANGE *p_range = bgp4_malloc(sizeof(tBGP4_RANGE), MEM_BGP_RANGE);
    if (p_range == NULL)   
    {
        return NULL;
    }    
            
    memcpy(&p_range->dest, p_addr, sizeof(tBGP4_ADDR));
    p_range->p_instance = p_instance;
    bgp4_avl_add(&p_instance->range_table, p_range);
    return p_range;
}

void 
bgp4_range_delete(tBGP4_RANGE *p_range)
{
    /*delete route of range*/
    if (p_range->p_route != NULL)
    {
        bgp4_route_release(p_range->p_route);
    }

    bgp4_avl_delete(&p_range->p_instance->range_table, p_range);
    bgp4_free(p_range, MEM_BGP_RANGE);
    return;
}

tBGP4_RANGE *
bgp4_range_lookup(
           tBGP4_VPN_INSTANCE *p_instance,
           tBGP4_ADDR *p_addr)
{
    tBGP4_RANGE range;
    memcpy(&range.dest, p_addr, sizeof(tBGP4_ADDR));
    return bgp4_avl_lookup(&p_instance->range_table, &range);
}

tBGP4_RANGE *
bgp4_range_match(
      tBGP4_VPN_INSTANCE *p_instance,
      tBGP4_ROUTE *p_route)
{
    tBGP4_RANGE *p_range = NULL;
    
    /*check aggregate, select the first matched*/
    for_each_bgp_range(&p_instance->range_table, p_range) 
    {       
        if ((p_range->row_status == SNMP_ACTIVE)
            && (p_route->dest.prefixlen > p_range->dest.prefixlen)
            && (bgp4_prefixmatch(p_route->dest.ip, 
                p_range->dest.ip, p_range->dest.prefixlen) == TRUE))
        {
            return p_range;
        }
    }    
    return NULL;
}

/*lookup as in a array*/ 
int 
bgp4_asarray_lookup(
        tBGP4_ASVECTOR *p_as, 
        u_short as_num)
{
    u_short  i = 0;
    for (i = 0 ; i < p_as->count; i++)
    {
        if (p_as->as[i] == as_num)
        {
            return TRUE;
        }
    }  
    return FALSE;
}

/*translate an as array into aspath-list
  only 2bytes as size supported*/
void 
bgp4_as_vector_to_aspath(
         avl_tree_t *p_table, 
         u_int type, 
         tBGP4_ASVECTOR *p_as)
{
    tBGP4_ASPATH *p_path;
    u_short *as = p_as->as;
    u_int fill = 0;
    u_int rest = 0;
    u_int i = 0;
    
    for (i = 0; i < p_as->count ; i += fill, as += fill)
    {
        /*decide option length*/
        rest = p_as->count - i;
        fill = (rest > 0xff) ? 0xff : rest;
        /*allocate aspath node,consider all as will be filled*/
        p_path = bgp4_malloc(sizeof(tBGP4_ASPATH) + (fill * 2), MEM_BGP_ASPATH) ;
        if (p_path) 
        {
            p_path->type = type;
            p_path->count = fill;
            memcpy(p_path->as, as, fill*2);
            bgp4_avl_add(p_table, p_path);
        }
    }
    return;
}

/*merege aspath*/
void 
bgp4_as_path_merge(
       tBGP4_PATH *p_dest, 
       tBGP4_PATH *p_src)   
{
    avl_tree_t new_list;
    avl_tree_t *list[2];
    tBGP4_ASPATH *p_aspath_to = NULL;
    tBGP4_ASPATH *p_aspath = NULL;
    tBGP4_ASPATH *p_next = NULL;
    tBGP4_ASVECTOR as_set;
    tBGP4_ASVECTOR as_common;
    u_short *p_as_to = NULL;
    u_short *p_as = NULL;
    u_char i = 0;   
    u_char pathid = 0;
    
    bgp4_unsort_avl_init(&new_list);
    as_set.count = 0;
    as_common.count = 0;
    list[0] = &p_dest->aspath_list;
    list[1] = &p_src->aspath_list;
    
    p_aspath_to = bgp4_avl_first(list[0]);
    p_aspath = bgp4_avl_first(list[1]);

    /*decide common as sequence,construct the first as-sequence
     from this common part*/
    if (p_aspath && p_aspath_to 
        && (p_aspath->type == p_aspath_to->type) 
        && (p_aspath->type == BGP_ASPATH_SEQ)) 
    {
        p_as = (u_short *)p_aspath->as;
        p_as_to = (u_short *)p_aspath_to->as;
        
        /*decide common part length*/
        for (as_common.count = 0 ; 
                (as_common.count < p_aspath->count) 
                && (as_common.count < p_aspath_to->count) 
                && (as_common.count < BGP4_MAX_AS_COUNT);
                as_common.count ++)
        {
            if (p_as[as_common.count] != p_as_to[as_common.count])
            {
                break; 
            }
            as_common.as[as_common.count] = p_as[as_common.count];
        }
    }
    /*copy all other aspath into as set*/
    for (pathid = 0 ; pathid < 2 ; pathid++)
    {
        bgp4_avl_for_each(list[pathid], p_aspath)
        {
            p_as = (u_short *)p_aspath->as;
            for (i = 0 ; i < p_aspath->count; i++, p_as++)
            {
                /*do not include duplicated as number*/
                if ((bgp4_asarray_lookup(&as_set, *p_as) == TRUE) 
                    || (bgp4_asarray_lookup(&as_common, *p_as) == TRUE)) 
                {
                    continue;
                }
                /*insert into new as-set*/
                as_set.as[as_set.count++] = *p_as;
                if (as_set.count >= BGP4_MAX_AS_COUNT)
                {
                    break;
                }
            }
        }
    }

    /*build as-seq aspath segment*/
    bgp4_as_vector_to_aspath(&new_list, BGP_ASPATH_SEQ, &as_common);

    /*build as-set aspath segment*/
    bgp4_as_vector_to_aspath(&new_list, BGP_ASPATH_SET, &as_set);

    /*release old aggregated as-path*/
    bgp4_apath_list_free(p_dest);  
    
    /*connect list*/
    bgp4_avl_for_each_safe(&new_list, p_aspath, p_next)
    {
        bgp4_avl_delete(&new_list, p_aspath);
        bgp4_avl_add(&p_dest->aspath_list, p_aspath);
    }
    return;
}

void
bgp4_community_merge(
       tBGP4_PATH *p_dest, 
       tBGP4_PATH *p_src)
{
    u_int merge[256];
    u_int count = 0;
    u_int readlen = 0;
    u_int i;
    u_int exist;
    u_int *p_community = NULL;
    
    if (p_src->p_community == NULL)
    {
        return;
    }
    /*if community same,do nothing*/
    if (p_dest->p_community
        && (p_dest->community_len == p_src->community_len)
        && (memcmp(p_src->p_community, p_dest->p_community, p_src->community_len) == 0))
    {
        return;
    }
    /*same current community*/
    if (p_dest->p_community)
    {
        memcpy(merge, p_dest->p_community, p_dest->community_len);
        count = p_dest->community_len/4;
    }
    
    p_community = (u_int *)p_src->p_community;
    for (readlen = 0; readlen < p_src->community_len; readlen += 4, p_community++)
    {
        exist = FALSE;
        /*if community exist in resulted array,do not add again*/
        for (i = 0 ; i < count ; i++)
        {
            if (*p_community == merge[i])
            {
                exist = TRUE;
                break;
            }
        }
        /*insert new community*/
        if (exist == FALSE)
        {
            merge[count] = *p_community;
            count++;
        }
    }
    if (count == 0)
    {
        return;
    }
    if (p_dest->p_community)
    {
        bgp4_free(p_dest->p_community, MEM_BGP_BUF);
    }
    
    p_dest->p_community = bgp4_malloc(count*4, MEM_BGP_BUF);
    if (p_dest->p_community)
    {
        memcpy(p_dest->p_community, merge, count*4);
        p_dest->community_len = count*4;
    }    
    return;
}

void 
bgp4_path_merge(
        tBGP4_ROUTE *p_dest, 
        tBGP4_ROUTE *p_src) 
{
    tBGP4_PATH *p_path_to = NULL;
    tBGP4_PATH *p_path = NULL;
    u_char agg[64];
    u_char special[64];
    
    bgp4_log(BGP_DEBUG_UPDATE, "aggregate route attribute,agg %s,special %s",
                                    bgp4_printf_route(p_src,agg),
                                    bgp4_printf_route(p_src,special));
    
    p_path_to = p_dest->p_path;
    p_path = p_src->p_path;
    if (p_path == NULL)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "This pointer p_path be NULL");
        return;
    }
    /*first route to be aggregated,copy full route info*/ 
    if (p_path_to == NULL) 
    {
        bgp4_log(BGP_DEBUG_UPDATE, "create new bgp aggregate attribute");
        
        p_path_to = bgp4_path_create(p_path->p_instance, p_path->af);
        if (p_path_to == NULL) 
        {
            bgp4_log(BGP_DEBUG_UPDATE, "create new bgp attribute failed");   
            return;
        }
        /*copy whole path infor of route*/ 
        bgp4_path_copy(p_path_to, p_path);
        bgp4_link_path(p_path_to, p_dest);
        p_dest->proto = M2_ipRouteProto_bgp;
        return;
    }

    /*aggregation of these two bgp infos is done. */
    /*filling up the origin attribute   */
    
    if ((p_path_to->origin == BGP4_ORIGIN_INCOMPLETE)
        || (p_path->origin == BGP4_ORIGIN_INCOMPLETE))
    {
        p_path_to->origin = BGP4_ORIGIN_INCOMPLETE;
    }
    else if ((p_path_to->origin == BGP4_ORIGIN_EGP)
        || (p_path->origin == BGP4_ORIGIN_EGP))
    {
        p_path_to->origin = BGP4_ORIGIN_EGP;
    }
    else
    {
        p_path_to->origin = BGP4_ORIGIN_IGP;           
    }

    if (bgp4_aspath_same(p_path, p_path_to) != TRUE) 
    {
        bgp4_as_path_merge(p_path_to, p_path);   
    }
    if (p_path->atomic_exist)
    {
        p_path_to->atomic_exist = TRUE;
    }

    /*community merge*/
    bgp4_community_merge(p_path_to, p_path);
    return;
}

#define bgp4_display_aggr_route_list(x) do {\
    tBGP4_LINK *p_linkx = NULL;\
    u_char addrstr[64];\
    bgp4_avl_for_each((x), p_linkx)\
    {\
        bgp4_log(1,"aggregated route: %s", bgp4_printf_route(p_linkx->p_route,addrstr));\
    }}while (0)

void 
bgp4_range_table_flush(tBGP4_VPN_INSTANCE *p_instance)
{
    bgp4_avl_walkup(&p_instance->range_table, bgp4_range_delete);
    return;
}

void 
bgp4_range_up(tBGP4_RANGE *p_range)
{
    tBGP4_VPN_INSTANCE *p_instance = p_range->p_instance;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_LINK *p_link = NULL;
    tBGP4_PATH *p_path = NULL;
    avl_tree_t rtlist;

    bgp4_unsort_avl_init(&rtlist);

    p_range->row_status = SNMP_ACTIVE;

    /*range route already exist,do nothing more*/
    if (p_range->p_route != NULL)
    {
        return;
    }
    
    /*create route for range*/
    p_route = bgp4_malloc(sizeof(tBGP4_ROUTE), MEM_BGP_ROUTE);
    if (p_route == NULL) 
    {
        return;
    }

    /*create empty path*/
    p_path = bgp4_path_create(p_instance, p_range->dest.afi);
    if (p_path == NULL)
    {
        bgp4_free(p_route, MEM_BGP_ROUTE);
        return;
    }
    p_path->origin_vrf = p_instance->vrf;
    memcpy(&p_route->dest, &p_range->dest, sizeof(p_range->dest));

    bgp4_link_path(p_path, p_route);
    p_route->proto = M2_ipRouteProto_bgp;
    
    /*indicate this is route for range*/
    p_route->summary_route = TRUE;
        
    /*insert to rib tree*/
    bgp4_route_rib_table_add(p_instance, p_route);

    /*obtain route list to be aggregated*/
    bgp4_aggregated_route_get(p_instance, &p_route->dest, &rtlist);
    
    bgp4_log(BGP_DEBUG_UPDATE, "get %d aggregate routing", bgp4_avl_count(&rtlist));

    /*copy first path*/
    if (bgp4_avl_first(&rtlist)) 
    {
        p_link = bgp4_avl_first(&rtlist);
        
        bgp4_path_copy(p_path, p_link->p_route->p_path);
    }
    
    bgp4_display_aggr_route_list(&rtlist);

    /*build aggregated path from special routes*/
    bgp4_avl_for_each(&rtlist, p_link)
    {
        bgp4_path_merge(p_route, p_link->p_route);

        /*if aggregated routes can not be advertised,send withdraw
          for them*/        
        if ((p_range->summaryonly == TRUE)
            && (bgp4_avl_count(&rtlist) >= 1))
        {
            bgp4_route_withdraw_flood_set(p_link->p_route);
            p_link->p_route->summary_filtered = TRUE;
            /*if route is originated from this vpn instance,schedule delete from
              other vrf */
            if ((p_instance->vrf != 0)
                && (p_link->p_route->p_path->origin_vrf == p_instance->vrf))  
            {
                bgp4_vrf_route_export_check(p_link->p_route, FALSE);
            }
        }
    }
    /*send update for active summary route*/
    if (bgp4_avl_count(&rtlist) >= 1) 
    {   
        bgp4_route_feasible_flood_set(p_route);
        p_route->active = TRUE;
    }
    
    p_range->p_route = p_route;    
    p_range->matched_route = bgp4_avl_count(&rtlist);
    /*export range route to other vrf.*/
    if ((bgp4_avl_count(&rtlist) >= 2)
        && (p_instance->vrf != 0)
        && (p_range->p_route->p_path->origin_vrf == p_instance->vrf))  
    {
        bgp4_vrf_route_export_check(p_range->p_route, TRUE);
    }
    /*release obtained rotue list*/
    bgp4_rtlist_clear(&rtlist);
    return;
}

/*disable a range*/
void 
bgp4_range_down(tBGP4_RANGE *p_range)
{
    tBGP4_VPN_INSTANCE *p_instance = p_range->p_instance;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_LINK *p_link = NULL;
    avl_tree_t rtlist;

    bgp4_unsort_avl_init(&rtlist);            

    p_range->row_status = SNMP_NOTINSERVICE;

    /*range route not be created,ignore*/
    p_route = p_range->p_route; 
    if (p_route == NULL) 
    {
        return;
    }

    /*obtain route list be aggregated*/
    bgp4_aggregated_route_get(p_instance, &p_route->dest, &rtlist);
    
    bgp4_log(BGP_DEBUG_UPDATE,"get %d aggregated route", bgp4_avl_count(&rtlist));
    
    bgp4_display_aggr_route_list(&rtlist);

    /*resend aggregated route if necessary*/
    if (p_range->summaryonly == TRUE)
    {
        bgp4_avl_for_each(&rtlist, p_link)
        {
            if (p_link->p_route->summary_filtered == TRUE)
            {
                p_link->p_route->summary_filtered = FALSE;
                bgp4_route_feasible_flood_set(p_link->p_route);
                if ((p_instance->vrf != 0)
                    && (p_link->p_route->p_path->origin_vrf == p_instance->vrf))  
                {
                    bgp4_vrf_route_export_check(p_link->p_route, TRUE);
                }
            }
        }
    }

    /*release range route*/
    p_range->p_route = NULL;

    /*if range is in used,send withdraw for range route.else
      delete it directly*/
    if (bgp4_avl_first(&rtlist)) 
    {   
        p_route->is_deleted = TRUE;
        bgp4_route_withdraw_flood_set(p_route);
        /*export range route to other vrf.*/
        if ((p_instance->vrf != 0)
            && (p_route->p_path->origin_vrf == p_instance->vrf))  
        {
            bgp4_vrf_route_export_check(p_route, FALSE);
        }
    } 
    else
    {
        bgp4_route_table_delete(p_route);
    }
    p_range->matched_route = 0;
    bgp4_rtlist_clear(&rtlist);
    return;
}

/*check if range's route path changed...*/
void
bgp4_range_update(tBGP4_RANGE *p_range)
{
    tBGP4_VPN_INSTANCE *p_instance = p_range->p_instance;
    tBGP4_ROUTE route;
    tBGP4_PATH path;
    avl_tree_t rtlist;
    tBGP4_LINK *p_link = NULL;
    u_int current_active = 0;
    u_int old_count = 0;

    /*ignore inactive range*/
    if (p_range->row_status != SNMP_ACTIVE)
    {
        return;
    }
    current_active = p_range->p_route->active;
    
    /*form new summary route*/
    memset(&route, 0, sizeof(route));
    memset(&path, 0, sizeof(path));
    bgp4_path_init(&path);
    path.af = p_range->dest.afi;
    path.p_instance = p_instance;
    bgp4_link_path(&path, &route);

    path.origin_vrf = p_instance->vrf;
    memcpy(&route.dest, &p_range->dest, sizeof(p_range->dest));
    route.proto = M2_ipRouteProto_bgp;
    
    bgp4_unsort_avl_init(&rtlist);
    
    /*obtain route list to be aggregated*/
    bgp4_aggregated_route_get(p_instance, &p_range->dest, &rtlist);
    old_count = p_range->matched_route;
    p_range->matched_route = bgp4_avl_count(&rtlist);

    /*copy first path*/
    if (bgp4_avl_first(&rtlist)) 
    {
        p_link = bgp4_avl_first(&rtlist);
        
        bgp4_path_copy(&path, p_link->p_route->p_path);
    }
    bgp4_avl_for_each(&rtlist, p_link)
    {
        bgp4_path_merge(&route, p_link->p_route);

        /*if some route has not been filtered,schedule withdraw*/
        if ((p_range->summaryonly == TRUE)
            && (p_range->matched_route >= 2)
            && (p_link->p_route->summary_filtered == FALSE))
        {
            bgp4_route_withdraw_flood_set(p_link->p_route);
            p_link->p_route->summary_filtered = TRUE;
        }
        /*if route is originated from this vpn instance,schedule delete from
          other vrf */
        if ((p_range->summaryonly == TRUE)
            && (p_range->matched_route >= 2)
            && (p_instance->vrf != 0)
            && (p_link->p_route->p_path->origin_vrf == p_instance->vrf))  
        {
            bgp4_vrf_route_export_check(p_link->p_route, FALSE);
        }
    }

    /*RFC 1997:if no atomic aggregate attribtue exist,merge
      community*/

    /*check result*/
    /*if path not changed,do nothing*/
    if (bgp4_path_same(&path, p_range->p_route->p_path) == TRUE)
    {
        if ((old_count < 2) 
            && (p_range->matched_route < 2))
        {
            goto END;
        }
        if ((old_count >= 2) 
            && (p_range->matched_route >= 2))
        {
            goto END;
        }
    }
    /*path changed,clear old route*/
    bgp4_route_table_delete(p_range->p_route);
    
    p_range->p_route = bgp4_route_duplicate(&route);
    if (p_range->p_route == NULL)
    {
        goto END;
    }
    /*insert to rib tree*/
    bgp4_route_rib_table_add(p_instance, p_range->p_route);

    p_range->p_route->summary_route = TRUE;
    
    /*if route count >= 2 it is active*/
    if (p_range->matched_route >= 2)
    {
        p_range->p_route->active = TRUE;
        bgp4_route_feasible_flood_set(p_range->p_route);
        /*export range route to other vrf.*/
        if ((p_instance->vrf != 0)
            && (p_range->p_route->p_path->origin_vrf == p_instance->vrf))  
        {
            bgp4_vrf_route_export_check(p_range->p_route, TRUE);
        }
    }
    else if (current_active == TRUE)
    {
        bgp4_route_withdraw_flood_set(p_range->p_route);

        /*export range route to other vrf.*/
        if ((p_instance->vrf != 0)
            && (p_range->p_route->p_path->origin_vrf == p_instance->vrf))  
        {
            bgp4_vrf_route_export_check(p_range->p_route, FALSE);
        }
        /*recover the last detailed route*/
        if (bgp4_avl_first(&rtlist)) 
        {
            p_link = bgp4_avl_first(&rtlist);

            if (p_link->p_route->summary_filtered == TRUE)
            {
                p_link->p_route->summary_filtered = FALSE;
                bgp4_route_feasible_flood_set(p_link->p_route);
                /*export route to other vrf.*/
                if ((p_instance->vrf != 0)
                    && (p_link->p_route->p_path->origin_vrf == p_instance->vrf))  
                {
                    /*force to originate*/
                    p_range->row_status = SNMP_NOTINSERVICE;
                    
                    bgp4_vrf_route_export_check(p_link->p_route, TRUE);

                    p_range->row_status = SNMP_ACTIVE;
                }
            }
        }
    }
END:    
    bgp4_path_clear(&path);
    bgp4_rtlist_clear(&rtlist);
    return;
}

/*check if a feasible update can be send when range exist
  if range not exist,or summary-only not set,we can send
  feasible update,if route is not in range's aggregated
  route list,it can be sent.
  OK:it can be send
  ERROR:it can not be send
*/
STATUS
bgp4_feasible_flood_check_against_range(tBGP4_ROUTE *p_route)    
{
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_RIB *p_rib = p_instance->rib[p_route->dest.afi];
    tBGP4_RANGE *p_range = NULL;
    tBGP4_ROUTE *p_checked = NULL;
    u_int count = 0;
    u_int self_appear = 0;
    u_int first_med = 0;
    u_int first_route = 0;

    if (p_rib == NULL)
    {
        return VOS_OK; 
    }
    
    p_route->summary_filtered = FALSE;
    /*summary route self do not checked*/
    if (p_route->summary_route == TRUE)
    {
        return VOS_OK;
    }

    /*get range matched*/
    p_range = bgp4_range_match(p_instance, p_route);
    /*no range exist, or range is inactive,can be send*/
    if ((p_range == NULL) || (p_range->row_status != SNMP_ACTIVE))
    {
        return VOS_OK;
    }

    /*schedule update range's state*/
    p_range->update_need = TRUE;

    if (!bgp4_timer_is_active(&p_instance->range_update_timer))
    {
        bgp4_timer_start(&p_instance->range_update_timer, 1);
    }
    /*range has not set summary-only,can be send*/        
    if (p_range->summaryonly == FALSE)
    {
        return VOS_OK;
    }

    /*get aggregated route set for range,
          if incoming route is in this set,
          and total route's count >= 2,
          do not send feasible for route*/    
    bgp4_avl_for_each(&p_rib->rib_table, p_checked)    
    {
       /*summary route can not be aggregated*/
        if (p_checked->summary_route)
        {
            continue;
        }
        /*deleted route can not be aggregated*/
        if (p_checked->is_deleted == TRUE)
        {
            continue;
        }
        /*af must same*/
        if (p_checked->dest.afi != p_route->dest.afi)
        {
            continue;
        }
        if (bgp4_prefixmatch(p_checked->dest.ip, 
             p_range->dest.ip, p_range->dest.prefixlen))
        {
            /*only obtain route can be aggregated.they must has 
              same med*/
            if (first_route == 0)
            {
                first_med = p_checked->p_path->med;
                first_route++;
            }
            else if (first_med != p_checked->p_path->med)
            {
                continue;
            }
            count++;
            if (p_route == p_checked)
            {
                self_appear = TRUE;
            }
            if ((count >= 2) && self_appear)
            {
                p_route->summary_filtered = TRUE;
                return VOS_ERR;
            }
        }
    }
    return VOS_OK;
}

/*check if a withdraw update can be send when range exist
  VOS_OK:it can be send
  ERROR:it can not be send
*/
STATUS
bgp4_withdraw_flood_check_against_range(tBGP4_ROUTE *p_route)    
{
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_RANGE *p_range = NULL;
    
    /*summary route self do not checked*/
    if (p_route->summary_route == TRUE)
    {
        return VOS_OK;
    }
    
    /*get range matched*/
    p_range = bgp4_range_match(p_instance, p_route);
    /*no range exist, or range is inactive,can be send*/
    if ((p_range == NULL) || (p_range->row_status != SNMP_ACTIVE))
    {
        return VOS_OK;
    }
    
    /*schedule update range's state*/
    p_range->update_need = TRUE;

    if (!bgp4_timer_is_active(&p_instance->range_update_timer))
    {
        bgp4_timer_start(&p_instance->range_update_timer, 1);
    }
    
    /*range has not set summary-only,can be send*/        
    if (p_range->summaryonly == FALSE)
    {
        return VOS_OK;
    }
    /*route is filtered by range,do not send*/
    if (p_route->summary_filtered == TRUE)
    {
        p_route->summary_filtered = FALSE;
        return VOS_ERR;
    }
    return VOS_OK;
}
#else
/*****************************************************************************
  bgp4aggr.c :contain aggregated route processing
******************************************************************************/
#include "bgp4com.h"
#include "bgp4aggr.h"
#include "bgp4main.h"

tBGP4_AGGR *bgp4_add_aggr(tBGP4_LIST* p_aggr_lst,tBGP4_ADDR void* p_aggr_rt)  
{
    tBGP4_AGGR *p_range = NULL;

    if(p_aggr_lst == NULL)
    { 
        bgp4_log(BGP_DEBUG_EVT,1,"bgp aggr add pointer bgp aggregate list is NULL");
    }
    
    p_range = (tBGP4_AGGR*)bgp4_malloc(sizeof(tBGP4_AGGR), MEM_BGP_ROUTE) ;
    if (p_range == NULL)   
    {
        return NULL;
    }    
            
    memcpy(&(p_range->dest),p_aggr_rt, sizeof(tBGP4_ADDR));
    bgp4_lstnodeinit(&p_range->node);
        
    bgp4_lstadd(p_aggr_lst, &p_range->node);
    
    return p_range;
}

void bgp4_delete_aggr(tBGP4_LIST* p_aggr_lst,tBGP4_AGGR* p_range)
{
    if(p_aggr_lst == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp aggr delete pointer bgp aggregate list is NULL");
    }
    

    bgp4_lstdelete(p_aggr_lst, &p_range->node);
    bgp4_free((void*)p_range, MEM_BGP_ROUTE);
    
    return;
}

tBGP4_AGGR* bgp4_aggregate_lookup(tBGP4_LIST* p_aggr_lst,tBGP4_ADDR * aggr_ip)
{
    tBGP4_AGGR* p_aggr = NULL;
    
    for_each_bgp_range(p_aggr_lst,p_aggr)
    {
        if (bgp4_prefixcmp(&p_aggr->dest, aggr_ip) == 0)
        {
            return p_aggr;
        }
    }
    return NULL;
}

/*decide if a list routes can be aggregated*/
int  bgp4_rtlist_aggregate_enable(tBGP4_LIST *p_list) 
{
    tBGP4_LINK *p_link = NULL;
    u_int first_med = 0,  med = 0;
    u_char first_route = TRUE;
 
    if ((p_list == NULL) || (bgp4_lstfirst(p_list) == NULL))
    {
        return FALSE ;
    }

    /*check this group of routes,if some nexthop or med 
    different,do not aggregate them*/ 
    LST_LOOP(p_list, p_link, node, tBGP4_LINK) 
    {
        med = p_link->p_route->p_path->out_med;
                
        /*record first value*/
        if (first_route == TRUE)
        {
            first_med = med;
            first_route = FALSE ;
        }
        else if (med != first_med)
        {
            return (FALSE);
        }
    }
        
    return TRUE ;
}

/*lookup as in a array*/ 
int bgp4_asarray_lookup(tBGP4_ASVECTOR *p_as, u_short as_num)
{
    u_short  i = 0;
    
    for (i = 0 ; i < p_as->count; i++)
    {
        if (p_as->as[i] == as_num)
        {
            return TRUE;
        }
    }  

    return FALSE;
}

/*function, translate a as array into list*/
void bgp4_build_aspath_list(tBGP4_LIST *p_list, u_int type, tBGP4_ASVECTOR *p_as)
{
    tBGP4_ASPATH *p_path ;
    int count = p_as->count;
    u_short *as = p_as->as;
    
    /*decide number of as-path node*/
    while (count > 0)
    {
        p_path =(tBGP4_ASPATH*)bgp4_malloc(sizeof(tBGP4_ASPATH),MEM_BGP_ASPATH) ;
        if (p_path == NULL) 
        {
            break;
        }
        bgp4_lstnodeinit(&p_path->node);
        p_path->type = type;
        p_path->len = (count > 0xff) ? 0xff : count;
        p_path->p_asseg = (u_char*)bgp4_malloc((p_path->len*2), MEM_BGP_BUF);
        if (p_path->p_asseg)
        {
            memcpy(p_path->p_asseg, as, p_path->len*2);
        }
        bgp4_lstadd_tail(p_list, &p_path->node);
        count -= p_path->len;
        as += p_path->len;
    }

    return ;    
}

/*merege aspath*/
void bgp4_aggregate_as_path(tBGP4_PATH *p_dest, tBGP4_PATH *p_src)   
{
    tBGP4_LIST list;
    tBGP4_ASPATH *p_aspath_to = NULL;
    tBGP4_ASPATH *p_aspath = NULL;
    u_short *p_aggr_asnos = NULL;
    u_short *p_asnos = NULL;
    u_char i = 0;   
    tBGP4_ASVECTOR as ;
    tBGP4_ASVECTOR asseq ;

    bgp4_lstinit(&list);

    as.count = 0;
    asseq.count = 0 ;
    
    p_aspath_to = (tBGP4_ASPATH *)bgp4_lstfirst(&p_dest->aspath_list);
    p_aspath = (tBGP4_ASPATH *)bgp4_lstfirst(&p_src->aspath_list);

    /*decide common part,must be asseq*/
    if ((p_aspath->type == p_aspath_to->type) && 
        (p_aspath->type == BGP_ASPATH_SEQ)) 
    {
        p_asnos = (u_short *)p_aspath->p_asseg;
        p_aggr_asnos = (u_short *)p_aspath_to->p_asseg;
        
        /*decide common part length*/
        for (asseq.count = 0 ; 
                (asseq.count < p_aspath->len) 
                && (asseq.count < p_aspath_to->len) 
                && (asseq.count < BGP4_MAX_AS_COUNT) ;
                asseq.count ++)
        {
            if (p_asnos[asseq.count] != p_aggr_asnos[asseq.count])
            {
                break; 
            }
            asseq.as[asseq.count] = p_asnos[asseq.count];
        }
    }
        
    /*copy all other aspath into as array*/
    LST_LOOP(&p_src->aspath_list, p_aspath_to, node, tBGP4_ASPATH)
    {
        p_asnos = (u_short *)p_aspath_to->p_asseg;
        
        if(p_aspath_to->len==0)
        {
            continue;
        }
         
        for (i = 0 ; i < p_aspath_to->len; i++, p_asnos++)
        {
            if ((bgp4_asarray_lookup(&as, *p_asnos) == TRUE) ||
                (bgp4_asarray_lookup(&asseq, *p_asnos) == TRUE)) 
            {
                continue ;
            }
            as.as[as.count++] = *p_asnos;
            if (as.count >= BGP4_MAX_AS_COUNT)
            {
                break;
            }
        }
    }
        
    /*copy all other aspath into as array*/
    LST_LOOP(&p_dest->aspath_list, p_aspath, node, tBGP4_ASPATH)
    {
        p_asnos = (u_short *)p_aspath->p_asseg;

        if(p_aspath->len==0)
        {
            continue;
        }
        for (i = 0 ; i <p_aspath->len; i++, p_asnos++)
        {
            if ((bgp4_asarray_lookup(&as, *p_asnos) == TRUE) ||
                (bgp4_asarray_lookup(&asseq, *p_asnos) == TRUE)) 
            {
                continue ;
            }
            
            as.as[as.count++] = *p_asnos;
            if (as.count >= BGP4_MAX_AS_COUNT)
            {
                break;
            }
        }
    }

    bgp4_build_aspath_list(&list, BGP_ASPATH_SEQ, &asseq);
    bgp4_build_aspath_list(&list, BGP_ASPATH_SET, &as);

    bgp4_apath_list_free(p_dest);  
    /*connect list to p_dest->aspath_list and list is null*/
    bgp4_lstconcat(&p_dest->aspath_list, &list);

    return;
}

void bgp4_aggregate_path(tBGP4_ROUTE *p_aggr_route, tBGP4_ROUTE *p_route) 
{
    tBGP4_PATH  *p_aggr_path = NULL;
    tBGP4_PATH  *p_path = NULL;
    u_char agg[64];
    u_char special[64];
    
    bgp4_log(BGP_DEBUG_UPDATE,1,"aggr route attribute,aggregate %s,special %s",
                                    bgp4_printf_route(p_aggr_route,agg),
                                    bgp4_printf_route(p_route,special));
    
    p_aggr_path = p_aggr_route->p_path;
    p_path = p_route->p_path;
    if (p_path == NULL)
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"This pointer variable is NUL");
        return ;
    }
    /*first route to be aggregated,copy full route info*/ 
    if (p_aggr_path == NULL) 
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"create new aggregate  bgp attribute");
        p_aggr_path = bgp4_add_path(p_route->dest.afi);
        if (p_aggr_path == NULL) 
        {
            bgp4_log(BGP_DEBUG_UPDATE,1,"failed to create new aggregate bgp attribute");   
            return;
        }
        /*copy whole path infor of route*/ 
        bgp4_path_copy(p_aggr_path, p_path);
        bgp4_link_path(p_aggr_path, p_aggr_route);
        p_aggr_route->proto = M2_ipRouteProto_bgp;
        return;
    }

    /*aggregation of these two bgp infos is done. */
    /*filling up the origin attribute   */
    
    if ((p_aggr_path->origin == BGP4_ORIGIN_INCOMPLETE)|| 
        (p_path->origin == BGP4_ORIGIN_INCOMPLETE))
    {
        p_aggr_path->origin = BGP4_ORIGIN_INCOMPLETE;
    }
    else if ( (p_aggr_path->origin == BGP4_ORIGIN_EGP)|| 
                (p_path->origin == BGP4_ORIGIN_EGP))
    {
        p_aggr_path->origin = BGP4_ORIGIN_EGP;
    }
    else
    {
        p_aggr_path->origin = BGP4_ORIGIN_IGP;           
    }
    
    if (bgp4_aspath_same(p_path, p_aggr_path) != TRUE) 
    {
        bgp4_aggregate_as_path(p_aggr_path, p_path);   
    }
    
    p_aggr_path->flags.atomic = p_path->flags.atomic;
    
    /*copy comunity list*/
    memcpy(p_aggr_path->community, p_path->community, sizeof(p_path->community));
    
    /*copy ex-community list*/
    
    return ;           
}

#define bgp4_display_aggr_list(x,y) do{\
    tBGP4_LINK *p_linkx;\
    u_char addrstr[64];\
    LST_LOOP(y,p_linkx, node, tBGP4_LINK)\
    {\
        bgp4_log(1,1,"withdraw route: %s",bgp4_printf_route(p_linkx->p_route,addrstr));\
    }\
    LST_LOOP(x,p_linkx, node, tBGP4_LINK)\
    {\
        bgp4_log(1,1,"viable route: %s",bgp4_printf_route(p_linkx->p_route,addrstr));\
    }}while (0)


tBGP4_AGGR *bgp4_aggr_match(tBGP4_VPN_INSTANCE* p_instance,u_int af, tBGP4_ROUTE *p_route)
{
    tBGP4_AGGR *p_range = NULL;
    
    /*check aggregate, select the first matched*/
    for_each_bgp_range(&p_instance->aggr_list,p_range) 
    {       
        if ((p_range->state == TRUE)
            && (p_route->dest.prefixlen > p_range->dest.prefixlen)
            && (bgp4_prefixmatch((u_char *)&p_route->dest.ip, 
            (u_char *)&p_range->dest.ip, p_range->dest.prefixlen) == TRUE))
        {
            return (p_range);
        }
    }    

    return NULL;
}

#define bgp4_display_aggr_route_list(x) do {\
    tBGP4_LINK *p_linkx;\
    u_char addrstr[64];\
    LST_LOOP((x), p_linkx, node ,tBGP4_LINK)\
    {\
        bgp4_log(1,1,"aggregate route: %s", bgp4_printf_route(p_linkx->p_route,addrstr));\
    }}while (0)

void bgp4_delete_all_aggregate(tBGP4_VPN_INSTANCE* p_instance) 
{
    tBGP4_AGGR *p_range = NULL;
    tBGP4_AGGR *p_next = NULL;
    tBGP4_LIST *p_aggr_list = &p_instance->aggr_list;

    if(p_aggr_list == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1," aggregate list is null");
        return;
    }
    
    LST_LOOP_SAFE(p_aggr_list, p_range, p_next, node, tBGP4_AGGR)
    {
        if(p_range->p_aggr_rt!=NULL)
        {
#if 0/*path will release later in bgp4_clear_unused_path()*/
            if(p_range->p_aggr_rt->p_path!=NULL)
            {
                bgp4_path_free(p_range->p_aggr_rt->p_path);
            }
#endif
            bgp4_rib_delete(&p_instance->rib,p_range->p_aggr_rt);
        }
        bgp4_delete_aggr(p_aggr_list,p_range);
    }
    
    return;
}

int bgp4_aggr_up(tBGP4_VPN_INSTANCE* p_instance,tBGP4_AGGR *p_range)
{
    tBGP4_ROUTE *p_aggr_rt=NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_LINK  *p_link = NULL;
    tBGP4_LIST rtlist;

    bgp4_lstinit(&rtlist);

    p_range->state=SNMP_ACTIVE;

    if(p_range->p_aggr_rt!=NULL)
    {
        return TRUE;
    }
    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"This pointer  is NULL.");
    }

    
    /*create route for range*/
    p_aggr_rt = bgp4_creat_route();   
    if (p_aggr_rt == NULL) 
    {
        return (FALSE);
    }
        
    memcpy(&p_aggr_rt->dest, &p_range->dest, sizeof(p_range->dest));
    p_aggr_rt->is_summary = TRUE;
        
    /*insert to rib tree*/
    bgp4_rib_add(&p_instance->rib, p_aggr_rt) ;

    /*obtain route list be aggregated*/
    bgp4_rib_get_by_range(&p_instance->rib, &p_aggr_rt->dest, &rtlist);
    
    bgp4_log(BGP_DEBUG_UPDATE,1,"get %d aggregate route", bgp4_lstcnt(&rtlist));
    
    bgp4_display_aggr_route_list(&rtlist);

    if (bgp4_rtlist_aggregate_enable(&rtlist) == TRUE) 
    {   
        /*for each entry in route list,aggregate route info*/
        LST_LOOP(&rtlist, p_link, node, tBGP4_LINK)
        {
            bgp4_aggregate_path(p_aggr_rt, p_link->p_route);
            
            if(p_range->summaryonly==TRUE)
            {
                bgp4_schedule_rib_update(p_instance,p_route,NULL,NULL);
                p_route->filter_by_range = TRUE;
            }
        }

        /*notify feasible aggr route*/
        bgp4_schedule_rib_update(p_instance,NULL,p_aggr_rt,NULL);
        p_aggr_rt->summary_active = TRUE;
    }

    p_range->p_aggr_rt=p_aggr_rt;   

    bgp4_rtlist_clear(&rtlist);
    
    return TRUE;
}

int bgp4_aggr_down(tBGP4_VPN_INSTANCE* p_instance,tBGP4_AGGR *p_range)
{
    tBGP4_ROUTE *p_aggr_rt=NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_LINK  *p_link = NULL;
    tBGP4_LIST rtlist;
    tBGP4_LIST flist;
    tBGP4_LIST wlist;

    bgp4_lstinit(&rtlist);            
    bgp4_lstinit(&flist);
    bgp4_lstinit(&wlist);

    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"pointer instance  is NULL");
    }



    /*create route for range*/
    if(p_range->state==SNMP_NOTINSERVICE)
    {
        return TRUE;
    }
    p_range->state=SNMP_NOTINSERVICE;
        
    p_aggr_rt =p_range->p_aggr_rt; 
    if (p_aggr_rt == NULL) 
    {
        return (FALSE);
    }
   
    /*obtain route list be aggregated*/
    bgp4_rib_get_by_range(&p_instance->rib, &p_aggr_rt->dest, &rtlist);
    
    bgp4_log(BGP_DEBUG_UPDATE,1,"get %d aggregate route", bgp4_lstcnt(&rtlist));
    
    bgp4_display_aggr_route_list(&rtlist);

    if (bgp4_rtlist_aggregate_enable(&rtlist) == TRUE) 
    {   
        /*merge them*/
        /*for each entry in route list,aggregate route info*/
        LST_LOOP(&rtlist, p_link, node, tBGP4_LINK)
        {
            p_route=p_link->p_route;

            if(p_range->summaryonly==TRUE)
            {
                if(p_route->filter_by_range==TRUE)
                {
                    p_route->filter_by_range=FALSE;
                    bgp4_schedule_rib_update(p_instance,NULL,p_route,NULL);
                }
            }
        }
        
        p_range->p_aggr_rt=NULL;
        p_aggr_rt->is_deleted = TRUE ;
        bgp4_schedule_rib_update(p_instance,p_aggr_rt,NULL,NULL);
    } 
    else
    {
        p_range->p_aggr_rt=NULL;
        bgp4_rib_delete(&p_instance->rib, p_aggr_rt) ;
        bgp4_release_route(p_aggr_rt);
    }
        
    bgp4_rtlist_clear(&rtlist);
    bgp4_rtlist_clear(&flist);
    bgp4_rtlist_clear(&wlist);

    return TRUE;
}



#endif
