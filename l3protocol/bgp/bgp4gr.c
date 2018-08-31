#include "bgp4_api.h"

#ifdef NEW_BGP_WANTED
#include "bgp4com.h"

/*bgp4_make_route_stale
set route's stale flag to true according to special afi/safi and peer
i point to peer;afi flag
o none
r none
*/
void 
bgp4_peer_stale_route_set(
        tBGP4_PEER *p_peer,
        u_int af)
{
    tBGP4_PATH *p_path = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_RIB *p_rib = p_peer->p_instance->rib[af];
    u_char str[64] = {0};

    if (p_rib == NULL)
    {
        return ;
    }
    /*delete all current stale routes*/  
    bgp4_peer_stale_route_clear(af, p_peer);

    /*set stale flag for all remaining routes*/
    bgp4_log(BGP_DEBUG_EVT, "set peer %s af %d route as stale", bgp4_printf_peer(p_peer, str), af);
    
    bgp4_avl_for_each(&p_rib->path_table, p_path)
    {
        if (p_path->p_peer == p_peer)
        {
            bgp4_avl_for_each(&p_path->route_list, p_route)
            {
                p_route->stale = TRUE;
            }
        }        
    }
    return;    
}

/*local node start gr process*/
int
bgp4_local_grace_restart(tBGP4_PEER *p_peer)
{
    u_int af = 0;
    u_char str[64];

    /*both sides supporte restart capability,and current is 
      not in restarting*/
    if ((gbgp4.restart_enable != TRUE) 
        || (p_peer->restart_enable != TRUE)
        || (p_peer->state != BGP4_NS_ESTABLISH)
        || (p_peer->restart_role != BGP4_GR_ROLE_NORMAL))
    {
        bgp4_log(BGP_DEBUG_CMN, "peer %s can not enter restart",
            bgp4_printf_peer(p_peer, str));      
        bgp4_log(BGP_DEBUG_CMN, "restart support local %d,remote %d",
            gbgp4.restart_enable, p_peer->restart_enable); 
        bgp4_log(BGP_DEBUG_CMN, "peer state %d,restart role %d",
            p_peer->state, p_peer->restart_role); 
        return VOS_ERR;
    }
    bgp4_log(BGP_DEBUG_CMN, "peer %s local restart",
            bgp4_printf_peer(p_peer, str));        

    /*Clear route learnt from peer,if local forwarding 
    flag set,set these route to stale,else delete*/
    for (af = 0 ; af < BGP4_PF_MAX ; af++)
    {
        if (flag_isset(p_peer->local_mpbgp_capability, af))
        {
            bgp4_peer_stale_route_set(p_peer, af);
        } 
        else
        {            
            bgp4_peer_route_clear(af, p_peer);    
        }
    }
            
   /*
       Set peer.local-Rbit 
       In re-establishing the session, the 'Restart State' bit in the
       Graceful Restart Capability of the OPEN message sent by the Receiving
       Speaker MUST NOT be set unless the Receiving Speaker has restarted.
       The presence and the setting of the 'Forwarding State' bit for an
       address family depend upon the actual forwarding state and
       configuration.
   */

    /*Clear recvd end-rib-flag,wait for all ribend*/
    p_peer->rxd_rib_end = 0;

    /*record current role state is restarter*/  
    p_peer->restart_role = BGP4_GR_ROLE_RESTART;

    /*close tcp connection using close,state to be idle
     ,do not send notify*/
    bgp4_log(BGP_DEBUG_CMN, "peer %s enter graceful restart",
        bgp4_printf_peer(p_peer, str));
    
    bgp4_fsm(p_peer, BGP4_EVENT_FATAL_ERROR);
    
    /*start global reset timer*/
    if (gbgp4.in_restarting == FALSE)
    {
        bgp4_timer_start(&gbgp4.gr_waiting_timer, gbgp4.restart_wait_period);
        gbgp4.in_restarting = TRUE;
    }
    return VOS_OK;
}

