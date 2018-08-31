
#include "bgp4tcph.h"
#ifdef NEW_BGP_WANTED
#include "bgp4com.h"
#if !defined(WIN32)
//#include "tcp.h" 
#endif
#if 0
#include "sockLib.h"
#include "errnoLib.h"
#endif

u_int
bgp4_ip2ifunit(
       u_int vrf_id,
       tBGP4_ADDR *p_ip)
{
    u_int af;
    u_int if_unit = 0;
    u_char str[64];
    u_char addr[16];
    
    af = (p_ip->afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;

    /* 获取接口unit */
    memset(addr, 0, sizeof(addr));
    if (af == AF_INET)
    {
        bgp_ip4(addr) = ntohl(bgp_ip4(p_ip->ip));
    }
    else
    {
        memcpy(addr, p_ip->ip, sizeof(addr));
    }
    if (uspGetIfUnitByIp(vrf_id, af, addr, (u_int *)&if_unit) == VOS_ERR)
    {
        bgp4_printf_addr(p_ip, str);
        bgp4_log(BGP_DEBUG_EVT, "can not get interface index by %s,vrf %d", str, vrf_id);
    }
    return if_unit;
}

int
bgp4_max_peer_socket(int exclude)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    int fdmax = 0;
    
    bgp4_instance_for_each(p_instance)
    {
        bgp4_peer_for_each(p_instance, p_peer) 
        {
            if ((p_peer->sock > 0) && (p_peer->sock != exclude))
            {
                fdmax = BGP4_MAX(fdmax, p_peer->sock);
            }
        }
    }
    return fdmax;
}

/*get tcp port and address for peer, need merge to ipv6*/
void 
bgp4_peer_tcp_addr_fill(tBGP4_PEER *p_peer)
{
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    
    int len;

    p_peer->local_tcp_port = 0;
    bgp_ip4(p_peer->local_ip.ip) = 0;
    p_peer->tcp_port = 0; 

    if (p_peer->sock <= 0)
    {
        return;
    }
    /*insert to read socket*/
    FD_SET(p_peer->sock, gbgp4.fd_read);
    FD_CLR(p_peer->sock, gbgp4.fd_write);

    /*update max socket*/
    gbgp4.max_sock = BGP4_MAX(gbgp4.max_sock, p_peer->sock);
        
    if (p_peer->ip.afi == BGP4_PF_IPUCAST)
    {
        len = sizeof(sa);
        if (getsockname(p_peer->sock,((struct sockaddr *)&sa), &len) == VOS_ERR) 
        {
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"get sock name by socket failed");
            return;
        }
        p_peer->local_tcp_port = ntohs(sa.sin_port);
        bgp_ip4(p_peer->local_ip.ip) = (sa.sin_addr.s_addr);  
        p_peer->local_ip.afi = BGP4_PF_IPUCAST;
        p_peer->local_ip.prefixlen = 4;
        if (getpeername(p_peer->sock, (struct sockaddr *)&sa, &len) == VOS_ERR) 
        {
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"get peer name by socket failed");
            return;
        }
        p_peer->tcp_port = ntohs(sa.sin_port);
    }
    else 
    {
        len = sizeof(sa6);

        if (getsockname(p_peer->sock, ((struct sockaddr *)&sa6), &len) == VOS_ERR) 
        {
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"get sock name by socket failed");
            return;
        }
        memcpy(p_peer->local_ip.ip,&(sa6.sin6_addr),16);     
        p_peer->local_tcp_port = ntohs(sa6.sin6_port);
        p_peer->local_ip.afi=BGP4_PF_IP6UCAST;
        p_peer->local_ip.prefixlen=16;
    #ifndef WIN32
        if (getpeername(p_peer->sock, (struct sockaddr *)&sa6, &len) == VOS_ERR) 
        {
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"get peer name by socket failed");
            return;
        }
    #endif
        p_peer->tcp_port = ntohs(sa6.sin6_port);
    }
    /*get if unit number*/
    p_peer->if_unit = bgp4_ip2ifunit(p_peer->p_instance->vrf, &p_peer->local_ip);
    return;
}

int
bgp4_set_sock_noblock(
             int s,
             int on)
{
    if (s <= 0)
    {
        return VOS_ERR;
    }
#if !defined(WIN32)    
    return ioctl(s, FIONBIO, (void *)&on);
#else
    return ioctlsocket(s, FIONBIO, (int)&on);
#endif
}

int
bgp4_set_sock_tcpnodelay(
             int s,
             int on)
{
    if (s <= 0)
    {
        return VOS_ERR;
    }
#ifndef USE_LINUX        
    return setsockopt (s, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof (on));
#endif
}

int
bgp4_set_sock_rxbuf(
             int s,
             int on)
{
    if ( s<= 0)
    {
        return VOS_ERR;
    }
    return setsockopt (s, SOL_SOCKET, SO_RCVBUF, (char *)&on, sizeof(on)) ;
}

int 
bgp4_set_sock_txbuf(
         int s,
         int on)
{
    if (s <= 0)
    {
        return VOS_ERR;    
    }
    return setsockopt (s, SOL_SOCKET, SO_SNDBUF, (char *)&on, sizeof(on)) ;
}

int
bgp4_set_sock_reuseaddr(
             int s,
             int on)
{
    if (s <= 0)
    {
        return VOS_ERR;
    }
#ifndef USE_LINUX        
    return setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) ;
#endif
}

int 
bgp4_set_sock_md5_support(
         int s,
         int on)
{
#if !defined(USE_LINUX) && !defined(WIN32) 
    int rc = VOS_ERR;

    if (s <= 0)
    {
        return VOS_ERR;
    }
    #if 0 /*TODO:需要在ipcom中添加该选项*/
    rc = setsockopt (s, IPPROTO_TCP, TCP_MD5SIG, (char*)&on, sizeof(on));
    #endif    
    bgp4_log(BGP_DEBUG_TCP,"set socket %d md5 signature on,ret %d",s,rc);

    return rc;
#else
    return VOS_OK;
#endif
}

void
bgp4_set_sock_md5_key(
          int s,
          int on,
          tBGP4_PEER *p_peer)
{
    #if 0 /*TODO:需要实现md5的相关功能*/
#if !defined(USE_LINUX) && !defined(WIN32) 
    u_char *p_key = p_peer->md5key;
    u_char len = p_peer->md5key_len;
    u_char pf = p_peer->ip.afi;
    tBGP4_ADDR *p_addr = &p_peer->ip;
    struct md5Key key;
    int rc = VOS_ERR;
    u_char addr[64] = {0};

    if ( s<= 0)
    {
        return;
    }
    
    memset(&key, 0, sizeof(struct md5Key));
    key.keyOnoff = on;
    key.keyLen = len;
    memcpy(key.keyData, p_key, len);

    if (pf == BGP4_PF_IPUCAST)
    {
        key.keyFamily = AF_INET;
        key.addr.keyAddr = bgp_ip4(p_addr->ip);
    }
    else if (pf == BGP4_PF_IP6UCAST)
    {
        key.keyFamily = AF_INET6;
        memcpy(key.addr.keyAddr6, p_addr->ip, 16);
    }
    
    rc = setsockopt(s, SOL_SOCKET, SO_MD5KEY, (char *) &key, sizeof (key));

    inet_ntop(AF_INET6, key.addr.keyAddr6, addr, 64);
   
    bgp4_log(BGP_DEBUG_TCP,"set socket %d md5 key,key af %d,addr %s,ret %d",
                   s,
                   key.keyFamily,
                   addr,
                   rc);

    return;
 #else
    return;
 #endif
 #endif
}

void
bgp4_tcp_set_peer_ttl(
         int s,
         tBGP4_PEER *p_peer)
{
#if !defined(USE_LINUX) && !defined(WIN32) 
    int maxttl = 255;
    #if 0 /*TODO:需要待确认SO_INMINTTL*/
    struct inMinttl ttl_bgp; 
    
    if (s <= 0)
    {
        return;
    }
    
    if (p_peer->ttl_hops && p_peer->ttl_hops < 256)
    {
        ttl_bgp.onOff = 1;
    }
    else if (p_peer->ttl_hops == 0)
    {
        ttl_bgp.onOff = 0;
    }

    ttl_bgp.minttl = 255 - p_peer->ttl_hops + 1;
    if (p_peer->ip.afi == BGP4_PF_IPUCAST)
    {
        ttl_bgp.family = AF_INET;
        ttl_bgp.ipaddr.addr = bgp_ip4(p_peer->ip.ip);
    }
    else if (p_peer->ip.afi == BGP4_PF_IP6UCAST)
    {
        ttl_bgp.family = AF_INET6;
        memcpy(ttl_bgp.ipaddr.addr6, p_peer->ip.ip, 16);
    }
    
    setsockopt(s, SOL_SOCKET, SO_INMINTTL, (char *)&ttl_bgp,sizeof(ttl_bgp));
    #endif

    if (p_peer->ttl_hops && p_peer->ttl_hops < 256)
    {
        maxttl = p_peer->ttl_hops;
    }
    setsockopt(s, IPPROTO_IP, IP_TTL,  (char *)&maxttl, sizeof (maxttl)); 
 #endif
    return;
}

void 
bgp4_tcp_close_socket(int s)
{
    if (s <= 0)
    {
        return;
    }
    /*clear from fdset*/
    FD_CLR(s, gbgp4.fd_read);
    FD_CLR(s, gbgp4.fd_write);
    if (s == gbgp4.max_sock)
    {
        gbgp4.max_sock = bgp4_max_peer_socket(s);
    }
    
#if !defined(WIN32)
    close (s);
#else
    closesocket (s);
#endif    
    return;
}

