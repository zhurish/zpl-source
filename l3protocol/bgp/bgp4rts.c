
#include "bgp4rts.h"
#ifdef NEW_BGP_WANTED
#include "bgp4com.h"

extern struct pqueue *g_pstBgpDelLabelQueue;

u_int bgp4_route_system_update_enable(tBGP4_ROUTE *p_route);

/*prepare send init update to peer for special address-family*/
void
bgp4_schedule_init_update(
    u_int af, 
    tBGP4_PEER *p_peer)
{
    tBGP4_ROUTE_VECTOR vector;/*contain routes with same dest*/
    tBGP4_ROUTE *p_route = NULL; 
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_VPN_INSTANCE *p_instance = p_peer->p_instance;
    tBGP4_RIB *p_rib = p_instance->rib[af];
    u_int update_count = 0;
    u_int afi = 0;
    u_int safi = 0;    
    u_int permit = 0;
    u_char addrstr[64] = {0};

    if (p_rib == NULL)
    {
        return;
    }
    
    /*peer must support this af*/
    if (!bgp4_af_support(p_peer, af))
    {
        return;
    }
    afi = bgp4_index_to_afi(af);
    safi = bgp4_index_to_safi(af);
    
    bgp4_log(BGP_DEBUG_CMN, "bgp schedule init update to %s,af %d/%d",
                bgp4_printf_peer(p_peer,addrstr), afi, safi);

    memset(&vector, 0, sizeof(vector));

    /*scan all routes*/
    bgp4_avl_for_each(&p_rib->rib_table, p_route)
    {
        /*af must macth*/
        if (af != p_route->dest.afi)
        {
            continue;
        }
        /*ignore inactive summary route*/
        if ((p_route->active != TRUE)
            && (p_route->summary_route == TRUE))
        {
            continue;
        }

        /*ignore route to be deleted*/
        if (p_route->is_deleted == TRUE)
        {
            continue;
        }
        
        /*if vector is not empty,and dest is not same,process this vector now*/
        if (vector.count && bgp4_prefixcmp(&vector.p_route[0]->dest, &p_route->dest))
        {
            p_best = bgp4_rib_vector_best(&vector);
            if (p_best != NULL)
            {
                /*ORF checking,if end of rib not send now,reject route 
                  filtered*/
                permit = bgp4_route_peer_orf_permitted(p_peer, &p_peer->orf_in_table, p_best);
                if (permit == TRUE)
                {
                    /*set update flag,do not check aggregate here*/
                    bgp4_feasible_flood_set(p_best, p_peer);
                    update_count++;
                }
                else if (flag_isset(p_peer->txd_rib_end, af))
                {
                    /*send withdraw for route previously advtertised*/
                    permit = bgp4_route_peer_orf_permitted(p_peer, &p_peer->orf_old_in_table, p_best);
                    if (permit == TRUE)
                    {
                        bgp4_withdraw_flood_set(p_best, p_peer);
                        update_count++;
                    }
                }
            }
            vector.count = 0;
        }
        /*add route to this vector:one vector contain route with same dest*/
        vector.p_route[vector.count++] = p_route;
    }
    /*last route vector,which has not been update in the rib-loop*/
    p_best = bgp4_rib_vector_best(&vector);
    if (p_best != NULL)
    {
         /*ORF checking,if end of rib not send now,reject route 
           filtered*/
         permit = bgp4_route_peer_orf_permitted(p_peer, &p_peer->orf_in_table, p_best);
         if (permit == TRUE)
         {
             /*set update flag,do not check aggregate here*/
             bgp4_feasible_flood_set(p_best, p_peer);
             update_count++;
         }
         else if (flag_isset(p_peer->txd_rib_end, af))
         {
             /*send withdraw for route previously advtertised*/
             permit = bgp4_route_peer_orf_permitted(p_peer, &p_peer->orf_old_in_table, p_best);
             if (permit == TRUE)
             {
                 bgp4_withdraw_flood_set(p_best, p_peer);
                 update_count++;
             }
         }
    }

    /*start adjout timerusing short value*/
    bgp4_timer_start_ms(&p_peer->adj_out_timer, 100);

    /*if no route been scheduled,check if EOB should been sent*/
    if ((update_count == 0) && (p_peer->state == BGP4_NS_ESTABLISH))
    {
        /*grace check*/
        if ((gbgp4.restart_enable == TRUE)
            && (p_peer->restart_enable == TRUE))
        {
            /*if local restatred,we must wait for endofrib before sending it*/
            if (p_peer->restart_role == BGP4_GR_ROLE_RESTART)
            {
                if (!flag_isset(p_peer->rxd_rib_end, p_rib->af))
                {
                    return;
                }
            }
            /*send end-of-rib at last,if it has not been sent before*/
            if (!flag_isset(p_peer->txd_rib_end, af))
            {
                bgp4_end_of_rib_output(p_peer, af);
            }
        }
        else/*normal*/
        {
            /*send end-of-rib at last,if it has not been sent before*/
            if (!flag_isset(p_peer->txd_rib_end, af))
            {
                bgp4_end_of_rib_output(p_peer, af);
            }
        }
    }
    return;
}

