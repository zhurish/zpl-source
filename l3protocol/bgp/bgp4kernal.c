#include "plateform.h"
#include "bgp4_api.h"
#include "zebra_api.h"
#ifdef NEW_BGP_WANTED
#include "bgp4com.h"

void 
bgp4_rtsock_init(void)
{
#if !defined(WIN32)
    int on;

    if (gbgp4.rtsock > 0)
    {
        return;
    }

    gbgp4.rtsock = socket(AF_ROUTE, SOCK_RAW, 0);
    if (gbgp4.rtsock == VOS_ERR)
    {
        gbgp4.rtsock = 0;
        return;
    }

    on = 1;
    bgp4_set_sock_noblock(gbgp4.rtsock, on);
    bgp4_set_sock_tcpnodelay(gbgp4.rtsock, on);

    on = 0;
    setsockopt(gbgp4.rtsock, SOL_SOCKET, SO_USELOOPBACK, (char *)&on, sizeof(on));

    on = BGP4_RTSOCK_BUFFER_LEN;
    bgp4_set_sock_rxbuf(gbgp4.rtsock, on);
    bgp4_set_sock_txbuf(gbgp4.rtsock, on);
#endif
    return;
}

void
bgp4_rtsock_close(void)
{
#if !defined(WIN32)
    close(gbgp4.rtsock);
    gbgp4.rtsock = 0;
#endif
    return;
}

#define ROUNDUP(a) \
        ((a) > 0 ? (1 + (((a) - 1) | (sizeof (long) - 1))) : sizeof (long))

#define ADVANCE(x, n) (x += ROUNDUP ((n)->sa_len))

void 
bgp4_rtsock_newroute_msg_input(u_char *p_buf)
{
    struct rt_newmsghdr *p_rtmsg = (struct rt_newmsghdr *)p_buf;
    struct rt4_newmsg *p_newroute = NULL;
    tBGP4_ROUTE route;
    tBGP4_PATH path;
    u_int i = 0;
    u_int vrf_id = p_rtmsg->vrfid;

    /*ignore bgp route self*/
    if (p_rtmsg->proto == M2_ipRouteProto_bgp)
    {
        return;
    }

    memset(&route, 0, sizeof(tBGP4_ROUTE));
    memset(&path, 0, sizeof(tBGP4_PATH));
    bgp4_path_init(&path);

    route.proto = p_rtmsg->proto;
    route.dest.afi = BGP4_PF_IPUCAST;
    bgp4_link_path(&path, &route);

    p_newroute = (struct rt4_newmsg*)(p_buf + sizeof(struct rt_newmsghdr));
    for (i = 0; i < p_rtmsg->count; i++, p_newroute++)
    {
        /*translate route msg into internal struct*/
        memcpy(route.dest.ip, &p_newroute->dest, 4);
        
        route.dest.prefixlen = bgp4_mask2len(p_newroute->mask);
        
        path.nexthop.afi = BGP4_PF_IPUCAST;
        
        memcpy(path.nexthop.ip, &p_newroute->nexthop, 4);

        /*local route has no nexthop*/
        if (route.proto == M2_ipRouteProto_local)
        {
            memset(path.nexthop.ip, 0, 4);
        }
        
        path.nexthop.prefixlen = 32;

        path.origin = BGP4_ORIGIN_IGP;
        
        path.med = p_newroute->cost;

        path.af = BGP4_PF_IPUCAST;

        bgp4_rtsock_route_change(vrf_id, &route, (p_newroute->action == RTM_ADD) ? TRUE : FALSE);
    }
    return;
}

void
bgp4_rtsock_newroute6_msg_input(u_char *p_buf)
{
    struct rt_newmsghdr *p_rtmsg = (struct rt_newmsghdr *)p_buf;
    struct rt6_newmsg *p_newroute = NULL;
    tBGP4_ROUTE route;
    tBGP4_PATH path;
    u_int i = 0;
    u_int vrf_id = p_rtmsg->vrfid;

    /*ignore bgp route self*/
    if (p_rtmsg->proto == M2_ipRouteProto_bgp)
    {
        return;
    }

    memset(&route, 0, sizeof(tBGP4_ROUTE));
    memset(&path, 0, sizeof(tBGP4_PATH));
    bgp4_path_init(&path);
    
    route.proto = p_rtmsg->proto;
    route.dest.afi = BGP4_PF_IP6UCAST;
    bgp4_link_path(&path, &route);
    
    p_newroute = (struct rt6_newmsg*)(p_buf + sizeof(struct rt_newmsghdr));
    
    for (i = 0; i < p_rtmsg->count; i++, p_newroute++)
    {
        memcpy(route.dest.ip, p_newroute->dest, 16);
        route.dest.prefixlen = p_newroute->prefixlen;
        path.nexthop.afi = BGP4_PF_IP6UCAST;
        memcpy(path.nexthop.ip, &p_newroute->nexthop, 16);
        path.nexthop.prefixlen = 128;
        path.origin = BGP4_ORIGIN_IGP;
        path.med = p_newroute->cost;

        path.af = BGP4_PF_IP6UCAST;

        if (route.proto == M2_ipRouteProto_local)
        {
            memset(path.nexthop.ip, 0, 16);
        }
        bgp4_rtsock_route_change(vrf_id, &route, (p_newroute->action == RTM_ADD) ? TRUE : FALSE);
    }
    return;
}

/*address change msg rxd*/
void
bgp4_rtsock_addr_msg_input(u_char *p_buf)
{
    struct sockaddr *sa[RTAX_MAX] = {NULL};
    struct sockaddr *s = NULL;
    struct sockaddr_in6 *sa6;
    struct ifa_msghdr *p_ifamsg = (struct ifa_msghdr *)p_buf;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_ADDR addr;
    caddr_t cp;
    u_int ifunit = p_ifamsg->ifam_ifunit;
    u_int i = 0;    
    u_char str[64] = {0};

    memset(&addr, 0, sizeof(tBGP4_ADDR));

    /*extract route from message*/
    cp = (caddr_t)(p_ifamsg + 1);

    for (i = 0;  (i < RTAX_MAX); i++)
    {
       if ((p_ifamsg->ifam_addrs & (1 << i)) == 0)
       {
           continue;
       }

       sa[i] = s = (struct sockaddr *)cp;
#ifndef WIN32
       /*TODO:如下代码sa_len系统中不存在，可能会影响下面代码的功能，待确认*/
       #if 0
       if (s->sa_len == 0)
       {
           sa[i] = NULL;
       }
       
       ADVANCE (cp, s);
       #endif
#endif
    }

    if (sa[RTAX_IFA] == NULL)
    {
        return;
    }
    
    if (sa[RTAX_IFA]->sa_family == AF_INET)
    {
        *(u_int*)(addr.ip) = (((struct sockaddr_in *)sa[RTAX_IFA])->sin_addr.s_addr);
        addr.afi = BGP4_PF_IPUCAST;
        addr.prefixlen = 32;           
    }
    else if (sa[RTAX_IFA]->sa_family == AF_INET6)
    {
        sa6 = (struct sockaddr_in6 *)sa[RTAX_IFA];
        memcpy(addr.ip, &sa6->sin6_addr, 16);
        addr.afi = BGP4_PF_IP6UCAST;
        addr.prefixlen = 128;
    }
    
    bgp4_log(BGP_DEBUG_EVT, "receive msg,address %s ",
                bgp4_printf_addr(&addr, str));
    
    /*when ip address is deleted,kernal will del all routes from the interface,*/
    bgp4_instance_for_each(p_instance)
    {
        bgp4_peer_for_each(p_instance, p_peer)
        {
            if ((bgp4_prefixcmp(&addr, &p_peer->local_ip) == 0)
                && (p_peer->if_unit == ifunit))
            {
                /*if both of peers support GR,then go to GR process,otherwise,go to normal restart process*/
                if (bgp4_local_grace_restart(p_peer) != VOS_OK)
                {
                    /*if still in GR process,should quit*/
                    if (gbgp4.restart_enable)
                    {
                        bgp4_peer_restart_finish(p_peer);
                    }

                    /*common peer reset*/
                    p_peer->notify.code = BGP4_CEASE;
                    p_peer->notify.sub_code = BGP4_ADMINISTRATIVE_RESET;
                    bgp4_fsm_invalid(p_peer);
                }
            }
        }
    }
    return;
}

void
bgp4_rtsock_route_msg_input(u_char *p_buf)
{
    struct sockaddr *sa[RTAX_MAX] = {NULL};
    struct sockaddr *s;    
    struct sockaddr_in6 *sa6;
    struct rt_msghdr *p_msg = (struct rt_msghdr *)p_buf;
    tBGP4_ROUTE route;
    tBGP4_PATH path;
    caddr_t cp ;
    u_int cmd = p_msg->rtm_type;
    u_int vrf_id = p_msg->rtm_vpnvrfid;
    u_int i = 0;
    
    bgp4_log(BGP_DEBUG_EVT, "receive route  msg %d ", cmd);
    
    if (p_msg->rtm_version != RTM_VERSION)
    {
        return;
    }

    if (p_msg->rtm_errno != 0)
    {
        return;
    }

    /*extract route from message*/
    cp = (caddr_t)(p_msg + 1);

    for (i = 0;  (i < RTAX_MAX); i++)
    {
        if ((p_msg->rtm_addrs & (1 << i)) == 0)
        {
            continue;
        }

        sa[i] = s = (struct sockaddr *)cp;
       #ifndef WIN32 
        #if 0 /*TODO:如下代码sa_len系统中不存在，可能会影响下面代码的功能，待确认*/
        if (s->sa_len == 0)
        {
            sa[i] = NULL;
        }
        ADVANCE(cp, s);
        #endif
       #endif        
    }
    
    /* if no destination address, reject the message */
    if (sa[RTAX_DST] == NULL)
    {
        return;
    }

    memset(&route, 0, sizeof(route));
    memset(&path, 0, sizeof(path));
    bgp4_path_init(&path);
    bgp4_link_path(&path, &route);

    /*ignore bgp route*/
    //route.proto = RT_PROTO_GET(sa[RTAX_DST]); /*TODO:待确认*/
    if (route.proto == M2_ipRouteProto_bgp)
    {
        return;
    }
    
    /* if destination address doesn't belong to the Internet family, reject
    * the message too
    */
    if (sa[RTAX_DST]->sa_family == AF_INET)
    {
        route.dest.afi = BGP4_PF_IPUCAST;
        *(u_int*)(route.dest.ip) = (((struct sockaddr_in *)sa[RTAX_DST])->sin_addr.s_addr);

        if (sa[RTAX_NETMASK])
        {
            route.dest.prefixlen = bgp4_mask2len(ntohl( ((struct sockaddr_in *)sa[RTAX_NETMASK])->sin_addr.s_addr));
        }
        else/*for host route esp.*/
        {
            route.dest.prefixlen = 32;
        }

        if (sa[RTAX_GATEWAY])
        {
            path.nexthop.afi = BGP4_PF_IPUCAST;
            path.nexthop.prefixlen = 32;
            *(u_int*)(path.nexthop.ip) = (((struct sockaddr_in *)sa[RTAX_GATEWAY])->sin_addr.s_addr);
        }

        path.origin = BGP4_ORIGIN_IGP;
        path.af = BGP4_PF_IPUCAST;

        path.med = p_msg->rtm_rmx.value1;

        /*local route has no nexthop*/
        if (route.proto == M2_ipRouteProto_local)
        {
            *(u_int*)path.nexthop.ip = 0;
        }
    }
    else if (sa[RTAX_DST]->sa_family == AF_INET6)
    {
        sa6 = (struct sockaddr_in6 *)sa[RTAX_DST];
        route.p_path = &path;
        route.dest.afi = BGP4_PF_IP6UCAST;
        path.af = BGP4_PF_IP6UCAST;

        memcpy(route.dest.ip, &sa6->sin6_addr, 16);

        if (sa[RTAX_NETMASK])
        {
            sa6 = (struct sockaddr_in6 *)sa[RTAX_NETMASK];
            route.dest.prefixlen = bgp4_ip6_mask2len(&sa6->sin6_addr);
        }
        else 
        {
            route.dest.prefixlen= 128;
        }

        if (sa[RTAX_GATEWAY])
        {
            path.nexthop.afi = BGP4_PF_IP6UCAST;
            path.nexthop.prefixlen = 128;
            sa6 = (struct sockaddr_in6 *)sa[RTAX_GATEWAY];
            memcpy(path.nexthop.ip, &sa6->sin6_addr, sizeof(sa6->sin6_addr));
        }
        path.origin = BGP4_ORIGIN_IGP; 
        path.med = p_msg->rtm_rmx.value1;

        if (route.proto == M2_ipRouteProto_local)
        {
            memset(path.nexthop.ip, 0, 16);
        }
    }
    bgp4_rtsock_route_change(vrf_id, &route, (cmd == RTM_ADD) ? TRUE : FALSE);
    return;
}

void
bgp4_rtsock_vpn_add_msg_input(u_char *p_buf)
{
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    struct vpn_msghdr *p_msg = (struct vpn_msghdr*)p_buf;
    u_int i = 0;
    u_int vrf;

    bgp4_log(BGP_DEBUG_EVT, "vpn create event detected");

    if (p_msg->vpn_state != 0)
    {
        bgp4_log(BGP_DEBUG_EVT, "this vpn state invalid");
        return;
    }

    for (i = 0; i < p_msg->cnt; i++)
    {
        gbgp4.stat.vrf_add++;

        vrf = p_msg->l3vpnMsg[i].vpn_vrid;
        
        p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, vrf);
        if (p_instance == NULL)
        {
            p_instance = bgp4_vpn_instance_create(BGP4_INSTANCE_IP, vrf);
            if (p_instance)
            {
                bgp4_log(BGP_DEBUG_EVT, "vpn instance %d create sucess", vrf);

                /*copy rd*/
                memcpy(p_instance->rd, p_msg->l3vpnMsg[i].rd, sizeof(p_instance->rd));
                
                /*newly created instance only need refresh remote vpn routes from PEs*/
                bgp4_rtsock_import_rtarget_add(p_instance);
            }
        }
    }
    return;
}

void
bgp4_rtsock_vpn_del_msg_input(u_char *p_buf)
{
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    struct vpn_msghdr *p_msg = (struct vpn_msghdr*)p_buf;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE delete_route;
    u_int i = 0;
    u_int vrf;
    u_int af = 0;
    
    bgp4_log(BGP_DEBUG_EVT, "vpn delete event detected");

    if (p_msg->vpn_state != 1)
    {
        bgp4_log(BGP_DEBUG_EVT, "this vpn state invalid");
        return;
    }

    for (i = 0; i < p_msg->cnt; i++)
    {    
        vrf = p_msg->l3vpnMsg[i].vpn_vrid;
        /*ignore public instance*/
        if (vrf == 0)
        {
            bgp4_log(BGP_DEBUG_EVT, "can not delete public instance");
            continue;
        }
        
        p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, vrf);
        if (p_instance == NULL)
        {
            continue;
        }
        
        bgp4_log(BGP_DEBUG_EVT, "vpn instance %d deleted", vrf);

        gbgp4.stat.vrf_delete++;

        /*close all peers*/
        bgp4_avl_walkup(&p_instance->peer_table, bgp4_fsm_invalid);
        
        /*purge all routes in this instance*/
        for (af = 0; af < BGP4_PF_MAX ; af++)
        {
            if (p_instance->rib[af] == NULL)
            {
                continue;
            }
            bgp4_avl_for_each(&p_instance->rib[af]->rib_table, p_route)
            {
                if (p_route->is_deleted == TRUE)
                {
                    continue;
                }

                /*build route to be deleted*/
                memset(&delete_route, 0, sizeof(delete_route));
                memcpy(&delete_route.dest, &p_route->dest, sizeof(tBGP4_ADDR));
                delete_route.proto = p_route->proto;
                delete_route.is_deleted = TRUE;
                delete_route.p_path = p_route->p_path;
                
                /*notify delete to public instance*/
                if (p_route->p_path->origin_vrf == p_instance->vrf)
                {
                    bgp4_vrf_route_export_check(&delete_route, FALSE);
                }
                            
                bgp4_rib_in_table_update(&delete_route);
            }
        }
        bgp4_vpn_instance_delete_timeout(p_instance);
    }    
    return;
}

void
bgp4_rtsock_vpn_rd_msg_input(u_char *p_buf)
{
    struct rt_newmsghdr *p_msg = (struct rt_newmsghdr*)p_buf;
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    u_char type = p_msg->reserved;
    
    bgp4_log(BGP_DEBUG_EVT, "vpn instance %d attr changed", p_msg->vrfid);

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_msg->vrfid);
    if (p_instance == NULL)
    {
        return;
    }
    switch (type) {
        /*such case must refresh PE routes,for that remote VPN routes may not in local RIB*/
        case BGP4_VPN_IMPORT_RT_ADD:
             bgp4_log(BGP_DEBUG_MPLS_VPN, "vpn %d import-extcommunity target add", p_instance->vrf);
 
             bgp4_rtsock_import_rtarget_add(p_instance);
             break;

        /*such case need not refresh PE routes,for remote VPN routes are all in local RIB*/
        case BGP4_VPN_IMPORT_RT_DEL:
             bgp4_log(BGP_DEBUG_MPLS_VPN, "vpn %d import-extcommunity target delete", p_instance->vrf);
             
             bgp4_rtsock_import_rtarget_delete(p_instance);
             break;

        /*in such case,EX_RT in local VPN routes changed*/
        case BGP4_VPN_EXPORT_RT_ADD:
             bgp4_log(BGP_DEBUG_MPLS_VPN, "vpn %d export-extcommunity target add", p_instance->vrf);
             
             bgp4_rtsock_export_rtarget_add(p_instance);
             break;

        case BGP4_VPN_EXPORT_RT_DEL:
             bgp4_log(BGP_DEBUG_MPLS_VPN, "vpn %d export-extcommunity target delete", p_instance->vrf);
             bgp4_rtsock_export_rtarget_delete(p_instance);
             break;
   
        default:
             bgp4_log(BGP_DEBUG_EVT, "vpn %d unkown change %d", p_instance->vrf, type);
             break;
    }
    return;
}

void
bgp4_rtsock_tcp_add_msg_input(u_char *p_buf)
{
    struct tcpsync_msghdr* p_msg = (struct tcpsync_msghdr*)p_buf;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_ADDR ip;
    u_char str6[64] = {0};
    u_char addr[64] = {0};
    u_char *ipv4_flag = "::ffff:";
    tBGP4_PEER *p_peer = NULL;
    int on = 1;
    u_int buflen = BGP4_TCP_BUFFER_LEN;

    /*************find matched peer*******************/
    memset(&ip, 0, sizeof(tBGP4_ADDR));
    if (p_msg->tcp_family == 0)/*0:IPv4*/
    {
        memcpy(ip.ip, p_msg->tcp_faddr, 4);
        ip.afi = BGP4_PF_IPUCAST;
        ip.prefixlen = 32;
    }
    else 
    {
        memset(str6, 0, sizeof(str6));
        inet_ntop(AF_INET6, p_msg->tcp_faddr, str6, sizeof(str6)-1);
                    
        if (strstr(str6, ipv4_flag))
        {
            ip.afi = BGP4_PF_IPUCAST;
            bgp_ip4(ip.ip) = *(u_int *)(p_msg->tcp_faddr + 12);
            ip.prefixlen = 32;
        }
        else
        {
            ip.afi = BGP4_PF_IP6UCAST;
            memcpy(&ip.ip, p_msg->tcp_faddr, 16);
            ip.prefixlen = 128;
        }
    }
    
    bgp4_log(BGP_DEBUG_EVT, "rtsock tcp add,vrf %d,fd %d,l(%d.%d.%d.%d,%d),r(%d.%d.%d.%d,%d)",
                p_msg->tcp_vrid,
                p_msg->tcp_fd,
                p_msg->tcp_laddr[0],
                p_msg->tcp_laddr[1],
                p_msg->tcp_laddr[2],
                p_msg->tcp_laddr[3],
                p_msg->tcp_lport,
                p_msg->tcp_faddr[0],
                p_msg->tcp_faddr[1],
                p_msg->tcp_faddr[2],
                p_msg->tcp_faddr[3],
                p_msg->tcp_fport);
    

    /*accept only when slave mode exist*/
    if (gbgp4.work_mode != BGP4_MODE_SLAVE)
    {
        return;
    }
    
    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_msg->tcp_vrid);
    if (p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT|BGP_DEBUG_ERROR, "can not find vpn instance by vrf %d ", p_msg->tcp_vrid);
        return;
    }

    if ((p_msg->tcp_fport != gbgp4.server_port)
        && (p_msg->tcp_lport != gbgp4.server_port))
    {
        return;
    }
        
    /*************if find,reset fd/addr/port/unit*******************/
    p_peer = bgp4_peer_lookup(p_instance, &ip);
    if (p_peer == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT|BGP_DEBUG_ERROR, "can not find peer");
        return;
    }

    bgp4_log(BGP_DEBUG_EVT,"find matched peer %s,record new sock %d in slave",
    bgp4_printf_addr(&p_peer->ip, addr), p_msg->tcp_fd);

    /*record server or client socket*/
    if (p_msg->tcp_fport == gbgp4.server_port)
    {
        if ((p_peer->sync_client_sock > 0)
            && (p_peer->sync_client_sock != p_msg->tcp_fd))
        {
            bgp4_log(BGP_DEBUG_EVT,"conflict client socket,local %d, rxd %d",
                     p_peer->sync_client_sock, p_msg->tcp_fd);
        }
        p_peer->sync_client_sock = p_msg->tcp_fd;
    }
    else
    {
        if ((p_peer->sync_server_sock > 0)
            && (p_peer->sync_server_sock != p_msg->tcp_fd))
        {
            bgp4_log(BGP_DEBUG_EVT,"conflict server socket,local %d, rxd %d",
                     p_peer->sync_server_sock, p_msg->tcp_fd);
        }
        p_peer->sync_server_sock = p_msg->tcp_fd;
    }    
    
    /*Set Sock Option*/
    bgp4_set_sock_noblock(p_msg->tcp_fd, on);                
    bgp4_set_sock_tcpnodelay(p_msg->tcp_fd, on);
    bgp4_set_sock_txbuf(p_msg->tcp_fd, buflen);
    if (p_peer->md5_support) 
    {
        bgp4_set_sock_md5_support(p_msg->tcp_fd, 1);
        bgp4_set_sock_md5_key(p_msg->tcp_fd, 1, p_peer);
    }
    bgp4_tcp_set_peer_ttl(p_msg->tcp_fd, p_peer);
    return;
}