/*open connection to neighbor,*/
int 
bgp4_peer_sock_connect(tBGP4_PEER *p_peer)
{
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    struct sockaddr *p_sa = NULL;
    int salen = 0;
    int s = 0;
    u_char addr[64];
    u_char errstr[64];
    int optval = 0;
    int on = 1;
    u_int buflen = BGP4_TCP_BUFFER_LEN;
    u_int loopTimes =0;
    u_int uiVrf = 0;
    
    if (bgp4_get_workmode() == BGP4_MODE_SLAVE)
    {
        bgp4_log(BGP_DEBUG_TCP, "bgp mode is slave  can't open socket!");
        return VOS_OK;
    }

    /*do not act as client*/     
    if (!gbgp4.tcp_client_enable)
    {
        bgp4_log(BGP_DEBUG_TCP,"tcp client connection is disabled");
        return VOS_ERR;
    } 
    /*route is halted,do nothing*/
    if (p_peer->admin_state == BGP4_PEER_HALTED)
    {
        bgp4_log(BGP_DEBUG_TCP,"peer is disabled");
        return VOS_ERR;
    }

    /*if there is any route learnt from peer,
     and peer and local is not in restarting,do not connect*/
    if ((p_peer->restart_role == BGP4_GR_ROLE_NORMAL)
        && (bgp4_peer_route_exist(p_peer) == TRUE))
    {
        bgp4_log(BGP_DEBUG_TCP, "wait route flush before connect to peer");
        return VOS_ERR;
    }

    if (p_peer->sock > 0)
    {
        bgp4_peer_sock_close(p_peer);
    }
    
    bgp4_peer_rxmsg_reset(p_peer);
    
    gbgp4.stat.sock.create++;
    
    /*create socket*/
    if (p_peer->ip.afi == BGP4_PF_IPUCAST)
    {
        s = socket(AF_INET, SOCK_STREAM, 0);
    }
    else
    {
        s = socket(AF_INET6, SOCK_STREAM, 0);
    }
    
    if (s < 0)
    { 
        gbgp4.stat.sock.create_fail++;
        bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"open socket to %s failed %s",
            bgp4_printf_peer(p_peer, addr),
            bgp4_printf_syserror(errnoGet(),errstr));
        return VOS_ERR;
    }
    #ifndef USE_LINUX_OS
    if(p_peer->p_instance)
    {
        uiVrf = p_peer->p_instance->vrf;
        if(uiVrf > 0)
        {
            setsockopt (s, SOL_SOCKET, SO_X_VR, &uiVrf, sizeof (uiVrf));
        }
    }
    #endif
    /*if update source is not zero,use the assigned value,ipv4 support now*/
    if (p_peer->update_source.prefixlen == 32)
    {
        memset(&sa, 0, sizeof(sa)); 
        
        sa.sin_family = AF_INET;        
        sa.sin_port = 0;
        memcpy(&sa.sin_addr.s_addr, p_peer->update_source.ip, 4);
        
        gbgp4.stat.sock.bind++;
        if (bind(s,(struct sockaddr *)&sa, sizeof(sa)) < 0)
        {
            bgp4_printf_syserror(errnoGet(), errstr);     
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"bind peer %s socket %d failed-%s",
                bgp4_printf_peer(p_peer, addr),
                s,
                errstr);

            gbgp4.stat.sock.bind_fail++;
            bgp4_fsm(p_peer, BGP4_EVENT_CONNECT_FAIL);
            bgp4_tcp_close_socket(s);
            return VOS_OK;
        }
    }
    else if (p_peer->update_source.prefixlen == 128)
    {
        memset(&sa6, 0, sizeof(sa6));

        sa6.sin6_family = AF_INET6;
        sa6.sin6_port = 0;
        memcpy(&sa6.sin6_addr, p_peer->update_source.ip, 16);
        
        gbgp4.stat.sock.bind++;
        if (bind(s, (struct sockaddr *)&sa6, sizeof(sa6)) < 0)
        {
            bgp4_printf_syserror(errnoGet(), errstr);     
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"bind socket %d failed %s",
                        s,
                        bgp4_printf_addr(&p_peer->update_source,addr),
                        errstr);

            gbgp4.stat.sock.bind_fail++;
            bgp4_fsm(p_peer, BGP4_EVENT_CONNECT_FAIL);
            bgp4_tcp_close_socket (s);
            return VOS_OK;
        }
    }
    
    /*io noblk*/
    on = 1;
    bgp4_set_sock_noblock(s, on);
        
    /*tcp no delay*/
    optval = 1;
    bgp4_set_sock_tcpnodelay(s, optval);

    /*rx&tx buf length*/
    bgp4_set_sock_rxbuf(s, buflen);  
    bgp4_set_sock_txbuf(s, buflen);  

    if (p_peer->md5_support) 
    {
        bgp4_set_sock_md5_support(s, 1);
        bgp4_set_sock_md5_key(s, 1, p_peer);
    }

    /*ttl set*/
    bgp4_tcp_set_peer_ttl(s, p_peer);
    
    gbgp4.stat.sock.connect++;

    if (p_peer->ip.afi == BGP4_PF_IPUCAST )
    {
        memset(&sa, 0, sizeof(sa)); 
        sa.sin_family = AF_INET;
        sa.sin_port = htons(gbgp4.server_port);
        sa.sin_addr.s_addr = (bgp_ip4(p_peer->ip.ip));
        p_sa = (struct sockaddr *)&sa;
        salen = sizeof(sa);
     #if 0
     #ifndef WIN32
       sa.vr_id = p_peer->p_instance->vrf;
     #else
        if (p_peer->fixed_tcp_port)
        {
            sa.sin_port = htons(p_peer->fixed_tcp_port);
        }
     #endif
     #endif
        if (p_peer->fixed_tcp_port)
        {
            sa.sin_port = htons(p_peer->fixed_tcp_port);
        }
    }
    else 
    {
        memset(&sa6, 0, sizeof(sa6));
        sa6.sin6_family = AF_INET6;
        sa6.sin6_port = htons(gbgp4.server_port);
        memcpy(&sa6.sin6_addr, &p_peer->ip.ip, 16);
        p_sa = (struct sockaddr *)&sa6;
        salen = sizeof(sa6);
       #if 0
      #ifndef WIN32
        sa6.vr_id = p_peer->p_instance->vrf;
      #else
        if (p_peer->fixed_tcp_port)
        {
            sa6.sin6_port = htons(p_peer->fixed_tcp_port);
        }
      #endif
      #endif
        if (p_peer->fixed_tcp_port)
        {
            sa6.sin6_port = htons(p_peer->fixed_tcp_port);
        }
        /*set ipv6 socket only receiving ipv6 packets*/
        #if 0 /*TODO:需要在ipcom中添加该选项*/
        setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&on, sizeof(on));
        #endif
    }

#ifndef WIN32    
    bgp4_log(BGP_DEBUG_TCP,"connect vrf %d socket %d to %s:%d",
                    p_peer->p_instance->vrf,
                    s,
                    bgp4_printf_peer(p_peer, addr),
                    gbgp4.server_port);
#else
    bgp4_log(BGP_DEBUG_TCP,"connect socket %d to %s:%d",
                    s,
                    bgp4_printf_peer(p_peer, addr),
                    gbgp4.server_port);
#endif

    if (connect(s, p_sa, salen) < 0)
    //while(connect(s, p_sa, salen) < 0)
    {
        int error = errnoGet () & 0xffff;
#if !defined(WIN32)
        if (error == EINPROGRESS || error == EALREADY)
#else
        if (error == WSAEINPROGRESS 
                || error == WSAEALREADY
                || error == WSAEWOULDBLOCK)
#endif
        {
            p_peer->sock = s ;
            p_peer->connect_inprogress = 1;

            /*insert to write socket*/
            FD_SET(s, gbgp4.fd_write);
            
            gbgp4.max_sock = BGP4_MAX(gbgp4.max_sock, p_peer->sock);
            
            bgp4_printf_syserror(error,errstr);          
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"connect socket %d to %s failed %s,just wait",
                        s,
                        bgp4_printf_peer(p_peer, addr),
                        errstr);
            return VOS_OK;
            #if 0
            loopTimes++;
            if(loopTimes > 100)
            {
                return VOS_OK;
            }
            vos_pthread_delay(10);
            continue;
            #endif
        }
                
        bgp4_printf_syserror(error,errstr);          
        bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"connect socket %d to %s failed %s",
                        s,
                        bgp4_printf_peer(p_peer, addr),
                        errstr);
        gbgp4.stat.sock.connect_fail++;
        bgp4_fsm(p_peer, BGP4_EVENT_CONNECT_FAIL);
        bgp4_tcp_close_socket(s);
        return VOS_OK;
    }
    
    /*connect successfully,add to read fd*/                
    bgp4_log(BGP_DEBUG_TCP,"connect socket %d to %s ok",
                    s,
                    bgp4_printf_peer(p_peer, addr));
    
    p_peer->sock = s;
    p_peer->connect_inprogress = 0;

    /*insert to read socket*/
    bgp4_peer_tcp_addr_fill(p_peer);
    p_peer->state = BGP4_NS_CONNECT;
    bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
    return VOS_OK;
}

/*close socket of peer*/
void 
bgp4_peer_sock_close(tBGP4_PEER *p_peer)
{
    u_char str[64];
    
    bgp4_peer_rxmsg_reset(p_peer);
    
    if (gbgp4.work_mode == BGP4_MODE_SLAVE)
    {
        return;
    }
    p_peer->local_tcp_port = 0;
    p_peer->tcp_port = 0;
    p_peer->version = BGP4_VERSION_4;
    p_peer->connect_inprogress = 0; 
    p_peer->router_id = 0;
    p_peer->if_unit = 0;
    
    if (p_peer->sock > 0)
    {
        bgp4_log(BGP_DEBUG_TCP,"close peer %s socket% d", 
          bgp4_printf_peer(p_peer, str), p_peer->sock);

        bgp4_tcp_close_socket(p_peer->sock);
    }
    p_peer->sock = BGP4_INVALID_SOCKET;    
    return;
}

/*init server socket,an ipv6 socket can accept both ip&ip6 connection*/
void 
bgp4_server_sock_open(u_int uiVrf) 
{
    struct sockaddr_in6 sa6 = {0};
    struct sockaddr_in sa = {0};
    int s6;
    int s;
    int on = 1;
    u_int buflen = 81920;
    int error =0;

    if (gbgp4.tcp_server_enable != TRUE)
    {
        return;
    }

    /*V4*/
    if (gbgp4.server_sock[uiVrf]  == BGP4_INVALID_SOCKET)        
    {
        gbgp4.stat.sock.create++;
        
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0)
        {  
            error=errnoGet();
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"open listen port failed,errno=%d",error);
            gbgp4.stat.sock.create_fail++;
            gbgp4.server_sock[uiVrf] = BGP4_INVALID_SOCKET;
            return;
        }
        bgp4_set_sock_noblock(s, on);
        memset(&sa, 0, sizeof(sa));

        sa.sin_family = AF_INET;
        sa.sin_port = htons(gbgp4.server_port);
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
#ifndef USE_LINUX_OS       
        sa.sin_len = sizeof (sa);
#endif
        bgp4_set_sock_rxbuf(s, buflen);  
        bgp4_set_sock_txbuf(s, buflen);  
                    
        on = 1;
        bgp4_set_sock_reuseaddr(s,on);
            
        on = 1;
        bgp4_set_sock_md5_support(s, on);

#ifndef USE_LINUX_OS
        setsockopt (s, SOL_SOCKET, SO_X_VR, &uiVrf, sizeof (uiVrf));
#endif

        gbgp4.stat.sock.bind++;

        if (bind (s, (struct sockaddr *) &sa, sizeof (sa)) < 0)
        {
            error = errnoGet();
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"bind listen port failed,erron=%d",error);
            gbgp4.stat.sock.bind_fail++;
            gbgp4.server_sock[uiVrf] = BGP4_INVALID_SOCKET;
            bgp4_tcp_close_socket(s);
            return;
        }

        /*backlog==32?*/
        if (listen(s, 32) == VOS_ERR)
        {
            error = errnoGet();
            bgp4_log(BGP_DEBUG_TCP,"set listen port failed,erron=%d",error);
            bgp4_tcp_close_socket(s);
            gbgp4.server_sock[uiVrf] = BGP4_INVALID_SOCKET;
            return;
        }
        gbgp4.server_sock[uiVrf] = s;
    }
        
    if (gbgp4.server_sock6[uiVrf]  == BGP4_INVALID_SOCKET)
    {
        gbgp4.stat.sock.create++;
        s6 = socket(AF_INET6, SOCK_STREAM, 0);
        if (s6 < 0)
        {
            error = errnoGet();
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"open listen ipv6 port failed,erron=%d",error);

            gbgp4.stat.sock.create_fail++;
            gbgp4.server_sock6[uiVrf] = BGP4_INVALID_SOCKET;
            return;
        }
        
        /*set ipv6 socket only receiving ipv6 packets*/
        #if 0 /*TODO:需要在ipcom中添加该选项*/
        setsockopt(s6, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&on, sizeof(on)); 
        #endif
        bgp4_set_sock_noblock(s6, on);
        
        memset(&sa6, 0, sizeof(sa6));
        sa6.sin6_family = AF_INET6;
        sa6.sin6_port = htons(gbgp4.server_port);
        #if 0 /*TODO:待确认 sa.sin_len*/
        #if !defined(USE_LINUX) && !defined(WIN32)      
        sa6.sin6_len = sizeof (struct sockaddr_in6); 
        #endif
        #endif
        bgp4_set_sock_rxbuf(s6, buflen);  
        bgp4_set_sock_txbuf(s6, buflen);  

        on = 1;
        bgp4_set_sock_reuseaddr(s6, on);
        
        on = 1;
        bgp4_set_sock_md5_support(s6, on);
        
        gbgp4.stat.sock.bind++;

        if (bind (s6, (struct sockaddr *) &sa6, sizeof (sa6)) < 0)
        {
            error = errnoGet();
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"bind listen ipv6 port failed,erron=%d",error);
            perror("Bind bgp tcp6 socket");
            gbgp4.stat.sock.bind_fail++;
            gbgp4.server_sock6[uiVrf] = BGP4_INVALID_SOCKET;
            bgp4_tcp_close_socket(s6);
            return;
        }
        
        /*backlog==32?*/
        if (listen(s6, 32) == VOS_ERR)
        {
            error = errnoGet();
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"set listen ipv6 port failed,erron=%d",error);
            bgp4_tcp_close_socket(s6);
            gbgp4.server_sock6[uiVrf] = BGP4_INVALID_SOCKET;
            return;
        }
        gbgp4.server_sock6[uiVrf] = s6;
    }
    return;
}

/*close listen port*/
void 
bgp4_server_sock_close(u_int uiVrf) 
{
    int on = 0;

    if (gbgp4.server_sock[uiVrf] != BGP4_INVALID_SOCKET)
    {
        bgp4_log(BGP_DEBUG_TCP,"bgp4 close server socket:%d", gbgp4.server_sock[uiVrf]);
        bgp4_set_sock_noblock(gbgp4.server_sock[uiVrf], on);/*always wait to down*/
        bgp4_tcp_close_socket(gbgp4.server_sock[uiVrf]);
        gbgp4.server_sock[uiVrf] = BGP4_INVALID_SOCKET;
    }
    
    if (gbgp4.server_sock6 != BGP4_INVALID_SOCKET)
    {
        bgp4_log(BGP_DEBUG_TCP,"bgp4 close ipv6 server socket:%d",gbgp4.server_sock6[uiVrf]);
        bgp4_set_sock_noblock(gbgp4.server_sock6[uiVrf], on);/*always wait to down*/ 
        bgp4_tcp_close_socket(gbgp4.server_sock6[uiVrf]);
        gbgp4.server_sock6[uiVrf] = BGP4_INVALID_SOCKET;
    }
    return;
}