/*decide if there is any route is learnt from special peer*/
u_int 
bgp4_peer_route_exist(tBGP4_PEER *p_peer)
{
    tBGP4_PATH *p_path = NULL;
    tBGP4_VPN_INSTANCE *p_instance = p_peer->p_instance;
    u_int af = 0;

    for (af = 0 ; af < BGP4_PF_MAX ; af++)
    {
        if (p_instance->rib[af] == NULL)
        {
            continue;
        }
        bgp4_avl_for_each(&p_instance->rib[af]->path_table, p_path)
        {
            if ((p_path->p_peer == p_peer) 
                && bgp4_avl_count(&p_path->route_list))
            {
                /*only one route need*/
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*clear all stale route learnt from peer*/
void
bgp4_peer_stale_route_clear(
        u_int af, 
        tBGP4_PEER *p_peer)
{
    tBGP4_VPN_INSTANCE *p_instance = p_peer->p_instance;
    tBGP4_RIB *p_rib = p_instance->rib[af];
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE dummy;
    tBGP4_ROUTE *p_nroute = NULL;
    if (p_rib == NULL)
    {
        return;
    }

    /*scan for all rib*/
    bgp4_avl_for_each_safe(&p_rib->rib_table, p_route, p_nroute)
    {
        /*ignore route from other peer*/
        if (p_route->p_path->p_peer != p_peer)
        {
            continue;
        }
        /*ignore non stale route*/
        if (p_route->stale != TRUE)
        {
            continue;
        }
        /*ignore route already be cleared*/
        if (p_route->is_deleted == TRUE)
        {
            continue;
        }
        p_route->stale = FALSE;
        memcpy(&dummy, p_route, sizeof(dummy));
        dummy.is_deleted = TRUE;
        dummy.stale = FALSE;

        /*delete exported vrf route*/
        bgp4_vrf_route_export_check(&dummy, FALSE);

        /*add this route to rib_in_table,wait for deleting*/
        bgp4_rib_in_table_update(&dummy);
    }
    return;
}
/*end connection for neighbor.delete route learnt from peer*/
void 
bgp4_peer_route_clear(
        u_int af, 
        tBGP4_PEER *p_peer)
{
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_nroute = NULL;
    tBGP4_ROUTE dummy;
    tBGP4_VPN_INSTANCE *p_instance = p_peer->p_instance;
    tBGP4_RIB *p_rib = p_instance->rib[af];
    if (p_rib == NULL)
    {
        return;
    }

    /*delete all routes in rib_in table*/
    bgp4_avl_for_each_safe(&p_rib->rib_in_table, p_route, p_nroute)
    {
        if (p_route->p_path->p_peer == p_peer)
        {
            bgp4_route_table_delete(p_route);
        }
    }
    bgp4_avl_for_each_safe(&p_rib->rib_table, p_route, p_nroute)
    {
        /*ignore route from other peer*/
        if (p_route->p_path->p_peer != p_peer)
        {
            continue;
        }
        if (p_route->is_deleted == TRUE)
        {
            gbgp4.stat.duplicate_delete++;
            continue;
        }

        memcpy(&dummy, p_route, sizeof(dummy));
        dummy.is_deleted = TRUE;

        /*remove route from other vrf*/
        bgp4_vrf_route_export_check(&dummy, FALSE);
        
        /*add this route to rib_in_table,wait for deleting*/
        bgp4_rib_in_table_update(&dummy);
    }
    return;
}

/*decide if send update for route to peer*/
int  
bgp4_update_output_verify(
          tBGP4_PEER *p_peer,
          tBGP4_ROUTE *p_route)
{
    tBGP4_PATH path;
    u_int type = bgp4_peer_type(p_peer);
    u_char route[64]= {0};
    u_char peer[64]= {0};
    u_char route_str[64] = {0};
    u_char next_hop_str[64] = {0};
    
    /*bgp4_log(BGP_DEBUG_UPDATE,"check if need update %s to %s",
        bgp4_printf_route(p_route,route),
        bgp4_printf_peer(p_peer,peer));*/

    if (p_route->dest.afi == BGP4_PF_IPUCAST)
    {   
        #if 0
        if (bgp4_prefixmatch(p_route->dest.ip,
              p_route->p_path->nexthop.ip, p_route->dest.prefixlen))
        {
            bgp4_log(BGP_DEBUG_RT,"route %s next hop %s is not in the same network",
                    bgp4_printf_route(p_route,route_str),
                    bgp4_printf_addr(&p_route->p_path->nexthop,next_hop_str));

            return FALSE;
        }
        #endif
    }
    else if (p_route->dest.afi == BGP4_PF_IP6UCAST)
    {
        if (bgp4_prefixmatch(p_route->dest.ip, 
               p_route->p_path->nexthop.ip, 
               p_route->dest.prefixlen))
        {
            bgp4_log(BGP_DEBUG_RT, "In the same network route and  next hop can't identical,the route %s   nexthop %s ",
                    bgp4_printf_route(p_route, route_str),
                    bgp4_printf_addr(&p_route->p_path->nexthop, next_hop_str));
            
            return FALSE;
        }
    }

    if (is_bgp_route(p_route))
    {
        if (p_route->p_path->p_peer == p_peer)
        {
            bgp4_log(BGP_DEBUG_UPDATE,"route %s learnt from %s,doesn't need update",
                bgp4_printf_route(p_route,route),
                bgp4_printf_peer(p_peer,peer) );
           
            return FALSE;
        }
        /*If the peer's AS or AS in route's as-path list,release it*/
        if (type == BGP4_EBGP)
        {
            if (bgp4_as_exist_in_aspath(&p_route->p_path->aspath_list, gbgp4.as) == TRUE)
            {
                bgp4_log(BGP_DEBUG_UPDATE,"route %s as path has loop,doesn't need update",
                    bgp4_printf_route(p_route,route));
                
                return FALSE;
            }
            if ((p_peer->as_substitute_enable == FALSE)
                && (bgp4_as_exist_in_aspath(&p_route->p_path->aspath_list, p_peer->as) == TRUE))
            {
                bgp4_log(BGP_DEBUG_UPDATE,"route %s as path has peer as,doesn't need update",
                    bgp4_printf_route(p_route,route));
                return FALSE;
            }
        }
        /*check community flag*/

        /*no export,can not export to pure external peer*/        
        if (p_route->p_path->community_no_export && (type == BGP4_EBGP))
        {
            bgp4_log(BGP_DEBUG_UPDATE,"route %s has no export community,doesn't need update",
                bgp4_printf_route(p_route,route));
            return FALSE;
        }

        /*no adv,do not send to any peer*/
        if (p_route->p_path->community_no_adv)
        {
            bgp4_log(BGP_DEBUG_UPDATE,"route %s has no adv community,doesn't need update",
                bgp4_printf_route(p_route,route));
            return FALSE;
        }
        /*no sub confed,only send to internal peer*/
        if (p_route->p_path->community_no_subconfed && (type != BGP4_IBGP))
        {
            bgp4_log(BGP_DEBUG_UPDATE,"route %s has no subconfed community,doesn't need update",
                bgp4_printf_route(p_route,route));

            return FALSE;
        }

        /* Route learned from the Internal BGP peer should not be
        * advertised to another internal BGP peer.
        */
        if (p_route->p_path->p_peer
            && (bgp4_peer_type(p_route->p_path->p_peer) == BGP4_IBGP)
            && (bgp4_peer_type(p_peer) == BGP4_IBGP))
        {
            /*route reflector function*/
            if (gbgp4.reflector_enable != TRUE)
            {
                bgp4_log(BGP_DEBUG_UPDATE,"%s doesn't need update for internal peer %s",
                    bgp4_printf_route(p_route,route),
                    bgp4_printf_peer(p_peer,peer));
                
                return FALSE;
            }

            if (bgp4_cluster_loop_check(p_route->p_path) != VOS_OK)
            {
                bgp4_log(BGP_DEBUG_UPDATE,"route %s cluster has loop,doesn't need update",
                    bgp4_printf_route(p_route,route));
                return FALSE;
            }

            if (bgp4_originid_loop_check(p_route->p_path) != VOS_OK)
            {
                bgp4_log(BGP_DEBUG_UPDATE,"route %s originator has loop,doesn't need update",
                    bgp4_printf_route(p_route,route));
                return FALSE;
            }
            if (p_route->p_path->p_peer != NULL)
            {
                /*none client route only reflect to client peer*/
                if (p_route->p_path->p_peer->is_reflector_client != TRUE)
                {                    
                    bgp4_log(BGP_DEBUG_UPDATE,"%s doesn't need update for internal peer %s",
                        bgp4_printf_route(p_route,route),
                        bgp4_printf_peer(p_peer,peer));
                    return p_peer->is_reflector_client;
                }
                else
                {
                    return TRUE;
                }
            }
        }
    }
    else
    {
        if (p_route->dest.afi == BGP4_PF_IPUCAST)
        {
            if (bgp_ip4(p_route->p_path->nexthop.ip)
                == bgp_ip4(p_peer->ip.ip))
            {
                bgp4_log(BGP_DEBUG_UPDATE,"route %s nexthop is peer,doesn't need update",
                    bgp4_printf_route(p_route,route));
                return FALSE;
            }
        }
        else if (p_route->dest.afi == BGP4_PF_IP6UCAST)
        {
             /*TODO*/
        }
    }
#if 0
    /*aggr check*/
    if ((bgp4_peer_type(p_peer) == BGP4_IBGP)&& p_route->summary_route)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "do not send aggregate route to IBGP nbr");
        return FALSE;
    }/*do not send feasible for route is filtered by range,do not prevent withdraw*/
    else
#endif        
    if (p_route->summary_filtered 
        && bgp4_feasible_flood_isset(p_route, p_peer))
    {
        bgp4_log(BGP_DEBUG_UPDATE,"route %s is aggregated,doesn't need update",
                    bgp4_printf_route(p_route,route));
        return FALSE;
    }

    /*add 6pe label*/
    if ((p_peer->send_label == FALSE)
        && (p_route->dest.afi == BGP4_PF_IP6LABEL))
    {
        p_route->in_label = 0;/*if not enable label send,clear route label*/
    }

    /*check policy for output*/
    memset(&path, 0, sizeof(path));
    if (bgp4_export_route_policy_apply(p_route, p_peer, &path) == BGP4_ROUTE_DENY)
    {
        bgp4_log(BGP_DEBUG_UPDATE,"route %s is filtered,doesn't need update",
                    bgp4_printf_route(p_route,route));

        bgp4_path_clear(&path); 
        gbgp4.stat.export_filtered_route++;
        return FALSE;
    }

    bgp4_path_clear(&path);

    /*do not forward vpn route to UPE peer*/
    if ((p_route->dest.afi == BGP4_PF_IPVPN)
        && (p_route->p_path->p_peer)
        && (p_peer->upe_enable == TRUE))
    {
        return FALSE;
    }
    /*summary route can only be send to upe peer*/
    if ((p_route->dest.afi == BGP4_PF_IPVPN)
        && (p_route->proto == M2_ipRouteProto_icmp)
        && (p_peer->upe_enable == FALSE))
    {
        return FALSE;
    }
    return TRUE ;
}

tBGP4_LINK *
bgp4_rtlist_add(
        avl_tree_t *p_list,
        tBGP4_ROUTE *p_route)
{
    tBGP4_LINK *p_link = NULL;

    p_link = bgp4_malloc(sizeof(tBGP4_LINK),MEM_BGP_LINK);
    if (p_link == NULL)
    {
        return NULL;
    }

    p_link->p_route = p_route; 
    bgp4_avl_add(p_list, p_link);
    return p_link;
}

void 
bgp4_rtlist_clear (avl_tree_t *p_list)
{
    tBGP4_LINK *p_link = NULL;
    tBGP4_LINK *p_next = NULL;

    bgp4_avl_for_each_safe(p_list, p_link, p_next)
    {
        bgp4_avl_delete(p_list, p_link);
        bgp4_free(p_link, MEM_BGP_LINK);
    }
    return;
}

/*decrease reference of route,if no reference need, delete it
   before calling this function and delete, route is not in rib tree
 */
void 
bgp4_route_release(tBGP4_ROUTE *p_route)
{
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_RIB *p_rib = p_instance->rib[p_route->dest.afi];
    u_int *puiLabel = NULL;
    
    if (p_rib == NULL)
    {
        return;
    }

    if(p_route->in_label && g_pstBgpDelLabelQueue != NULL)
    {
        puiLabel = XMALLOC(MTYPE_MSGQUEUE,sizeof(u_int));
        if(puiLabel != NULL)
        {
            *puiLabel = p_route->in_label;
            if(VOS_OK != pqueue_enqueue(puiLabel, g_pstBgpDelLabelQueue))
            {
                XFREE(MTYPE_MSGQUEUE,puiLabel);
                puiLabel = NULL;
            }
        }
    }
    /*remove from path*/
    if (p_route->p_path)
    {
        bgp4_route_flood_clear(p_route);
        p_instance = p_route->p_path->p_instance;
        bgp4_avl_delete(&p_route->p_path->route_list, p_route);
        p_route->p_path = NULL;
    }
    if (p_rib->p_next_walk == p_route)
    {
        p_rib->p_next_walk = bgp4_avl_first(&p_rib->rib_table);
    }
    if (p_rib->p_change_route == p_route)
    {
        p_rib->p_change_route = bgp4_avl_first(&p_rib->rib_table);
    }
    bgp4_free(p_route, MEM_BGP_ROUTE);
    return;
}

u_int
bgp4_system_update_finished(tBGP4_ROUTE *p_route)
{
    u_int vrf_id = p_route->p_path->p_instance->vrf;
    u_int i = 0;
    
    if (p_route->system_selected == TRUE)
    {
        /*if old nexthop exist,some update not finished*/
        if (p_route->p_path->p_old_nexthop
            && p_route->p_path->p_old_nexthop->count)
        {
            return FALSE;
        }

        /*VPLS has no ECMP*/
        if (p_route->dest.afi == BGP4_PF_L2VPLS)
        {
            if ((p_route->in_kernalx == 0) || (p_route->in_hardwarex == 0))
            {
                return FALSE;
            }
        }
        else
        {
            /*check if all nexthop update finished*/
            if (p_route->p_path->p_direct_nexthop)
            {
                for (i = 0 ; i < p_route->p_path->p_direct_nexthop->count; i++)
                {
                    if (!BIT_LST_TST(&p_route->in_kernalx, i))
                    {
                        return FALSE;
                    }
                    if (!BIT_LST_TST(&p_route->in_hardwarex, i))
                    {
                        return FALSE;
                    }
                }
            }
        }
        /*vpn case,check if mpls entry update finished,no ECMP considered*/
        if ((vrf_id != p_route->p_path->origin_vrf) && p_route->out_label)
        {
            if (p_route->in_mpls_hardware == FALSE)
            {
                return FALSE;
            }
        }
    }
    else
    {
        /*VPLS has no ECMP*/
        if (p_route->dest.afi == BGP4_PF_L2VPLS)
        {
            if ((p_route->in_kernalx != 0) || (p_route->in_hardwarex != 0))
            {
                return FALSE;
            }
        }
        else
        {
            if (p_route->p_path->p_direct_nexthop)
            {
                /*check if all nexthop update finished*/
                for (i = 0 ; i < p_route->p_path->p_direct_nexthop->count; i++)
                {
                    if (BIT_LST_TST(&p_route->in_kernalx, i))
                    {
                        return FALSE;
                    }
                    if (BIT_LST_TST(&p_route->in_hardwarex, i))
                    {
                        return FALSE;
                    }
                }
            }
        }
        /*vpn case,check if mpls entry update finished,no ECMP considered*/
        if ((vrf_id != p_route->p_path->origin_vrf) && p_route->out_label)
        {
            if (p_route->in_mpls_hardware == TRUE)
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

/*update a ipucast route to system*/
int 
bgp4_ipv4_sys_route_update(tBGP4_ROUTE *p_route)
{
    tBGP4_RIB *p_rib = p_route->p_path->p_instance->rib[p_route->dest.afi];
    u_int vrf_id = p_route->p_path->p_instance->vrf;
    u_int msgneed = TRUE;
    u_int i = 0;
    if (p_rib == NULL)
    {
        return VOS_OK;
    }
    
    if (p_route->p_path->p_direct_nexthop == NULL)
    {
        return VOS_OK;
    }
    
    if (p_rib->p_hwmsg == NULL)
    {
        p_rib->p_hwmsg = bgp4_malloc(1200, MEM_BGP_BUF);
        if (p_rib->p_hwmsg == NULL)
        {                
            return VOS_ERR;
        }
    }
    
    if (p_route->system_selected == TRUE)
    {                
        for (i = 0; i < p_route->p_path->p_direct_nexthop->count ; i++)
        {
            msgneed = TRUE;
            if (!BIT_LST_TST(&p_route->in_kernalx, i))
            {
                if (bgp4_sys_iproute_add(vrf_id, p_route, i, &msgneed) == VOS_ERR)
                {
                    return VOS_ERR;
                }
                BIT_LST_SET(&p_route->in_kernalx, i);
            }
            if (msgneed == FALSE)
            {
                BIT_LST_SET(&p_route->in_hardwarex, i);
            }
            if (!BIT_LST_TST(&p_route->in_hardwarex, i))
            {
                /*insert into message failed.stop processing*/
                if (bgp4_sys_msg_add(vrf_id, p_rib->p_hwmsg, 
                      p_route, i) == VOS_ERR)
                {
                    return VOS_ERR;
                }
                /*send msg finished*/
                BIT_LST_SET(&p_route->in_hardwarex, i);
            }
        }
    }
    else
    {
        for (i = 0; i < p_route->p_path->p_direct_nexthop->count ; i++)
        {
            msgneed = TRUE;
            if (BIT_LST_TST(&p_route->in_kernalx, i))
            {
                if (bgp4_sys_iproute_delete(vrf_id, p_route, i, &msgneed) == VOS_ERR)
                {
                    return VOS_ERR;
                }
                BIT_LST_SET(&p_route->in_kernalx, i);
            }
            if (msgneed == FALSE)
            {
                BIT_LST_SET(&p_route->in_hardwarex, i);
            }
            if (BIT_LST_TST(&p_route->in_hardwarex, i))
            {
                /*insert into message failed.stop processing*/
                if (bgp4_sys_msg_add(vrf_id, p_rib->p_hwmsg, 
                      p_route, i) == VOS_ERR)
                {
                    return VOS_ERR;
                }
                /*send msg finished*/
                BIT_LST_SET(&p_route->in_hardwarex, i);
            }
        }
    }
    return VOS_OK;
}

int 
bgp4_ipv6_sys_route_update(tBGP4_ROUTE *p_route)
{
    tBGP4_RIB *p_rib = p_route->p_path->p_instance->rib[p_route->dest.afi];
    u_int vrf_id = p_route->p_path->p_instance->vrf;
    u_int msgneed = TRUE;
    u_int i = 0;

    if (p_rib == NULL)
    {
        return VOS_OK;
    }
    
    if (p_route->p_path->p_direct_nexthop == NULL)
    {
        return VOS_OK;
    }

    /*insert into ipv4 or ipv6 msg buffer*/
    if (p_rib->p_hwmsg == NULL)
    {
        p_rib->p_hwmsg = bgp4_malloc(1200, MEM_BGP_BUF);
        if (p_rib->p_hwmsg == NULL)
        {                
            return VOS_ERR;
        }
    }
    
    if (p_route->system_selected == TRUE)
    {
        for (i = 0; i < p_route->p_path->p_direct_nexthop->count ; i++)
        {
            msgneed = TRUE;
            if (!BIT_LST_TST(&p_route->in_kernalx, i))
            {
                if (bgp4_sys_ip6route_add(vrf_id, p_route, i, &msgneed) == VOS_ERR)
                {
                    return VOS_ERR;
                }
                BIT_LST_SET(&p_route->in_kernalx, i);
            }
            if (msgneed == FALSE)
            {
                BIT_LST_SET(&p_route->in_hardwarex, i);
            }
            if (!BIT_LST_TST(&p_route->in_hardwarex, i))
            {
                /*insert into message failed.stop processing*/
                if (bgp4_sys_msg_add(vrf_id, p_rib->p_hwmsg, 
                      p_route, i) == VOS_ERR)
                {
                    return VOS_ERR;
                }
                /*send msg finished*/
                BIT_LST_SET(&p_route->in_hardwarex, i);
            }
        }
    }
    else
    {
        for (i = 0; i < p_route->p_path->p_direct_nexthop->count ; i++)
        {
            msgneed = TRUE;
            if (BIT_LST_TST(&p_route->in_kernalx, i))
            {
                if (bgp4_sys_ip6route_delete(vrf_id, p_route, i, &msgneed) == VOS_ERR)
                {
                    return VOS_ERR;
                }
                BIT_LST_SET(&p_route->in_kernalx, i);
            }
            if (msgneed == FALSE)
            {
                BIT_LST_SET(&p_route->in_hardwarex, i);
            }
            if (BIT_LST_TST(&p_route->in_hardwarex, i))
            {
                /*insert into message failed.stop processing*/
                if (bgp4_sys_msg_add(vrf_id, p_rib->p_hwmsg, 
                      p_route, i) == VOS_ERR)
                {
                    return VOS_ERR;
                }
                /*send msg finished*/
                BIT_LST_SET(&p_route->in_hardwarex, i);
            }
        }
    }
    return VOS_OK;
}

int 
bgp4_vpls_sys_route_update(tBGP4_ROUTE *p_route)
{
#ifdef BGP_VPLS_WANTED
    if (p_route->system_selected == TRUE)
    {
        if (p_route->in_kernalx == FALSE)
        {
            if (bgp4_sys_vpls_xc_add(p_route) == VOS_ERR)
            {
                return VOS_ERR;
            }
            p_route->in_kernalx = TRUE;
        }
        if (p_route->in_hardwarex == FALSE)
        {
            /*insert into message failed.stop processing*/
            if (bgp4_sys_vpls_msg_send(p_route, TRUE) == VOS_ERR)
            {
                return VOS_ERR;
            }
            /*send msg finished*/
            p_route->in_hardwarex = TRUE;
        }
    }
    else
    {
        if (p_route->in_kernalx == TRUE)
        {
            if (bgp4_sys_vpls_xc_delete(p_route) == VOS_ERR)
            {
                return VOS_ERR;
            }
            p_route->in_kernalx = FALSE;
        }
        if (p_route->in_hardwarex == TRUE)
        {               
            /*insert into message failed.stop processing*/
            if (bgp4_sys_vpls_msg_send(p_route, FALSE) == VOS_ERR)
            {
                return VOS_ERR;
            }
            /*send msg finished*/
            p_route->in_hardwarex = FALSE;
        }
    }
#endif    
    return VOS_OK;
}

#define bgp4_vpnv6_sys_route_update(x) bgp4_vpn_sys_route_update(x)
#define bgp4_6pe_sys_route_update(x) bgp4_vpn_sys_route_update(x)


int 
bgp4_vpn_sys_route_update(tBGP4_ROUTE *p_route)
{
    STATUS rc = 0;
    
    /*update ip kernal*/
    if (p_route->dest.afi == BGP4_PF_IPUCAST)
    {
        rc = bgp4_ipv4_sys_route_update(p_route);
    }
    else
    {
        rc = bgp4_ipv6_sys_route_update(p_route);
    }

    if (rc != VOS_OK)
    {
        return VOS_ERR;
    }
    //printf("is here p_route->system_selected %d p_route->in_mpls_hardware %d\n",p_route->system_selected,p_route->in_mpls_hardware);
    if (p_route->system_selected == TRUE)
    {
        if (p_route->in_mpls_hardware == FALSE)
        {
            /*notify failed,stop processing*/
            if (bgp4_sys_mpls_route_update(p_route) == VOS_ERR)
            {
                return VOS_ERR;
            }
            p_route->in_mpls_hardware = TRUE;
        }
    }
    else
    {
        if (p_route->in_mpls_hardware == TRUE)
        {
            /*notify failed,stop processing*/
            if (bgp4_sys_mpls_route_update(p_route) == VOS_ERR)
            {
                return VOS_ERR;
            }
            p_route->in_mpls_hardware = FALSE;
        }
    }
    return VOS_OK;
}

int
bgp4_ipmcast_sys_route_update(tBGP4_ROUTE *p_route)
{
    return VOS_OK;/*TO be implemented*/
}

/*check a route's ip update request*/
int 
bgp4_route_system_update(tBGP4_ROUTE *p_route)
{
    u_int vrf_id = p_route->p_path->p_instance->vrf;
    u_char addr[64];
    u_char nhop[64];
    u_char dnhop[64];
    
    /*if nothing to update,return directly*/
    if (bgp4_system_update_finished(p_route))
    {
        return VOS_OK;
    }
    bgp4_log(BGP_DEBUG_RT, "%s vrf %d system route %s,nhop %s/%s",
        (p_route->system_selected == TRUE) ? "add" : "delete", vrf_id,
        bgp4_printf_route(p_route,addr),
        bgp4_printf_addr(&p_route->p_path->nexthop, nhop),
        bgp4_printf_addr(&p_route->p_path->p_direct_nexthop->ip[0], dnhop));

    switch (p_route->dest.afi) {
        case BGP4_PF_IPUCAST:
             /*if it is normal ip route,update ip system route
               route is learnt from same instance,or has no label
              */
             if ((vrf_id == p_route->p_path->origin_vrf) 
                || (p_route->out_label == 0))
             {
                 return bgp4_ipv4_sys_route_update(p_route);
             }
             else/*it is vpn route with label*/
             {
                 return bgp4_vpn_sys_route_update(p_route);
             }
             break;

        case BGP4_PF_IP6UCAST:
             /*if route's orign vrf and current vrf are same,
              and out lable is 0,it is normal ipv6 ucast route*/
             if ((vrf_id == p_route->p_path->origin_vrf) 
                && (p_route->out_label == 0))
             {
                 return bgp4_ipv6_sys_route_update(p_route);
             }/*same vrf ,and label exist,it is 6pe route with label*/
             else if ((vrf_id == p_route->p_path->origin_vrf) && (p_route->out_label != 0))
             {
                 return bgp4_6pe_sys_route_update(p_route);
             }
             else/*vpn6 route with label*/
             {
                 return bgp4_vpnv6_sys_route_update(p_route);
             }
             break;
             
#ifdef BGP_VPLS_WANTED 
        case BGP4_PF_L2VPLS:
             return bgp4_vpls_sys_route_update(p_route);
             break;
#endif
        case BGP4_PF_IPMCAST:
             return bgp4_ipmcast_sys_route_update(p_route);
             break;
             
        default:
             break;
    }
    /*operation ok*/
    return VOS_OK;
}

/*check all route's ip update request*/
int 
bgp4_route_system_update_check_all(tBGP4_RIB *p_rib)
{
    tBGP4_ROUTE *p_route = NULL;
    u_int start = vos_get_system_tick();
    u_int now = 0;
    /*start from route to be update*/
    for (p_route = p_rib->p_next_walk; 
         p_route ; 
         p_route = bgp4_avl_next(&p_rib->rib_table, p_route))
    {
        gbgp4.stat.system_loop++;
        /*try to update ip table*/
        if (bgp4_system_update_finished(p_route))
        {
            continue;
        }

        if (bgp4_route_system_update(p_route) != VOS_OK)
        {
            /*operation failed,if no wait need,just stop processing*/
            p_rib->system_check_need = TRUE;
  
            /*ip update failed,restart from it*/
            p_rib->p_next_walk = p_route;
  
            bgp4_log(BGP_DEBUG_RT, "system update failed,schedule next update");
  
            return VOS_ERR;
        }
        /*if do not wait, we will stop if time exceed limit:900ticks*/
        now = vos_get_system_tick();
        if ((now < start) || (now > (start + 900)))
        {
            p_rib->system_check_need = TRUE;
  
            /*ip update not finished,start from next one*/
            p_rib->p_next_walk = bgp4_avl_next(&p_rib->rib_table, p_route);
            return VOS_ERR;
        }
    }
    /*all ip process finished,clear update route*/
    p_rib->p_next_walk = NULL;
    return VOS_OK;
}

void 
bgp4_unused_route_clear(tBGP4_RIB *p_rib)
{
    tBGP4_VPN_INSTANCE *p_instance = p_rib->p_instance;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_next = NULL;
    u_int rest = 0;
    u_char rt_str[64];
    
    /*delete unused routes from rib*/
    bgp4_avl_for_each_safe(&p_rib->rib_table, p_route, p_next)
    {       
        if (p_route->is_deleted == FALSE)
        {
            continue;
        }
        
        if ((bgp4_system_update_finished(p_route) == TRUE)
            && (bgp4_route_flood_is_pending(p_route) == FALSE))
        {
            bgp4_log(BGP_DEBUG_RT, "delete unused route %s from instance %d",
                        bgp4_printf_route(p_route,rt_str),
                        p_instance->vrf);

            bgp4_route_table_delete(p_route);
        }
        else
        {
            rest = 1;
        }
    }
    if (rest)
    {
       bgp4_timer_start(&p_rib->route_clear_timer, BGP4_ROUTE_CLEAR_INTERVAL);
    }
    return;
}

/*wait:TRUE--wait for all operations finished,FALSE--do not wait*/
void
bgp4_rib_system_update_check(tBGP4_RIB *p_rib)
{
    /*if walkup not requested,do nothing*/
    if (p_rib->system_check_need != TRUE)
    {
        /*try to send route msg buffer*/
        if (p_rib->p_hwmsg)
        {
            if (bgp4_sys_msg_send(p_rib->p_hwmsg) == VOS_OK)
            {
                bgp4_free(p_rib->p_hwmsg, MEM_BGP_BUF);
                p_rib->p_hwmsg = NULL;
            }
            else
            {
                /*start timer*/
                bgp4_timer_start_ms(&p_rib->system_timer, 100);
            }
        }
        return;
    }

    /*clear request flag*/
    p_rib->system_check_need = FALSE;
    if (bgp4_route_system_update_check_all(p_rib) != VOS_OK)
    {
        /*start timer*/
        bgp4_timer_start_ms(&p_rib->system_timer, 50);
        return;
    }

    /*start timer for last route msg*/
    bgp4_timer_start_ms(&p_rib->system_timer, 100);
    
    /*flush any route to be deleted*/
    bgp4_unused_route_clear(p_rib);
    return;
}

void
bgp4_schedule_system_add(tBGP4_ROUTE *p_route)
{
    tBGP4_RIB *p_rib = p_route->p_path->p_instance->rib[p_route->p_path->af];

    if (p_rib == NULL)
    {
        return;
    }

    /*if action same as route's current action,do nothing*/
    if (p_route->system_selected == TRUE)
    {
        return;
    }

    /*ignore route without nexthop or in waiting*/
    if (p_route->igp_sync_wait 
        || p_route->system_no_nexthop
        || p_route->system_filtered)
    {
        return;
    }

    /*ignore local 6pe route,it is translated by other route*/
    if ((p_route->dest.afi == BGP4_PF_IP6LABEL)
        && (p_route->out_label == 0))
    {
        return;
    }

    /*set global event*/
    p_rib->system_check_need = TRUE;
    p_rib->p_next_walk = bgp4_avl_first(&p_rib->rib_table);

    p_route->system_selected = TRUE;

    /*clear all update flag*/
    p_route->in_kernalx = 0;
    p_route->in_hardwarex = 0;
    p_route->in_mpls_hardware = FALSE;

    /*start timer*/
    /*if (!bgp4_timer_is_active(&p_rib->system_timer))*/
    {
        bgp4_timer_start_ms(&p_rib->system_timer, 100);
    }
    return;
}

void
bgp4_schedule_system_delete(tBGP4_ROUTE *p_route)
{
    tBGP4_RIB *p_rib = p_route->p_path->p_instance->rib[p_route->p_path->af];

    if (p_rib == NULL)
    {
        return;
    }
    
    /*if action is none,or same as route's current action,do nothing*/
    if (p_route->system_selected == FALSE)
    {
        return;
    }
    
    /*ignore local 6pe route,it is translated by other route*/
    if ((p_route->dest.afi == BGP4_PF_IP6LABEL)
        && (p_route->out_label == 0))
    {
        return;
    }
    
    p_route->system_selected = FALSE;
    
    /*set global event*/
    p_rib->system_check_need = TRUE;
    p_rib->p_next_walk = bgp4_avl_first(&p_rib->rib_table);

    /*start timer*/
    /*if (!bgp4_timer_is_active(&p_rib->system_timer))*/
    {
        bgp4_timer_start_ms(&p_rib->system_timer, 100);
    }    
    return;
}

/*set a route's withdraw flag to all peer*/
void
bgp4_route_withdraw_flood_set(tBGP4_ROUTE *p_route)
{
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_PEER *p_peer = NULL;

    /*reset current flag*/
    bgp4_route_flood_clear(p_route);

    /*route is filtered by range*/
    if (bgp4_withdraw_flood_check_against_range(p_route) != VOS_OK)
    {
        return;    
    }
    /*scan for all peers*/
    bgp4_peer_for_each(p_instance, p_peer)
    {
        bgp4_withdraw_flood_set(p_route, p_peer);
    }    
    return;
}

/*set a route's feasible flag to all peer*/
void
bgp4_route_feasible_flood_set(tBGP4_ROUTE *p_route)    
{
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_PEER *p_peer = NULL;

    /*reset current flag*/
    bgp4_route_flood_clear(p_route);

    /*check if route is filtered by range*/
    if (bgp4_feasible_flood_check_against_range(p_route) != VOS_OK)
    {
        return;    
    }

    /*scan for all peers*/
    bgp4_peer_for_each(p_instance, p_peer)
    {
        /*ORF checking*/
        if (bgp4_route_peer_orf_permitted(p_peer, &p_peer->orf_in_table, p_route) == TRUE)
        {
            bgp4_feasible_flood_set(p_route, p_peer);
        }
    }    
    return;
}

/*schedule withdraw update to peer*/
void
bgp4_withdraw_flood_set(
        tBGP4_ROUTE *p_route,
        tBGP4_PEER *p_peer)
{
    tBGP4_EXT_FLOOD_FLAG *p_eflag = NULL;
    tBGP4_RIB *p_rib = p_route->p_path->p_instance->rib[p_route->dest.afi];

    if (p_rib == NULL)
    {
        return;
    }

    if ((p_peer->state != BGP4_NS_ESTABLISH)
        || (!bgp4_af_support(p_peer, p_route->dest.afi)))
    {
        return;
    }
    
    if (p_route->p_withdraw == NULL)
    {
        p_route->p_withdraw = bgp4_malloc(sizeof(*p_route->p_withdraw), MEM_BGP_FLOOD);
        if (p_route->p_withdraw == NULL)
        {
            return;
        }
        /*insert to rib's withdraw table*/
        p_route->p_withdraw->p_route = p_route;
        bgp4_avl_add(&p_rib->out_withdraw_table, p_route->p_withdraw);
    }
        /*check base index*/
    if (p_peer->bit_index <= BGP4_MAX_BASE_PEER_ID)
    {
        BIT_LST_SET(p_route->p_withdraw->bits, p_peer->bit_index);
    }
    else if (p_peer->bit_index <= (BGP4_MAX_BASE_PEER_ID + BGP4_EXT_PEER_ID_COUNT))
    {
        /*use the first extend id*/
        p_eflag = p_route->p_withdraw->p_extend[0];
        if (p_eflag == NULL)
        {
            p_eflag = bgp4_malloc(sizeof(tBGP4_EXT_FLOOD_FLAG), MEM_BGP_EXTFLOOD);
            if (p_eflag)
            {
                p_route->p_withdraw->p_extend[0] = p_eflag;
                p_eflag->min_id = BGP4_MAX_BASE_PEER_ID + 1;
                p_eflag->max_id = (BGP4_MAX_BASE_PEER_ID + BGP4_EXT_PEER_ID_COUNT);
            }
        }
        if (p_eflag)
        {
            BIT_LST_SET(p_eflag->bits, p_peer->bit_index - p_eflag->min_id);
        }
    }
    else if (p_peer->bit_index <= (BGP4_MAX_BASE_PEER_ID + BGP4_EXT_PEER_ID_COUNT + BGP4_EXT_PEER_ID_COUNT))
    {
        /*use the second extend id*/
        p_eflag = p_route->p_withdraw->p_extend[1];
        if (p_eflag == NULL)
        {
            p_eflag = bgp4_malloc(sizeof(tBGP4_EXT_FLOOD_FLAG), MEM_BGP_EXTFLOOD);
            if (p_eflag)
            {
                p_route->p_withdraw->p_extend[1] = p_eflag;
                p_eflag->min_id = BGP4_MAX_BASE_PEER_ID + BGP4_EXT_PEER_ID_COUNT + 1;
                p_eflag->max_id = (BGP4_MAX_BASE_PEER_ID + BGP4_EXT_PEER_ID_COUNT + BGP4_EXT_PEER_ID_COUNT);
            }
        }
        if (p_eflag)
        {
            BIT_LST_SET(p_eflag->bits, p_peer->bit_index - p_eflag->min_id);
        }
    }    
    /*start rib sending timer*/
    bgp4_timer_start(&p_peer->adj_out_timer, BGP4_PEER_ADJOUT_INTERVAL);
    return;
}

/*schedule feasible update to peer*/
void
bgp4_feasible_flood_set(
        tBGP4_ROUTE *p_route,
        tBGP4_PEER *p_peer)
{
    tBGP4_EXT_FLOOD_FLAG *p_eflag = NULL;
    tBGP4_RIB *p_rib = p_route->p_path->p_instance->rib[p_route->dest.afi];

    if (p_rib == NULL)
    {
        return;
    }

    if ((p_peer->state != BGP4_NS_ESTABLISH)
        || (!bgp4_af_support(p_peer, p_route->dest.afi)))
    {
        return;
    }

    if (p_route->p_feasible == NULL)
    {
        p_route->p_feasible = bgp4_malloc(sizeof(*p_route->p_feasible), MEM_BGP_FLOOD);
        if (p_route->p_feasible == NULL)
        {
            return;
        }
        /*insert to rib's feasible table*/
        p_route->p_feasible->p_route = p_route;
        bgp4_avl_add(&p_rib->out_feasible_table, p_route->p_feasible);
    }
    /*check base index*/
    if (p_peer->bit_index <= BGP4_MAX_BASE_PEER_ID)
    {
        BIT_LST_SET(p_route->p_feasible->bits, p_peer->bit_index);
    }
    else if (p_peer->bit_index <= (BGP4_MAX_BASE_PEER_ID + BGP4_EXT_PEER_ID_COUNT))
    {
        /*use the first extend id*/
        p_eflag = p_route->p_feasible->p_extend[0];
        if (p_eflag == NULL)
        {
            p_eflag = bgp4_malloc(sizeof(tBGP4_EXT_FLOOD_FLAG), MEM_BGP_EXTFLOOD);
            if (p_eflag)
            {
                p_route->p_feasible->p_extend[0] = p_eflag;
                p_eflag->min_id = BGP4_MAX_BASE_PEER_ID + 1;
                p_eflag->max_id = (BGP4_MAX_BASE_PEER_ID + BGP4_EXT_PEER_ID_COUNT);
            }
        }
        if (p_eflag)
        {
            BIT_LST_SET(p_eflag->bits, p_peer->bit_index - p_eflag->min_id);
        }
    }
    else if (p_peer->bit_index <= (BGP4_MAX_BASE_PEER_ID + BGP4_EXT_PEER_ID_COUNT + BGP4_EXT_PEER_ID_COUNT))
    {
        /*use the second extend id*/
        p_eflag = p_route->p_feasible->p_extend[1];
        if (p_eflag == NULL)
        {
            p_eflag = bgp4_malloc(sizeof(tBGP4_EXT_FLOOD_FLAG), MEM_BGP_EXTFLOOD);
            if (p_eflag)
            {
                p_route->p_feasible->p_extend[1] = p_eflag;
                p_eflag->min_id = BGP4_MAX_BASE_PEER_ID + BGP4_EXT_PEER_ID_COUNT + 1;
                p_eflag->max_id = (BGP4_MAX_BASE_PEER_ID + BGP4_EXT_PEER_ID_COUNT + BGP4_EXT_PEER_ID_COUNT);
            }
        }
        if (p_eflag)
        {
            BIT_LST_SET(p_eflag->bits, p_peer->bit_index - p_eflag->min_id);
        }
    }    
    
    /*start rib sending timer*/
    if (!bgp4_timer_is_active(&p_peer->adj_out_timer))
    {
        bgp4_timer_start(&p_peer->adj_out_timer, BGP4_PEER_ADJOUT_INTERVAL);
    }
    return;
}

/*remove update flag for route and peer*/
void
bgp4_withdraw_flood_clear(
      tBGP4_ROUTE *p_route,
      tBGP4_PEER *p_peer)
{
    tBGP4_FLOOD_FLAG zero;
    tBGP4_EXT_FLOOD_FLAG zero_extend;
    tBGP4_EXT_FLOOD_FLAG *p_extend;
    tBGP4_RIB *p_rib = p_route->p_path->p_instance->rib[p_route->dest.afi];
    u_int i = 0;
    
    if (p_rib == NULL)
    {
        return;
    }
    if (p_route->p_withdraw == NULL)
    {
        return;
    }
    
    /*clear flag*/   
    if (p_peer->bit_index <= BGP4_MAX_BASE_PEER_ID)
    {
        BIT_LST_SET(p_route->p_withdraw->bits, p_peer->bit_index);
    }
    else
    {
        for (i = 0; i < 2 ; i++)
        {
            p_extend = p_route->p_withdraw->p_extend[i];
            if (p_extend == NULL)
            {
                continue;
            }
            if ((p_peer->bit_index >= p_extend->min_id)
                && (p_peer->bit_index <= p_extend->max_id))
            {
                BIT_LST_SET(p_extend->bits, (p_peer->bit_index - p_extend->min_id));
            }
        }
    }

    memset(&zero, 0, sizeof(zero));
    /*check if empty*/
    if (memcmp(p_route->p_withdraw->bits, zero.bits, sizeof(zero.bits)) != 0)
    {
        return;
    }

    for (i = 0; i < 2 ; i++)
    {
        p_extend = p_route->p_withdraw->p_extend[i];
        if (p_extend)
        {
            /*check extend index*/
            memset(&zero_extend, 0, sizeof(zero_extend));
            if (memcmp(zero_extend.bits, p_extend->bits, sizeof(p_extend->bits)) != 0) 
            {
                return;
            }
            p_route->p_withdraw->p_extend[i] = NULL;
            bgp4_free(p_extend, MEM_BGP_EXTFLOOD);
        }
    }
    p_route->p_withdraw->p_route = NULL;
    bgp4_avl_delete(&p_rib->out_withdraw_table, p_route->p_withdraw);
    bgp4_free(p_route->p_withdraw, MEM_BGP_FLOOD);
    p_route->p_withdraw = NULL;
    return;
}

/*remove update flag for route and peer*/
void
bgp4_feasible_flood_clear(
      tBGP4_ROUTE *p_route,
      tBGP4_PEER *p_peer)
{
    tBGP4_FLOOD_FLAG zero;
    tBGP4_EXT_FLOOD_FLAG zero_extend;
    tBGP4_EXT_FLOOD_FLAG *p_extend;
    tBGP4_RIB *p_rib = p_route->p_path->p_instance->rib[p_route->dest.afi];
    u_int i = 0;
        
    if (p_rib == NULL)
    {
        return;
    }
    if (p_route->p_feasible == NULL)
    {
        return;
    }
    
    /*clear flag*/   
    if (p_peer->bit_index <= BGP4_MAX_BASE_PEER_ID)
    {
        BIT_LST_SET(p_route->p_feasible->bits, p_peer->bit_index);
    }
    else
    {
        /*check extend index*/
        for (i = 0; i < 2 ; i++)
        {
            p_extend = p_route->p_feasible->p_extend[i];
            if (p_extend == NULL)
            {
                continue;
            }
            if ((p_peer->bit_index >= p_extend->min_id)
                && (p_peer->bit_index <= p_extend->max_id))
            {
                BIT_LST_SET(p_extend->bits, (p_peer->bit_index - p_extend->min_id));
            }
        }
    }

    memset(&zero, 0, sizeof(zero));
    /*check if empty*/
    if (memcmp(p_route->p_feasible->bits, zero.bits, sizeof(zero.bits)) != 0)
    {
        return;
    }

    for (i = 0; i < 2 ; i++)
    {
        p_extend = p_route->p_feasible->p_extend[i];
        if (p_extend)
        {
            /*check extend index*/
            memset(&zero_extend, 0, sizeof(zero_extend));
            if (memcmp(zero_extend.bits, p_extend->bits, sizeof(p_extend->bits)) != 0) 
            {
                return;
            }
            p_route->p_feasible->p_extend[i] = NULL;
            bgp4_free(p_extend, MEM_BGP_EXTFLOOD);
        }
    }

    /*release all*/
    p_route->p_feasible->p_route = NULL;
    bgp4_avl_delete(&p_rib->out_feasible_table, p_route->p_feasible);
    bgp4_free(p_route->p_feasible, MEM_BGP_FLOOD);
    p_route->p_feasible = NULL;
    return;
}

/*force to delete flooding control struct*/
void
bgp4_route_flood_clear(tBGP4_ROUTE *p_route)
{
    tBGP4_RIB *p_rib = p_route->p_path->p_instance->rib[p_route->dest.afi];
    if (p_rib == NULL)
    {
        return;
    }

    if (p_route->p_feasible)
    {
        /*free extend flag*/
        if (p_route->p_feasible->p_extend[0])
        {
            bgp4_free(p_route->p_feasible->p_extend[0], MEM_BGP_EXTFLOOD);
            p_route->p_feasible->p_extend[0] = NULL;
        }
        /*free extend flag*/
        if (p_route->p_feasible->p_extend[1])
        {
            bgp4_free(p_route->p_feasible->p_extend[1], MEM_BGP_EXTFLOOD);
            p_route->p_feasible->p_extend[1] = NULL;
        }
        
        p_route->p_feasible->p_route = NULL;
        bgp4_avl_delete(&p_rib->out_feasible_table, p_route->p_feasible);
        bgp4_free(p_route->p_feasible, MEM_BGP_FLOOD);
        p_route->p_feasible = NULL;
    }
    if (p_route->p_withdraw)
    {
        /*free extend flag*/
        if (p_route->p_withdraw->p_extend[0])
        {
            bgp4_free(p_route->p_withdraw->p_extend[0], MEM_BGP_EXTFLOOD);
            p_route->p_withdraw->p_extend[0] = NULL;
        }
        /*free extend flag*/
        if (p_route->p_withdraw->p_extend[1])
        {
            bgp4_free(p_route->p_withdraw->p_extend[1], MEM_BGP_EXTFLOOD);
            p_route->p_withdraw->p_extend[1] = NULL;
        }
        p_route->p_withdraw->p_route = NULL;
        bgp4_avl_delete(&p_rib->out_withdraw_table, p_route->p_withdraw);
        bgp4_free(p_route->p_withdraw, MEM_BGP_FLOOD);
        p_route->p_withdraw = NULL;
    }
    return;
}

/*clear flooding flag for special peer,called when peer deleted*/
void
bgp4_peer_flood_clear(tBGP4_PEER *p_peer)
{
    tBGP4_VPN_INSTANCE *p_instance = p_peer->p_instance;
    tBGP4_RIB *p_rib = NULL;    
    tBGP4_FLOOD_FLAG *p_flood = NULL;
    tBGP4_FLOOD_FLAG *p_next = NULL;
    u_int af = 0;
    
    /*scan all rib of instance*/
    for (af = 0; af < BGP4_PF_MAX ; af++)
    {
        p_rib = p_instance->rib[af];
        if (p_instance->rib[af] == NULL)
        {
            continue;
        }
        /*check all withdraw route*/
        bgp4_avl_for_each_safe(&p_rib->out_withdraw_table, p_flood, p_next)
        {
            /*clear update flag of this route*/
            bgp4_withdraw_flood_clear(p_flood->p_route, p_peer);
        }

        bgp4_avl_for_each_safe(&p_rib->out_feasible_table, p_flood, p_next)
        {
            /*clear update flag of this route*/
            bgp4_feasible_flood_clear(p_flood->p_route, p_peer);
        }
        /*schedule clear unused route*/
        bgp4_timer_start(&p_rib->route_clear_timer, BGP4_ROUTE_CLEAR_INTERVAL);
    }
    return;
}
/*check if withdraw flag set for peer*/
u_int
bgp4_withdraw_flood_isset(
              tBGP4_ROUTE *p_route,
              tBGP4_PEER *p_peer)
{
    tBGP4_FLOOD_FLAG *p_flag = p_route->p_withdraw;
    u_int i;
    u_int offset = 0;
    
    if (p_flag == NULL)
    {
        return FALSE;
    }
    /*check base index*/
    if (p_peer->bit_index <= BGP4_MAX_BASE_PEER_ID)
    {
        if (BIT_LST_TST(p_flag->bits, p_peer->bit_index))
        {
            return TRUE;
        }
        return FALSE;
    }
    /*check extend index*/
    for (i = 0; i < 2 ; i++)
    {
        if (p_flag->p_extend[i] == NULL)
        {
            continue;
        }
        if ((p_peer->bit_index >= p_flag->p_extend[i]->min_id)
            && (p_peer->bit_index <= p_flag->p_extend[i]->max_id))
        {
            offset = p_peer->bit_index - p_flag->p_extend[i]->min_id;
            if (BIT_LST_TST(p_flag->p_extend[i]->bits, offset))
            {
                return TRUE;
            }
            return FALSE;
        }
    }
    return FALSE;
}    

/*check if feasible flag set for peer*/
u_int
bgp4_feasible_flood_isset(
              tBGP4_ROUTE *p_route,
              tBGP4_PEER *p_peer)
{
    tBGP4_FLOOD_FLAG *p_flag = p_route->p_feasible;
    u_int i;
    u_int offset = 0;
    
    if (p_flag == NULL)
    {
        return FALSE;
    }
    /*check base index*/
    if (p_peer->bit_index <= BGP4_MAX_BASE_PEER_ID)
    {
        if (BIT_LST_TST(p_flag->bits, p_peer->bit_index))
        {
            return TRUE;
        }
        return FALSE;
    }
    /*check extend index*/
    for (i = 0; i < 2 ; i++)
    {
        if (p_flag->p_extend[i] == NULL)
        {
            continue;
        }
        if ((p_peer->bit_index >= p_flag->p_extend[i]->min_id)
            && (p_peer->bit_index <= p_flag->p_extend[i]->max_id))
        {
            offset = p_peer->bit_index - p_flag->p_extend[i]->min_id;
            if (BIT_LST_TST(p_flag->p_extend[i]->bits, offset))
            {
                return TRUE;
            }
            return FALSE;
        }
    }
    return FALSE;
}    


/*decide if flooding buffer is in use*/
u_int 
bgp4_route_flood_is_pending(tBGP4_ROUTE *p_route)
{
    tBGP4_FLOOD_FLAG zero;
    tBGP4_EXT_FLOOD_FLAG zero_extend;
    
    if ((p_route->p_withdraw == NULL)
        && (p_route->p_feasible == NULL))
    {
        return FALSE;
    }
    memset(&zero, 0, sizeof(zero));
    memset(&zero_extend, 0, sizeof(zero_extend));

    if (p_route->p_withdraw)
    {
        if (memcmp(p_route->p_withdraw->bits, zero.bits, sizeof(zero.bits)))
        {
            return TRUE;
        }
        
        if (p_route->p_withdraw->p_extend[0]
            && (memcmp(p_route->p_withdraw->p_extend[0]->bits, 
                zero_extend.bits, sizeof(zero_extend.bits))))
        {
            return TRUE;
        }                
        if (p_route->p_withdraw->p_extend[1]
            && (memcmp(p_route->p_withdraw->p_extend[1]->bits, 
                zero_extend.bits, sizeof(zero_extend.bits))))
        {
            return TRUE;
        }                
    }
    
    if (p_route->p_feasible)
    {
        if (memcmp(p_route->p_feasible->bits, zero.bits, sizeof(zero.bits)))
        {
            return TRUE;
        }
        
        if (p_route->p_feasible->p_extend[0]
            && (memcmp(p_route->p_feasible->p_extend[0]->bits, 
                zero_extend.bits, sizeof(zero_extend.bits))))
        {
            return TRUE;
        }                
        if (p_route->p_feasible->p_extend[1]
            && (memcmp(p_route->p_feasible->p_extend[1]->bits, 
                zero_extend.bits, sizeof(zero_extend.bits))))
        {
            return TRUE;
        }                
    }
    return FALSE;
}

/*update rib input table of instance for rxd route*/
void
bgp4_rib_in_table_update(tBGP4_ROUTE *p_route)
{
    tBGP4_ROUTE_VECTOR vector;
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_RIB *p_rib = p_instance->rib[p_route->dest.afi];
    avl_tree_t *p_table = &p_rib->rib_in_table;
    tBGP4_ROUTE *p_current = NULL;
    tBGP4_ROUTE *p_new = NULL;
    u_int finish = 0;

    if (p_rib == NULL)
    {
        return;
    }
    
    if (p_route->is_deleted == TRUE)
    {
        bgp4_timer_start(&p_rib->route_clear_timer, BGP4_ROUTE_CLEAR_INTERVAL);
    }
    
    /*get all route with same dest*/
    bgp4_rib_lookup_dest(p_table, &p_route->dest, &vector);
    
    /*lookup current route*/
    p_current = bgp4_rib_vector_lookup(&vector, p_route);

    /*if current route not exist,and nexthop processing
      not scheduled,try to update first*/
    if ((p_current == NULL) 
        && (p_rib->nexthop_changed == FALSE)
        && (gbgp4.work_mode != BGP4_MODE_SLAVE))
    {
        /*do rib update once,if not finished,add to rib_in*/
        if (p_route->is_deleted == FALSE)
        {
            finish = bgp4_rib_in_new_route_check(p_route);
        }
        else
        {
            finish = bgp4_rib_in_withdraw_route_check(p_route);
        }
        if (finish == TRUE)
        {
            return;
        }
        p_new = bgp4_route_duplicate(p_route);
        if (!p_new)
        {
            return;
        }
        /*add to rib*/
        bgp4_route_table_add(p_table, p_new);
    
        /*start rib input timer if not started*/
        if (bgp4_timer_is_active(&p_rib->rib_in_timer) == FALSE)
        {
            bgp4_timer_start_ms(&p_rib->rib_in_timer, BGP4_RIB_IN_CHECK_INTERVAL);
        }
        return;
    }

    if (p_current)
    {
        /*if current route is feasible,and new route is withdraw
          ,delete current route,do nothing more*/
        if ((p_current->is_deleted == FALSE) 
            && (p_route->is_deleted == TRUE))
        {
            bgp4_route_table_delete(p_current);
            gbgp4.stat.rib_in_discard++;
            return;
        }
        /*if both delete request,do nothing
        */
        if ((p_current->is_deleted == TRUE) 
            && (p_route->is_deleted == TRUE))
        {
            return;
        }
        /*delete current route,and insert new route*/
        bgp4_route_table_delete(p_current);
    }
    /*insert new route node*/
    p_new = bgp4_route_duplicate(p_route);
    if (!p_new)
    {
        return;
    }
    /*add to rib*/
    bgp4_route_table_add(p_table, p_new);

    /*start rib input timer if not started*/
    if (bgp4_timer_is_active(&p_rib->rib_in_timer) == FALSE)
    {
        bgp4_timer_start_ms(&p_rib->rib_in_timer, BGP4_RIB_IN_CHECK_INTERVAL);
    }

    /*use larger timer for backup card*/
    if (gbgp4.work_mode == BGP4_MODE_SLAVE)
    {
        bgp4_timer_start(&p_rib->rib_in_timer, BGP4_RIB_IN_SLAVE_CHECK_INTERVAL);
    }
    return;
}

/*allocate new route according to input route,copy all fields,and
  add path*/
tBGP4_ROUTE *
bgp4_route_duplicate(tBGP4_ROUTE *p_route)
{
    tBGP4_ROUTE *p_new = bgp4_malloc(sizeof(tBGP4_ROUTE), MEM_BGP_ROUTE);
    tBGP4_PATH *p_path = NULL;
    
    if (!p_new)
    {
        return NULL;
    }
    
    memcpy(&p_new->dest, &p_route->dest, sizeof(p_route->dest));
    p_new->proto = p_route->proto;
    p_new->is_deleted = p_route->is_deleted;
    p_new->igp_sync_wait = p_route->igp_sync_wait; 
    p_new->in_label = p_route->in_label;
    p_new->out_label = p_route->out_label;
    p_new->upe_label = p_route->upe_label;
    p_new->preference = p_route->preference;
    p_new->processId = p_route->processId;
    
    /*allocate path info*/
    p_path = bgp4_path_add(p_route->p_path);
    if (!p_path)
    {
        bgp4_free(p_new, MEM_BGP_ROUTE);
        return NULL;
    }
    /*link route and path info*/
    bgp4_link_path(p_path, p_new);
    return p_new;
}

#if 1/*support fast backup route*/
/*calculate weight for a route in special vector*/
void
bgp4_route_vector_weight_calculate(
              tBGP4_ROUTE_VECTOR *p_vector,
              tBGP4_ROUTE *p_route)
{
    tBGP4_ROUTE_VECTOR save;
    tBGP4_ROUTE_VECTOR sort;/*route is listed in sort*/
    u_int best_id = 0;
    u_int id = 0;
    
    memset(&sort, 0, sizeof(sort));
    memcpy(&save, p_vector, sizeof(save));
    
    /*sorting*/
    for (best_id = bgp4_rib_vector_best_index(&save);
         best_id < BGP4_MAX_ECMP;
         best_id = bgp4_rib_vector_best_index(&save))
    {
        sort.p_route[sort.count++] = save.p_route[best_id];
        save.p_route[best_id] = NULL;
    }
    /*decide expected route's place*/
    for (id = 0; id < sort.count; id++)
    {
        if (sort.p_route[id] == p_route)
        {
            /*this is the best one*/
            if (id == 0)
            {
                /*no more route exist,use base value*/
                if (sort.count == 1)
                {
                    p_route->preference = BGP4_BASE_PREFERENCE;
                }
                else
                {
                    /*use less value than the next value.do not consider conflict*/
                    p_route->preference = sort.p_route[id + 1]->preference/2;
                }                
            }
            else if (id == (sort.count - 1))
            {
                /*this is the worst one,use larger value*/
                p_route->preference = sort.p_route[id - 1]->preference*2;
            }
            else
            {
                /*select middle value*/
                p_route->preference = (sort.p_route[id-1]->preference + sort.p_route[id+1]->preference)/2;
            }
            break;
        }
    }    
    return;
}

/*check incoming rib ,update active rib
  return TRUE:route is processed,it will be
  deleted from rib_in after processing;
  FALSE:route is not processed,we must wait for
  next process*/
u_int 
bgp4_rib_in_new_route_check_backup(tBGP4_ROUTE *p_route)
{
    tBGP4_ROUTE_VECTOR vector;
    tBGP4_ROUTE *p_current = NULL;
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_ROUTE *p_newbest = NULL;
    tBGP4_ROUTE *p_new = NULL;
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_RIB *p_rib = p_instance->rib[p_route->dest.afi];
    u_int i = 0;

    /*get current route with same dest*/
    bgp4_rib_lookup_dest(&p_rib->rib_table, &p_route->dest, &vector);
    /*if any route has pending system update request,do not process
      route in this loop.do not care about flooding flag*/
    for (i = 0 ; i < vector.count ; i++)
    {
        if (vector.p_route[i] 
            && (!bgp4_system_update_finished(vector.p_route[i])))
        {
            bgp4_log(BGP_DEBUG_UPDATE, "system update is pending,wait for next loop");
            return FALSE;
        }
    }

    /*IGP sync checking*/
    p_route->igp_sync_wait = FALSE;
    if ((gbgp4.igp_sync_enable == TRUE) 
        && p_route->p_path->p_peer
        && (bgp4_peer_type(p_route->p_path->p_peer) == BGP4_IBGP))
    {
       if (bgp4_sysroute_lookup(p_instance->vrf, p_route) != VOS_OK)
       {
           p_route->igp_sync_wait = TRUE;
           
           /*start IGP sync checking timer,in short value*/
           bgp4_timer_start(&gbgp4.igp_sync_timer, BGP4_IGP_SYNC_MIN_INTERVAL);
       }
    }

    /*get expected route*/
    p_current = bgp4_rib_vector_lookup(&vector, p_route);

    /*current route not exist,it is a pure new route*/
    if (p_current == NULL)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "current route not exist,create one");
        /*get current best route*/
        p_best = bgp4_rib_vector_best(&vector);
    
        /*current route not exist,create new route*/
        p_new = bgp4_route_duplicate(p_route);
        if (p_new == NULL)
        {
            return FALSE;
        }
        bgp4_route_table_add(&p_rib->rib_table, p_new);
    
        vector.p_route[vector.count++] = p_new;

        /*calculate cost for route vector*/
        bgp4_route_vector_weight_calculate(&vector, p_new);
        
        /*get new best route after updated*/
        p_newbest = bgp4_rib_vector_best(&vector);

        /*if input route can be updated to system,schedule system update*/
        if (bgp4_route_system_update_enable(p_new) == TRUE)
        {
            bgp4_schedule_system_add(p_new);
        }
        
        /*case:new best is not input route,do nothing*/
        if (p_newbest != p_new)
        {
            return TRUE;
        }
        bgp4_log(BGP_DEBUG_UPDATE, "new route is best,delete old system route"); 

        /*new best is input route*/
        /*if old best route exist,schedule system delete,and clear flooding flag*/
        if (p_best)
        {            
            bgp4_route_flood_clear(p_best);
            /*set withdraw flag for new route's peer*/
            if (p_new->p_path->p_peer)
            {
                /*route is filtered by range*/
                if (bgp4_withdraw_flood_check_against_range(p_best) == VOS_OK)
                {
                    bgp4_withdraw_flood_set(p_best, p_new->p_path->p_peer);
                }    
            }
        }
        
        /*schedule flooding for new route*/
        bgp4_route_feasible_flood_set(p_new);
        return TRUE;
    }

    /*current route exist*/
    /*current route is to be deleted,all system update finished
      delete it directly.wait for adding in next loop*/
    if (p_current->is_deleted == TRUE)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "delete current route,wait for next loop");
        
        bgp4_route_table_delete(p_current);
        return FALSE;
    }

    /*current route is active*/

    /*if path not changed,do nothing*/
    if (bgp4_path_same(p_current->p_path, p_route->p_path) == TRUE)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "route has no change,doesn't need more add");
        /*clear stale flag*/
        p_current->stale = FALSE;
        return TRUE;
    }
    /*path changed,more checking*/

    /*delete current route from kernal*/
    if (bgp4_route_system_update_enable(p_current) == TRUE)
    {
        bgp4_schedule_system_delete(p_current);
        bgp4_route_flood_clear(p_current);
        return FALSE;
    }

    /*current route is not in kernal*/
    
    /*get current best route*/
    p_best = bgp4_rib_vector_best(&vector);

    /*set current route state to inactive,and
      insert new route into vector*/
    p_current->is_deleted = TRUE;

    vector.p_route[vector.count++] = p_route;

    /*get new best route after updated*/
    p_newbest = bgp4_rib_vector_best(&vector);

    /*case:current and new best node both input route.
      if current route is in system,schedule to delete it.
      do not schedule flooding.wait for add in next loop*/
    if ((p_best == p_current) && (p_newbest == p_route))
    {
        bgp4_log(BGP_DEBUG_UPDATE, "current and new best are same,replace it");
        
        /*use current preference directly*/
        p_route->preference = p_current->preference;
        
        /*route can not be exported to system,just replace it*/
        bgp4_route_table_delete(p_current);
        
        /*copy new route*/
        p_new = bgp4_route_duplicate(p_route);
        if (p_new == NULL)
        {
            return FALSE;
        }
        bgp4_route_table_add(&p_rib->rib_table, p_new);

        /*schedule flooding*/
        bgp4_route_feasible_flood_set(p_new);
        return TRUE;
    }

    /*calculate preference for route with same dest*/
    bgp4_route_vector_weight_calculate(&vector, p_route);
    
    /*case:current and new best node is not input route,replace
         directly*/ 
    if ((p_best != p_current) && (p_newbest != p_route))
    {
        bgp4_log(BGP_DEBUG_UPDATE, "route is not best,immediately add");
        
        bgp4_route_table_delete(p_current);
        
        /*copy new route*/
        p_new = bgp4_route_duplicate(p_route);
        if (p_new == NULL)
        {
            return FALSE;
        }
        bgp4_route_table_add(&p_rib->rib_table, p_new);
        if (bgp4_route_system_update_enable(p_new) == TRUE)
        {
            bgp4_schedule_system_add(p_new);                   
        }
        return TRUE;
    }

    /*case:current best is this route,new best is not this route
        */
    if ((p_best == p_current) && (p_newbest != p_route))
    {
        bgp4_log(BGP_DEBUG_UPDATE, "route change from best to not best");
        
        /*schedule flooding of new best*/
        bgp4_route_feasible_flood_set(p_newbest);
        
        /*insert route directly,no flood need*/
        bgp4_route_table_delete(p_current);
        
        /*copy new route*/
        p_new = bgp4_route_duplicate(p_route);
        if (p_new == NULL)
        {
            return FALSE;
        }
        bgp4_route_table_add(&p_rib->rib_table, p_new);
        if (bgp4_route_system_update_enable(p_new) == TRUE)
        {
            bgp4_schedule_system_add(p_new);                   
        }
        return TRUE;
    }

    /*case:old best is not this route,new best is this route
      replace directly,schedule system delete of old route,
      schedule system add of new route,and flooding*/
    if ((p_best != p_current) && (p_newbest == p_route))
    {
        bgp4_log(BGP_DEBUG_UPDATE, "route change from not best to best");
        
        bgp4_route_flood_clear(p_best);

        bgp4_route_table_delete(p_current);
        
        /*copy new route*/
        p_new = bgp4_route_duplicate(p_route);
        if (p_new == NULL)
        {
            return FALSE;
        }
        bgp4_route_table_add(&p_rib->rib_table, p_new);
        if (bgp4_route_system_update_enable(p_new) == TRUE)
        {
            bgp4_schedule_system_add(p_new);                   
        }
        bgp4_route_feasible_flood_set(p_new);
        return TRUE;
    }
    /*impossible*/
    return TRUE;
}
/*check incoming rib ,update active rib
  return TRUE:route is processed,it will be
  deleted from rib_in after processing;
  FALSE:route is not processed,we must wait for
  next process*/
