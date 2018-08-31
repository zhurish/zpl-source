#include "bgp4_api.h"
#include "bgp4peer.h"
#include "bgp4msgh.h"
#ifdef NEW_BGP_WANTED

extern int 
bgp4_open_capablity_extract(
         tBGP4_RXOPEN_PACKET *p_packet,
         u_char *p_opt, 
         u_short opt_len);
extern u_short
bgp4_open_capablilty_fill(
       tBGP4_PEER *p_peer, 
       u_char *p_buf);
extern void 
bgp4_display_update_input(
    tBGP4_PEER *p_peer, 
    avl_tree_t *flist, 
    avl_tree_t *wlist);
extern u_int
bgp4_end_of_rib_check(
          tBGP4_PEER *p_peer,
          u_char *p_pdu,
          u_int len);


STATUS 
bgp4_msg_verify(
      tBGP4_PEER *p_peer,
      u_char *p_pdu)
{
    tBGP4_MSGHDR hdr;
    u_char marker[BGP4_MARKER_LEN];
    u_int len = 0;
    u_char str[64];
    u_int len_error = FALSE;
    
    memcpy(&hdr, p_pdu, sizeof(tBGP4_MSGHDR));           
    len = ntohs(hdr.len);

    /*protocol define the min&max bytes of a message*/
    if ((len < BGP4_MIN_MSG_LEN) || (len > BGP4_MAX_MSG_LEN)) 
    {
        bgp4_log(BGP_DEBUG_CMN|BGP_DEBUG_ERROR,"receive msg from %s , invalid length %d!!", bgp4_printf_peer(p_peer, str),len);
        
        p_peer->stat.msg_len_err++;
        
        p_peer->notify.code = BGP4_MSG_HDR_ERR;
        p_peer->notify.sub_code = BGP4_BAD_MSG_LEN;
        p_peer->notify.len = 2;
        memcpy(p_peer->notify.data, &hdr.len, 2);

        p_peer->notify.close_peer = TRUE;
        return VOS_ERR;    
    }

    memset(marker, 0xff, sizeof(marker));
    /*Authenticate rcvd message,compare the first 16 bytes ,they must be all 1*/ 
    if (memcmp(marker, hdr.marker, BGP4_MARKER_LEN))
    {
        bgp4_log(BGP_DEBUG_CMN|BGP_DEBUG_ERROR,"receive msg from %s , wrong marker field!!", bgp4_printf_peer(p_peer, str));
        
        p_peer->stat.msg_marker_err++;
        
        p_peer->notify.code = BGP4_MSG_HDR_ERR;
        p_peer->notify.sub_code = BGP4_CONN_NOT_SYNC;
        p_peer->notify.close_peer = TRUE;
        
        return VOS_ERR;    
    }
    p_peer->stat.rx_msg++;

    switch (hdr.type) {
        case BGP4_MSG_OPEN:        
             gbgp4.stat.msg_rx.open++;
             p_peer->stat.open_rx++;
             if (len < BGP4_OPEN_HLEN) 
             {
                 len_error = TRUE;
             } 
             break;

        case BGP4_MSG_UPDATE:
             gbgp4.stat.msg_rx.update++;
             if (len < BGP4_UPDATE_HLEN) 
             {
                 len_error = TRUE;
             }  
             break;

        case BGP4_MSG_NOTIFY:
             gbgp4.stat.msg_rx.notify++;
             if (len < BGP4_NOTIFY_HLEN) 
             {
                 len_error = TRUE;
             } 
             break;

        case BGP4_MSG_KEEPALIVE:
             gbgp4.stat.msg_rx.keepalive++;
             if (len != BGP4_KEEPALIVE_LEN) 
             {
                 len_error = TRUE;
             } 
             break;
             
        case BGP4_MSG_REFRESH:
             if (len < BGP4_ROUTE_REFRESH_LEN) 
             {
                 len_error = TRUE;
             }  
             break;

        default:
            bgp4_log(BGP_DEBUG_CMN|BGP_DEBUG_ERROR, "receive msg from %s,invalid type", bgp4_printf_peer(p_peer, str));
            
            p_peer->notify.code = BGP4_MSG_HDR_ERR;
            p_peer->notify.sub_code = BGP4_BAD_MSG_TYPE;
            p_peer->notify.len = 1;
            memcpy(p_peer->notify.data, &hdr.type, 1);
            return VOS_ERR;
            break;        
    }

    if (len_error == TRUE)
    {
        bgp4_log(BGP_DEBUG_CMN|BGP_DEBUG_ERROR, "receive msg from %s,invalid length", bgp4_printf_peer(p_peer, str));
        
        p_peer->notify.code = BGP4_MSG_HDR_ERR;
        p_peer->notify.sub_code = BGP4_BAD_MSG_LEN;
        p_peer->notify.len = 2; 
        memcpy(p_peer->notify.data, &hdr.len, 2);
        return VOS_ERR;
    }
    return VOS_OK;
}

/*parse open msg*/
STATUS
bgp4_open_decode(tBGP4_RXOPEN_PACKET *p_packet)
{
    tBGP4_MSGHDR hdr;
    u_int len = 0;
    u_int total_optlen;
    u_char read_len = 0;
    u_char opt_type = 0;
    u_char opt_len = 0;
    u_char *p_pdu = p_packet->p_pdu;
    
    memcpy(&hdr, p_pdu, sizeof(tBGP4_MSGHDR));           
    len = ntohs(hdr.len);

    p_pdu += BGP4_HLEN;
    len -= BGP4_HLEN;

    p_packet->version = *p_pdu;
    p_pdu++;

    bgp4_get_2bytes(p_packet->as, p_pdu);
    p_pdu += 2;

    bgp4_get_2bytes(p_packet->hold_time, p_pdu);
    p_pdu += 2;

    bgp4_get_4bytes(p_packet->router_id, p_pdu);
    p_pdu += 4;

    /*parse option param*/
    total_optlen = *p_pdu;
    p_pdu += 1; 

    for (; read_len < total_optlen; read_len += (2 + opt_len))
    {
        /*get optional params type */
        opt_type = *p_pdu;
        p_pdu++;
                         
        /*get params length,length do not include type-length pair */  
        opt_len = *p_pdu;
        p_pdu++;

        /*only process capability*/
        if (opt_type == BGP4_OPEN_OPTION_CAPABILITY)
        {
            if (bgp4_open_capablity_extract(p_packet, p_pdu, opt_len) != VOS_OK)
            {
                return VOS_ERR;
            }
        }
    }
    return VOS_OK;
}

/*open msg processing*/
void 
bgp4_open_input(
      tBGP4_PEER *p_peer,
      u_char *p_pdu)
{
    tBGP4_MSGHDR hdr;
    tBGP4_RXOPEN_PACKET packet;
    u_int len = 0;
    u_short hold_time = 0;
    u_int err_len = 0;
    u_int af = 0;
    u_int unsupport_capability = 0;
    u_char errmsg[512];
    u_char str[64];

    memcpy(&hdr, p_pdu, sizeof(tBGP4_MSGHDR));           
    len = ntohs(hdr.len);
    
    bgp4_log(BGP_DEBUG_CMN, "receive open msg from %s,len %d", bgp4_printf_peer(p_peer, str), len);

    if (p_peer->state != BGP4_NS_OPENSENT)
    {
        bgp4_log(BGP_DEBUG_CMN|BGP_DEBUG_ERROR, "neighbor state invlalid for open");
        
        if (p_peer->state > BGP4_NS_OPENSENT)
        {
           p_peer->notify.code = BGP4_FSM_ERR;
        }
        goto CLOSE_PEER;
    }
    memset(&packet, 0, sizeof(packet));
    packet.p_pdu = p_pdu;
    
    if (bgp4_open_decode(&packet) != VOS_OK)
    {
        p_peer->notify.code = BGP4_OPEN_MSG_ERR;
        p_peer->notify.sub_code = BGP4_OPT_PARM_UNRECOGNIZED;
        goto CLOSE_PEER;
    }
    
    /*version mismatch*/
    if (packet.version != BGP4_VERSION_4) 
    {
        if (packet.version > BGP4_VERSION_4)
        {
            packet.version = BGP4_VERSION_4;
        }
        
        p_peer->notify.code = BGP4_OPEN_MSG_ERR;
        p_peer->notify.sub_code = BGP4_UNSUPPORTED_VER_NO;
        p_peer->notify.len = 1;
        memcpy(p_peer->notify.data, &packet.version, 1); 
        goto CLOSE_PEER;
    }
    
    /*as mismatch*/
    if (packet.as!= p_peer->as)
    {
        p_peer->notify.code = BGP4_OPEN_MSG_ERR;
        p_peer->notify.sub_code = BGP4_AS_UNACCEPTABLE;
        p_peer->notify.len = 2;
        memcpy(p_peer->notify.data, &packet.as, 2);
        goto CLOSE_PEER;
    }

    if ((packet.router_id == 0) 
        || (packet.router_id == 0xFFFFFFFF)
        || (packet.router_id == 0x7F000001)) 
    {         
        p_peer->notify.code = BGP4_OPEN_MSG_ERR;
        p_peer->notify.sub_code = BGP4_BGPID_INCORRECT;
        p_peer->notify.len = 4;
        memcpy(p_peer->notify.data, &packet.router_id, 4); 
        goto CLOSE_PEER;
    }

    /*neg hold time*/
    hold_time = packet.hold_time;
    if (hold_time < p_peer->hold_interval) 
    {
        if (hold_time && (hold_time < BGP4_MIN_HOLD_INTERVAL)) 
        {
           p_peer->notify.code = BGP4_OPEN_MSG_ERR;
           p_peer->notify.sub_code = BGP4_HOLD_TMR_UNACCEPTABLE;
           p_peer->notify.len = 2;
           memcpy(p_peer->notify.data, &hold_time, 2);        
           goto CLOSE_PEER;
        } 
        p_peer->neg_hold_interval = hold_time;
        if (p_peer->hold_interval)                  
        {
            p_peer->neg_keep_interval = (hold_time * p_peer->keep_interval)/p_peer->hold_interval;
        }
        p_peer->neg_keep_interval = p_peer->neg_keep_interval ? p_peer->neg_keep_interval : 1;
    }
    else 
    {
        p_peer->neg_hold_interval = p_peer->hold_interval;
        p_peer->neg_keep_interval = p_peer->keep_interval;
    }

    bgp4_timer_stop(&p_peer->hold_timer);

    /*init rib send&rx flag*/

    /*if no GR tlv exist,and peer is in restarting,stop GR*/
    if (p_peer->in_restart && (packet.restart_enable == FALSE))
    {        
        bgp4_log(BGP_DEBUG_CMN, "peer not support graceful restarting,stop restarting");

        bgp4_peer_restart_timeout(p_peer);
    }

    /*if GR-AF change,clear all unused AF*/
    if (p_peer->restart_mpbgp_capability != packet.restart_mpbgp_capability)
    {
        for (af = 0; af < BGP4_PF_MAX ; af++)
        {
            if (p_peer->p_instance->rib[af] == NULL)
            {
                continue;
            }
            if (bgp4_af_support(p_peer, af) 
                && (!flag_isset(packet.restart_mpbgp_capability, af)))
            {                
                bgp4_log(BGP_DEBUG_CMN, "peer not support af %d restarting,stop restarting", af);
                    
                bgp4_peer_stale_route_clear(af, p_peer);

                bgp4_rib_in_check_timeout(p_peer->p_instance->rib[af]);

                bgp4_rib_system_update_check(p_peer->p_instance->rib[af]);
            }
        }
    }
    p_peer->txd_rib_end = 0; 
    p_peer->rxd_rib_end = 0; 
    p_peer->version = packet.version;
    p_peer->router_id = packet.router_id;
    p_peer->restart_enable = packet.restart_enable;
    p_peer->refresh_enable = packet.refresh_enable;
    p_peer->in_restart = packet.in_restart;
    p_peer->restart_time = packet.restart_time;
    p_peer->mpbgp_capability = packet.mpbgp_capability;
    p_peer->as4_enable = packet.as4_enable;

    /*record orf capability*/
    p_peer->orf_remote_recv = packet.orf_recv_capability;
    p_peer->orf_remote_send = packet.orf_send_capability;

    /*clear all previously learnt orf*/
    bgp4_avl_walkup(&p_peer->orf_in_table, bgp4_orf_delete);
    bgp4_avl_walkup(&p_peer->orf_old_in_table, bgp4_orf_delete);
    
    if ((packet.mpbgp_capability == 0)
        && (p_peer->ip.afi == BGP4_PF_IPUCAST))
    {
        flag_set(p_peer->mpbgp_capability, BGP4_PF_IPUCAST);
    }
    
    p_peer->restart_mpbgp_capability = packet.restart_mpbgp_capability;
    
    /*add for cap advertisement,compare remote and local cap*/
    unsupport_capability = TRUE;

    /*MP protocol,if any afi supported, accept peer*/
    if ((p_peer->local_mpbgp_capability == 0)
        || ((p_peer->mpbgp_capability & p_peer->local_mpbgp_capability)))
    {
       unsupport_capability = FALSE;
    }
    /*GR:if both local and peer support GR,accept it*/
    if (gbgp4.restart_enable && p_peer->restart_enable)
    {
       unsupport_capability = FALSE;
    }

    /*refresh:if both local and peer support refresh,accept it*/
    if (p_peer->local_refresh_enable && p_peer->refresh_enable)
    {
       unsupport_capability = FALSE;
    }

    /*if all capability not support,close peer*/
    if (unsupport_capability == TRUE)
    {
        p_peer->unsupport_capability = TRUE;

        err_len = bgp4_open_capablilty_fill(p_peer, errmsg);
        p_peer->notify.code = BGP4_OPEN_MSG_ERR;
        p_peer->notify.sub_code = BGP4_UNSUPPORTED_CAPABILITY;
        p_peer->notify.len = err_len%64; 
        memcpy(p_peer->notify.data, errmsg, p_peer->notify.len);
                 
        goto CLOSE_PEER;
    }

    bgp4_timer_start(&p_peer->hold_timer, p_peer->neg_hold_interval);
    bgp4_timer_start(&p_peer->keepalive_timer, bgp4_rand(p_peer->neg_keep_interval));
    bgp4_keepalive_output(p_peer);
    
    p_peer->state = BGP4_NS_OPENCONFIRM;
    return;
    
CLOSE_PEER:
    p_peer->stat.open_err++;
    p_peer->notify.close_peer = TRUE;
    return;    
}

extern tBGP4_ATTR_DESC desc[256];
/*translate update msg into internal struct*/
STATUS 
bgp4_update_verify(tBGP4_RXUPDATE_PACKET *p_packet)
{
    tBGP4_PEER *p_peer = p_packet->p_peer;
    u_char *p_msg = p_packet->p_msg;
    u_char *p_option = NULL;
    u_short len = p_packet->len;
    u_short read_len = 0;
    u_int payload_len;
    u_char hlen = 0;
    u_char flag = 0;
    u_char type = 0;
    u_char type_missed = 0;
    /*
       withdraw_len(2bytes)
       withdraw_route(withdraw_len bytes)
       attribute_len(2bytes)
       attribute(attribute_len bytes)
       feasible_route(rest)
     */
    /*2bytes widraw length*/
    bgp4_get_2bytes(p_packet->withdraw_len, p_msg);        
    if ((p_packet->withdraw_len + 4) > len) 
    {        
        p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
        p_peer->notify.sub_code = BGP4_MALFORMED_ATTR_LIST;
        return VOS_ERR;
    }

    /*set withdraw pointer*/
    p_packet->p_withdraw = p_msg + 2;

    /*get 2 bytes attribute length*/
    bgp4_get_2bytes(p_packet->attribute_len, \
        (p_packet->p_withdraw + p_packet->withdraw_len));   

    /*set attribute pointer*/
    p_packet->p_attribute = p_packet->p_withdraw + p_packet->withdraw_len + 2;

    /*get rest feasible route length*/
    p_packet->nlri_len = len - 
                          p_packet->withdraw_len -
                          p_packet->attribute_len - 4;
    p_packet->p_nlri = p_packet->p_attribute + p_packet->attribute_len;


    /*set each attribute*/
    p_option = p_packet->p_attribute;
    for (; 
         read_len < p_packet->attribute_len; 
         read_len += hlen + payload_len,
         p_option += hlen + payload_len)
    {
        /*decide header length and option length*/
        flag = *p_option;
        type = *(p_option + 1);
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

        /*any attribute can only appear once*/
        if (p_packet->p_option[type])
        {
            bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "attribute %d appear multiple",type);

            p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
            p_peer->notify.sub_code = BGP4_MALFORMED_ATTR_LIST;
            return VOS_ERR;
        }
        p_packet->p_option[type] = p_option;
        p_packet->option_len[type] = payload_len;
        p_packet->option_hlen[type] = hlen;
        
        /*check if flag is correct*/
        if (desc[type].flag
            && (desc[type].flag != (flag & (BGP4_ATTR_FLAG_OPT | BGP4_ATTR_FLAG_TRANSIT))))
        {
            bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR,"invalid attribute %d flag %02x",type, flag);

            p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
            p_peer->notify.sub_code = BGP4_ATTR_FLAG_ERR;
            p_peer->notify.len = (payload_len + hlen)%64; 
            memcpy(p_peer->notify.data, p_option, p_peer->notify.len);
            return VOS_ERR;
        }

        /*check length*/
        if ((desc[type].zerolen && payload_len) 
            || (desc[type].fixlen 
                 && (payload_len != desc[type].fixlen)) 
            || (desc[type].unitlen && (payload_len%desc[type].unitlen)))
        {
            bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR,"invalid attribute %d len %d",type, payload_len);
        
            p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
            p_peer->notify.sub_code = BGP4_ATTR_LEN_ERR;
            p_peer->notify.len = (payload_len + hlen)%64; 
            memcpy(p_peer->notify.data, p_option, p_peer->notify.len);
            return VOS_ERR;
        }        
    }

    /*decide if any fixed option need*/
    /*if there is no feasible routes,may have no path attribute*/
    if ((p_packet->nlri_len == 0)
       && (!p_packet->p_option[BGP4_ATTR_MP_NLRI]))
    {
        return VOS_OK;
    }
    
    /*in cisco,when mpbgp update sent,do not include nexthop attribute!,so in this case
        do not return failed.ipsoft ddp 2004-11-29*/
    if (!p_packet->p_option[BGP4_ATTR_ORIGIN] 
        && !p_packet->p_option[BGP4_ATTR_MP_UNREACH_NLRI])
    {    
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "origin option not appear");
        
        type_missed = BGP4_ATTR_ORIGIN;
    }

    if (!type_missed 
        && !p_packet->p_option[BGP4_ATTR_PATH] 
        && !p_packet->p_option[BGP4_ATTR_MP_UNREACH_NLRI])
    {        
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "path option not appear");
        
        type_missed = BGP4_ATTR_PATH;
    }

    if (!type_missed 
        && !p_packet->p_option[BGP4_ATTR_NEXT_HOP] 
        && !p_packet->p_option[BGP4_ATTR_MP_UNREACH_NLRI] 
        && !p_packet->p_option[BGP4_ATTR_MP_NLRI])
    {    
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "nexthop option not appear");
    
        type_missed = BGP4_ATTR_NEXT_HOP;
    }

    if (type_missed)
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR,"missing wellknown attribtue %d",type_missed);

        p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
        p_peer->notify.sub_code = BGP4_MISSING_WELLKNOWN_ATTR;
        p_peer->notify.len = 1; 
        memcpy(p_peer->notify.data, &type_missed, 1);
        return VOS_ERR;
    }
    /*add by cheng to check lp attribute*/
    if (!p_packet->p_option[BGP4_ATTR_LOCAL_PREF]
        && (bgp4_peer_type(p_peer) == BGP4_IBGP)
        && !p_packet->p_option[BGP4_ATTR_MP_UNREACH_NLRI])
    {
        bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR,"missing wellknown attribtue %d",type_missed);

        type_missed = BGP4_ATTR_LOCAL_PREF;

        p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
        p_peer->notify.sub_code = BGP4_MISSING_WELLKNOWN_ATTR;
        p_peer->notify.len = 1; 
        memcpy(p_peer->notify.data, &type_missed, 1);
        return VOS_ERR;
    }
    return VOS_OK;
}