/*detect peer is restarted,self is receiver*/
int 
bgp4_remote_grace_restart(tBGP4_PEER *p_peer)
{
    u_int af = 0;
    u_char str[64];
    /*both sides supporte restart capability,and current is 
      not in restarting*/
    if ((gbgp4.restart_enable != TRUE) 
        || (p_peer->restart_enable != TRUE)
        || (p_peer->state != BGP4_NS_ESTABLISH)
        || (p_peer->restart_role != BGP4_GR_ROLE_NORMAL))
    {
        return VOS_ERR;
    }
    bgp4_log(BGP_DEBUG_CMN, "peer %s restarted",
            bgp4_printf_peer(p_peer, str));        

    /*Clear route learnt from peer,if local forwarding 
    flag set,set these route to stale,else delete*/        
    for (af = 0 ; af < BGP4_PF_MAX ; af++)
    {
        if (flag_isset(p_peer->local_mpbgp_capability, af))
        {
            bgp4_peer_stale_route_set(p_peer, af);
        } 
        else
        {
            bgp4_peer_route_clear(af, p_peer);    
        }
    }
            
    /*Clear end-rib-flag*/
    p_peer->rxd_rib_end = 0;

    /*record current role state*/  
    p_peer->restart_role = BGP4_GR_ROLE_RECIEVE;

    /*close tcp connection using close,state to be idle*/
    bgp4_log(BGP_DEBUG_EVT, "peer %s enter graceful restart",
        bgp4_printf_peer(p_peer, str));
    
    bgp4_fsm(p_peer, BGP4_EVENT_FATAL_ERROR);
    
    /*start reset timer*/
    bgp4_timer_start(&p_peer->gr_timer, p_peer->restart_time);
    return VOS_OK;
}

/*bgp4_peer_restart_finish
exit gr mode by administration or notification
in point to peer
out/return none
*/
void 
bgp4_peer_restart_finish(tBGP4_PEER *p_peer)
{
    u_char str[64];
    
    bgp4_log(BGP_DEBUG_EVT, "peer %s %s restart finished",
           bgp4_printf_peer(p_peer, str), 
           p_peer->restart_role == BGP4_GR_ROLE_RECIEVE ? "remote" : "local");

    /*restore state*/
    p_peer->restart_role = BGP4_GR_ROLE_NORMAL;
        
    /*end flag clear*/
    p_peer->rxd_rib_end = 0;
    
    /*remote infor clear*/
    p_peer->in_restart = FALSE;
    
    /*stop timer*/
    bgp4_timer_stop(&p_peer->gr_timer);
    return;
}

/*bgp4_gr_reset_timeout
process reset timeout
in point to peer
out
return none
*/
void 
bgp4_peer_restart_timeout(tBGP4_PEER *p_peer)
{
    u_char addrstr[64] = {0};
    u_int af = 0;

    bgp4_log(BGP_DEBUG_CMN, "reset time out for peer %s,role %d",
           bgp4_printf_peer(p_peer, addrstr), p_peer->restart_role);

    /*clear stale route*/  
    for (af = 0; af < BGP4_PF_MAX; af++)
    {
        if (bgp4_af_support(p_peer, af))
        {
            bgp4_peer_stale_route_clear(af, p_peer);
        }
    }
    
    /*Set Stale to normal*/    
    bgp4_peer_restart_finish(p_peer);
    return;
}