/*send data on socket*/
int 
bgp4_peer_sock_send(
     tBGP4_PEER *p_peer, 
     u_char *p_msg, 
     u_int len)
{
    u_char addrstr[64];
    u_char errstr[64];
    int check_error = 0;

    if (gbgp4.work_mode == BGP4_MODE_SLAVE)
    {
        bgp4_log(BGP_DEBUG_TCP, "bgp work mode is slave do not send msg to socket!");
        return VOS_OK;
    }
                
    if (p_peer->sock <= 0) 
    {
        bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"peer sock invalid,do not send");
        return VOS_OK;
    }
    
    gbgp4.stat.sock.tx++;
    switch (p_msg[18]){
        case BGP4_MSG_KEEPALIVE:
             gbgp4.stat.msg_tx.keepalive++;
             break;
             
        case BGP4_MSG_UPDATE:
             gbgp4.stat.msg_tx.update++;
             break;
             
        case BGP4_MSG_NOTIFY:
             gbgp4.stat.msg_tx.notify++;
             break;
             
        case BGP4_MSG_OPEN:
             gbgp4.stat.msg_tx.open++;
             break;
             
        default:
             break;
    }
    gbgp4.stat.msg_tx.bytes += len;
    if (send(p_peer->sock, p_msg, len, 0) == VOS_ERR) 
    {  
        int error = errnoGet();
        check_error++;
        bgp4_printf_syserror(error,errstr);
        gbgp4.stat.sock.tx_fail++;

        bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"send packet error %s,peer %s, type %d,len %d",errstr, 
            bgp4_printf_peer(p_peer, addrstr), 
            p_msg[18], len);
        
        bgp4_fsm(p_peer, BGP4_EVENT_FATAL_ERROR);
        return VOS_ERR;
    }
    p_peer->stat.tx_msg++;
    bgp4_log(BGP_DEBUG_TCP, "send data to %s, socket %d,size %d",bgp4_printf_peer(p_peer, addrstr),p_peer->sock,len);

    if (gbgp4.debug & BGP_DEBUG_PKT)
    {
        bgp4_debug_packet(p_msg, len);
    }     
    return VOS_OK;
}

/*notify msg r&t*/
void 
bgp4_notify_output_no_peer(int sock)
{
    u_char buf[BGP4_MAX_MSG_LEN];
    tBGP4_MSGHDR *p_msg = (tBGP4_MSGHDR *)buf; 
    tBGP4_NOTIFYMSG *p_notify = NULL; 
    u_int len = BGP4_HLEN ;
        
    bgp4_init_msg_hdr(buf, BGP4_MSG_NOTIFY);

    p_notify = (tBGP4_NOTIFYMSG *)(buf + BGP4_HLEN);
    p_notify->code = BGP4_CEASE;
    p_notify->sub_code = BGP4_CONNECTION_REJECTED;    
    len += 2;
    p_msg->len =  htons(len);
    send(sock, buf, len, 0);
    return;
}


/*recv data from peer socket
 result,VOS_OK--a full packet rxd,
 VOS_ERR-some error detected or packet not rxd*/ 
int 
bgp4_peer_sock_recv(tBGP4_PEER *p_peer)
{
    u_short msglen = 0;
    u_char *p_msg = gbgp4.p_rxbuf;
    int  s  = 0;
    int readlen = 0;
    int rxlen = 0;
    int flag = 0;
    int would_block = 0;
    int buflen = 0;
    int error = 0;
    u_char estr[64];   
    u_char astr[64];
    u_char sstr[64];

#if !defined(WIN32)
    flag = MSG_DONTWAIT;
    would_block = EWOULDBLOCK;
#else
    flag = 0;
    would_block = WSAEWOULDBLOCK;
#endif

    if ((p_peer->admin_state != BGP4_PEER_RUNNING) 
        || (p_peer->state < BGP4_NS_OPENSENT))
    {
        bgp4_log(0,"peer %s state is %s,do not recieve data",
            bgp4_printf_peer(p_peer, astr),
            bgp4_printf_state(p_peer->state,sstr));
        return VOS_ERR;
    } 
    
    s = p_peer->sock;
    if (s <= 0)
    {
        bgp4_log(BGP_DEBUG_TCP,"invalid peer socket %d",s);
        return VOS_ERR;
    }    

    if (p_peer->rx_len >= BGP4_MAX_MSG_LEN)
    {
        bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"invalid current buf length");
        return VOS_ERR;
    }

     /*copy current msg to rxbuf*/
    if (p_peer->rx_len && p_peer->p_rxbuf)
    {
        memcpy(p_msg, p_peer->p_rxbuf, p_peer->rx_len);
        buflen = p_peer->rx_len;
    }

    /*recv data as many as possible*/
    rxlen = recv(s, p_msg + buflen, BGP4_RXBUF_LEN - buflen, flag);
    if (rxlen <= 0)
    {
        error = errnoGet();
        if ((error != would_block) && error)
        {
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"receive from %s error,socket %d-%s,length %d",
                bgp4_printf_peer(p_peer, astr),s,
                bgp4_printf_syserror(error, estr),
                rxlen);
            
            gbgp4.stat.sock.rx_fail++;
            bgp4_fsm(p_peer, BGP4_EVENT_TCP_CLOSED);
        }
        /*nothing to rxd,remain current msg buf*/
        return VOS_ERR;    
    }  
    
    bgp4_log(BGP_DEBUG_TCP,"socket receive from %s,length %d",
        bgp4_printf_peer(p_peer, astr), rxlen);

    /*release current rxd buf*/
    bgp4_peer_rxmsg_reset(p_peer);
    
    /*something read,update length*/
    buflen += rxlen;
    
    /*try to process multiple message in rxbuf*/
    for (readlen = 0; 
         buflen > 0; 
         p_msg += msglen, 
         readlen += msglen,
         buflen -= msglen)
    {
        /*if rest length do not include header,stop*/
        if (buflen < BGP4_HLEN)
        {
            break;
        }
        /*now,we got message header,we need read message length from buffer*/          
        bgp4_get_2bytes(msglen, (p_msg + BGP4_MARKER_LEN));
        
        /*validate message length*/          
        if ((msglen > BGP4_MAX_MSG_LEN) || (msglen < BGP4_HLEN))
        {
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"invalid message length %d", msglen); 

            p_peer->notify.code = BGP4_MSG_HDR_ERR;
            p_peer->notify.sub_code = BGP4_BAD_MSG_LEN;
            p_peer->notify.len = 2; 
            msglen = htons(msglen);
            memcpy(p_peer->notify.data, &msglen, 2);
            bgp4_notify_output(p_peer);
            return VOS_ERR;
        }

        /*full msg not rxd,wait for next read*/
        if (buflen < msglen)
        {
            break;
        } 

        bgp4_log(BGP_DEBUG_TCP, "msg received from %s,length %d",
            bgp4_printf_peer(p_peer, astr), msglen);
        
        if (gbgp4.debug & BGP_DEBUG_PKT)
        {
            bgp4_debug_packet(p_msg, msglen);
        }
        gbgp4.stat.msg_rx.bytes += msglen;
        
        /*current data contain the full msg,process it*/
        bgp4_msg_input(p_peer, p_msg);
    }
        
    /*if some msg fragment remained,store them into peer rxbuf*/ 
    if (buflen > 0)
    {
        p_peer->p_rxbuf = bgp4_malloc(buflen, MEM_BGP_RXMSG);
        if (p_peer->p_rxbuf)
        {
            memcpy(p_peer->p_rxbuf, p_msg, buflen);
            p_peer->rx_len = buflen;
        }
        else
        {
            bgp4_fsm(p_peer, BGP4_EVENT_TCP_CLOSED);
            return VOS_ERR;
        }
    }    
    return VOS_OK;
}

