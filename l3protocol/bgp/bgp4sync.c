
#include "bgp4sync.h"

#ifdef NEW_BGP_WANTED
/*****************************************************************************
  bgp4sync.c :sync operation
******************************************************************************/

#include "bgp4com.h"

#define MASTER_SLAVE_MODE 1


#define MASTER_SLAVE_MODE_MASTER    1
#define MASTER_SLAVE_MODE_SLAVE 2


int master_slave_get_api(u_int uiCmd,void *pValue)
{
    
}

u_int 
bgp4_get_workmode(void)
{
   u_int new_mode = BGP4_MODE_OTHER;
   #if 0
   if ((uspHwScalarGet((void *)NULL, HW_SYS_SWRUNNINGROLEISMASTER, NULL) == VOS_OK)&&
        (uspHwScalarGet(NULL,HW_SYS_HAROLEISMASTER,NULL) == VOS_OK))
   {
         new_mode = BGP4_MODE_MASTER;
   }
   else if((uspHwScalarGet((void *)NULL, HW_SYS_SWRUNNINGROLEISSLAVE, NULL) == VOS_OK)||
        (uspHwScalarGet(NULL,HW_SYS_HAROLEISSLAVE,NULL) == VOS_OK))
   {
         new_mode = BGP4_MODE_SLAVE;
   }
   #endif
   master_slave_get_api(MASTER_SLAVE_MODE,&new_mode);
   if(MASTER_SLAVE_MODE_MASTER == new_mode)
   {
       new_mode = BGP4_MODE_MASTER;
   }
   else if(MASTER_SLAVE_MODE_SLAVE == new_mode)
   {
       new_mode = BGP4_MODE_SLAVE;
   }
   else
   {
         new_mode = BGP4_MODE_OTHER;
   }
   
   
   if (new_mode != gbgp4.work_mode)
   {
        bgp4_log(BGP_DEBUG_SYNC,"mode change: old: %d,set new:%d(1:master,2:slave,3:other)", gbgp4.work_mode,new_mode); 
   }

   gbgp4.work_mode = new_mode;

   
   return new_mode;
}

/*fill send buffer,send if it is full*/
void 
bgp4_sync_send(
       u_char *p_buf,
       u_int len,
       u_int flag)
{
    octetstring octet = {0};
    #if 0 /*TODO:添加主备同步消息发送*/
    u_int sync_cmd = HW_SYS_BGP4CMDSTART + BGP_GBL_SYNPKT;
    int rc = VOS_OK;

    octet.len = len;
    octet.pucBuf = p_buf;

    bgp4_log(BGP_DEBUG_SYNC, "send sync msg len %d,flag %x", octet.len, flag|gbgp4.sync_flag);

    rc = uspHwScalarSync(NULL, sync_cmd, &octet, flag|gbgp4.sync_flag);
    
    if (rc != VOS_OK)
    {
        gbgp4.stat.sync.fail++;
        bgp4_log(BGP_DEBUG_SYNC|BGP_DEBUG_ERROR,"sync SEND ERROR!");
    }
    #endif
    return;
}

/*fill peer item*/
void 
bgp4_sync_peer_send(
    tBGP4_PEER *p_peer)
{
    u_char buf[BGP4_SYNC_BUFSIZE];
    tBGP4_SYNC_MSG_HDR *p_hdr = (tBGP4_SYNC_MSG_HDR*)buf;
    tBGP4_SYNC_PEER *p_msg = NULL;
    u_char addr[64] = {0};
    u_int len = BGP4_SYNC_HDRLEN + sizeof(tBGP4_SYNC_PEER);

    memset(buf, 0, len);

    /*********fill sync msg header*********/
    p_hdr->sync_action = TRUE;
    p_hdr->type = BGP4_SYNC_PEER;
    p_hdr->len = htons(sizeof(tBGP4_SYNC_PEER));

    /*********copy peer info*********/
    p_msg = (tBGP4_SYNC_PEER*)(buf + BGP4_SYNC_HDRLEN);
    memcpy(&p_msg->remote_ip,&p_peer->ip,sizeof(tBGP4_ADDR));

    p_msg->vrf_id = htonl(p_peer->p_instance->vrf);
    p_msg->router_id = htonl(p_peer->router_id);
    p_msg->af = htonl(p_peer->mpbgp_capability); 
    p_msg->reset_af = htonl(p_peer->restart_mpbgp_capability); 
    p_msg->neg_hold_interval = htonl(p_peer->neg_hold_interval); 
    p_msg->neg_keep_interval = htonl(p_peer->neg_keep_interval); 
    p_msg->rxd_rib_end = htonl(p_peer->rxd_rib_end); 
    /*statstic*/
    p_msg->established_transitions = htonl(p_peer->stat.established_transitions); 
    p_msg->uptime = htonl(p_peer->stat.uptime); 
    p_msg->rx_updatetime = htonl(p_peer->stat.rx_updatetime); 
    p_msg->rx_update = htonl(p_peer->stat.rx_update); 
    p_msg->tx_update = htonl(p_peer->stat.tx_update); 
    p_msg->rx_msg = htonl(p_peer->stat.rx_msg); 
    p_msg->tx_msg = htonl(p_peer->stat.tx_msg); 
    p_msg->last_errcode = htons(p_peer->stat.last_errcode); 
    p_msg->last_subcode = htons(p_peer->stat.last_subcode); 
    p_msg->peer_if_unit = htonl(p_peer->if_unit);
    p_msg->reset_time = htons(p_peer->restart_time); 
    p_msg->refresh_enable = p_peer->refresh_enable;
    p_msg->in_restart = p_peer->in_restart;
    p_msg->reset_enable = p_peer->restart_enable;
    p_msg->state = p_peer->state;
    p_msg->version = p_peer->version;
    p_msg->event = p_peer->stat.event;
    p_msg->orf_recv = htonl(p_peer->orf_recv);
    p_msg->orf_send = htonl(p_peer->orf_send);

    /*********send buffer*********/
    bgp4_log(BGP_DEBUG_SYNC,"sync vrf %d peer %s,sync len %d",
                ntohl(p_msg->vrf_id),
                bgp4_printf_addr(&p_peer->ip,addr), sizeof(tBGP4_SYNC_PEER));

    bgp4_sync_send(buf, len, USP_SYNC_LOCAL|USP_SYNC_OCTETDATA);
    return;
}
/*send orf*/
void 
bgp4_sync_orf_send(
    tBGP4_PEER *p_peer,
    tBGP4_ORF_ENTRY *p_orf)
{
    u_char buf[BGP4_SYNC_BUFSIZE];
    tBGP4_SYNC_MSG_HDR *p_hdr = (tBGP4_SYNC_MSG_HDR*)buf;
    tBGP4_SYNC_PEER_ORF *p_msg = NULL;
    u_char addr[64] = {0};
    u_int len = BGP4_SYNC_HDRLEN + sizeof(tBGP4_SYNC_PEER_ORF);

    memset(buf, 0, len);

    /*********fill sync msg header*********/
    p_hdr->sync_action = TRUE;
    p_hdr->type = BGP4_SYNC_PEER;
    p_hdr->len = htons(sizeof(tBGP4_SYNC_PEER_ORF));

    /*********copy peer info*********/
    p_msg = (tBGP4_SYNC_PEER_ORF*)(buf + BGP4_SYNC_HDRLEN);
    
    memcpy(&p_msg->remote_ip, &p_peer->ip, sizeof(tBGP4_ADDR));
    p_msg->vrf_id = htonl(p_peer->p_instance->vrf);
    p_msg->seq = htonl(p_orf->seqnum);
    p_msg->match = p_orf->match;
    p_msg->minlen = p_orf->minlen;
    p_msg->maxlen = p_orf->maxlen;
    p_msg->rowstatus = p_orf->rowstatus;

    memcpy(&p_msg->prefix, &p_orf->prefix, sizeof(tBGP4_ADDR));
    
    /*********send buffer*********/
    bgp4_log(BGP_DEBUG_SYNC,"sync vrf %d orf,sync len %d",
                ntohl(p_msg->vrf_id),
                bgp4_printf_addr(&p_peer->ip,addr), sizeof(tBGP4_SYNC_PEER_ORF));

    bgp4_sync_send(buf, len, USP_SYNC_LOCAL|USP_SYNC_OCTETDATA);
    return;
}


/*sync update msg directly*/
void 
bgp4_sync_pdu_send(
        tBGP4_PEER *p_peer,
        u_char *p_pdu,
        u_int len)
{
    u_char buf[BGP4_SYNC_BUFSIZE];
    tBGP4_SYNC_MSG_HDR *p_hdr = NULL;
    tBGP4_SYNC_PDU *p_msg = NULL;
    u_char addr[64] = {0};
    u_int total_len = BGP4_SYNC_HDRLEN + sizeof(tBGP4_SYNC_PDU) - BGP4_MAX_MSG_LEN + len;
        
    if (len == 0)
    {
        bgp4_log(BGP_DEBUG_SYNC,"pdu len is zero,error!");
        return;
    }

    if (total_len > BGP4_SYNC_BUFSIZE)
    {
        bgp4_log(BGP_DEBUG_SYNC, "sync pdu len %d too long,error!", len);
        return;
    }

    memset(buf, 0, total_len);

    /*********fill sync msg header*********/
    p_hdr = (tBGP4_SYNC_MSG_HDR*)buf;
    p_hdr->sync_action = 1;/*meaningless*/
    p_hdr->type = BGP4_SYNC_PDU;
    p_hdr->len = htons(total_len - BGP4_SYNC_HDRLEN);

    /*********copy update info*********/
    p_msg = (tBGP4_SYNC_PDU*)(buf + BGP4_SYNC_HDRLEN);
    memcpy(&p_msg->remote_ip, &p_peer->ip, sizeof(tBGP4_ADDR));
    p_msg->vrf_id = htonl(p_peer->p_instance->vrf);
    p_msg->msg_len = htonl(len);
    memcpy(p_msg->msg, p_pdu, len);
    
    gbgp4.stat.sync.tx++;
    /*********send buffer*********/
    bgp4_sync_send(buf, total_len, USP_SYNC_LOCAL|USP_SYNC_OCTETDATA/*|USP_SYNC_NONBLOCK*/);

    bgp4_log(BGP_DEBUG_SYNC,"bgp vrf %d pdu is sent by peer %s,payload length %d,pdu length %d",
                ntohl(p_msg->vrf_id),
                bgp4_printf_addr(&p_peer->ip,addr),
                ntohs(p_hdr->len),
                len);
    return;
}

void
bgp4_sync_fragment_send(
        tBGP4_PEER *p_peer,
        u_char *p_fragment,
        u_int fragment_len,
        u_int msg_id,
        u_int fragment_id)
{
    u_char buf[BGP4_SYNC_BUFSIZE];
    u_char str[256];
    u_char *p_data = NULL;
    tBGP4_SYNC_MSG_HDR *p_hdr = NULL;
    tBGP4_SYNC_FRAGMENT_PDU *p_msg = NULL;
    u_char addr[64] = {0};
    u_int total_len = BGP4_SYNC_HDRLEN + sizeof(tBGP4_SYNC_FRAGMENT_PDU) - BGP4_MAX_MSG_LEN + fragment_len;

    memset(buf, 0, total_len);

    p_hdr = (tBGP4_SYNC_MSG_HDR*)buf;
    p_hdr->sync_action = 1;/*meaningless*/
    p_hdr->type = BGP4_SYNC_FRAGMENT_PDU;
    p_hdr->len = htons(total_len - BGP4_SYNC_HDRLEN);

    /*********copy update info*********/
    p_msg = (tBGP4_SYNC_FRAGMENT_PDU*)(buf + BGP4_SYNC_HDRLEN);
    memcpy(&p_msg->remote_ip, &p_peer->ip, sizeof(tBGP4_ADDR));
    p_msg->vrf_id = htonl(p_peer->p_instance->vrf);
    p_msg->msg_id = htons(msg_id);
    p_msg->fragment_id = fragment_id;
    p_msg->fragment_len = htons(fragment_len);
    memcpy(p_msg->msg, p_fragment, fragment_len);

    gbgp4.stat.sync.tx++;
    bgp4_sync_send(buf, total_len, USP_SYNC_LOCAL|USP_SYNC_OCTETDATA/*|USP_SYNC_NONBLOCK*/);

    bgp4_log(BGP_DEBUG_SYNC, "vrf %d fragment pdu is sent by %s,fragment length %d, id %d",
                ntohl(p_msg->vrf_id),
                bgp4_printf_addr(&p_peer->ip,addr),
                fragment_len,
                fragment_id);
    /*display first 16bytes*/
    if (gbgp4.debug & BGP_DEBUG_SYNC)
    {
        p_data = p_msg->msg;
        sprintf(str, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
            p_data[0], p_data[1], p_data[2], p_data[3],
            p_data[4], p_data[5], p_data[6], p_data[7],
            p_data[8], p_data[9], p_data[10], p_data[11],
            p_data[12], p_data[13], p_data[14], p_data[15]);
        bgp4_log(BGP_DEBUG_SYNC, "fragment data %s",str);
    }
    return;
}