/*form graceful restart capability in Open msg's Option filed
in point to peer
point to msg
out :fill capability into msg if necessary
return 0 if no capability,or else bytes filled ,including code,length and value
*/
u_short 
bgp4_restart_capability_fill(
      tBGP4_PEER *p_peer,
      u_char *p_buf)
{
    u_short bytes = 0 ;
    u_short restart_time = 0 ;
    u_char *p_len  = (p_buf + 1);/*record pointer of length*/
    u_short afi = 0;
    u_char safi = 0;
    u_char forward = 0 ;
    u_char i = 0;
    
    /*fill cap code,skip length*/
    bgp4_put_char(p_buf, BGP4_CAP_GRACEFUL_RESTART);
    bgp4_put_char(p_buf, 0);  
    
    /*fill Reset Flag and Reset Time.reset time must less than
    hold time in open
    */ 
    restart_time = gbgp4.restart_period;
    
    if ((restart_time == 0) || 
        (restart_time >= p_peer->hold_interval))
    {
        restart_time = p_peer->hold_interval - 1;
    }
    
    /*Reset Flag*/   
    if (gbgp4.in_restarting)
    {
        restart_time |= BGP4_GR_RESET_FLAG;
    }
        
    /*Fill 2bytes time&restart flag*/  
    bgp4_put_2bytes(p_buf, restart_time);
    bytes += 2;

    if (p_peer->restart_role != BGP4_GR_ROLE_NORMAL)/*reset role must be graceful restart,else clear f bit*/
    {
        forward = BGP4_GR_FORWARDING_FLAG;
    }
    
    /*fill supported AF*/  
    for (i = 0 ; i < BGP4_PF_MAX ; i++)
    {
        if (flag_isset(p_peer->local_mpbgp_capability, i))
        {
            afi = bgp4_index_to_afi(i);
            safi = bgp4_index_to_safi(i);
            bgp4_put_2bytes(p_buf, afi);                
            bgp4_put_char(p_buf, safi); 
            bgp4_put_char(p_buf, forward); 
            bytes += 4;            
        }
    }  

    /*set final length*/
    *p_len = bytes;        
    return (bytes + 2);/*including value and code,length*/
}


/*bgp4_gr_if_can_update
check if can send first update;
for restarting node,it will wait end-of-rib from all gr peer for all afi/safi
in none
out none
return bgp-success if can update;or bgp-failure if can not update
*/
void 
bgp4_local_restart_state_update(void)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    /*not in restart state*/
    if (gbgp4.in_restarting != TRUE)
    {
         return;
    }
    /*scan all peer with valid tcp connection,if one's role is restarting
    do not send update*/
    bgp4_instance_for_each(p_instance)
    {
        bgp4_peer_for_each(p_instance, p_peer) 
        {
            if (p_peer->restart_role == BGP4_GR_ROLE_RESTART)
            {
                return;
            }
        }
    }
    /*start timer for send init update when restart finished*/
    bgp4_timer_start(&gbgp4.init_update_timer, 1);

    /*fast finish GR*/
    bgp4_timer_start_ms(&gbgp4.gr_waiting_timer, 100);
    return;
}

/*bgp4_schedule_all_init_update
send first update if necessary
in none
out send update to peer
return none
*/
void 
bgp4_schedule_all_init_update(void)
{
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    u_int af = 0;
    
    /*restarting finished,schedule all init update*/
     /*stop selection deferral timer*/
    bgp4_timer_stop(&gbgp4.gr_waiting_timer);

    bgp4_instance_for_each(p_instance)
    {
        bgp4_peer_for_each(p_instance, p_peer) 
        {
            if (p_peer->state == BGP4_NS_ESTABLISH)
            {
                for (af = 0; af < BGP4_PF_MAX; af++)
                {
                    bgp4_schedule_init_update(af, p_peer);
                }
            }
        }
    }        
    gbgp4.in_restarting = FALSE;
    return;
}

/*local restart timeout,can not complete all restarting*/
void 
bgp4_restart_waiting_timeout(void)
{
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    u_int af = 0;

    if (gbgp4.in_restarting == TRUE)
    {
        gbgp4.in_restarting = FALSE;   
        bgp4_schedule_all_init_update();
    }

    /*delete all stale routes from peers which are still in GR process*/
    bgp4_instance_for_each(p_instance)
    {
        bgp4_peer_for_each(p_instance, p_peer) 
        {
            /*clear stale route*/  
            for (af = 0; af < BGP4_PF_MAX; af++)
            {
                bgp4_peer_stale_route_clear(af, p_peer);
            }
            /*stop restart*/
            bgp4_peer_restart_finish(p_peer);
        }
    }
    return;
}
#else
#include "bgp4com.h"