u_int 
bgp4_rib_in_withdraw_route_check_backup(tBGP4_ROUTE *p_route)
{
    tBGP4_ROUTE_VECTOR vector;
    tBGP4_ROUTE *p_current = NULL;
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_ROUTE *p_newbest = NULL;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_RIB *p_rib = p_instance->rib[p_route->dest.afi];    
    u_int i = 0;

    /*get current route with same dest*/
    bgp4_rib_lookup_dest(&p_rib->rib_table, &p_route->dest, &vector);
    
    /*if any route has pending system update request,do not process
      route in this loop*/
    for (i = 0 ; i < vector.count ; i++)
    {
        if (vector.p_route[i] 
            && (!bgp4_system_update_finished(vector.p_route[i]))
            && (!bgp4_route_flood_is_pending(vector.p_route[i])))
        {
            bgp4_log(BGP_DEBUG_UPDATE, "system update is pending,wait for next loop");
            return FALSE;
        }
    }
    /*get expected route*/
    p_current = bgp4_rib_vector_lookup(&vector, p_route);
    /*no such route,do nothing*/
    if (p_current == NULL)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "no such route,ignore it");
        return TRUE;
    }
    /*if already deleted,do nothing*/
    if (p_current->is_deleted == TRUE)
    {    
        bgp4_log(BGP_DEBUG_UPDATE, "route already to be deleted");
        return TRUE;
    }

    /*schedule system delete if necessary*/
    if (bgp4_route_system_update_enable(p_current) == TRUE)
    {
        bgp4_schedule_system_delete(p_current);
    }

    /*get best route before delete*/
    p_best = bgp4_rib_vector_best(&vector);

    p_current->is_deleted = TRUE;

    p_newbest = bgp4_rib_vector_best(&vector);
    /*case:best route not changed,delete directly*/
    if (p_best == p_newbest)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "it is not best route,immediately delete");
        return TRUE;
    }

    /*if new best route not exist,schedule withdraw flood*/
    if (p_newbest == NULL)
    {
        bgp4_route_withdraw_flood_set(p_best);
    }
    else
    {
         bgp4_route_feasible_flood_set(p_newbest);

        /*special process,if can not send feasible route to 
         some peer,send withdraw for it.N2x testing and 
         RFC4271 9.1.3*/
         bgp4_peer_for_each(p_instance, p_peer)
         {
             if (bgp4_update_output_verify(p_peer, p_newbest) == FALSE)
             {
                 bgp4_feasible_flood_clear(p_newbest, p_peer);
   
                 bgp4_withdraw_flood_set(p_best, p_peer);
             }
         }
    }
    /*if no update need,delete current route directly*/
    if ((p_current->p_path->p_peer == NULL)
         && (p_current->p_path->origin_vrf == p_instance->vrf)
         && p_newbest
         && (bgp4_route_flood_is_pending(p_current) == FALSE))
    {
        bgp4_log(BGP_DEBUG_UPDATE, "no pending operation,immediately delete");
        
        bgp4_route_table_delete(p_current);
    }
    return TRUE;
}
#endif