void
bgp4_sync_fragment_pdu_send(
        tBGP4_PEER *p_peer,
        u_char *p_pdu,
        u_int len)
{
    u_short fragment_len[4];
    u_char *fragment_msg[4];
    u_short average = len/4;
    u_int i;
    
    /*decide fragment length*/
    fragment_len[0] = average;
    fragment_len[1] = average;
    fragment_len[2] = average;
    fragment_len[3] = len - (average * 3);

    /*decide fragment msg pointer*/
    fragment_msg[0] = p_pdu;
    fragment_msg[1] = fragment_msg[0] + fragment_len[0];
    fragment_msg[2] = fragment_msg[1] + fragment_len[1];
    fragment_msg[3] = fragment_msg[2] + fragment_len[2];

    /*decide new fragment id*/
    gbgp4.sync_pdu_id++;
    
    /*send sync for each fragment*/
    for (i = 0; i < 4; i++)
    {
        bgp4_sync_fragment_send(p_peer, 
               fragment_msg[i], 
               fragment_len[i], 
               gbgp4.sync_pdu_id, 
               i);
    }
    return;
}

void 
bgp4_sync_update_send(tBGP4_UPDATE_PACKET *p_packet)
{
    tBGP4_PEER *p_peer = p_packet->p_peer;

    /*msg header*/ 
    bgp4_init_msg_hdr(p_packet->p_buf, BGP4_MSG_UPDATE);
                    
    /*length*/
    bgp4_fill_2bytes(p_packet->p_buf + BGP4_MARKER_LEN, p_packet->len);
    
    /*withdraw length*/                                
    bgp4_fill_2bytes(p_packet->p_buf + BGP4_HLEN, p_packet->withdraw_len);        
    
    /*attr length*/
    bgp4_fill_2bytes(p_packet->p_buf + BGP4_HLEN + 2 + p_packet->withdraw_len, p_packet->attribute_len);

    /*************for sync update route***************/
    bgp4_sync_pdu_send(p_peer, p_packet->p_buf, p_packet->len);

    bgp4_update_packet_init(p_packet);
    return;
}

u_short 
bgp4_sync_aspath_fill(
        u_char *p_msg, 
        tBGP4_PATH *p_path)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_char as_count = 0;
    u_short len = 0;
    u_char *p_start = p_msg;
    if ((p_path != NULL) && (bgp4_avl_first(&p_path->aspath_list))) 
    { 
        /*contain normal as set/seq*/
        bgp4_avl_for_each(&p_path->aspath_list,p_aspath)
        {  
            len += 2; /*type + count*/
            len += (p_aspath->count * 2 );                 
        }
    }
    
    p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_PATH, len);
    
    if (len)
    {
        bgp4_avl_for_each(&p_path->aspath_list, p_aspath)
        {
            /*type*/
            bgp4_put_char(p_msg, p_aspath->type); 

            /*as-count*/
            as_count = p_aspath->count;    
            bgp4_put_char(p_msg, as_count); 

            /*as-part,one as contain 2 bytes*/
            bgp4_put_string(p_msg, p_aspath->as, (as_count * 2));
        }
    }    
    return ((u_long)p_msg - (u_long)p_start);
}


u_short 
bgp4_sync_as4path_fill(
        u_char *p_msg, 
        tBGP4_PATH *p_path)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_char as_count = 0;
    u_short len = 0;
    u_char *p_start = p_msg;
    if ((p_path != NULL) && (bgp4_avl_first(&p_path->as4path_list))) 
    { 
        /*contain normal as set/seq*/
        bgp4_avl_for_each(&p_path->as4path_list,p_aspath)
        {  
            len += 2; /*type + count*/
            len += (p_aspath->count * 4);                 
        }
    }
    
    p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_NEW_PATH, len);
    
    if (len)
    {
        bgp4_avl_for_each(&p_path->as4path_list, p_aspath)
        {
            /*type*/
            bgp4_put_char(p_msg, p_aspath->type); 

            /*as-count*/
            as_count = p_aspath->count;    
            bgp4_put_char(p_msg, as_count); 

            /*as-part,one as contain 2 bytes*/
            bgp4_put_string(p_msg, p_aspath->as, (as_count * 4));
        }
    }    
    return ((u_long)p_msg - (u_long)p_start);
}

u_short  
bgp4_sync_aggregate_fill(
        u_char *p_msg, 
        tBGP4_ROUTE *p_route)
{
    u_short asnum = 0;          
    tBGP4_PATH *p_path = NULL;
    u_int aggr = 0;
    u_char *p_start = p_msg ;   
   
    p_path = p_route->p_path;
    
       /* Filling the Atomic Aggregate field  */
    if ((is_bgp_route(p_route)) && (p_path->atomic_exist) ) 
    {
        p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_ATOMIC_AGGR,BGP4_ATOMIC_LEN);
    }
    
    /* Filling the Aggregator field  */          
    if ((is_bgp_route(p_route)) && p_path->p_aggregator)
    {        
        p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_AGGREGATOR,BGP4_AGGR_LEN);
        asnum = p_path->p_aggregator->as; 
        aggr = bgp_ip4(p_path->p_aggregator->ip.ip);
        bgp4_put_2bytes(p_msg, asnum);
        bgp4_put_4bytes(p_msg, aggr);   
    } 
    return ((u_long)p_msg - (u_long)p_start);
}

u_short  
bgp4_sync_community_fill(
           u_char *p_msg, 
           tBGP4_PATH *p_path)
{
    u_char len = 0; 
    u_char *p_start = p_msg;
    
    /*decide length*/
    len += p_path->community_len;

    if (len && p_path->p_community)
    {
        p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_COMMUNITY, len);

        bgp4_put_string(p_msg, p_path->p_community, p_path->community_len);
    }
    return ((u_long)p_msg-(u_long)p_start);
}

u_short 
bgp4_sync_ecommunity_fill(
                 u_char *p_msg, 
                 tBGP4_PATH *p_path)
{
    /*for mpls vpn route target*/
    u_char *p_start = p_msg;

    if (p_path->excommunity_len && p_path->p_ecommunity)
    {
        p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_EXT_COMMUNITY, p_path->excommunity_len);
        memcpy(p_msg, p_path->p_ecommunity, p_path->excommunity_len);
        p_msg += p_path->excommunity_len;
    }
    return ((u_long)p_msg - (u_long)p_start );
}

u_short  
bgp4_sync_cluster_fill(
         u_char *p_msg, 
         tBGP4_PATH *p_path)
{
    u_char len = p_path->cluster_len;
    u_char *p_start = p_msg;
    if (len && p_path->p_cluster)
    {
        p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_CLUSTERLIST, len);
        bgp4_put_string(p_msg, p_path->p_cluster, len);
    }
    return ((u_long)p_msg - (u_long)p_start );
}

/*fill attribute,not include mpreach and unreach*/
u_short 
bgp4_sync_path_fill(
         tBGP4_ROUTE *p_route,
         u_char *p_msg)         
{
    tBGP4_PATH *p_path = NULL; 
    u_long msg_start = (u_long)p_msg;

    if ((p_route == NULL)||(p_msg == NULL))
    {
        return 0;
    }
    
    p_path = p_route->p_path;
    if (p_path == NULL)
    {
        return 0;
    }

    /*fill origin*/
    p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_ORIGIN, BGP4_ORIGIN_LEN);
    bgp4_put_char(p_msg,p_path->origin);

    /*fill aspath*/
    p_msg += bgp4_sync_aspath_fill(p_msg, p_path);

    /*fill 4bytes aspath*/
    p_msg += bgp4_sync_as4path_fill(p_msg, p_path);
    
    /*  Filling the Next Hop ,in MBGP,do not send nexthop*/
    if (p_route->dest.afi == BGP4_PF_IPUCAST)
    {
        p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_NEXT_HOP, BGP4_NEXTHOP_LEN);
        bgp4_put_4bytes(p_msg, bgp_ip4(p_path->nexthop.ip));
    }

    /*fill med*/ 
    if(p_path->med_exist)
    {
        p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_MED,BGP4_MED_LEN) ;  
        bgp4_put_4bytes(p_msg, p_path->med);
    }

    /*fill lp*/
    if(p_path->local_pref_exist)
    {
        p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_LOCAL_PREF,BGP4_LPREF_LEN) ;
        bgp4_put_4bytes(p_msg, p_path->localpref); 
    }

    p_msg +=  bgp4_sync_aggregate_fill(p_msg, p_route);

    p_msg +=  bgp4_sync_community_fill(p_msg, p_path);

    /*fill origin id*/
    if(p_path->origin_id)
    {
        p_msg += bgp4_attribute_hdr_fill(p_msg, BGP4_ATTR_ORIGINATOR, BGP4_ORIGINATOR_LEN);
        bgp4_put_4bytes(p_msg, p_path->origin_id); 
    }

    p_msg += bgp4_sync_cluster_fill(p_msg, p_path);

    p_msg += bgp4_sync_ecommunity_fill(p_msg, p_path);

    p_msg += bgp4_unkown_fill(p_msg, p_path);

    return (((u_long)p_msg) - msg_start);        
    
}