/*get route information from update msg*/

STATUS 
bgp4_update_decode_route(
      tBGP4_RXUPDATE_PACKET *p_packet,
      avl_tree_t *p_flist,
      avl_tree_t *p_wlist)
{
    tBGP4_PATH path;
    tBGP4_PATH *p_new_path = NULL;
    tBGP4_PEER *p_peer = p_packet->p_peer;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_next = NULL;
    tBGP4_ASPATH *p_aspath = NULL;
    u_short *p_as = NULL;
    u_int type = 0;
    u_int unkown_len = 0;
    u_char unkown_option[BGP4_MAX_UNKOWN_OPT_LEN];
    
    /*prepare set path struct*/
    memset(&path, 0, sizeof(path));

    bgp4_path_init(&path);
    
    /*default IPv4 unicast af*/
    path.af = BGP4_PF_IPUCAST;
    path.p_peer = p_peer;
    path.p_instance = p_peer->p_instance;
    path.origin_vrf = p_peer->p_instance->vrf;

    /*extract path attribute.do not obtain any mp route here*/
    if (bgp4_origin_extract(&path, p_packet) != VOS_OK)
    {
        goto ERROR_END;
    }

    /*2bytes as path*/
    if (((gbgp4.as4_enable == FALSE) || (p_peer->as4_enable == FALSE))
        || (gbgp4.work_mode == BGP4_MODE_SLAVE))
    {
        if (bgp4_aspath_extract(&path, p_packet) != VOS_OK)
        {
            goto ERROR_END;
        }
    }

    /*for EBGP peer,the first AS number must be peer's as number*/
    if (bgp4_peer_type(p_peer) == BGP4_EBGP)
    {
        p_aspath = bgp4_avl_first(&path.aspath_list);
        if (p_aspath)
        {
            p_as = (u_short *)p_aspath->as;
            if (ntohs(*p_as) != p_peer->as)
            {
                p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
                p_peer->notify.sub_code = BGP4_MALFORMED_AS_PATH;
                goto ERROR_END;
            }
        }
    }
    
    /*4bytes aspath*/
    if (bgp4_as4path_extract(&path, p_packet) != VOS_OK)
    {
        goto ERROR_END;
    }
    
    if (bgp4_nexthop_extract(&path, p_packet) != VOS_OK)
    {
        goto ERROR_END;
    }

    if (bgp4_med_extract(&path, p_packet) != VOS_OK)
    {
        goto ERROR_END;
    }

    if (bgp4_lpref_extract(&path, p_packet) != VOS_OK)
    {
        goto ERROR_END;
    }
    
    if (bgp4_atomic_extract(&path, p_packet) != VOS_OK)
    {
        goto ERROR_END;
    }
    
    if (bgp4_aggregator_extract(&path, p_packet) != VOS_OK)
    {
        goto ERROR_END;
    }

    if (bgp4_community_extract(&path, p_packet) != VOS_OK)
    {
        goto ERROR_END;
    }
    if (bgp4_originator_extract(&path, p_packet) != VOS_OK)
    {
        goto ERROR_END;
    }

    if (bgp4_cluster_extract(&path, p_packet) != VOS_OK)
    {
        goto ERROR_END;
    }

    if (bgp4_ecommunity_extract(&path, p_packet) != VOS_OK)
    {
        goto ERROR_END;
    }

    /*record all un-recognized attribute*/
    for (type = 0; type < 255 ; type++)
    {
        if (p_packet->p_option[type] == NULL)
        {
            continue;
        }
        switch (type){
            case BGP4_ATTR_ORIGIN:
            case BGP4_ATTR_PATH:
            case BGP4_ATTR_NEXT_HOP:
            case BGP4_ATTR_MED:
            case BGP4_ATTR_LOCAL_PREF:
            case BGP4_ATTR_ATOMIC_AGGR:
            case BGP4_ATTR_AGGREGATOR:
            case BGP4_ATTR_COMMUNITY:
            case BGP4_ATTR_ORIGINATOR:
            case BGP4_ATTR_CLUSTERLIST:
            case BGP4_ATTR_UNKNOWN:
            case BGP4_ATTR_MP_NLRI:
            case BGP4_ATTR_MP_UNREACH_NLRI:
            case BGP4_ATTR_EXT_COMMUNITY:
            case BGP4_ATTR_NEW_PATH:
            case BGP4_ATTR_NEW_AGGREGATOR:
                 break;

            default:
                 if ((p_packet->option_hlen[type] +
                      p_packet->option_len[type] +
                      unkown_len) > BGP4_MAX_UNKOWN_OPT_LEN)
                 {
                     break;
                 }
                 /*copy*/
                 memcpy(unkown_option + unkown_len, 
                        p_packet->p_option[type],
                        p_packet->option_hlen[type] +
                        p_packet->option_len[type]);
                 unkown_len += p_packet->option_hlen[type] + p_packet->option_len[type];
                 break;
        }
    }
    /*save unkown option*/
    if (unkown_len)
    {
        path.p_unkown = bgp4_malloc(unkown_len, MEM_BGP_BUF);
        if (path.p_unkown)
        {
            memcpy(path.p_unkown, unkown_option, unkown_len);
            path.unknown_len = unkown_len;
        }
    }
    
    /*MP/unreach,only path attribute need,nlri do not parsed here*/
    if (bgp4_mpreach_extract(&path, p_packet) != VOS_OK)
    {
        goto ERROR_END;
    }

    if (bgp4_mpunreach_extract(&path, p_packet) != VOS_OK)
    {
        goto ERROR_END;
    }

    /*unkown.todo*/

    /*insert path to global path table*/
    p_new_path = bgp4_path_add(&path); 
    if (p_new_path == NULL)
    {
        goto ERROR_END;
    }
    bgp4_path_clear(&path);

    /*decide direct nexthop for feasible route*/
    if (bgp_ip4(p_new_path->nexthop.ip) != 0)
    {
        bgp4_direct_nexthop_calculate(p_new_path);
    }    
    /*parse route*/
    if (bgp4_nlri_extract(p_new_path, 
                 p_packet->p_withdraw, 
                 p_packet->withdraw_len,
                 p_wlist) != VOS_OK)
    {
        goto ERROR_END;
    }
    if (bgp4_nlri_extract(p_new_path, 
                 p_packet->p_nlri, 
                 p_packet->nlri_len, 
                 p_flist) != VOS_OK)
    {
        goto ERROR_END;
    }

    if (bgp4_mpreach_nlri_extract(p_new_path, 
            p_packet, p_flist) != VOS_OK)
    {
        goto ERROR_END;
    }
    
    if (bgp4_mpunreach_nlri_extract(p_new_path, 
            p_packet, p_wlist) != VOS_OK)
    {
        goto ERROR_END;
    }
    /*parse success*/
    return VOS_OK;
 ERROR_END:
    p_peer->stat.update_err++; 
    p_peer->notify.close_peer = TRUE;      

    bgp4_avl_for_each_safe(p_flist, p_route, p_next)
    {
        bgp4_route_table_delete(p_route);
    }
    bgp4_avl_for_each_safe(p_wlist, p_route, p_next)
    {
        bgp4_route_table_delete(p_route);
    }
    bgp4_path_clear(&path);
    return VOS_ERR;
}

void 
bgp4_update_input(
      tBGP4_PEER *p_peer,
      u_char *p_pdu)
{
    tBGP4_RXUPDATE_PACKET packet;
    tBGP4_MSGHDR hdr;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_next = NULL;
    avl_tree_t flist;
    avl_tree_t wlist;
    u_int tv = vos_get_system_tick();
    u_int len = 0;
    u_char str[64];
    u_char pstr[64];

    memcpy(&hdr, p_pdu, sizeof(tBGP4_MSGHDR));           
    len = ntohs(hdr.len);

    bgp4_log((BGP_DEBUG_CMN|BGP_DEBUG_UPDATE), "receive update msg from %s,length %d", bgp4_printf_peer(p_peer, str), len);
    
    if ((p_peer->state != BGP4_NS_ESTABLISH) 
        && (gbgp4.work_mode != BGP4_MODE_SLAVE))
    {
        bgp4_log((BGP_DEBUG_CMN|BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR), "peer's state is not found");

        p_peer->stat.update_err++; 
        if (p_peer->state >= BGP4_NS_OPENSENT)
        {
           p_peer->notify.code = BGP4_FSM_ERR;
        }
        p_peer->notify.close_peer = TRUE;
        return;
    }
    p_peer->stat.rx_update++;
    p_peer->stat.rx_updatetime = time(NULL);

    bgp4_unsort_avl_init(&flist);
    bgp4_unsort_avl_init(&wlist);

    p_pdu += BGP4_HLEN;
    len -= BGP4_HLEN;

    /*check if rcvd update is an end-of-rib mark,if so,obtian related
        afi/safi,and enter graceful-restart process*/        
    if (bgp4_end_of_rib_check(p_peer, p_pdu, len) == TRUE)
    {
        return;
    }

    /*decode update packet*/
    memset(&packet, 0, sizeof(packet));
    packet.p_peer = p_peer;
    packet.p_msg = p_pdu;
    packet.len = len;

    if (bgp4_update_verify(&packet) != VOS_OK)
    {
        p_peer->stat.update_err++; 
        p_peer->notify.close_peer = TRUE;  
        return;
    }
    
    /*parse update msg*/
    if (bgp4_update_decode_route(&packet, &flist, &wlist) != VOS_OK)
    {
        p_peer->stat.update_err++; 
        p_peer->notify.close_peer = TRUE;    
        return;
    }
    
    /*add by cheng to implement debug print*/
    bgp4_display_update_input(p_peer, &flist, &wlist);

    p_peer->stat.rx_fea_route += bgp4_avl_count(&flist);
    p_peer->stat.rx_with_route += bgp4_avl_count(&wlist);    

    bgp4_log(BGP_DEBUG_UPDATE, "update rib from peer %s,w=%d,f=%d",
                    bgp4_printf_peer(p_peer, pstr),
                    bgp4_avl_count(&wlist),
                    bgp4_avl_count(&flist));
    
    bgp4_avl_for_each_safe(&wlist, p_route, p_next)
    {
        p_route->is_deleted = TRUE;
        
        bgp4_vrf_route_export_check(p_route, FALSE);

        bgp4_rib_in_table_update(p_route);

        bgp4_route_table_delete(p_route);
    }

    p_route = bgp4_avl_first(&flist);
    if (p_route)
    {
        /*validate path for feasible routes*/
        if (bgp4_path_loop_check(p_route->p_path) != VOS_OK)
        {
            bgp4_log(BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR, "loop detected,discard feasible rouet");
            
            bgp4_avl_for_each_safe(&flist, p_route, p_next)
            {
                bgp4_route_table_delete(p_route);
            }
            return;
        }
    }
    bgp4_avl_for_each_safe(&flist, p_route, p_next)
    {
        /*check origin import route policy*/
        if (bgp4_import_route_policy_apply(p_route) == BGP4_ROUTE_DENY)
        {
            bgp4_log(BGP_DEBUG_UPDATE,"route %s is filtered by route policy",
                        bgp4_printf_route(p_route, str));
            
            gbgp4.stat.import_filtered_route++;
            bgp4_route_table_delete(p_route);
            continue;
        }
        bgp4_vrf_route_export_check(p_route, TRUE);

        bgp4_rib_in_table_update(p_route);

        bgp4_route_table_delete(p_route);
    }
                        
    if ((p_peer->current_prefix > p_peer->max_prefix)
        && p_peer->max_prefix)
    {
        bgp4_log(BGP_DEBUG_CMN|BGP_DEBUG_ERROR, "rib count exceed limit");
        
        p_peer->notify.code = BGP4_CEASE;
        p_peer->notify.sub_code = BGP4_OVER_MAX_PREFIX;
        return;
    }
    
    tv = vos_get_system_tick() - tv;
    if (tv > p_peer->stat.max_rx_updatetime)
    {
        p_peer->stat.max_rx_updatetime = tv;
    }
    bgp4_timer_start(&p_peer->hold_timer, p_peer->neg_hold_interval);
    return;
}

void
bgp4_notify_input(
      tBGP4_PEER *p_peer,
      u_char *p_pdu)
{
    tBGP4_NOTIFYMSG notify;
    u_char err_str[32];                         
    u_char suberr_str[64];
    u_char addstr[64];

    memcpy(&notify, p_pdu + BGP4_HLEN, sizeof(notify));
    
    if (gbgp4.debug & (BGP_DEBUG_CMN|BGP_DEBUG_EVT))
    {
        bgp4_printf_notify(notify.code, notify.sub_code, err_str, suberr_str);

        bgp4_log(BGP_DEBUG_CMN|BGP_DEBUG_ERROR, "received notification from %s :e=%d(%s),s=%d(%s).",
            bgp4_printf_peer(p_peer, addstr),
            notify.code, err_str,
            notify.sub_code, suberr_str);
    } 
    
    p_peer->stat.notify_rx++; 

    /*ignored when state is invalid*/
    if ((p_peer->state != BGP4_NS_OPENSENT) 
        && (p_peer->state != BGP4_NS_OPENCONFIRM) 
        && (p_peer->state != BGP4_NS_ESTABLISH))
    {
        return;
    }

    p_peer->stat.last_errcode = notify.code;
    p_peer->stat.last_subcode = notify.sub_code;

    if ((notify.code == BGP4_OPEN_MSG_ERR)
        && (notify.sub_code == BGP4_OPT_PARM_UNRECOGNIZED))
    {
        p_peer->option_tx_fail = TRUE;
    }

    /*wait a little time ,just a test wait for low layer tcp end connection*/
    vos_pthread_delay(30);
   
    /*when recieving notification,do not enter graceful-restart process*/
    bgp4_peer_restart_finish(p_peer);

    bgp4_fsm(p_peer, BGP4_EVENT_FATAL_ERROR); 
    return;
}

