
#include "bgp4peer.h"
#ifdef NEW_BGP_WANTED

#include "bgp4com.h"

tBGP4_PEER * 
bgp4_peer_create(
    tBGP4_VPN_INSTANCE *p_instance,
    tBGP4_ADDR *ip)
{
    tBGP4_PEER *p_new = NULL; 
    u_int i = 0;
    
    /*allocate index for peer,start from 1*/
    for (i = 1; i < BGP4_MAX_PEER_ID + 1 ; i++)
    {
        if (bgp4_peer_lookup_by_index(p_instance, i) == NULL)
        {            
            break;
        }
    }
    if (i >= BGP4_MAX_PEER_ID + 1)
    {
        return NULL;
    }
    
    p_new = bgp4_malloc(sizeof(tBGP4_PEER), MEM_BGP_PEER);
    if (p_new == NULL)
    {
        return NULL;
    }
    p_new->p_instance = p_instance;
    p_new->retry_interval = BGP4_DFLT_CONNRETRYINTERVAL;
    p_new->hold_interval = BGP4_DFLT_HOLDINTERVAL;
    p_new->keep_interval = BGP4_DFLT_KEEPALIVEINTERVAL;
    p_new->origin_interval = BGP4_DFLT_MINASORIGINTERVAL;
    p_new->adv_interval  = BGP4_DFLT_MINROUTEADVINTERVAL;
    p_new->state = BGP4_NS_IDLE;
    p_new->start_interval = BGP4_DFLT_STARTINTERVAL;
    p_new->version = BGP4_VERSION_4;
    p_new->admin_state = BGP4_PEER_HALTED;
    p_new->local_refresh_enable = TRUE;
    
    /*add for cap advertisement*/
    p_new->option_tx_fail = FALSE;
    p_new->unsupport_capability = FALSE;

    p_new->neg_hold_interval = p_new->hold_interval;
    p_new->neg_keep_interval = p_new->keep_interval;

    p_new->ttl_hops = 255;
    p_new->sock = BGP4_INVALID_SOCKET;

    p_new->fake_as = gbgp4.as;

    /*support ipv4 unicast by default*/
    if (ip->afi == BGP4_PF_IPUCAST)
    {
        p_new->local_ip.afi = BGP4_PF_IPUCAST;
        flag_set(p_new->local_mpbgp_capability, BGP4_PF_IPUCAST);
    }

    if (ip->afi == BGP4_PF_IP6UCAST)
    {
        p_new->local_ip.afi = BGP4_PF_IP6UCAST;
        flag_set(p_new->local_mpbgp_capability, BGP4_PF_IP6UCAST);
    }

    memcpy(&p_new->ip, ip, sizeof(tBGP4_ADDR));

    /*init timer*/
    bgp4_timer_init(&p_new->connect_timer, bgp4_peer_connect_timer_expired, p_new);
    bgp4_timer_init(&p_new->retry_timer, bgp4_peer_retry_timer_expired, p_new);
    bgp4_timer_init(&p_new->hold_timer, bgp4_peer_holdtimer_expired, p_new);
    bgp4_timer_init(&p_new->keepalive_timer, bgp4_peer_keepalive_timer_expired, p_new);
    bgp4_timer_init(&p_new->gr_timer, bgp4_peer_gr_expired, p_new);
    bgp4_timer_init(&p_new->delete_timer, bgp4_peer_delete_expired, p_new);
    bgp4_timer_init(&p_new->adj_out_timer, bgp4_peer_adjout_expire, p_new);
    bgp4_timer_init(&p_new->bfd_timer, bgp4_bind_bfd, p_new);
    
    /*policy list*/
    bgp4_avl_init(&p_new->policy_table, bgp4_policy_lookup_cmp);

    /*orf table init*/
    bgp4_avl_init(&p_new->orf_out_table, bgp4_orf_lookup_cmp);
    bgp4_avl_init(&p_new->orf_in_table, bgp4_orf_lookup_cmp);
    bgp4_avl_init(&p_new->orf_old_in_table, bgp4_orf_lookup_cmp);    
    
    /*insert into index table*/
    p_new->bit_index = i;
    bgp4_avl_add(&p_instance->peer_index_table, p_new);
    
    /*sorted by ipaddress*/
    bgp4_avl_add(&p_instance->peer_table, p_new); 

    gbgp4.uiPeerCount++;
    return p_new;
}

void 
bgp4_peer_delete(tBGP4_PEER *p_peer)
{
    tBGP4_VPN_INSTANCE *p_instance = p_peer->p_instance;

    bgp4_peer_sock_close(p_peer);  
    
    bgp4_peer_rxmsg_reset(p_peer);
    
    bgp4_policy_delete_all(&p_peer->policy_table);

    bgp4_peer_flood_clear(p_peer);

    /*clear all orf*/    
    bgp4_avl_walkup(&p_peer->orf_out_table, bgp4_orf_delete);
    bgp4_avl_walkup(&p_peer->orf_in_table, bgp4_orf_delete);
    bgp4_avl_walkup(&p_peer->orf_old_in_table, bgp4_orf_delete);

    /*stop all timers*/
    bgp4_timer_stop(&p_peer->connect_timer);
    bgp4_timer_stop(&p_peer->retry_timer);  
    bgp4_timer_stop(&p_peer->hold_timer);
    bgp4_timer_stop(&p_peer->keepalive_timer); 
    bgp4_timer_stop(&p_peer->gr_timer);
    bgp4_timer_stop(&p_peer->delete_timer);
    bgp4_timer_stop(&p_peer->adj_out_timer);
    bgp4_timer_stop(&p_peer->bfd_timer);    

    /*remove from index table*/
    bgp4_avl_delete(&p_instance->peer_index_table, p_peer);
    
    /*remove from peer table*/
    bgp4_avl_delete(&p_instance->peer_table, p_peer);

    bgp4_free(p_peer, MEM_BGP_PEER);

    /*update upe count*/
    bgp4_vpn_upe_peer_count_update(p_instance);

    gbgp4.uiPeerCount--;
    return;
}

int
bgp4_peer_lookup_cmp(
    tBGP4_PEER *p1,
    tBGP4_PEER *p2)
{
    return bgp4_prefixcmp(&p1->ip, &p2->ip);
}

int
bgp4_peer_index_lookup_cmp(
    tBGP4_PEER *p1,
    tBGP4_PEER *p2)
{
    if (p1->bit_index != p2->bit_index)
    {
        return (p1->bit_index > p2->bit_index) ? 1 : -1;
    }
    return 0;
}