/*check accept data*/
void 
bgp4_server_sock_accept(u_int uiVrf)  
{
    int s;
    int server = gbgp4.server_sock[uiVrf] ;
    unsigned int len;
    u_char sstr[64];
    u_char astr[64];
    int optval, on;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_ADDR ip;
    struct sockaddr_in sa;
    struct sockaddr_in  sa_local;
    u_int buflen = BGP4_TCP_BUFFER_LEN;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    memset(&sa,0,sizeof(sa));
    len = sizeof(sa);     

    if (server <= 0)
    {
        return;
    }
    
    while (1)
    {
        s = accept(server, (struct sockaddr *)&sa, &len) ;
        if (s <= 0)
        {
            return;
        }
        inet_ntoa_1(astr,ntohl(sa.sin_addr.s_addr));
        bgp4_log(BGP_DEBUG_TCP,"accept connection %d,from %s ", 
                s,astr);
                
        /*need to check ipv6 or ipv4 address*/      
        memset(&ip, 0, sizeof(ip));
        
        if (sa.sin_family == AF_INET)
        {
            bgp_ip4(ip.ip) = sa.sin_addr.s_addr;/*network order*/
            ip.afi = BGP4_PF_IPUCAST;
            ip.prefixlen = 32;
        }

        /*TODO:get vrf id from accept sock data*/
    #ifndef WIN32
        bgp_sem_take();
        p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, uiVrf);
        if (p_instance == NULL)
        {
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"can not find such instance %d",uiVrf);
        
            bgp4_notify_output_no_peer(s);
            bgp4_tcp_close_socket(s);
            bgp_sem_give();
            continue;
        }
        
        p_peer = bgp4_peer_lookup(p_instance, &ip);
        if (p_peer == NULL) 
        {
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"vrf %d no such peer,discard connection",uiVrf);

            bgp4_notify_output_no_peer(s);
            bgp4_tcp_close_socket(s);
            /*we should close the fd,but n2x said we wrong*/
            bgp_sem_give();
            continue;
        }
    #else
        bgp4_instance_for_each(p_instance)
        {
            p_peer = bgp4_peer_lookup(p_instance, &ip);
            if (p_peer)
            {
                break;
            }
        }
        if (p_peer == NULL) 
        {
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"vrf %d no such peer,discard connection",uiVrf);

            bgp4_notify_output_no_peer(s);
            bgp4_tcp_close_socket(s);
            /*we should close the fd,but n2x said we wrong*/
            bgp_sem_give();
            continue;
        }
    #endif
        if (p_peer->unsupport_capability == TRUE)
        {
            bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"peer has unsupported capability,discard connection");
            bgp4_tcp_close_socket(s);
            bgp_sem_give();
            continue;
        }    
         
        /*if update source is configured ,need compare it*/
        if (p_peer->update_source.prefixlen == 32)
        {                         
            memset(&sa_local, 0, sizeof(sa_local));
            if (getsockname(s, ((struct sockaddr *)&sa_local), &len) == VOS_OK) 
            {
                if (memcmp(p_peer->update_source.ip, &sa_local.sin_addr.s_addr,4) != 0)
                {
                    inet_ntoa_1(astr, (sa_local.sin_addr.s_addr));
                    bgp4_log(BGP_DEBUG_TCP|BGP_DEBUG_ERROR,"different local address %s",astr);
                    bgp4_tcp_close_socket(s);
                    bgp_sem_give();
                    continue;
                }
            }
        }
         
        on = 1;
        bgp4_set_sock_noblock(s, on);
         
        optval = 1;
        bgp4_set_sock_tcpnodelay(s, optval);
        
        /*rx&tx buf length*/
        bgp4_set_sock_rxbuf(s, buflen);  
        bgp4_set_sock_txbuf(s, buflen);  

        if (p_peer->md5_support) 
        {
            bgp4_set_sock_md5_support(s, 1);
            bgp4_set_sock_md5_key(s, 1, p_peer);
        }
        bgp4_tcp_set_peer_ttl(s, p_peer);
        
        bgp4_log(BGP_DEBUG_TCP,"find expected peer,state %s",
                 bgp4_printf_state(p_peer->state, sstr));
        
        if (p_peer->admin_state != BGP4_PEER_RUNNING) 
        {
            bgp4_log(BGP_DEBUG_TCP,"close new socket %d,peer is disabled",s);
            bgp4_tcp_close_socket(s);
            bgp_sem_give();
            continue ;
        }                           
         
        switch (p_peer->state) {
            case BGP4_NS_IDLE:
            {
                bgp4_timer_stop(&p_peer->connect_timer);
                bgp4_timer_start(&p_peer->retry_timer, p_peer->retry_interval);

                p_peer->state = BGP4_NS_ACTIVE;
                
            }/* Fall through */              
            case BGP4_NS_ACTIVE:
            {
                bgp4_log(BGP_DEBUG_TCP,"accpet new socket %d",s);
                /*close current socket*/ 
                bgp4_peer_sock_close(p_peer);
                p_peer->sock = s;
                bgp4_peer_tcp_addr_fill(p_peer);
                bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
                break;
            }
             
            case BGP4_NS_CONNECT:
            case BGP4_NS_OPENSENT:
            case BGP4_NS_OPENCONFIRM:
            {
                /*if peer's router id is known,use active connection
                 opened by larger id*/
                if (p_peer->router_id > 0)
                {
                    /*if local id larger,close new accepted socket*/
                    if (gbgp4.router_id > p_peer->router_id)
                    {
                        bgp4_log(BGP_DEBUG_TCP,
                        "local router id is larger,close new socket %d", s);
                        
                        bgp4_tcp_close_socket(s);
                        break;
                    }
                    /*aeept new socket*/
                }
                else
                {
                    /*peer's id is unkown,compare ip address,select larger
                      one*/
                    memset(&sa_local, 0, sizeof(sa_local));
                    if(getsockname(s, ((struct sockaddr *)&sa_local), &len) != VOS_OK)
                    {
                        bgp4_log(BGP_DEBUG_TCP,"get socket %d info err", s);
                    }
                    if (memcmp(&sa_local.sin_addr.s_addr, p_peer->ip.ip, 4) > 0)
                    {
                        bgp4_log(BGP_DEBUG_TCP,
                        "local socket addr is larger,close new socket %d", s);
                        
                        bgp4_tcp_close_socket(s);
                        break;
                    }
                    /*aeept new socket*/
                }
                
                if ((p_peer->state == BGP4_NS_OPENSENT)
                    || (p_peer->state == BGP4_NS_OPENCONFIRM))
                {
                    bgp4_log(BGP_DEBUG_TCP,
                        "close existing socket %d,use new accepted %d",
                        p_peer->sock,s);

                    p_peer->notify.code = BGP4_CEASE;
                    p_peer->notify.sub_code = BGP4_CONNECTION_COLLISION_RESOLUTION;
                    bgp4_notify_output(p_peer);
                     
                    bgp4_tcp_close_socket(p_peer->sock);                    
                    p_peer->sock = BGP4_INVALID_SOCKET;
                    if (p_peer->state == BGP4_NS_OPENSENT)
                    {
                        bgp4_timer_stop(&p_peer->hold_timer);
                    }
                } 
                if (p_peer->sock > 0) 
                {
                    bgp4_log(BGP_DEBUG_TCP,
                        "close current socket %d,use new accepted %d",
                        p_peer->sock,s);
                    
                    bgp4_tcp_close_socket(p_peer->sock); 
                    p_peer->sock = BGP4_INVALID_SOCKET;
                }
                p_peer->sock = s;
                bgp4_peer_tcp_addr_fill(p_peer);
                bgp4_timer_start(&p_peer->retry_timer, p_peer->retry_interval);
             
                p_peer->state = BGP4_NS_ACTIVE ;
                bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
                break;
            }
            case BGP4_NS_ESTABLISH:
            {
                /*for GR comformance test,TCP reset*/
                /*if local restart not supported,reject it*/
                if ((gbgp4.restart_enable == FALSE)
                    || (p_peer->restart_enable == FALSE))
                {
                    bgp4_log(BGP_DEBUG_TCP, "restart not supported,close new socket %d", s);
                    
                    bgp4_tcp_close_socket(s);
                    break;
                }

                bgp4_log(BGP_DEBUG_TCP,"close established socket %d,use new accepted %d",
                        p_peer->sock,s);

                if (p_peer->sock > 0) 
                {
                    bgp4_tcp_close_socket(p_peer->sock); 
                    p_peer->sock = BGP4_INVALID_SOCKET;
                }
                p_peer->sock = s;
                bgp4_peer_tcp_addr_fill(p_peer);/*keep the state until receiving open msg*/
                
                /*remote close action,peer may be in restarting*/
                bgp4_remote_grace_restart(p_peer);

                bgp4_timer_start(&p_peer->retry_timer, p_peer->retry_interval); 
                
                p_peer->state = BGP4_NS_ACTIVE ;
                bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
                
                break; 
            }
            default :
                break;
        }
        bgp_sem_give();
    }
    return;
}

void 
bgp4_ip6_server_accept(u_int uiVrf)  
{
    int s = 0;
    int server = gbgp4.server_sock6[uiVrf];
    unsigned int len;
    u_char sstr[64];
    int optval, on;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_ADDR ip;
    u_char *ipv4_flag="::ffff:";
    u_char ipv6_str[64];
    struct sockaddr_in6 sa;
    struct sockaddr_in6 sa_local;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    
    if (server <= 0)
    {
        return;
    }

    memset(&sa,0,sizeof(sa));
    len = sizeof(sa);
    while (1)
    {
        s = accept(server, (struct sockaddr *)&sa, &len);
        if (s < 0)
        {
            return;
        }
        bgp4_log(BGP_DEBUG_TCP,"accept connection %d,from %s ", 
                    s, inet_ntop(AF_INET6, &sa.sin6_addr, ipv6_str, 64));

        /*need to check ipv6 or ipv4 address*/      
        memset(&ip, 0, sizeof(ip));        
        memset(ipv6_str, 0, 64);
        inet_ntop(AF_INET6, &sa.sin6_addr, ipv6_str, 64);
                
        if (strstr(ipv6_str,  ipv4_flag))
        {
            ip.afi = BGP4_PF_IPUCAST;
            bgp_ip4(ip.ip) = *(u_int *)(((u_char *)&sa.sin6_addr) + 12);
            ip.prefixlen = 32;
        }
        else
        {
            ip.afi = BGP4_PF_IP6UCAST;
            memcpy(&ip.ip, &sa.sin6_addr, sizeof(sa.sin6_addr));
            ip.prefixlen = 128;
        }
        
        bgp_sem_take();
        p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, uiVrf);
        if (p_instance == NULL)
        {
            bgp4_notify_output_no_peer(s);
			bgp4_tcp_close_socket(s); 
            bgp4_log(BGP_DEBUG_TCP,"can not find such instance %d",uiVrf);
            bgp_sem_give();
            continue;
        }
        
        p_peer = bgp4_peer_lookup(p_instance, &ip);        
        if (p_peer == NULL) 
        {
            bgp4_log(BGP_DEBUG_TCP,"vrf %d no such peer,discard connection %d",p_instance->vrf,s);
            bgp4_tcp_close_socket(s);
            bgp_sem_give();
            continue;
        }

        /*add for cap advertisement,by zxq*/
        if(p_peer->unsupport_capability == TRUE)
        {
            bgp4_log(BGP_DEBUG_TCP,"peer has unsupported capability,discard connection %d",s);
            bgp4_tcp_close_socket(s); 
            bgp_sem_give();
            continue;
        }    
         
        /*if update source is configured ,need compare it*/
        if (p_peer->update_source.prefixlen == 128)
        {     
            memset(&sa_local, 0, sizeof(sa_local));
            if (getsockname(s, ((struct sockaddr *)&sa_local), &len) == VOS_OK) 
            {                
                if (memcmp(p_peer->update_source.ip, &sa_local.sin6_addr, 16) != 0)
                {
                    u_char addr[64] = {0};
                    bgp4_log(BGP_DEBUG_TCP,"different local address %s,socket %d",
                                bgp4_printf_addr(&p_peer->update_source,addr),
                                s);
                    bgp4_tcp_close_socket(s); 
                    bgp_sem_give();
                    continue;
                }
            }
        }
         
        on = 1;
        bgp4_set_sock_noblock(s, on);
         
        optval = 1;
        bgp4_set_sock_tcpnodelay(s, optval);

        if (p_peer->md5_support) 
        {
            bgp4_set_sock_md5_support(s, 1);
            bgp4_set_sock_md5_key(s, 1, p_peer);
        }
        
        bgp4_tcp_set_peer_ttl(s, p_peer);

        bgp4_log(BGP_DEBUG_TCP,"find expected peer state %s",
                bgp4_printf_state(p_peer->state,sstr));

        if (p_peer->admin_state != BGP4_PEER_RUNNING) 
        {
            bgp4_log(BGP_DEBUG_TCP,"close new socket %d,peer is disabled",s);
            bgp4_tcp_close_socket(s);
            bgp_sem_give();
            continue ;
        }                           
         
        switch (p_peer->state) {
            case BGP4_NS_IDLE:
            {
                bgp4_timer_stop(&p_peer->connect_timer);     
                bgp4_timer_start(&p_peer->retry_timer, p_peer->retry_interval);
                p_peer->state = BGP4_NS_ACTIVE ;
            }/* Fall through */
                
            case BGP4_NS_ACTIVE:
            {
                /*close current socket*/ 
                bgp4_peer_sock_close(p_peer);
                p_peer->sock = s;
                bgp4_peer_tcp_addr_fill(p_peer);
                bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
                break;
            }
             
            case BGP4_NS_CONNECT:
            case BGP4_NS_OPENSENT:
            case BGP4_NS_OPENCONFIRM:
            {
                /*if peer's router id is known,use active connection
                 opened by larger id*/
                if (p_peer->router_id > 0)
                {
                    /*if local id larger,close new accepted socket*/
                    if (gbgp4.router_id > p_peer->router_id)
                    {
                        bgp4_log(BGP_DEBUG_TCP,
                        "local router id is larger,close new socket %d", s);
                        
                        bgp4_tcp_close_socket(s);
                        break;
                    }
                    /*aeept new socket*/
                }
                else
                {
                    if (memcmp(&sa_local.sin6_addr, p_peer->ip.ip, 16) > 0)
                    {
                        bgp4_log(BGP_DEBUG_TCP,
                        "local socket addr is larger,close new socket %d", s);
                        
                        bgp4_tcp_close_socket(s);
                        break;
                    }
                    /*aeept new socket*/
                }
                
                if (p_peer->state == BGP4_NS_OPENSENT || 
                    p_peer->state == BGP4_NS_OPENCONFIRM) 
                {
                    bgp4_log(BGP_DEBUG_TCP,
                        "close existing socket %d,use new accepted %d",
                        p_peer->sock,s);
                 
                    /*close current socket*/ 
                    bgp4_peer_sock_close(p_peer);
                    p_peer->sock = BGP4_INVALID_SOCKET;
                    if (p_peer->state == BGP4_NS_OPENSENT)
                    {
                        bgp4_timer_stop(&p_peer->hold_timer);
                    }
                } 
                /*close current socket*/ 
                bgp4_peer_sock_close(p_peer);
                p_peer->sock = s;
                bgp4_peer_tcp_addr_fill(p_peer);
                bgp4_timer_start(&p_peer->retry_timer, p_peer->retry_interval);

                p_peer->state = BGP4_NS_ACTIVE;
                bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
                break;
               }
            case BGP4_NS_ESTABLISH:
            {
                bgp4_log(BGP_DEBUG_TCP,"close new socket %d,peer is up",s);
             
                bgp4_tcp_close_socket(s); 
                break; 
            }
            default :
                break;
        }
        bgp_sem_give();
    }
    return ;
}