/*check incoming rib ,update active rib
  return TRUE:route is processed,it will be
  deleted from rib_in after processing;
  FALSE:route is not processed,we must wait for
  next process*/
u_int 
bgp4_rib_in_new_route_check(tBGP4_ROUTE *p_route)
{
    tBGP4_ROUTE_VECTOR vector;
    tBGP4_ROUTE *p_current = NULL;
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_ROUTE *p_newbest = NULL;
    tBGP4_ROUTE *p_new = NULL;
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_RIB *p_rib = p_instance->rib[p_route->dest.afi];
    u_int i = 0;
    u_char str[64];
    u_char pstr[64];

    if (p_rib == NULL)
    {
        return TRUE;
    }
    if (p_route->p_path->p_peer)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "process vrf %d feasible route %s from %s", 
            p_instance->vrf,
            bgp4_printf_route(p_route, str),
            bgp4_printf_peer(p_route->p_path->p_peer, pstr));
    }
    else
    {
        bgp4_log(BGP_DEBUG_UPDATE, "process vrf %d feasible route %s from local", 
            p_instance->vrf,
            bgp4_printf_route(p_route, str));
    }    
    
    /*damp checking*/
    if (gbgp4.damp_enable 
        && (bgp4_damp_route_supressed(p_route, TRUE) == TRUE))
    {
         bgp4_log(BGP_DEBUG_UPDATE, "route is suppressed,wait for next loop");
         return FALSE;
    }
    /*allocate incoming label for vpn route*/
    if ((p_instance->vrf == 0) 
        && (p_route->in_label == 0)
        && (p_route->dest.afi == BGP4_PF_IPVPN)
        && ((p_route->p_path) && (bgp4_index_to_safi(p_route->p_path->af) != BGP4_SAF_VLBL))) /*caoyong add need test 2018-3-12*/
    {
        bgp4_vpn_route_in_label_get(p_route);
    }

    /* support fast backup route*/
    /*if backup path supported,try to update all kernal routes*/
    if (gbgp4.backup_path_enable == TRUE)
    {
        return bgp4_rib_in_new_route_check_backup(p_route);
    }
    
    /*get current route with same dest*/
    bgp4_rib_lookup_dest(&p_rib->rib_table, &p_route->dest, &vector);
    /*if any route has pending system update request,do not process
      route in this loop.do not care about flooding flag*/
    for (i = 0 ; i < vector.count ; i++)
    {
        if (vector.p_route[i] 
            && (!bgp4_system_update_finished(vector.p_route[i])))
        {
            bgp4_log(BGP_DEBUG_UPDATE, "system update is pending,wait for next loop");
            return FALSE;
        }
    }

    /*IGP sync checking*/
    p_route->igp_sync_wait = FALSE;
    if ((gbgp4.igp_sync_enable == TRUE) 
        && p_route->p_path->p_peer
        && (bgp4_peer_type(p_route->p_path->p_peer) == BGP4_IBGP))
    {
       if (bgp4_sysroute_lookup(p_instance->vrf, p_route) != VOS_OK)
       {
           p_route->igp_sync_wait = TRUE;
           
           /*start IGP sync checking timer,in short value*/
           bgp4_timer_start(&gbgp4.igp_sync_timer, BGP4_IGP_SYNC_MIN_INTERVAL);
       }
    }
    
    /*get expected route*/
    p_current = bgp4_rib_vector_lookup(&vector, p_route);
    /*current route exist*/
    if (p_current)
    {
        /*current route is to be deleted,all system update finished
          delete it directly.wait for adding*/
        if (p_current->is_deleted == TRUE)
        {
            bgp4_log(BGP_DEBUG_UPDATE, "delete current route,wait for next loop");
            
            bgp4_route_table_delete(p_current);
            return FALSE;
        }
        /*current route is active*/

        /*if path not changed,do nothing*/
        if (bgp4_path_same(p_current->p_path, p_route->p_path) == TRUE)
        {
            bgp4_log(BGP_DEBUG_UPDATE, "route has no change,no more add need");
            /*clear stale flag*/
            p_current->stale = FALSE;
            return TRUE;
        }
        /*path changed,more checking*/

        /*get current best route*/
        p_best = bgp4_rib_vector_best(&vector);

        /*set current route state to inactive,and
          insert new route into vector*/
        p_current->is_deleted = TRUE;

        vector.p_route[vector.count++] = p_route;

        /*get new best route after updated*/
        p_newbest = bgp4_rib_vector_best(&vector);

        /*case:current and new best node is not input route,replace
         directly*/ 
        if ((p_best != p_current) && (p_newbest != p_route))
        {
            bgp4_log(BGP_DEBUG_UPDATE, "route is not best,immediately add");
            
            bgp4_route_table_delete(p_current);
            
            /*copy new route*/
            p_new = bgp4_route_duplicate(p_route);
            if (p_new == NULL)
            {
                return FALSE;
            }
            bgp4_route_table_add(&p_rib->rib_table, p_new);
            return TRUE;
        }

        /*case:current and new best node both input route.
          if current route is in system,schedule to delete it.
          do not schedule flooding.wait for add in next loop
         */
        if ((p_best == p_current) && (p_newbest == p_route))
        {
            bgp4_log(BGP_DEBUG_UPDATE, "current and new best are same,replace it");
            
            if (bgp4_route_system_update_enable(p_current) == TRUE)
            {
                bgp4_schedule_system_delete(p_current);
                bgp4_route_flood_clear(p_current);
                return FALSE;
            }
            /*route can not be exported to system,just replace it*/
            bgp4_route_table_delete(p_current);
            
            /*copy new route*/
            p_new = bgp4_route_duplicate(p_route);
            if (p_new == NULL)
            {
                return FALSE;
            }
            bgp4_route_table_add(&p_rib->rib_table, p_new);

            /*schedule flooding*/
            bgp4_route_feasible_flood_set(p_new);
            return TRUE;
        }

        /*case:current best is this route,new best is not this route
        */
        if ((p_best == p_current) && (p_newbest != p_route))
        {
            bgp4_log(BGP_DEBUG_UPDATE, "route change from best to not best");
            
            /*new best must exist,do schedule system add of new route*/ /*caoyong�޸�2018-1-4*/
            if (bgp4_route_system_update_enable(p_newbest) == TRUE)
            {
                bgp4_schedule_system_add(p_newbest);
            }
            /*schedule flooding of new best*/
            bgp4_route_feasible_flood_set(p_newbest);
            
            if (bgp4_route_system_update_enable(p_current) == TRUE)
            {
                bgp4_schedule_system_delete(p_current);
                bgp4_route_flood_clear(p_current);
                return FALSE;
            }
            /*insert route directly,no flood need*/
            bgp4_route_table_delete(p_current);
            
            /*copy new route*/
            p_new = bgp4_route_duplicate(p_route);
            if (p_new == NULL)
            {
                return FALSE;
            }
            bgp4_route_table_add(&p_rib->rib_table, p_new);
            return TRUE;
        }
        /*case:old best is not this route,new best is this route
          replace directly,schedule system delete of old route,
          schedule system add of new route,and flooding*/
        if ((p_best != p_current) && (p_newbest == p_route))
        {
            bgp4_log(BGP_DEBUG_UPDATE, "route change from not best to best");
            
            if (bgp4_route_system_update_enable(p_best) == TRUE)
            {
                bgp4_schedule_system_delete(p_best);
            }
            bgp4_route_flood_clear(p_best);

            bgp4_route_table_delete(p_current);
            
            /*copy new route*/
            p_new = bgp4_route_duplicate(p_route);
            if (p_new == NULL)
            {
                return FALSE;
            }
            bgp4_route_table_add(&p_rib->rib_table, p_new);
            if (bgp4_route_system_update_enable(p_new) == TRUE)
            {
                bgp4_schedule_system_add(p_new);                   
            }
            bgp4_route_feasible_flood_set(p_new);
            return TRUE;
        }
        /*impossible*/
        return TRUE;
    }

    bgp4_log(BGP_DEBUG_UPDATE, "current route not exist,create one");
    /*get current best route*/
    p_best = bgp4_rib_vector_best(&vector);

    /*current route not exist,create new route*/
    p_new = bgp4_route_duplicate(p_route);
    if (p_new == NULL)
    {
        return FALSE;
    }
    bgp4_route_table_add(&p_rib->rib_table, p_new);

    vector.p_route[vector.count++] = p_new;

    /*get new best route after updated*/
    p_newbest = bgp4_rib_vector_best(&vector);

    /*case:new best is not input route,do nothing*/
    if (p_newbest != p_new)
    {
        return TRUE;
    }
    bgp4_log(BGP_DEBUG_UPDATE, "new route is best,delete old system route"); 
    
    /*new best is input route*/
    /*if old best route exist,schedule system delete,and clear flooding flag*/
    if (p_best)
    {
        if (bgp4_route_system_update_enable(p_best) == TRUE)
        {
            bgp4_schedule_system_delete(p_best);
        }
        bgp4_route_flood_clear(p_best);

        /*set withdraw flag for new route's peer*/
        if (p_new->p_path->p_peer)
        {
            /*route is filtered by range*/
            if (bgp4_withdraw_flood_check_against_range(p_best) == VOS_OK)
            {
                bgp4_withdraw_flood_set(p_best, p_new->p_path->p_peer);
            }    
        }
    }
    /*if input route can be updated to system,schedule system update*/
    if (bgp4_route_system_update_enable(p_new) == TRUE)
    {
        bgp4_schedule_system_add(p_new);
    }    
    /*schedule flooding*/
    bgp4_route_feasible_flood_set(p_new);
    return TRUE;
}

/*check incoming rib ,update active rib
  return TRUE:route is processed,it will be
  deleted from rib_in after processing;
  FALSE:route is not processed,we must wait for
  next process*/
u_int 
bgp4_rib_in_withdraw_route_check(tBGP4_ROUTE *p_route)
{
    tBGP4_ROUTE_VECTOR vector;
    tBGP4_ROUTE *p_current = NULL;
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_ROUTE *p_newbest = NULL;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_RIB *p_rib = p_instance->rib[p_route->dest.afi];    
    u_int i = 0;
    u_char str[64];
    u_char pstr[64];

    if (p_rib == NULL)
    {
        return TRUE;
    }

    if (p_route->p_path->p_peer)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "process vrf %d withdraw route %s from %s", 
            p_instance->vrf,
            bgp4_printf_route(p_route, str),
            bgp4_printf_peer(p_route->p_path->p_peer, pstr));
    }
    else
    {
        bgp4_log(BGP_DEBUG_UPDATE, "process vrf %d withdraw route %s from local", 
            p_instance->vrf,
            bgp4_printf_route(p_route, str));
    }
    
    /*damp checking*/
    if (gbgp4.damp_enable 
        && (bgp4_damp_route_supressed(p_route, FALSE) == TRUE))
    {
         bgp4_log(BGP_DEBUG_UPDATE, "route is suppressed,wait for next loop");
         return FALSE;
    }
    /*support fast backup route*/
    /*if backup path supported,try to update all kernal routes*/
    if (gbgp4.backup_path_enable == TRUE)
    {
        return bgp4_rib_in_withdraw_route_check_backup(p_route);
    }

    /*get current route with same dest*/
    bgp4_rib_lookup_dest(&p_rib->rib_table, &p_route->dest, &vector);
    
    /*if any route has pending system update request,do not process
      route in this loop*/
    for (i = 0 ; i < vector.count ; i++)
    {
        if (vector.p_route[i] 
            && (!bgp4_system_update_finished(vector.p_route[i]))
            && (!bgp4_route_flood_is_pending(vector.p_route[i])))
        {
            bgp4_log(BGP_DEBUG_UPDATE, "system update is pending,wait for next loop");
            return FALSE;
        }
    }
    /*get expected route*/
    p_current = bgp4_rib_vector_lookup(&vector, p_route);
    /*no such route,do nothing*/
    if (p_current == NULL)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "no such route,ignore it");
        return TRUE;
    }
    /*if already deleted,do nothing*/
    if (p_current->is_deleted == TRUE)
    {    
        bgp4_log(BGP_DEBUG_UPDATE, "route already to be deleted");
        return TRUE;
    }    
    /*get best route before delete*/
    p_best = bgp4_rib_vector_best(&vector);

    p_current->is_deleted = TRUE;

    p_newbest = bgp4_rib_vector_best(&vector);
    /*case:best route not changed,delete directly*/
    if (p_best == p_newbest)
    {
        bgp4_log(BGP_DEBUG_UPDATE, "it is not best route,immediately delete");
        bgp4_route_table_delete(p_current);
        return TRUE;
    }

    /*case:best route changed.route must be the old best*/
    /*schedule system delete if necessary*/
    if (bgp4_route_system_update_enable(p_current) == TRUE)
    {
        bgp4_schedule_system_delete(p_current);
    }
    /*if new best route not exist,schedule withdraw flood*/
    if (p_newbest == NULL)
    {
        bgp4_route_withdraw_flood_set(p_best);
    }
    else
    {
        /*update system route using new best route,schedule flood*/
         if (bgp4_route_system_update_enable(p_newbest) == TRUE)
         {
             bgp4_schedule_system_add(p_newbest);
         }
         bgp4_route_feasible_flood_set(p_newbest);

        /*special process,if can not send feasible route to 
         some peer,send withdraw for it.N2x testing and 
         RFC4271 9.1.3*/
         bgp4_peer_for_each(p_instance, p_peer)
         {
             if (bgp4_update_output_verify(p_peer, p_newbest) == FALSE)
             {
                 bgp4_feasible_flood_clear(p_newbest, p_peer);
   
                 bgp4_withdraw_flood_set(p_best, p_peer);
             }
         }
    }
    /*if no update need,delete current route directly*/
    if ((p_current->p_path->p_peer == NULL)
         && (p_current->p_path->origin_vrf == p_instance->vrf)
         && p_newbest
         && (bgp4_route_flood_is_pending(p_current) == FALSE))
    {
        bgp4_log(BGP_DEBUG_UPDATE, "no pending operation,immediately delete");
        
        bgp4_route_table_delete(p_current);
    }
    return TRUE;
}