/*lookup peer according to ip address*/
tBGP4_PEER *
bgp4_peer_lookup(
     tBGP4_VPN_INSTANCE *p_instance,
     tBGP4_ADDR *ip)
{
    tBGP4_PEER peer;
    memset(&peer, 0, sizeof(peer));
    memcpy(&peer.ip, ip, sizeof(*ip));
    return bgp4_avl_lookup(&p_instance->peer_table, &peer);
}

/*lookup peer according to index*/
tBGP4_PEER *
bgp4_peer_lookup_by_index(
     tBGP4_VPN_INSTANCE *p_instance,
     u_int id)
{
    tBGP4_PEER peer;
    peer.bit_index = id;
    return bgp4_avl_lookup(&p_instance->peer_index_table, &peer);
}

void 
bgp4_peer_reset_all(void)
{
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    bgp4_log(BGP_DEBUG_CMN, "reset all peers");

    bgp4_instance_for_each(p_instance)
    {
        bgp4_peer_for_each(p_instance, p_peer)
        {
            if (bgp4_local_grace_restart(p_peer) != VOS_OK)
            {
                p_peer->cease_reason = BGP4_ADMINISTRATIVE_RESET;
                bgp4_fsm(p_peer, BGP4_EVENT_STOP);
                bgp4_fsm(p_peer, BGP4_EVENT_START);
            }
        }
    }

    if (gbgp4.restart_enable)
    {
        /*start global reset timer*/
        if (gbgp4.in_restarting == FALSE)
        {
            bgp4_timer_start(&gbgp4.gr_waiting_timer, gbgp4.restart_wait_period);
            gbgp4.in_restarting = TRUE;
        }
    }
    return;
}