/*check all socket's state,decide connection and data*/
void 
bgp4_tcp_socket_recv(
       fd_set *p_rfd,
       fd_set *p_wfd)
{
    tBGP4_PEER *p_peer = NULL;
    struct sockaddr sa = {0};
    int  len=sizeof(struct sockaddr);
    char peerstr[64];
    u_int rx_count = 0 ;
    u_int time_start = 0 ;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int uiErr = 0;
    int i = 0;

    for(i = 0; i < L3_VPN_INSTANCE_NUM + 1; i++)
    {
        if (gbgp4.server_sock[i] > 0 && FD_ISSET(gbgp4.server_sock[i], p_rfd))
        {
            bgp4_server_sock_accept(i);
        }
            
        if (gbgp4.server_sock6[i] > 0 && FD_ISSET(gbgp4.server_sock6[i], p_rfd))
        {
            bgp4_ip6_server_accept(i);
        }
    }

    time_start = vos_get_system_tick();
    bgp_sem_take();
    bgp4_instance_for_each(p_instance)
    {
        bgp4_peer_for_each(p_instance, p_peer) 
        {
            if (p_peer->sock <= 0)
            {
                continue;
            }
            if (FD_ISSET(p_peer->sock, p_rfd))
            {
                gbgp4.stat.sock.rx++;
                
                rx_count = 0 ;
                /*rx multiple packet*/
                while (bgp4_peer_sock_recv(p_peer) == VOS_OK)
                {
                    rx_count++;
                    if (rx_count >= 32)
                    {
                        break;
                    }
                }
            }
            else if (FD_ISSET(p_peer->sock, p_wfd))
            {        
                if(VOS_OK == getsockopt(p_peer->sock, SOL_SOCKET, SO_ERROR, &uiErr, &len) && uiErr == 0)
                {                        
                    /*connect ok*/
                    bgp4_log(BGP_DEBUG_TCP,"In task connect socket %d to %s VOS_OK",
                            p_peer->sock, 
                            bgp4_printf_peer(p_peer, peerstr));

                    p_peer->connect_inprogress = 0 ;
                    bgp4_peer_tcp_addr_fill(p_peer);
                                        
                    bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
                }
                else
                {
                    bgp4_peer_sock_close(p_peer);
                    bgp4_fsm(p_peer, BGP4_EVENT_CONNECT_FAIL);
                }
            }
        }
    }
    bgp_sem_give();
    return;        
}
#else
#include "bgp4com.h"
#if !defined(USE_LINUX) && !defined(WIN32)
#include "tcp.h" 
#endif
#if !defined(WIN32)



#endif

u_int
bgp4_ip2ifunit(u_int vrf_id,tBGP4_ADDR *p_ip)
{
    u_int addr_type;
    u_int if_unit = 0;

    addr_type =  (p_ip->afi == BGP4_PF_IPUCAST) ? AF_INET : AF_INET6;

    /* 获取接口unit */
    if(uspGetIfUnitByIp(vrf_id, addr_type, p_ip->ip, &if_unit) == VOS_ERR)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 ip2 interface unit get interface unit failed,vrf_id %d\n\r",vrf_id);
    }

    return if_unit;
}

/*get tcp port and address for peer, need merge to ipv6*/
void bgp4_fill_tcp_address(tBGP4_PEER *p_peer)
{
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    int len;

    p_peer->local.port = 0;
    bgp_ip4(p_peer->local.ip.ip) = 0;
    p_peer->remote.port = 0 ;                 

    if(p_peer->sock <= 0)
    {
        return;
    }
    

    if(p_peer->remote.ip.afi==BGP4_PF_IPUCAST)
    {
        len = sizeof(sa);
        if (getsockname(p_peer->sock,((struct sockaddr *)&sa), &len) == VOS_ERR) 
        {
            bgp4_log(BGP_DEBUG_EVT,1,"get local name for socket failed");
            return;
        }
        p_peer->local.port = ntohs(sa.sin_port);
        bgp_ip4(p_peer->local.ip.ip) = ntohl(sa.sin_addr.s_addr);  
        p_peer->local.ip.afi=BGP4_PF_IPUCAST;
        p_peer->local.ip.prefixlen=4;
        if (getpeername(p_peer->sock, (struct sockaddr *)&sa, &len) == VOS_ERR) 
        {
            bgp4_log(BGP_DEBUG_EVT,1,"get remote name for socket failed");
            return;
        }
        
        p_peer->remote.port = ntohs(sa.sin_port);
    }

    else if(p_peer->remote.ip.afi==BGP4_PF_IP6UCAST)
    {
        len=sizeof(sa6);

        if (getsockname(p_peer->sock,((struct sockaddr *)&sa6), &len) == VOS_ERR) 
        {
            bgp4_log(BGP_DEBUG_EVT,1,"get local name for socket failed");
            return;
        }
        memset(p_peer->local.ip.ip,0,16);
        memcpy(p_peer->local.ip.ip,&(sa6.sin6_addr),16);     
        p_peer->local.port = ntohs(sa6.sin6_port);
        p_peer->local.ip.afi=BGP4_PF_IP6UCAST;
        p_peer->local.ip.prefixlen=16;
        
        if (getpeername(p_peer->sock, (struct sockaddr *)&sa6, &len) == VOS_ERR) 
        {
            bgp4_log(BGP_DEBUG_EVT,1,"get remote name for socket failed");             
            return;
        }           

        p_peer->remote.port = ntohs(sa6.sin6_port);
    }
    
    /*get if unit number*/
    p_peer->if_unit = bgp4_ip2ifunit(p_peer->p_instance->instance_id,&p_peer->local.ip);
    
    return;
}

int
bgp4_set_sock_noblock(
                         int s,
                         int on)
{
    if(s<=0)
    {
        return VOS_ERR;
    }
#if !defined(WIN32)    
    return ioctl(s, FIONBIO, (int)&on);
#else
    return ioctlsocket(s, FIONBIO, (int)&on);
#endif
}

int
bgp4_set_sock_tcpnodelay(
                         int s,
                         int on)
{
    if(s<=0)
    {
        return VOS_ERR;
    }
#ifndef USE_LINUX       
    return setsockopt (s, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof (on));
#endif
}

int
bgp4_set_sock_rxbuf(
                         int s,
                         int on)
{
    if(s<=0)
    {
        return VOS_ERR;
    }
    return setsockopt (s, SOL_SOCKET, SO_RCVBUF, (char *)&on, sizeof(on)) ;
}

int 
bgp4_set_sock_txbuf(
                         int s,
                         int on)
{
    if(s<=0)
    {
        return VOS_ERR;    
    }
    return setsockopt (s, SOL_SOCKET, SO_SNDBUF, (char *)&on, sizeof(on)) ;
}

int
bgp4_set_sock_reuseaddr(
                         int s,
                         int on)
{
    if(s<=0)
    {
        return VOS_ERR;
    }
#ifndef USE_LINUX       
    return setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) ;
#endif
}

int 
bgp4_set_sock_md5(
                         int s,
                         int on)
{
#if !defined(USE_LINUX) && !defined(WIN32) 
    int rc = VOS_ERR;

    if(s<=0)
    {
        return VOS_ERR;
    }
     #if 0 /*TODO:需要在ipcom中添加该选项*/
    rc = setsockopt (s,IPPROTO_TCP,TCP_MD5SIG,(char*)&on,sizeof(on));
    #endif    
    bgp4_log(BGP_DEBUG_EVT,1,"Turn socket %d md5 signature on,ret %d",s,rc);

    return rc;
#else
    return VOS_OK;
#endif
}

int
bgp4_set_sock_md5_key(
                          int s,
                          int on,
                          u_char *p_key,
                          u_char len,
                          u_char pf,
                          tBGP4_ADDR *p_addr
                      )
{
#if !defined(USE_LINUX) && !defined(WIN32) 
    struct md5Key key;
    int rc = VOS_ERR;
    u_char addr[64] = {0};

    if(s<=0)
    {
        return VOS_ERR;

    }
    
    memset(&key, 0, sizeof(struct md5Key));
    key.keyOnoff = on;
    key.keyLen = len;
    memcpy(key.keyData, p_key, len);

    if (pf == BGP4_PF_IPUCAST)
    {
        key.keyFamily = AF_INET;
        key.addr.keyAddr = bgp_ip4(p_addr->ip);
    }
    else if (pf == BGP4_PF_IP6UCAST)
    {
        key.keyFamily = AF_INET6;
        memcpy(key.addr.keyAddr6, p_addr->ip, 16);
    }
    
    rc = setsockopt(s, SOL_SOCKET, SO_MD5KEY, (char *) &key, sizeof (key));

    inet_ntop(AF_INET6, key.addr.keyAddr6, addr, 64);
   
    bgp4_log(BGP_DEBUG_EVT,1,"set socket %d md5 key,key af %d,address %s,ret %d",
                s,
                key.keyFamily,
                addr,
                rc);

    return rc;
 #else
    return TRUE;
 #endif
}

void 
bgp4_tcp_set_peer_md5(
                         int s,
                         tBGP4_PEER *p_peer)
{
    if(s<=0)
    {
        return ;
    }
    
    if (p_peer->md5_support) 
    {
        bgp4_set_sock_md5(s, 1);
        bgp4_set_sock_md5_key(s, 1, p_peer->md5key, p_peer->md5key_len, p_peer->remote.ip.afi, &p_peer->remote.ip);
    }
    
    return ;
}
void
bgp4_tcp_set_peer_ttl(
                         int s,
                         tBGP4_PEER *p_peer)
{
#if !defined(USE_LINUX) && !defined(WIN32) 
    int maxttl = 255;
    struct inMinttl ttl_bgp; 

    if(s>0)
    {
        if(p_peer->ttl_hops>0&&p_peer->ttl_hops<256)
        {
            ttl_bgp.onOff=1;
        }
        else if(p_peer->ttl_hops==0)
        {
            ttl_bgp.onOff=0;
        }

        ttl_bgp.minttl=255-p_peer->ttl_hops+1;
        if (p_peer->remote.ip.afi == BGP4_PF_IPUCAST)
        {
            ttl_bgp.family = AF_INET;
            ttl_bgp.ipaddr.addr = bgp_ip4(p_peer->remote.ip.ip);
        }
        else if (p_peer->remote.ip.afi == BGP4_PF_IP6UCAST)
        {
            ttl_bgp.family = AF_INET6;
            memcpy(ttl_bgp.ipaddr.addr6, p_peer->remote.ip.ip, 16);
        }
        setsockopt(s, SOL_SOCKET, SO_INMINTTL, (char *)&ttl_bgp,sizeof(ttl_bgp));
        setsockopt(s, IPPROTO_IP, IP_TTL,  (char *)&maxttl, sizeof (maxttl)); 
        
        bgp4_log(BGP_DEBUG_EVT,1,"set peer ttl socket %d valid ttl [%d,%d]",s,ttl_bgp.minttl,maxttl);

    }
 #endif
    return ;
}