u_short 
bgp4_sync_mpnexthop_fill(
               u_char *p_msg,
               tBGP4_ROUTE *p_route)         
{
    tBGP4_PATH *p_path = NULL;
    u_char  hoplen = 0;
    u_char *p_nexthop = NULL;
    u_int nhop = 0 ;
    u_char ip6_nhop[32];
    u_char flag = 0;

    if(p_msg == NULL || p_route == NULL)
    {
        return 0;
    }

    p_nexthop = (p_msg + 1);/*empty 1 byte for nexthop len*/
    p_path = p_route->p_path;

    if (p_path == NULL)
    {
        return 0;
    }
    
    memset(ip6_nhop, 0,32);

    
    /*fill for different nexthop type*/
    switch (p_route->dest.afi)
    {
        case BGP4_PF_IPUCAST:
        case BGP4_PF_IPMCAST:
        case BGP4_PF_IPLABEL:
        {
            if (*(u_int*)p_path->nexthop.ip )                 
            {
                hoplen = 4;
                nhop = *(u_int*)p_path->nexthop.ip;
            }
            else 
            {
                return 0;
            }
            break;
        }
        case BGP4_PF_IPVPN:    
        {
            if ((*(u_int*)p_path->nexthop.ip ))
            {
                /*nexthop's RD is 0(8 bytes)*/
                memset(p_nexthop, 0, 8);                                          
                p_nexthop += 8;
                   
                hoplen = 12;
                nhop = bgp_ip4(p_path->nexthop.ip);
            }
            else 
            {
                return 0;
            }
                
            break ;
        }
        case BGP4_PF_IP6VPN:/*mpls l3 vpn only use ipv4 backbone network,so ipv4 nexthop is valid*/
        /*|                80 位                 | 16 |      32 位          |
        +--------------------------------------+--------------------------+
        |0000..............................0000|FFFF|    IPv4 地址     |
        +--------------------------------------+----+---------------------+*/
        {
            
            if (*(u_int*)p_path->nexthop.ip)
            {
                /*8 bytes 0 for RD*/
                memset(ip6_nhop, 0, 8);
                memset(ip6_nhop+8, 0, 10);
                memset(ip6_nhop+18, 0xff, 2);
            
                memcpy(ip6_nhop+20,p_path->nexthop.ip,4);
                hoplen = 24 ;  
            }
            else
            {
                return 0;
            }            
            
            break ;
        }
        case BGP4_PF_IP6LABEL:
        /*|                80 位                 | 16 |      32 位          |
        +--------------------------------------+--------------------------+
        |0000..............................0000|FFFF|    IPv4 地址     |
        +--------------------------------------+----+---------------------+*/
        {
            if (*(u_int*)p_path->nexthop.ip )
            {
                memset(ip6_nhop, 0, 10);
                memset(ip6_nhop+10, 0xff, 2);
                memcpy(ip6_nhop+12,p_path->nexthop.ip,4);
                hoplen = 16 ;  
            }
            else
            {
                return 0;
            }    
            break ;
        }        
        case BGP4_PF_IP6UCAST:
        case BGP4_PF_IP6MCAST:
        {
            if (memcmp(ip6_nhop, p_path->nexthop.ip, 16) != 0)
            {
                hoplen += 16;
                memcpy(ip6_nhop, p_path->nexthop.ip, 16);
                flag = 1;
            }
            if (memcmp(ip6_nhop + 16, p_path->linklocal_nexthop.ip, 16) != 0)
            {
                hoplen += 16;
                if (flag)
                {
                    memcpy(ip6_nhop + 16, p_path->linklocal_nexthop.ip, 16);
                }
                else
                {
                    memcpy(ip6_nhop, p_path->linklocal_nexthop.ip, 16);
                }
            }

            if (hoplen != 16 && hoplen != 32)
            {
                return 0;
            }
            break;
        }
        default :    
            return 0;
            break;
    }
    
    if (bgp4_index_to_afi(p_route->dest.afi) == BGP4_AF_IP)
    {
        nhop = ntohl(nhop);
        bgp4_fill_4bytes(p_nexthop, nhop);
    }    
    else if (bgp4_index_to_afi(p_route->dest.afi) == BGP4_AF_IP6)
    {
        memcpy(p_nexthop, ip6_nhop, hoplen);
    }
    
    /*set nexthop length*/
    *(p_msg) = hoplen;
       
    return (hoplen + 1);/*including 1byte of length*/
}

/*if input message is null,just return length*/
u_short 
bgp4_sync_mp_nlri_fill(
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
        bgp4_label_fill(p_buf, p_route->out_label);
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

void 
bgp4_sync_peer_mpupdate_send(
             tBGP4_UPDATE_PACKET *p_packet,
             u_int af,
             u_int count,
             tBGP4_ROUTE **pp_route)
{
    u_char mp[1600];/*contaning mp attribute,do not include flag&type&len,af/safi*/
    u_char nexthop[64];/*mpnexthop,64 is enough*/ 
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_PATH *p_path = NULL;
    u_short len = 0;
    u_short filllen;
    u_short path_len = 0;
    u_short nexthop_len = 0;
    u_int i = 0;

    bgp4_update_packet_init(p_packet);
    p_packet->p_msg += 2;    

    p_route = pp_route[0];
    if (p_route)
    {
        p_path = p_route->p_path;
        
        /*build attribute for this path*/
        path_len = bgp4_sync_path_fill(p_route, mp); 
        nexthop_len = bgp4_sync_mpnexthop_fill(nexthop, p_route);
        /*copy path,update length*/
        memcpy(p_packet->p_msg, mp, path_len);
        p_packet->p_msg += path_len;
        p_packet->len += path_len;
        p_packet->attribute_len += path_len;
    }

    for (i = 0; i < count ; i++, p_route = pp_route[i])
    {
        /*copy nlri into mp*/
        len += bgp4_sync_mp_nlri_fill(mp+len, p_route);
        p_packet->feasible_count++;
    }
    if (p_packet->feasible_count)
    {
        /*construct mp-reach nlti*/
        filllen = bgp4_mp_reach_fill(p_packet->p_msg, af, nexthop, nexthop_len, mp, len);
        p_packet->len += filllen;
        p_packet->attribute_len += filllen;
        bgp4_sync_update_send(p_packet);
    }
    return;
}

u_int 
bgp4_sync_peer_update_send (
            tBGP4_PEER *p_peer,
            u_int count,
            tBGP4_ROUTE **pp_route)
{
    tBGP4_UPDATE_PACKET packet;
    u_char buf[BGP4_MAX_MSG_LEN];
    u_char mpreach[BGP4_MAX_MSG_LEN];
    u_char mpunreach[BGP4_MAX_MSG_LEN];
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_PATH *p_path = NULL;
    u_short path_len = 0;
    u_short filled_len = 0;
    u_int send_pkt = 0;
    u_int af = 0;
    u_int i = 0;
    u_char attr[512]; 

    if (count == 0)
    {
        return 0;
    }
    
    p_route = pp_route[0];
    af = p_route->dest.afi;
    
    memset(&packet, 0, sizeof(packet));
    packet.p_peer = p_peer; 
    packet.p_buf = buf;
    packet.p_mp_reach = mpreach;
    packet.p_mp_unreach = mpunreach;
        
    bgp4_update_packet_init(&packet);
    packet.p_msg += 2;
    
    if (af != BGP4_PF_IPUCAST)
    {
        bgp4_sync_peer_mpupdate_send(&packet, af, count, pp_route);
        return 0;
    }

    /*fill first path*/
    p_path = p_route->p_path;
    /*build attribute for this path*/
    path_len = bgp4_sync_path_fill(p_route, attr); 

    /*fill attribute*/
    memcpy(packet.p_msg, attr, path_len);
    packet.p_msg += path_len;
    packet.len += path_len;
    packet.attribute_len += path_len;
    
    for (i = 0; i < count ; i++, p_route = pp_route[i])
    {
        filled_len = bgp4_nlri_fill(packet.p_msg, p_route);
        packet.len += filled_len;
        packet.p_msg += filled_len;
        packet.feasible_count++;
    }
    if (packet.feasible_count)
    {
        send_pkt++;
        bgp4_sync_update_send(&packet);
        packet.p_msg += 2;
    }
    return send_pkt;
}

/*send all route to backup card*/
void 
bgp4_sync_init_update_send(void)
{
    u_int af = 0;
    tBGP4_PATH *p_path = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_RIB *p_rib = NULL;
    tBGP4_ROUTE *pp_route[BGP4_SYNC_MAX_ROUTE];
    u_char fill[128];
    u_int rt_num = 0;
    u_int rt_send=0;
    u_int nlri_len = 0;
    
    bgp4_instance_for_each(p_instance)
    {
        for (af = 0;af < BGP4_PF_MAX;af++)
        {
            p_rib = p_instance->rib[af];
            if (p_instance->rib[af] == NULL)
            {
                continue;
            }
            bgp4_avl_for_each(&p_rib->path_table, p_path)
            {
                if (p_path->p_peer == NULL)/*only routes learned from other peers should be sent*/
                {
                    continue;
                }
                /*unoriginal vpn routes do not sync to others*/                
                if (p_path->p_instance->vrf != p_path->origin_vrf)
                
                {
                    continue;
                }
                
                memset(pp_route, 0, sizeof(pp_route));
                rt_num = 0;
                nlri_len = 0;
                
                bgp4_avl_for_each(&p_path->route_list, p_route)
                {
                    rt_send++;
                    if (rt_send%5000 == 0)
                    {
                        vos_pthread_delay(1);
                    }
                    pp_route[rt_num] = p_route;
                    rt_num++;

                    /*decide nlri length*/
                    if (af == BGP4_PF_IPUCAST)
                    {
                        nlri_len += bgp4_nlri_fill(fill, p_route);
                    }
                    else
                    {
                        nlri_len += bgp4_sync_mp_nlri_fill(fill, p_route);
                    }
                    /*limit pdu length*/
                    if (nlri_len >= 1200)
                    {
                        bgp4_sync_peer_update_send(p_path->p_peer, rt_num, pp_route);
                        memset(pp_route, 0, sizeof(pp_route));
                        rt_num = 0;
                        nlri_len = 0;
                    }
                }
                if (rt_num > 0)
                {
                    bgp4_sync_peer_update_send(p_path->p_peer, rt_num, pp_route);
                    memset(pp_route, 0, sizeof(pp_route));
                    rt_num = 0;
                }
            }
        }
    }
    return;
}

void 
bgp4_sync_peer_recv(
       tBGP4_SYNC_PEER *p_sync_peer,
       u_int action)
{
    tBGP4_PEER* p_peer = NULL;
    u_char addr[64] = {0};
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_sync_peer->vrf_id);
    if (p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,"peer vpn instance %d not exist,do nothing!",
                    p_sync_peer->vrf_id);
        return;
    }
    
    /*********find the peer if exist*********/
    p_peer = bgp4_peer_lookup(p_instance, &p_sync_peer->remote_ip);
    if (p_peer == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,"peer %s not exist,do nothing!",
                    bgp4_printf_addr(&p_sync_peer->remote_ip,addr));

        return;/*peer conf should sync before*/
    }
        
    bgp4_log(BGP_DEBUG_SYNC,"receive sync peer %s,action modify!",
        bgp4_printf_addr(&p_sync_peer->remote_ip,addr),action);

    /*********get peer info*********/
    p_peer->router_id = ntohl(p_sync_peer->router_id);
    p_peer->mpbgp_capability = ntohl(p_sync_peer->af); 
    p_peer->restart_mpbgp_capability = ntohl(p_sync_peer->reset_af); 
    p_peer->neg_hold_interval = ntohl(p_sync_peer->neg_hold_interval); 
    p_peer->neg_keep_interval= ntohl(p_sync_peer->neg_keep_interval); 
    p_peer->rxd_rib_end = ntohl(p_sync_peer->rxd_rib_end); 
    /*statstic*/
    p_peer->stat.established_transitions = ntohl(p_sync_peer->established_transitions); 
    p_peer->stat.uptime = ntohl(p_sync_peer->uptime); 
    p_peer->stat.rx_updatetime = ntohl(p_sync_peer->rx_updatetime); 
    p_peer->stat.rx_update = ntohl(p_sync_peer->rx_update); 
    p_peer->stat.tx_update = ntohl(p_sync_peer->tx_update); 
    p_peer->stat.rx_msg = ntohl(p_sync_peer->rx_msg); 
    p_peer->stat.tx_msg = ntohl(p_sync_peer->tx_msg); 
    p_peer->stat.last_errcode = ntohl(p_sync_peer->last_errcode); 
    p_peer->stat.last_subcode = ntohl(p_sync_peer->last_subcode); 
    p_peer->if_unit = ntohl(p_sync_peer->peer_if_unit);
    p_peer->restart_time = ntohs(p_sync_peer->reset_time); 
    p_peer->refresh_enable = p_sync_peer->refresh_enable;
    p_peer->in_restart = p_sync_peer->in_restart;
    p_peer->restart_enable = p_sync_peer->reset_enable;
    p_peer->state = p_sync_peer->state;
    p_peer->version = p_sync_peer->version;
    p_peer->stat.event = p_sync_peer->event;
    p_peer->orf_recv = ntohl(p_sync_peer->orf_recv);
    p_peer->orf_send = ntohl(p_sync_peer->orf_send);

    /*FSM checking*/
    if (p_sync_peer->event < BGP4_EVENT_MAX)
    {
        bgp4_fsm(p_peer, p_peer->stat.event);
    }
    return;
}