/*obtain invalid event*/
void 
bgp4_fsm_invalid(tBGP4_PEER *p_peer)
{
    u_int af = 0 ;
    u_char str[64] = {0};
    u_char reason_str[64] = {0};
    u_char creason_str[64] = {0};
    u_short error_code = p_peer->stat.last_errcode;

    error_code = (error_code << 8) + p_peer->stat.last_subcode;

    /*end rib for established state*/
    if ((p_peer->state == BGP4_NS_ESTABLISH)
        && (p_peer->restart_role == BGP4_GR_ROLE_NORMAL))
    {
        p_peer->stat.uptime = time(NULL);
        for (af = 0 ; af < BGP4_PF_MAX; af++)
        {
            bgp4_peer_route_clear(af, p_peer);
        }
    }

    if (gbgp4.work_mode == BGP4_MODE_SLAVE)
    {
        return;
    }

    /*stop all timers*/
    bgp4_timer_stop(&p_peer->connect_timer);
    bgp4_timer_stop(&p_peer->retry_timer);
    bgp4_timer_stop(&p_peer->hold_timer);
    bgp4_timer_stop(&p_peer->keepalive_timer);

    /*clear all learnt orf*/
    bgp4_avl_walkup(&p_peer->orf_in_table, bgp4_orf_delete);
    bgp4_avl_walkup(&p_peer->orf_old_in_table, bgp4_orf_delete);
    
    bgp4_peer_sock_close(p_peer);

    if (p_peer->state == BGP4_NS_ESTABLISH)
    {
        bgp4_printf_notify(p_peer->notify.code, p_peer->notify.sub_code, reason_str, creason_str);
        bgp4_log(BGP_DEBUG_FSMCHANGE, "peer %s state change from Established to Idle reason:%s subreason:%s",
            bgp4_printf_peer(p_peer, str), reason_str, creason_str); 
    }

    /*clear notify*/
    p_peer->notify.code = 0;
    p_peer->notify.sub_code = 0;
    p_peer->notify.len = 0;
    
    p_peer->state = BGP4_NS_IDLE;
    if (p_peer->unsupport_capability == TRUE)
    {
        bgp4_log(BGP_DEBUG_EVT, "peer has unsupported capability,do not restart it!");
        return;
    }

    if (gbgp4.trap_enable)
    {
        sendBgpBackwardTransitionTrap(
             (p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6,
              p_peer->ip.ip, p_peer->state, (char *)&error_code);
    }
    
    /*prepare next connection*/
    if (p_peer->admin_state != BGP4_PEER_HALTED)
    {
        bgp4_timer_start(&p_peer->connect_timer, p_peer->start_interval);
        bgp4_peer_start_time_backoff(p_peer);
    }
    return;
}

void
bgp4_fsm_start(tBGP4_PEER *p_peer)
{
    int s = 0;
    if ((p_peer->state != BGP4_NS_IDLE)
        || (p_peer->admin_state != BGP4_PEER_RUNNING))
    {
        return;
    }

    if (p_peer->unsupport_capability)
    {
        bgp4_log(BGP_DEBUG_TCP, "active tcp connection is disabled for upsupport capability");
        return ;
    }

    /*open connection*/
    p_peer->state = BGP4_NS_CONNECT;
    if (bgp4_peer_sock_connect(p_peer) == VOS_OK)
    {
        /*if tcp connection is connected,break*/
        if (p_peer->state < BGP4_NS_OPENSENT)
        {
            /*tcp connection ok but not connected,wait for peer up*/
            bgp4_timer_start(&p_peer->retry_timer, p_peer->retry_interval);
        }
    }
    else
    {
        /*prepare next open*/
        bgp4_timer_start(&p_peer->connect_timer, p_peer->start_interval);
        bgp4_peer_start_time_backoff(p_peer);
    }

    s = (p_peer->ip.afi == BGP4_PF_IPUCAST) ? gbgp4.server_sock[p_peer->p_instance->vrf] : gbgp4.server_sock6[p_peer->p_instance->vrf];
    /*add md5*/
    if (p_peer->md5_support && (p_peer->md5_server_set == FALSE))
    {
        bgp4_set_sock_md5_key(s, 1, p_peer);
        p_peer->md5_server_set = TRUE;
    }
    if (p_peer->ttl_hops && (p_peer->ttl_hops < 256))
    {
        bgp4_tcp_set_peer_ttl(s, p_peer);
    }
    return;
}

void
bgp4_fsm_stop(tBGP4_PEER *p_peer)
{
    int s = 0;
    if (p_peer->admin_state == BGP4_PEER_HALTED)
    {
        return;
    }

    /*peer is administrtive down,no graceful-restart*/
    bgp4_peer_restart_finish(p_peer);

    /*send cease to peer*/
    if ((p_peer->state == BGP4_NS_OPENSENT) ||
        (p_peer->state == BGP4_NS_OPENCONFIRM) ||
        (p_peer->state == BGP4_NS_ESTABLISH))
    {
        p_peer->notify.code = BGP4_CEASE;
        p_peer->notify.sub_code = p_peer->cease_reason;        
        bgp4_notify_output(p_peer);
    }
    s = (p_peer->ip.afi == BGP4_PF_IPUCAST) ? gbgp4.server_sock[p_peer->p_instance->vrf] : gbgp4.server_sock6[p_peer->p_instance->vrf];
    if (p_peer->md5_support && (p_peer->md5_server_set == TRUE))
    {
        bgp4_set_sock_md5_key(s, 0, p_peer);
        p_peer->md5_server_set = FALSE;
    }

    /*close socket*/
    p_peer->notify.code = BGP4_CEASE;
    p_peer->notify.sub_code = BGP4_ADMINISTRATIVE_SHUTDOWN;
    bgp4_fsm_invalid(p_peer);

    /*clear some flags*/
    p_peer->option_tx_fail = FALSE;
    p_peer->unsupport_capability = FALSE;
    return ;
}

void 
bgp4_fsm(
    tBGP4_PEER *p_peer,
    u_short event)
{
    u_char addrstr[64];
    u_char statestr[32];
    u_char evtstr[64];

    bgp4_log(BGP_DEBUG_EVT,"nbr %s,state %s,event %s",
            bgp4_printf_peer(p_peer, addrstr),
            bgp4_printf_state(p_peer->state, statestr),
            bgp4_printf_event(event, evtstr));

    p_peer->stat.event = event;/*recording last event for sync peer state*/

    /*clear sync socket*/
    if (gbgp4.work_mode != BGP4_MODE_SLAVE)
    {
        p_peer->sync_client_sock = 0;
        p_peer->sync_server_sock = 0;
    }
    
    if (gbgp4.work_mode == BGP4_MODE_MASTER)
    {
        bgp4_sync_peer_send(p_peer);
    }

    switch (event) {
        case BGP4_EVENT_START:
             bgp4_fsm_start(p_peer);
             break;

        case BGP4_EVENT_STOP:
             bgp4_fsm_stop(p_peer);
             break;

        case BGP4_EVENT_TCP_OPENED:
             /*restarting process*/
             /*if peer is restarter or receiver,remain current
               restart role,else enter receiver role if supported*/
             if (p_peer->restart_role == BGP4_GR_ROLE_NORMAL)
             {
                 /*remote close action,peer may be in restarting*/
                 if (bgp4_remote_grace_restart(p_peer) == VOS_OK)
                 {
                     break;
                 }
    
                 /*if there is any route learnt from peer,
                  and peer and local is not in restarting,do not connect*/
                 if (bgp4_peer_route_exist(p_peer) == TRUE)
                 {
                     bgp4_log(BGP_DEBUG_TCP, "wait route flush before connect to peer");
                     bgp4_fsm_invalid(p_peer);
                     break;
                 }
             }
             
             if ((p_peer->state == BGP4_NS_CONNECT) 
                 || (p_peer->state == BGP4_NS_ACTIVE))
             {
                 /*action on connection opened.send open */
                 //p_peer->stat.rx_msg = 0;
                 //p_peer->stat.tx_msg = 0;
 
                 /*no need connection more*/
                 bgp4_timer_stop(&p_peer->retry_timer);
 
                 /*build open msg*/
                 bgp4_open_output(p_peer);
                 bgp4_timer_start(&p_peer->hold_timer, p_peer->neg_hold_interval);
                 p_peer->state = BGP4_NS_OPENSENT;
             }
             break;

        case BGP4_EVENT_TCP_CLOSED:
             gbgp4.stat.tcp_close++;
             /*restarting process*/
              /*if peer is restarter or receiver,remain current
                restart role,else enter receiver role if supported*/
              p_peer->notify.code = BGP4_CEASE;
              p_peer->notify.sub_code = BGP4_ADMINISTRATIVE_SHUTDOWN;
             if (p_peer->restart_role != BGP4_GR_ROLE_NORMAL)
             {
                 if (p_peer->state == BGP4_NS_OPENSENT)
                 {
                     p_peer->state = BGP4_NS_ACTIVE;
                     bgp4_peer_sock_close(p_peer);
                     bgp4_timer_start(&p_peer->retry_timer, p_peer->retry_interval);
                     break;
                 }
                 bgp4_fsm_invalid(p_peer);
                 break;
             }
             /*remote close action,peer may be in restarting*/
             if (bgp4_remote_grace_restart(p_peer) == VOS_OK)
             {
                 break;
             }
 
             if (p_peer->state == BGP4_NS_OPENSENT)
             {
                 p_peer->state = BGP4_NS_ACTIVE;
                 bgp4_peer_sock_close(p_peer);
                 bgp4_timer_start(&p_peer->retry_timer, p_peer->retry_interval);
                 break;
             }
             bgp4_fsm_invalid(p_peer);
             break;

        case BGP4_EVENT_FATAL_ERROR:
             p_peer->notify.code = BGP4_CEASE;
             p_peer->notify.sub_code = BGP4_CONNECTION_COLLISION_RESOLUTION;
             bgp4_fsm_invalid(p_peer);
             break;

        case BGP4_EVENT_CONNECT_FAIL:
             if (p_peer->state == BGP4_NS_CONNECT)
             {
                 bgp4_timer_start(&p_peer->retry_timer, p_peer->retry_interval);
                 p_peer->state = BGP4_NS_ACTIVE;
             }
             else if (p_peer->state == BGP4_NS_ACTIVE)
             {
                 bgp4_peer_sock_close(p_peer);
                 bgp4_timer_start(&p_peer->retry_timer, p_peer->retry_interval);
             }
             break;

        case BGP4_EVENT_RETRY_TIMEOUT:
             if ((p_peer->state == BGP4_NS_CONNECT)
                || (p_peer->state == BGP4_NS_ACTIVE))
             {
                 bgp4_timer_start(&p_peer->retry_timer, p_peer->retry_interval);
                 bgp4_peer_sock_close(p_peer);
                 bgp4_peer_sock_connect(p_peer);
                 p_peer->state = BGP4_NS_CONNECT;
             }
             break;

        case BGP4_EVENT_HOLD_TMR_EXP:
             if (p_peer->state >= BGP4_NS_OPENSENT)
             {
                 p_peer->notify.code = BGP4_HOLD_TMR_EXPIRED_ERR;
                 bgp4_notify_output(p_peer);
             }
             p_peer->notify.code = BGP4_HOLD_TMR_EXPIRED_ERR;
             p_peer->notify.sub_code = 0;
             bgp4_fsm_invalid(p_peer);
             break;

        case BGP4_EVENT_KEEPALIVE_TMR_EXP:
             if ((p_peer->state == BGP4_NS_OPENCONFIRM) 
                || (p_peer->state == BGP4_NS_ESTABLISH))
             {
                 bgp4_timer_start(&p_peer->keepalive_timer, bgp4_rand(p_peer->neg_keep_interval));
                 bgp4_keepalive_output(p_peer);
             }
             break;

        case BGP4_EVENT_GR_TMR_EXP:
             bgp4_peer_restart_timeout(p_peer);
             break;

        default:
            break;
    }
    return;
}

u_char 
bgp4_peer_type(tBGP4_PEER *p_peer)
{
     if (p_peer->as == gbgp4.as)
     {
         return BGP4_IBGP;
     }
     if (bgp4_confedration_as_lookup(p_peer->as))
     {
         return BGP4_CONFEDBGP;
     }
     return BGP4_EBGP;
}

void 
bgp4_peer_af_set(
     tBGP4_PEER *p_peer,
     u_int af_cmd)
{
    long af = 0;

    af = (af_cmd) & (~0xf0000000);
    if (af >= BGP4_PF_MAX)
    {
        return;
    }

    if (!(af_cmd & 0xf0000000))
    {
        /*enable 6PE peer afi*/
        if ((af == BGP4_PF_IP6UCAST)
            && (p_peer->local_ip.afi == BGP4_PF_IPUCAST))
        {
            flag_set(p_peer->local_mpbgp_capability, BGP4_PF_IP6LABEL);
        }
        else
        {
            flag_set(p_peer->local_mpbgp_capability, af);
        }
    }
    else
    {
        /*disable 6PE peer afi*/
        if ((af == BGP4_PF_IP6UCAST)
            && (p_peer->local_ip.afi == BGP4_PF_IPUCAST))
        {
            flag_clear(p_peer->local_mpbgp_capability, BGP4_PF_IP6LABEL);
        }
        else
        {
            flag_clear(p_peer->local_mpbgp_capability, af);
        }
    }
    return;
}

void
bgp4_peer_gr_expired(tBGP4_PEER* p_peer)
{
    bgp4_fsm(p_peer, BGP4_EVENT_GR_TMR_EXP);
    return;
}

void
bgp4_peer_holdtimer_expired(tBGP4_PEER* p_peer)
{
    bgp4_fsm(p_peer, BGP4_EVENT_HOLD_TMR_EXP);
    return;
}

void
bgp4_peer_keepalive_timer_expired(tBGP4_PEER* p_peer)
{
    bgp4_fsm(p_peer, BGP4_EVENT_KEEPALIVE_TMR_EXP);
    return;
}

void
bgp4_peer_connect_timer_expired(tBGP4_PEER* p_peer)
{
    /*route is halted,do nothing*/
    if (p_peer->admin_state == BGP4_PEER_HALTED)
    {
        return;
    }

    /*ignored if already established*/
    if ((p_peer->state == BGP4_NS_OPENCONFIRM)
        || (p_peer->state == BGP4_NS_ESTABLISH))
    {
        return;
    }
        
    bgp4_peer_sock_close(p_peer);
    p_peer->state = BGP4_NS_IDLE;
        
    bgp4_fsm(p_peer, BGP4_EVENT_START);
    return;
}

void
bgp4_peer_retry_timer_expired(tBGP4_PEER* p_peer)
{
    bgp4_fsm(p_peer, BGP4_EVENT_RETRY_TIMEOUT);
    return;
}

void
bgp4_peer_delete_expired(tBGP4_PEER* p_peer)
{
    u_char str[64];

    /*send notify*/
    
    /*if there is any route learnt from peer,do not delete
     peer now,waiting for next loop*/
    if (bgp4_peer_route_exist(p_peer) == TRUE)
    {
        bgp4_timer_start(&p_peer->delete_timer, 1);
        return;
    }
    bgp4_log(BGP_DEBUG_FSMCHANGE, "peer %s route cleared,delete peer",
        bgp4_printf_peer(p_peer, str)); 
    
    /*all learnt route deleted,we can delete peer now*/
    bgp4_peer_delete(p_peer);
    return;
}    

void
bgp4_peer_adjout_expire(tBGP4_PEER *p_peer)
{
    tBGP4_UPDATE_PACKET packet; 
    tBGP4_VPN_INSTANCE *p_instance = p_peer->p_instance;
    tBGP4_RIB *p_rib = NULL;    
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_FLOOD_FLAG *p_flood = NULL;
    tBGP4_FLOOD_FLAG *p_next = NULL;
    u_int af = 0;
    u_int max_last_len = (gbgp4.max_update_len*3)/4;
    
    /*do not send any packet during restart*/
    /*GR:before sending init update,restarting node must wait for
    end-of-rib from all valid peer.*/
    if (gbgp4.in_restarting == TRUE)
    {
        bgp4_log(BGP_DEBUG_RESET, "restart node is waiting for end  of Route Information Base,do not send update");

        bgp4_timer_start(&p_peer->adj_out_timer, BGP4_PEER_ADJOUT_INTERVAL);
        return;
    } 
    
    if (p_peer->state != BGP4_NS_ESTABLISH)
    {
        bgp4_peer_flood_clear(p_peer);
        return;
    }

    /*if current update limit timer not running,start it*/
    if (!bgp4_timer_is_active(&gbgp4.update_ratelimit_timer))
    {
        bgp4_timer_start_ms(&gbgp4.update_ratelimit_timer, 100);
    }

    /*if current update size exceed limit,do not send now*/
    if (gbgp4.update_tx_len >= BGP4_MAX_FLOOD_SIZE)
    {
        bgp4_timer_start_ms(&p_peer->adj_out_timer, 50);
        return;
    }
    
    memset(&packet, 0, sizeof(packet));
    packet.p_peer = p_peer; 
    packet.p_buf = bgp4_malloc(BGP4_MAX_MSG_LEN, MEM_BGP_BUF);
    packet.p_mp_reach = bgp4_malloc(BGP4_MAX_MSG_LEN, MEM_BGP_BUF);
    packet.p_mp_unreach = bgp4_malloc(BGP4_MAX_MSG_LEN, MEM_BGP_BUF);
    p_peer->p_txupdate = &packet;

    bgp4_update_packet_init(&packet);
    /*scan all rib of instance*/
    for (af = 0; af < BGP4_PF_MAX ; af++)
    {
        /*ignore unsupported af*/
        if (!bgp4_af_support(p_peer, af))
        {
            continue;
        }
        p_rib = p_instance->rib[af];
        if (p_instance->rib[af] == NULL)
        {
            continue;
        }
        /*if some system update is pending,wait for finish*/
        if (p_rib->system_check_need == TRUE)
        {
            bgp4_timer_start(&p_peer->adj_out_timer, BGP4_PEER_ADJOUT_INTERVAL);
            goto END;
        }

        /*set af*/
        packet.af = af;
        
        /*check all withdraw route*/
        bgp4_avl_for_each_safe(&p_rib->out_withdraw_table, p_flood, p_next)
        {
            p_route = p_flood->p_route;

            /*IGP sync check*/
            if (p_route->igp_sync_wait == TRUE)
            {
                /*ignore non-sync routes temporarily*/
                bgp4_route_flood_clear(p_route);
                continue;
            }
            /*add route to peer's update buffer*/
            if (!bgp4_withdraw_flood_isset(p_route, p_peer))
            {
                continue;
            }
            if (bgp4_update_output_verify(p_peer, p_route) != TRUE)
            {
                /*clear update flag of this route*/
                bgp4_withdraw_flood_clear(p_route, p_peer);
                continue;
            }
            
            /*if too many packet sent,wait for next sending*/
            if ((gbgp4.update_tx_len >= BGP4_MAX_FLOOD_SIZE)
                && ((packet.len + packet.mp_reach_len + 
                     packet.mp_unreach_len) >= max_last_len))
            {
                bgp4_update_output(&packet);
                
                bgp4_timer_start_ms(&p_peer->adj_out_timer, 50);
                goto END; 
            }
            /*clear update flag of this route*/
            bgp4_withdraw_flood_clear(p_route, p_peer);
            
            while (bgp4_update_packet_withdraw_insert(&packet, p_route) != VOS_OK)
            {
                gbgp4.update_tx_len += bgp4_update_output(&packet);
            }
        }

        bgp4_avl_for_each_safe(&p_rib->out_feasible_table, p_flood, p_next)
        {
            p_route = p_flood->p_route;
        
            /*IGP sync check*/
            if (p_route->igp_sync_wait == TRUE)
            {
                /*ignore non-sync routes temporarily*/
                bgp4_route_flood_clear(p_route);
                continue;
            }
            if (!bgp4_feasible_flood_isset(p_route, p_peer))
            {
                continue;
            }
            
            if (bgp4_update_output_verify(p_peer, p_route) != TRUE)
            {                
                /*clear update flag of this route*/
                bgp4_feasible_flood_clear(p_route, p_peer);
                continue;
            }
            
            /*if too many packet sent,wait for next sending*/
            if ((gbgp4.update_tx_len >= BGP4_MAX_FLOOD_SIZE)
                && ((packet.len + packet.mp_reach_len + 
                     packet.mp_unreach_len) >= max_last_len))
            {
                bgp4_update_output(&packet);
                
                bgp4_timer_start_ms(&p_peer->adj_out_timer, 50);
                goto END; 
            }
            
            /*clear update flag of this route*/
            bgp4_feasible_flood_clear(p_route, p_peer);
            
            while (bgp4_update_packet_feasible_insert(&packet, p_route) != VOS_OK)
            {
                gbgp4.update_tx_len += bgp4_update_output(&packet);
            }
        }

        /*all route checked,send update at last*/
        bgp4_update_output(&packet);

        /*check end-of-rib send*/
        /*if local restatred,we must wait for endofrib before sending it*/
        if (p_peer->restart_role == BGP4_GR_ROLE_RESTART)
        {
            if (!flag_isset(p_peer->rxd_rib_end, p_rib->af))
            {
                break;
            }
        }
        /*send end-of-rib at last,if it has not been sent before*/
        if (!flag_isset(p_peer->txd_rib_end, p_rib->af))
        {
            bgp4_end_of_rib_output(p_peer, p_rib->af);
        }   

        /*schedule clear unused route*/
        bgp4_timer_start(&p_rib->route_clear_timer, BGP4_ROUTE_CLEAR_INTERVAL);
    }
END:    
    bgp4_free(packet.p_buf, MEM_BGP_BUF);
    bgp4_free(packet.p_mp_reach, MEM_BGP_BUF);
    bgp4_free(packet.p_mp_unreach, MEM_BGP_BUF);
    return;
}

#else


#include "bgp4com.h"

extern  int sendBgpBackwardTransitionTrap(int addrType,char* peerid,
                                                                int state,char *LastError);

tBGP4_PEER * bgp4_add_peer(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR *ip)
{
    u_int i = 0;
    tBGP4_PEER *p_new = NULL;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_PEER *p_prev = NULL;

    if(p_instance == NULL)
    {
        return NULL;
    }

    p_new = bgp4_malloc(sizeof(tBGP4_PEER),MEM_BGP_PEER);

    if (p_new == NULL)
    {
        return (NULL);
    }

    memset(p_new,0,sizeof(tBGP4_PEER));
    p_new->p_instance = p_instance;
    p_new->retry_interval = BGP4_DFLT_CONNRETRYINTERVAL;
    p_new->hold_interval = BGP4_DFLT_HOLDINTERVAL;
    p_new->keep_interval = BGP4_DFLT_KEEPALIVEINTERVAL;
    p_new->origin_interval = BGP4_DFLT_MINASORIGINTERVAL;
    p_new->adv_interval  = BGP4_DFLT_MINROUTEADVINTERVAL;
    p_new->state          = BGP4_NS_IDLE;
    p_new->start_interval  = BGP4_DFLT_STARTINTERVAL;
    p_new->version         = BGP4_VERSION_4;
    p_new->admin       = BGP4_PEER_HALTED;

    p_new->bfd_discribe = BGP4_BFD_DISABLE;

    /*add for cap advertisement*/
    p_new->send_open_option = TRUE;
    p_new->unsupport_capability = FALSE;

    p_new->neg_hold_interval = p_new->hold_interval;
    p_new->neg_keep_interval = p_new->keep_interval;

    p_new->ttl_hops=255;
    p_new->sock = BGP4_INVALID_SOCKET;

    p_new->fake_as = gBgp4.asnum;

    /*support ipv4 unicast by default*/
    if(ip->afi==BGP4_PF_IPUCAST)
    {
        p_new->local.ip.afi = BGP4_PF_IPUCAST;
        af_set(p_new->local.af,BGP4_PF_IPUCAST);
        af_set(p_new->local.capability,BGP4_PF_IPUCAST);
    }

#ifdef BGP_IPV6_WANTED
    if(ip->afi==BGP4_PF_IP6UCAST)
    {
        p_new->local.ip.afi = BGP4_PF_IP6UCAST;
        af_set(p_new->local.af,BGP4_PF_IP6UCAST);
        af_set(p_new->local.capability,BGP4_PF_IP6UCAST);
    }
#endif

    memcpy(&p_new->remote.ip, ip, sizeof(tBGP4_ADDR));

    bgp4_lstnodeinit(&p_new->node);

    /*sorted by ipaddress*/
    LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER)
    {
        if (bgp4_prefixcmp(&p_peer->remote.ip, ip) > 0)
            break;
        else
            p_prev = p_peer;
    }

    if (p_prev)
        bgp4_lstinsert(&p_instance->peer_list, &p_prev->node, &p_new->node);
    else
        bgp4_lstadd(&p_instance->peer_list, &p_new->node);

    /*policy list*/
    bgp4_lstinit(&p_new->rt_policy_import);
    bgp4_lstinit(&p_new->rt_policy_export);

    /*allocate index for peer*/
    for (i = 1; i < BGP4_MAX_PEER_ID ; i++)
    {
        if (gBgp4.peer_array[i] == NULL)
        {
            gBgp4.peer_array[i] = p_new;
            p_new->bit_index = i ;
            break;
        }
    }

    return (p_new);
}