void bgp4_delete_stale_route(tBGP4_PEER *p_peer,u_int af,u_int force)
{
    u_char peer_addr[64];

    bgp4_end_peer_route(af,p_peer,1,force);

    bgp4_log(BGP_DEBUG_RT,1,"bgp4 delete stale route: del peer %s, af=%x",
                            bgp4_printf_peer(p_peer,peer_addr),
                            af);
    
    return;
        
}

/*bgp4_make_route_stale
set route's stale flag to true according to special afi/safi and peer
i point to peer;afi flag
o none
r none
*/
void bgp4_make_route_stale(tBGP4_PEER *p_peer,u_int af)
{
    u_char addrstr[64] = {0};
    tBGP4_PATH* p_path = NULL;
    tBGP4_ROUTE *p_route = NULL;

    /*if no root selected ,nothing to do*/
    if (p_peer == NULL)
        return ;

    if(af >= BGP4_PF_MAX)
        return;
    
    /*delete all stale routes*/  
    bgp4_delete_stale_route(p_peer,af,1);
    
    /*set stale flag for all remaining routes*/
    bgp4_log(BGP_DEBUG_EVT,1,"set peer %s af<%d> route stale!",bgp4_printf_peer(p_peer,addrstr),af);   
    
    LST_LOOP(&gBgp4.attr_list[af], p_path, node, tBGP4_PATH)
    {
        if (p_path->p_peer == p_peer)
        {
            LST_LOOP(&p_path->route_list, p_route, node, tBGP4_ROUTE)
            {
                p_route->stale = 1;/*Set the Flag*/
            }
        }
        
        
    }

    return;
    
}

/*bgp4_peer_execute_gr
    peer graceful reset operation
*/
int bgp4_peer_execute_gr(tBGP4_PEER *p_peer,u_int role/*role state wanted*/)
{
    u_int af = 0;
    u_char addr_str[64];
    
    if ((p_peer == NULL) || 
        (gBgp4.gr_enable != TRUE) ||
        (p_peer->state != BGP4_NS_ESTABLISH) ||
        (p_peer->remote.reset_enable != TRUE) ||
        (p_peer->reset_role != BGP4_GR_ROLE_NORMAL)/*last role state*/)
    {
        bgp4_log(BGP_DEBUG_CMN,1,"peer %s can not accomplish graceful restart operation",
                bgp4_printf_peer(p_peer,addr_str));        
        
            return FALSE;
    }

    /*Clear route learnt from peer,if local forwarding 
    flag set,set these route to stale,else delete*/        
    for (af = 0 ; af < BGP4_PF_MAX ; af++)
    {
        if (af_isset(p_peer->send_capability,af))
        {
            bgp4_make_route_stale(p_peer,af);
        } 
        else
        {
            bgp4_log(BGP_DEBUG_EVT,1,"gr request af<%d> do not support gr,delete such routes",af); 
            bgp4_end_peer_route(af,p_peer,0,1); 
        }
    }
            
/*
    Set peer.local-Rbit 
    In re-establishing the session, the 'Restart State' bit in the
    Graceful Restart Capability of the OPEN message sent by the Receiving
    Speaker MUST NOT be set unless the Receiving Speaker has restarted.
    The presence and the setting of the 'Forwarding State' bit for an
    address family depend upon the actual forwarding state and
    configuration.
*/
    if(role == BGP4_GR_ROLE_RESTART)
    {
        p_peer->local.reset_bit = TRUE;
    }
    else if((role == BGP4_GR_ROLE_RECIEVE)
            &&(p_peer->local.reset_bit==TRUE))
    {
        p_peer->local.reset_bit = TRUE;
    }
    else
    {
        p_peer->local.reset_bit=0;
    }

    /*Clear end-rib-flag*/
    p_peer->rib_end_af = 0 ;

    /*record current role state*/  
    p_peer->reset_role = role;

    /*close tcp connection using close,state to be idle*/
    bgp4_log(BGP_DEBUG_EVT,1,"peer %s prepare to accomplish graceful restarting ,restart role %d......",
                            bgp4_printf_peer(p_peer,addr_str),p_peer->reset_role);
    
#if 0   
    bgp4_fsm_invalid(p_peer);
#endif
    bgp4_fsm(p_peer,BGP4_EVENT_FATAL_ERROR);
    /*start reset timer*/
    if(role == BGP4_GR_ROLE_RECIEVE)
    {
        bgp4_start_peer_gr_timer(p_peer) ;
    }
    else if(role == BGP4_GR_ROLE_RESTART &&
            gBgp4.gr_restart_flag == FALSE)
    {
        bgp4_start_deferral_timer();
        gBgp4.gr_restart_flag = TRUE;
    }

    return TRUE;
    
}