void
bgp4_rib_in_check_timeout(tBGP4_RIB *p_rib)
{
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_next = NULL;
    u_int finish = 0;
    u_int count = 0;

    /*if nexthop change event is scheduled,do not process incoming
      route*/
    if (p_rib->nexthop_changed == TRUE)
    {
        bgp4_timer_start_ms(&p_rib->rib_in_timer, BGP4_RIB_IN_CHECK_INTERVAL);
        return;
    }
    bgp4_avl_for_each_safe(&p_rib->rib_in_table, p_route, p_next)
    {
        if (p_route->is_deleted == FALSE)
        {
            finish = bgp4_rib_in_new_route_check(p_route);
        }
        else
        {
            finish = bgp4_rib_in_withdraw_route_check(p_route);
        }
        /*process finished,delete route from rib_in*/
        if (finish == TRUE)
        {
            bgp4_route_table_delete(p_route);
        }
        /*process 1000 route per loop*/
        if (count++ >= BGP4_RIB_IN_CHECK_LIMIT)
        {
            break;
        }
    }
    /*if rib_in route not processed fully,restart timer*/
    if (bgp4_avl_count(&p_rib->rib_in_table))
    {
        bgp4_timer_start_ms(&p_rib->rib_in_timer, BGP4_RIB_IN_CHECK_INTERVAL);
    }
    return;
}

/*decide if a valid route can be updated to system table
   IPUCAST|IP6UCAST:
   �� �����ǰVRF��ԴVRF��ͬ����Ҫ��Э��ΪBGP�����ھӷǿ�
   �� �����ǰVRFΪ0��ԴVRF��0���Ƿ��������
   �� �����ǰVRF��0�����ǹǸɻ�����VRF�����VPN·�ɡ�
     ��ʱҪ����BGP���Ͳ����±�
   IPVPN|IP6VPN��
   �� �����±�ֻ�ܵ�����ӦVRFת��ΪIPUCAST�±�
   VPLS��
   �� VRF0�����±�
   �� ��0VRF��Ҫ��Э��ΪBGP������һ����0    
*/
u_int
bgp4_route_system_update_enable(tBGP4_ROUTE *p_route)
{
    tBGP4_PATH *p_path = p_route->p_path;

    switch (p_route->dest.afi) {
        case BGP4_PF_IPUCAST:
        case BGP4_PF_IP6UCAST:
             if (p_path->p_instance->vrf == p_path->origin_vrf)
             {
                 if ((p_route->proto == M2_ipRouteProto_bgp)
                    && (p_path->p_peer != NULL))
                 {
                     return TRUE;
                 }
             }
             else if (p_path->p_instance->vrf)
             {
                 if (p_route->proto == M2_ipRouteProto_bgp)
                 {
                     return TRUE;
                 }
             }
             break;

        case BGP4_PF_L2VPLS:
             if (p_path->p_instance->vrf 
                && (p_route->proto == M2_ipRouteProto_bgp))
             {
                 return TRUE;
             }
             break;
             
        case BGP4_PF_IPVPN:
        case BGP4_PF_IP6VPN:
             break;
             
        default:
             break;
    }
    return FALSE;
}

void 
bgp4_nexthop_check_timout(tBGP4_RIB *p_rib)
{
    tBGP4_PATH *p_path = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE route;
    tBGP4_PATH path;
    tBGP4_NEXTHOP_BLOCK nexthop;
    u_int vrf = p_rib->p_instance->vrf;
    u_int hwneed = 0;
    u_int i = 0;
    STATUS rc = 0;
    
    /*rib's system update not finished
     ,do not check nexthop change*/
    if (p_rib->system_check_need) 
    {
        bgp4_timer_start_ms(&p_rib->nexthop_timer, 500);
        return;
    }

    memset(&route, 0, sizeof(route));
    memset(&path, 0, sizeof(path));
    
    /*if nexthop change flag is not set,try to decide if
     some nexthop change exist*/
    if (p_rib->nexthop_changed == FALSE)
    {
        /*check all path in rib,ignore local route*/
        bgp4_avl_for_each(&p_rib->path_table, p_path)
        {
            if (p_path->p_peer == NULL)
            {
                continue;
            }
            
            if (p_path->p_direct_nexthop == NULL)
            {
                continue;
            }
            /*store current nexthop*/
            memcpy(&nexthop, p_path->p_direct_nexthop, sizeof(nexthop));
            /*reset current nexthop*/
            memset(p_path->p_direct_nexthop, 0, sizeof(tBGP4_NEXTHOP_BLOCK));
            
            /*get new nexthop and interface*/
            bgp4_direct_nexthop_calculate(p_path);

            /*next hop not changed,do not update route*/
            if (memcmp(&nexthop, p_path->p_direct_nexthop, sizeof(nexthop)) == 0)
            {
                continue;
            }
            /*nexthop change detected,set old nexthop for update*/
            if (p_path->p_old_nexthop == NULL)
            {
                p_path->p_old_nexthop = bgp4_malloc(sizeof(tBGP4_NEXTHOP_BLOCK), MEM_BGP_NEXTHOP);
            }
            memcpy(p_path->p_old_nexthop, &nexthop, sizeof(tBGP4_NEXTHOP_BLOCK));
            
            /*set nexthop change flag,and prepare to scan route from
              starting*/
            p_rib->nexthop_changed = TRUE;
            p_rib->p_change_route = bgp4_avl_first(&p_rib->rib_table);
        }
    }

    /*if nexthop not changed,do nothing*/
    if (p_rib->nexthop_changed == FALSE)
    {
        bgp4_timer_start(&p_rib->nexthop_timer, BGP4_IGP_SYNC_MAX_INTERVAL); 
        return;
    }

    /*start from current route*/
    for (p_route = p_rib->p_change_route;
         p_route;
         p_route = bgp4_avl_next(&p_rib->rib_table, p_route))
    {
        /*ignore deleted/filtered/unselected route*/
        if ((p_route->is_deleted == TRUE)
            || (p_route->system_filtered == TRUE)
            || (p_route->igp_sync_wait == TRUE)
            || (p_route->system_selected == FALSE))
        {
            continue;
        }
        p_path = p_route->p_path;
        
        /*ignore route without change and already processed*/
        if ((p_path->p_old_nexthop == NULL)
            || (p_path->p_old_nexthop->count == 0)
            || ((p_route->in_kernalx == 0) && (p_route->in_hardwarex == 0)))
        {
            continue;
        }
        /*build dummy route*/
        memcpy(&route.dest, &p_route->dest, sizeof(route.dest)); 
        route.p_path = &path;
        
        memcpy(&path.nexthop, &p_path->nexthop, sizeof(tBGP4_ADDR));

        path.p_direct_nexthop = p_path->p_old_nexthop;

        route.system_selected = FALSE;

        for (i = 0; i < path.p_direct_nexthop->count; i++)
        {
            hwneed = TRUE;

            /*delete current route from kernal*/
            if (BIT_LST_TST(&p_route->in_kernalx, i))
            {
                if (route.dest.afi == BGP4_PF_IPUCAST)
                {
                    rc = bgp4_sys_iproute_delete(vrf, &route, i, &hwneed);
                }
                else if (route.dest.afi == BGP4_PF_IP6UCAST)
                {
                    rc = bgp4_sys_ip6route_delete(vrf, &route, i, &hwneed);
                }    
                if (rc == VOS_ERR)
                {
                    /*record this route,*/
                    p_rib->p_change_route = p_route;
                    bgp4_timer_start_ms(&p_rib->nexthop_timer, 100);
                    return;
                }
                BIT_LST_SET(&p_route->in_kernalx, i);
    
                if (hwneed == FALSE)
                {
                    BIT_LST_SET(&p_route->in_hardwarex, i);
                }
            }
            
            if (BIT_LST_TST(&p_route->in_hardwarex, i))
            {
                /*insert into ipv4 or ipv6 msg buffer*/
                if (p_rib->p_hwmsg == NULL)
                {
                    p_rib->p_hwmsg = bgp4_malloc(1200, MEM_BGP_BUF);
                    if (p_rib->p_hwmsg == NULL)
                    {
                        /*record this route,*/
                        p_rib->p_change_route = p_route;
                        bgp4_timer_start_ms(&p_rib->nexthop_timer, 100);
                        return;
                    }
                }
    
                /*insert into message failed.stop processing*/
                if (bgp4_sys_msg_add(vrf, p_rib->p_hwmsg, &route, i) == VOS_ERR)
                {
                    /*record this route,*/
                    p_rib->p_change_route = p_route;
                    bgp4_timer_start_ms(&p_rib->nexthop_timer, 100);
                    return;
                }
                /*send msg finished*/
                BIT_LST_SET(&p_route->in_hardwarex, i);
            }
            /*time limit.TODO*/
        }
    }

    /*all old routes cleared,reset change flag and route*/
    p_rib->nexthop_changed = FALSE;
    p_rib->p_change_route = NULL;

    /*clear all path's old nexthop*/
    bgp4_avl_for_each(&p_rib->path_table, p_path)
    {
        if (p_path->p_old_nexthop)
        {
            bgp4_free(p_path->p_old_nexthop, MEM_BGP_NEXTHOP);
            p_path->p_old_nexthop = NULL;
        }
    }

    /*schedule add new nexthop route*/
    bgp4_avl_for_each(&p_rib->rib_table, p_route)
    {
        /*ignore deleted/filtered/unselected route*/
        if ((p_route->is_deleted == TRUE)
            || (p_route->system_filtered == TRUE)
            || (p_route->igp_sync_wait == TRUE)
            || (p_route->system_selected == FALSE))
        {
            continue;
        }
        /*schedule add if current is not in kernal*/
        if ((p_route->in_kernalx == 0)
            && (p_route->in_hardwarex == 0))
        {
            p_route->system_selected = FALSE;
            bgp4_schedule_system_add(p_route);
        }
    }
    if (bgp4_avl_count(&p_rib->path_table))
    {
        bgp4_timer_start(&p_rib->nexthop_timer, BGP4_IGP_SYNC_MAX_INTERVAL); 
    }
    return;
}

/*damp related*/
/*compare function of damp route*/
int32_t
bgp4_damp_route_lookup_cmp(
             tBGP4_DAMP_ROUTE *p1,
             tBGP4_DAMP_ROUTE *p2)
{
    int32_t rc = 0;
    rc = memcmp(&p1->dest, &p2->dest, sizeof(tBGP4_ADDR));
    if (rc)
    {
        return (rc > 0) ? 1 : -1;
    }
    rc = memcmp(&p1->peer, &p2->peer, sizeof(tBGP4_ADDR));
    if (rc)
    {
        return (rc > 0) ? 1 : -1;
    }
    return 0;
}

/*create damp route*/
tBGP4_DAMP_ROUTE *
bgp4_damp_route_create(
            tBGP4_RIB *p_rib, 
            tBGP4_ADDR *dest,
            tBGP4_ADDR *peer)
{
    tBGP4_DAMP_ROUTE *p_damp;
    p_damp = bgp4_malloc(sizeof(*p_damp), MEM_BGP_DAMP);
    if (p_damp)
    {
        memcpy(&p_damp->dest, dest, sizeof(tBGP4_ADDR));
        memcpy(&p_damp->peer, peer, sizeof(tBGP4_ADDR));
        p_damp->p_rib = p_rib;
        bgp4_timer_init(&p_damp->timer, bgp4_damp_route_update_timeout, p_damp);
        
        bgp4_avl_add(&p_rib->damp_table, p_damp);
    }
    return p_damp;
}

/*delete damp route*/
void 
bgp4_damp_route_delete(tBGP4_DAMP_ROUTE *p_damp)
{
    tBGP4_RIB *p_rib = p_damp->p_rib;
    bgp4_avl_delete(&p_rib->damp_table, p_damp);
    bgp4_timer_stop(&p_damp->timer);
    bgp4_free(p_damp, MEM_BGP_DAMP);
    return;
}

/*lookup damp route*/
tBGP4_DAMP_ROUTE *
bgp4_damp_route_lookup(
            tBGP4_RIB *p_rib, 
            tBGP4_ADDR *dest,
            tBGP4_ADDR *peer)
{
    tBGP4_DAMP_ROUTE damp;
    memcpy(&damp.dest, dest, sizeof(tBGP4_ADDR));
    memcpy(&damp.peer, peer, sizeof(tBGP4_ADDR));    
    return bgp4_avl_lookup(&p_rib->damp_table, &damp);
}

/*check against damp route,return TRUE if route is supressed,
  return FALSE if route is not supressed*/
u_int 
bgp4_damp_route_supressed(
           tBGP4_ROUTE *p_route,
           u_int feasible)
{
    tBGP4_DAMP_ROUTE *p_damp = NULL;
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_RIB *p_rib = p_instance->rib[p_route->dest.afi];    

    if (p_rib == NULL)
    {
        return TRUE;
    }

    /*ignore route without damp checking*/
    if ((p_route->p_path->p_peer == NULL)
        || (bgp4_peer_type(p_route->p_path->p_peer) == BGP4_IBGP))
    {
        return FALSE;
    }
    
    /*get current damp node*/
    p_damp = bgp4_damp_route_lookup(p_rib, &p_route->dest, &p_route->p_path->p_peer->ip);

    /*checking for feasible route*/
    if (feasible == TRUE)
    {
        /*no damp node exist,accept it now*/
        if (p_damp == NULL)
        {
            return FALSE;
        }
        /*damp node exist,but penalty value is greator than reuse
          flag,supress it*/
        if (p_damp->penalty >= gbgp4.penalty_reuse)
        {
            return TRUE;
        }
    }
    else
    {
        /*checking for withdraw route*/
        /*create new damp route if not exist*/
        if (p_damp == NULL)
        {
            p_damp = bgp4_damp_route_create(p_rib, &p_route->dest, &p_route->p_path->p_peer->ip);
            if (p_damp == NULL)
            {
                return FALSE;
            }
        }
        /*increase penalty value if route has not checked*/
        if (p_route->damp_checked == FALSE)
        {
            p_damp->penalty += BGP4_PENALTY_INCREASE;
            if (p_damp->penalty > gbgp4.penalty_max)
            {
                p_damp->penalty = gbgp4.penalty_max;
            }
            
            /*recalculate penalty decrease value*/
            p_damp->penalty_decrease = (p_damp->penalty/2)/(gbgp4.penalty_half_time/BGP4_PENALTY_UPDATE_INTERVAL);
            if (p_damp->penalty_decrease == 0)
            {
                p_damp->penalty_decrease = 1;
            }
            
            /*restart damp timer*/
            bgp4_timer_start(&p_damp->timer, BGP4_PENALTY_UPDATE_INTERVAL);

            /*do not add penalty in next loop*/
            p_route->damp_checked = TRUE;
        }
        
        /*if penalty exceed suppress value,supress it*/
        if (p_damp->penalty >= gbgp4.penalty_supress)
        {
            return TRUE;
        }
    }
    /*do not suppress it*/
    return FALSE;
}

/*damp route penalty checking timer*/
void
bgp4_damp_route_update_timeout(tBGP4_DAMP_ROUTE *p_damp)
{
    /*crease route's penalty*/
    p_damp->penalty -= p_damp->penalty_decrease;

    /*no penalty exist,delete damp route*/
    if (p_damp->penalty <= 0)
    {
        bgp4_damp_route_delete(p_damp);
        return;
    }
    /*restart timer*/
    bgp4_timer_start(&p_damp->timer, BGP4_PENALTY_UPDATE_INTERVAL);
    return;
}

#else
#include "bgp4com.h"

#if !defined(USE_LINUX) && !defined(WIN32)
#include "hwroute_nm.h"
#include "m2Ipv4Nm.h"
#endif

void bgp4_force_ip_rib_update(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_route);
void bgp4_force_route_ip_update(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_route);

tBGP4_ROUTE* bgp4_creat_route()
{
    tBGP4_ROUTE* p_route;
    p_route=(tBGP4_ROUTE*)bgp4_malloc(sizeof(tBGP4_ROUTE), MEM_BGP_ROUTE);

    if(p_route==NULL)
        return NULL;

    memset(p_route,0,sizeof(tBGP4_ROUTE));
    bgp4_lstinit(&p_route->node);
    return p_route;
}


/*get afi of a list of route*/
int bgp4_verify_nlri_afi(tBGP4_LIST *p_list)
{
        u_int afi = BGP4_PF_IPUCAST;
        tBGP4_LINK *p_link = NULL;
        tBGP4_LINK *p_next = NULL;

        LST_LOOP_SAFE(p_list, p_link, p_next, node, tBGP4_LINK)
        {
            afi = p_link->p_route->dest.afi;

            break;
        }

        return afi;
}

void bgp4_init_update_route_vector(tBGP4_ROUTE_VECTOR* p_vector,
    tBGP4_VPN_INSTANCE* p_dst_instance,
    u_char* rt_update,
    tBGP4_PEER *p_peer)
{
    tBGP4_ROUTE  *p_best_oldrt = NULL;
    tBGP4_ROUTE  *p_best_newrt = NULL;
    tBGP4_ROUTE  *p_route = NULL;
    u_int i = 0 ;

    /*scan all routes in this vector*/
    for (i = 0 ; i < p_vector->count ; i++)
    {
        p_best_oldrt =  bgp4_best_route_vector(p_vector);

        p_route = p_vector->p_route[i];

        if (p_route->deferral == TRUE)
        {
            p_route->deferral = FALSE;

            p_best_newrt = bgp4_best_route_vector(p_vector);

            if (p_route->is_deleted == TRUE)
            {
                bgp4_schedule_ip_update(p_route, BGP4_IP_ACTION_DELETE);
            }

            if (p_best_oldrt != p_best_newrt)/*best no change,and new best must be the route*/
            {
                if (p_best_oldrt)
                {
                    bgp4_schedule_ip_update(p_best_oldrt,BGP4_IP_ACTION_DELETE);
                }
                bgp4_schedule_ip_update(p_route, BGP4_IP_ACTION_ADD);
                bgp4_schedule_rib_update_with_range(p_dst_instance,p_best_oldrt, p_best_newrt);
            }
            continue;
        }
        if (p_best_oldrt != p_route)
        {
            continue ;
        }
        /*set update flag,do not check aggregate here*/
        bgp4_schedule_rib_update(p_dst_instance,NULL, p_route, p_peer);
        *rt_update = TRUE;
    }

    /*reset vector*/
    p_vector->count = 0 ;


}

void
bgp4_schedule_init_update(u_int af, tBGP4_PEER *p_peer )
{
    tBGP4_ROUTE_VECTOR vector;/*contain routes with same dest*/
    tBGP4_ROUTE  *p_scanroute = NULL;
    tBGP4_ROUTE  *p_next = NULL;
    u_char rt_update=FALSE;
    tBGP4_VPN_INSTANCE* p_dst_instance = NULL;
    u_char addrstr[64] = {0};


    p_dst_instance = p_peer->p_instance;
    if(p_dst_instance == NULL)
    {
        return;
    }

    if (!bgp4_af_support(p_peer, af))
    {
        bgp4_log(BGP_DEBUG_EVT,1,"peer %s does not support peer/route's af:%d/%d,do not send init update to this peer",
                    bgp4_printf_peer(p_peer,addrstr),p_peer->remote.af,af);
            return ;
    }

    memset(&vector, 0, sizeof(vector));

    /*scan all routes*/
    RIB_LOOP(&p_dst_instance->rib, p_scanroute, p_next)
    {

        /*afi must macth*/
        if (af != p_scanroute->dest.afi)
        {
            continue ;
        }


        /*ignore inactive summary route*/
        if (p_scanroute->summary_active != TRUE && p_scanroute->is_summary == TRUE)
        {
            continue;
        }

         /*if vector is not empty,and dest is not same,process this vector now*/
        if (vector.count && bgp4_prefixcmp(&vector.p_route[0]->dest, &p_scanroute->dest))
        {
            bgp4_init_update_route_vector(&vector,p_dst_instance,&rt_update,p_peer);
        }
        /*add route to this vector*/
        vector.p_route[vector.count++] = p_scanroute;
    }
    /*last route vector,which has not been update in the rib-loop*/
    if(vector.count)
    {
        bgp4_init_update_route_vector(&vector,p_dst_instance,&rt_update,p_peer);
    }

    /*if no route been scheduled,check if EOB should been sent*/
    if ((rt_update == FALSE)
            && (p_peer->state == BGP4_NS_ESTABLISH))
    {
        /*grace check*/
        if(gBgp4.gr_enable == 1
            &&p_peer->remote.reset_enable == 1)
        {
            /*send end-of-rib at last,if it has not been sent before*/
            if(p_peer->rib_end_af!=0
                && !af_isset(p_peer->send_rib_end_af,af))
            {
                bgp4_send_rib_end(p_peer, af) ;

                /*if all end-of-rib for each af has been sent*/
                if(p_peer->send_rib_end_af == p_peer->send_capability)
                {
                    p_peer->send_rib_end = 1;

                    /*end of this peer,clear its total update routes num*/
                    p_peer->total_update = 0;
                }
            }
        }
        else/*normal*/
        {
            /*send end-of-rib at last,if it has not been sent before*/
            if(!af_isset(p_peer->send_rib_end_af,af))
            {
                bgp4_send_rib_end(p_peer, af);

                /*end of address family loop,all end-of-rib sent*/
                if(p_peer->send_rib_end_af == p_peer->send_capability)
                {
                    p_peer->send_rib_end = 1;

                    /*end of this peer,clear its total update routes num*/
                    p_peer->total_update = 0;
                }
            }
        }
    }

    bgp4_log(BGP_DEBUG_UPDATE,1,"bgp peer->send rib end af %d,bgp peer->send capability %d",
        p_peer->send_rib_end_af,p_peer->send_capability);

    return ;
}