void bgp4_delete_peer(tBGP4_PEER *p_peer)
{
    u_int i = 0 ;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    if(p_peer == NULL)
        return;

    p_instance = p_peer->p_instance;

    if(p_instance == NULL)
    {
        return;
    }

    /*wait for all update finished*/
    bgp4_rib_walkup(TRUE);

    /*free index for peer*/
    for (i = 1; i < BGP4_MAX_PEER_ID ; i++)
    {
        if (gBgp4.peer_array[i] == p_peer)
        {
            gBgp4.peer_array[i] = NULL;
            break;
        }
    }

    if (p_peer->rr_client == TRUE)
    {
        if (gBgp4.stat.rr_client !=0)
        {
            gBgp4.stat.rr_client --;
            if(gBgp4.stat.rr_client ==0)
                gBgp4.is_reflector = 0;
        }
        else
            gBgp4.is_reflector = 0;
    }

    /*stop all timers*/
    for (i = 0 ; i < BGP4_MAX_TIMER; i++)
    {
        bgp4_stop_timer(&p_peer->timer[i]);
    }

    /*sync peer*/
    if(gBgp4.work_mode == BGP4_MODE_MASTER)
    {
        bgp4_send_sync_peer(p_peer,0);
    }

    bgp4_delete_policy_list(&p_peer->rt_policy_import);

    bgp4_delete_policy_list(&p_peer->rt_policy_export);

    bgp4_lstdelete(&p_instance->peer_list, &p_peer->node);

    bgp4_free(p_peer, MEM_BGP_PEER);
    return;
}