void
bgp4_rtsock_tcp_del_msg_input(u_char *p_buf)
{
    struct tcpsync_msghdr *p_msg = (struct tcpsync_msghdr*)p_buf;
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_ADDR ip;
    u_char addr[64] = {0};
    u_char *ipv4_flag = "::ffff:";
    u_char str6[64] = {0};

    bgp4_log(BGP_DEBUG_EVT, "rtsock tcp delete,vrf %d,fd %d,l(%d.%d.%d.%d,%d),r(%d.%d.%d.%d,%d)",
                p_msg->tcp_vrid,
                p_msg->tcp_fd,
                p_msg->tcp_laddr[0],
                p_msg->tcp_laddr[1],
                p_msg->tcp_laddr[2],
                p_msg->tcp_laddr[3],
                p_msg->tcp_lport,
                p_msg->tcp_faddr[0],
                p_msg->tcp_faddr[1],
                p_msg->tcp_faddr[2],
                p_msg->tcp_faddr[3],
                p_msg->tcp_fport);

    /*accept only when slave mode exist*/
    if (gbgp4.work_mode != BGP4_MODE_SLAVE)
    {
        return;
    }

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, p_msg->tcp_vrid);
    if (p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT|BGP_DEBUG_ERROR, "can not find vpn instance by vrf id %d ", p_msg->tcp_vrid);
        return;
    }
    if ((p_msg->tcp_fport != gbgp4.server_port)
        && (p_msg->tcp_lport != gbgp4.server_port))
    {
        return;
    }
    
    /*************find matched peer*******************/
    memset(&ip, 0, sizeof(tBGP4_ADDR));
    if (p_msg->tcp_family == 0)/*0:IPv4*/
    {
        memcpy(ip.ip, p_msg->tcp_faddr, 4);
        ip.afi = BGP4_PF_IPUCAST;
        ip.prefixlen = 32;
    }
    else 
    {
        memset(str6, 0, sizeof(str6));
        inet_ntop(AF_INET6, p_msg->tcp_faddr, str6, sizeof(str6)-1);
                    
        if (strstr(str6, ipv4_flag))
        {
            ip.afi = BGP4_PF_IPUCAST;
            bgp_ip4(ip.ip) = *(u_int *)(p_msg->tcp_faddr + 12);
            ip.prefixlen = 32;
        }
        else
        {
            ip.afi = BGP4_PF_IP6UCAST;
            memcpy(&ip.ip, p_msg->tcp_faddr, 16);
            ip.prefixlen = 128;
        }
    }
    p_peer = bgp4_peer_lookup(p_instance, &ip);
    if (p_peer == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT|BGP_DEBUG_ERROR, "can not find peer");
        return;
    }
    bgp4_log(BGP_DEBUG_EVT,"find matched peer %s,sock %d has been deleted",
        bgp4_printf_addr(&p_peer->ip,addr), p_msg->tcp_fd);

    /*remove sync server or client socket*/
    if (p_msg->tcp_fport == gbgp4.server_port)
    {
        if (p_peer->sync_client_sock != p_msg->tcp_fd)
        {
            bgp4_log(BGP_DEBUG_EVT,"invalid client socket,local %d, rxd %d",
                     p_peer->sync_client_sock, p_msg->tcp_fd);
        }
        p_peer->sync_client_sock = 0;
    }
    else
    {
        if (p_peer->sync_server_sock != p_msg->tcp_fd)
        {
            bgp4_log(BGP_DEBUG_EVT,"invalid server socket,local %d, rxd %d",
                     p_peer->sync_server_sock, p_msg->tcp_fd);
        }
        p_peer->sync_server_sock = 0;
    }    
    if (p_peer->sock == p_msg->tcp_fd)
    {
        p_peer->sock = 0;
    }
    return;
}

void
bgp4_rtsock_bfd_msg_input(u_char *p_buf)
{
    tBGP4_PEER * p_peer = NULL;
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    struct bfd_msghdr *p_msg = (struct bfd_msghdr *)p_buf ;

    bgp4_log(BGP_DEBUG_EVT, "receive bfd rt sock, bfd diag%d,state %d ", p_msg->diag, p_msg->state);

    /*only process bfd down event*/
    if (p_msg->state != BFD_DOWN)
    {
        return;
    }

    
    bgp4_instance_for_each(p_instance)
    {
        bgp4_peer_for_each(p_instance,p_peer)
        {
            if (p_peer->bfd_discribe != p_msg->discr)
            {
                continue;
            }
            
            if (p_msg->diag == BFD_ADMINISTRATIVELY_DOWN)
            {
                bgp4_unbind_bfd(p_peer);
            }
            else if (p_msg->diag == BFD_NEIGHBOR_SIGNALED_SESSION_DOWN)
            {
                /*nothing to do*/
            }
            else if (p_peer->state == BGP4_NS_ESTABLISH)
            {
                bgp4_fsm(p_peer, BGP4_EVENT_TCP_CLOSED);
            }
        }
    }
    return;
}

void 
bgp4_rtsock_subsys_up(void * buf)
{
    struct card_msghdr *pmsg = (struct card_msghdr  *)buf;
    u_int syncflag = (pmsg->sysId <<USP_SYNCSYS_SHIFT) |
                        (USP_SYNC_DOWNSTREAM <<USP_SYNCFLAG_SHIFT)|
                        (USP_SYNC_HORIZONTAL <<USP_SYNCFLAG_SHIFT);

    bgp4_get_workmode();
 
    if (uspSelfSysIndex() == pmsg->sysId)
    {
        return;
    }
    /*TODO:master slave sync*/
    #if 0
    if ((uspHwScalarGet(NULL, HW_SYS_SWRUNNINGROLEISMASTER, NULL) == VOS_OK)
        || (uspHwScalarGet(NULL,HW_SYS_HAROLEISMASTER ,NULL) == VOS_OK))
    {
        bgp4_log(BGP_DEBUG_EVT,"receive rt sock RTM_SUBSYSTEM_UP");
        
        gbgp4.sync_flag = syncflag;        
        bgp4_timer_start(&gbgp4.init_sync_timer, BGP4_INIT_SYNC_DELAY_TIME);
    }
    #endif
    return;
}

void 
bgp4_rtsock_card_up(void * buf)
{    
    struct card_msghdr *pmsg = (struct card_msghdr  *)buf;
    tCardIndex cardIdx; 
    u_int syncflag = (pmsg->sysId <<USP_SYNCSYS_SHIFT) | 
               (pmsg->slotId <<USP_SYNCSLOT_SHIFT) | 
               (pmsg->cardId <<USP_SYNCCARD_SHIFT) | 
               (USP_SYNC_DOWNSTREAM <<USP_SYNCFLAG_SHIFT); 

    if (uspSelfSysIndex() != pmsg->sysId)/*本系统只负责对本地线卡同步*/
    {
        return;
    }
    cardIdx.sys = pmsg->sysId;
    cardIdx.slot = pmsg->slotId;
    cardIdx.card = pmsg->cardId;
    cardIdx.type = pmsg->type;
    /*TODO:master slave sync*/
    #if 0   
    if (uspHwCardGet(&cardIdx, HW_CARD_ISMPU, NULL) == VOS_OK)/*新上线的卡为主控卡*/
    {
        if ((uspHwScalarGet(NULL, HW_SYS_SWRUNNINGROLEISMASTER, NULL) == VOS_OK)||
            (uspHwScalarGet(NULL, HW_SYS_HAROLEISMASTER, NULL) == VOS_OK))
        {
            bgp4_log(BGP_DEBUG_EVT,"receive rt sock RTM_CARD_UP");
            
            gbgp4.sync_flag = syncflag;        
            bgp4_timer_start(&gbgp4.init_sync_timer, BGP4_INIT_SYNC_DELAY_TIME);
        }
    }  
    #endif
    return;
}

void 
bgp4_rtsock_slot_up(void * buf)
{
    struct card_msghdr *pmsg = (struct card_msghdr  *)buf;
    tSlotIndex slotIdx; 
    u_int syncflag = (pmsg->sysId <<USP_SYNCSYS_SHIFT) | 
                (pmsg->slotId <<USP_SYNCSLOT_SHIFT) |
                (USP_SYNC_DOWNSTREAM <<USP_SYNCFLAG_SHIFT)|
                (USP_SYNC_HORIZONTAL <<USP_SYNCFLAG_SHIFT);
 
    if (uspSelfSysIndex() != pmsg->sysId)/*本系统只负责对本地线卡同步*/
    {
        return;
    }
    slotIdx.sys = pmsg->sysId;
    slotIdx.slot = pmsg->slotId;
    slotIdx.type = pmsg->type;
    /*TODO:master slave sync*/
    #if 0
    if (uspHwSlotGet(&slotIdx, HW_SLOT_ISMPU, NULL) == VOS_OK)/*上线线卡是主控*/
    {
        if((uspHwScalarGet(NULL,HW_SYS_SWRUNNINGROLEISMASTER,NULL) == VOS_OK)||
           (uspHwScalarGet(NULL,HW_SYS_HAROLEISMASTER ,NULL) == VOS_OK))
        {
            bgp4_log(BGP_DEBUG_EVT,"receive rt sock RTM_SLOT_UP");
            
            gbgp4.sync_flag = syncflag;        
            bgp4_timer_start(&gbgp4.init_sync_timer, BGP4_INIT_SYNC_DELAY_TIME);
        }
    }
    #endif
    return;
}

void 
bgp4_rtsock_workmode_change(void)
{
    bgp4_workmode_update_timeout();
    return;
}

/*rtsock callback when route add.only applied to ipv4 unicast*/
void 
bgp4_rtsock_route_change(
        u_int vrf,
        tBGP4_ROUTE *p_route, 
        u_int add)
{
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    u_char str[64];
    u_char str2[64];

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, vrf);
    if (p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT|BGP_DEBUG_ERROR, "vpn instance %d does not exist", vrf);
        return;
    }

    /*ignore bgp route*/
    if (p_route->proto == M2_ipRouteProto_bgp)
    {
        return;
    }
    if (p_instance->rib[p_route->dest.afi] == NULL)
    {
        return;
    }
        
    bgp4_timer_start(&p_instance->rib[p_route->dest.afi]->nexthop_timer, BGP4_IGP_SYNC_MIN_INTERVAL); 

    /*get self id in some case*/
    if ((p_route->proto == M2_ipRouteProto_local) 
        && (gbgp4.router_id == 0))
    {
        gbgp4.router_id = bgp4_get_max_ifipaddr();
    }

    /*tag verify,todo*/
    bgp4_log(BGP_DEBUG_EVT,"proc %s redistribute route %s,nexthop %s",
                    (add == TRUE) ? "add" : "delete",
                    bgp4_printf_route(p_route, str),
                    bgp4_printf_addr(&p_route->p_path->nexthop, str2));

    p_route->p_path->p_instance = p_instance;
    p_route->p_path->origin_vrf = vrf;

    if ((add == FALSE) 
        || (bgp4_redistribute_is_permitted(p_instance, p_route) == TRUE))
    {
        bgp4_redistribute_route(p_instance, p_route, add);
    }
    return;
} 

#ifdef BGP_VPLS_WANTED
void
bgp4_rtsock_vpls_add(u_char *p_buf)
{
    struct l2vpn_msghdr *p_msg = (struct l2vpn_msghdr*)p_buf;
    u_int i = 0;
    
    for (i = 0; i < p_msg->cnt; i++)
    {
        bgp4_vpls_add_event_process(&p_msg->l2vpnBgpMsg[i]);
    }
    return;
}

void
bgp4_rtsock_vpls_delete(u_char *p_buf)
{
    struct l2vpn_msghdr *p_msg = (struct l2vpn_msghdr*)p_buf;
    u_int i = 0;
        
    for (i = 0; i < p_msg->cnt; i++)
    {    
        bgp4_vpls_del_event_process(&p_msg->l2vpnBgpMsg[i]);
    }     
    return;
}

void
bgp4_rtsock_vpls_update(u_char *p_buf)
{
    struct l2vpn_msghdr *p_msg = (struct l2vpn_msghdr*)p_buf;
    u_int i = 0;
    
    /*get public instance*/
    for (i = 0; i < p_msg->cnt; i++)
    {
        bgp4_vpls_update_event_process(&p_msg->l2vpnBgpMsg[i]);
    }
    return;
}
#endif

/*receive message from routing socket,and deliver to special
   callback functions,just process one message in this function*/
void
bgp4_rtsock_if_msg_input(u_char *p_buf)
{
    struct if_msghdr *p_ifmsg = (struct if_msghdr *)p_buf;
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    
    bgp4_log(BGP_DEBUG_EVT, "receive system interface change,unit %d, flag %x", 
        p_ifmsg->ifm_ifunit, p_ifmsg->ifm_flags);
    
    /*ignore slave node*/
    if (gbgp4.work_mode == BGP4_MODE_SLAVE)
    {
        return;
    }

    /*ignore up event*/
    if (p_ifmsg->ifm_flags & IFF_UP)
    {
        return;
    }
    /*scan for all established peer,timeout peer on same interface*/
    bgp4_avl_for_each(&gbgp4.instance_table, p_instance)
    {
        bgp4_avl_for_each(&p_instance->peer_table, p_peer)
        {
            if ((p_peer->if_unit == p_ifmsg->ifm_ifunit)
                && (p_peer->state == BGP4_NS_ESTABLISH))
            {
                bgp4_timer_stop(&p_peer->hold_timer);
                bgp4_peer_holdtimer_expired(p_peer);
            }
        }
    }
    return;
}

void 
bgp4_rtsock_recv(void)
{
    struct rt_msghdr *p_msg = NULL;
    u_char buf[1024];
    int number = 0;
    int len = 0;
    
    /*route socket not created,do nothing*/
    if (gbgp4.rtsock <= 0)
    {
        return;
    }

    /*read multiple msg*/
    while (number++ < 255)
    {
        len = recv(gbgp4.rtsock, (char*)buf, sizeof(buf), 0);
        if (len <= 0)
        {
            gbgp4.stat.rtsock_rx_error++;
            break;
        }
        gbgp4.stat.rtsock_rx++;

        p_msg = (struct rt_msghdr *)buf;
        /* extract and process the command from the routing socket message */
        gbgp4.stat.rtsock_cmd = p_msg->rtm_type;
        switch (p_msg->rtm_type){
            default:
            case RTM_LOSING:   /* PCB cache discard */
            case RTM_MISS:     /* Failed search */
            case RTM_LOCK:     /* Changes to some metrics disabled */
            case RTM_OLDADD:   /* Unused message for SIOCADDRT command */
            case RTM_OLDDEL:   /* Unused message for SIOCDELRT command */
            case RTM_RESOLVE:  /* Creating entry for ARP information */
            case RTM_CHANGE:   /* Change Metrics or flags */
            case RTM_REDIRECT:
                /*ignored*/
                break;

            case RTM_NEWADDR:  /* Interface is getting a new address */
            case RTM_DELADDR:  /* Address removed from interface */
                 bgp4_rtsock_addr_msg_input(buf);
                 break;
                 
            case RTM_ADD:
            case RTM_DELETE:
                 bgp4_rtsock_route_msg_input(buf);
                 break;

            case RTM_VPN_ADD:/*for vpn add,should send route refresh to PE peers*/
                 bgp4_rtsock_vpn_add_msg_input(buf);
                 break;
                 
            case RTM_VPN_DEL:/*for vpn delete,should delete peers belong to such vpn instance*/
                 bgp4_rtsock_vpn_del_msg_input(buf);
                 break;
                 
            case RTM_VPN_RT_CHANGE:
                 bgp4_rtsock_vpn_rd_msg_input(buf);
                 break;

            case RTM_TCPSYNC_ADD:
                 bgp4_rtsock_tcp_add_msg_input(buf);
                 break;
                 
            case RTM_TCPSYNC_DEL:
                 bgp4_rtsock_tcp_del_msg_input(buf);
                 break;
            
            case RTM_SUBSYSTEM_UP:
                 bgp4_rtsock_subsys_up(buf);
                 break;
  
            case RTM_SLOT_UP:
                 bgp4_rtsock_slot_up(buf);
                 break;
   
            case RTM_CARD_UP:
                 bgp4_rtsock_card_up(buf);
                 break;
  
            case RTM_CARD_DOWN:
                 break;

            case RTM_CARD_ROLECHANGE:
                 bgp4_log(BGP_DEBUG_EVT,"receive sys msg card role change");

                 bgp4_rtsock_workmode_change();
                 break;
 
            case RTM_NEW_RTMSG:
            {
                 bgp4_log(BGP_DEBUG_EVT, "receive sys msg route change");
  
                 if (((struct rt_newmsghdr *)buf)->family == AF_INET)
                 {
                     bgp4_rtsock_newroute_msg_input(buf);
                 }
                 else if (((struct rt_newmsghdr *)buf)->family == AF_INET6)
                 {
                     bgp4_rtsock_newroute6_msg_input(buf);
                 }
                 break;
            }

            case RTM_BFD_SESSION:
                 bgp4_rtsock_bfd_msg_input(buf);
                 break;

            case RTM_BFD_IFSTATUS:
                 /*ignored now*/
                 break;
                 
            case RTM_IFINFO:
                 bgp4_rtsock_if_msg_input(buf);
                 break;
                 
#ifdef BGP_VPLS_WANTED
            case RTM_L2VPNVPLS_ADD:
                 bgp4_rtsock_vpls_add(buf);
                 break;

            case RTM_L2VPNVPLS_DEL:
                 bgp4_rtsock_vpls_delete(buf);
                 break;

            case RTM_L2VPNVPLS_CHANGE:
                 bgp4_rtsock_vpls_update(buf);
                 break;
#endif
        }
    }
    return;
}

