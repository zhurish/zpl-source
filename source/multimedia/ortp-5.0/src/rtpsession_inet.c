/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of oRTP.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#ifdef HAVE_CONFIG_H
#include "ortp-config.h"
#endif
#include "rtpsession_priv.h"
#include <ortp/rtp_queue.h>
#include <ortp/telephonyevents.h>
#include <ortp/rtpsession.h>
#include <ortp/event.h>
#include <ortp/logging.h>
#include <ortp/rtpsignaltable.h>
#include <ortp/ortp.h>
#include "ortp/rtcp.h"
#include "utils.h"
#include "jitterctl.h"

#if (defined(_WIN32) || defined(_WIN32_WCE)) && defined(ORTP_WINDOWS_DESKTOP)
#include <mswsock.h>
#else
#define USE_RECVMSG
#define USE_SENDMSG
#endif


#define can_connect(s)	( (s)->use_connect && !(s)->symmetric_rtp)



#ifndef IN6_IS_ADDR_MULTICAST
#define IN6_IS_ADDR_MULTICAST(i)	(((uint8_t *) (i))[0] == 0xff)
#endif

static int
_rtp_session_set_remote_addr_full (RtpSession * session, const char * rtp_addr, int rtp_port, const char * rtcp_addr, int rtcp_port, bool_t is_aux);

static bool_t try_connect(ortp_socket_t fd, const struct ipstack_sockaddr *dest, socklen_t addrlen){
    if (ipstack_connect(fd, dest, addrlen)<0){
        ortp_warning("Could not connect() socket: %s",getSocketError());
        return FALSE;
    }
    return TRUE;
}

static int set_multicast_group(ortp_socket_t sock, const char *addr){
#ifndef __hpux

    int err, ai_family = 0;
    if(strstr(addr, "."))
        ai_family = IPSTACK_AF_INET;
    else if(strstr(addr, ":"))
        ai_family = IPSTACK_AF_INET6;
    switch (ai_family){
    case IPSTACK_AF_INET:
    {
        struct ipstack_ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(addr);
        mreq.imr_interface.s_addr = IPSTACK_INADDR_ANY;
        err = ipstack_setsockopt(sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_ADD_MEMBERSHIP, (SOCKET_OPTION_VALUE) &mreq, sizeof(mreq));
        if (err < 0){
            ortp_warning ("Fail to join address group: %s.", getSocketError());
        } else {
            ortp_message ("RTP socket [%i] has joined address group [%s]",sock, addr);
        }
    }
        break;
    case IPSTACK_AF_INET6:
    {
        struct ipstack_ipv6_mreq mreq;
        //mreq.ipv6mr_multiaddr = inet6_addr(addr);
        mreq.ipv6mr_interface = 0;
        err = ipstack_setsockopt(sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_JOIN_GROUP, (SOCKET_OPTION_VALUE)&mreq, sizeof(mreq));
        if (err < 0 ){
            ortp_warning ("Fail to join address group: %s.", getSocketError());
        } else {
            ortp_message ("RTP socket 6 [%i] has joined address group [%s]",sock, addr);
        }
    }
        break;
    }
    return 0;
#else
    return -1;
#endif
}


static ortp_socket_t create_and_bind(const char *addr, int port, int sock_family,
                                     bool_t reuse_addr){
    int err;
    int optval = 1;
    ortp_socket_t sock;
    ipstack_bzero(sock);
    if (port==0)
        reuse_addr=FALSE;
    struct ipstack_sockaddr sa;
    socklen_t salen;

    sock = ipstack_socket(IPSTACK_IPCOM, sock_family, IPSTACK_SOCK_DGRAM, 0);
    if (ipstack_invalid(sock)){
        ortp_error("Cannot create a socket with family=[%i] and socktype=[%i]: %s", sock_family, IPSTACK_SOCK_DGRAM, getSocketError());
        return sock;
    }
    if (reuse_addr){
        err = ipstack_setsockopt (sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_REUSEADDR,
                          (SOCKET_OPTION_VALUE)&optval, sizeof (optval));
        if (err < 0)
        {
            ortp_warning ("Fail to set rtp address reusable: %s.", getSocketError());
        }
#ifdef IPSTACK_SO_REUSEPORT
        /*IPSTACK_SO_REUSEPORT is required on mac and ios especially for doing multicast*/
        err = ipstack_setsockopt (sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_REUSEPORT,
                          (SOCKET_OPTION_VALUE)&optval, sizeof (optval));
        if (err < 0)
        {
            ortp_warning ("Fail to set rtp port reusable: %s.", getSocketError());
        }
#endif
    }
    /*enable dual stack operation, default is enabled on unix, disabled on windows.*/
    if (sock_family==IPSTACK_AF_INET6){
        optval=0;
        err=ipstack_setsockopt(sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_V6ONLY, (const char*)&optval, sizeof(optval));
        if (err < 0){
            ortp_warning ("Fail to IPSTACK_IPV6_V6ONLY: %s.",getSocketError());
        }
    }

#ifdef IPSTACK_SO_TIMESTAMP
    optval=1;
    err = ipstack_setsockopt (sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_TIMESTAMP,
                      (SOCKET_OPTION_VALUE)&optval, sizeof (optval));
    if (err < 0)
    {
        ortp_warning ("Fail to set rtp timestamp: %s.",getSocketError());
    }
#endif
    err = 0;
    optval=1;
    switch (sock_family) {
    default:
    case IPSTACK_AF_INET:
#ifdef IPSTACK_IP_RECVTTL
        err = ipstack_setsockopt(sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_RECVTTL, (SOCKET_OPTION_VALUE)&optval, sizeof(optval));
#endif
        break;
    case IPSTACK_AF_INET6:
#ifdef IPSTACK_IPV6_RECVHOPLIMIT
        err = ipstack_setsockopt(sock, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_RECVHOPLIMIT, (SOCKET_OPTION_VALUE)&optval, sizeof(optval));
#endif
        break;
    }
    if (err < 0) {
        ortp_warning("Fail to set recv TTL/HL socket option: %s.", getSocketError());
    }
    ortp_address_to_sockaddr(sock_family, addr, port, &sa, &salen);
    err = ipstack_bind(sock, &sa, (int)salen);
    if (err != 0){
        ortp_error ("Fail to bind rtp/rtcp socket to (addr=%s port=%i) : %s.", addr, port, getSocketError());
        close_socket(sock);
        return sock;
    }
    /*compatibility mode. New applications should use rtp_session_set_multicast_group() instead*/
    //set_multicast_group(sock, addr);


#if defined(_WIN32) || defined(_WIN32_WCE)
    if (ortp_WSARecvMsg == NULL) {
        GUID guid = WSAID_WSARECVMSG;
        DWORD bytes_returned;
        if (  WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
                       &ortp_WSARecvMsg, sizeof(ortp_WSARecvMsg), &bytes_returned, NULL, NULL) == SOCKET_ERROR) {
            ortp_warning("WSARecvMsg function not found [%d].", WSAGetLastError());
        }
    }
#endif
    set_non_blocking_socket (sock);
    return sock;
}

void _rtp_session_apply_socket_sizes(RtpSession * session){
    int err;
    bool_t done=FALSE;
    ortp_socket_t sock = session->rtp.gs.socket;
    unsigned int sndbufsz = session->rtp.snd_socket_size;
    unsigned int rcvbufsz = session->rtp.rcv_socket_size;

    if (ipstack_invalid(sock)) return;

    if (sndbufsz>0){
#ifdef IPSTACK_SO_SNDBUFFORCE
        err = ipstack_setsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_SNDBUFFORCE, (void *)&sndbufsz, sizeof(sndbufsz));
        if (err == -1) {
            ortp_warning("Fail to increase socket's send buffer size with IPSTACK_SO_SNDBUFFORCE: %s.", getSocketError());
        }else done=TRUE;
#endif
        if (!done){
            err = ipstack_setsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_SNDBUF, (void *)&sndbufsz, sizeof(sndbufsz));
            if (err == -1) {
                ortp_error("Fail to increase socket's send buffer size with IPSTACK_SO_SNDBUF: %s.", getSocketError());
            }
        }
    }
    done=FALSE;
    if (rcvbufsz>0){
#ifdef IPSTACK_SO_RCVBUFFORCE
        err = ipstack_setsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_RCVBUFFORCE, (void *)&rcvbufsz, sizeof(rcvbufsz));
        if (err == -1) {
            ortp_warning("Fail to increase socket's recv buffer size with IPSTACK_SO_RCVBUFFORCE: %s.", getSocketError());
        }
#endif
        if (!done){
            err = ipstack_setsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_RCVBUF, (void *)&rcvbufsz, sizeof(rcvbufsz));
            if (err == -1) {
                ortp_error("Fail to increase socket's recv buffer size with IPSTACK_SO_RCVBUF: %s.", getSocketError());
            }
        }

    }
}

/**
 *rtp_session_set_local_addr:
 *@param session:		a rtp session freshly created.
 *@param addr:		a local IP address in the xxx.xxx.xxx.xxx form.
 *@param rtp_port:		a local port or -1 to let oRTP choose the port randomly
 *@param rtcp_port:		a local port or -1 to let oRTP choose the port randomly
 *
 *	Specify the local addr to be use to listen for rtp packets or to send rtp packet from.
 *	In case where the rtp session is send-only, then it is not required to call this function:
 *	when calling rtp_session_set_remote_addr(), if no local address has been set, then the
 *	default INADRR_ANY (0.0.0.0) IP address with a random port will be used. Calling
 *	rtp_session_set_local_addr() is mandatory when the session is recv-only or duplex.
 *
 *	Returns: 0 on success.
**/

int
rtp_session_set_local_addr (RtpSession * session, const char * addr, int rtp_port, int rtcp_port)
{
    ortp_socket_t sock;
    int sockfamily = IPSTACK_AF_INET;

    // Stop async rtp recv thread before recreating the socket
    rtp_session_reset_recvfrom(session);

    if (!ipstack_invalid(session->rtp.gs.socket)){
        /* don't rebind, but close before*/
        _rtp_session_release_sockets(session, FALSE);
    }
    /* try to bind the rtp port */
    ortp_address_to_sockaddr(sockfamily, addr, rtp_port, (struct ipstack_sockaddr *)&session->rtp.gs.loc_addr.addr, &session->rtp.gs.loc_addr.len);

    sock=create_and_bind(addr, rtp_port,sockfamily,session->reuseaddr);
    if (!ipstack_invalid(sock)){
        session->rtp.gs.sockfamily=sockfamily;
        session->rtp.gs.socket=sock;
        session->rtp.gs.loc_port=rtp_port;
        _rtp_session_apply_socket_sizes(session);
        /*try to bind rtcp port */
        sock=create_and_bind(addr, rtcp_port,sockfamily,session->reuseaddr);
        if (!ipstack_invalid(sock)){
            session->rtcp.gs.sockfamily=sockfamily;
            session->rtcp.gs.socket=sock;
            session->rtcp.gs.loc_port=rtcp_port;
        }else {
            ortp_error("Could not create and bind rtcp socket for session [%p]",session);
            return -1;
        }

        /* set socket options (but don't change chosen states) */
        rtp_session_set_multicast_ttl( session, -1 );
        rtp_session_set_multicast_loopback( session, -1 );
        if (session->use_pktinfo) rtp_session_set_pktinfo(session, TRUE);
        ortp_message("RtpSession bound to [%s] ports [%i] [%i] [%i %i]", addr, rtp_port, rtcp_port, session->rtp.gs.socket, session->rtcp.gs.socket);
        rtp_session_set_dscp(session, session->dscp);
        return 0;
    }
    ortp_error("Could not bind RTP socket to %s on port %i for session [%p]",addr,rtp_port,session);
    return -1;
}

