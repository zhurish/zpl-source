
#include "bgp4tree.h"

#ifdef NEW_BGP_WANTED
#include "bgp4com.h"

/*compare function used in rib tree*/
int 
bgp4_route_lookup_cmp(
        tBGP4_ROUTE *p1, 
        tBGP4_ROUTE *p2)
{
    tBGP4_ADDR dest1;
    tBGP4_ADDR dest2;
    
    int rc = 0;
    if (p1 == NULL || p2 == NULL)
    {
        return p1 ? 1 : -1;
    }

    /*vpls compate dest directly*/
    if ((p1->dest.afi == BGP4_PF_L2VPLS)
        && (p2->dest.afi == BGP4_PF_L2VPLS))
    {
        rc = memcmp(p1->dest.ip, p2->dest.ip, BGP4_VPLS_NLRI_LEN);
        if (rc)
        {
            return (rc > 0) ? 1 : -1;
        }
        
        /*protocol*/
        if (p1->proto != p2->proto)
        {
            return (p1->proto > p2->proto) ? 1 : -1;
        }
        return 0;
    }

    /*special for ipvpn*/
    if ((p1->dest.afi == BGP4_PF_IPVPN)
        && (p2->dest.afi == BGP4_PF_IPVPN))
    {
        /*compare RD*/
        rc = memcmp(p1->dest.ip, p2->dest.ip, BGP4_VPN_RD_LEN);
        if (rc)
        {
            return (rc > 0) ? 1 : -1;
        }
        memset(&dest1, 0, sizeof(dest1));
        memset(&dest2, 0, sizeof(dest2));
        dest1.afi = BGP4_PF_IPUCAST;
        dest1.prefixlen = p1->dest.prefixlen - (BGP4_VPN_RD_LEN * 8);
        memcpy(dest1.ip, p1->dest.ip + BGP4_VPN_RD_LEN, 4);
    
        dest2.afi = BGP4_PF_IPUCAST;
        dest2.prefixlen = p2->dest.prefixlen - (BGP4_VPN_RD_LEN * 8);
        memcpy(dest2.ip, p2->dest.ip + BGP4_VPN_RD_LEN, 4);
    
        /*ipv4 prefix compare*/
        rc = bgp4_prefixcmp(&dest1, &dest2);
        if (rc)
        {
            return (rc > 0) ? 1 : -1;
        }
    }
    else
    {
        /*prefix compare*/
        rc = bgp4_prefixcmp(&p1->dest, &p2->dest);
        if (rc)
        {
            return (rc > 0) ? 1 : -1;
        }
    }
    
    /*protocol*/
    if (p1->proto != p2->proto)
    {
        return (p1->proto > p2->proto) ? 1 : -1;
    }
    if (!p1->p_path || !p2->p_path)
    {
        if (p2->summary_route || p1->summary_route)
        {
            return 0;
        }
        return p1->p_path ? 1 : -1;
    }

    /*peer*/
    if (p1->p_path->p_peer != p2->p_path->p_peer)
    {
        if (p1->p_path->p_peer && p2->p_path->p_peer)
        {
            rc = bgp4_prefixcmp(&p1->p_path->p_peer->ip, &p2->p_path->p_peer->ip);
            if (rc)
            {
                return (rc > 0) ? 1 : -1;
            }
            return 0;
        }
        return p1->p_path->p_peer ? 1 : -1;
    }
    /*imported route,for IGP ECMP,only compare nexthop*/
    if ((p1->proto != M2_ipRouteProto_bgp)
        && (p1->proto != M2_ipRouteProto_local)
        && (p1->proto != M2_ipRouteProto_other))
    {
        if (p1->dest.afi == BGP4_PF_IPUCAST)
        {
            rc = bgp4_prefixcmp(&p1->p_path->nexthop, &p2->p_path->nexthop);
        }       
        if (rc )
        {
            return (rc > 0) ? 1 : -1;
        }
    }
    return 0;
}

/*clear all rib nodes*/
void
bgp4_route_table_flush(avl_tree_t *p_root)
{
    bgp4_avl_walkup(p_root, bgp4_route_table_delete);
    return;
}

/*add route to rib,increase reference*/
void 
bgp4_route_table_add(
        avl_tree_t *p_root,
        tBGP4_ROUTE *p_route)
{
    /*only insert to table once*/
    if (p_route->p_table != NULL)
    {
        return;
    }
    bgp4_avl_add(p_root, p_route);
    p_route->p_table = p_root;

    return;
}

/*remove route from rib tree,and decrease reference,we must ensure that input route is already in tree*/
void 
bgp4_route_table_delete(tBGP4_ROUTE *p_route)
{
    /*stopped if no table exist*/
    if (p_route->p_table == NULL)
    {
        return;
    }

    bgp4_avl_delete(p_route->p_table, p_route);
    p_route->p_table = NULL;
    /*decrease reference,may delete it*/
    bgp4_route_release(p_route);
    return;
}