STATUS
bgp4_sys_iproute_add(
     u_int vrf,
     tBGP4_ROUTE *p_route,
     u_int hop_id,
     u_int *p_hwneed)
{
    tBGP4_NEXTHOP_BLOCK *p_nexthop = p_route->p_path->p_direct_nexthop;
    int  flag = 0;
    u_int dest = 0;
    u_int mask = 0;
    u_int nexthop = 0;    
    u_int uiPre = gbgp4.ucEbgpPre;/*default 255*/
    u_int uiCost = p_route->p_path->med;
    STATUS rc = 0;
    int error = 0;

    if (p_nexthop == NULL)
    {
        return VOS_OK;
    }
    
    *p_hwneed = FALSE; /*不需要bgp直接下硬件*/
    /* support fast backup route*/
    if (gbgp4.backup_path_enable == TRUE)
    {
        uiPre = p_route->preference;
    }
    if (p_nexthop->ifunit[hop_id] >= 0)
    {
        flag |= 0x80000000;/*for non-direct-nexthop*/
    }

    dest = ntohl(*(u_int*)p_route->dest.ip);
    /*mask = bgp4_len2mask(p_route->dest.prefixlen);*/

    /*vpn route use nexthop..not directly nexthop*/
    if (vrf && (vrf != p_route->p_path->origin_vrf))
    {
        nexthop = ntohl(*(u_int*)p_route->p_path->nexthop.ip);
        flag |= 0x80000000;/*for non-direct-nexthop*/
    }
    else
    {
        nexthop = ntohl(*(u_int*)p_nexthop->ip[hop_id].ip);
    }

    if(gbgp4.as == p_route->p_path->p_peer->as)
    {
        flag |= ZEBRA_FLAG_IBGP;
        uiPre = gbgp4.ucIbgpPre;
    }

    if(p_route->summary_route)
    {
        flag |= ZEBRA_FLAG_SUMMARY;
        uiPre = gbgp4.ucLocalPre;
    }
    
    gbgp4.stat.sys_route.add++;
    rc = systemIPv4RouteAdd(vrf, M2_ipRouteProto_bgp, dest, nexthop, p_route->dest.prefixlen, flag, uiPre, uiCost);
    
    if (rc != VOS_OK)
    {
        error = errnoGet();
        gbgp4.stat.sys_route.add_error = error;
        gbgp4.stat.sys_route.fail++;
        bgp4_log(BGP_DEBUG_RT|BGP_DEBUG_ERROR, "failed add vrf %d to sys route , %d", vrf, error);
        
        if ((error == ERTMSGNOBUF) || (error == ENOBUFS))
        {
            return VOS_ERR;
        }
        gbgp4.stat.sys_route.add_nomsg_error = error;
        *p_hwneed = FALSE;
    }
    return VOS_OK;
}

STATUS
bgp4_sys_ip6route_add(
     u_int vrf,
     tBGP4_ROUTE *p_route,
     u_int hop_id,
     u_int *p_hwneed)
{
    tBGP4_NEXTHOP_BLOCK *p_nexthop = p_route->p_path->p_direct_nexthop;
    int  flag = 0x100;
    u_int mask = 0;
    u_int cost = 255;/*default 100*/
    STATUS rc = 0;
    int error = 0;

    if (p_nexthop == NULL)
    {
        return VOS_OK;
    }
        
    /*support fast backup route*/
    if (gbgp4.backup_path_enable == TRUE)
    {
        cost = p_route->preference;
    }
    
    if (p_nexthop->ifunit[hop_id] >= 0)
    {
        flag |= 0x80000000;/*for non-direct-nexthop*/
    }

    mask = p_route->dest.prefixlen;

    gbgp4.stat.sys_route6.add++;
    rc = systemIPv6RouteAdd2(gbgp4.rtsock, vrf, 
            M2_ipRouteProto_bgp, p_route->dest.ip,
            p_nexthop->ip[hop_id].ip,
            mask, flag, cost);
    
    if (rc != VOS_OK)
    {
        error = errnoGet();
        gbgp4.stat.sys_route6.fail++;
        gbgp4.stat.sys_route6.add_error = error;

        bgp4_log(BGP_DEBUG_RT|BGP_DEBUG_ERROR, "faile add vrf %d to sys route  %d", vrf, error);
        
        if ((error == ERTMSGNOBUF) || (error==ENOBUFS))
        {
            return VOS_ERR;
        }
        gbgp4.stat.sys_route6.add_nomsg_error = error;
        *p_hwneed = FALSE;
    }
    return VOS_OK;
}

STATUS 
bgp4_sys_iproute_delete(
     u_int vrf,
     tBGP4_ROUTE *p_route,
     u_int hop_id,
     u_int *p_hwneed)
{
    tBGP4_NEXTHOP_BLOCK *p_nexthop = p_route->p_path->p_direct_nexthop;
    int flag = 0;
    u_int dest = 0;
    u_int mask = 0;
    u_int nexthop = 0;
    STATUS rc = 0;
    int error = 0;

    if (p_nexthop == NULL)
    {
        return VOS_OK;
    }

    *p_hwneed = FALSE; /*不需要bgp直接下硬件*/
    if (p_nexthop->ifunit[hop_id] >= 0)
    {
        flag |= 0x80000000;/*for non-direct-nexthop*/
    }

    dest = ntohl(*(u_int*)p_route->dest.ip);
    mask = bgp4_len2mask(p_route->dest.prefixlen);

    /*vpn route use nexthop..not directly nexthop*/
    if (vrf && (vrf != p_route->p_path->origin_vrf))
    {
        nexthop = ntohl(*(u_int*)p_route->p_path->nexthop.ip);
        flag |= 0x80000000;/*for non-direct-nexthop*/
    }
    else
    {
        nexthop = ntohl(*(u_int*)p_nexthop->ip[hop_id].ip);
    }
    
    if(gbgp4.as == p_route->p_path->p_peer->as)
    {
        flag |= ZEBRA_FLAG_IBGP;
    }

    gbgp4.stat.sys_route.delete++;

    rc = systemIPv4RouteDelete(vrf, M2_ipRouteProto_bgp, dest, nexthop, p_route->dest.prefixlen, flag, 0, 0);

    if (rc != VOS_OK)
    {
        error = errnoGet();
        gbgp4.stat.sys_route.delete_error = error;
        gbgp4.stat.sys_route.delete_fail++;
        
        bgp4_log(BGP_DEBUG_RT|BGP_DEBUG_ERROR, "failed delete vrf %d to sys route,%d", vrf, error);
        
        if ((error == ERTMSGNOBUF) || (error == ENOBUFS))
        {
            return VOS_ERR;
        }
        /*no rtmsg send if error is not NO route*/
        if (error != ESRCH)
        {
            *p_hwneed = FALSE;
        }
        gbgp4.stat.sys_route.delete_nomsg_error = error;
    }
    return VOS_OK;
}

STATUS 
bgp4_sys_ip6route_delete(
     u_int vrf,
     tBGP4_ROUTE *p_route,
     u_int hop_id,
     u_int *p_hwneed)
{
    tBGP4_NEXTHOP_BLOCK *p_nexthop = p_route->p_path->p_direct_nexthop;
    int flag = 0x100;
    u_int mask = 0;
    STATUS rc = 0;
    int error = 0;

    if (p_nexthop == NULL)
    {
        return VOS_OK;
    }

    *p_hwneed = TRUE; 
    if (p_nexthop->ifunit[hop_id] >= 0)
    {
        flag |= 0x80000000;/*for non-direct-nexthop*/
    }

    mask = p_route->dest.prefixlen;

    gbgp4.stat.sys_route6.delete++;
    rc = systemIPv6RouteDelete2(gbgp4.rtsock, vrf,
                M2_ipRouteProto_bgp,
                p_route->dest.ip,
                p_nexthop->ip[hop_id].ip,
                mask, flag, 0);
    if (rc != VOS_OK)
    {
        error = errnoGet();
        gbgp4.stat.sys_route6.delete_error = error;
        gbgp4.stat.sys_route6.delete_fail++;
        
        bgp4_log(BGP_DEBUG_RT|BGP_DEBUG_ERROR, "failed to delete vrf %d sys route,%d", vrf, error);

        if ((error == ERTMSGNOBUF) || (error== ENOBUFS))
        {
            return VOS_ERR;
        }
        /*no rtmsg send if error is not NO route*/
        if (error != ESRCH)
        {
            *p_hwneed = FALSE;
        }  
        gbgp4.stat.sys_route6.delete_nomsg_error = error;
    }
    return VOS_OK;
}

STATUS
bgp4_sys_iproute_lookup(
      u_int vrf,
      tBGP4_ROUTE *p_route)
{
    M2_IPROUTETBL m2route;
    u_int mask = 0;

    memset(&m2route, 0, sizeof(m2route));
    mask = bgp4_len2mask(p_route->dest.prefixlen);
    if (ipv4v6RouteLookup(vrf, AF_INET, p_route->dest.ip, mask, &m2route) == VOS_OK)
    {
        bgp4_log(BGP_DEBUG_RT, "get a sys route %#x,mask %#x,proto type %d",
            m2route.ipRouteDest,
            m2route.ipRouteMask,
            m2route.ipRouteProto);
        return VOS_OK;
    }
    return VOS_ERR;
}

STATUS
bgp4_sys_ip6route_lookup(
      u_int vrf,
      tBGP4_ROUTE *p_route)
{
    M2_IPV6ROUTETBL m2route6;
    u_int mask = 0;
    u_char addstr[64];

    memset(&m2route6, 0, sizeof(m2route6));
    mask = p_route->dest.prefixlen;
    if (ipv4v6RouteLookup(vrf, AF_INET6, p_route->dest.ip, mask, &m2route6) == VOS_OK)
    {
        inet_ntop(AF_INET6,(m2route6.ipv6RouteDest),addstr,64);
    
        bgp4_log(BGP_DEBUG_RT,"get a sys ip6route %s,prefix length %d,proto type %d",
            addstr,
            m2route6.ipv6RoutePfxLength,
            m2route6.ipv6RouteProtocol);
    
        return VOS_OK;
    }

    return VOS_ERR;
}

/*lookup in kernal route*/
int 
bgp4_sysroute_lookup(
      u_int vrf,
      tBGP4_ROUTE *p_route)
{
    if (p_route->dest.afi == BGP4_PF_IPUCAST)
    {
       return bgp4_sys_iproute_lookup(vrf, p_route);
    }
    else if (p_route->dest.afi == BGP4_PF_IP6UCAST)
    {
        return bgp4_sys_ip6route_lookup(vrf, p_route);
    }
    return VOS_ERR;
}

u_char bgp4_hw_route = 1;

STATUS 
bgp4_sys_msg_send(u_char*p_buf)
{
    struct rt_newmsghdr *p_rtmsg = (struct rt_newmsghdr *)p_buf;
    u_int len;
    int error = 0;
    
    if (bgp4_hw_route == 0)
    {
        return VOS_OK;
    }

    if (p_buf == NULL)
    {
        return VOS_ERR;
    }

    if (p_rtmsg->count == 0)
    {
        return VOS_OK;
    }

    len = p_rtmsg->rtm_msglen;
   
    p_rtmsg->rtm_msglen = htons(len);

    if (p_rtmsg->family == AF_INET)
    {
        gbgp4.stat.sys_route.msg_send++;
        gbgp4.stat.sys_route.msg_total_len += len;
    }
    else if (p_rtmsg->family == AF_INET6)
    {
        gbgp4.stat.sys_route6.msg_send++;
        gbgp4.stat.sys_route6.msg_total_len += len;
    }
    
    bgp4_log(BGP_DEBUG_RT, "send route msg to hw,count %d", p_rtmsg->count);
    if ((gbgp4.rtsock > 0) && (send(gbgp4.rtsock, p_buf, len, 0) < 0))
    {
        bgp4_log(BGP_DEBUG_RT|BGP_DEBUG_ERROR, "send route msg to hw failed");
        if (p_rtmsg->family == AF_INET)
        {
            gbgp4.stat.sys_route.msg_fail++;
        }
        else if (p_rtmsg->family == AF_INET6)
        {
            gbgp4.stat.sys_route6.msg_fail++;
        }
        
        error = errnoGet();
        if ((error == ERTMSGNOBUF) || (error == ENOBUFS))
        {
            return VOS_ERR;
        }
    }
    if (p_rtmsg->family == AF_INET)
    {
        gbgp4.stat.sys_route.msg_total_route += p_rtmsg->count;
    }
    else if (p_rtmsg->family == AF_INET6)
    {
        gbgp4.stat.sys_route6.msg_total_route += p_rtmsg->count;
    }
    /*reset buffer*/
    p_rtmsg->rtm_msglen = 0;
    p_rtmsg->count = 0;
    return VOS_OK;
}

STATUS
bgp4_sys_ip_msg_insert(
     u_int vrf,
     u_char *p_buf,
     tBGP4_ROUTE *p_route,
     u_int hop_id)
{
    struct rt_newmsghdr *p_msg = (struct rt_newmsghdr *)p_buf;
    struct rt4_newmsg *p_newroute = NULL;
    tBGP4_NEXTHOP_BLOCK *p_nexthop = p_route->p_path->p_direct_nexthop;
    u_int len = 0;
    u_int nexthop = 0;
    u_int cost = 100;/*default 100*/

    len = p_msg->rtm_msglen;
    if (((len + sizeof(struct rt4_newmsg)) > RT_MSG_MAXLEN) 
        || p_msg->vrfid != vrf)
    {
        if (bgp4_sys_msg_send(p_buf) != VOS_OK)
        {
            gbgp4.stat.sys_route.msg_fail++;
            bgp4_log(BGP_DEBUG_RT|BGP_DEBUG_ERROR, "hardware update failed");
            return VOS_ERR;
        }
    }

    if (p_route->p_path->med)
    {
        cost = p_route->p_path->med;
    }

    /*init hdr*/
    if (p_msg->rtm_msglen == 0)
    {
        p_msg->rtm_msglen = sizeof(struct rt_newmsghdr );
        p_msg->rtm_version = RTM_VERSION;
        p_msg->rtm_type = RTM_NEW_RTMSG;
        p_msg->proto = M2_ipRouteProto_bgp;
        p_msg->family = AF_INET;
        p_msg->count = 0;
        p_msg->vrfid = vrf;
    }
    p_newroute = (struct rt4_newmsg *)(p_buf+ p_msg->rtm_msglen);
    p_newroute->dest = *(u_int*)p_route->dest.ip;
    p_newroute->mask = bgp4_len2mask(p_route->dest.prefixlen);

    if (p_route->out_label)
    {
        nexthop = ntohl(*(u_int*)p_route->p_path->nexthop.ip);
    }
    else
    {
        nexthop = ntohl(*(u_int*)p_nexthop->ip[hop_id].ip);
    }
    if (p_nexthop->ifunit[hop_id] >= 0)
    {
        p_newroute->if_unit = p_nexthop->ifunit[hop_id];
    }
    else
    {
        p_newroute->if_unit = p_route->p_path->p_peer->if_unit;
    }
    p_newroute->action = (p_route->system_selected == TRUE) ? RTM_ADD : RTM_DELETE;

    p_newroute->nexthop = nexthop;

    p_newroute->cost = cost;

    if (p_route->out_label)
    {
        p_newroute->flag = RTFLAG_VPN_REMOTE;
    }
    else
    {
        p_newroute->flag = 0;
    }

    p_msg->count ++;
    p_msg->rtm_msglen += sizeof(struct rt4_newmsg);

    if (p_route->system_selected == TRUE)
    {
        gbgp4.stat.sys_route.msg_add++;
    }
    else 
    {
        gbgp4.stat.sys_route.msg_delete++;
    }
    return VOS_OK;
}

STATUS 
bgp4_sys_ip6_msg_insert(
       u_int vrf,
       u_char *p_buf,
       tBGP4_ROUTE *p_route,
       u_int hop_id)
{
    struct rt_newmsghdr *p_rtmsg = (struct rt_newmsghdr  *)p_buf;
    struct rt6_newmsg *p_newroute = NULL;
    tBGP4_NEXTHOP_BLOCK *p_nexthop = p_route->p_path->p_direct_nexthop;    
    u_int len = 0;
    u_int cost = 100;/*default 100*/

    if (p_buf == NULL)
    {
        return VOS_ERR;
    }

    len = p_rtmsg->rtm_msglen;
    if (((len + sizeof(struct rt6_newmsg)) > RT_MSG_MAXLEN) 
        || (p_rtmsg->vrfid != vrf))
    {
        if (bgp4_sys_msg_send(p_buf) != VOS_OK)
        {
            bgp4_log(BGP_DEBUG_RT|BGP_DEBUG_ERROR,"hardware update failed");
            gbgp4.stat.sys_route6.msg_fail++;
            return VOS_ERR;
        }
    }

    if (p_route->p_path->med)
    {
        cost = p_route->p_path->med;
    }

    if (p_rtmsg->rtm_msglen == 0)
    {
        p_rtmsg->rtm_msglen = sizeof(struct rt_newmsghdr );
        p_rtmsg->rtm_version = RTM_VERSION;
        p_rtmsg->rtm_type = RTM_NEW_RTMSG;
        p_rtmsg->proto = M2_ipRouteProto_bgp;
        p_rtmsg->family = AF_INET6;
        p_rtmsg->count = 0;
        p_rtmsg->vrfid = vrf;
    }

    p_newroute = (struct rt6_newmsg *)(p_buf + p_rtmsg->rtm_msglen);
    memcpy(p_newroute->dest, p_route->dest.ip, 16);

    p_newroute->prefixlen = p_route->dest.prefixlen;

    if (p_nexthop->ifunit[hop_id] >= 0)
    {
        memcpy(p_newroute->nexthop, p_nexthop->ip[hop_id].ip, 16);
        p_newroute->ifunit = p_nexthop->ifunit[hop_id];
    }
    else
    {
        memcpy(p_newroute->nexthop, p_route->p_path->nexthop.ip, 16);
        p_newroute->ifunit = p_route->p_path->p_peer->if_unit;
    }

    p_newroute->action = (p_route->system_selected == TRUE) ? RTM_ADD : RTM_DELETE;

    p_newroute->cost = cost;
    
    if (p_route->out_label)
    {
        p_newroute->flag = RTFLAG_VPN_REMOTE;
    }
    else
    {
        p_newroute->flag = 0;
    }

    p_rtmsg->count ++;
    p_rtmsg->rtm_msglen += sizeof(struct rt6_newmsg);

    if (p_route->system_selected == TRUE)
    {
        gbgp4.stat.sys_route6.msg_add++;
    }
    else
    {
        gbgp4.stat.sys_route6.msg_delete++;
    }
    return VOS_OK;
}

STATUS 
bgp4_sys_msg_add(
     u_int vrf,
     u_char *p_buf,
     tBGP4_ROUTE *p_route,
     u_int hop_id)
{
#if 0 /*暂时应该不需要caoyong 2017-11-14*/
    if (p_buf == NULL)
    {
        return VOS_ERR;
    }
    /*consider ipv6 route*/
    if (p_route->dest.afi == BGP4_PF_IP6UCAST)
    {
        return bgp4_sys_ip6_msg_insert(vrf, p_buf, p_route, hop_id);
    }
    else if (p_route->dest.afi == BGP4_PF_IPUCAST)
    {
        return bgp4_sys_ip_msg_insert(vrf, p_buf, p_route, hop_id);
    }
#endif
    return VOS_OK;
}

/*get linklocal address from a global ipv6 address*/
void 
bgp4_ip6_linklocal_nexthop_get(
          u_char *p_global,
          u_char *p_local)
{
    u_char addr[64];
    octetstring octet;
    u_int if_number = 0;
    u_char buf[19];
    u_char local_buf[512];

    memset(p_local, 0, 16);
    
    memset(buf, 0, 19);
    octet.pucBuf = buf;
    octet.len = 19;
    buf[2] = IP_V6FAMILY;

    memcpy(buf+3, p_global, 16);
    sysIndexTrans(&octet, SYS_IPADDR2LOGICAL_NUM, &if_number);

    /*get local address*/
    memset(local_buf, 0, 512);
    memset(addr, 0, 64);
    octet.pucBuf = local_buf;
    octet.len = 512;
    if(VOS_OK != zebra_if_get_api(if_number,ZEBRA_IF_GET_IPV6_ADDR_EX_BY_IF,&octet))
    {
        return;
    }
    /*
    地址格式:
    --------------------------------------------------------------------
    |  1byte          |  1byte          |     16byte    | 1bype | 1byte  |
    --------------------------------------------------------------------
    |  type            |  prefix         |   address   | role  |  type  |
    ---------------------------------------------------------------------
    #define USP_IPROLE_PRIMARY    0
    #define USP_IPROLE_SECONDARY    1
    #define USP_IPROLE_LINKLOCAL    2
    #define USP_IPROLE_LOOPBACK        3
            role:3,
            used:1,
    #define USP_IPTYPE_STATIC    0
    #define USP_IPTYPE_DHCP        1
    #define USP_IPTYPE_AUTO        2    自动配置ipv6 linklocal地址，如果网管要配置linklocal地址要用USP_IPTYPE_STATIC类型
    #define USP_IPTYPE_VRRP        3
    #define USP_IPTYPE_UNNUMBERED    4
    */
    /*get wanted octet string*/
    while (octet.len >= 20)
    {
        if((*(octet.pucBuf)==IP_V6FAMILY)
            &&(*(octet.pucBuf + 18) == 2))
        {
            memcpy(p_local, octet.pucBuf+2, 16);
            break;
        }
        octet.len -= 20;
        octet.pucBuf += 20;
    }
    return;
}