static void _rtp_session_recreate_sockets(RtpSession *session){
    char addr[NI_MAXHOST];
    int err = ortp_sockaddr_to_address((struct ipstack_sockaddr *)&session->rtp.gs.loc_addr, session->rtp.gs.loc_addr.len, addr, sizeof(addr), NULL);
    if (err != 0) return;
    /*re create and bind sockets as they were done previously*/
    ortp_message("RtpSession %p is going to re-create its socket.", session);
    rtp_session_set_local_addr(session, addr, session->rtp.gs.loc_port, session->rtcp.gs.loc_port);
}

static void _rtp_session_check_socket_refresh(RtpSession *session){
    if (session->flags & RTP_SESSION_SOCKET_REFRESH_REQUESTED){
        session->flags &= ~RTP_SESSION_SOCKET_REFRESH_REQUESTED;
        _rtp_session_recreate_sockets(session);
    }
}

/**
 * Requests the session to re-create and bind its RTP and RTCP sockets same as they are currently.
 * This is used when a change in the routing rules of the host or process was made, in order to have
 * this routing rules change taking effect on the RTP/RTCP packets sent by the session.
**/
void rtp_session_refresh_sockets(RtpSession *session){
    if (!ipstack_invalid(session->rtp.gs.socket)){
        session->flags |= RTP_SESSION_SOCKET_REFRESH_REQUESTED;
    }
}

int rtp_session_join_multicast_group(RtpSession *session, const char *ip){
    int err;
    if (ipstack_invalid(session->rtp.gs.socket)){
        ortp_error("rtp_session_set_multicast_group() must be done only on bound sockets, use rtp_session_set_local_addr() first");
        return -1;
    }
    err=set_multicast_group(session->rtp.gs.socket,ip);
    set_multicast_group(session->rtcp.gs.socket,ip);
    return err;
}

/**
 *rtp_session_set_pktinfo:
 *@param session: a rtp session
 *@param activate: activation flag (0 to deactivate, other value to activate)
 *
 * (De)activates packet info for incoming and outgoing packets.
 *
 * Returns: 0 on success.
 *
**/
int rtp_session_set_pktinfo(RtpSession *session, int activate)
{
    int retval;
    int optname;
#if defined(_WIN32) || defined(_WIN32_WCE)
    char optval[sizeof(DWORD)];
    int optlen = sizeof(optval);
#else
    int *optval = &activate;
    int optlen = sizeof(activate);
#endif
    session->use_pktinfo = activate;
    // Dont't do anything if socket hasn't been created yet
    if (ipstack_invalid(session->rtp.gs.socket)) return 0;

#if defined(_WIN32) || defined(_WIN32_WCE)
    memset(optval, activate, sizeof(optval));
#endif

#ifdef IPSTACK_IP_PKTINFO
    optname = IPSTACK_IP_PKTINFO;
#else
    optname = IP_RECVDSTADDR;
#endif
    retval = ipstack_setsockopt(session->rtp.gs.socket, IPSTACK_IPPROTO_IP, optname, optval, optlen);
    if (retval < 0) {
        ortp_warning ("Fail to set IPv4 packet info on RTP socket: %s.", getSocketError());
    }
    retval = ipstack_setsockopt(session->rtcp.gs.socket, IPSTACK_IPPROTO_IP, optname, optval, optlen);
    if (retval < 0) {
        ortp_warning ("Fail to set IPv4 packet info on RTCP socket: %s.", getSocketError());
    }

    if (session->rtp.gs.sockfamily != IPSTACK_AF_INET) {
#if defined(_WIN32) || defined(_WIN32_WCE)
        memset(optval, activate, sizeof(optval));
#endif

#ifdef IPSTACK_IPV6_RECVPKTINFO
        optname = IPSTACK_IPV6_RECVPKTINFO;
#else
        optname = IPV6_RECVDSTADDR;
#endif
        retval = ipstack_setsockopt(session->rtp.gs.socket, IPSTACK_IPPROTO_IPV6, optname, optval, optlen);
        if (retval < 0) {
            ortp_warning("Fail to set IPv6 packet info on RTP socket: %s.", getSocketError());
        }
        retval = ipstack_setsockopt(session->rtcp.gs.socket, IPSTACK_IPPROTO_IPV6, optname, optval, optlen);
        if (retval < 0) {
            ortp_warning("Fail to set IPv6 packet info on RTCP socket: %s.", getSocketError());
        }
    }

    return retval;
}


/**
 *rtp_session_set_multicast_ttl:
 *@param session: a rtp session
 *@param ttl: desired Multicast Time-To-Live
 *
 * Sets the TTL (Time-To-Live) for outgoing multicast packets.
 *
 * Returns: 0 on success.
 *
**/
int rtp_session_set_multicast_ttl(RtpSession *session, int ttl)
{
    int retval;

    // Store new TTL if one is specified
    if (ttl>0) session->multicast_ttl = ttl;

    // Don't do anything if socket hasn't been created yet
    if (ipstack_invalid(session->rtp.gs.socket)) return 0;

    switch (session->rtp.gs.sockfamily) {
    case IPSTACK_AF_INET: {

        retval= ipstack_setsockopt(session->rtp.gs.socket, IPSTACK_IPPROTO_IP, IPSTACK_IP_MULTICAST_TTL,
                           (SOCKET_OPTION_VALUE)  &session->multicast_ttl, sizeof(session->multicast_ttl));

        if (retval<0) break;

        retval= ipstack_setsockopt(session->rtcp.gs.socket, IPSTACK_IPPROTO_IP, IPSTACK_IP_MULTICAST_TTL,
                           (SOCKET_OPTION_VALUE)	   &session->multicast_ttl, sizeof(session->multicast_ttl));

    } break;
    case IPSTACK_AF_INET6: {

        retval= ipstack_setsockopt(session->rtp.gs.socket, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_MULTICAST_HOPS,
                           (SOCKET_OPTION_VALUE)&session->multicast_ttl, sizeof(session->multicast_ttl));

        if (retval<0) break;

        retval= ipstack_setsockopt(session->rtcp.gs.socket, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_MULTICAST_HOPS,
                           (SOCKET_OPTION_VALUE) &session->multicast_ttl, sizeof(session->multicast_ttl));
    } break;
    default:
        retval=-1;
    }

    if (retval<0)
        ortp_warning("Failed to set multicast TTL on socket.");


    return retval;
}


/**
 *rtp_session_get_multicast_ttl:
 *@param session: a rtp session
 *
 * Returns the TTL (Time-To-Live) for outgoing multicast packets.
 *
**/
int rtp_session_get_multicast_ttl(RtpSession *session)
{
    return session->multicast_ttl;
}


/**
 *@param session: a rtp session
 *@param yesno: enable multicast loopback
 *
 * Enable multicast loopback.
 *
 * Returns: 0 on success.
 *
**/
int rtp_session_set_multicast_loopback(RtpSession *session, int yesno)
{
    int retval;

    // Store new loopback state if one is specified
    if (yesno==0) {
        // Don't loop back
        session->multicast_loopback = 0;
    } else if (yesno>0) {
        // Do loop back
        session->multicast_loopback = 1;
    }

    // Don't do anything if socket hasn't been created yet
    if (ipstack_invalid(session->rtp.gs.socket)) return 0;

    switch (session->rtp.gs.sockfamily) {
    case IPSTACK_AF_INET: {

        retval= ipstack_setsockopt(session->rtp.gs.socket, IPSTACK_IPPROTO_IP, IPSTACK_IP_MULTICAST_LOOP,
                           (SOCKET_OPTION_VALUE)   &session->multicast_loopback, sizeof(session->multicast_loopback));

        if (retval<0) break;

        retval= ipstack_setsockopt(session->rtcp.gs.socket, IPSTACK_IPPROTO_IP, IPSTACK_IP_MULTICAST_LOOP,
                           (SOCKET_OPTION_VALUE)   &session->multicast_loopback, sizeof(session->multicast_loopback));

    } break;
    case IPSTACK_AF_INET6: {

        retval= ipstack_setsockopt(session->rtp.gs.socket, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_MULTICAST_LOOP,
                           (SOCKET_OPTION_VALUE)	&session->multicast_loopback, sizeof(session->multicast_loopback));

        if (retval<0) break;

        retval= ipstack_setsockopt(session->rtcp.gs.socket, IPSTACK_IPPROTO_IPV6, IPSTACK_IPV6_MULTICAST_LOOP,
                           (SOCKET_OPTION_VALUE)	&session->multicast_loopback, sizeof(session->multicast_loopback));
    } break;
    default:
        retval=-1;
    }

    if (retval<0)
        ortp_warning("Failed to set multicast loopback on socket.");

    return retval;
}


/**
 *rtp_session_get_multicast_loopback:
 *@param session: a rtp session
 *
 * Returns the multicast loopback state of rtp session (true or false).
 *
**/
int rtp_session_get_multicast_loopback(RtpSession *session)
{
    return session->multicast_loopback;
}

/**
 *rtp_session_set_dscp:
 *@param session: a rtp session
 *@param dscp: desired DSCP PHB value
 *
 * Sets the DSCP (Differentiated Services Code Point) for outgoing RTP packets.
 *
 * Returns: 0 on success.
 *
**/
int rtp_session_set_dscp(RtpSession *session, int dscp){
    int retval=0;
    int tos;
    int proto;
    int value_type;

    // Store new DSCP value if one is specified
    if (dscp>=0) session->dscp = dscp;

    // Don't do anything if socket hasn't been created yet
    if (ipstack_invalid(session->rtp.gs.socket)) return 0;

        // DSCP value is in the upper six bits of the TOS field
        tos = (session->dscp << 2) & 0xFC;
        switch (session->rtp.gs.sockfamily) {
        case IPSTACK_AF_INET:
            proto=IPSTACK_IPPROTO_IP;
            value_type=IPSTACK_IP_TOS;
            break;
        case IPSTACK_AF_INET6:
            proto=IPSTACK_IPPROTO_IPV6;
#	ifdef IPSTACK_IPV6_TCLASS /*seems not defined by my libc*/
            value_type=IPSTACK_IPV6_TCLASS;
#	else
            value_type=IPSTACK_IP_TOS;
#	endif
            break;
        default:
            ortp_error("Cannot set DSCP because socket family is unspecified.");
            return -1;
        }
        retval = ipstack_setsockopt(session->rtp.gs.socket, proto, value_type, (SOCKET_OPTION_VALUE)&tos, sizeof(tos));
        if (retval==-1)
            ortp_error("Fail to set DSCP value on rtp socket: %s",getSocketError());
        if (!ipstack_invalid(session->rtcp.gs.socket)){
            if (ipstack_setsockopt(session->rtcp.gs.socket, proto, value_type, (SOCKET_OPTION_VALUE)&tos, sizeof(tos))==-1){
                ortp_error("Fail to set DSCP value on rtcp socket: %s",getSocketError());
            }
        }
    return retval;
}


/**
 *rtp_session_get_dscp:
 *@param session: a rtp session
 *
 * Returns the DSCP (Differentiated Services Code Point) for outgoing RTP packets.
 *
**/
int rtp_session_get_dscp(const RtpSession *session)
{
    return session->dscp;
}


/**
 *rtp_session_get_local_port:
 *@param session:	a rtp session for which rtp_session_set_local_addr() or rtp_session_set_remote_addr() has been called
 *
 *	This function can be useful to retrieve the local port that was randomly choosen by
 *	rtp_session_set_remote_addr() when rtp_session_set_local_addr() was not called.
 *
 *	Returns: the local port used to listen for rtp packets, -1 if not set.
**/

int rtp_session_get_local_port(const RtpSession *session){
    return (session->rtp.gs.loc_port>0) ? session->rtp.gs.loc_port : -1;
}

int rtp_session_get_local_rtcp_port(const RtpSession *session){
    return (session->rtcp.gs.loc_port>0) ? session->rtcp.gs.loc_port : -1;
}