/*compare two route's path*/
int 
bgp4_route_priority_cmp(
       tBGP4_ROUTE *p1, 
       tBGP4_ROUTE *p2)
{
    tBGP4_PATH *p_path1 = NULL;
    tBGP4_PATH *p_path2 = NULL;
    u_char type1 = BGP4_IBGP;
    u_char type2 = BGP4_IBGP;
    u_int len1 = 0;
    u_int len2 = 0;
    int rc = 0;
    u_short as1 = 0, as2 = 0;

    /*pointer checking*/
    if ((p1 == NULL) || (p2 == NULL))
    {
        return ((u_long)p1 > (u_long)p2) ? 1 : -1;
    }

    /*deleted route has lowest proirity*/
    if (p1->is_deleted != p2->is_deleted)
    {
        return (p1->is_deleted > p2->is_deleted) ? -1 : 1;
    }

    p_path1 = p1->p_path;
    p_path2 = p2->p_path;

    if (!p_path1 || !p_path2)
    {
        return p_path1 ? 1 : -1;
    }
#if 0/* support fast backup route*/
    /*protocol preference value,compare it firstly*/
    if(p1->preference != p2->preference)
    {
        return p1->preference > p2->preference ? 1 : -1;
    }
#endif
    /*summary route has higher priority*/
    if (p1->summary_route != p2->summary_route)
    {
        return p1->summary_route > p2->summary_route ? 1 : -1;
    }

    /*aggregate is prefered*/
    if (p_path1->atomic_exist != p_path2->atomic_exist)
    {
        return p_path1->atomic_exist > p_path2->atomic_exist ? 1 : -1;
    }
    if (p1->proto != p2->proto)
    {
        return (p2->proto > p1->proto) ? 1 : -1;
    }

    /*short aspath prefered*/
    len1 = bgp4_path_aspath_len(p_path1);
    len2 = bgp4_path_aspath_len(p_path2);
    if (len1 != len2)
    {
        return len2 > len1 ? 1 : -1;
    }

    /*less origin prefered*/
    if (p_path1->origin != p_path2->origin)
    {
        return (p_path2->origin > p_path1->origin) ? 1 : -1;
    }

    bgp4_as_first_in_aspath(&p_path1->aspath_list, &as1);
    bgp4_as_first_in_aspath(&p_path2->aspath_list, &as2);
    /*for ebgp route,less med is prefered*/
    if (as1 && as2)
    {
        /*only compare MED in the same AS*/
        if ((as1 == as2) && (as1 != gbgp4.as))
        {
            if (p_path1->med != p_path2->med)
            {
                return (p_path2->med > p_path1->med) ? 1 : -1;
            }
        }
    }
    #if 0
    /*for ebgp route,less med is prefered*/
    if (p_path1->p_peer && p_path2->p_peer)
    {
        /*only compare MED in the same AS*/
        if ((p_path1->p_peer->as == p_path2->p_peer->as)
            && (bgp4_peer_type(p_path1->p_peer) == BGP4_EBGP))
        {
            if (p_path1->med != p_path2->med)
            {
                return (p_path2->med > p_path1->med) ? 1 : -1;
            }
        }
    }
    #endif
    /*following is the case of bgp routes*/
    /*external route is more prefered than internal route*/
    if (p_path1->p_peer)
    {
        type1 = bgp4_peer_type(p_path1->p_peer);
    }
    if (p_path2->p_peer)
    {
        type2 = bgp4_peer_type(p_path2->p_peer);
    }
    if (type1 != type2)
    {
        return (type2 > type1) ? 1 : -1;
    }

    /*for ibgp route,larger local-pref is prefered*/
    if ((type1 == BGP4_IBGP) || (type1 == BGP4_CONFEDBGP))
    {
        if (p_path1->localpref != p_path2->localpref)
        {
            return (p_path1->localpref > p_path2->localpref) ? 1 : -1;
        }
    }

    /*less igp metric prefered.tbi*/
    rc = bgp4_nexthop_metric_cmp(p_path1, p_path2);
    if (rc != 0)
    {
        return rc;
    }

    /*less peer id is prefered*/
    if (p_path1->p_peer && p_path2->p_peer)
    {
        return (p_path2->p_peer->router_id > p_path1->p_peer->router_id) ? 1 : -1;
    }

    /*at last,assume same,support ecmp????*/
    return 0 ;
}

/*for a group of routes with same dest,select the best one for forwarding*/
tBGP4_ROUTE *
bgp4_best_route(
    avl_tree_t *p_root, 
    tBGP4_ADDR *p_dest)
{
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_ROUTE route;

    memset(&route, 0, sizeof(route));
    memcpy(&route.dest, p_dest, sizeof(route.dest));

    bgp4_avl_for_each_greater(p_root, p_route, &route)
    {
        /*prefix must match*/
        if (bgp4_prefixcmp(p_dest, &p_route->dest))
        {
            break;
        }
        /*ignore any route will be deleted*/
        if (p_route->is_deleted)
        {
            continue;
        }
        if (bgp4_route_priority_cmp(p_route, p_best) > 0)
        {
            p_best = p_route;
        }
    }
    return p_best;
}