void 
bgp4_keepalive_input(
      tBGP4_PEER *p_peer,
      u_char *p_pdu)
{
    u_char str[64];
    u_char stateStr[32];
    u_int af = 0 ;
    u_short errcode = 0;

    bgp4_log(BGP_DEBUG_CMN, "receive keepalive information from %s.", bgp4_printf_peer(p_peer, str));
    
    p_peer->stat.keepalive_rx++; 
    
    if ((p_peer->state != BGP4_NS_OPENCONFIRM) 
        && (p_peer->state != BGP4_NS_ESTABLISH))
    {
        bgp4_log(BGP_DEBUG_CMN|BGP_DEBUG_ERROR, "peer state invalid for keepalive");

        if (p_peer->state >= BGP4_NS_OPENSENT)
        {
            p_peer->notify.code = BGP4_FSM_ERR;
        }
        p_peer->notify.close_peer = TRUE;
        return;
    }
    
    bgp4_timer_start(&p_peer->hold_timer, p_peer->neg_hold_interval);

    /*special process for openconfirm state*/
    if (p_peer->state == BGP4_NS_OPENCONFIRM)
    {
        errcode = p_peer->stat.last_errcode;
        errcode = (errcode << 8) + p_peer->stat.last_subcode;
         
        p_peer->stat.established_transitions ++;                 
        //p_peer->stat.rx_update = 0;
        //p_peer->stat.tx_update = 0;
        p_peer->stat.uptime = time(NULL) ;
        
        bgp4_log(BGP_DEBUG_FSMCHANGE,"peer %s change from %s to established", bgp4_printf_peer(p_peer, str), bgp4_printf_state(p_peer->state,stateStr));
        
        p_peer->state = BGP4_NS_ESTABLISH;

        if (p_peer->bfd_enable && (p_peer->bfd_discribe <= 0))
        {                           
            bgp4_bind_bfd(p_peer);
        }
        
        if (gbgp4.in_restarting == FALSE)
        {    
            bgp4_log(BGP_DEBUG_CMN, "can't in restarting,prepare init update");
            for (af = 0 ; af < BGP4_PF_MAX ; af++)
            {
                /*send refresh msg if need*/
                if (flag_isset(p_peer->orf_send, af) 
                    && flag_isset(p_peer->orf_remote_recv, af))
                {
                    bgp4_refresh_output(p_peer, af, &p_peer->orf_out_table);
                }
                
                /*if local support orf rx,remote support orf tx,wait for refresh msg*/
                if (flag_isset(p_peer->orf_recv, af) 
                    && flag_isset(p_peer->orf_remote_send, af))
                {
                    continue;
                }
                bgp4_schedule_init_update(af, p_peer);
            }                        
        }
        
        if (gbgp4.trap_enable)
        {
            sendBgpEstablishedTrap(
                 (p_peer->ip.afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6,
                  p_peer->ip.ip, p_peer->state, (char *)&errcode);
        }
    }
    return;
}

/*extract orf table from refresh msg*/
void
bgp4_refresh_orf_extract(
                u_int af,
                u_char *p_buf,
                u_short len,
                avl_tree_t *p_table)
{
    tBGP4_ORF_ENTRY *p_orf = NULL;
    tBGP4_ORF_ENTRY dummy;
    u_int seqnum = 0;
    u_short readlen = 0;
    u_short orf_len = 0;
    u_char flag = 0;
    u_char action = 0;
    u_char byte = 0;
    u_char str[128];
    
    /*ORF entry
         +---------------------------------+
         |   Action (2 bit)                |
         +---------------------------------+
         |   Match (1 bit)                 |
         +---------------------------------+
         |   Reserved (5 bits)             |
         +--------------------------------+
         |   Sequence (4 octets)          |
         +--------------------------------+
         |   Minlen   (1 octet)           |
         +--------------------------------+
         |   Maxlen   (1 octet)           |
         +--------------------------------+
         |   Length   (1 octet)           |
         +--------------------------------+
         |   Prefix   (variable length)   |
         +--------------------------------+
    */
    for (; readlen < len ; readlen += orf_len)
    {
        orf_len = 0;
        
        memset(&dummy, 0, sizeof(dummy));
        flag = *p_buf;
        p_buf++;
        orf_len++;

        action = flag >> 6;
        /*special..remove all*/
        if (action == BGP4_ORF_ACTION_REMOVE_ALL)
        {
            bgp4_orf_create(p_table, af, 0xffffffff);

            bgp4_log(BGP_DEBUG_CMN, "remove all learnt orf");
            break;
        }
        /*get seq*/
        bgp4_get_4bytes(seqnum, p_buf);
        p_buf += 4;
        orf_len += 4;
        
        /*create new orf*/
        p_orf = bgp4_orf_create(p_table, af, seqnum);
        if (p_orf == NULL)
        {
            break;
        }

        /*set field*/
        if (action == BGP4_ORF_ACTION_REMOVE)
        {
            p_orf->wait_delete = TRUE;
        }

        p_orf->match = (flag >> 5) & 0x01;
        
        p_orf->minlen = *p_buf;
        p_buf++;
        orf_len++;

        p_orf->maxlen = *p_buf;
        p_buf++;
        orf_len++;

        p_orf->prefix.prefixlen = *p_buf;
        p_buf++;
        orf_len++;

        byte = bgp4_bit2byte(p_orf->prefix.prefixlen);
        if (byte)
        {
            memcpy(p_orf->prefix.ip, p_buf, byte);
            p_buf += byte;
            orf_len += byte;
        }
        bgp4_log(BGP_DEBUG_CMN, "get orf %d,%s/%d,action %s,match %s", 
                 p_orf->seqnum,
                 bgp4_printf_addr(&p_orf->prefix, str),
                 p_orf->prefix.prefixlen,
                 (action == BGP4_ORF_ACTION_REMOVE) ? "remove" : "add",
                 (p_orf->match == BGP4_ORF_MATCH_DENY) ? "deny" : "permit");
    }
    return;
}

void 
bgp4_refresh_input(
      tBGP4_PEER *p_peer,
      u_char *p_pdu)
{
    tBGP4_MSGHDR hdr;
    avl_tree_t orf_table;
    u_short afi = 0;
    u_char safi = 0;
    u_int flag = 0;
    u_int len = 0;
    u_char when_to_refresh = 0;
    u_char orf_type = 0;
    u_char str[64];
    
    memcpy(&hdr, p_pdu, sizeof(tBGP4_MSGHDR));           
    len = ntohs(hdr.len);
    bgp4_log(BGP_DEBUG_CMN,"receive route refresh msg from %s,length %d" , bgp4_printf_peer(p_peer, str), len);
    
    if (p_peer->state != BGP4_NS_ESTABLISH)
    {
        return;
    }

    p_pdu += BGP4_HLEN;
    len -= BGP4_HLEN;

    /*check if peer support,if not ignored*/        
    if ((p_peer->local_refresh_enable == FALSE) 
        || (p_peer->refresh_enable == FALSE))
    {
        /*if peer support orf sending,accept this refresh msg*/
        if (p_peer->orf_recv && p_peer->orf_remote_send)
        {
           ;
        }
        else
        {
            bgp4_log(BGP_DEBUG_CMN,"peer %s not support route refresh", bgp4_printf_peer(p_peer, str));
            
            return;
        }
    }
    /*4byte: af af 0 safi*/
    bgp4_get_2bytes(afi, p_pdu);
    safi = *(p_pdu + 3);
    flag = bgp4_afi_to_index(afi, safi);

    bgp4_log(BGP_DEBUG_CMN, "get afi/safi of refresh msg: %d/%d", afi, safi);
    
    if (!bgp4_af_support(p_peer, flag))
    {
        bgp4_log(BGP_DEBUG_CMN, "peer %s can not support this afi/safi", bgp4_printf_peer(p_peer, str));
        return;
    }
    /*decode orf if exist*/
    if (len == 4)
    {
        bgp4_schedule_init_update(flag, p_peer);
        return;
    }
    /*ORF processing*/
    bgp4_unsort_avl_init(&orf_table);

    when_to_refresh = *(p_pdu + 4);
    orf_type = *(p_pdu + 5);
    
    bgp4_log(BGP_DEBUG_CMN, "get when to refresh %d, orf type %d", when_to_refresh, orf_type);

    if (orf_type == BGP4_ORF_TYPE_IPPREFIX)
    {
        bgp4_refresh_orf_extract(flag, p_pdu + 8, len - 8, &orf_table);
    }
    /*update orf table*/
    bgp4_peer_orf_update(p_peer, flag, &orf_table);

    /*release orf created*/
    bgp4_avl_walkup(&orf_table, bgp4_orf_delete);
    
    if (when_to_refresh == BGP4_ORF_FRESH_IMMEDIATE)
    {
        bgp4_schedule_init_update(flag, p_peer);
    }
    return;
}

/*message input process*/
void 
bgp4_msg_input(
      tBGP4_PEER *p_peer,
      u_char *p_pdu)
{
    tBGP4_MSGHDR hdr; 
    u_int len = 0 ;

    /*reset current notify buffer*/
    memset(&p_peer->notify, 0, sizeof(&p_peer->notify));
    
    /*verrify msg before processing*/
    if (bgp4_msg_verify(p_peer, p_pdu) != VOS_OK)
    {
        goto FINSH;
    }

    memcpy(&hdr, p_pdu, sizeof(tBGP4_MSGHDR));

    switch (hdr.type) {
        case BGP4_MSG_OPEN:
             bgp4_open_input(p_peer, p_pdu);
             break;

        case BGP4_MSG_UPDATE:
             bgp4_update_input(p_peer, p_pdu);
             break;

        case BGP4_MSG_NOTIFY:
             bgp4_notify_input(p_peer, p_pdu);
             break;

        case BGP4_MSG_KEEPALIVE:
             bgp4_keepalive_input(p_peer, p_pdu);
             break;

        case BGP4_MSG_REFRESH:
             bgp4_refresh_input(p_peer, p_pdu); 
             break;

        default:
             break;
    }

FINSH:
    if (p_peer->notify.code || p_peer->notify.close_peer)
    {
        if (p_peer->notify.code)
        {
            bgp4_notify_output(p_peer);
        }
        if (p_peer->notify.close_peer)
        {
            bgp4_fsm(p_peer, BGP4_EVENT_FATAL_ERROR);
        }
        return;
    }
    
    /*************for sync update route***************/
    len = ntohs(hdr.len);
    if (gbgp4.work_mode == BGP4_MODE_MASTER)
    {
        if (len > BGP4_SYNC_MAX_UPDATE_SIZE)
        {
            gbgp4.stat.msg_rx.long_update++;
            bgp4_sync_fragment_pdu_send(p_peer, p_pdu, len);
        }
        else
        {
            bgp4_sync_pdu_send(p_peer, p_pdu, len);
        }
    }
    return;
}

/*open message tx&rx*/
void 
bgp4_open_output(tBGP4_PEER *p_peer)
{
    u_char buf[BGP4_MAX_MSG_LEN];
    tBGP4_MSGHDR *p_hdr = (tBGP4_MSGHDR *)buf;
    u_char *p_fill = NULL;
    u_int as = 0;
    u_char opt_len = 0; 
    u_char str[64];
    
    bgp4_init_msg_hdr(buf, BGP4_MSG_OPEN); 
    
    /*move to open hdr*/    
    p_fill = buf + BGP4_HLEN;
    
    bgp4_put_char(p_fill, BGP4_VERSION_4);

    /*decide as number in open,use local as default*/
    as = gbgp4.as;
    
    /*changed by cheng to enable change the local-as of a EBGP peer*/
    /*IPSOFT cheng add for confederation*/
    if ((bgp4_peer_type(p_peer) == BGP4_EBGP))
    {
        if (p_peer->fake_as != gbgp4.as)
        {
            as = p_peer->fake_as;
        }
        else if (gbgp4.confedration_id)
        {
            as = gbgp4.confedration_id;
        }
    }

    /*4bytes as support*/
    if (gbgp4.as4_enable && (as > 0x0000ffff))
    {
        as = BGP4_AS_TRANS;
    }
    
    bgp4_put_2bytes(p_fill, as);
    
    bgp4_put_2bytes(p_fill, p_peer->hold_interval);    
    
    bgp4_put_4bytes(p_fill, gbgp4.router_id);
    
    /*form optional params skip option length filed*/
    opt_len = bgp4_open_capablilty_fill(p_peer, p_fill + 1);
    
    /*fill opt length*/
    *p_fill = opt_len;
    
    /*open fix length + opt length*/           
    p_hdr->len = htons(BGP4_OPEN_HLEN + opt_len);  
        
    bgp4_log(BGP_DEBUG_CMN, "send open to %s,length %d",
        bgp4_printf_peer(p_peer, str), ntohs(p_hdr->len));
        
    bgp4_peer_sock_send(p_peer, buf, ntohs(p_hdr->len));
        
    p_peer->stat.open_tx++;    
    return;
}

/*keepalive messge send*/
void 
bgp4_keepalive_output(tBGP4_PEER *p_peer)
{
    u_char buf[BGP4_KEEPALIVE_LEN];
    tBGP4_MSGHDR *p_hdr = (tBGP4_MSGHDR *)buf; 
    u_char str[64];

    bgp4_log(BGP_DEBUG_CMN,"send keepalive msg to %s." , bgp4_printf_peer(p_peer, str));
        
    bgp4_init_msg_hdr(buf, BGP4_MSG_KEEPALIVE);
    p_hdr->len = htons(BGP4_HLEN);
   
    bgp4_peer_sock_send(p_peer, buf, BGP4_HLEN); 
    p_peer->stat.keepalive_tx++;    
    return ;
}

/*notify msg r&t*/
void  
bgp4_notify_output(tBGP4_PEER *p_peer)
{
    u_char buf[BGP4_MAX_MSG_LEN];
    tBGP4_MSGHDR *p_hdr = (tBGP4_MSGHDR *)buf; 
    tBGP4_NOTIFYMSG notify; 
    u_int len = BGP4_HLEN ;
    u_char addstr[64];
    u_char err_str[32];                         
    u_char suberr_str[64];
    
    /*msg hdr*/
    bgp4_init_msg_hdr(buf, BGP4_MSG_NOTIFY);

    /*notify header*/
    notify.code = p_peer->notify.code;
    notify.sub_code = p_peer->notify.sub_code;    
    memcpy(buf + BGP4_HLEN, &notify, sizeof(notify));
    len += sizeof(notify);
    
    /*copy error data*/
    if (p_peer->notify.len) 
    {
        memcpy(buf + len, p_peer->notify.data, p_peer->notify.len);
        len += p_peer->notify.len;
    }
    
    p_hdr->len = htons(len);
    
    if (gbgp4.debug & BGP_DEBUG_CMN)
    {
        bgp4_printf_notify(notify.code, notify.sub_code, err_str, suberr_str);
        bgp4_log(BGP_DEBUG_CMN,"send notify to %s.error:%d(%s),suberror:%d(%s).",
                bgp4_printf_peer(p_peer,addstr),
                notify.code,
                err_str,
                notify.sub_code,
                suberr_str);
    }  
    
    /*if sending notification,do not enter graceful-restart process ddp */ 
    bgp4_peer_restart_finish(p_peer);
    bgp4_peer_sock_send(p_peer, buf, len);

    p_peer->stat.last_errcode = notify.code;
    p_peer->stat.last_subcode = notify.sub_code;
    p_peer->stat.notify_tx++;    

    /*clear notify*/
    p_peer->notify.code = 0;
    p_peer->notify.sub_code = 0;
    p_peer->notify.len = 0;

    /*wait for peer accept this msg*/
    vos_pthread_delay(1);
    return;
}

#define ORF_HDR_FILL(buf, action, match) do{*(buf) = (((action)<<6) + ((match) << 5));}while(0)

void 
bgp4_refresh_output(
     tBGP4_PEER *p_peer,
     u_int af_flag,
     avl_tree_t *p_orf_table/*ORF support*/)
{
    u_char buf[BGP4_MAX_MSG_LEN];
    tBGP4_MSGHDR *p_hdr = (tBGP4_MSGHDR *)buf;
    tBGP4_ORF_ENTRY *p_orf = NULL;
    u_char *p_fill = NULL;
    u_char *p_orf_len = NULL;
    u_short len = BGP4_HLEN;
    u_char byte = 0;
    u_char addrstr[64];
    u_short afi = 0;
    u_short safi = 0;
    u_short orf_len = 0;
    
    bgp4_init_msg_hdr(buf, BGP4_MSG_REFRESH);   
    
    /*fill afi/.safi*/
    p_fill = buf + BGP4_HLEN;/*skip common header*/

    /*body length is 4bytes,clear firstly*/
    memset(p_fill, 0, 4);    
    
    /*afi:2bytes*/
    afi = bgp4_index_to_afi(af_flag);
    bgp4_fill_2bytes(p_fill, afi);

    /*rsvd:1byte,skipped*/
    
    /*safi:1bytes*/
    safi = bgp4_index_to_safi(af_flag);
    *(p_fill + 3) = safi;
    len += 4;/*4bytes filed*/

    /*ORF fill if necessary
      orf part format
         +--------------------------------------------------+
         | When-to-refresh (1 octet)                        |
         +--------------------------------------------------+
         | ORF Type (1 octet)                               |
         +--------------------------------------------------+
         | Length of ORF entries (2 octets)                 |
         +--------------------------------------------------+
         | First ORF entry (variable)                       |
         +--------------------------------------------------+
         | Second ORF entry (variable)                      |
         +--------------------------------------------------+
         | ...                                              |
         +--------------------------------------------------+
         | N-th ORF entry (variable)                        |

         ORF entry
         +---------------------------------+
         |   Action (2 bit)                |
         +---------------------------------+
         |   Match (1 bit)                 |
         +---------------------------------+
         |   Reserved (5 bits)             |
         +--------------------------------+
         |   Sequence (4 octets)          |
         +--------------------------------+
         |   Minlen   (1 octet)           |
         +--------------------------------+
         |   Maxlen   (1 octet)           |
         +--------------------------------+
         |   Length   (1 octet)           |
         +--------------------------------+
         |   Prefix   (variable length)   |
         +--------------------------------+
    */
    if (flag_isset(p_peer->orf_remote_recv, af_flag)
        && p_orf_table
        && bgp4_avl_count(p_orf_table))
    {
        p_fill += 4;

        /*when to refresh:always immidate now*/
        *p_fill = BGP4_ORF_FRESH_IMMEDIATE;
        p_fill++;
        len++;
        
        /*orf type:only ip prefix supported*/
        *p_fill = BGP4_ORF_TYPE_IPPREFIX;
        p_fill++;
        len++;

        /*orf length not deicided.skip it*/  
        p_orf_len = p_fill;
        p_fill += 2;
        len += 2;

        /*fill each orf entry*/
        bgp4_avl_for_each(p_orf_table, p_orf)
        {
            /*if seqnum is max vaule,delete all orf*/
            if (p_orf->seqnum == 0xffffffff)
            {
                /*only head need
                  +---------------------------------+
                  |   Action (2 bit)                |
                  +---------------------------------+
                  |   Match (1 bit)                 |
                  +---------------------------------+
                  |   Reserved (5 bits)             |
                  +--------------------------------+
                */
                ORF_HDR_FILL(p_fill, BGP4_ORF_ACTION_REMOVE_ALL, BGP4_ORF_MATCH_PERMIT);
                p_fill++;
                len++;
                orf_len++;
                break;
            }
            /*insert normal orf
              ORF entry
              +---------------------------------+
              |   Action (2 bit)                |
              +---------------------------------+
              |   Match (1 bit)                 |
              +---------------------------------+
              |   Reserved (5 bits)             |
              +--------------------------------+
              |   Sequence (4 octets)          |
              +--------------------------------+
              |   Minlen   (1 octet)           |
              +--------------------------------+
              |   Maxlen   (1 octet)           |
              +--------------------------------+
              |   Length   (1 octet)           |
              +--------------------------------+
              |   Prefix   (variable length)   |
              +--------------------------------+
             */
            if (p_orf->wait_delete == TRUE)
            {
                ORF_HDR_FILL(p_fill, BGP4_ORF_ACTION_REMOVE, p_orf->match);
            }
            else
            {
                ORF_HDR_FILL(p_fill, BGP4_ORF_ACTION_ADD, p_orf->match);
            }
            p_fill++;
            len++;
            orf_len++;

            /*seq*/
            bgp4_fill_4bytes(p_fill, p_orf->seqnum);
            p_fill += 4;
            len += 4;
            orf_len += 4;

            /*min*/
            *p_fill = p_orf->minlen;
            p_fill++;
            len++;
            orf_len++;

            /*max*/
            *p_fill = p_orf->maxlen;
            p_fill++;
            len++;
            orf_len++;

            /*prefix length*/
            *p_fill = p_orf->prefix.prefixlen;
            p_fill++;
            len++;
            orf_len++;

            /*prefix self*/
            byte = bgp4_bit2byte(p_orf->prefix.prefixlen);  /*prefix length bytes*/
            if (byte)
            {
                memcpy(p_fill, p_orf->prefix.ip, byte);
                p_fill += byte;
                len += byte;
                orf_len += byte;
            }
        }

        /*fill orf length*/
        bgp4_fill_2bytes(p_orf_len, orf_len);
    }
    
    p_hdr->len = htons(len);
    
    /*Send*/   
    bgp4_log(BGP_DEBUG_CMN,"send refresh to %s.afi/safi %d/%d", 
            bgp4_printf_peer(p_peer,addrstr),
            afi, safi);
    
    bgp4_peer_sock_send(p_peer, buf, len);
    return;
}
/*
         ORF capability entry....
         +--------------------------------------------------+
         | Address Family Identifier (2 octets)             |
         +--------------------------------------------------+
         | Reserved (1 octet)                               |
         +--------------------------------------------------+
         | Subsequent Address Family Identifier (1 octet)   |
         +--------------------------------------------------+
         | Number of ORFs (1 octet)                         |
         +--------------------------------------------------+
         | ORF Type (1 octet)                               |
         +--------------------------------------------------+
         | Send/Receive (1 octet)                           |
         +--------------------------------------------------+
         | ...                                              |
         +--------------------------------------------------+
         | ORF Type (1 octet)                               |
         +--------------------------------------------------+
         | Send/Receive (1 octet)                           |
         +--------------------------------------------------+
*/
int 
bgp4_orf_capability_extract(
         tBGP4_RXOPEN_PACKET *p_packet,
         u_char *p_opt, 
         u_short len)
{
    tBGP4_AFID af_id;
    u_int readlen = 0;
    u_int orf_len = 0;
    u_int rest_len = 0;
    u_char *p_buf = p_opt;
    u_int orf_count = 0;
    u_int i = 0;
    u_int afflag = 0;
    u_char orf_type = 0;
    u_char orf_flag = 0;
    
    for (; readlen < len; readlen += orf_len)
    {
        /*decide rest length*/
        rest_len = len - readlen;

        /*must include 5bytes fixed length*/
        if (rest_len < 5)
        {
            return VOS_ERR;
        }
        /*get afi/safi*/
        memcpy(&af_id, p_buf, sizeof(af_id));  
        afflag = bgp4_afi_to_index(ntohs(af_id.afi), af_id.safi);
        p_buf += 4;

        /*get orf count*/
        orf_count = *p_buf;
        p_buf++;

        orf_len = (5 + 2*orf_count);

        /*rest length must include all orfs*/
        if (rest_len < orf_len)
        {
            return VOS_ERR;
        }
        /*parse all orf*/
        for (i = 0; i < orf_count ; i++,p_buf += 2)
        {
            orf_type = p_buf[0];
            orf_flag = p_buf[1];
            /*only ip orf expected*/
            if (orf_type == BGP4_ORF_TYPE_IPPREFIX)
            {
                /*set rx/tx flag*/
                if (orf_flag & BGP4_ORF_SEND_FLAG)
                {
                    flag_set(p_packet->orf_send_capability, afflag);
                }
                if (orf_flag & BGP4_ORF_RECV_FLAG)
                {
                    flag_set(p_packet->orf_recv_capability, afflag);
                }
            }
        }
    }
    return VOS_OK;
}

int 
bgp4_open_capablity_extract(
         tBGP4_RXOPEN_PACKET *p_packet,
         u_char *p_opt, 
         u_short opt_len)
{
    u_char type = 0;
    u_char len = 0;
    u_int afflag = 0;
    u_short readlen = 0;
    u_short restart_len = 0;
    u_char *p_restart = NULL;
    u_char in_restart = 0;
    u_short restart_time = 0;
    tBGP4_AFID af_id;
    tBGP4_RESTART_AFID restart_af;
    
    bgp4_log(BGP_DEBUG_EVT, "check open capability,length %d",opt_len);

    for (; readlen < opt_len; readlen += 2 + len, p_opt += len)      
    {
        /*getting capability code:1,MP-BGP 2,Rt refresh*/
        type = p_opt[0];
        len = p_opt[1];
        
        /*skip to value*/
        p_opt += 2;
        
        /*proce length + cap length can not exceed total length*/
        if ((readlen + 2 + len) > opt_len)
        {
            bgp4_log(BGP_DEBUG_EVT|BGP_DEBUG_ERROR, "invalid capability length %d", len); 
            return VOS_ERR;
        }
          
        switch (type){
            case BGP4_CAP_MULTI_PROTOCOL: 
                 if (len != 4)
                 {
                     bgp4_log(BGP_DEBUG_EVT|BGP_DEBUG_ERROR, "invalid bgp4 capability length %d", len);
                     return VOS_ERR;
                 }
                 memcpy(&af_id, p_opt, sizeof(af_id));  

                 /*update peer*/  
                 bgp4_log(BGP_DEBUG_EVT, "check bgp4 capability ,afi/safi %d/%d", ntohs(af_id.afi), af_id.safi);

                 /*update remote peer af flag*/  
                 afflag = bgp4_afi_to_index(ntohs(af_id.afi), af_id.safi);
     
                 flag_set(p_packet->mpbgp_capability, afflag);
     
                 /*add for cap advertisement,by zxq,record remote capability*/
                 break;
              
            case BGP4_CAP_ROUTE_REFRESH: 
                 bgp4_log(BGP_DEBUG_EVT, "check route refresh capability");        
                 if (len != 0)
                 {
                     bgp4_log(BGP_DEBUG_EVT|BGP_DEBUG_ERROR, "invalid rt refresh capability length %d", len);
                     return VOS_ERR;
                 }
                 /*record route refresh*/
                 p_packet->refresh_enable = TRUE;
                 break;

            case BGP4_CAP_GRACEFUL_RESTART:
                 /*validate length and input */
                 if ((len < 2) || (((len - 2) % 4) != 0))          
                 {
                     bgp4_log(BGP_DEBUG_EVT|BGP_DEBUG_ERROR, "restart capability param error");
                     
                     return VOS_ERR;
                 }
                      
                 /*clear last record remote graceful-restart information*/
                 p_packet->restart_enable = FALSE;
                 p_packet->restart_time = 0;
                 p_packet->restart_mpbgp_capability = 0; 

                 /*if local node not support GR,discard silently*/         
                 if (gbgp4.restart_enable != TRUE)         
                 {
                     bgp4_log(BGP_DEBUG_EVT, "restart not configured,ignore it");
                     break;
                 }

                 p_restart = p_opt;

                 /*2bytes common part*/         
                 bgp4_get_2bytes(restart_time, p_restart);
                 p_restart += 2; 
                 restart_len = 2;
                 in_restart = (restart_time & BGP4_GR_RESET_FLAG) == 0 ? FALSE : TRUE;
                 restart_time &= 0x0fff;

                 bgp4_log(BGP_DEBUG_EVT, "check restart capability");   
                 bgp4_log(BGP_DEBUG_EVT, "restart bit %d", in_restart);  
                 bgp4_log(BGP_DEBUG_EVT, "restart time %d",restart_time);

                 p_packet->restart_enable = TRUE;
                 p_packet->in_restart = in_restart;
                 p_packet->restart_time = restart_time;

                 /*scan for all supported af*/
                 for (; restart_len < len ; 
                       restart_len += sizeof(tBGP4_RESTART_AFID),
                       p_restart += sizeof(tBGP4_RESTART_AFID))
                 {
                     memcpy(&restart_af, p_restart, sizeof(tBGP4_RESTART_AFID));
                     restart_af.afi = ntohs(restart_af.afi);
                
                     bgp4_log(BGP_DEBUG_EVT, "check restart af %d/%d,flag %d",
                         restart_af.afi,
                         restart_af.safi,(restart_af.flag & BGP4_GR_FORWARDING_FLAG) == 0 ? 0 : 1);
        
                     if (restart_af.flag & BGP4_GR_FORWARDING_FLAG)
                     {
                         flag_set(p_packet->restart_mpbgp_capability, 
                             bgp4_afi_to_index(restart_af.afi, restart_af.safi));
                     }
                 }                 
                 break;

            case BGP4_CAP_4BYTE_AS:
                 if (len != 4)
                 {
                     bgp4_log(BGP_DEBUG_EVT|BGP_DEBUG_ERROR, "invalid 4 byte as length %d", len);
                     return VOS_ERR;
                 }
                 p_packet->as4_enable = TRUE;
                 if (gbgp4.as4_enable)
                 {
                     bgp4_get_4bytes(p_packet->as, p_opt);
                     
                     bgp4_log(BGP_DEBUG_EVT, "4 byte as %d", p_packet->as);
                 }
                 break;
            /*orf*/     
            case BGP4_CAP_ORF:
                 if (bgp4_orf_capability_extract(p_packet, p_opt, len) != VOS_OK)
                 {
                     return VOS_ERR;
                 }
                 break;
                 
            default:
                bgp4_log(BGP_DEBUG_EVT|BGP_DEBUG_ERROR, "unrecognized capability,code %d,length %d",type,len);              
                break;
        }
    }
    return VOS_OK;
}

/*return option length,buf start from the first option byte.
do not include 1byte option length field self*/
u_short
bgp4_open_capablilty_fill(
       tBGP4_PEER *p_peer, 
       u_char *p_buf)
{
    u_short len = 0;
    u_short afi = 0;
    u_char safi = 0;
    u_char orf_flag = 0;
    u_int i = 0;
    u_char restart_len = 0;
    u_int capability_enable = FALSE;
    u_char *p_len = NULL;
      
    /*add for cap advertisement,by zxq*/      
    if ((p_peer->option_tx_fail == TRUE)
        || (p_peer->local_mpbgp_capability == 0))
    {
        return 0;
    }

    /*in some case,do not send any capability*/
    if (p_peer->local_refresh_enable)
    {
        capability_enable = TRUE;
    }
    if (gbgp4.restart_enable == TRUE)
    {
        capability_enable = TRUE;
    }
    for (i = 0 ; i < BGP4_PF_MAX ; i++)
    {
        if (flag_isset(p_peer->local_mpbgp_capability, i) && (i != BGP4_PF_IPUCAST))
        {
            capability_enable = TRUE;
        }
    }
    
    if (gbgp4.as4_enable == TRUE)
    {
        capability_enable = TRUE;
    }
    
    /*orf capability decision*/
    if (p_peer->orf_recv || p_peer->orf_send)
    {
        capability_enable = TRUE;
    }
    if (capability_enable != TRUE)
    {
        return 0;
    }
    
    /*capability option type --2*/
    bgp4_put_char(p_buf, 2);
    len++;

    /*record capability option length--unkown yet*/   
    p_len = p_buf;
    
    /*fill 0*/
    bgp4_put_char(p_buf, 0);
    len++;

    /*start form mpbgp capability*/
    for (i = 0 ; i < BGP4_PF_MAX ; i++)
    {
        if (flag_isset(p_peer->local_mpbgp_capability, i))
        {
            afi = bgp4_index_to_afi(i);
            safi = bgp4_index_to_safi(i);
            bgp4_put_char(p_buf, BGP4_CAP_MULTI_PROTOCOL); 
            bgp4_put_char(p_buf, 4); 
            bgp4_put_char(p_buf, 0); 
            bgp4_put_char(p_buf, afi); /*current af has only 1byte*/
            bgp4_put_char(p_buf, 0); 
            bgp4_put_char(p_buf, safi); 
            len += 6 ; 
        }
    }

    /*form route refresh capability.orf capability force to enable refresh*/
    if (p_peer->local_refresh_enable || p_peer->orf_recv || p_peer->orf_send)
    {
        bgp4_put_char(p_buf, BGP4_CAP_ROUTE_REFRESH);
        bgp4_put_char(p_buf, 0);  
        len += 2 ; 
    }

    /*form graceful restart capability if enable*/
    if (gbgp4.restart_enable == TRUE)
    {
        restart_len = bgp4_restart_capability_fill(p_peer, p_buf);
        len += restart_len;
        p_buf += restart_len;
    }    

    /*4bytes as support*/
    if (gbgp4.as4_enable == TRUE)
    {
        bgp4_put_char(p_buf, BGP4_CAP_4BYTE_AS);
        bgp4_put_char(p_buf, 4);  
        bgp4_put_4bytes(p_buf, gbgp4.as);  
        len += 6 ; 
    }

    /*ORF capability:only one orf type supported
      format:7bytes
         +--------------------------------------------------+
         | Address Family Identifier (2 octets)             |
         +--------------------------------------------------+
         | Reserved (1 octet)                               |
         +--------------------------------------------------+
         | Subsequent Address Family Identifier (1 octet)   |
         +--------------------------------------------------+
         | Number of ORFs (1 octet)                         |
         +--------------------------------------------------+
         | ORF Type (1 octet)                               |
         +--------------------------------------------------+
         | Send/Receive (1 octet)                           |
     */
    if (p_peer->orf_recv || p_peer->orf_send)
    {
        for (i = 0 ; i < BGP4_PF_MAX ; i++)
        {
            /*decide flag*/
            orf_flag = 0;
            if (flag_isset(p_peer->orf_recv, i))
            {
                orf_flag |= BGP4_ORF_RECV_FLAG;
            }
            if (flag_isset(p_peer->orf_send, i))
            {
                orf_flag |= BGP4_ORF_SEND_FLAG;
            }
            
            if (orf_flag == 0)
            {
                continue;
            }
            
            afi = bgp4_index_to_afi(i);
            safi = bgp4_index_to_safi(i);
            bgp4_put_char(p_buf, BGP4_CAP_ORF); 
            bgp4_put_char(p_buf, 7); 
            /*AF*/
            bgp4_put_char(p_buf, 0); 
            bgp4_put_char(p_buf, afi); /*current af has only 1byte*/
            bgp4_put_char(p_buf, 0); 
            bgp4_put_char(p_buf, safi); 
            /*ORF count,only one*/
            bgp4_put_char(p_buf, 1); 
            
            /*ORF type:only ip prefix used*/
            bgp4_put_char(p_buf, BGP4_ORF_TYPE_IPPREFIX); 

            /*set flag*/                
            bgp4_put_char(p_buf, orf_flag); 

            /*length is fixed*/
            len += 9; 
        }
    }
    
    /*length field do not include 2bytes header*/
    if (len > 2)
    {
        *p_len = (len - 2);
    }
    /*returned total length,include 2bytes hdr*/
    return (len > 2) ? len : 0;
}

/*update message*/
void 
bgp4_display_update_input(
    tBGP4_PEER *p_peer, 
    avl_tree_t *flist, 
    avl_tree_t *wlist)
{
    tBGP4_ROUTE *p_route = NULL;
    u_char addstr[64];
    u_char addstr2[64];
    
    bgp4_log((BGP_DEBUG_CMN|BGP_DEBUG_UPDATE), "recvd update, f=%d,w=%d",
        bgp4_avl_count(flist),
        bgp4_avl_count(wlist));
    
    if (gbgp4.debug & BGP_DEBUG_UPDATE)
    {
        bgp4_avl_for_each(wlist, p_route)
        {
            bgp4_log(BGP_DEBUG_UPDATE," wroute: %s",bgp4_printf_route(p_route,addstr));
        }
        
        bgp4_avl_for_each(flist, p_route)
        {
            bgp4_log(BGP_DEBUG_UPDATE," froute:%s,nexthop %s.",
               bgp4_printf_route(p_route, addstr),
               bgp4_printf_addr(&p_route->p_path->nexthop, addstr2));
        }
    }
    return;
}

/*get ip route from update msg.only for standard ipv4 route*/
int 
bgp4_nlri_extract(
                   tBGP4_PATH *p_path,
                   u_char *p_buf,
                   int len, 
                   avl_tree_t *p_list)
{
    tBGP4_ROUTE *p_route = NULL;
    int bytes = 0; 
    u_int read_len = 0;
    u_int prefix[16];
    u_int dest_ip = 0;

    for (; read_len < len ; read_len += 1 + bytes, p_buf += 1 + bytes)
    {
        p_route = bgp4_malloc(sizeof(tBGP4_ROUTE), MEM_BGP_ROUTE);
        if (p_route == NULL) 
        {
            bgp4_log(BGP_DEBUG_CMN|BGP_DEBUG_ERROR, "not memory for withdrawn");
            return VOS_ERR;
        }
        bgp4_route_table_add(p_list, p_route);

        bgp4_link_path(p_path, p_route);          
             
        p_route->dest.afi = BGP4_PF_IPUCAST;    
        p_route->proto = M2_ipRouteProto_bgp;

        /*get prefix length*/  
        p_route->dest.prefixlen = *p_buf;  
        /*invalid prefix length*/
        if (p_route->dest.prefixlen > 32)
        {
            p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
            p_path->p_peer->notify.sub_code = BGP4_INVALID_NLRI;
            return VOS_ERR;
        }
        
        /*real bytes used*/
        bytes = bgp4_bit2byte(p_route->dest.prefixlen);

        /*buffer length checking*/
        if ((read_len + 1 + bytes) > len) 
        {        
            p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
            p_path->p_peer->notify.sub_code = BGP4_MALFORMED_ATTR_LIST;
            return VOS_ERR;
        }

        /*get prefix*/
        memset(prefix, 0, 16);
        memcpy(prefix, p_buf + 1, bytes);

        /*dest verify*/
        bgp4_get_4bytes(dest_ip, (u_char *)prefix);
        
        if ((dest_ip == ~0) || (dest_ip == 0x7F000001))
        {
            p_path->p_peer->notify.code = BGP4_UPDATE_MSG_ERR;
            p_path->p_peer->notify.sub_code = BGP4_INVALID_NLRI;
            return VOS_ERR;
        }
        /*stored in network order*/
        *(u_int*)p_route->dest.ip = htonl(dest_ip);
    }
    return VOS_OK;
}

/*bgp4_is_rib_end
check if a message is end-of-rib 
in msg,msg length
out :related afi/safi
return bgp4-success if end-of-rib;else bgp-failure
*/
int 
bgp4_is_end_of_rib(
    u_char *p_msg,
    u_short len,
    u_short *p_afi,
    u_char *p_safi)
{
    u_char std_end[4] = {0,0,0,0};
    u_char mp_ext_end[11] = {0,0,0,7,0x90,15,0,3,0,0,0};
    u_char mp_std_end[10] = {0,0,0,6,0x80,15,3,0,0,0};
    int rc = FALSE;
    
    /*update length must valid*/
    if ((len != 4) && (len != 11) && (len != 10))
    {
        return FALSE;
    }
    
    if ((len == 4) && (memcmp(std_end, p_msg, 4) == 0))
    {
        *p_afi = BGP4_AF_IP;
        *p_safi = BGP4_SAF_UCAST;
        rc = TRUE;        
    }
    else if ((len == 10) && (memcmp(mp_std_end, p_msg, 7) == 0))
    {
        *p_afi = *(p_msg + 8);
        *p_safi = *(p_msg + 9);
        rc = TRUE;        
    }
    else if (memcmp(mp_ext_end, p_msg, 8) == 0)
    {
        *p_afi = *(p_msg + 9);
        *p_safi = *(p_msg + 10);
        rc = TRUE;        
    }
    if (rc == TRUE)
    {
        bgp4_log(BGP_DEBUG_EVT, "end of rib detected,afi/safi %d/%d", *p_afi, *p_safi);
    }
    return rc;
}

/*bgp4_rib_end_input
process end-of-rib of a special afi/safi from peer
in point to peer
afi/safi
out may change peer's endofrib flag,or trigger update
return always success;
*/
u_int
bgp4_end_of_rib_check(
          tBGP4_PEER *p_peer,
          u_char *p_pdu,
          u_int len)
{
    u_short afi = 0;
    u_char safi = 0; 
    u_int af_flag = 0;
    u_char str[64];
    
    if (bgp4_is_end_of_rib(p_pdu, len, &afi, &safi) != TRUE)
    {
        return FALSE;
    }
  
    bgp4_log(BGP_DEBUG_CMN,"proc end of rib from %s,afi/safi %d/%d", bgp4_printf_peer(p_peer, str),afi,safi);  

    /*get flag according to afi/safi*/ 
    af_flag = bgp4_afi_to_index(afi, safi);
    
    /*if either side not support gr capability,ignore*/  
    if ((gbgp4.restart_enable == FALSE) 
        || (p_peer->restart_enable == FALSE))
    {
        bgp4_log(BGP_DEBUG_CMN,"peer not support graceful restart,ignore it");
        return TRUE;
    }
    
    /*if already rcvd the same end-of-rib,ignore*/
    if (flag_isset(p_peer->rxd_rib_end, af_flag))
    {
        bgp4_log(BGP_DEBUG_CMN,"end of rib has been processed previously");
        return TRUE;
    }
    
    /*set the end-of-rib flag*/
    flag_set(p_peer->rxd_rib_end, af_flag);
    
    /*clear rest stale route for the afi/safi*/
    bgp4_peer_stale_route_clear(af_flag, p_peer);

    /*some af's end of rib not rxd,wait for it*/
    if (p_peer->rxd_rib_end != p_peer->local_mpbgp_capability)
    {
        return TRUE;
    }
    
    /*all end-rib recieved,indicating restart is finished,
      check if need send init update*/  
    bgp4_log(BGP_DEBUG_CMN, "peer %s all end of rib received,restart role %d",
        bgp4_printf_peer(p_peer, str), p_peer->restart_role);         

    if (p_peer->restart_role != BGP4_GR_ROLE_NORMAL)
    {
        bgp4_peer_restart_finish(p_peer);
    }
    
    /*if prev state is restart,check if can send first update*/
    if (gbgp4.in_restarting)
    {
        /*decide if local restart is finished*/
        bgp4_local_restart_state_update();
    }
    return TRUE;
}

/*bgp4_mh_send_end_of_rib
send end-of-rib update for a special afi/safi
in peer,afi/safi
out send packet
return none
*/
void 
bgp4_end_of_rib_output(
    tBGP4_PEER *p_peer, 
    u_int af_flag)
{
    u_char buf[32] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                /*len*/0,0,/*type*/2,/*unfeasible length*/
                0,0,/*attrlen*/0,0,/*MP-unreach*/0x90,15,0,3,
                0,0,0,0,0};
    u_char *p_len = &buf[17];
    u_char *p_attr_len = &buf[22];
    u_char *p_afi = &buf[28];
    u_char *p_safi = &buf[29];
    u_short afi = bgp4_index_to_afi(af_flag);
    u_short safi = bgp4_index_to_safi(af_flag);
    u_char str[64];
       
    /*send end-of-rib at last,only if it has not been sent before*/
    if (flag_isset(p_peer->txd_rib_end, af_flag))
    {
        return;
    }
    
    /*set EOB sent flag for such af*/
    flag_set(p_peer->txd_rib_end, af_flag);    

    /*if both side support gr,send end-or-rib*/
    if ((gbgp4.restart_enable == FALSE)
        || (p_peer->restart_enable == FALSE))
    {
        return;
    }
        
    /*fill message*/ 
    if ((afi == BGP4_AF_IP) && (safi == BGP4_SAF_UCAST))
    {
        *p_len = 23;
    }
    else
    {
        *p_len = 30;
        *p_attr_len  = 7;
        *p_afi = afi;/*normal afi is one byte,so set it directly*/          
        *p_safi = safi;
    }

    bgp4_log(BGP_DEBUG_CMN,"send end of rib to %s,afi/safi %d/%d,len %d",
        bgp4_printf_peer(p_peer, str), afi, safi, *p_len);
    bgp4_peer_sock_send(p_peer, buf, *p_len);
    return;
}