/**
 *rtp_session_set_remote_addr:
 *@param session:		a rtp session freshly created.
 *@param addr:		a remote IP address in the xxx.xxx.xxx.xxx form.
 *@param port:		a remote port.
 *
 *	Sets the remote address of the rtp session, ie the destination address where rtp packet
 *	are sent. If the session is recv-only or duplex, it also sets the origin of incoming RTP
 *	packets. Rtp packets that don't come from addr:port are discarded.
 *
 *	Returns: 0 on success.
**/
int
rtp_session_set_remote_addr (RtpSession * session, const char * addr, int port){
    return rtp_session_set_remote_addr_full(session, addr, port, addr, port+1);
}

/**
 *rtp_session_set_remote_addr_full:
 *@param session:		a rtp session freshly created.
 *@param rtp_addr:		a remote IP address in the xxx.xxx.xxx.xxx form.
 *@param rtp_port:		a remote rtp port.
 *@param rtcp_addr:		a remote IP address in the xxx.xxx.xxx.xxx form.
 *@param rtcp_port:		a remote rtcp port.
 *
 *	Sets the remote address of the rtp session, ie the destination address where rtp packet
 *	are sent. If the session is recv-only or duplex, it also sets the origin of incoming RTP
 *	packets. Rtp packets that don't come from addr:port are discarded.
 *
 *	Returns: 0 on success.
**/

int
rtp_session_set_remote_addr_full (RtpSession * session, const char * rtp_addr, int rtp_port, const char * rtcp_addr, int rtcp_port){
    return _rtp_session_set_remote_addr_full(session,rtp_addr,rtp_port,rtcp_addr,rtcp_port,FALSE);
}

static int
_rtp_session_set_remote_addr_full (RtpSession * session, const char * rtp_addr, int rtp_port, const char * rtcp_addr, int rtcp_port, bool_t is_aux){
    char rtp_printable_addr[64];
    char rtcp_printable_addr[64];
    int err;
    //struct addrinfo *res0, *res;
    OrtpAddress *rtp_saddr=&session->rtp.gs.rem_addr;
    socklen_t *rtp_saddr_len=&session->rtp.gs.rem_addr.len;
    OrtpAddress *rtcp_saddr=&session->rtcp.gs.rem_addr;
    socklen_t *rtcp_saddr_len=&session->rtcp.gs.rem_addr.len;
    OrtpAddress *aux_rtp=NULL,*aux_rtcp=NULL;
    struct ipstack_sockaddr sa;
    socklen_t salen = 0;
    int sa_family = 0;

    if (is_aux){
        aux_rtp=ortp_malloc(sizeof(OrtpAddress));
        rtp_saddr=aux_rtp;
        rtp_saddr_len=&aux_rtp->len;
        aux_rtcp=ortp_malloc(sizeof(OrtpAddress));
        rtcp_saddr=aux_rtcp;
        rtcp_saddr_len=&aux_rtcp->len;
    }
    err = ortp_address_to_sockaddr(ipstack_invalid(session->rtp.gs.socket) ? IPSTACK_AF_INET : session->rtp.gs.sockfamily,
                                   rtp_addr, rtp_port, &sa, &salen);
    //res0 = bctbx_name_to_addrinfo((session->rtp.gs.socket == -1) ? IPSTACK_AF_UNSPEC : session->rtp.gs.sockfamily, IPSTACK_SOCK_DGRAM, rtp_addr, rtp_port);
    //if (res0 == NULL) {
    if (err != 0) {
        ortp_error("_rtp_session_set_remote_addr_full(): cannot set RTP destination to %s port %i.", rtp_addr, rtp_port);
        err=-1;
        goto end;
    } else {
        ortp_sockaddr_to_print_address(&sa, salen, rtp_printable_addr, sizeof(rtp_printable_addr));
    }
    if (ipstack_invalid(session->rtp.gs.socket)){
        /* the session has not its socket bound, do it */
        ortp_message ("Setting random local addresses.");
        /* bind to an address type that matches the destination address */
        if (sa_family==IPSTACK_AF_INET6)
            err = rtp_session_set_local_addr (session, "::", -1, -1);
        else err=rtp_session_set_local_addr (session, "0.0.0.0", -1, -1);
        if (err<0) {
            err=-1;
            goto end;
        }
    }

    err=-1;

    memcpy(&rtp_saddr->addr, &sa, salen);
    *rtp_saddr_len=(socklen_t)salen;
    rtp_saddr->family = sa.sa_family;
    rtp_saddr->len = salen;
    err=0;

    if (err) {
        ortp_warning("Could not set destination for RTP socket to %s:%i.",rtp_addr,rtp_port);
        goto end;
    }

    if ((rtcp_addr != NULL) && (rtcp_port > 0)) {
        err = ortp_address_to_sockaddr(ipstack_invalid(session->rtp.gs.socket) ? IPSTACK_AF_INET : session->rtcp.gs.sockfamily, rtcp_addr, rtcp_port, &sa, &salen);

        //res0 = bctbx_name_to_addrinfo(ipstack_invalid(session->rtcp.gs.socket) ? IPSTACK_AF_UNSPEC : session->rtcp.gs.sockfamily, IPSTACK_SOCK_DGRAM, rtcp_addr, rtcp_port);
        //if (res0 == NULL) {
        if (err != 0) {
            ortp_error("_rtp_session_set_remote_addr_full(): cannot set RTCP destination to %s port %i.", rtcp_addr, rtcp_port);
            err=-1;
            goto end;
        } else {
            ortp_sockaddr_to_print_address(&sa, salen, rtcp_printable_addr, sizeof(rtcp_printable_addr));
        }
        err=-1;

        memcpy(&rtcp_saddr->addr, &sa, salen);
        *rtcp_saddr_len=(socklen_t)salen;
        rtcp_saddr->family = sa.sa_family;
        rtcp_saddr->len = salen;
        err = 0;

        if (err) {
            ortp_warning("Could not set destination for RCTP socket to %s:%i.",rtcp_addr,rtcp_port);
            goto end;
        }

        if (can_connect(session)){
            if (try_connect(session->rtp.gs.socket,(struct ipstack_sockaddr*)&session->rtp.gs.rem_addr.addr,session->rtp.gs.rem_addr.len))
                session->flags|=RTP_SOCKET_CONNECTED;
            if (!ipstack_invalid(session->rtcp.gs.socket)){
                if (try_connect(session->rtcp.gs.socket,(struct ipstack_sockaddr*)&session->rtcp.gs.rem_addr.addr,session->rtcp.gs.rem_addr.len))
                    session->flags|=RTCP_SOCKET_CONNECTED;
            }
        }else if (session->flags & RTP_SOCKET_CONNECTED){
            /*must dissolve association done by connect().
            See connect(2) manpage*/
            //struct ipstack_sockaddr sa;
            sa.sa_family=IPSTACK_AF_UNSPEC;
            if (ipstack_connect(session->rtp.gs.socket,&sa,sizeof(sa))<0){
                ortp_error("Cannot dissolve connect() association for rtp socket: %s", getSocketError());
            }
            if (ipstack_connect(session->rtcp.gs.socket,&sa,sizeof(sa))<0){
                ortp_error("Cannot dissolve connect() association for rtcp socket: %s", getSocketError());
            }
            session->flags&=~RTP_SOCKET_CONNECTED;
            session->flags&=~RTCP_SOCKET_CONNECTED;
        }

        ortp_message("RtpSession [%p] sending to rtp %s rtcp %s %s", session, rtp_printable_addr, rtcp_printable_addr, is_aux ? "as auxiliary destination" : "");
    } else {
        ortp_message("RtpSession [%p] sending to rtp %s %s", session, rtp_printable_addr, is_aux ? "as auxiliary destination" : "");
    }
    /*Apply DSCP setting. On windows the destination address is required for doing this.*/
    rtp_session_set_dscp(session, -1);
end:
    if (is_aux){
        if (err==-1){
            ortp_free(aux_rtp);
            ortp_free(aux_rtcp);
        }else{
            session->rtp.gs.aux_destinations=o_list_append(session->rtp.gs.aux_destinations,aux_rtp);
            session->rtcp.gs.aux_destinations=o_list_append(session->rtcp.gs.aux_destinations,aux_rtcp);
        }
    }
    return err;
}

int rtp_session_set_remote_addr_and_port(RtpSession * session, const char * addr, int rtp_port, int rtcp_port){
    return rtp_session_set_remote_addr_full(session,addr,rtp_port,addr,rtcp_port);
}

/**
 *rtp_session_add_remote_aux_addr_full:
 *@param session:		a rtp session freshly created.
 *@param rtp_addr:		a local IP address in the xxx.xxx.xxx.xxx form.
 *@param rtp_port:		a local rtp port.
 *@param rtcp_addr:		a local IP address in the xxx.xxx.xxx.xxx form.
 *@param rtcp_port:		a local rtcp port.
 *
 *	Add an auxiliary remote address for the rtp session, ie a destination address where rtp packet
 *	are sent.
 *
 *	Returns: 0 on success.
**/

int
rtp_session_add_aux_remote_addr_full(RtpSession * session, const char * rtp_addr, int rtp_port, const char * rtcp_addr, int rtcp_port){
    return _rtp_session_set_remote_addr_full(session,rtp_addr,rtp_port,rtcp_addr,rtcp_port,TRUE);
}

void rtp_session_clear_aux_remote_addr(RtpSession * session){
    ortp_stream_clear_aux_addresses(&session->rtp.gs);
    ortp_stream_clear_aux_addresses(&session->rtcp.gs);
}

void rtp_session_set_sockets(RtpSession *session, ortp_socket_t rtpfd, ortp_socket_t rtcpfd){
    if (ipstack_invalid(rtpfd)) set_non_blocking_socket(rtpfd);
    if (ipstack_invalid(rtcpfd)) set_non_blocking_socket(rtcpfd);
    session->rtp.gs.socket=rtpfd;
    session->rtcp.gs.socket=rtcpfd;
    /*
    if (rtpfd!=-1 || rtcpfd!=-1 )
        session->flags|=(RTP_SESSION_USING_EXT_SOCKETS|RTP_SOCKET_CONNECTED|RTCP_SOCKET_CONNECTED);
    else session->flags&=~(RTP_SESSION_USING_EXT_SOCKETS|RTP_SOCKET_CONNECD|RTCP_SOCKET_CONNECTETED);
    */
    if (ipstack_invalid(rtpfd) || ipstack_invalid(rtcpfd) )
        session->flags|=(RTP_SESSION_USING_EXT_SOCKETS);
    else
        session->flags&=~(RTP_SESSION_USING_EXT_SOCKETS);

    _rtp_session_apply_socket_sizes(session);
}

void rtp_session_set_overtcp(RtpSession *session, bool_t rtsp, int rtp_channel, int rtcp_channel)
{
    session->rtp_channel=rtp_channel;
    session->rtcp_channel=rtcp_channel;
    if(rtsp)
    {
        if (rtcp_channel!=-1 || rtp_channel!=-1 )
            session->flags|=(RTP_SESSION_USING_OVER_RTSPTCP_SOCKETS);
        else
            session->flags&=~(RTP_SESSION_USING_OVER_RTSPTCP_SOCKETS);
    }
    else
    {
        if (rtcp_channel!=-1 || rtp_channel!=-1 )
            session->flags|=(RTP_SESSION_USING_OVER_TCP_SOCKETS);
        else
            session->flags&=~(RTP_SESSION_USING_OVER_TCP_SOCKETS);
    }
}

void rtp_session_set_transports(RtpSession *session, struct _RtpTransport *rtptr, struct _RtpTransport *rtcptr)
{
    session->rtp.gs.tr = rtptr;
    session->rtcp.gs.tr = rtcptr;
    if (rtptr)
        rtptr->session=session;
    if (rtcptr)
        rtcptr->session=session;

    if (rtptr || rtcptr )
        session->flags|=(RTP_SESSION_USING_TRANSPORT);
    else session->flags&=~(RTP_SESSION_USING_TRANSPORT);
}