/*lookup all routes with same dest*/
void
bgp4_rib_lookup_dest(
          avl_tree_t *p_root,
          tBGP4_ADDR *p_dest,
          tBGP4_ROUTE_VECTOR *p_vector)
{
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE route;

    /*clear output vector*/
    memset(p_vector, 0, sizeof(*p_vector));

    /*set dest*/
    memset(&route, 0, sizeof(route));
    memcpy(&route.dest, p_dest, sizeof(route.dest));

    /*scan all routes with same dest*/
    bgp4_avl_for_each_greater(p_root, p_route, &route)
    {
        /*prefix must match*/
        if (bgp4_prefixcmp(p_dest, &p_route->dest))
        {
            break;
        }
        if (p_vector->count < BGP4_MAX_ECMP)
        {
            p_vector->p_route[p_vector->count++] = p_route;
        }
    }
    return ;
}
/*get special route in an vector*/
tBGP4_ROUTE *
bgp4_rib_vector_lookup(
           tBGP4_ROUTE_VECTOR *p_vector, 
           tBGP4_ROUTE *p_route)
{
    u_int i = 0;
    for (i = 0 ; i < p_vector->count ; i++)
    {
        if (p_vector->p_route[i] 
            && (bgp4_route_lookup_cmp(p_vector->p_route[i], p_route) == 0))
        {
            return p_vector->p_route[i];
        }
    }
    return NULL;
}

/*get best route in vector*/
tBGP4_ROUTE *
bgp4_rib_vector_best(tBGP4_ROUTE_VECTOR *p_vector)
{
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_ROUTE *p_route = NULL;
    u_int i = 0;
    for (i = 0 ; i < p_vector->count ; i++)
    {
        p_route = p_vector->p_route[i];
        if (p_route == NULL)
        {
            continue;
        }
        /*ignore any route will be deleted*/
        if (p_route->is_deleted)
        {
            continue;
        }
        /*ignore route waiting for igp sync*/
        if (p_route->igp_sync_wait)
        {
            continue;
        }
        
        if ((p_best == NULL) 
            || (bgp4_route_priority_cmp(p_route, p_best) > 0))
        {
            p_best = p_route;
        }
    }
    return p_best;
}

/*get best route index in vector*/
u_int
bgp4_rib_vector_best_index(tBGP4_ROUTE_VECTOR *p_vector)
{
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_ROUTE *p_route = NULL;
    u_int i = 0;
    u_int best_id = BGP4_MAX_ECMP;
    
    for (i = 0 ; i < p_vector->count ; i++)
    {
        p_route = p_vector->p_route[i];
        if (p_route == NULL)
        {
            continue;
        }
        /*ignore any route will be deleted*/
        if (p_route->is_deleted)
        {
            continue;
        }
        /*ignore route waiting for igp sync*/
        if (p_route->igp_sync_wait)
        {
            continue;
        }
        
        if ((p_best == NULL) 
            || (bgp4_route_priority_cmp(p_route, p_best) > 0))
        {
            p_best = p_route;
            best_id = i;
        }
    }
    return best_id;
}

/*compare two route's path*/
int 
bgp4_route_ECMP_priority_cmp(
        tBGP4_ROUTE *p1, 
        tBGP4_ROUTE *p2)
{
    tBGP4_PATH *p_path1 = NULL;
    tBGP4_PATH *p_path2 = NULL;   
    u_char type1 = BGP4_IBGP;
    u_char type2 = BGP4_IBGP;
    u_int len1 = 0;
    u_int len2 = 0;
     
    /*pointer checking*/
    if ((p1 == NULL) || (p2 == NULL))
    {
        return ((u_long)p1 > (u_long)p2) ? 1 : -1;
    }

    /*deleted route has lowest proirity*/
    if (p1->is_deleted != p2->is_deleted)
    {
        return (p1->is_deleted > p2->is_deleted) ? -1 : 1;
    }
             
    p_path1 = p1->p_path;
    p_path2 = p2->p_path;   

    if (!p_path1 || !p_path2)
    {
        return p_path1 ? 1 : -1;
    }

    /*protocol preference value,compare it firstly*/
    if (p1->preference != p2->preference)
    {
        return p1->preference > p2->preference ? 1 : -1;
    }
    /*for ibgp route,larger local-pref is prefered*/
    if (type1 == BGP4_IBGP)   
    {
        if (p_path1->localpref != p_path2->localpref)
        {
            return (p_path1->localpref > p_path2->localpref) ? 1 : -1;
        }
    }
    
    /*summary route has lower priority*/
    if (p1->summary_route != p2->summary_route) 
    {
        return p2->summary_route > p1->summary_route ? 1 : -1;
    }

    /*not-aggregate is prefered*/ 
    if (p_path1->atomic_exist != p_path2->atomic_exist)
    {
        return p_path2->atomic_exist > p_path1->atomic_exist ? 1 : -1;
    }

    /*short aspath prefered*/
    len1 = bgp4_path_aspath_len(p_path1) ;
    len2 = bgp4_path_aspath_len(p_path2) ;
    if (len1 != len2) 
    {
        return len2 > len1 ? 1 : -1;
    }

    /*less origin prefered*/
    if (p_path1->origin != p_path2->origin)
    {
        return (p_path2->origin > p_path1->origin) ? 1 : -1;
    }

    if (p1->proto != p2->proto) 
    {
        return (p2->proto > p1->proto) ? 1 : -1;
    }

    if(p_path1->med != p_path2->med)
    {
        return (p_path1->med < p_path2->med) ? 1 : -1;
    }

    /*for ebgp route,less med is prefered*/
    if(p_path1->p_peer && p_path2->p_peer)
    {
        /*only compare MED in the same AS*/
        if (p_path1->p_peer->as == p_path2->p_peer->as)
        {
            if (p_path1->med != p_path2->med)
            {
                return (p_path2->med > p_path1->med) ? 1 : -1;
            }
        }
    }        
     
    /*following is the case of bgp routes*/
    /*external route is more prefered than internal route*/ 
    if (p_path1->p_peer)
    {
        type1 = bgp4_peer_type(p_path1->p_peer);
    }
    if (p_path2->p_peer)
    {
        type2 = bgp4_peer_type(p_path2->p_peer);
    }
    if (type1 != type2)
    {
        return (type2 > type1) ? 1 : -1;
    }
    return 0;
}