/*end connection for neighbor*/
void bgp4_end_peer_route (u_int af, tBGP4_PEER *p_peer,u_char stale_flag,u_char send_flag)
{
    tBGP4_PATH *p_path = NULL ;
    tBGP4_PATH *p_npath = NULL ;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_nroute = NULL;
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_ROUTE *p_new_best = NULL;
    tBGP4_ADDR ip;
    tBGP4_ROUTE_VECTOR vector;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ROUTE_VECTOR ecmp_vector;
    u_char zerobit[BGP4_MAX_PEER_ID/8] = {0};
    u_int i = 0;

    LST_LOOP_SAFE(&gBgp4.attr_list[af], p_path, p_npath, node, tBGP4_PATH)
    {
        if (p_path->p_peer != p_peer)
        {
            continue ;
        }

        p_instance = p_path->p_instance;
        if(p_instance == NULL)
        {
            continue;
        }
        /*check route*/
        LST_LOOP_SAFE(&p_path->route_list, p_route, p_nroute, node, tBGP4_ROUTE)
        {
            if ((af != p_route->dest.afi))
            {
                continue ;
            }

            /*for GR special*/
            if(stale_flag && p_route->stale == 0)
            {
                continue;
            }

            /*avoid reduplicated deletion*/
            if(p_route->is_deleted == TRUE)
            {
                gBgp4.duplicate_deleted_route_count++;
                continue;
            }

            memcpy(&ip, &p_route->dest, sizeof(ip));

            bgp4_rib_lookup_vector(&p_instance->rib, &ip, &vector);
            p_best = bgp4_best_route_vector(&vector);

            if (p_route->igp_sync_wait == TRUE)
            {
                /*delete directly*/
                bgp4_rib_delete(&p_instance->rib, p_route);
                continue ;
            }

            /*set delete flag*/
            p_route->is_deleted = TRUE ;
            
            /*get new best route*/
            p_new_best = bgp4_best_route_vector(&vector);

            /*for GR special*/
            if(send_flag ||
                p_peer->reset_role != BGP4_GR_ROLE_RESTART)
            {
                bgp4_schedule_rib_update_with_range(p_instance,p_best,p_new_best);
            }

            /*update ip table,for ECMP*/
            if(p_route->ip_table)
            {
                bgp4_schedule_ip_update(p_route,BGP4_IP_ACTION_DELETE);

                /*get new ECMP routes,update ip table*/
                memset(&ecmp_vector,0,sizeof(ecmp_vector));
                bgp4_best_ECMP_route(&vector,&ecmp_vector);
    
                for(i = 0;i < ecmp_vector.count;i++)
                {
                    if(ecmp_vector.p_route[i])
                    {
                        bgp4_schedule_ip_update(ecmp_vector.p_route[i],BGP4_IP_ACTION_ADD);
                    }
                }
            }
            else
            {   
                    if ((memcmp(p_route->withdraw_bits, zerobit, sizeof(p_route->withdraw_bits)) == 0)
                        && (memcmp(p_route->update_bits, zerobit, sizeof(p_route->update_bits)) == 0))
                    {
                        /*delete directly*/
                        bgp4_rib_delete(&p_instance->rib, p_route);
                    }
            }
        }
    }

    return ;
}
#if 1
/*special process for the case:there is only one rib node for this route*/
void
bgp4_update_single_withdraw_route(
                    tBGP4_PEER *p_peer,
                    tBGP4_VPN_INSTANCE*p_instance,
                    tBGP4_ROUTE *p_old,
                    tBGP4_ROUTE_VECTOR *p_vector)
{
     tBGP4_RIB *p_root = NULL;

     p_root = &p_instance->rib;

     /*old route must exist,and delete flag must not set*/

     /*if it is not the best one,delete directly*/
     if (bgp4_best_route_vector(p_vector) != p_old)
     {
        bgp4_rib_delete(p_root, p_old);
        return ;
     }


     /*if ip add action finished or not done,do not update any,else must wait for finish*/
     if (p_old->ip_action == BGP4_IP_ACTION_ADD)
     {
     #if 0
    /*ip and rtmsg both not send,delete directly*/
         if ((p_old->ip_finish == FALSE) && (p_old->rtmsg_finish == FALSE))
         {
            bgp4_rib_delete(p_root, p_old);
            return ;
         }
     #endif
         /*wait ip update finish,and re-scedule delete operation*/
         bgp4_force_ip_rib_update(p_instance,p_old);
     }

     if(p_vector->p_route[0]->deferral == TRUE)/*GR in progress,delay following operation */
     {
          p_old->deferral = TRUE;
          return;
     }

     /*schedule delete*/
     bgp4_schedule_ip_update(p_old, BGP4_IP_ACTION_DELETE);

     /*set delete flag*/
     p_old->is_deleted = TRUE ;

     /*schedule send withdraw update,even if current update not sent*/
     memset(p_old->update_bits, 0, sizeof(p_old->update_bits));
     memset(p_old->withdraw_bits, 0, sizeof(p_old->withdraw_bits));

     bgp4_schedule_rib_update_with_range(p_instance,p_old, NULL);
     return ;
}
#endif
void bgp4_update_withdraw_route(
                    tBGP4_PEER *p_peer,
                    tBGP4_VPN_INSTANCE* p_instance,
                    tBGP4_ROUTE *p_route)
{
    tBGP4_ROUTE *p_old = NULL;
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_ROUTE *p_new_best = NULL;
    tBGP4_ROUTE_VECTOR vector;
    tBGP4_ROUTE_VECTOR ecmp_vector;
    tBGP4_RIB* p_root = NULL;
    u_char addr[64];
    u_char zerobit[BGP4_MAX_PEER_ID/8] = {0};
    u_int i = 0;

    if(p_peer)
    {
        p_peer->rx_with_route++;
    }

    if(p_instance == NULL || p_route == NULL)
    {
        return;
    }


    p_root = &p_instance->rib;

    /*get all route with same dest*/
    bgp4_rib_lookup_vector(p_root, &p_route->dest, &vector);

    /*get matched route*/
    p_old = bgp4_special_route_vector(&vector, p_route);
    /*if current route exist,or delete flag is set,do nothing*/
    if ((p_old == NULL) || p_old->is_deleted)
    {
        return;
    }
#if 1
    /*special process:this the only one route for the same dest*/ 
    if (vector.count == 1)
    {
        bgp4_update_single_withdraw_route(p_peer, p_instance, p_old, &vector);
        return ;
    }
#endif
    /*get current best route*/
    p_best = bgp4_best_route_vector(&vector) ;

    /*case : if best not changed,and isn't this route, delete form rib directly*/
    if (p_old->igp_sync_wait == TRUE)
    {
        bgp4_rib_delete(p_root, p_old);
        return ;
    }

    /*try to finish current update task*/
    if (p_old && (p_old->is_deleted == FALSE))
    {
        /*clear old route's update flag*/
        memset(p_old->withdraw_bits, 0, sizeof(p_old->withdraw_bits));
        memset(p_old->update_bits, 0, sizeof(p_old->update_bits));


        /*finish ip update*/
        bgp4_force_ip_rib_update(p_instance,p_old);

    }

    /*set route's delete flag*/
    p_old->is_deleted = TRUE ;

    /*get new best route*/
    p_new_best = bgp4_best_route_vector(&vector) ;

    /*case : best route changed,in this case, old route must the old best*/
    if ((p_best != p_new_best) && (p_best == p_old))
    {
        if(p_route->deferral == TRUE)/*GR in progress,delay following operation */
        {
            p_best->deferral = TRUE;
            return;
        }

        bgp4_schedule_rib_update_with_range(p_instance,p_best,p_new_best);

    }
    /*update ip table,for ECMP*/
    if(p_old->ip_table)
    {
        if(p_route->deferral == TRUE)/*GR in progress,delay following operation */
        {
            p_old->deferral = TRUE;
            return;
        }
        bgp4_schedule_ip_update(p_old,BGP4_IP_ACTION_DELETE);

        /*get new ECMP routes,update ip table*/
        memset(&ecmp_vector,0,sizeof(ecmp_vector));
        bgp4_best_ECMP_route(&vector,&ecmp_vector);

        for(i = 0;i < ecmp_vector.count;i++)
        {
            if(ecmp_vector.p_route[i])
            {
                bgp4_schedule_ip_update(ecmp_vector.p_route[i],BGP4_IP_ACTION_ADD);
            }
        }
    }
    else
    {
        if ((memcmp(p_old->withdraw_bits, zerobit, sizeof(p_old->withdraw_bits)) == 0)
            && (memcmp(p_old->update_bits, zerobit, sizeof(p_old->update_bits)) == 0))
        {
            bgp4_rib_delete(p_root,p_old);
        }
    }
    return ;
}
#if 1
/*special process for the case:there is only one rib node for this route*/
void
bgp4_update_single_new_route(
                    tBGP4_PEER *p_peer,
                    tBGP4_VPN_INSTANCE*p_instance,
                    tBGP4_ROUTE *p_old,
                    tBGP4_ROUTE *p_route,
                    tBGP4_ROUTE_VECTOR *p_vector)
{
    tBGP4_ROUTE *p_current_best = NULL;
    tBGP4_ROUTE *p_new_best = NULL;
    u_int ip_finish = 0 ;
    u_int delete_flag = 0 ;
    u_int ip_action = BGP4_IP_ACTION_NONE;
    tBGP4_RIB *p_root = NULL;

    p_root = &p_instance->rib;

    /*old route must exist*/

    /*route delete flag not set,and path are same,do nothing*/
    if ((p_old->is_deleted == FALSE)
        && (bgp4_same_path(p_route->p_path, p_old->p_path) == TRUE))
    {
        /*clear stale flag,if same route exist*/
        if (p_old->stale)
        {
            p_old->stale = 0;
        }
        return;
    }
#if 0
    /*decide old and new route's best state*/
    delete_flag = p_old->is_deleted;
    p_old->is_deleted = FALSE ;
    p_current_best = bgp4_best_route_vector(p_vector) ;

    p_vector->p_route[0] = p_route;
    p_new_best = bgp4_best_route_vector(p_vector) ;

    /*process any pending ip update*/
    if (p_old->ip_action == BGP4_IP_ACTION_NONE)
    {
        ip_finish = 1;
    }
    else if (p_old->ip_finish == FALSE
            && p_old->rtmsg_finish == FALSE
            && p_old->mpls_notify_finish == FALSE)
    {
        ip_finish = 0;
    }
    else
    {
        bgp4_force_route_ip_update(p_instance,p_old);
        ip_finish = 1;
    }

    /*replace old route by new route*/
    bgp4_rib_delete(p_root, p_old);

    /*replace by new route*/
    bgp4_rib_add (p_root, p_route);

    /*if old route is not the best,just replace it*/
    if (p_current_best == NULL)
    {
        /*if new route is not the best,do nothing,else schedule ip and msg udpate*/
        if (p_new_best == p_route)
        {
            /*update ip table,add old best's ip route*/
            bgp4_schedule_ip_update(p_route, BGP4_IP_ACTION_ADD);

            /*schedule send withdraw of old best to new best's peer
            aggregate operation is considered in the function
            */
            bgp4_schedule_rib_update_with_range(p_instance,NULL, p_route);
        }
        return ;
    }

    /*schedule update message,if new route is best,schedule update,else schedule withdraw
        do not consider old route state now
    */
    if (p_new_best == p_route)
    {
        bgp4_schedule_rib_update_with_range(p_instance,NULL, p_route);
    }
    else
    {
        bgp4_schedule_rib_update_with_range(p_instance,p_route, NULL);
    }

    /*decide ip action*/
    /*route is not in delete state*/
    if (delete_flag == FALSE)
    {
        /*new route is the best,if current ip add not peformed,add it,*/
        if (p_new_best == p_route)
        {
            /*if ip update not performed,schedule it*/
            if (ip_finish == 0)
            {
                ip_action = BGP4_IP_ACTION_ADD;
            }
        }
        else
        {
            /*there is no new best route, if ip update performed,schedule to delete it*/
            if (ip_finish == 1)
            {
                ip_action = BGP4_IP_ACTION_DELETE;
            }
        }
    }
    else/*route is in delete state,path changed or not changed*/
    {
            /*new route is the best,if already delete,add again*/
            if (p_new_best == p_route)
            {
                if (ip_finish == 1)
                {
                    ip_action = BGP4_IP_ACTION_ADD;
                }
            }
            else
            {
                /*new route is not the best,if not delete,delete it*/
                if (ip_finish == 0)
                {
                    ip_action = BGP4_IP_ACTION_DELETE;
                }
            }
        }
    if (ip_action != BGP4_IP_ACTION_NONE)
    {
        bgp4_schedule_ip_update(p_route, ip_action);
    }
    return ;
#endif
    if(p_old->ip_table)
    {
        bgp4_schedule_ip_update(p_old, BGP4_IP_ACTION_DELETE);
        bgp4_force_route_ip_update(p_instance,p_old);
    }
    
    /*replace old route by new route*/
    bgp4_rib_delete(p_root, p_old);

    /*replace by new route*/
    bgp4_rib_add (p_root, p_route);

    bgp4_schedule_ip_update(p_route, BGP4_IP_ACTION_ADD);

    bgp4_schedule_rib_update_with_range(p_instance,NULL, p_route);
    
    return ;
}
#endif
void bgp4_update_new_route(tBGP4_PEER *p_peer,
                            tBGP4_VPN_INSTANCE* p_instance,
                            tBGP4_ROUTE *p_route)
{
    tBGP4_ROUTE_VECTOR vector;
    tBGP4_PATH *p_path = NULL;
    tBGP4_ROUTE *p_old = NULL;
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_ROUTE *p_new_best = NULL;
    u_int i = 0 ;
    tBGP4_RIB *p_root = NULL;
    tBGP4_ROUTE_VECTOR ecmp_vector = {0};
    u_int flag = 0;

    if(p_peer)
    {
        p_peer->rx_fea_route++;
    }

    if(p_instance == NULL || p_route == NULL)
    {
        return;
    }

    p_root = &p_instance->rib;
    p_path = p_route->p_path;

    if(p_path == NULL)
        return;

    if(p_path->p_peer==NULL)
        return;

    /*clear route's delete and withdraw flag*/
    p_route->is_deleted = FALSE ;
    p_route->ip_action = BGP4_IP_ACTION_NONE ;
    memset(p_route->withdraw_bits, 0, sizeof(p_route->withdraw_bits));
    memset(p_route->update_bits, 0, sizeof(p_route->update_bits));

    /*get all route with same dest*/
    bgp4_rib_lookup_vector(p_root, &p_route->dest, &vector);

    /*lookup current route*/
    p_old = bgp4_special_route_vector(&vector, p_route);
#if 1
    /*special case :this is the only one route with same dest*/
    if (p_old && (vector.count == 1))
    {
        bgp4_update_single_new_route(p_peer, p_instance, p_old, p_route, &vector);
        return ;
    }
#endif
    /*get best route before add new route*/
    p_best = bgp4_best_route_vector(&vector) ;

    memset(&ecmp_vector,0,sizeof(ecmp_vector));
    bgp4_best_ECMP_route(&vector,&ecmp_vector);
    /*same path do nothing*/
    if (p_old && (bgp4_same_path(p_path, p_old->p_path) == TRUE))
    {
        /*clear stale flag,if same route exist*/
        if (p_old->stale)
        {
            p_old->stale = 0 ;
        }

        /*clear delete flag*/
        if (p_old->is_deleted==FALSE)
        {
            return ;
        }

        bgp4_force_ip_rib_update(p_instance,p_old);
        p_old->is_deleted=FALSE;
        return;
    }

    /*try to finish current update task*/
    if (p_old)
    {
        bgp4_force_ip_rib_update(p_instance,p_old);
    }

    /*current route not exist,just add to rib*/
    if (p_old == NULL)
    {
        bgp4_rib_add (p_root, p_route);
    }
    else /*rib exist and changed,delete current and add new one*/
    {
        /*delete old route from vector*/
        for (i = 0 ; i < vector.count ; i++)
        {
            if (vector.p_route[i] == p_old)
            {
                vector.p_route[i] = NULL;
                break;
            }
        }
        /*old route should delete later*/
        if(p_best == p_old)
        {
            /*schedule send withdraw update,even if current update not sent*/
            memset(p_old->update_bits, 0, sizeof(p_old->update_bits));
            memset(p_old->withdraw_bits, 0, sizeof(p_old->withdraw_bits));

            /*set delete flag*/
            p_old->is_deleted = TRUE;
#if 0
            /*force ip route deletion to be done,and send withdrawing immediately*/
            bgp4_schedule_ip_update(p_old, BGP4_IP_ACTION_DELETE);
#endif
            bgp4_schedule_rib_update_with_range(p_instance,p_old,NULL);

            bgp4_force_ip_rib_update(p_instance,p_old);

            p_best = p_route;

        }
        if(p_old->ip_table)
        {
            /*set delete flag*/
            p_old->is_deleted = TRUE;

            /*force ip route deletion to be done,and send withdrawing immediately*/
            bgp4_schedule_ip_update(p_old, BGP4_IP_ACTION_DELETE);

            bgp4_force_ip_rib_update(p_instance,p_old);

        }
        /*remove route from rib*/
        bgp4_rib_delete(p_root, p_old);

        /*replace by new route*/
        bgp4_rib_add (p_root, p_route);
    }

    /*update ip table,for ECMP*/
    if (ecmp_vector.count == 0 ||
            bgp4_route_ECMP_priority_cmp(p_route, ecmp_vector.p_route[0]) == 0)
     {
        bgp4_schedule_ip_update(p_route, BGP4_IP_ACTION_ADD);
        flag = 1;
     }
     else if(bgp4_route_ECMP_priority_cmp(p_route, ecmp_vector.p_route[0]) > 0)
     {
        for(i = 0;i < ecmp_vector.count;i++)
        {
            if(ecmp_vector.p_route[i])
            {
                bgp4_schedule_ip_update(ecmp_vector.p_route[i], BGP4_IP_ACTION_DELETE);
            }
        }

        bgp4_schedule_ip_update(p_route, BGP4_IP_ACTION_ADD);
        flag = 1;
     }

    /*insert new route to vector*/
    if (vector.count < BGP4_MAX_ECMP)
    {
        vector.p_route[vector.count] = p_route;
        vector.count++;
    }

    /*get new best route*/
    p_new_best = bgp4_best_route_vector(&vector) ;


    /*check if need IGP sync*/
    if(gBgp4.sync_enable == TRUE &&
        bgp4_peer_type(p_peer) != BGP4_EBGP)
    {
        if(bgp4_sysroute_lookup(p_instance->instance_id,p_new_best) == FALSE)
        {
            p_new_best->igp_sync_wait = TRUE;
            if(flag)
            {
                p_route->igp_sync_wait = TRUE;
            }
        }
        else
        {
            p_new_best->igp_sync_wait = FALSE;
            if(flag)
            {
                p_route->igp_sync_wait = FALSE;
            }
        }
    }


    /*schedule send withdraw of old best to new best's peer
    aggregate operation is considered in the function
    */
    if (p_new_best != p_best)
    {
        bgp4_schedule_rib_update_with_range(p_instance,p_best,p_new_best);
    }


    return ;
}