/*lookup peer according to ip address*/
tBGP4_PEER *bgp4_peer_lookup(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR *ip)
{
    tBGP4_PEER *p_peer = NULL;

    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 peer lookup,input instance null");
        return NULL;
    }

    LST_LOOP (&p_instance->peer_list,p_peer, node, tBGP4_PEER)
    {
        if (bgp4_prefixcmp(ip, &p_peer->remote.ip) == 0)
        {
            return p_peer;
        }
    }
    return NULL;
}

void bgp4_delete_all_peer_route()
{
    tBGP4_PEER *p_peer = NULL;
    tBGP4_PEER *p_next_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_VPN_INSTANCE* p_instance_next = NULL;

    LST_LOOP_SAFE(&gBgp4.vpn_instance_list,p_instance,p_instance_next,node,tBGP4_VPN_INSTANCE)
    {
        LST_LOOP_SAFE(&p_instance->peer_list, p_peer, p_next_peer, node, tBGP4_PEER)
        {
            bgp4_fsm_invalid(p_peer);
        }
    }

    return;
}

void bgp4_delete_all_peer(tBGP4_LIST* p_peer_list)
{
    tBGP4_PEER *p_peer = NULL;
    tBGP4_PEER *p_next_peer = NULL;

    LST_LOOP_SAFE(p_peer_list, p_peer, p_next_peer, node, tBGP4_PEER)
    {
        bgp4_stop_start_timer(p_peer);
        bgp4_stop_retry_timer(p_peer);
        bgp4_stop_hold_timer(p_peer);
        bgp4_stop_keepalive_timer(p_peer);

        bgp4_sock_close(p_peer );
        bgp4_delete_peer(p_peer);
    }
    return ;
}