void 
bgp4_rib_vector_ecmp_best(
      tBGP4_ROUTE_VECTOR *p_vector,
      tBGP4_ROUTE_VECTOR *p_ecmp)
{
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_best = NULL;
    u_int i = 0;
    int rc = 0;
    
    for (i = 0 ; i < p_vector->count ; i++)
    {
        p_route = p_vector->p_route[i];
        if (p_route == NULL)
        {
            continue ;
        }
            
        /*ignore any route will be deleted*/
        if (p_route->is_deleted)
        {
            continue ;
        }
        rc = bgp4_route_ECMP_priority_cmp(p_route, p_best); 
        if (rc < 0)
        {
            continue;
        }
        if (rc > 0)
        {
            memset(p_ecmp, 0, sizeof(*p_ecmp));
        }
        p_best = p_route;
        p_ecmp->p_route[p_ecmp->count ++]= p_best;
    }
    return;
}

/*get routes matched to summary dest*/
void 
bgp4_aggregated_route_get(
          tBGP4_VPN_INSTANCE *p_instance,
          tBGP4_ADDR *p_dest, 
          avl_tree_t *p_list)
{
    tBGP4_ROUTE *p_route = NULL;
    avl_tree_t *p_table = &p_instance->rib[p_dest->afi]->rib_table;
    u_int first_med = 0;
    u_int first_route = 0;
    if (p_table == NULL)
    {
        return;
    }

    /*scan for all rib nodes,may changed to just contain summary*/
    bgp4_avl_for_each(p_table, p_route)
    {
        /*summary route can not be aggregated*/
        if (p_route->summary_route)
        {
            continue;
        }
        /*deleted route can not be aggregated*/
        if (p_route->is_deleted == TRUE)
        {
            continue;
        }

        /*af must same*/
        if (p_route->dest.afi != p_dest->afi)
        {
            continue;
        }
        /*prefix must belong to range*/
        if (bgp4_prefixmatch(p_route->dest.ip, p_dest->ip, p_dest->prefixlen))
        {
            /*only obtain route can be aggregated.they must has 
              same med*/
            if (first_route == 0)
            {
                first_med = p_route->p_path->med;
                first_route++;
            }
            else if (first_med != p_route->p_path->med)
            {
                continue;
            }
            bgp4_rtlist_add(p_list, p_route);
        }
    }
    return ;
}

/*unsorted cmp function for avl,it is like a linklist*/
int
bgp4_avl_unsort_cmp(
       void *p1,
       void *p2)
{
    /*always return 1*/
    return 1;
}

#else

#include "bgp4com.h"


/*compare function used in rib tree*/
static int rib_cmp(void *p1 , void *p2)
{
    tBGP4_ROUTE *p_route1 = (tBGP4_ROUTE *)p1;
    tBGP4_ROUTE *p_route2 = (tBGP4_ROUTE *)p2;
    int rc = 0;


    if (p_route1 == NULL || p_route2 == NULL)
    {
        return p_route1 ? 1 : -1;
    }
    
    /*special for ipvpn*/
    if ((p_route1->dest.afi == BGP4_PF_IPVPN)
        && (p_route2->dest.afi == BGP4_PF_IPVPN))
    {
        tBGP4_ADDR dest1;
        tBGP4_ADDR dest2;
        /*compare RD*/
        rc = memcmp(p_route1->dest.ip, p_route2->dest.ip, BGP4_VPN_RD_LEN);
        if (rc)
        {
            return (rc > 0) ? 1 : -1;
        }
        memset(&dest1, 0, sizeof(dest1));
        memset(&dest2, 0, sizeof(dest2));
        dest1.afi = BGP4_PF_IPUCAST;
        dest1.prefixlen = p_route1->dest.prefixlen - (BGP4_VPN_RD_LEN * 8);
        memcpy(dest1.ip, p_route1->dest.ip + BGP4_VPN_RD_LEN, 4);
    
        dest2.afi = BGP4_PF_IPUCAST;
        dest2.prefixlen = p_route2->dest.prefixlen - (BGP4_VPN_RD_LEN * 8);
        memcpy(dest2.ip, p_route2->dest.ip + BGP4_VPN_RD_LEN, 4);
    
        /*ipv4 prefix compare*/
        rc = bgp4_prefixcmp(&dest1, &dest2);
        if (rc)
        {
            return (rc > 0) ? 1 : -1;
        }
    }
    else
    {
        /*prefix compare*/
        rc = bgp4_prefixcmp(&p_route1->dest, &p_route2->dest);
        if (rc)
        {
            return (rc > 0) ? 1 : -1;
        }
    }
    /*protocol*/
    if (p_route1->proto != p_route2->proto)
    {

        return (p_route1->proto > p_route2->proto) ? 1 : -1;
    }
    if (!p_route1->p_path || !p_route2->p_path)
    {
        if(p_route2->is_summary||p_route1->is_summary)
        {
            return 0;
        }
        else
        {
            return p_route1->p_path ? 1 : -1;
        }
    }

    /*peer*/
    if (p_route1->p_path->p_peer != p_route2->p_path->p_peer)
    {
        if (p_route1->p_path->p_peer && p_route2->p_path->p_peer)
        {
            rc = bgp4_prefixcmp(&p_route1->p_path->p_peer->remote.ip, &p_route2->p_path->p_peer->remote.ip);
            if (rc)
            {

                return (rc > 0) ? 1 : -1;
            }
            else if(rc==0)
            {
                return 0;
            }
        }

        return p_route1->p_path->p_peer ? 1 : -1;
    }
    /*imported route,for IGP ECMP,only compare nexthop*/
    if(p_route1->proto != M2_ipRouteProto_bgp && 
        p_route1->proto != M2_ipRouteProto_local&&
         p_route1->proto != M2_ipRouteProto_other)
    {
        if(p_route1->dest.afi == BGP4_PF_IPUCAST)
        {
            rc = bgp4_prefixcmp(&p_route1->p_path->nexthop, &p_route2->p_path->nexthop);
        }
        else if(p_route1->dest.afi == BGP4_PF_IP6UCAST)
        {
            rc = bgp4_prefixcmp(&p_route1->p_path->nexthop_global, &p_route2->p_path->nexthop_global);
        }

        if (rc )
        {

            return (rc > 0) ? 1 : -1;
        }
        else if(rc == 0)
        {
            return 0;
        }

    }
    return 0 ;
}