/*update message contruct ok,need sends,return send msg length*/
u_int 
bgp4_update_output(tBGP4_UPDATE_PACKET *p_packet)
{
    tBGP4_PEER *p_peer = p_packet->p_peer;
    u_int optlen = 0;
    u_int len = 0;
    u_char addr[64];

    if ((p_packet->feasible_count == 0)
        && (p_packet->withdraw_count== 0))
    {
        /*reset buffer*/    
        bgp4_update_packet_init(p_packet);
        return 0;
    }
    
    if (p_packet->af != BGP4_PF_IPUCAST)
    {
        if (p_packet->feasible_count)
        {
            optlen = bgp4_mp_reach_fill(p_packet->p_msg, 
                            p_packet->af, 
                            p_packet->mp_nexthop, 
                            p_packet->mp_nexthop_len, 
                            p_packet->p_mp_reach, 
                            p_packet->mp_reach_len);
        }
        else if (p_packet->withdraw_count)
        {
            /*build mp-unreach attribute*/
            optlen = bgp4_mp_unreach_fill(p_packet->p_msg, 
                       p_packet->af, p_packet->p_mp_unreach, 
                       p_packet->mp_unreach_len);
        }
        p_packet->len += optlen;
        p_packet->attribute_len += optlen;
        p_packet->p_msg += optlen;
    }
    /*msg header*/ 
    bgp4_init_msg_hdr(p_packet->p_buf, BGP4_MSG_UPDATE);
    
    /*length*/
    bgp4_fill_2bytes(p_packet->p_buf + BGP4_MARKER_LEN, p_packet->len);
    
    /*withdraw length*/                                
    bgp4_fill_2bytes(p_packet->p_buf + BGP4_HLEN, p_packet->withdraw_len);        
    
    /*attr length*/
    bgp4_fill_2bytes(p_packet->p_buf + BGP4_HLEN + 2 + p_packet->withdraw_len, p_packet->attribute_len);

    if (bgp4_peer_sock_send(p_peer, p_packet->p_buf, p_packet->len) != VOS_OK) 
    {
        bgp4_log((BGP_DEBUG_CMN|BGP_DEBUG_UPDATE|BGP_DEBUG_ERROR),
                    "send update to %s ,w %d,f %d.failed",
                    bgp4_printf_peer(p_peer,addr),
                    p_packet->withdraw_count,
                    p_packet->feasible_count);
        
        /*reset buffer*/    
        bgp4_update_packet_init(p_packet);
        return 0;
    }

    bgp4_log((BGP_DEBUG_CMN|BGP_DEBUG_UPDATE),"send update to %s,len %d, w %d,f %d", 
                bgp4_printf_peer(p_peer,addr),
                p_packet->len,
                p_packet->withdraw_count,
                p_packet->feasible_count);

    p_peer->stat.tx_withdraw += p_packet->withdraw_count;
    p_peer->stat.tx_feasible += p_packet->feasible_count;
    p_peer->stat.tx_update++;

    len = p_packet->len;
    /*reset buffer*/    
    bgp4_update_packet_init(p_packet);
    
    return len;/*total length of msg*/
}