void rtp_session_get_transports(const RtpSession *session, RtpTransport **rtptr, RtpTransport **rtcptr){
    if (rtptr) *rtptr=session->rtp.gs.tr;
    if (rtcptr) *rtcptr=session->rtcp.gs.tr;
}


/**
 *rtp_session_flush_sockets:
 *@param session: a rtp session
 *
 * Flushes the sockets for all pending incoming packets.
 * This can be usefull if you did not listen to the stream for a while
 * and wishes to start to receive again. During the time no receive is made
 * packets get bufferised into the internal kernel socket structure.
 *
**/
void rtp_session_flush_sockets(RtpSession *session){
    rtp_session_set_flag(session, RTP_SESSION_FLUSH);
    rtp_session_rtp_recv(session, 0);
    rtp_session_unset_flag(session, RTP_SESSION_FLUSH);
}



#ifdef USE_SENDMSG
#if 1
static int rtp_sendmsg(ortp_socket_t sock,mblk_t *m, const struct ipstack_sockaddr *rem_addr, socklen_t addr_len){
    struct ipstack_msghdr msg;
    struct ipstack_iovec iov[3];
    int iovlen = 1;
    u_char control_buffer[512] = {0};
    int controlSize = 0;				// Used to reset msg.msg_controllen to the real control size
    struct ipstack_cmsghdr *cmsg = NULL;
    int error = 0;

    iov[0].iov_base=m->b_rptr;
    iov[0].iov_len=m->b_wptr-m->b_rptr;

    msg.msg_name=(void*)rem_addr;
    msg.msg_namelen=addr_len;
    msg.msg_iov=&iov[0];
    msg.msg_iovlen=iovlen;
    msg.msg_flags=0;
    msg.msg_control=control_buffer;
    msg.msg_controllen=sizeof(control_buffer);

    cmsg = CMSG_FIRSTHDR(&msg);
#ifdef IPV6_PKTINFO
    if( m->recv_addr.family == IPSTACK_AF_INET6 && !IN6_IS_ADDR_UNSPECIFIED(&m->recv_addr.addr.ipi6_addr) && !IN6_IS_ADDR_LOOPBACK(&m->recv_addr.addr.ipi6_addr))
    {// Add IPV6 to the message control. We only add it if the IP is specified and is not link local
        {
            struct ipstack_in6_pktinfo *pktinfo;
            cmsg->cmsg_len = CMSG_LEN(sizeof(struct ipstack_in6_pktinfo));
            cmsg->cmsg_level = IPSTACK_IPPROTO_IPV6;
            cmsg->cmsg_type = IPV6_PKTINFO;
            pktinfo = (struct ipstack_in6_pktinfo*) CMSG_DATA(cmsg);
            pktinfo->ipi6_ifindex = 0;	// Set to 0 to let the kernel to use routable interface
            pktinfo->ipi6_addr = m->recv_addr.addr.ipi6_addr;
            controlSize += CMSG_SPACE(sizeof(struct ipstack_in6_pktinfo));
            cmsg = CMSG_NXTHDR(&msg, cmsg);
        }
    }
#endif
#ifdef IPSTACK_IP_PKTINFO
    if( m->recv_addr.family == IPSTACK_AF_INET)
    {// Add IPV4 to the message control
        struct ipstack_in_pktinfo *pktinfo;
        cmsg->cmsg_len = CMSG_LEN(sizeof(struct ipstack_in_pktinfo));
        cmsg->cmsg_level = IPSTACK_IPPROTO_IP;
        cmsg->cmsg_type = IPSTACK_IP_PKTINFO;
        pktinfo = (struct ipstack_in_pktinfo*) CMSG_DATA(cmsg);
        pktinfo->ipi_spec_dst = m->recv_addr.addr.ipi_addr;
        controlSize += CMSG_SPACE(sizeof(struct ipstack_in_pktinfo));
        cmsg = CMSG_NXTHDR(&msg, cmsg);
    }
#endif

    msg.msg_controllen = controlSize;
    if( controlSize==0) // Have to reset msg_control to NULL as msg_controllen is not sufficient on some platforms
        msg.msg_control = NULL;

    error = ipstack_sendmsg(sock,&msg,0);

    if( error == -1 && controlSize != 0 && (errno == EINVAL || errno==ENETUNREACH || errno==EFAULT)) {
        fprintf(stdout, "============rtp_sendmsg=========%s\r\n", strerror(errno));
        msg.msg_controllen =0;
        msg.msg_control = NULL;
        error = ipstack_sendmsg(sock,&msg,0);
    }
    else if(error < 0 && errno==EAGAIN)
    {
        fprintf(stdout, "============rtp_sendmsg====aaa=====%s\r\n", strerror(errno));
        if(controlSize != 0)
        {
            msg.msg_controllen =0;
            msg.msg_control = NULL;
        }
        error = ipstack_sendmsg(sock,&msg,0);
    }
    return error;
}
#else
#define MAX_IOV 64
static int rtp_sendmsg(ortp_socket_t sock,mblk_t *m, const struct sockaddr *rem_addr, socklen_t addr_len){
    struct msghdr msg;
    struct iovec iov[MAX_IOV];
    int iovlen;
    u_char control_buffer[512] = {0};
    mblk_t * m_track = m;
    int controlSize = 0;				// Used to reset msg.msg_controllen to the real control size
    struct cmsghdr *cmsg;
    int error;

    for(iovlen=0; iovlen<MAX_IOV && m_track!=NULL; m_track=m_track->b_cont,iovlen++){
        iov[iovlen].iov_base=m_track->b_rptr;
        iov[iovlen].iov_len=m_track->b_wptr-m_track->b_rptr;
    }
    if (iovlen==MAX_IOV){
        int count = 0;
        while (m_track!=NULL){
            count++;
            m_track=m_track->b_cont;
        }
        ortp_error("Too long msgb (%i fragments) , didn't fit into iov, end discarded.", MAX_IOV+count);
    }
    msg.msg_name=(void*)rem_addr;
    msg.msg_namelen=addr_len;
    msg.msg_iov=&iov[0];
    msg.msg_iovlen=iovlen;
    msg.msg_flags=0;
    msg.msg_control=control_buffer;
    msg.msg_controllen=sizeof(control_buffer);

    cmsg = CMSG_FIRSTHDR(&msg);
#ifdef IPV6_PKTINFO
    if( m->recv_addr.family == IPSTACK_AF_INET6 && !IN6_IS_ADDR_UNSPECIFIED(&m->recv_addr.addr.ipi6_addr) && !IN6_IS_ADDR_LOOPBACK(&m->recv_addr.addr.ipi6_addr))
    {// Add IPV6 to the message control. We only add it if the IP is specified and is not link local
        {
            struct in6_pktinfo *pktinfo;
            cmsg->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
            cmsg->cmsg_level = IPSTACK_IPPROTO_IPV6;
            cmsg->cmsg_type = IPV6_PKTINFO;
            pktinfo = (struct in6_pktinfo*) CMSG_DATA(cmsg);
            pktinfo->ipi6_ifindex = 0;	// Set to 0 to let the kernel to use routable interface
            pktinfo->ipi6_addr = m->recv_addr.addr.ipi6_addr;
            controlSize += CMSG_SPACE(sizeof(struct in6_pktinfo));
            cmsg = CMSG_NXTHDR(&msg, cmsg);
        }
    }
#endif
#ifdef IPSTACK_IP_PKTINFO
    if( m->recv_addr.family == IPSTACK_AF_INET)
    {// Add IPV4 to the message control
        struct in_pktinfo *pktinfo;
        cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
        cmsg->cmsg_level = IPSTACK_IPPROTO_IP;
        cmsg->cmsg_type = IPSTACK_IP_PKTINFO;
        pktinfo = (struct in_pktinfo*) CMSG_DATA(cmsg);
        pktinfo->ipi_spec_dst = m->recv_addr.addr.ipi_addr;
        controlSize += CMSG_SPACE(sizeof(struct in_pktinfo));
        cmsg = CMSG_NXTHDR(&msg, cmsg);
    }
#endif

    msg.msg_controllen = controlSize;
    if( controlSize==0) // Have to reset msg_control to NULL as msg_controllen is not sufficient on some platforms
        msg.msg_control = NULL;
    error = sendmsg((int)sock,&msg,0);
    //fprintf(stdout, "============rtp_sendmsg=========%s\r\n", strerror(errno));
    if( error == -1 && controlSize != 0 && (errno == EINVAL || errno==ENETUNREACH || errno==EFAULT)) {
        fprintf(stdout, "============rtp_sendmsg=========%s\r\n", strerror(errno));
        msg.msg_controllen =0;
        msg.msg_control = NULL;
        error = sendmsg((int)sock,&msg,0);
    }
    else if(error < 0 && errno==EAGAIN)
    {
        fprintf(stdout, "============rtp_sendmsg====aaa=====%s\r\n", strerror(errno));
        if(controlSize != 0)
        {
            msg.msg_controllen =0;
            msg.msg_control = NULL;
        }
        error = sendmsg((int)sock,&msg,0);
    }
    return error;
}
#endif
#endif


ortp_socket_t rtp_session_get_socket(RtpSession *session, bool_t is_rtp){
    return is_rtp ? session->rtp.gs.socket : session->rtcp.gs.socket;
}



int _ortp_sendto(ortp_socket_t sockfd, mblk_t *m, int flags, const struct ipstack_sockaddr *destaddr, socklen_t destlen) {
    int sent_bytes = 0;
#if defined(_WIN32) || defined(_WIN32_WCE) || defined(USE_SENDMSG)
    sent_bytes = rtp_sendmsg(sockfd, m, destaddr, destlen);
#else
    if (m->b_cont != NULL)
        msgpullup(m, -1);

    //fprintf(stdout, "============================_ortp_sendto==========================seq=%d\r\n",ntohs(rtp_session_mblk_get_seq(m)));
    //fflush(stdout);

    uint8_t *ptr = (uint8_t*)m->b_rptr;
    int ptrsize = (int)(m->b_wptr - m->b_rptr);
    int sbyte = 0;
    ortp_hdr_log_out((char*)ptr);
    while(ptrsize)
    {
        sbyte = ipstack_sendto(sockfd, (char*)ptr, ptrsize, 0, destaddr, destlen);
        if(sbyte == ptrsize)
        {
            sent_bytes += sbyte;
            break;
        }
        else if(sbyte > 0)
        {
            sent_bytes += sbyte;
            ptr += sbyte;
            ptrsize -= sbyte;
        }
        else if(sbyte == 0)
        {
            if(errno==EAGAIN || errno==EINTR)
            {
                continue;
            }
            else//if(errno==EPIPE)
            {
                sent_bytes = -1;
                break;
            }
        }
        else
        {
            if(errno==EAGAIN || errno==EINTR)
            {
                continue;
            }
            else //if(errno==EPIPE)
            {
                sent_bytes = -1;
                break;
            }
        }
        sent_bytes += sbyte;
        break;
    }
    if(sent_bytes <= 0)
        fprintf(stdout, "============_ortp_sendto=========%s\r\n", strerror(errno));
    /*
    sent_bytes = sendto(sockfd, (char*)ptr, ptrsize, 0, destaddr, destlen);

    if(sent_bytes < 0 && errno==EAGAIN)
    {
        sent_bytes = sendto(sockfd, (char*)rptr, ptrsize, 0, destaddr, destlen);
    }
*/
#endif
    return sent_bytes;
}