void 
bgp4_redistribute_set(
         tBGP4_VPN_INSTANCE* p_instance,
         u_char af/*internal IPUCAST,IP6UCAST*/,
         u_char proto, 
         u_char enable,
         u_int processId)
{
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE delete_route;
    tBGP4_ROUTE *p_next = NULL;
    tBGP4_RIB *p_rib = NULL;
    
    rtSockOptionSet(gbgp4.rtsock, IP_ROUTE_REDIS_PROTO(proto), enable);

    if (enable == TRUE)
    {        
        if (af == BGP4_PF_IPUCAST)
        {
            bgp4_log(BGP_DEBUG_RT,"set ipv4 route: vrf %d action %d,proto %d",
                        p_instance->vrf,
                        enable,
                        proto);
            
            ipRouteRedisSet(p_instance->vrf, AF_INET, proto, M2_ipRouteProto_bgp,processId);
        }
        else if (af == BGP4_PF_IP6UCAST)
        {
            bgp4_log(BGP_DEBUG_RT,"set ipv6 route: vrf %d  action %d,proto %d",
                        p_instance->vrf,
                        enable,
                        proto);
            
            ipRouteRedisSet(p_instance->vrf, AF_INET6, proto, M2_ipRouteProto_bgp,processId);
        }

        rtSockOptionSet(gbgp4.rtsock, proto, enable);
    }
    else
    {        
        rtSockOptionSet(gbgp4.rtsock, proto, enable);
        
       /*delete route in this instance*/
        p_rib = p_instance->rib[af];
        if (p_rib == NULL)
        {
            return;
        }      
        bgp4_avl_for_each_safe(&p_rib->rib_table, p_route, p_next)
        {
            if ((p_route->proto == proto)
                && (p_route->p_path->origin_vrf
                    == p_instance->vrf))
            {
                if(proto == M2_ipRouteProto_ospf && p_route->processId != processId)
                {
                    return VOS_ERR;
                }
                /*build route to be deleted*/
                memset(&delete_route, 0, sizeof(delete_route));
                memcpy(&delete_route.dest, &p_route->dest, sizeof(tBGP4_ADDR));
                delete_route.proto = p_route->proto;
                delete_route.is_deleted = TRUE;
                delete_route.p_path = p_route->p_path;

                bgp4_vrf_route_export_check(&delete_route, FALSE);

                /*processed as withdraw recv*/
                bgp4_rib_in_table_update(&delete_route);
            }
        }
    }
    return;
}

u_int 
bgp4_sys_ifunit_to_prefixlen(
     u_int ifunit,
     u_int af)
{
    u_int prefix_len = 0;
    //u_char buf[256] = {0};
    //octetstring octet;
    struct prefix_ipv4 ipAddrInfo;
    u_int cmd = 0;

    /* 获取接口掩码信息
        地址格式:
        ---------------------------------------------
        |  1byte          |  1byte          |     4byte or 16byte     |
        ---------------------------------------------
        |  type            |  prefix len         |   address                   |
        ---------------------------------------------
    */

    cmd = (af == BGP4_PF_IPUCAST) ? ZEBRA_IF_IPADRESS_GET : ZEBRA_IF_IPV6ADRESS_GET;
    
    if(VOS_OK != zebra_if_get_api(ifunit,cmd,&ipAddrInfo))
    {
        ;
    }
    
    prefix_len = ipAddrInfo.prefixlen;
    return prefix_len;
}

void 
bgp4_set_router_id(u_int id)
{
	ZEBRA_ROUTE_ID_T stRouteIdIndex = {0}; 
	
	if (id != 0)
	{
		gbgp4.BgpRouteIdFlag = 1;
		gbgp4.router_id = id;

	}
	else
	{
	    gbgp4.BgpRouteIdFlag = 0;
        gbgp4.router_id = bgp_select_router_id(0);
	}
	
    return;
}

int 
bgp4_nexthop_metric_cmp(
        tBGP4_PATH *p1,
        tBGP4_PATH *p2)
{
    ZEBRA_ROUTE_MSG_T route1;
    ZEBRA_ROUTE_MSG_T route2;
    M2_IPV6ROUTETBL v6route1;
    M2_IPV6ROUTETBL v6route2;
    u_int vrf = 0;

    /*address family must same*/
    if (p1->af != p2->af)
    {
        return (p1->af > p2->af) ? 1 : -1;
    }
    
    if (bgp4_index_to_safi(p1->af) != BGP4_SAF_UCAST)
    {
        return 0;
    }

    if ((p1->p_instance == NULL)
       || (p2->p_instance == NULL) 
       || (p1->p_instance->vrf 
           != p2->p_instance->vrf))
    {
        return 0;
    }

    vrf = p1->p_instance->vrf;

    if (bgp4_index_to_afi(p1->af) == BGP4_AF_IP)
    {
        memset(&route1, 0, sizeof(route1));
        memset(&route2, 0, sizeof(route2));
        if ((ip_route_match(vrf, ntohl(bgp_ip4(p1->nexthop.ip)), &route1) == VOS_OK)
           && (ip_route_match(vrf, ntohl(bgp_ip4(p2->nexthop.ip)), &route2) == VOS_OK))
        {
            if (route1.uiMetric != route2.uiMetric)
            {
                return (route1.uiMetric < route2.uiMetric) ? 1 : -1;
            }
        }
    }
    else if (bgp4_index_to_afi(p1->af) == BGP4_AF_IP6)
    {
        memset(&v6route1, 0, sizeof(v6route1));
        memset(&v6route2, 0, sizeof(v6route2));

        if ((ip6_route_match(vrf, p1->nexthop.ip, &v6route1) == VOS_OK)
            && (ip6_route_match(vrf, p2->nexthop.ip, &v6route2) == VOS_OK))
        {
            if (v6route1.ipv6RouteMetric != v6route2.ipv6RouteMetric)
            {
                return (v6route1.ipv6RouteMetric < v6route2.ipv6RouteMetric) ? 1 : -1;
            }
        }
    }
    
    return 0;
}

int 
bgp4_nexthop_is_reachable(
       u_int vrf,
       tBGP4_ADDR* p_nexthop)
{
    return VOS_OK;/*TODO*/
}

u_int 
bgp4_is_local_network(
         u_short vrf,
         u_char afi,
         u_char *p_ip)
{
    octetstring octet = {0};
    u_char buf[20] = {0};
    u_int ifunit = 0;
    u_short vrid = htons(vrf);

    /*
        ------------------------------------------------------------
        |  2byte vrid   | 1byte family |    4 or 16 byte ip address |
        ------------------------------------------------------------

        1 byte faily:IP_V4FAMILY,IP_V6FAMILY
        vrid:net byte order
        ip address:net byte order
    */


    memset(buf, 0, sizeof(buf));
    octet.pucBuf = buf;
    octet.len = (afi == BGP4_AF_IP) ? 7 : 19;

    memcpy(buf,&vrid,2);

    buf[2] = (afi == BGP4_AF_IP) ? AF_INET: AF_INET6;

    memcpy(buf + 3, p_ip, 16);
    return (sysIndexTrans(&octet, SYS_IPNET2LOGICAL_NUM, &ifunit));
}

int 
bgp4_direct_nexthop_calculate(tBGP4_PATH *p_path)
{
    ZEBRA_ROUTE_MSG_T route;
    M2_IPV6ROUTETBL route6;
    tBGP4_ADDR nexthop;
    int rc = VOS_OK;
    int ifunit = -1;
    u_int vrf = 0;
    u_int safi = bgp4_index_to_safi(p_path->af);
    u_int afi = bgp4_index_to_afi(p_path->af);
    u_char str[64] = {0};
    memset(&route, 0, sizeof(route));
    memset(&route6, 0, sizeof(route6));

    /*no need for route from local*/
    if (p_path->p_peer == NULL)
    {
        return VOS_OK;
    }

    /*if ipv6 route is from 6PE peer,do not calculate nexthop*/
    if ((p_path->af == BGP4_PF_IP6UCAST)
        && (bgp4_af_support(p_path->p_peer, BGP4_PF_IP6LABEL)))
    {
        memcpy(&nexthop, &p_path->nexthop, sizeof(tBGP4_ADDR));
        nexthop.afi = BGP4_PF_IP6UCAST;
        nexthop.prefixlen = 128;

        if (p_path->p_direct_nexthop == NULL)
        {
            p_path->p_direct_nexthop = bgp4_malloc(sizeof(tBGP4_NEXTHOP_BLOCK), MEM_BGP_NEXTHOP);
        }
        
        memcpy(&p_path->p_direct_nexthop->ip[0], &nexthop, sizeof(tBGP4_ADDR));
        p_path->p_direct_nexthop->ifunit[0] = ifunit;
        p_path->p_direct_nexthop->count = 1;
        return VOS_OK;
    }
    /*mpls remote vpn route's nexthop will be found by mpls*/
    if ((safi == BGP4_SAF_LBL) || (safi == BGP4_SAF_VLBL))
    {
        bgp4_log(BGP_DEBUG_RT, "the remote routing of MPLS is determined by MPLS.");

        if (afi == BGP4_AF_IP)
        {
            memcpy(&nexthop, &p_path->nexthop, sizeof(tBGP4_ADDR));
            nexthop.afi = BGP4_PF_IPUCAST;
            nexthop.prefixlen = 32;

            if (p_path->p_direct_nexthop == NULL)
            {
                p_path->p_direct_nexthop = bgp4_malloc(sizeof(tBGP4_NEXTHOP_BLOCK), MEM_BGP_NEXTHOP);
            }
            
            memcpy(&p_path->p_direct_nexthop->ip[0], &nexthop, sizeof(tBGP4_ADDR));
            p_path->p_direct_nexthop->ifunit[0] = ifunit;
            p_path->p_direct_nexthop->count = 1;
        }
        return VOS_OK;
    }

    if (p_path->af == BGP4_PF_L2VPLS)
    {
        afi = BGP4_AF_IP;
        safi = BGP4_SAF_UCAST;
    }

    if (afi == BGP4_AF_IP)
    {
        memcpy(&nexthop, &p_path->nexthop, sizeof(tBGP4_ADDR));
        nexthop.afi = BGP4_PF_IPUCAST;
        nexthop.prefixlen = 32;
    }
    else if (afi == BGP4_AF_IP6)
    {
        memcpy(&nexthop, &p_path->nexthop, sizeof(tBGP4_ADDR));
        nexthop.afi = BGP4_PF_IP6UCAST;
        nexthop.prefixlen = 128;
    }
    else
    {
        bgp4_log(BGP_DEBUG_RT, "this is invalid path type for direct nexthop");
        return VOS_OK;
    }

    vrf = p_path->p_instance->vrf;

    if (vrf != p_path->origin_vrf)
    {
        bgp4_log(BGP_DEBUG_RT, "vpn route use remote nexthop");
        if (p_path->p_direct_nexthop == NULL)
        {
            p_path->p_direct_nexthop = bgp4_malloc(sizeof(tBGP4_NEXTHOP_BLOCK), MEM_BGP_NEXTHOP);
        }
        
        memcpy(&p_path->p_direct_nexthop->ip[0], &nexthop, sizeof(tBGP4_ADDR));
        p_path->p_direct_nexthop->ifunit[0] = ifunit;
        p_path->p_direct_nexthop->count = 1;
        return VOS_OK;
    }
    
    bgp4_log(BGP_DEBUG_RT,"decide direct nexthop for %s,vrf %d, af %d",
        bgp4_printf_addr(&nexthop, str), vrf, afi);

    if (bgp4_is_local_network(vrf, afi, nexthop.ip) == VOS_OK)
    {
        if (p_path->p_direct_nexthop == NULL)
        {
            p_path->p_direct_nexthop = bgp4_malloc(sizeof(tBGP4_NEXTHOP_BLOCK), MEM_BGP_NEXTHOP);
        }
        
        memcpy(&p_path->p_direct_nexthop->ip[0], &nexthop, sizeof(tBGP4_ADDR));
        p_path->p_direct_nexthop->ifunit[0] = ifunit;
        p_path->p_direct_nexthop->count = 1;
        
        bgp4_log(BGP_DEBUG_RT, "original nexthop is directed");
        return VOS_OK;
    }

    /*try to get final direct nexthop*/
    while (bgp4_is_local_network(vrf, afi, nexthop.ip) == VOS_ERR)
    {
        /*ipv6 linklocal address is direct nexthop*/
        if ((afi == BGP4_AF_IP6)
            && (nexthop.ip[0] == 0xfe) 
            && (nexthop.ip[1] == 0x80))
        {
            bgp4_log(BGP_DEBUG_RT, "nexthop is linklocal,must direct nexthop");
            break;
        }

        if (afi == BGP4_AF_IP)
        {
            rc = ip_route_match(vrf, ntohl(bgp_ip4(nexthop.ip)), &route);
            if (rc != VOS_OK)
            {
                bgp4_log(BGP_DEBUG_RT, "no route for recursive nexthop");
                return VOS_ERR;
            }
            /*error case:if current nexthop and obtained nexthop are
              same,do nothing mor*/
            if (memcmp(nexthop.ip, &route.stNexthop, 4) == 0)
            {
                bgp4_log(BGP_DEBUG_RT, "same nexthop and route,but not local route");
                return VOS_ERR;
            }
            memcpy(nexthop.ip, &route.stNexthop, 4);
            nexthop.afi = BGP4_PF_IPUCAST;
            nexthop.prefixlen = 32;

            bgp4_log(BGP_DEBUG_RT, "get recursive nexthop %s", bgp4_printf_addr(&nexthop, str));
            #if 0 /*caoyong 修改,应该不需要了*/
            if(zebra_if_get_api(route.uiIfIndex,ZEBRA_IF_GET_BY_IP_IF,&ifunit) == VOS_ERR)
            {
                bgp4_log(BGP_DEBUG_RT, "no ip interface %d", route.uiIfIndex);

                return VOS_ERR;
            }
            #endif
        }
        else
        {
            rc = ip6_route_match(vrf, p_path->nexthop.ip, &route6);
            if (rc != VOS_OK)
            {
                bgp4_log(BGP_DEBUG_RT, "nexthop not found");
                
                return VOS_ERR;
            }
            /*error case:if current nexthop and obtained nexthop are
              same,do nothing mor*/
            if (memcmp(nexthop.ip, &route.stNexthop, 16) == 0)
            {
                bgp4_log(BGP_DEBUG_RT, "nexthop and route is same,but not local route");
                return VOS_ERR;
            }
            memcpy(nexthop.ip, route6.ipv6RouteNextHop, 16);
            nexthop.afi = BGP4_PF_IP6UCAST;
            nexthop.prefixlen = 128;
            bgp4_log(BGP_DEBUG_RT, "get recursive v6nexthop %s", bgp4_printf_addr(&nexthop, str));
            #if 0 /*caoyong 修改,应该不需要了*/
            if(zebra_if_get_api(route6.ipv6RouteIfIndex,ZEBRA_IF_GET_BY_IP_IF,&ifunit) == VOS_ERR)
            {
                bgp4_log(BGP_DEBUG_RT, "no ip interface %d", route6.ipv6RouteIfIndex);
                return VOS_ERR;
            }
            #endif
        }
    }
    if (p_path->p_direct_nexthop == NULL)
    {
        p_path->p_direct_nexthop = bgp4_malloc(sizeof(tBGP4_NEXTHOP_BLOCK), MEM_BGP_NEXTHOP);
    }
    
    memcpy(&p_path->p_direct_nexthop->ip[0], &nexthop, sizeof(tBGP4_ADDR));
    p_path->p_direct_nexthop->ifunit[0] = ifunit;
    p_path->p_direct_nexthop->count = 1;
    
    bgp4_log(BGP_DEBUG_RT, "direct nexthop %s,interface unit %d",
                bgp4_printf_addr(&p_path->p_direct_nexthop->ip[0], str),
                p_path->p_direct_nexthop->ifunit[0]);

    return VOS_OK;
}

/*must bind when peer state is ESTABLISH*/
void  
bgp4_bind_bfd(tBGP4_PEER *p_peer)
{
    u_char str[64] = {0};

    if (p_peer->bfd_enable == FALSE)
    {
        return;
    }

    if (p_peer->bfd_discribe > 0)
    {
        return;
    }

    bgp4_log(BGP_DEBUG_EVT, "bind bfd for %s", bgp4_printf_peer(p_peer, str));

    if (p_peer->ip.afi == BGP4_PF_IPUCAST)
    {
        p_peer->bfd_discribe = bfdSessionBind(IPPROTO_BGP4, 
                                  p_peer->if_unit, BFD_IPV4, 4,
                                  p_peer->local_ip.ip, 
                                  p_peer->ip.ip, NULL);
    }
    else if (p_peer->ip.afi == BGP4_PF_IP6UCAST)
    {
        p_peer->bfd_discribe = bfdSessionBind(IPPROTO_BGP4, 
                                  p_peer->if_unit, BFD_IPV6, 16,
                                  p_peer->local_ip.ip, 
                                  p_peer->ip.ip, NULL);                   
    }
    if (p_peer->bfd_discribe <= 0)
    {
        bgp4_log(BGP_DEBUG_EVT, "bind bfd failed");

        /*start bfd checking timer*/
        bgp4_timer_start(&p_peer->bfd_timer, BGP4_PEER_ADJOUT_INTERVAL);
    }
    return;
}

void  
bgp4_unbind_bfd(tBGP4_PEER *p_peer)
{
    if (p_peer->bfd_discribe > 0)
    {
        bgp4_log(BGP_DEBUG_EVT, "unbind bfd discribe %x", p_peer->bfd_discribe);
        
        bfdSessionUnbind(IPPROTO_BGP4, p_peer->bfd_discribe, NULL);
    }
    p_peer->bfd_discribe = 0;

    /*restart bfd timer if bfd enabled*/
    if (p_peer->bfd_enable)
    {
        bgp4_timer_start(&p_peer->bfd_timer, BGP4_PEER_ADJOUT_INTERVAL);
    }
    return;
}


/*notify other protocol,bgp started*/
void 
bgp4_up_notify(void)
{
    struct vpnGen_msghdr rtmsg = {0};
    u_int len = 0;
    
    rtmsg.rtm_msglen = sizeof(struct vpnGen_msghdr);
    rtmsg.rtm_version = RTM_VERSION;
    rtmsg.rtm_type = RTM_BGP_UP;
    rtmsg.process_id = 0;

    len = rtmsg.rtm_msglen;
    
    if (send(gbgp4.rtsock, (u_char *)&rtmsg, len, 0) < 0)
    {
    }
    return;
}

#else

#include "bgp4com.h"


extern STATUS ipv4v6RouteLookup(u_int routeInstance,int addrFamily,char *dest, int mask,void *pRouteInfo);
extern STATUS ip_route_match(u_int routeInstance,u_int dest, void *p_info);
extern STATUS ip6_route_match(u_int routeInstance,char *dest, M2_IPV6ROUTETBL *p_info);

int bgp4_init_rtsock()
{
#if !defined(WIN32)
    int on;

    if (gBgp4.rtsock)
    {
        return TRUE;
    }

    gBgp4.rtsock = socket (AF_ROUTE, SOCK_RAW, 0);
    if (gBgp4.rtsock == VOS_ERR )
    {
        gBgp4.rtsock = 0 ;
        return (VOS_ERR);
    }

    on = 1;
    bgp4_set_sock_noblock(gBgp4.rtsock,on);
    bgp4_set_sock_tcpnodelay(gBgp4.rtsock,on);

    on = 0;
    setsockopt (gBgp4.rtsock, SOL_SOCKET, SO_USELOOPBACK, (char *)&on, sizeof (on)) ;

    on = BGP4_RTSOCK_BUFFER_LEN;
    bgp4_set_sock_rxbuf(gBgp4.rtsock,on);
    bgp4_set_sock_txbuf(gBgp4.rtsock,on);
#endif
    return TRUE;

}

int bgp4_close_rtsock()
{
#if !defined(WIN32)
    close(gBgp4.rtsock) ;
    gBgp4.rtsock = 0 ;
#endif
    return TRUE;
}

#define ROUNDUP(a) \
        ((a) > 0 ? (1 + (((a) - 1) | (sizeof (long) - 1))) : sizeof (long))

#define ADVANCE(x, n) (x += ROUNDUP ((n)->sa_len))