/*insert a withdraw route into update packet*/
STATUS
bgp4_update_packet_withdraw_insert(
         tBGP4_UPDATE_PACKET *p_packet,
         tBGP4_ROUTE *p_route)
{
    u_int len = 0;
    u_char fill[8];

    if (p_route->dest.afi != BGP4_PF_IPUCAST)
    {
        return bgp4_update_packet_mpunreach_insert(p_packet, p_route);
    }
    /*if current buffer full,do not add it*/
    len = bgp4_nlri_fill(fill, p_route);

    if ((p_packet->len + len) > gbgp4.max_update_len)
    {
        return VOS_ERR;
    }
    /*insert into msg*/
    memcpy(p_packet->p_msg, fill, len);
    
    /*extend buffer*/
    p_packet->p_msg += len;

    /*increase withdraw length*/
    p_packet->withdraw_len += len;

    /*increase total length*/
    p_packet->len += len;

    /*increase withdraw route count*/
    p_packet->withdraw_count++;
    return VOS_OK;
}

/*insert a feasible route into update packet*/
STATUS
bgp4_update_packet_feasible_insert(
         tBGP4_UPDATE_PACKET *p_packet,
         tBGP4_ROUTE *p_route)
{
    tBGP4_PEER *p_peer = p_packet->p_peer;
    u_char *p_path = 0;
    u_int path_len;
    u_int len;
    u_char new_path[2048];
    u_char nlri[8];
    
    if (p_route->dest.afi != BGP4_PF_IPUCAST)
    {
        return bgp4_update_packet_mpreach_insert(p_packet, p_route);
    }
    
    /*build path attribute for this route*/
    path_len = bgp4_path_fill(p_peer, p_route, new_path); 
    len = bgp4_nlri_fill(nlri, p_route);

    /*if feasible route already exist,must compare path attribtue*/
    if (p_packet->feasible_count)
    {
        /*get pointer of path attribute*/
        p_path = p_packet->p_buf + BGP4_HLEN + 4 + p_packet->withdraw_len;
        
        if ((p_packet->attribute_len != path_len)
            || (memcmp(new_path, p_path, path_len) != 0))
        {
            return VOS_ERR;
        }
    }
    else
    {
        /*try to insert path attribute*/
        if ((p_packet->len + len + path_len) > gbgp4.max_update_len)
        {
            return VOS_ERR;
        }

        /*skip attribute length*/
        p_packet->p_msg += 2;

        /*fill path*/
        memcpy(p_packet->p_msg, new_path, path_len);
        p_packet->p_msg += path_len;
        p_packet->len += path_len;
        p_packet->attribute_len = path_len;
    }

    /*try to insert route into msg*/
    if (p_packet->feasible_count
        && ((p_packet->len + len) > gbgp4.max_update_len))
    {
        return VOS_ERR;
    }  

    memcpy(p_packet->p_msg, nlri, len);
    p_packet->len += len;
    p_packet->p_msg += len;
    
    p_packet->feasible_count++;

    return VOS_OK;
}

STATUS
bgp4_update_packet_mpunreach_insert(
         tBGP4_UPDATE_PACKET *p_packet,
         tBGP4_ROUTE *p_route)
{
    u_int len = bgp4_mp_nlri_fill(NULL, p_route);
    u_int hdrlen = 7;

    /*if this is first route,skip attribute length*/
    if (p_packet->withdraw_count == 0)
    {
        bgp4_update_packet_init(p_packet);
        p_packet->p_msg += 2;
    }
    
    /*length too long,send now*/
    if ((p_packet->len + hdrlen + 
         p_packet->mp_unreach_len + len) > gbgp4.max_update_len)
    {
        return VOS_ERR;
    }
    /*fill mp-nlri*/
    p_packet->mp_unreach_len +=
                    bgp4_mp_nlri_fill(
                        p_packet->p_mp_unreach +
                        p_packet->mp_unreach_len,
                        p_route);
    
    p_packet->withdraw_count++;

    bgp4_route_in_label_set_to_sys(p_route, MPLSL3VPN_DEL_ROUTE);
    
    return VOS_OK;
}

STATUS
bgp4_update_packet_mpreach_insert(
         tBGP4_UPDATE_PACKET *p_packet,
         tBGP4_ROUTE *p_route)
{
    tBGP4_PEER *p_peer = p_packet->p_peer;
    u_int nlrilen = 0;
    u_int path_len;
    u_int nexthop_len;
    u_int hdrlen = 7;
    u_int optlen = 0;
    u_char path[2048];/*path attribute,do not include mp reach/unreach nlri,used for path merge*/    
    u_char nexthop[64];/*mpnexthop,64 is enough*/
    u_char nlri[128];

    /*if no withdraw or feasible route,skip attribute length*/
    if ((p_packet->withdraw_count == 0)
        && (p_packet->feasible_count == 0))
    {
        p_packet->p_msg += 2;
    }

    nlrilen = bgp4_mp_nlri_fill(nlri, p_route);
    
    /*if no feasible exist,try to form mp-unreach option*/
    if (p_packet->feasible_count == 0)
    {
        if (p_packet->mp_unreach_len)
        {
            /*build mp unreach option for all nlri*/
            optlen = bgp4_mp_unreach_fill(p_packet->p_msg, 
                           p_packet->af, 
                           p_packet->p_mp_unreach, 
                           p_packet->mp_unreach_len);

            p_packet->p_msg += optlen;
            p_packet->len += optlen;
            p_packet->attribute_len += optlen;

            /*reset nlri field*/
            p_packet->mp_path_len = 0;
            p_packet->mp_unreach_len = 0;
        }
        /*set first path and nexthop*/
        p_packet->mp_path_len = 
                bgp4_path_fill(p_peer, p_route, p_packet->mp_path); 

        p_packet->mp_nexthop_len = bgp4_mp_nexthop_fill(p_peer, p_packet->mp_nexthop, p_route);
        
        if ((p_packet->len + hdrlen + 
             p_packet->mp_path_len + 
             p_packet->mp_nexthop_len + nlrilen) > gbgp4.max_update_len)
        {
            p_packet->mp_path_len = 0;
            p_packet->mp_nexthop_len = 0;
            return VOS_ERR;
        }
        /*insert path*/
        memcpy(p_packet->p_msg, p_packet->mp_path, p_packet->mp_path_len);
        p_packet->len += p_packet->mp_path_len;
        p_packet->p_msg += p_packet->mp_path_len;
        p_packet->attribute_len += p_packet->mp_path_len;
    }    
    else
    {
        /*compare path,if not same,reject packet*/
        path_len = bgp4_path_fill(p_peer, p_route, path); 
        nexthop_len = bgp4_mp_nexthop_fill(p_peer, nexthop, p_route);
        if ((path_len != p_packet->mp_path_len)
            || (nexthop_len != p_packet->mp_nexthop_len)
            || (memcmp(p_packet->mp_path, path, path_len))
            || (memcmp(p_packet->mp_nexthop, nexthop, nexthop_len)))
        {
            return VOS_ERR;
        }
    }

    if (p_packet->feasible_count
        && ((p_packet->len + hdrlen + 
         p_packet->mp_path_len + 
         p_packet->mp_nexthop_len +
         p_packet->mp_reach_len + nlrilen) > gbgp4.max_update_len))
    {
        return VOS_ERR;
    }

    p_packet->mp_reach_len += bgp4_mp_nlri_fill(
                    p_packet->p_mp_reach + 
                    p_packet->mp_reach_len,
                    p_route);
    
    p_packet->feasible_count++; 

    bgp4_route_in_label_set_to_sys(p_route, p_peer, MPLSL3VPN_ADD_ROUTE);

    return VOS_OK;
}

#else

#include "bgp4com.h" 

u_short bgp4_build_open_option(tBGP4_PEER* p_peer, u_char* p_option);
int bgp4_is_rib_end(u_char *p_msg,u_short len,u_short *p_afi,u_char *p_safi);
int bgp4_rib_end_input(tBGP4_PEER *p_peer,u_short afi,u_char safi);
int bgp4_extract_iproute(tBGP4_PATH * p_path, u_char * p_buf, int len, tBGP4_LIST * p_list);
void bgp4_send_mp_update_to_peer(tBGP4_PEER * p_peer, u_int af, tBGP4_LIST * p_flist, tBGP4_LIST * p_wlist);
int bgp4_process_capability(tBGP4_PEER *p_peer,u_char *p_opt, u_short opt_len);
void bgp4_display_update_input(tBGP4_PEER *p_peer, tBGP4_LIST  *flist, tBGP4_LIST *wlist );
void bgp4_gr_send_all_init_update();
extern int sendBgpEstablishedTrap(int addrType,char* peerid,
                               int state, char *LastError);