int rtp_session_sendto(RtpSession *session, bool_t is_rtp, mblk_t *m, int flags, const struct ipstack_sockaddr *destaddr, socklen_t destlen){
    int ret;


    _rtp_session_check_socket_refresh(session);

    if (session->net_sim_ctx && (session->net_sim_ctx->params.mode==OrtpNetworkSimulatorOutbound
                                 || session->net_sim_ctx->params.mode==OrtpNetworkSimulatorOutboundControlled)){
        ret=(int)msgdsize(m);
        m=dupmsg(m);
        if(destaddr->sa_family == IPSTACK_AF_INET)
        {
            memcpy(&m->net_addr.addr.ipv4,destaddr,destlen);
            m->net_addr.len=destlen;
        }
        if(destaddr->sa_family == IPSTACK_AF_INET6)
        {
            memcpy(&m->net_addr.addr.ipv6,destaddr,destlen);
            m->net_addr.len=destlen;
        }
        m->net_addr.family = destaddr->sa_family;
        m->reserved1=is_rtp;
        ortp_mutex_lock(&session->net_sim_ctx->mutex);
        putq(&session->net_sim_ctx->send_q, m);
        ortp_mutex_unlock(&session->net_sim_ctx->mutex);
    }else{
        ortp_socket_t sockfd = rtp_session_get_socket(session, is_rtp || session->rtcp_mux);
        if (!ipstack_invalid(sockfd)){
            //fprintf(stdout, "============================rtp_session_sendto=======================seq=%d\r\n",ntohs(rtp_session_mblk_get_seq(m)));
            //fflush(stdout);
            ret=_ortp_sendto(sockfd, m, flags, destaddr, destlen);
        }else{
            ret = -1;
        }
    }
    return ret;
}



int rtp_session_recvfrom(RtpSession *session, bool_t is_rtp, mblk_t *m, int flags, struct ipstack_sockaddr *from, socklen_t *fromlen) {
    int ret = rtp_session_rtp_recv_abstract(is_rtp ? session->rtp.gs.socket : session->rtcp.gs.socket, m, flags, from, fromlen);
    if (ret >= 0)
    {
        /* Store the local port in the recv_addr of the mblk_t, the address is already filled in rtp_session_rtp_recv_abstract */
        m->recv_addr.port = htons(is_rtp ? session->rtp.gs.loc_port : session->rtcp.gs.loc_port);

        /*if(session->rtp.gs.rem_addr.family == 0)
        {
            if(from->sa_family == IPSTACK_AF_INET)
            {
                memcpy(&session->rtp.gs.rem_addr.addr.ipv4,from,*fromlen);
            }
            if(from->sa_family == IPSTACK_AF_INET6)
            {
                memcpy(&session->rtp.gs.rem_addr.addr.ipv6,from,*fromlen);
            }
            session->rtp.gs.rem_addr.family = from->sa_family;
            session->rtp.gs.rem_addr.len = *fromlen;
        }*/
    }
    return ret;
}

void update_sent_bytes(OrtpStream *os, int nbytes) {
    int overhead = ortp_stream_is_ipv6(os) ? IP6_UDP_OVERHEAD : IP_UDP_OVERHEAD;
    if ((os->sent_bytes == 0) && (os->send_bw_start.tv_sec == 0) && (os->send_bw_start.tv_usec == 0)) {
        /* Initialize bandwidth computing time when has not been started yet. */
        ortp_gettimeofday(&os->send_bw_start, NULL);
    }
    os->sent_bytes += nbytes + overhead;
}

static void update_recv_bytes(OrtpStream *os, size_t nbytes, const struct timeval *recv_time) {
    int overhead = ortp_stream_is_ipv6(os) ? IP6_UDP_OVERHEAD : IP_UDP_OVERHEAD;
    if ((os->recv_bytes == 0) && (os->recv_bw_start.tv_sec == 0) && (os->recv_bw_start.tv_usec == 0)) {
        ortp_gettimeofday(&os->recv_bw_start, NULL);
    }
    os->recv_bytes += (unsigned int)(nbytes + overhead);
    ortp_bw_estimator_packet_received(&os->recv_bw_estimator, nbytes + overhead, recv_time);
}

/*
void _log_send_error_func(const char *func, int line, RtpSession *session, const char *type, mblk_t *m, struct sockaddr *destaddr, socklen_t destlen){
    char printable_ip_address[65]={0};
    int errnum = getSocketErrorCode();
    const char *errstr = getSocketError();
    ortp_sockaddr_to_print_address(destaddr, destlen, printable_ip_address, sizeof(printable_ip_address));
    ortp_error ("%s[%d]: RtpSession error sending [%s] packet to %s: %s [%d]",
                func, line, type, printable_ip_address, errstr, errnum);
}
*/

static int  rtp_session_rtp_tcp_sendto(RtpSession * session, mblk_t *msg , int rtp, const struct ipstack_sockaddr *to, socklen_t tolen)
{
    uint8_t *ptr = (uint8_t*)msg->b_rptr;
    int ptrsize = (int)(msg->b_wptr - msg->b_rptr);
    uint8_t tmpbuf[UDP_MAX_SIZE];
    rtp_tcp_header_t *hdr = (rtp_tcp_header_t *)tmpbuf;
    memset(tmpbuf, 0, sizeof(tmpbuf));
    memcpy(tmpbuf + sizeof(rtp_tcp_header_t), ptr, ptrsize);
    hdr->masker = '$';
    hdr->channel = rtp ? session->rtp_channel:session->rtcp_channel;
    hdr->length = htons(ptrsize);
    return ipstack_send(rtp ? session->rtp.gs.socket : session->rtcp.gs.socket, (char*)tmpbuf, ptrsize + sizeof(rtp_tcp_header_t), 0);
}

static int  rtp_session_rtp_tcp_recvfrom(RtpSession * session, mblk_t *msg , int rtp, const struct ipstack_sockaddr *to, socklen_t tolen)
{
    ortp_socket_t sock = rtp ? session->rtp.gs.socket : session->rtcp.gs.socket;
    int bufsz = (int) (msg->b_datap->db_lim - msg->b_datap->db_base);
    int ret = ipstack_recv(sock, (char *)msg->b_wptr, bufsz, 0);
    /*if(ret > sizeof(rtp_tcp_header_t))
    {
        msg->b_wptr += sizeof(rtp_tcp_header_t);
        return (ret - sizeof(rtp_tcp_header_t));
    }*/
    return ret;
}

int  rtp_session_tcp_forward(ortp_socket_t sock, const uint8_t *msg , int tolen)
{
    int ret = ipstack_send(sock, (char*)msg, tolen, 0);
    return ret;
}

static int rtp_session_rtp_sendto(RtpSession * session, mblk_t * m, struct ipstack_sockaddr *destaddr, socklen_t destlen, bool_t is_aux){
    int error = 0;

    if(session->flags & RTP_SESSION_USING_OVER_RTSPTCP_SOCKETS ||
            session->flags & RTP_SESSION_USING_OVER_TCP_SOCKETS)
    {
        error = rtp_session_rtp_tcp_sendto(session, m , 1, destaddr, destlen);
    }
    else
    {
        if (rtp_session_using_transport(session, rtp)){
            //fprintf(stdout, "============================rtp_session_rtp_sendto======rtp_session_using_transport=================seq=%d\r\n",ntohs(rtp_session_mblk_get_seq(m)));
            //fflush(stdout);
            error = (session->rtp.gs.tr->t_sendto) (session->rtp.gs.tr,m,0,destaddr,destlen);
        }else{
            //fprintf(stdout, "============================rtp_session_rtp_sendto=======================seq=%d\r\n",ntohs(rtp_session_mblk_get_seq(m)));
            //fflush(stdout);
            error=rtp_session_sendto(session, TRUE, m, 0, destaddr, destlen);
        }
    }
    if (!is_aux){
        /*errors to auxiliary destinations are not notified*/
        if (error < 0){
            if (session->on_network_error.count>0){
                rtp_signal_table_emit3(&session->on_network_error,"Error sending RTP packet",ORTP_INT_TO_POINTER(getSocketErrorCode()));
            }
            else
            {
                char printable_ip_address[65]={0};
                int errnum = getSocketErrorCode();
                const char *errstr = getSocketError();
                ortp_sockaddr_to_print_address(destaddr, destlen, printable_ip_address, sizeof(printable_ip_address));
                ortp_error (" RtpSession error sending [%s] packet to %s: %s [%d]",
                            "rtp", printable_ip_address, errstr, errnum);
            }
            session->rtp.send_errno=getSocketErrorCode();
        }else{
            update_sent_bytes(&session->rtp.gs, error);
        }
    }
    return error;
}

int rtp_session_rtp_send (RtpSession * session, mblk_t * m){
    int error=0;
    int i;
    rtp_header_t *hdr;
    struct ipstack_sockaddr *destaddr=(struct ipstack_sockaddr*)&session->rtp.gs.rem_addr.addr;
    socklen_t destlen=session->rtp.gs.rem_addr.len;
    OList *elem=NULL;

    if (session->is_spliced) {
        freemsg(m);
        return 0;
    }
    if( m->recv_addr.family == IPSTACK_AF_UNSPEC && session->rtp.gs.used_loc_addr.len>0 )
        ortp_sockaddr_to_recvaddr((const struct sipstack_ockaddr*)&session->rtp.gs.used_loc_addr.addr, &m->recv_addr);	// update recv_addr with the source of rtp
    hdr = (rtp_header_t *) m->b_rptr;
    if (hdr->version == 0) {
        /* We are probably trying to send a STUN packet so don't change its content. */
    } else {
        /* perform host to network conversions */
        hdr->ssrc = htonl (hdr->ssrc);
        hdr->timestamp = htonl (hdr->timestamp);
        hdr->seq_number = htons (hdr->seq_number);
        for (i = 0; i < hdr->cc; i++)
            hdr->csrc[i] = htonl (hdr->csrc[i]);
    }

    if (session->flags & RTP_SOCKET_CONNECTED) {
        destaddr=NULL;
        destlen=0;
    }
    //fprintf(stdout, "============================rtp_session_rtp_send==========================seq=%d\r\n",ntohs(rtp_session_mblk_get_seq(m)));
    //fflush(stdout);

    /*first send to main destination*/
    error=rtp_session_rtp_sendto(session,m,destaddr,destlen,FALSE);
    /*then iterate over auxiliary destinations*/
    for(elem=session->rtp.gs.aux_destinations;elem!=NULL;elem=elem->next){
        OrtpAddress *oaddr=(OrtpAddress*)elem->data;
        //fprintf(stdout, "============================rtp_session_rtp_send======aux_destinations=================seq=%d\r\n",ntohs(rtp_session_mblk_get_seq(m)));
        //fflush(stdout);
        rtp_session_rtp_sendto(session,m,(struct ipstack_sockaddr*)&oaddr->addr,oaddr->len,TRUE);
    }
    freemsg(m);
    return error;
}

static int rtp_session_rtcp_sendto(RtpSession * session, mblk_t * m, struct ipstack_sockaddr *destaddr, socklen_t destlen, bool_t is_aux){
    int error=0;
    if(session->flags & RTP_SESSION_USING_OVER_RTSPTCP_SOCKETS ||
            session->flags & RTP_SESSION_USING_OVER_TCP_SOCKETS)
    {
        error = rtp_session_rtp_tcp_sendto(session, m , 0, NULL, 0);
    }
    else
    {
        /* Even in RTCP mux, we send through the RTCP RtpTransport, which will itself take in charge to do the sending of the packet
         * through the RTP endpoint*/
        if (rtp_session_using_transport(session, rtcp)){
            error = (session->rtcp.gs.tr->t_sendto) (session->rtcp.gs.tr, m, 0, destaddr, destlen);
        }else{
            error=_ortp_sendto(rtp_session_get_socket(session, session->rtcp_mux),m,0,destaddr,destlen);
        }
    }
    if (!is_aux){
        if (error < 0){
            if (session->on_network_error.count>0){
                rtp_signal_table_emit3(&session->on_network_error,"Error sending RTCP packet",ORTP_INT_TO_POINTER(getSocketErrorCode()));
            }else{
                char printable_ip_address[65]={0};
                int errnum = getSocketErrorCode();
                const char *errstr = getSocketError();
                ortp_sockaddr_to_print_address(destaddr, destlen, printable_ip_address, sizeof(printable_ip_address));
                ortp_error (" RtpSession error sending [%s] packet to %s: %s [%d]",
                            "rtcp", printable_ip_address, errstr, errnum);
            }
        } else {
            update_sent_bytes(&session->rtcp.gs, error);
            update_avg_rtcp_size(session, error);
        }
    }
    return error;
}