void bgp4_reset_all_peers(u_int force_flag)
{
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    bgp4_log(BGP_DEBUG_EVT,1,"bgp4 reset all peers!");


    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
        LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER)
        {
            if((bgp4_peer_execute_gr(p_peer,BGP4_GR_ROLE_RESTART) == FALSE) &&
                force_flag == 1)
            {
                p_peer->cease_reason = 4;
                bgp4_fsm_invalid(p_peer);
            }
        }
    }
}

/*obtain invalid event*/
void bgp4_fsm_invalid(tBGP4_PEER *p_peer)
{
    u_int af = 0 ;
    u_char peer_str[64] = {0};
    u_short error_code=p_peer->last_errcode;

    error_code =  (error_code << 8) + p_peer->last_subcode;

    /*end rib for established state*/
    if (p_peer->state == BGP4_NS_ESTABLISH && p_peer->reset_role == BGP4_GR_ROLE_NORMAL)
    {
        bgp4_log(BGP_DEBUG_RT,1,"peer %s invalid,release routes learned from it",
                    bgp4_printf_peer(p_peer,peer_str));
        p_peer->uptime = time(NULL) ;
        for (af = 0 ; af < BGP4_PF_MAX; af++)
        {
            bgp4_end_peer_route(af, p_peer,0,1);
        }
    }

    if(gBgp4.work_mode==BGP4_MODE_SLAVE)
    {
            return;
    }

    /*stop all timers*/
    bgp4_stop_start_timer(p_peer);
    bgp4_stop_retry_timer(p_peer);
    bgp4_stop_hold_timer(p_peer);
    bgp4_stop_keepalive_timer(p_peer);

    bgp4_sock_close(p_peer);

    if(p_peer->state == BGP4_NS_ESTABLISH)
    {
        bgp4_log(BGP_DEBUG_FSMCHANGE,1,"BGP4 neighbor %s state : from Established to Idle...",bgp4_printf_peer(p_peer,peer_str)); 
    }

    p_peer->state = BGP4_NS_IDLE;
    if (p_peer->unsupport_capability == TRUE)
    {
        bgp4_log(BGP_DEBUG_CMN,1,"peer has unsupported capability,do not restart it!");
        return;
    }
    if (gBgp4.trap_enable)
    {
        if(p_peer->remote.ip.afi==BGP4_PF_IPUCAST)
        {
            sendBgpBackwardTransitionTrap(AF_INET,p_peer->remote.ip.ip,p_peer->state,&error_code);
        }
        else if(p_peer->remote.ip.afi==BGP4_PF_IP6UCAST)
        {
            sendBgpBackwardTransitionTrap(AF_INET6,p_peer->remote.ip.ip,p_peer->state,&error_code);
        }
    }
    /*prepare next connection*/
    if(p_peer->admin != BGP4_PEER_HALTED)
    {
        bgp4_start_start_timer(p_peer);
        bgp4_update_start_time(p_peer);
    }

    return;
}
void
bgp4_fsm_start(tBGP4_PEER *p_peer)
{
    if (p_peer->state != BGP4_NS_IDLE || p_peer->admin!=BGP4_PEER_RUNNING)
    {
        return;
    }
    bgp_ip4(p_peer->local.ip.ip)= gBgp4.router_id;
    p_peer->local.as = gBgp4.asnum;
    p_peer->send_capability = p_peer->local.capability;

    if(p_peer->local.refresh)
        af_set(p_peer->send_capability,BGP4_RTREFRESH);

    if (p_peer->unsupport_capability)
    {
        bgp4_log(BGP_DEBUG_TCP,1,"active tcp connection is disabled for upsupport capability");
        return ;
    }

    /*open connection*/
    p_peer->state=BGP4_NS_CONNECT;
    if (bgp4_sock_open(p_peer) == TRUE)
    {
        /*if tcp connection is connected,break*/
        if(p_peer->state < BGP4_NS_OPENSENT)
        {
            /*tcp connection ok but not connected,wait for peer up*/
            bgp4_start_retry_timer(p_peer);
        }
    }
    else
    {
        /*prepare next open*/
        bgp4_start_start_timer(p_peer);
        bgp4_update_start_time(p_peer);
    }

    /*add md5*/
    if ((p_peer->md5_support) && (p_peer->md5_server_set == FALSE))
    {
        if (p_peer->remote.ip.afi == BGP4_PF_IPUCAST)
        {
            bgp4_set_sock_md5_key(gBgp4.server_sock, 1, p_peer->md5key, p_peer->md5key_len, BGP4_PF_IPUCAST, &p_peer->remote.ip);
        }
        else if (p_peer->remote.ip.afi == BGP4_PF_IP6UCAST)
        {
            bgp4_set_sock_md5_key(gBgp4.server_sock6, 1, p_peer->md5key, p_peer->md5key_len, BGP4_PF_IP6UCAST, &p_peer->remote.ip);
        }
        p_peer->md5_server_set = TRUE;
    }
    if(p_peer->ttl_hops>0&&p_peer->ttl_hops<256)
    {
        if (p_peer->remote.ip.afi == BGP4_PF_IPUCAST)
        {
            bgp4_tcp_set_peer_ttl(gBgp4.server_sock, p_peer);
        }
        else if (p_peer->remote.ip.afi == BGP4_PF_IP6UCAST)
        {
            bgp4_tcp_set_peer_ttl(gBgp4.server_sock6, p_peer);
        }
    }
    return;
}