void 
bgp4_sync_orf_recv(
       tBGP4_SYNC_PEER_ORF *p_sync_orf,
       u_int action)
{
    tBGP4_PEER* p_peer = NULL;
    tBGP4_ORF_ENTRY *p_orf = NULL;
    u_char addr[64] = {0};
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_sync_orf->vrf_id);
    if (p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,"peer vpn instance %d not exist,do nothing!",
                    p_sync_orf->vrf_id);
        return;
    }
    
    /*********find the peer if exist*********/
    p_peer = bgp4_peer_lookup(p_instance, &p_sync_orf->remote_ip);
    if (p_peer == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,"peer %s not exist,do nothing!",
                    bgp4_printf_addr(&p_sync_orf->remote_ip,addr));

        return;/*peer conf should sync before*/
    }
        
    p_orf = bgp4_orf_lookup(&p_peer->orf_out_table, p_sync_orf->prefix.afi, p_sync_orf->seq);

    if (p_sync_orf->rowstatus == SNMP_DESTROY)    
    {
        if (p_orf)
        {
            bgp4_orf_delete(p_orf);
        }
    }
    else
    {
        if (p_orf == NULL)
        {
            p_orf = bgp4_orf_create(&p_peer->orf_out_table, p_sync_orf->prefix.afi, p_sync_orf->seq);
        }

        if (p_orf)
        {
            p_orf->seqnum = ntohl(p_sync_orf->seq);
            p_orf->match = p_sync_orf->match;
            p_orf->minlen = p_sync_orf->minlen;
            p_orf->maxlen = p_sync_orf->maxlen;
            p_orf->rowstatus = p_sync_orf->rowstatus;
        
            memcpy(&p_orf->prefix, &p_sync_orf->prefix, sizeof(tBGP4_ADDR));
        }
    }
    return;
}

void 
bgp4_sync_pdu_recv(tBGP4_SYNC_PDU* p_msg)
{
    tBGP4_PEER* p_peer = NULL;
    u_char addr[64] = {0};
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_msg->vrf_id);
    if (p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,"peer vpn instance %d not exist,do nothing!",
                    p_msg->vrf_id);
        return;
    }
    
    /*********find the peer*********/
    p_peer = bgp4_peer_lookup(p_instance, &p_msg->remote_ip);
    if (p_peer == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,"peer %s not exist,do nothing!",
                bgp4_printf_addr(&p_msg->remote_ip,addr));
        return;                        
    }

    bgp4_log(BGP_DEBUG_SYNC,"receive sync peer %s msg pdu",
                bgp4_printf_addr(&p_msg->remote_ip,addr));

    bgp4_msg_input(p_peer, p_msg->msg);
    return;
}

void
bgp4_sync_fragment_pdu_clear(void)
{
    u_int i = 0;
    for (i = 0; i < 4 ; i++)
    {
        if (gbgp4.sync_fragment_pdu[i])
        {
            bgp4_free(gbgp4.sync_fragment_pdu[i], MEM_BGP_BUF);
            gbgp4.sync_fragment_pdu[i] = NULL;
        }
    }
    return;
}

void
bgp4_sync_fragment_pdu_recv(tBGP4_SYNC_FRAGMENT_PDU* p_msg)
{
    tBGP4_PEER* p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_SYNC_FRAGMENT_PDU *p_current = NULL;
    u_char buf[BGP4_MAX_MSG_LEN];
    u_int buflen = 0;
    u_int fragment_len;
    u_int msg_id;
    u_int fragment_id;
    u_int i = 0;
    u_char *p_data = NULL;
    u_char addr[64] = {0};
    u_char str[256];

    bgp4_log(BGP_DEBUG_SYNC,"receive sync peer %s fragment pdu,vrf %d,id %d,fragment id %d",
                bgp4_printf_addr(&p_msg->remote_ip,addr),
                ntohl(p_msg->vrf_id),
                ntohs(p_msg->msg_id),
                p_msg->fragment_id);    
    /*display first 16bytes*/
    if (gbgp4.debug & BGP_DEBUG_SYNC)
    {
        p_data = p_msg->msg;
        sprintf(str, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
            p_data[0], p_data[1], p_data[2], p_data[3],
            p_data[4], p_data[5], p_data[6], p_data[7],
            p_data[8], p_data[9], p_data[10], p_data[11],
            p_data[12], p_data[13], p_data[14], p_data[15]);
        bgp4_log(BGP_DEBUG_SYNC, "fragment data %s",str);
    }
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, ntohl(p_msg->vrf_id));
    if (p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,"peer vpn instance %d not exist,do nothing!",
                    p_msg->vrf_id);
        return;
    }
    p_peer = bgp4_peer_lookup(p_instance, &p_msg->remote_ip);
    if (p_peer == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,"peer %s not exist,do nothing!",
                bgp4_printf_addr(&p_msg->remote_ip,addr));
        return;                        
    }

    /*get pdu id,if not same,clear current msg information*/
    msg_id = ntohs(p_msg->msg_id);
    fragment_id = p_msg->fragment_id;
    fragment_len = ntohs(p_msg->fragment_len);

    for (i = 0; i < 4 ; i++)
    {
        if (gbgp4.sync_fragment_pdu[i])
        {
            p_current = gbgp4.sync_fragment_pdu[i];
            if (p_current->msg_id != msg_id)
            {
                bgp4_log(BGP_DEBUG_SYNC,"clear current fragment pdu,msg id %d",
                    p_current->msg_id);
                
                bgp4_sync_fragment_pdu_clear();
                break;
            }
        }
    }

    /*ignore invalid fragment id*/
    if (fragment_id > 3)
    {
        bgp4_log(BGP_DEBUG_SYNC,"invalid fragment id %d", fragment_id);
        return;
    }

    /*ignore existed fragment*/
    if (gbgp4.sync_fragment_pdu[fragment_id])
    {
        bgp4_log(BGP_DEBUG_SYNC,"fragment pdu id exist", fragment_id);
        return;
    }

    /*create buffer to store input fragment*/
    p_current = bgp4_malloc(sizeof(tBGP4_SYNC_FRAGMENT_PDU) - BGP4_MAX_MSG_LEN + fragment_len, MEM_BGP_BUF);
    if (p_current == NULL)
    {
        return;
    }
    gbgp4.sync_fragment_pdu[fragment_id] = p_current;
    
    /*copy data*/
    p_current->msg_id = msg_id;
    p_current->fragment_len = fragment_len;
    memcpy(p_current->msg, p_msg->msg, fragment_len);

    /*check if all data rxd,if so,rebuild full msg,and call input*/
    for (i = 0; i < 4 ; i++)
    {
        if (gbgp4.sync_fragment_pdu[i] == NULL)
        {
            return;
        }
    }
    for (i = 0; i < 4 ; i++)
    {
        p_current = gbgp4.sync_fragment_pdu[i];
        memcpy(buf + buflen, p_current->msg, p_current->fragment_len);
        buflen += p_current->fragment_len;
    }
    bgp4_log(BGP_DEBUG_SYNC,"all fragment pdu receive ,total length %d", buflen);

    bgp4_msg_input(p_peer, buf);
    return;
}

/*check sync event and send sync msg*/
void 
bgp4_sync_init_send(void)
{
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int event = 0;
    
    if (gbgp4.work_mode != BGP4_MODE_MASTER)
    {
        bgp4_log(BGP_DEBUG_SYNC,"sync role  error,work mode %d (1:master,2:slave,3:other),do not send init sync msg!",
                gbgp4.work_mode);
        return;
    }
    
    /********send peer table*********/
    bgp4_instance_for_each(p_instance)
    {
        bgp4_peer_for_each(p_instance, p_peer) 
        {
            /*set ignored FSM event*/
            event = p_peer->stat.event;
            p_peer->stat.event = BGP4_EVENT_MAX;
            
            bgp4_sync_peer_send(p_peer);
            
            /*recover event*/
            p_peer->stat.event = event;
        }
    }
    
    vos_pthread_delay(60);
    /********send route update*********/
    bgp4_sync_init_update_send();
    return;
}

/*receive sync data,by slave*/
void 
bgp4_sync_recv(octetstring *p_octet)
{
    u_char *p_msg = NULL;
    u_int msg_len = 0;
    u_int read_len = 0;
    tBGP4_SYNC_MSG_HDR* p_hdr = NULL;

    if (bgp4_get_workmode() != BGP4_MODE_SLAVE)
    {
        bgp4_log(BGP_DEBUG_SYNC,"sync role  error,work mode %d (1:master,2:slave,3:other),do not process sync msg!",
                gbgp4.work_mode);
        return;
    }

    bgp4_log(BGP_DEBUG_SYNC,"Receive sync msgs,msg total length %d",p_octet->len);

    for (p_msg = p_octet->pucBuf; 
         read_len < p_octet->len ; 
         read_len += msg_len)
    {
        p_hdr = (tBGP4_SYNC_MSG_HDR *)(p_msg + read_len);
        msg_len = ntohs(p_hdr->len) + BGP4_SYNC_HDRLEN;

        if(msg_len <= BGP4_SYNC_HDRLEN)
        {
            bgp4_log(BGP_DEBUG_SYNC,"sync msg len error,msg length %d",msg_len);
            break;
        }

        bgp4_log(BGP_DEBUG_SYNC,"extract a sync msg,msg length %d,sync data length %d",
            msg_len,msg_len - BGP4_SYNC_HDRLEN);

        switch (p_hdr->type) 
        {
            case BGP4_SYNC_PEER:
            {
                bgp4_sync_peer_recv((tBGP4_SYNC_PEER*)(p_hdr + 1),p_hdr->sync_action);
                break;
            }

            case BGP4_SYNC_PDU:
            {
                gbgp4.stat.sync.rx++;
                bgp4_sync_pdu_recv((tBGP4_SYNC_PDU*)(p_hdr + 1));
                break;
            }            

            case BGP4_SYNC_FRAGMENT_PDU:
                bgp4_sync_fragment_pdu_recv((tBGP4_SYNC_FRAGMENT_PDU*)(p_hdr + 1));
                break;

            case BGP4_SYNC_ORF:
                bgp4_sync_orf_recv((tBGP4_SYNC_PEER_ORF *)(p_hdr + 1), p_hdr->sync_action);
                break;
                
            default :
            {
                bgp4_log(BGP_DEBUG_SYNC,"invalid sync msg type %d,ignore it",p_hdr->type);            
                break;
            }
        }
    }
    return;
}

/*state change from slave to master.restart holding timer for established
  nbr*/
void
bgp4_sync_become_master(void)    
{
   tBGP4_PEER *p_peer = NULL;
   tBGP4_VPN_INSTANCE *p_instance = NULL;
   u_int af = 0;
   u_char str[64];

   /*clear current fd set*/
   FD_ZERO(gbgp4.fd_write);
   FD_ZERO(gbgp4.fd_read); 
   
   bgp4_instance_for_each(p_instance)
   {
       bgp4_peer_for_each(p_instance, p_peer)
       {
           /*restart peer restarting timer*/
           if (p_peer->in_restart == TRUE)
           {
               bgp4_timer_start(&p_peer->gr_timer, p_peer->restart_time);
           }

           /*decide socket to be used*/
           /*invalid case:both server and client socket exist,or not exist*/
           if ((p_peer->sync_client_sock > 0)
               && (p_peer->sync_server_sock > 0))
           {
               bgp4_log(BGP_DEBUG_SYNC, "peer %s server and client socket exist %d %d", 
                 bgp4_printf_peer(p_peer, str),
                 p_peer->sync_server_sock,
                 p_peer->sync_client_sock); 
           }
           if ((p_peer->sync_client_sock <= 0)
               && (p_peer->sync_server_sock <= 0))
           {
               bgp4_log(BGP_DEBUG_SYNC, "peer %s server and client socket not exist", 
                 bgp4_printf_peer(p_peer, str)); 
           }

           /*use stored sync socket information if current socket not exist*/
           if (p_peer->sock <= 0)
           {
               p_peer->sock = (p_peer->sync_client_sock > 0) ? p_peer->sync_client_sock : p_peer->sync_server_sock;
           }
           
           if (p_peer->state == BGP4_NS_ESTABLISH)
           {
               if (p_peer->sock <= 0)
               {                   
                   bgp4_log(BGP_DEBUG_SYNC, "peer %s state is establish,without tcp connection", bgp4_printf_peer(p_peer, str)); 
                   bgp4_timer_start(&p_peer->hold_timer, 1);
                   continue;
               }
               
               bgp4_timer_start(&p_peer->hold_timer, p_peer->neg_hold_interval);
               bgp4_timer_start(&p_peer->keepalive_timer, 1);
               bgp4_peer_tcp_addr_fill(p_peer);
           }
           else
           {
               bgp4_log(BGP_DEBUG_SYNC, "peer %s state is establish when switch to master", bgp4_printf_peer(p_peer, str)); 

               /*delete invalid route*/
               if (p_peer->in_restart != TRUE)
               {
                   for (af = 0 ; af < BGP4_PF_MAX; af++)
                   {
                       bgp4_peer_route_clear(af, p_peer);
                   }
                   bgp4_fsm(p_peer, BGP4_EVENT_HOLD_TMR_EXP);
               }
           }
           /*clear all sync socket*/
           p_peer->sync_client_sock = 0;
           p_peer->sync_server_sock = 0;
       }
   }
   gbgp4.max_sock = bgp4_max_peer_socket(0);
   return;
}