int
rtp_session_rtcp_sendm_raw (RtpSession * session, mblk_t * m){
    int error=0;
    ortp_socket_t sockfd=session->rtcp.gs.socket;
    struct ipstack_sockaddr *destaddr=session->rtcp_mux ? (struct ipstack_sockaddr*)&session->rtp.gs.rem_addr.addr : (struct ipstack_sockaddr*)&session->rtcp.gs.rem_addr.addr;
    socklen_t destlen=session->rtcp_mux ? session->rtp.gs.rem_addr.len : session->rtcp.gs.rem_addr.len;
    OList *elem=NULL;
    bool_t using_connected_socket=(session->flags & RTCP_SOCKET_CONNECTED)!=0;

    if (session->is_spliced) {
        freemsg(m);
        return 0;
    }
    if (using_connected_socket) {
        destaddr=NULL;
        destlen=0;
    }
    if( m->recv_addr.family == IPSTACK_AF_UNSPEC && session->rtcp.gs.used_loc_addr.len>0 )
        ortp_sockaddr_to_recvaddr((const struct ipstack_sockaddr*)&session->rtcp.gs.used_loc_addr.addr, &m->recv_addr);	// update recv_addr with the source of rtcp
    if (session->rtcp.enabled){
        if ( (!ipstack_invalid(sockfd) && (destlen>0 || using_connected_socket))
             || rtp_session_using_transport(session, rtcp) ) {
            rtp_session_rtcp_sendto(session,m,destaddr,destlen,FALSE);
        }
        for(elem=session->rtcp.gs.aux_destinations;elem!=NULL;elem=elem->next){
            OrtpAddress *oaddr=(OrtpAddress*)elem->data;
            rtp_session_rtcp_sendto(session,m,(struct ipstack_sockaddr*)&oaddr->addr,oaddr->len,TRUE);
        }
    }else ortp_message("Not sending rtcp report, rtcp disabled.");

    //ortp_rtcp_msg_debug(session, (unsigned char*)m->b_rptr, (uint32_t)(m->b_wptr - m->b_rptr), NULL);
    freemsg(m);
    return error;
}

#ifdef USE_RECVMSG
static int rtp_recvmsg(ortp_socket_t socket, mblk_t *msg, int flags, struct ipstack_sockaddr *from, socklen_t *fromlen, struct ipstack_msghdr *msghdr, int bufsz) {
    struct ipstack_iovec iov;
    int error = 0;

    memset(&iov, 0, sizeof(iov));
    iov.iov_base = msg->b_wptr;
    iov.iov_len = bufsz;
    if ((from != NULL) && (fromlen != NULL)) {
        msghdr->msg_name = from;
        msghdr->msg_namelen = *fromlen;
    }
    msghdr->msg_iov = &iov;
    msghdr->msg_iovlen = 1;
    error = ipstack_recvmsg(socket, msghdr, flags);
    if (fromlen != NULL)
        *fromlen = msghdr->msg_namelen;
    return error;
}
#endif
int rtp_session_rtp_recv_abstract(ortp_socket_t socket, mblk_t *msg, int flags, struct ipstack_sockaddr *from, socklen_t *fromlen) {
    int ret;
    int bufsz = (int) (msg->b_datap->db_lim - msg->b_datap->db_base);
    char control[512] = {0};
    struct ipstack_msghdr msghdr = {0};
    msghdr.msg_control = control;
    msghdr.msg_controllen = sizeof(control);
    /*fd_set rset;
    FD_ZERO(&rset);
    FD_SET(socket, &rset);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 2000;
    ret = select(socket + 1, &rset, NULL, NULL, &tv);
    if(ret <= 0)
        return ret;*/
#ifdef USE_RECVMSG
    ret = rtp_recvmsg(socket, msg, flags, from, fromlen, &msghdr, bufsz);
#else
    ret = ipstack_recvfrom(socket, msg->b_wptr, bufsz, flags, from, fromlen);
#endif
    if(ret >= 0) {
        struct ipstack_cmsghdr *cmsghdr;

        for (cmsghdr = IPSTACK_CMSG_FIRSTHDR(&msghdr); cmsghdr != NULL ; cmsghdr = IPSTACK_CMSG_NXTHDR(&msghdr, cmsghdr)) {
#ifdef IPSTACK_SO_TIMESTAMP
            if (cmsghdr->cmsg_level == IPSTACK_SOL_SOCKET && cmsghdr->cmsg_type == SCM_TIMESTAMP) {
                memcpy(&msg->timestamp, (struct timeval *)IPSTACK_CMSG_DATA(cmsghdr), sizeof(struct timeval));
            }
#endif
#ifdef IPSTACK_IP_PKTINFO
            if ((cmsghdr->cmsg_level == IPSTACK_IPPROTO_IP) && (cmsghdr->cmsg_type == IPSTACK_IP_PKTINFO)) {
                struct ipstack_in_pktinfo *pi = (struct ipstack_in_pktinfo *)IPSTACK_CMSG_DATA(cmsghdr);
                memcpy(&msg->recv_addr.addr.ipi_addr, &pi->ipi_addr, sizeof(msg->recv_addr.addr.ipi_addr));
                msg->recv_addr.family = IPSTACK_AF_INET;
            }
#endif
#ifdef IPSTACK_IPV6_PKTINFO
            if ((cmsghdr->cmsg_level == IPSTACK_IPPROTO_IPV6) && (cmsghdr->cmsg_type == IPSTACK_IPV6_PKTINFO)) {
                struct ipstack_in6_pktinfo *pi = (struct ipstack_in6_pktinfo *)IPSTACK_CMSG_DATA(cmsghdr);
                memcpy(&msg->recv_addr.addr.ipi6_addr, &pi->ipi6_addr, sizeof(msg->recv_addr.addr.ipi6_addr));
                msg->recv_addr.family = IPSTACK_AF_INET6;
            }
#endif
#ifdef IPSTACK_IP_RECVDSTADDR
            if ((cmsghdr->cmsg_level == IPSTACK_IPPROTO_IP) && (cmsghdr->cmsg_type == IPSTACK_IP_RECVDSTADDR)) {
                struct ipstack_in_addr *ia = (struct in_addr *)IPSTACK_CMSG_DATA(cmsghdr);
                memcpy(&msg->recv_addr.addr.ipi_addr, ia, sizeof(msg->recv_addr.addr.ipi_addr));
                msg->recv_addr.family = IPSTACK_AF_INET;
            }
#endif
#ifdef IPSTACK_IPV6_RECVDSTADDR
            if ((cmsghdr->cmsg_level == IPSTACK_IPPROTO_IPV6) && (cmsghdr->cmsg_type == IPSTACK_IPV6_RECVDSTADDR)) {
                struct ipstack_in6_addr *ia = (struct in6_addr *)IPSTACK_CMSG_DATA(cmsghdr);
                memcpy(&msg->recv_addr.addr.ipi6_addr, ia, sizeof(msg->recv_addr.addr.ipi6_addr));
                msg->recv_addr.family = IPSTACK_AF_INET6;
            }
#endif
#ifdef IPSTACK_IP_RECVTTL
            if ((cmsghdr->cmsg_level == IPSTACK_IPPROTO_IP) && (cmsghdr->cmsg_type == IPSTACK_IP_TTL)) {
                uint32_t *ptr = (uint32_t *)IPSTACK_CMSG_DATA(cmsghdr);
                msg->ttl_or_hl = (*ptr & 0xFF);
            }
#endif
#ifdef IPSTACK_IPV6_RECVHOPLIMIT
            if ((cmsghdr->cmsg_level == IPSTACK_IPPROTO_IPV6) && (cmsghdr->cmsg_type == IPSTACK_IPV6_HOPLIMIT)) {
                uint32_t *ptr = (uint32_t *)IPSTACK_CMSG_DATA(cmsghdr);
                msg->ttl_or_hl = (*ptr & 0xFF);
            }
#endif
        }
        /*store recv addr for use by modifiers*/
        if (from && fromlen) {
            //fprintf(stdout, "%s[%d] ortp_WSARecvMsg net_addr %d \r\n", __func__, __LINE__, msg->net_addr);
            if(from->sa_family == IPSTACK_AF_INET)
            {
                memcpy(&msg->net_addr.addr.ipv4,from,*fromlen);
            }
            if(from->sa_family == IPSTACK_AF_INET6)
            {
                memcpy(&msg->net_addr.addr.ipv6,from,*fromlen);
            }
            msg->net_addr.family = from->sa_family;
            msg->net_addr.len = *fromlen;
        }
        //fprintf(stdout, "%s[%d] ========== %d byte\r\n", __func__, __LINE__, ret);
    }else{

        if(errno != EAGAIN)
            fprintf(stdout, "%s[%d] ============== errno %d(%s)\r\n", __func__, __LINE__, errno, strerror(errno));
    }
    return ret;
}

static void rtp_session_notify_inc_rtcp(RtpSession *session, mblk_t *m, bool_t received_via_rtcp_mux){
    if (session->eventqs!=NULL){
        OrtpEvent *ev=ortp_event_new(ORTP_EVENT_RTCP_PACKET_RECEIVED);
        OrtpEventData *d=ortp_event_get_data(ev);
        d->packet=m;
        d->info.socket_type = received_via_rtcp_mux ? OrtpRTPSocket : OrtpRTCPSocket;
        rtp_session_dispatch_event(session,ev);
    }
    else freemsg(m);  /* avoid memory leak */
}

static void compute_rtt(RtpSession *session, const struct timeval *now, uint32_t lrr, uint32_t dlrr){
    uint64_t curntp=ortp_timeval_to_ntp(now);
    uint32_t approx_ntp=(curntp>>16) & 0xFFFFFFFF;
    /*ortp_message("rtt approx_ntp=%u, lrr=%u, dlrr=%u",approx_ntp,lrr,dlrr);*/
    if (lrr!=0 && dlrr!=0){
        /*we cast to int32_t to check for crazy RTT time (negative)*/
        double rtt_frac=(int32_t)(approx_ntp-lrr-dlrr);
        if (rtt_frac>=0){
            rtt_frac/=65536.0;

            session->rtt=(float)rtt_frac;
            /*ortp_message("rtt estimated to %f s",session->rtt);*/
        }else ortp_warning("Negative RTT computation, maybe due to clock adjustments.");
    }
}

static void compute_rtt_from_report_block(RtpSession *session, const struct timeval *now, const report_block_t *rb) {
    uint32_t last_sr_time = report_block_get_last_SR_time(rb);
    uint32_t sr_delay = report_block_get_last_SR_delay(rb);
    compute_rtt(session, now, last_sr_time, sr_delay);
    session->cum_loss = report_block_get_cum_packet_lost(rb);
}