/*message input process*/
void bgp4_msg_input(tBGP4_PEER *p_peer,u_char *p_pdu)
{
    tBGP4_MSGHDR  hdr;
    u_char marker[BGP4_MARKER_LEN] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                        0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    u_char code = 0;
    u_char sub_code = 0;
    u_int len = 0 ;
    u_char pestr[64];
    tBGP4_LIST flist;
    tBGP4_LIST wlist;
    u_int tv = vos_get_system_tick();
    u_char is_big_update = FALSE;

    memcpy(&hdr, p_pdu, sizeof(tBGP4_MSGHDR));           
    len = ntohs(hdr.len);

    if(len > BGP4_SYNC_MAX_UPDATE_SIZE)
    {
        is_big_update = TRUE;
        gBgp4.stat.rx_big_update++;
    }
        bgp4_printf_peer(p_peer,pestr);    
    bgp4_log(BGP_DEBUG_PKT,1,"received buffer from %s,ptr %#x,length %d",pestr,p_pdu,len);
    bgp4_debug_packet(p_pdu, len)  ;
    
    
    /*protocol define the min&max bytes of a message*/
    if ((len < BGP4_MIN_MSG_LEN) || (len > BGP4_MAX_MSG_LEN)) 
    {
        bgp4_log(BGP_DEBUG_CMN,1,"receive msg from %s with invalid length %d!!",pestr,len);
        
        p_peer->msg_len_err++;
        bgp4_send_notify(p_peer, BGP4_MSG_HDR_ERR,
                            BGP4_BAD_MSG_LEN, (u_char *)&(hdr.len), 
                            sizeof (u_short));
        bgp4_fsm(p_peer, BGP4_EVENT_FATAL_ERROR);
        return;    
    }
    
    /*Authenticate rcvd message,compare the first 16 bytes ,they must be all 1*/ 
    if (memcmp(marker, hdr.marker, BGP4_MARKER_LEN))
    {
        bgp4_log(BGP_DEBUG_EVT,1,"receive msg from %s with wrong marker field!!",pestr);
        p_peer->msg_marker_err++;
        bgp4_send_notify(p_peer,BGP4_MSG_HDR_ERR,
                            BGP4_CONN_NOT_SYNC, NULL, 0);
                            bgp4_fsm(p_peer, BGP4_EVENT_FATAL_ERROR);
        return;    
    }
    p_peer->rx_msg++;

    p_pdu += BGP4_HLEN;
    /*len -= BGP4_HLEN;*/
    switch (hdr.type) {
        case BGP4_MSG_OPEN :        
        {
            gBgp4.stat.rx_open++;
                p_peer->open_rx++;
            if (len < BGP4_OPEN_HLEN) 
            {
                code = BGP4_MSG_HDR_ERR;
                sub_code = BGP4_BAD_MSG_LEN;
                        p_peer->open_err++;
                break;
            } 
            bgp4_log(BGP_DEBUG_CMN,1,"receive open from %s.",pestr);

            if (p_peer->state == BGP4_NS_OPENSENT)
            {
                bgp4_stop_hold_timer(p_peer);  
        
                if (bgp4_open_input(p_peer, p_pdu, len-BGP4_HLEN) != TRUE)    
                {
                            p_peer->open_err++;
                    bgp4_fsm(p_peer, BGP4_EVENT_FATAL_ERROR);
                    break;
                }
                bgp4_start_hold_timer(p_peer);    
                bgp4_start_keepalive_timer(p_peer);
                bgp4_send_keepalive(p_peer);
            
                p_peer->state = BGP4_NS_OPENCONFIRM;
            }
            else
            {
                if(p_peer->state > BGP4_NS_OPENSENT)
                {
                    bgp4_send_notify(p_peer, BGP4_FSM_ERR, 0, NULL, 0);
                }
                bgp4_log(BGP_DEBUG_EVT,1,"receive open peer->state > BGP4_NS_OPENSENT!!");
                bgp4_fsm(p_peer,BGP4_EVENT_FATAL_ERROR);
            }
            break;
        }
        case BGP4_MSG_UPDATE :
        {
            gBgp4.stat.rx_update++;
            if (len < BGP4_UPDATE_HLEN) 
            {
                code = BGP4_MSG_HDR_ERR;
                sub_code = BGP4_BAD_MSG_LEN;
                p_peer->update_err++; 
                break;
            } 
            bgp4_log(BGP_DEBUG_CMN|BGP_DEBUG_UPDATE,1,"receive update from %s.",pestr);
            if (p_peer->state == BGP4_NS_ESTABLISH)
            {
                p_peer->rx_update++;
                p_peer->rx_updatetime = time(NULL) ;
    
                bgp4_lstinit(&flist);
                bgp4_lstinit(&wlist);
                if (bgp4_update_input(p_peer, p_pdu, len-BGP4_HLEN, &flist, &wlist) == TRUE) 
                {                    
                    bgp4_update_rib(p_peer, &flist, &wlist);
                                
                    if((p_peer->peer_route_number>p_peer->max_prefix)&&(p_peer->max_prefix!=0))
                    {
                        bgp4_log(BGP_DEBUG_EVT,1,"peer->peer route number>peer->max prefix");
                        bgp4_send_notify(p_peer, BGP4_CEASE, 1, NULL, 0);
                        bgp4_fsm(p_peer,BGP4_EVENT_FATAL_ERROR);
                    }
                    else
                    {
                        tv =  vos_get_system_tick() - tv ;
                        if (tv > p_peer->max_rx_updatetime)
                        {
                            p_peer->max_rx_updatetime = tv;
                        }
                        bgp4_restart_hold_timer(p_peer);   
                    }
                    if(is_big_update == TRUE &&
                        gBgp4.work_mode == BGP4_MODE_MASTER)
                    {
                        bgp4_rebuild_sync_pdu(&flist,&wlist,p_peer);
                    }
    
                }  
                else
                {
                    bgp4_log(BGP_DEBUG_EVT,1,"bgp4 update input ERROR!");
                    p_peer->update_err++; 
                    bgp4_fsm(p_peer, BGP4_EVENT_FATAL_ERROR);  
                
                }
                bgp4_rtlist_clear (&flist);
                bgp4_rtlist_clear (&wlist);
            }
            else
            {
                        p_peer->update_err++; 
                if(p_peer->state >= BGP4_NS_OPENSENT)
                {
                    bgp4_send_notify(p_peer, BGP4_FSM_ERR, 0, NULL, 0);
                }
                bgp4_log(BGP_DEBUG_EVT,1,"peer's state is not ESTABLISH");
                bgp4_fsm(p_peer,BGP4_EVENT_FATAL_ERROR);
            }
            break;
        }

        case BGP4_MSG_NOTIFY  :
        {       
                gBgp4.stat.rx_notify++;
            if (len < BGP4_NOTIFY_HLEN) 
            {
                code = BGP4_MSG_HDR_ERR;
                sub_code = BGP4_BAD_MSG_LEN;
                break;
            }
                p_peer->notify_rx++; 
            bgp4_log(BGP_DEBUG_CMN,1,"receive notification from %s.",pestr);
            if ((p_peer->state == BGP4_NS_OPENSENT) ||
                    (p_peer->state == BGP4_NS_OPENCONFIRM) ||
                    (p_peer->state == BGP4_NS_ESTABLISH))
            {
                bgp4_notify_input(p_peer, p_pdu, len-BGP4_HLEN);
                bgp4_log(BGP_DEBUG_EVT,1,"receive notify packet");
                bgp4_fsm(p_peer, BGP4_EVENT_FATAL_ERROR); 
            }
            break;
        }    
        case BGP4_MSG_KEEPALIVE     :
        {
            gBgp4.stat.rx_keepalive++;
            if (len != BGP4_KEEPALIVE_LEN) 
            {
                code = BGP4_MSG_HDR_ERR;
                sub_code = BGP4_BAD_MSG_LEN;
                break;
            }
            p_peer->keepalive_rx++; 
            bgp4_log(BGP_DEBUG_CMN,1,"receive keepalive from %s.",pestr);
            
            if ((p_peer->state == BGP4_NS_OPENCONFIRM) ||
                    (p_peer->state == BGP4_NS_ESTABLISH))
            {
                bgp4_restart_hold_timer(p_peer);
                if (p_peer->state == BGP4_NS_OPENCONFIRM)
                {
                    u_int af = 0 ;
                    u_short error_code=p_peer->last_errcode;
                    error_code =  (error_code << 8) + p_peer->last_subcode;
                     
                    p_peer->established_transitions ++;                 
                    p_peer->rx_update = 0;
                    p_peer->tx_update = 0;
                    p_peer->uptime = time(NULL) ;
                    p_peer->state = BGP4_NS_ESTABLISH;

                    bgp4_log(BGP_DEBUG_FSMCHANGE,1,"BGP4 neighbor %s state : Change to Established...",pestr); 

                        if ((p_peer->bfd_enable)&&(p_peer->bfd_discribe <= 0))
                        {                           
                                bgp4_bind_bfd(p_peer);
                        }

                    bgp4_process_gr_open(p_peer,1);
                
                    if(bgp4_gr_if_can_update() == TRUE)
                    {   
                        for (af = 0 ; af < BGP4_PF_MAX ; af++)
                        {
                            bgp4_schedule_init_update(af, p_peer);
                        }                       
                    }
                    if (gBgp4.trap_enable)
                    {
                        if(p_peer->remote.ip.afi==BGP4_PF_IPUCAST)
                        {
                            sendBgpEstablishedTrap(AF_INET,p_peer->remote.ip.ip,p_peer->state,&error_code);
                        }
                        else if(p_peer->remote.ip.afi==BGP4_PF_IP6UCAST)
                        {
                            sendBgpEstablishedTrap(AF_INET6,p_peer->remote.ip.ip,p_peer->state,&error_code);
                        }               
                    }
                }
            }
            else
            {
                if(p_peer->state >= BGP4_NS_OPENSENT)
                {
                    bgp4_send_notify(p_peer, BGP4_FSM_ERR, 0, NULL, 0);
                }
                bgp4_log(BGP_DEBUG_EVT,1,"receive keepalive while peer state is not establish");
                bgp4_fsm(p_peer,BGP4_EVENT_FATAL_ERROR);
            }
            break;
        }
        case BGP4_MSG_REFRESH   :
        {
            if (len < BGP4_ROUTE_REFRESH_LEN) 
            {
                code = BGP4_MSG_HDR_ERR;
                sub_code = BGP4_BAD_MSG_LEN;
                break;
            } 
            bgp4_log(BGP_DEBUG_CMN,1,"receive route refresh from %s." ,pestr);
            if (p_peer->state == BGP4_NS_ESTABLISH)
            {
                bgp4_refresh_input (p_peer, p_pdu, len-BGP4_HLEN);
            }
            break;
        }
        default :
        {
            code = BGP4_MSG_HDR_ERR;
            sub_code = BGP4_BAD_MSG_TYPE;
            break;
        }       
            
    }
    
    if (code) 
    {
        bgp4_log(BGP_DEBUG_CMN,1,"receive msg from %s with wrong header,type=%d,len=%d,code=%d,subcode=%d.",
            pestr, hdr.type, ntohs(hdr.len), code, sub_code);        
        if ( sub_code == BGP4_BAD_MSG_TYPE)
        {
            bgp4_send_notify(p_peer, code, sub_code, (u_char *)&(hdr.type), sizeof(u_char));
        }
        else
        {
            bgp4_send_notify(p_peer, code, sub_code, (u_char *)&(hdr.len), sizeof (u_short));
        }
    }

    /*************for sync update route***************/
    if(is_big_update == FALSE && 
        gBgp4.work_mode == BGP4_MODE_MASTER)
    {
        bgp4_send_sync_pdu(p_peer,p_pdu -BGP4_HLEN,len);
    }   
    
    return;
}
int bgp4_notify_input(tBGP4_PEER *p_peer, 
                                  u_char *p_buf, u_int len)
{
    tBGP4_NOTIFYMSG *p_notify = NULL;
    u_char err_str[32];                         
    u_char suberr_str[64];
    u_char addstr[64];
    
    p_notify = (tBGP4_NOTIFYMSG *) p_buf;
    
    p_peer->last_errcode = p_notify->code;
    p_peer->last_subcode = p_notify->sub_code;
    
    if (gBgp4.dbg_flag &( BGP_DEBUG_CMN|BGP_DEBUG_EVT))
    {
        bgp4_printf_notify(p_notify->code, p_notify->sub_code,err_str,suberr_str);
            bgp4_log(BGP_DEBUG_CMN|BGP_DEBUG_EVT,1,"received notification from %s :e=%d(%s),s=%d(%s).",
                bgp4_printf_peer(p_peer,addstr),
                p_notify->code,err_str,
                p_notify->sub_code,suberr_str);
    } 

    if ((p_notify->code == BGP4_OPEN_MSG_ERR)
            && (p_notify->sub_code == BGP4_OPT_PARM_UNRECOGNIZED))
    {
        p_peer->send_open_option = 0;
    }

    /*wait a little time ,just a test wait for low layer tcp end connection*/
    vos_pthread_delay(30);

    /*when recieving notification,do not enter graceful-restart process*/
    bgp4_peer_exit_gr(p_peer);

    return (TRUE);
}

/*open message tx&rx*/
int bgp4_open_input(tBGP4_PEER *p_peer, u_char *p_msg, u_int len)
 {
    u_char errmsg[512] ;
    u_char ver = 0;
    u_short as;     
    u_int id;
    u_int total_optlen;
    u_short hold_time = 0;
    u_short keep_time = 0;
    u_int code = BGP4_OPEN_MSG_ERR ;
    u_int subcode = 0 ;
    u_char *p_err = NULL ;
    u_int errlen = 0 ;
    u_char opt_type = 0;
    u_char opt_len = 0;
    u_char read_len = 0; 
    u_char gr_hint = 0 ;     
    u_char addstr[64];
    u_char addstr2[16];
    u_int cap_intersection = 0 ;
     
    /*init params of peer*/
    p_peer->remote.refresh = 0 ;
    p_peer->remote.af = 0 ;

    /*add for cap advertisement,by zxq*/
    p_peer->remote.capability = 0;
     
    /*for ipv4 peer that not send mp option for ipv4 capabilty*/
    if(p_peer->remote.ip.afi == BGP4_PF_IPUCAST)
    {
        af_set(p_peer->remote.af,BGP4_PF_IPUCAST);
        af_set(p_peer->remote.capability,BGP4_PF_IPUCAST);
    }
    
    /*init rib send flag*/
    p_peer->send_rib_end = 0;    
    p_peer->send_rib_end_af = 0;    
    
    /*get fix part of open*/     
    /*ipsoft add to decode open message */
    ver = *p_msg;
    p_msg += 1;
     
    bgp4_get_2bytes(as, p_msg);
    p_msg += 2;
     
    bgp4_get_2bytes(hold_time,p_msg);
    p_msg += 2;
     
    bgp4_get_4bytes(id, p_msg);
    p_msg += 4;
     
    total_optlen = *p_msg;
    p_msg += 1;
     
    /*ipsoft end decode */
    /*version mismatch*/
    if (ver != BGP4_VERSION_4) 
    {
        if (ver > BGP4_VERSION_4)
        {
            ver = 4;
        }
        subcode = BGP4_UNSUPPORTED_VER_NO ;
        p_err = &ver ;
        errlen = sizeof(u_char) ;
        goto OPEN_ERROR;
    }
     
    /*as mismatch*/
    if (as != p_peer->remote.as) 
    {
        subcode = BGP4_AS_UNACCEPTABLE ;
        goto OPEN_ERROR;
    }
    
    /*check bgp id */
    if ((id == 0) || (id == 0xFFFFFFFF) || (id == 0x7F000001)) 
    {         
        subcode = BGP4_BGPID_INCORRECT ;
        goto OPEN_ERROR;
    }

    /*neg hold time*/
    if (hold_time < p_peer->hold_interval) 
    {
        if (p_peer->hold_interval)                  
        {
            keep_time = (hold_time * p_peer->keep_interval)/p_peer->hold_interval;
        }
        keep_time = (keep_time)?keep_time:1;

        if (hold_time && (hold_time < BGP4_MIN_HOLD_INTERVAL)) 
        {
            subcode = BGP4_HOLD_TMR_UNACCEPTABLE ;
            goto OPEN_ERROR;
        } 
    }
    else 
    {
        hold_time = p_peer->hold_interval;
        keep_time = p_peer->keep_interval;
    }

    /*option*/
    if(total_optlen && p_peer->local.capability)
    {
        /*IPSOFT cheng add to support mp-bgp*/
        /*ipsoft ddp change option process*/         
        while (read_len < total_optlen)
        {  
            /*get optional params type */
            opt_type = *p_msg;
            p_msg ++ ;
                             
            /*get params length,length do not include type-length pair */  
            opt_len = *p_msg ;
            p_msg ++ ;
                 
            read_len += (2 + opt_len);   
                 
            switch (opt_type){
                case 1 :/*authentication.ignored*/
                {
                    p_msg += opt_len ;
                    break ;
                } 
                case 2 :
                {
                    gr_hint = TRUE ;/*capability option exist,so there may 
                                    be graceful-restart capability in the option*/
                     
                    if (bgp4_process_capability(p_peer, p_msg, opt_len) == FALSE)        
                    {
                        subcode = BGP4_OPT_PARM_UNRECOGNIZED ;
                        goto OPEN_ERROR;
                    }
                    p_msg += opt_len ;
                     
                    break ;
                }  
                default :/*ignore unknown option*/
                {
                    p_msg += opt_len ;                      
                    break ;

                }
            }
        }/*while*/
    
        /*add for cap advertisement,compare remote and local cap*/
        cap_intersection = p_peer->remote.capability & p_peer->local.capability;

        if (!cap_intersection && p_peer->local.capability)
        {
            p_peer->unsupport_capability = 1;
            subcode = BGP4_UNSUPPORTED_CAPABILITY ;
            p_err = errmsg ;
            errlen = bgp4_build_open_option(p_peer, errmsg);
            goto OPEN_ERROR;
        }
          /*ysq change for n2x test*/
#if 0         
          if (cap_intersection != p_peer->send_capability)/*send open again using cap_intersection*/
          {
                p_peer->send_capability = cap_intersection;
                return FALSE;
          }
#endif
    }/*optlength*/
#if 0
      else if (p_peer->local.capability > 1) 
      {
             /*add for cap advertisement,by zxq*/
             p_peer->unsupport_capability = 1;
             subcode = BGP4_UNSUPPORTED_CAPABILITY ;
             p_err = errmsg ;
             errlen = bgp4_build_open_option(p_peer, errmsg);
             goto OPEN_ERROR;
       }
        else
        {
            af_set(p_peer->remote.af, BGP4_PF_IPUCAST);
            af_set(p_peer->remote.capability, BGP4_PF_IPUCAST);
        }
#endif
    p_peer->router_id = id;
    p_peer->neg_hold_interval = hold_time;
    p_peer->neg_keep_interval  = keep_time;
    p_peer->version = ver;

    /* implement debug print*/
    bgp4_log(BGP_DEBUG_EVT,1,"recvd open from %s:bgpId:%s,hold:%d,keepalive:%d,peer remote cap %X.",
                    bgp4_printf_peer(p_peer,addstr),
                    inet_ntoa_1(addstr2,(p_peer->router_id)),
                    hold_time,
                    keep_time,
                    p_peer->remote.capability);

    return (TRUE);

    OPEN_ERROR :

    bgp4_send_notify(p_peer, code, subcode, p_err, errlen);
    
    return FALSE ;
}
int bgp4_update_input(
              tBGP4_PEER *p_peer,
              u_char  *p_msg, 
              u_int  len,
              tBGP4_LIST  *p_flist, 
              tBGP4_LIST *p_wlist )
{
    tBGP4_PATH * p_path = NULL;
    int with_len = 0; 
    int attr_len  = 0; 
    int nlri_len = 0; 
    u_short afi = 0;
    u_char safi = 0; 
    u_int af = 0;
    
    /*check if rcvd update is an end-of-rib mark,if so,obtian related
    afi/safi,and enter graceful-restart process*/        
    if (bgp4_is_rib_end(p_msg, len, &afi, &safi) == TRUE)
    {
        return bgp4_rib_end_input(p_peer, afi, safi);
    }     
    
    /*unfeasible length*/        
    bgp4_get_2bytes(with_len, p_msg);        
    
    /*length + length-field,not exceed max len*/
    if ((with_len + 4) > len) 
    {        
        goto SEND_ERROR ;
    }
    
    /*path attr-len*/
    bgp4_get_2bytes(attr_len, (p_msg+with_len+2));        
    
    if ((with_len + attr_len + 4) > len) 
    {        
        goto SEND_ERROR ;
    }

    p_path = bgp4_add_path(BGP4_PF_MAX); 
    if (p_path == NULL) 
    {
        bgp4_log(BGP_DEBUG_CMN,1,"not enough memory for attributes!!");
        return (FALSE);
    }
    p_path->p_peer = p_peer; 
    p_path->p_instance = p_peer->p_instance;
    p_path->src_instance_id = p_peer->p_instance->instance_id;

    /*get with route*/
    if (bgp4_extract_iproute(p_path, (p_msg + 2), with_len, p_wlist) == FALSE) 
    {
        bgp4_rtlist_clear(p_wlist);        
        return (FALSE);
    }

    p_msg += with_len + 4 ;
    if (attr_len > 0)
    {
        if (bgp4_extract_path_attribute(p_path,  p_msg, attr_len, p_flist, p_wlist) == FALSE)
        {
            /*the path is not in attr_list yet*/
            bgp4_path_free(p_path);
            return FALSE;
        }
    }
    p_msg += attr_len;

    /*merge same path with routes*/
    p_path = bgp4_path_merge(p_path);   
    
    /*nlri len*/
    nlri_len = len - (with_len + attr_len + 4);
    if (bgp4_extract_iproute(p_path, p_msg, nlri_len, p_flist) == FALSE) 
    {
        bgp4_rtlist_clear(p_wlist);
        bgp4_rtlist_clear(p_flist);    
        return (FALSE);
    }   

    /*implement debug print*/
    bgp4_display_update_input(p_peer, p_flist,p_wlist);
    
    
    return (TRUE);

    SEND_ERROR :
    
    bgp4_send_notify(p_peer,BGP4_UPDATE_MSG_ERR, 
                        BGP4_MALFORMED_ATTR_LIST, NULL, 0);
    
    return FALSE ;
}