#else
/*****************************************************************************
  bgp4sync.c :sync operation
******************************************************************************/

#include "bgp4com.h"
#include "uspIfApi.h"


extern void bgp4_fill_tcp_address(tBGP4_PEER *p_peer);
extern u_char * bgp4_printf_addr(tBGP4_ADDR *p, u_char *str);
extern void bgp4_debug_packet(u_char *p_msg,u_short len);
u_short bgp4_fill_attr_hdr(u_char * p_msg, u_char type,u_short len);


void bgp4_tcp_sync_add(tBGP4_VPN_INSTANCE* p_instance,u_int af,u_char* p_laddr,u_char* p_faddr,u_int lport,u_int fport,int sockfd)
{
    tBGP4_ADDR peer_ip;
    u_char ipv6_str[64] = {0};
    u_char addr[64] = {0};
    u_char *ipv4_flag="::ffff:";
    tBGP4_PEER *p_peer = NULL;
    int on = 1;
    u_int buflen = BGP4_TCP_BUFFER_LEN;


    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_TCP,1,"bgp4 tcp sync add input instance null");
        return;
    }

    /*************find matched peer*******************/
    memset(&peer_ip, 0, sizeof(tBGP4_ADDR));
    if(af == 0)/*0:IPv4*/
    {
        memcpy(peer_ip.ip,p_faddr,4);
        peer_ip.afi = BGP4_PF_IPUCAST;
        peer_ip.prefixlen =32;
    }
    else if(af == 1)/*1:IPv6*/
    {
        
        memset(ipv6_str, 0, sizeof(ipv6_str));
        inet_ntop(AF_INET6,p_faddr,ipv6_str,sizeof(ipv6_str));
                    
        if(strstr(ipv6_str,ipv4_flag))
        {
            peer_ip.afi = BGP4_PF_IPUCAST ;
            bgp_ip4(peer_ip.ip) = *(u_int *)(p_faddr + 12);
            peer_ip.prefixlen =32;
        }
        else
        {
            peer_ip.afi = BGP4_PF_IP6UCAST;
            memcpy(&peer_ip.ip, p_faddr, 16);
            peer_ip.prefixlen =128;
        }
    }
    
    /*************if find,reset fd/addr/port/unit*******************/
    p_peer = bgp4_peer_lookup(p_instance,&peer_ip);
    if (p_peer) 
    {
        gBgp4.add_check_tcp_count++;
        bgp4_log(BGP_DEBUG_EVT,1,"find matched peer %s,record new sock %d in slave",
            bgp4_printf_addr(&p_peer->remote.ip,addr),sockfd);
            
        /*two connections may exist at same time,record both*/  
        p_peer->last_sync_fd = p_peer->sock;/*retain last sock fd*/
        p_peer->sock = sockfd;/*record new sock*/
        
        /*Set Sock Option*/
        bgp4_set_sock_noblock(p_peer->sock,on);             
        bgp4_set_sock_tcpnodelay(p_peer->sock,on);
        bgp4_set_sock_txbuf(p_peer->sock,buflen);
        bgp4_tcp_set_peer_md5(p_peer->sock, p_peer);
        bgp4_tcp_set_peer_ttl(p_peer->sock, p_peer);
        
    }
        
    return;

}


void bgp4_tcp_sync_del(tBGP4_VPN_INSTANCE* p_instance,u_int af,u_char* p_laddr,u_char* p_faddr,u_int lport,u_int fport,int sockfd)
{
    tBGP4_PEER *p_peer = NULL;
    u_char addr[64] = {0};
    tBGP4_ADDR peer_ip;
    u_char *ipv4_flag="::ffff:";
    u_char ipv6_str[64] = {0};
    
    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_TCP,1,"bgp4 tcp sync del input instance null");
        return;
    }

    gBgp4.del_check_tcp_count++;

    /*************find matched peer*******************/
    memset(&peer_ip, 0, sizeof(tBGP4_ADDR));
    if(af == 0)/*0:IPv4*/
    {
        memcpy(peer_ip.ip,p_faddr,4);
        peer_ip.afi = BGP4_PF_IPUCAST;
        peer_ip.prefixlen =32;
    }
    else if(af == 1)/*1:IPv6*/
    {
        memset(ipv6_str, 0, sizeof(ipv6_str));
        inet_ntop(AF_INET6,p_faddr,ipv6_str,sizeof(ipv6_str));
                    
        if(strstr(ipv6_str,ipv4_flag))
        {
            peer_ip.afi = BGP4_PF_IPUCAST ;
            bgp_ip4(peer_ip.ip) = *(u_int *)(p_faddr + 12);
            peer_ip.prefixlen =32;
        }
        else
        {
            peer_ip.afi = BGP4_PF_IP6UCAST;
            memcpy(&peer_ip.ip, p_faddr, 16);
            peer_ip.prefixlen =128;
        }
    }

    p_peer = bgp4_peer_lookup(p_instance,&peer_ip);

    if (p_peer) 
    {
        gBgp4.add_check_tcp_count++;
        bgp4_log(BGP_DEBUG_EVT,1,"find matched peer %s,sock %d has been deleted",
            bgp4_printf_addr(&p_peer->remote.ip,addr),sockfd);

        /*two connections may exist at same time,should decide which one is to be deleted*/
        if(p_peer->last_sync_fd == sockfd)
        {
            p_peer->last_sync_fd = 0;
        }

        if(p_peer->sock == sockfd)/*retained fd must be in use now(zero fd means be deleted)*/
        {
            p_peer->sock = p_peer->last_sync_fd;
        }   
        
    }

    return;
}

u_int bgp4_get_workmode()
{
   u_int new_mode = BGP4_MODE_OTHER;
   
   if((uspHwScalarGet((void *)NULL, HW_SYS_SWRUNNINGROLEISMASTER, NULL) == VOS_OK)&&
        (uspHwScalarGet(NULL,HW_SYS_HAROLEISMASTER,NULL) == VOS_OK))
   {
         new_mode = BGP4_MODE_MASTER;
   }
   else if((uspHwScalarGet((void *)NULL, HW_SYS_SWRUNNINGROLEISSLAVE, NULL) == VOS_OK)||
        (uspHwScalarGet(NULL,HW_SYS_HAROLEISSLAVE,NULL) == VOS_OK))
   {
         new_mode = BGP4_MODE_SLAVE;
   }
   else
   {
         new_mode = BGP4_MODE_OTHER;
   }
   
   if (new_mode != gBgp4.work_mode)
   {
        bgp4_log(BGP_DEBUG_SYNC,1,"\r\nmode change: old: %d,set new:%d(1:master,2:slave,3:other)", gBgp4.work_mode,new_mode); 
   }

   gBgp4.work_mode = new_mode;

   
   return new_mode;
}

/*fill send buffer,send if it is full*/
void bgp4_send_sync_msg(u_char* p_send_buff,u_int send_len,u_int sync_flag)
{
    octetstring octet = {0};
    u_int sync_cmd = HW_SYS_BGP4CMDSTART + BGP_GBL_SYNPKT;
    int rc = VOS_OK;

    octet.len = send_len;
    octet.pucBuf = p_send_buff;

    bgp4_log(BGP_DEBUG_SYNC,1 ,"send sync msg ,total msg length %d,sync_flag %x",octet.len,sync_flag|gBgp4.sync_flag);

    rc = uspHwScalarSync(NULL, sync_cmd, &octet, sync_flag|gBgp4.sync_flag);
    
    if (rc != VOS_OK)
    {
        gBgp4.stat.sync_msg_fail++;
        bgp4_log(BGP_DEBUG_SYNC,1 ,"sync SEND ERROR!");
    }
    
        return ;
}

/*fill peer item*/
void bgp4_send_sync_peer(tBGP4_PEER* p_peer,u_int action)
{
    u_char send_buff[BGP4_SYNC_BUFSIZE];
    tBGP4_SYNC_MSG_HDR* p_msg_hdr = (tBGP4_SYNC_MSG_HDR*)send_buff;
    tBGP4_SYNC_PEER* p_sync_peer = NULL;
    u_char addr[64] = {0};
    u_int total_len = BGP4_SYNC_HDRLEN + sizeof(tBGP4_SYNC_PEER);

    if(p_peer == NULL || p_peer->p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,1, "bgp4 send sync peer peer is NULL,ERROR!");
        return;
    }   

    memset(send_buff,0,BGP4_SYNC_BUFSIZE);

    /*********fill sync msg header*********/
    p_msg_hdr->sync_action = action;
    p_msg_hdr->type = BGP4_SYNC_PEER;
    p_msg_hdr->len = htons(sizeof(tBGP4_SYNC_PEER));

    /*********copy peer info*********/
    p_sync_peer = (tBGP4_SYNC_PEER*)(send_buff + BGP4_SYNC_HDRLEN);
    memcpy(&p_sync_peer->remote_ip,&p_peer->remote.ip,sizeof(tBGP4_ADDR));

    p_sync_peer->vrf_id = htonl(p_peer->p_instance->instance_id);
    p_sync_peer->router_id = htonl(p_peer->router_id);
    p_sync_peer->af = htonl(p_peer->remote.af); 
    p_sync_peer->reset_af = htonl(p_peer->remote.reset_af); 
    p_sync_peer->capability = htonl(p_peer->remote.capability); 
    p_sync_peer->neg_hold_interval = htonl(p_peer->neg_hold_interval); 
    p_sync_peer->neg_keep_interval = htonl(p_peer->neg_keep_interval); 
    p_sync_peer->rib_end_af = htonl(p_peer->rib_end_af); 
    p_sync_peer->send_capability = htonl(p_peer->send_capability); 
    /*statstic*/
    p_sync_peer->established_transitions = htonl(p_peer->established_transitions); 
    p_sync_peer->uptime = htonl(p_peer->uptime); 
    p_sync_peer->rx_updatetime = htonl(p_peer->rx_updatetime); 
    p_sync_peer->rx_update = htonl(p_peer->rx_update); 
    p_sync_peer->tx_update = htonl(p_peer->tx_update); 
    p_sync_peer->rx_msg = htonl(p_peer->rx_msg); 
    p_sync_peer->tx_msg = htonl(p_peer->tx_msg); 
    p_sync_peer->last_errcode = htons(p_peer->last_errcode); 
    p_sync_peer->last_subcode = htons(p_peer->last_subcode); 
    p_sync_peer->peer_if_unit = htonl(p_peer->if_unit);

    p_sync_peer->reset_time= htons(p_peer->remote.reset_time); 
    p_sync_peer->refresh = p_peer->remote.refresh;
    p_sync_peer->reset_bit = p_peer->remote.reset_bit;
    p_sync_peer->reset_enable = p_peer->remote.reset_enable;
    p_sync_peer->state = p_peer->state;
    p_sync_peer->version = p_peer->version;
    p_sync_peer->event = p_peer->event;

    /*********send buffer*********/
    bgp4_log(BGP_DEBUG_SYNC,1 ,"bgp4 send sync peer sync vrf %d peer %s,sync length %d",
                ntohl(p_sync_peer->vrf_id),
                bgp4_printf_addr(&p_peer->remote.ip,addr),sizeof(tBGP4_SYNC_PEER));

    bgp4_send_sync_msg(send_buff,total_len,USP_SYNC_LOCAL|USP_SYNC_OCTETDATA);

    return;

}