/*init rib tree*/
int bgp4_init_rib(tBGP4_RIB *p_root)
{
    avl_create(p_root, (void*)rib_cmp , sizeof(tBGP4_ROUTE), 0);
    return TRUE;
}

/*clear all rib nodes*/
int  bgp4_clear_rib(tBGP4_RIB *p_root)
{
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_next = NULL;

    RIB_LOOP(p_root, p_route, p_next)
    {
        bgp4_rib_delete(p_root, p_route);
    }
    avl_destroy(p_root);
    return (TRUE);
}

/*add route to rib,increase reference*/
STATUS bgp4_rib_add(
                    tBGP4_RIB *p_root,
                    tBGP4_ROUTE  *p_route )
{
    if (!p_root || !p_route)
    {
        return VOS_ERR;
    }

    if(p_route->rib_add==1)
    {
        return VOS_OK;
    }
    avl_add(p_root, p_route);
    p_route->rib_add=1;
    p_route->refer++;
    return VOS_OK;
}

u_int bgp4_rib_count(tBGP4_RIB *p_root)
{
    u_int route_num=0;
    if(p_root==NULL)
    {
        return route_num;
    }

    route_num=avl_numnodes(p_root);
    return route_num;
}

/*remove route from rib tree,and decrease reference,we must ensure that input route is already in tree*/
void bgp4_rib_delete(tBGP4_RIB *p_root, tBGP4_ROUTE *p_route)
{
    if(p_route->rib_add==0)
    {
        return ;
    }

    avl_remove(p_root, p_route);
    p_route->rib_add=0;
    /*decrease reference,may delete it*/
    bgp4_release_route(p_route);
    return;
}

/*lookup a special route with same dest,protocol and peer*/
tBGP4_ROUTE *bgp4_rib_lookup (tBGP4_RIB *p_root, tBGP4_ROUTE *p_route)
{
    avl_index_t where;
    return (tBGP4_ROUTE *)avl_find(p_root, p_route, &where);
}