#if !defined(USE_LINUX) && !defined(WIN32)
void bgp4_parse_routemsg(u_char *pbuf)
{
    struct rt_newmsghdr  *p_rtmsg = (struct rt_newmsghdr  *)pbuf;
    struct rt4_newmsg *p_newroute = NULL;
    struct rt6_newmsg *p_newroute6= NULL ;

    tBGP4_ROUTE route;
    tBGP4_PATH path;
    u_int family=0;
    u_int i=0;
    u_int vrf_id = 0;

    u_char*p_msg=pbuf;

    if(p_rtmsg->family == AF_INET)
        family=BGP4_PF_IPUCAST;
    else if(p_rtmsg->family == AF_INET6)
        family=BGP4_PF_IP6UCAST;

    vrf_id = p_rtmsg->vrfid;

    memset(&route, 0, sizeof(route));

    p_msg+=sizeof(struct rt_newmsghdr);
    for (i =0; i<p_rtmsg->count; i++)
    {
        memset(&route,0,sizeof(tBGP4_ROUTE));
        memset(&path,0,sizeof(tBGP4_PATH));
        bgp4_lstnodeinit(&path.node);
        bgp4_lstinit(&path.aspath_list);
        bgp4_lstinit(&path.route_list);
        route.p_path = &path;

        if(family==BGP4_PF_IPUCAST)
        {
            p_newroute=(struct rt4_newmsg*)p_msg;
            route.proto =p_rtmsg->proto;
            if(route.proto==M2_ipRouteProto_bgp)
                continue;

            route.p_path = &path;
            route.dest.afi = family;

            memcpy(route.dest.ip,&p_newroute->dest,4);
            route.dest.prefixlen=bgp4_mask2len(p_newroute->mask);
            path.nexthop.afi=BGP4_PF_IPUCAST;
            memcpy(path.nexthop.ip, &p_newroute->nexthop, 4);
            path.nexthop.prefixlen = 32;
            path.origin = BGP4_ORIGIN_IGP;
            path.out_med=p_newroute->cost;

            path.afi = BGP4_AF_IP;
            path.safi = BGP4_SAF_UCAST;

            if (route.proto == M2_ipRouteProto_local)
            {
                memset(path.nexthop.ip ,0,4);
            }

            bgp4_rtsock_route_change(vrf_id,&route, (RTM_ADD == p_newroute->action) ? TRUE : FALSE);
            p_msg+=sizeof(struct rt4_newmsg);
        }
        else if(family==BGP4_PF_IP6UCAST)
        {
            p_newroute6=(struct rt6_newmsg*)p_msg;
            route.proto =p_rtmsg->proto;
            if(route.proto==M2_ipRouteProto_bgp)
                continue;

            route.p_path = &path;
            route.dest.afi = family;

            memcpy(route.dest.ip,p_newroute6->dest,16);
            route.dest.prefixlen= p_newroute6->prefixlen;
            path.nexthop_global.afi = BGP4_PF_IP6UCAST;
            memcpy(path.nexthop_global.ip, &p_newroute6->nexthop, 16);
            path.nexthop_global.prefixlen = 128;
            path.origin = BGP4_ORIGIN_IGP;
            path.out_med=p_newroute6->cost;

            path.afi = BGP4_AF_IP6;
            path.safi = BGP4_SAF_UCAST;

            if (route.proto == M2_ipRouteProto_local)
            {
                memset(path.nexthop_global.ip ,0,16);
            }
            bgp4_rtsock_route_change(vrf_id,&route, (RTM_ADD == p_newroute6->action) ? TRUE : FALSE);
            p_msg+=sizeof(struct rt6_newmsg);
        }

    }
    return;
}
#endif

/*receive message from routing socket,and deliver to special
   callback functions,just process one message in this function*/
STATUS bgp4_rtsock_recv()
{
#if !defined(WIN32)
    struct sockaddr *sa[RTAX_MAX] = {NULL};
    struct sockaddr *s;
    signed int read_byte;
    u_char buf[1024];
    struct rt_msghdr *p_msg = NULL;
    tBGP4_ROUTE route;
    tBGP4_PATH path;
    caddr_t cp ;
    int i=0;
    int number=0;
    int cmd;
    u_int vrf_id = 0;

    int ret;
    fd_set rfdset;
    struct timeval wait_time;
    while(number<255)
    {
        wait_time.tv_sec=0;
        wait_time.tv_usec=0;
        number++;
        FD_ZERO(&rfdset);
        FD_SET(gBgp4.rtsock,&rfdset);
        ret=select(gBgp4.rtsock+1,&rfdset,NULL,NULL,&wait_time);
        if(ret<=0)
        {
            break;
        }

        if(!FD_ISSET(gBgp4.rtsock,&rfdset))
        {
            continue;
        }

        read_byte = recv (gBgp4.rtsock, (char*)buf, sizeof(buf), 0);

        if ( read_byte <= 0 )
        {
            return VOS_ERR;
        }
#ifndef USE_LINUX
        p_msg = (struct rt_msghdr *)buf;

        bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock msg %d", p_msg->rtm_type);

        if (p_msg->rtm_version != RTM_VERSION )
        {
            break;
        }                                

        /* extract and process the command from the routing socket message */
        cmd = p_msg->rtm_type;
        switch( cmd )
        {
            default:
            case RTM_LOSING:   /* PCB cache discard */
            case RTM_MISS:     /* Failed search */
            case RTM_LOCK:     /* Changes to some metrics disabled */
            case RTM_OLDADD:   /* Unused message for SIOCADDRT command */
            case RTM_OLDDEL:   /* Unused message for SIOCDELRT command */
            case RTM_RESOLVE:  /* Creating entry for ARP information */
            case RTM_ADDEXTRA:  /* Report creation of duplicate route */
            case RTM_DELEXTRA:  /* Report deletion of duplicate route */
            case RTM_CHANGE:   /* Change Metrics or flags */
            case RTM_NEWCHANGE:  /* Change gateway of duplicate route */
            case RTM_NEWIPROUTE:
            case RTM_OLDIPROUTE:
            case RTM_REDIRECT:
                /*ignored*/
                break;

            case RTM_NEWADDR:  /* Interface is getting a new address */
            case RTM_DELADDR:  /* Address removed from interface */
            {
                tBGP4_ADDR peer_if_addr ;
                u_char addStr[64] = {0};
                struct ifa_msghdr *p_ifamsg = (struct ifa_msghdr *)buf;
                u_int ifunit = p_ifamsg->ifam_ifunit;

                memset(&peer_if_addr,0,sizeof(tBGP4_ADDR));

                /*extract route from message*/
                        cp = (caddr_t)(p_ifamsg + 1);

                        for (i = 0;  (i < RTAX_MAX); i++)
                        {
                            if ((p_ifamsg->ifam_addrs & (1 << i)) == 0)
                            {
                                    continue;
                            }

                            sa[i] = s = (struct sockaddr *)cp;

                            if (s->sa_len == 0)
                            {
                                    sa[i] = NULL;
                            }
                            ADVANCE (cp, s);
                        }

                if ( sa[RTAX_IFA]->sa_family == AF_INET )
                        {

                    if (NULL != sa[RTAX_IFA])
                        {
                            *(u_int*)(peer_if_addr.ip) = ntohl( ((struct sockaddr_in *)sa[RTAX_IFA])->sin_addr.s_addr);
                        peer_if_addr.afi = BGP4_PF_IPUCAST;
                        peer_if_addr.prefixlen = 32;
                        bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock RTM_NEWADDR(RTM_DELADDR) ,ip address %s ",
                                        bgp4_printf_addr(&peer_if_addr,addStr));
                        }
                }


#ifdef BGP_IPV6_WANTED

                else if( sa[RTAX_IFA]->sa_family == AF_INET6 )
                {
                    if (NULL != sa[RTAX_IFA])
                        {
                            struct sockaddr_in6 * sa6;
                        sa6 = (struct sockaddr_in6 *)sa[RTAX_IFA];
                        memcpy(peer_if_addr.ip,&sa6->sin6_addr,16);
                        peer_if_addr.afi = BGP4_PF_IP6UCAST;
                        peer_if_addr.prefixlen = 128;
                        bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock RTM_NEWADDR(RTM_DELADDR) ,ip address %s ",
                                        bgp4_printf_addr(&peer_if_addr,addStr));

                        }
                }
#endif
                else
                        {
                                break;
                        }
                /*when ip address is deleted,kernal will del all routes from the interface,*/
                /*so we have to stay the same with it  */
                bgp4_peer_if_addr_change(&peer_if_addr,ifunit);

                break;
            }
            case RTM_ADD:
            case RTM_DELETE:
            {
                bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock RTM_ADD(DEL) ,msg type %d ",cmd);
                gBgp4.nexthop_lookup_need = 1;

                if (p_msg->rtm_errno != 0 )
                {
                    break;
                }

                if ( (p_msg->rtm_flags & RTF_BLACKHOLE) && (cmd == RTM_ADD) )
                {
                    break;
                }

                /*extract route from message*/
                cp = (caddr_t)(p_msg + 1);

                for (i = 0;  (i < RTAX_MAX); i++)
                {
                    if ((p_msg->rtm_addrs & (1 << i)) == 0)
                    {
                        continue;
                    }

                    sa[i] = s = (struct sockaddr *)cp;
                    if (s->sa_len == 0)
                    {
                        sa[i] = NULL;
                    }
                    ADVANCE (cp, s);
                }
                /* if no destination address, reject the message */
                if (sa[RTAX_DST] == NULL )
                {
                    break;
                }

                vrf_id = p_msg->rtm_vpnvrfid;

                /* if destination address doesn't belong to the Internet family, reject
                * the message too
                */
                if ( sa[RTAX_DST]->sa_family == AF_INET )
                {
                    memset(&route, 0, sizeof(route));

                    memset(&path, 0, sizeof(path));
                    bgp4_lstnodeinit(&path.node);
                    bgp4_lstinit(&path.aspath_list);
                    bgp4_lstinit(&path.route_list);
                    route.p_path = &path;

                    //route.proto = RT_PROTO_GET(sa[RTAX_DST]);/*TODO:待确认*/
                    if(route.proto==M2_ipRouteProto_bgp)
                    {
                        break;
                    }

                    route.dest.afi = BGP4_PF_IPUCAST;
                    *(u_int*)(route.dest.ip) = ntohl( ((struct sockaddr_in *)sa[RTAX_DST])->sin_addr.s_addr);

                    if (sa[RTAX_NETMASK])
                    {
                        route.dest.prefixlen = bgp4_mask2len(ntohl( ((struct sockaddr_in *)sa[RTAX_NETMASK])->sin_addr.s_addr));
                    }
                    else/*for host route esp.*/
                    {
                        route.dest.prefixlen = 32;
                    }

                    if (sa[RTAX_GATEWAY])
                    {
                        path.nexthop.afi=BGP4_PF_IPUCAST;
                        path.nexthop.prefixlen = 32;
                        *(u_int*)(path.nexthop.ip) = ntohl( ((struct sockaddr_in *)sa[RTAX_GATEWAY])->sin_addr.s_addr);
                    }

                    path.origin = BGP4_ORIGIN_IGP;
                    path.afi = BGP4_AF_IP;
                    path.safi = BGP4_SAF_UCAST;

                    path.out_med = p_msg->rtm_rmx.value1;

                    /*local route has no nexthop*/
                    if (route.proto == 2)
                    {
                        *(u_int*)path.nexthop.ip = 0 ;
                    }
                }
#ifdef BGP_IPV6_WANTED
                else if( sa[RTAX_DST]->sa_family == AF_INET6 )
                {
                    struct sockaddr_in6 *sa6;
                    memset(&route, 0, sizeof(route));

                    memset(&path, 0, sizeof(path));
                    bgp4_lstnodeinit(&path.node);
                    bgp4_lstinit(&path.aspath_list);
                    bgp4_lstinit(&path.route_list);
                    route.p_path = &path;

                    route.proto = RT_PROTO_GET(sa[RTAX_DST]);
                    if(route.proto==M2_ipRouteProto_bgp)
                    {
                        break;
                    }

                    sa6 = (struct sockaddr_in6 *)sa[RTAX_DST];
                    route.p_path = &path;
                    route.dest.afi = BGP4_PF_IP6UCAST;
                    path.afi = BGP4_AF_IP6;
                    path.safi = BGP4_SAF_UCAST;


                    memcpy(route.dest.ip,&sa6->sin6_addr,16);

                    if (sa[RTAX_NETMASK])
                    {
                        sa6 = (struct sockaddr_in6 *)sa[RTAX_NETMASK];
                        route.dest.prefixlen= bgp4_ip6_mask2len(&sa6->sin6_addr);
                    }
                    else/*for host route esp.*/
                    {
                        route.dest.prefixlen= 128;
                    }

                    if (sa[RTAX_GATEWAY])
                    {
                        path.nexthop_global.afi=BGP4_PF_IP6UCAST;
                        path.nexthop_global.prefixlen = 128;
                        sa6 = (struct sockaddr_in6 *)sa[RTAX_GATEWAY];
                        memcpy(path.nexthop_global.ip, &sa6->sin6_addr, sizeof(sa6->sin6_addr));
                    }
                    path.origin = BGP4_ORIGIN_IGP; 
                    path.out_med=p_msg->rtm_rmx.value1;

                    if (route.proto == 2)
                    {
                        memset(path.nexthop.ip ,0,16);
                    }
                }
#endif
                else
                {
                    break;
                }

                bgp4_rtsock_route_change(vrf_id,&route, (RTM_ADD == cmd) ? TRUE : FALSE);
                break;
            }
#if 0
            case RTM_MPLS_EGR_VPN_ADD:
            case RTM_MPLS_ING_VPN_ADD:

            case RTM_MPLS_EGR_VPN_DEL:
            case RTM_MPLS_ING_VPN_DEL:

            case RTM_VPN_ROUTE_ADD:
            case RTM_VPN_ROUTE_DEL:
            {
                struct vpnRt_msghdr* p_vpn_msg = NULL;
                tRtMsgVpn* p_vpn_route_info = NULL;

                if(cmd == RTM_MPLS_EGR_VPN_ADD || cmd == RTM_MPLS_ING_VPN_ADD)
                {
                    cmd1 = RTM_VPN_ROUTE_ADD;
                }

                if(cmd == RTM_MPLS_EGR_VPN_DEL || cmd == RTM_MPLS_ING_VPN_DEL)
                {
                    cmd1 = RTM_VPN_ROUTE_DEL;
                }

                bgp4_log(1,1,"receive rt sock %s",(cmd1 == RTM_VPN_ROUTE_ADD) ? "RTM_VPN_ROUTE_ADD" :"RTM_VPN_ROUTE_DEL");


                p_vpn_msg = (struct vpnRt_msghdr*)buf;
                /*vpn info*/
                p_vpn_route_info = &p_vpn_msg->rtm_vpn;
                bgp4_log(1,1,"vpn %s ,route direction %d,rt num %d,rd %s,label %d,rt %s\r\npeer ip %x,peer type %d",
                        p_vpn_route_info->vpn_name+1,
                        p_vpn_route_info->route_direction,
                        p_vpn_route_info->rt_num,p_vpn_route_info->vpn_rd,p_vpn_route_info->mpls_label,
                        p_vpn_route_info->vpn_rt,
                        bgp_ip4(p_vpn_route_info->peer_ip),
                        p_vpn_route_info->peer_type);

                /*extract route from message*/
                cp = (caddr_t)(p_vpn_msg + 1);

                for (i = 0;  (i < RTAX_MAX); i++)
                {
                    if ((p_vpn_msg->rtm_addrs & (1 << i)) == 0)
                    {
                        continue;
                    }

                    sa[i] = s = (struct sockaddr *)cp;
                    if (s->sa_len == 0)
                    {
                        sa[i] = NULL;
                    }
                    ADVANCE (cp, s);
                }

                /* if no destination address, reject the message */
                if (sa[RTAX_DST] == NULL )
                {
                    bgp4_log(BGP_DEBUG_EVT,1,"RTM_VPN_ROUTE msg with no dest,error");
                    break;
                }

                bgp4_log(BGP_DEBUG_EVT,1,"RTM_VPN_ROUTE msg  afi %d(2:AF_INET,28:AF_INET6)",sa[RTAX_DST]->sa_family);

                /* if destination address doesn't belong to the Internet family, reject
                * the message too
                */
                if ( sa[RTAX_DST]->sa_family == AF_INET )
                {
                    u_char buff[64] = {0};
                    memset(&route, 0, sizeof(route));
                    memset(&path, 0, sizeof(path));
                    bgp4_lstnodeinit(&path.node);
                    bgp4_lstinit(&path.aspath_list);
                    bgp4_lstinit(&path.route_list);
                    route.p_path = &path;

                    route.proto = RT_PROTO_GET(sa[RTAX_DST]);

                    route.dest.afi = BGP4_PF_IPUCAST;
                    *(u_int*)(route.dest.ip) = ntohl( ((struct sockaddr_in *)sa[RTAX_DST])->sin_addr.s_addr);

                    if (sa[RTAX_NETMASK])
                    {
                        route.dest.prefixlen = bgp4_mask2len(ntohl( ((struct sockaddr_in *)sa[RTAX_NETMASK])->sin_addr.s_addr));
                        bgp4_log(1,1,"[bgp4_rtsock_recv]:received a prefix len %d",route.dest.prefixlen);

                    }

                    if (sa[RTAX_GATEWAY])
                    {
                        path.nexthop.afi=BGP4_PF_IPUCAST;
                        *(u_int*)(path.nexthop.ip) = ntohl( ((struct sockaddr_in *)sa[RTAX_GATEWAY])->sin_addr.s_addr);
                        bgp4_log(1,1,"[bgp4_rtsock_recv]:received a nexthop %d.%d.%d.%d",path.nexthop.ip[0],
                        path.nexthop.ip[1],path.nexthop.ip[2],path.nexthop.ip[3]);

                    }

                    path.origin = BGP4_ORIGIN_IGP; 
                    path.out_med=p_vpn_msg->rtm_metric;

                    /*local route has no nexthop*/
                    if (route.proto == 2)
                    {
                        *(u_int*)path.nexthop.ip = 0 ;
                    }
                    bgp4_log(1,1,"[bgp4_rtsock_recv]:received a ipv4 vpn route msg,route %s,origin protocol %d",bgp4_printf_route(&route,buff),route.proto);
                }
#ifdef BGP_IPV6_WANTED
                else if ( sa[RTAX_DST]->sa_family == AF_INET6)
                {
                    struct sockaddr_in6 *sa6;
                    u_char addr[64] = {0};
                    memset(&route, 0, sizeof(route));
                    memset(&path, 0, sizeof(path));
                    bgp4_lstnodeinit(&path.node);
                    bgp4_lstinit(&path.aspath_list);
                    bgp4_lstinit(&path.route_list);
                    route.p_path = &path;

                    bgp4_log(1,1,"[bgp4_rtsock_recv]:received a ipv6 vpn route msg,route");

                    route.proto = RT_PROTO_GET(sa[RTAX_DST]);

                    sa6 = (struct sockaddr_in6 *)sa[RTAX_DST];
                    route.p_path = &path;
                    route.dest.afi = BGP4_PF_IP6UCAST;

                    memcpy(route.dest.ip,&sa6->sin6_addr,16);

                    if (sa[RTAX_NETMASK])
                    {
                        sa6 = (struct sockaddr_in6 *)sa[RTAX_NETMASK];
                        route.dest.prefixlen= bgp4_ip6_mask2len(&sa6->sin6_addr);
                    }

                    if (sa[RTAX_GATEWAY])
                    {
                        path.nexthop.afi=BGP4_PF_IP6UCAST;
                        sa6 = (struct sockaddr_in6 *)sa[RTAX_GATEWAY];
                        memcpy(path.nexthop_global.ip, &sa6->sin6_addr, sizeof(sa6->sin6_addr));
                    }
                    path.origin = BGP4_ORIGIN_IGP; 
                    path.out_med=p_vpn_msg->rtm_metric;

                    if (route.proto == 2)
                    {
                        memset(path.nexthop.ip ,0,16);
                    }
                    bgp4_log(BGP_DEBUG_EVT,1,"[bgp4_rtsock_recv]:received a ipv6 vpn route msg,route %s",
                    bgp4_printf_route(&route,addr));
                }
        #endif
                else
                {
                    printf("\r\n[bgp4_rtsock_recv]sa[RTAX_DST]->sa_family VOS_ERR!");
                    break;

                }
                bgp4_vpn_route_change(p_vpn_route_info,&route,(RTM_VPN_ROUTE_ADD == cmd1) ? TRUE : FALSE);

                break;
            }

#endif

            case RTM_VPN_ADD:/*for vpn add,should send route refresh to PE peers*/
            {
                u_int count = 0;
                u_char all_zero[BGP4_VPN_RD_LEN] = {0,0,0,0,0,0,0,0};
                u_int i = 0;
                u_int vpnVrfid;
                tBGP4_VPN_INSTANCE* p_add_instance =NULL;
                struct vpn_msghdr* p_vpn_msg = NULL;

                p_vpn_msg= (struct vpn_msghdr*)buf;

                if(p_vpn_msg->vpn_state != 0)
                {
                    bgp4_log(BGP_DEBUG_EVT,1,"conflict rt msg action with type");
                    break;
                }

                count = p_vpn_msg->cnt;

                if(count>0)
                {
                    for(i = 0;i<count;i++)
                    {
                        vpnVrfid = p_vpn_msg->l3vpnMsg[i].vpn_vrid;
                        p_add_instance = bgp4_mplsvpn_add(vpnVrfid);
                        if(p_add_instance)
                        {
                            memcpy(p_add_instance->rd,p_vpn_msg->l3vpnMsg[i].rd,BGP4_VPN_RD_LEN);
                            bgp4_log(BGP_DEBUG_MPLS_VPN,1,"get rd %c.%c.%c.%c.%c.%c.%c.%c",
                                    p_add_instance->rd[0],p_add_instance->rd[1],p_add_instance->rd[2],
                                    p_add_instance->rd[3],p_add_instance->rd[4],p_add_instance->rd[5],
                                    p_add_instance->rd[6],p_add_instance->rd[7]);
                        }
                        bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock RTM_VPN_ADD vrf id %d",vpnVrfid);
                    }
                }

                break;
            }
            case RTM_VPN_DEL:/*for vpn delete,should delete peers belong to such vpn instance*/
            {
                u_int count = 0;
                u_int i = 0;
                u_int vpnVrfid = 0;
                struct vpn_msghdr* p_vpn_msg = NULL;
                tBGP4_VPN_INSTANCE* p_instance = NULL;

                bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock RTM_VPN_DEL ");
                p_vpn_msg= (struct vpn_msghdr*)buf;
                if(p_vpn_msg->vpn_state != 1)
                {
                    bgp4_log(BGP_DEBUG_EVT,1,"conflict rt msg action with type");
                    break;
                }

                count = p_vpn_msg->cnt;

                if(count>0)
                {
                    for(i = 0;i<count;i++)
                    {   
                        vpnVrfid = p_vpn_msg->l3vpnMsg[i].vpn_vrid;
                        p_instance = bgp4_vpn_instance_lookup(vpnVrfid);
                        if(p_instance)
                        {
                            bgp4_log(1,1,"receive rt sock RTM_VPN_DEL,delete instance %d ",vpnVrfid);
                            bgp4_mplsvpn_delete(p_instance);
                        }
                    }               
                }
                break;
            }
            case RTM_VPN_RT_CHANGE:
            {
                struct rt_newmsghdr* p_vpn_msg = NULL;
                tBGP4_VPN_INSTANCE* p_instance = NULL;

                p_vpn_msg= (struct rt_newmsghdr*)buf;

                bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock instance %d RTM_VPN_RT_CHANGE ,type %d",
                            p_vpn_msg->vrfid,p_vpn_msg->reserved);

                p_instance = bgp4_vpn_instance_lookup(p_vpn_msg->vrfid);
                if(p_instance)
                {
                    bgp4_mplsvpn_change(p_instance,p_vpn_msg->reserved);
                }

                break;

            }

            case RTM_TCPSYNC_ADD:
            {
                struct tcpsync_msghdr* p_tcpsync_msg = (struct tcpsync_msghdr*)buf;
                tBGP4_VPN_INSTANCE* p_instance = NULL;

                bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock RTM_TCPSYNC_ADD,vrf id %d,new fd %d,local port %d,remote port %d",
                            p_tcpsync_msg->tcp_vrid,
                            p_tcpsync_msg->tcp_fd,
                            p_tcpsync_msg->tcp_lport,
                            p_tcpsync_msg->tcp_fport);
                gBgp4.add_tcp_msg_count++;

                p_instance = bgp4_vpn_instance_lookup(p_tcpsync_msg->tcp_vrid);
                if(p_instance == NULL)
                {
                    bgp4_log(BGP_DEBUG_EVT,1,"RTM_TCPSYNC_ADD,can not find vrf id %d instance", p_tcpsync_msg->tcp_vrid);
                    break;
                }

                if(p_tcpsync_msg->tcp_fport == gBgp4.client_port||
                    p_tcpsync_msg->tcp_lport == gBgp4.server_port)
                {
                    bgp4_tcp_sync_add(p_instance,
                                    p_tcpsync_msg->tcp_family,
                                    p_tcpsync_msg->tcp_laddr,
                                    p_tcpsync_msg->tcp_faddr,
                                    p_tcpsync_msg->tcp_lport,
                                    p_tcpsync_msg->tcp_fport,
                                    p_tcpsync_msg->tcp_fd);

                }
                break;
            }
            case RTM_TCPSYNC_DEL:
            {
                struct tcpsync_msghdr* p_tcpsync_msg = (struct tcpsync_msghdr*)buf;
                tBGP4_VPN_INSTANCE* p_instance = NULL;

                bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock RTM_TCPSYNC_DEL,old fd %d,local port %d,remote port %d",
                            p_tcpsync_msg->tcp_fd,
                            p_tcpsync_msg->tcp_lport,
                            p_tcpsync_msg->tcp_fport);

                gBgp4.del_tcp_msg_count++;
                p_instance = bgp4_vpn_instance_lookup(p_tcpsync_msg->tcp_vrid);
                if(p_instance == NULL)
                {
                    bgp4_log(BGP_DEBUG_EVT,1,"RTM_TCPSYNC_ADD,can not find vrf id %d instance", p_tcpsync_msg->tcp_vrid);
                    break;
                }

                if(p_tcpsync_msg->tcp_fport == gBgp4.client_port ||
                    p_tcpsync_msg->tcp_lport == gBgp4.server_port)
                {
                    bgp4_tcp_sync_del(p_instance,
                                    p_tcpsync_msg->tcp_family,
                                    p_tcpsync_msg->tcp_laddr,
                                    p_tcpsync_msg->tcp_faddr,
                                    p_tcpsync_msg->tcp_lport,
                                    p_tcpsync_msg->tcp_fport,
                                    p_tcpsync_msg->tcp_fd);

                }

                break;
            }
            case RTM_SUBSYSTEM_UP:
            {
                bgp4_subsys_up_sync((void *)buf);
                    break;
            }
            case RTM_SLOT_UP:
            {
                bgp4_slot_up_sync((void *)buf);
                    break;
            }
            case RTM_CARD_UP:
            {
                bgp4_card_up_sync((void *)buf);
                break;
            }
            case RTM_CARD_DOWN:
            {
                /*do nothing*/
                break;
            }


            case RTM_CARD_ROLECHANGE :
            {
                bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock RTM_CARD_ROLECHANGE");

                bgp4_card_role_change();

                break;
            }

            case RTM_NEW_RTMSG :
            {
                bgp4_log(BGP_DEBUG_EVT,1,"receive rt sock RTM_NEW_RTMSG");
                gBgp4.nexthop_lookup_need = 1;
                bgp4_parse_routemsg(buf);
                break;
            }

            case RTM_BFD_SESSION:
            {
                tBGP4_PEER * p_peer = NULL;
                tBGP4_VPN_INSTANCE* p_instance = NULL;

                struct bfd_msghdr * bfdmsg = (struct bfd_msghdr *) buf ;

                bgp4_log(BGP_DEBUG_EVT, 1 , "receive bfd rt sock, bfd diag%d,state %d ", bfdmsg->diag , bfdmsg->state);

                    if (BFD_DOWN != bfdmsg->state)
                    {
                        break ;
                    }

                LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
                {
                    LST_LOOP (&p_instance->peer_list,p_peer, node, tBGP4_PEER)
                    {
                        if(p_peer->bfd_discribe == bfdmsg->discr)
                        {
                            if((BFD_ADMINISTRATIVELY_DOWN == bfdmsg->diag)
                                    ||(BFD_NEIGHBOR_SIGNALED_SESSION_DOWN == bfdmsg->diag))
                            {
                                bgp4_unbind_bfd(p_peer);
                            }
                            else if(p_peer->state == BGP4_NS_ESTABLISH)
                            {
                                bgp4_fsm(p_peer,BGP4_EVENT_TCP_CLOSED);
                            }
                        }
                    }
                }
                        break;
            }

            case RTM_BFD_IFSTATUS:
            {
                tBGP4_PEER * p_peer = NULL;
                tBGP4_VPN_INSTANCE* p_instance = NULL;
                u_int ifunit = 0;
                struct bfd_msghdr * bfdmsg = (struct bfd_msghdr *) buf ;

                bgp4_log(BGP_DEBUG_EVT, 1 , "receive bfd rt sock, bfd interface unit %d,state %d ", bfdmsg->discr, bfdmsg->state);

                ifunit = bfdmsg->discr;             
                
                if(BFD_UP!=bfdmsg->state)
                {
                    break;
                }
                
                LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
                {
                    LST_LOOP (&p_instance->peer_list,p_peer, node, tBGP4_PEER)
                    {
                        if(VOS_ERR != p_peer->bfd_discribe)
                        {
                            continue;
                        }
                    
                        if((ifunit == p_peer->if_unit)&&(p_peer->state == BGP4_NS_ESTABLISH))
                        {               
                            bgp4_bind_bfd(p_peer);
                        }
                    }
                }
                break;
            }

            case RTM_IFINFO:
                bgp4_log(BGP_DEBUG_EVT,1,"receive new rt sock RTM_IFINFO");
                break;
        }
