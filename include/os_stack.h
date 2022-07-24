/*
 * os_stack.h
 *
 *  Created on: Jun 19, 2018
 *      Author: zhurish
 */

#ifndef __OS_STACK_H__
#define __OS_STACK_H__

#ifdef __cplusplus
extern "C" {
#endif

#define IPSTACK_AF_UNSPEC     AF_UNSPEC
#define IPSTACK_AF_UNIX        AF_UNIX
#define IPSTACK_AF_INET       AF_INET
#define IPSTACK_AF_NETLINK    AF_NETLINK
#define IPSTACK_AF_ROUTE      AF_ROUTE
#ifdef AF_LINK
#define IPSTACK_AF_LINK       AF_LINK
#endif
#define IPSTACK_AF_PACKET     AF_PACKET
#define IPSTACK_AF_INET6      AF_INET6
#define IPSTACK_AF_KEY        AF_KEY
#define IPSTACK_AF_ETHERNET        AF_ETHERNET
#define IPSTACK_AF_MAX        AF_MAX

#define IPSTACK_PF_UNIX        PF_UNIX
#define IPSTACK_PF_INET       PF_INET
#define IPSTACK_PF_NETLINK    PF_NETLINK
#define IPSTACK_PF_ROUTE      PF_ROUTE
#define IPSTACK_PF_LINK       PF_LINK
#define IPSTACK_PF_PACKET     PF_PACKET
#define IPSTACK_PF_INET6      PF_INET6
#define IPSTACK_PF_KEY        PF_KEY


/*
 *===========================================================================
 *                         Socket types
 *===========================================================================
 */

#define IPSTACK_SOCK_STREAM      SOCK_STREAM
#define IPSTACK_SOCK_DGRAM       SOCK_DGRAM
#define IPSTACK_SOCK_RAW         SOCK_RAW
#define IPSTACK_SOCK_PACKET      SOCK_PACKET
/*
 *===========================================================================
 *                         IP protocol values
 *===========================================================================
 */

#define IPSTACK_IPPROTO_IP          IPPROTO_IP
#define IPSTACK_IPPROTO_ICMP        IPPROTO_ICMP
#define IPSTACK_IPPROTO_IGMP        IPPROTO_IGMP
#define IPSTACK_IPPROTO_IPIP        IPPROTO_IPIP
#define IPSTACK_IPPROTO_IPV4        IPPROTO_IPV4
#define IPSTACK_IPPROTO_TCP         IPPROTO_TCP
#define IPSTACK_IPPROTO_UDP         IPPROTO_UDP
#define IPSTACK_IPPROTO_IPV6        IPPROTO_IPV6
#define IPSTACK_IPPROTO_RSVP        IPPROTO_RSVP
#define IPSTACK_IPPROTO_GRE         IPPROTO_GRE
#define IPSTACK_IPPROTO_ESP         IPPROTO_ESP
#define IPSTACK_IPPROTO_AH          IPPROTO_AH
#define IPSTACK_IPPROTO_ICMPV6      IPPROTO_ICMPV6
#define IPSTACK_IPPROTO_OSPFIGP     IPPROTO_OSPFIGP
#define IPSTACK_IPPROTO_PIM         IPPROTO_PIM
#define IPSTACK_IPPROTO_RAW         IPPROTO_RAW
#define IPSTACK_IPPROTO_MAX         IPPROTO_MAX


/* IPv6 Non-IP Protocol Next Header field values */

#define IPSTACK_IPPROTO_HOPOPTS     IPPROTO_HOPOPTS
#define IPSTACK_IPPROTO_ROUTING     IPPROTO_ROUTING
#define IPSTACK_IPPROTO_FRAGMENT    IPPROTO_FRAGMENT
#define IPSTACK_IPPROTO_NONE        IPPROTO_NONE
#define IPSTACK_IPPROTO_DSTOPTS     IPPROTO_DSTOPTS



/*
 *===========================================================================
 *                  Standard IP address macros.
 *===========================================================================
 */

#define IPSTACK_INADDR_ANY             INADDR_ANY
#define IPSTACK_INADDR_DEFAULT         INADDR_DEFAULT
#define IPSTACK_INADDR_LOOPBACK        INADDR_LOOPBACK
#define IPSTACK_INADDR_BROADCAST       INADDR_BROADCAST
#define	IPSTACK_INADDR_NONE            INADDR_NONE

#define IPSTACK_INADDR_UNSPEC_GROUP    INADDR_UNSPEC_GROUP
#define IPSTACK_INADDR_ALLHOSTS_GROUP  INADDR_ALLHOSTS_GROUP

#define IPSTACK_IPV4_NET127    IPV4_NET127
#define IPSTACK_IPV4_LINKLOCAL  IPV4_LINKLOCAL


/*
 *===========================================================================
 *                         NETMASK
 *===========================================================================
 */

#define IPSTACK_NETMASK(xxip)          NETMASK(xxip)
#define IPSTACK_NETBITS(xxip)          NETBITS(xxip)

#define	IPSTACK_IN_CLASSA(xxip)	       IN_CLASSA(xxip)
#define	IPSTACK_IN_CLASSB(xxip)        IN_CLASSB(xxip)
#define	IPSTACK_IN_CLASSC(xxip)        IN_CLASSC(xxip)
#define	IPSTACK_IN_CLASSD(xxip)        IN_CLASSD(xxip)
#define	IPSTACK_IN_EXPERIMENTAL(xxip)  IN_EXPERIMENTAL(xxip)
#define IPSTACK_INET_ADDRSTRLEN        INET_ADDRSTRLEN
#define IPSTACK_IN_MULTICAST(xxip)     IN_CLASSD(xxip)

/*
 *===========================================================================
 *                      IPv6 address macros
 *===========================================================================
 */


#define IPSTACK_IN6_ARE_ADDR_EQUAL(addr1,addr2)      IN6_ARE_ADDR_EQUAL(addr1, addr2)
#define IPSTACK_IN6_IS_ADDR_UNSPECIFIED(addr)        IN6_IS_ADDR_UNSPECIFIED(addr)
#define IPSTACK_IN6_IS_ADDR_LOOPBACK(addr)           IN6_IS_ADDR_LOOPBACK(addr)
#define IPSTACK_IN6_IS_ADDR_V4COMPAT(addr)           IN6_IS_ADDR_V4COMPAT(addr)
#define IPSTACK_IN6_IS_ADDR_V4MAPPED(addr)           IN6_IS_ADDR_V4MAPPED(addr)
#define IPSTACK_IN6_IS_ADDR_AGGR_GLOB_UNICAST(addr)  IN6_IS_ADDR_AGGR_GLOB_UNICAST(addr)
#define IPSTACK_IN6_IS_ADDR_SITE_LOCAL(addr)         IN6_IS_ADDR_SITELOCAL(addr)
#define IPSTACK_IN6_IS_ADDR_LINK_LOCAL(addr)         IN6_IS_ADDR_LINKLOCAL(addr)
#define IPSTACK_IN6_IS_ADDR_MULTICAST(addr)          IN6_IS_ADDR_MULTICAST(addr)
#define IPSTACK_IN6_IS_ADDR_LINKLOCAL(addr)          IN6_IS_ADDR_LINKLOCAL(addr)


/*
 *===========================================================================
 *                         setsockopt/getsockopt
 *===========================================================================
 */

#define IPSTACK_SOL_SOCKET         SOL_SOCKET

/***** SOL_SOCKET socket options: *****/
#define IPSTACK_SO_REUSEADDR       SO_REUSEADDR
#define IPSTACK_SO_KEEPALIVE       SO_KEEPALIVE
#define IPSTACK_SO_DONTROUTE       SO_DONTROUTE
#define IPSTACK_SO_BROADCAST       SO_BROADCAST
#define IPSTACK_SO_REUSEPORT       SO_REUSEPORT
#define IPSTACK_SO_SNDBUF          SO_SNDBUF
#define IPSTACK_SO_RCVBUF          SO_RCVBUF
#define IPSTACK_SO_RCVTIMEO        SO_RCVTIMEO
#define IPSTACK_SO_ERROR           SO_ERROR
#define IPSTACK_SO_TYPE            SO_TYPE
#define IPSTACK_SO_BINDTODEVICE    SO_BINDTODEVICE
#ifdef SO_MARK
#define IPSTACK_SO_MARK             SO_MARK
#endif
#ifdef IP_SO_X_VR
#define IPSTACK_SO_X_VR            IP_SO_X_VR
#endif
/***** IPPROTO_IP level socket options: *****/
#define IPSTACK_HDRINCL         HDRINCL

#define IPSTACK_IP_TOS             IP_TOS
#define IPSTACK_IP_TTL             IP_TTL
#define IPSTACK_IPTOS_LOWDELAY  IPTOS_LOWDELAY
//#define IPSTACK_PKTINFO         PKTINFO
#define IPSTACK_IP_PKTINFO      IP_PKTINFO

#define IPSTACK_IP_MULTICAST_IF    IP_MULTICAST_IF
#define IPSTACK_IP_MULTICAST_TTL   IP_MULTICAST_TTL
#define IPSTACK_IP_MULTICAST_LOOP  IP_MULTICAST_LOOP


#define	IPSTACK_IP_ADD_MEMBERSHIP IP_ADD_MEMBERSHIP
#define	IPSTACK_IP_DROP_MEMBERSHIP  IP_DROP_MEMBERSHIP
#define	IPSTACK_MCAST_JOIN_GROUP	MCAST_JOIN_GROUP
#define	IPSTACK_MCAST_LEAVE_GROUP	MCAST_LEAVE_GROUP

#define IPSTACK_IP_RECVIF          IP_RECVIF
#define IPSTACK_ROUTER_ALERT    ROUTER_ALERT
#define IPSTACK_DONTFRAG        DONTFRAG
#define IPSTACK_TCP_INFO        TCP_INFO
#define IPSTACK_TCP_MD5SIG  TCP_MD5SIG
#define IPSTACK_TCP_CORK    TCP_CORK
#define IPSTACK_IP_MINTTL   IP_MINTTL
#define IPSTACK_IPV6_MINHOPCNT  IPV6_MINHOPCNT
#define IPSTACK_IPV6_V6ONLY IPV6_V6ONLY



/***** IPPROTO_IPV6 level socket options: *****/
#define IPSTACK_IPV6_UNICAST_HOPS     IPV6_UNICAST_HOPS
#define IPSTACK_IPV6_MULTICAST_IF     IPV6_MULTICAST_IF
#define IPSTACK_IPV6_MULTICAST_HOPS   IPV6_MULTICAST_HOPS
#define IPSTACK_IPV6_MULTICAST_LOOP   IPV6_MULTICAST_LOOP
#define	IPSTACK_IPV6_JOIN_GROUP       IPV6_JOIN_GROUP
#define IPSTACK_IPV6_ADD_MEMBERSHIP   IPV6_ADD_MEMBERSHIP
#define	IPSTACK_IPV6_LEAVE_GROUP      IPV6_LEAVE_GROUP
#define IPSTACK_IPV6_DROP_MEMBERSHIP  IPV6_DROP_MEMBERSHIP
#define IPSTACK_IPV6_PKTINFO          IPV6_PKTINFO
#define IPSTACK_IPV6_TCLASS           IPV6_TCLASS
#define IPSTACK_IPV6_NEXTHOP          IPV6_NEXTHOP
#define IPSTACK_IPV6_RTHDR            IPV6_RTHDR
#define IPSTACK_IPV6_HOPOPTS          IPV6_HOPOPTS
#define IPSTACK_IPV6_DSTOPTS          IPV6_DSTOPTS
#define IPSTACK_IPV6_RTHDRDSTOPTS     IPV6_RTHDRDSTOPTS
#define IPSTACK_IPV6_RECVPKTINFO      IPV6_RECVPKTINFO
#define IPSTACK_IPV6_RECVHOPLIMIT     IPV6_RECVHOPLIMIT
#define IPSTACK_IPV6_RECVTCLASS       IPV6_RECVTCLASS
#define IPSTACK_IPV6_RECVRTHDR        IPV6_RECVRTHDR
#define IPSTACK_IPV6_RECVHOPOPTS      IPV6_RECVHOPOPTS
#define IPSTACK_IPV6_RECVDSTOPTS      IPV6_RECVDSTOPTS
#define IPSTACK_IPV6_CHECKSUM         IPV6_CHECKSUM
#define IPSTACK_IPV6_RECVIF           IPV6_RECVIF

/***** IPPROTO_ICMPV6 level socket options: *****/
#define IPSTACK_ICMP6_FILTER          ICMP6_FILTER

/***** IPPROTO_TCP level socket options: *****/
#define IPSTACK_TCP_NODELAY           TCP_NODELAY

/* NOTE: This option can NOT be used with getsockopt/setsockopt!
 * Only sent/rcvd with ancillary data!
 */
#define IPSTACK_IPV6_HOPLIMIT         IPV6_HOPLIMIT

#define	IPSTACK_ARPHRD_ETHER        ARPHRD_ETHER
#define	IPSTACK_ARPHRD_ATM          ARPHRD_ATM
#define	IPSTACK_ARPHRD_SLIP         ARPHRD_SLIP
#define	IPSTACK_ARPHRD_CSLIP        ARPHRD_CSLIP
#define	IPSTACK_ARPHRD_SLIP6        ARPHRD_SLIP6
#define	IPSTACK_ARPHRD_CSLIP6       ARPHRD_CSLIP6
#define	IPSTACK_ARPHRD_PPP          ARPHRD_PPP
#define	IPSTACK_ARPHRD_CISCO        ARPHRD_CISCO
#define	IPSTACK_ARPHRD_RAWHDLC      ARPHRD_RAWHDLC
#define	IPSTACK_ARPHRD_TUNNEL       ARPHRD_TUNNEL
#define	IPSTACK_ARPHRD_TUNNEL6      ARPHRD_TUNNEL6
#define	IPSTACK_ARPHRD_LOOPBACK     ARPHRD_LOOPBACK
#define	IPSTACK_ARPHRD_SIT          ARPHRD_SIT
#define	IPSTACK_ARPHRD_IPGRE        ARPHRD_IPGRE
#define	IPSTACK_ARPHRD_IEEE80211    ARPHRD_IEEE80211
#ifdef IPSTACK_ARPHRD_IP6GRE
#define	IPSTACK_ARPHRD_IP6GRE       ARPHRD_IP6GRE
#endif

#define IPSTACK_MSG_OOB			MSG_OOB                    /* Specify to send/receive out-of-band data */
#define IPSTACK_MSG_PEEK		MSG_PEEK                   /* Leave the data on the receive buffer (receive only) */
#define IPSTACK_MSG_DONTROUTE	MSG_DONTROUTE              /* Bypass normal routing, the packet will be sent
                                            on the interface that matches the network part
                                            of the destination address */
#define IPSTACK_MSG_WAITALL		MSG_WAITALL               /* Wait for full request or error (receive only) */
#define IPSTACK_MSG_DONTWAIT	MSG_DONTWAIT              /* Enables non-blocking operation */

#define IPSTACK_MSG_NOTIFICATION	MSG_NOTIFICATION    /* SCTP Notification */
#define IPSTACK_MSG_EOR				MSG_EOR

#define IPSTACK_MSG_MORE		MSG_MORE                /* Disable Nagle algorithm (send only) */
#ifdef IPCOM_ZEROCOPY
#define IPSTACK_MSG_ZBUF		MSG_ZBUF               /* zbuf */
#endif
#define IPSTACK_TCP_NODELAY		TCP_NODELAY

//#endif
/*
 *===========================================================================
 *                         ioctl
 *===========================================================================
 */
/* socket ioctl */
#define IPSTACK_SIOCSPGRP          SIOCSPGRP
#define IPSTACK_SIOCGPGRP          SIOCGPGRP
#define IPSTACK_FIONBIO            FIONBIO

/* AF_INET ioctl. */
#define IPSTACK_SIOCGIFADDR        SIOCGIFADDR
#define IPSTACK_SIOCSIFADDR        SIOCSIFADDR
#define IPSTACK_SIOCGIFBRDADDR     SIOCGIFBRDADDR
#define IPSTACK_SIOCSIFBRDADDR     SIOCSIFBRDADDR
#define IPSTACK_SIOCGIFNETMASK     SIOCGIFNETMASK
#define IPSTACK_SIOCSIFNETMASK     SIOCSIFNETMASK
#define IPSTACK_SIOCGIFDSTADDR     SIOCGIFDSTADDR
#define IPSTACK_SIOCSIFDSTADDR     SIOCSIFDSTADDR
#define IPSTACK_SIOCAIFADDR        SIOCAIFADDR
#define IPSTACK_SIOCDIFADDR        SIOCDIFADDR
#define IPSTACK_SIOCADDRT          SIOCADDRT
#define IPSTACK_SIOCDDDRT          SIOCDDDRT

/* AF_INET6 ioctl. */
#define IPSTACK_SIOCAIFADDR_IN6    SIOCAIFADDR_IN6
#define IPSTACK_SIOCDIFADDR_IN6    SIOCDIFADDR_IN6
#define IPSTACK_SIOCGIFDSTADDR_IN6 SIOCGIFDSTADDR_IN6

/* interface ioctl. */
#define IPSTACK_SIOCGIFFLAGS       SIOCGIFFLAGS
#define IPSTACK_SIOCSIFFLAGS       SIOCSIFFLAGS
#define IPSTACK_SIOCGIFMTU         SIOCGIFMTU
#define IPSTACK_SIOCSIFMTU         SIOCSIFMTU
#define IPSTACK_SIOCGIFMETRIC      SIOCGIFMETRIC
#define IPSTACK_SIOCSIFMETRIC      SIOCSIFMETRIC
#define IPSTACK_SIOCSIFRTAB        SIOCSIFRTAB
#define IPSTACK_SIOCGIFRTAB        SIOCGIFRTAB
#define IPSTACK_SIOCGETTUNNEL      SIOCGETTUNNEL
#define IPSTACK_SIOCADDTUNNEL      SIOCADDTUNNEL
#define IPSTACK_SIOCCHGTUNNEL      SIOCCHGTUNNEL
#define IPSTACK_SIOCDELTUNNEL      SIOCDELTUNNEL
#define IPSTACK_SIOCDELTUNNEL      SIOCDELTUNNEL
#define IPSTACK_TUNSETIFF      TUNSETIFF
#define IPSTACK_SIOCSIFVLAN     SIOCSIFVLAN
#define IPSTACK_SIOCBRDELIF SIOCBRDELIF
#define IPSTACK_SIOCDEVPRIVATE  SIOCDEVPRIVATE
#define IPSTACK_SIOCBRADDIF SIOCBRADDIF
#define IPSTACK_SIOCBRDELBR SIOCBRDELBR
#define IPSTACK_SIOCBRADDBR SIOCBRADDBR
#define IPSTACK_SIOCSIFBR   SIOCSIFBR

/* arp ioctl. */
#define IPSTACK_SIOCSARP           SIOCSARP
#define IPSTACK_SIOCGARP           SIOCGARP
#define IPSTACK_SIOCDARP           SIOCDARP
#define IPSTACK_ATF_COM         ATF_COM     //已完成的邻居(成员ha有效，且含有正确的MAC地址)
#define IPSTACK_ATF_PERM        ATF_PERM    //永久性的邻居(邻居状态有NUD_PERMANENT)
#define IPSTACK_ATF_PUBL        ATF_PUBL    //发布该邻居
#define IPSTACK_ATF_USETRAILERS ATF_USETRAILERS //不是非常清楚
#define IPSTACK_ATF_NETMASK     ATF_NETMASK //仅用于代理ARP
#define IPSTACK_ATF_DONTPUB     ATF_DONTPUB //不处理该邻居

#define IPSTACK_SIOCGIFHWADDR          SIOCGIFHWADDR
#define IPSTACK_SIOCGIFCONF            SIOCGIFCONF
#define	IPSTACK_SIOCGIFTXQLEN          SIOCGIFTXQLEN
#define	IPSTACK_FIONREAD          FIONREAD

#define	IPSTACK_ETH_P_ARP   ETH_P_ARP
#define	IPSTACK_ETH_P_ALL   ETH_P_ALL
#define	IPSTACK_ETH_P_IPV6   ETH_P_IPV6
#define	IPSTACK_ETH_P_IP   ETH_P_IP
#define	IPSTACK_ETHERTYPE_IP   ETHERTYPE_IP
#define	IPSTACK_ETHERTYPE_ARP   ETHERTYPE_ARP
#define	IPSTACK_ETH_ALEN        ETH_ALEN
#define	IPSTACK_ARPOP_REQUEST   ARPOP_REQUEST
/*
 *===========================================================================
 *                         IFF_X
 *===========================================================================
 */
#define IPSTACK_IFF_UP          IFF_UP
#define IPSTACK_IFF_BROADCAST   IFF_BROADCAST
#define IPSTACK_IFF_DEBUG       IFF_DEBUG
#define IPSTACK_IFF_LOOPBACK    IFF_LOOPBACK
#define IPSTACK_IFF_POINTOPOINT IFF_POINTOPOINT
#define IPSTACK_IFF_RUNNING     IFF_RUNNING
#define IPSTACK_IFF_NOARP       IFF_NOARP
#define IPSTACK_IFF_PROMISC     IFF_PROMISC
#define IPSTACK_IFF_ALLMULTI    IFF_ALLMULTI
#ifdef IFF_OACTIVE
#define IPSTACK_IFF_OACTIVE     IFF_OACTIVE
#endif
#ifdef IFF_SIMPLEX
#define IPSTACK_IFF_SIMPLEX     IFF_SIMPLEX
#endif
#ifdef IFF_LINK0
#define IPSTACK_IFF_LINK0       IFF_LINK0
#endif
#ifdef IFF_LINK1
#define IPSTACK_IFF_LINK1       IFF_LINK1
#endif
#ifdef IFF_LINK2
#define IPSTACK_IFF_LINK2       IFF_LINK2
#endif
#define IPSTACK_IFF_MULTICAST   IFF_MULTICAST


/*
 *===========================================================================
 *                    recvxx() argument flags
 *===========================================================================
 */
#define IPSTACK_MSG_PEEK           MSG_PEEK


/*
 *===========================================================================
 *                    shutdown() argument flags
 *===========================================================================
 */
#define IPSTACK_SHUT_RD            SHUT_RD
#define IPSTACK_SHUT_WR            SHUT_WR
#define IPSTACK_SHUT_RDWR          SHUT_RDWR



#define IPSTACK_SOCKADDR_DL_LLADDR(sa)   SOCKADDR_DL_LLADDR(sa)

/* Packet types 'sll_pkttype'.    Packet is addressed to: */
#define IPSTACK_PACKET_HOST          PACKET_HOST
#define IPSTACK_PACKET_BROADCAST     PACKET_BROADCAST
#define IPSTACK_PACKET_MULTICAST     PACKET_MULTICAST
#define IPSTACK_PACKET_OTHERHOST     PACKET_OTHERHOST
#define IPSTACK_PACKET_OUTGOING      PACKET_OUTGOING



/******************************************************************************/
/******************************************************************************/
/*_IPCOM_DEFINE_H_*/

#define IPSTACK_NETLINK_ROUTE   		NETLINK_ROUTE
#define IPSTACK_RT_TABLE_MAIN   		RT_TABLE_MAIN


#define IPSTACK_RTM_VERSION            RTM_VERSION
#define IPSTACK_RTM_NEW_RTMSG          RTM_NEWROUTE
#define IPSTACK_RTM_ADD                RTM_ADD
#define IPSTACK_RTM_DELETE             RTM_DELETE
#define IPSTACK_RTM_MISS               RTM_MISS
#define IPSTACK_RTM_LOCK               RTM_LOCK
#define IPSTACK_RTM_OLDADD             RTM_OLDADD
#define IPSTACK_RTM_OLDDEL             RTM_OLDDEL
#define IPSTACK_RTM_RESOLVE            RTM_RESOLVE
#define IPSTACK_RTM_CHANGE             RTM_CHANGE
#define IPSTACK_RTM_REDIRECT           RTM_REDIRECT
#define IPSTACK_RTM_IFINFO             RTM_IFINFO


#define IPSTACK_RTM_VPN_ADD            0xfff1
#define IPSTACK_RTM_VPN_DEL            0xfff2
#define IPSTACK_RTM_VPN_RT_CHANGE      0xfff3
#define IPSTACK_RTM_TCPSYNC_ADD        0xfff4
#define IPSTACK_RTM_TCPSYNC_DEL        0xfff5
#define IPSTACK_RTM_SUBSYSTEM_UP       0xfff6
#define IPSTACK_RTM_SLOT_UP            0xfff7
#define IPSTACK_RTM_CARD_UP            0xfff8
#define IPSTACK_RTM_CARD_DOWN          0xfff9
#define IPSTACK_RTM_CARD_ROLECHANGE    0xfffa
#define IPSTACK_RTM_BFD_SESSION        0xfffb
#define IPSTACK_RTM_BFD_IFSTATUS       0xfffc
#define IPSTACK_RTM_LOSING             0xfffd



#define IPSTACK_RTMGRP_LINK            RTMGRP_LINK
#define IPSTACK_RTMGRP_IPV4_ROUTE      RTMGRP_IPV4_ROUTE
#define IPSTACK_RTMGRP_IPV4_IFADDR     RTMGRP_IPV4_IFADDR
#define IPSTACK_RTMGRP_IPV6_ROUTE      RTMGRP_IPV6_ROUTE
#define IPSTACK_RTMGRP_IPV6_IFADDR     RTMGRP_IPV6_IFADDR

#define IPSTACK_RTM_NEWLINK    		RTM_NEWLINK
#define IPSTACK_RTM_DELLINK    		RTM_DELLINK
#define IPSTACK_RTM_GETLINK    		RTM_GETLINK
#define IPSTACK_RTM_SETLINK    		RTM_SETLINK
#define IPSTACK_RTM_NEWADDR    		RTM_NEWADDR
#define IPSTACK_RTM_DELADDR    		RTM_DELADDR
#define IPSTACK_RTM_GETADDR    		RTM_GETADDR
#define IPSTACK_RTM_NEWROUTE    	RTM_NEWROUTE
#define IPSTACK_RTM_DELROUTE    	RTM_DELROUTE
#define IPSTACK_RTM_GETROUTE    	RTM_GETROUTE
#define IPSTACK_RTM_NEWNEIGH    	RTM_NEWNEIGH
#define IPSTACK_RTM_DELNEIGH    	RTM_DELNEIGH
#define IPSTACK_RTM_GETNEIGH    	RTM_GETNEIGH
#define IPSTACK_RTM_NEWRULE    		RTM_NEWRULE
#define IPSTACK_RTM_DELRULE    		RTM_DELRULE
#define IPSTACK_RTM_GETRULE    		RTM_GETRULE
#define IPSTACK_RTM_NEWQDISC    	RTM_NEWQDISC
#define IPSTACK_RTM_DELQDISC    	RTM_DELQDISC
#define IPSTACK_RTM_GETQDISC    	RTM_GETQDISC
#define IPSTACK_RTM_NEWTCLASS    	RTM_NEWTCLASS
#define IPSTACK_RTM_DELTCLASS    	RTM_DELTCLASS
#define IPSTACK_RTM_GETTCLASS    	RTM_GETTCLASS
#define IPSTACK_RTM_NEWTFILTER    	RTM_NEWTFILTER
#define IPSTACK_RTM_DELTFILTER    	RTM_DELTFILTER
#define IPSTACK_RTM_GETTFILTER    	RTM_GETTFILTER
#define IPSTACK_RTM_NEWACTION    	RTM_NEWACTION
#define IPSTACK_RTM_DELACTION    	RTM_DELACTION
#define IPSTACK_RTM_GETACTION    	RTM_GETACTION
#define IPSTACK_RTM_NEWPREFIX    	RTM_NEWPREFIX
#define IPSTACK_RTM_GETPREFIX    	RTM_GETPREFIX
#define IPSTACK_RTM_GETMULTICAST    RTM_GETMULTICAST
#define IPSTACK_RTM_GETANYCAST    	RTM_GETANYCAST
#define IPSTACK_RTM_NEWNEIGHTBL    	RTM_NEWNEIGHTBL
#define IPSTACK_RTM_GETNEIGHTBL    	RTM_GETNEIGHTBL
#define IPSTACK_RTM_SETNEIGHTBL    	RTM_SETNEIGHTBL
#define IPSTACK_RTM_NEWVR    		RTM_NEWVR
#define IPSTACK_RTM_DELVR    		RTM_DELVR
#define IPSTACK_RTM_GETVR    		RTM_GETVR
#define IPSTACK_RTM_CHANGEVR    	RTM_CHANGEVR
#define IPSTACK_RTM_NEWMIP    		RTM_NEWMIP
#define IPSTACK_RTM_DELMIP    		RTM_DELMIP
#define IPSTACK_RTM_GETMIP    		RTM_GETMIP
#define IPSTACK_RTM_SETMIP    		RTM_SETMIP
#define IPSTACK_RTM_NEWSEND    		RTM_NEWSEND
#define IPSTACK_RTM_GETSEND    		RTM_GETSEND
#define IPSTACK_RTM_SEND_SIGN_REQ   RTM_SEND_SIGN_REQ

#define IPSTACK_RTM_RTA             RTM_RTA


#define IPSTACK_RTM_F_CLONED        RTM_F_CLONED

#define IPSTACK_RTN_UNSPEC    		RTN_UNSPEC
#define IPSTACK_RTN_UNICAST    		RTN_UNICAST
#define IPSTACK_RTN_LOCAL    		RTN_LOCAL
#define IPSTACK_RTN_BROADCAST    	RTN_BROADCAST
#define IPSTACK_RTN_ANYCAST    		RTN_ANYCAST
#define IPSTACK_RTN_MULTICAST    	RTN_MULTICAST
#define IPSTACK_RTN_BLACKHOLE    	RTN_BLACKHOLE
#define IPSTACK_RTN_UNREACHABLE    	RTN_UNREACHABLE
#define IPSTACK_RTN_PROHIBIT    	RTN_PROHIBIT
#define IPSTACK_RTN_THROW    		RTN_THROW
#define IPSTACK_RTN_NAT    			RTN_NAT
#define IPSTACK_RTN_XRESOLVE    	RTN_XRESOLVE
#define IPSTACK_RTN_PROXY    		RTN_PROXY
#define IPSTACK_RTN_CLONE    		RTN_CLONE


#define IPSTACK_RTNH_DATA              RTNH_DATA
#define IPSTACK_RTNH_NEXT              RTNH_NEXT
#define IPSTACK_RTNH_F_ONLINK          RTNH_F_ONLINK


#define IPSTACK_RTPROT_UNSPEC    	RTPROT_UNSPEC
#define IPSTACK_RTPROT_REDIRECT    	RTPROT_REDIRECT
#define IPSTACK_RTPROT_KERNEL    	RTPROT_KERNEL
#define IPSTACK_RTPROT_BOOT    		RTPROT_BOOT
#define IPSTACK_RTPROT_STATIC    	RTPROT_STATIC
#define IPSTACK_RTPROT_GATED    	RTPROT_GATED
#define IPSTACK_RTPROT_RA    		RTPROT_RA
#define IPSTACK_RTPROT_MRT    		RTPROT_MRT
#define IPSTACK_RTPROT_ZEBRA    	RTPROT_ZEBRA
#define IPSTACK_RTPROT_BIRD    		RTPROT_BIRD
#define IPSTACK_RTPROT_DNROUTED    	RTPROT_DNROUTED
#define IPSTACK_RTPROT_XORP    		RTPROT_XORP
#define IPSTACK_RTPROT_NTK    		RTPROT_NTK


#define IPSTACK_RTA_ALIGN           RTA_ALIGN
#define IPSTACK_RTA_LENGTH          RTA_LENGTH
#define IPSTACK_RTA_OK              RTA_OK
#define IPSTACK_RTA_NEXT            RTA_NEXT
#define IPSTACK_RTA_PAYLOAD         RTA_PAYLOAD
#define IPSTACK_RTA_DATA            RTA_DATA
#define IPSTACK_RTA_UNSPEC   		RTA_UNSPEC
#define IPSTACK_RTA_DST   			RTA_DST
#define IPSTACK_RTA_SRC   			RTA_SRC
#define IPSTACK_RTA_IIF   			RTA_IIF
#define IPSTACK_RTA_OIF   			RTA_OIF
#define IPSTACK_RTA_GATEWAY   		RTA_GATEWAY
#define IPSTACK_RTA_PRIORITY   		RTA_PRIORITY
#define IPSTACK_RTA_PREFSRC   		RTA_PREFSRC
#define IPSTACK_RTA_METRICS   		RTA_METRICS
#define IPSTACK_RTA_MULTIPATH   	RTA_MULTIPATH
#define IPSTACK_RTA_PROTOINFO   	RTA_PROTOINFO
#define IPSTACK_RTA_FLOW   			RTA_FLOW
#define IPSTACK_RTA_CACHEINFO   	RTA_CACHEINFO
#define IPSTACK_RTA_SESSION   		RTA_SESSION
#define IPSTACK_RTA_MP_ALGO   		RTA_MP_ALGO
#define IPSTACK_RTA_TABLE   		RTA_TABLE
#define IPSTACK_RTA_NH_PROTO  		RTA_NH_PROTO
#define IPSTACK_RTA_NH_PROTO_DATA   RTA_NH_PROTO_DATA
#define IPSTACK_RTA_PROXY_ARP_LLADDR   RTA_PROXY_ARP_LLADDR
#define IPSTACK_RTA_VR   			RTA_VR
#define IPSTACK_RTA_TABLE_NAME   	RTA_TABLE_NAME
#define IPSTACK_RTA_MAX    			RTA_MAX

#define IPSTACK_RTAX_IFA                        RTAX_IFA
#define IPSTACK_RTAX_DST                       	RTAX_DST
#define IPSTACK_RTAX_GATEWAY              		RTAX_GATEWAY
#define IPSTACK_RTAX_NETMASK               		RTAX_NETMASK
#define IPSTACK_RTAX_MAX                       	RTAX_MAX
#define IPSTACK_RTAX_MTU 						RTAX_MTU


#define IPSTACK_IFLA_UNSPEC    		IFLA_UNSPEC
#define IPSTACK_IFLA_ADDRESS    	IFLA_ADDRESS
#define IPSTACK_IFLA_BROADCAST    	IFLA_BROADCAST
#define IPSTACK_IFLA_IFNAME        	IFLA_IFNAME
#define IPSTACK_IFLA_MTU       		IFLA_MTU
#define IPSTACK_IFLA_LINK    		IFLA_LINK
#define IPSTACK_IFLA_QDISC    		IFLA_QDISC
#define IPSTACK_IFLA_STATS    		IFLA_STATS
#define IPSTACK_IFLA_COST    		IFLA_COST
#define IPSTACK_IFLA_PRIORITY    	IFLA_PRIORITY
#define IPSTACK_IFLA_MASTER    		IFLA_MASTER
#define IPSTACK_IFLA_WIRELESS    	IFLA_WIRELESS
#define IPSTACK_IFLA_PROTINFO    	IFLA_PROTINFO
#define IPSTACK_IFLA_TXQLEN    		IFLA_TXQLEN
#define IPSTACK_IFLA_MAP    		IFLA_MAP
#define IPSTACK_IFLA_WEIGHT    		IFLA_WEIGHT
#define IPSTACK_IFLA_OPERSTATE    	IFLA_OPERSTATE
#define IPSTACK_IFLA_LINKMODE    	IFLA_LINKMODE
#define IPSTACK_IFLA_LINKINFO    	IFLA_LINKINFO
#define IPSTACK_IFLA_MAX         	IFLA_MAX
#define IPSTACK_IFLA_RTA         	IFLA_RTA


#define IPSTACK_NLMSG_ALIGNTO  NLMSG_ALIGNTO
#define IPSTACK_NLMSG_ALIGN    NLMSG_ALIGN
#define IPSTACK_NLMSG_LENGTH   NLMSG_LENGTH
#define IPSTACK_NLMSG_SPACE    NLMSG_SPACE
#define IPSTACK_NLMSG_DATA     NLMSG_DATA
#define IPSTACK_NLMSG_NEXT     NLMSG_NEXT
#define IPSTACK_NLMSG_OK       NLMSG_OK
#define IPSTACK_NLMSG_PAYLOAD  NLMSG_PAYLOAD
#define IPSTACK_NLMSG_NOOP     NLMSG_NOOP
#define IPSTACK_NLMSG_ERROR    NLMSG_ERROR
#define IPSTACK_NLMSG_DONE     NLMSG_DONE
#define IPSTACK_NLMSG_OVERRUN  NLMSG_OVERRUN
#define IPSTACK_NLMSG_LENGTH   NLMSG_LENGTH


#define IPSTACK_NLM_F_ROOT 			NLM_F_ROOT
#define IPSTACK_NLM_F_MATCH 		NLM_F_MATCH
#define IPSTACK_NLM_F_REQUEST 		NLM_F_REQUEST
#define IPSTACK_NLM_F_CREATE 		NLM_F_CREATE
#define IPSTACK_NLM_F_MULTI  		NLM_F_MULTI
#define IPSTACK_NLM_F_ACK   		NLM_F_ACK
#define IPSTACK_NLM_F_REPLACE   	NLM_F_REPLACE


#define IPSTACK_IFA_MAX     		IFA_MAX
#define IPSTACK_IFA_UNSPEC    		IFA_UNSPEC
#define IPSTACK_IFA_ADDRESS    		IFA_ADDRESS
#define IPSTACK_IFA_LOCAL    		IFA_LOCAL
#define IPSTACK_IFA_LABEL    		IFA_LABEL
#define IPSTACK_IFA_BROADCAST    	IFA_BROADCAST
#define IPSTACK_IFA_ANYCAST    		IFA_ANYCAST
#define IPSTACK_IFA_CACHEINFO    	IFA_CACHEINFO
#define IPSTACK_IFA_MULTICAST    	IFA_MULTICAST
#define IPSTACK_IFA_VR    			IFA_VR
#define IPSTACK_IFA_TABLE    		IFA_TABLE
#define IPSTACK_IFA_TABLE_NAME    	IFA_TABLE_NAME
#define IPSTACK_IFA_X_INFO    		IFA_X_INFO
#define IPSTACK_IFA_F_SECONDARY 	IFA_F_SECONDARY
#define IPSTACK_IFA_RTA         	IFA_RTA


#define IPSTACK_RTF_UP              RTF_UP




#define IPSTACK_RT_SCOPE_UNIVERSE 		RT_SCOPE_UNIVERSE
#define IPSTACK_RT_SCOPE_SITE 			RT_SCOPE_SITE
#define IPSTACK_RT_SCOPE_LINK 			RT_SCOPE_LINK
#define IPSTACK_RT_SCOPE_HOST 			RT_SCOPE_HOST
#define IPSTACK_RT_SCOPE_NOWHERE 		RT_SCOPE_NOWHERE

#define IPSTACK_IFT_ETHER                   IFT_ETHER
#define IPSTACK_IFT_BRIDGE                  IFT_BRIDGE
#define IPSTACK_IFT_IEEE8023ADLAG       	IFT_IEEE8023ADLAG
#define IPSTACK_IFT_L2VLAN                  IFT_L2VLAN

#define IPSTACK_RTV_VALUE1                   RTV_VALUE1




#define IPSTACK_FD_ZERO FD_ZERO 
#define IPSTACK_FD_CLR FD_CLR
#define IPSTACK_FD_SET FD_SET
#define IPSTACK_FD_ISSET FD_ISSET

#define ipstack_fd_set fd_set

#define IPSTACK_IPFD_ZERO(dsetp)         IPSTACK_FD_ZERO(dsetp) 
#define IPSTACK_IPFD_CLR(IFD, fdsetp)    IPSTACK_FD_CLR(IFD._fd, dsetp) 
#define IPSTACK_IPFD_SET(IFD, fdsetp)    IPSTACK_FD_SET(IFD._fd, dsetp) 
#define IPSTACK_IPFD_ISSET(IFD, fdsetp)  IPSTACK_FD_ISSET(IFD._fd, dsetp) 

#define ipstack_ipfd_set ipstack_fd_set


/*
 *===========================================================================
 *                   host <-> network endian
 *===========================================================================
 */
/*
#ifndef htons
#define htons(x)          htons(x)
#endif
#ifndef htonl
#define htonl(x)          htonl(x)
#endif
#ifndef ntohs
#define ntohs(x)          ip_ntohs(x)
#endif
#ifndef ntohl
#define ntohl(x)          ip_ntohl(x)
#endif
*/

/*
 *===========================================================================
 *                    IP types
 *===========================================================================
 */
/*
#define id		  id
#define len		  tot_len
#define off		  frag_off

#define ipstack_in_addr_t     in_addr_t
#define ipstack_sa_family_t   sa_family_t
#define ipstack_in_port_t     in_port_t
#define ipstack_socklen_t     socklen_t
*/
#define ipstack_in6_addr      in6_addr
#define ipstack_in6_addr_t    in6_addr_t
//#define s6_addr       in6.addr8
#define ipstack_ethaddr    ethaddr

/*
 *===========================================================================
 *                    iovec
 *===========================================================================
 */
#define ipstack_iovec         iovec


/*
 *===========================================================================
 *                    in_addr
 *===========================================================================
 */
#ifdef s_addr
#undef s_addr
#endif

#define ipstack_in_addr       in_addr


/*
 *===========================================================================
 *                    sockaddr
 *===========================================================================
 */
#define ipstack_sockaddr_in     sockaddr_in
#define ipstack_sockaddr_in6    sockaddr_in6
//#define s6_addr32		s6_addr32
#define ipstack_sockaddr_dl     sockaddr_dl
#define ipstack_sockaddr_ll     sockaddr_ll
#define ipstack_sockaddr        sockaddr
#define ipstack_sockaddr_un     sockaddr_un


/*
 *===========================================================================
 *                         IPv6 Extension Headers
 *===========================================================================
 */
#define ipstack_pkt_ip6_ext_hdr        pkt_ip6_ext_hdr
#define ipstack_pkt_ip6_hbh            pkt_ip6_hbh
#define ipstack_pkt_ip6_dest           pkt_ip6_dest
#define ipstack_pkt_ip6_rthdr          pkt_ip6_rthdr
#define IPSTACK_IPV6_RTHDR_TYPE_0      IPV6_RTHDR_TYPE_0
#define ipstack_pkt_ip6_rthdr0         pkt_ip6_rthdr0
#define ipstack_pkt_ip6_frag           pkt_ip6_frag

#define IPSTACK_IP6F_OFF_MASK          IP6F_OFF_MASK
#define IPSTACK_IP6F_RESERVED_MASK     IP6F_RESERVED_MASK
#define IPSTACK_IP6F_MORE_FRAG         IP6F_MORE_FRAG

#define IPSTACK_IP6F_SIZE              IP6F_SIZE
#define IPSTACK_IP6F_GET_OFFSET(hdr)   IP6F_GET_OFFSET(hdr)


/*
 *===========================================================================
 *                      Extension Header Options
 *===========================================================================
 */
#define ipstack_pkt_ip6_opt              pkt_ip6_opt
#define IPSTACK_IP6OPT_TYPE(o)           IP6OPT_TYPE(o)
#define IPSTACK_IP6OPT_TYPE_SKIP         IP6OPT_TYPE_SKIP
#define IPSTACK_IP6OPT_TYPE_DISCARD      IP6OPT_TYPE_DISCARD
#define IPSTACK_IP6OPT_TYPE_FORCEICMP    IP6OPT_TYPE_FORCEICMP
#define IPSTACK_IP6OPT_TYPE_ICMP         IP6OPT_TYPE_ICMP
#define IPSTACK_IP6OPT_MUTABLE           IP6OPT_MUTABLE
#define IPSTACK_IP6OPT_PAD1              IP6OPT_PAD1
#define IPSTACK_IP6OPT_PADN              IP6OPT_PADN
#define IPSTACK_IP6OPT_JUMBO             IP6OPT_JUMBO
#define IPSTACK_IP6OPT_NSAP_ADDR         IP6OPT_NSAP_ADDR
#define IPSTACK_IP6OPT_TUNNEL_LIMIT      IP6OPT_TUNNEL_LIMIT
#define IPSTACK_IP6OPT_ROUTER_ALERT      IP6OPT_ROUTER_ALERT

#define ipstack_ip6_opt_router           ip6_opt_router
#define IPSTACK_IP6_ALERT_MLD            IP6_ALERT_MLD
#define IPSTACK_IP6_ALERT_RSVP           IP6_ALERT_RSVP
#define IPSTACK_IP6_ALERT_AN             IP6_ALERT_AN


/*
 *===========================================================================
 *                    IPv6 icmp filter
 *===========================================================================
 */
#define IPSTACK_ICMP6_FILTER_WILLPASS(type, f)      ICMP6_FILTER_WILLPASS(type, f)
#define IPSTACK_ICMP6_FILTER_WILLBLOCK(type, f)     ICMP6_FILTER_WILLBLOCK(type, f)
#define IPSTACK_ICMP6_FILTER_SETPASS(type, f)       ICMP6_FILTER_SETPASS(type, f)
#define IPSTACK_ICMP6_FILTER_SETBLOCK(type, f)      ICMP6_FILTER_SETBLOCK(type, f)
#define IPSTACK_ICMP6_FILTER_SETPASSALL(filterp)    ICMP6_FILTER_SETPASSALL(filterp)
#define IPSTACK_ICMP6_FILTER_SETBLOCKALL(filterp)   ICMP6_FILTER_SETBLOCKALL(filterp)


/*
 *===========================================================================
 *                    msghdr
 *===========================================================================
 */
#define ipstack_msghdr        msghdr

/* msg_flags values */
#define IPSTACK_MSG_TRUNC     MSG_TRUNC
#define IPSTACK_MSG_CTRUNC    MSG_CTRUNC
#define IPSTACK_MSG_NOSIGNAL    MSG_NOSIGNAL
#define IPSTACK_MSG_INDICATION    MSG_INDICATION
/*
 *===========================================================================
 *                    cmsghdr
 *===========================================================================
 */
#define ipstack_cmsghdr        cmsghdr

#define IPSTACK_CMSG_ALIGN(len)          CMSG_ALIGN(len)
#define IPSTACK_CMSG_FIRSTHDR(mhdrptr)   CMSG_FIRSTHDR(mhdrptr)
#define IPSTACK_CMSG_NXTHDR(mhdr,cmsg)   CMSG_NXTHDR(mhdr,cmsg)
#define IPSTACK_CMSG_DATA(cmsgptr)       CMSG_DATA(cmsgptr)
#define IPSTACK_CMSG_SPACE(len)          CMSG_SPACE(len)
#define IPSTACK_CMSG_LEN(len)            CMSG_LEN(len)


/*
 *===========================================================================
 *                    if_nameindex
 *===========================================================================
 */

//#define if_index ni.if_index
//#define if_name  ni.if_name

/*
 *===========================================================================
 *                    ioctl structures
 *===========================================================================
 */
#define ipstack_ip_mreq             ip_mreq
#define ipstack_ip_mreqn            ip_mreqn

#define ipstack_arpreq              arpreq
#define ipstack_ortentry            ortentry
#define ipstack_aliasreq            ifaliasreq
#define ifra_broadaddr      ifra_dstaddr

#define ipstack_ip6_mreq            ipv6_mreq
#define ipstack_ipv6_mreq           ipv6_mreq
#define ipstack_in6_pktinfo         in6_pktinfo
#define ipstack_icmp6_filter        icmp6_filter
#define ipstack_in6_addrlifetime    in6_addrlifetime
#define ipstack_in6_aliasreq        in6_aliasreq
#define ipstack_ether_header        ether_header
#define ipstack_ether_arp           ether_arp
#define ipstack_sockaddr_storage    sockaddr_storage

#define ipstack_icmp6_hdr    icmp6_hdr



#define IPSTACK_ND_ROUTER_SOLICIT ND_ROUTER_SOLICIT           
#define IPSTACK_ND_ROUTER_ADVERT ND_ROUTER_ADVERT            
#define IPSTACK_ND_NEIGHBOR_SOLICIT ND_NEIGHBOR_SOLICIT         
#define IPSTACK_ND_NEIGHBOR_ADVERT ND_NEIGHBOR_ADVERT          
#define IPSTACK_ND_REDIRECT ND_REDIRECT                 
#define IPSTACK_ND_RA_FLAG_MANAGED       ND_RA_FLAG_MANAGED
#define IPSTACK_ND_RA_FLAG_OTHER         ND_RA_FLAG_OTHER
#define IPSTACK_ND_RA_FLAG_HOME_AGENT    ND_RA_FLAG_HOME_AGENT


#define IPSTACK_ND_OPT_SOURCE_LINKADDR		ND_OPT_SOURCE_LINKADDR
#define IPSTACK_ND_OPT_TARGET_LINKADDR		ND_OPT_TARGET_LINKADDR
#define IPSTACK_ND_OPT_PREFIX_INFORMATION	ND_OPT_PREFIX_INFORMATION
#define IPSTACK_ND_OPT_REDIRECTED_HEADER	ND_OPT_REDIRECTED_HEADER
#define IPSTACK_ND_OPT_MTU			        ND_OPT_MTU
#define IPSTACK_ND_OPT_RTR_ADV_INTERVAL		ND_OPT_RTR_ADV_INTERVAL
#define IPSTACK_ND_OPT_HOME_AGENT_INFO		ND_OPT_HOME_AGENT_INFO

#define IPSTACK_ND_OPT_PI_FLAG_ONLINK	ND_OPT_PI_FLAG_ONLINK
#define IPSTACK_ND_OPT_PI_FLAG_AUTO	    ND_OPT_PI_FLAG_AUTO
#define IPSTACK_ND_OPT_PI_FLAG_RADDR	ND_OPT_PI_FLAG_RADDR


#define ipstack_nd_router_advert  nd_router_advert
/*
 *===========================================================================
 *                        in_pktinfo
 *===========================================================================
 */
#define ipstack_in_pktinfo      in_pktinfo


/*
 *===========================================================================
 *                        hostent
 *===========================================================================
 */
#define ipstack_hostent         hostent

/*
 *===========================================================================
 *                        ifreq
 *===========================================================================
 */
#define ipstack_ifreq           ifreq

#define ifr_addr        ifr_ifru.ifru_addr
#define ifr_dstaddr     ifr_ifru.ifru_dstaddr
#define ifr_broadaddr   ifr_ifru.ifru_broadaddr
#define ifr_flags       ifr_ifru.ifru_flags
#define ifr_metric      ifr_ifru.ifru_ivalue
#define ifr_mtu         ifr_ifru.ifru_mtu
#define ifr_data        ifr_ifru.ifru_data
#define ifr_rtab        ifr_ifru.ifru_rtab


/*
 *===========================================================================
 *                        in6_ifreq
 *===========================================================================
 */
#define ipstack_in6_ifreq       in6_ifreq

#define IPSTACK_IN6_IFF_TENTATIVE   IN6_IFF_TENTATIVE


/*
 ****************************************************************************
 * 6                    FUNCTIONS
 ****************************************************************************
 */



/* functions used by both sockets & files. */

/*
 *===========================================================================
 *                        socket only functions
 *===========================================================================
 */

#define ipstack_inet_pton(family,strptr,addrptr)      inet_pton(family,(const char *)strptr,addrptr)
#define ipstack_inet_ntop(family,addrptr,strptr,len)  inet_ntop(family,(const void *)addrptr,strptr,(size_t)len)

#define ipstack_if_nametoindex(ifname)             if_nametoindex((const char *)ifname)
#define ipstack_if_indextoname(ifindex,ifname)     if_indextoname((int)ifindex, (char *)ifname)
#define ipstack_if_nameindex()                     if_nameindex()
#define ipstack_if_freenameindex(pif)              if_freenameindex(pif)

//#define ipstack_inet_aton inet_aton
//#define ipstack_inet_pton inet_pton
//#define ipstack_inet_ntoa inet_ntoa
//#define ipstack_inet_addr inet_addr
#define ipstack_inet_ntoa(in)           inet_ntoa(in)
#define ipstack_inet_addr(cp)           inet_addr(cp)
#define ipstack_inet_aton(cp,addr)      inet_aton(cp,addr)

#define ipstack_inet6_rth_space(t,s)               inet6_rth_space(t,s)
#define ipstack_inet6_rth_init(b,l,t,s)            inet6_rth_init(b,l,t,s)
#define ipstack_inet6_rth_add(b,a)                 inet6_rth_add(b,a)
#define ipstack_inet6_rth_reverse(i,o)             inet6_rth_reverse(i,o)
#define ipstack_inet6_rth_segments(b)              inet6_rth_segments(b)
#define ipstack_inet6_rth_getaddr(b,i)             inet6_rth_getaddr(b,i)

#define ipstack_inet6_opt_init(b,l)                inet6_opt_init(b,l)
#define ipstack_inet6_opt_append(eb,el,o,t,l,a,db) inet6_opt_append(eb,el,o,t,l,a,db)
#define ipstack_inet6_opt_finish(eb,el,o)          inet6_opt_finish(eb,el,o)
#define ipstack_inet6_opt_set_val(db,o,v,l)        inet6_opt_set_val(db,o,v,l)
#define ipstack_inet6_opt_next(eb,el,o,t,l,db)     inet6_opt_next(eb,el,o,t,l,db)
#define ipstack_inet6_opt_find(eb,el,o,t,l,db)     inet6_opt_find(eb,el,o,t,l,db)
#define ipstack_inet6_opt_get_val(db,o,v,l)        inet6_opt_get_val(db,o,v,l)


/*
 ****************************************************************************
 * 7                    SOCK2
 ****************************************************************************
 */

#define ipstack_protoent   protoent
#define ipstack_servent    servent
#define ipstack_addrinfo   addrinfo
#define ipstack_group_req   group_req


/* port/src/gethostby.c (use getipnodebyname/getipnodebyaddr for all new code!) */
#define ipstack_gethostbyname(name)     gethostbyname(name)
#define ipstack_gethostbyname2(addrptr, family)    gethostbyname2(addrptr, family)

#define ipstack_gethostbyaddr(a,l,t)    gethostbyaddr(a,l,t)

/* src/sock2.c */
#define ipstack_getprotobyname(name)    getprotobyname(name)
#define ipstack_getprotobynumber(proto) getprotobynumber(proto)

/* port/src/getservby.c */
#define ipstack_getservbyname(n,p)      getservbyname(n,p)
#define ipstack_getservbyport(p,p2)     getservbyport(p,p2)


#define ipstack_getaddrinfo(family,addrptr,strptr,len)   getaddrinfo(family,addrptr,strptr,len)
#define ipstack_freeaddrinfo(pif)            freeaddrinfo(pif)
#define ipstack_getnameinfo              getnameinfo



#define ipstack_nlmsghdr               	nlmsghdr
#define ipstack_ifaddrmsg 				ifaddrmsg
#define ipstack_rtmsg     				rtmsg
#define ipstack_rtgenmsg  				rtgenmsg
#define ipstack_sockaddr_nl   			sockaddr_nl
#define ipstack_nlmsgerr      			nlmsgerr
#define ipstack_rtattr        			rtattr
#define ipstack_ifinfomsg     			ifinfomsg
#define ipstack_ifa_cacheinfo 			ifa_cacheinfo
#define ipstack_rtnexthop      			rtnexthop

#define  ipstack_ifconf                 ifconf
//#define  ifc_req                        ip_ifc_req
#define  ipstack_iphdr                  iphdr
#define  ipstack_ip                     ip
#define  ipstack_ip_auth_hdr            ip_auth_hdr
#define  ipstack_ip_esp_hdr             ip_esp_hdr
#define  ipstack_ip_comp_hdr            ip_comp_hdr
#define  ipstack_ip_beet_phdr           ip_beet_phdr

#define  ipstack_ipv6hdr                ipv6hdr
#define  ipstack_ip6_hdr                ip6_hdr
#define  ipstack_ethhdr                 ethhdr
#define  ipstack_arphdr                 arphdr
#define  ipstack_icmphdr                icmphdr
#define  ipstack_icmp6hdr               icmp6hdr
#define  ipstack_igmphdr                igmphdr
#define  ipstack_igmpv3_query           igmpv3_query
#define  ipstack_igmpv3_report          igmpv3_report
#define  ipstack_igmpv3_grec            igmpv3_grec

#define  ipstack_vxlanhdr                 vxlanhdr
#define  ipstack_udphdr                 udphdr
#define  ipstack_tcphdr                tcphdr

#define  ipstack_vlan_hdr                 vlan_hdr
#define  ipstack_vlan_ethhdr                 vlan_ethhdr

#define  IPSTACK_LLADDR                 SOCKADDR_DL_LLADDR


#define ipstack_ip_mreqn           ip_mreqn
#define ipstack_tcp_info           tcp_info


#define ipstack_ifaddrs   ifaddrs
#define ipstack_getifaddrs  getifaddrs
#define ipstack_freeifaddrs freeifaddrs
#define ipstack_if_data   Ipnet_if_data

#define   ipstack_utsname                       utsname
#define   ipstack_uname                          uname


/*
 *===========================================================================
 *                    timeval
 *===========================================================================
 */
#define ipstack_timeval       timeval


//#define ipstack_strerror   strerror




/*
 *===========================================================================
 *                         ERRNO
 *===========================================================================
 */
#define IPSTACK_ERRNO_EDESTADDRREQ       EDESTADDRREQ
#define IPSTACK_ERRNO_ENETUNREACH        ENETUNREACH
#define IPSTACK_ERRNO_ENETRESET          ENETRESET
#define IPSTACK_ERRNO_ECONNABORTED       ECONNABORTED
#define IPSTACK_ERRNO_ECONNRESET         ECONNRESET
#define IPSTACK_ERRNO_ENOBUFS            ENOBUFS
#define IPSTACK_ERRNO_EISCONN            EISCONN
#define IPSTACK_ERRNO_ENOTCONN           ENOTCONN
#define IPSTACK_ERRNO_ESHUTDOWN          ESHUTDOWN
#define IPSTACK_ERRNO_ETOOMANYREFS       ETOOMANYREFS
#define IPSTACK_ERRNO_ECONNREFUSED       ECONNREFUSED
#define IPSTACK_ERRNO_ENETDOWN           ENETDOWN
#define IPSTACK_ERRNO_EHOSTUNREACH       EHOSTUNREACH
#define IPSTACK_ERRNO_EINPROGRESS        EINPROGRESS
#define IPSTACK_ERRNO_EALREADY           EALREADY
#define IPSTACK_ERRNO_EINVAL             EINVAL
#define IPSTACK_ERRNO_EHOSTDOWN          EHOSTDOWN
#define IPSTACK_ERRNO_ETIMEDOUT          ETIMEDOUT
#define IPSTACK_ERRNO_ETIME              ETIMEDOUT
#define IPSTACK_ERRNO_EADDRINUSE         EADDRINUSE
#define IPSTACK_ERRNO_EOPNOTSUPP         EOPNOTSUPP

#define	IPSTACK_ERRNO_EIO                            EIO
#define IPSTACK_ERRNO_E2BIG                          E2BIG
#define	IPSTACK_ERRNO_ENOENT                         ENOENT

#define IPSTACK_ERRNO_EINTR       EINTR
#define IPSTACK_ERRNO_EWOULDBLOCK EWOULDBLOCK
#define IPSTACK_ERRNO_EAGAIN      EAGAIN
#define IPSTACK_ERRNO_ENODEV      ENODEV
#define IPSTACK_ERRNO_ESRCH       ESRCH
#define IPSTACK_ERRNO_EEXIST      EEXIST

#define IPSTACK_ERRNO_ENOMEM     	ENOMEM
#define IPSTACK_ERRNO_EBUSY        	EBUSY
#define IPSTACK_ERRNO_EFAULT       	EFAULT
#define IPSTACK_ERRNO_EADDRINUSE   EADDRINUSE
#define IPSTACK_ERRNO_EPIPE       	EPIPE
#define IPSTACK_ERRNO_EBADF       	EBADF
//#define IPSTACK_ERRNO_EIO       	EIO

//#define IPSTACK_ERRNO_E2BIG    E2BIG
//#define IPSTACK_ERRNO_ENOENT   ENOENT

#define IPSTACK_ERRNO_ERANGE       	ERANGE
#define  ipstack_errno       	errno

#ifdef __cplusplus
}
#endif

#endif /* __OS_STACK_H__ */