/*process peer asyn tcp close in graceful restart*/
int bgp4_peer_down_to_gr(tBGP4_PEER *p_peer)
{
    u_char addrstr[64] = {0};

    bgp4_log(BGP_DEBUG_EVT,1,"perceive peer %s down",
                            bgp4_printf_peer(p_peer,addrstr));
    
    
    if (p_peer == NULL 
        || gBgp4.gr_enable == 0 
        ||  p_peer->remote.reset_enable == 0
        || p_peer->state != BGP4_NS_ESTABLISH)
    {
        bgp4_log(BGP_DEBUG_CMN,1,"peer not support graceful  restart");
            
        /*need clear all stale route here ?*/
        return FALSE;
    }

    switch(p_peer->reset_role)
    {
        case BGP4_GR_ROLE_NORMAL:
        {
            bgp4_peer_execute_gr(p_peer,BGP4_GR_ROLE_RECIEVE);
            
            /*current state is normal,set role to recieve*/
            bgp4_log(BGP_DEBUG_EVT,1,"chang peer gr role,from normal to recieve");           
                    
            break;
        }
        case BGP4_GR_ROLE_RESTART:
        {
            /*reset peer last role state*/
            p_peer->reset_role = BGP4_GR_ROLE_NORMAL ;
            
            bgp4_peer_execute_gr(p_peer,BGP4_GR_ROLE_RESTART);
            
            bgp4_log(BGP_DEBUG_EVT,1,"keep up peer bgp4  gr role restart");
            
            break;
        }
        case BGP4_GR_ROLE_RECIEVE:
        {
            /*reset peer last role state*/
            p_peer->reset_role = BGP4_GR_ROLE_NORMAL ;
                
            bgp4_peer_execute_gr(p_peer,BGP4_GR_ROLE_RECIEVE);
            
            bgp4_log(BGP_DEBUG_EVT,1,"keep up peer  bgp4 gr role recieve");
            break;
        }
        default:
        {
            bgp4_log(BGP_DEBUG_EVT,1,"peer restart role error,do not accomplish graceful restart!");
            
            /*need clear all stale route here ?*/
            return FALSE;
        }
    }

    return TRUE ;
}

/*bgp4_peer_exit_gr
exit gr mode by administration or notification
in point to peer
out/return none
*/
void bgp4_peer_exit_gr(tBGP4_PEER *p_peer)
{
    /*restore state*/
    p_peer->reset_role = BGP4_GR_ROLE_NORMAL;
    
    /*reset bit will reset*/
    p_peer->local.reset_bit = 0;
    
    /*end flag clear*/
    p_peer->rib_end_af = 0 ;
    
    /*remote infor clear*/
    p_peer->remote.reset_bit = 0 ;
    p_peer->remote.reset_enable = 0 ;
    p_peer->remote.reset_time = 0 ;
    p_peer->remote.reset_af = 0 ;
    
    /*stop timer*/
    bgp4_stop_peer_gr_timer(p_peer) ;
    
    return ;
}