void bgp4_tcp_close_socket(int s)
{
    if(s < 0)
    {
        return;
    }
#if !defined(WIN32)
    close (s);
#else
    closesocket (s);
#endif    
}
/*open connection to neighbor,*/
int bgp4_sock_open(tBGP4_PEER *p_peer)
{
    struct sockaddr_in sa;
#ifdef BGP_IPV6_WANTED
    struct sockaddr_in6 sa6;
#endif
    struct sockaddr *p_sa = NULL;

    int salen = 0;
    int s = 0;
    u_char addr[64];
    u_char errstr[64];
    int optval = 0;
    int on = 1;
    u_int buflen = BGP4_TCP_BUFFER_LEN;
    
    
    if(bgp4_get_workmode() == BGP4_MODE_SLAVE)
    {
        bgp4_log(BGP_DEBUG_TCP,1,"bgp4 sock open,slave card do not open sock!");
        return TRUE;
    }

    if(p_peer->p_instance == NULL)
    {
        return FALSE;
    }
    
#ifdef BGP_BFD_WANTED
        if ((p_peer->bfd_enable)&&((p_peer->state) != BGP4_NS_ESTABLISH))
        {
            bgp4_unbind_bfd(p_peer);
        }
#endif

    /*do not act as client*/     
    if (!gBgp4.active_open || p_peer->noactive_open)
    {
        bgp4_log(BGP_DEBUG_TCP,1,"active tcp connection is disabled");
        return FALSE;
    } 

    p_peer->rxmsg.len = 0 ;
    
    gBgp4.stat.sock++;
    
    /*create socket*/
    if(p_peer->remote.ip.afi==BGP4_PF_IPUCAST)
        s = socket(AF_INET, SOCK_STREAM, 0);
    
#ifdef BGP_IPV6_WANTED
    else if(p_peer->remote.ip.afi==BGP4_PF_IP6UCAST)
        s = socket(AF_INET6, SOCK_STREAM, 0);
#endif

    if (s == VOS_ERR)
    { 
        gBgp4.stat.sockfail++;
            bgp4_printf_syserror(errnoGet(),errstr);
        bgp4_log(BGP_DEBUG_TCP,1,"open socket to %s failed %s",
            bgp4_printf_peer(p_peer, addr),
            bgp4_printf_syserror(errnoGet(),errstr));
        return (FALSE);
    }
    
    /*if update source is not zero,use the assigned value,ipv4 support now*/
    if ((p_peer->update_source.prefixlen) == 32)
    {
        memset(&sa, 0, sizeof(sa)); 
        
        sa.sin_family = AF_INET;        
        sa.sin_port = 0;
        memcpy(&sa.sin_addr.s_addr,p_peer->update_source.ip,4);
        
        gBgp4.stat.bind++;
        if (bind(s,(struct sockaddr *)&sa,sizeof(sa)) < 0)
        {
            bgp4_printf_syserror(errnoGet(),errstr);     
            bgp4_log(BGP_DEBUG_TCP,1,"bind peer %s socket %d failed %s",
                bgp4_printf_peer(p_peer, addr),
                s,
                errstr);

            gBgp4.stat.bindfail++;
            bgp4_fsm(p_peer, BGP4_EVENT_CONNECT_FAIL);
            bgp4_tcp_close_socket(s);
            return (TRUE);
        }
    }

#ifdef BGP_IPV6_WANTED
    else if(p_peer->update_source.prefixlen == 128)
    {
        memset(&sa6, 0, sizeof(sa6));

        sa6.sin6_family = AF_INET6;
        sa6.sin6_port = 0;
        memcpy(&sa6.sin6_addr,p_peer->update_source.ip,16);
        
        gBgp4.stat.bind++;
        if (bind(s,(struct sockaddr *)&sa6,sizeof(sa6)) < 0)
        {
            bgp4_printf_syserror(errnoGet(),errstr);     
            bgp4_log(BGP_DEBUG_TCP,1,"bind socket %d failed %s",
                        s,
                        bgp4_printf_addr(&p_peer->update_source,addr),
                        errstr);

            gBgp4.stat.bindfail++;
            bgp4_fsm(p_peer, BGP4_EVENT_CONNECT_FAIL);
            if(s>0)
                close (s);

            return (TRUE);
        }
    }
#endif
    
    /*io noblk*/
    on = 1;
    bgp4_set_sock_noblock(s,on);
        
    /*tcp no delay*/
    optval = 1;
    bgp4_set_sock_tcpnodelay(s,optval);

    /*rx&tx buf length*/
    bgp4_set_sock_rxbuf(s,buflen);  
    bgp4_set_sock_txbuf(s,buflen);  

    bgp4_tcp_set_peer_md5(s, p_peer);    

    /*ttl set*/
    bgp4_tcp_set_peer_ttl(s, p_peer);
    
    gBgp4.stat.connect++;

    if (p_peer->remote.ip.afi ==BGP4_PF_IPUCAST )
    {
        memset(&sa, 0, sizeof(sa)); 
        sa.sin_family = AF_INET;
        sa.sin_port = htons(gBgp4.client_port);
        sa.sin_addr.s_addr = htonl(bgp_ip4(p_peer->remote.ip.ip));
        p_sa = (struct sockaddr *)&sa;
        salen = sizeof(sa);
        sa.vr_id = p_peer->p_instance->instance_id;
    }
#ifdef BGP_IPV6_WANTED
    else 
    {
        memset(&sa6,0,sizeof(sa6));
        sa6.sin6_family=AF_INET6;
        sa6.sin6_port=htons(gBgp4.client_port);
        memcpy(&sa6.sin6_addr,&p_peer->remote.ip.ip,16);
        p_sa = (struct sockaddr *)&sa6;
        salen = sizeof(sa6);
        sa6.vr_id = p_peer->p_instance->instance_id;
        /*set ipv6 socket only receiving ipv6 packets*/
        #if 0 /*TODO:需要在ipcom中添加该选项*/
        setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&on, sizeof(on));
        #endif
    }
#endif

    
    bgp4_log(BGP_DEBUG_TCP,1,"connect vrf %d socket %d to %s:%d",
                    sa.vr_id,
                    s,
                    bgp4_printf_peer(p_peer, addr),
                    gBgp4.client_port);
    
    if (connect(s, p_sa, salen) < 0)
    {
        
#ifndef USE_LINUX
        int error = errnoGet () & 0xffff;
#else
        int error = errno;
#endif
#if !defined(WIN32)
        if (error == EINPROGRESS || error == EALREADY)
#else
        if (error == WSAEINPROGRESS 
                || error == WSAEALREADY
                || error == WSAEWOULDBLOCK)
#endif
        {
            p_peer->sock = s ;
            p_peer->connect_inprogress = 1;
            bgp4_printf_syserror(error,errstr);          
            bgp4_log(BGP_DEBUG_TCP,1,"connect socket %d to %s failed %s,just wait",
                        s,
                        bgp4_printf_peer(p_peer, addr),
                        errstr);
            return TRUE ;
        }
                
        bgp4_printf_syserror(error,errstr);          
        bgp4_log(BGP_DEBUG_TCP,1,"connect socket %d to %s failed %s",
                        s,
                        bgp4_printf_peer(p_peer, addr),
                        errstr);
        /*perror("Open connection");*/
        gBgp4.stat.connectfail++;
        bgp4_fsm(p_peer, BGP4_EVENT_CONNECT_FAIL);
        bgp4_tcp_close_socket(s);
        return (TRUE);
    }
    
    /*connect successfully,add to read fd*/                
    bgp4_log(BGP_DEBUG_TCP,1,"connect socket %d to %s successfully",
                    s,
                    bgp4_printf_peer(p_peer, addr));
    p_peer->sock = s;
    bgp4_fill_tcp_address(p_peer);
    p_peer->state = BGP4_NS_CONNECT;
    bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
    return TRUE ;
}

/*close socket of peer*/
int bgp4_sock_close(tBGP4_PEER *p_peer)
{
    p_peer->rxmsg.len = 0 ;

    if(gBgp4.work_mode==BGP4_MODE_SLAVE)
        return TRUE;
#if 0   
    memset(&p_peer->local.ip, 0, sizeof(tBGP4_ADDR));
#endif

    p_peer->local.port = 0 ;
    p_peer->remote.port = 0;
    p_peer->version = BGP4_VERSION_4;
    p_peer->remote.af = 0 ;
    p_peer->connect_inprogress = 0; 

    if(p_peer->sock>0)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"close peer socket:%d",p_peer->sock);
        bgp4_tcp_close_socket(p_peer->sock);
    }
    p_peer->sock = BGP4_INVALID_SOCKET;

    return (TRUE);
}

/*init server socket,an ipv6 socket can accept both ip&ip6 connection*/
int bgp4_server_open() 
{
#ifdef BGP_IPV6_WANTED
    struct sockaddr_in6 sa6;
    int s6;
#endif
    struct sockaddr_in sa;
    int s;
    int on = 1;
    u_int buflen = 81920;
    int error =0;

    if(gBgp4.server_open!=TRUE)
        return TRUE;
    
    if (gBgp4.server_sock  == BGP4_INVALID_SOCKET)        
    {
        gBgp4.stat.sock++;
        
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0)
        {  
            error=errnoGet();
            bgp4_log(BGP_DEBUG_TCP,1,"open listen port failed,errno=%d",error);
            gBgp4.stat.sockfail++;
            gBgp4.server_sock = BGP4_INVALID_SOCKET;
            return (FALSE);
        }
        bgp4_set_sock_noblock(s,on);
        memset(&sa, 0, sizeof(sa));

        sa.sin_family = AF_INET;
        sa.sin_port = htons(gBgp4.server_port);
#if !defined(USE_LINUX) && !defined(WIN32)      
        sa.sin_len = sizeof (sa);
#endif
        
            bgp4_set_sock_rxbuf(s,buflen);  
            bgp4_set_sock_txbuf(s,buflen);  
                    
        on=1;
            bgp4_set_sock_reuseaddr(s,on);
            
            on=1;
        bgp4_set_sock_md5(s, on);

        gBgp4.stat.bind++;

        if (bind (s, (struct sockaddr *) &sa, sizeof (sa)) < 0)
        {
            error=errnoGet();
            bgp4_log(BGP_DEBUG_TCP,1,"bind listen port failed,erron=%d",error);
            gBgp4.stat.bindfail++;
            gBgp4.server_sock = BGP4_INVALID_SOCKET;
            bgp4_tcp_close_socket(s);
            return (FALSE);
        }

        /*backlog==32?*/
        if (listen(s, 32) == VOS_ERR)
        {
            error=errnoGet();
            bgp4_log(BGP_DEBUG_TCP,1,"set listen port failed,erron=%d",error);
            bgp4_tcp_close_socket(s);
            gBgp4.server_sock = BGP4_INVALID_SOCKET;
            return FALSE;
        }
        gBgp4.server_sock = s;
    }
        
#ifdef BGP_IPV6_WANTED
    if(gBgp4.server_sock6  == BGP4_INVALID_SOCKET)
    {
        gBgp4.stat.sock++;
        s6 = socket(AF_INET6, SOCK_STREAM, 0);
        if (s6 < 0)
        {
            error=errnoGet();
            bgp4_log(BGP_DEBUG_TCP,1,"open listen ipv6 port failed,erron=%d",error);

            gBgp4.stat.sockfail++;
            gBgp4.server_sock6 = BGP4_INVALID_SOCKET;
            return (FALSE);
        }
        
        /*set ipv6 socket only receiving ipv6 packets*/
        #if 0 /*TODO:需要在ipcom中添加该选项*/
        setsockopt(s6, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&on, sizeof(on));   
        #endif
        bgp4_set_sock_noblock(s6,on);
        
        memset(&sa6, 0, sizeof(sa6));
        sa6.sin6_family = AF_INET6;
        sa6.sin6_port = htons(gBgp4.server_port);
#if !defined(USE_LINUX) && !defined(WIN32)      
        sa6.sin6_len = sizeof (struct sockaddr_in6); 
#endif
        bgp4_set_sock_rxbuf(s6,buflen);  
        bgp4_set_sock_txbuf(s6,buflen);  

        on=1;
        bgp4_set_sock_reuseaddr(s6,on);
        
        on=1;
        bgp4_set_sock_md5(s6, on);
        
        gBgp4.stat.bind++;

        if (bind (s6, (struct sockaddr *) &sa6, sizeof (sa6)) < 0)
        {
            error=errnoGet();
            bgp4_log(BGP_DEBUG_TCP,1,"bind listen ipv6 port failed,erron=%d",error);
            perror("Bind bgp tcp6 socket");
            gBgp4.stat.bindfail++;
            gBgp4.server_sock6 = BGP4_INVALID_SOCKET;
            bgp4_tcp_close_socket(s6);
            return (FALSE);
        }
        
        /*backlog==32?*/
        if (listen(s6, 32) == VOS_ERR)
        {
            error=errnoGet();
            bgp4_log(BGP_DEBUG_TCP,1,"set listen ipv6 port failed,erron=%d",error);
            bgp4_tcp_close_socket(s6);
            gBgp4.server_sock6 = BGP4_INVALID_SOCKET;
            return FALSE;
        }
        gBgp4.server_sock6 = s6;
    }
#endif
    return TRUE;
}

/*close listen port*/
void bgp4_server_close() 
{
    int on = 0;

    if (gBgp4.server_sock != BGP4_INVALID_SOCKET)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 close server socket:%d",gBgp4.server_sock);
        bgp4_set_sock_noblock(gBgp4.server_sock,on);/*always wait to down*/
        bgp4_tcp_close_socket(gBgp4.server_sock);
        gBgp4.server_sock = BGP4_INVALID_SOCKET;
    }
    
#ifdef BGP_IPV6_WANTED
    if (gBgp4.server_sock6 != BGP4_INVALID_SOCKET)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 close ipv6 server socket:%d",gBgp4.server_sock6);
        bgp4_set_sock_noblock(gBgp4.server_sock6,on);/*always wait to down*/ 
        bgp4_tcp_close_socket(gBgp4.server_sock6);
        gBgp4.server_sock6 = BGP4_INVALID_SOCKET;
    }
#endif

    return ;
}