/*fill peer fsm*/
void bgp4_send_sync_fsm(tBGP4_PEER* p_peer)
{
    u_char send_buff[BGP4_SYNC_BUFSIZE];
    tBGP4_SYNC_MSG_HDR* p_msg_hdr = (tBGP4_SYNC_MSG_HDR*)send_buff;
    tBGP4_SYNC_FSM* p_sync_fsm = NULL;
    u_char addr[64] = {0};
    u_int total_len = BGP4_SYNC_HDRLEN + sizeof(tBGP4_SYNC_FSM);

    if(p_peer == NULL|| p_peer->p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,1, "bgp4 send sync fsm peer is NULL,ERROR!");
        return;
    }   

    memset(send_buff,0,BGP4_SYNC_BUFSIZE);

    /*********fill sync msg header*********/
    p_msg_hdr->sync_action = 1;/*meaningless*/
    p_msg_hdr->type = BGP4_SYNC_PEER_FSM;
    p_msg_hdr->len = htons(sizeof(tBGP4_SYNC_FSM));
    
    /*********copy peer info*********/
    p_sync_fsm = (tBGP4_SYNC_FSM*)(send_buff + BGP4_SYNC_HDRLEN);
    memcpy(&p_sync_fsm->remote_ip,&p_peer->remote.ip,sizeof(tBGP4_ADDR));
    p_sync_fsm->vrf_id = htonl(p_peer->p_instance->instance_id);

    
    p_sync_fsm->state = p_peer->state;
    p_sync_fsm->event = p_peer->event;
    p_sync_fsm->peer_if_unit = htonl(p_peer->if_unit);

    /*********send buffer*********/
    bgp4_send_sync_msg(send_buff,total_len,USP_SYNC_LOCAL|USP_SYNC_OCTETDATA);

    bgp4_log(BGP_DEBUG_SYNC,1 ,"bgp4 send sync fsm sync vrf %d peer %s FSM,peer interface unit %d,sync len %d",
                ntohl(p_sync_fsm->vrf_id),
                bgp4_printf_addr(&p_peer->remote.ip,addr),p_peer->if_unit,sizeof(tBGP4_SYNC_FSM));


    return;
}

/*sync update msg directly*/
void bgp4_send_sync_pdu(tBGP4_PEER *p_peer,u_char *p_pdu,u_int pdu_len)
{
    u_char send_buff[BGP4_SYNC_BUFSIZE];
    tBGP4_SYNC_MSG_HDR* p_msg_hdr = NULL;
    tBGP4_SYNC_PDU* p_sync_pdu = NULL;
    u_char addr[64] = {0};
    u_int total_len = BGP4_SYNC_HDRLEN + sizeof(tBGP4_SYNC_PDU) - BGP4_MAX_MSG_LEN + pdu_len;

    if(p_peer == NULL || p_pdu == NULL|| p_peer->p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,1, "bgp4 send sync pdu, peer is NULL or pdu is NULL,ERROR!");
        return;
    }

    if(pdu_len == 0)
    {
        bgp4_log(BGP_DEBUG_SYNC,1, "bgp4 send sync pdu, pdu length is zero,ERROR!");
        return;
    }

    if(total_len > BGP4_SYNC_BUFSIZE)
    {
        bgp4_log(BGP_DEBUG_SYNC,1, "bgp4 send sync pdu,sync pdu length %d too long,ERROR!",pdu_len);
        return;
    }

    
    memset(send_buff,0,BGP4_SYNC_BUFSIZE);

    /*********fill sync msg header*********/
    p_msg_hdr = (tBGP4_SYNC_MSG_HDR*)send_buff;
    p_msg_hdr->sync_action = 1;/*meaningless*/
    p_msg_hdr->type = BGP4_SYNC_ROUTE_UPDATE;
    p_msg_hdr->len = htons(total_len - BGP4_SYNC_HDRLEN);

    /*********copy update info*********/
    p_sync_pdu = (tBGP4_SYNC_PDU*)(send_buff + BGP4_SYNC_HDRLEN);
    memcpy(&p_sync_pdu->remote_ip,&p_peer->remote.ip,sizeof(tBGP4_ADDR));
    p_sync_pdu->vrf_id = htonl(p_peer->p_instance->instance_id);
    p_sync_pdu->msg_len = htonl(pdu_len);
    memcpy(p_sync_pdu->msg,p_pdu,pdu_len);
    
    gBgp4.stat.sync_msg_send++;
    /*********send buffer*********/
    bgp4_send_sync_msg(send_buff,total_len,USP_SYNC_LOCAL|USP_SYNC_OCTETDATA/*|USP_SYNC_NONBLOCK*/);

    bgp4_log(BGP_DEBUG_EVT,1 ,"bgp4 send sync pdu,bgp vrf %d pdu is sent by peer %s,payload length %d,pdu length %d",
                ntohl(p_sync_pdu->vrf_id),
                bgp4_printf_addr(&p_peer->remote.ip,addr),
                ntohs(p_msg_hdr->len),
                pdu_len);


}

void bgp4_finish_sync_update(tBGP4_UPDATE_INFO *p_update,tBGP4_PEER *p_peer)
{
    /*msg header*/ 
    bgp4_init_msg_hdr(p_update->buf, BGP4_MSG_UPDATE);
                    
    /*length*/
    bgp4_fill_2bytes(p_update->buf + BGP4_MARKER_LEN, p_update->len);
    
    /*withdraw length*/                                
    bgp4_fill_2bytes(p_update->buf + BGP4_HLEN, p_update->with_len);        
    
    /*attr length*/
    bgp4_fill_2bytes(p_update->buf + BGP4_HLEN + 2 + p_update->with_len,p_update->attr_len);

    bgp4_debug_packet(p_update->buf,p_update->len);

    /*************for sync update route***************/
    bgp4_send_sync_pdu(p_peer,p_update->buf,p_update->len);

    bgp4_init_update(p_update);

}

u_short bgp4_fill_sync_aspath(u_char *p_msg, tBGP4_PATH *p_path)
{
    tBGP4_ASPATH *p_aspath = NULL;
    u_char as_count = 0;
    u_short len = 0;
    u_char *p_start = p_msg ;

    if ((p_path != NULL) && (bgp4_lstfirst(&p_path->aspath_list))) 
    { 
        /*contain normal as set/seq*/
        LST_LOOP(&p_path->aspath_list,p_aspath, node, tBGP4_ASPATH)
        {  
            len += 2; /*type + count*/
            len += (p_aspath->len * 2 );                 
        }
            
    }
    
    p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_PATH, len);
    
    if(len)
    {
        
        LST_LOOP(&p_path->aspath_list,p_aspath, node, tBGP4_ASPATH)
        {
            /*type*/
            bgp4_put_char(p_msg, p_aspath->type); 

            /*as-count*/
            as_count = p_aspath->len;    
            bgp4_put_char(p_msg, as_count); 

            /*as-part,one as contain 2 bytes*/
            bgp4_put_string(p_msg, p_aspath->p_asseg, (as_count * 2));
        }
    }   
      
    return ((u_int)p_msg - (u_int)p_start);
}

u_short  bgp4_fill_sync_aggregate(u_char *p_msg, tBGP4_ROUTE *p_route)
{
    u_short asnum = 0;          
    tBGP4_PATH *p_path = NULL;
    u_int aggr = 0;
    u_char *p_start = p_msg ;   
   
    p_path = p_route->p_path;
    
    /* Filling the Atomic Aggregate field  */
    if ((is_bgp_route(p_route)) && (p_path->flags.atomic) ) 
    {
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_ATOMIC_AGGR,BGP4_ATOMIC_LEN);
    }
    
    /* Filling the Aggregator field  */          
    if ((is_bgp_route(p_route)) && (p_path->flags.aggr)) 
    {        
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_AGGREGATOR,BGP4_AGGR_LEN);
        asnum = p_path->aggregator.asnum; 
        aggr = bgp_ip4(p_path->aggregator.addr.ip);
        bgp4_put_2bytes(p_msg, asnum);
        bgp4_put_4bytes(p_msg, aggr);   
    } 
    return ((u_int)p_msg - (u_int)p_start);
}
u_short  bgp4_fill_sync_community(u_char  *p_msg, 
                           tBGP4_PATH * p_path)
{
    u_char i = 0 ;
    u_char len = 0 ; 
    u_char *p_start = p_msg ;
    
    /*decide length*/
    for (i = 0 ; i < BGP4_MAX_COMMNUITY ; i++)
    {
        if (p_path->community[i])
        {
            len += 4;
        }
    }

    if(len)
    {
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_COMMUNITY, len);

        for (i = 0 ; i < BGP4_MAX_COMMNUITY ; i++)
        {
            if (p_path->community[i])
            {
                bgp4_put_4bytes(p_msg, p_path->community[i]);
            }
        }

    }
    
    return ((u_int)p_msg-(u_int)p_start);
}

u_short bgp4_fill_sync_ext_community(u_char  *p_msg, 
                              tBGP4_PATH * p_path)
{
    /*for mpls vpn route target*/
    u_char *p_start = p_msg ;
    u_int len = 0;

    if(p_path == NULL || p_msg == NULL)
    {
        return 0;
    }

    if(p_path->flags.excommunity_count)
    {
        len = p_path->flags.excommunity_count*BGP4_VPN_RT_LEN;
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_EXT_COMMUNITY, len);
        memcpy(p_msg,p_path->ecommunity,len);
        p_msg += len;
    }
    
    return ((u_int)p_msg - (u_int)p_start );
}

u_short  bgp4_fill_sync_cluster_list(u_char  *p_msg, 
                              tBGP4_PATH* p_path)
{
    u_char len = 0;
    u_char i = 0 ;
    u_char *p_start = p_msg ;

    /*cluster id len*/
    for (i = 0 ; i < BGP4_MAX_CLUSTERID ; i++)
    {
        if (p_path->cluster[i])
        len += 4;
    }

    if(len)
    {
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_CLUSTERLIST, len);
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

/*fill attribute,not include mpreach and unreach*/
u_short bgp4_build_sync_attr (tBGP4_ROUTE   *p_route,u_char *p_msg )         
{
    tBGP4_PATH  *p_path = NULL; 
    u_int msg_start = (u_int)p_msg;

    if ((p_route == NULL)||(p_msg == NULL))
    {
        return 0;
    }
    
    p_path   = p_route->p_path;

    if(p_path == NULL)
    {
        return 0;
    }

    /*fill origin*/
    p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_ORIGIN, BGP4_ORIGIN_LEN);
    bgp4_put_char(p_msg,p_path->origin);

    /*fill aspath*/
    p_msg += bgp4_fill_sync_aspath(p_msg, p_path);

    /*  Filling the Next Hop ,in MBGP,do not send nexthop*/
    if (p_route->dest.afi == BGP4_PF_IPUCAST)
    {
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_NEXT_HOP, BGP4_NEXTHOP_LEN);
        bgp4_put_4bytes(p_msg, bgp_ip4(p_path->nexthop.ip));
    }

    /*fill med*/ 
    if(p_path->flags.med)
    {
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_MED,BGP4_MED_LEN) ;  
        bgp4_put_4bytes(p_msg, p_path->rcvd_med);
    }

    /*fill lp*/
    if(p_path->flags.lp)
    {
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_LOCAL_PREF,BGP4_LPREF_LEN) ;
        bgp4_put_4bytes(p_msg, p_path->rcvd_localpref); 
    }

    p_msg +=  bgp4_fill_sync_aggregate(p_msg, p_route);

    p_msg +=  bgp4_fill_sync_community(p_msg, p_path);

    /*fill origin id*/
    if(p_path->origin_id)
    {
        p_msg += bgp4_fill_attr_hdr(p_msg, BGP4_ATTR_ORIGINATOR, BGP4_ORIGINATOR_LEN);
        bgp4_put_4bytes(p_msg, p_path->origin_id); 
    }

    p_msg += bgp4_fill_sync_cluster_list(p_msg, p_path);

    p_msg += bgp4_fill_sync_ext_community(p_msg, p_path);

    p_msg += bgp4_fill_unkown(p_msg, p_path);

    return (((u_int)p_msg) - msg_start);        
    
}

