#include "bgp4_api.h"
#include "bgp4peer.h"
#include "bgp4com.h"
#include "bgp4kernal.h"
#include "bgp4util.h"
#ifdef NEW_BGP_WANTED


u_int bgp4_get_number_of_route(u_short type, tBGP4_PEER * p_peer);

#define bgp4_busy_warning() printf("\r\nBGP is busy now,please try again later")

STATUS 
bgpGlobalGetApi(
          void *index,
          u_int cmd,
          void *var)
{
    STATUS rc = VOS_OK;
    u_int *p_lval = (u_int *) var;
    octetstring *p_str = (octetstring *)var;

    /*some set api need not sem lock*/
    if (cmd == BGP_LOCAL_AS)
    {
        *p_lval = gbgp4.as;
        return rc;
    }
    #if 0
    if (bgp_sem_take_timeout() == VOS_ERR)
    {
        bgp4_busy_warning();
        return VOS_ERR;
    }
    #endif
    //bgp_sem_take_timeout();
    bgp_sem_take();
    switch (cmd){
        case BGP_VERSION_NUMBER:
             /*octect string,len 1
               bit 0~7,set bit (v-1),verseion 4 == 0 0 0 1 0 0 0 0 */
             *p_lval = 0x10;
             break;
             
        case BGP_IDENTIFIER_NUMBER:
             *p_lval = gbgp4.router_id;
             break;
             
        case BGP_CLUSTER_ID:
             *p_lval = gbgp4.cluster_id;
             break;

        case BGP_COMMUNITY_ACTION:
             *p_lval = gbgp4.community_action;
             break;

        case BGP_COMMUNITY:
             *p_lval = gbgp4.community;
             break;

        case BGP_CONFEDERATION_ID:
             *p_lval = gbgp4.confedration_id;
             break;

        case BGP_TRAP_SUPPORT:
             *p_lval = gbgp4.trap_enable;
             break;

        case BGP_DEFAULT_LOCALPREF:
             *p_lval = gbgp4.local_pref;
             break;

        case BGP_DEFAULT_MED:
             *p_lval = gbgp4.med;
             break;

        case BGP_SYNC_STATUS:
             *p_lval = gbgp4.igp_sync_enable;
             break;

        case BGP_CEASE_STATUS:
             break;

        case BGP_LOCAL4BYTES_AS:
             *p_lval = 0;
             break;

        case BGP_SERVER_ENABLE:
             *p_lval = gbgp4.tcp_server_enable;
             break;

        case BGP_CLIENT_ENABLE:
             *p_lval = gbgp4.tcp_client_enable;
             break;

        case BGP_UPDATE_SIZE:
             *p_lval = gbgp4.max_update_len;
             break;

        case BGP_TASK_PRIO: 
             break;

        case BGP_MAX_STACKSIZE: 
             break;

        case BGP_MAX_PEER:
             *p_lval = gbgp4.max_peer;
             break;

        case BGP_MAX_ROUTE:
             *p_lval = gbgp4.max_route;
             break;

        case BGP_MAX_PATH:
             *p_lval = gbgp4.max_path;
             break;
                
        case BGP_PROTO_RTNUM:
        {
             u_int proto[8] = {M2_ipRouteProto_local,
                               M2_ipRouteProto_netmgmt,
                               M2_ipRouteProto_rip,
                               M2_ipRouteProto_is_is,
                               M2_ipRouteProto_ospf,
                               M2_ipRouteProto_other,
                               0,0};
             u_int number = 0;
             u_char *p_buf = p_str->pucBuf;
             u_int i;

             for (i = 0 ; i < 8 ; i++)
             {
                 if (proto[i])
                 {
                     number = bgp4_get_number_of_route(proto[i], NULL);
                     *p_buf = proto[i];
                     p_buf++;
                     memcpy(p_buf, &number, 4);
                     p_buf += 4;
                 }
             }
             p_str->len = (u_long)p_buf - (u_long)p_str->pucBuf;
             break;
        }
        case BGP_BIGRT_UPDATE:
             *p_lval = 1;
             break;

        case BGP_GR_ENABLE:
             *p_lval = gbgp4.restart_enable;
             break;

        case BGP_GR_TIME:
             *p_lval = gbgp4.restart_period;
             break;

        case BGP_DEFERRAL_TIME:
             *p_lval = gbgp4.restart_wait_period;
             break;
        case BGP_DAMP_ENABLE:
             *p_lval = gbgp4.damp_enable;
             break;
             
        case BGP_DAMP_PENALTYHALFTIME:
             *p_lval = gbgp4.penalty_half_time;
             break;
    
        case BGP_DAMP_PENALTYREUSE:
             *p_lval = gbgp4.penalty_reuse;
             break;
    
        case BGP_DAMP_PENALTYSUPRESS:
             *p_lval = gbgp4.penalty_supress;
             break;
    
        case BGP_DAMP_PENALTYMAX:
             *p_lval = gbgp4.penalty_max;
             break;

        /* support fast backup route*/     
        case BGP_BACKUP_PATH_ENABLE:
             *p_lval = gbgp4.backup_path_enable;
             break;
             
        case  BGP_IPV4VPNV4:
            *p_lval = gbgp4.ipv4_vpnv4;
            break;           
        case BGP_PREFRENCE:
            p_str->pucBuf[0] = gbgp4.ucEbgpPre;
            p_str->pucBuf[1] = gbgp4.ucIbgpPre;
            p_str->pucBuf[2] = gbgp4.ucLocalPre;
            p_str->len = 3;
            break; 
        case BGP_CURRENT_PEER_COUNT:
            *p_lval = gbgp4.uiPeerCount;
            break;
        default:
             rc = VOS_ERR;
             break;
    }
    bgp_sem_give();
    return rc;
}

STATUS 
_bgpGlobalSetApi(
        void *index,
        u_int cmd,
        void *var,
        u_int flag)
{
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_RIB *p_rib = NULL;
    tBGP4_PEER *p_peer = NULL;
    STATUS rc = VOS_OK;
    u_int lval = *(u_int*)var;
    u_int af = 0;
    octetstring *p_str = (octetstring *)var;
    
    /*sync config to others*/
    if (cmd == BGP_GBL_SYNPKT)
    {
        flag |= USP_SYNC_OCTETDATA;
    }
    /*TODO:master slave sync*/
    #if 0
    if (uspHwScalarSync(index, HW_SYS_BGP4CMDSTART+cmd, (void *)var,  flag) != VOS_OK)
    {
        return VOS_ERR;
    }
    #endif
    /*some set api need not sem lock*/
    switch (cmd){
        case BGP_DEBUG_ON:
             gbgp4.debug |= lval;
             return VOS_OK;
             break;
             
        case BGP_DEBUG_OFF:
             gbgp4.debug &= ~(lval);
             return VOS_OK;
             break;

        default:
             break;
    }

    bgp_sem_take();

    switch (cmd){
        case BGP_LOCAL_AS:
             if (lval > 0)
             {
                 rc = bgp4_enable(lval);
             }
             else if (gbgp4.enable && (lval == 0))
             {
                 bgp4_disable();
             }
             break;
        
        case BGP_IDENTIFIER_NUMBER:
             bgp4_set_router_id(lval);
             break;

        case BGP_CLUSTER_ID:
             gbgp4.cluster_id = lval;
             gbgp4.reflector_enable = gbgp4.cluster_id ? TRUE : FALSE;
             break;

        case BGP_COMMUNITY_ACTION:
             gbgp4.community_action = lval;
             break;

        case BGP_COMMUNITY:
             gbgp4.community = lval;
             break;

        case BGP_CONFEDERATION_ID:
             gbgp4.confedration_id = lval;
             break;

        case BGP_TRAP_SUPPORT:
             gbgp4.trap_enable = lval;
             break;

        case BGP_DEFAULT_LOCALPREF:
             if(gbgp4.local_pref != lval)
             {
                gbgp4.local_pref = lval;
                if(!bgp4_timer_is_active(&gbgp4.init_update_timer))
                {
                    bgp4_timer_start(&gbgp4.init_update_timer, 1);
                }
             }
             break;

        case BGP_DEFAULT_MED:
             if(gbgp4.med != lval)
             {
                gbgp4.med = lval;
                if(!bgp4_timer_is_active(&gbgp4.init_update_timer))
                {
                    bgp4_timer_start(&gbgp4.init_update_timer, 1);
                }
             }             
             break;

        case BGP_SYNC_STATUS:
             gbgp4.igp_sync_enable = lval;
             break;

        case BGP_CEASE_STATUS:
             break;

        case BGP_LOCAL4BYTES_AS:
             break;

        case BGP_SERVER_ENABLE:
             gbgp4.tcp_server_enable = lval;
             bgp4_instance_for_each(p_instance)
             {
                if(TRUE == gbgp4.tcp_server_enable)
                {
                    bgp4_server_sock_open(p_instance->vrf);
                }
                else
                {
                    bgp4_server_sock_close(p_instance->vrf);
                }
             }
             break;

        case BGP_CLIENT_ENABLE:
             gbgp4.tcp_client_enable = lval;
             break;

        case BGP_TASK_PRIO: 
             break;

        case BGP_MAX_STACKSIZE: 
             break;

        case BGP_UPDATE_SIZE:
             gbgp4.max_update_len = lval;
             break;

        case BGP_MAX_PEER:
             gbgp4.max_peer = lval;
             break;

        case BGP_MAX_ROUTE:
             gbgp4.max_route = lval;
             break;

        case BGP_MAX_PATH:
             gbgp4.max_path = lval;
             break;

        case BGP_GBL_SYNPKT:
             bgp4_sync_recv(var);
             break;

        case BGP_BIGRT_UPDATE:
             break;

        case BGP_GR_ENABLE:
             gbgp4.restart_enable = lval;
             break;

        case BGP_GR_TIME:
             gbgp4.restart_period = lval;
             break;

        case BGP_DEFERRAL_TIME:
             gbgp4.restart_wait_period = lval;
             break;

        case BGP_RESET_ALL:
             if(!gbgp4.BgpRouteIdFlag)
             {
                bgp4_set_router_id(0);
             }
             bgp4_peer_reset_all();
             break;
                
        case BGP_DAMP_ENABLE:
             if (lval == TRUE)
             {
                 bgp4_damp_enable();
             }
             else
             {
                 bgp4_damp_disable();
             }
             break;
        case BGP_DAMP_PENALTYHALFTIME:
             gbgp4.penalty_half_time = lval;
             break;
       
        case BGP_DAMP_PENALTYREUSE:
             gbgp4.penalty_reuse = lval;
             break;
            
        case BGP_DAMP_PENALTYSUPRESS:
             gbgp4.penalty_supress = lval;
             break;
       
        case BGP_DAMP_PENALTYMAX:
             gbgp4.penalty_max = lval;
             break;
             
        /* support fast backup route*/     
        case BGP_BACKUP_PATH_ENABLE:
             if (gbgp4.backup_path_enable == lval)
             {
                 break;
             }
             /*stop all peer*/
             bgp4_instance_for_each(p_instance)
             {
                 bgp4_peer_for_each(p_instance, p_peer)
                 {
                     p_peer->cease_reason = BGP4_ADMINISTRATIVE_RESET;
                     bgp4_fsm(p_peer, BGP4_EVENT_STOP);
                 }
                 /*wait for kernal update*/
                 for (af = 0 ; af < BGP4_PF_MAX ; af++)
                 {            
                     p_rib = p_instance->rib[af];
                     if (p_rib == NULL)
                     {
                         continue;
                     }
                     while (p_rib->system_check_need == TRUE)
                     {
                        bgp4_rib_system_update_check(p_rib);
                        vos_pthread_delay(1);
                        bgp4_rtsock_recv();
                     }
                     /*send msg for delete*/
                     while (p_rib->p_hwmsg)
                     {
                         if (bgp4_sys_msg_send(p_rib->p_hwmsg) != VOS_OK)
                         {
                             vos_pthread_delay(1);
                             bgp4_rtsock_recv();
                         }
                         else
                         {
                             bgp4_free(p_rib->p_hwmsg, MEM_BGP_BUF);
                             p_rib->p_hwmsg = NULL;
                         }
                     }
                 }

             }

             /*set new capability*/
             gbgp4.backup_path_enable = lval;
             
             /*restart all peers*/
             bgp4_instance_for_each(p_instance)
             {
                 bgp4_peer_for_each(p_instance, p_peer)
                 {
                     bgp4_fsm(p_peer, BGP4_EVENT_START);
                 }
             }
             break;
        case BGP_IPV4VPNV4:
            gbgp4.ipv4_vpnv4 = lval;
            break;
        case BGP_PREFRENCE:
            if(p_str->pucBuf[0] != gbgp4.ucEbgpPre
            || p_str->pucBuf[1] != gbgp4.ucIbgpPre
            || p_str->pucBuf[2] != gbgp4.ucLocalPre)
            {
                bgp_modify_system_route_distance(p_str->pucBuf[0], p_str->pucBuf[1], p_str->pucBuf[2]);
            }
            gbgp4.ucEbgpPre = p_str->pucBuf[0];
            gbgp4.ucIbgpPre = p_str->pucBuf[1];
            gbgp4.ucLocalPre = p_str->pucBuf[2];
            break;
        default:
             rc = VOS_ERR;
             break;
    }
    bgp_sem_give();
    return rc;
}

STATUS 
bgpGlobalSetApi(
      void *index,
      u_int cmd,
      void *var)
{
    return _bgpGlobalSetApi(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}

STATUS 
bgpGlobalSyncApi(
      void *index,
      u_int cmd,
      void *var)
{
    return _bgpGlobalSetApi(index, cmd, var, USP_SYNC_LOCAL);
}

int 
bgpPeerGetFirst(tBgpPeerIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    bgp4_instance_for_each(p_instance)
    {
        p_peer = bgp4_avl_first(&p_instance->peer_table);
        if (p_peer)
        {
            p_index->addr_type = (p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
            memcpy(p_index->addr, p_peer->ip.ip, 16);
            p_index->vrf_id = p_instance->vrf;
            rc = VOS_OK;
            break;
        }
    }
    bgp_sem_give();
    return rc;
}

int 
bgpPeerGetNext(
      tBgpPeerIndex *p_index ,
      tBgpPeerIndex *p_next_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_PEER search_peer;
    tBGP4_VPN_INSTANCE search_instance;
    STATUS rc = VOS_ERR;

    memset(&search_peer.ip, 0, sizeof(search_peer.ip));
    memcpy(search_peer.ip.ip, p_index->addr, 16);
    search_peer.ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 : 128;
    search_peer.ip.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;

    bgp_sem_take();

    /*step1:get next peer in same instance*/
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance)
    {
        p_peer = bgp4_avl_greator(&p_instance->peer_table, &search_peer);
        if (p_peer)
        {
            rc = VOS_OK;
            goto END;
        }
    }
    /*step2:get first peer in other instance*/
    search_instance.vrf = p_index->vrf_id;
    search_instance.type = BGP4_INSTANCE_IP;
    bgp4_avl_for_each_greater(&gbgp4.instance_table, p_instance, &search_instance)
    {
        p_peer = bgp4_avl_first(&p_instance->peer_table);
        if (p_peer)
        {
            rc = VOS_OK;
            goto END;
        }
    }
END:    
    if (rc == VOS_OK)
    {
        p_next_index->addr_type = (p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
        memcpy(p_next_index->addr, p_peer->ip.ip, 16);
        p_next_index->vrf_id = p_instance->vrf;
    }
    bgp_sem_give();
    return rc;
}

STATUS 
bgpPeerGetApi(
       tBgpPeerIndex *p_index,
       u_int cmd,
       void *var)
{
    STATUS rc = VOS_OK;
    tBGP4_ADDR peer_ip;
    u_int *p_lval = (u_int *)var;
    octetstring *p_str = (octetstring *)var;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    memset(&peer_ip, 0, sizeof(peer_ip));
    peer_ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 : 128;
    peer_ip.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;
    memcpy(peer_ip.ip, p_index->addr, 16);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    p_peer = bgp4_peer_lookup(p_instance, &peer_ip);
    if (p_peer == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    switch(cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_PEER_REMOTEIDENTIFIER:
             *p_lval = p_peer->router_id;
             break;
             
        case BGP_PEER_TYPE:
             *p_lval = bgp4_peer_type(p_peer);
             break;

        /*
        * EbgpPeerState_idle 1
        * EbgpPeerState_connect 2
        * EbgpPeerState_active 3
        * EbgpPeerState_opensent 4
        * EbgpPeerState_openconfirm 5
        * EbgpPeerState_established 6
        */
        case BGP_PEER_STATE:
             *p_lval = p_peer->state;
             break;

        case BGP_PEER_ADMINSTATUS:
             *p_lval = p_peer->admin_state;
             break;

        case BGP_PEER_NEGOTIATEDVERSION:
             *p_lval = p_peer->version;
             break;

        case BGP_PEER_LOCALADDRTYPE:
             *p_lval = p_peer->local_ip.afi;
             break;

        case BGP_PEER_LOCALADDR:
             p_str->len = p_peer->local_ip.prefixlen;
             memcpy(p_str->pucBuf, p_peer->local_ip.ip, p_str->len);
             break;

        case BGP_PEER_LOCALPORT:
             *p_lval = p_peer->local_tcp_port;
             break;

        case BGP_PEER_REMOTEPORT:
             *p_lval = p_peer->tcp_port;
             break;
                
        case BGP_PEER_REMOTEAS:
             *p_lval = p_peer->as;
             break;

        case BGP_PEER_INUPDATES:
             *p_lval = p_peer->stat.rx_update;
             break;

        case BGP_PEER_OUTUPDATES:
             *p_lval = p_peer->stat.tx_update;
             break;
             
        case BGP_PEER_INTOTALMSGS:
             *p_lval = p_peer->stat.rx_msg;
             break;

        case BGP_PEER_OUTTOTALMSGS:
             *p_lval = p_peer->stat.tx_msg;
             break;
             
        case BGP_PEER_LASTERRORCODERECV:
        {
             u_short error_code = p_peer->stat.last_errcode;
             error_code =  (error_code << 8) + p_peer->stat.last_subcode;
             p_str->len = 2;
             memcpy(p_str->pucBuf, &error_code, p_str->len);
             break;
        }
        case BGP_PEER_FSMESTABLISHEDTRANSITIONS:
             *p_lval = p_peer->stat.established_transitions;
             break;

        case BGP_PEER_FSMESTABLISHEDTIME:
             if (p_peer->state != BGP4_NS_ESTABLISH)
             {
                 *p_lval = 0;
             }
             else
             {
                 *p_lval = time(NULL) - p_peer->stat.uptime;
             }
             break;

        case BGP_PEER_CONNECTRETRYINTERVAL:
             *p_lval = p_peer->retry_interval;
             break;

        case BGP_PEER_HOLDTIME:
             *p_lval = p_peer->neg_hold_interval;
             break;

        case BGP_PEER_KEEPALIVE:
             *p_lval = p_peer->neg_keep_interval;
             break;

        case BGP_PEER_HOLDTIMECONFIGURED:
             *p_lval = p_peer->hold_interval;
             break;

        case BGP_PEER_KEEPALIVECONFIGURED:
             *p_lval = p_peer->keep_interval;
             break;

        case BGP_PEER_MINASORIGINTERVAL:
             *p_lval = p_peer->origin_interval;
             break;

        case BGP_PEER_MINROUTEADVERINTERVAL:
             *p_lval = p_peer->adv_interval;
             break;
 
        case BGP_PEER_UPDATESELAPSEDTIME:
             if (p_peer->state != BGP4_NS_ESTABLISH)
             {
                 *p_lval = 0;
             }
             else
             {
                 *p_lval = time(NULL) - p_peer->stat.rx_updatetime;
             }
             break;

        case BGP_PEER_PASSWORDSUPPORT:
             *p_lval = p_peer->md5_support;
             break;

        case BGP_PEER_PASSWORD:
             if (p_peer->md5key_len > 0)
             {
                 p_str->len = p_peer->md5key_len;
                 memcpy(p_str->pucBuf, p_peer->md5key, p_peer->md5key_len);
             }
             break;

        case BGP_PEER_RTREFLECTORSTATE:
             *p_lval = p_peer->is_reflector_client;
             break;

        case BGP_PEER_ATTACHEDVRF:
             *p_lval = 0;
             break;

        case BGP_PEER_SENDCONNUNITY:
             *p_lval = p_peer->send_community;
             break;

        case BGP_PEER_RAWSTATUS:
             *p_lval = p_peer->row_status;
             break;

        case BGP_PEER_UPDATESOURCE:
             p_str->len = p_peer->update_source.prefixlen/8;
             memcpy(p_str->pucBuf, p_peer->update_source.ip, p_str->len);
             break;
             
        case BGP_PEER_IFTYPE:
             p_str->len = strlen(p_peer->if_typename);
             memcpy(p_str->pucBuf, p_peer->if_typename, p_str->len);
             break;
             
        case BGP_PEER_LOCALAFIFLAG:
             *p_lval = p_peer->local_mpbgp_capability;
             break;
 
        case BGP_PEER_REMOTEAFIFLAG:
             *p_lval = p_peer->mpbgp_capability;
             break;
 
        case BGP_PEER_LOCALREFRESH:
             *p_lval = p_peer->local_refresh_enable;
             break;

        case BGP_PEER_REMOTEREFRESH:
             *p_lval = p_peer->refresh_enable;
             break;

        case BGP_PEER_EBGPMTHOP:
             *p_lval = p_peer->mhop_ebgp_enable;
             break;

        case BGP_PEER_VALIDTTLHOP:
             *p_lval = p_peer->ttl_hops;
             break;

        case BGP_PEER_GRREMOTESUPPORT:
             *p_lval = p_peer->restart_enable;
             break;
                
        case BGP_PEER_GRREMOTERESETTIME:
             *p_lval = p_peer->restart_time;
             break;
                
        case BGP_PEER_GRREMOTERESETAF:
             *p_lval = p_peer->restart_mpbgp_capability;
             break;
                
        case BGP_PEER_GRREMOTERESETBIT:
             *p_lval = p_peer->in_restart;
             break;
                
        case BGP_PEER_GRLOCALROLE:
             *p_lval = p_peer->restart_role;
             break;
       
        case BGP_PEER_CEASEREASON:
             *p_lval = p_peer->cease_reason;
             break;

        case BGP_PEER_CEASERESTART:
             break;

        case BGP_PEER_CEASEDOWN:
             break;

        case BGP_PEER_CEASEMAXPREFIX:
             break;

        case BGP_PEER_CEASERESLACK:
             break;

        case BGP_PEER_CEASEREJECTCONN:
             break;

        case BGP_PEER_CEASEEBGP:
             break;

        case BGP_PEER_UNCONFIG:
             break;

        case BGP_PEER_SENDLABEL:
             *p_lval = p_peer->send_label;
             break;

        case BGP_PEER_RTNUM:
             *p_lval = bgp4_get_number_of_route(M2_ipRouteProto_bgp, p_peer);
             break;
                
        case BGP_PEER_NEXTHOPSELF:
             *p_lval = p_peer->nexthop_self;
             break;
                
        case BGP_PEER_BFD_ENABLE:
             *p_lval = p_peer->bfd_enable;
             break;
                
        case BGP_PEER_PUBASONLY:
             *p_lval = p_peer->public_as_only;
             break;
        
        case BGP_PEER_SUBASENABLE:
             *p_lval = p_peer->as_substitute_enable;
             break;
        
        case BGP_PEER_ALLOWLOOPTIMES:
             *p_lval = p_peer->allow_as_loop_times;
             break;
                
        case BGP_PEER_FAKEAS:
             *p_lval = p_peer->fake_as;
             break;
        case BGP_PEER_UPE:/*get upe role*/
             *p_lval = p_peer->upe_enable;
             break;
        case BGP_PEER_EVENT:
             *p_lval = p_peer->stat.event;
             break;
        case BGP_PEER_STAT:
             p_str->len = sizeof(struct bgp4_peer_stat);
             memcpy(p_str->pucBuf, &p_peer->stat, p_str->len);
             break;
        case BGP_PEER_IFUINT:
             *p_lval = p_peer->if_unit;
    }
END:    
    bgp_sem_give();
    return rc;
}

STATUS 
_bgpPeerSetApi(
        tBgpPeerIndex *p_index,
        u_int cmd,
        void *var,
        u_int flag)
{
    STATUS rc = VOS_OK;
    tBGP4_ADDR peer_ip;
    u_int lval = *(u_int*)var;
    octetstring *p_str = (octetstring *)var;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE *p_instance = NULL;

    /*sync config to others*/
    if ((cmd == BGP_PEER_PASSWORD) 
        || (cmd == BGP_PEER_VPNNAME))
    {
        flag |= USP_SYNC_OCTETDATA;
    }
    #if 0
    if (uspHwBgp4PeerSync(p_index, HW_BGP4PEER_CMDSTART+cmd, var, flag) != VOS_OK)
    {
        return VOS_ERR;
    }
    #endif
    memset(&peer_ip, 0, sizeof(peer_ip));
    peer_ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 : 128;
    peer_ip.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;
    memcpy(&peer_ip.ip, p_index->addr, 16);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        /*vpn instance need to be created*/
        if (/*(p_index->vrf_id == 0) ||*/ (cmd != BGP_PEER_RAWSTATUS))
        {
            rc = VOS_ERR;
            goto END;
        }
    }

    /*do not configure instance to be deleted*/
    if (p_instance && bgp4_timer_is_active(&p_instance->delete_timer))
    {
        rc = VOS_ERR;
        goto END;
    }
    
    if(p_instance != NULL)
    {
        p_peer = bgp4_peer_lookup(p_instance, &peer_ip);
    }
    
    if ((p_peer == NULL) && (cmd != BGP_PEER_RAWSTATUS))
    {
        rc = VOS_ERR;
        goto END;
    }

    /*can not set any attribute for route to be deleted*/
    if (p_peer && (bgp4_timer_is_active(&p_peer->delete_timer) == TRUE))
    {
        rc = VOS_ERR;
        goto END;
    }
    
    switch (cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_PEER_ADMINSTATUS:
             if (lval == BGP4_PEER_RUNNING)
             {
                 p_peer->admin_state = BGP4_PEER_RUNNING;
                 bgp4_fsm(p_peer, BGP4_EVENT_START);
             }
             else
             {
                 /*re-init start interval*/
                 p_peer->start_interval = BGP4_DFLT_STARTINTERVAL;
                 p_peer->notify.sub_code = BGP4_ADMINISTRATIVE_SHUTDOWN;
                 bgp4_fsm(p_peer, BGP4_EVENT_STOP);
                 p_peer->admin_state = BGP4_PEER_HALTED;
             }
             break;
             
        case BGP_PEER_REMOTEAS:
             /*can not change as when peer is running*/
             if (p_peer->admin_state == BGP4_PEER_RUNNING)
             {
                 rc = VOS_ERR;
             }
             else
             {
                 p_peer->as = lval;

                 if (bgp4_peer_type(p_peer) == BGP4_IBGP)
                 {
                     p_peer->adv_interval = BGP4_DFLT_IBGP_MINROUTEADVINTERVAL;
                 }
                 else
                 {
                    p_peer->ttl_hops = 1;
                 }
             }
             break;

        case BGP_PEER_CONNECTRETRYINTERVAL:
             p_peer->retry_interval = lval;
             break;
                
        case BGP_PEER_HOLDTIMECONFIGURED:
             p_peer->hold_interval = lval;
             break;
                
        case BGP_PEER_KEEPALIVECONFIGURED:
             p_peer->keep_interval = lval;
             break;
       
        case BGP_PEER_MINASORIGINTERVAL:
             p_peer->origin_interval = lval;
             break;
                
        case BGP_PEER_MINROUTEADVERINTERVAL:
             p_peer->adv_interval = lval;
             break;
                
        case BGP_PEER_PASSWORDSUPPORT:
             p_peer->md5_support = lval;
             break;
                
        case BGP_PEER_PASSWORD:
             if (p_str->len == 0)
             {
                 p_peer->md5key_len = 0;
                 break;
             }
             if (p_str->len <= 80)
             {
                 memcpy(p_peer->md5key, p_str->pucBuf, p_str->len);
                 p_peer->md5key_len = p_str->len;
             }
             break;
       
        case BGP_PEER_LOCALAFIFLAG:
             bgp4_peer_af_set(p_peer, lval);
             
             /*update 6pe peer*/
             p_instance->v6pe_count = bgp4_6pe_peer_calculate(p_instance);
             break;
                
        /*
        *EbgpPeerRtReflectorState_reflector = 1,
        *EbgpPeerRtReflectorState_client = 2,
        *EbgpPeerRtReflectorState_non_client = 3,
        *EbgpPeerRtReflectorState_ebgp = 4
        */
        case BGP_PEER_RTREFLECTORSTATE:
             p_peer->is_reflector_client = lval;
             break;
                
        /*
        * EbgpPeerSendCommunity_no_send 0
        * EbgpPeerSendCommunity_send 1
        */
        case BGP_PEER_SENDCONNUNITY:
             p_peer->send_community = lval;
             break;

        case BGP_PEER_RAWSTATUS: 
             switch (lval){
                 case SNMP_CREATEANDGO:
                 case SNMP_CREATEANDWAIT:
                      if (p_peer == NULL)
                      {
                          if (p_instance == NULL)/*create vpn instance first*/
                          {
                              p_instance = bgp4_vpn_instance_create(BGP4_INSTANCE_IP, p_index->vrf_id);
                              if (p_instance == NULL)
                              {
                                  rc = VOS_ERR;
                                  break;
                              }
                          }
                          if(gbgp4.uiPeerCount >= gbgp4.max_peer)
                          {
                            printf("no peer can be created. max peer num is %d\n",gbgp4.max_peer);
                            rc = VOS_ERR;
                            break;
                          }
                          p_peer = bgp4_peer_create(p_instance, &peer_ip);
                          if (p_peer == NULL)
                          {
                              rc = VOS_ERR;
                              break;
                          }
                      }
                      p_peer->row_status = lval;
                      break;
                      
                 case SNMP_ACTIVE:
                 case SNMP_NOTINSERVICE:
                 case SNMP_NOTREADY:
                      if (p_peer)
                      {
                          p_peer ->row_status = lval;
                      }
                      else
                      {
                          rc = VOS_ERR;
                      }
                      break;
                 
                 case SNMP_DESTROY:
                      if (p_peer)
                      {
                          /*call peer delete hander,may start a timer*/
                          p_peer->notify.sub_code = BGP4_PEER_NOT_CONFIGURED;
                          bgp4_fsm(p_peer, BGP4_EVENT_STOP);
                          p_peer->admin_state = BGP4_PEER_HALTED;
                          
                          bgp4_peer_delete_expired(p_peer);
                      }
                      else
                      {
                           rc = VOS_ERR;
                      }
                      break;
             }
             /*update 6pe peer*/
             p_instance->v6pe_count = bgp4_6pe_peer_calculate(p_instance);
             break;
    
        case BGP_PEER_UPDATESOURCE:
             if (p_str->len == 4)
             {
                 p_peer->update_source.afi = BGP4_PF_IPUCAST;
                 p_peer->update_source.prefixlen = 32;
                 memcpy(p_peer->update_source.ip, p_str->pucBuf, 4);
             }
             else if (p_str->len == 16)
             {
                 p_peer->update_source.afi = BGP4_PF_IP6UCAST;
                 p_peer->update_source.prefixlen = 128;
                 memcpy(p_peer->update_source.ip, p_str->pucBuf, 16);
             }
             else if (p_str->len == 0)/*undo cmd*/
             {
                 memset(&p_peer->update_source, 0, sizeof(tBGP4_ADDR));
             }
             break;
             
        case BGP_PEER_IFTYPE:
		memset(p_peer->if_typename,0,sizeof(p_peer->if_typename));
             p_str->len = strlen(p_str->pucBuf);
             memcpy(p_peer->if_typename, p_str->pucBuf, p_str->len);
             break;
                
        case BGP_PEER_LOCALREFRESH:
             p_peer->local_refresh_enable = lval;
             break;
                
        case BGP_PEER_RTREFRESH:/*send route refresh msg to peer*/
             if (p_peer->state != BGP4_NS_ESTABLISH) 
             {
                 break;
             }
             if ((p_peer->local_refresh_enable == FALSE)
                || (p_peer->refresh_enable == FALSE))
             {
                 break;
             }
                 
             if (!bgp4_af_support(p_peer, lval))
             {
                 break;
             }
             bgp4_refresh_output(p_peer, lval, &p_peer->orf_out_table);
             break;
        
        case BGP_PEER_EBGPMTHOP:
             p_peer->mhop_ebgp_enable = lval;
             break;
                
        case BGP_PEER_VALIDTTLHOP:
             p_peer->ttl_hops = lval;
             if ((p_peer->sock > 0) && (p_peer->ttl_hops > 0))
             {
                 if (p_peer->ttl_hops < 256)
                 {
                     bgp4_tcp_set_peer_ttl(p_peer->sock, p_peer);
                 }
             }
             break;
                
        case BGP_PEER_SENDLABEL:
             if (p_peer->send_label != lval)
             {
                 p_peer->send_label = lval;

                 /*set 6PE enable flag for ipv4 peer*/
                 if (p_peer->local_ip.afi == BGP4_PF_IPUCAST)
                 {
                     if (lval == TRUE)
                     {
                         flag_set(p_peer->local_mpbgp_capability, BGP4_PF_IP6LABEL);
                     }
                     else
                     {
                         flag_clear(p_peer->local_mpbgp_capability, BGP4_PF_IP6LABEL);
                     }
                 }
                 /*update 6pe peer*/
                 p_instance->v6pe_count = bgp4_6pe_peer_calculate(p_instance);
             }
             break;
                
        case BGP_PEER_CEASERESTART:
             p_peer->cease_reason = 4;
             break;
                
        case BGP_PEER_CEASEDOWN:
             p_peer->cease_reason = 2;
             break;
                
        case BGP_PEER_CEASEMAXPREFIX:
             p_peer->cease_reason = 1;
             break;
                
        case BGP_PEER_CEASERESLACK:
             p_peer->cease_reason = 8;
             break;
               
        case BGP_PEER_CEASEREJECTCONN:
             p_peer->cease_reason = 5;
             break;
               
        case BGP_PEER_CEASEEBGP:
             p_peer->cease_reason = 6;
             break;
               
        case BGP_PEER_UNCONFIG:
             p_peer->cease_reason = 3;
             break;
               
        case BGP_PEER_NEXTHOPSELF:
             p_peer->nexthop_self = lval;
             break;
                
        case BGP_PEER_BFD_ENABLE:
             p_peer->bfd_enable = lval;
             if (p_peer->bfd_enable) 
             {
                 bgp4_bind_bfd(p_peer);
             }
             else
             {
                 bgp4_unbind_bfd(p_peer);
             }
             break;

        case BGP_PEER_PUBASONLY:
             p_peer->public_as_only = lval;
             break;
             
        case BGP_PEER_SUBASENABLE:
             p_peer->as_substitute_enable = lval;
             break;
                
        case BGP_PEER_ALLOWLOOPTIMES:
             p_peer->allow_as_loop_times = lval;
             break;
                
        case BGP_PEER_FAKEAS:
             p_peer->fake_as = lval;
             break;
      #ifdef WIN32
        case BGP_PEER_REMOTEPORT:
             p_peer->fixed_tcp_port = lval;
             break;
      #endif
        case BGP_PEER_UPE:/*set upe role*/
             p_peer->upe_enable = lval;

             /*update upe count*/
             bgp4_vpn_upe_peer_count_update(p_instance);
             break;
        case BGP_PEER_IFUINT:
            p_peer->if_unit = lval;
            break;
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
bgpPeerSetApi(
     tBgpPeerIndex *p_index,
     u_int cmd,
     void *var)
{
    return _bgpPeerSetApi(p_index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}

STATUS 
bgpPeerSyncApi(
     tBgpPeerIndex *p_index,
     u_int cmd,
     void *var)
{
    return _bgpPeerSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

STATUS 
bgpRouteGetIPV4First(tBgpRtIndex *p_index)
{
    tBGP4_ROUTE* p_route = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_RIB *p_rib = NULL;
    STATUS rc = VOS_ERR;
    
    bgp_sem_take();

    /*instance is assigned*/
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        goto END;
    }
    p_rib = p_instance->rib[BGP4_PF_IPUCAST];
    if (p_rib == NULL)
    {
        goto END;
    }
    bgp4_avl_for_each(&p_rib->rib_table, p_route)
    {
        if (p_route->proto == M2_ipRouteProto_bgp)
        {
            p_index->proto = p_route->proto;
            p_index->dest.prefixlen = p_route->dest.prefixlen;
            p_index->dest.afi = AF_INET;
            memcpy(p_index->dest.ip, p_route->dest.ip, 4);
            p_index->peer.addr_type = AF_INET;
            memcpy(p_index->peer.addr, p_route->p_path->p_peer->ip.ip, 4);
            rc = VOS_OK;
            break;
        }
    }
END:    
    bgp_sem_give();
    return rc;
}

STATUS 
bgpRouteGetIPV4Next(
        tBgpRtIndex* p_index,
        tBgpRtIndex* p_nextindex)
{
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE search_route;
    tBGP4_PATH search_path;
    tBGP4_PEER search_peer;
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_RIB *p_rib = NULL;
    STATUS rc = VOS_ERR;
    
    memset(&search_route, 0, sizeof(tBGP4_ROUTE));
    memset(&search_path, 0, sizeof(tBGP4_PATH));
    memset(&search_peer, 0, sizeof(tBGP4_PEER));

    /*construct virtul route*/
    search_route.dest.prefixlen = p_index->dest.prefixlen;

    search_route.dest.afi = BGP4_PF_IPUCAST;
    memcpy(search_route.dest.ip, p_index->dest.ip, 4);

    search_route.proto = p_index->proto;
    search_route.p_path = &search_path;

    if (search_route.proto == M2_ipRouteProto_bgp)
    {
        search_peer.ip.afi = BGP4_PF_IPUCAST;
        memcpy(search_peer.ip.ip, p_index->peer.addr, 4);
        search_peer.ip.prefixlen = 32;
        search_path.p_peer = &search_peer;
    }
    
    bgp_sem_take();

    /*instance is fixed*/
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        goto END;
    }

    /*get next of route*/
    p_rib = p_instance->rib[BGP4_PF_IPUCAST];
    if (p_rib == NULL)
    {
        goto END;
    } 
    bgp4_avl_for_each_greater(&p_rib->rib_table, p_route, &search_route)
    {
        if (p_route->proto == M2_ipRouteProto_bgp)
        {
            p_nextindex->proto = p_route->proto;
            p_nextindex->dest.prefixlen = p_route->dest.prefixlen;
            p_nextindex->dest.afi = AF_INET;
            memcpy(p_nextindex->dest.ip, p_route->dest.ip, 4);
            memcpy(p_nextindex->peer.addr, p_route->p_path->p_peer->ip.ip, 4);
            p_nextindex->peer.addr_type = AF_INET;
            rc = VOS_OK;
            break;
        }
    }
END:
    bgp_sem_give();
    return rc;
}

static void
bgp4_route_to_nm_index(
         tBGP4_ROUTE* p_route,
         tBgpRtIndex* p_index)
{
    memset(p_index, 0, sizeof(*p_index));

    p_index->vrf_id = p_route->p_path->p_instance->vrf;
    p_index->proto = p_route->proto;
    p_index->dest.prefixlen = p_route->dest.prefixlen;
    p_index->dest.afi = (p_route->dest.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
    memcpy(p_index->dest.ip, p_route->dest.ip, 16);
    
    if (p_route->p_path->p_peer)
    {
        p_index->peer.addr_type = (p_route->p_path->p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
        memcpy(p_index->peer.addr, p_route->p_path->p_peer->ip.ip, 16);
        p_index->peer.vrf_id = p_route->p_path->p_peer->p_instance->vrf;

        memcpy(p_index->nexthop, p_route->p_path->nexthop.ip, 16);
    }
    else
    {
        memset(&p_index->peer, 0, sizeof(tBgpPeerIndex));
        if (p_route->dest.afi == BGP4_PF_IPUCAST)
        {
            memcpy(p_index->nexthop, p_route->p_path->nexthop.ip, 16);
            p_index->peer.addr_type = AF_INET;
        }
        else
        {
            memcpy(p_index->nexthop, p_route->p_path->nexthop.ip, 16);
            p_index->peer.addr_type = AF_INET6;
        }
    }  
    return;
}

static void
bgp4_nm_index_to_route(
              tBgpRtIndex *p_index,
              tBGP4_ROUTE *p_route,
              tBGP4_PATH *p_path,
              tBGP4_PEER *p_peer)
{
    u_char zero[16] = {0};
    memset(p_route, 0, sizeof(tBGP4_ROUTE));
    memset(p_path, 0, sizeof(tBGP4_PATH));
    memset(p_peer, 0, sizeof(tBGP4_PEER));
    
    /*construct virtul route*/
    p_route->dest.prefixlen = p_index->dest.prefixlen;
    
    if (p_index->dest.afi == AF_INET)
    {
        p_route->dest.afi = BGP4_PF_IPUCAST;
        memcpy(p_route->dest.ip, p_index->dest.ip, 4);
    }
    else if (p_index->dest.afi == AF_INET6)
    {
        p_route->dest.afi = BGP4_PF_IP6UCAST;
        memcpy(p_route->dest.ip, p_index->dest.ip, 16);
    }
    
    p_route->proto = p_index->proto;
    p_route->p_path = p_path;
    
    if (p_route->proto == M2_ipRouteProto_bgp)
    {
        p_path->p_peer = p_peer;

        if (p_index->peer.addr_type == AF_INET)
        {
            p_peer->ip.afi = BGP4_PF_IPUCAST;
            memcpy(p_peer->ip.ip, p_index->peer.addr, 4);
            p_peer->ip.prefixlen = 32;

            /*summary route has null peer,check this case*/
            if (bgp_ip4(p_peer->ip.ip) == 0)
            {
                p_path->p_peer = NULL;
            }
        }
        else if (p_index->peer.addr_type == AF_INET6)
        {
            p_peer->ip.afi = BGP4_PF_IP6UCAST;
            memcpy(p_peer->ip.ip, p_index->peer.addr, 16);
            p_peer->ip.prefixlen = 128;
        }
        if (memcmp(p_index->peer.addr, zero, 16) == 0)
        {
            /*summary route has null peer,check this case*/
            p_path->p_peer = NULL;
        }
    }
    else
    {
        p_path->p_peer = NULL;
        if (p_index->dest.afi == AF_INET)
        {
            p_path->nexthop.afi = BGP4_PF_IPUCAST;
            memcpy(p_path->nexthop.ip, p_index->nexthop, 4);
            p_path->nexthop.prefixlen = 32;
        }
        else if (p_index->dest.afi == AF_INET6)
        {
            p_path->nexthop.afi = BGP4_PF_IP6UCAST;
            memcpy(p_path->nexthop.ip, p_index->nexthop, 16);
            p_path->nexthop.prefixlen = 128;
        }
    }
    return;
}

STATUS 
bgpRouteGetFirst(tBgpRtIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ROUTE* p_route;
    tBGP4_RIB *p_rib = NULL;
    u_int af[2] = {BGP4_PF_IPUCAST, BGP4_PF_IP6UCAST};
    u_int i = 0;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        goto END;
    }
    for (i = 0; i < 2 ; i++)
    {
        p_rib = p_instance->rib[af[i]];
        if (p_rib == NULL)
        {
            continue;
        } 
        bgp4_avl_for_each(&p_rib->rib_table, p_route)
        {
            /*only normal route checked*/
            if ((p_route->dest.afi != BGP4_PF_IPUCAST)
                && (p_route->dest.afi != BGP4_PF_IP6UCAST))
            {
                continue;
            }
            p_index->vrf_id = p_instance->vrf;
            bgp4_route_to_nm_index(p_route, p_index);
            rc = VOS_OK;
            goto END;
        }
    }
END:    
    bgp_sem_give();
    return rc;
}

STATUS 
bgpRouteGetNext(
     tBgpRtIndex* p_index,
     tBgpRtIndex* p_nextindex)
{
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE search_route;
    tBGP4_PATH path;
    tBGP4_PEER peer;
    STATUS rc = VOS_ERR;

    bgp4_nm_index_to_route(p_index, &search_route, &path, &peer);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        goto END;
    }
    if (p_instance->rib[search_route.dest.afi] == NULL)
    {
        goto END;
    }    
    bgp4_avl_for_each_greater(&p_instance->rib[search_route.dest.afi]->rib_table, p_route, &search_route)
    {
        /*only normal route checked*/
        if ((p_route->dest.afi != BGP4_PF_IPUCAST)
            && (p_route->dest.afi != BGP4_PF_IP6UCAST))
        {
            continue;
        }
        p_nextindex->vrf_id = p_instance->vrf;
        bgp4_route_to_nm_index(p_route, p_nextindex);
        rc = VOS_OK;
        goto END;
    }

    if (search_route.dest.afi == BGP4_PF_IP6UCAST)
    {
        if (p_instance->rib[BGP4_PF_IP6UCAST] == NULL)
        {
            goto END;
        }
        
        bgp4_avl_for_each(&p_instance->rib[BGP4_PF_IP6UCAST]->rib_table, p_route)
        {
            /*only normal route checked*/
            if ((p_route->dest.afi != BGP4_PF_IPUCAST)
                && (p_route->dest.afi != BGP4_PF_IP6UCAST))
            {
                continue;
            }
            p_nextindex->vrf_id = p_instance->vrf;
            bgp4_route_to_nm_index(p_route, p_nextindex);
            rc = VOS_OK;
            goto END;
        }
    }

END:    
    bgp_sem_give();
    return rc;
}

STATUS 
bgpRouteGetApi(
       tBgpRtIndex *p_index,
       u_int cmd,
       void *var)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PATH *p_path = NULL;
    u_int* p_lval =(u_int*)var;
    octetstring *p_str = (octetstring *)var;
    STATUS  rc = VOS_OK;
    char value_c[512];
    tBGP4_ROUTE* p_route;
    tBGP4_ROUTE search_route;
    tBGP4_PATH path;
    tBGP4_PEER peer;
    u_int af = 0;
    bgp4_nm_index_to_route(p_index, &search_route, &path, &peer);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);/*if no peer,instance id is also given*/
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    af = search_route.dest.afi;
    if (p_instance->rib[af] == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    p_route = bgp4_avl_lookup(&p_instance->rib[af]->rib_table, &search_route);
    if ((p_route == NULL) || (p_route->p_path == NULL))
    {
        rc = VOS_ERR;
        goto END;
    }
    p_path = p_route->p_path;
    switch (cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_ROUTE_NEXTHOP:
             if (p_route->dest.afi == BGP4_PF_IPUCAST)
             {
                 memcpy(p_str->pucBuf, p_path->nexthop.ip, 4);
                 p_str->len = 4;
             }
             else if (p_route->dest.afi == BGP4_PF_IP6UCAST)
             {
                 memcpy(p_str->pucBuf, &p_path->nexthop.ip, 16);
                 p_str->len = 16;
             }
             break;
                
        case BGP_ROUTE_MED:
             *p_lval = p_path->med;
             break;

       
        case BGP_ROUTE_LOCALPREF:
             *p_lval = p_path->localpref;
             break;
            
        case BGP_ROUTE_ASSQUENCE:
             {
                 tBGP4_ASPATH *p_aspath = NULL;
                 u_char *p_asseq = value_c;                 
                 u_short pathlen = 0;
                 
                 /*if there is no as-path exist,only return hdr*/
                 if (bgp4_avl_count(&p_path->aspath_list) == 0)
                 {
                    p_str->len = 2;
                    p_str->pucBuf[0] = BGP_ASPATH_SEQ;
                    p_str->pucBuf[1] = 0;
                    break;
                 }
                 /*construct aspath*/
                 bgp4_avl_for_each(&p_path->aspath_list, p_aspath)
                 {
                     /*type*/
                     *p_asseq = p_aspath->type;
                      p_asseq++;
                      pathlen++;

                     /*count*/
                     *p_asseq = p_aspath->count;
                      p_asseq++;
                      pathlen++;

                     /*as copy*/
                      memcpy(p_asseq, p_aspath->as, p_aspath->count*2);
                      p_asseq += p_aspath->count*2;
                      pathlen += p_aspath->count*2;
                 }

                 if(p_str->len > pathlen)
                 {
                    p_str->len = pathlen;
                    memcpy(p_str->pucBuf, value_c, p_str->len);
                 }
             }
             break;
                
        case BGP_ROUTE_ORIGIN:
             *p_lval = p_path->origin + 1;/*port to mib*/
             break;
                
        case BGP_ROUTE_ATOMICAGGR:
             *p_lval = p_path->atomic_exist;
             break;
        
        case BGP_ROUTE_AGGREGATORADDR:
             if (p_path->p_aggregator)
             {
                 memcpy(p_str->pucBuf, p_path->p_aggregator->ip.ip, 16);
                 p_str->len = 16;
             }
             else
             {
                 p_str->len = 0;
             }
             break;
             
        case BGP_ROUTE_CALCLOCALPREF:
             *p_lval = p_path->localpref;
             break;
        
        case BGP_ROUTE_BEST:
             *p_lval = (bgp4_best_route(&p_instance->rib[af]->rib_table, &p_route->dest)) ? VOS_TRUE : VOS_FALSE;
             break;

        case BGP_ROUTE_ATTRUNKNOWN:
             /*max length is 255*/
              p_str->len = p_path->unknown_len & 0x000000ff;
             if (p_str->len)
             {
                 memcpy(p_str->pucBuf, p_path->p_unkown, p_str->len);
             }
             break;
        
        case BGP_ROUTE_RD:
             if (p_str->len > 32)
             {
                 bgp4_printf_vpn_rd(p_route, p_str->pucBuf);
                 p_str->len = strlen(p_str->pucBuf);
             }
             break;
                
        case BGP_ROUTE_INLABEL:
             *p_lval = p_route->in_label;
             break;
             
        case BGP_ROUTE_OUTLABEL:
             *p_lval = p_route->out_label;
             break;
#if 1            
        case BGP4_ROUTE_COMMUNITY:
             if (p_path->community_len 
                && p_path->p_community
                && (p_str->len >= p_path->community_len))
             {
                 memcpy(p_str->pucBuf, p_path->p_community, p_path->community_len);
                 p_str->len = p_path->community_len;
             }
             else
             {
                 p_str->len = 0;
             }
             break;
             
        case BGP4_ROUTE_EXTCOMMUNITY:
             if (p_path->excommunity_len
                && p_path->p_ecommunity
                && (p_str->len >= p_path->excommunity_len))
             {
                 memcpy(p_str->pucBuf, p_path->p_ecommunity, p_path->excommunity_len);
                 p_str->len = p_path->excommunity_len;
             }
             else
             {
                 p_str->len = 0;
             }
             break;
             
        case BGP4_ROUTE_CLUSTERID:
             if (p_path->cluster_len
                && p_path->p_cluster
                && (p_str->len >= p_path->cluster_len))
             {
                 memcpy(p_str->pucBuf, p_path->p_cluster, p_path->cluster_len);
                 p_str->len = p_path->cluster_len;
             }
             else
             {
                 p_str->len = 0;
             }
             break;
         case BGP_ROUTE_PROTOCOL:
            *p_lval = p_route->proto;
            break;
#endif             
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
bgpMcastRouteGetFirst(tBgpRtIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ROUTE* p_route;
    tBGP4_RIB *p_rib = NULL;
    u_int af = BGP4_PF_IPMCAST;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        goto END;
    }
    p_rib = p_instance->rib[af];
    if (p_rib == NULL)
    {
        goto END;
    } 
    bgp4_avl_for_each(&p_rib->rib_table, p_route)
    {
        p_index->vrf_id = p_instance->vrf;
        bgp4_route_to_nm_index(p_route, p_index);
        rc = VOS_OK;
        goto END;
    }

END:    
    bgp_sem_give();
    return rc;
}

STATUS 
bgpMcastRouteGetNext(
     tBgpRtIndex* p_index,
     tBgpRtIndex* p_nextindex)
{
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE search_route;
    tBGP4_PATH path;
    tBGP4_PEER peer;
    u_int af = BGP4_PF_IPMCAST;
    STATUS rc = VOS_ERR;

    bgp4_nm_index_to_route(p_index, &search_route, &path, &peer);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        goto END;
    }
    if (p_instance->rib[af] == NULL)
    {
        goto END;
    }    
    bgp4_avl_for_each_greater(&p_instance->rib[af]->rib_table, p_route, &search_route)
    {
        p_nextindex->vrf_id = p_instance->vrf;
        bgp4_route_to_nm_index(p_route, p_nextindex);
        rc = VOS_OK;
        goto END;
    }

END:    
    bgp_sem_give();
    return rc;
}

STATUS 
bgpMcastRouteGetApi(
       tBgpRtIndex *p_index,
       u_int cmd,
       void *var)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PATH *p_path = NULL;
    u_int* p_lval =(u_int*)var;
    octetstring *p_str = (octetstring *)var;
    STATUS  rc = VOS_OK;
    char value_c[512];
    tBGP4_ROUTE* p_route;
    tBGP4_ROUTE search_route;
    tBGP4_PATH path;
    tBGP4_PEER peer;
    u_int af = BGP4_PF_IPMCAST;
    bgp4_nm_index_to_route(p_index, &search_route, &path, &peer);

    bgp_sem_take();
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);/*if no peer,instance id is also given*/
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    if (p_instance->rib[af] == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    p_route = bgp4_avl_lookup(&p_instance->rib[af]->rib_table, &search_route);
    if ((p_route == NULL) || (p_route->p_path == NULL))
    {
        rc = VOS_ERR;
        goto END;
    }
    p_path = p_route->p_path;
    switch (cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_ROUTE_NEXTHOP:
             memcpy(p_str->pucBuf, p_path->nexthop.ip, 4);
             p_str->len = 4;
             break;
                
        case BGP_ROUTE_MED:
             *p_lval = p_path->med;
             break;

        case BGP_ROUTE_LOCALPREF:
             *p_lval = p_path->localpref;
             break;
            
        case BGP_ROUTE_ASSQUENCE:
             {
                 tBGP4_ASPATH *p_aspath = NULL;
                 u_char *p_asseq = value_c;                 
                 u_short pathlen = 0;
                 
                 /*if there is no as-path exist,only return hdr*/
                 if (bgp4_avl_count(&p_path->aspath_list) == 0)
                 {
                    p_str->len = 2;
                    p_str->pucBuf[0] = BGP_ASPATH_SEQ;
                    p_str->pucBuf[1] = 0;
                    break;
                 }
                 /*construct aspath*/
                 bgp4_avl_for_each(&p_path->aspath_list, p_aspath)
                 {
                     /*type*/
                     *p_asseq = p_aspath->type;
                      p_asseq++;
                      pathlen++;

                     /*count*/
                     *p_asseq = p_aspath->count;
                      p_asseq++;
                      pathlen++;

                     /*as copy*/
                      memcpy(p_asseq, p_aspath->as, p_aspath->count*2);
                      p_asseq += p_aspath->count*2;
                      pathlen += p_aspath->count*2;
                 }
                 if (p_str->len > pathlen)
                 {
                     p_str->len = pathlen;
                 }
                 p_str->len = p_str->len & 0x000000ff;
                 memcpy(p_str->pucBuf, value_c, p_str->len);
             }
             break;
                
        case BGP_ROUTE_ORIGIN:
             *p_lval = p_path->origin + 1;
             break;
                
        case BGP_ROUTE_ATOMICAGGR:
             *p_lval = p_path->atomic_exist;
             break;
        
        case BGP_ROUTE_AGGREGATORADDR:
             if (p_path->p_aggregator)
             {
                 memcpy(p_str->pucBuf, p_path->p_aggregator->ip.ip, 16);
                 p_str->len = 16;
             }
             else
             {
                 p_str->len = 0;
             }
             break;
             
        case BGP_ROUTE_CALCLOCALPREF:
             *p_lval = p_path->localpref;
             break;
        
        case BGP_ROUTE_BEST:
             *p_lval = (bgp4_best_route(&p_instance->rib[af]->rib_table, &p_route->dest)) ? VOS_TRUE : VOS_FALSE;
             break;

        case BGP_ROUTE_ATTRUNKNOWN:
             /*max length is 255*/
              p_str->len = p_path->unknown_len & 0x000000ff;
             if (p_str->len)
             {
                 memcpy(p_str->pucBuf, p_path->p_unkown, p_str->len);
             }
             break;
        
        case BGP_ROUTE_RD:
             if (p_str->len > 32)
             {
                 bgp4_printf_vpn_rd(p_route, p_str->pucBuf);
                 p_str->len = strlen(p_str->pucBuf);
             }
             break;
                
        case BGP_ROUTE_INLABEL:
             *p_lval = p_route->in_label;
             break;
             
        case BGP_ROUTE_OUTLABEL:
             *p_lval = p_route->out_label;
             break;

        case BGP4_ROUTE_COMMUNITY:
             if (p_path->community_len 
                && p_path->p_community
                && (p_str->len >= p_path->community_len))
             {
                 memcpy(p_str->pucBuf, p_path->p_community, p_path->community_len);
                 p_str->len = p_path->community_len;
             }
             else
             {
                 p_str->len = 0;
             }
             break;
             
        case BGP4_ROUTE_EXTCOMMUNITY:
             if (p_path->excommunity_len
                && p_path->p_ecommunity
                && (p_str->len >= p_path->excommunity_len))
             {
                 memcpy(p_str->pucBuf, p_path->p_ecommunity, p_path->excommunity_len);
                 p_str->len = p_path->excommunity_len;
             }
             else
             {
                 p_str->len = 0;
             }
             break;
             
        case BGP4_ROUTE_CLUSTERID:
             if (p_path->cluster_len
                && p_path->p_cluster
                && (p_str->len >= p_path->cluster_len))
             {
                 memcpy(p_str->pucBuf, p_path->p_cluster, p_path->cluster_len);
                 p_str->len = p_path->cluster_len;
             }
             else
             {
                 p_str->len = 0;
             }
             break;
    }
END:
    bgp_sem_give();
    return rc;
}


static void
bgp4_vpnroute_to_nmindex(
            tBGP4_ROUTE *p_route,  
            tBgpVpnRtIndex *p_index)
{
    p_index->proto = p_route->proto;
    p_index->dest.prefixlen = p_route->dest.prefixlen - 64;
    p_index->dest.afi = (p_route->dest.afi == BGP4_PF_IPVPN) ? AF_INET : AF_INET6;
    memcpy(p_index->vpn_rd, p_route->dest.ip, 8);
    memcpy(p_index->dest.ip, p_route->dest.ip+8, 16);

    if (p_route->proto == M2_ipRouteProto_bgp && (p_route->p_path->p_peer))
    {
        p_index->peer.addr_type = (p_route->p_path->p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
        memcpy(p_index->peer.addr,p_route->p_path->p_peer->ip.ip,16);
        p_index->peer.vrf_id = p_route->p_path->p_peer->p_instance->vrf;
    }
    else
    {
        memset(&p_index->peer, 0, sizeof(tBgpPeerIndex));
    }
    p_index->vrf_id = 0;
    return;
}

static void
bgp4_nmindex_to_vpnroute(
              tBgpVpnRtIndex *p_index,
              tBGP4_ROUTE *p_route,
              tBGP4_PATH *p_path,
              tBGP4_PEER *p_peer)
{
    memset(p_route, 0, sizeof(tBGP4_ROUTE));
    memset(p_path, 0, sizeof(tBGP4_PATH));
    memset(p_peer, 0, sizeof(tBGP4_PEER));
    
    /*construct virtul route*/
    p_route->dest.prefixlen = p_index->dest.prefixlen + 64;
    
    if (p_index->dest.afi == AF_INET)
    {
        p_route->dest.afi = BGP4_PF_IPVPN;
        memcpy(p_route->dest.ip, p_index->vpn_rd, 8);
        memcpy(p_route->dest.ip+8, p_index->dest.ip, 4);
    }
    else if (p_index->dest.afi == AF_INET6)
    {
        p_route->dest.afi = BGP4_PF_IP6VPN;
        memcpy(p_route->dest.ip, p_index->vpn_rd, 8);
        memcpy(p_route->dest.ip+8, p_index->dest.ip, 16);
    }
    
    p_route->proto = p_index->proto;
    p_route->p_path = p_path;
    
    if (p_route->proto == M2_ipRouteProto_bgp)
    {
        if (p_index->peer.addr_type == AF_INET)
        {
            p_peer->ip.afi = BGP4_PF_IPUCAST;
            memcpy(p_peer->ip.ip, p_index->peer.addr, 4);
            p_peer->ip.prefixlen = 32;
        }
        else if (p_index->peer.addr_type == AF_INET6)
        {
            p_peer->ip.afi = BGP4_PF_IP6UCAST;
            memcpy(p_peer->ip.ip, p_index->peer.addr, 16);
            p_peer->ip.prefixlen = 128;
        }
        p_path->p_peer = p_peer;
    }
    else
    {
        p_path->p_peer = NULL;
    }
    return;
}

/*vpnv4/vpnv6 routes*/
STATUS 
bgpVpnRouteGetFirst(tBgpVpnRtIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ROUTE* p_route = NULL;
    u_int af = BGP4_PF_IPVPN;
    STATUS rc = VOS_ERR;

    bgp_sem_take();
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, 0);
    if (p_instance == NULL)
    {
        goto END;
    }
    
    if (p_instance->rib[af] == NULL)
    {
        goto END;
    }
    bgp4_avl_for_each(&p_instance->rib[af]->rib_table, p_route)
    {
        if ((p_route->dest.afi != BGP4_PF_IPVPN)
            && (p_route->dest.afi != BGP4_PF_IP6VPN))
        {
            continue;
        }
        bgp4_vpnroute_to_nmindex(p_route, p_index);
        rc = VOS_OK;
        break;
    }
END:    
    bgp_sem_give();
    return rc;
}

/*vpnv4/vpnv6 routes*/
STATUS 
bgpVpnRouteGetNext(
        tBgpVpnRtIndex* p_index,
        tBgpVpnRtIndex* p_nextindex)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE search;
    tBGP4_PATH path;
    tBGP4_PEER peer;
    u_int af = BGP4_PF_IPVPN;
    STATUS rc = VOS_ERR;

    bgp4_nmindex_to_vpnroute(p_index, &search, &path, &peer);

    bgp_sem_take();
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, 0);
    if (p_instance == NULL)
    {
        goto END;
    }
    
    if (p_instance->rib[af] == NULL)
    {
        goto END;
    }
    bgp4_avl_for_each_greater(&p_instance->rib[af]->rib_table, p_route, &search)
    {
        if ((p_route->dest.afi == BGP4_PF_IPVPN)
            || (p_route->dest.afi == BGP4_PF_IP6VPN))
        {
            bgp4_vpnroute_to_nmindex(p_route, p_nextindex);
            rc = VOS_OK;
            break;
        }
    }
END:    
    bgp_sem_give();
    return rc;
}