static void compute_rtcp_xr_statistics(RtpSession *session, mblk_t *block, const struct timeval *now) {
    uint64_t ntp_timestamp;
    OrtpRtcpXrStats *stats = &session->rtcp_xr_stats;

    switch (rtcp_XR_get_block_type(block)) {
    case RTCP_XR_RCVR_RTT:
        ntp_timestamp = rtcp_XR_rcvr_rtt_get_ntp_timestamp(block);
        stats->last_rcvr_rtt_ts = (ntp_timestamp >> 16) & 0xffffffff;
        stats->last_rcvr_rtt_time.tv_sec = now->tv_sec;
        stats->last_rcvr_rtt_time.tv_usec = now->tv_usec;
        break;
    case RTCP_XR_DLRR:
        compute_rtt(session, now, rtcp_XR_dlrr_get_lrr(block), rtcp_XR_dlrr_get_dlrr(block));
        break;
    default:
        break;
    }
}

static void handle_rtcp_rtpfb_packet(RtpSession *session, mblk_t *block) {
    switch (rtcp_RTPFB_get_type(block)) {
    case RTCP_RTPFB_TMMBR:
        if (session->rtcp.tmmbr_info.received) freemsg(session->rtcp.tmmbr_info.received);
        session->rtcp.tmmbr_info.received = copymsg(block);
        rtp_session_send_rtcp_fb_tmmbn(session, rtcp_RTPFB_get_packet_sender_ssrc(block));
        break;
    case RTCP_RTPFB_TMMBN:
        if (session->rtcp.tmmbr_info.sent) {
            rtcp_fb_tmmbr_fci_t *tmmbn_fci = rtcp_RTPFB_tmmbr_get_fci(block);
            rtcp_fb_tmmbr_fci_t *tmmbr_fci = rtcp_RTPFB_tmmbr_get_fci(session->rtcp.tmmbr_info.sent);
            if ((ntohl(tmmbn_fci->ssrc) == rtp_session_get_send_ssrc(session)) && (tmmbn_fci->value == tmmbr_fci->value)) {
                freemsg(session->rtcp.tmmbr_info.sent);
                session->rtcp.tmmbr_info.sent = NULL;
            }
        }
        break;
    default:
        break;
    }
}

/*
 * @brief : for SR packets, retrieves their timestamp, gets the date, and stores these information into the session descriptor. The date values may be used for setting some fields of the report block of the next RTCP packet to be sent.
 * @param session : the current session descriptor.
 * @param block : the block descriptor that may contain a SR RTCP message.
 * @return 0 if the packet is a real RTCP packet, -1 otherwise.
 * @note a basic parsing is done on the block structure. However, if it fails, no error is returned, and the session descriptor is left as is, so it does not induce any change in the caller procedure behaviour.
 * @note the packet is freed or is taken ownership if -1 is returned
 */
static int process_rtcp_packet( RtpSession *session, mblk_t *block, struct ipstack_sockaddr *addr, socklen_t addrlen ) {
    rtcp_common_header_t *rtcp;
    RtpStream * rtpstream = &session->rtp;

    int msgsize = (int) ( block->b_wptr - block->b_rptr );
    if ( msgsize < RTCP_COMMON_HEADER_SIZE ) {
        ortp_warning( "Receiving a too short RTCP packet" );
        freemsg(block);
        return -1;
    }

    rtcp = (rtcp_common_header_t *)block->b_rptr;
    ortp_rtcp_msg_debug(session, (unsigned char*)block->b_rptr, (uint32_t)(block->b_wptr - block->b_rptr), NULL);
    if (rtcp->version != 2){
        /* try to see if it is a STUN packet */
        uint16_t stunlen = *((uint16_t *)(block->b_rptr + sizeof(uint16_t)));
        stunlen = ntohs(stunlen);
        if (stunlen + 20 == block->b_wptr - block->b_rptr) {
            /* this looks like a stun packet */
            rtp_session_update_remote_sock_addr(session, block, FALSE, TRUE);

            if (session->eventqs != NULL) {
                OrtpEvent *ev = ortp_event_new(ORTP_EVENT_STUN_PACKET_RECEIVED);
                OrtpEventData *ed = ortp_event_get_data(ev);
                ed->packet = block;
                ed->source_addr.len=addrlen;
                ed->source_addr.family=addr->sa_family;
                if(addr->sa_family == IPSTACK_AF_INET)
                {
                    memcpy(&ed->source_addr.addr.ipv4,addr,addrlen);
                }
                if(addr->sa_family == IPSTACK_AF_INET6)
                {
                    memcpy(&ed->source_addr.addr.ipv6,addr,addrlen);
                }
                ed->info.socket_type = OrtpRTCPSocket;
                rtp_session_dispatch_event(session, ev);
                return -1;
            }
        }else{
            ortp_warning("RtpSession [%p] receiving rtcp packet with version number != 2, discarded", session);
        }
        freemsg(block);
        return -1;
    }

    update_recv_bytes(&session->rtcp.gs, (int)(block->b_wptr - block->b_rptr), &block->timestamp);

    /* compound rtcp packet can be composed by more than one rtcp message */
    do{
        struct timeval reception_date;
        const report_block_t *rb;

        /* Getting the reception date from the main clock */
        ortp_gettimeofday( &reception_date, NULL );

        if (rtcp_is_SR(block) ) {
            rtcp_sr_t *sr = (rtcp_sr_t *) rtcp;

            /* The session descriptor values are reset in case there is an error in the SR block parsing */
            rtpstream->last_rcv_SR_ts = 0;
            rtpstream->last_rcv_SR_time.tv_usec = 0;
            rtpstream->last_rcv_SR_time.tv_sec = 0;

            if ( ntohl( sr->ssrc ) != session->rcv.ssrc ) {
                ortp_debug( "Receiving a RTCP SR packet from an unknown ssrc" );
                return 0;
            }

            if ( msgsize < RTCP_COMMON_HEADER_SIZE + RTCP_SSRC_FIELD_SIZE + RTCP_SENDER_INFO_SIZE + ( RTCP_REPORT_BLOCK_SIZE * sr->ch.rc ) ) {
                ortp_debug( "Receiving a too short RTCP SR packet" );
                return 0;
            }

            /* Saving the data to fill LSR and DLSR field in next RTCP report to be transmitted */
            /* This value will be the LSR field of the next RTCP report (only the central 32 bits are kept, as described in par.4 of RC3550) */
            rtpstream->last_rcv_SR_ts = ( ntohl( sr->si.ntp_timestamp_msw ) << 16 ) | ( ntohl( sr->si.ntp_timestamp_lsw ) >> 16 );
            /* This value will help in processing the DLSR of the next RTCP report ( see report_block_init() in rtcp.cc ) */
            rtpstream->last_rcv_SR_time.tv_usec = reception_date.tv_usec;
            rtpstream->last_rcv_SR_time.tv_sec = reception_date.tv_sec;
            rb=rtcp_SR_get_report_block(block,0);
            if (rb) compute_rtt_from_report_block(session,&reception_date,rb);
        }else if ( rtcp_is_RR(block)){
            rb=rtcp_RR_get_report_block(block,0);
            if (rb) compute_rtt_from_report_block(session,&reception_date,rb);
        } else if (rtcp_is_XR(block)) {
            compute_rtcp_xr_statistics(session, block, &reception_date);
        } else if (rtcp_is_RTPFB(block)) {
            handle_rtcp_rtpfb_packet(session, block);
        }
    }while (rtcp_next_packet(block));
    rtcp_rewind(block);

    rtp_session_update_remote_sock_addr(session, block, FALSE, FALSE);
    return 0;
}

static void reply_to_collaborative_rtcp_xr_packet(RtpSession *session, mblk_t *block) {
    if (rtcp_is_XR(block) && (rtcp_XR_get_block_type(block) == RTCP_XR_RCVR_RTT)) {
        session->rtcp.rtcp_xr_dlrr_to_send = TRUE;
    }
}

static void rtp_process_incoming_packet(RtpSession * session, mblk_t * mp, bool_t is_rtp_packet, uint32_t user_ts, bool_t received_via_rtcp_mux) {
    bool_t sock_connected=(is_rtp_packet && !!(session->flags & RTP_SOCKET_CONNECTED))
            || (!is_rtp_packet && !!(session->flags & RTCP_SOCKET_CONNECTED));

    struct ipstack_sockaddr *remaddr = NULL;
    socklen_t addrlen;
    remaddr = (struct ipstack_sockaddr *)&mp->net_addr.addr;
    addrlen = mp->net_addr.len;


    if (session->spliced_session){
        /*this will forward all traffic to the spliced session*/
        rtp_session_do_splice(session, mp, is_rtp_packet);
    }

    /*
     * Symmetric RTP policy
     * - if a STUN packet is received AND it is the first packet received, switch destination.
     * - if a RTP or RTCP packet is received, switch destination.
     * In all other cases, we don't switch.
     * This logic is implemented in rtp_session_rtp_parse() and process_rtcp_packet().
    **/

    if (is_rtp_packet){
        if (session->use_connect && session->symmetric_rtp && !sock_connected ){
            /* In the case where use_connect is false, */
            if (try_connect(session->rtp.gs.socket,remaddr,addrlen)) {
                session->flags|=RTP_SOCKET_CONNECTED;
            }
        }
        /* then parse the message and put on jitter buffer queue */
        update_recv_bytes(&session->rtp.gs, (size_t)(mp->b_wptr - mp->b_rptr), &mp->timestamp);
        rtp_session_rtp_parse(session, mp, user_ts, remaddr,addrlen);
        /*for bandwidth measurements:*/
    }else {
        if (session->use_connect && session->symmetric_rtp && !sock_connected){
            if (try_connect(session->rtcp.gs.socket,remaddr,addrlen)) {
                session->flags|=RTCP_SOCKET_CONNECTED;
            }
        }
        if (process_rtcp_packet(session, mp, remaddr, addrlen) == 0){
            /* a copy is needed since rtp_session_notify_inc_rtcp will free the mp,
            and we don't want to send RTCP XR packet before notifying the application
            that a message has been received*/
            mblk_t * copy = copymsg(mp);
            session->stats.recv_rtcp_packets++;
            /* post an event to notify the application */
            rtp_session_notify_inc_rtcp(session, mp, received_via_rtcp_mux);
            /* reply to collaborative RTCP XR packets if needed. */
            if (session->rtcp.xr_conf.enabled == TRUE){
                reply_to_collaborative_rtcp_xr_packet(session, copy);
            }
            freemsg(copy);
        }
    }
}

void rtp_session_process_incoming(RtpSession * session, mblk_t *mp, bool_t is_rtp_packet, uint32_t ts, bool_t received_via_rtcp_mux) {
    if (session->net_sim_ctx && session->net_sim_ctx->params.mode == OrtpNetworkSimulatorInbound) {
        /*drain possible packets queued in the network simulator*/
        mp = rtp_session_network_simulate(session, mp, &is_rtp_packet);
        if (mp) rtp_process_incoming_packet(session, mp, is_rtp_packet, ts, received_via_rtcp_mux); /*BUG here: received_via_rtcp_mux is not preserved by network simulator*/
    } else if (mp != NULL) {
        rtp_process_incoming_packet(session, mp, is_rtp_packet, ts, received_via_rtcp_mux);
    }
}