#endif

    }
#endif
    return VOS_OK;
}

/*update system ip route.only consider ipv4 here*/
int bgp4_add_sysroute(u_int vrf_id,tBGP4_ROUTE *p_route)
{
    int  flag = 0x100;
    u_int dest = 0,mask = 0,nexthop = 0;
    u_int cost_value = 100;/*default 100*/

    if(p_route->p_path->out_med)
    {
        cost_value = p_route->p_path->out_med;
    }

    if(p_route->route_direction == MPLSL3VPN_ROUTE_REMOTE ||
        p_route->route_direction == MPLS_6PE_REMOTE_ROUTE)
    {
        flag|=0x80000000;
    }

    if(p_route->p_path->direct_nexthop_ifunit >= 0)
        {
            flag|=0x80000000;/*for non-direct-nexthop*/
        }


    if (p_route->dest.afi == BGP4_PF_IPUCAST)
    {
        dest = *(u_int*)p_route->dest.ip;
        mask = bgp4_len2mask(p_route->dest.prefixlen);

        nexthop = *(u_int*)p_route->p_path->nexthop.ip;

        {
            u_char addrstr[64];
            u_char nhopstr[64];
            bgp4_log(BGP_DEBUG_RT,1,"add route %s to ip route table vrf %d,nhop %s,flag %#x,med %d",
                        bgp4_printf_route(p_route,addrstr),
                        vrf_id,
                        bgp4_printf_addr(&p_route->p_path->nexthop,nhopstr),
                        flag,
                        cost_value);
        }

        gBgp4.stat.iprtadd++;
        if (systemIPv4RouteAdd2(gBgp4.rtsock, vrf_id,M2_ipRouteProto_bgp, dest, nexthop, mask,flag,cost_value) != VOS_OK)
        {
            int error = errnoGet();
            gBgp4.stat.iprtadderror = error;
                    gBgp4.stat.iprtfail++;
                bgp4_log(BGP_DEBUG_RT,1,"add route from ip route table failed ,error no %d",error);
            if (error == ERTMSGNOBUF||error==ENOBUFS)
            {
                bgp4_rtsock_recv() ;
                return FALSE;
            }
            /*no rtmsg send*/
            p_route->rtmsg_finish = TRUE;
            }
        }
#ifdef BGP_IPV6_WANTED
    else if(p_route->dest.afi == BGP4_PF_IP6UCAST)
    {
        mask=p_route->dest.prefixlen;
        {
            u_char addrstr[64];
            u_char nhopstr[64];
            bgp4_log(BGP_DEBUG_RT,1,"add route %s to ip route table vrf %d,nexthop %s,flag %#x,med%d",
                        bgp4_printf_route(p_route,addrstr),
                        vrf_id,
                        bgp4_printf_addr(&p_route->p_path->nexthop_global,nhopstr),
                        flag,
                        cost_value);
        }

        gBgp4.stat.ip6rtadd++;
        if(systemIPv6RouteAdd2 (gBgp4.rtsock, vrf_id,M2_ipRouteProto_bgp,p_route->dest.ip,p_route->p_path->nexthop_global.ip,mask,flag,cost_value)!= VOS_OK)
        {
            int error = errnoGet();
            gBgp4.stat.ip6rtfail++;
            gBgp4.stat.ip6rtadderror = error;

            bgp4_log(BGP_DEBUG_RT,1,"add route from ip route table failed ,error no %d",error);
            
            if (error == ERTMSGNOBUF||error==ENOBUFS)
            {
                bgp4_rtsock_recv() ;
                return FALSE;
            }
            /*no rtmsg send*/
            p_route->rtmsg_finish = TRUE;
        }
    }
#endif

    return TRUE;
}

int bgp4_del_sysroute(u_int vrf_id,tBGP4_ROUTE *p_route)
{
    int flag = 0x100;
    u_int dest = 0,mask = 0,nexthop = 0;
    u_int cost_value = 100;

    if(p_route->p_path->out_med)
    {
        cost_value = p_route->p_path->out_med;
    }

    if(p_route->route_direction == MPLSL3VPN_ROUTE_REMOTE ||
        p_route->route_direction == MPLS_6PE_REMOTE_ROUTE)
    {
        flag|=0x80000000;
    }

    if(p_route->p_path->direct_nexthop_ifunit >= 0)
        {
            flag|=0x80000000;/*for non-direct-nexthop*/
        }

    if (p_route->dest.afi == BGP4_PF_IPUCAST)
    {
        dest = *(u_int*)p_route->dest.ip;
        mask = bgp4_len2mask(p_route->dest.prefixlen);

        nexthop = *(u_int*)p_route->p_path->nexthop.ip;

        {
            u_char addrstr[64];
            u_char nhopstr[64];

            bgp4_log(BGP_DEBUG_RT,1,"delete route %s from ip route table vrf %d,nexthop %s,flag %#x,med %d",
                        bgp4_printf_route(p_route,addrstr),
                        vrf_id,
                        bgp4_printf_addr(&p_route->p_path->nexthop,nhopstr),
                        flag,
                        cost_value);
        }

        gBgp4.stat.iprtdel++;
        if (systemIPv4RouteDelete2(gBgp4.rtsock, vrf_id,M2_ipRouteProto_bgp, dest, nexthop, mask, flag, cost_value) != VOS_OK)
        {
            int error = errnoGet();
            gBgp4.stat.iprtdelerror = error;

            gBgp4.stat.iprtdelfail++;
            bgp4_log(BGP_DEBUG_RT,1,"delete route from ip route table failed ,error no %d",error);
            if (error == ERTMSGNOBUF||error==ENOBUFS)
            {
                bgp4_rtsock_recv() ;
                return FALSE;
            }
            /*no rtmsg send if error is not NO route*/
            if (error != ESRCH)
            {
                p_route->rtmsg_finish = TRUE;
            }
        }
    }
#ifdef BGP_IPV6_WANTED
    else if(p_route->dest.afi == BGP4_PF_IP6UCAST)
    {
        mask = p_route->dest.prefixlen;
        {
            u_char addrstr[64];
            u_char nhopstr[64];
            bgp4_log(BGP_DEBUG_RT,1,"delete route %s from ip route table vrf %d,nexthop %s,flag %#x,med %d",
                        bgp4_printf_route(p_route,addrstr),
                        vrf_id,
                        bgp4_printf_addr(&p_route->p_path->nexthop_global,nhopstr),
                        flag,
                        cost_value);
        }

        gBgp4.stat.ip6rtdel++;
        if(systemIPv6RouteDelete2 (gBgp4.rtsock, vrf_id,M2_ipRouteProto_bgp,p_route->dest.ip,p_route->p_path->nexthop_global.ip,mask,flag,cost_value)!= VOS_OK)
        {
            int error = errnoGet();
            gBgp4.stat.ip6rtdelerror = error;

            gBgp4.stat.ip6rtdelfail++;
            bgp4_log(BGP_DEBUG_RT,1,"delete route from ip route table failed ,error no %d",error);
            if (error == ERTMSGNOBUF||error==ENOBUFS)
            {
                bgp4_rtsock_recv() ;
                return FALSE;
            }
            /*no rtmsg send if error is not NO route*/
            if (error != ESRCH)
            {
                p_route->rtmsg_finish = TRUE;
            }
        }
    }
#endif

    return TRUE;
}

/*lookup in kernal route.tbi*/
int bgp4_sysroute_lookup(u_int vrf_id,tBGP4_ROUTE *p_route)
{
    M2_IPROUTETBL sys_route_ipv4 = {0};
    M2_IPV6ROUTETBL sys_route_ipv6 = {0};
    u_int mask = 0;
    u_char addstr[64];

    if(p_route == NULL)
    {
        return FALSE;
    }

    if(p_route->dest.afi == BGP4_PF_IPUCAST)
    {
        mask = bgp4_len2mask(p_route->dest.prefixlen);
        if(ipv4v6RouteLookup(vrf_id,AF_INET,p_route->dest.ip,mask,&sys_route_ipv4) == VOS_OK)
        {
            bgp4_log(BGP_DEBUG_RT,1,"get a sysroute %#x,mask %#x,proto type %d",
                sys_route_ipv4.ipRouteDest,
                sys_route_ipv4.ipRouteMask,
                sys_route_ipv4.ipRouteProto);
            return TRUE;
        }


    }
    else if(p_route->dest.afi == BGP4_PF_IP6UCAST)
    {
        mask = p_route->dest.prefixlen;
        if(ipv4v6RouteLookup(vrf_id,AF_INET6,p_route->dest.ip,mask,&sys_route_ipv6) == VOS_OK)
        {
            inet_ntop(AF_INET6,(sys_route_ipv6.ipv6RouteDest),addstr,64);

            bgp4_log(BGP_DEBUG_RT,1,"get a sysroute %s,prefix length %d,proto type %d",
                addstr,
                sys_route_ipv6.ipv6RoutePfxLength,
                sys_route_ipv6.ipv6RouteProtocol);

            return TRUE;
        }

    }

    return FALSE;
}

#if !defined(USE_LINUX) && !defined(WIN32)
u_char bgp4_hw_route=1;
#else
u_char bgp4_hw_route=0;
#endif

STATUS bgp4_send_routemsg(u_char*msg_buf)
{
    struct rt_newmsghdr  *p_rtmsg = (struct rt_newmsghdr  *)msg_buf;
    u_int  len;

    if (msg_buf==NULL)
    {
        return FALSE;
    }


    len = p_rtmsg->rtm_msglen;

    if (p_rtmsg->count)
    {
        p_rtmsg->rtm_msglen = htons(len);

        gBgp4.stat.ipmsgsend++;
        gBgp4.stat.ipmsglen+=len;

        if(bgp4_hw_route==1)
        {
            if (0 > send(gBgp4.rtsock, msg_buf, len, 0))
            {
                gBgp4.stat.ipmsgfail++;
                return FALSE;
            }

            bgp4_debug_row_buffer(msg_buf,len);
        }
        p_rtmsg->rtm_msglen = 0;
        p_rtmsg->count = 0;
    }

    return TRUE;
}