/*update rib according to new rxd routes*/
void bgp4_update_rib (tBGP4_PEER *p_peer,tBGP4_LIST *p_flist, tBGP4_LIST *p_wlist)
{
    tBGP4_LINK *p_link = NULL;
    u_char peer[64];
    tBGP4_ROUTE* p_new_route = NULL;
    tBGP4_ROUTE* p_first_route= NULL;
    u_int input_vrf_id = p_peer->p_instance->instance_id;
    u_int vrf_id_array[BGP4_MAX_VRF_ID] = {0};
    u_int vrf_num = 1;
    u_int i = 0;
    tBGP4_VPN_INSTANCE* p_src_instance = p_peer->p_instance;
    tBGP4_VPN_INSTANCE* p_dst_instance = NULL;
    u_int direction = 0;


    bgp4_log(BGP_DEBUG_UPDATE,1,"update RIB from vrf %d peer %s,w=%d,f=%d",
                    input_vrf_id,
                    bgp4_printf_peer(p_peer,peer),
                    bgp4_lstcnt(p_wlist),
                    bgp4_lstcnt(p_flist));


    p_link = (tBGP4_LINK *)bgp4_lstfirst(p_wlist);
    if(p_link)
    {
        LST_LOOP(p_wlist, p_link, node, tBGP4_LINK)/*withdraw routes may differ in path*/
        {
            /*withdraw routes have no path,different vpn routes may be mixed in one packet*/
            vrf_num = bgp4_mpls_vpn_get_vrf_id_list(p_link->p_route,0,p_src_instance,&direction,vrf_id_array);
            for(i = 0;i < vrf_num;i++)
            {
                p_dst_instance = bgp4_vpn_instance_lookup(vrf_id_array[i]);
                if(p_dst_instance == NULL)
                    break;

                /*for 6PE local*/
                if(direction == MPLS_6PE_LOCAL_ROUTE)
                {
                    bgp4_gr_update_deferral_route(p_link->p_route);

                    bgp4_update_withdraw_route(p_peer,p_dst_instance, p_link->p_route);

                }

                p_new_route = bgp4_mpls_rebuild_local_vpn_route(input_vrf_id,direction,p_dst_instance,p_link->p_route);

                if(p_new_route == NULL)
                {
                    continue;
                }

                bgp4_gr_update_deferral_route(p_new_route);

                bgp4_update_withdraw_route(p_peer,p_dst_instance, p_new_route);

                if(p_new_route != p_link->p_route)/*rebuild withdraw route should be released here*/
                {
                    bgp4_release_route(p_new_route);
                }

            }

        }
    }

    p_link = (tBGP4_LINK *)bgp4_lstfirst(p_flist);
    if (p_link)
    {
        p_first_route = p_link->p_route;

        /*validate path for feasible routes*/
        if(p_first_route == NULL ||
            bgp4_verify_path(p_peer, p_first_route->p_path) != VOS_OK)
        {
            bgp4_log(BGP_DEBUG_RT,1,"update RIB fatal VOS_ERR,p_flist first link with no route, OR path VOS_ERR");
            return;
        }

        LST_LOOP(p_flist, p_link, node,  tBGP4_LINK)
        {

            /*check origin import route policy*/
            if(bgp4_check_route_policy(p_link->p_route,p_peer,BGP4_POLICY_IMPORT_DIRECTION) == BGP4_ROUTE_DENY)
            {
                {
                    u_char route_str[64] = {0};

                    bgp4_log(BGP_DEBUG_RT,1,"bgp4 update new route, route %s is filtered by bgp4 check route policy",
                            bgp4_printf_route(p_link->p_route,route_str));
                }
                gBgp4.import_filtered_route_count ++;
                return;
            }

            /*feasible routes in path may come from different vpn*/
            vrf_num = bgp4_mpls_vpn_get_vrf_id_list(p_link->p_route,1,p_src_instance,&direction,vrf_id_array);
            for(i = 0;i < vrf_num;i++)
            {
                p_dst_instance = bgp4_vpn_instance_lookup(vrf_id_array[i]);
                if(p_dst_instance == NULL)
                {
                    break;
                }

                /*for 6PE local*/
                if(direction == MPLS_6PE_LOCAL_ROUTE)
                {
                    bgp4_gr_update_deferral_route(p_link->p_route);

                    bgp4_update_new_route(p_peer, p_dst_instance, p_link->p_route);

                }

                p_new_route = bgp4_mpls_rebuild_local_vpn_route(input_vrf_id,direction,p_dst_instance,p_link->p_route);

                if(p_new_route == NULL)
                {
                    continue;
                }
                bgp4_gr_update_deferral_route(p_new_route);

                bgp4_update_new_route(p_peer, p_dst_instance, p_new_route);

                if(p_new_route != p_link->p_route &&
                        p_new_route->refer == 0)
                {
                    bgp4_release_route(p_new_route);
                }

            }
        }
    }

    return;
}

/*decide if update to peer*/
int  bgp4_send_check(tBGP4_PEER *p_peer,
                                tBGP4_ROUTE   *p_route)
{
    u_int type = bgp4_peer_type(p_peer);
    u_char route[64]= {0};
    u_char peer[64]= {0};

    bgp4_log(BGP_DEBUG_UPDATE,1,"check interface need update %s to %s",
        bgp4_printf_route(p_route,route),
        bgp4_printf_peer(p_peer,peer));

    if(p_route->dest.afi==BGP4_PF_IPUCAST)
    {
        if(bgp4_prefixmatch(p_route->dest.ip, p_route->p_path->nexthop.ip, p_route->dest.prefixlen))
        {
            u_char route_str[64] = {0};
            u_char next_hop_str[64] = {0};
            bgp4_log(BGP_DEBUG_RT,1,"bgp4 send check route %s next hop %s is not in the same network",
                    bgp4_printf_route(p_route,route_str),
                    bgp4_printf_addr(&p_route->p_path->nexthop,next_hop_str));
            return FALSE;
        }
    }
    else if(p_route->dest.afi==BGP4_PF_IP6UCAST)
    {
        u_char route_str[64] = {0};
        u_char next_hop_str[64] = {0};
        if(bgp4_prefixmatch(p_route->dest.ip, p_route->p_path->nexthop_global.ip, p_route->dest.prefixlen))
        {
            bgp4_log(BGP_DEBUG_RT,1,"bgp4 send check route %s next hop %s is not in the same network",
                    bgp4_printf_route(p_route,route_str),
                    bgp4_printf_addr(&p_route->p_path->nexthop_global,next_hop_str));
            return FALSE;
        }
    }

    if (is_bgp_route(p_route))
    {
        if ( p_route->p_path->p_peer == p_peer)
        {
            bgp4_log(BGP_DEBUG_UPDATE,1,"route is send from peer self");
            return FALSE;
        }
        /*If the peer's AS or AS in route's as-path list,release it*/
        if (type == BGP4_EBGP)
        {
            if (bgp4_asseq_exist(&p_route->p_path->aspath_list, gBgp4.asnum) == TRUE)
            {
                bgp4_log(BGP_DEBUG_UPDATE,1,"route as path contain local as");
                return FALSE;
            }
            if (p_peer->as_substitute_enable == 0 &&
                    bgp4_asseq_exist(&p_route->p_path->aspath_list, p_peer->remote.as ) == TRUE)
            {
                bgp4_log(BGP_DEBUG_UPDATE,1,"route as path contain peer self as");
                return FALSE;
            }
        }
        /*check community flag*/
        if (p_route->p_path->flags.notto_external && (type == BGP4_EBGP))
        {
            bgp4_log(BGP_DEBUG_UPDATE,1,"route flags notto external");
            return FALSE;
        }

        if (p_route->p_path->flags.notto_internal && (type == BGP4_IBGP))
        {
            bgp4_log(BGP_DEBUG_UPDATE,1,"route flags notto internal");
            return FALSE;
        }

        /* Route learned from the Internal BGP peer should not be
        * advertised to another internal BGP peer.
        */
        if (p_route->p_path->p_peer
                && is_ibgp_peer(p_route->p_path->p_peer)
                && is_ibgp_peer(p_peer))
        {
            /*route reflector function*/
            if (gBgp4.is_reflector != TRUE)
            {
                bgp4_log(BGP_DEBUG_UPDATE,1,"self is not reflector,not update to internal peer");
                return FALSE;
            }

            if (bgp4_check_clusterlist(p_route->p_path) != VOS_OK)
            {
                bgp4_log(BGP_DEBUG_UPDATE,1,"route loop detected");
                return FALSE;
            }

            if (bgp4_check_originid(p_route->p_path) != VOS_OK)
            {
                bgp4_log(BGP_DEBUG_UPDATE,1,"route path origin self");
                return FALSE;
            }
            if (p_route->p_path->p_peer != NULL)
            {
                bgp4_log(BGP_DEBUG_UPDATE,1,"local is reflector,Speer is %d,Dpeer is %d",
                            p_route->p_path->p_peer->rr_client,
                            p_peer->rr_client);

                /*none client route only reflect to client peer*/
                if (p_route->p_path->p_peer->rr_client != TRUE)
                {
                    return  p_peer->rr_client ;
                }
                else
                {
                    return TRUE;
                }
            }
        }
    }
    else
    {
        if(p_route->dest.afi==BGP4_PF_IPUCAST)
        {
            if (bgp_ip4(p_route->p_path->nexthop.ip)==bgp_ip4(p_peer->remote.ip.ip))
            {
                bgp4_log(BGP_DEBUG_UPDATE,1,"route is sent from peer self,no update need");
                return FALSE;
            }
        }
        else if(p_route->dest.afi==BGP4_PF_IP6UCAST)
        {

        }
    }


    /*aggr check*/
    if (is_ibgp_peer(p_peer) && p_route->is_summary)
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"aggr ibgp route,no update need");
        return FALSE;
    }
    else if (p_route->filter_by_range)
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,"aggr route,no update need");
        return FALSE;
    }

    /*add 6pe label*/
    if(p_peer->send_label == 0 &&
        p_route->dest.afi == BGP4_PF_IP6LABEL)
    {
        p_route->vpn_label = 0;/*if not enable label send,clear route label*/
    }

    return TRUE ;
}

#if 0
void  bgp4_send_update(tBGP4_LIST* p_peer_list,tBGP4_LIST *p_flist, tBGP4_LIST *p_wlist)
{
    tBGP4_LIST new_wlist;
    tBGP4_LIST new_flist;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_LINK *p_link = NULL;
    tBGP4_ROUTE  *p_route = NULL;
    u_int af = BGP4_PF_IPUCAST;
    u_char addrstr[64] = {0};

    /*no route to send*/
    if (bgp4_lstfirst(p_flist)== NULL && bgp4_lstfirst(p_wlist) == NULL)
    {
        return;
    }

    if (bgp4_lstfirst(p_flist))
    {
        af = bgp4_verify_nlri_afi(p_flist);
    }
    else
    {
        af = bgp4_verify_nlri_afi(p_wlist);
    }

    LST_LOOP(p_peer_list, p_peer, node, tBGP4_PEER )
    {
        if (p_peer->state != BGP4_NS_ESTABLISH)
        {
            bgp4_log(1,1,"peer %s has not been established,do not send update to this peer", bgp4_printf_peer(p_peer,addrstr));
            continue;
        }
  #if 0
        /*tcp connection must exist*/
        if (p_peer->sock <= 0 ||getsockname(p_peer->sock,(struct sockaddr*)&sa, &len) != VOS_OK)
        {
            bgp4_log(1,1,"peer %s socket is closed,do not send update", bgp4_peerstring(p_peer,addrstr));
            continue;
        }
  #endif
        /*peer must support this af*/
        if (!bgp4_af_support(p_peer, af))
        {
            bgp4_log(1,1,"peer %s does not support peer/route's af:%d/%d,do not send update to this peer",
                bgp4_printf_peer(p_peer,addrstr),p_peer->remote.af,af);
            continue;
        }

        if (gBgp4.dbg_flag & BGP_DEBUG_UPDATE)
        {
            bgp4_log(1,1,"prepare send update to peer %s", bgp4_printf_peer(p_peer,addrstr));
        }
        bgp4_lstinit (&new_wlist);
        bgp4_lstinit (&new_flist);

        LST_LOOP(p_wlist, p_link, node, tBGP4_LINK)
        {
            p_route = p_link->p_route;
            if (bgp4_send_check(p_peer, p_route) == TRUE )
            {
                bgp4_rtlist_add (&new_wlist, p_route);
            }
        }

        LST_LOOP(p_flist, p_link, node, tBGP4_LINK)
        {
            p_route = p_link->p_route;
            if (bgp4_send_check(p_peer, p_route) == TRUE )
            {
                bgp4_rtlist_add(&new_flist, p_route);
            }
        }

        bgp4_send_update_to_peer( p_peer, af, &new_flist, &new_wlist);

        /*release sending list*/

        bgp4_rtlist_clear(&new_wlist);
        bgp4_rtlist_clear(&new_flist);
    }
    return ;
}
#endif
/*search route in a list of route*/
tBGP4_LINK *bgp4_rtlist_lookup(tBGP4_LIST *p_list, tBGP4_ROUTE *p_route)
{
    tBGP4_LINK *p_link = NULL;

    LST_LOOP(p_list, p_link, node,  tBGP4_LINK)
    {
        /*only compare dest???*/
        if ((bgp4_prefixcmp(&p_link->p_route->dest, &p_route->dest) == 0)
            && (p_link->p_route->proto == p_route->proto)
            && (p_link->p_route->p_path->p_peer == p_route->p_path->p_peer))
        {
            return p_link ;
        }
    }

    return  NULL;
}

tBGP4_LINK *bgp4_rtlist_add(tBGP4_LIST *p_list,
                            tBGP4_ROUTE *p_route)
{
    tBGP4_LINK *p_link = NULL;

    p_link = (tBGP4_LINK*)bgp4_malloc(sizeof(tBGP4_LINK),MEM_BGP_LINK);
    if (p_link == NULL)
    {
        return NULL;
    }

    p_link->p_route = p_route;
    p_route->refer++;

    bgp4_lstnodeinit(&p_link->node);
    bgp4_lstadd(p_list, &p_link->node);
    return p_link;
}

tBGP4_LINK *bgp4_rtlist_add_tail(tBGP4_LIST *p_list,
                            tBGP4_ROUTE *p_route)
{
    tBGP4_LINK *p_link = NULL;

    p_link = (tBGP4_LINK*)bgp4_malloc(sizeof(tBGP4_LINK),MEM_BGP_LINK);
    if (p_link == NULL)
    {
        return NULL;
    }

    p_link->p_route = p_route;
    p_route->refer++;

    bgp4_lstnodeinit(&p_link->node);
    bgp4_lstadd_tail(p_list, &p_link->node);

    return p_link;
}

void bgp4_rtlist_clear (tBGP4_LIST *p_list)
{
    tBGP4_LINK *p_link = NULL;
    tBGP4_LINK *p_next = NULL;

    LST_LOOP_SAFE(p_list, p_link, p_next, node, tBGP4_LINK)
    {
        bgp4_lstdelete(p_list, &p_link->node);
        bgp4_link_delete(p_link);
    }
    return ;
}

/*decrease reference of route,if no reference need, delete it
   before calling this function and delete, route is not in rib tree
 */
void bgp4_release_route(tBGP4_ROUTE *p_route)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    if(p_route==NULL)
        return ;

    if (p_route->refer > 0)
    {
        p_route->refer--;
    }
    if (p_route->refer > 0)
    {
        return ;
    }

    /*remove from path*/
    if (p_route->p_path)
    {
        p_instance = p_route->p_path->p_instance;
        bgp4_lstdelete(&p_route->p_path->route_list, &p_route->node);
        p_route->p_path = NULL ;
    }

        /*if this route is route to be updated,reset update route to the first one*/
    if (p_instance->p_route_update == p_route)
    {
        bgp4_init_next_ip_update_route(p_instance);
    }

    if (gBgp4.p_rib_update == p_route)
    {
        bgp4_init_next_rib_update_route();
    }

    bgp4_free(p_route, MEM_BGP_ROUTE);

    return ;
}

/*reset next ip update route*/
void
bgp4_init_next_ip_update_route(tBGP4_VPN_INSTANCE* p_instance)
{

    if(p_instance)
    {
        p_instance->p_route_update = bgp4_rib_first(&p_instance->rib);

    }
}

/*check a route's ip update request*/
u_int
bgp4_process_ip_update(u_int vrf_id,tBGP4_ROUTE  *p_route)
{
    u_int rc = TRUE ;
    u_char addr[64];

    if (p_route->ip_action == BGP4_IP_ACTION_NONE)
    {
        return TRUE ;
    }

    bgp4_log(BGP_DEBUG_RT,1,"bgp4 process ip update route %s,ip action %d,igp sync wait %d",
        bgp4_printf_route(p_route,addr),
        p_route->ip_action,
        p_route->igp_sync_wait);

    /*IGP sync check*/
    if(p_route->igp_sync_wait == TRUE)
    {
        if(gBgp4.sync_enable &&
            bgp4_sysroute_lookup(vrf_id,p_route) == FALSE)
        {
            /*ignore non-sync routes temporarily*/
            return TRUE;
        }
        else
        {
            bgp4_log(BGP_DEBUG_RT,1,"route %s IGP sync secure",bgp4_printf_route(p_route,addr));
            p_route->igp_sync_wait = FALSE;
        }
    }

    if(p_route->route_direction != MPLSL3VPN_ROUTE_REMOTE &&
        p_route->p_path->p_peer &&
            /*is_ibgp_peer(p_route->p_path->p_peer) &&*/
                p_route->p_path->direct_nexthop_ifunit < 0 &&
                    p_route->ip_action == BGP4_IP_ACTION_ADD)
    {
        if(bgp4_get_direct_nexthop(p_route,NULL) == FALSE)
        {
            /*add to delay list for update IP again,but not affect propagation to other peers*/
            if(bgp4_rtlist_lookup(&gBgp4.delay_update_list, p_route) == NULL)
            {
                bgp4_rtlist_add(&gBgp4.delay_update_list, p_route);
            }
            return TRUE;
        }
    }

    /*update ip table if necessary*/
    if (p_route->ip_finish == FALSE)
    {
        /*add or delete ip route from kernal*/
        if (p_route->ip_action == BGP4_IP_ACTION_ADD)
        {
            rc = bgp4_add_sysroute(vrf_id,p_route);
        }
        else if (p_route->ip_action == BGP4_IP_ACTION_DELETE)
        {
            rc = bgp4_del_sysroute(vrf_id,p_route);
        }

        /*update failed,stop processing*/
        if (rc == FALSE)
        {
            return FALSE;
        }
        /*update finished*/
        p_route->ip_finish = TRUE;
        /*for ECMP*/
        p_route->ip_table = (p_route->ip_action == BGP4_IP_ACTION_ADD) ? 1 : 0;
    }

    /*notify mpls if need*/
    if(gBgp4.max_vpn_instance && p_route->mpls_notify_finish == FALSE)
    {
        rc = bgp4_mpls_vpn_route_notify(p_route);

        /*notify failed,stop processing*/
        if (rc == FALSE)
        {
            return FALSE;
        }
        /*update finished*/
        p_route->mpls_notify_finish = TRUE;

    }
    else
    {
        p_route->mpls_notify_finish = TRUE;

    }

    /*send rtmsg if necessary*/
    if (p_route->rtmsg_finish == FALSE)
    {
        /*insert into ipv4 or ipv6 msg buffer*/
        if (p_route->dest.afi == BGP4_PF_IPUCAST)
        {
            rc = bgp4_add_routemsg(vrf_id,gBgp4.ip_msg_buf , p_route,NULL,0);
        }
        else if (p_route->dest.afi == BGP4_PF_IP6UCAST)
        {
            rc = bgp4_add_route6msg(vrf_id,gBgp4.ip6_msg_buf, p_route,NULL,0);
        }
        /*insert into message failed.stop processing*/
        if (rc == FALSE)
        {
            return FALSE ;
        }
        /*send msg finished*/
        p_route->rtmsg_finish = TRUE;
    }
    /*if both operation ok,clear ip action*/
    if ((p_route->ip_finish == TRUE)
        && (p_route->rtmsg_finish == TRUE)
        && (p_route->mpls_notify_finish== TRUE))
    {
        p_route->ip_action = BGP4_IP_ACTION_NONE;
    }
    /*operation ok*/
    return TRUE ;
}

/*check all route's ip update request*/
u_int
bgp4_process_ip_update_all(tBGP4_VPN_INSTANCE* p_instance,u_int wait_finish)
{
    tBGP4_ROUTE  *p_route = NULL;
    u_int start = vos_get_system_tick();
    u_int now = 0 ;
    u_char addr_str[64];

    if(p_instance == NULL)
    {
        return TRUE;
    }


    /*start from route to be update*/
    for (p_route = p_instance->p_route_update; p_route ; p_route = bgp4_rib_next(&p_instance->rib, p_route))
    {
        /*try to update ip table*/
        if (p_route->ip_action == BGP4_IP_ACTION_NONE)
        {
            continue ;
        }

        while (bgp4_process_ip_update(p_instance->instance_id,p_route) != TRUE)
        {
            /*operation failed,if no wait need,just stop processing*/
            if (!wait_finish)
            {
                gBgp4.rib_walkup_need = TRUE ;

                /*ip update failed,restart from it*/
                p_instance->p_route_update = p_route;

                bgp4_log(BGP_DEBUG_RT,1,"bgp4 process ip update fail,instance %d,route %s,action %d,bgp route->mpls notify finish %d",
                    p_instance->instance_id,bgp4_printf_route(p_route,addr_str),p_route->ip_action,
                    p_route->mpls_notify_finish);

                return FALSE;
            }
            bgp4_log(BGP_DEBUG_RT,1,"bgp4 process ip update failed ,try again>>>>>>");
            /*need wait,so do some delay,and retry*/
            vos_pthread_delay(1);
        }
        /*if do not wait, we will stop if time exceed limit:300ticks*/
        if (!wait_finish)
        {
            now = vos_get_system_tick();
            if ((now < start) || (now > (start + 300)))
            {
                gBgp4.rib_walkup_need = TRUE ;

                /*ip update not finished,start from next one*/
                p_instance->p_route_update = bgp4_rib_next(&p_instance->rib, p_route);
                return FALSE ;
            }
        }
    }
    /*all ip process finished,clear update route*/
    p_instance->p_route_update = NULL;
    return TRUE ;
}

/*record the first route in path table*/
void
bgp4_init_next_rib_update_route(void)
{
    tBGP4_PATH *p_path = NULL ;
    tBGP4_ROUTE  *p_route = NULL;
    tBGP4_ROUTE  *p_next = NULL;
    u_int i = 0;
    u_char addr[64];

    for(i = 0;i < BGP4_PF_MAX;i++)
    {
        LST_LOOP(&gBgp4.attr_list[i], p_path, node, tBGP4_PATH)
        {
            LST_LOOP_SAFE(&p_path->route_list, p_route, p_next, node, tBGP4_ROUTE)
            {
                gBgp4.p_rib_update = p_route;
                return ;
            }
        }
    }

    return ;
}