/*compare two route's path*/
static signed int bgp4_route_priority_cmp(tBGP4_ROUTE *p_route1, tBGP4_ROUTE *p_route2)
{
    tBGP4_PATH *p_path1 = NULL ;
    tBGP4_PATH *p_path2 = NULL ;
    u_char type1 = BGP4_IBGP ;
    u_char type2 = BGP4_IBGP ;
    u_int len1 = 0 ;
    u_int len2 = 0 ;
    int rc = 0;

    /*pointer checking*/
    if ((p_route1 == NULL) || (p_route2 == NULL))
    {
        return ((u_long)p_route1 > (u_long)p_route2) ? 1 : -1;
    }

    /*deleted route has lowest proirity*/
    if (p_route1->is_deleted != p_route2->is_deleted)
    {
        return (p_route1->is_deleted > p_route2->is_deleted) ? -1 : 1;
    }

    p_path1 = p_route1->p_path ;
    p_path2 = p_route2->p_path ;

    if (!p_path1 || !p_path2)
    {
        return p_path1 ? 1 : -1;
    }

    /*protocol preference value,compare it firstly*/
    if(p_route1->preference != p_route2->preference)
    {
        return p_route1->preference > p_route2->preference ? 1 : -1;
    }

    /*for ibgp route,larger local-pref is prefered*/
    if (type1 == BGP4_IBGP)
    {
        if (p_path1->out_localpref != p_path2->out_localpref)
        {
            return (p_path1->out_localpref > p_path2->out_localpref) ? 1 : -1;
        }
    }
#if 0
    if(p_route1->cost != p_route2->cost)/*set by policy*/
    {
        return p_route1->cost < p_route2->cost ? 1 : -1;
    }
#endif

    /*summary route has higher priority*/
    if (p_route1->is_summary != p_route2->is_summary)
    {
        return p_route1->is_summary > p_route2->is_summary ? 1 : -1;
    }

    /*aggregate is prefered*/
    if (p_path1->flags.atomic != p_path2->flags.atomic)
    {
        return p_path1->flags.atomic > p_path2->flags.atomic ? 1 : -1;
    }
#if 0
    /*igp route is more prefered than bgp route*/
    if (is_bgp_route(p_route1) && (!is_bgp_route(p_route2)))
    {
        return (-1) ;
    }

    if ((!is_bgp_route(p_route1)) && is_bgp_route(p_route2))
    {
        return 1 ;
    }

    /*if both non-bgp route, less protocol id is prefered*/
    if ((!is_bgp_route(p_route2)) && (!is_bgp_route(p_route1)))
    {
        return (p_route2->proto > p_route1->proto) ? 1 : -1;
    }
#endif
    if (p_route1->proto != p_route2->proto)
    {
        return (p_route2->proto > p_route1->proto) ? 1 : -1;
    }

    /*short aspath prefered*/
    len1 = bgp4_path_aspath_len(p_path1) ;
    len2 = bgp4_path_aspath_len(p_path2) ;
    if (len1 != len2)
    {
        return len2 > len1 ? 1 : -1;
    }

    /*less origin prefered*/
    if (p_path1->origin != p_path2->origin)
    {
        return (p_path2->origin > p_path1->origin) ? 1 : -1;
    }

    if(p_path1->out_med != p_path2->out_med)
    {
        return (p_path1->out_med < p_path2->out_med) ? 1 : -1;
    }

    /*for ebgp route,less med is prefered*/
    if(p_path1->p_peer && p_path2->p_peer)
    {
        /*only compare MED in the same AS*/
        if (p_path1->p_peer->remote.as == p_path2->p_peer->remote.as)
        {
            if (p_path1->out_med != p_path2->out_med)
            {
                return (p_path2->out_med > p_path1->out_med) ? 1 : -1;
            }
        }
    }

    /*following is the case of bgp routes*/
    /*external route is more prefered than internal route*/
    if (p_path1->p_peer)
    {
        type1 = bgp4_peer_type(p_path1->p_peer);
    }
    if (p_path2->p_peer)
    {
        type2 = bgp4_peer_type(p_path2->p_peer);
    }
    if (type1 != type2)
    {
        return (type2 > type1) ? 1 : -1;
    }

    /*less igp metric prefered.tbi*/
    rc = bgp4_nexthop_metric_comp(p_path1,p_path2);
    if(rc != 0)
    {
        return rc;
    }

    /*less peer id is prefered*/
    if (p_path1->p_peer && p_path2->p_peer)
    {
        return (p_path2->p_peer->router_id > p_path1->p_peer->router_id) ? 1 : -1;
    }

    /*at last,assume same,support ecmp????*/

    return 0 ;
}

/*for a group of routes with same dest,select the best one for forwarding*/
tBGP4_ROUTE *bgp4_best_route(tBGP4_RIB *p_root, tBGP4_ADDR *p_dest)
{
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_RIBNODE *p_node = NULL;
    tBGP4_ROUTE *p_best = NULL;
    avl_index_t where;
    tBGP4_ROUTE route;

    memset(&route, 0, sizeof(route));
    memcpy(&route.dest, p_dest, sizeof(route.dest));

    p_node = avl_find(p_root, &route, &where);
    if (p_node == NULL)
    {
        p_node = avl_nearest(p_root, where, AVL_AFTER);
    }

    for (; p_node; p_node = AVL_NEXT(p_root, p_node))
    {
        p_route = (tBGP4_ROUTE *)p_node;
        /*prefix must match*/
        if (bgp4_prefixcmp(p_dest, &p_route->dest))
        {
            break;
        }
        /*ignore any route will be deleted*/
        if (p_route->is_deleted)
        {
            continue ;
        }
        /*ignore any route is deferral,check for grace*/
        if(p_route->deferral)
        {
            continue;
        }
        if (bgp4_route_priority_cmp(p_route, p_best) > 0)
        {
            p_best = p_route;
        }
    }
    return p_best;
}

/*get route set for the first network*/
STATUS
bgp4_rib_network_first(
          tBGP4_RIB *p_root,
          tBGP4_ROUTE_VECTOR *p_vector)
{

    tBGP4_ROUTE  *p_scanroute = NULL;
    tBGP4_ROUTE  *p_next = NULL;

    memset(p_vector, 0, sizeof(tBGP4_ROUTE_VECTOR));

    /*scan all routes*/
    RIB_LOOP(p_root, p_scanroute, p_next)
    {
         /*if vector is not empty,and dest is not same,process this vector now*/
        if (p_vector->count && bgp4_prefixcmp(&p_vector->p_route[0]->dest, &p_scanroute->dest))
        {
            break;
        }
        /*add route to this vector*/
        p_vector->p_route[p_vector->count++] = p_scanroute;
    }
    return p_vector->count ? VOS_OK : VOS_ERR;
}