/*refresh message*/
int bgp4_refresh_input(tBGP4_PEER *p_peer,u_char *p_buf,u_int msg_len)
{
    u_short afi = 0;
    u_char safi = 0;
    u_int flag = 0;
    u_char addrstr[64];
    
    if ((!p_peer) || (!p_buf))
        return FALSE;

    
    /*check if peer support,if not ignored*/        
    if ((p_peer->local.refresh == 0) ||
        (p_peer->remote.refresh == 0))
    {
        bgp4_log(BGP_DEBUG_CMN,1,"peer %s not support refresh",bgp4_printf_peer(p_peer,addrstr));
        
        return TRUE;
    }
    
    bgp4_get_2bytes(afi, p_buf);
    safi = *(p_buf+3);
    flag = bgp4_afi_to_index(afi, safi) ;

    bgp4_log(BGP_DEBUG_CMN,1,"obtain afi/safi of refresh msg: %d/%d",afi,safi);

    if (!bgp4_af_support(p_peer, flag))
    {
        bgp4_log(BGP_DEBUG_CMN,1,"peer %s can not support this afi/safi",bgp4_printf_peer(p_peer,addrstr));
        return TRUE;
    }

    bgp4_schedule_init_update(flag, p_peer);
    return TRUE ; 
}

void bgp4_send_open(tBGP4_PEER *p_peer)
{
    u_char buf[BGP4_MAX_MSG_LEN];
    tBGP4_MSGHDR  *p_msg = (tBGP4_MSGHDR *)buf;
    u_char *p_buf = NULL;
    u_int id = 0;
    u_char opt_len = 0;  
    bgp4_init_msg_hdr(buf, BGP4_MSG_OPEN); 
    
    /*move to open hdr*/    
    p_buf = buf + BGP4_HLEN;
    
    bgp4_put_char(p_buf, BGP4_VERSION_4);

    /*default AS num*/
    id = p_peer->local.as ;
    
    /* enable change the local-as of a EBGP peer*/
    /*IPSOFT  for confederation*/
    if ((bgp4_peer_type(p_peer) == BGP4_EBGP))
    {
        if(p_peer->fake_as != gBgp4.asnum)
        {
            id = p_peer->fake_as;
        }
        else if( gBgp4.confed_id)
        {
            id = gBgp4.confed_id;
        }
        
    }
        
    bgp4_put_2bytes(p_buf, id) ;
    
    bgp4_put_2bytes(p_buf, p_peer->hold_interval);    
    
    bgp4_put_4bytes(p_buf, gBgp4.router_id);
    
    /*form optional params*/
    opt_len = bgp4_build_open_option(p_peer, p_buf + 1/*skip option length filed*/);
    
    /*fill opt length*/
    *p_buf = opt_len ;
    
    /*open fix length + opt length*/           
    p_msg->len = htons(BGP4_OPEN_HLEN + opt_len);  
        
    {
        u_char addstr[64];
        u_char addstr2[16];
        bgp4_log(BGP_DEBUG_CMN,1,"send open to %s.bgp router Id:%s,hold:%d.",
                    bgp4_printf_peer(p_peer,addstr),
                    inet_ntoa_1(addstr2, gBgp4.router_id),
                    p_peer->hold_interval);
    }
        
    bgp4_sock_send(p_peer, buf, ntohs(p_msg->len));
        
    p_peer->open_tx++;    
    return ;

}

/*keepalive messge send*/
void bgp4_send_keepalive(tBGP4_PEER *p_peer)
{
    u_char buf[BGP4_KEEPALIVE_LEN];
    tBGP4_MSGHDR  * p_msg = (tBGP4_MSGHDR *)buf; 
    u_int len = BGP4_HLEN ;
    u_char addstr[64];

    bgp4_init_msg_hdr(buf, BGP4_MSG_KEEPALIVE);
    p_msg->len = htons(len);
   
        bgp4_log(BGP_DEBUG_CMN,1,"send keepalive to %s." , bgp4_printf_peer(p_peer,addstr));
    bgp4_sock_send(p_peer, buf, len); 
        p_peer->keepalive_tx++;    
    return ;
}

/*notify msg r&t*/
int bgp4_send_notify(tBGP4_PEER *p_peer, u_char code, 
                        u_char sub_code,u_char *p_data, 
                        u_int errlen)
{
    u_char buf[BGP4_MAX_MSG_LEN];
    tBGP4_MSGHDR  * p_msg = (tBGP4_MSGHDR *)buf; 
    tBGP4_NOTIFYMSG *p_notify = NULL; 
    u_int len = BGP4_HLEN ;
    u_char addstr[64];
    u_char err_str[32];                         
    u_char suberr_str[64];

    if(gBgp4.stat.mem[MEM_BGP_ROUTE].fail)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 send notify,memory exhausted,no sending!");
        return;
    }
        
    bgp4_init_msg_hdr(buf, BGP4_MSG_NOTIFY);

    p_notify = (tBGP4_NOTIFYMSG *)(((u_long)p_msg) + BGP4_HLEN);
    p_notify->code = code;
    p_notify->sub_code = sub_code;    
    len += 2;
    
    /*copy error data*/
    if (errlen && p_data) 
    {
        memcpy((((u_char *)p_notify) + 2), p_data, errlen);
        len += errlen;
    }
    
    p_msg->len =  htons(len);
    
    if (gBgp4.dbg_flag & BGP_DEBUG_EVT)
    {
        bgp4_printf_notify(code,sub_code,err_str,suberr_str);
        bgp4_log(BGP_DEBUG_EVT,1,"send notifiy to %s.error:%d(%s),suberror:%d(%s).",
                bgp4_printf_peer(p_peer,addstr),
                code,
                err_str,
                sub_code,
                suberr_str);
    }  
    
    /*if sending notification,do not enter graceful-restart process ddp */ 
    bgp4_peer_exit_gr(p_peer);
     
    bgp4_sock_send(p_peer, buf, len);

    p_peer->last_errcode = code;
    p_peer->last_subcode = sub_code;
        p_peer->notify_tx++;    
    return (TRUE);
}

int bgp4_send_refresh(tBGP4_PEER *p_peer,u_int af_flag)
{
    u_char buf[BGP4_MAX_MSG_LEN];
    tBGP4_MSGHDR  * p_hdr = (tBGP4_MSGHDR *)buf;
    u_char *p_msg = NULL ;
    u_short len = BGP4_HLEN ;
    u_char addrstr[64];
    u_short afi= 0;
    u_short safi = 0;
    
    if (p_peer->state != BGP4_NS_ESTABLISH) 
    {
        bgp4_log(BGP_DEBUG_CMN,1,"peer is not established,do not send refresh msg");
        return VOS_ERR;
    }
    
    if ((p_peer->local.refresh == 0) ||
        (p_peer->remote.refresh == 0))        
    {
        bgp4_log(BGP_DEBUG_CMN,1,"peer not support refresh");        
        return VOS_ERR;
    }
    
    if (!bgp4_af_support(p_peer, af_flag))
    {
        bgp4_log(BGP_DEBUG_CMN,1,"peer not support this afi/safi,af:%#x,remote af:%#x,local af:%#x",
                    af_flag,
                    p_peer->remote.af,
                    p_peer->local.af);
        return VOS_ERR;
    }

    bgp4_init_msg_hdr(buf, BGP4_MSG_REFRESH);   
    
    /*fill afi/.safi*/
    p_msg = (u_char *)p_hdr;
    
    p_msg += BGP4_HLEN;/*skip common header*/
    
    *(p_msg+2) = 0; 
    
    /*afi*/
    afi = bgp4_index_to_afi(af_flag);
    bgp4_fill_2bytes(p_msg, afi);
    
    /*safi*/
    safi = bgp4_index_to_safi(af_flag);
    *(p_msg + 3) = safi;
    len += 4;/*4bytes filed*/
      
    p_hdr->len = htons(len);
    
    /*Send*/   
    bgp4_log(BGP_DEBUG_EVT,1,"send refresh to %s.afi/safi %d/%d", 
            bgp4_printf_peer(p_peer,addrstr),
            afi,
            safi);
    
    if (bgp4_sock_send(p_peer, buf, len) == FALSE) 
    {
#if 0
        bgp4_fsm_invalid(p_peer);
#endif
        bgp4_fsm(p_peer,BGP4_EVENT_FATAL_ERROR);
    }          
    return VOS_OK;
}


int bgp4_process_capability(tBGP4_PEER *p_peer,u_char *p_opt, u_short opt_len)
  {
    u_char type = 0;
    u_char len = 0;
    u_short afi = 0;
    u_char safi = 0;
    u_int afflag = 0;
    u_short readlen = 0;

    bgp4_log(BGP_DEBUG_CMN,1,"process capability in open,length %d",opt_len);
      
    while (readlen < opt_len )
    {
        /*getting capability code:1,MP-BGP 2,Rt refresh*/
        type = *p_opt ;
        p_opt ++ ;
          
        len = *p_opt ;
        p_opt ++ ;
          
        /*proce length + cap length can not exceed total length*/
        if ((readlen + 2 + len) > opt_len)
        {
            bgp4_log(BGP_DEBUG_CMN,1,"encounter capability length,stop processing"); 
            return FALSE ;
        }
          
        switch (type){
            case BGP4_CAP_MULTI_PROTOCOL : 
            {
                if (len != 4)
                {                           
                    bgp4_log(BGP_DEBUG_CMN,1,"invalid mpbgp capability length %d",len);
                    return FALSE ;
                }
    
                /*supported afi :ipv4&ipv6*/
                afi = *(p_opt + 1);
                p_opt += 3 ;
              
                /*supported safi:ipv4 unicast,ipv4 labeled unicast*/                                           
                safi = *(p_opt) ;
                p_opt ++ ; 

                /*update peer*/  
                bgp4_log(BGP_DEBUG_CMN,1,"process mpbgp capability ,afi/safi %d/%d",afi,safi);

                /*update remote peer af flag*/  
                afflag = bgp4_afi_to_index(afi, safi);

                af_set(p_peer->remote.af, afflag);
                af_set(p_peer->remote.capability, afflag);

                /*for cap advertisement,record remote capability*/
                break ;
            }              
              
            case BGP4_CAP_ROUTE_REFRESH: 
            {
                bgp4_log(BGP_DEBUG_CMN,1,"process route refresh capability");        
                if (len != 0)
                {
                    bgp4_log(BGP_DEBUG_CMN,1,"invalid rt refresh capability length %d", len);
                    return FALSE ;
                }
                /*record route refresh*/
                p_peer->remote.refresh = TRUE ;
                af_set(p_peer->remote.capability, BGP4_RTREFRESH);
                break ;
            }
            case BGP4_CAP_GRACEFUL_RESTART :
            {
                if (bgp4_extract_gr_capability(p_peer, p_opt, len) != TRUE)
                {
                    return FALSE ;
                }
                p_opt += len ;
                /*tbi*/
                break;
            }
            default  :
            {
                bgp4_log(BGP_DEBUG_CMN,1,"unrecognized capability,code %d,length %d",type,len);              
                p_opt  += len ;
                break ;
            }
        }
        readlen += (len + 2) ;                  
    }
        
    return TRUE ;
 }


/*return option length,buf start from the first option byte*/
u_short bgp4_build_open_option(tBGP4_PEER *p_peer, u_char *p_option)
{
    u_short len = 0 ;
    u_char *p_buf = p_option ;
    u_short afi = 0 ;
    u_char safi = 0 ;
    u_int i = 0 ;
    u_char grlen = 0;
    u_char *p_len = NULL;
      
    /*for cap advertisement*/      
    if (!p_peer->send_open_option)
    {
        return 0;
    }
    /*for cap advertisement*/
    if (!p_peer->send_capability)
    {
        return 0;
    }

    /*capability option type --2*/
    bgp4_put_char(p_buf, 2);
    len ++ ;

    /*capability option length--unkown yet*/   
    p_len = p_buf ; 
    /*fill 0*/
    bgp4_put_char(p_buf, 0);
    len ++ ;

    /*start form mpbgp capability*/
    for (i = 0 ; i < BGP4_PF_MAX ; i++)
    {
        if (af_isset(p_peer->send_capability, i))
        {
            afi = bgp4_index_to_afi(i);
            safi = bgp4_index_to_safi(i);
            bgp4_put_char(p_buf, BGP4_CAP_MULTI_PROTOCOL); 
            bgp4_put_char(p_buf, 4); 
            bgp4_put_char(p_buf, 0); 
            bgp4_put_char(p_buf, afi); 
            bgp4_put_char(p_buf, 0); 
            bgp4_put_char(p_buf, safi); 
            len += 6 ; 
            *p_len += 6;
        }
    }

    /*form route refresh capability*/     
    if (af_isset(p_peer->send_capability,BGP4_RTREFRESH))
    {
        bgp4_put_char(p_buf, BGP4_CAP_ROUTE_REFRESH);
        bgp4_put_char(p_buf, 0);  
        len += 2 ; 
        *p_len += 2;
    }

    /*form graceful restart capability if enable*/
    if(gBgp4.gr_enable == TRUE)
    {
        grlen = bgp4_build_gr_capability(p_peer,p_buf);
        *p_len += grlen;
        len += grlen;
        p_buf += grlen;
    }   
    
    if (*p_len)/*if no option filed exist,do nothing*/
        return len ;
    else
        return 0;        
}


/*update message*/
void bgp4_display_update_input(tBGP4_PEER *p_peer, tBGP4_LIST  *flist, tBGP4_LIST *wlist )
{
    tBGP4_LINK *p_link = NULL;
    u_char addstr[64];
    u_char addstr2[16];
    
    inet_ntoa_1(addstr,htonl(*(u_int*)p_peer->remote.ip.ip));
    bgp4_log(BGP_DEBUG_UPDATE,1,"recvd update, f=%d,w=%d",bgp4_lstcnt(flist),bgp4_lstcnt(wlist));
    
    if (gBgp4.dbg_flag & BGP_DEBUG_UPDATE)
    {
        LST_LOOP(wlist,p_link, node, tBGP4_LINK)
        {
            bgp4_log(BGP_DEBUG_UPDATE,1,"withdraw route: %s",bgp4_printf_route(p_link->p_route,addstr));
        }
        
        LST_LOOP(flist,p_link, node, tBGP4_LINK)
        {
            bgp4_log(BGP_DEBUG_UPDATE,1,"feasiable route:%s,nexthop %s.",bgp4_printf_route(p_link->p_route,addstr),inet_ntoa_1(addstr2,htonl(*(u_int*)p_link->p_route->p_path->nexthop.ip)));
        }

    }

    return ;
}

int bgp4_extract_iproute(
                   tBGP4_PATH *p_path,
                   u_char *p_buf,
                   int len, 
                   tBGP4_LIST *p_list )
{
    int bytes = 0; 
    u_int prefix[16];
    u_int subcode = 0 ;
    tBGP4_ROUTE * p_route = NULL;


    while (len > 0)
    {
        p_route = bgp4_creat_route();
        if (p_route == NULL) 
        {
            bgp4_log(BGP_DEBUG_CMN,1,"not memory for withdrawn");
            return (FALSE);
        }
        
        if (bgp4_rtlist_add(p_list, p_route) == NULL)
        {
            bgp4_free(p_route, MEM_BGP_ROUTE);
            return (FALSE);
        }

        bgp4_link_path (p_path, p_route);          
             
        p_route->dest.afi = BGP4_PF_IPUCAST;    
        p_route->proto = M2_ipRouteProto_bgp;
        p_route->dest.prefixlen =  *p_buf;  /*get prefix length*/  
        bytes = bgp4_bit2byte(*p_buf);

        /*prefix length*/
        p_buf++;
        len--;
        
        if (bytes > len) 
        {            
            subcode = BGP4_MALFORMED_ATTR_LIST ;
            goto SEND_ERROR ;
        }
        
        /*real prefix*/
        memset(prefix,0,16);
        bgp4_get_prefix(p_buf, prefix, bytes);
        
        /*need use host order*/    
        if(bytes <= 4)
        {
                    u_int dest_ip = 0;
            bgp4_get_4bytes(*(u_int*)p_route->dest.ip, (u_char *)prefix);
                    dest_ip = *(u_int*)p_route->dest.ip;

                    /*invalid prefix*/
                    if ((dest_ip == ~0) 
                        ||(dest_ip == 0x7F000001)
                        || (p_route->dest.prefixlen > 32))
            {
                subcode = BGP4_INVALID_NLRI ;
                goto SEND_ERROR;
            }
        }
        else if(bytes==16)
        {
            memcpy(&p_route->dest.ip,(u_char*)prefix,16);
            if ((p_route->dest.prefixlen > 32))
            {
                subcode = BGP4_INVALID_NLRI ;
                goto SEND_ERROR;
            }
        }
        /*decrease rest length*/         
        len -= bytes;
    }
        
    return (TRUE);

    SEND_ERROR :
    bgp4_send_notify(p_path->p_peer,BGP4_UPDATE_MSG_ERR, subcode , NULL, 0);
    return FALSE ;
}


/*bgp4_is_rib_end
check if a message is end-of-rib 
in msg,msg length
out :related afi/safi
return bgp4-success if end-of-rib;else bgp-failure
*/
int bgp4_is_rib_end(u_char *p_msg,u_short len,u_short *p_afi,u_char *p_safi)
{
    u_char std_end[4] = {0,0,0,0};
    u_char mp_ext_end[11] = {0,0,0,7,0x90,15,0,3,0,0,0};
    u_char mp_std_end[10] = {0,0,0,6,0x80,15,3,0,0,0};
    int ret = FALSE;
    
    /*update length must valid*/
    if ((len != 4) && (len != 11) && (len != 10))
    {
        return FALSE ;
    }
    
    if ((len == 4) && (memcmp(std_end,p_msg,4) == 0))
    {
        *p_afi = BGP4_AF_IP ;
        *p_safi = BGP4_SAF_UCAST ;
        ret = TRUE;        
    }
    else if ((len == 10) && (memcmp(mp_std_end,p_msg,7) == 0))
    {
        *p_afi = *(p_msg + 8);
        *p_safi = *(p_msg + 9);
        ret = TRUE;        
    }
    else if (memcmp(mp_ext_end,p_msg,8) == 0)
    {
        *p_afi = *(p_msg + 9);
        *p_safi = *(p_msg + 10);
        ret = TRUE;        
    }
    if (ret == TRUE)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"end of rib detected,afi/safi %d/%d",*p_afi,*p_safi);
    }
    return ret;
}