void
bgp4_fsm_stop(tBGP4_PEER *p_peer)
{
    if (p_peer->admin == BGP4_PEER_HALTED)
    {
        return ;
    }

    /*peer is administrtive down,no graceful-restart*/
    bgp4_peer_exit_gr(p_peer);

    /*send cease to peer*/
    if ((p_peer->state == BGP4_NS_OPENSENT) ||
        (p_peer->state == BGP4_NS_OPENCONFIRM) ||
        (p_peer->state == BGP4_NS_ESTABLISH))
    {
        bgp4_send_notify(p_peer, BGP4_CEASE, p_peer->cease_reason, NULL, 0);
    }

    if ((p_peer->md5_support) && (p_peer->md5_server_set == TRUE))
    {
        if (p_peer->remote.ip.afi == BGP4_PF_IPUCAST)
        {
            bgp4_set_sock_md5_key(gBgp4.server_sock, 0, p_peer->md5key, p_peer->md5key_len, BGP4_PF_IPUCAST, &p_peer->remote.ip);
        }
        else if (p_peer->remote.ip.afi == BGP4_PF_IP6UCAST)
        {
            bgp4_set_sock_md5_key(gBgp4.server_sock6, 0, p_peer->md5key, p_peer->md5key_len, BGP4_PF_IP6UCAST, &p_peer->remote.ip);
        }
        p_peer->md5_server_set = FALSE;
    }

    /*close socket*/
    bgp4_fsm_invalid(p_peer);

    /*clear some flags*/
    p_peer->send_open_option = TRUE;
    p_peer->unsupport_capability = FALSE;
    p_peer->remote.capability = 0;

    return ;
}