u_short bgp4_fill_sync_mp_nexthop(u_char *p_msg,
                              tBGP4_ROUTE *p_route)         
{
    u_char  hoplen = 0;
    u_char *p_nexthop = NULL;
    tBGP4_PATH *p_path = NULL;
    u_int nhop = 0 ;
    u_char ip6_nhop[32];
    u_char flag = 0;

    if(p_msg == NULL || p_route == NULL)
    {
        return 0;
    }

    p_nexthop = (p_msg + 1);/*empty 1 byte for nexthop len*/
    p_path = p_route->p_path ;

    if(p_path == NULL)
    {
        return 0;
    }
    
    memset(ip6_nhop, 0,32);

    
    /*fill for different nexthop type*/
    switch (p_route->dest.afi)
    {
        case BGP4_PF_IPUCAST :
        case BGP4_PF_IPMCAST:
        case BGP4_PF_IPLABEL:
        {
            if (*(u_int*)p_path->nexthop.ip )                 
            {
                hoplen = 4 ;
                nhop = *(u_int*)p_path->nexthop.ip  ;
            }
            else 
            {
                return 0;
            }
        
            break ;
        }
        case BGP4_PF_IPVPN:    
        {
            if ((*(u_int*)p_path->nexthop.ip ))
            {
                /*nexthop's RD is 0(8 bytes)*/
                memset(p_nexthop, 0, 8);                                          
                p_nexthop += 8 ;
                   
                hoplen = 12 ;
                nhop = bgp_ip4(p_path->nexthop.ip) ;
            }
            else 
            {
                return 0;
            }
                
            break ;
        }
        case BGP4_PF_IP6VPN:/*mpls l3 vpn only use ipv4 backbone network,so ipv4 nexthop is valid*/
        /*|                80 位                 | 16 |      32 位          |
        +--------------------------------------+--------------------------+
        |0000..............................0000|FFFF|    IPv4 地址     |
        +--------------------------------------+----+---------------------+*/
        {
            
            if (*(u_int*)p_path->nexthop.ip)
            {
                /*8 bytes 0 for RD*/
                memset(ip6_nhop, 0, 8);
                memset(ip6_nhop+8, 0, 10);
                memset(ip6_nhop+18, 0xff, 2);
            
                memcpy(ip6_nhop+20,p_path->nexthop.ip,4);
                hoplen = 24 ;  
            }
            else
            {
                return 0;
            }           
            
            break ;
        }
        case BGP4_PF_IP6LABEL:
        /*|                80 位                 | 16 |      32 位          |
        +--------------------------------------+--------------------------+
        |0000..............................0000|FFFF|    IPv4 地址     |
        +--------------------------------------+----+---------------------+*/
        {
            if (*(u_int*)p_path->nexthop.ip )
            {
                memset(ip6_nhop, 0, 10);
                memset(ip6_nhop+10, 0xff, 2);
                memcpy(ip6_nhop+12,p_path->nexthop.ip,4);
                hoplen = 16 ;  
            }
            else
            {
                return 0;
            }   
            break ;
        }       
        case BGP4_PF_IP6UCAST:
        case BGP4_PF_IP6MCAST:
        {
            if(memcmp(ip6_nhop, p_path->nexthop_global.ip,16)!=0)
            {
                hoplen += 16;
                memcpy(ip6_nhop, p_path->nexthop_global.ip,16);
                flag = 1;
            }
            if(memcmp(ip6_nhop+16, p_path->nexthop_local.ip,16)!=0)
            {
                hoplen += 16;
                if(flag)
                {
                    memcpy(ip6_nhop+16, p_path->nexthop_local.ip,16);
                }
                else
                {
                    memcpy(ip6_nhop, p_path->nexthop_local.ip,16);
                }
            }

            if(hoplen != 16 && hoplen != 32)
            {
                return 0;
            }
             
            break;
        }
        default :    
            return 0 ;    
    }
    
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
       
    return (hoplen+1);/*including 1byte of length*/
}

void bgp4_send_sync_mp_update_to_peer(tBGP4_PEER *p_src_peer,
                                  u_int af,
                                  tBGP4_LIST *p_flist,
                                  tBGP4_LIST *p_wlist)
{
    u_char mp[1600];/*contaning mp attribute,do not include flag&type&len,af/safi*/
    u_char nexthop[64];/*mpnexthop,64 is enough*/
    tBGP4_LINK *p_link = NULL;
    tBGP4_LINK *p_next = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_PATH *p_path = NULL ;
    tBGP4_UPDATE_INFO update ;
    u_short len = 0 ;
    u_short filllen;
    u_short nlrilen;
    u_int fixlen = 7;/*4bytes option head + 3 bytes afi/safi*/
    u_short path_len = 0 ;
    u_short nexthop_len = 0 ;
    
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
            bgp4_finish_sync_update(&update,p_src_peer);
            /*reset total nlrilen*/
            len = 0 ;
            update.p_msg += 2;
        }
        /*fill mp-nlri*/
        len += bgp4_fill_mp_nlri(af,mp + len, p_route);
        update.with_count++;
    }

    /*only withdraw route sending*/
    if (bgp4_lstfirst(p_flist) == NULL)
    {
            bgp4_finish_sync_update(&update,p_src_peer);
            return ;
    }
            
    /*feasible route*/   
    while (bgp4_lstfirst(p_flist))
    {
        /*prepare send update for the first path*/
        p_link = (tBGP4_LINK *)bgp4_lstfirst(p_flist) ;        
        p_route = p_link->p_route ;
        p_path = p_route->p_path ;

        /*build attribute for this path*/
        path_len = bgp4_build_sync_attr(p_route, mp); 

        nexthop_len = bgp4_fill_sync_mp_nexthop(nexthop, p_route);
        
        /*decide length,if exceed,send first,consider 1 byte rsvd field*/
        if ((update.len + fixlen + path_len + nexthop_len + 1) > gBgp4.max_len)
        {
            /*no new attribute need,send directly*/
            bgp4_finish_sync_update(&update,p_src_peer);
                    
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
            if ((p_path != p_route->p_path) 
                && (bgp4_same_path(p_path, p_route->p_path) == FALSE))
            {
                continue ;
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
            bgp4_finish_sync_update(&update,p_src_peer);
            update.p_msg += 2;
        }
    }

    return ;

}

u_int bgp4_build_update_and_sync (tBGP4_PEER *p_src_peer,
                                     u_int af,
                                     tBGP4_LIST *p_flist,
                                     tBGP4_LIST *p_wlist)
{
    u_char attr[2048] ;
    tBGP4_LINK *p_link = NULL;
    tBGP4_LINK *p_next = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_PATH *p_path = NULL ;
    tBGP4_UPDATE_INFO update;
    u_short path_len = 0 ;
    u_short filled_len = 0 ;
    u_int send_pkt=0;

    if ((bgp4_lstfirst(p_flist) == NULL) && (bgp4_lstfirst(p_wlist) == NULL))
    {
        bgp4_log(BGP_DEBUG_CMN,1,"sync no route to build");        
        return 0;
    }

    if (af != BGP4_PF_IPUCAST)
    {
        bgp4_send_sync_mp_update_to_peer(p_src_peer, af, p_flist,p_wlist);
        return 0;
    }

    bgp4_init_update(&update);

    /*insert withdraw routes*/
    LST_LOOP(p_wlist, p_link, node, tBGP4_LINK)
    {
        p_route  = p_link->p_route;   

        /*exceed max length, send update first*/
        if ((update.len + bgp4_bit2byte(p_route->dest.prefixlen) + 1) > gBgp4.max_len)
        {
            send_pkt++;
            bgp4_finish_sync_update(&update,p_src_peer);
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
            send_pkt++;
            bgp4_finish_sync_update(&update,p_src_peer);
            return send_pkt;
    }

        update.p_msg += 2;
        /*send update with feasible routes*/
        /*enter sending loop,input route list may have multipe path attribute,so 
        we need send one update for each path attribute*/
        while (bgp4_lstfirst(p_flist))
        {
        /*prepare send update for the first path*/
        p_link = (tBGP4_LINK *)bgp4_lstfirst(p_flist) ;        
        p_route = p_link->p_route ;
        p_path = p_route->p_path ;

        /*build attribute for this path*/
        path_len = bgp4_build_sync_attr( p_route, attr); 

        /*send msg if space full*/
        if ((update.len + path_len) > gBgp4.max_len)
        {
            send_pkt++;
            bgp4_finish_sync_update(&update,p_src_peer);
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
            if ((p_path != p_route->p_path) 
                && (bgp4_same_path(p_path, p_route->p_path) == FALSE))
            {
                continue ;
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
            bgp4_rtlist_delete(p_flist, p_link);
            
        }
        if (update.fea_count)
        {
            send_pkt++;
            bgp4_finish_sync_update(&update,p_src_peer);
            update.p_msg += 2;
        }
    }
    
    return send_pkt;

}
void bgp4_send_sync_route_all()
{
    u_int af = 0;
    tBGP4_PATH * p_path = NULL;
    tBGP4_ROUTE* p_route = NULL;
    tBGP4_LIST flist;
    tBGP4_LIST wlist;
    tBGP4_PEER *p_src_peer = NULL;
    u_int rt_num = 0;
    u_int rt_send=0;
    u_int sync_pkt=0;

    for(af = 0;af < BGP4_PF_MAX;af++)
    {
        LST_LOOP(&gBgp4.attr_list[af], p_path, node, tBGP4_PATH)
        {
                p_src_peer = p_path->p_peer;
                
            if(p_src_peer == NULL)/*only routes learned from other peers should be sent*/
            {
                continue;
            }

            if(p_path->p_instance != p_src_peer->p_instance)/*unoriginal vpn routes do not sync to others*/
            {
                continue;
            }

            bgp4_lstinit(&flist);
            bgp4_lstinit(&wlist);
            
            LST_LOOP(&p_path->route_list, p_route, node, tBGP4_ROUTE)
            {
                rt_num++;
                if(rt_num%5000==0)
                    vos_pthread_delay(1);

                if(p_route->dest.afi != af )
                {
                    break;
                }
                bgp4_rtlist_add(&flist, p_route);
                rt_send++;
            }
                         
            sync_pkt+=bgp4_build_update_and_sync (p_src_peer,af, &flist,&wlist);
                    
            bgp4_rtlist_clear (&flist);
            bgp4_rtlist_clear (&wlist);

          }
    }
    
    
}
void bgp4_rebuild_sync_pdu(tBGP4_LIST* p_flist,tBGP4_LIST* p_wlist,tBGP4_PEER* p_src_peer)
{
    u_int af = 0;
    tBGP4_LINK *p_link = NULL;
    tBGP4_ROUTE  *p_route = NULL;

    p_link = (tBGP4_LINK *)bgp4_lstfirst(p_flist);
    if(p_link == NULL)
    {
        p_link = (tBGP4_LINK *)bgp4_lstfirst(p_wlist);
    }

    if(p_link)
    {
        p_route = p_link->p_route;
        if(p_route)
        {
            af = p_route->dest.afi;/*routes's afi in a update msg must be the same*/
        }
        
        bgp4_build_update_and_sync (p_src_peer,af, p_flist,p_wlist);
    }

    return;
    
    
}

void bgp4_extract_sync_peer(tBGP4_SYNC_PEER* p_sync_peer,u_int action)
{
    tBGP4_PEER* p_peer = NULL;
    u_char addr[64] = {0};
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    p_instance = bgp4_vpn_instance_lookup(p_sync_peer->vrf_id);
    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1 ,"bgp4 extract sync peer,peer instance %d not exist,do nothing!",
                    p_sync_peer->vrf_id);
        return;
            
    }
    
    /*********find the peer if exist*********/
    p_peer = bgp4_peer_lookup(p_instance,&p_sync_peer->remote_ip);
    if(p_peer == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,1 ,"bgp4 extract sync peer,peer %s not exist,do nothing!",
                    bgp4_printf_addr(&p_sync_peer->remote_ip,addr));

        return;/*peer conf should sync before*/
            
    }
        
    
    if(action)/*modify*/
    {
        bgp4_log(BGP_DEBUG_SYNC,1 ,"bgp4 extract sync peer,receive sync peer %s,action MODIFY!",
            bgp4_printf_addr(&p_sync_peer->remote_ip,addr),action);

        /*********get peer info*********/
        p_peer->router_id = ntohl(p_sync_peer->router_id);
        p_peer->remote.af = ntohl(p_sync_peer->af); 
        p_peer->remote.reset_af = ntohl(p_sync_peer->reset_af); 
        p_peer->remote.capability = ntohl(p_sync_peer->capability); 
        p_peer->neg_hold_interval = ntohl(p_sync_peer->neg_hold_interval); 
        p_peer->neg_keep_interval= ntohl(p_sync_peer->neg_keep_interval); 
        p_peer->rib_end_af = ntohl(p_sync_peer->rib_end_af); 
        p_peer->send_capability = ntohl(p_sync_peer->send_capability); 
        /*statstic*/
        p_peer->established_transitions = ntohl(p_sync_peer->established_transitions); 
        p_peer->uptime = ntohl(p_sync_peer->uptime); 
        p_peer->rx_updatetime = ntohl(p_sync_peer->rx_updatetime); 
        p_peer->rx_update = ntohl(p_sync_peer->rx_update); 
        p_peer->tx_update = ntohl(p_sync_peer->tx_update); 
        p_peer->rx_msg = ntohl(p_sync_peer->rx_msg); 
        p_peer->tx_msg = ntohl(p_sync_peer->tx_msg); 
        p_peer->last_errcode = ntohl(p_sync_peer->last_errcode); 
        p_peer->last_subcode = ntohl(p_sync_peer->last_subcode); 
        p_peer->if_unit = ntohl(p_sync_peer->peer_if_unit);

        p_peer->remote.reset_time= ntohs(p_sync_peer->reset_time); 
        p_peer->remote.refresh = p_sync_peer->refresh;
        p_peer->remote.reset_bit = p_sync_peer->reset_bit;
        p_peer->remote.reset_enable = p_sync_peer->reset_enable;
        p_peer->state = p_sync_peer->state;
        p_peer->version = p_sync_peer->version;
        p_peer->event = p_sync_peer->event;

    }
    else/*delete*/
    {
        bgp4_log(BGP_DEBUG_SYNC,1 ,"receive sync peer %s,action DELETE!",
            bgp4_printf_addr(&p_sync_peer->remote_ip,addr));
        
        bgp4_delete_peer(p_peer);
        
    }
    return;
}
void bgp4_extract_sync_fsm(tBGP4_SYNC_FSM* p_sync_fsm)
{
    tBGP4_PEER* p_peer = NULL;
    u_char addr[64] = {0};
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    p_instance = bgp4_vpn_instance_lookup(p_sync_fsm->vrf_id);
    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1 ,"bgp4 extract sync fsm,peer instance %d not exist,do nothing!",
                    p_sync_fsm->vrf_id);
        return;
            
    }
    /*********find the peer if exist*********/
    p_peer = bgp4_peer_lookup(p_instance,&p_sync_fsm->remote_ip);
    if(p_peer == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,1 ,"sync peer %s not exist,do nothing!",
                    bgp4_printf_addr(&p_sync_fsm->remote_ip,addr));

        return;/*peer conf should sync before*/
            
    }
        
    p_peer->state = p_sync_fsm->state;
    p_peer->event = p_sync_fsm->event;
    p_peer->if_unit = ntohl(p_sync_fsm->peer_if_unit);
    bgp4_log(BGP_DEBUG_SYNC,1 ,"receive sync peer %s fsm,event %d,peer interfce unit %d!",
            bgp4_printf_addr(&p_sync_fsm->remote_ip,addr),p_peer->event,p_peer->if_unit);


    /********sync peer fsm*******/
    bgp4_fsm(p_peer,p_peer->event);
    
    return;
}