STATUS bgp4_add_routemsg(u_int vrf_id,u_char*msg_buf ,tBGP4_ROUTE *p_route,
                    tBGP4_ADDR* old_direct_nexthop,u_int old_if_unit)
{
    struct rt_newmsghdr  *p_rtmsg = (struct rt_newmsghdr  *)msg_buf;
    struct rt4_newmsg *p_newroute = NULL;
    u_int len=0;
    u_int nexthop = 0;
    u_int cost_value = 100;/*default 100*/

    if (p_route==NULL||msg_buf==NULL)
    {
        return FALSE;
    }

    len = p_rtmsg->rtm_msglen;
    if (RT_MSG_MAXLEN < (len + sizeof(struct rt4_newmsg)) ||
        p_rtmsg->vrfid != vrf_id)
    {
        if (TRUE != bgp4_send_routemsg(msg_buf))
        {
            gBgp4.stat.rt_msg_fail++;
            bgp4_log(BGP_DEBUG_RT,1,"bgp4 send route msg failed %d", errnoGet());
            return FALSE;
        }
    }

    if(p_route->p_path->out_med)
    {
        cost_value = p_route->p_path->out_med;
    }

    if (0 == p_rtmsg->rtm_msglen)
    {
        p_rtmsg->rtm_msglen = sizeof(struct rt_newmsghdr );
        p_rtmsg->rtm_version = RTM_VERSION;
        p_rtmsg->rtm_type = RTM_NEW_RTMSG;
        p_rtmsg->proto = M2_ipRouteProto_bgp;
        p_rtmsg->family = AF_INET;
        p_rtmsg->count = 0;
        p_rtmsg->vrfid = vrf_id;
    }
    p_newroute = (struct rt4_newmsg *)(msg_buf+ p_rtmsg->rtm_msglen);
    p_newroute->dest = *(u_long*)p_route->dest.ip;
    p_newroute->mask= bgp4_len2mask(p_route->dest.prefixlen);

    if(old_direct_nexthop)
    {
        nexthop = *(u_int*)old_direct_nexthop->ip;
        p_newroute->if_unit = old_if_unit;
        p_newroute->action = RTM_DELETE;
    }
    else
    {
        if (p_route->p_path->direct_nexthop_ifunit >= 0)
        {
            nexthop = *(u_int*)p_route->p_path->direct_nexthop.ip;
            p_newroute->if_unit =p_route->p_path->direct_nexthop_ifunit;
        }
        else
        {
            nexthop = *(u_int*)p_route->p_path->nexthop.ip;
            p_newroute->if_unit =p_route->p_path->p_peer->if_unit;
        }

        p_newroute->action = (p_route->ip_action == BGP4_IP_ACTION_ADD) ? RTM_ADD : RTM_DELETE;
    }


    p_newroute->nexthop = nexthop;

    p_newroute->cost = cost_value;

    if(p_route->route_direction == MPLSL3VPN_ROUTE_REMOTE)
    {
        p_newroute->flag = RTFLAG_VPN_REMOTE;
    }
    else
    {
        p_newroute->flag = 0;
    }

    p_rtmsg->count ++;
    p_rtmsg->rtm_msglen += sizeof(struct rt4_newmsg);

    if(p_route->ip_action == BGP4_IP_ACTION_ADD)
    {
        gBgp4.stat.rt_msg_rt_add++;
    }
    else if(p_route->ip_action==RTM_DELETE)
    {
        gBgp4.stat.rt_msg_rt_del++;
    }

    {
        u_char addstr[64] = {0};
        u_char nhopstr[64] = {0};
        bgp4_log(BGP_DEBUG_RT,1,"send HW route %s  to ip route table vrf %d,action %d(1:add,2:del),direct nexthop %s,interface unit %d",
                bgp4_printf_route(p_route,addstr),
                vrf_id,
                p_newroute->action,
                bgp4_printf_addr(&p_route->p_path->direct_nexthop,nhopstr),
                p_newroute->if_unit);
    }
    
        return TRUE;
}

STATUS bgp4_add_route6msg(u_int vrf_id,u_char*msg_buf6 ,tBGP4_ROUTE *p_route,
                            tBGP4_ADDR* old_direct_nexthop,u_int old_if_unit)
{
    struct rt_newmsghdr  *p_rtmsg = (struct rt_newmsghdr  *)msg_buf6;
    struct rt6_newmsg *p_newroute = NULL;
    u_int len = 0;
    u_int cost_value = 100;/*default 100*/

    if (p_route==NULL||msg_buf6==NULL)
    {
        return FALSE;
    }


    len = p_rtmsg->rtm_msglen;
    if (RT_MSG_MAXLEN < (len + sizeof(struct rt6_newmsg)) ||
        p_rtmsg->vrfid != vrf_id)
    {
        if (TRUE != bgp4_send_routemsg(msg_buf6))
        {
            bgp4_log(BGP_DEBUG_RT,1,"\r\nbgp4 send route6 msg failed ......");
            gBgp4.stat.rt6_msg_fail++;
            return FALSE;
        }
    }

    if(p_route->p_path->out_med)
    {
        cost_value = p_route->p_path->out_med;
    }



    if (0 == p_rtmsg->rtm_msglen)
    {
        p_rtmsg->rtm_msglen = sizeof(struct rt_newmsghdr );
        p_rtmsg->rtm_version = RTM_VERSION;
        p_rtmsg->rtm_type = RTM_NEW_RTMSG;
        p_rtmsg->proto = M2_ipRouteProto_bgp;
        p_rtmsg->family = AF_INET6;
        p_rtmsg->count = 0;
        p_rtmsg->vrfid = vrf_id;


    }

    p_newroute = (struct rt6_newmsg *)(msg_buf6+ p_rtmsg->rtm_msglen);
    memcpy(p_newroute->dest,p_route->dest.ip,16);

    p_newroute->prefixlen= p_route->dest.prefixlen;

    if(old_direct_nexthop)
    {
        memcpy(p_newroute->nexthop,old_direct_nexthop->ip,16);
        p_newroute->ifunit =old_if_unit;
        p_newroute->action = RTM_DELETE;
    }
    else
    {
        if (p_route->p_path->direct_nexthop_ifunit >= 0)
        {
            memcpy(p_newroute->nexthop,p_route->p_path->direct_nexthop.ip,16);
            p_newroute->ifunit =p_route->p_path->direct_nexthop_ifunit;
        }
        else
        {
            memcpy(p_newroute->nexthop,p_route->p_path->nexthop_global.ip,16);
            p_newroute->ifunit =p_route->p_path->p_peer->if_unit;
        }

        p_newroute->action = (p_route->ip_action == BGP4_IP_ACTION_ADD) ? RTM_ADD : RTM_DELETE;
    }

    p_newroute->cost = cost_value;

    if(p_route->route_direction == MPLSL3VPN_ROUTE_REMOTE)
    {
        p_newroute->flag = RTFLAG_VPN_REMOTE;
    }
    else
    {
        p_newroute->flag = 0;
    }

    p_rtmsg->count ++;
    p_rtmsg->rtm_msglen += sizeof(struct rt6_newmsg);

    if(p_route->ip_action == BGP4_IP_ACTION_ADD)
    {
        gBgp4.stat.rt6_msg_rt_add++;
    }
    else if(p_route->ip_action==RTM_DELETE)
    {
        gBgp4.stat.rt6_msg_rt_del++;
    }

    {
            u_char addstr[64] = {0};
            u_char nhopstr[64] = {0};
            bgp4_log(BGP_DEBUG_RT,1,"send HW route %s to ip route table vrf id %d,action %d(1:add,2:del),direct nexthop %s,interface unit %d",
                bgp4_printf_route(p_route,addstr),
                vrf_id,
                p_newroute->action,
                bgp4_printf_addr(&p_route->p_path->direct_nexthop,nhopstr),
                p_newroute->ifunit);
    }

    return TRUE;
}

#ifdef  BGP_IPV6_WANTED
int bgp4_ipv6_get_local_addr(u_char* p_gaddr6,u_char* p_laddr6)
{
    u_char addr[64];
    octetstring octet;
    u_int if_number=0;
    u_char buff[19];
    u_char local_buf[512];

    memset(buff, 0, 19);
    octet.pucBuf = buff;
    octet.len = 19;
    buff[2] = IP_V6FAMILY;

    if(p_gaddr6==NULL||p_laddr6==NULL)
    {
        return FALSE;
    }

    memcpy(buff+3,p_gaddr6,16);
    sysIndexTrans(&octet, SYS_IPADDR2LOGICAL_NUM, &if_number);
    bgp4_log(BGP_DEBUG_RT,1,"bgp4 ipv6 get local address get interface=%d",if_number);

    /*get local address*/
    memset(local_buf,0,512);
    memset(addr, 0, 64);
    octet.pucBuf = local_buf;
    octet.len = 512;
    uspIfGetApi (if_number, SYS_IF_IPV6ADDREX, &octet);
    /*
    地址格式:
    --------------------------------------------------------------------
    |  1byte          |  1byte          |     16byte    | 1bype | 1byte  |
    --------------------------------------------------------------------
    |  type            |  prefix         |   address   | role  |  type  |
    ---------------------------------------------------------------------
    #define USP_IPROLE_PRIMARY  0
    #define USP_IPROLE_SECONDARY    1
    #define USP_IPROLE_LINKLOCAL    2
    #define USP_IPROLE_LOOPBACK     3
            role:3,
            used:1,
    #define USP_IPTYPE_STATIC   0
    #define USP_IPTYPE_DHCP     1
    #define USP_IPTYPE_AUTO     2   自动配置ipv6 linklocal地址，如果网管要配置linklocal地址要用USP_IPTYPE_STATIC类型
    #define USP_IPTYPE_VRRP     3
    #define USP_IPTYPE_UNNUMBERED   4
    */
    /*get wanted octet string*/
    while(octet.len>=20)
    {
        if((*(octet.pucBuf)==IP_V6FAMILY)
            &&(*(octet.pucBuf+18)==2))
        {
            memcpy(p_laddr6,octet.pucBuf+2,16);
            break;
        }

        octet.len-=20;
        octet.pucBuf+=20;
    }

    inet_ntop(AF_INET6,p_laddr6,addr,64);
    bgp4_log(BGP_DEBUG_RT,1,"bgp4 ipv6 get local addr,p_laddress6=%s",addr);

    return TRUE;
}
#endif

#if 0/*don't use walkup to redistribute anymore*/
void bgp4_redistribute_ip_routes(tBGP4_VPN_INSTANCE* p_instance,u_char af,u_char proto, u_char action)
{
    int s;
    u_char para[6];
    u_char afi = af;
    u_char protocol = proto;
    u_char redis_protocol = 0;

    if(p_instance == NULL)
    {
        return;
    }
    
    para[0] = action;
    para[1] = protocol;
    memcpy(para+2,&p_instance,4);
    
    /*set rtSocket option*/
    if(action==0)
    {
        rtSockOptionSet(gBgp4.rtsock,protocol,action);
    }

    s = splnet ();

    if(afi == BGP4_PF_IPUCAST)
    {
        bgp4_log(BGP_DEBUG_RT,1,"[bgp4_redistribute_ip_routes]:set vrf %d ipv4 route action %d,proto %d,instance pointer %#x",
                    p_instance->instance_id,
                    para[0],
                    para[1],
                    p_instance);
        ipv4v6RouteTableWalk(p_instance->instance_id,AF_INET,protocol, bgp4_redistribute_walkup,(void*)para);
    }
#ifdef BGP_IPV6_WANTED
    else if(afi == BGP4_PF_IP6UCAST)
    {
        bgp4_log(BGP_DEBUG_RT,1,"[bgp4_redistribute_ip_routes]:set vrf %d ipv6 route action %d,proto %d",
                    p_instance->instance_id,
                    para[0],
                    para[1]);
        ipv4v6RouteTableWalk(p_instance->instance_id,AF_INET6,protocol, bgp4_ipv6_redistribute_walkup,(void*)para);
    }
#endif

    splx (s);

    /*set rtSocket option*/
    if(action==1)
        rtSockOptionSet(gBgp4.rtsock,protocol,action);

    return ;
}

int bgp4_redistribute_walkup(M2_IPROUTETBL* p_m2_rt,void* p_para)
{
    u_char action;
    u_char proto ;
    tBGP4_PATH path;
    tBGP4_ROUTE rt;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    action = *(u_char*)p_para;
    proto  = *((u_char*)p_para+1);

    memcpy(&p_instance,p_para+2,4);

    bgp4_log(BGP_DEBUG_EVT,1,"bgp4_redistribute_walkup route %#x,proto %d,vrf %d",
                p_m2_rt->ipRouteDest,p_m2_rt->ipRouteProto,p_instance->instance_id);

    if(p_instance == NULL)
    {
        return VOS_ERR;
    }

    memset(&rt, 0,sizeof(rt));
    memset(&path, 0, sizeof(path));

    bgp4_lstnodeinit(&path.node);
    bgp4_lstinit(&path.aspath_list);
    bgp4_lstinit(&path.route_list);
    rt.dest.afi = BGP4_PF_IPUCAST;
    *(u_int*)rt.dest.ip = p_m2_rt->ipRouteDest ;
    rt.dest.prefixlen = bgp4_mask2len(p_m2_rt->ipRouteMask);
    rt.proto = p_m2_rt->ipRouteProto;
    rt.p_path = &path;

    path.afi = BGP4_AF_IP;
    path.safi = BGP4_SAF_UCAST;
    path.p_instance = p_instance;

    *(u_int*)path.nexthop.ip = p_m2_rt->ipRouteNextHop ;
    path.nexthop.afi = BGP4_PF_IPUCAST;
    path.nexthop.prefixlen = 32;

    path.out_med = (u_int)p_m2_rt->ipRouteMetric1;

    if (proto == M2_ipRouteProto_local)
    {
        *(u_int*)path.nexthop.ip = 0 ;
    }

    if (action == 0 || bgp4_redistribute_check_route(p_instance,&rt)>0)
    {
        bgp4_import_route(p_instance,&rt, action);
    }

    return VOS_OK;
}

int bgp4_ipv6_redistribute_walkup(M2_IPV6ROUTETBL* p_m2_rt, void* p_para)
{
    u_char action;
    u_char proto ;
    tBGP4_PATH path;
    tBGP4_ROUTE rt;
    u_char dest[64];
    u_char nexthop[64];
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    memset(dest, 0, 64);
    memset(nexthop, 0, 64);
    memset(&path ,0 ,sizeof(tBGP4_PATH));
    memset(&rt ,0 ,sizeof(tBGP4_ROUTE));
    bgp4_lstnodeinit(&path.node);
    bgp4_lstinit(&path.aspath_list);
    bgp4_lstinit(&path.route_list);
    action = *(u_char*)p_para;
    proto  = *((u_char*)p_para+1);
    memcpy(&p_instance,p_para+2,4);


    if(p_instance == NULL)
    {
        return VOS_ERR;
    }

    rt.dest.afi = BGP4_PF_IP6UCAST;
    memcpy(rt.dest.ip,p_m2_rt->ipv6RouteDest,16);

    if(memcmp(rt.dest.ip,nexthop,16)==0)
        rt.dest.prefixlen=0;
    else
        rt.dest.prefixlen = p_m2_rt->ipv6RoutePfxLength;

    rt.proto = p_m2_rt->ipv6RouteProtocol;
    rt.p_path = &path;

    path.afi = BGP4_AF_IP6;
    path.safi = BGP4_SAF_UCAST;
    path.p_instance = p_instance;


    memcpy(path.nexthop_global.ip,p_m2_rt->ipv6RouteNextHop,16);
    path.nexthop_global.afi = BGP4_PF_IP6UCAST;
    path.nexthop_global.prefixlen = 128;

    path.out_med = (u_int)p_m2_rt->ipv6RouteMetric;

    bgp4_log(BGP_DEBUG_RT,1,"[bgp4_redistribute_walkup]:get a  ipv6 route from vrf %d,dest=%s,nexthop=%s,prefix=%d,proto=%d",
                    p_instance->instance_id,
                    inet_ntop(AF_INET6, rt.dest.ip, dest, 64),
                    inet_ntop(AF_INET6, path.nexthop_global.ip, nexthop, 64),
                    rt.dest.prefixlen,rt.proto);

    if (proto == M2_ipRouteProto_local)
    {
        memset(path.nexthop_global.ip,0,16) ;
    }

    if (action ==0 || bgp4_redistribute_check_route(p_instance,&rt) >0)
    {
        bgp4_import_route(p_instance,&rt, action);
    }

    return VOS_OK;
}
#endif

void
bgp4_schedule_import_update(
              u_int vrf,
              tBGP4_ROUTE *p_route)
{
    tBGP4_ROUTE dummy;
    tBGP4_PATH path;

    memset(&dummy,0,sizeof(tBGP4_ROUTE));
    memset(&path,0,sizeof(tBGP4_PATH));
    bgp4_lstnodeinit(&path.node);
    bgp4_lstinit(&path.aspath_list);
    bgp4_lstinit(&path.route_list);
    dummy.p_path = &path;

    dummy.proto = p_route->proto;

    memcpy(&dummy.dest, &p_route->dest, sizeof(tBGP4_ADDR));
    
    memcpy(&path.nexthop, &p_route->p_path->nexthop, sizeof(tBGP4_ADDR));
    
    path.afi = p_route->p_path->afi;
    path.safi = p_route->p_path->safi;

    bgp4_rtsock_route_change(vrf, &dummy, FALSE);
    return;
}

void bgp4_redistribute_ip_routes(tBGP4_VPN_INSTANCE* p_instance,u_char af,u_char proto, u_char action)
{
    int s;
    u_char para[6];
    u_char afi = af;
    u_char protocol = proto;
    u_char redis_protocol = 0;

    if(p_instance == NULL)
    {
        return;
    }

    rtSockOptionSet(gBgp4.rtsock,IP_ROUTE_REDIS_PROTO(protocol),action);

    if(action == 1)
    {       
        if(afi == BGP4_PF_IPUCAST)
        {
            bgp4_log(BGP_DEBUG_RT,1,"set vrf %d ipv4 route action %d,proto %d,instance pointer %#x",
                        p_instance->instance_id,
                        action,
                        protocol,
                        p_instance);
            
            ipRouteRedisSet(p_instance->instance_id,AF_INET,protocol,M2_ipRouteProto_bgp);
        }
#ifdef BGP_IPV6_WANTED
        else if(afi == BGP4_PF_IP6UCAST)
        {
            bgp4_log(BGP_DEBUG_RT,1,"set vrf %d ipv6 route action %d,proto %d",
                        p_instance->instance_id,
                        action,
                        protocol);
            
            ipRouteRedisSet(p_instance->instance_id,AF_INET6,protocol,M2_ipRouteProto_bgp);
        }

        rtSockOptionSet(gBgp4.rtsock,protocol,action);
#endif
    }
    else
    {
        tBGP4_VPN_INSTANCE* p_pub_instance = NULL;
        tBGP4_ROUTE  *p_scanroute = NULL;
        tBGP4_ROUTE  *p_next = NULL;
        tBGP4_ROUTE dummy;
        tBGP4_PATH path;
        
        rtSockOptionSet(gBgp4.rtsock,protocol,action);

#if 1
        RIB_LOOP(&p_instance->rib, p_scanroute, p_next)
        {
            if((p_scanroute->proto == protocol) && (p_scanroute->dest.afi == afi))
            {
            #if 0
                p_scanroute->is_deleted = TRUE;
                bgp4_schedule_rib_update(p_instance,p_scanroute,NULL,NULL);
            #else
                bgp4_schedule_import_update(p_instance->instance_id, p_scanroute);
            #endif
            }
        }
       if (p_instance->instance_id != 0)
       {
            p_pub_instance = bgp4_vpn_instance_lookup(0);
            if(p_pub_instance == NULL)
            {
                return;
            }
            p_scanroute = NULL;
            p_next = NULL;
            RIB_LOOP(&p_pub_instance->rib, p_scanroute, p_next)
            {
                if(p_scanroute->proto == protocol 
                    && (p_scanroute->p_path->src_instance_id == p_instance->instance_id)
                    && ((afi == BGP4_PF_IPUCAST && (p_scanroute->dest.afi == BGP4_PF_IPVPN))
                       || (afi == BGP4_PF_IP6UCAST && (p_scanroute->dest.afi == BGP4_PF_IP6VPN))))
                {
                  #if 0
                    p_scanroute->is_deleted = TRUE;
                    bgp4_schedule_rib_update(p_pub_instance,p_scanroute,NULL,NULL);
                  #else
                     bgp4_schedule_import_update(p_pub_instance->instance_id, p_scanroute);
                  #endif
                }
            }
        }
#endif        
    }

    return ;
}

int bgpIfaddrMatch(u_int addr,u_int mask)
{
    return IpIsLocalNet(addr, mask);
}

void bgp4_get_max_ifipaddr(u_int *pu4_max_ip)
{
    *pu4_max_ip = findMaxIfIpAddr() ;
    return;
}

u_int bgp4_get_ifnet_from_ifaddr(u_int ifaddr,u_int get_type, void *val)
{
    M2_IPADDRTBL m2Iptable;
    u_int   ret = TRUE;

    if (val == NULL)
        return VOS_ERR;

    memset(&m2Iptable, 0, sizeof(M2_IPADDRTBL));

    m2Iptable.ipAdEntAddr = ifaddr;

    ret = m2IpAddrTblEntryGet(M2_EXACT_VALUE,&m2Iptable);

    if (ret!= VOS_OK)
    {
        return ret;
    }
    switch(get_type)
    {

        case BGP4_OS_IFNET_INDEX:
            *(u_int*)val = m2Iptable.ipAdEntIfIndex;
            break;

        case BGP4_OS_IFNET_FLAG:
            *(u_int*)val = /*m2Iptable.ipAdIfFlag*/1/*ifaddr_to_flag(ifaddr)*/;
            break;

        case BGP4_OS_IFNET_MASK:
            *(u_int*)val = m2Iptable.ipAdEntNetMask;
            break;

        case BGP4_OS_IFNET_PHYSPEED:
            /*output speed unit is kbits/s*/
            break;

        default:
            break;

    }

    return ret;
}