static void* rtp_session_recvfrom_async(void* obj) {
    RtpSession *session = (RtpSession*) obj;
    int error;
    struct ipstack_sockaddr remaddr;
    socklen_t addrlen = sizeof (remaddr);
    mblk_t *mp;

            bool_t sock_connected=!!(session->flags & RTP_SOCKET_CONNECTED);

            mp = msgb_allocator_alloc(&session->rtp.gs.allocator, session->recv_buf_size);

            if(session->flags & RTP_SESSION_USING_OVER_RTSPTCP_SOCKETS ||
                    session->flags & RTP_SESSION_USING_OVER_TCP_SOCKETS)
            {
                error = rtp_session_rtp_tcp_recvfrom(session, mp , 1, NULL, 0);
            }
            else
            {
                if (sock_connected){
                    error = rtp_session_recvfrom(session, TRUE, mp, 0, NULL, NULL);
                }else if (rtp_session_using_transport(session, rtp)) {
                    error = (session->rtp.gs.tr->t_recvfrom)(session->rtp.gs.tr, mp, 0, (struct ipstack_sockaddr *) &remaddr, &addrlen);
                } else {
                    error = rtp_session_recvfrom(session, TRUE, mp, 0, (struct ipstack_sockaddr *) &remaddr, &addrlen);
                }
            }
            if (error > 0) {
                if (mp->timestamp.tv_sec == 0){
                    static int warn_once = 1; /*VERY BAD to use a static but there is no context in this function to hold this variable*/
                    if (warn_once){
                        ortp_warning("The transport layer doesn't implement IPSTACK_SO_TIMESTAMP, will use gettimeofday() instead.");
                        warn_once = 0;
                    }
                    ortp_gettimeofday(&mp->timestamp, NULL);
                }

                mp->b_wptr+=error;


                putq(&session->rtp.sockrq, mp);

            } else {
                int errnum;
                if (error==-1 && !is_would_block_error((errnum=getSocketErrorCode())) )
                {
                    if (session->on_network_error.count>0){
                        rtp_signal_table_emit3(&session->on_network_error,"Error receiving RTP packet",ORTP_INT_TO_POINTER(getSocketErrorCode()));

                    }else ortp_warning("Error receiving RTP packet err num [%i], error [%i]: %s",errnum,error,getSocketError());
#if TARGET_OS_IPHONE
                    /*hack for iOS and non-working socket because of background mode*/
                    if (errnum==ENOTCONN){
                        /*re-create new sockets */
                        rtp_session_set_local_addr(session,session->rtp.gs.sockfamily==IPSTACK_AF_INET ? "0.0.0.0" : "::0",session->rtp.gs.loc_port,session->rtcp.gs.loc_port);
                    }
#endif
                }
                freemsg(mp);
            }


    return NULL;
}

int rtp_session_rtp_recv (RtpSession * session, uint32_t user_ts) {
    mblk_t *mp;

    if ((!ipstack_invalid(session->rtp.gs.socket)) && !rtp_session_using_transport(session, rtp)) return -1;  /*session has no sockets for the moment*/

    while (1)
    {
        bool_t packet_is_rtp = TRUE;
        if (!session->bundle || (session->bundle && session->is_primary)) {

            rtp_session_recvfrom_async((void*)session);

        }

        if (!session->bundle || (session->bundle && session->is_primary)) {

            mp = getq(&session->rtp.sockrq);

        } else {
            ortp_mutex_lock(&session->bundleq_lock);
            mp = getq(&session->bundleq);
            ortp_mutex_unlock(&session->bundleq_lock);

            if (mp && session->rtcp_mux){
                /* The packet could be a RTCP one */
                if (rtp_get_version(mp) == 2){
                    int pt = rtp_get_payload_type(mp);
                    if (pt >= 64 && pt <= 95){
                        /*this is assumed to be an RTCP packet*/
                        packet_is_rtp = FALSE;
                    }
                }
            }
        }

        if (mp != NULL) {
            mp->reserved1 = user_ts;
            rtp_session_process_incoming(session, mp, packet_is_rtp, user_ts, !packet_is_rtp);
        } else {
            rtp_session_process_incoming(session, NULL, packet_is_rtp, user_ts, FALSE);
            return -1;
        }
    }
    return -1;
}

int rtp_session_rtcp_recv (RtpSession * session) {
    int error;
    struct ipstack_sockaddr remaddr;

    socklen_t addrlen = sizeof (remaddr);
    mblk_t *mp;

    if (!ipstack_invalid(session->rtcp.gs.socket) && !rtp_session_using_transport(session, rtcp)) return -1;  /*session has no RTCP sockets for the moment*/

    /* In bundle mode, rtcp-mux is used. There is nothing that needs to be read on the rtcp socket.
     * The RTCP packets are received on the RTP socket, and dispatched to the "bundleq" of their corresponding session.
     * These RTCP packets queued on the bundleq will be processed by rtp_session_rtp_recv(), which has the ability to
     * manage rtcp-mux.
     */
    if (session->bundle) return 0;
    while (1)
    {
        bool_t sock_connected=!!(session->flags & RTCP_SOCKET_CONNECTED);

        mp = msgb_allocator_alloc(&session->rtcp.gs.allocator, session->recv_buf_size);
        mp->reserved1 = session->rtp.rcv_last_app_ts;

        if(session->flags & RTP_SESSION_USING_OVER_RTSPTCP_SOCKETS ||
                session->flags & RTP_SESSION_USING_OVER_TCP_SOCKETS)
        {
            error = rtp_session_rtp_tcp_recvfrom(session, mp , 0, NULL, 0);
        }
        else
        {
            if (sock_connected){
                error=rtp_session_recvfrom(session, FALSE, mp, 0, NULL, NULL);
            }else{
                addrlen=sizeof (remaddr);

                if (rtp_session_using_transport(session, rtcp)){
                    error=(session->rtcp.gs.tr->t_recvfrom)(session->rtcp.gs.tr, mp, 0,
                                                            (struct ipstack_sockaddr *) &remaddr,
                                                            &addrlen);
                }else{
                    error=rtp_session_recvfrom(session, FALSE, mp, 0,
                                               (struct ipstack_sockaddr *) &remaddr,
                                               &addrlen);
                }
            }
        }
        if (error > 0)
        {
            mp->b_wptr += error;
            if (mp->timestamp.tv_sec == 0) ortp_gettimeofday(&mp->timestamp, NULL);
            rtp_session_process_incoming(session, mp, FALSE, session->rtp.rcv_last_app_ts, FALSE);
        }
        else
        {
            int errnum;
            if (error==-1 && !is_would_block_error((errnum=getSocketErrorCode())) )
            {
                if (session->on_network_error.count>0){
                    rtp_signal_table_emit3(&session->on_network_error,"Error receiving RTCP packet",ORTP_INT_TO_POINTER(getSocketErrorCode()));

                }else ortp_warning("Error receiving RTCP packet: %s, err num  [%i],error [%i]",getSocketError(),errnum,error);
#if TARGET_OS_IPHONE
                /*hack for iOS and non-working socket because of background mode*/
                if (errnum==ENOTCONN){
                    /*re-create new sockets */
                    rtp_session_set_local_addr(session,session->rtcp.gs.sockfamily==IPSTACK_AF_INET ? "0.0.0.0" : "::0",session->rtcp.gs.loc_port,session->rtcp.gs.loc_port);
                }
#endif
                session->rtp.recv_errno=errnum;
            }else{
                /*EWOULDBLOCK errors or transports returning 0 are ignored.*/
                rtp_session_process_incoming(session, NULL, FALSE, session->rtp.rcv_last_app_ts, FALSE);
            }

            freemsg(mp);
            return -1; /* avoids an infinite loop ! */
        }
    }
    return error;
}

int rtp_session_update_remote_sock_addr(RtpSession * session, mblk_t * mp, bool_t is_rtp,bool_t only_at_start) {
    struct ipstack_sockaddr * rem_addr = NULL;
    socklen_t *rem_addrlen;
    const char* socket_type;
    bool_t sock_connected;
    bool_t do_address_change = /*(rtp_get_version(mp) == 2 && */ !only_at_start;

    if (!rtp_session_get_symmetric_rtp(session))
        return -1; /*nothing to try if not rtp symetric*/

    if (session->bundle && !session->is_primary){
        /* A session part of a bundle not owning the transport layer shall not update remote address.*/
        return -1;
    }
    if (is_rtp) {
        rem_addr = (struct ipstack_sockaddr *)&session->rtp.gs.rem_addr.addr;
        rem_addrlen = &session->rtp.gs.rem_addr.len;
        socket_type = "rtp";
        sock_connected = session->flags & RTP_SOCKET_CONNECTED;
        do_address_change =  (!ipstack_invalid(session->rtp.gs.socket)) && ( do_address_change || rtp_session_get_stats(session)->packet_recv == 0);
    } else {
        rem_addr = (struct ipstack_sockaddr *)&session->rtcp.gs.rem_addr.addr;
        rem_addrlen = &session->rtcp.gs.rem_addr.len;
        sock_connected = session->flags & RTCP_SOCKET_CONNECTED;
        socket_type = "rtcp";
        do_address_change = (!ipstack_invalid(session->rtcp.gs.socket))  && (do_address_change || rtp_session_get_stats(session)->recv_rtcp_packets == 0);
    }

    if (do_address_change
            && rem_addr
            && !sock_connected
            && !ortp_is_multicast_addr((const struct ipstack_sockaddr*)rem_addr)
            && memcmp(rem_addr,&mp->net_addr.addr,mp->net_addr.len) !=0) {
        char current_ip_address[64]={0};
        char new_ip_address[64]={0};

        ortp_sockaddr_to_print_address((struct ipstack_sockaddr *)rem_addr, *rem_addrlen, current_ip_address, sizeof(current_ip_address));
        ortp_sockaddr_to_print_address((struct ipstack_sockaddr *)&mp->net_addr.addr, mp->net_addr.len, new_ip_address, sizeof(new_ip_address));
        ortp_message("Switching %s destination from %s to %s for session [%p]"
                     , socket_type
                     , current_ip_address
                     , new_ip_address
                     , session);

        memcpy(rem_addr,&mp->net_addr.addr,mp->net_addr.len);
        *rem_addrlen = mp->net_addr.len;
#ifdef WIN32
        if (is_rtp){
            /*re-apply dscp settings for the new destination (windows specific).*/
            rtp_session_set_dscp( session, -1 );
        }
#endif
        return 0;
    }
    return -1;
}

void rtp_session_use_local_addr(RtpSession * session, const char * rtp_local_addr, const char * rtcp_local_addr) {
    //struct addrinfo * session_addr_info;
    struct ipstack_sockaddr sa;
    socklen_t salen = 0;
    if(rtp_local_addr[0] != '\0')
    {// rtp_local_addr should not be NULL
        ortp_address_to_sockaddr(session->rtp.gs.sockfamily, rtp_local_addr, 0, &sa, &salen);

        //session_addr_info = bctbx_ip_address_to_addrinfo(session->rtp.gs.sockfamily , IPSTACK_SOCK_DGRAM ,rtp_local_addr, 0);
        memcpy(&session->rtp.gs.used_loc_addr.addr, &sa, salen);
        session->rtp.gs.used_loc_addr.len = (socklen_t)salen;
        session->rtp.gs.used_loc_addr.family = sa.sa_family;
        //bctbx_freeaddrinfo(session_addr_info);
    }else {
        session->rtp.gs.used_loc_addr.len = 0;
        memset(&session->rtp.gs.used_loc_addr, 0, sizeof(session->rtp.gs.used_loc_addr));// To not let tracks on memory
    }
    if(rtcp_local_addr[0] != '\0')
    {// rtcp_local_addr should not be NULL
        ortp_address_to_sockaddr(session->rtp.gs.sockfamily, rtcp_local_addr, 0, &sa, &salen);

        //session_addr_info = bctbx_ip_address_to_addrinfo(session->rtcp.gs.sockfamily , IPSTACK_SOCK_DGRAM ,rtcp_local_addr, 0);
        memcpy(&session->rtcp.gs.used_loc_addr.addr, &sa, salen);
        session->rtcp.gs.used_loc_addr.len = (socklen_t)salen;
        session->rtcp.gs.used_loc_addr.family = sa.sa_family;
        //bctbx_freeaddrinfo(session_addr_info);
    }else {
        session->rtcp.gs.used_loc_addr.len = 0;
        memset(&session->rtcp.gs.used_loc_addr, 0, sizeof(session->rtcp.gs.used_loc_addr));// To not let tracks on memory
    }
    ortp_message("RtpSession set sources to [%s] and [%s]",rtp_local_addr, rtcp_local_addr );
}