/*get route set for the next network*/
STATUS
bgp4_rib_network_next(
          tBGP4_RIB *p_root,
          tBGP4_ROUTE_VECTOR *p_vector)
{
    tBGP4_ROUTE  *p_scanroute = NULL;
    tBGP4_ROUTE  *p_next = NULL;

       if (p_vector->count == 0 || p_vector->p_route[p_vector->count-1] == NULL)
       {
           return VOS_ERR;
       }
       p_scanroute = p_vector->p_route[p_vector->count-1];

    memset(p_vector, 0, sizeof(tBGP4_ROUTE_VECTOR));

    /*scan all routes*/
    for (; p_scanroute; p_scanroute = AVL_NEXT(p_root, p_scanroute))
    {
         /*if vector is not empty,and dest is not same,process this vector now*/
        if (p_vector->count && bgp4_prefixcmp(&p_vector->p_route[0]->dest, &p_scanroute->dest))
        {
            break;
        }
        /*add route to this vector*/
        p_vector->p_route[p_vector->count++] = p_scanroute;
    }
    return p_vector->count ? VOS_OK : VOS_ERR;
}

/*lookup all routes with same dest*/
void
bgp4_rib_lookup_vector(
          tBGP4_RIB *p_root,
          tBGP4_ADDR *p_dest,
          tBGP4_ROUTE_VECTOR *p_vector)
{
    tBGP4_RIBNODE *p_node = NULL;
    tBGP4_ROUTE *p_route = NULL;
    avl_index_t where;
    tBGP4_ROUTE route;

    /*clear output vector*/
    memset(p_vector, 0, sizeof(*p_vector));

    /*set dest*/
    memset(&route, 0, sizeof(route));
    memcpy(&route.dest, p_dest, sizeof(route.dest));

    /*scan all routes with same dest*/
    p_node = avl_find(p_root, &route, &where);
    if (p_node == NULL)
    {
        p_node = avl_nearest(p_root, where, AVL_AFTER);
    }

    for (; p_node; p_node = AVL_NEXT(p_root, p_node))
    {
        p_route = (tBGP4_ROUTE *)p_node;
        /*prefix must match*/
        if (bgp4_prefixcmp(p_dest, &p_route->dest))
        {
            break;
        }
        if (p_vector->count < BGP4_MAX_ECMP)
        {
            p_vector->p_route[p_vector->count++] = p_route;
        }
    }
    return ;
}
/*compare two route's path*/
int bgp4_route_ECMP_priority_cmp(tBGP4_ROUTE *p_route1, tBGP4_ROUTE *p_route2)
{
    tBGP4_PATH *p_path1 = NULL ;
    tBGP4_PATH *p_path2 = NULL ;   
    u_char type1 = BGP4_IBGP ;
    u_char type2 = BGP4_IBGP ;
    u_int len1 = 0 ;
    u_int len2 = 0 ;
    int rc = 0;
     
    /*pointer checking*/
    if ((p_route1 == NULL) || (p_route2 == NULL))
    {
        return ((u_long)p_route1 > (u_long)p_route2) ? 1 : -1;
    }

    /*deleted route has lowest proirity*/
    if (p_route1->is_deleted != p_route2->is_deleted)
    {
        return (p_route1->is_deleted > p_route2->is_deleted) ? -1 : 1;
    }

    /*small proto number has better priority*/
    if (p_route1->proto != p_route2->proto) 
    {
        return (p_route2->proto > p_route1->proto) ? 1 : -1;
    }
             
    p_path1 = p_route1->p_path ;
    p_path2 = p_route2->p_path ;   

    if (!p_path1 || !p_path2)
    {
        return p_path1 ? 1 : -1;
    }

    /*protocol preference value,compare it firstly*/
    if(p_route1->preference != p_route2->preference)
    {
        return p_route1->preference > p_route2->preference ? 1 : -1;
    }
#if 0
    if(p_route1->cost != p_route2->cost)/*set by policy*/
    {
        return p_route1->cost < p_route2->cost ? 1 : -1;
    }
#endif
    /*for ibgp route,larger local-pref is prefered*/
    if (type1 == BGP4_IBGP)   
    {
        if (p_path1->out_localpref != p_path2->out_localpref)
        {
            return (p_path1->out_localpref > p_path2->out_localpref) ? 1 : -1;
        }
    }
    
    /*summary route has lower priority*/
    if (p_route1->is_summary != p_route2->is_summary) 
    {
        return p_route2->is_summary > p_route1->is_summary ? 1 : -1;
    }

    /*not-aggregate is prefered*/ 
    if (p_path1->flags.atomic != p_path2->flags.atomic)
    {
        return p_path2->flags.atomic > p_path1->flags.atomic ? 1 : -1;
    }

    /*short aspath prefered*/
    len1 = bgp4_path_aspath_len(p_path1) ;
    len2 = bgp4_path_aspath_len(p_path2) ;
    if (len1 != len2) 
    {
        return len2 > len1 ? 1 : -1;
    }

    /*less origin prefered*/
    if (p_path1->origin != p_path2->origin)
    {
        return (p_path2->origin > p_path1->origin) ? 1 : -1;
    }

    if(p_path1->out_med != p_path2->out_med)
    {
        return (p_path1->out_med < p_path2->out_med) ? 1 : -1;
    }

    /*for ebgp route,less med is prefered*/
    if(p_path1->p_peer && p_path2->p_peer)
    {
        /*only compare MED in the same AS*/
        if (p_path1->p_peer->remote.as == p_path2->p_peer->remote.as)
        {
            if (p_path1->out_med != p_path2->out_med)
            {
                return (p_path2->out_med > p_path1->out_med) ? 1 : -1;
            }
        }
    }        
     
    /*following is the case of bgp routes*/
    /*external route is more prefered than internal route*/ 
    if (p_path1->p_peer)
    {
        type1 = bgp4_peer_type(p_path1->p_peer);
    }
    if (p_path2->p_peer)
    {
        type2 = bgp4_peer_type(p_path2->p_peer);
    }
    if (type1 != type2)
    {
        return (type2 > type1) ? 1 : -1;
    }
    /*less igp metric prefered.tbi*/
    rc = bgp4_nexthop_metric_comp(p_path1, p_path2);
    if(rc != 0)
    {
        return rc;
    }

    /*less peer id is prefered*/
    if (p_path1->p_peer && p_path2->p_peer)
    {
        return (p_path2->p_peer->router_id > p_path1->p_peer->router_id) ? 1 : -1;
    }

    /*at last,assume same,support ecmp????*/
    return 0 ;
}