/*vpnv4/vpnv6 routes*/
STATUS 
bgpVpnRouteGetApi(
     tBgpVpnRtIndex *p_index,
     u_int cmd,
     void *var)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PATH *p_path = NULL;
    u_int* p_lval=(u_int*)var;
    octetstring *p_str = (octetstring *)var;
    STATUS  rc = VOS_OK;
    u_char value_c[512];
    tBGP4_ROUTE* p_route;
    tBGP4_ROUTE search;
    tBGP4_PATH path;
    tBGP4_PEER peer;
    u_int af = BGP4_PF_IPVPN;
    
    bgp4_nmindex_to_vpnroute(p_index, &search, &path, &peer);

    bgp_sem_take();
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, 0);
    if (p_instance == NULL)
    {    
        rc = VOS_ERR;
        goto END;
    }
    if (p_instance->rib[af] == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    p_route = bgp4_avl_lookup(&p_instance->rib[af]->rib_table, &search);
    if ((p_route == NULL) || (p_route->p_path == NULL))
    {
        rc = VOS_ERR;
        goto END;
    }
    p_path = p_route->p_path;
    
    switch (cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_ROUTE_NEXTHOP:
             memcpy(p_str->pucBuf, p_path->nexthop.ip, 4);
             
             p_str->len = 4;
             break;
             
        case BGP_ROUTE_MED:
             *p_lval = p_path->med;
             break;
             
        case BGP_ROUTE_LOCALPREF:
             *p_lval = p_path->localpref;
             break;
            
        case BGP_ROUTE_ASSQUENCE:
             {
                 tBGP4_ASPATH *p_aspath = NULL;
                 u_char *p_asseq = value_c;                 
                 u_short pathlen = 0;
                 
                 /*if there is no as-path exist,only return hdr*/
                 if (bgp4_avl_count(&p_path->aspath_list) == 0)
                 {
                    p_str->len = 2;
                    p_str->pucBuf[0] = BGP_ASPATH_SEQ;
                    p_str->pucBuf[1] = 0;
                    break;
                 }
                 /*construct aspath*/
                 bgp4_avl_for_each(&p_path->aspath_list, p_aspath)
                 {
                     /*type*/
                     *p_asseq = p_aspath->type;
                      p_asseq++;
                      pathlen++;

                     /*count*/
                     *p_asseq = p_aspath->count;
                      p_asseq++;
                      pathlen++;

                     /*as copy*/
                      memcpy(p_asseq, p_aspath->as, p_aspath->count*2);
                      p_asseq += p_aspath->count*2;
                      pathlen += p_aspath->count*2;
                 }
                 if(p_str->len > pathlen)
                 {
                    p_str->len = pathlen;
                    memcpy(p_str->pucBuf, value_c, p_str->len);
                 }
             }
             break;
             
        case BGP_ROUTE_ORIGIN:
             *p_lval = p_path->origin + 1;
             break;
        
        case BGP_ROUTE_ATOMICAGGR:
             *p_lval = p_path->atomic_exist;
             break;

        case BGP_ROUTE_AGGREGATORADDR:
             if (p_path->p_aggregator)
             {
                 memcpy(p_str->pucBuf, p_path->p_aggregator->ip.ip, 16);
                 p_str->len = 16;
             }
             else
             {
                 p_str->len = 0;
             }
             break;
        
        case BGP_ROUTE_CALCLOCALPREF:
             *p_lval = p_path->localpref;
             break;
                
        case BGP_ROUTE_BEST:
             if (bgp4_best_route(&p_instance->rib[af]->rib_table,
                   &p_route->dest) == p_route)
             {
                 *p_lval = 2;
             }
             else
             {
                 *p_lval = 1;
             }
             break;
 
        case BGP_ROUTE_ATTRUNKNOWN:
             if (p_path->unknown_len)
             {
                 /*max length is 255*/
                  p_str->len = p_path->unknown_len & 0x000000ff;
                 if (p_str->len)
                 {
                     memcpy(p_str->pucBuf, p_path->p_unkown, p_str->len);
                 }
             }
             break;
        
        case BGP_ROUTE_RD:
             if (p_str->len > 32)
             {
                 bgp4_printf_vpn_rd(p_route, p_str->pucBuf);
                 p_str->len = strlen(p_str->pucBuf);
             }
             break;
                
        case BGP_ROUTE_INLABEL:
             *p_lval = p_route->in_label;
             break;
             
        case BGP_ROUTE_OUTLABEL:
             *p_lval = p_route->out_label;
             break;

        case BGP4_ROUTE_COMMUNITY:
             if (p_path->community_len 
                && p_path->p_community
                && (p_str->len >= p_path->community_len))
             {
                 memcpy(p_str->pucBuf, p_path->p_community, p_path->community_len);
                 p_str->len = p_path->community_len;
             }
             else
             {
                 p_str->len = 0;
             }
             break;
             
        case BGP4_ROUTE_EXTCOMMUNITY:
             if (p_path->excommunity_len
                && p_path->p_ecommunity
                && (p_str->len >= p_path->excommunity_len))
             {
                 memcpy(p_str->pucBuf, p_path->p_ecommunity, p_path->excommunity_len);
                 p_str->len = p_path->excommunity_len;
             }
             else
             {
                 p_str->len = 0;
             }
             break;
             
        case BGP4_ROUTE_CLUSTERID:
             if (p_path->cluster_len
                && p_path->p_cluster
                && (p_str->len >= p_path->cluster_len))
             {
                 memcpy(p_str->pucBuf, p_path->p_cluster, p_path->cluster_len);
                 p_str->len = p_path->cluster_len;
             }
             else
             {
                 p_str->len = 0;
             }
             break;
    }
END:
    bgp_sem_give();
    return rc;
} 