void bgp4_fsm(tBGP4_PEER *p_peer,u_short event)
{
    u_char addrstr[64];
    u_char statestr[32] ;
    u_char evtstr[64];

    if ((gBgp4.dbg_flag & BGP_DEBUG_EVT)&&(event!=BGP4_EVENT_KEEPALIVE_TMR_EXP))
    {
        bgp4_log(BGP_DEBUG_EVT,1,"fsm:neighbor:%s,state:%s,event:%s",
            bgp4_printf_peer(p_peer,addrstr),
            bgp4_printf_state(p_peer->state,statestr),
            bgp4_printf_event(event,evtstr));
    }
    else if((gBgp4.dbg_flag & BGP_DEBUG_CMN)&&(event==BGP4_EVENT_KEEPALIVE_TMR_EXP))
    {
        bgp4_log(BGP_DEBUG_CMN,1,"fsm:neighbor:%s,state:%s,event:%s",
            bgp4_printf_peer(p_peer,addrstr),
            bgp4_printf_state(p_peer->state,statestr),
            bgp4_printf_event(event,evtstr));
    }

    p_peer->event = event;/*recording last event for sync peer state*/
    if(gBgp4.work_mode == BGP4_MODE_MASTER)
    {
        bgp4_send_sync_fsm(p_peer);
    }

    switch (event) {
        case BGP4_EVENT_START:
            bgp4_fsm_start(p_peer);
                    break;

        case BGP4_EVENT_STOP:
                    bgp4_fsm_stop(p_peer);
                    break;

        case BGP4_EVENT_TCP_OPENED:
                if(bgp4_peer_down_to_gr(p_peer) == TRUE)
                        break;

            if ((p_peer->state == BGP4_NS_CONNECT) || (p_peer->state == BGP4_NS_ACTIVE))
            {

                /*action on connection opened.send open */
                p_peer->rx_msg = 0;
                p_peer->tx_msg = 0;

                /*no need connection more*/
                bgp4_stop_retry_timer(p_peer);

                /*build open msg*/
                bgp4_send_open(p_peer);
                bgp4_restart_hold_timer(p_peer);
                p_peer->state = BGP4_NS_OPENSENT ;
            }
            break;

        case BGP4_EVENT_TCP_CLOSED:
        {
            gBgp4.tcp_connection_closed_times ++;
            if(bgp4_peer_down_to_gr(p_peer) == TRUE)
                break;

            if(p_peer->state == BGP4_NS_OPENSENT)
            {
                p_peer->state = BGP4_NS_ACTIVE;
                bgp4_sock_close(p_peer );
                bgp4_restart_retry_timer(p_peer);
                break;
            }
            bgp4_fsm_invalid(p_peer);
            break;

        }

        case BGP4_EVENT_FATAL_ERROR:
        {
            if(gBgp4.stat.mem[MEM_BGP_ROUTE].fail == 0)
            {
                bgp4_log(BGP_DEBUG_EVT,1,"BGP4_EVENT_FATAL_ERROR,reset peer!");
                bgp4_fsm_invalid(p_peer);
            }
            else
            {
                bgp4_log(BGP_DEBUG_EVT,1,"BGP4_EVENT_FATAL_ERROR,memory exhausted!");
            }


            break;
        }

        case BGP4_EVENT_CONNECT_FAIL:
        {
            if (p_peer->state == BGP4_NS_CONNECT)
            {
                bgp4_restart_retry_timer(p_peer);
                p_peer->state = BGP4_NS_ACTIVE;
            }
            else if (p_peer->state == BGP4_NS_ACTIVE)
            {
                bgp4_sock_close(p_peer );
                bgp4_restart_retry_timer(p_peer);
            }
            break;

        }

        case BGP4_EVENT_RETRY_TIMEOUT:
        {
            if (p_peer->state == BGP4_NS_CONNECT || p_peer->state == BGP4_NS_ACTIVE)
            {
                bgp4_start_retry_timer(p_peer);
                bgp4_sock_close(p_peer );
                bgp4_sock_open(p_peer);
                p_peer->state = BGP4_NS_CONNECT;
            }
            break;
        }
        case BGP4_EVENT_HOLD_TMR_EXP:
        {
            if (p_peer->state >= BGP4_NS_OPENSENT)
            {
                bgp4_send_notify(p_peer, BGP4_HOLD_TMR_EXPIRED_ERR, 0, NULL, 0);
            }
            bgp4_fsm_invalid(p_peer);
            break;

        }

        case BGP4_EVENT_KEEPALIVE_TMR_EXP:
        {
            if ((p_peer->state == BGP4_NS_OPENCONFIRM) ||(p_peer->state == BGP4_NS_ESTABLISH))
            {
                bgp4_start_keepalive_timer(p_peer);
                bgp4_send_keepalive(p_peer);
            }
            else
            {
                if(p_peer->state == BGP4_NS_OPENSENT)
                    bgp4_send_notify(p_peer, BGP4_FSM_ERR, 0, NULL, 0);
                bgp4_fsm_invalid(p_peer);
            }
            break;
        }


        case BGP4_EVENT_GR_TMR_EXP:
        {
            bgp4_peer_gr_timeout(p_peer);
            break;

        }

        default:
            break;
    }
    return;
}

u_char bgp4_peer_type(tBGP4_PEER *p_peer)
{
     if (p_peer->remote.as == gBgp4.asnum)
     {
           return BGP4_IBGP;
     }
     if (bgp4_is_confed_peer(p_peer->remote.as) == TRUE)
     {
           return BGP4_CONFEDBGP;
     }
     return BGP4_EBGP;
}

void bgp4_set_peer_af(tBGP4_PEER *p_peer,u_int af_cmd)
{
    long af = 0;

    af = (af_cmd) & (~0xf0000000);
    if (af >= BGP4_PF_MAX)
    {
        return;
    }

    if (!(af_cmd & 0xf0000000))
    {
        /*enable 6PE peer afi*/
        if(af == BGP4_PF_IP6UCAST
                && p_peer->local.ip.afi == BGP4_PF_IPUCAST)
        {
            af_set(p_peer->local.af, BGP4_PF_IP6LABEL);
            af_set(p_peer->local.capability, BGP4_PF_IP6LABEL);
            af_set(p_peer->send_capability, BGP4_PF_IP6LABEL);
        }
        else
        {
            af_set(p_peer->local.af, af);
            af_set(p_peer->local.capability, af);
            af_set(p_peer->send_capability, af);
        }

    }
    else
    {
        /*disable 6PE peer afi*/
        if(af == BGP4_PF_IP6UCAST
                && p_peer->local.ip.afi == BGP4_PF_IPUCAST)
        {
            af_clear(p_peer->local.af, BGP4_PF_IP6LABEL);
            af_clear(p_peer->local.capability, BGP4_PF_IP6LABEL);
            af_clear(p_peer->send_capability, BGP4_PF_IP6LABEL);
        }
        else
        {
            af_clear(p_peer->local.af, af);
            af_clear(p_peer->local.capability, af);
            af_clear(p_peer->send_capability, af);
        }

    }
}

/*if interface address changed,the peer from it should be restart,in order to stay the same with kernals*/
void bgp4_peer_if_addr_change(tBGP4_ADDR* p_if_addr,u_int if_unit)
{
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
        LST_LOOP (&p_instance->peer_list,p_peer, node, tBGP4_PEER)
        {
            /*different peer from different instance may has the same IP addr.,IF unit can help us to distinguish*/
            if (bgp4_prefixcmp(p_if_addr , &(p_peer->local.ip)) == 0 &&
                p_peer->if_unit == if_unit)
            {
                /*if both of peers support GR,then go to GR process,otherwise,go to normal restart process*/
                if(bgp4_peer_execute_gr(p_peer,BGP4_GR_ROLE_RESTART) == FALSE)
                {
                    /*if still in GR process,should quit*/
                    if(gBgp4.gr_enable)
                    {
                        bgp4_peer_exit_gr(p_peer);
                    }

                    /*common peer reset*/
                    bgp4_fsm_invalid(p_peer);
                }
            }
        }

    }


    return;
}


#endif