void bgp4_best_ECMP_route(tBGP4_ROUTE_VECTOR *p_vector_in,tBGP4_ROUTE_VECTOR *p_vector_out)
{
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_best = NULL;
    u_int i = 0 ;
    u_int j = 0;
    

    for (i = 0 ; i < p_vector_in->count ; i++)
    {
        p_route = p_vector_in->p_route[i];
        if (p_route == NULL)
        {
            continue ;
        }
            
        /*ignore any route will be deleted*/
        if (p_route->is_deleted)
        {
            continue ;
        }
        /*ignore any route is deferral,check for grace*/
        if(p_route->deferral)
        {
            continue;
        } 
        if (bgp4_route_ECMP_priority_cmp(p_route, p_best) == 0)
        {
            p_best = p_route;
            p_vector_out->p_route[p_vector_out->count ++]= p_best;
        }
        else if(bgp4_route_ECMP_priority_cmp(p_route, p_best) > 0)
        {
            for(j = 0;j < p_vector_out->count;j++)
            {
                p_vector_out->p_route[j] = NULL;
                p_vector_out->count = 0;
            }
            p_best = p_route;
            p_vector_out->p_route[p_vector_out->count ++]= p_best;
        }
    }
    
    return ;

}

/*get best route from route vector*/
tBGP4_ROUTE *
bgp4_best_route_vector(tBGP4_ROUTE_VECTOR *p_vector)
{
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_ROUTE *p_route = NULL;
    u_int i = 0 ;

    for (i = 0 ; i < p_vector->count ; i++)
    {
        p_route = p_vector->p_route[i];
        if (p_route == NULL)
        {
            continue ;
        }

        /*ignore any route will be deleted*/
        if (p_route->is_deleted)
        {
            continue ;
        }
        /*ignore any route is deferral,check for grace*/
        if(p_route->deferral)
        {
            continue;
        }
        if (bgp4_route_priority_cmp(p_route, p_best) > 0)
        {
            p_best = p_route;
        }
    }
    return p_best;
}

/*get special route in vector*/
tBGP4_ROUTE *
bgp4_special_route_vector(
                tBGP4_ROUTE_VECTOR *p_vector,
                tBGP4_ROUTE *p_expect)
{
    tBGP4_ROUTE *p_route = NULL;
    u_int i = 0 ;

    for (i = 0 ; i < p_vector->count ; i++)
    {
        p_route = p_vector->p_route[i];
        if (p_route == NULL)
        {
            continue ;
        }
        if (rib_cmp(p_route, p_expect) == 0)
        {
            return p_route;
        }
    }
    return NULL;
}
/*scan for route*/
/*first*/
tBGP4_ROUTE *bgp4_rib_first(tBGP4_RIB *p_root)
{
    return (tBGP4_ROUTE *)avl_first(p_root) ;
}

/*caller must ensure input route is in rib*/
tBGP4_ROUTE *bgp4_rib_next(tBGP4_RIB *p_root, tBGP4_ROUTE *p_route)
{
    if (p_route == NULL)
    {
        return NULL ;
    }
    return (tBGP4_ROUTE *)AVL_NEXT(p_root, p_route);
}

/*get routes matched to summary dest*/
void bgp4_rib_get_by_range(tBGP4_RIB *p_root, tBGP4_ADDR *p_dest, tBGP4_LIST *p_list)
{
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_next = NULL;
    /*scan for all rib nodes,may changed to just contain summary*/
    RIB_LOOP(p_root, p_route, p_next)
    {
        /*summary route can not be aggregated*/
        if(p_route->is_summary)
        {
            continue;
        }
        /*delete route can not be aggregated*/
        if(p_route->ip_action==BGP4_IP_ACTION_DELETE)
        {
            continue;
        }

        if(p_route->dest.afi!=p_dest->afi)
        {
            continue;
        }

        if(p_dest==&(p_route->dest))
        {
            continue;
        }

        if (bgp4_prefixmatch(
            (u_char *)&p_route->dest.ip,
            (u_char *)&p_dest->ip, p_dest->prefixlen))
        {
            bgp4_rtlist_add(p_list, p_route);
        }
    }
    return ;
}


#endif