STATUS 
bgpPathAttrGetApi(
    tBgpRtIndex *p_index,
    u_int cmd,
    void *var)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int* p_lval=(u_int*)var;
    octetstring *p_str = (octetstring *)var;
    STATUS  rc = VOS_OK;
    char value_c[512];
    tBGP4_ROUTE* p_route = NULL;
    tBGP4_ROUTE search;
    tBGP4_PATH path;
    tBGP4_PEER peer;
    tBGP4_PATH *p_path = NULL;
    tBGP4_PEER *p_peer = NULL;
    u_int af = 0;
    
    bgp4_nm_index_to_route(p_index, &search, &path, &peer);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->peer.vrf_id);/*if no peer,instance id is also given*/
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    af = path.af;
    
    if (p_instance->rib[af] == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    p_route = bgp4_avl_lookup(&p_instance->rib[af]->rib_table, &search);
    if (p_route == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    p_path = p_route->p_path;
    if (p_path == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    p_peer = p_path->p_peer;
    switch (cmd) {
        default:
             rc = VOS_ERR;
             break;
    
        case BGP_PATHATTR_PEER:
             if (p_peer == NULL)
             {
                 *p_lval = 0;
             }
             else
             {
                 if (p_peer->ip.afi == BGP4_PF_IPUCAST)
                 {
                     memcpy( p_str->pucBuf, p_peer->ip.ip, 4);
                     p_str->len = 4;
                 }
                 else if (p_peer->ip.afi == BGP4_PF_IP6UCAST)
                 {
                     memcpy( p_str->pucBuf, p_peer->ip.ip, 16);
                     p_str->len = 16;
                 }
             }
             break;
     
        case BGP_PATHATTR_IPADDRPREFIXLEN:
             *p_lval = p_route->dest.prefixlen;
             break;
            
        case BGP_PATHATTR_IPADDRPREFIX:
             if (p_route->dest.afi == BGP4_PF_IPUCAST)
             {
                 memcpy(p_str->pucBuf, p_route->dest.ip, 4);
                 p_str->len = 4;
             }
             else if (p_route->dest.afi == BGP4_PF_IP6UCAST)
             {
                 memcpy(p_str->pucBuf, p_route->dest.ip, 16);
                 p_str->len = 16;
             }
             break;
             
        case BGP_PATHATTR_ORIGIN:
             *p_lval = p_path->origin + 1;
             break;
            
        case BGP_PATHATTR_ASPATHSEGMENT:
             {
                 tBGP4_ASPATH *p_aspath = NULL;
                 u_char *p_asseq = value_c;                 
                 u_short pathlen = 0;
                 
                 /*if there is no as-path exist,only return hdr*/
                 if (bgp4_avl_count(&p_path->aspath_list) == 0)
                 {
                    p_str->len = 2;
                    p_str->pucBuf[0] = BGP_ASPATH_SEQ;
                    p_str->pucBuf[1] = 0;
                    break;
                 }
                 /*construct aspath*/
                 bgp4_avl_for_each(&p_path->aspath_list, p_aspath)
                 {
                     /*type*/
                     *p_asseq = p_aspath->type;
                      p_asseq++;
                      pathlen++;

                     /*count*/
                     *p_asseq = p_aspath->count;
                      p_asseq++;
                      pathlen++;

                     /*as copy*/
                      memcpy(p_asseq, p_aspath->as, p_aspath->count*2);
                      p_asseq += p_aspath->count*2;
                      pathlen += p_aspath->count*2;
                 }
                 if (p_str->len > pathlen)
                 {
                     p_str->len = pathlen;
                 }
                 p_str->len = p_str->len & 0x000000ff;
                 memcpy(p_str->pucBuf, value_c, p_str->len);
             }
             break;
                        
        case BGP_PATHATTR_NEXTHOP:
             if (p_route->dest.afi == BGP4_PF_IPUCAST)
             {
                 memcpy(p_str->pucBuf, p_path->nexthop.ip, 4);
                 p_str->len = 4;
             }
             else if (p_route->dest.afi == BGP4_PF_IP6UCAST)
             {
                 memcpy(p_str->pucBuf, p_path->nexthop.ip, 16);
                 p_str->len = 16; 
             }
             break;
             
        case BGP_PATHATTR_MULTIEXITDISC:
             *p_lval = p_path->med;
             break;
            
        case BGP_PATHATTR_LOCALPREF:
             *p_lval = p_path->localpref;
            break;
            
        case BGP_PATHATTR_ATOMICAGGREGATE:
             if (p_path->atomic_exist)
             {
                 *p_lval = 2;
             }
             else
             {
                 *p_lval = 1;
             }
             break;
             
        case BGP_PATHATTR_AGGREGATORAS:
             if (p_path->p_aggregator)
             {
                 *p_lval = p_path->p_aggregator->as;
             }
             else
             {
                 *p_lval = 0;
             }
             break;
            
        case BGP_PATHATTR_AGGREGATORADDR:
             if (p_path->p_aggregator)
             {
                 *p_lval = *(u_int*)p_path->p_aggregator->ip.ip;
             }
             else
             {
                 *p_lval = 0;
             }
             break;
            
        case BGP_PATHATTR_CALCLOCALPREF:
             *p_lval = (p_path->localpref) == 0 ? gbgp4.local_pref : (p_path->localpref);
             break;
            
        /*
        * Ebgp4PathAttrBest_false 1
        * Ebgp4PathAttrBest_true 2
        */
        case BGP_PATHATTR_BEST:
             if (bgp4_best_route(&p_instance->rib[af]->rib_table, 
                &p_route->dest) == p_route)
             {
                 *p_lval = 2;
             }
             else
             {
                 *p_lval = 1;
             }
             break;

        case BGP_PATHATTR_UNKNOWN:
             /*max length is 255*/
             p_str->len = p_path->unknown_len & 0x000000ff;
             if (p_str->len != 0)
             {
                 memcpy(p_str->pucBuf, p_path->p_unkown, p_str->len);
             }
             break;

        case BGP_PATHATTR_NEWASPATHSEGMENT:
             p_str->len = 0;
             break;
             
        case BGP_PATHATTR_NEWAGGREGATORAS:
             *p_lval = 0;
             break;
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
bgpConfedPeerGetFirst(u_int *p_index)
{
    u_int i = 0;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    for (i = 0 ;i < BGP4_MAX_CONFEDPEER ; i++)
    {
        if (gbgp4.confedration_as[i])
        {
            *p_index = gbgp4.confedration_as[i];
            rc = VOS_OK;
            break;
        }
    }
    bgp_sem_give();
    return rc;
}

int 
bgpConfedPeerGetNext(
         u_int as, 
         u_int *p_nextas)
{
    u_int i = 0 ;
    u_int find = 0;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    for (i = 0 ; i < BGP4_MAX_CONFEDPEER; i++)
    {
        if (gbgp4.confedration_as[i] == 0)
        {
            continue;
        }
        if (gbgp4.confedration_as[i] == as)
        {
            find = 1;
        }
        else if (find == 1)
        {
            *p_nextas = gbgp4.confedration_as[i];
            rc = VOS_OK;
            break;
        }
    }
    bgp_sem_give();
    return rc;
}

STATUS 
bgpConfedPeerGet(
     u_int index,
     u_int cmd,
     void *var)
{
    STATUS  rc = VOS_OK;
    u_int i = 0 ;
    u_int* p_lval = (u_int*)var;
    u_int as = index;

    bgp_sem_take();

    for (i = 0 ;i < BGP4_MAX_CONFEDPEER ; i++)
    {
        if (gbgp4.confedration_as[i] == as)
        {
            break;
        }
    }
    if (i >= BGP4_MAX_CONFEDPEER)
    {
        rc = VOS_ERR;
        goto END;
    }

    switch (cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_CONFEDERATION_PEERINDEX :    /* Table Object INTEGER, read-only */
             *p_lval = i;
             break;

        case BGP_CONFEDERATION_PEERASNUMBER:    /* Table Object INTEGER, read-write */
             *p_lval = as;
             break;

        case BGP_CONFEDERATION_RAWSTATUS:    /* Table Object INTEGER, read-write */
             *p_lval= SNMP_ACTIVE;
             break;
}
END:
    bgp_sem_give();
    return rc;
}

STATUS 
_bgpConfedPeerSet(
       u_int index,
       u_int cmd,
       void *var,
       u_int flag)
{
    STATUS rc = VOS_OK;
    u_int lval = *(u_int*)var;
    u_int as = index;
    /*TODO:master slave sync*/
    #if 0
    /*sync config to others*/
    if (uspHwBgp4ConfederationSync((void*)as, HW_BGP4CONFEDERATION_CMDSTART+cmd, (void *)var, flag) != VOS_OK)
    {
        return VOS_ERR;
    }
    #endif
    bgp_sem_take();

    switch (cmd) {
        case BGP_CONFEDERATION_PEERINDEX :
             rc = VOS_ERR;
             break;

        case BGP_CONFEDERATION_PEERASNUMBER:
             rc = VOS_ERR;
             break;

        case BGP_CONFEDERATION_RAWSTATUS:
             if ((lval == SNMP_CREATEANDGO)
                 || (lval == SNMP_CREATEANDWAIT)
                 || (lval == SNMP_ACTIVE))
             {
                 if (bgp4_confedration_as_lookup(as) == NULL)
                 {
                     rc = bgp4_confedration_as_add(as);
                 }
             }
             else if (as == 0)
             {
                 bgp4_confedration_as_flush();
             }
             else
             {
                 bgp4_confedration_as_delete(as);
             }
             break;
             
        default:
             rc = VOS_ERR;
             break;
    }
    bgp_sem_give();
    return rc;
}

STATUS 
bgpConfedPeerSet(
       u_int index,
       u_int cmd,
       void *var)
{
    return _bgpConfedPeerSet(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}

STATUS 
bgpConfedPeerSync(
        u_int index,
        u_int cmd,
        void *var)
{
    return _bgpConfedPeerSet(index, cmd, var, USP_SYNC_LOCAL);
}

STATUS 
bgpRedistributeGetFirst(tBgpRedistributePolicyIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS rc = VOS_ERR;
    tBGP4_REDISTRIBUTE_CONFIG * p_policy = NULL;

    bgp_sem_take();

    bgp4_instance_for_each(p_instance)
    {
        p_policy = bgp4_avl_first(&p_instance->redistribute_policy_table);
        if (p_policy)
        {
            p_index->vrf_id = p_instance->vrf;
            p_index->policy_index = p_policy->policy;
            p_index->afi = p_policy->af;
            p_index->proto = p_policy->proto;
            p_index->processId = p_policy->processId;
            rc = VOS_OK;
            break;
        }
    }
    bgp_sem_give();
    return rc;
}

STATUS
bgpRedistributeGetNext(
    tBgpRedistributePolicyIndex* p_index, 
    tBgpRedistributePolicyIndex* p_nextIndex)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_VPN_INSTANCE search_instance;
    STATUS rc = VOS_ERR;
    tBGP4_REDISTRIBUTE_CONFIG* p_policy = NULL;
    tBGP4_REDISTRIBUTE_CONFIG search;

    bgp_sem_take();

    /*next node in same instance*/
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance)
    {
        search.af = p_index->afi;
        search.policy = p_index->policy_index;
        search.proto = p_index->proto;
        search.processId = p_index->processId;
        
        p_policy = bgp4_avl_greator(&p_instance->redistribute_policy_table, &search);
        if (p_policy)
        {
            rc = VOS_OK;
            goto END;
        }
    }

    /*first node in other instance*/
    search_instance.vrf = p_index->vrf_id;
    search_instance.type = BGP4_INSTANCE_IP;
    bgp4_avl_for_each_greater(&gbgp4.instance_table, p_instance, &search_instance)
    {
        p_policy = bgp4_avl_first(&p_instance->redistribute_policy_table);
        if (p_policy)
        {
            rc = VOS_OK;
            break;
        }
    }
END:    
    if (rc == VOS_OK)
    {
        p_nextIndex->vrf_id = p_instance->vrf;
        p_nextIndex->policy_index = p_policy->policy;
        p_nextIndex->afi = p_policy->af;
        p_nextIndex->proto = p_policy->proto;
        p_nextIndex->processId = p_policy->processId;
    }
    bgp_sem_give();
    return rc;
}

STATUS 
bgpRedistributeGetApi(
       tBgpRedistributePolicyIndex* p_index,
       u_int cmd,
       void *var)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS  rc = VOS_OK;
    u_int *p_lval = (u_int*)var; 
    tBGP4_REDISTRIBUTE_CONFIG* p_policy = NULL;
    tBGP4_REDISTRIBUTE_CONFIG search;
    
    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    search.af = p_index->afi;
    search.policy = p_index->policy_index;
    search.proto = p_index->proto;
    search.processId = p_index->processId;
    
    p_policy = bgp4_avl_lookup(&p_instance->redistribute_policy_table, &search);
    if (p_policy == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    switch(cmd) {
        case BGP_REDISTRIBUTE_STATUS:
             *p_lval = p_policy->status;
             break;
                    
        case BGP_REDISTRIBUTE_MED:
             *p_lval = p_policy->med;
             break;
             
        default:
             rc = VOS_ERR;
             break;
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
_bgpRedistributeSetApi(
        tBgpRedistributePolicyIndex* p_index,
        u_int cmd,
        void *var,
        u_int flag)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS  rc = VOS_OK;
    u_int lval = *(u_int*)var; 
    u_int sync_cmd = 0;
    tBGP4_REDISTRIBUTE_CONFIG* p_policy = NULL;
    tBGP4_REDISTRIBUTE_CONFIG search;

    /*sync config to others*/
    if (cmd == BGP_REDISTRIBUTE_VPNNAME)
    {
        flag |= USP_SYNC_OCTETDATA;
    }
    #if 0
    sync_cmd = HW_BGP4REDISTRIBUTION_CMDSTART+cmd;
    rc = uspHwBgp4RedistributionSync(p_index, sync_cmd, (void *) var,  flag);
    if (rc != VOS_OK)
    {
        return rc;
    }
    #endif
    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        if (/*p_index->vrf_id == 0 ||*/ cmd != BGP_REDISTRIBUTE_STATUS)/*vpn instance need to be created*/
        {
            rc = VOS_ERR;
            goto END;
        }
        p_instance = bgp4_vpn_instance_create(BGP4_INSTANCE_IP, p_index->vrf_id);
        if (p_instance == NULL)
        {
            rc = VOS_ERR;
            goto END;
        }
    }
    /*do not configure instance to be deleted*/
    if (bgp4_timer_is_active(&p_instance->delete_timer))
    {
        rc = VOS_ERR;
        goto END;
    }
    search.af = p_index->afi;
    search.policy = p_index->policy_index;
    search.proto = p_index->proto;
    search.processId = p_index->processId;
    
    p_policy = bgp4_avl_lookup(&p_instance->redistribute_policy_table, &search);
    if (p_policy == NULL && cmd != BGP_REDISTRIBUTE_STATUS)
    {
        rc = VOS_ERR;
        goto END;
    }

    switch (cmd) {
        case BGP_REDISTRIBUTE_STATUS:
             switch (lval){
                 case SNMP_CREATEANDWAIT:
                 case SNMP_CREATEANDGO:
                      if (p_policy == NULL)
                      {
                          p_policy = bgp4_redistribute_policy_create(
                                        p_instance,
                                        p_index->afi,
                                        p_index->proto,
                                        p_index->policy_index,
                                        p_index->processId);
                          if (p_policy == NULL)
                          {
                              rc = VOS_ERR;
                              break;
                          }
                          p_policy->status = lval;
                      }
                      break;
                      
                 case SNMP_ACTIVE:
                      if (p_policy)
                      {
                          p_policy->status = lval;
                          bgp4_redistribute_set(p_instance, p_policy->af, p_policy->proto, TRUE,p_index->processId);
                          if (p_policy->policy && gbgp4.policy_ref_func)
                          {
                              gbgp4.policy_ref_func(p_policy->policy, 1);
                          }
                      } 
                      break;
                      
                 case SNMP_NOTINSERVICE:
                 case SNMP_NOTREADY:
                      p_policy ->status = lval;
                      break;
                      
                 case SNMP_DESTROY:
                      if (p_policy)
                      {
                          bgp4_redistribute_set(p_instance, p_policy->af, p_policy->proto, FALSE,p_index->processId);
                          bgp4_redistribute_policy_delete(p_policy); 
                      }
                      break;
             }
             break;

        case BGP_REDISTRIBUTE_MED:
             p_policy->med = lval;
             break;             
            
        default:
             rc = VOS_ERR;
             break;
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
bgpRedistributeSetApi(
       tBgpRedistributePolicyIndex* index,
       u_int cmd,
       void *var)
{
    return _bgpRedistributeSetApi(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}

STATUS 
bgpRedistributeSyncApi(
        tBgpRedistributePolicyIndex* index,
        u_int cmd,
        void *var)
{
    return _bgpRedistributeSetApi(index, cmd, var, USP_SYNC_LOCAL);
}

STATUS 
bgpFilterGetFirst(tBgpFilterPolicyIndex *p_index)
{
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    STATUS rc = VOS_ERR;
    tBGP4_POLICY_CONFIG *p_policy = NULL;

    bgp_sem_take();

    bgp4_instance_for_each(p_instance)
    {
        p_policy = bgp4_avl_first(&p_instance->policy_table);
        if (p_policy)
        {
            p_index->vrf_id = p_instance->vrf;
            p_index->policy_index = p_policy->policy;
            p_index->afi = p_policy->af;
            p_index->proto = 0;
            p_index->direction = p_policy->direction;
            rc = VOS_OK;
            break;
        }
        /*if not found neither,then go to next instance to find*/
    }
    bgp_sem_give();
    return rc;
}

STATUS 
bgpFilterGetNext(
       tBgpFilterPolicyIndex* p_index, 
       tBgpFilterPolicyIndex* p_nextIndex)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_VPN_INSTANCE search_instance;
    STATUS rc = VOS_ERR;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_POLICY_CONFIG search; 

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance)
    {
       search.af = p_index->afi;
       search.direction = p_index->direction;
       search.policy = p_index->policy_index;
       p_policy = bgp4_avl_greator(&p_instance->policy_table, &search);
       if (p_policy)
       {
           rc = VOS_OK;
           goto END;
       }
    }
    search_instance.vrf = p_index->vrf_id;    
    search_instance.type = BGP4_INSTANCE_IP;
    bgp4_avl_for_each_greater(&gbgp4.instance_table, p_instance, &search_instance)
    {
        p_policy = bgp4_avl_first(&p_instance->policy_table);
        if (p_policy)
        {
            rc = VOS_OK;
            goto END;
        }
    }    
END:
    if (rc == VOS_OK)
    {
        p_nextIndex->vrf_id = p_instance->vrf;
        p_nextIndex->policy_index =  p_policy->policy;
        p_nextIndex->afi = p_policy->af;
        p_nextIndex->proto = 0;
        p_nextIndex->direction = p_policy->direction;
    }
    bgp_sem_give();
    return rc;
}

STATUS 
bgpFilterGetApi(
      tBgpFilterPolicyIndex* p_index,
      u_int cmd,
      void *var)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS rc = VOS_OK;
    u_int*p_lval = (u_int*)var;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_POLICY_CONFIG search;

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    search.af = p_index->afi;
    search.direction = p_index->direction;
    search.policy = p_index->policy_index;

    p_policy = bgp4_avl_lookup(&p_instance->policy_table, &search);
    if (p_policy == 0)
    {
        rc = VOS_ERR;
        goto END;
    }

    switch (cmd){
        case BGP_FILTER_STATUS:
             *p_lval = p_policy->status;
             break;

        default:
             rc = VOS_ERR;
             break;
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
_bgpFilterSetApi(
       tBgpFilterPolicyIndex* p_index,
       u_int cmd,
       void *var,
       u_int flag)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS  rc = VOS_OK;
    u_int lval = *(u_int*)var;
    u_int sync_cmd = 0;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_POLICY_CONFIG search;

    /*sync config to others*/
    if (cmd == BGP_FILTER_VPNNAME)
    {
        flag |= USP_SYNC_OCTETDATA;
    }
    /*TODO:master slave sync*/
    #if 0
    sync_cmd = HW_BGP4FILTER_CMDSTART+cmd;
    rc = uspHwBgp4FilterSync(p_index, sync_cmd, (void *) var, flag);
    if (rc != VOS_OK)
    {
        return VOS_ERR;
    }
    #endif
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        if (/*p_index->vrf_id == 0 ||*/cmd != BGP_FILTER_STATUS)/*vpn instance need to be created*/
        {
            rc = VOS_ERR;
            goto END;
        }
        p_instance = bgp4_vpn_instance_create(BGP4_INSTANCE_IP, p_index->vrf_id);
        if (p_instance == NULL)
        {
            rc = VOS_ERR;
            goto END;
        }
    }
    /*do not configure instance to be deleted*/
    if (bgp4_timer_is_active(&p_instance->delete_timer))
    {
        rc = VOS_ERR;
        goto END;
    }
    search.af = p_index->afi;
    search.direction = p_index->direction;
    search.policy = p_index->policy_index;
    
    p_policy = bgp4_avl_lookup(&p_instance->policy_table, &search);
    if (p_policy == NULL && cmd != BGP_FILTER_STATUS)
    {
        rc = VOS_ERR;
        goto END;
    }

    switch (cmd){
        case BGP_FILTER_STATUS:
             switch (lval){
                 case SNMP_CREATEANDWAIT:
                 case SNMP_CREATEANDGO:
                      if (p_policy == NULL)
                      {
                          p_policy = bgp4_policy_create(
                                             &p_instance->policy_table,
                                             p_index->afi, 
                                             p_index->direction,
                                             p_index->policy_index);
                          if (p_policy == NULL)
                          {
                              rc = VOS_ERR;
                              goto END;
                          }
                          p_policy->status = lval;                                                
                      }
                      break;
                      
                 case SNMP_ACTIVE:
                      if (p_policy->status == SNMP_ACTIVE)
                      {
                          break;
                      }
                      p_policy->status = SNMP_ACTIVE;
                      if (p_policy && p_policy->policy && gbgp4.policy_ref_func)
                      {
                          gbgp4.policy_ref_func(p_policy->policy, 1);
                      }
                      break;
                      
                 case SNMP_NOTINSERVICE:
                 case SNMP_NOTREADY:
                      if (p_policy)
                      {
                          p_policy ->status = lval;
                      }
                      break;
                 
                 case SNMP_DESTROY:
                      bgp4_policy_delete(p_policy);
                      break;
             }
             break;

        default:
             rc = VOS_ERR;
             break;
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
bgpFilterSetApi(
       tBgpFilterPolicyIndex* index,
       u_int cmd,
       void *var)
{
    return _bgpFilterSetApi(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}

STATUS 
bgpFilterSyncApi(
     tBgpFilterPolicyIndex* index,
     u_int cmd,
     void *var)
{
    return  _bgpFilterSetApi(index, cmd, var,USP_SYNC_LOCAL);
}

STATUS 
bgpPeerRoutePolicyGetFirst(tBgpPeerRtPolicyIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    STATUS rc = VOS_ERR;
    tBGP4_POLICY_CONFIG* p_policy = NULL;

    bgp_sem_take();

    bgp4_instance_for_each(p_instance)
    {
        bgp4_avl_for_each(&p_instance->peer_table, p_peer)
        {
            p_policy = bgp4_avl_first(&p_peer->policy_table);
            if (p_policy)
            {
                p_index->addr_type = (p_peer->ip.afi == BGP4_PF_IPUCAST) ?  AF_INET : AF_INET6 ;
                memcpy(p_index->addr, p_peer->ip.ip, 16);
                p_index->vrf_id = p_instance->vrf;
                p_index->direction = p_policy->direction;
                p_index->rt_policy_index = p_policy->policy;
                p_index->addr_family = p_policy->af;
                rc = VOS_OK;
                goto END;
            }
        }
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
bgpPeerRoutePolicyGetNext(
       tBgpPeerRtPolicyIndex* p_index,
       tBgpPeerRtPolicyIndex* p_next_index)
{
    STATUS rc = VOS_ERR;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_ADDR peer_addr;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER* p_peer = NULL;
    tBGP4_POLICY_CONFIG search;
    tBGP4_PEER search_peer;

    memset(&peer_addr, 0, sizeof(peer_addr));
    memcpy(peer_addr.ip, p_index->addr, 16);
    peer_addr.prefixlen = (p_index->addr_type == AF_INET) ? 32 :128;
    peer_addr.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    p_peer = bgp4_peer_lookup(p_instance, &peer_addr);
    if (p_peer != NULL)
    {
        search.af = p_index->addr_family;
        search.direction = p_index->direction;
        search.policy = p_index->rt_policy_index;
        p_policy = bgp4_avl_greator(&p_peer->policy_table, &search);
        if (p_policy)
        {
            rc = VOS_OK;
            goto END;
        }
    }

    /*get from next peer*/
    memset(&search_peer, 0, sizeof(search_peer));
    memcpy(&search_peer.ip, &peer_addr, sizeof(peer_addr));

    bgp4_avl_for_each_greater(&p_instance->peer_table, p_peer, &search_peer)
    {
        p_policy = bgp4_avl_first(&p_peer->policy_table);
        if (p_policy)
        {
            rc = VOS_OK;
            goto END;
        }
    }
END:
    if (rc == VOS_OK)
    {
        p_next_index->vrf_id = p_instance->vrf;
        p_next_index->rt_policy_index = p_policy->policy;
        p_next_index->addr_family = p_policy->af;
        p_next_index->direction = p_policy->direction;
        p_next_index->addr_type = (p_peer->ip.afi == BGP4_PF_IPUCAST) ?  AF_INET : AF_INET6;
        memcpy(p_next_index->addr, p_peer->ip.ip, 16);
    }
    bgp_sem_give();
    return rc;
}

STATUS 
bgpPeerRoutePolicyGetApi(
        tBgpPeerRtPolicyIndex* p_index,
        u_int cmd,
        void *var)
{
    STATUS  rc = VOS_OK;
    u_int*p_lval = (u_int*)var;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_PEER* p_peer = NULL;
    tBGP4_ADDR peer_addr;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_POLICY_CONFIG search;

    memset(&peer_addr, 0, sizeof(peer_addr));
    memcpy(peer_addr.ip, p_index->addr, 16);
    peer_addr.prefixlen = (p_index->addr_type == AF_INET) ? 32 :128;
    peer_addr.afi = (p_index->addr_type==AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    p_peer = bgp4_peer_lookup(p_instance, &peer_addr);
    if (p_peer == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    search.af = p_index->addr_family;
    search.direction = p_index->direction;
    search.policy = p_index->rt_policy_index;

    p_policy = bgp4_avl_lookup(&p_peer->policy_table, &search);
    if (p_policy == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    switch (cmd){
        case BGP_PEER_RTPOLICY_STATUS:
             *p_lval = p_policy->status;
             break;

        default:
             rc = VOS_ERR;
             break;
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
_bgpPeerRoutePolicySetApi(
        tBgpPeerRtPolicyIndex* p_index,
        u_int cmd,
        void *var,
        u_int flag)
{
    STATUS  rc = VOS_OK;
    u_int lval = *(u_int*)var;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_PEER* p_peer = NULL;
    tBGP4_ADDR peer_addr;
    u_int sync_cmd = 0;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_POLICY_CONFIG search;

    /*sync config to others*/
    if (cmd == BGP_PEER_RTPOLICY_VPNNAME)
    {
        flag |= USP_SYNC_OCTETDATA;
    }
    /*TODO:master slave sync*/
    #if 0
    sync_cmd = HW_BGP4PEERPOLICY_CMDSTART+cmd;
    rc = uspHwBgp4PeerPolicySync(p_index, sync_cmd, (void *) var, flag);
    if (rc != VOS_OK)
    {
        return rc;
    }
    #endif
    memset(&peer_addr, 0, sizeof(peer_addr));
    memcpy(peer_addr.ip, p_index->addr, 16);
    peer_addr.prefixlen = (p_index->addr_type == AF_INET) ? 32 :128;
    peer_addr.afi = (p_index->addr_type==AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    /*do not configure instance to be deleted*/
    if (bgp4_timer_is_active(&p_instance->delete_timer))
    {
        rc = VOS_ERR;
        goto END;
    }
    p_peer = bgp4_peer_lookup(p_instance, &peer_addr);
    if (p_peer == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    search.af = p_index->addr_family;
    search.direction = p_index->direction;
    search.policy = p_index->rt_policy_index;
    
    p_policy = bgp4_avl_lookup(&p_peer->policy_table, &search);
    if (p_policy == NULL && cmd != BGP_PEER_RTPOLICY_STATUS)
    {
        rc = VOS_ERR;
        goto END;
    }

    switch (cmd){
        case BGP_PEER_RTPOLICY_STATUS:
        {
             switch (lval){
                 case SNMP_CREATEANDWAIT:
                 case SNMP_CREATEANDGO:
                      if (p_policy == NULL)
                      {
                          p_policy = bgp4_policy_create(
                                         &p_peer->policy_table,
                                         p_index->addr_family,
                                         p_index->direction, 
                                         p_index->rt_policy_index);
                          if (p_policy == NULL)
                          {
                              rc = VOS_ERR;
                              goto END;
                          }
                          p_policy->status = SNMP_ACTIVE;
                      }
                      break;
                      
                 case SNMP_ACTIVE:
                      if (p_policy && p_policy->policy&& gbgp4.policy_ref_func)
                      {
                          p_policy->status = lval;
                          gbgp4.policy_ref_func(p_policy->policy,1);
                      }
                      break;
                 
                 case SNMP_NOTINSERVICE:
                 case SNMP_NOTREADY:
                      if (p_policy)
                      {
                          p_policy ->status = lval;
                      }
                      break;
                         
                 case SNMP_DESTROY:
                      bgp4_policy_delete(p_policy);
                      break;
            }
            break;
        }
        default:
            rc = VOS_ERR;
            break;
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
bgpPeerRoutePolicySetApi(
        tBgpPeerRtPolicyIndex* p_index,
        u_int cmd,
        void *var)
{
    return  _bgpPeerRoutePolicySetApi(p_index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}

STATUS 
bgpPeerRoutePolicySyncApi(
      tBgpPeerRtPolicyIndex* p_index,
      u_int cmd,
      void *var)
{
    return  _bgpPeerRoutePolicySetApi(p_index, cmd, var,USP_SYNC_LOCAL);
}

STATUS 
bgpAggregateGetFirst(tBgpAggrIndex*p_index)
{
    tBGP4_RANGE *p_range = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    bgp4_instance_for_each(p_instance)
    {
        p_range = bgp4_avl_first(&p_instance->range_table);
        if (p_range)
        {
            p_index->vrf_id = p_instance->vrf;
            p_index->afi = (p_range->dest.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
            p_index->prefixlen = p_range->dest.prefixlen;
            memcpy(p_index->ip, p_range->dest.ip, 16);
            rc = VOS_OK;
            break;
        }
    }
    bgp_sem_give();
    return rc;
}

STATUS 
bgpAggregateGetNext(
     tBgpAggrIndex *p_index, 
     tBgpAggrIndex *p_nextindex)
{
    tBGP4_RANGE *p_range = NULL;
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_VPN_INSTANCE search_instance;
    tBGP4_RANGE search;
    STATUS rc = VOS_ERR;

    memset(&search, 0, sizeof(search));

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance)
    {
        search.dest.prefixlen = p_index->prefixlen;
        memcpy(search.dest.ip, p_index->ip, 16);
        search.dest.afi = (p_index->afi == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;
        
        p_range = bgp4_avl_greator(&p_instance->range_table, &search);
        if (p_range)
        {
            rc = VOS_OK;
            goto END;
        }
    }
    search_instance.vrf = p_index->vrf_id;    
    search_instance.type = BGP4_INSTANCE_IP;
    bgp4_avl_for_each_greater(&gbgp4.instance_table, p_instance, &search_instance)
    {
       p_range = bgp4_avl_first(&p_instance->range_table);
       if (p_range)
       {
           rc = VOS_OK;
           goto END;
       }
    }
    
END:    
    if (rc == VOS_OK)
    {
        p_nextindex->afi = (p_range->dest.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
        memcpy(p_nextindex->ip, p_range->dest.ip, 16);
        p_nextindex->prefixlen = p_range->dest.prefixlen;
        p_nextindex->vrf_id = p_instance->vrf;
    }
    bgp_sem_give();
    return rc;
}

STATUS 
bgpAggregateGetApi(
        tBgpAggrIndex *p_index,
        u_int cmd,
        void *var)
{
    u_int *p_lval = var;
    tBGP4_RANGE *p_range = NULL;
    tBGP4_ADDR addr;
    STATUS rc = VOS_OK;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    memset(&addr, 0, sizeof(tBGP4_ADDR));

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    addr.prefixlen = p_index->prefixlen;
    memcpy(addr.ip, p_index->ip, 16);
    addr.afi = (p_index->afi == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;

    p_range = bgp4_range_lookup(p_instance, &addr);
    if (p_range == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    switch (cmd) {
        default:
             rc = VOS_ERR;
             break;

        case BGP_AGGREGATE_STATUS:    /* Table Object INTEGER, read-write */
             *p_lval = p_range->row_status;
             break;
        /*
        * EbgpAggregateAdvertise_summary 1
        * EbgpAggregateAdvertise_all 2
        */
        case BGP_AGGREGATE_ADVERTISE:    /* Table Object INTEGER, read-write */
             *p_lval = (p_range->summaryonly == TRUE) ? 1 : 2;
             break;
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
_bgpAggregateSetApi(
         tBgpAggrIndex *p_index,
         u_int cmd,
         void *var,
         u_int flag)
{
    u_int lval = *(u_int*)var;
    tBGP4_RANGE *p_range = NULL;
    tBGP4_ADDR addr;
    STATUS  rc = VOS_OK;
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    u_int sync_cmd = 0;
    /*TODO:master slave sync*/
    #if 0
    /*sync config to others*/
    sync_cmd = HW_BGP4AGGREGATE_CMDSTART+cmd;
    rc = uspHwBgp4AggregateSync(p_index, sync_cmd, (void *) var, flag);
    if (rc != VOS_OK)
    {
        return rc;
    }
    #endif
    memset(&addr, 0, sizeof(tBGP4_ADDR));

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    /*do not configure instance to be deleted*/
    if (bgp4_timer_is_active(&p_instance->delete_timer))
    {
        rc = VOS_ERR;
        goto END;
    }

    addr.prefixlen = p_index->prefixlen;
    memcpy(addr.ip, p_index->ip, 16);
    addr.afi = (p_index->afi == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;

    p_range = bgp4_range_lookup(p_instance, &addr);
    if (p_range == NULL && (cmd != BGP_AGGREGATE_STATUS))
    {
        rc = VOS_ERR;
        goto END;
    }

    switch (cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_AGGREGATE_STATUS:    /* Table Object INTEGER, read-write */
             if ((lval == SNMP_CREATEANDGO)
                     || (lval == SNMP_CREATEANDWAIT))
             {
                 if (p_range == NULL)
                 {
                     bgp4_range_create(p_instance, &addr);
                 }
             }
             else if (lval == SNMP_ACTIVE)
             {
                 if (p_range)
                 {
                     bgp4_range_up(p_range);
                 }
             }
             else if ((lval == SNMP_NOTINSERVICE)
                     || (lval == SNMP_NOTREADY))
             {
                 if (p_range)
                 {
                     bgp4_range_down(p_range);
                 }
             }
             else
             {
                 if (p_range)
                 {
                     bgp4_range_delete(p_range);
                 }
             }
             break;
        /*
        * EbgpAggregateAdvertise_summary 1
        * EbgpAggregateAdvertise_all 2
        */
        case BGP_AGGREGATE_ADVERTISE:
             p_range->summaryonly = (lval == 1) ? TRUE : FALSE;
             break;
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
bgpAggregateSetApi(
        tBgpAggrIndex *index,
        u_int cmd,
        void *var)
{
    return  _bgpAggregateSetApi(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}

STATUS 
bgpAggregateSyncApi (tBgpAggrIndex *index,u_int cmd,void *var)
{
    return  _bgpAggregateSetApi(index, cmd, var,USP_SYNC_LOCAL);
}

STATUS 
bgpDampGetFirst(void*p_index)
{
    return VOS_ERR;
}

STATUS 
bgpDampGetNext(
      void*p_index, 
      void*pNextIndex)
{
    return VOS_ERR;
}

STATUS 
bgpDampGetApi(
      void* p_index,
      u_int cmd,
      void *var)
{
    return VOS_ERR;
}

STATUS 
bgpDampSetApi(
     void*p_index,
     u_int cmd,
     void *var)
{
    return VOS_ERR;
}

STATUS 
bgpNetworkGetFirst(tBgpNetworkIndex* p_index)
{
    tBGP4_NETWORK *p_network;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    bgp4_instance_for_each(p_instance)
    {
        p_network = bgp4_avl_first(&p_instance->network_table);
        if (p_network)
        {
            p_index->vrf_id = p_instance->vrf;
            memcpy(p_index->ip, p_network->net.ip, 16);
            p_index->afi = p_network->net.afi;
            p_index->prefixlen = p_network->net.prefixlen;
            rc = VOS_OK;
            break;
        }
    }
    bgp_sem_give();
    return rc;
}

STATUS
bgpNetworkGetNext(
     tBgpNetworkIndex* p_index, 
     tBgpNetworkIndex* p_nextindex)
{
    tBGP4_NETWORK *p_network = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_VPN_INSTANCE search_vrf;
    tBGP4_NETWORK search;
    STATUS rc = VOS_ERR;

    memset(&search, 0, sizeof(search));

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance)
    {
        memcpy(search.net.ip, p_index->ip, 16);
        search.net.afi = p_index->afi;
        search.net.prefixlen = p_index->prefixlen;
        p_network = bgp4_avl_greator(&p_instance->network_table, &search);
        if (p_network)
        {
            rc = VOS_OK;
            goto END;
        }
    }
    search_vrf.vrf = p_index->vrf_id;    
    search_vrf.type = BGP4_INSTANCE_IP;
    bgp4_avl_for_each_greater(&gbgp4.instance_table, p_instance, &search_vrf)
    {
        p_network = bgp4_avl_first(&p_instance->network_table);
        if (p_network)
        {
            rc = VOS_OK;
            goto END;
        }
    }
    
END:    
    if (rc == VOS_OK)
    {
        memcpy(p_nextindex->ip, p_network->net.ip, 16);
        p_nextindex->prefixlen = p_network->net.prefixlen;
        p_nextindex->afi = p_network->net.afi;
        p_nextindex->vrf_id = p_instance->vrf;
    }
    bgp_sem_give();
    return rc ;
}

STATUS 
bgpNetworkGetApi(
      tBgpNetworkIndex*p_index,
      u_int cmd,
      void *var)
{
    tBGP4_NETWORK *p_network;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int* p_lval=(u_int*)var;
    tBGP4_ADDR net_addr;
    STATUS  rc = VOS_OK;
    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    memset(&net_addr, 0, sizeof(tBGP4_ADDR));
    net_addr.afi = p_index->afi;
    net_addr.prefixlen = p_index->prefixlen;
    memcpy(net_addr.ip, p_index->ip, 16);

    p_network = bgp4_network_lookup(p_instance, &net_addr);
    if (p_network == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    switch (cmd){
        case BGP_NETWORK_RAWSTATUS:
             *p_lval = p_network->status;
             break;
                
        default:
             rc = VOS_ERR;
             break;
    }
END:    
    bgp_sem_give();
    return rc;
}

STATUS 
_bgpNetworkSetApi(
      tBgpNetworkIndex *p_index,
      u_int cmd,
      void *var,
      u_int flag)
{
    tBGP4_NETWORK *p_network;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int lval= *(u_int*)var;
    tBGP4_ADDR net_addr;
    STATUS  rc = VOS_OK;
    u_int sync_cmd = 0;
    /*TODO:master slave sync*/
    #if 0
    /*sync config to others*/
    sync_cmd = HW_BGP4NETWORK_CMDSTART+cmd;
    rc = uspHwBgp4NetworkSync(p_index, sync_cmd, (void *) var,  flag);
    if (rc != VOS_OK)
    {
        return rc;
    }
    #endif
    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    /*do not configure instance to be deleted*/
    if (bgp4_timer_is_active(&p_instance->delete_timer))
    {
        rc = VOS_ERR;
        goto END;
    }

    memset(&net_addr,0,sizeof(tBGP4_ADDR));
    net_addr.afi = p_index->afi;
    net_addr.prefixlen = p_index->prefixlen;
    memcpy(net_addr.ip, p_index->ip, 16);

    p_network = bgp4_network_lookup(p_instance, &net_addr);
    if (p_network == NULL && (cmd != BGP_NETWORK_RAWSTATUS))
    {
        rc = VOS_ERR;
        goto END;
    }

    switch (cmd){
        case BGP_NETWORK_RAWSTATUS:
             switch (lval){
                 case SNMP_CREATEANDWAIT:
                 case SNMP_CREATEANDGO:
                      if (p_network == NULL)
                      {
                          p_network = bgp4_network_add(p_instance, &net_addr);
                          if (p_network == NULL)
                          {
                              rc = VOS_ERR;
                              goto END;
                          }
                          p_network->status = lval;
                      }
                      break;
                         
                 case SNMP_ACTIVE:
                      if (p_network)
                      {
                          bgp4_network_up(p_network);
                          p_network->status = lval;
                      }
                      break;
                  
                 case SNMP_NOTINSERVICE:
                 case SNMP_NOTREADY:
                      if (p_network)
                      {
                          p_network->status = lval;
                      } 
                      break;
                         
                 case SNMP_DESTROY:
                      if (p_network)
                      {
                          bgp4_network_delete(p_network);
                      }
                      break;
                      
                 default:
                      break;
             }
             break;

        default:
             rc = VOS_ERR;
             break;
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
bgpNetworkSetApi(
     void *index,
     u_int cmd,
     void *var)
{
    return  _bgpNetworkSetApi(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}

STATUS 
bgpNetworkSyncApi (
     void *index,
     u_int cmd,
     void *var)
{
    return  _bgpNetworkSetApi(index, cmd, var, USP_SYNC_LOCAL);
}

STATUS 
bgpExCommGetFirst(void *p_index)
{
    tBGP4_EXT_COMM* p_excomm = NULL;
    tBgpExtCommIndex* p_excomm_index = (tBgpExtCommIndex*)p_index;
    STATUS rc = VOS_ERR;
    
    bgp_sem_take();

    p_excomm = bgp4_avl_first(&gbgp4.ecom_table);
    if (p_excomm != NULL)
    {
        p_excomm_index->as = p_excomm->as;
        p_excomm_index->ipaddr = p_excomm->address;
        rc = VOS_OK;
    }
    bgp_sem_give();
    return rc;
}

STATUS 
bgpExCommGetNext(
       void *p_index, 
       void *p_nextindex)
{
    tBGP4_EXT_COMM* p_ext_comm=NULL;
    tBGP4_EXT_COMM* p_next =NULL;
    tBgpExtCommIndex* ext_comm_index=(tBgpExtCommIndex*)p_index;
    tBgpExtCommIndex* next_ext_comm_index=(tBgpExtCommIndex*)p_nextindex;
    STATUS rc = VOS_ERR;
    
    bgp_sem_take();

    bgp4_avl_for_each(&gbgp4.ecom_table, p_ext_comm)
    {
        if (p_ext_comm->as != ext_comm_index->as)
        {
            continue;
        }
        if (p_ext_comm->address != ext_comm_index->ipaddr)
        {
            continue;
        }
        p_next = bgp4_avl_next(&gbgp4.ecom_table, p_ext_comm);
        if (p_next != NULL)
        {
            next_ext_comm_index->as = p_next->as;
            next_ext_comm_index->ipaddr = p_next->address;
            rc = VOS_OK;
            goto END;
        }
    }

END:
    bgp_sem_give();
    return rc;
}

STATUS 
bgpExCommGetApi (
    void *p_index,
    u_int cmd,
    void *var)
{
    u_int* p_lval = var;
    tBGP4_EXT_COMM* p_ext_comm = NULL;
    tBGP4_EXT_COMM ext_comm;
    STATUS  rc = VOS_OK;
    tBgpExtCommIndex* ext_index = (tBgpExtCommIndex*)p_index;

    bgp_sem_take();

    memset(&ext_comm, 0, sizeof(tBGP4_EXT_COMM));
    ext_comm.as = ext_index->as;
    ext_comm.address = ext_index->ipaddr;

    p_ext_comm = bgp4_ext_comm_lookup(&ext_comm);
    if (p_ext_comm == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    switch (cmd){
         default:
              rc = VOS_ERR;
              break;
 
         case BGP_EXCOMMUNITY_MAJFLAG:
              *p_lval = p_ext_comm->main_type;
              break;
 
         case BGP_EXCOMMUNITY_SUBFLAG:
              *p_lval = p_ext_comm->sub_type;
              break;
              
         case BGP_EXCOMMUNITY_ADD:
              *p_lval = p_ext_comm->additive;
              break;
    }
END:
    bgp_sem_give();
    return rc;
}

STATUS 
_bgpExCommSetApi (
        void *p_index,
        u_int cmd,
        void *var,
        u_int flag)
{
    u_int* p_lval = var;
    u_int sync_cmd = 0;
    tBGP4_EXT_COMM  ext_comm;
    tBGP4_EXT_COMM* p_ext_comm = NULL;
    STATUS  rc = VOS_OK;
    tBgpExtCommIndex* ext_index = (tBgpExtCommIndex*)p_index;
    /*TODO:master slave sync*/
    #if 0
    /*sync config to others*/
    sync_cmd = HW_BGP4EXCOMMUNITY_CMDSTART+cmd;
    rc = uspHwBgp4ExcommunitySync(p_index, sync_cmd, (void *) var, flag);
    if (rc != VOS_OK)
    {
        return rc;
    }
    #endif

    bgp_sem_take();

    memset(&ext_comm, 0, sizeof(tBGP4_EXT_COMM));

    ext_comm.as = ext_index->as;
    ext_comm.address = ext_index->ipaddr;

    p_ext_comm = bgp4_ext_comm_lookup(&ext_comm);

    if (p_ext_comm == NULL && cmd != BGP_EXCOMMUNITY_STATUS)
    {
        rc = VOS_ERR;
        goto END;
    }
    switch (cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_EXCOMMUNITY_STATUS:
             if ((*p_lval == SNMP_CREATEANDGO)
                        || (*p_lval == SNMP_CREATEANDWAIT))
             {
                 if (p_ext_comm == NULL)
                 {
                     bgp4_add_ext_comm(&ext_comm);
                 }
                 else
                 {
                     rc = VOS_ERR;
                 }
             }
             else
             {
                 if (p_ext_comm)
                 {
                     bgp4_delete_ext_comm(p_ext_comm);
                 }
                 else
                 {
                     rc = VOS_ERR;
                 }
             }
             break;

        case BGP_EXCOMMUNITY_MAJFLAG:
             p_ext_comm->main_type = *p_lval;
             break;

        case BGP_EXCOMMUNITY_SUBFLAG:
             p_ext_comm->sub_type = *p_lval;
             break;

        case BGP_EXCOMMUNITY_ADD:
             p_ext_comm->additive = *p_lval;
             break;
}
END:
    bgp_sem_give();
    return rc;
}

STATUS 
bgpExCommSetApi(
       void *index,
       u_int cmd,
       void *var)
{
    return  _bgpExCommSetApi(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}

STATUS 
bgpExCommSyncApi (
     void *index,
     u_int cmd,
     void *var)
{
    return  _bgpExCommSetApi(index, cmd, var,USP_SYNC_LOCAL);
}

/*local orf capability table*/
int 
bgpPeerLocalOrfCapabilityGetFirst(tBgpPeerOrfCapabilityIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    bgp4_instance_for_each(p_instance)
    {
        p_peer = bgp4_avl_first(&p_instance->peer_table);
        if (p_peer)
        {
            p_index->addr_type = (p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
            memcpy(p_index->addr, p_peer->ip.ip, 16);
            p_index->vrf_id = p_instance->vrf;
            p_index->af = 0;
            rc = VOS_OK;
            break;
        }
    }
    bgp_sem_give();
    return rc;
}

int 
bgpPeerLocalOrfCapabilityGetNext(
      tBgpPeerOrfCapabilityIndex *p_index ,
      tBgpPeerOrfCapabilityIndex *p_next_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_PEER search_peer;
    tBGP4_VPN_INSTANCE search_instance;
    STATUS rc = VOS_ERR;

    memset(&search_peer.ip, 0, sizeof(search_peer.ip));
    memcpy(search_peer.ip.ip, p_index->addr, 16);
    search_peer.ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 : 128;
    search_peer.ip.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;

    bgp_sem_take();

    /*step1:get next peer in same instance*/
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance)
    {
        p_peer = bgp4_avl_lookup(&p_instance->peer_table, &search_peer);
        if (p_peer && (p_index->af < (BGP4_PF_MAX - 1)))
        {
            p_next_index->af = p_index->af + 1;
            rc = VOS_OK;
            goto END;
        }
        /*get next peer*/
        p_peer = bgp4_avl_greator(&p_instance->peer_table, &search_peer);
        if (p_peer)
        {
            p_next_index->af = 0;
            rc = VOS_OK;
            goto END;
        }
    }
    /*step2:get first peer in other instance*/
    search_instance.vrf = p_index->vrf_id;
    search_instance.type = BGP4_INSTANCE_IP;
    bgp4_avl_for_each_greater(&gbgp4.instance_table, p_instance, &search_instance)
    {
        p_peer = bgp4_avl_first(&p_instance->peer_table);
        if (p_peer)
        {
            p_next_index->af = 0;
            rc = VOS_OK;
            goto END;
        }
    }
END:    
    if (rc == VOS_OK)
    {
        p_next_index->addr_type = (p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
        memcpy(p_next_index->addr, p_peer->ip.ip, 16);
        p_next_index->vrf_id = p_instance->vrf;
    }
    bgp_sem_give();
    return rc;
}

STATUS 
bgpPeerLocalOrfCapabilityGetApi(
       tBgpPeerOrfCapabilityIndex *p_index,
       u_int cmd,
       void *var)
{
    STATUS rc = VOS_OK;
    tBGP4_ADDR peer_ip;
    u_int *p_lval = (u_int *)var;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    if (p_index->af >= (BGP4_PF_MAX - 1))
    {
        return VOS_ERR;
    }
    
    memset(&peer_ip, 0, sizeof(peer_ip));
    peer_ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 : 128;
    peer_ip.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;
    memcpy(peer_ip.ip, p_index->addr, 16);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    p_peer = bgp4_peer_lookup(p_instance, &peer_ip);
    if (p_peer == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    switch(cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_PEER_ORF_CAP_AF:
             *p_lval = p_index->af;
             break;
             
        case BGP_PEER_ORF_CAP_VALUE:
             *p_lval = 0;
             if (flag_isset(p_peer->orf_recv, p_index->af))
             {
                *p_lval |= BGP4_ORF_RECV_FLAG;
             }
             if (flag_isset(p_peer->orf_send, p_index->af))
             {
                *p_lval |= BGP4_ORF_SEND_FLAG;
             }
             break;            
    }
END:    
    bgp_sem_give();
    return rc;
}

STATUS 
_bgpPeerLocalOrfCapabilitySetApi(
        tBgpPeerOrfCapabilityIndex *p_index,
        u_int cmd,
        void *var,
        u_int flag)
{
    STATUS rc = VOS_OK;
    tBGP4_ADDR peer_ip;
    u_int lval = *(u_int*)var;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    
    if (p_index->af >= (BGP4_PF_MAX - 1))
    {
        return VOS_ERR;
    }
    
    memset(&peer_ip, 0, sizeof(peer_ip));
    peer_ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 : 128;
    peer_ip.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;
    memcpy(&peer_ip.ip, p_index->addr, 16);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        /*vpn instance need to be created*/
        if (/*(p_index->vrf_id == 0) ||*/ (cmd != BGP_PEER_RAWSTATUS))
        {
            rc = VOS_ERR;
            goto END;
        }
    }

    /*do not configure instance to be deleted*/
    if (bgp4_timer_is_active(&p_instance->delete_timer))
    {
        rc = VOS_ERR;
        goto END;
    }
    p_peer = bgp4_peer_lookup(p_instance, &peer_ip);
    if (p_peer == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    /*can not set any attribute for route to be deleted*/
    if (p_peer && (bgp4_timer_is_active(&p_peer->delete_timer) == TRUE))
    {
        rc = VOS_ERR;
        goto END;
    }
    switch (cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_PEER_ORF_CAP_VALUE:
             if (lval & BGP4_ORF_RECV_FLAG)
             {
                 flag_set(p_peer->orf_recv, p_index->af);
             }
             else
             {
                 flag_clear(p_peer->orf_recv, p_index->af);
             }   
             if (lval & BGP4_ORF_SEND_FLAG)
             {
                 flag_set(p_peer->orf_send, p_index->af);
             }
             else
             {
                 flag_clear(p_peer->orf_send, p_index->af);
             }   
             break;             
    }
END:
    bgp_sem_give();
    return rc;
}
STATUS 
bgpPeerLocalOrfCapabilitySetApi(
     tBgpPeerOrfCapabilityIndex *p_index,
     u_int cmd,
     void *var)
{
    return _bgpPeerLocalOrfCapabilitySetApi(p_index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}

STATUS 
bgpPeerLocalOrfCapabilitySyncApi(
     tBgpPeerOrfCapabilityIndex *p_index,
     u_int cmd,
     void *var)
{
    return _bgpPeerLocalOrfCapabilitySetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

/*remote orf capability table*/
int 
bgpPeerRemoteOrfCapabilityGetFirst(tBgpPeerOrfCapabilityIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    bgp4_instance_for_each(p_instance)
    {
        p_peer = bgp4_avl_first(&p_instance->peer_table);
        if (p_peer)
        {
            p_index->addr_type = (p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
            memcpy(p_index->addr, p_peer->ip.ip, 16);
            p_index->vrf_id = p_instance->vrf;
            p_index->af = 0;
            rc = VOS_OK;
            break;
        }
    }
    bgp_sem_give();
    return rc;
}

int 
bgpPeerRemoteOrfCapabilityGetNext(
      tBgpPeerOrfCapabilityIndex *p_index ,
      tBgpPeerOrfCapabilityIndex *p_next_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_PEER search_peer;
    tBGP4_VPN_INSTANCE search_instance;
    STATUS rc = VOS_ERR;

    memset(&search_peer.ip, 0, sizeof(search_peer.ip));
    memcpy(search_peer.ip.ip, p_index->addr, 16);
    search_peer.ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 : 128;
    search_peer.ip.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;

    bgp_sem_take();

    /*step1:get next peer in same instance*/
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance)
    {
        p_peer = bgp4_avl_lookup(&p_instance->peer_table, &search_peer);
        if (p_peer && (p_index->af < (BGP4_PF_MAX - 1)))
        {
            p_next_index->af = p_index->af + 1;
            rc = VOS_OK;
            goto END;
        }
        /*get next peer*/
        p_peer = bgp4_avl_greator(&p_instance->peer_table, &search_peer);
        if (p_peer)
        {
            p_next_index->af = 0;
            rc = VOS_OK;
            goto END;
        }
    }
    /*step2:get first peer in other instance*/
    search_instance.vrf = p_index->vrf_id;
    search_instance.type = BGP4_INSTANCE_IP;
    bgp4_avl_for_each_greater(&gbgp4.instance_table, p_instance, &search_instance)
    {
        p_peer = bgp4_avl_first(&p_instance->peer_table);
        if (p_peer)
        {
            p_next_index->af = 0;
            rc = VOS_OK;
            goto END;
        }
    }
END:    
    if (rc == VOS_OK)
    {
        p_next_index->addr_type = (p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
        memcpy(p_next_index->addr, p_peer->ip.ip, 16);
        p_next_index->vrf_id = p_instance->vrf;
    }
    bgp_sem_give();
    return rc;
}

STATUS 
bgpPeerRemoteOrfCapabilityGetApi(
       tBgpPeerOrfCapabilityIndex *p_index,
       u_int cmd,
       void *var)
{
    STATUS rc = VOS_OK;
    tBGP4_ADDR peer_ip;
    u_int *p_lval = (u_int *)var;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    if (p_index->af >= (BGP4_PF_MAX - 1))
    {
        return VOS_ERR;
    }
    
    memset(&peer_ip, 0, sizeof(peer_ip));
    peer_ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 : 128;
    peer_ip.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;
    memcpy(peer_ip.ip, p_index->addr, 16);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    p_peer = bgp4_peer_lookup(p_instance, &peer_ip);
    if (p_peer == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    switch(cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_PEER_ORF_CAP_AF:
             *p_lval = p_index->af;
             break;
             
        case BGP_PEER_ORF_CAP_VALUE:
             *p_lval = 0;
             if (flag_isset(p_peer->orf_remote_recv, p_index->af))
             {
                *p_lval |= BGP4_ORF_RECV_FLAG;
             }
             if (flag_isset(p_peer->orf_remote_send, p_index->af))
             {
                *p_lval |= BGP4_ORF_SEND_FLAG;
             }
             break;            
    }
END:    
    bgp_sem_give();
    return rc;
}

/*ORF entry*/
/*local orf capability table*/
int 
bgpPeerLocalOrfGetFirst(tBgpPeerOrfIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ORF_ENTRY *p_orf = NULL;
    tBGP4_PEER *p_peer = NULL;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    bgp4_instance_for_each(p_instance)
    {
        bgp4_avl_for_each(&p_instance->peer_table, p_peer)
        {
            p_orf = bgp4_avl_first(&p_peer->orf_out_table);
            if (p_orf)
            {
                p_index->addr_type = (p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
                memcpy(p_index->addr, p_peer->ip.ip, 16);
                p_index->vrf_id = p_instance->vrf;
                p_index->af = p_orf->prefix.afi;
                p_index->seqnum = p_orf->seqnum;
                rc = VOS_OK;
                goto END;
            }
        }
    }
END:    
    bgp_sem_give();
    return rc;
}

int 
bgpPeerLocalOrfGetNext(
       tBgpPeerOrfIndex* p_index,
       tBgpPeerOrfIndex* p_nextindex)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_ORF_ENTRY *p_orf = NULL;
    tBGP4_PEER search_peer;
    tBGP4_VPN_INSTANCE search_instance;
    tBGP4_ORF_ENTRY search_orf;
    STATUS rc = VOS_ERR;

    memset(&search_peer.ip, 0, sizeof(search_peer.ip));
    memcpy(search_peer.ip.ip, p_index->addr, 16);
    search_peer.ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 : 128;
    search_peer.ip.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;

    bgp_sem_take();

    /*step1:get next peer in same instance*/
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance)
    {
        p_peer = bgp4_avl_lookup(&p_instance->peer_table, &search_peer);
        if (p_peer)
        {
            search_orf.prefix.afi = p_index->af;
            search_orf.seqnum = p_index->seqnum;
            p_orf = bgp4_avl_greator(&p_peer->orf_out_table, &search_orf);
            if (p_orf)
            {
                rc = VOS_OK;
                goto END;
            }
        }
        /*get next peer*/
        bgp4_avl_for_each_greater(&p_instance->peer_table, p_peer, &search_peer)
        {
            p_orf = bgp4_avl_first(&p_peer->orf_out_table);
            if (p_orf)
            {
                rc = VOS_OK;
                goto END;
            }
        }
    }
    
    /*step2:get first peer in other instance*/
    search_instance.vrf = p_index->vrf_id;
    search_instance.type = BGP4_INSTANCE_IP;
    bgp4_avl_for_each_greater(&gbgp4.instance_table, p_instance, &search_instance)
    {
        bgp4_avl_for_each(&p_instance->peer_table, p_peer)
        {
            p_orf = bgp4_avl_first(&p_peer->orf_out_table);
            if (p_orf)
            {
                rc = VOS_OK;
                goto END;
            }
        }
    }
END:    
    if (rc == VOS_OK)
    {
        p_nextindex->addr_type = (p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
        memcpy(p_nextindex->addr, p_peer->ip.ip, 16);
        p_nextindex->vrf_id = p_instance->vrf;
        p_nextindex->af = p_orf->prefix.afi;
        p_nextindex->seqnum = p_orf->seqnum;
    }
    bgp_sem_give();
    return rc;
}


STATUS 
bgpPeerLocalOrfGetApi(
       tBgpPeerOrfIndex *p_index,
       u_int cmd,
       void *var)
{
    STATUS rc = VOS_OK;
    tBGP4_ADDR peer_ip;
    u_int *p_lval = (u_int *)var;
    octetstring *p_str = (octetstring *)var;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ORF_ENTRY *p_orf = NULL;
    
    if (p_index->af >= (BGP4_PF_MAX - 1))
    {
        return VOS_ERR;
    }
    
    memset(&peer_ip, 0, sizeof(peer_ip));
    peer_ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 : 128;
    peer_ip.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;
    memcpy(peer_ip.ip, p_index->addr, 16);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    p_peer = bgp4_peer_lookup(p_instance, &peer_ip);
    if (p_peer == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    
    p_orf = bgp4_orf_lookup(&p_peer->orf_out_table, p_index->af, p_index->seqnum);
    if (p_orf == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    
    switch(cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_PEER_ORF_AF:
             *p_lval = p_index->af;
             break;
             
        case BGP_PEER_ORF_SEQ:
             *p_lval = p_index->seqnum;
             break;
             
        case BGP_PEER_ORF_MATCH:
             *p_lval = p_orf->match;
             break;
             
        case BGP_PEER_ORF_MINLEN:
             *p_lval = p_orf->minlen;
             break;
             
        case BGP_PEER_ORF_MAXLEN:
             *p_lval = p_orf->maxlen;
             break;
             
        case BGP_PEER_ORF_LEN:
             *p_lval = p_orf->prefix.prefixlen;
             break;
             
        case BGP_PEER_ORF_DEST:
             if (p_str->len >= bgp4_bit2byte(p_orf->prefix.prefixlen))
             {
                 p_str->len = bgp4_bit2byte(p_orf->prefix.prefixlen);
                 memcpy(p_str->pucBuf, p_orf->prefix.ip, p_str->len);
             }
             break;
             
        case BGP_PEER_ORF_ROWSTATUS:
             *p_lval = p_orf->rowstatus;
             break;
    }
END:    
    bgp_sem_give();
    return rc;
}

STATUS 
_bgpPeerLocalOrfSetApi(
       tBgpPeerOrfIndex *p_index,
       u_int cmd,
       void *var,
       u_int flag)
{
    STATUS rc = VOS_OK;
    tBGP4_ADDR peer_ip;
    u_int lval = *(u_int *)var;
    octetstring *p_str = (octetstring *)var;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ORF_ENTRY *p_orf = NULL;
    tBGP4_ORF_ENTRY orf_dummy;
    avl_tree_t table;
    
    if (p_index->af >= (BGP4_PF_MAX - 1))
    {
        return VOS_ERR;
    }
    
    memset(&peer_ip, 0, sizeof(peer_ip));
    peer_ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 : 128;
    peer_ip.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;
    memcpy(peer_ip.ip, p_index->addr, 16);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    p_peer = bgp4_peer_lookup(p_instance, &peer_ip);
    if (p_peer == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    
    p_orf = bgp4_orf_lookup(&p_peer->orf_out_table, p_index->af, p_index->seqnum);
    if ((p_orf == NULL) && (cmd != BGP_PEER_ORF_ROWSTATUS))
    {
        rc = VOS_ERR;
        goto END;
    }
    
    switch(cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_PEER_ORF_MATCH:
             p_orf->match = lval;
             break;
             
        case BGP_PEER_ORF_MINLEN:
             p_orf->minlen = lval;
             break;
             
        case BGP_PEER_ORF_MAXLEN:
             p_orf->maxlen = lval;
             break;
             
        case BGP_PEER_ORF_LEN:
             p_orf->prefix.prefixlen = lval;
             break;
             
        case BGP_PEER_ORF_DEST:
             if (p_str->len <= BGP4_IP6ADDR_LEN)
             {
                 memcpy(p_orf->prefix.ip, p_str->pucBuf, p_str->len);
             }
             break;
             
        case BGP_PEER_ORF_ROWSTATUS:
             switch (lval) {
                 case SNMP_ACTIVE:
                      if (p_orf && (p_orf->rowstatus != SNMP_ACTIVE))
                      {
                          p_orf->rowstatus = SNMP_ACTIVE;

                          /*try to send refresh*/
                          if (flag_isset(p_peer->orf_send, p_index->af)
                              && flag_isset(p_peer->orf_remote_recv, p_index->af)
                              && (p_peer->state == BGP4_NS_ESTABLISH))
                          {
                              bgp4_unsort_avl_init(&table);
                              memcpy(&orf_dummy, p_orf, sizeof(tBGP4_ORF_ENTRY));
                              bgp4_avl_add(&table, &orf_dummy);
                              bgp4_refresh_output(p_peer, p_index->af, &table);
                          }
                      }
                      break;

                 case SNMP_NOTINSERVICE:
                      if (p_orf)
                      {
                          p_orf->rowstatus = SNMP_NOTINSERVICE;
                      }
                      break;
                      
                 case SNMP_NOTREADY:
                      break;
                      
                 case SNMP_CREATEANDGO:
                      if (p_orf == NULL)
                      {
                          p_orf = bgp4_orf_create(&p_peer->orf_out_table, p_index->af, p_index->seqnum);
                          if (p_orf)
                          {
                              p_orf->rowstatus = SNMP_ACTIVE;
                          }
                      }
                      break;
                      
                 case SNMP_CREATEANDWAIT:
                      if (p_orf == NULL)
                      {
                          p_orf = bgp4_orf_create(&p_peer->orf_out_table, p_index->af, p_index->seqnum);
                          if (p_orf)
                          {
                              p_orf->rowstatus = SNMP_NOTINSERVICE;
                          }
                      }
                      break;
                      
                 case SNMP_DESTROY:
                      if (p_orf)
                      {
                          /*try to send refresh*/
                          if (flag_isset(p_peer->orf_send, p_index->af)
                              && flag_isset(p_peer->orf_remote_recv, p_index->af)
                              && (p_peer->state == BGP4_NS_ESTABLISH))
                          {
                              bgp4_unsort_avl_init(&table);
                              memcpy(&orf_dummy, p_orf, sizeof(tBGP4_ORF_ENTRY));
                              orf_dummy.wait_delete = TRUE;
                              bgp4_avl_add(&table, &orf_dummy);
                              bgp4_refresh_output(p_peer, p_index->af, &table);
                          }

                          bgp4_orf_delete(p_orf);
                      }
                      break;

                 default:
                      break;
             }
             break;
    }
END:    
    bgp_sem_give();
    return rc;
}

STATUS 
bgpPeerLocalOrfSetApi(
     tBgpPeerOrfIndex *p_index,
     u_int cmd,
     void *var)
{
    return _bgpPeerLocalOrfSetApi(p_index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}

STATUS 
bgpPeerLocalOrfSyncApi(
     tBgpPeerOrfIndex *p_index,
     u_int cmd,
     void *var)
{
    return _bgpPeerLocalOrfSetApi(p_index, cmd, var, USP_SYNC_LOCAL);
}

/*remote orf capability table*/
int 
bgpPeerRemoteOrfGetFirst(tBgpPeerOrfIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ORF_ENTRY *p_orf = NULL;
    tBGP4_PEER *p_peer = NULL;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    bgp4_instance_for_each(p_instance)
    {
        bgp4_avl_for_each(&p_instance->peer_table, p_peer)
        {
            p_orf = bgp4_avl_first(&p_peer->orf_in_table);
            if (p_orf)
            {
                p_index->addr_type = (p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
                memcpy(p_index->addr, p_peer->ip.ip, 16);
                p_index->vrf_id = p_instance->vrf;
                p_index->af = p_orf->prefix.afi;
                p_index->seqnum = p_orf->seqnum;
                rc = VOS_OK;
                goto END;
            }
        }
    }
END:    
    bgp_sem_give();
    return rc;
}

int 
bgpPeerRemoteOrfGetNext(
       tBgpPeerOrfIndex* p_index,
       tBgpPeerOrfIndex* p_nextindex)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_ORF_ENTRY *p_orf = NULL;
    tBGP4_PEER search_peer;
    tBGP4_VPN_INSTANCE search_instance;
    tBGP4_ORF_ENTRY search_orf;
    STATUS rc = VOS_ERR;

    memset(&search_peer.ip, 0, sizeof(search_peer.ip));
    memcpy(search_peer.ip.ip, p_index->addr, 16);
    search_peer.ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 : 128;
    search_peer.ip.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;

    bgp_sem_take();

    /*step1:get next peer in same instance*/
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance)
    {
        p_peer = bgp4_avl_lookup(&p_instance->peer_table, &search_peer);
        if (p_peer)
        {
            search_orf.prefix.afi = p_index->af;
            search_orf.seqnum = p_index->seqnum;
            p_orf = bgp4_avl_greator(&p_peer->orf_in_table, &search_orf);
            if (p_orf)
            {
                rc = VOS_OK;
                goto END;
            }
        }
        /*get next peer*/
        bgp4_avl_for_each_greater(&p_instance->peer_table, p_peer, &search_peer)
        {
            p_orf = bgp4_avl_first(&p_peer->orf_in_table);
            if (p_orf)
            {
                rc = VOS_OK;
                goto END;
            }
        }
    }
    
    /*step2:get first peer in other instance*/
    search_instance.vrf = p_index->vrf_id;
    search_instance.type = BGP4_INSTANCE_IP;
    bgp4_avl_for_each_greater(&gbgp4.instance_table, p_instance, &search_instance)
    {
        bgp4_avl_for_each(&p_instance->peer_table, p_peer)
        {
            p_orf = bgp4_avl_first(&p_peer->orf_in_table);
            if (p_orf)
            {
                rc = VOS_OK;
                goto END;
            }
        }
    }
END:    
    if (rc == VOS_OK)
    {
        p_nextindex->addr_type = (p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
        memcpy(p_nextindex->addr, p_peer->ip.ip, 16);
        p_nextindex->vrf_id = p_instance->vrf;
        p_nextindex->af = p_orf->prefix.afi;
        p_nextindex->seqnum = p_orf->seqnum;
    }
    bgp_sem_give();
    return rc;
}


STATUS 
bgpPeerRemoteOrfGetApi(
       tBgpPeerOrfIndex *p_index,
       u_int cmd,
       void *var)
{
    STATUS rc = VOS_OK;
    tBGP4_ADDR peer_ip;
    u_int *p_lval = (u_int *)var;
    octetstring *p_str = (octetstring *)var;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ORF_ENTRY *p_orf = NULL;
    
    if (p_index->af >= (BGP4_PF_MAX - 1))
    {
        return VOS_ERR;
    }
    
    memset(&peer_ip, 0, sizeof(peer_ip));
    peer_ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 : 128;
    peer_ip.afi = (p_index->addr_type == AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;
    memcpy(peer_ip.ip, p_index->addr, 16);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_index->vrf_id);
    if (p_instance == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }

    p_peer = bgp4_peer_lookup(p_instance, &peer_ip);
    if (p_peer == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    
    p_orf = bgp4_orf_lookup(&p_peer->orf_in_table, p_index->af, p_index->seqnum);
    if (p_orf == NULL)
    {
        rc = VOS_ERR;
        goto END;
    }
    
    switch(cmd){
        default:
             rc = VOS_ERR;
             break;

        case BGP_PEER_ORF_AF:
             *p_lval = p_index->af;
             break;
             
        case BGP_PEER_ORF_SEQ:
             *p_lval = p_index->seqnum;
             break;
             
        case BGP_PEER_ORF_MATCH:
             *p_lval = p_orf->match;
             break;
             
        case BGP_PEER_ORF_MINLEN:
             *p_lval = p_orf->minlen;
             break;
             
        case BGP_PEER_ORF_MAXLEN:
             *p_lval = p_orf->maxlen;
             break;
             
        case BGP_PEER_ORF_LEN:
             *p_lval = p_orf->prefix.prefixlen;
             break;
             
        case BGP_PEER_ORF_DEST:
             if (p_str->len >= bgp4_bit2byte(p_orf->prefix.prefixlen))
             {
                 p_str->len = bgp4_bit2byte(p_orf->prefix.prefixlen);
                 memcpy(p_str->pucBuf, p_orf->prefix.ip, p_str->len);
             }
             break;
             
        case BGP_PEER_ORF_ROWSTATUS:
             *p_lval = p_orf->rowstatus;
             break;
    }
END:    
    bgp_sem_give();
    return rc;
}

/*function to obtain route count with special type
input:u2type---route type
u4Peer---only valid when route type is BGP
*/
u_int 
bgp4_get_number_of_route(
          u_short type, 
          tBGP4_PEER *p_peer)
{
    u_int i = 0 ;
    tBGP4_ROUTE *p_route = NULL; 
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PATH *p_path = NULL;
    u_int af = 0;
    
    bgp4_instance_for_each(p_instance)
    {
        for (af = 0; af < BGP4_PF_MAX; af++)
        {
            if (p_instance->rib[af] == NULL)
            {
                continue;
            }
            bgp4_avl_for_each(&p_instance->rib[af]->rib_table, p_route)
            {
                p_path = p_route->p_path;
                /*6PE route and VPN route should not be counted*/
                if (bgp4_index_to_safi(p_path->af) == BGP4_SAF_LBL ||
                    bgp4_index_to_safi(p_path->af) == BGP4_SAF_VLBL)
                {
                    break;
                }
    
                if (p_peer)
                {
                    if (p_path->p_peer == p_peer &&
                            p_path->p_peer->p_instance == p_instance)
                    {
                        i++;
                    }
                }
                else if (p_route->proto == type)
                {
                    if (p_path->origin_vrf == p_path->p_instance->vrf)
                    {
                        i++;
                    }
                }
            }
        }
    }
    return i;
}

struct vty;
#define VTY_NEWLINE "\n\r"
extern int vty_log(char * proto_str, int len);
extern int vty_out (struct vty *, const char *, ...);

int 
bgp_display_routes(
       tBGP4_VPN_INSTANCE* p_instance,
       struct vty * vty,u_int vrf_id,u_int* flag)
{
    u_char dstr[64],mstr[64],hstr[64];
    tBGP4_ROUTE *p_rt;
    int first =  1;
    u_int af = 0;
    u_char szcBuffer[1024] = {0};
    u_long ulLine = 0;
    
    bgp_sem_take();

    for (af = 0; af < BGP4_PF_MAX ; af++)
    {
        if (p_instance->rib[af] == NULL)
        {
            continue;
        }
        bgp4_avl_for_each(&p_instance->rib[af]->rib_table, p_rt)
        {
            memset(szcBuffer,0,sizeof(szcBuffer));
            if (*flag)
            {
                if (p_rt->dest.afi == BGP4_PF_IPMCAST)
                        vty_out(vty,"%s  Address Family:IPv4 Multicast%s",VTY_NEWLINE,VTY_NEWLINE);
                else
                        vty_out(vty,"%s",VTY_NEWLINE);
    
                vty_out(vty,"  %-16s%-16s%-16s%-7s%-9s%-10s%s%s","Destination","NetMask","NextHop","MED","LocPref","Proto","Vrf",VTY_NEWLINE);
                *flag = 0;
            }
    
            if (p_rt->dest.afi == BGP4_PF_IPUCAST)
            {
                inet_ntoa_1(dstr, ntohl(bgp_ip4(p_rt->dest.ip)));
                inet_ntoa_1(mstr, bgp4_len2mask(p_rt->dest.prefixlen));
            }
            else if (p_rt->dest.afi == BGP4_PF_IP6UCAST)
            {
                inet_ntop(AF_INET6, p_rt->dest.ip, dstr, 64);
            }
            else if (p_rt->dest.afi == BGP4_PF_IPVPN)
            {
                continue;
                inet_ntoa_1(dstr, ntohl(bgp_ip4(p_rt->dest.ip + BGP4_VPN_RD_LEN)));
                inet_ntoa_1(mstr, bgp4_len2mask(p_rt->dest.prefixlen - 8*BGP4_VPN_RD_LEN));
            }
    
            if (p_rt->p_path)
            {
                if (p_rt->dest.afi ==BGP4_PF_IPUCAST || p_rt->dest.afi ==BGP4_PF_IPVPN)
                {
                    inet_ntoa_1(hstr,ntohl(bgp_ip4(p_rt->p_path->nexthop.ip)));
                    sprintf(szcBuffer,"  %-16s%-16d%-16s%-7d%-9d%-10s%d%s", 
                                            dstr,p_rt->dest.prefixlen,hstr,
                                            p_rt->p_path->med,p_rt->p_path->localpref,
                                            bgp4_get_route_desc(p_rt->proto),vrf_id,VTY_NEWLINE);
                                            
                }
                else if(p_rt->dest.afi ==BGP4_PF_IP6UCAST)
                {
                    inet_ntop(AF_INET6,p_rt->p_path->nexthop.ip,hstr,64);
                    sprintf(szcBuffer,"  %-16s%-16d%-16s%-7d%-9d%-10s%d%s",
                                            dstr,p_rt->dest.prefixlen,hstr,
                                            p_rt->p_path->med,p_rt->p_path->localpref,
                                            bgp4_get_route_desc(p_rt->proto),vrf_id,VTY_NEWLINE);
                }
            }
            else
            {
                if(p_rt->dest.afi ==BGP4_PF_IPUCAST|| p_rt->dest.afi ==BGP4_PF_IPVPN)
                {
                    sprintf(szcBuffer,"  %-16s%-16s%-16s%-7s%-9s%-10s%d%s",
                                            dstr,mstr,"0.0.0.0",
                                            "N/A","N/A",
                                            bgp4_get_route_desc(p_rt->proto),vrf_id,VTY_NEWLINE);
                }
                else if(p_rt->dest.afi ==BGP4_PF_IP6UCAST)
                {
                    sprintf(szcBuffer,"  %-16s%-16d%-16s%-7s%-9s%-10s%d%s",
                                            dstr,p_rt->p_path->nexthop.prefixlen,"0.0.0.0",
                                            "N/A","N/A",
                                            bgp4_get_route_desc(p_rt->proto),vrf_id,VTY_NEWLINE);
                }
            }

            if (CMD_SUCCESS != vty_out_to_display(vty, &ulLine, szcBuffer))
            {
                return CMD_WARNING;
            }
        }
    }
    bgp_sem_give();
    return CMD_SUCCESS;
}

void bgp4_display_resource(struct vty * vty)
{ 
    tBGP4_ROUTE* p_route=NULL; 
    tBGP4_PEER *p_peer = NULL;
    tBGP4_RIB *p_rib = NULL;
    u_int update_rt=0;
    u_int establish = 0; 
    u_int af = 0;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int in_ip_process = 0;
    u_int rib_in = 0;
    u_int rib_use = 0;   
    u_int peer_state[BGP4_NS_MAX] = {0};
    u_int i = 0;
    u_char str[64];
    u_char afstr[64];

    /*Route Table Update*/
    vty_out(vty, "%sStatistic of BGP Rib to IPV4 forwarding table%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, " Add Route:%d,Add Route Fail:%d,Delete Route:%d,Delete Route Fail:%d%s",
		 gbgp4.stat.sys_route.add, gbgp4.stat.sys_route.fail,
		 gbgp4.stat.sys_route.delete,gbgp4.stat.sys_route.delete_fail,VTY_NEWLINE);
    vty_out(vty, " Add Last Error:%d/%d,Del Last Error:%d/%d%s",
         gbgp4.stat.sys_route.add_error,
         gbgp4.stat.sys_route.add_nomsg_error,
         gbgp4.stat.sys_route.delete_error,
         gbgp4.stat.sys_route.delete_nomsg_error,VTY_NEWLINE);
    
    vty_out(vty, " Add hardware route:%d,Delete hardware route:%d,send RT MSG fail:%d%s",
         gbgp4.stat.sys_route.msg_add, gbgp4.stat.sys_route.msg_delete,
         gbgp4.stat.sys_route.msg_fail, VTY_NEWLINE);
    vty_out(vty, " Route Msg Send:%d,Route %d,Failed %d%s",
         gbgp4.stat.sys_route.msg_send,
         gbgp4.stat.sys_route.msg_total_route,
         gbgp4.stat.sys_route.msg_fail,VTY_NEWLINE);
    
    vty_out(vty, "%sStatistic of BGP Rib to IPV6 forwarding table%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, " Add Route:%d,Add Route Fail:%d,Delete Route:%d,Delete Route Fail:%d%s",
         gbgp4.stat.sys_route6.add,gbgp4.stat.sys_route6.fail,
         gbgp4.stat.sys_route6.delete,gbgp4.stat.sys_route6.delete_fail,VTY_NEWLINE);
    
    vty_out(vty, " Add Last Error:%d/%d,Del Last Error:%d/%d%s",
         gbgp4.stat.sys_route6.add_error,
         gbgp4.stat.sys_route6.add_nomsg_error,
         gbgp4.stat.sys_route6.delete_error,
         gbgp4.stat.sys_route6.delete_nomsg_error, VTY_NEWLINE);
    vty_out(vty, " Add hardware route:%d,Del hardware route:%d,send RT MSG fail:%d%s",
         gbgp4.stat.sys_route6.msg_add, gbgp4.stat.sys_route6.msg_delete,
         gbgp4.stat.sys_route6.msg_fail,VTY_NEWLINE);
    vty_out(vty, " Route Msg Send:%d,Route %d,Failed %d%s",
         gbgp4.stat.sys_route6.msg_send,
         gbgp4.stat.sys_route6.msg_total_route,
         gbgp4.stat.sys_route6.msg_fail,VTY_NEWLINE);

    vty_out(vty, "%sStatistic of BGP Messages%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, " Send: update:%d,keepalive:%d,open:%d ,notify:%d,%dbytes%s",
        gbgp4.stat.msg_tx.update, gbgp4.stat.msg_tx.keepalive,
        gbgp4.stat.msg_tx.open,gbgp4.stat.msg_tx.notify,gbgp4.stat.msg_tx.bytes, VTY_NEWLINE);
    vty_out(vty, " Recv: update:%d,keepalive:%d,open:%d ,notify:%d,%dbytes%s",
        gbgp4.stat.msg_rx.update, gbgp4.stat.msg_rx.keepalive,
        gbgp4.stat.msg_rx.open,gbgp4.stat.msg_rx.notify,gbgp4.stat.msg_rx.bytes,VTY_NEWLINE);

    vty_out(vty, "%sStatistic of BGP MPLS VPN%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, " vpn instance add :%d,delete:%d%s",
        gbgp4.stat.vrf_add, gbgp4.stat.vrf_delete,VTY_NEWLINE);
    vty_out(vty, " vpn instance import route target add :%d,delete:%d%s",
        gbgp4.stat.import_rt_add, gbgp4.stat.import_rt_delete,VTY_NEWLINE);
    vty_out(vty, " vpn instance export route target add :%d,delete:%d%s",
        gbgp4.stat.export_rt_add, gbgp4.stat.export_rt_delete,VTY_NEWLINE);
    vty_out(vty, " mpls remote route notify add route:%d,failed:%d,last error:%d%s",
        gbgp4.stat.mpls_route.add, gbgp4.stat.mpls_route.fail,gbgp4.stat.mpls_route.add_error,VTY_NEWLINE);
    
    vty_out(vty, " mpls remote route notify del route:%d,failed:%d,last error %d%s",
        gbgp4.stat.mpls_route.delete, gbgp4.stat.mpls_route.delete_fail,
        gbgp4.stat.mpls_route.delete_error,VTY_NEWLINE);
    
    vty_out(vty, " remote route RT search Vrf fail:%d%s",
        gbgp4.stat.vrf_not_exist,VTY_NEWLINE);

    vty_out(vty, "%sStatistic of BGP Synchronization Information%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, " Sync msg send %d,msg recv %d,send msg error %d%s",gbgp4.stat.sync.tx,gbgp4.stat.sync.rx,gbgp4.stat.sync.fail,VTY_NEWLINE);
    vty_out(vty, " Recv too big update msg %d%s",gbgp4.stat.msg_rx.long_update,VTY_NEWLINE);

    bgp_sem_take();
    bgp4_instance_for_each(p_instance)
    {
        for (af = 0;af < BGP4_PF_MAX;af++)
        {
            if (p_instance->rib[af] == NULL)
            {
                continue;
            }
            p_rib = p_instance->rib[af];
            
            rib_in += bgp4_avl_count(&p_rib->rib_in_table);
            rib_use += bgp4_avl_count(&p_rib->rib_table);
            
            bgp4_avl_for_each(&p_rib->rib_table, p_route)
            {
                /*check if update need*/
                /*scan for each peer*/
				if (p_route->p_withdraw || p_route->p_feasible)
                {
                    update_rt++;
                }
                if (bgp4_system_update_finished(p_route) == FALSE)
                {
                    in_ip_process++;
                }
            }
        }
        
        bgp4_peer_for_each(p_instance, p_peer)
        {
            if (p_peer->state == BGP4_NS_ESTABLISH)
            {
                establish++;
            }
            vty_out(vty, "  \n\r Peer %s,Sock %d/%d/%d,Rx(f/w):%d/%d,Tx(f/w):%d/%d%s",
                bgp4_printf_peer(p_peer, str), 
                p_peer->sock,
                p_peer->sync_client_sock,
                p_peer->sync_server_sock,
                p_peer->stat.rx_fea_route,
                p_peer->stat.rx_with_route,
                p_peer->stat.tx_feasible,
                p_peer->stat.tx_withdraw, VTY_NEWLINE);

            peer_state[p_peer->state]++;
        }
    }
    bgp_sem_give();
    /*display peer state*/
    vty_out(vty, "%s Statistic of BGP Peer State%s",VTY_NEWLINE,VTY_NEWLINE);
    if (peer_state[BGP4_NS_IDLE])
    vty_out(vty, "  %-18s:%d\n\r", "Idle", peer_state[BGP4_NS_IDLE]);
    
    if (peer_state[BGP4_NS_CONNECT])
    vty_out(vty, "  %-18s:%d\n\r", "Connect", peer_state[BGP4_NS_CONNECT]);
    
    if (peer_state[BGP4_NS_ACTIVE])
    vty_out(vty, "  %-18s:%d\n\r", "Active", peer_state[BGP4_NS_ACTIVE]);

    if (peer_state[BGP4_NS_OPENSENT])
    vty_out(vty, "  %-18s:%d\n\r", "OpenSent", peer_state[BGP4_NS_OPENSENT]);

    if (peer_state[BGP4_NS_OPENCONFIRM])
    vty_out(vty, "  %-18s:%d\n\r", "OpenConfirm", peer_state[BGP4_NS_OPENCONFIRM]);

    if (peer_state[BGP4_NS_ESTABLISH])    
    vty_out(vty, "  %-18s:%d\n\r", "Establish", peer_state[BGP4_NS_ESTABLISH]);

    vty_out(vty, "  \n\r Total Use Route %d,Pend Route %d,Pend Discard %d%s",
        rib_use, rib_in, gbgp4.stat.rib_in_discard, VTY_NEWLINE);

    vty_out(vty, "%sStatistic of BGP Routes Updating Information%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, "  %d Routes Need to be Updated in IP table now %s",in_ip_process, VTY_NEWLINE);
    vty_out(vty, "  %d Routes Need to be Advertised now, Established Peers %d%s",update_rt, establish, VTY_NEWLINE);
    vty_out(vty, "  Walkup Count During System Update:%d%s",gbgp4.stat.system_loop, VTY_NEWLINE);

    if (gbgp4.stat.duplicate_delete)
    {
        vty_out(vty, " Existing duplicate deleted route,count %d...... ",
                                gbgp4.stat.duplicate_delete,
                                VTY_NEWLINE);
    }

    if (gbgp4.stat.import_filtered_route)
    {
        vty_out(vty, " Routes filtered by import route policy,count %d...... ",
                                gbgp4.stat.import_filtered_route,
                                VTY_NEWLINE);
    }

    if (gbgp4.stat.export_filtered_route)
    {
        vty_out(vty, " Routes filtered by export route policy,count %d...... ",
                                gbgp4.stat.export_filtered_route,
                                VTY_NEWLINE);
    }

   /* RIB Statistic
     
     VRF Peer AF Proto Count*/
     
    vty_out(vty, "\n\r RIB Count Statistics%s", VTY_NEWLINE);
    vty_out(vty, " %-8s%-10s%-16s%-12s%s%s", 
        "VRF", "AF", "PEER", "Proto", "Count", VTY_NEWLINE); 

    bgp_sem_take();

    u_int peer_count[BGP4_MAX_PEER_ID];
    u_int proto_count[24];
    bgp4_instance_for_each(p_instance)
    {
        for (af = 0; af < BGP4_PF_MAX; af++)
        {
            if (p_instance->rib[af] == NULL)
            {
                continue;
            }
            p_rib = p_instance->rib[af];
            if (bgp4_avl_count(&p_rib->rib_table) == 0)
            {
                continue;
            }
            memset(peer_count, 0, sizeof(peer_count));
            memset(proto_count, 0, sizeof(proto_count));
            bgp4_avl_for_each(&p_rib->rib_table, p_route)
            {
                if (p_route->p_path->p_peer 
                    && (p_route->p_path->origin_vrf == p_instance->vrf))
                {
                    peer_count[p_route->p_path->p_peer->bit_index]++;
                }
                else
                {
                    proto_count[p_route->proto]++;
                }
            }
            for (i = 0; i < BGP4_MAX_PEER_ID ; i++)
            {                    
                if (peer_count[i] == 0)
                {
                    continue;
                }
                p_peer = bgp4_peer_lookup_by_index(p_instance, i);
                if (p_peer == NULL)
                {
                    continue;
                }
                vty_out(vty, " %-8d%-10s%-16s%-12s%d%s", 
                    p_instance->vrf, bgp4_printf_af(af, afstr), 
                    bgp4_printf_addr(&p_peer->ip, str), "bgp", peer_count[i], VTY_NEWLINE);                     
            }
            for (i = 0; i < 16 ; i++)
            {
                if (proto_count[i])
                {
                    bgp4_printf_af(af, afstr); 
                    vty_out(vty, " %-8d%-10s%-16s%-12s%d%s", 
                    p_instance->vrf, afstr, 
                    "NULL", bgp4_get_route_desc(i), proto_count[i], VTY_NEWLINE);
                }
            }
            
            vty_out(vty, "\n\r %s", VTY_NEWLINE);
        }
    }
    bgp_sem_give();
        
    /*socket*/
    vty_out(vty, "%sStatistic of BGP Socket Operation%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, " %-10s%-8s%-8s%s","Operation","Total","Fail",VTY_NEWLINE);
    vty_out(vty, " %-10s%-8d%-8d%s","Open",
        gbgp4.stat.sock.create, gbgp4.stat.sock.create_fail,VTY_NEWLINE);
    vty_out(vty, " %-10s%-8d%-8d%s","Connect",
        gbgp4.stat.sock.connect,gbgp4.stat.sock.connect_fail,VTY_NEWLINE);
    vty_out(vty, " %-10s%-8d%-8d%s","Send",
        gbgp4.stat.sock.tx,gbgp4.stat.sock.tx_fail,VTY_NEWLINE);
    vty_out(vty, " %-10s%-8d%-8d%s","Recieve",
        gbgp4.stat.sock.rx, gbgp4.stat.sock.rx_fail,VTY_NEWLINE);
    vty_out(vty, " %-10s%-8d%-8d%s","Closed",gbgp4.stat.tcp_close,0,VTY_NEWLINE);

    /*global information*/
    vty_out(vty, "%s WorkMode %d,rtsock %d,server %d, server6 %d%s",
       VTY_NEWLINE, gbgp4.work_mode, gbgp4.rtsock, gbgp4.server_sock[0], gbgp4.server_sock6[0], VTY_NEWLINE);    
    vty_out(vty, " Server Support %d,Client Support %d%s",
          gbgp4.tcp_server_enable, gbgp4.tcp_client_enable, VTY_NEWLINE);
    #if 0 /*caoyong delete 2017.9.28 TODO:timerRemain*/
    vty_out(vty, "%s Restart Enable:%d,State:%d,Timer:%d%s",
        VTY_NEWLINE, gbgp4.restart_enable, gbgp4.in_restarting, 
        timerRemain(&gbgp4.gr_waiting_timer), VTY_NEWLINE);    
    #endif
    vty_out(vty, "%s Rtsock rx:%d,Error:%d,Last msg:%d%s",
     VTY_NEWLINE, gbgp4.stat.rtsock_rx, gbgp4.stat.rtsock_rx_error, 
     gbgp4.stat.rtsock_cmd, VTY_NEWLINE);    

    bgp4_display_memory(vty);
    return;
}

#else

#include "bgp4com.h"
#if !defined(USE_LINUX) && !defined(WIN32)
#include "In_var.h"
#endif
#if !defined(WIN32)

#endif

u_int bgp4_get_number_of_route(u_short type, tBGP4_PEER * p_peer);

STATUS bgpGlobalGetApi(void *index,u_int cmd,void *var)
{
    STATUS rc=VOS_OK;
    u_int *p_lval = (u_int *) var;
    octetstring *p_str = (octetstring *)var;

    /*some set api need not sem lock*/
    if(cmd == BGP_LOCAL_AS)
    {
        *p_lval = gBgp4.asnum ;
        return rc;
    }

    bgp_sem_take();

    switch(cmd)
    {
        case BGP_VERSION_NUMBER:
            /*octect string,len 1
              bit 0~7,set bit (v-1),verseion 4 == 0 0 0 1 0 0 0 0 */
            *p_lval = 0x10;
            
            break;
#if 0
        case BGP_LOCAL_AS:
            *p_lval = gBgp4.asnum ;
            break;
#endif
        case BGP_IDENTIFIER_NUMBER:
            *p_lval=gBgp4.router_id;
            break;
#if 0/*move to neighbour conf*/
        case BGP_NEXTHOP_SELF:
            *p_lval=gBgp4.self_nexthop;
            break;
#endif
        case BGP_CLUSTER_ID:
            *p_lval=gBgp4.cluster_id;
            break;

        case BGP_COMMUNITY_ACTION:
            *p_lval=gBgp4.community_action;
            break;

        case BGP_COMMUNITY:
            *p_lval=gBgp4.community;
            break;

        case BGP_CONFEDERATION_ID:
            *p_lval=gBgp4.confed_id;
            break;

        case BGP_TRAP_SUPPORT:
            *p_lval=gBgp4.trap_enable;
            break;

        case BGP_DEFAULT_LOCALPREF:
            *p_lval=gBgp4.default_lpref;
            break;

        case BGP_DEFAULT_MED:
            *p_lval = gBgp4.default_med;
            break;

        case BGP_SYNC_STATUS:
            *p_lval=gBgp4.sync_enable;
            break;

        case BGP_CEASE_STATUS:
            *p_lval=gBgp4.cease_enable;
            break;

        case BGP_LOCAL4BYTES_AS:
            *p_lval=0;
            break;

        case BGP_SERVER_ENABLE:
            *p_lval=gBgp4.server_open;
            break;

        case BGP_CLIENT_ENABLE:
            *p_lval=gBgp4.active_open;
            break;

        case BGP_UPDATE_SIZE:
            *p_lval=gBgp4.max_len;
            break;

        case BGP_TASK_PRIO:
            *p_lval=gBgp4.priority;
            break;

        case BGP_MAX_STACKSIZE:
            *p_lval=gBgp4.stacksize;
            break;

        case BGP_MAX_PEER:
            *p_lval=gBgp4.max_peer;
            break;

        case BGP_MAX_ROUTE:
            *p_lval=gBgp4.max_route;
            break;

        case BGP_MAX_PATH:
            *p_lval=gBgp4.max_path;
            break;
        case BGP_PROTO_RTNUM:
        {
            u_int number=0;
            number=bgp4_get_number_of_route(M2_ipRouteProto_local,NULL);
            *(p_str->pucBuf)=M2_ipRouteProto_local;
            memcpy(p_str->pucBuf+1,&number,4);
            number=bgp4_get_number_of_route(M2_ipRouteProto_netmgmt,NULL);
            *(p_str->pucBuf+5)=M2_ipRouteProto_netmgmt;
            memcpy(p_str->pucBuf+6,&number,4);
            number=bgp4_get_number_of_route(M2_ipRouteProto_rip,NULL);
            *(p_str->pucBuf+10)=M2_ipRouteProto_rip;
            memcpy(p_str->pucBuf+11,&number,4);
            number=bgp4_get_number_of_route(M2_ipRouteProto_is_is,NULL);
            *(p_str->pucBuf+15)=M2_ipRouteProto_is_is;
            memcpy(p_str->pucBuf+16,&number,4);
            number=bgp4_get_number_of_route(M2_ipRouteProto_ospf,NULL);
            *(p_str->pucBuf+20)=M2_ipRouteProto_ospf;
            memcpy(p_str->pucBuf+21,&number,4);
            number=bgp4_get_number_of_route(M2_ipRouteProto_other,NULL);
            *(p_str->pucBuf+25)=M2_ipRouteProto_other;
            memcpy(p_str->pucBuf+26,&number,4);
            p_str->len=30;

            break;
        }
        case BGP_BIGRT_UPDATE:
        {
                *p_lval=1;
            break;
        }
        case BGP_GR_ENABLE:
        {
            *p_lval = gBgp4.gr_enable;
            break;
        }
        case BGP_GR_TIME:
        {
            *p_lval = gBgp4.gr_restart_time;
            break;
        }
        case BGP_DEFERRAL_TIME:
        {
            *p_lval = gBgp4.gr_selection_deferral_time;
            break;
        }
        default:
            rc= VOS_ERR;
            break;
    }

    bgp_sem_give();

    return rc;
}

STATUS _bgpGlobalSetApi(void *index,u_int cmd,void *var,u_int flag)
{
    STATUS rc=VOS_OK;
    u_int sync_cmd = 0;
    u_int *p_lval=(u_int*) var;

    /*sync config to others*/
    if(cmd == BGP_GBL_SYNPKT)
    {
        flag |= USP_SYNC_OCTETDATA;
    }

    sync_cmd = HW_SYS_BGP4CMDSTART+cmd;
    if (sync_cmd)
    {
        rc = uspHwScalarSync(index, sync_cmd, (void *)var,  flag);
        if(rc != VOS_OK)
        {
            printf("\r\n sync error,config failed");
            return rc;
        }
    }

    /*some set api need not sem lock*/
    if(cmd == BGP_DEBUG_ON)
    {
        gBgp4.dbg_flag |= *p_lval;
        return rc;
    }
    else if(cmd == BGP_DEBUG_OFF)
    {
        gBgp4.dbg_flag &= ~(*p_lval);
        return rc;
    }
    else if(cmd == BGP_LOCAL_AS)
    {
        if((*p_lval )> 0)
        {
            rc = bgp4_enable((u_int)*p_lval);
        }
        else if(gBgp4.admin == TRUE && (*p_lval==0))
        {
            gBgp4.asnum=0;
        }
        else
            rc = VOS_ERR;

        return rc;
    }

    if((BGP4_MODE_SLAVE == gBgp4.work_mode)
        &&(BGP_GBL_SYNPKT == cmd))
    {
        bgp_sem_take();
    }
    bgp_sem_take();

    switch(cmd)
    {
        case BGP_IDENTIFIER_NUMBER:
        {
            u_int id = *p_lval;

            bgp4_set_router_id(id);

            break;
        }
#if 0/*move to neighbour conf*/
        case BGP_NEXTHOP_SELF:
            gBgp4.self_nexthop = *p_lval;
            break;
#endif
        case BGP_CLUSTER_ID:
            gBgp4.cluster_id = *p_lval;
            gBgp4.is_reflector = gBgp4.cluster_id ? TRUE : FALSE;
            break;

        case BGP_COMMUNITY_ACTION:
            gBgp4.community_action = *p_lval;
            break;

        case BGP_COMMUNITY:
            gBgp4.community = *p_lval;
            break;

        case BGP_CONFEDERATION_ID:
            gBgp4.confed_id = *p_lval;
            break;

        case BGP_TRAP_SUPPORT:
            gBgp4.trap_enable = (u_int)*p_lval;
            break;

        case BGP_DEFAULT_LOCALPREF:
            gBgp4.default_lpref = (u_int)*p_lval;
            break;

        case BGP_DEFAULT_MED:
            gBgp4.default_med = (u_int)*p_lval;
            break;

        case BGP_SYNC_STATUS:
            gBgp4.sync_enable = (u_int)*p_lval;
            break;

        case BGP_CEASE_STATUS:
            gBgp4.cease_enable=(u_int)*p_lval;
            break;

        case BGP_LOCAL4BYTES_AS:
            break;

        case BGP_SERVER_ENABLE:
        {
            gBgp4.server_open=(u_int)*p_lval;
            bgp4_server_open();
            break;
        }

        case BGP_CLIENT_ENABLE:
        {
            gBgp4.active_open=(u_int)*p_lval;
            break;
        }

        case BGP_TASK_PRIO:
            gBgp4.priority=*p_lval;
            break;

        case BGP_MAX_STACKSIZE:
            gBgp4.stacksize=*p_lval;
            break;

        case BGP_UPDATE_SIZE:
            gBgp4.max_len=(u_int)*p_lval;
            break;

        case BGP_MAX_PEER:
            gBgp4.max_peer = *p_lval;
            break;

        case BGP_MAX_ROUTE:
            gBgp4.max_route = *p_lval;
            break;

        case BGP_MAX_PATH:
            gBgp4.max_path = *p_lval;
            break;

        case BGP_GBL_SYNPKT:
            bgp4_recv_sync_all(var);
            break;

        case BGP_BIGRT_UPDATE:
        {
            break;
        }
        case BGP_GR_ENABLE:
        {
            gBgp4.gr_enable = *p_lval;
            break;
        }
        case BGP_GR_TIME:
        {
            gBgp4.gr_restart_time = *p_lval;
            break;
        }
        case BGP_DEFERRAL_TIME:
        {
            gBgp4.gr_selection_deferral_time = *p_lval;
            break;
        }
        case BGP_RESET_ALL:
        {
            bgp4_reset_all_peers(1);
            break;
        }
        default:
        {
            rc= VOS_ERR;
            break;
        }
    }

    bgp_sem_give();

    return rc;
}

STATUS bgpGlobalSetApi(void *index,u_int cmd,void *var)
{
    return  _bgpGlobalSetApi(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}
STATUS bgpGlobalSyncApi(void *index,u_int cmd,void *var)
{
    return  _bgpGlobalSetApi(index, cmd, var,USP_SYNC_LOCAL);
}

int bgpPeerGetFirst(tBgpPeerIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
        if(p_instance)
        {
            p_peer = (tBGP4_PEER *)bgp4_lstfirst(&p_instance->peer_list);
            if (p_peer)
            {
                if(p_peer->remote.ip.afi==BGP4_PF_IPUCAST)
                    p_index->addr_type = AF_INET;
                else if(p_peer->remote.ip.afi==BGP4_PF_IP6UCAST)
                    p_index->addr_type = AF_INET6;

                memcpy(p_index->addr, p_peer->remote.ip.ip, 16);
                p_index->vrf_id = p_instance->instance_id;
                rc = VOS_OK;
                break;
            }
        }

    }

    bgp_sem_give();


    return rc;
}

int bgpPeerGetNext(tBgpPeerIndex *p_index ,tBgpPeerIndex *p_next_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_PEER *p_next_peer = NULL;
    tBGP4_ADDR addr;
    STATUS rc = VOS_ERR;

    memset(&addr, 0, sizeof(addr));
    memcpy(addr.ip, p_index->addr, 16);
    addr.prefixlen = (p_index->addr_type == AF_INET) ? 32 :128;
    addr.afi = (p_index->addr_type==AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(p_index->vrf_id);
    if(p_instance)
    {
        p_peer = bgp4_peer_lookup(p_instance,&addr);
        if(p_peer)
        {
            p_next_peer = (tBGP4_PEER*)bgp4_lstnext(&p_instance->peer_list,&p_peer->node);
        }

        while(p_peer)
        {
            if(p_next_peer)
            {
                p_next_index->addr_type = (p_next_peer->remote.ip.afi==BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
                memcpy(p_next_index->addr, p_next_peer->remote.ip.ip, 16);
                p_next_index->vrf_id = p_instance->instance_id;
                rc = VOS_OK;
                break;
            }
            else/*find in next instance*/
            {
                p_instance = (tBGP4_VPN_INSTANCE*)bgp4_lstnext(&gBgp4.vpn_instance_list,&p_instance->node);
                if(p_instance)
                {
                    p_next_peer = (tBGP4_PEER*)bgp4_lstfirst(&p_instance->peer_list);
                    continue;
                }
                break;
            }

        }
    }

    bgp_sem_give();

    return rc;
}

STATUS bgpPeerGetApi(tBgpPeerIndex *p_index,u_int cmd,void *var)
{
    STATUS  rc = VOS_OK;
    tBGP4_ADDR peer_ip;
    u_int *p_lval = ( u_int * ) var;
    octetstring *p_str = (octetstring *)var;
    tBGP4_PEER *p_peer=NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    memset(&peer_ip, 0, sizeof(peer_ip));
    peer_ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 :128;
    peer_ip.afi = (p_index->addr_type==AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;
    memcpy(peer_ip.ip, p_index->addr, 16);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(p_index->vrf_id);
    if(p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    p_peer = bgp4_peer_lookup(p_instance,&peer_ip);

    if (p_peer == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    switch(cmd)
    {
        default:
        {
            rc = VOS_ERR;
            break;
        }
        case BGP_PEER_REMOTEIDENTIFIER:
        {
            *p_lval=p_peer->router_id;
            break;
        }
        case BGP_PEER_TYPE:
        {
            *p_lval = bgp4_peer_type(p_peer);
            break;
        }
        /*
        * EbgpPeerState_idle 1
        * EbgpPeerState_connect 2
        * EbgpPeerState_active 3
        * EbgpPeerState_opensent 4
        * EbgpPeerState_openconfirm 5
        * EbgpPeerState_established 6
        */
        case BGP_PEER_STATE:
        {
            *p_lval=p_peer->state;
            break;
        }
        case BGP_PEER_ADMINSTATUS:
        {
            *p_lval=p_peer->admin;
            break;
        }
        case BGP_PEER_NEGOTIATEDVERSION:
        {
            *p_lval=p_peer->version;
            break;
        }
        case BGP_PEER_LOCALADDRTYPE:
        {
            *p_lval = p_peer->local.ip.afi;
            break;
        }
        case BGP_PEER_LOCALADDR   :
        {
            p_str->len = p_peer->local.ip.prefixlen;
            memcpy(p_str->pucBuf, p_peer->local.ip.ip, p_str->len);
            break;
        }
        case BGP_PEER_LOCALPORT   :
        {
            *p_lval=p_peer->local.port;
            break;
        }

        case BGP_PEER_REMOTEPORT  :
        {
            *p_lval=p_peer->remote.port;
            break;
        }
        case BGP_PEER_REMOTEAS  :
        {
            *p_lval=p_peer->remote.as;
            break;
        }
        case BGP_PEER_INUPDATES  :
        {
            *p_lval = p_peer->rx_update;
            break;
        }
        case BGP_PEER_OUTUPDATES  :
        {
            *p_lval=p_peer->tx_update;
            break;
        }
        case BGP_PEER_INTOTALMSGS   :
        {
            *p_lval=p_peer->rx_msg;
            break;
        }
        case BGP_PEER_OUTTOTALMSGS   :
        {
            *p_lval=p_peer->tx_msg;
            break;
        }
        case BGP_PEER_LASTERRORCODERECV:
        {
            u_short error_code=p_peer->last_errcode;
            error_code =  (error_code << 8) + p_peer->last_subcode;
            p_str->len=2;
            memcpy(p_str->pucBuf,&error_code,p_str->len);
            break;
        }
        case BGP_PEER_FSMESTABLISHEDTRANSITIONS   :
        {
            *p_lval=p_peer->established_transitions;
            break;
        }
        case BGP_PEER_FSMESTABLISHEDTIME:
        {
            if(p_peer->state!=BGP4_NS_ESTABLISH)
                *p_lval=0;
            else
                *p_lval=time(NULL) -p_peer->uptime;
            break;
        }
        case BGP_PEER_CONNECTRETRYINTERVAL    :
        {
            *p_lval=p_peer->retry_interval;
            break;
        }
        case BGP_PEER_HOLDTIME:
        {
            *p_lval=p_peer->neg_hold_interval;
            break;
        }
        case BGP_PEER_KEEPALIVE   :
        {
            *p_lval=p_peer->neg_keep_interval;
            break;
        }
        case BGP_PEER_HOLDTIMECONFIGURED    :
        {
            *p_lval=p_peer->hold_interval;
            break;
        }
        case BGP_PEER_KEEPALIVECONFIGURED    :
        {
            *p_lval=p_peer->keep_interval;
            break;
        }
        case BGP_PEER_MINASORIGINTERVAL:
        {
            *p_lval=p_peer->origin_interval;
            break;
        }
        case BGP_PEER_MINROUTEADVERINTERVAL:
        {
            *p_lval=p_peer->adv_interval;
            break;
        }
        case BGP_PEER_UPDATESELAPSEDTIME:
        {
            if(p_peer->state!=BGP4_NS_ESTABLISH)
                *p_lval=0;
            else
                *p_lval=time(NULL) -p_peer->rx_updatetime;
            break;
        }
        case BGP_PEER_PASSWORDSUPPORT  :
        {
            *p_lval = p_peer->md5_support;
            break;
        }
        case BGP_PEER_PASSWORD:
        {
            if (p_peer->md5key_len > 0)
            {
                p_str->len = p_peer->md5key_len;
                memcpy(p_str->pucBuf, p_peer->md5key, p_peer->md5key_len);
            }
            break;
        }
        case BGP_PEER_RTREFLECTORSTATE   :
        {
            /*
            EbgpPeerRtReflectorState_reflector = 1,
            EbgpPeerRtReflectorState_client = 2,
            EbgpPeerRtReflectorState_non_client = 3,
            EbgpPeerRtReflectorState_ebgp = 4,
            */
            *p_lval=p_peer->rr_client;
            break;
        }
        case BGP_PEER_ATTACHEDVRF :
        {
            *p_lval=0;
            break;
        }
        case BGP_PEER_SENDCONNUNITY   :
        {
            /*
            * EbgpPeerSendCommunity_no_send 0
            * EbgpPeerSendCommunity_send 1
            */
            *p_lval=p_peer->send_community;
            break;
        }

        case BGP_PEER_RAWSTATUS :
        {
            *p_lval = p_peer->row_status;
            break;
        }
        case BGP_PEER_UPDATESOURCE :
        {
            p_str->len = p_peer->update_source.prefixlen/8;
            memcpy(p_str->pucBuf, p_peer->update_source.ip, p_str->len);
            break;
        }

        case BGP_PEER_LOCALAFIFLAG    :
        {
            *p_lval = p_peer->local.af;
            break;
        }

        case BGP_PEER_REMOTEAFIFLAG:
        {
            *p_lval = p_peer->remote.af;
            break;
        }

        case BGP_PEER_LOCALREFRESH:
        {
            *p_lval=p_peer->local.refresh;
            break;
        }
        case BGP_PEER_REMOTEREFRESH :
        {
            *p_lval=p_peer->remote.refresh;
            break;
        }
        case BGP_PEER_EBGPMTHOP:
        {
            *p_lval=p_peer->ebgp_mhop;
            break;
        }

        case BGP_PEER_VALIDTTLHOP:
        {
            *p_lval=p_peer->ttl_hops;
            break;
        }

        case BGP_PEER_GRREMOTESUPPORT  :
        {
            *p_lval=p_peer->remote.reset_enable;
            break;
        }
        case BGP_PEER_GRREMOTERESETTIME   :
        {
            *p_lval = p_peer->remote.reset_time;
            break;
        }
        case BGP_PEER_GRREMOTERESETAF  :
        {
            *p_lval = p_peer->remote.reset_af;
            break;

        }
        case BGP_PEER_GRREMOTERESETBIT  :
        {
            *p_lval=p_peer->remote.reset_bit;
            break;
        }
        case BGP_PEER_GRLOCALROLE    :
        {
            *p_lval=p_peer->reset_role;
            break;
        }
        case BGP_PEER_CEASEREASON:
        {
            *p_lval=p_peer->cease_reason;
            break;
        }

        case BGP_PEER_CEASERESTART:
        {
            if(p_peer->cease_reason ==4)
                *p_lval=p_peer->cease_reason;
            else
                *p_lval=0;
            break;
        }

        case BGP_PEER_CEASEDOWN:
        {
            if(p_peer->cease_reason ==2)
                *p_lval=p_peer->cease_reason;
            else
                *p_lval=0;
            break;
        }

        case BGP_PEER_CEASEMAXPREFIX:
        {
            if(p_peer->cease_reason ==1)
                *p_lval=p_peer->cease_reason;
            else
                *p_lval=0;
            break;
        }

        case BGP_PEER_CEASERESLACK:
        {
            if(p_peer->cease_reason ==8)
                *p_lval=p_peer->cease_reason;
            else
                *p_lval=0;
            break;
        }

        case BGP_PEER_CEASEREJECTCONN:
        {
            if(p_peer->cease_reason ==5)
                *p_lval=p_peer->cease_reason;
            else
                *p_lval=0;
            break;
        }

        case BGP_PEER_CEASEEBGP:
        {
            if(p_peer->cease_reason ==6)
                *p_lval=p_peer->cease_reason;
            else
                *p_lval=0;
            break;
        }

        case BGP_PEER_UNCONFIG:
        {
            if(p_peer->cease_reason ==3)
                *p_lval=p_peer->cease_reason;
            else
                *p_lval=0;
            break;
        }


        case BGP_PEER_SENDLABEL:
        {
            *p_lval = p_peer->send_label;
            break;
        }

        case BGP_PEER_RTNUM:
        {
            *p_lval=bgp4_get_number_of_route(M2_ipRouteProto_bgp, p_peer);
            break;
        }
        case BGP_PEER_NEXTHOPSELF:
        {
            *p_lval = p_peer->nexthop_self;
            break;
        }
        case BGP_PEER_BFD_ENABLE:
        {
            *p_lval = p_peer->bfd_enable;;
            break;
        }
        case BGP_PEER_PUBASONLY:
        {
            *p_lval = p_peer->public_as_only;
            break;
        }
        case BGP_PEER_SUBASENABLE:
        {
            *p_lval = p_peer->as_substitute_enable;
            break;
        }
        case BGP_PEER_ALLOWLOOPTIMES:
        {
            *p_lval = p_peer->allow_as_loop_times;
            break;
        }
        case BGP_PEER_FAKEAS:
        {
            *p_lval = p_peer->fake_as;
            break;
        }



    }

    bgp_sem_give();

    return( rc );
}

STATUS _bgpPeerSetApi(tBgpPeerIndex *p_index,u_int cmd,void *var,u_int flag)
{
    STATUS  rc = VOS_OK;
    u_int sync_cmd = 0;
    tBGP4_ADDR peer_ip;
    u_int *p_lval = (u_int*) var;
    octetstring *p_str = (octetstring *)var;
    tBGP4_PEER *p_peer=NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    /*sync config to others*/
    if(cmd == BGP_PEER_PASSWORD || cmd == BGP_PEER_VPNNAME)
    {
        flag |= USP_SYNC_OCTETDATA;
    }
    sync_cmd = HW_BGP4PEER_CMDSTART+cmd;
    if (sync_cmd)
    {
        rc = uspHwBgp4PeerSync(p_index, sync_cmd, (void * )var, flag);
        if(rc != VOS_OK)
        {
            printf("\r\nsync error,config failed");
            return rc;
        }
    }

    memset(&peer_ip, 0, sizeof(peer_ip));
    peer_ip.prefixlen = (p_index->addr_type == AF_INET) ? 32 :128;
    peer_ip.afi = (p_index->addr_type==AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;
    memcpy(&peer_ip.ip, p_index->addr, 16);

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(p_index->vrf_id);
    if(p_instance == NULL)
    {
        if(p_index->vrf_id == 0 ||cmd != BGP_PEER_RAWSTATUS)/*vpn instance need to be created*/
        {
            bgp_sem_give();
            return VOS_ERR;
        }
    }


    p_peer = bgp4_peer_lookup(p_instance,&peer_ip);
    if (p_peer == NULL && cmd != BGP_PEER_RAWSTATUS)
    {
        bgp_sem_give();
        return VOS_ERR;
    }
    switch(cmd)
    {
        default:
            rc = VOS_ERR;
            break;

        case BGP_PEER_ADMINSTATUS:
        {
            if (*p_lval == BGP4_PEER_RUNNING)
            {
                p_peer->admin=BGP4_PEER_RUNNING;
                bgp4_fsm(p_peer, BGP4_EVENT_START);
            }
            else
            {
                p_peer->start_interval = BGP4_DFLT_STARTINTERVAL;
                p_peer->cease_reason = 2;
                bgp4_fsm(p_peer, BGP4_EVENT_STOP);
                p_peer->admin=BGP4_PEER_HALTED;
            }
            break;
        }
        case BGP_PEER_REMOTEAS  :
        {
            if (p_peer->admin == BGP4_PEER_RUNNING)
            {
                rc=VOS_ERR;
            }
            else
            {
                p_peer->remote.as = *p_lval;
                if (bgp4_peer_type(p_peer) == BGP4_IBGP)
                {
                    p_peer->adv_interval = BGP4_DFLT_IBGP_MINROUTEADVINTERVAL;
                }
            }
            break;
        }
        case BGP_PEER_CONNECTRETRYINTERVAL    :
        {
            p_peer->retry_interval = *p_lval;
            break;
        }
        case BGP_PEER_HOLDTIMECONFIGURED    :
        {
            p_peer->hold_interval = *p_lval;
            break;
        }
        case BGP_PEER_KEEPALIVECONFIGURED    :
        {
            p_peer->keep_interval = *p_lval;
            break;
        }
        case BGP_PEER_MINASORIGINTERVAL:
        {
            p_peer->origin_interval= *p_lval;
            break;
        }
        case BGP_PEER_MINROUTEADVERINTERVAL:
        {
            p_peer->adv_interval = *p_lval;
            break;
        }
        case BGP_PEER_PASSWORDSUPPORT  :
        {
            p_peer->md5_support= *p_lval;
            break;
        }
        case BGP_PEER_PASSWORD:
        {
            if (p_str->len== 0)
            {
                p_peer->md5key_len = 0;
                break;
            }
            if (p_str->len<= 80)
            {
                memcpy(p_peer->md5key, p_str->pucBuf, p_str->len);
                p_peer->md5key_len = p_str->len;
            }
            else
                rc = VOS_ERR;

            break;
        }
        case BGP_PEER_LOCALAFIFLAG:
        {
            bgp4_set_peer_af(p_peer,*p_lval);
            break;
        }
        /*
        *EbgpPeerRtReflectorState_reflector = 1,
        *EbgpPeerRtReflectorState_client = 2,
        *EbgpPeerRtReflectorState_non_client = 3,
        *EbgpPeerRtReflectorState_ebgp = 4
        */
        case BGP_PEER_RTREFLECTORSTATE   :
        {
            p_peer->rr_client =((u_int)*p_lval);
            break;
        }
        /*
        * EbgpPeerSendCommunity_no_send 0
        * EbgpPeerSendCommunity_send 1
        */
        case BGP_PEER_SENDCONNUNITY   :
        {
            p_peer->send_community = ((u_int)*p_lval);
            break;
        }

        case BGP_PEER_RAWSTATUS :
        {
            switch (*p_lval)
            {
                case SNMP_CREATEANDGO:
                case SNMP_CREATEANDWAIT:
                {
                    if (p_peer == NULL)
                    {
                        if(p_instance == NULL)/*create vpn instance first*/
                        {
                            p_instance = bgp4_vpn_instance_create(p_index->vrf_id);
                        }
                        p_peer = bgp4_add_peer(p_instance,&peer_ip);
                        if(p_peer == NULL)
                        {
                            rc = VOS_ERR;
                            break;
                        }
                    }
                    p_peer ->row_status = *p_lval;
                    break;
                }
                case SNMP_ACTIVE:
                case SNMP_NOTINSERVICE:
                case SNMP_NOTREADY:
                {
                    if(p_peer)
                    {
                        p_peer ->row_status = *p_lval;
                    }
                    else
                    {
                        rc = VOS_ERR;
                    }
                    break;
                }
                case SNMP_DESTROY:
                {
                    if(p_peer)
                    {
                        p_peer->cease_reason = 3;
                        bgp4_fsm(p_peer, BGP4_EVENT_STOP);
                        p_peer->admin=BGP4_PEER_HALTED;
                        bgp4_delete_peer ( p_peer );
                    }
                    else
                    {
                        rc = VOS_ERR;
                    }
                    break;
                }
            }

            break;
        }
        case BGP_PEER_UPDATESOURCE :
        {
            if(p_str->len == 4)
            {
                p_peer->update_source.afi = BGP4_PF_IPUCAST;
                p_peer->update_source.prefixlen = 32;
                memcpy(p_peer->update_source.ip,p_str->pucBuf,4);
            }
            else if(p_str->len == 16)
            {
                p_peer->update_source.afi = BGP4_PF_IP6UCAST;
                p_peer->update_source.prefixlen = 128;
                memcpy(p_peer->update_source.ip,p_str->pucBuf,16);
            }
            else if(p_str->len == 0)/*undo cmd*/
            {
                memset(&p_peer->update_source,0,sizeof(tBGP4_ADDR));
            }
            else
            {
                rc = VOS_ERR;
            }
            break;
        }
        case BGP_PEER_LOCALREFRESH  :
        {
            p_peer->local.refresh = *p_lval;
            if(p_peer->local.refresh == 1)
                af_set(p_peer->local.capability, BGP4_RTREFRESH);
            else
                af_clear(p_peer->local.capability, BGP4_RTREFRESH);
            break;
        }
        case BGP_PEER_RTREFRESH:/*send route refresh msg to peer*/
        {
            bgp4_send_refresh(p_peer,*p_lval);
            break;
        }
        case BGP_PEER_EBGPMTHOP:
        {
            p_peer->ebgp_mhop = *p_lval;
            break;
        }
        case BGP_PEER_VALIDTTLHOP:
        {
            p_peer->ttl_hops=*p_lval;
                if(p_peer->sock>0&&p_peer->ttl_hops>0)
                {
                    if(p_peer->ttl_hops<256)
                    {
                        bgp4_tcp_set_peer_ttl(p_peer->sock,p_peer);
                    }
                }
            break;
        }
        case BGP_PEER_SENDLABEL:
        {
            p_peer->send_label= *p_lval;
            break;
        }
        case BGP_PEER_CEASERESTART:
        {
            p_peer->cease_reason = 4;
            break;
        }
        case BGP_PEER_CEASEDOWN:
        {
            p_peer->cease_reason = 2;
            break;
        }
        case BGP_PEER_CEASEMAXPREFIX:
        {
            p_peer->cease_reason = 1;
            break;
        }
        case BGP_PEER_CEASERESLACK:
        {
            p_peer->cease_reason = 8;
            break;
        }
        case BGP_PEER_CEASEREJECTCONN:
        {
            p_peer->cease_reason = 5;
            break;
        }
        case BGP_PEER_CEASEEBGP:
        {
            p_peer->cease_reason=6;
            break;
        }
        case BGP_PEER_UNCONFIG:
        {
            p_peer->cease_reason=3;
            break;
        }
        case BGP_PEER_NEXTHOPSELF:
        {
            p_peer->nexthop_self = *p_lval;
            break;
        }
        case BGP_PEER_BFD_ENABLE:
        {
                    p_peer->bfd_enable = *p_lval;
                if((p_peer->state == BGP4_NS_ESTABLISH)
                &&(p_peer->bfd_enable))
                {
                        bgp4_bind_bfd(p_peer);
                }
            else if(!(p_peer ->bfd_enable))
            {
                bgp4_unbind_bfd(p_peer);
            }
                    break;
            }
        case BGP_PEER_PUBASONLY:
        {
            p_peer->public_as_only = *p_lval;
            break;
        }
        case BGP_PEER_SUBASENABLE:
        {
            p_peer->as_substitute_enable = *p_lval;
            break;
        }
        case BGP_PEER_ALLOWLOOPTIMES:
        {
            p_peer->allow_as_loop_times = *p_lval;
            break;
        }
        case BGP_PEER_FAKEAS:
        {
            p_peer->fake_as = *p_lval;
            break;
        }

    }

    bgp_sem_give();

    return( rc );
}

STATUS bgpPeerSetApi(tBgpPeerIndex *p_index,u_int cmd,void *var)
{
    return  _bgpPeerSetApi(p_index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}
STATUS bgpPeerSyncApi(tBgpPeerIndex *p_index,u_int cmd,void *var)
{
    return  _bgpPeerSetApi(p_index, cmd, var,USP_SYNC_LOCAL);
}

STATUS bgpRouteGetIPV4First(tBgpRtIndex* p_index)
{
    tBGP4_ROUTE* p_route = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(p_index->vrf_id);

    if(p_instance)
    {
        p_route = bgp4_rib_first(&p_instance->rib);

        while (p_route)
        {
            p_index->proto=p_route->proto;
            p_index->dest.prefixlen=p_route->dest.prefixlen;

            if(p_route->dest.afi==BGP4_PF_IPUCAST &&
                p_route->proto==M2_ipRouteProto_bgp)
            {
                p_index->dest.afi=AF_INET;
                memcpy(p_index->dest.ip,p_route->dest.ip,4);
                p_index->peer.addr_type=AF_INET;
                memcpy(p_index->peer.addr,p_route->p_path->p_peer->remote.ip.ip,4);
                bgp_sem_give();
                return VOS_OK;
            }
            else
            {
                p_route = bgp4_rib_next(&p_instance->rib,p_route);
                continue;
            }


        }
    }

    bgp_sem_give();
    return VOS_ERR;
}

STATUS bgpRouteGetIPV4Next(tBgpRtIndex* p_index,tBgpRtIndex* p_nextindex)
{
    tBGP4_ROUTE* p_rt=NULL;
    tBGP4_ROUTE* p_next_rt=NULL;
    tBGP4_ROUTE cur_rt;
    tBGP4_PATH      cur_path;
    tBGP4_PEER      cur_peer;
    tBGP4_VPN_INSTANCE* p_instance = NULL;


    memset(&cur_rt,0,sizeof(tBGP4_ROUTE));
    memset(&cur_path,0,sizeof(tBGP4_PATH));
    memset(&cur_peer,0,sizeof(tBGP4_PEER));

    /*construct virtul route*/
    cur_rt.dest.prefixlen=p_index->dest.prefixlen;

    if(p_index->dest.afi==AF_INET)
    {
        cur_rt.dest.afi=BGP4_PF_IPUCAST;
        memcpy(cur_rt.dest.ip,p_index->dest.ip,4);
    }
    else if(p_index->dest.afi==AF_INET6)
    {
        cur_rt.dest.afi=BGP4_PF_IP6UCAST;
        memcpy(cur_rt.dest.ip,p_index->dest.ip,16);
    }

    cur_rt.proto=p_index->proto;
    cur_rt.p_path=&cur_path;

    if(cur_rt.proto==M2_ipRouteProto_bgp)
    {

        if(p_index->dest.afi == AF_INET)
        {
            cur_peer.remote.ip.afi=BGP4_PF_IPUCAST;
            memcpy(cur_peer.remote.ip.ip,p_index->peer.addr,4);
            cur_peer.remote.ip.prefixlen = 32;
        }
        else if(p_index->dest.afi == AF_INET6)
        {
            cur_peer.remote.ip.afi=BGP4_PF_IP6UCAST;
            memcpy(cur_peer.remote.ip.ip,p_index->peer.addr,16);
            cur_peer.remote.ip.prefixlen = 128;
        }
        cur_path.p_peer=&cur_peer;
    }
    else
        cur_path.p_peer=NULL;

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(p_index->vrf_id);

    if(p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    p_rt=bgp4_rib_lookup(&p_instance->rib, &cur_rt);

    if(p_rt==NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    p_next_rt=bgp4_rib_next(&p_instance->rib,p_rt);
    while (p_next_rt)
    {
        p_nextindex->proto=p_next_rt->proto;

        p_nextindex->dest.prefixlen=p_next_rt->dest.prefixlen;

        if(p_next_rt->dest.afi==BGP4_PF_IPUCAST
            && p_next_rt->proto == M2_ipRouteProto_bgp)
        {
            p_nextindex->dest.afi=AF_INET;
            memcpy(p_nextindex->dest.ip, p_next_rt->dest.ip,4);
            memcpy(p_nextindex->peer.addr,p_next_rt->p_path->p_peer->remote.ip.ip ,4);
            p_nextindex->peer.addr_type=AF_INET;
            bgp_sem_give();
            return VOS_OK;
        }
        else
        {
            p_next_rt = bgp4_rib_next(&p_instance->rib,p_next_rt);
            continue;
        }

    }
    bgp_sem_give();
    return VOS_ERR;
}

STATUS bgpRouteGetFirst(tBgpRtIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ROUTE* p_route;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(p_index->vrf_id);

    if(p_instance)
    {
        p_route = bgp4_rib_first(&p_instance->rib);

        if (p_route &&
            (p_route->dest.afi == BGP4_PF_IPUCAST ||
                    p_route->dest.afi == BGP4_PF_IP6UCAST))
        {
            p_index->proto=p_route->proto;
            p_index->dest.prefixlen=p_route->dest.prefixlen;
            p_index->dest.afi = (p_route->dest.afi==BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
            memcpy(p_index->dest.ip,p_route->dest.ip,16);

            if(p_route->proto == M2_ipRouteProto_bgp)
            {
                if (p_route->p_path->p_peer)
                {
                    p_index->peer.addr_type = (p_route->p_path->p_peer->remote.ip.afi==BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
                    memcpy(p_index->peer.addr,p_route->p_path->p_peer->remote.ip.ip,16);
                    p_index->peer.vrf_id = p_route->p_path->p_peer->p_instance->instance_id;
                }
            }
            else
            {
                memset(&p_index->peer,0,sizeof(tBgpPeerIndex));
                if(p_route->dest.afi == BGP4_PF_IPUCAST)
                {
                    memcpy(p_index->nexthop,p_route->p_path->nexthop.ip,16);
                    p_index->peer.addr_type = AF_INET;
                }
                else
                {
                    memcpy(p_index->nexthop,p_route->p_path->nexthop_global.ip,16);
                    p_index->peer.addr_type = AF_INET6;
                }

            }

            p_index->vrf_id = p_instance->instance_id;
            rc =  VOS_OK;
        }

    }

    bgp_sem_give();

    return rc;
}

STATUS bgpRouteGetNext(tBgpRtIndex* p_index,tBgpRtIndex* p_nextindex)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ROUTE* p_rt=NULL;
    tBGP4_ROUTE* p_next_rt=NULL;
    tBGP4_ROUTE cur_rt;
    tBGP4_PATH      cur_path;
    tBGP4_PEER      cur_peer;
    STATUS rc = VOS_ERR;

    memset(&cur_rt,0,sizeof(tBGP4_ROUTE));
    memset(&cur_path,0,sizeof(tBGP4_PATH));
    memset(&cur_peer,0,sizeof(tBGP4_PEER));

    /*construct virtul route*/
    cur_rt.dest.prefixlen=p_index->dest.prefixlen;

    if(p_index->dest.afi==AF_INET)
    {
        cur_rt.dest.afi=BGP4_PF_IPUCAST;
        memcpy(cur_rt.dest.ip,p_index->dest.ip,4);
    }
    else if(p_index->dest.afi==AF_INET6)
    {
        cur_rt.dest.afi=BGP4_PF_IP6UCAST;
        memcpy(cur_rt.dest.ip,p_index->dest.ip,16);
    }

    cur_rt.proto=p_index->proto;
    cur_rt.p_path=&cur_path;

    if(cur_rt.proto==M2_ipRouteProto_bgp)
    {

        if(p_index->peer.addr_type == AF_INET)
        {
            cur_peer.remote.ip.afi=BGP4_PF_IPUCAST;
            memcpy(cur_peer.remote.ip.ip,p_index->peer.addr,4);
            cur_peer.remote.ip.prefixlen = 32;
        }
        else if(p_index->peer.addr_type==AF_INET6)
        {
            cur_peer.remote.ip.afi=BGP4_PF_IP6UCAST;
            memcpy(cur_peer.remote.ip.ip,p_index->peer.addr,16);
            cur_peer.remote.ip.prefixlen = 128;
        }
        cur_path.p_peer=&cur_peer;
    }
    else
    {
        cur_path.p_peer=NULL;
        if(p_index->dest.afi == AF_INET)
        {
            cur_path.nexthop.afi=BGP4_PF_IPUCAST;
            memcpy(cur_path.nexthop.ip,p_index->nexthop,4);
            cur_path.nexthop.prefixlen = 32;
        }
        else if(p_index->dest.afi == AF_INET6)
        {
            cur_path.nexthop_global.afi=BGP4_PF_IP6UCAST;
            memcpy(cur_path.nexthop_global.ip,p_index->nexthop,16);
            cur_path.nexthop_global.prefixlen = 128;
        }
    }

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(p_index->vrf_id);

    if(p_instance)
    {
        p_rt = bgp4_rib_lookup(&p_instance->rib, &cur_rt);

        if(p_rt)
        {
            p_next_rt=bgp4_rib_next(&p_instance->rib,p_rt);
            if(p_next_rt &&
                (p_next_rt->dest.afi == BGP4_PF_IPUCAST ||
                    p_next_rt->dest.afi == BGP4_PF_IP6UCAST))
            {
                p_nextindex->vrf_id = p_instance->instance_id;
                p_nextindex->proto=p_next_rt->proto;
                p_nextindex->dest.afi = (p_next_rt->dest.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
                memcpy(p_nextindex->dest.ip,p_next_rt->dest.ip,16);
                p_nextindex->dest.prefixlen = p_next_rt->dest.prefixlen;

                if(p_next_rt->proto==M2_ipRouteProto_bgp)
                {
                    p_nextindex->peer.addr_type = (p_next_rt->p_path->p_peer->remote.ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
                    memcpy(p_nextindex->peer.addr,p_next_rt->p_path->p_peer->remote.ip.ip,16);
                    p_nextindex->peer.vrf_id = p_next_rt->p_path->p_peer->p_instance->instance_id;
                }
                else
                {
                    memset(&p_nextindex->peer,0,sizeof(tBgpPeerIndex));
                    if(p_next_rt->dest.afi == BGP4_PF_IPUCAST)
                    {
                        memcpy(p_nextindex->nexthop,p_next_rt->p_path->nexthop.ip,16);
                        p_nextindex->peer.addr_type = AF_INET;
                    }
                    else
                    {
                        memcpy(p_nextindex->nexthop,p_next_rt->p_path->nexthop_global.ip,16);
                        p_nextindex->peer.addr_type = AF_INET6;
                    }
                }
                rc = VOS_OK;
            }
        }

    }
    bgp_sem_give();
    return rc;
}

STATUS bgpRouteGetApi(tBgpRtIndex *p_index,u_int cmd,void *var)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int* p_lval=(u_int*)var;
    octetstring *p_str = (octetstring *)var;
    STATUS  rc = VOS_OK;

    char value_c[512];
    tBGP4_ROUTE* p_route;
    tBGP4_ROUTE cur_rt;
    tBGP4_PATH      cur_path;
    tBGP4_PEER      cur_peer;

    memset(&cur_rt,0,sizeof(tBGP4_ROUTE));
    memset(&cur_path,0,sizeof(tBGP4_PATH));
    memset(&cur_peer,0,sizeof(tBGP4_PEER));

    /*construct virtul route*/
    cur_rt.dest.prefixlen=p_index->dest.prefixlen;

    if(p_index->dest.afi==AF_INET)
    {
        cur_rt.dest.afi=BGP4_PF_IPUCAST;
        memcpy(cur_rt.dest.ip,p_index->dest.ip,4);
    }
    else if(p_index->dest.afi==AF_INET6)
    {
        cur_rt.dest.afi=BGP4_PF_IP6UCAST;
        memcpy(cur_rt.dest.ip,p_index->dest.ip,16);
    }

    cur_rt.proto=p_index->proto;

    cur_rt.p_path=&cur_path;


    if(cur_rt.proto==M2_ipRouteProto_bgp)
    {
        memcpy(&cur_peer.remote.ip,&p_index->peer,sizeof(tBgpIpIndex));

        if(p_index->peer.addr_type == AF_INET)
        {
            cur_peer.remote.ip.afi=BGP4_PF_IPUCAST;
            cur_peer.remote.ip.prefixlen=32;
            memcpy(cur_peer.remote.ip.ip,p_index->peer.addr,4);
        }
        else if(p_index->peer.addr_type == AF_INET6)
        {
            cur_peer.remote.ip.afi=BGP4_PF_IP6UCAST;
            cur_peer.remote.ip.prefixlen=128;
            memcpy(cur_peer.remote.ip.ip,p_index->peer.addr,16);
        }
        cur_path.p_peer=&cur_peer;
    }
    else
    {
        cur_path.p_peer=NULL;

        if(p_index->dest.afi == AF_INET)
        {
            cur_path.nexthop.afi=BGP4_PF_IPUCAST;
            memcpy(cur_path.nexthop.ip,p_index->nexthop,4);
            cur_path.nexthop.prefixlen = 32;
        }
        else if(p_index->dest.afi == AF_INET6)
        {
            cur_path.nexthop_global.afi=BGP4_PF_IP6UCAST;
            memcpy(cur_path.nexthop_global.ip,p_index->nexthop,16);
            cur_path.nexthop_global.prefixlen = 128;
        }
    }


    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(p_index->vrf_id);/*if no peer,instance id is also given*/
    if (p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    p_route = bgp4_rib_lookup(&p_instance->rib,&cur_rt);

    if (p_route == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    switch(cmd)
    {
        default:
            rc = VOS_ERR;
            break;

        case BGP_ROUTE_NEXTHOP:
        {
            if(p_route->dest.afi == BGP4_PF_IPUCAST)
            {
                memcpy(p_str->pucBuf,p_route->p_path->nexthop.ip,4);
                p_str->len=4;

            }
            else if(p_route->dest.afi == BGP4_PF_IP6UCAST)
            {

                memcpy(p_str->pucBuf,&p_route->p_path->nexthop_global.ip,16);
                p_str->len=16;
            }
            break;
        }
        case BGP_ROUTE_MED:
        {
            if(p_route->p_path!=NULL)
            {
                *p_lval=p_route->p_path->out_med;
            }
            else
                *p_lval=0;
            break;
        }
        case BGP_ROUTE_LOCALPREF:
        {
            if(p_route->p_path!=NULL)
            {
                *p_lval=p_route->p_path->rcvd_localpref;
            }
            else
            *p_lval=0;

            break;
        }
        case BGP_ROUTE_ASSQUENCE:
        {
            if (p_route->p_path!=NULL)
            {
       
                 tBGP4_ASPATH *p_aspath = NULL;
                 u_char *p_asseq = value_c;                 
                 u_short pathlen = 0;
                 
                 /*construct aspath*/
                 LST_LOOP(&p_route->p_path->aspath_list, p_aspath, node, tBGP4_ASPATH)
                 {
                     /*type*/
                     *p_asseq = p_aspath->type;
                      p_asseq++;
                      pathlen++;

                     /*count*/
                     *p_asseq = p_aspath->len;
                      p_asseq++;
                      pathlen++;

                     /*as copy*/
                      memcpy(p_asseq, p_aspath->p_asseg, p_aspath->len*2);
                      p_asseq += p_aspath->len*2;
                      pathlen += p_aspath->len*2;
                 }
                 
                 /*if there is no as-path exist,only return hdr*/
                 if (pathlen == 0)
                 {
                     p_str->len = 2;
                     p_str->pucBuf[0] = BGP_ASPATH_SEQ;
                     p_str->pucBuf[1] = 0;
                     break;
                 }
                 if (p_str->len > pathlen)
                 {
                     p_str->len = pathlen;
                 }
                 p_str->len = p_str->len & 0x000000ff;
                memcpy(p_str->pucBuf, value_c, p_str->len);
            }

            break;
        }
        case BGP_ROUTE_ORIGIN:
        {
            if(p_route->p_path!=NULL)
            {
                *p_lval=p_route->p_path->origin;
            }
            else
                *p_lval=0;

            break;
        }
        case BGP_ROUTE_ATOMICAGGR:
        {
            if(p_route->p_path!=NULL)
            {
                *p_lval=p_route->p_path->flags.atomic;
            }
            else
                *p_lval=0;
            break;
        }
        case BGP_ROUTE_AGGREGATORADDR:
        {
            if(p_route->p_path!=NULL)
            {
                memcpy(p_str->pucBuf,p_route->p_path->aggregator.addr.ip,16);
                p_str->len=16;
            }
            break;
        }
        case BGP_ROUTE_CALCLOCALPREF:
        {
            if(p_route->p_path!=NULL)
            {
                *p_lval=p_route->p_path->out_localpref;
            }
            break;
        }
        case BGP_ROUTE_BEST:
        {
            tBGP4_ROUTE* p_best;
            if(p_route->p_path!=NULL)
            {
                p_best=bgp4_best_route(&p_instance->rib,&p_route->dest);
                if(p_best == p_route)
                    *p_lval=2;
                else
                    *p_lval=1;
            }
            else
            {
                *p_lval=1;
            }
            break;
        }
        case BGP_ROUTE_ATTRUNKNOWN:
        {
            if(p_route->p_path!=NULL)
            {
                /*max length is 255*/
                p_str->len = p_route->p_path->unknown_len & 0x000000ff;
                memcpy(p_str->pucBuf,p_route->p_path->p_unkown,p_str->len);
            }
            break;
        }
        case BGP_ROUTE_RD:
        {
            bgp4_translate_vpn_RD(p_route,p_str);
            break;
        }
        case BGP_ROUTE_INLABEL:
        {
            if(p_route->route_direction == MPLSL3VPN_ROUTE_LOCAL)
            {
                *p_lval = p_route->vpn_label;
            }
            break;
        }
        case BGP_ROUTE_OUTLABEL:
        {
            if(p_route->route_direction == MPLSL3VPN_ROUTE_REMOTE)
            {
                *p_lval = p_route->vpn_label;
            }
            break;
        }
#if 0        
        case BGP_ROUTE_COMMUNITY:
             p_str->len = 0;
             break;
             
        case BGP_ROUTE_EXTCOMMUNITY:
             p_str->len = 0;
             break;
             
        case BGP_ROUTE_CLUSTERID:
             p_str->len = 0;
             break;
#endif
    }

    bgp_sem_give();

    return( rc );
}

/*vpnv4/vpnv6 routes*/
STATUS bgpVpnRouteGetFirst(tBgpVpnRtIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ROUTE* p_route = NULL;
    tBGP4_ROUTE* p_next = NULL;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(0);

    
    if(p_instance)
    {
        RIB_LOOP(&p_instance->rib, p_route, p_next)
        {
            if (p_route &&
                (p_route->dest.afi == BGP4_PF_IPVPN||
                    p_route->dest.afi == BGP4_PF_IP6VPN))
            {
                p_index->proto=p_route->proto;
                p_index->dest.prefixlen=p_route->dest.prefixlen-64;
                p_index->dest.afi = (p_route->dest.afi == BGP4_PF_IPVPN) ? AF_INET : AF_INET6;
                memcpy(p_index->vpn_rd,p_route->dest.ip,8);
                memcpy(p_index->dest.ip,p_route->dest.ip+8,16);

                if(p_route->proto==M2_ipRouteProto_bgp && (p_route->p_path->p_peer))
                {
                    p_index->peer.addr_type = (p_route->p_path->p_peer->remote.ip.afi==BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
                    memcpy(p_index->peer.addr,p_route->p_path->p_peer->remote.ip.ip,16);
                    p_index->peer.vrf_id = p_route->p_path->p_peer->p_instance->instance_id;
                }
                else
                {
                    memset(&p_index->peer,0,sizeof(tBgpPeerIndex));
                }

                p_index->vrf_id = p_instance->instance_id;
                rc =  VOS_OK;
                break;
            }
        }
    }

    bgp_sem_give();

    return rc;
}
/*vpnv4/vpnv6 routes*/
STATUS bgpVpnRouteGetNext(tBgpVpnRtIndex* p_index,tBgpVpnRtIndex* p_nextindex)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ROUTE* p_rt=NULL;
    tBGP4_ROUTE* p_next_rt=NULL;
    tBGP4_ROUTE cur_rt;
    tBGP4_PATH      cur_path;
    tBGP4_PEER      cur_peer;
    STATUS rc = VOS_ERR;

    memset(&cur_rt,0,sizeof(tBGP4_ROUTE));
    memset(&cur_path,0,sizeof(tBGP4_PATH));
    memset(&cur_peer,0,sizeof(tBGP4_PEER));

    /*construct virtul route*/
    cur_rt.dest.prefixlen=p_index->dest.prefixlen+64;

    if(p_index->dest.afi == AF_INET)
    {
        cur_rt.dest.afi=BGP4_PF_IPVPN;
        memcpy(cur_rt.dest.ip,p_index->vpn_rd,8);
        memcpy(cur_rt.dest.ip+8,p_index->dest.ip,4);
    }
    else if(p_index->dest.afi == AF_INET6)
    {
        cur_rt.dest.afi=BGP4_PF_IP6VPN;
        memcpy(cur_rt.dest.ip,p_index->vpn_rd,8);
        memcpy(cur_rt.dest.ip+8,p_index->dest.ip,16);
    }

    cur_rt.proto=p_index->proto;
    cur_rt.p_path=&cur_path;

    if(cur_rt.proto==M2_ipRouteProto_bgp)
    {

        if(p_index->peer.addr_type == AF_INET)
        {
            cur_peer.remote.ip.afi=BGP4_PF_IPUCAST;
            memcpy(cur_peer.remote.ip.ip,p_index->peer.addr,4);
            cur_peer.remote.ip.prefixlen = 32;
        }
        else if(p_index->peer.addr_type==AF_INET6)
        {
            cur_peer.remote.ip.afi=BGP4_PF_IP6UCAST;
            memcpy(cur_peer.remote.ip.ip,p_index->peer.addr,16);
            cur_peer.remote.ip.prefixlen = 128;
        }
        cur_path.p_peer=&cur_peer;
    }
    else
        cur_path.p_peer=NULL;

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(0);

    if(p_instance)
    {
        p_rt = bgp4_rib_lookup(&p_instance->rib, &cur_rt);

        if(p_rt)
        {
            p_next_rt=bgp4_rib_next(&p_instance->rib,p_rt);
            if(p_next_rt &&
                (p_next_rt->dest.afi == BGP4_PF_IPVPN||
                    p_next_rt->dest.afi == BGP4_PF_IP6VPN))
            {
                p_nextindex->vrf_id = p_instance->instance_id;
                p_nextindex->proto=p_next_rt->proto;
                p_nextindex->dest.afi = (p_next_rt->dest.afi == BGP4_PF_IPVPN) ? AF_INET : AF_INET6;
                memcpy(p_nextindex->vpn_rd,p_next_rt->dest.ip,8);
                memcpy(p_nextindex->dest.ip,p_next_rt->dest.ip+8,16);
                p_nextindex->dest.prefixlen = p_next_rt->dest.prefixlen - 64;

                if(p_next_rt->proto==M2_ipRouteProto_bgp)
                {
                    p_nextindex->peer.addr_type = (p_next_rt->p_path->p_peer->remote.ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
                    memcpy(p_nextindex->peer.addr,p_next_rt->p_path->p_peer->remote.ip.ip,16);
                    p_nextindex->peer.vrf_id = p_next_rt->p_path->p_peer->p_instance->instance_id;
                }
                else
                {
                    memset(&p_nextindex->peer,0,sizeof(tBgpPeerIndex));
                }

                rc = VOS_OK;
            }
        }

    }
    bgp_sem_give();
    return rc;
}
/*vpnv4/vpnv6 routes*/
STATUS bgpVpnRouteGetApi(tBgpVpnRtIndex *p_index,u_int cmd,void *var)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int* p_lval=(u_int*)var;
    octetstring *p_str = (octetstring *)var;
    STATUS  rc = VOS_OK;

    char value_c[512];
    tBGP4_ROUTE* p_route;
    tBGP4_ROUTE cur_rt;
    tBGP4_PATH      cur_path;
    tBGP4_PEER      cur_peer;

    memset(&cur_rt,0,sizeof(tBGP4_ROUTE));
    memset(&cur_path,0,sizeof(tBGP4_PATH));
    memset(&cur_peer,0,sizeof(tBGP4_PEER));

    /*construct virtul route*/
    cur_rt.dest.prefixlen=p_index->dest.prefixlen + 64;

    if(p_index->dest.afi == AF_INET)
    {
        cur_rt.dest.afi=BGP4_PF_IPVPN;
        memcpy(cur_rt.dest.ip,p_index->vpn_rd,8);
        memcpy(cur_rt.dest.ip+8,p_index->dest.ip,4);
    }
    else if(p_index->dest.afi == AF_INET6)
    {
        cur_rt.dest.afi=BGP4_PF_IP6VPN;
        memcpy(cur_rt.dest.ip,p_index->vpn_rd,8);
        memcpy(cur_rt.dest.ip+8,p_index->dest.ip,16);
    }

    cur_rt.proto=p_index->proto;

    cur_rt.p_path=&cur_path;


    if(cur_rt.proto==M2_ipRouteProto_bgp)
    {
        memcpy(&cur_peer.remote.ip,&p_index->peer,sizeof(tBgpIpIndex));

        if(p_index->peer.addr_type == AF_INET)
        {
            cur_peer.remote.ip.afi=BGP4_PF_IPUCAST;
            cur_peer.remote.ip.prefixlen=32;
            memcpy(cur_peer.remote.ip.ip,p_index->peer.addr,4);
        }
        else if(p_index->peer.addr_type == AF_INET6)
        {
            cur_peer.remote.ip.afi=BGP4_PF_IP6UCAST;
            cur_peer.remote.ip.prefixlen=128;
            memcpy(cur_peer.remote.ip.ip,p_index->peer.addr,16);
        }
        cur_path.p_peer=&cur_peer;
    }
    else
        cur_path.p_peer=NULL;


    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(0);
    if (p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    p_route = bgp4_rib_lookup(&p_instance->rib,&cur_rt);

    if (p_route == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    switch(cmd)
    {
        default:
            rc = VOS_ERR;
            break;

        case BGP_ROUTE_NEXTHOP:
        {
            if(p_route->dest.afi == BGP4_PF_IPVPN ||
                p_route->dest.afi == BGP4_PF_IP6VPN)/*nexthop is always ipv4 with zero RD*/
            {
                memcpy(p_str->pucBuf,p_route->p_path->nexthop.ip,4);
                p_str->len=4;

            }
            break;
        }
        case BGP_ROUTE_MED:
        {
            if(p_route->p_path!=NULL)
            {
                *p_lval=p_route->p_path->out_med;
            }
            else
                *p_lval=0;
            break;
        }
        case BGP_ROUTE_LOCALPREF:
        {
            if(p_route->p_path!=NULL)
            {
                *p_lval=p_route->p_path->rcvd_localpref;
            }
            else
            *p_lval=0;

            break;
        }
        case BGP_ROUTE_ASSQUENCE:
        {
            if(p_route->p_path!=NULL)
            {
                
                 tBGP4_ASPATH *p_aspath = NULL;
                 u_char *p_asseq = value_c;                 
                 u_short pathlen = 0;
                 
                 /*construct aspath*/
                 LST_LOOP(&p_route->p_path->aspath_list, p_aspath, node, tBGP4_ASPATH)
                 {
                     /*type*/
                     *p_asseq = p_aspath->type;
                      p_asseq++;
                      pathlen++;

                     /*count*/
                     *p_asseq = p_aspath->len;
                      p_asseq++;
                      pathlen++;

                     /*as copy*/
                      memcpy(p_asseq, p_aspath->p_asseg, p_aspath->len*2);
                      p_asseq += p_aspath->len*2;
                      pathlen += p_aspath->len*2;
                 }
                 
                 /*if there is no as-path exist,only return hdr*/
                 if (pathlen == 0)
                 {
                     p_str->len = 2;
                     p_str->pucBuf[0] = BGP_ASPATH_SEQ;
                     p_str->pucBuf[1] = 0;
                     break;
                 }
                 if (p_str->len > pathlen)
                 {
                     p_str->len = pathlen;
                 }
                 p_str->len = p_str->len & 0x000000ff;
                memcpy(p_str->pucBuf, value_c, p_str->len);
            }

            break;
        }
        case BGP_ROUTE_ORIGIN:
        {
            if(p_route->p_path!=NULL)
            {
                *p_lval=p_route->p_path->origin;
            }
            else
                *p_lval=0;

            break;
        }
        case BGP_ROUTE_ATOMICAGGR:
        {
            if(p_route->p_path!=NULL)
            {
                *p_lval=p_route->p_path->flags.atomic;
            }
            else
                *p_lval=0;
            break;
        }
        case BGP_ROUTE_AGGREGATORADDR:
        {
            if(p_route->p_path!=NULL)
            {
                memcpy(p_str->pucBuf,p_route->p_path->aggregator.addr.ip,16);
                p_str->len=16;
            }
            break;
        }
        case BGP_ROUTE_CALCLOCALPREF:
        {
            if(p_route->p_path!=NULL)
            {
                *p_lval=p_route->p_path->out_localpref;
            }
            break;
        }
        case BGP_ROUTE_BEST:
        {
            tBGP4_ROUTE* p_best;
            if(p_route->p_path!=NULL)
            {
                p_best=bgp4_best_route(&p_instance->rib,&p_route->dest);
                if(p_best == p_route)
                    *p_lval=2;
                else
                    *p_lval=1;
            }
            else
            {
                *p_lval=1;
            }
            break;
        }
        case BGP_ROUTE_ATTRUNKNOWN:
        {
            if(p_route->p_path!=NULL)
            {
                /*max length is 255*/
                p_str->len = p_route->p_path->unknown_len & 0x000000ff;
                memcpy(p_str->pucBuf,p_route->p_path->p_unkown,p_str->len);
            }
            break;
        }
        case BGP_ROUTE_RD:
        {
            bgp4_translate_vpn_RD(p_route,p_str);
            break;
        }
        case BGP_ROUTE_INLABEL:
        {
            if(p_route->route_direction == MPLSL3VPN_ROUTE_LOCAL)
            {
                *p_lval = bgp4_translate_vpn_label(p_route->vpn_label);
            }
            break;
        }
        case BGP_ROUTE_OUTLABEL:
        {
            if(p_route->route_direction == MPLSL3VPN_ROUTE_REMOTE)
            {
                *p_lval = bgp4_translate_vpn_label(p_route->vpn_label);
            }
            break;
        }
#if 1        
        case BGP4_ROUTE_COMMUNITY:
             p_str->len = 0;
             break;
             
        case BGP4_ROUTE_EXTCOMMUNITY:
             p_str->len = 0;
             break;
             
        case BGP4_ROUTE_CLUSTERID:
             p_str->len = 0;
             break;
#endif
    }

    bgp_sem_give();

    return( rc );
}

STATUS bgpPathAttrGetApi(tBgpRtIndex *p_index,u_int cmd,void *var)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int* p_lval=(u_int*)var;
    octetstring *p_str = (octetstring *)var;
    STATUS  rc = VOS_OK;

    char value_c[512];
    tBGP4_ROUTE* p_route=NULL;
    tBGP4_ROUTE cur_rt;
    tBGP4_PATH      cur_path;
    tBGP4_PEER      cur_peer;

    tBGP4_PATH *p_path=NULL;
    tBGP4_PEER *p_peer=NULL;

    memset(&cur_rt,0,sizeof(tBGP4_ROUTE));
    memset(&cur_path,0,sizeof(tBGP4_PATH));
    memset(&cur_peer,0,sizeof(tBGP4_PEER));

    /*construct virtul route*/
    cur_rt.dest.prefixlen=p_index->dest.prefixlen;

    if(p_index->dest.afi==AF_INET)
    {
        cur_rt.dest.afi=BGP4_PF_IPUCAST;
        memcpy(cur_rt.dest.ip,p_index->dest.ip,4);
    }
    else if(p_index->dest.afi==AF_INET6)
    {
        cur_rt.dest.afi=BGP4_PF_IP6UCAST;
        memcpy(cur_rt.dest.ip,p_index->dest.ip,16);
    }

    cur_rt.proto=p_index->proto;

    cur_rt.p_path=&cur_path;

    if(cur_rt.proto==M2_ipRouteProto_bgp)
    {
        memcpy(&cur_peer.remote.ip,&p_index->peer,sizeof(tBgpIpIndex));

        if(p_index->dest.afi==AF_INET)
        {
            cur_peer.remote.ip.afi=BGP4_PF_IPUCAST;
            cur_peer.remote.ip.prefixlen=32;
            memcpy(cur_peer.remote.ip.ip,p_index->peer.addr,4);
        }
        else if(p_index->dest.afi==AF_INET6)
        {
            cur_peer.remote.ip.afi=BGP4_PF_IP6UCAST;
            cur_peer.remote.ip.prefixlen=128;
            memcpy(cur_peer.remote.ip.ip,p_index->peer.addr,16);
        }
        cur_path.p_peer=&cur_peer;
    }
    else
        cur_path.p_peer=NULL;

    bgp_sem_take();

    cur_peer.p_instance = bgp4_vpn_instance_lookup(p_index->peer.vrf_id);/*if no peer,instance id is also given*/
    p_instance = cur_peer.p_instance;
    if (p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    p_route = bgp4_rib_lookup(&p_instance->rib,&cur_rt);

    if (p_route == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    p_path=p_route->p_path;
    if(p_path==NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    p_peer=p_path->p_peer;
    switch(cmd)
    {
        default:
            rc = VOS_ERR;
            break;

        case BGP_PATHATTR_PEER:
        {
            if(p_peer==NULL)
            {
                *p_lval=0;
            }
            else
            {
                if(p_peer->remote.ip.afi==BGP4_PF_IPUCAST)
                {
                    memcpy( p_str->pucBuf, p_peer->remote.ip.ip, 4);
                    p_str->len=4;
                }
                else if(p_peer->remote.ip.afi==BGP4_PF_IP6UCAST)
                {
                    memcpy( p_str->pucBuf, p_peer->remote.ip.ip, 16);
                    p_str->len=16;
                }
            }
            break;
        }
        case BGP_PATHATTR_IPADDRPREFIXLEN:
        {
            *p_lval=p_route->dest.prefixlen;
            break;
        }
        case BGP_PATHATTR_IPADDRPREFIX:
        {
            if(p_route->dest.afi==BGP4_PF_IPUCAST)
            {
                memcpy(p_str->pucBuf,p_route->dest.ip,4);
                p_str->len=4;
            }
            else if(p_route->dest.afi==BGP4_PF_IP6UCAST)
            {
                memcpy(p_str->pucBuf,p_route->dest.ip,16);
                p_str->len=16;
            }
            break;
        }
        case BGP_PATHATTR_ORIGIN:
        {
        /*
        * Ebgp4PathAttrOrigin_igp 1
        * Ebgp4PathAttrOrigin_egp 2
        * Ebgp4PathAttrOrigin_incomplete 3
        */
            *p_lval=p_path->origin+1;
            break;
        }
        case BGP_PATHATTR_ASPATHSEGMENT:
        {
            if(p_route->p_path!=NULL)
            {
                
                 tBGP4_ASPATH *p_aspath = NULL;
                 u_char *p_asseq = value_c;                 
                 u_short pathlen = 0;
                 
                 /*construct aspath*/
                 LST_LOOP(&p_route->p_path->aspath_list, p_aspath, node, tBGP4_ASPATH)
                 {
                     /*type*/
                     *p_asseq = p_aspath->type;
                      p_asseq++;
                      pathlen++;

                     /*count*/
                     *p_asseq = p_aspath->len;
                      p_asseq++;
                      pathlen++;

                     /*as copy*/
                      memcpy(p_asseq, p_aspath->p_asseg, p_aspath->len*2);
                      p_asseq += p_aspath->len*2;
                      pathlen += p_aspath->len*2;
                 }
                 
                 /*if there is no as-path exist,only return hdr*/
                 if (pathlen == 0)
                 {
                     p_str->len = 2;
                     p_str->pucBuf[0] = BGP_ASPATH_SEQ;
                     p_str->pucBuf[1] = 0;
                     break;
                 }
                 if (p_str->len > pathlen)
                 {
                     p_str->len = pathlen;
                 }
                 p_str->len = p_str->len & 0x000000ff;
                memcpy(p_str->pucBuf, value_c, p_str->len);
            }
            break;
        }
        case BGP_PATHATTR_NEXTHOP:
        {
        if(p_route->dest.afi == BGP4_PF_IPUCAST)
        {
            memcpy(p_str->pucBuf,p_path->nexthop.ip,4);
            p_str->len=4;
        }
        else if(p_route->dest.afi == BGP4_PF_IP6UCAST)
        {
            memcpy(p_str->pucBuf,p_path->nexthop_global.ip,16);
            p_str->len=16;

        }
            break;
        }
        case BGP_PATHATTR_MULTIEXITDISC:
        {
            *p_lval=p_path->rcvd_med;
            break;
        }
        case BGP_PATHATTR_LOCALPREF:
        {
            *p_lval=p_path->rcvd_localpref;
            break;
        }
        case BGP_PATHATTR_ATOMICAGGREGATE:
        {
        /*
        * Ebgp4PathAttrAtomicAggregate_lessSpecificRrouteNotSelected 1
        * Ebgp4PathAttrAtomicAggregate_lessSpecificRouteSelected 2
            */
            if (p_path->flags.atomic)
                *p_lval=2;
            else
                *p_lval=1;
            break;
        }
        case BGP_PATHATTR_AGGREGATORAS:
        {
            *p_lval=p_path->aggregator.asnum;
            break;
        }
        case BGP_PATHATTR_AGGREGATORADDR:
        {
           *p_lval=*(u_int*)p_path->aggregator.addr.ip;
            break;
        }
        case BGP_PATHATTR_CALCLOCALPREF:
        {
            *p_lval=(p_path->out_localpref)==0 ? gBgp4.default_lpref : (p_path->out_localpref);
            break;
        }
        /*
        * Ebgp4PathAttrBest_false 1
        * Ebgp4PathAttrBest_true 2
        */
        case BGP_PATHATTR_BEST:
        {
            tBGP4_ROUTE* p_best;
            if(p_route->p_path!=NULL)
            {
                p_best=bgp4_best_route(&p_instance->rib,&p_route->dest);
                if(p_best == p_route)
                    *p_lval=2;
                else
                    *p_lval=1;
            }
            else
            {
                *p_lval=1;
            }
            break;
        }
        case BGP_PATHATTR_UNKNOWN:
        {
            UCHAR *p_attr = p_path->p_unkown ;
            /*max length is 255*/
            p_str->len = p_path->unknown_len & 0x000000ff;
            if (p_str->len!= 0)
            {
                memcpy(p_str->pucBuf,p_attr,p_str->len);
            }
            else
                p_str->pucBuf=NULL;
            break;
        }
        case BGP_PATHATTR_NEWASPATHSEGMENT:
        {
            p_str->len=0;
            p_str->pucBuf=NULL;
            break;
        }
        case BGP_PATHATTR_NEWAGGREGATORAS:
        {
            *p_lval=0;
            break;
        }
    }

    bgp_sem_give();
    return( rc );
}


STATUS bgpConfedPeerGetFirst(u_int *p_index)
{
    u_int as;
    u_int i = 0 ;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    for (i = 0 ;i < BGP4_MAX_CONFEDPEER ; i++)
    {
        as = gBgp4.confedpeer[i];
        if (as)
        {
            *p_index = as;
            rc = VOS_OK;
            break;
        }
    }
    bgp_sem_give();
    return rc;
}

int bgpConfedPeerGetNext(u_int as, u_int *p_nextas)
{
    u_int i = 0 ;
    u_int find = 0;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    for (i = 0 ;i < BGP4_MAX_CONFEDPEER ; i++)
    {
        if(gBgp4.confedpeer[i] == 0)
            continue;

        if (gBgp4.confedpeer[i] == as)
        {
            find = 1;
        }
        else if(find == 1)
        {
            *p_nextas = gBgp4.confedpeer[i];
            rc = VOS_OK;
            break;
        }
    }

    bgp_sem_give();
    return rc;
}

STATUS bgpConfedPeerGet(u_int index,u_int cmd,void *var)
{
    STATUS  rc = VOS_OK;
    u_int i = 0 ;
    u_int* p_lval=(u_int*)var;
    u_int as = index;

    bgp_sem_take();

    for (i = 0 ;i < BGP4_MAX_CONFEDPEER ; i++)
    {
        if (gBgp4.confedpeer[i] == as)
        {
            break;
        }
    }
    if (i >= BGP4_MAX_CONFEDPEER)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    switch(cmd)
    {
        default:
            rc = VOS_ERR;
            break;

        case BGP_CONFEDERATION_PEERINDEX :    /* Table Object INTEGER, read-only */
        {
            *p_lval = i;
            break;
        }

        case BGP_CONFEDERATION_PEERASNUMBER:    /* Table Object INTEGER, read-write */
        {
            *p_lval = as;
            break;
        }

        case BGP_CONFEDERATION_RAWSTATUS:    /* Table Object INTEGER, read-write */
        {
            *p_lval= SNMP_ACTIVE;
            break;
        }
    }

    bgp_sem_give();
    return( rc );
}

STATUS _bgpConfedPeerSet(u_int index,u_int cmd,void *var,u_int flag)
{
    STATUS  rc = VOS_OK;
    u_int sync_cmd = 0;
    u_int* p_lval=(u_int*)var;
    u_int as = index;

    /*sync config to others*/
    sync_cmd = HW_BGP4CONFEDERATION_CMDSTART+cmd;
    if (sync_cmd)
    {
        rc = uspHwBgp4ConfederationSync(as, sync_cmd, (void *)var, flag);
        if(rc != VOS_OK)
        {
            printf("\r\n sync error,config failed");
            return rc;
        }
    }

    bgp_sem_take();

    switch(cmd)
    {
        case BGP_CONFEDERATION_PEERINDEX :
        {
            rc=VOS_ERR;
            break;
        }

        case BGP_CONFEDERATION_PEERASNUMBER:
        {
            rc=VOS_ERR;
            break;
        }

        case BGP_CONFEDERATION_RAWSTATUS:
        {
            if ((*p_lval == SNMP_CREATEANDGO)
                || (*p_lval == SNMP_CREATEANDWAIT)
                || (*p_lval == SNMP_ACTIVE))
            {
                if (bgp4_is_confed_peer(as)==FALSE)
                {
                    bgp4_add_confed_peer(as);
                }
            }
            else if (as == 0)
            {
                bgp4_delete_all_confed_peer();
            }
            else
            {
                bgp4_delete_confed_peer(as);
            }
            break;
        }

        default:
        {
            rc = VOS_ERR;
            break;
        }
    }
    bgp_sem_give();

    return( rc );
}

STATUS bgpConfedPeerSet(u_int index,u_int cmd,void *var)
{
    return  _bgpConfedPeerSet(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}
STATUS bgpConfedPeerSync(u_int index,u_int cmd,void *var)
{
    return  _bgpConfedPeerSet(index, cmd, var,USP_SYNC_LOCAL);
}

STATUS bgpRedistributeGetFirst(tBgpRedistributePolicyIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS rc = VOS_ERR;
    tBGP4_POLICY_CONFIG* p_policy = NULL;

    bgp_sem_take();

    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
        p_policy = (tBGP4_POLICY_CONFIG *)bgp4_lstfirst(&p_instance->import_policy);
        if(p_policy)
        {
            p_index->vrf_id = p_instance->instance_id;
            p_index->policy_index =  p_policy->policy_index;
            p_index->afi = p_policy->af;
            p_index->proto = p_policy->apply_protocol;
            rc = VOS_OK;
            break;
        }

    }


    bgp_sem_give();

    return rc;
}
STATUS bgpRedistributeGetNext(tBgpRedistributePolicyIndex* p_index, tBgpRedistributePolicyIndex* p_nextIndex)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
        STATUS rc = VOS_ERR;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_POLICY_CONFIG* p_policy_next = NULL;
    u_char found = 0;

    bgp_sem_take();

    p_instance = (tBGP4_VPN_INSTANCE*)bgp4_vpn_instance_lookup(p_index->vrf_id);

    if(p_instance)
    {
        LST_LOOP(&p_instance->import_policy,p_policy,node,tBGP4_POLICY_CONFIG)
        {
            if(p_index->afi == p_policy->af &&
                p_index->policy_index == p_policy->policy_index &&
                p_index->proto == p_policy->apply_protocol)
            {
                found = 1;
                p_policy_next = (tBGP4_POLICY_CONFIG *)bgp4_lstnext(&p_instance->import_policy,&p_policy->node);
                break;
            }
        }
        while(found == 1)
        {
            if(p_policy_next)
            {
                p_nextIndex->vrf_id = p_instance->instance_id;
                p_nextIndex->policy_index =  p_policy_next->policy_index;
                p_nextIndex->afi = p_policy_next->af;
                p_nextIndex->proto = p_policy_next->apply_protocol;

                rc = VOS_OK;
                break;/*found*/
            }
            else/*find in next instance*/
            {
                p_instance = (tBGP4_VPN_INSTANCE*)bgp4_lstnext(&gBgp4.vpn_instance_list,&p_instance->node);
                if(p_instance)
                {
                    p_policy_next = (tBGP4_POLICY_CONFIG *)bgp4_lstfirst(&p_instance->import_policy);
                    continue;
                }
                break;/*while loop end,inexistence*/
            }
        }

    }

    bgp_sem_give();
    return rc;
}

STATUS bgpRedistributeGetApi(tBgpRedistributePolicyIndex* p_index,u_int cmd,void *var)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS  rc = VOS_OK;
        u_int*p_lval = (u_int*)var;
    octetstring *p_str = (octetstring *)var;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    u_char found = 0;

    bgp_sem_take();

    p_instance = (tBGP4_VPN_INSTANCE*)bgp4_vpn_instance_lookup(p_index->vrf_id);

    if(p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    LST_LOOP(&p_instance->import_policy,p_policy,node,tBGP4_POLICY_CONFIG)
    {
        if(p_index->afi == p_policy->af &&
                p_index->policy_index == p_policy->policy_index &&
                p_index->proto == p_policy->apply_protocol)
        {
            found = 1;
            break;
        }
    }

    if(found == 0)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    switch(cmd)
        {
            case BGP_REDISTRIBUTE_STATUS:
            {
                *p_lval = p_policy->status;
                    break;
            }
        case BGP_REDISTRIBUTE_MED:
        {
            *p_lval= p_policy->med;
                    break;
        }
        default:
        {
                    rc = VOS_ERR;
            break;
        }

    }

        bgp_sem_give();
        return rc;
}

STATUS _bgpRedistributeSetApi(tBgpRedistributePolicyIndex* p_index,u_int cmd,void *var,u_int flag)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS  rc = VOS_OK;
    u_int*p_lval = (u_int*)var;
    octetstring *p_str = (octetstring *)var;
    u_int sync_cmd = 0;
    tBGP4_POLICY_CONFIG* p_policy = NULL;

    /*sync config to others*/
    if(cmd == BGP_REDISTRIBUTE_VPNNAME)
    {
        flag |= USP_SYNC_OCTETDATA;
    }
    sync_cmd = HW_BGP4REDISTRIBUTION_CMDSTART+cmd;
    if (sync_cmd)
    {
        rc = uspHwBgp4RedistributionSync(p_index, sync_cmd, (void *) var,  flag);
        if(rc != VOS_OK)
        {
            printf("sync error,config failed");
            return rc;
        }
    }

    bgp_sem_take();

    p_instance = (tBGP4_VPN_INSTANCE*)bgp4_vpn_instance_lookup(p_index->vrf_id);

    if(p_instance == NULL)
    {
        if(p_index->vrf_id == 0 ||cmd != BGP_REDISTRIBUTE_STATUS)/*vpn instance need to be created*/
        {
            bgp_sem_give();
            return VOS_ERR;
        }
        p_instance = bgp4_vpn_instance_create(p_index->vrf_id);
        if(p_instance == NULL)
        {
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 vpn instance create failed ");
            bgp_sem_give();
            return VOS_ERR;
        }
    }

    p_policy = bgp4_policy_lookup(&p_instance->import_policy,p_index->afi,p_index->policy_index,p_index->proto);

    if(p_policy == NULL && cmd != BGP_REDISTRIBUTE_STATUS)
    {
        printf("found == 0 && cmd != BGP_REDISTRIBUTE_STATUS %d",cmd);
        bgp_sem_give();
        return VOS_ERR;
    }

    switch(cmd)
        {
            case BGP_REDISTRIBUTE_STATUS:
            {
                switch(*p_lval)
                {
                    case SNMP_CREATEANDWAIT:
                        case SNMP_CREATEANDGO:
                {
                    if(p_policy == NULL)
                    {
                        p_policy = bgp4_policy_add(&p_instance->import_policy);
                        if(p_policy == NULL)
                        {
                            rc = VOS_ERR;
                            break;
                        }
                        p_policy->status = *p_lval;
                        p_policy->af = p_index->afi;
                        p_policy->apply_protocol = p_index->proto;
                        p_policy->policy_index = p_index->policy_index;
                    }
                    else
                    {
                        rc = VOS_ERR;
                    }
                    break;
                }
                case SNMP_ACTIVE:
                {
                    if(p_policy)
                    {
                        p_policy->status = *p_lval;
                        bgp4_redistribute_ip_routes(p_instance,p_policy->af,p_policy->apply_protocol,1);
                        if(p_policy->policy_index && gBgp4.routePolicyRefCntFunc)
                        {
                            gBgp4.routePolicyRefCntFunc(p_policy->policy_index,1);
                        }
                    }

                    break;
                }
                case SNMP_NOTINSERVICE:
                case SNMP_NOTREADY:
                {
                    p_policy ->status = *p_lval;
                    break;

                }
                case SNMP_DESTROY:
                      {
                    if(p_policy)
                    {
                        bgp4_redistribute_ip_routes(p_instance,p_policy->af,p_policy->apply_protocol,0);

                        bgp4_policy_del(&p_instance->import_policy,p_policy);


                    }
                           break;
                      }
                }
                    break;
            }

        case BGP_REDISTRIBUTE_MED:
        {
            p_policy->med= *p_lval;
                    break;
        }

        default:
        {
            rc = VOS_ERR;
            break;
        }

    }

    bgp_sem_give();
    return rc;
}
STATUS bgpRedistributeSetApi(tBgpRedistributePolicyIndex* index,u_int cmd,void *var)
{
    return  _bgpRedistributeSetApi(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}
STATUS bgpRedistributeSyncApi(tBgpRedistributePolicyIndex* index,u_int cmd,void *var)
{
    return  _bgpRedistributeSetApi(index, cmd, var,USP_SYNC_LOCAL);
}

STATUS bgpFilterGetFirst(tBgpFilterPolicyIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS rc = VOS_ERR;
    tBGP4_POLICY_CONFIG* p_policy = NULL;

    bgp_sem_take();

    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
        /*try to find in import policy list first*/
        p_policy = (tBGP4_POLICY_CONFIG *)bgp4_lstfirst(&p_instance->route_policy_import);
        /*if not found ,then find in export policy list*/
        if(p_policy == NULL)
        {
            p_policy = (tBGP4_POLICY_CONFIG *)bgp4_lstfirst(&p_instance->route_policy_export);
        }

        if(p_policy)
        {
            p_index->vrf_id = p_instance->instance_id;
            p_index->policy_index =  p_policy->policy_index;
            p_index->afi = p_policy->af;
            p_index->proto = p_policy->apply_protocol;
            p_index->direction = p_policy->apply_direction;
            rc = VOS_OK;
            break;
        }
        /*if not found neither,then go to next instance to find*/
    }


    bgp_sem_give();

    return rc;


}
STATUS bgpFilterGetNext(tBgpFilterPolicyIndex* p_index, tBgpFilterPolicyIndex* p_nextIndex)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS rc = VOS_ERR;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_POLICY_CONFIG* p_policy_next = NULL;
    u_char found = 0;
    tBGP4_LIST* p_policy_list = NULL;
    tBGP4_LIST* p_policy_list_next = NULL;

    bgp_sem_take();

    p_instance = (tBGP4_VPN_INSTANCE*)bgp4_vpn_instance_lookup(p_index->vrf_id);

    if(p_instance)
    {
        found = 0;

        if(p_index->direction == BGP4_POLICY_IMPORT_DIRECTION)
        {
            p_policy_list = &p_instance->route_policy_import;
            p_policy_list_next = &p_instance->route_policy_export;
        }
        else
        {
            p_policy_list = &p_instance->route_policy_export;
            p_policy_list_next = NULL;
        }

        LST_LOOP(p_policy_list,p_policy,node,tBGP4_POLICY_CONFIG)/*find in import table*/
        {
            if(p_index->afi == p_policy->af &&
                p_index->policy_index == p_policy->policy_index &&
                p_index->proto == p_policy->apply_protocol)
            {
                found = 1;
                p_policy_next = (tBGP4_POLICY_CONFIG *)bgp4_lstnext(p_policy_list,&p_policy->node);

                break;
            }
        }

        while(found == 1)
        {
            if(p_policy_next)
            {
                p_nextIndex->vrf_id = p_instance->instance_id;
                p_nextIndex->policy_index =  p_policy_next->policy_index;
                p_nextIndex->afi = p_policy_next->af;
                p_nextIndex->proto = p_policy_next->apply_protocol;
                p_nextIndex->direction = p_policy_next->apply_direction;
                rc = VOS_OK;
                break;/*found*/
            }
            else
            {
                if(p_policy_list_next)
                {
                    p_policy_next = (tBGP4_POLICY_CONFIG *)bgp4_lstfirst(p_policy_list_next);
                    p_policy_list_next = NULL;
                    continue;
                }
                else
                {
                    p_instance = (tBGP4_VPN_INSTANCE*)bgp4_lstnext(&gBgp4.vpn_instance_list,&p_instance->node);/*find in next instance*/
                    if(p_instance)
                    {
                        if(p_index->direction == BGP4_POLICY_IMPORT_DIRECTION)
                        {
                            p_policy_list = &p_instance->route_policy_import;
                            p_policy_list_next = &p_instance->route_policy_export;
                        }
                        else
                        {
                            p_policy_list = &p_instance->route_policy_export;
                            p_policy_list_next = NULL;
                        }
                        p_policy_next = (tBGP4_POLICY_CONFIG *)bgp4_lstfirst(p_policy_list);
                        continue;
                    }
                    break;/*while loop end,inexistence*/
                }
            }


        }

    }

    bgp_sem_give();
    return rc;

}

STATUS bgpFilterGetApi(tBgpFilterPolicyIndex* p_index,u_int cmd,void *var)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS  rc = VOS_OK;
        u_int*p_lval = (u_int*)var;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    u_char found = 0;
    tBGP4_LIST* p_policy_list = NULL;

    bgp_sem_take();

    p_instance = (tBGP4_VPN_INSTANCE*)bgp4_vpn_instance_lookup(p_index->vrf_id);

    if(p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    if(p_index->direction == BGP4_POLICY_IMPORT_DIRECTION)
    {
        p_policy_list = &p_instance->route_policy_import;
    }
    else
    {
        p_policy_list = &p_instance->route_policy_export;
    }

    LST_LOOP(p_policy_list,p_policy,node,tBGP4_POLICY_CONFIG)
    {
        if(p_index->afi == p_policy->af &&
                p_index->policy_index == p_policy->policy_index &&
                p_index->proto == p_policy->apply_protocol)
        {
            found = 1;
            break;
        }
    }

    if(found == 0)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    switch(cmd)
    {
        case BGP_FILTER_STATUS:
        {
            *p_lval = p_policy->status;
            break;
        }
        default:
        {
            rc = VOS_ERR;
            break;
        }

    }

    bgp_sem_give();
    return rc;
}

STATUS _bgpFilterSetApi(tBgpFilterPolicyIndex* p_index,u_int cmd,void *var,u_int flag)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS  rc = VOS_OK;
    u_int*p_lval = (u_int*)var;
    u_int sync_cmd = 0;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_LIST* p_policy_list = NULL;

    /*sync config to others*/
    if(cmd == BGP_FILTER_VPNNAME)
    {
        flag |= USP_SYNC_OCTETDATA;
    }
    sync_cmd = HW_BGP4FILTER_CMDSTART+cmd;
    if (sync_cmd)
    {
        rc = uspHwBgp4FilterSync(p_index, sync_cmd, (void *) var, flag);
        if(rc != VOS_OK)
        {
            printf("\r\n sync error,config failed");
            return rc;
        }
    }


    p_instance = (tBGP4_VPN_INSTANCE*)bgp4_vpn_instance_lookup(p_index->vrf_id);

    if(p_instance == NULL)
    {
        if(p_index->vrf_id == 0 ||cmd != BGP_FILTER_STATUS)/*vpn instance need to be created*/
        {
            bgp_sem_give();
            return VOS_ERR;
        }
        p_instance = bgp4_vpn_instance_create(p_index->vrf_id);
        if(p_instance == NULL)
        {
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4 vpn instance create failed ");
            bgp_sem_give();
            return VOS_ERR;
        }

    }
    if(p_index->direction == BGP4_POLICY_IMPORT_DIRECTION)
    {
        p_policy_list = &p_instance->route_policy_import;
    }
    else
    {
        p_policy_list = &p_instance->route_policy_export;
    }

    p_policy = bgp4_policy_lookup(p_policy_list,p_index->afi,p_index->policy_index,p_index->proto);

    if(p_policy == NULL && cmd != BGP_FILTER_STATUS)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    switch(cmd)
        {
            case BGP_FILTER_STATUS:
            {
                switch(*p_lval)
                {
                    case SNMP_CREATEANDWAIT:
                        case SNMP_CREATEANDGO:
                {
                    if(p_policy == NULL)
                    {
                        p_policy = bgp4_policy_add(p_policy_list);
                        if(p_policy == NULL)
                        {
                            bgp_sem_give();
                            return VOS_ERR;
                        }
                        p_policy->status = *p_lval;                     
                        p_policy->policy_index = p_index->policy_index;
                        p_policy->af = p_index->afi;
                        p_policy->apply_direction = p_index->direction;
                    }
                    break;
                }
                case SNMP_ACTIVE:
                {
                    if(p_policy && p_policy->policy_index && gBgp4.routePolicyRefCntFunc)
                    {
                        gBgp4.routePolicyRefCntFunc(p_policy->policy_index,1);
                    }
                }
                case SNMP_NOTINSERVICE:
                case SNMP_NOTREADY:
                {
                    if(p_policy)
                    {
                        p_policy ->status = *p_lval;

                    }

                    break;

                }
                case SNMP_DESTROY:
                      {
                           bgp4_policy_del(p_policy_list,p_policy);
                           break;
                      }
                }
                    break;
            }

        default:
        {
                    rc = VOS_ERR;
            break;
        }

    }

        bgp_sem_give();
        return rc;
}

STATUS bgpFilterSetApi(tBgpFilterPolicyIndex* index,u_int cmd,void *var)
{
    return  _bgpFilterSetApi(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}
STATUS bgpFilterSyncApi(tBgpFilterPolicyIndex* index,u_int cmd,void *var)
{
    return  _bgpFilterSetApi(index, cmd, var,USP_SYNC_LOCAL);
}

STATUS bgpPeerRoutePolicyGetFirst(tBgpPeerRtPolicyIndex* p_index)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    STATUS rc = VOS_ERR;
    tBGP4_POLICY_CONFIG* p_policy = NULL;

    bgp_sem_take();

    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
        p_peer = (tBGP4_PEER *)bgp4_lstfirst(&p_instance->peer_list);
        while (p_peer)
        {
            /*start from the import list*/
            p_policy = (tBGP4_POLICY_CONFIG *)bgp4_lstfirst(&p_peer->rt_policy_import);

            /*if import list is empty,then go to the export list*/
            if(p_policy == NULL)
            {
                p_policy = (tBGP4_POLICY_CONFIG *)bgp4_lstfirst(&p_peer->rt_policy_export);
            }

            if(p_policy)
            {
                p_index->rt_policy_index = p_policy->policy_index;
                p_index->addr_type = (p_peer->remote.ip.afi == BGP4_PF_IPUCAST) ?  AF_INET : AF_INET6 ;
                memcpy(p_index->addr,p_peer->remote.ip.ip,16);
                p_index->vrf_id = p_instance->instance_id;
                p_index->direction = p_policy->apply_direction;
                p_index->rt_policy_index = p_policy->policy_index;
                p_index->addr_family = p_policy->af;
                rc = VOS_OK;
                break;
            }
            else/*if no policy exist in such peer,than go to next peer*/
            {
                p_peer = (tBGP4_PEER*)bgp4_lstnext(&p_instance->peer_list,&p_peer->node);
            }

        }
    }

    bgp_sem_give();

    return rc;

}
STATUS bgpPeerRoutePolicyGetNext(tBgpPeerRtPolicyIndex* p_index,tBgpPeerRtPolicyIndex* p_next_index)
{
    STATUS rc = VOS_ERR;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_POLICY_CONFIG* p_policy_next = NULL;
    tBGP4_ADDR peer_addr;
    tBGP4_LIST* p_policy_list = NULL;
    tBGP4_LIST* p_policy_list_next = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER* p_peer = NULL;
    u_char found = 0;

    memset(&peer_addr, 0, sizeof(peer_addr));
    memcpy(peer_addr.ip, p_index->addr, 16);
    peer_addr.prefixlen = (p_index->addr_type == AF_INET) ? 32 :128;
    peer_addr.afi = (p_index->addr_type==AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;

    bgp_sem_take();

    p_instance = (tBGP4_VPN_INSTANCE*)bgp4_vpn_instance_lookup(p_index->vrf_id);
    if(p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    p_peer = bgp4_peer_lookup(p_instance,&peer_addr);

    if (p_peer == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    if(p_index->direction == BGP4_POLICY_IMPORT_DIRECTION)
    {
        p_policy_list = &p_peer->rt_policy_import;
        p_policy_list_next = &p_peer->rt_policy_export;
    }
    else
    {
        p_policy_list = &p_peer->rt_policy_export;
        p_policy_list_next = NULL;
    }

    LST_LOOP(p_policy_list,p_policy,node,tBGP4_POLICY_CONFIG)
    {
        if(p_index->addr_family == p_policy->af &&
                p_index->rt_policy_index == p_policy->policy_index)
        {
            found = 1;
            break;
        }
    }

    if(found == 0)
    {
        bgp_sem_give();
        return VOS_ERR;
    }


    p_policy_next = (tBGP4_POLICY_CONFIG *)bgp4_lstnext(p_policy_list,&p_policy->node);
    if((p_policy_next == NULL)&& (p_policy_list_next))
    {
        p_policy_next = (tBGP4_POLICY_CONFIG *)bgp4_lstfirst(p_policy_list_next);
    }
    if(p_policy_next)
    {
        p_next_index->vrf_id = p_instance->instance_id;
        p_next_index->rt_policy_index =  p_policy_next->policy_index;
        p_next_index->addr_family = p_policy_next->af;
        p_next_index->direction = p_policy_next->apply_direction;
        p_next_index->addr_type = (p_peer->remote.ip.afi == BGP4_PF_IPUCAST) ?  AF_INET : AF_INET6 ;
        memcpy(p_next_index->addr,p_peer->remote.ip.ip,16);
        rc = VOS_OK;
    }
    else
    {
        p_peer = (tBGP4_PEER*)bgp4_lstnext(&p_instance->peer_list,&p_peer->node);
        while(p_instance)
        {
            while(p_peer)
            {
                /*start from the import list*/
                p_policy_next = (tBGP4_POLICY_CONFIG *)bgp4_lstfirst(&p_peer->rt_policy_import);

                /*if import list is empty,then go to the export list*/
                if(p_policy_next == NULL)
                {
                    p_policy_next = (tBGP4_POLICY_CONFIG *)bgp4_lstfirst(&p_peer->rt_policy_export);
                }

                if(p_policy_next)
                {
                    p_next_index->vrf_id = p_instance->instance_id;
                    p_next_index->rt_policy_index =  p_policy_next->policy_index;
                    p_next_index->addr_family = p_policy_next->af;
                    p_next_index->direction = p_policy_next->apply_direction;
                    p_next_index->addr_type = (p_peer->remote.ip.afi == BGP4_PF_IPUCAST) ?  AF_INET : AF_INET6 ;
                    memcpy(p_next_index->addr,p_peer->remote.ip.ip,16);
                    rc = VOS_OK;
                    break;
                }

                p_peer = (tBGP4_PEER*)bgp4_lstnext(&p_instance->peer_list,&p_peer->node);

            }
            if(rc == VOS_OK)
            {
                /*if found,return*/
                break;
            }
            /*if not found in current instance,then go to next instance*/
            p_instance = (tBGP4_VPN_INSTANCE*)bgp4_lstnext(&gBgp4.vpn_instance_list,&p_instance->node);
            if(p_instance)
            {
                p_peer = (tBGP4_PEER*)bgp4_lstfirst(&p_instance->peer_list);
            }
        }
    }

    bgp_sem_give();
    return rc;
}

STATUS bgpPeerRoutePolicyGetApi(tBgpPeerRtPolicyIndex* p_index,u_int cmd,void *var)
{
    STATUS  rc = VOS_OK;
    u_int*p_lval = (u_int*)var;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_PEER* p_peer = NULL;
    tBGP4_ADDR peer_addr;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_LIST* p_policy_list = NULL;
    u_char found = 0;

    memset(&peer_addr, 0, sizeof(peer_addr));
    memcpy(peer_addr.ip, p_index->addr, 16);
    peer_addr.prefixlen = (p_index->addr_type == AF_INET) ? 32 :128;
    peer_addr.afi = (p_index->addr_type==AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;



    bgp_sem_take();

    p_instance = (tBGP4_VPN_INSTANCE*)bgp4_vpn_instance_lookup(p_index->vrf_id);
    if(p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    p_peer = bgp4_peer_lookup(p_instance,&peer_addr);

    if (p_peer == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    if(p_index->direction == BGP4_POLICY_IMPORT_DIRECTION)
    {
        p_policy_list = &p_peer->rt_policy_import;
    }
    else
    {
        p_policy_list = &p_peer->rt_policy_export;
    }

    LST_LOOP(p_policy_list,p_policy,node,tBGP4_POLICY_CONFIG)
    {
        if(p_index->addr_family == p_policy->af &&
                p_index->rt_policy_index == p_policy->policy_index)
        {
            found = 1;
            break;
        }
    }

    if(found == 0)
    {
        bgp_sem_give();
        return VOS_ERR;
    }


    switch(cmd)
    {
        case BGP_PEER_RTPOLICY_STATUS:
        {
            *p_lval = p_policy->status;
            break;
        }

        default:
        {
                    rc = VOS_ERR;
            break;
        }

    }

    bgp_sem_give();
    return rc;
}

STATUS _bgpPeerRoutePolicySetApi(tBgpPeerRtPolicyIndex* p_index,u_int cmd,void *var,u_int flag)
{
    STATUS  rc = VOS_OK;
    u_int*p_lval = (u_int*)var;
    tBGP4_POLICY_CONFIG* p_policy = NULL;
    tBGP4_PEER* p_peer = NULL;
    tBGP4_ADDR peer_addr;
    u_int sync_cmd = 0;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_LIST* p_policy_list = NULL;

    /*sync config to others*/
    if(cmd == BGP_PEER_RTPOLICY_VPNNAME)
    {
        flag |= USP_SYNC_OCTETDATA;
    }
    sync_cmd = HW_BGP4PEERPOLICY_CMDSTART+cmd;
    if (sync_cmd)
    {
        rc = uspHwBgp4PeerPolicySync(p_index, sync_cmd, (void *) var, flag);
        if(rc != VOS_OK)
        {
            printf("\r\nsync error,config failed");
            return rc;
        }
    }


    memset(&peer_addr, 0, sizeof(peer_addr));
    memcpy(peer_addr.ip, p_index->addr, 16);
    peer_addr.prefixlen = (p_index->addr_type == AF_INET) ? 32 :128;
    peer_addr.afi = (p_index->addr_type==AF_INET) ? BGP4_PF_IPUCAST : BGP4_PF_IP6UCAST;

    bgp_sem_take();

    p_instance = (tBGP4_VPN_INSTANCE*)bgp4_vpn_instance_lookup(p_index->vrf_id);

    if(p_instance == NULL)
    {
#if 0
        if(p_index->vrf_id == 0 ||cmd != BGP_PEER_RTPOLICY_STATUS)/*vpn instance need to be created*/
        {
            bgp_sem_give();
            return VOS_ERR;
        }
        p_instance = bgp4_vpn_instance_create(p_index->vrf_id);
        if(p_instance == NULL)
        {
            bgp4_log(BGP_DEBUG_EVT,1,"bgp4_vpn_instance_create failed in _bgpPeerRoutePolicySetApi");
            bgp_sem_give();
            return VOS_ERR;
        }
#endif
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 vpn instance create failed ");
        bgp_sem_give();
        return VOS_ERR;
    
    }
        p_peer = bgp4_peer_lookup(p_instance,&peer_addr);

        if (p_peer == NULL)
        {
            bgp4_log(BGP_DEBUG_EVT,1,"peer not found in instance %d ",p_instance->instance_id);
            bgp_sem_give();
            return VOS_ERR;
        }

    if(p_index->direction == BGP4_POLICY_IMPORT_DIRECTION)
    {
        p_policy_list = &p_peer->rt_policy_import;
    }
    else
    {
        p_policy_list = &p_peer->rt_policy_export;
    }

    p_policy = bgp4_policy_lookup(p_policy_list,p_index->addr_family,p_index->rt_policy_index,0);

    if(p_policy == NULL && cmd != BGP_PEER_RTPOLICY_STATUS)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    switch(cmd)
    {
        case BGP_PEER_RTPOLICY_STATUS:
        {
            switch(*p_lval)
            {
                case SNMP_CREATEANDWAIT:
                case SNMP_CREATEANDGO:
                {
                    if(p_policy == NULL)
                    {
                        p_policy = bgp4_policy_add(p_policy_list);
                        if(p_policy == NULL)
                        {
                            bgp_sem_give();
                            return VOS_ERR;
                        }
                        p_policy->status = *p_lval;
                        p_policy->policy_index = p_index->rt_policy_index;
                        p_policy->af = p_index->addr_family;
                        p_policy->apply_direction = p_index->direction;
                    }
                    break;
                }
                case SNMP_ACTIVE:
                {
                    if(p_policy && p_policy->policy_index && gBgp4.routePolicyRefCntFunc)
                    {
                        gBgp4.routePolicyRefCntFunc(p_policy->policy_index,1);
                    }
                }
                case SNMP_NOTINSERVICE:
                case SNMP_NOTREADY:
                {
                    if(p_policy)
                    {
                        p_policy ->status = *p_lval;
                    }

                    break;

                }
                case SNMP_DESTROY:
                {
                    bgp4_policy_del(p_policy_list,p_policy);
                    break;
                }
            }
            break;
        }

        default:
        {
            rc = VOS_ERR;
            break;
        }

    }

    bgp_sem_give();
    return rc;
}

STATUS bgpPeerRoutePolicySetApi(tBgpPeerRtPolicyIndex* p_index,u_int cmd,void *var)
{
    return  _bgpPeerRoutePolicySetApi(p_index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}
STATUS bgpPeerRoutePolicySyncApi(tBgpPeerRtPolicyIndex* p_index,u_int cmd,void *var)
{
    return  _bgpPeerRoutePolicySetApi(p_index, cmd, var,USP_SYNC_LOCAL);
}


STATUS bgpAggregateGetFirst(tBgpAggrIndex*p_index)
{
    tBGP4_AGGR* p_aggr = NULL;
    tBgpIpIndex aggr_addr;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS rc = VOS_ERR;

    memset(&aggr_addr, 0, sizeof(tBgpIpIndex));

    bgp_sem_take();

    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
        p_aggr = (tBGP4_AGGR *)bgp4_lstfirst(&p_instance->aggr_list);
        if(p_aggr)
        {
            memcpy(aggr_addr.ip, p_aggr->dest.ip, 16);
            aggr_addr.prefixlen=p_aggr->dest.prefixlen;

            if(p_aggr->dest.afi==BGP4_PF_IPUCAST)
                aggr_addr.afi=AF_INET;
            else if(p_aggr->dest.afi==BGP4_PF_IP6UCAST)
                aggr_addr.afi=AF_INET6;

            p_index->vrf_id = p_instance->instance_id;
            p_index->afi = aggr_addr.afi;
            p_index->prefixlen = aggr_addr.prefixlen;
            memcpy(p_index->ip, &aggr_addr.ip, 16);
            rc = VOS_OK;
            break;

        }
    }

    bgp_sem_give();
    return (rc);
}

STATUS bgpAggregateGetNext(tBgpAggrIndex *p_index, tBgpAggrIndex *p_nextindex)
{
    tBGP4_AGGR* p_aggr=NULL;
    tBGP4_AGGR* p_next_aggr =NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ADDR aggr_ip;
    STATUS rc = VOS_ERR;

    memset(&aggr_ip,0,sizeof(tBGP4_ADDR));

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(p_index->vrf_id);
    if(p_instance)
    {
        aggr_ip.prefixlen = p_index->prefixlen;
        memcpy(aggr_ip.ip, p_index->ip, 16);
        aggr_ip.afi = (p_index->afi == AF_INET) ? BGP4_PF_IPUCAST: BGP4_PF_IP6UCAST;

        p_aggr = bgp4_aggregate_lookup(&p_instance->aggr_list,(tBGP4_ADDR*)&aggr_ip);
        if(p_aggr)
        {
            p_next_aggr = (tBGP4_AGGR*)bgp4_lstnext(&p_instance->aggr_list,&p_aggr->node);
        }

        while(p_aggr)
        {
            if(p_next_aggr)
            {
                p_nextindex->afi= (p_next_aggr->dest.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;
                memcpy(p_nextindex->ip, p_next_aggr->dest.ip, 16);
                p_nextindex->prefixlen = p_next_aggr->dest.prefixlen;
                p_nextindex->vrf_id = p_instance->instance_id;

                rc = VOS_OK;
                break;
            }
            else/*find in next instance*/
            {
                p_instance = (tBGP4_VPN_INSTANCE*)bgp4_lstnext(&gBgp4.vpn_instance_list,&p_instance->node);
                if(p_instance)
                {
                    p_next_aggr = (tBGP4_AGGR*)bgp4_lstfirst(&p_instance->aggr_list);
                    continue;
                }
                break;
            }

        }

    }

    bgp_sem_give();

    return  rc ;
}

STATUS bgpAggregateGetApi (tBgpAggrIndex *p_index,u_int cmd,void *var)
{
    u_int* p_lval=var;
    tBGP4_AGGR* p_aggr=NULL;
    tBGP4_ADDR aggr_ip;
    STATUS  rc = VOS_OK;
    tBGP4_VPN_INSTANCE* p_instance = NULL;


    memset(&aggr_ip, 0, sizeof(tBGP4_ADDR));

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(p_index->vrf_id);

    if(p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    aggr_ip.prefixlen = p_index->prefixlen;
    memcpy(aggr_ip.ip, p_index->ip, 16);
    aggr_ip.afi = (p_index->afi == AF_INET) ? BGP4_PF_IPUCAST: BGP4_PF_IP6UCAST;

    p_aggr = bgp4_aggregate_lookup(&p_instance->aggr_list,(tBGP4_ADDR*)&aggr_ip);
    if(p_aggr == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }


    switch(cmd)
    {
        default:
            rc = VOS_ERR;
            break;

        case BGP_AGGREGATE_STATUS:    /* Table Object INTEGER, read-write */
        {
            *p_lval= p_aggr->state;
            break;
        }
        /*
        * EbgpAggregateAdvertise_summary 1
        * EbgpAggregateAdvertise_all 2
        */
        case BGP_AGGREGATE_ADVERTISE:    /* Table Object INTEGER, read-write */
        {
            *p_lval = (p_aggr->summaryonly ==TRUE) ? 1 : 2;
            break;
        }
    }

    bgp_sem_give();
    return rc;
}

STATUS _bgpAggregateSetApi (tBgpAggrIndex *p_index,u_int cmd,void *var,u_int flag)
{
    u_int* p_lval=var;
    tBGP4_AGGR* p_aggr=NULL;
    tBGP4_ADDR aggr_ip;
    STATUS  rc = VOS_OK;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int sync_cmd = 0;

    /*sync config to others*/
    sync_cmd = HW_BGP4AGGREGATE_CMDSTART+cmd;
    if (sync_cmd)
    {
        rc = uspHwBgp4AggregateSync(p_index, sync_cmd, (void *) var, flag);
        if(rc != VOS_OK)
        {
            printf("\r\nsync error,config failed");
            return rc;
        }
    }



    memset(&aggr_ip, 0, sizeof(tBGP4_ADDR));

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(p_index->vrf_id);

    if(p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    aggr_ip.prefixlen = p_index->prefixlen;
    memcpy(aggr_ip.ip, p_index->ip, 16);
    aggr_ip.afi = (p_index->afi == AF_INET) ? BGP4_PF_IPUCAST: BGP4_PF_IP6UCAST;

    p_aggr = bgp4_aggregate_lookup(&p_instance->aggr_list,(tBGP4_ADDR*)&aggr_ip);
    if(p_aggr == NULL && (cmd!=BGP_AGGREGATE_STATUS))
    {
        bgp_sem_give();
        return VOS_ERR;
    }


    switch(cmd)
    {
        default:
            rc = VOS_ERR;
            break;

        case BGP_AGGREGATE_STATUS:    /* Table Object INTEGER, read-write */
        {
            if ((*p_lval == SNMP_CREATEANDGO)
                || (*p_lval == SNMP_CREATEANDWAIT))
            {
                if (p_aggr == NULL)
                {
                    bgp4_add_aggr(&p_instance->aggr_list,&aggr_ip);
                }
            }
            else if (*p_lval == SNMP_ACTIVE)
            {
                if (p_aggr)
                    bgp4_aggr_up(p_instance,p_aggr);
            }
            else if ((*p_lval == SNMP_NOTINSERVICE)
                        || (*p_lval == SNMP_NOTREADY))
            {
                if (p_aggr)
                    bgp4_aggr_down(p_instance,p_aggr);
            }
            else
            {
                if (p_aggr)
                    bgp4_delete_aggr(&p_instance->aggr_list,p_aggr);
            }
            break;
        }
        /*
        * EbgpAggregateAdvertise_summary 1
        * EbgpAggregateAdvertise_all 2
        */
        case BGP_AGGREGATE_ADVERTISE:
        {
            p_aggr->summaryonly = (*p_lval == 1) ? TRUE : FALSE;
            break;
        }
    }

    bgp_sem_give();

    return( rc );
}

STATUS bgpAggregateSetApi (tBgpAggrIndex *index,u_int cmd,void *var)
{
    return  _bgpAggregateSetApi(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}
STATUS bgpAggregateSyncApi (tBgpAggrIndex *index,u_int cmd,void *var)
{
    return  _bgpAggregateSetApi(index, cmd, var,USP_SYNC_LOCAL);
}

STATUS bgpDampGetFirst(void*p_index)
{
        tBGP4_POLICY* p_damp=NULL;
        u_int* pDampId=p_index;

        bgp_sem_take();
        p_damp=bgp_first_damp;
        if(p_damp==NULL)
    {
            bgp_sem_give();
            return VOS_ERR;
    }
    (*pDampId)=p_damp->id;

        bgp_sem_give();
        return VOS_OK;
return VOS_ERR;
}

STATUS bgpDampGetNext(void*p_index, void*pNextIndex)
{
    tBGP4_POLICY*            p_damp=NULL;
    tBGP4_POLICY*            p_next_damp=NULL;
    u_int* pDampId=p_index;
    u_int* pDampIdNext=pNextIndex;

    bgp_sem_take();
    for_each_bgp_damp(p_damp)
    {
        if(p_damp->id==*pDampId)
        {
                p_next_damp=(tBGP4_POLICY*)bgp_next_damp(p_damp);
                if(p_next_damp!=NULL)
            {
                (*pDampIdNext)=p_next_damp->id;
                    bgp_sem_give();
                    return (VOS_OK);
            }
        }
    }
    bgp_sem_give();
    return (VOS_ERR);
}

STATUS bgpDampGetApi (void* p_index,u_int cmd,void *var)
{
    STATUS  rc = VOS_OK;
    u_int* p_lval=var;
    u_int* pDampId=p_index;
    tBGP4_POLICY*            p_damp=NULL;

    bgp_sem_take();
    if (bgp4_taskup() != TRUE)
    {
            bgp_sem_give();
        return VOS_ERR;
    }

        for_each_bgp_damp(p_damp)
    {
            if(p_damp->id==*pDampId)
                break;
    }
        if(p_damp==NULL)
    {
            bgp_sem_give();
            return VOS_ERR;
     }

    switch(cmd)
    {
        default:
            rc=VOS_ERR;
        break;

        case BGP_DAMP_INDEX:
        {
            *p_lval=p_damp->id;
            break;
        }
            case BGP_DAMP_ADMINSTATUS:
        {
            *p_lval=p_damp->status;
                break;
        }
        case BGP_DAMP_IPADDRESS:
        {
            *p_lval=p_damp->dest.addr.addr_v4;
            break;
        }
        case BGP_DAMP_PREFIXLEN:
        {
            *p_lval=p_damp->dest.prefix_len;
            break;
        }
        case BGP_DAMP_PEERADDRESS:
        {
            *p_lval=p_damp->peer ;
            break;
        }
        case BGP_DAMP_TIMEVALUE   :
        {
            *p_lval=p_damp->tv ;
            break;
        }
    }
        bgp_sem_give();
    return( rc );
return VOS_ERR;
}

STATUS bgpDampSetApi (void*p_index,u_int cmd,void *var)
{
        STATUS  rc = VOS_OK;
        u_int* pDampId=p_index;
        u_int* p_lval=var;
        tBGP4_POLICY*            p_damp=NULL;

    if (bgp4_taskup() != TRUE)
    return VOS_ERR;

    printf("\r\n  cmd=%d",cmd);
    bgp_sem_take();
    for_each_bgp_damp(p_damp)
    {
        if(p_damp->id==*pDampId)
            break;
    }

    if(p_damp==NULL)
    {
            if(cmd!=BGP_DAMP_ADMINSTATUS)
        {
                bgp_sem_give();
                return VOS_ERR;
        }
    }

    switch(cmd)
    {
        default:
            rc=VOS_ERR;
        break;

        case BGP_DAMP_ADMINSTATUS:
        {
                if(p_damp==NULL)
            {
                    if(*p_lval!=BGP4_CREATE)
                {
                        rc=VOS_ERR;
                        break;
                }
            }
                else
            {
                    if(p_damp->status==*p_lval)
                        break;
            }

                if(*p_lval==BGP4_CREATE)
            {
                    if(p_damp==NULL)
                {
                }
                    else
                        rc=VOS_ERR;
            }
                else if(*p_lval==BGP4_DELETE)
            {
            }
                else if(TRUE== *p_lval)
            {
            }
                else if(FALSE== *p_lval)
            {
            }
                break;
        }
        case BGP_DAMP_IPADDRESS:
        {
            p_damp->dest.addr.addr_v4 =*p_lval;
            break;
        }
        case BGP_DAMP_PREFIXLEN:
        {
            p_damp->dest.prefix_len=*p_lval;
            break;
        }
        case BGP_DAMP_PEERADDRESS:
        {
            p_damp->peer =*p_lval;
            break;
        }
        case BGP_DAMP_TIMEVALUE   :
        {
            p_damp->tv =*p_lval;
            break;
        }
    }
        bgp_sem_give();
    return( rc );
return VOS_ERR;
}

int bgpNetworkGetFirst(tBgpNetworkIndex* p_index)
{
    tBGP4_NETWORK *p_network;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    STATUS rc = VOS_ERR;

    bgp_sem_take();

    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
        p_network = (tBGP4_NETWORK*)bgp4_lstfirst(&p_instance->network);
        if(p_network)
        {
            p_index->vrf_id = p_instance->instance_id;
            memcpy(p_index->ip,p_network->net.ip,16);
            p_index->afi = p_network->net.afi;
            p_index->prefixlen = p_network->net.prefixlen;
            rc = VOS_OK;
            break;
        }
    }

    bgp_sem_give();
    return rc;
}

int bgpNetworkGetNext(tBgpNetworkIndex* p_index, tBgpNetworkIndex* p_nextindex)
{
    tBGP4_NETWORK *p_network = NULL;
    tBGP4_NETWORK *p_next_network = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ADDR network_ip;
    STATUS rc = VOS_ERR;

    memset(&network_ip,0,sizeof(tBGP4_ADDR));

    bgp_sem_take();

    p_instance = bgp4_vpn_instance_lookup(p_index->vrf_id);
    if(p_instance)
    {
        memcpy(network_ip.ip,p_index->ip,16);
        network_ip.afi = p_index->afi;
        network_ip.prefixlen = p_index->prefixlen;
        p_network = bgp4_network_lookup(p_instance,&network_ip);
        if(p_network)
        {
            p_next_network = (tBGP4_NETWORK*)bgp4_lstnext(&p_instance->network,&p_network->node);
        }

        while(p_network)
        {
            if(p_next_network)
            {
                memcpy(p_nextindex->ip, p_next_network->net.ip,16);
                p_nextindex->prefixlen = p_next_network->net.prefixlen;
                p_nextindex->afi = p_next_network->net.afi;
                p_nextindex->vrf_id = p_instance->instance_id;
                rc = VOS_OK;
                break;
            }
            else/*find in next instance*/
            {
                p_instance = (tBGP4_VPN_INSTANCE*)bgp4_lstnext(&gBgp4.vpn_instance_list,&p_instance->node);
                if(p_instance)
                {
                    p_next_network = (tBGP4_NETWORK*)bgp4_lstfirst(&p_instance->network);
                    continue;
                }
                break;
            }

        }

    }

    bgp_sem_give();
    return rc ;
}

STATUS bgpNetworkGetApi(tBgpNetworkIndex*p_index,u_int cmd,void *var)
{
    tBGP4_NETWORK *p_network;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int* p_lval=(u_int*)var;
    tBGP4_ADDR net_addr;
    STATUS  rc = VOS_OK;

    bgp_sem_take();

    p_instance = (tBGP4_VPN_INSTANCE*)bgp4_vpn_instance_lookup(p_index->vrf_id);
    if(p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    memset(&net_addr,0,sizeof(tBGP4_ADDR));
    net_addr.afi = p_index->afi;
    net_addr.prefixlen = p_index->prefixlen;
    memcpy(net_addr.ip,p_index->ip,16);

    p_network = bgp4_network_lookup(p_instance,&net_addr);

    if (p_network == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    switch(cmd)
    {
        case BGP_NETWORK_RAWSTATUS:
        {
            *p_lval = p_network->status;
            break;
        }
        default:
            rc = VOS_ERR;
            break;

    }
    bgp_sem_give();
    return rc;
}


STATUS _bgpNetworkSetApi(tBgpNetworkIndex *p_index,u_int cmd,void *var,u_int flag)
{
    tBGP4_NETWORK *p_network;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int* p_lval=(u_int*)var;
    tBGP4_ADDR net_addr;
    STATUS  rc = VOS_OK;
    u_int sync_cmd = 0;

    /*sync config to others*/
    sync_cmd = HW_BGP4NETWORK_CMDSTART+cmd;
    if (sync_cmd)
    {
        rc = uspHwBgp4NetworkSync(p_index, sync_cmd, (void *) var,  flag);
        if(rc != VOS_OK)
        {
            bgp4_log(BGP_DEBUG_EVT,1,"sync error,config failed");
            return rc;
        }
    }

    bgp_sem_take();

    p_instance = (tBGP4_VPN_INSTANCE*)bgp4_vpn_instance_lookup(p_index->vrf_id);
    if(p_instance == NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }

    memset(&net_addr,0,sizeof(tBGP4_ADDR));
    net_addr.afi = p_index->afi;
    net_addr.prefixlen = p_index->prefixlen;
    memcpy(net_addr.ip,p_index->ip,16);

    p_network = bgp4_network_lookup(p_instance,&net_addr);

    if (p_network == NULL && (cmd != BGP_NETWORK_RAWSTATUS))
    {
        bgp_sem_give();
        return VOS_ERR;
    }


    switch(cmd)
    {

        case BGP_NETWORK_RAWSTATUS:
        {
            switch(*p_lval)
            {
                case SNMP_CREATEANDWAIT:
                case SNMP_CREATEANDGO:
                {
                    if(p_network == NULL)
                    {
                        p_network = bgp4_add_network(p_instance,&net_addr);
                        if(p_network == NULL)
                        {
                            bgp_sem_give();
                            return VOS_ERR;
                        }
                        p_network->status = *p_lval;
                    }
                    break;
                }
                case SNMP_ACTIVE:
                {
                    bgp4_network_up(p_instance,&p_network->net);
                }
                case SNMP_NOTINSERVICE:
                case SNMP_NOTREADY:
                {
                    if(p_network)
                    {
                        p_network ->status = *p_lval;

                    }

                    break;

                }
                case SNMP_DESTROY:
                {
                    bgp4_network_down(p_instance,&p_network->net);
                    bgp4_delete_network(p_instance,p_network);
                    break;
                }
                default:
                    break;
            }
            break;

        }
        default:
        {
            rc = VOS_ERR;
            break;
        }
    }

    bgp_sem_give();
    return( rc );
}

STATUS bgpNetworkSetApi(void *index,u_int cmd,void *var)
{
    return  _bgpNetworkSetApi(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}
STATUS bgpNetworkSyncApi (void *index,u_int cmd,void *var)
{
    return  _bgpNetworkSetApi(index, cmd, var,USP_SYNC_LOCAL);
}

STATUS bgpExCommGetFirst(void *p_index)
{
    tBGP4_EXT_COMM* p_excomm = NULL;
    tBgpExtCommIndex* p_excomm_index=(tBgpExtCommIndex*)p_index;

    bgp_sem_take();

    p_excomm = (tBGP4_EXT_COMM *)bgp4_lstfirst(&gBgp4.ext_community_list);

    if (p_excomm!= NULL)
    {
        p_excomm_index->as=p_excomm->as;
        p_excomm_index->ipaddr=p_excomm->address;
        bgp_sem_give();
        return (VOS_OK);
    }

    bgp_sem_give();

    return (VOS_ERR);
}

STATUS bgpExCommGetNext(void *p_index, void *p_nextindex)
{
    tBGP4_EXT_COMM* p_ext_comm=NULL;
    tBGP4_EXT_COMM* p_next =NULL;
    tBgpExtCommIndex* ext_comm_index=(tBgpExtCommIndex*)p_index;
    tBgpExtCommIndex* next_ext_comm_index=(tBgpExtCommIndex*)p_nextindex;

    bgp_sem_take();

    LST_LOOP(&gBgp4.ext_community_list, p_ext_comm, node, tBGP4_EXT_COMM)
    {
        if(p_ext_comm->as!=ext_comm_index->as)
            continue;

        if(p_ext_comm->address!=ext_comm_index->ipaddr)
            continue;

        p_next =(tBGP4_EXT_COMM*)bgp4_lstnext(&gBgp4.ext_community_list, &p_ext_comm->node);
        if (p_next != NULL)
        {
            next_ext_comm_index->as=p_next->as;
            next_ext_comm_index->ipaddr=p_next->address;
            bgp_sem_give();
            return VOS_OK;
        }
    }

    bgp_sem_give();

    return  VOS_ERR ;
}

STATUS bgpExCommGetApi (void *p_index,u_int cmd,void *var)
{
    u_int* p_lval=var;
    tBGP4_EXT_COMM* p_ext_comm=NULL;
    tBGP4_EXT_COMM ext_comm;
    STATUS  rc = VOS_OK;
    tBgpExtCommIndex* ext_index=(tBgpExtCommIndex*)p_index;


    bgp_sem_take();

    memset(&ext_comm,0,sizeof(tBGP4_EXT_COMM));
    ext_comm.as=ext_index->as;
    ext_comm.address=ext_index->ipaddr;

    p_ext_comm=bgp4_ext_comm_lookup(&ext_comm);

    if(p_ext_comm==NULL)
    {
        bgp_sem_give();
        return VOS_ERR;
    }
    switch(cmd)
    {
        default:
            rc = VOS_ERR;
            break;

        case BGP_EXCOMMUNITY_MAJFLAG:
        {
            *p_lval= p_ext_comm->main_type;
            break;
        }

        case BGP_EXCOMMUNITY_SUBFLAG:
        {
            *p_lval= p_ext_comm->sub_type;
            break;
        }

        case BGP_EXCOMMUNITY_ADD:
        {
            *p_lval=p_ext_comm->additive;
            break;
        }
    }

    bgp_sem_give();
    return rc;
}

STATUS _bgpExCommSetApi (void *p_index,u_int cmd,void *var,u_int flag)
{
    u_int* p_lval=var;
    u_int sync_cmd = 0;
    tBGP4_EXT_COMM  ext_comm;
    tBGP4_EXT_COMM* p_ext_comm=NULL;
    STATUS  rc = VOS_OK;
    tBgpExtCommIndex* ext_index=(tBgpExtCommIndex*)p_index;
    /*TODO:master slave sync*/
    #if 0
    /*sync config to others*/
    sync_cmd = HW_BGP4EXCOMMUNITY_CMDSTART+cmd;
    if (sync_cmd)
        {
        rc = uspHwBgp4ExcommunitySync(p_index, sync_cmd, (void *) var, flag);
        if(rc != VOS_OK)
        {
            printf("\r\n[_bgpExCommSetApi]:sync error,config failed");
            return rc;
        }
        }
    #endif
    bgp_sem_take();

    memset(&ext_comm,0,sizeof(tBGP4_EXT_COMM));

    ext_comm.as=ext_index->as;
    ext_comm.address=ext_index->ipaddr;

    p_ext_comm=bgp4_ext_comm_lookup(&ext_comm);

    if(p_ext_comm==NULL&&cmd!=BGP_EXCOMMUNITY_STATUS)
    {
        bgp_sem_give();
        return VOS_ERR;
    }
    switch(cmd)
    {
        default:
            rc = VOS_ERR;
            break;

        case BGP_EXCOMMUNITY_STATUS:
        {
            if ((*p_lval == SNMP_CREATEANDGO)
                || (*p_lval == SNMP_CREATEANDWAIT))
            {
                if (p_ext_comm == NULL)
                    bgp4_add_ext_comm(&ext_comm);
                else
                    rc=VOS_ERR;
            }
            else
            {
                if (p_ext_comm)
                    bgp4_delete_ext_comm(p_ext_comm);
                else
                    rc=VOS_ERR;
            }
            break;
        }

        case BGP_EXCOMMUNITY_MAJFLAG:
        {
            p_ext_comm->main_type=*p_lval;
            break;
        }

        case BGP_EXCOMMUNITY_SUBFLAG:
        {
            p_ext_comm->sub_type=*p_lval;
            break;
        }

        case BGP_EXCOMMUNITY_ADD:
        {
            p_ext_comm->additive=*p_lval;
            break;
        }
    }

    bgp_sem_give();



    return( rc );
}

STATUS bgpExCommSetApi(void *index,u_int cmd,void *var)
{
    return  _bgpExCommSetApi(index, cmd, var, (USP_SYNC_LOCAL|USP_SYNC_REMOTE));
}
STATUS bgpExCommSyncApi (void *index,u_int cmd,void *var)
{
    return  _bgpExCommSetApi(index, cmd, var,USP_SYNC_LOCAL);
}


/*function to obtain route count with special type
input:u2type---route type
u4Peer---only valid when route type is BGP
*/
u_int bgp4_get_number_of_route(u_short type, tBGP4_PEER * p_peer)
{
    u_int i = 0 ;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_next_route = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
        RIB_LOOP(&p_instance->rib,p_route,p_next_route)
        {
            /*6PE route and VPN route should not be counted*/
            if(p_route->p_path->safi == BGP4_SAF_LBL ||
                p_route->p_path->safi == BGP4_SAF_VLBL)
            {
                break;
            }

            if(p_peer)
            {
                if(p_route->p_path->p_peer == p_peer &&
                    p_route->p_path->p_peer->p_instance == p_instance)
                {
                    i++;
                }
            }
            else if (p_route->proto == type)
            {
                if(p_route->p_path->src_instance_id == p_route->p_path->p_instance->instance_id)
                {
                    i++;
                }
            }
        }
    }

    return i ;
}

 void bgp4_peer_other_write_file(u_int peer_ip)
 {
 #if 0
     u_char nbrstr[20];
     u_char srcstr[20];
     inet_ntoa_1(nbrstr,peer_ip);
     /*Route Refresh*/
     {
         tBGP4_PEER *p_peer = bgp4_peer_lookup ((u_char*)&peer_ip,BGP4_PEER_IPV4);
         if (p_peer->local.refresh)
         {
             inet_ntoa_1(srcstr,peer_ip);
             vty_out(vty," neighbor %s capability route-refresh%s",
                 nbrstr,VTY_NEWLINE);
         }
     }
#endif
     return ;
 }

struct vty;
#define VTY_NEWLINE "\n\r"
extern int vty_log(char * proto_str, int len);
extern int vty_out (struct vty *, const char *, ...);

void bgp_display_routes(tBGP4_VPN_INSTANCE* p_instance,struct vty * vty,u_int vrf_id)
{
    u_char dstr[64],mstr[64],hstr[64];
    tBGP4_ROUTE *p_rt;
    int first =  1;

    bgp_sem_take();

    for(p_rt=bgp4_rib_first(&p_instance->rib);p_rt;
                p_rt=bgp4_rib_next(&p_instance->rib,p_rt))
    {
        if (first )
        {
            if (p_rt->dest.afi == BGP4_PF_IPMCAST)
                vty_out(vty,"%s  Address Family:IPv4 Multicast%s",VTY_NEWLINE,VTY_NEWLINE);
            else
                vty_out(vty,"%s",VTY_NEWLINE);

            vty_out(vty,"  %-16s%-16s%-16s%-7s%-9s%s%s","Destination","NetMask","NextHop","MED","LocPref","Proto",VTY_NEWLINE);
            first = 0;
        }

        if(p_rt->dest.afi ==BGP4_PF_IPUCAST)
        {
            inet_ntoa_1(dstr, bgp_ip4(p_rt->dest.ip));
            inet_ntoa_1(mstr, bgp4_len2mask(p_rt->dest.prefixlen));
        }
        else if(p_rt->dest.afi ==BGP4_PF_IP6UCAST)
        {
            inet_ntop(AF_INET6,p_rt->dest.ip,dstr,64);
        }
        else if(p_rt->dest.afi ==BGP4_PF_IPVPN)
        {
            inet_ntoa_1(dstr, bgp_ip4(p_rt->dest.ip + BGP4_VPN_RD_LEN));
            inet_ntoa_1(mstr, bgp4_len2mask(p_rt->dest.prefixlen - 8*BGP4_VPN_RD_LEN));
        }

        if (p_rt->p_path)
        {
            if(p_rt->dest.afi ==BGP4_PF_IPUCAST || p_rt->dest.afi ==BGP4_PF_IPVPN)
            {
                inet_ntoa_1(hstr,bgp_ip4(p_rt->p_path->nexthop.ip));
                vty_out(vty,"  %-16s%-16s%-16s%-7d%-9d%s%s",
                            dstr,mstr,hstr,
                            p_rt->p_path->out_med,p_rt->p_path->out_localpref,
                            bgp4_get_route_desc(p_rt->proto),
                            VTY_NEWLINE);
            }
            else if(p_rt->dest.afi ==BGP4_PF_IP6UCAST)
            {
                inet_ntop(AF_INET6,p_rt->p_path->nexthop_global.ip,hstr,64);
                vty_out(vty,"  %-16s%-16d%-16s%-7d%-9d%s%s",
                            dstr,p_rt->dest.prefixlen,hstr,
                            p_rt->p_path->out_med,p_rt->p_path->out_localpref,
                            bgp4_get_route_desc(p_rt->proto),
                            VTY_NEWLINE);
            }
        }
        else
        {
            if(p_rt->dest.afi ==BGP4_PF_IPUCAST|| p_rt->dest.afi ==BGP4_PF_IPVPN)
            {
                vty_out(vty,"  %-16s%-16s%-16s%-7s%-9s%s%s",
                            dstr,mstr,"0.0.0.0",
                            "N/A","N/A",
                            bgp4_get_route_desc(p_rt->proto),
                            VTY_NEWLINE);
            }
            else if(p_rt->dest.afi ==BGP4_PF_IP6UCAST)
            {
                vty_out(vty,"  %-16s%-16d%-16s%-7s%-9s%s%s",
                            dstr,p_rt->p_path->nexthop.prefixlen,"0.0.0.0",
                            "N/A","N/A",
                            bgp4_get_route_desc(p_rt->proto),
                            VTY_NEWLINE);
            }
        }
    }
    bgp_sem_give();
}

void bgp4_display_peer_time(u_int peer_addr)
{
#if 0
   u_int u4Time;
   tBGP4_PEER *p_peer = bgp4_peer_lookup((u_char*)&peer_addr,BGP4_PEER_IPV4);

   if (!p_peer)
       return;

   /*KEEPALIVE*/
   if (bgp4_remaintime(BGP4_KEEPALIVE_TIMER,
       p_peer->KeepAliveTmr,&u4Time) != TRUE)
   {
       vty_out(vty,"\tKeepAlive Timer:Not Running%s",VTY_NEWLINE);
   }
   else
   {
       vty_out(vty,"\tKeepAlive Timer:Expired in %d seconds%s",u4Time,VTY_NEWLINE);
   }
   /*HOLD*/
   if (bgp4_remaintime(BGP4_HOLD_TIMER,
       p_peer->HoldTmr,&u4Time) != TRUE)
   {
       vty_out(vty,"\tHold Timer :not Running%s",VTY_NEWLINE);
   }
   else
   {
       vty_out(vty,"\tHold Timer:Expired in %d seconds%s",u4Time,VTY_NEWLINE);
   }
   /*CONNECTRETRY*/
   if (bgp4_remaintime(BGP4_CONNECTRETRY_TIMER,
       p_peer->ConnRetryTmr,&u4Time) != TRUE)
   {
       vty_out(vty,"\tConnectRetry Timer:not Running%s",VTY_NEWLINE);
   }
   else
   {
       vty_out(vty,"\tConnectRetry Timer:Expired in %d seconds%s",u4Time,VTY_NEWLINE);
   }

   /*START*/
   if (bgp4_remaintime(BGP4_START_TIMER,
       p_peer->StartTmr,&u4Time) != TRUE)
   {
       vty_out(vty,"\tStart Timer:not Running%s",VTY_NEWLINE);
   }
   else
   {
       vty_out(vty,"\tStart Timer:Expired in %d seconds%s",u4Time,VTY_NEWLINE);
   }
   /*other*/
   if (bgp4_remaintime(BGP4_MINASORIG_TIMER,
       p_peer->OrigTmr,&u4Time) != TRUE)
   {
       vty_out(vty,"\tMinAsOrignate Timer:not Running%s",VTY_NEWLINE);
   }
   else
   {
       vty_out(vty,"\tMinAsOrignate Timer:Expired in %d seconds%s",u4Time,VTY_NEWLINE);
   }
   if (bgp4_remaintime(BGP4_MINROUTEADV_TIMER,
       p_peer->RouteAdvTmr,&u4Time) != TRUE)
   {
       vty_out(vty,"\tMinRouteUpdate Timer:not Running%s",VTY_NEWLINE);
   }
   else
   {
       vty_out(vty,"\tMinRouteUpdate Timer:Expired in %d seconds%s",u4Time,VTY_NEWLINE);
   }
   #endif
}

void bgp4_display_resource(struct vty * vty)
{
    tBGP4_PATH *  p_path=NULL;
    tBGP4_ROUTE* p_route=NULL;
    tBGP4_ROUTE* p_next=NULL;
    tBGP4_PEER *p_peer = NULL;
    u_int update_rt=0;
    u_int establish = 0;
    u_char zero[32] = {0};
    u_int af = 0;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int in_ip_process = 0;

    /*Route Table Update*/
       vty_out(vty, "%sStatistic of BGP Rib to IPV4 forwarding table%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, " Add Route:%d,Add Route Fail:%d,Delete Route:%d,Delete Route Fail:%d%s",gBgp4.stat.iprtadd,gBgp4.stat.iprtfail,gBgp4.stat.iprtdel,gBgp4.stat.iprtdelfail,VTY_NEWLINE);
    vty_out(vty, " Add Last Error:%d,Del Last Error:%d%s",gBgp4.stat.iprtadderror,gBgp4.stat.iprtdelerror,VTY_NEWLINE);
    vty_out(vty, " add hardware route:%d,del hardware route:%d,send RT MSG fail:%d%s",gBgp4.stat.rt_msg_rt_add, gBgp4.stat.rt_msg_rt_del,gBgp4.stat.rt_msg_fail,VTY_NEWLINE);

    vty_out(vty, "%sStatistic of BGP Rib to IPV6 forwarding table%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, " Add Route:%d,Add Route Fail:%d,Delete Route:%d,Delete Route Fail:%d%s",gBgp4.stat.ip6rtadd,gBgp4.stat.ip6rtfail,gBgp4.stat.ip6rtdel,gBgp4.stat.ip6rtdelfail,VTY_NEWLINE);
    vty_out(vty, " Add Last Error:%d,Del Last Error:%d%s",gBgp4.stat.ip6rtadderror,gBgp4.stat.ip6rtdelerror,VTY_NEWLINE);
    vty_out(vty, " add hardware route:%d,del hardware route:%d,send RT MSG fail:%d%s",gBgp4.stat.rt6_msg_rt_add, gBgp4.stat.rt6_msg_rt_del,gBgp4.stat.rt6_msg_fail,VTY_NEWLINE);

    vty_out(vty, "%sStatistic of BGP Messages%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, " Send: update:%d,keepalive:%d,open:%d ,notify:%d%s",gBgp4.stat.tcp_update, gBgp4.stat.tcp_keepalive,gBgp4.stat.tcp_open,gBgp4.stat.tcp_notify,VTY_NEWLINE);
    vty_out(vty, " Recv: update:%d,keepalive:%d,open:%d ,notify:%d%s",gBgp4.stat.rx_update, gBgp4.stat.rx_keepalive,gBgp4.stat.rx_open,gBgp4.stat.rx_notify,VTY_NEWLINE);

    vty_out(vty, "%sStatistic of BGP MPLS VPN%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, " vpn instance add :%d,delete:%d%s",gBgp4.vpn_instance_add_count, gBgp4.vpn_instance_del_count,VTY_NEWLINE);
    vty_out(vty, " vpn instance import route target add :%d,delete:%d%s",gBgp4.vpn_import_rt_add_count, gBgp4.vpn_import_rt_del_count,VTY_NEWLINE);
    vty_out(vty, " vpn instance export route target add :%d,delete:%d%s",gBgp4.vpn_export_rt_add_count, gBgp4.vpn_export_rt_del_count,VTY_NEWLINE);
    vty_out(vty, " mpls remote route notify add route:%d,failed:%d,last error:%d%s",gBgp4.stat.mpls_add_notify, gBgp4.stat.mpls_add_notify_failed,gBgp4.stat.mpls_add_notify_error,VTY_NEWLINE);
    vty_out(vty, " mpls remote route notify del route:%d,failed:%d,last error %d%s",gBgp4.stat.mpls_del_notify, gBgp4.stat.mpls_del_notify_failed,gBgp4.stat.mpls_del_notify_error,VTY_NEWLINE);
    vty_out(vty, " remote route RT search Vrf fail:%d%s",gBgp4.stat.vpn_remote_vrf_no_find_count,VTY_NEWLINE);

    vty_out(vty, "%sStatistic of BGP Synchronization Information%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, " Sync msg send %d,msg recv %d,send msg error %d%s",gBgp4.stat.sync_msg_send,gBgp4.stat.sync_msg_recv,gBgp4.stat.sync_msg_fail,VTY_NEWLINE);
    vty_out(vty, " Recv too big update msg %d%s",gBgp4.stat.rx_big_update,VTY_NEWLINE);
    vty_out(vty, " Route Socket %d%s",gBgp4.rtsock,VTY_NEWLINE);

    for(af = 0;af < BGP4_PF_MAX;af++)
    {
        LST_LOOP(&gBgp4.attr_list[af], p_path, node, tBGP4_PATH)
        {
            LST_LOOP_SAFE(&p_path->route_list, p_route, p_next, node, tBGP4_ROUTE)
            {
                /*check if update need*/
                /*scan for each peer*/
                if ((memcmp(p_route->withdraw_bits, zero, sizeof(p_route->withdraw_bits)))
                    || (memcmp(p_route->update_bits, zero, sizeof(p_route->update_bits)) ))
                {
                    update_rt++;
                }
                if(p_route->ip_action != BGP4_IP_ACTION_NONE)
                {
                    in_ip_process++;
                }
            }
        }
    }


    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
        LST_LOOP (&p_instance->peer_list,p_peer, node, tBGP4_PEER)
        {
            if(p_peer->state == BGP4_NS_ESTABLISH)
            {
                establish++;
            }
        }

    }


    vty_out(vty, "%sStatistic of BGP Routes Updating Information%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, "  %d Routes Need to be Updated in IP table now %s",in_ip_process, VTY_NEWLINE);
       vty_out(vty, "  %d Routes Need to be Advertised now, Established Peers %d%s",update_rt, establish, VTY_NEWLINE);
    if(gBgp4.rib_update_wait)
    {
        vty_out(vty, " !Waiting HW ROUTE to Finish Updating now,Kernal Routes Count %d,HW Routes Count %d...... %s",
                    gBgp4.kernel_routes_count,
                    gBgp4.hw_routes_count,
                    VTY_NEWLINE);
    }

    if(gBgp4.duplicate_deleted_route_count)
    {
        vty_out(vty, " Existing duplicate deleted route,count %d...... ",
                    gBgp4.duplicate_deleted_route_count,
                    VTY_NEWLINE);
    }

    if(gBgp4.import_filtered_route_count)
    {
        vty_out(vty, " Routes filtered by import route policy,count %d...... ",
                    gBgp4.import_filtered_route_count,
                    VTY_NEWLINE);
    }

    if(gBgp4.export_filtered_route_count)
    {
        vty_out(vty, " Routes filtered by export route policy,count %d...... ",
                    gBgp4.export_filtered_route_count,
                    VTY_NEWLINE);
    }

    {
        u_int delaylst_route_count = bgp4_lstcnt(&gBgp4.delay_update_list);
        if(delaylst_route_count)
        {
            vty_out(vty, "Delay routes count(nexthop unreachable) : %d...... ",
                        delaylst_route_count,
                        VTY_NEWLINE);
        }
    }
    vty_out(vty, " Nexthop Change time %d",gBgp4.direct_nexthop_changed_times, VTY_NEWLINE);

    /*socket*/
    vty_out(vty, "%sStatistic of BGP Socket Operation%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty, " %-10s%-8s%-8s%s","Operation","Total","Fail",VTY_NEWLINE);
    vty_out(vty, " %-10s%-8d%-8d%s","Open",gBgp4.stat.sock,gBgp4.stat.sockfail,VTY_NEWLINE);
    vty_out(vty, " %-10s%-8d%-8d%s","Connect",gBgp4.stat.connect,gBgp4.stat.connectfail,VTY_NEWLINE);
    vty_out(vty, " %-10s%-8d%-8d%s","Send",gBgp4.stat.tx,gBgp4.stat.txfail,VTY_NEWLINE);
    vty_out(vty, " %-10s%-8d%-8d%s","Recieve",gBgp4.stat.rcv,gBgp4.stat.rcvfail,VTY_NEWLINE);
    vty_out(vty, " %-10s%-8d%-8d%s","Closed",gBgp4.tcp_connection_closed_times,0,VTY_NEWLINE);

    /*Dynamic mem object*/
    vty_out(vty, "%sDynamic Memory Object Table%s",VTY_NEWLINE,VTY_NEWLINE);

    vty_out(vty, "%-20s%-6s%-8s%-16s%-16s%-16s%-16s%-16s%s","description","size","maximum","create","create-fail","delete","delete-fail","current",VTY_NEWLINE);

#define BGP4_MEMSTAT_SHOW(type, len, name) \
    do{\
       vty_out(vty, "%-20s%-6d%-8d%-16d%-16d%-16d%-16d%-16d%s",(name),\
       (len), bgp4_mem_init_maximum(type),gBgp4.stat.mem[type].add, gBgp4.stat.mem[type].fail,\
       gBgp4.stat.mem[type].del,0,(gBgp4.stat.mem[type].add - gBgp4.stat.mem[type].fail - gBgp4.stat.mem[type].del),VTY_NEWLINE);}while(0)

     BGP4_MEMSTAT_SHOW(MEM_BGP_NETWORK, sizeof(tBGP4_NETWORK), "StaticNetwork");
/*
     BGP4_MEMSTAT_SHOW(MEM_BGP_CONFED, sizeof(tBGP4_CONFEDPEER), "ConfederationPeer");
*/
     BGP4_MEMSTAT_SHOW(MEM_BGP_LINK, sizeof(tBGP4_LINK), "LinkNode");

     BGP4_MEMSTAT_SHOW(MEM_BGP_PEER, sizeof(tBGP4_PEER), "Peer");

     BGP4_MEMSTAT_SHOW(MEM_BGP_INFO, sizeof(tBGP4_PATH), "RtAttribute");

     BGP4_MEMSTAT_SHOW(MEM_BGP_ROUTE, sizeof(tBGP4_ROUTE), "RouteEntry");

     BGP4_MEMSTAT_SHOW(MEM_BGP_BUF, 0, "LinearBuffer");

     BGP4_MEMSTAT_SHOW(MEM_BGP_ASPATH, sizeof(tBGP4_ASPATH), "ASPath");

     BGP4_MEMSTAT_SHOW(MEM_BGP_VPN_INSTANCE, sizeof(tBGP4_VPN_INSTANCE), "VpnInstance");
/*
     BGP4_MEMSTAT_SHOW(MEM_BGP_CLUSTER, sizeof(tBGP4_CLUSTER), "ClusterNode");

     BGP4_MEMSTAT_SHOW(MEM_BGP_COMMUNITY, sizeof(tBGP4_COMMUNITY), "Community");
*/
      return ;
}

/*shell show func*/
void bgp_show_vrf_routes(u_int vrf_id)
{
  #if 1
    u_char dstr[64],mstr[64],hstr[64];
    tBGP4_ROUTE *p_rt;
    int first =  1;
    tBGP4_VPN_INSTANCE* p_instance = bgp4_vpn_instance_lookup(vrf_id);

    if(p_instance == NULL)
        return;

    bgp_sem_take();

    for(p_rt=bgp4_rib_first(&p_instance->rib);p_rt;
        p_rt=bgp4_rib_next(&p_instance->rib,p_rt))
    {
        if (first )
        {
            if (p_rt->dest.afi == BGP4_PF_IPMCAST)
                printf("%s  Address Family:IPv4 Multicast%s",VTY_NEWLINE,VTY_NEWLINE);
            else
                printf("%s",VTY_NEWLINE);

            printf("  %-16s%-16s%-16s%-7s%-9s%s%s","Destination","NetMask","NextHop","MED","LocPref","Proto",VTY_NEWLINE);
            first = 0;
        }

        if(p_rt->dest.afi ==BGP4_PF_IPUCAST)
        {
                    inet_ntoa_1(dstr, bgp_ip4(p_rt->dest.ip));
                    inet_ntoa_1(mstr, bgp4_len2mask(p_rt->dest.prefixlen));
        }
        else if(p_rt->dest.afi ==BGP4_PF_IP6UCAST)
        {
            inet_ntop(AF_INET6,p_rt->dest.ip,dstr,64);
        }
        else if(p_rt->dest.afi ==BGP4_PF_IPVPN)
        {
                    inet_ntoa_1(dstr, bgp_ip4(p_rt->dest.ip + BGP4_VPN_RD_LEN));
                inet_ntoa_1(mstr, bgp4_len2mask(p_rt->dest.prefixlen - 8*BGP4_VPN_RD_LEN));
        }

        if (p_rt->p_path)
            {
            if(p_rt->dest.afi ==BGP4_PF_IPUCAST || p_rt->dest.afi ==BGP4_PF_IPVPN)
                {
                inet_ntoa_1(hstr,bgp_ip4(p_rt->p_path->nexthop.ip));
                printf("  %-16s%-16s%-16s%-7d%-9d%s%s",
                        dstr,mstr,hstr,
                        p_rt->p_path->out_med,p_rt->p_path->out_localpref,
                        bgp4_get_route_desc(p_rt->proto),
                        VTY_NEWLINE);
            }
            else if(p_rt->dest.afi ==BGP4_PF_IP6UCAST)
            {
                inet_ntop(AF_INET6,p_rt->p_path->nexthop_global.ip,hstr,64);
                printf("  %-16s%-16d%-16s%-7d%-9d%s%s",
                        dstr,p_rt->dest.prefixlen,hstr,
                        p_rt->p_path->out_med,p_rt->p_path->out_localpref,
                        bgp4_get_route_desc(p_rt->proto),
                        VTY_NEWLINE);
            }
            }
        else
            {
            if(p_rt->dest.afi ==BGP4_PF_IPUCAST|| p_rt->dest.afi ==BGP4_PF_IPVPN)
            {
                printf("  %-16s%-16s%-16s%-7s%-9s%s%s",
                        dstr,mstr,"0.0.0.0",
                        "N/A","N/A",
                        bgp4_get_route_desc(p_rt->proto),
                        VTY_NEWLINE);
            }
            else if(p_rt->dest.afi ==BGP4_PF_IP6UCAST)
            {
                printf("  %-16s%-16d%-16s%-7s%-9s%s%s",
                        dstr,p_rt->p_path->nexthop.prefixlen,"0.0.0.0",
                        "N/A","N/A",
                        bgp4_get_route_desc(p_rt->proto),
                        VTY_NEWLINE);
            }
            }
        }
    bgp_sem_give();
    #endif
}
void bgp4_show_path()
{
    tBGP4_PATH *p_path = NULL ;
    tBGP4_PATH *p_next = NULL ;
    u_int af = 0;
    u_int path_cnt = 0;
    u_char peer_str[64] = {0};

    bgp_sem_take();

    for(af = 0;af < BGP4_PF_MAX;af++)
    {
        LST_LOOP_SAFE(&gBgp4.attr_list[af], p_path, p_next, node, tBGP4_PATH)
        {
            path_cnt++;
            printf("\r\n path %d,its af is %d,origin peer is %s,route count %d",
                        path_cnt,
                        af,
                        (p_path->p_peer) ? (bgp4_printf_peer(p_path->p_peer,peer_str)) : ("NULL"),
                        bgp4_lstcnt(&p_path->route_list));
        }
    }
    printf("\r\n TOTAL COUNT %d",path_cnt);

    bgp_sem_give();

    return;

}

void bgp4_set_rt_match(u_char set_flag)
{
    gBgp4.rt_matching_vpnv4 = gBgp4.rt_matching_vpnv6 = set_flag;

    return;
}

void bgp4_set_as_substitution(u_int vrf_id,u_char set_flag)
{
    tBGP4_PEER* p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = bgp4_vpn_instance_lookup(vrf_id);

    if(p_instance == NULL)
    {
        return;
    }

    bgp_sem_take();

    LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER)
    {
        p_peer->as_substitute_enable = set_flag;
    }

    bgp_sem_give();

    return;
}

void bgp4_clear_statistics()
{
    gBgp4.stat.iprtadd =0;
    gBgp4.stat.iprtfail=0;
    gBgp4.stat.iprtdel=0;
    gBgp4.stat.iprtdelfail=0;

    gBgp4.stat.iprtadderror=0;
    gBgp4.stat.iprtdelerror=0;
    
    gBgp4.stat.rt_msg_rt_add =0;
    gBgp4.stat.rt_msg_rt_del=0;
    gBgp4.stat.rt_msg_fail=0;

    gBgp4.stat.ip6rtadd=0;
    gBgp4.stat.ip6rtfail=0;
    gBgp4.stat.ip6rtdel=0;
    gBgp4.stat.ip6rtdelfail=0;

    gBgp4.stat.ip6rtadderror=0;
    gBgp4.stat.ip6rtdelerror=0;

    gBgp4.stat.rt6_msg_rt_add=0;
    gBgp4.stat.rt6_msg_rt_del=0;
    gBgp4.stat.rt6_msg_fail=0;

    gBgp4.stat.tcp_update=0;
    gBgp4.stat.tcp_keepalive=0;
    gBgp4.stat.tcp_open=0;
    gBgp4.stat.tcp_notify=0;

    gBgp4.stat.rx_update=0;
    gBgp4.stat.rx_keepalive=0;
    gBgp4.stat.rx_open=0;
    gBgp4.stat.rx_notify=0;


    gBgp4.vpn_instance_add_count=0;
    gBgp4.vpn_instance_del_count=0;

    gBgp4.vpn_import_rt_add_count=0;
    gBgp4.vpn_import_rt_del_count=0;
    
    gBgp4.vpn_export_rt_add_count=0;
    gBgp4.vpn_export_rt_del_count=0;
    
    gBgp4.stat.mpls_add_notify=0;
    gBgp4.stat.mpls_add_notify_failed=0;
    gBgp4.stat.mpls_add_notify_error=0;
    
    gBgp4.stat.mpls_del_notify=0;
    gBgp4.stat.mpls_del_notify_failed=0;
    gBgp4.stat.mpls_del_notify_error=0;
    
    gBgp4.stat.vpn_remote_vrf_no_find_count=0;

    gBgp4.stat.sync_msg_send=0;
    gBgp4.stat.sync_msg_recv=0;
    gBgp4.stat.sync_msg_fail=0;

    gBgp4.duplicate_deleted_route_count=0;

    gBgp4.import_filtered_route_count=0;
    gBgp4.export_filtered_route_count=0;

    gBgp4.stat.sock=0;
    gBgp4.stat.sockfail=0;
    gBgp4.stat.connect=0;
    gBgp4.stat.connectfail=0;
    gBgp4.stat.tx=0;
    gBgp4.stat.txfail=0;
    gBgp4.stat.rcv=0;
    gBgp4.stat.rcvfail=0;
    gBgp4.tcp_connection_closed_times=0;    

    gBgp4.stat.rx_big_update = 0;
    
    return;
}

#endif