/*send data on socket*/
int bgp4_sock_send(tBGP4_PEER *p_peer, u_char *p_msg, u_int len)
{
    u_char addrstr[64];
    u_char errstr[64];
    int check_error=0;
#if !defined(WIN32)
    int sock_error_no1 = ENOBUFS;
    int sock_error_no2 = EWOULDBLOCK;
#else
    int sock_error_no1 = WSAENOBUFS;
    int sock_error_no2 = WSAEWOULDBLOCK;
#endif

    if(gBgp4.work_mode == BGP4_MODE_SLAVE)
    {
        bgp4_log(BGP_DEBUG_TCP,1,"slave card do not send msg to socket!");
        return TRUE;
    }
        
    if (gBgp4.dbg_flag & BGP_DEBUG_PKT)
    {
        bgp4_log(BGP_DEBUG_PKT,1,"send buffer to %s by sock %d,length %d", 
            bgp4_printf_peer(p_peer, addrstr), 
            p_peer->sock,
            len);
        bgp4_debug_packet(p_msg, len)  ;
    }
                
    if (p_peer->sock <= 0) 
    {
        bgp4_log(BGP_DEBUG_PKT,1,"bgp4 sock send,peer sock invalid,do not send");
        return TRUE;
    }

    
SEND_TRY :
    gBgp4.stat.tx++;
    switch(p_msg[18])
    {
        case BGP4_MSG_KEEPALIVE:
            gBgp4.stat.tcp_keepalive++;
            break;
        case BGP4_MSG_UPDATE:
            gBgp4.stat.tcp_update++;
            break;
        case BGP4_MSG_NOTIFY:
            gBgp4.stat.tcp_notify++;
            break;
        case BGP4_MSG_OPEN:
            gBgp4.stat.tcp_open++;
            break;
        default :
        break;
    }

#if 0
    if (getsockname(p_peer->sock,(struct sockaddr*)&sa, &salen) != VOS_OK)   
    {
        bgp4_log(BGP_DEBUG_EVT,1,"peer %s socket is closed,do not send packet", bgp4_printf_peer(p_peer,addrstr));
        return (FALSE);
    }
#endif    
    if (send(p_peer->sock, p_msg, len, 0) == VOS_ERR) 
    {  
        int error = errnoGet();
        check_error++;
        bgp4_printf_syserror(error,errstr);
        gBgp4.stat.txfail++;

        if ((error == sock_error_no1 || error == sock_error_no2)
            &&(check_error<5))
        {
            vos_pthread_delay(5);
            goto SEND_TRY;
        }
        
        bgp4_log(BGP_DEBUG_EVT,1,"send packet error %s,peer %s, type %d,length %d",errstr, 
            bgp4_printf_peer(p_peer, addrstr), 
            p_msg[18],
            len);
        bgp4_fsm(p_peer, BGP4_EVENT_FATAL_ERROR);

        return (FALSE);
    }
    
    p_peer->tx_msg++;
    bgp4_log(BGP_DEBUG_TCP,1, "send data to %s, socket %d,size %d",addrstr,p_peer->sock,len);
     
    return (TRUE);
}

/*notify msg r&t*/
int bgp4_send_notify_without_peer(u_int sock_id, u_char code, 
                        u_char sub_code,u_char *p_data, 
                        u_int errlen)
{
    u_char buf[BGP4_MAX_MSG_LEN];
    tBGP4_MSGHDR  * p_msg = (tBGP4_MSGHDR *)buf; 
    tBGP4_NOTIFYMSG *p_notify = NULL; 
    u_int len = BGP4_HLEN ;
        
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
         
    if (send(sock_id, p_msg, len, 0) == VOS_ERR) 
    {
        return (FALSE);
    }
    
    return (TRUE);
}


/*recv data from peer socket*/
INT1 bgp4_sock_recv(tBGP4_PEER* p_peer)
{
    u_short msglen = 0;
    u_int expect = 0 ;
    u_char *p_msg = NULL;
    int  s  = 0;
    int readlen = 0 ;
    int restlen = 0 ;
    int i = 0 ;
    u_char estr[64];   
    u_char astr[64];
    u_char sstr[64];
    int recv_flag = 0;
    int recv_error_no = 0;

#if !defined(WIN32)
    recv_flag = MSG_DONTWAIT;
    recv_error_no = EWOULDBLOCK;
#else
    recv_flag = 0;
    recv_error_no = WSAEWOULDBLOCK;
#endif



    if ((p_peer->admin != BGP4_PEER_RUNNING) || 
            (p_peer->state < BGP4_NS_OPENSENT ))
    {
        bgp4_printf_state(p_peer->state,sstr);
        bgp4_log(BGP_DEBUG_TCP,1,"peer %s state is %s,can not recieve data",
            bgp4_printf_peer(p_peer, astr),
            sstr);
        return FALSE;
    } 
    
    s = p_peer->sock;
    if (s <= 0)
    {
        bgp4_log(BGP_DEBUG_TCP,1,"tcp invalid peer socket %d",s);
        return FALSE;
    }    

    if (p_peer->rxmsg.len >= BGP4_MAX_MSG_LEN)
    {
        bgp4_log(BGP_DEBUG_TCP,1,"invalid current length");
        return FALSE;
    }

    expect = BGP4_MAX_MSG_LEN - p_peer->rxmsg.len ;
    p_msg = p_peer->rxmsg.buf + p_peer->rxmsg.len ;

    /*now,prepare recv data*/
    readlen = recv(s, p_msg, expect, recv_flag);
    if (readlen <=0)
    {
        if (errnoGet() != recv_error_no)
        {
            bgp4_printf_syserror(errnoGet(), estr);
            bgp4_log(BGP_DEBUG_EVT,1,"receive data from %s socket %d %s",
                bgp4_printf_peer(p_peer, astr),
                s,
                estr);
            gBgp4.stat.rcvfail++;

            bgp4_fsm(p_peer, BGP4_EVENT_TCP_CLOSED);/*modify for n2x conformance test*/
        }
        return FALSE ;  
    
    }  
    /*something read,update length*/
    p_peer->rxmsg.len += readlen ;

    /*try to process multiple message in rxbuf*/
    restlen = p_peer->rxmsg.len ;
    p_msg = p_peer->rxmsg.buf;
    readlen = 0 ;
    

    while (restlen > 0)
    {

        /*if rest length do not include header,stop*/
        if (restlen < BGP4_HLEN)
        {
            break;
        }
        
        /*now,we got message header,we need read message length from buffer*/          
        bgp4_get_2bytes(msglen, (p_msg + BGP4_MARKER_LEN)) ;
        bgp4_log(BGP_DEBUG_TCP,1,"msg header rceived,length %d",msglen); 
        
        /*validate message length*/          
        if ((BGP4_MAX_MSG_LEN < msglen)|| (BGP4_HLEN > msglen))
        {
            bgp4_log(BGP_DEBUG_EVT,1,"invalid message length %d",msglen); 
            bgp4_fsm(p_peer, BGP4_EVENT_TCP_CLOSED);
            return FALSE;
        }
        /*if rest data containing full packet?*/
        if (restlen < msglen)
        {
            break;
        } 
        /*current data contain the full msg,process it*/
        bgp4_msg_input(p_peer, p_msg);

        /*skip to next msg header*/
        p_msg += msglen ;
        readlen += msglen ;
        restlen -= msglen ;
    }
        
    /*process end,buffer may contain no data,or have msg fragment*/
    p_peer->rxmsg.len = restlen ;
    if (restlen && readlen)
    {
        /*copy rest data into buffer header*/
        for (i = 0 ; i < restlen ; i++)
        {
            p_peer->rxmsg.buf[i] = p_peer->rxmsg.buf[i+readlen];
        }
    }
    
    return TRUE ;
}

/*check accept data*/
void bgp4_server_accept()  
{
    int s;
    int server = gBgp4.server_sock ;
    unsigned int len;
    u_char sstr[64];
    u_char astr[64];
    u_char perstr[64];
    int optval, on;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_ADDR ip;
    struct sockaddr_in sa;
    struct sockaddr_in  sa_local;
    u_int buflen = BGP4_TCP_BUFFER_LEN;
    u_int vrf_id = 0;/*vpn instance id*/
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    memset(&sa,0,sizeof(sa));
    len = sizeof(sa);     

    if(server<=0)
        return;

    for (; ;)
    {
        s = accept(server, (struct sockaddr *)&sa, &len) ;
        if (s <= 0)
        {
            return;
        }
        bgp4_log(BGP_DEBUG_TCP,1,"accept connection %d,from %s ", 
                s,
                inet_ntoa_1(astr,ntohl(sa.sin_addr.s_addr)));
                
        /*need to check ipv6 or ipv4 address*/      
        memset(&ip, 0, sizeof(ip));
        
        if(sa.sin_family == AF_INET)
        {
            bgp_ip4(ip.ip) = ntohl(sa.sin_addr.s_addr);
            ip.afi = BGP4_PF_IPUCAST;
            ip.prefixlen =32;
        }

        /*TODO:get vrf id from accept sock data*/
        vrf_id = sa.vr_id;
        p_instance = bgp4_vpn_instance_lookup(vrf_id);
        if(p_instance == NULL)
        {
            bgp4_send_notify_without_peer(s, BGP4_CEASE, 5, NULL, 0);
            bgp4_log(BGP_DEBUG_TCP,1,"can not find such instance %d",vrf_id);
            continue;
        }
        
        p_peer = bgp4_peer_lookup(p_instance,&ip);
        if (p_peer == NULL) 
        {
            bgp4_send_notify_without_peer(s, BGP4_CEASE, 5, NULL, 0);
            bgp4_log(BGP_DEBUG_TCP,1,"vrf %d no such peer,discard connection sock %d",vrf_id,s);
            bgp4_tcp_close_socket(s);
            /*we should close the fd,but n2x said we wrong*/
            continue;
        }
                
        if(p_peer->unsupport_capability == TRUE)
        {
            bgp4_log(BGP_DEBUG_TCP,1,"peer has unsupported capability,discard connection %d",s);
            bgp4_tcp_close_socket(s);
            continue;
        }    
         
        /*if update source is configured ,need compare it*/
        if (p_peer->update_source.prefixlen == 32)
        {                         
            memset(&sa_local,0,sizeof(sa_local));
            if (getsockname(s,((struct sockaddr *)&sa_local), &len) == VOS_OK) 
            {
                if (memcmp(p_peer->update_source.ip,&sa_local.sin_addr.s_addr,4) != 0)
                {
                    inet_ntoa_1(astr,(sa_local.sin_addr.s_addr));
                    bgp4_log(BGP_DEBUG_TCP,1,"different local address %s,socket %d",astr,s);
                    bgp4_tcp_close_socket(s);
                    continue;
                }
            }
        }
         
        on = 1;
        bgp4_set_sock_noblock(s,on);
         
        optval = 1;
        bgp4_set_sock_tcpnodelay(s,optval);
        
        /*rx&tx buf length*/
        bgp4_set_sock_rxbuf(s,buflen);  
        bgp4_set_sock_txbuf(s,buflen);  

        bgp4_tcp_set_peer_md5(s,p_peer);
        
        bgp4_tcp_set_peer_ttl(s, p_peer);
        
        bgp4_log(BGP_DEBUG_TCP,1,"find expected peer %s,state %s",
                        inet_ntoa_1(astr,(sa.sin_addr.s_addr)),
                        bgp4_printf_state(p_peer->state,sstr));
        
        if (p_peer->admin != BGP4_PEER_RUNNING) 
        {
            bgp4_log(BGP_DEBUG_TCP,1,"close new socket %d,peer is disabled",s);
            bgp4_tcp_close_socket(s);
            continue ;
        }                           
         
        switch (p_peer->state) {
            case BGP4_NS_IDLE :
            {
                bgp4_stop_start_timer(p_peer);             
                bgp4_start_retry_timer(p_peer);

                p_peer->state = BGP4_NS_ACTIVE ;
                
            }/* Fall through */              
            case BGP4_NS_ACTIVE :
            {
                p_peer->sock = s;
                bgp4_fill_tcp_address(p_peer);
                bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
                break;
            }
             
            case BGP4_NS_CONNECT         :
            case BGP4_NS_OPENSENT        :
            case BGP4_NS_OPENCONFIRM :
            {

                if (p_peer->state == BGP4_NS_OPENSENT || 
                        p_peer->state == BGP4_NS_OPENCONFIRM) 
                {
                    bgp4_log(BGP_DEBUG_TCP,1,
                        "close existing socket %d,use new accepted %d",
                        p_peer->sock,s);
                    bgp4_tcp_close_socket(p_peer->sock);                    
                    p_peer->sock = BGP4_INVALID_SOCKET;
                    if (p_peer->state == BGP4_NS_OPENSENT)
                    {
                        bgp4_stop_hold_timer(p_peer);
                    }
                } 
                if (p_peer->sock > 0) 
                {
                    bgp4_tcp_close_socket(p_peer->sock); 
                    p_peer->sock = BGP4_INVALID_SOCKET;
                }
                p_peer->sock = s;
                bgp4_fill_tcp_address(p_peer);
                bgp4_start_retry_timer(p_peer);
             
                p_peer->state = BGP4_NS_ACTIVE ;
                bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
                break;
            }
            case BGP4_NS_ESTABLISH :
            {
#if 0            
                bgp4_log(BGP_DEBUG_TCP,1,"close new socket %d,peer is up",s);             
                bgp4_tcp_close_socket(s); 
#endif
                /*for GR comformance test,TCP reset*/
                bgp4_log(BGP_DEBUG_EVT,1,"close existing socket %d,use new accepted %d",
                        p_peer->sock,s);
                
                if (p_peer->sock > 0) 
                {
                    bgp4_tcp_close_socket(p_peer->sock); 
                    p_peer->sock = BGP4_INVALID_SOCKET;
                }
                p_peer->sock = s;
                bgp4_fill_tcp_address(p_peer);/*keep the state until receiving open msg*/
                
                bgp4_peer_down_to_gr(p_peer);
                bgp4_start_retry_timer(p_peer);      
                
                p_peer->state = BGP4_NS_ACTIVE ;
                bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
                
                break; 
            }
            default :
                break;
        }
    }
    return ;
}