static void
bgp4_send_peer_update_list(
                      tBGP4_PEER *p_peer,
                      u_int af)
{
    u_char addrstr[100]={0};


     if(p_peer->wcount==0&&p_peer->fcount==0)
     {
        return;
     }

    bgp4_log(BGP_DEBUG_UPDATE,1,"prepare send update to peer %s,with_draw route=%d,fea route=%d",
        bgp4_printf_peer(p_peer,addrstr),p_peer->wcount,p_peer->fcount);


    bgp4_send_update_to_peer(p_peer, af, &p_peer->flist, &p_peer->wlist);

    /*clear list*/
    bgp4_rtlist_clear(&p_peer->flist);
    bgp4_rtlist_clear(&p_peer->wlist);
    p_peer->wcount = 0;
    p_peer->fcount = 0 ;
    return ;
}

/*process rib update msg of a special route*/
u_int
bgp4_process_rib_update(
                          tBGP4_ROUTE  *p_route,
                          tBGP4_PEER *p_peer,
                          u_int *p_with_count,
                          u_int *p_fea_count
                            )
{
    u_int af = p_route->dest.afi;
    u_char addrstr[64] = {0};
    u_char rc = 0 ;

    if (p_peer->state != BGP4_NS_ESTABLISH)
    {
        return 0;
    }

    if (!bgp4_af_support(p_peer, af))
    {
        bgp4_log(BGP_DEBUG_EVT,1,"peer %s does not support peer/route's af:%d/%d,do not send update to this peer",
                    bgp4_printf_peer(p_peer,addrstr),p_peer->remote.af,af);
            return 0;
    }

    /*IGP sync check*/
    if(p_route->igp_sync_wait == TRUE)
    {
        if(gBgp4.sync_enable &&
            bgp4_sysroute_lookup(p_route->p_path->p_instance->instance_id,p_route) == FALSE)
        {

            return FALSE;
        }
        else
        {
            bgp4_log(BGP_DEBUG_RT,1,"route %s IGP sync secure",bgp4_printf_route(p_route,addrstr));
            p_route->igp_sync_wait = FALSE;
        }
    }

    if (BIT_LST_TST(p_route->withdraw_bits, p_peer->bit_index)
        || BIT_LST_TST(p_route->update_bits, p_peer->bit_index))
    {
        if (bgp4_send_check(p_peer, p_route) == TRUE )
        {
            if (BIT_LST_TST(p_route->withdraw_bits, p_peer->bit_index))
            {
                bgp4_rtlist_add(&p_peer->wlist, p_route);
                p_peer->wcount++;
                p_peer->total_update++;
                rc++;
                (*p_with_count)++;
            }
            else if (BIT_LST_TST(p_route->update_bits, p_peer->bit_index))
            {
                bgp4_rtlist_add(&p_peer->flist, p_route);
                p_peer->fcount++;
                p_peer->total_update++;
                rc++;
                (*p_fea_count)++;
            }
        }
        /*route count exceed limit,send packet now*/
        if ((p_peer->wcount + p_peer->fcount) >= 300)
        {
            bgp4_send_peer_update_list(p_peer, af);
        }
    }

    bgp4_log(BGP_DEBUG_RT,1,"peer with route count %d,feasible route count %d",
                    p_peer->wcount,p_peer->fcount);
    return rc;
}

/*process all rib update function*/
u_int
bgp4_process_rib_update_all(u_int wait_finish)
{
    tBGP4_PATH *p_path = NULL ;
    tBGP4_ROUTE  *p_route = NULL;
    tBGP4_ROUTE  *p_next = NULL;
    tBGP4_PEER *p_peer = NULL;
    u_char zero[32] = {0};
    u_int checked = 0 ;
    u_int first_checked = 0 ;
    u_int with_count = 0 ;
    u_int fea_count = 0 ;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    static u_int last_call = 0 ;
    u_int af;
    u_int now;
    u_int max_checked = 0 ;
    u_char addrstr[64] = {0};

    /*if no rib update need, do nothing*/
    if (gBgp4.p_rib_update == NULL)
    {
        return TRUE;
    }


    /*limit rate,at least use 20 ticks as interval*/
    now = vos_get_system_tick();
    if ((now > last_call) && (now < (last_call + 20)))
    {
        gBgp4.rib_walkup_need = TRUE ;
        return FALSE;
    }

    last_call = now ;
    max_checked = 5000;

    /*clear all peer's update list*/
    LST_LOOP(&gBgp4.vpn_instance_list, p_instance, node, tBGP4_VPN_INSTANCE)
    {
        LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER )
        {
            bgp4_lstinit(&p_peer->wlist);
            bgp4_lstinit(&p_peer->flist);
            p_peer->wcount = 0 ;
            p_peer->fcount = 0 ;
            p_peer->total_update = 0 ;
        }
    }

    p_instance = NULL;

    for(af = 0;af< BGP4_PF_MAX;af++)
    {
        LST_LOOP(&gBgp4.attr_list[af], p_path, node, tBGP4_PATH)
        {
        #if 0/*check all path*/
            /*start from next rib's path*/
            if (p_path == ((tBGP4_ROUTE *)gBgp4.p_rib_update)->p_path)
            {
                first_checked = 1;
            }
            else if (!first_checked)
            {
                continue ;
            }
              #endif
            p_instance = p_path->p_instance;
            if(p_instance == NULL)
            {
                bgp4_log(BGP_DEBUG_EVT,1,"bgp4_process_rib_update_all,p_instance null");
                continue;
            }

            with_count = 0 ;
            fea_count = 0 ;

            /*check route*/
            LST_LOOP_SAFE(&p_path->route_list, p_route, p_next, node, tBGP4_ROUTE)
            {

                /*IGP sync check*/
                if(p_route->igp_sync_wait == TRUE)
                {
                    if(gBgp4.sync_enable &&
                        bgp4_sysroute_lookup(p_instance->instance_id,p_route) == FALSE)
                    {
                        /*ignore non-sync routes temporarily*/
                        continue;
                    }
                    else
                    {
                        bgp4_log(BGP_DEBUG_RT,1,"route %s IGP sync secure",bgp4_printf_route(p_route,addrstr));
                        p_route->igp_sync_wait = FALSE;
                    }
                }
                /*check if update need*/
                /*scan for each peer*/
                if ((memcmp(p_route->withdraw_bits, zero, sizeof(p_route->withdraw_bits)))
                    || (memcmp(p_route->update_bits, zero, sizeof(p_route->update_bits)) ))
                {
                    LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER )
                    {
                        /*add route to peer's update list*/
                        checked += bgp4_process_rib_update(p_route, p_peer, &with_count, &fea_count);
                    }

                }
                /*clear all update flag of this route*/
                memset(p_route->withdraw_bits, 0, sizeof(p_route->withdraw_bits));
                memset(p_route->update_bits, 0, sizeof(p_route->update_bits));

                /*if not wait,and route exceed limit,stop processing,jump out*/
                if (!wait_finish && (checked >= max_checked))
                {
                    gBgp4.rib_walkup_need = TRUE ;

                    LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER )
                    {
                        bgp4_send_peer_update_list(p_peer, af);
                    }
                    /*next we will start check from this route*/
                    gBgp4.p_rib_update = p_route;
                    return FALSE;
                }
              #if 0 /*do not delete route here*/ 
                /*delete unused route from rib*/
                if ((p_route->is_deleted == TRUE)
                    && (p_route->ip_action == BGP4_IP_ACTION_NONE)
                        &&(p_route->deferral != TRUE))
                {
                    bgp4_rib_delete(&p_instance->rib, p_route);
                }
              #endif
            }
            /*if some feasible route exist, send update for all peer*/
            if (fea_count || with_count)
            {
                LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER )
                {
                    bgp4_send_peer_update_list(p_peer, af);
                }
            }
        }/*end for path loop*/
        p_instance = NULL;

         /*scan end,check all peer,if any peer need to be sent update routes to ,at last,
                send end-of-rib of it.
        */
        LST_LOOP(&gBgp4.vpn_instance_list, p_instance, node, tBGP4_VPN_INSTANCE)
        {
            LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER)
            {
                if (p_peer->state != BGP4_NS_ESTABLISH ||
                    p_peer->total_update == 0 ||
                    !bgp4_af_support(p_peer,af))
                {
                    continue ;
                }

                /*send update  if necessary*/
                bgp4_send_peer_update_list(p_peer, af);

                /*send end-of-rib at last,if it has not been sent before*/
                if(!af_isset(p_peer->send_rib_end_af,af))
                {
                    bgp4_send_rib_end(p_peer, af);

                    /*end of address family loop,all end-of-rib sent*/
                    if(p_peer->send_rib_end_af == p_peer->send_capability)
                    {
                        p_peer->send_rib_end = 1;

                        /*end of this peer,clear its total update routes num*/
                            p_peer->total_update = 0;
                    }
                }
             }
        }
    }

     /*clear update rib*/
     gBgp4.p_rib_update = NULL;
     return TRUE;
}

void
bgp4_delete_unused_route(void)
{
    tBGP4_ROUTE  *p_route = NULL;
    tBGP4_ROUTE  *p_next = NULL;
    u_char zero[32] = {0};
    tBGP4_VPN_INSTANCE* p_instance = NULL;

       /*delete unused routes from rib*/
    LST_LOOP(&gBgp4.vpn_instance_list,p_instance, node, tBGP4_VPN_INSTANCE)
    {
        RIB_LOOP(&p_instance->rib, p_route, p_next)
        {
            if ((p_route->is_deleted==TRUE)
                &&(p_route->deferral != TRUE)
                &&(p_route->ip_action == BGP4_IP_ACTION_NONE)
                && (memcmp(p_route->withdraw_bits, zero, sizeof(p_route->withdraw_bits)) == 0)
                && (memcmp(p_route->update_bits, zero, sizeof(p_route->update_bits)) == 0))
            {
                u_char rt_str[64];
                bgp4_log(BGP_DEBUG_EVT,1,"\r\nbgp4 delete unused route %s from instance %d",
                            bgp4_printf_route(p_route,rt_str),
                            p_instance->instance_id);

                bgp4_rib_delete(&p_instance->rib, p_route);
            }
        }
    }

    return;
}

/*force to finish ip update*/
void bgp4_force_route_ip_update(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_route)
{
    if (p_route->ip_action != BGP4_IP_ACTION_NONE)
    {
        bgp4_init_next_rib_update_route();
        bgp4_init_next_ip_update_route(p_instance);

        /*need wait,so do some delay,and retry*/
        while (bgp4_process_ip_update(p_instance->instance_id,p_route) != TRUE)
        {
            if(gBgp4.asnum==0)/*bgp disabled*/
            {
                return;
            }
            vos_pthread_delay(1);
        }
    }
    return ;
}
/*force to finish ip & rib update of a route*/
void
bgp4_force_ip_rib_update(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_route)
{
    tBGP4_PEER *p_peer = NULL;
    u_int with_count = 0 ;
    u_int fea_count = 0 ;
    u_char zerobit[BGP4_MAX_PEER_ID/8] = {0};

    /*if route's update not processed,stop*/
    if ((p_route->ip_action != BGP4_IP_ACTION_NONE)
             && p_route->ip_finish == FALSE
             && p_route->rtmsg_finish == FALSE)
    {
        p_route->ip_action = BGP4_IP_ACTION_NONE;
        memset(p_route->withdraw_bits, 0, sizeof(p_route->withdraw_bits));
        memset(p_route->update_bits, 0, sizeof(p_route->update_bits));
        return ;
    }

    bgp4_force_route_ip_update(p_instance,p_route);

    if ((memcmp(p_route->withdraw_bits, zerobit, sizeof(p_route->withdraw_bits)) == 0)
        && (memcmp(p_route->update_bits, zerobit, sizeof(p_route->update_bits)) == 0))
    {
        return ;
    }

    if(p_route->is_deleted == TRUE)
    {
        return;
    }

    /*clear all peer's update list*/
    LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER )
    {
        bgp4_lstinit(&p_peer->wlist);
        bgp4_lstinit(&p_peer->flist);
        p_peer->wcount = 0 ;
        p_peer->fcount = 0 ;
        p_peer->total_update = 0 ;
        bgp4_process_rib_update(p_route, p_peer, &with_count, &fea_count);
        bgp4_send_peer_update_list(p_peer, p_route->dest.afi);
    }

    /*clear all update flag of this route*/
    memset(p_route->withdraw_bits, 0, sizeof(p_route->withdraw_bits));
    memset(p_route->update_bits, 0, sizeof(p_route->update_bits));
    return ;
}

/*wait:TRUE--wait for all operations finished,FALSE--do not wait*/
void
bgp4_rib_walkup(u_int wait_finish)
{
    u_int ip_tick_end=0;
    u_int time_start = 0 ;
    int rc = 0 ;
    u_int tmp_count = 0;
    u_int swcount = 0;
    u_int hwcount = 0;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    /*if walkup not requested,do nothing*/
    if (gBgp4.rib_walkup_need != TRUE)
    {
        /*try to send route msg buffer*/
        bgp4_send_routemsg(gBgp4.ip_msg_buf);
        bgp4_send_routemsg(gBgp4.ip6_msg_buf);
        return ;
    }

    /*clear request flag*/
    gBgp4.rib_walkup_need = FALSE ;

    /*if ip update failed,do not do any forward operation*/
    time_start = vos_get_system_tick();

    LST_LOOP(&gBgp4.vpn_instance_list,p_instance, node, tBGP4_VPN_INSTANCE)
    {
        tmp_count = 0;

        if (p_instance->p_route_update == NULL)
        {
            bgp4_send_routemsg(gBgp4.ip_msg_buf);
            bgp4_send_routemsg(gBgp4.ip6_msg_buf);
        }


        rc = bgp4_process_ip_update_all(p_instance,wait_finish);
        gBgp4.ipupdate_time += (vos_get_system_tick() - time_start);

        if (rc != TRUE)
        {
            return ;
        }

#if !defined(USE_LINUX) && !defined(WIN32)
        m2IpRouteScalarGet(&p_instance->instance_id,M2IP_ROUTE_CURRENTNUM,&tmp_count);

        swcount += tmp_count;
#endif

    }

#if !defined(USE_LINUX) && !defined(WIN32)
     /*wait for hardware finished*/
    hwTableStatsGetApi(NULL,HWROUTE_IP_HWROUTENUM,&hwcount);
    if ((swcount > (hwcount + 100)) || (hwcount > (swcount + 100)))
    {
        gBgp4.rib_walkup_need = TRUE ;
        gBgp4.rib_update_wait = TRUE;
        gBgp4.kernel_routes_count = swcount;
        gBgp4.hw_routes_count = hwcount;
        return ;
    }
    else
    {
        gBgp4.rib_update_wait = FALSE;
    }


#endif
    ip_tick_end=vos_get_system_tick();
    /*check and send update to peer*/
    rc = bgp4_process_rib_update_all(wait_finish);
    gBgp4.output_time += (vos_get_system_tick() - ip_tick_end);
    if (rc != TRUE)
    {
        return ;
    }

    /*flush any route to be deleted*/
    bgp4_delete_unused_route();

    return ;
}

/*schedule update ip table for a route entry*/
void
bgp4_schedule_ip_update(
                   tBGP4_ROUTE *p_route,
                   u_int action
                   )
{
    /*ignore self route*/
    if(p_route==NULL||p_route->p_path==NULL)
    {
        return;
    }
#if 0
    if (p_route->p_path->p_peer == NULL)
    {
        return ;
    }
 #endif
    if (p_route->proto != M2_ipRouteProto_bgp)
    {
        return;
    }
    
    /*if action is none,or same as route's current action,do nothing*/
    if ((p_route->ip_action == action)
        || (action == BGP4_IP_ACTION_NONE))
    {
        return ;
    }

    /*set global event*/
    gBgp4.rib_walkup_need = TRUE ;

    /*ip update operation changed,so reset route_update to first one*/
    bgp4_init_next_ip_update_route(p_route->p_path->p_instance);

    /*if current action is none,set directly*/
    if (p_route->ip_action == BGP4_IP_ACTION_NONE)
    {
        p_route->ip_action = action;
        p_route->ip_finish = FALSE ;
        p_route->rtmsg_finish = FALSE ;
        p_route->mpls_notify_finish = FALSE;

        return ;
    }
    /*set action,and set flag*/
    p_route->ip_action = action;
    p_route->ip_finish = !p_route->ip_finish;
    p_route->rtmsg_finish = !p_route->rtmsg_finish;
    p_route->mpls_notify_finish = !p_route->mpls_notify_finish;


    /*if new flag both 1,clear action,nothing to do*/
    if ((p_route->ip_finish == TRUE)
        && (p_route->rtmsg_finish == TRUE)
        && (p_route->mpls_notify_finish == TRUE))
    {
        p_route->ip_action = BGP4_IP_ACTION_NONE ;
    }
    return ;
}

 void
bgp4_schedule_rib_update(
                tBGP4_VPN_INSTANCE* p_instance,
            tBGP4_ROUTE *p_old,
            tBGP4_ROUTE *p_new,
            tBGP4_PEER *p_target_peer
                  )
{
    tBGP4_PEER *p_peer = NULL;

#if 0/*slave card should also do this scheduling*/
    if(gBgp4.work_mode==BGP4_MODE_SLAVE)
    {
        return ;
    }
#endif
    /*if old and new route are same,or both null,stop*/
    if (p_old == p_new || p_instance == NULL)
    {
        return ;
    }

    /*global event*/
    gBgp4.rib_walkup_need = TRUE ;
    bgp4_init_next_rib_update_route();

    /*scan for all peers*/
    LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER)
    {

        if(p_target_peer&&p_target_peer!=p_peer)
        {
            continue;
        }
        /*if new route exist,and old route not exist, set update flag directly*/
        if (p_new && (p_old == NULL))
        {
            BIT_LST_SET(p_new->update_bits, p_peer->bit_index);
            continue ;
        }
        /*if new route not exist,old route exist,set withdraw flag directly*/
        if (p_old && (p_new == NULL))
        {
            BIT_LST_SET(p_old->withdraw_bits, p_peer->bit_index);
            continue ;
        }
        /*both old and new route exist,if new route can be sent to this peer,set update flag
        else set withdraw flag
        */
        if (bgp4_send_check(p_peer, p_new) == TRUE)
        {
            BIT_LST_SET(p_new->update_bits, p_peer->bit_index);
        }
        else
        {
            BIT_LST_SET(p_old->withdraw_bits, p_peer->bit_index);
        }
    }
    return ;
}


void
bgp4_schedule_rib_update_with_range(
            tBGP4_VPN_INSTANCE* p_instance,
            tBGP4_ROUTE *p_old,
            tBGP4_ROUTE *p_new
            )
{
    tBGP4_AGGR *p_range = NULL ;
    tBGP4_ROUTE *p_range_route = NULL ;
    tBGP4_ROUTE *p_check_route = NULL;
    tBGP4_LINK *p_link = NULL;
    tBGP4_LIST rtlist;
    tBGP4_ROUTE *p_best_route=NULL;

#if 0   /*slave card should also do this scheduling*/
    if(gBgp4.work_mode==BGP4_MODE_SLAVE)
    {
        return ;
    }
#endif
    /*if old and new route are same,or both null,stop*/
    if (p_old == p_new)
    {
        return ;
    }

    /*global event*/
    gBgp4.rib_walkup_need = TRUE ;

    bgp4_init_next_rib_update_route();

    /*check aggregate operation,
    decide matched range for this route*/
    p_check_route = p_old ? p_old : p_new ;
    p_range = bgp4_aggr_match(p_instance,p_check_route->dest.afi, p_check_route);

    /*case:range not exist or inactive,action as above*/
    if ((p_range == NULL) || (p_range->p_aggr_rt == NULL))
    {
        bgp4_schedule_rib_update(p_instance,p_old, p_new,NULL);
        return ;
    }

    /*summary only is not set,set update for route*/
    if (p_range->summaryonly == FALSE)
    {
        bgp4_schedule_rib_update(p_instance,p_old, p_new,NULL);
    }

    /*range exist,and active,check change of range route
    */
    bgp4_lstinit(&rtlist);

    p_range_route = p_range->p_aggr_rt;
    if (p_range_route == NULL)
    {
        return ;
    }

    bgp4_rib_get_by_range(&p_instance->rib, &p_range_route->dest, &rtlist);

    if (bgp4_rtlist_aggregate_enable(&rtlist) == TRUE)
    {
        LST_LOOP(&rtlist, p_link, node, tBGP4_LINK)
        {
            bgp4_aggregate_path(p_range_route, p_link->p_route);
        }
        p_range_route->summary_active = TRUE;
        /*set update for this range route*/
        bgp4_schedule_rib_update(p_instance,NULL, p_range_route,NULL);
    }
    else if (p_range_route->summary_active == TRUE)
    {
        p_range_route->summary_active = FALSE;
        /*set withdraw for this range route*/
        bgp4_schedule_rib_update(p_instance,p_range_route, NULL,NULL);

        /*here should set fea for ranged route*/
        if(p_range->summaryonly==TRUE)
        {
            LST_LOOP(&rtlist,p_link, node, tBGP4_LINK)
            {
                p_best_route=NULL;
                p_best_route=bgp4_best_route(&p_instance->rib,&p_link->p_route->dest);
                if(p_best_route==p_link->p_route)
                {
                    bgp4_schedule_rib_update(p_instance,NULL, p_range_route,NULL);
                }
            }
        }
    }
    /*release route list*/
    bgp4_rtlist_clear(&rtlist);
    return ;
}




#endif