/*bgp4_gr_reset_timeout
process reset timeout
in point to peer
out
return none
*/
void bgp4_peer_gr_timeout(tBGP4_PEER *p_peer)
{
    u_char prev_role = p_peer->reset_role ;
    u_char addrstr[64] = {0};
    u_int af = 0;
    
    if (p_peer == NULL)
    {
        return ;
    }

    bgp4_log(BGP_DEBUG_EVT,1,"reset time out for peer %s,role %d",
                        bgp4_printf_peer(p_peer,addrstr), prev_role);
    
    /*clear stale route*/  
    for(af = 0;af < BGP4_PF_MAX;af++)
    {
        if(bgp4_af_support(p_peer,af))
        {
            bgp4_delete_stale_route(p_peer,af,0);
        }
            
    }
    
    /*Set Stale to normal*/    
    p_peer->reset_role = BGP4_GR_ROLE_NORMAL;
    
    p_peer->local.reset_bit = 0 ;

    for(af = 0; af < BGP4_PF_MAX; af++)
    {
        bgp4_schedule_init_update(af, p_peer);
    }

#if 0
    if (p_peer->state==BGP4_NS_ESTABLISH)
    {
        /*update rib to ip table and peers*/
        if (bgp4_gr_if_can_update() == TRUE)
        { 
            bgp4_gr_send_all_init_update(); 
        }
    }
#endif

}

/*bgp4_extract_gr_capability
process gr capability in open option
in point to peer
point to msg,start from Rbit
length of capability ,start from Rbit
out :fill peer's related remote gr attribute
return :bgp4-success/bgp4_failure      
*/
int bgp4_extract_gr_capability(tBGP4_PEER *p_peer,u_char *p_msg,u_short msg_len)
{
    u_short restart_time = 0;
    u_char restart_flag = 0;
    u_short read_len = 0 ;
    u_short afi = 0;
    u_char safi = 0;
    u_char fwd_flag = 0;  
    u_int af_flag = 0;  
    
    bgp4_log(BGP_DEBUG_EVT,1,"process graceful restart capability");
    
    /*validate length and input */
    if ((p_peer == NULL) || (p_msg == NULL) || (msg_len < 2) ||
        (((msg_len-2)%4) != 0))          
    {
        bgp4_log(BGP_DEBUG_EVT,1,"gr capability param error");
        
        return FALSE ;
    }
    
    /*clear last record remote graceful-restart information*/
    p_peer->remote.reset_bit = 0 ;
    p_peer->remote.reset_enable = 0 ;
    p_peer->remote.reset_time = 0 ;
    p_peer->remote.reset_af = 0 ;         
    
    /*if local node not support GR,discard silently*/         
    if (gBgp4.gr_enable != TRUE)         
    {
        bgp4_log(BGP_DEBUG_EVT,1,"graceful restart not configured,ignore it");
        return TRUE;
    }
    
    /*record Rtime*/         
    bgp4_get_2bytes(restart_time, p_msg);
    p_msg += 2; 
    read_len = 2;
    restart_flag = (restart_time&BGP4_GR_RESET_FLAG) == 0 ? 0 : 1;
    restart_time &= 0x0fff;
    
    bgp4_log(BGP_DEBUG_EVT,1,"obtain Reset Bit %d",restart_flag);  
    bgp4_log(BGP_DEBUG_EVT,1,"obtain Reset Time %d seconds",restart_time);
    
    /*rtime must less than hold time*/   
    if (restart_time > p_peer->hold_interval)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"invalid reset time setting");
        restart_time = 30;/*for GR comformance test*/ 
    }
    p_peer->remote.reset_enable = TRUE;
    p_peer->remote.reset_bit = restart_flag ;
    p_peer->remote.reset_time = restart_time;
    af_set(p_peer->remote.capability,BGP4_CAPABILITY_GR);
    
    while (read_len < msg_len)
    {
        bgp4_get_2bytes(afi,p_msg);
        p_msg += 2;
        
        safi = *p_msg;
        p_msg ++ ;
        
        fwd_flag = *p_msg;
        p_msg ++ ;
        
        read_len += 4;
        
        bgp4_log(BGP_DEBUG_EVT,1,"proc afi/safi %d/%d,flag %d",
            afi,safi,(fwd_flag&BGP4_GR_FORWARDING_FLAG) == 0 ? 0 : 1);
        
        if (fwd_flag & BGP4_GR_FORWARDING_FLAG)
        {
            af_flag = bgp4_afi_to_index(afi, safi);
            af_set(p_peer->remote.reset_af,af_flag);
        }
    }
        
    return TRUE;
}