void bgp4_extract_sync_pdu(tBGP4_SYNC_PDU* p_sync_pdu)
{

    tBGP4_PEER* p_peer = NULL;
    u_char addr[64] = {0};
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    p_instance = bgp4_vpn_instance_lookup(p_sync_pdu->vrf_id);
    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1 ,"bgp4 extract sync pdu,peer instance %d not exist,do nothing!",
                    p_sync_pdu->vrf_id);
        return;
            
    }
    /*********find the peer*********/
    p_peer = bgp4_peer_lookup(p_instance,&p_sync_pdu->remote_ip);
    if(p_peer == NULL)
    {
        bgp4_log(BGP_DEBUG_SYNC,1 ,"peer %s not exist,do nothing!",
                bgp4_printf_addr(&p_sync_pdu->remote_ip,addr));
        return;                     
    }

    bgp4_log(BGP_DEBUG_SYNC,1 ,"receive sync peer %s msg pdu",
                bgp4_printf_addr(&p_sync_pdu->remote_ip,addr));

    bgp4_msg_input(p_peer,p_sync_pdu->msg);
    
}

/*check sync event and send sync msg*/
void bgp4_send_sync_all()
{
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    
    if (gBgp4.work_mode != BGP4_MODE_MASTER)
    {
        bgp4_log(BGP_DEBUG_SYNC,1 ,"sync role %d (1:master,2:slave,3:other) error,do not send init sync msg!",
                gBgp4.work_mode);
        return;
    }
    
    /********send peer table*********/
    LST_LOOP (&gBgp4.vpn_instance_list,p_instance, node, tBGP4_VPN_INSTANCE) 
    {
        LST_LOOP (&p_instance->peer_list,p_peer, node, tBGP4_PEER) 
        {
            bgp4_send_sync_peer(p_peer,1);
        }
    }
    
    vos_pthread_delay(60);
    /********send route update*********/
    bgp4_send_sync_route_all();

}

/*receive sync data,by slave*/
void bgp4_recv_sync_all(octetstring *p_octet)
{
    u_char *p_msg = NULL;
    u_int msg_len = 0;
    u_int read_len = 0;
    tBGP4_SYNC_MSG_HDR* p_msg_hdr = NULL;

    if (bgp4_get_workmode() != BGP4_MODE_SLAVE)
    {
        bgp4_log(BGP_DEBUG_SYNC,1 ,"sync role %d error,do not process sync msg!",
                gBgp4.work_mode);
        return;
    }

    bgp4_log(BGP_DEBUG_SYNC,1 ,"Receive sync msgs,msg total length %d",p_octet->len);
    

    for (p_msg = p_octet->pBuf; read_len < p_octet->len ; read_len += msg_len)
    {
        p_msg_hdr = (tBGP4_SYNC_MSG_HDR *)(p_msg+read_len);
        msg_len = ntohs(p_msg_hdr->len)+BGP4_SYNC_HDRLEN;

        if(msg_len <= BGP4_SYNC_HDRLEN)
        {
            bgp4_log(BGP_DEBUG_SYNC,1 ,"extract sync msg length ERROR,msg length %d",msg_len);
            break;
        }

        bgp4_log(BGP_DEBUG_SYNC,1 ,"extract a sync msg,msg length %d,sync data length %d",
            msg_len,msg_len - BGP4_SYNC_HDRLEN);


        switch (p_msg_hdr->type) 
        {
            case BGP4_SYNC_PEER:
            {
                bgp4_extract_sync_peer((tBGP4_SYNC_PEER*)(p_msg_hdr + 1),p_msg_hdr->sync_action);
                break;
            }

            case BGP4_SYNC_PEER_FSM:
            {
                bgp4_extract_sync_fsm((tBGP4_SYNC_FSM*)(p_msg_hdr + 1));
                break;
            }
                              
            case BGP4_SYNC_ROUTE_UPDATE:
            {
                gBgp4.stat.sync_msg_recv++;
                bgp4_extract_sync_pdu((tBGP4_SYNC_PDU*)(p_msg_hdr + 1));
                break;
            }                 
            default :
            {
                bgp4_log(BGP_DEBUG_SYNC,1 ,"invalid sync msg type %d,ignore it",p_msg_hdr->type);           
                break;
            }
        }
    }

    return;
     
    
}

void bgp4_subsys_up_sync(void * buf)
{
    struct card_msghdr *pmsg=(struct card_msghdr  *)buf;
        u_int syncflag = (pmsg->sysId <<USP_SYNCSYS_SHIFT) |
                        (USP_SYNC_DOWNSTREAM <<USP_SYNCFLAG_SHIFT)|
                        (USP_SYNC_HORIZONTAL <<USP_SYNCFLAG_SHIFT);

    bgp4_get_workmode();
 
        if(uspSelfSysIndex() == pmsg->sysId)
                return ;
            
        if((uspHwScalarGet(NULL,HW_SYS_SWRUNNINGROLEISMASTER,NULL) == VOS_OK)||
        (uspHwScalarGet(NULL,HW_SYS_HAROLEISMASTER ,NULL) == VOS_OK))
        {
            bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock RTM_SUBSYSTEM_UP");
        bgp4_up_init_sync(syncflag);
        }

    return;
}

void bgp4_card_up_sync(void * buf)
{   
    struct card_msghdr *pmsg=(struct card_msghdr  *)buf;
        tCardIndex cardIdx; 
        u_int syncflag = (pmsg->sysId <<USP_SYNCSYS_SHIFT) | 
                    (pmsg->slotId <<USP_SYNCSLOT_SHIFT) | 
                    (pmsg->cardId <<USP_SYNCCARD_SHIFT) | 
                    (USP_SYNC_DOWNSTREAM <<USP_SYNCFLAG_SHIFT); 

        if(uspSelfSysIndex() != pmsg->sysId)/*本系统只负责对本地线卡同步*/
                return ;
      
    cardIdx.sys= pmsg->sysId;
    cardIdx.slot= pmsg->slotId;
    cardIdx.card = pmsg->cardId;
    cardIdx.type = pmsg->type;
       
    if(uspHwCardGet(&cardIdx,HW_CARD_ISMPU,NULL) == VOS_OK)/*新上线的卡为主控卡*/
    {
            if((uspHwScalarGet(NULL,HW_SYS_SWRUNNINGROLEISMASTER,NULL) == VOS_OK)||
            (uspHwScalarGet(NULL,HW_SYS_HAROLEISMASTER ,NULL) == VOS_OK))
            {
                bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock RTM_CARD_UP");
            bgp4_up_init_sync(syncflag);
            }
    }  
    
    return;
}

void bgp4_slot_up_sync(void * buf)
{
    struct card_msghdr *pmsg=(struct card_msghdr  *)buf;
        tSlotIndex slotIdx; 
        u_int syncflag = (pmsg->sysId <<USP_SYNCSYS_SHIFT) | 
                        (pmsg->slotId <<USP_SYNCSLOT_SHIFT) |
                        (USP_SYNC_DOWNSTREAM <<USP_SYNCFLAG_SHIFT)|
                        (USP_SYNC_HORIZONTAL <<USP_SYNCFLAG_SHIFT);
 
        if(uspSelfSysIndex() != pmsg->sysId)/*本系统只负责对本地线卡同步*/
                return ;
      
    slotIdx.sys= pmsg->sysId;
    slotIdx.slot= pmsg->slotId;
    slotIdx.type = pmsg->type;
       
    if(uspHwSlotGet(&slotIdx,HW_SLOT_ISMPU,NULL) == VOS_OK)/*上线线卡是主控*/
    {
            if((uspHwScalarGet(NULL,HW_SYS_SWRUNNINGROLEISMASTER,NULL) == VOS_OK)||
            (uspHwScalarGet(NULL,HW_SYS_HAROLEISMASTER ,NULL) == VOS_OK))
            {
                bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock RTM_SLOT_UP");
            bgp4_up_init_sync(syncflag);
            }
    }     
    return;
}

void bgp4_up_init_sync(u_int syncflag)
{   
    if(gBgp4.slave_up == 1)
    {   
        bgp4_send_sync_all();
    }

    gBgp4.have_slave = 1;
    gBgp4.sync_flag = syncflag;     
    gBgp4.slave_up=1;
    gBgp4.sync_tick=vos_get_system_tick();
        
    return;
}


void bgp4_card_role_change(void)
{
    u_int old_mode = gBgp4.work_mode;
    
    bgp4_get_workmode();

    if(old_mode != BGP4_MODE_MASTER &&
        gBgp4.work_mode == BGP4_MODE_MASTER)/*change to master*/
    {
        /*if enable GR,perform GR operation automatically*/
        if(gBgp4.gr_enable)
        {
            bgp4_reset_all_peers(0);
        }
    }
                
}



#endif