/*bgp4_rib_end_input
process end-of-rib of a special afi/safi from peer
in point to peer
afi/safi
out may change peer's endofrib flag,or trigger update
return always success;
*/
int bgp4_rib_end_input(tBGP4_PEER *p_peer, u_short afi, u_char safi)
{
    u_int u4_check_flag = 0;
    u_int u4_support_flag = 0;
    u_char u1_prev_state = 0;
    u_char addrstr[64];
    
    bgp4_printf_peer(p_peer,addrstr);
    bgp4_log(BGP_DEBUG_EVT,1,"process end of rib from %s,afi/safi %d/%d",addrstr,afi,safi);  
    
    /*obtain local supported af-flag*/
    u4_support_flag = p_peer->send_capability;
    
    /*get flag according to afi/safi*/ 
    u4_check_flag = bgp4_afi_to_index(afi, safi);
    
    /*if either side not support gr capability,ignore*/  
    if ((gBgp4.gr_enable == 0) ||
        (p_peer->remote.reset_enable == 0))
    {
        bgp4_log(BGP_DEBUG_EVT,1,"peer not support graceful restart,ignore it");
        
        return TRUE;
    }
    
    /*if already rcvd the same end-of-rib,ignore*/
    if (af_isset(p_peer->rib_end_af,u4_check_flag))
    {
        bgp4_log(BGP_DEBUG_EVT,1,"end of rib has been processed previously");
        
        return TRUE ;
    }
    
    /*set the end-of-rib flag*/
    af_set(p_peer->rib_end_af ,u4_check_flag);
    
    /*clear rest stale route for the afi/safi*/
    bgp4_delete_stale_route(p_peer,u4_check_flag,0);
    
    /*if all end-rib recieved,may some special process:check if 
    need send init update
    */  
    if (p_peer->rib_end_af == u4_support_flag)
    {
        /*stop reset timer*/
        bgp4_stop_peer_gr_timer(p_peer);
        
        /*record prev state,new state will be normal*/
        u1_prev_state = p_peer->reset_role ;

        p_peer->local.reset_bit =  0;
        
        p_peer->reset_role = BGP4_GR_ROLE_NORMAL;
        
        bgp4_log(BGP_DEBUG_EVT,1,"peer %s all end of rib received,set role to normal,prev restart state %d",
            addrstr,u1_prev_state);         
        
        /*if prev state is restart,check if can send first update*/
        if (u1_prev_state == BGP4_GR_ROLE_RESTART)
        {
            bgp4_gr_send_all_init_update();             
            
        }
    }
    
    return TRUE;
}

/*bgp4_mh_send_end_of_rib
send end-of-rib update for a special afi/safi
in peer,afi/safi
out send packet
return none
*/
void bgp4_send_rib_end(tBGP4_PEER *p_peer, u_int af)
{
    u_char buf[32] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                /*len*/0,0,/*type*/2,/*unfeasible length*/
                0,0,/*attrlen*/0,0,/*MP-unreach*/0x90,15,0,3,
                0,0,0,0,0};
    u_char *p_msg_len = &buf[17];
    u_char *p_attr_len = &buf[22];
    u_char *p_afi = &buf[28];
    u_char *p_safi = &buf[29];
    u_short afi = bgp4_index_to_afi(af);
    u_short safi = bgp4_index_to_safi(af);
    u_char  addrstr[64];
       
    bgp4_log(BGP_DEBUG_EVT,1,"send end of rib to %s,afi/safi %d/%d",
                    bgp4_printf_peer(p_peer,addrstr),
                    afi,
                    safi);

    /*send end-of-rib at last,only if it has not been sent before*/
    if(af_isset(p_peer->send_rib_end_af,af))
    {
        return;
    }
    
    /*fill message*/ 
    if ((afi == BGP4_AF_IP) && 
            (safi == BGP4_SAF_UCAST))
    {
        *p_msg_len = 23;
    }
    else
    {
        *p_msg_len = 30;
        *p_attr_len  = 7;
        *p_afi = afi;/*normal afi is one byte,so set it directly*/          
        *p_safi = safi;
    }
        
#if 0
    /*20090224 agilent test---Bgp4RestartingSpeakerRestartFlags  before the DUT restart, 
    R bit = 0,and Reserved bit = 0;  after restarted,R bit = 1,and Reserved bit = 0.*/  
    /*agilent test---Bgp4RestartingSpeakerForwardingStatePreservation  
    if DUT passively restart, then R bit = 0,and F bit = 0;  if it actively restart,then R bit = 1,and F bit = 1*/ 
    if(p_peer->reset_role == BGP4_GR_ROLE_RESTART)  
    { 
        p_peer->local.reset_bit = 1;
        af_set(p_peer->local.reset_af,af);  
    }  
    else  
    {  
        p_peer->local.reset_bit = 0;
        af_clear(p_peer->local.reset_af ,af);  
    }
#endif  

    bgp4_sock_send(p_peer, buf, *p_msg_len);

    /*set EOB sent flag for such af*/
    af_set(p_peer->send_rib_end_af,af);
    
    return ;
}



/*update message contruct ok,need sends*/
__inline__ u_char bgp4_finish_update(tBGP4_PEER *p_peer, tBGP4_UPDATE_INFO *p_update)
{
    u_char addr[64];

    /*msg header*/ 
    bgp4_init_msg_hdr(p_update->buf, BGP4_MSG_UPDATE);
    /*length*/
    bgp4_fill_2bytes(p_update->buf + BGP4_MARKER_LEN, p_update->len);
    
    /*withdraw length*/                                
    bgp4_fill_2bytes(p_update->buf + BGP4_HLEN, p_update->with_len);        
    
    /*attr length*/
    bgp4_fill_2bytes(p_update->buf + BGP4_HLEN + 2 + p_update->with_len, p_update->attr_len);
    
    if (bgp4_sock_send(p_peer, p_update->buf, p_update->len) == FALSE) 
    {
        bgp4_log(BGP_DEBUG_UPDATE,1,
                    "send update to %s ,w %d,f %d.failed",
                    bgp4_printf_peer(p_peer,addr),
                    p_update->with_count,
                    p_update->fea_count);
        
        /*reset buffer*/    
        bgp4_init_update(p_update);
        return FALSE ;
    }

    bgp4_log(BGP_DEBUG_UPDATE,1, "send update to %s , w:%d,f %d.", 
                bgp4_printf_peer(p_peer,addr),
                p_update->with_count,
                p_update->fea_count);

    p_peer->tx_withdraw += p_update->with_count;
    p_peer->tx_feasible += p_update->fea_count;
    p_peer->tx_update++;

    /*reset buffer*/    
    bgp4_init_update(p_update);
    return TRUE;
}

/*send update to peer,*/
void bgp4_send_update_to_peer(tBGP4_PEER *p_peer,
                                  u_int af,
                                  tBGP4_LIST *p_flist,
                                  tBGP4_LIST *p_wlist)
{    
    u_char attr[2048] ;
    tBGP4_LINK *p_link = NULL;
    tBGP4_LINK *p_next = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_first_route = NULL;
    tBGP4_PATH *p_path = NULL ;
    tBGP4_PATH *p_out_path = NULL ;
    tBGP4_UPDATE_INFO update;
    u_short path_len = 0 ;
    u_short filled_len = 0 ;
    u_int rtcnt = 0;
    u_int local_rtcnt = 0 ; 
            
    /*GR:before sending init update,restarting node must wait for
    end-of-rib from all valid peer.ddp*/
    if (bgp4_gr_if_can_update() != TRUE)
    {
        bgp4_log(BGP_DEBUG_CMN,1,"graceful restarting node must wait for end of rib,do not send update to such peer");        
        return;
    }                

    if ((bgp4_lstfirst(p_flist) == NULL)&&(bgp4_lstfirst(p_wlist) == NULL))
    {
        bgp4_log(BGP_DEBUG_CMN,1,"no route send to peer");        
        return;
    }

    if (af != BGP4_PF_IPUCAST)
    {
        bgp4_send_mp_update_to_peer(p_peer, af, p_flist, p_wlist);
        return ;
    }

    LST_LOOP_SAFE(p_flist, p_link, p_next, node, tBGP4_LINK)
    {
        p_route  = p_link->p_route;   
        if(bgp4_check_route_policy(p_route,p_peer,BGP4_POLICY_EXPORT_DIRECTION)==BGP4_ROUTE_DENY)
        {
            u_char route_str[64] = {0};
                bgp4_log(BGP_DEBUG_RT,1,"route %s is filtered by bgp4 check route policy,will not be send out in feasible",
                                bgp4_printf_route(p_route,route_str));
                
            bgp4_rtlist_delete(p_flist, p_link);
            gBgp4.export_filtered_route_count ++;
        }
    }   

    bgp4_init_update(&update);

    /*insert withdraw routes*/
    LST_LOOP(p_wlist, p_link, node, tBGP4_LINK)
    {
        p_route  = p_link->p_route; 

        /*exceed max length, send update first*/
        if ((update.len + bgp4_bit2byte(p_route->dest.prefixlen) + 1) > gBgp4.max_len)
        {
              /*if (++fragment%10 == 0)
                    vos_pthread_delay(1);*/
            bgp4_finish_update(p_peer, &update);
        }
        /*insert nlri*/
        filled_len = bgp4_fill_nlri(update.p_msg, p_route);         
        update.p_msg += filled_len;
        update.with_len  += filled_len;       
        update.len += filled_len;

        update.with_count++ ;
    }
        
    /*only withdraw route sending*/
    if (bgp4_lstfirst(p_flist) == NULL)
    {
        bgp4_finish_update(p_peer, &update);
        return ;
    }

    update.p_msg += 2;
    /*send update with feasible routes*/
    /*enter sending loop,input route list may have multipe path attribute,so 
    we need send one update for each path attribute*/
    while (bgp4_lstfirst(p_flist))
    {
        /*prepare send update for the first path*/
        p_link = (tBGP4_LINK *)bgp4_lstfirst(p_flist) ; 
        p_first_route = p_link->p_route ;
        p_path = p_first_route->p_path ;
        p_out_path = p_first_route->p_out_path;

        /*build attribute for this path*/
        path_len = bgp4_fill_attr(af,p_peer, p_first_route, attr); 

        /*send msg if space full*/
        if ((update.len + path_len) > gBgp4.max_len)
        {
            bgp4_finish_update(p_peer, &update);
            /*skip 2bytes path len*/
            update.p_msg += 2;
        }
                
        /*fill attribute*/
        memcpy(update.p_msg, attr, path_len);
        update.p_msg += path_len;
        update.len += path_len;
        update.attr_len += path_len ;

        /*scan for all routes,add route with same path*/
        LST_LOOP_SAFE(p_flist, p_link, p_next, node, tBGP4_LINK)
        {
            p_route = p_link->p_route; 

            if(p_out_path == NULL)
            {
                if((p_route != p_first_route)&&(p_route->p_out_path)&&(bgp4_same_path(p_path, p_route->p_out_path) == FALSE))
                    continue;
                else if((p_route != p_first_route)&&bgp4_same_path(p_path,p_route->p_path) == FALSE)
                    continue;
            }
            else
            {
                if((p_route != p_first_route)&&(p_route->p_out_path)&&(bgp4_same_path(p_out_path,p_route->p_out_path) == FALSE))
                    continue;
                else if((p_route != p_first_route)&&(bgp4_same_path(p_out_path, p_route->p_path) == FALSE))
                    continue;
            }

            /*send msg if space full*/
            if ((update.len + bgp4_bit2byte(p_route->dest.prefixlen) + 1) > gBgp4.max_len)
            {
                break;
            }  
            filled_len = bgp4_fill_nlri(update.p_msg, p_route);
            update.len += filled_len;
            update.p_msg += filled_len;
            update.fea_count ++ ;
            /*remove route from list*/

            if(p_route->p_out_path)
            {
                bgp4_path_free(p_route->p_out_path);
                memset(p_route->p_out_path,0,4);
            }
            
            bgp4_rtlist_delete(p_flist, p_link);

            if (is_bgp_route(p_route))
            {
                rtcnt ++;
            }
            else
            {
                local_rtcnt++;
            }
        }
        if (update.fea_count)
        {
            bgp4_finish_update(p_peer, &update);
            /*skip 2bytes path len*/
            update.p_msg += 2;
        }
    }
    if (rtcnt > 0)
    {
        p_peer->remote.update_time =  time(NULL) ;
    }
    if (local_rtcnt > 0)
    {
        p_peer->local.update_time =  time(NULL) ;
    }

    return ;
}


/*send update to peer for mutiprotocol*/
void bgp4_send_mp_update_to_peer(tBGP4_PEER *p_peer,
                                  u_int af,
                                  tBGP4_LIST *p_flist,
                                  tBGP4_LIST *p_wlist)
{
    u_char mp[1600];/*contaning mp attribute,do not include flag&type&len,af/safi*/
    u_char nexthop[64];/*mpnexthop,64 is enough*/
    tBGP4_LINK *p_link = NULL;
    tBGP4_LINK *p_next = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_first_route = NULL;
    tBGP4_PATH *p_path = NULL ;
    tBGP4_PATH *p_out_path = NULL ;
    tBGP4_UPDATE_INFO update ;
    u_short len = 0 ;
    u_short filllen;
    u_short nlrilen;
    u_int fixlen = 7;/*4bytes option head + 3 bytes afi/safi*/
    u_short path_len = 0 ;
    u_short nexthop_len = 0 ;

    LST_LOOP_SAFE(p_flist, p_link, p_next, node, tBGP4_LINK)
    {
        p_route  = p_link->p_route;   
        if(bgp4_check_route_policy(p_route,p_peer,BGP4_POLICY_EXPORT_DIRECTION)==BGP4_ROUTE_DENY)
        {
            u_char route_str[64] = {0};
                bgp4_log(BGP_DEBUG_RT,1,"route %s is filtered by bgp4 check route policy,will not be send out in feasible",
                                bgp4_printf_route(p_route,route_str));
                
            bgp4_rtlist_delete(p_flist, p_link);
            gBgp4.export_filtered_route_count ++;
        }
    }   
        
    bgp4_init_update(&update);

    update.p_msg += 2;

    /*insert withdraw routes*/
    LST_LOOP(p_wlist, p_link, node, tBGP4_LINK)
    {
        p_route  = p_link->p_route;   
        /*get length of route*/
        nlrilen = bgp4_fill_mp_nlri(af,NULL, p_route);
        /*length too long,send now*/
        if ((update.len + fixlen + len + nlrilen) > gBgp4.max_len)
        {
            /*build mp-unreach attribute*/
            filllen = bgp4_fill_mp_unreach(update.p_msg, af, mp, len);
            update.len += filllen;
            update.attr_len += filllen;
            bgp4_finish_update(p_peer, &update);
            /*reset total nlrilen*/
            len = 0 ;
            update.p_msg += 2;
        }
        /*fill mp-nlri*/
        len += bgp4_fill_mp_nlri(af,mp + len, p_route);
        update.with_count++;
    }

    /*build mp-unreach attribute*/
    if (len)
    {
        filllen = bgp4_fill_mp_unreach(update.p_msg, af, mp, len);
        update.p_msg+=filllen;
        update.len += filllen;
        update.attr_len += filllen;
        len = 0 ;
    }
    
    /*only withdraw route sending*/
    if (bgp4_lstfirst(p_flist) == NULL)
    {
        bgp4_finish_update(p_peer, &update);
        return ;
    }
            
    /*feasible route*/   
    while (bgp4_lstfirst(p_flist))
    {
        /*prepare send update for the first path*/      
        p_link = (tBGP4_LINK *)bgp4_lstfirst(p_flist) ; 
        p_first_route = p_link->p_route ;
        p_path = p_first_route->p_path ;
        p_out_path = p_first_route->p_out_path;

        /*build attribute for this path*/
        path_len = bgp4_fill_attr(af,p_peer, p_first_route, mp); 

        nexthop_len = bgp4_fill_mp_nexthop(af,p_peer, nexthop, p_first_route);
        
        /*decide length,if exceed,send first,consider 1 byte rsvd field*/
        if ((update.len + fixlen + path_len + nexthop_len + 1) > gBgp4.max_len)
        {
            /*no new attribute need,send directly*/
            bgp4_finish_update(p_peer, &update);
            /*reset total nlrilen*/
            len = 0 ;
            update.p_msg += 2;
        }
        /*copy path,update length*/
        memcpy(update.p_msg, mp, path_len);
        update.p_msg += path_len ;
        update.len += path_len ;
        update.attr_len += path_len ;
        
        /*nexthop is copied later*/

        /*scan for all routes,add route with same path*/
        LST_LOOP_SAFE(p_flist, p_link, p_next, node, tBGP4_LINK)
        {
            p_route = p_link->p_route;    

            if(p_out_path == NULL)
            {
                if((p_route != p_first_route)&&(p_route->p_out_path)&&(bgp4_same_path(p_path, p_route->p_out_path) == FALSE))
                    continue;
                else if((p_route != p_first_route)&&bgp4_same_path(p_path,p_route->p_path) == FALSE)
                    continue;
            }
            else
            {
                if((p_route != p_first_route)&&(p_route->p_out_path)&&(bgp4_same_path(p_out_path,p_route->p_out_path) == FALSE))
                    continue;
                else if((p_route != p_first_route)&&(bgp4_same_path(p_out_path, p_route->p_path) == FALSE))
                    continue;
            }

            /*send msg if space full*/
            nlrilen = bgp4_fill_mp_nlri(af,NULL, p_route);
            if ((update.len + fixlen + nexthop_len + 1 + len + nlrilen) > gBgp4.max_len)
            {
                break;
            }
                        
            /*copy nlri into mp*/
            len += bgp4_fill_mp_nlri(af,mp+len, p_route);
            update.fea_count ++ ;

            if(p_route->p_out_path)
            {
                bgp4_path_free(p_route->p_out_path);
                memset(p_route->p_out_path,0,4);
            }
            
            /*remove route from list*/                      
            bgp4_rtlist_delete(p_flist, p_link);
        }
                
        if (update.fea_count)
        {
            /*construct mp-reach nlti*/
            filllen = bgp4_fill_mp_reach(update.p_msg, af, nexthop, nexthop_len, mp, len);
            update.len += filllen;
            update.attr_len += filllen;
            len = 0 ;            
            bgp4_finish_update(p_peer, &update);
            update.p_msg += 2;
        }
    }

    return ;
}



#endif