/*form graceful restart capability in Open msg's Option filed
in point to peer
point to msg
out :fill capability into msg if necessary
return 0 if no capability,or else bytes filled ,including code,length and value
*/
u_short bgp4_build_gr_capability(tBGP4_PEER *p_peer,u_char *p_buf)
{
    u_short bytes = 0 ;
    u_short restart_time = 0 ;
    u_char *p_opt_len  = (p_buf + 1);
    u_short afi = 0;
    u_char safi = 0;
    u_char forward = 0 ;
    u_char i = 0;
    
    /*validate input point*/
    if ((p_peer == NULL) || (p_buf == NULL))
    {
        return 0;        
    }
    
    
    /*fill cap code,skip length*/
    bgp4_put_char(p_buf,BGP4_CAP_GRACEFUL_RESTART);
    bgp4_put_char(p_buf,0);  
    
    /*fill Reset Flag and Reset Time.reset time must less than
    hold time in open
    */ 
    restart_time = gBgp4.gr_restart_time;
    
    if ((restart_time == 0) || 
        (restart_time >= p_peer->hold_interval))
    {
        restart_time = p_peer->hold_interval-1;
    }
    
    /*Reset Flag*/   
    if (p_peer->local.reset_bit)
    {
        restart_time |= BGP4_GR_RESET_FLAG;
    }

    /*p_peer->local.reset_bit=1;*/
        
    /*Fill 2bytes*/  
    bgp4_put_2bytes(p_buf, restart_time);
    
    bytes += 2;
    
    /*fill flag if need*/  
    for (i = 0 ; i < BGP4_PF_MAX ; i++)
    {
        if (af_isset(p_peer->send_capability, i))
        {
            afi = bgp4_index_to_afi(i);
            safi = bgp4_index_to_safi(i);
            bgp4_put_2bytes(p_buf,afi);                
            bgp4_put_char(p_buf,safi); 
            
            if (p_peer->reset_role != BGP4_GR_ROLE_NORMAL)/*reset role must be graceful restart,else clear f bit*/
            {
                forward = BGP4_GR_FORWARDING_FLAG ;
            }
            
            bgp4_put_char(p_buf,forward); 
            bytes += 4;            
        }
    }  
    
    *p_opt_len = bytes;       
    return (bytes + 2) ;/*including value and code,length*/
}


/*bgp4_gr_if_can_update
check if can send first update;
for restarting node,it will wait end-of-rib from all gr peer for all afi/safi
in none
out none
return bgp-success if can update;or bgp-failure if can not update
*/
u_int bgp4_gr_if_can_update()
{
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    
    gBgp4.gr_last_update = gBgp4.gr_current_update ;
    
    if(gBgp4.gr_restart_flag == TRUE)
    {
        LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
        {
            /*scan all peer with valid tcp connection,if one's role is restarting
            do not send update*/
            LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER) 
            {
                if (p_peer->reset_role == BGP4_GR_ROLE_RESTART)
                {
                    gBgp4.gr_current_update = FALSE ;            
                    return FALSE ;
                }
            }
       
        }
    }
        
    gBgp4.gr_current_update = TRUE ;
    
    return TRUE ;
    
}