#ifdef BGP_IPV6_WANTED
void bgp4_ip6_server_accept()  
{
    int s=0;
    int server = gBgp4.server_sock6 ;
    unsigned int len;
    u_char sstr[64];
    int optval, on;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_ADDR ip;
    u_char *ipv4_flag="::ffff:";
    u_char ipv6_str[64];
    u_char perstr[64];
    struct sockaddr_in6 sa;
    struct sockaddr_in6 sa_local;
    u_int vrf_id = 0;/*vpn instance id*/
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    
    if(server<=0)
    {
        return ;
    }

    memset(&sa,0,sizeof(sa));
    len = sizeof(sa);
    for (; ;)
    {
        s = accept(server, (struct sockaddr *)&sa, &len) ;

        if (s < 0)
        {
            return;
        }
        else
        {
            inet_ntop(AF_INET6, &sa.sin6_addr, ipv6_str, 64);
            bgp4_log(BGP_DEBUG_TCP,1,"accept connection %d,from %s ", s,ipv6_str);
        }
                
        bgp4_log(BGP_DEBUG_TCP,1,"accept connection %d,from %s ", 
                    s,
                    inet_ntop(AF_INET6, &sa.sin6_addr, ipv6_str, 64));
                
        /*need to check ipv6 or ipv4 address*/      
        memset(&ip, 0, sizeof(ip));        
        memset(ipv6_str, 0, 64);
        inet_ntop(AF_INET6,&sa.sin6_addr,ipv6_str,64);
                
        if(strstr(ipv6_str,ipv4_flag))
        {
            ip.afi = BGP4_PF_IPUCAST ;
            bgp_ip4(ip.ip) = *(u_int *)(((u_char *)&sa.sin6_addr) + 12);
            ip.prefixlen =32;
        }
        else
        {
            ip.afi = BGP4_PF_IP6UCAST;
            memcpy(&ip.ip, &sa.sin6_addr, sizeof(sa.sin6_addr));
            ip.prefixlen =128;
        }
                
        /*TODO:get vrf id from accept sock data*/
        vrf_id = sa.vr_id;
        p_instance = bgp4_vpn_instance_lookup(vrf_id);
        if(p_instance == NULL)
        {
            bgp4_send_notify_without_peer(s, BGP4_CEASE, 5, NULL, 0);
            bgp4_log(BGP_DEBUG_TCP,1,"can not find such instance %d",vrf_id);
            continue;
        }
        
        p_peer = bgp4_peer_lookup(p_instance,&ip);      
        if (p_peer == NULL) 
        {
            bgp4_log(BGP_DEBUG_TCP,1,"vrf %d no such peer,discard connection %d",p_instance->instance_id,s);
            bgp4_tcp_close_socket(s); 
            continue;
        }

        /*add for cap advertisement,by zxq*/
        if(p_peer->unsupport_capability == TRUE)
        {
            bgp4_log(BGP_DEBUG_TCP,1,"peer has unsupported capability,discard connection %d",s);
                    bgp4_tcp_close_socket(s); 
            continue;
        }    
         
        /*if update source is configured ,need compare it*/
        if (p_peer->update_source.prefixlen == 128)
        {     
            memset(&sa_local,0,sizeof(sa_local));
            if (getsockname(s,((struct sockaddr *)&sa_local), &len) == VOS_OK) 
            {               
                if (memcmp(p_peer->update_source.ip,&sa_local.sin6_addr,16) != 0)
                {
                    u_char addr[64] = {0};
                    bgp4_log(BGP_DEBUG_TCP,1,"different local address %s,socket %d",
                                bgp4_printf_addr(&p_peer->update_source,addr),
                                s);
                    close (s);
                    continue;
                }
            }
        }
         
        on = 1;
        bgp4_set_sock_noblock(s,on);
         
        optval = 1;
        bgp4_set_sock_tcpnodelay(s,optval);

        bgp4_tcp_set_peer_md5( s,p_peer);        
        bgp4_tcp_set_peer_ttl(s, p_peer);

        bgp4_log(BGP_DEBUG_TCP,1,"find expected peer %s,state %s",
                inet_ntop(AF_INET6,&sa.sin6_addr,ipv6_str,64),
                bgp4_printf_state(p_peer->state,sstr));

        if (p_peer->admin != BGP4_PEER_RUNNING) 
        {
            bgp4_log(BGP_DEBUG_TCP,1,"close new socket %d,peer is disabled",s);
            bgp4_tcp_close_socket(s); 
            continue ;
        }                           
         
        switch (p_peer->state) {
            case BGP4_NS_IDLE :
            {
                bgp4_stop_start_timer(p_peer);             
                bgp4_start_retry_timer(p_peer);
                p_peer->state = BGP4_NS_ACTIVE ;
            }/* Fall through */
                
            case BGP4_NS_ACTIVE :
            {
                p_peer->sock = s;
                bgp4_fill_tcp_address(p_peer);
                bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
                break;
            }
             
            case BGP4_NS_CONNECT         :
            case BGP4_NS_OPENSENT        :
            case BGP4_NS_OPENCONFIRM :
            {
                if (p_peer->state == BGP4_NS_OPENSENT || 
                    p_peer->state == BGP4_NS_OPENCONFIRM) 
                {
                    bgp4_log(BGP_DEBUG_TCP,1,
                        "close existing socket %d,use new accepted %d",
                        p_peer->sock,s);
                 
                    if(p_peer->sock>0)
                    {
                        bgp4_tcp_close_socket(p_peer->sock);                         
                    }
                    p_peer->sock = BGP4_INVALID_SOCKET;
                    if (p_peer->state == BGP4_NS_OPENSENT)
                    {
                        bgp4_stop_hold_timer(p_peer);
                    }
                } 
                p_peer->sock = s;
                bgp4_fill_tcp_address(p_peer);
                bgp4_start_retry_timer(p_peer);

                p_peer->state = BGP4_NS_ACTIVE ;
                bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
                break;
               }
            case BGP4_NS_ESTABLISH :
            {
                bgp4_log(BGP_DEBUG_TCP,1,"close new socket %d,peer is up",s);
             
                bgp4_tcp_close_socket(s); 
                break; 
            }
            default :
                break;
        }
    }
    return ;
}
#endif

/*check all socket's state,decide connection and data*/
void bgp4_check_socket()
{
    tBGP4_PEER *p_peer = NULL;
    struct timeval tv;
    struct sockaddr_in6 sa6;
    fd_set rfdset, wfdset;
    int fd_max = 0;
    int  len=sizeof(struct sockaddr);
    char peerstr[64];
    u_char on=0; 
    u_int rx_count = 0 ;
    u_int time_start = 0 ;
    int rc ;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    static int sucess = 0 ;

    tv.tv_sec  = 0;
    tv.tv_usec =100000;/*100 ms*/

    bgp_sem_take();

    FD_ZERO(&rfdset);
    FD_ZERO(&wfdset);

    if (gBgp4.server_open && gBgp4.server_sock>0)
    {
        FD_SET(gBgp4.server_sock , &rfdset);
        fd_max = gBgp4.server_sock;
    }

    if (gBgp4.server_open && gBgp4.server_sock6>0)
    {
        FD_SET(gBgp4.server_sock6 , &rfdset);
        if(gBgp4.server_sock6>fd_max)
            fd_max = gBgp4.server_sock6;
    }

    if(gBgp4.rtsock>0)
    {
        FD_SET(gBgp4.rtsock , &rfdset);
        if(gBgp4.rtsock>fd_max)
            fd_max = gBgp4.rtsock;
    }

    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
#if 0/*no need decrease wait time,10 ms is too short,will induce cpu usage come to 100% */
        /*decrease to 10 ms if some ip update need*/
        if (p_instance->p_route_update)
        {
            tv.tv_usec =10000;
        }    
#endif

        LST_LOOP(&p_instance->peer_list,p_peer,node,tBGP4_PEER) 
        {
            if(p_peer == NULL)
            {
                break;
            }
            len = sizeof(sa6) ;

            if ((int)p_peer->sock <= 0)
                continue ;
                    
            /*peer is in connecting,check if ok*/
            if (p_peer->connect_inprogress)
            {
                FD_SET(p_peer->sock, &wfdset);
                if (p_peer->sock > fd_max)
                {
                    fd_max = p_peer->sock;
                }
            }
                else if (1/*(getsockname(p_peer->sock,(struct sockaddr*)&sa, &len))== VOS_OK*/)
            {
                FD_SET(p_peer->sock, &rfdset);
                if (fd_max < p_peer->sock)
                {
                    fd_max = p_peer->sock;
                }
            }
            else
            {   
                bgp4_log(BGP_DEBUG_EVT,1,"connection has been closed by peer %s",
                bgp4_printf_peer(p_peer,peerstr));

                bgp4_fsm(p_peer, BGP4_EVENT_TCP_CLOSED);
                p_peer->sock = BGP4_INVALID_SOCKET;
            }
        }
    
    }
        
    bgp_sem_give();

    if (fd_max == 0)
    {
        return ;
    }

    time_start = vos_get_system_tick();
    taskSafe();

    rc = select(fd_max + 1, &rfdset, &wfdset, 0, &tv);
    taskUnsafe();

    gBgp4.select_time += (vos_get_system_tick() - time_start);
            
    if (rc <= 0 ) 
    {
        return ;
    }

    bgp_sem_take(); 
    
    if (gBgp4.server_sock > 0 && FD_ISSET(gBgp4.server_sock, &rfdset))
    {
        bgp4_server_accept();
    }
        
#ifdef BGP_IPV6_WANTED
    if (gBgp4.server_sock6 > 0 && FD_ISSET(gBgp4.server_sock6, &rfdset))
    {
        bgp4_ip6_server_accept();
    }
#endif

    time_start = vos_get_system_tick();

    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
        LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER) 
        {
            if ((int)p_peer->sock <= 0)
            {
                continue;
            }
                    
            if (FD_ISSET(p_peer->sock, &rfdset))
            {
                gBgp4.stat.rcv++;
                
                rx_count = 0 ;
                /*rx packet*/
                while (bgp4_sock_recv(p_peer) == TRUE)
                {
                    rx_count++;
                    if (rx_count >= 16)
                    {
                        break;
                    }
                }
            }
            else if (FD_ISSET(p_peer->sock, &wfdset))
            {       
                len = sizeof(sa6) ;            
                memset(&sa6, 0, sizeof(sa6));
        #if !defined(WIN32)            
                if (getpeername(p_peer->sock,(struct sockaddr*)&sa6, &len) == VOS_OK)
        #else
                if (p_peer->remote.ip.afi == BGP4_PF_IP6UCAST
                        || getpeername(p_peer->sock,(struct sockaddr*)&sa6, &len) == VOS_OK)
        #endif
                {   

                    on = 0;     /* turn OFF the non-blocking I/O */
                    if (bgp4_set_sock_noblock(p_peer->sock,on)== VOS_ERR)
                    {
                        
                    }

                    
                    /*connect ok*/
                    bgp4_log(BGP_DEBUG_TCP,1,"connect socket %d to %s ok",
                            p_peer->sock, 
                            bgp4_printf_peer(p_peer, peerstr));

                    p_peer->connect_inprogress = 0 ;
                    bgp4_fill_tcp_address(p_peer);
                                        
                    /*p_peer->state = BGP4_NS_CONNECT ;*/
                    bgp4_fsm(p_peer, BGP4_EVENT_TCP_OPENED);
                }
                else
                {
                    bgp4_sock_close(p_peer );
                    bgp4_fsm(p_peer, BGP4_EVENT_CONNECT_FAIL);
                }
            }
        }
    }


    if (gBgp4.rtsock> 0 && FD_ISSET(gBgp4.rtsock, &rfdset))
    {
        bgp4_rtsock_recv();
    }
    
    gBgp4.input_time += (vos_get_system_tick() - time_start);
    
    bgp_sem_give();

    /*20120401*//*release cpu per 100 sucessful select*/
    if (++sucess > 100)
    {
        sucess = 0 ;
        vos_pthread_delay(1);
    }
    return;        
}


#endif