u_int bgp4_get_ifmask(u_int if_unit,u_int addr_type)
{
    u_int prefix_len = 0;
    u_char temp_buf[32] = {0};
    octetstring oct_str;
    u_int if_addr_type = 0;

    /* 获取接口掩码信息
        地址格式:
        ---------------------------------------------
        |  1byte          |  1byte          |     4byte or 16byte     |
        ---------------------------------------------
        |  type            |  prefix len         |   address                   |
        ---------------------------------------------
    */

    memset(&oct_str,0,sizeof(octetstring));
    oct_str.pBuf = temp_buf;
    oct_str.len = 32;

    if_addr_type =  (addr_type == BGP4_PF_IPUCAST) ? SYS_IF_IPV4ADDR : SYS_IF_IPV6ADDR;


    if(uspIfGetApi(if_unit, if_addr_type, &oct_str) == VOS_ERR)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 get interface mask get interface mask failed,interface unit %d\n\r",if_unit);
    }


    prefix_len = temp_buf[1];


    return prefix_len;
}

void bgp4_set_router_id(u_int id)
{
    u_int temp_id = 0;

    if(id != 0)
    {
        if(gBgp4.router_id == id)
        {
            return;
        }

        temp_id = id;
    }
    else
    {
        if(uspIpRouterId(0, AF_INET, &temp_id) != VOS_OK)
        {
            bgp4_get_max_ifipaddr(&temp_id);
        }
    }

    gBgp4.router_id = temp_id;

    /*router id changed,all of the peers should be restart*/
    bgp4_reset_all_peers(1);

    return;
}

int bgp4_nexthop_metric_comp(tBGP4_PATH* p_path1,tBGP4_PATH* p_path2)
{
    M2_IPROUTETBL nexthop_metric1 = {0};
    M2_IPROUTETBL nexthop_metric2 = {0};
    M2_IPV6ROUTETBL nexthop6_metric1 = {0};
    M2_IPV6ROUTETBL nexthop6_metric2 = {0};
    u_int instance_id = 0;

#if !defined(USE_LINUX) && !defined(WIN32)
    if(p_path1->safi != BGP4_SAF_UCAST || p_path2 ->safi != BGP4_SAF_UCAST)
    {
        return 0;
    }

    if(p_path1->p_instance == NULL ||
        p_path2->p_instance == NULL ||
            p_path1->p_instance->instance_id != p_path2->p_instance->instance_id)
    {
        return 0;
    }

    instance_id = p_path1->p_instance->instance_id;

    if(p_path1->afi == BGP4_AF_IP && p_path2->afi == BGP4_AF_IP)
    {
        if(ip_route_match(instance_id,bgp_ip4(p_path1->nexthop.ip), &nexthop_metric1) == VOS_OK &&
            ip_route_match(instance_id,bgp_ip4(p_path2->nexthop.ip), &nexthop_metric2) == VOS_OK)
        {
            if(nexthop_metric1.ipRouteMetric1 != nexthop_metric2.ipRouteMetric1)
            {
                return (nexthop_metric1.ipRouteMetric1 < nexthop_metric2.ipRouteMetric1)?1:-1;
            }
        }
    }
    else if(p_path1->afi == BGP4_AF_IP6 && p_path2->afi == BGP4_AF_IP6)
    {
        if(ip6_route_match(instance_id,p_path1->nexthop_global.ip, &nexthop6_metric1) == VOS_OK &&
            ip6_route_match(instance_id,p_path2->nexthop_global.ip, &nexthop6_metric2) == VOS_OK)
        {
            if(nexthop6_metric1.ipv6RouteMetric != nexthop6_metric2.ipv6RouteMetric)
            {
                return (nexthop6_metric1.ipv6RouteMetric < nexthop6_metric2.ipv6RouteMetric)?1:-1;
            }
        }
    }

#endif
    return 0;
}

int bgp4_nexthop_is_reachable(u_int instance_id,tBGP4_ADDR* p_nexthop)
{
    M2_IPROUTETBL route_info = {0};
    M2_IPV6ROUTETBL route6_info = {0};
    int rc = VOS_OK;
    u_char addr[64] = {0};



#if !defined(USE_LINUX) && !defined(WIN32)
    if(p_nexthop->afi == BGP4_PF_IPUCAST)
    {
        rc = ip_route_match(instance_id,bgp_ip4(p_nexthop->ip),&route_info);
    }
    else if(p_nexthop->afi == BGP4_PF_IP6UCAST)
    {
        rc = ip6_route_match(instance_id,p_nexthop->ip,&route6_info);
    }
    else
    {
        rc = VOS_ERR;
    }
    bgp4_log(BGP_DEBUG_EVT,1,"bgp4 nexthop is reachable,nexthop %s,resource %d",
                        bgp4_printf_addr(p_nexthop,addr),rc);
#endif

    return rc;
}

u_int bgp4_if_direct_connect_addr(u_short vrf_id,u_char afi,u_char *p_ip)
{
    octetstring octet = {0};
    u_char buf[20] = {0};
    u_int ifunit = 0;
    u_short vrid = htons(vrf_id);

    /*
        ------------------------------------------------------------
        |  2byte vrid   | 1byte family |    4 or 16 byte ip address |
        ------------------------------------------------------------

        1 byte faily:IP_V4FAMILY,IP_V6FAMILY
        vrid:net byte order
        ip address:net byte order
    */


    memset(buf, 0, sizeof(buf));
    octet.pucBuf = buf;
    octet.len = (afi == BGP4_AF_IP) ? 7 : 19;

    memcpy(buf,&vrid,2);

    buf[2] = (afi == BGP4_AF_IP) ? AF_INET : AF_INET6;

    memcpy(buf + 3, p_ip, 16);

    return (sysIndexTrans(&octet, SYS_IPNET2LOGICAL_NUM, &ifunit));

}

int bgp4_get_direct_nexthop(tBGP4_ROUTE* p_route,tBGP4_PATH* p_path)
{
    M2_IPROUTETBL route_info = {0};
    M2_IPV6ROUTETBL route6_info = {0};
    int rc = VOS_OK;
    u_int addr_str[64] = {0};
    tBGP4_ADDR recursive_nexthop = {0};
    int nexthop_if_unit = -1;
    u_int loop_depth = 0;
    u_int vrf_id = 0;
    tBGP4_PATH* p_tmp_path = NULL;

    if(p_route)
    {
        p_tmp_path = p_route->p_path;
    }
    else
    {
        p_tmp_path = p_path;
    }

    /*mpls remote vpn route's nexthop will be found by mpls*/
#if 0
    if(p_route->route_direction == MPLSL3VPN_ROUTE_REMOTE ||
        p_route->route_direction == MPLS_6PE_REMOTE_ROUTE )
    {
        bgp4_log(BGP_DEBUG_RT,1,"bgp4_get_direct_nexthop,mpls remote route's direct nexthop will be found by MPLS");
        return TRUE;
    }
#endif

    if(p_tmp_path == NULL || p_tmp_path->p_instance == NULL)
    {
        return TRUE;
    }

    if((p_tmp_path->safi == BGP4_SAF_LBL)||
        (p_tmp_path->safi == BGP4_SAF_VLBL))
    {
        bgp4_log(BGP_DEBUG_RT,1,"bgp4 get direct nexthop,mpls remote route's direct nexthop will be found by MPLS");
        return TRUE;
    }

    if(p_tmp_path->afi == BGP4_AF_IP)
    {
        memcpy(&recursive_nexthop,&p_tmp_path->nexthop,sizeof(tBGP4_ADDR));
        recursive_nexthop.afi = BGP4_PF_IPUCAST;
        recursive_nexthop.prefixlen = 32;
    }
    else if(p_tmp_path->afi == BGP4_AF_IP6)
    {

        memcpy(&recursive_nexthop,&p_tmp_path->nexthop_global,sizeof(tBGP4_ADDR));
        recursive_nexthop.afi = BGP4_PF_IP6UCAST;
        recursive_nexthop.prefixlen = 128;
    }
    else
    {
        bgp4_log(BGP_DEBUG_RT,1,"bgp4 get direct nexthop,path invalid");
        return TRUE;
    }

    vrf_id = p_tmp_path->p_instance->instance_id;

    if(bgp4_if_direct_connect_addr(vrf_id,p_tmp_path->afi,recursive_nexthop.ip) == VOS_OK)
    {
        bgp4_log(BGP_DEBUG_RT,1,"bgp4 get direct nexthop,nexthop is already direct");
        return TRUE;
    }


    while(bgp4_if_direct_connect_addr(vrf_id,p_tmp_path->afi,recursive_nexthop.ip) == VOS_ERR)
    {
        bgp4_log(BGP_DEBUG_RT,1,"bgp4 get direct nexthop,nexthop is %s",
            bgp4_printf_addr(&recursive_nexthop,addr_str));

        if(p_tmp_path->afi == BGP4_AF_IP6 &&
            recursive_nexthop.ip[0] == 0xfe && recursive_nexthop.ip[1] == 0x80)
        {
            bgp4_log(BGP_DEBUG_RT,1,"bgp4 get direct nexthop,ipv6 local route");
            break;
        }

        if(loop_depth ++ >= 10)
        {
            bgp4_log(BGP_DEBUG_RT,1,"bgp4 get direct nexthop,loop too many times");
            break;
        }

        if(p_tmp_path->afi == BGP4_AF_IP)
        {
            rc = ip_route_match(vrf_id,bgp_ip4(recursive_nexthop.ip),&route_info);
            if(rc == VOS_OK)
            {
                memcpy(recursive_nexthop.ip,&route_info.ipRouteNextHop,4);
                recursive_nexthop.afi = BGP4_PF_IPUCAST;
                recursive_nexthop.prefixlen = 32;
                if(ifIndexToIfunit(route_info.ipRouteIfIndex,&nexthop_if_unit) == VOS_ERR)
                {
                    bgp4_log(BGP_DEBUG_RT,1,"interface Index to Interface unit ERROR!nexthop index %d",route_info.ipRouteIfIndex);
                    memset(recursive_nexthop.ip,0,24);
                    nexthop_if_unit = -1;
                    break;
                }
            }
            else/*not found*/
            {
                bgp4_log(BGP_DEBUG_RT,1,"bgp4 get direct nexthop,nexthop not found");
                nexthop_if_unit = -1;
                break;
            }
        }
        else
        {
            rc = ip6_route_match(vrf_id,p_tmp_path->nexthop_global.ip,&route6_info);
            if(rc == VOS_OK)
            {
                memcpy(recursive_nexthop.ip,route6_info.ipv6RouteNextHop,16);
                recursive_nexthop.afi = BGP4_PF_IP6UCAST;
                recursive_nexthop.prefixlen = 128;
                if(ifIndexToIfunit(route6_info.ipv6RouteIfIndex,&nexthop_if_unit) == VOS_ERR)
                {
                    bgp4_log(BGP_DEBUG_RT,1,"interface Index to Interface unit ERROR!nexthop index %d",route6_info.ipv6RouteIfIndex);
                    memset(recursive_nexthop.ip,0,24);
                    nexthop_if_unit = -1;
                    break;
                }

            }
            else/*not found*/
            {
                bgp4_log(BGP_DEBUG_RT,1,"bgp4 get direct nexthop,nexthop not found");
                nexthop_if_unit = -1;
                break;
            }
        }
    }
    if(nexthop_if_unit >= 0)/*found*/
    {
        memcpy(&p_tmp_path->direct_nexthop,&recursive_nexthop,sizeof(tBGP4_ADDR));
        p_tmp_path->direct_nexthop_ifunit = nexthop_if_unit;
        bgp4_log(BGP_DEBUG_RT,1,"bgp4 get direct nexthop found,nexthop %s,interface unit %d",
                    bgp4_printf_addr(&p_tmp_path->direct_nexthop,addr_str),
                    p_tmp_path->direct_nexthop_ifunit);

        gBgp4.direct_nexthop_exist = 1;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void bgp4_recalculate_direct_nexthop(void)
{
    tBGP4_PATH *p_path = NULL ;
    tBGP4_ROUTE* p_route = NULL;
    tBGP4_ADDR direct_nexthop = {0};
    int ifunit_old = 0;
    u_int af = 0;
    u_int vrf_id = 0;

    for(af = 0;af< BGP4_PF_MAX;af++)
    {
        LST_LOOP(&gBgp4.attr_list[af], p_path, node, tBGP4_PATH)
        {
            if(p_path->p_instance == NULL)
            {
                continue;
            }
            vrf_id = p_path->p_instance->instance_id;

            if(p_path->direct_nexthop_ifunit >= 0)
            {
                memcpy(&direct_nexthop,&p_path->direct_nexthop,sizeof(tBGP4_ADDR));
                ifunit_old = p_path->direct_nexthop_ifunit;
                bgp4_get_direct_nexthop(NULL,p_path);

                /*next hop changed*/
                if(ifunit_old != p_path->direct_nexthop_ifunit)
                {
                    LST_LOOP(&p_path->route_list, p_route, node, tBGP4_ROUTE)
                    {
                        if(p_path->afi == BGP4_AF_IP)
                        {
                            /*only renotify HW ROUTE*/
                            while (bgp4_add_routemsg(vrf_id,
                                   gBgp4.ip_msg_buf,p_route,
                                   &direct_nexthop,ifunit_old) != TRUE)
                            {
                                bgp4_rtsock_recv();
                                vos_pthread_delay(1);
                            }
                            p_route->ip_action = BGP4_IP_ACTION_ADD;
                            while (bgp4_add_routemsg(vrf_id,
                                   gBgp4.ip_msg_buf,p_route,
                                   NULL,0) != TRUE)
                            {
                                bgp4_rtsock_recv();
                                vos_pthread_delay(1);
                            }
                            p_route->ip_action = BGP4_IP_ACTION_NONE;
                        }
                        else if(p_path->afi == BGP4_AF_IP6)
                        {
                            /*only renotify HW ROUTE*/
                            while (bgp4_add_route6msg(vrf_id,
                                   gBgp4.ip6_msg_buf,p_route,
                                   &direct_nexthop,ifunit_old) != TRUE)
                            {
                                bgp4_rtsock_recv();
                                vos_pthread_delay(1);
                            }
                            p_route->ip_action = BGP4_IP_ACTION_ADD;
                            while (bgp4_add_route6msg(vrf_id,
                                   gBgp4.ip6_msg_buf,p_route,
                                   NULL,0) != TRUE)
                            {
                                bgp4_rtsock_recv();
                                vos_pthread_delay(1);
                            }   
                            p_route->ip_action = BGP4_IP_ACTION_NONE;
                        }
                    }

                    gBgp4.direct_nexthop_changed_times++;
                }
            }
        }
    }

    return;

}

#if 0
int bgp4_bfd_notify (struct notifier_block *self, u_int event, void *session, void *arg)
{
    struct _bfd_info *p_bfd_info = (struct _bfd_info *)arg;

    struct tBGP4_PEER *p_peer = (struct tBGP4_PEER *)((u_char *)self - (u_char *)mbroffset(tBGP4_PEER , bfd_cookie));

    bgp4_log(BGP_DEBUG_EVT,1, "call bgp4_bfd_notify");

    bgp4_log(BGP_DEBUG_EVT, 1 , "bgp notify bfd event %d, state %d ", event , p_bfd_info->state);

    if(self == NULL)
        {
            bgp4_log(BGP_DEBUG_EVT,1, "notify msg notifier_block is NULL");
            return NOTIFY_OK ;
        }

    /*PROTO_BFD_SESSION */
    if (SYS_SYNC_BFDSESSION != event)
    {
            return NOTIFY_OK ;
    }

    /*ignore up*/
    if (BFD_DOWN != p_bfd_info->state)
    {
            return NOTIFY_OK ;
    }

    bgp4_log(BGP_DEBUG_EVT,1, "bfd callback close");

    bgp4_fsm(p_peer,BGP4_EVENT_TCP_CLOSED);

    return NOTIFY_OK;
}
#endif

/*must bind when peer state is ESTABLISH*/
void  bgp4_bind_bfd(tBGP4_PEER *p_peer)
{
    u_char addStr[64] = {0};
#if 0
    struct notifier_block *p_notifer = (struct notifier_block *)p_peer->bfd_cookie;
    p_notifer->notifier_call = bgp4_bfd_notify;
#endif

    if(p_peer->bfd_enable == 0)
    {
        return;
    }

    bgp4_printf_peer(p_peer ,addStr);
    bgp4_log(BGP_DEBUG_EVT, 1 , "bgp bind bfd neighbor address %s",addStr);


#ifdef BGP_BFD_WANTED
    if(p_peer->remote.ip.afi == BGP4_PF_IPUCAST)
    {
        p_peer->bfd_discribe = bfdSessionBind(IPPROTO_BGP4, p_peer->if_unit , BFD_IPV4, 4,
            &p_peer->local.ip.ip, &p_peer->remote.ip.ip,  /*p_notifer*/NULL);
    }
#ifdef BGP_IPV6_WANTED
    else if(p_peer->remote.ip.afi == BGP4_PF_IP6UCAST)
    {
        p_peer->bfd_discribe = bfdSessionBind(IPPROTO_BGP4, p_peer->if_unit , BFD_IPV6, 16,
            &p_peer->local.ip.ip, &p_peer->remote.ip.ip,  /*p_notifer*/NULL);
    }
#endif
    else
    {
        p_peer->bfd_discribe = VOS_ERR;
    }

    if (VOS_ERR == p_peer->bfd_discribe)
    {
        bgp4_log(BGP_DEBUG_EVT, 1 , "bgp bind bfd neighbor failed");
    }
#endif
    return ;
}

void  bgp4_unbind_bfd(tBGP4_PEER *p_peer)
{
#if 0
    struct notifier_block *p_notifer = (struct notifier_block *)p_peer->bfd_cookie;
#endif
#ifdef BGP_BFD_WANTED

    if (0 <=  p_peer->bfd_discribe)
    {
        bgp4_log(BGP_DEBUG_EVT, 1 , "bgp unbind bfd neighbor, bfd discribe=%x",p_peer->bfd_discribe);
            if (bfdSessionUnbind(IPPROTO_BGP4, p_peer->bfd_discribe, /*p_notifer*/NULL) != VOS_OK)
            {
            bgp4_log(BGP_DEBUG_EVT, 1 , "bfd session unbind error");
            }
        bgp4_log(BGP_DEBUG_EVT,1, "bfd unbind success");
    }
    p_peer->bfd_discribe = -1 ;
#endif
    return ;
}

void bgp_test1(u_char* ipv4_addr)
{
    tBGP4_PATH* p_path = NULL;
    u_int ip = 0;

    bgp_sem_take();

    p_path = bgp4_add_path(0);

    if(p_path == NULL || ipv4_addr == NULL)
    {
        bgp_sem_give();
        return;
    }

    p_path->p_instance = bgp4_vpn_instance_lookup(0);

    if(p_path->p_instance == NULL)
    {
        bgp_sem_give();
        return;
    }

    p_path->nexthop.afi = BGP4_PF_IPUCAST;
    ip = inet_addr(/*"3.3.3.1"*/ipv4_addr);
    memcpy(p_path->nexthop.ip,&ip,4);

    bgp4_get_direct_nexthop(NULL,p_path);

    bgp4_log(BGP_DEBUG_RT,1,"direct nexthop %d.%d.%d.%d",p_path->direct_nexthop.ip[0],
        p_path->direct_nexthop.ip[1],p_path->direct_nexthop.ip[2],p_path->direct_nexthop.ip[3]);

    bgp_sem_give();
}

void bgp_test2(u_char* ipv6_addr)
{
    tBGP4_PATH* p_path = NULL;
    u_char ip[16] = {0};
    u_char addr_str1[64] = {0};
    u_char addr_str2[64] = {0};

    bgp_sem_take();

    p_path = bgp4_add_path(0);

    if(p_path == NULL || ipv6_addr == NULL)
    {
        bgp_sem_give();
        return;
    }

    p_path->p_instance = bgp4_vpn_instance_lookup(0);

    if(p_path->p_instance == NULL)
    {
        bgp_sem_give();
        return;
    }


    p_path->afi = BGP4_AF_IP6;
    p_path->safi = BGP4_SAF_UCAST;

    p_path->nexthop_global.afi = BGP4_PF_IP6UCAST;
    /*fe80:3::204:6710:d098:500e*/
    inet_pton(AF_INET6,ipv6_addr,p_path->nexthop_global.ip);

    bgp4_get_direct_nexthop(NULL,p_path);

    bgp4_log(BGP_DEBUG_RT,1,"direct nexthop %s",bgp4_printf_addr(&p_path->direct_nexthop,addr_str1));


    bgp_sem_give();

    return;

}

#endif