/*bgp4_process_gr_open
first process graceful restart after process open message
in point to peer
gr capability flag
out:may change peer's gr state
return :none
*/
void bgp4_process_gr_open(tBGP4_PEER *p_peer,u_char gr_exist)
{
    u_int af = 0;
    
    if (p_peer == NULL)
    {
        return ;
    }

    /*if no gr capability exist,clear remote infor*/         
    if (gr_exist == 0)         
    {
        p_peer->remote.reset_bit = 0 ;
        p_peer->remote.reset_enable = 0 ;
        p_peer->remote.reset_time = 0 ;
        p_peer->remote.reset_af = 0 ;         
    }

    /*do not support grace*/
    if((gBgp4.gr_enable == 0) ||
        (p_peer->remote.reset_enable == 0) ||
        (p_peer->reset_role == BGP4_GR_ROLE_NORMAL))
    {
        bgp4_log(BGP_DEBUG_CMN,1,"remote or local do not support GR!");
        return;
    }

    if (((p_peer->reset_role == BGP4_GR_ROLE_RESTART) && (p_peer->remote.reset_bit == 1)) ||
        ((p_peer->reset_role == BGP4_GR_ROLE_RECIEVE) && (p_peer->remote.reset_bit == 0)))
    {
        bgp4_log(BGP_DEBUG_EVT,1,"some error when GR restarting,switch to normal process,peer reset role %d,peer remote R bit %d",
            p_peer->reset_role,p_peer->remote.reset_bit);
        /*Set Role and reset reason to normal*/
        p_peer->reset_role = BGP4_GR_ROLE_NORMAL;
        p_peer->local.reset_bit = 0;         

        for (af = 0 ; af < BGP4_PF_MAX ; af++)
        {
            bgp4_end_peer_route(af,p_peer,0,1); 
        }
        /*Stop Reset Timer*/
        bgp4_stop_peer_gr_timer(p_peer);
        
        return ;         
    }
    
    /*process afi/safi one by one,if local reservd and remote not reserved
    delete stale route of this afi/safi
    */
    for (af = 0 ; af < BGP4_PF_MAX ; af++)
    {
        if ((af_isset(p_peer->send_capability,af))
            && !af_isset(p_peer->remote.reset_af,af))
        {
            bgp4_log(BGP_DEBUG_EVT,1,"remote af unsupported,delete such af  %d stale routes",af);
            bgp4_end_peer_route(af,p_peer,0,1); 
        }
    }
    
    /*check peer grace state,only start gr timer when remote is commencing a restart*/
    if(p_peer->remote.reset_bit == TRUE)
    {
        bgp4_stop_peer_gr_timer(p_peer);
        bgp4_start_peer_gr_timer(p_peer) ;

    }
    
    return ;
}

/*bgp4_gr_send_all_init_update
send first update if necessary
in none
out send update to peer
return none
*/
void bgp4_gr_send_all_init_update()
{
    tBGP4_PEER *p_peer = NULL;
    u_int af = 0;
    tBGP4_VPN_INSTANCE* p_instance = NULL;


    bgp4_gr_if_can_update();

    if ((gBgp4.gr_current_update != gBgp4.gr_last_update) &&
        (gBgp4.gr_current_update == TRUE))
    {
        bgp4_log(BGP_DEBUG_EVT,1,"gr over,start route selection now!"); 

        /*stop selection deferral timer*/
        bgp4_stop_deferral_timer(); 

        LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
        {
            LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER) 
            {
                if (p_peer->state == BGP4_NS_ESTABLISH)
                {
                    for(af = 0; af < BGP4_PF_MAX; af++)
                    {
                        bgp4_schedule_init_update(af, p_peer);
                    }
                }
                   
            }
        }
                        
        gBgp4.gr_restart_flag = FALSE;

    }

}

/*update deferal route to rib*/
void bgp4_gr_update_deferral_route(tBGP4_ROUTE*p_route)
{
    u_char rt_str[64];

    /*check if need set deferral flag*/
    if(bgp4_gr_if_can_update() == TRUE)
    {
        return ;
    }

    if(p_route == NULL)
    {
        return ;
    }

    p_route->deferral = TRUE;

    bgp4_log(BGP_DEBUG_UPDATE,1,"GR in progress,check route %s,set route deferral",
                bgp4_printf_route(p_route,rt_str));

    return; 
    
}

void bgp4_gr_deferral_seletion_timeout(void)
{
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int af = 0;

    /*delete all stale routes from peers which are still in GR process*/
    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
            LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER) 
            {
            if (p_peer->reset_role != BGP4_GR_ROLE_NORMAL)
            {
                /*clear stale route*/  
                for(af = 0;af < BGP4_PF_MAX;af++)
                {
                    bgp4_delete_stale_route(p_peer,af,0);
                }
            }
        }

    }


    if(gBgp4.gr_restart_flag == TRUE)
    {
        gBgp4.gr_restart_flag = FALSE;   
        bgp4_gr_send_all_init_update();
    }
    
}






#endif
