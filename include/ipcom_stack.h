/*
 * ipcom_stack.h
 *
 *  Created on: Jun 19, 2018
 *      Author: zhurish
 */

#ifndef __IPCOM_STACK_H__
#define __IPCOM_STACK_H__

#ifdef __cplusplus
extern "C" {
#endif

#define IPSTACK_AF_UNSPEC     IP_AF_UNSPEC
#define IPSTACK_AF_INET       IP_AF_INET
#define IPSTACK_AF_NETLINK    IP_AF_NETLINK
#define IPSTACK_AF_ROUTE      IP_AF_ROUTE
#define IPSTACK_AF_LINK       IP_AF_LINK
#define IPSTACK_AF_PACKET     IP_AF_PACKET
#define IPSTACK_AF_INET6      IP_AF_INET6
#define IPSTACK_AF_KEY        IP_AF_KEY
#define IPSTACK_AF_MAX        IP_AF_MAX

#define IPSTACK_PF_INET       IP_PF_INET
#define IPSTACK_PF_NETLINK    IP_PF_NETLINK
#define IPSTACK_PF_ROUTE      IP_PF_ROUTE
#define IPSTACK_PF_LINK       IP_PF_LINK
#define IPSTACK_PF_PACKET     IP_PF_PACKET
#define IPSTACK_PF_INET6      IP_PF_INET6
#define IPSTACK_PF_KEY        IP_PF_KEY


/*
 *===========================================================================
 *                         Socket types
 *===========================================================================
 */

#define IPSTACK_SOCK_STREAM      IP_SOCK_STREAM
#define IPSTACK_SOCK_DGRAM       IP_SOCK_DGRAM
#define IPSTACK_SOCK_RAW         IP_SOCK_RAW
#define IPSTACK_SOCK_PACKET      IP_SOCK_PACKET
/*
 *===========================================================================
 *                         IP protocol values
 *===========================================================================
 */

#define IPSTACK_IPPROTO_IP          IP_IPPROTO_IP
#define IPSTACK_IPPROTO_ICMP        IP_IPPROTO_ICMP
#define IPSTACK_IPPROTO_IGMP        IP_IPPROTO_IGMP
#define IPSTACK_IPPROTO_IPIP        IP_IPPROTO_IPIP
#define IPSTACK_IPPROTO_IPV4        IP_IPPROTO_IPV4
#define IPSTACK_IPPROTO_TCP         IP_IPPROTO_TCP
#define IPSTACK_IPPROTO_UDP         IP_IPPROTO_UDP
#define IPSTACK_IPPROTO_IPV6        IP_IPPROTO_IPV6
#define IPSTACK_IPPROTO_RSVP        IP_IPPROTO_RSVP
#define IPSTACK_IPPROTO_GRE         IP_IPPROTO_GRE
#define IPSTACK_IPPROTO_ESP         IP_IPPROTO_ESP
#define IPSTACK_IPPROTO_AH          IP_IPPROTO_AH
#define IPSTACK_IPPROTO_ICMPV6      IP_IPPROTO_ICMPV6
#define IPSTACK_IPPROTO_OSPFIGP     IP_IPPROTO_OSPFIGP
#define IPSTACK_IPPROTO_PIM         IP_IPPROTO_PIM
#define IPSTACK_IPPROTO_RAW         IP_IPPROTO_RAW
#define IPSTACK_IPPROTO_MAX         IP_IPPROTO_MAX


/* IPv6 Non-IP Protocol Next Header field values */

#define IPSTACK_IPPROTO_HOPOPTS     IP_IPPROTO_HOPOPTS
#define IPSTACK_IPPROTO_ROUTING     IP_IPPROTO_ROUTING
#define IPSTACK_IPPROTO_FRAGMENT    IP_IPPROTO_FRAGMENT
#define IPSTACK_IPPROTO_NONE        IP_IPPROTO_NONE
#define IPSTACK_IPPROTO_DSTOPTS     IP_IPPROTO_DSTOPTS



/*
 *===========================================================================
 *                  Standard IP address macros.
 *===========================================================================
 */

#define IPSTACK_INADDR_ANY             IP_INADDR_ANY
#define IPSTACK_INADDR_DEFAULT         IP_INADDR_DEFAULT
#define IPSTACK_INADDR_LOOPBACK        IP_INADDR_LOOPBACK
#define IPSTACK_INADDR_BROADCAST       IP_INADDR_BROADCAST
#define	IPSTACK_INADDR_NONE            IP_INADDR_NONE

#define IPSTACK_INADDR_UNSPEC_GROUP    IP_INADDR_UNSPEC_GROUP
#define IPSTACK_INADDR_ALLHOSTS_GROUP  IP_INADDR_ALLHOSTS_GROUP


/*
 *===========================================================================
 *                         NETMASK
 *===========================================================================
 */

#define IPSTACK_NETMASK(xxip)          IP_NETMASK(xxip)
#define IPSTACK_NETBITS(xxip)          IP_NETBITS(xxip)

#define	IPSTACK_IN_CLASSA(xxip)	       IP_IN_CLASSA(xxip)
#define	IPSTACK_IN_CLASSB(xxip)        IP_IN_CLASSB(xxip)
#define	IPSTACK_IN_CLASSC(xxip)        IP_IN_CLASSC(xxip)
#define	IPSTACK_IN_CLASSD(xxip)        IP_IN_CLASSD(xxip)
#define	IPSTACK_IN_EXPERIMENTAL(xxip)  IP_IN_EXPERIMENTAL(xxip)
#define IPSTACK_INET_ADDRSTRLEN        IP_INET_ADDRSTRLEN
#define IPSTACK_IN_MULTICAST(xxip)     IPSTACK_IN_CLASSD(xxip)

/*
 *===========================================================================
 *                      IPv6 address macros
 *===========================================================================
 */


#define IPSTACK_IN6_ARE_ADDR_EQUAL(addr1)            IP_IN6_ARE_ADDR_EQUAL(addr1)
#define IPSTACK_IN6_IS_ADDR_UNSPECIFIED(addr)        IP_IN6_IS_ADDR_UNSPECIFIED(addr)
#define IPSTACK_IN6_IS_ADDR_LOOPBACK(addr)           IP_IN6_IS_ADDR_LOOPBACK(addr)
#define IPSTACK_IN6_IS_ADDR_V4COMPAT(addr)           IP_IN6_IS_ADDR_V4COMPAT(addr)
#define IPSTACK_IN6_IS_ADDR_V4MAPPED(addr)           IP_IN6_IS_ADDR_V4MAPPED(addr)
#define IPSTACK_IN6_IS_ADDR_AGGR_GLOB_UNICAST(addr)  IP_IN6_IS_ADDR_AGGR_GLOB_UNICAST(addr)
#define IPSTACK_IN6_IS_ADDR_SITE_LOCAL(addr)         IP_IN6_IS_ADDR_SITE_LOCAL(addr)
#define IPSTACK_IN6_IS_ADDR_LINK_LOCAL(addr)         IP_IN6_IS_ADDR_LINK_LOCAL(addr)
#define IPSTACK_IN6_IS_ADDR_MULTICAST(addr)          IP_IN6_IS_ADDR_MULTICAST(addr)
#define IPSTACK_IN6_IS_ADDR_LINKLOCAL(addr)          IP_IN6_IS_ADDR_LINK_LOCAL(addr)


/*
 *===========================================================================
 *                         setsockopt/getsockopt
 *===========================================================================
 */

#define IPSTACK_SOL_SOCKET         IP_SOL_SOCKET

/***** IP_SOL_SOCKET socket options: *****/
#define IPSTACK_SO_REUSEADDR       IP_SO_REUSEADDR
#define IPSTACK_SO_KEEPALIVE       IP_SO_KEEPALIVE
#define IPSTACK_SO_DONTROUTE       IP_SO_DONTROUTE
#define IPSTACK_SO_BROADCAST       IP_SO_BROADCAST
#define IPSTACK_SO_REUSEPORT       IP_SO_REUSEPORT
#define IPSTACK_SO_SNDBUF          IP_SO_SNDBUF
#define IPSTACK_SO_RCVBUF          IP_SO_RCVBUF
#define IPSTACK_SO_RCVTIMEO        IP_SO_RCVTIMEO
#define IPSTACK_SO_ERROR           IP_SO_ERROR
#define IPSTACK_SO_TYPE            IP_SO_TYPE
#define IPSTACK_SO_BINDTODEVICE    IP_SO_BINDTODEVICE

/***** IP_IPPROTO_IP level socket options: *****/
#define IPSTACK_IP_HDRINCL         IP_IP_HDRINCL
#define IPSTACK_IP_TOS             IP_IP_TOS
#define IPSTACK_IP_TTL             IP_IP_TTL
#define IPSTACK_IP_PKTINFO         IP_IP_PKTINFO
#define IPSTACK_IP_MULTICAST_IF    IP_IP_MULTICAST_IF
#define IPSTACK_IP_MULTICAST_TTL   IP_IP_MULTICAST_TTL
#define IPSTACK_IP_MULTICAST_LOOP  IP_IP_MULTICAST_LOOP
#define IPSTACK_IP_ADD_MEMBERSHIP  IP_IP_ADD_MEMBERSHIP
#define	IPSTACK_IP_JOIN_GROUP      IP_IP_JOIN_GROUP
#define IPSTACK_IP_DROP_MEMBERSHIP IP_IP_DROP_MEMBERSHIP
#define	IPSTACK_IP_LEAVE_GROUP     IP_IP_LEAVE_GROUP
#define IPSTACK_IP_RECVIF          IP_IP_RECVIF
#define IPSTACK_IP_ROUTER_ALERT    IP_IP_ROUTER_ALERT
#define IPSTACK_IP_DONTFRAG        IP_IP_DONTFRAG

#define	IPSTACK_MCAST_JOIN_GROUP	IP_JOIN_GROUP
#define	IPSTACK_MCAST_LEAVE_GROUP	IP_LEAVE_GROUP


/***** IP_IPPROTO_IPV6 level socket options: *****/
#define IPSTACK_IPV6_UNICAST_HOPS     IP_IPV6_UNICAST_HOPS
#define IPSTACK_IPV6_MULTICAST_IF     IP_IPV6_MULTICAST_IF
#define IPSTACK_IPV6_MULTICAST_HOPS   IP_IPV6_MULTICAST_HOPS
#define IPSTACK_IPV6_MULTICAST_LOOP   IP_IPV6_MULTICAST_LOOP
#define	IPSTACK_IPV6_JOIN_GROUP       IP_IPV6_JOIN_GROUP
#define IPSTACK_IPV6_ADD_MEMBERSHIP   IP_IPV6_ADD_MEMBERSHIP
#define	IPSTACK_IPV6_LEAVE_GROUP      IP_IPV6_LEAVE_GROUP
#define IPSTACK_IPV6_DROP_MEMBERSHIP  IP_IPV6_DROP_MEMBERSHIP
#define IPSTACK_IPV6_PKTINFO          IP_IPV6_PKTINFO
#define IPSTACK_IPV6_TCLASS           IP_IPV6_TCLASS
#define IPSTACK_IPV6_NEXTHOP          IP_IPV6_NEXTHOP
#define IPSTACK_IPV6_RTHDR            IP_IPV6_RTHDR
#define IPSTACK_IPV6_HOPOPTS          IP_IPV6_HOPOPTS
#define IPSTACK_IPV6_DSTOPTS          IP_IPV6_DSTOPTS
#define IPSTACK_IPV6_RTHDRDSTOPTS     IP_IPV6_RTHDRDSTOPTS
#define IPSTACK_IPV6_RECVPKTINFO      IP_IPV6_RECVPKTINFO
#define IPSTACK_IPV6_RECVHOPLIMIT     IP_IPV6_RECVHOPLIMIT
#define IPSTACK_IPV6_RECVTCLASS       IP_IPV6_RECVTCLASS
#define IPSTACK_IPV6_RECVRTHDR        IP_IPV6_RECVRTHDR
#define IPSTACK_IPV6_RECVHOPOPTS      IP_IPV6_RECVHOPOPTS
#define IPSTACK_IPV6_RECVDSTOPTS      IP_IPV6_RECVDSTOPTS
#define IPSTACK_IPV6_CHECKSUM         IP_IPV6_CHECKSUM
#define IPSTACK_IPV6_RECVIF           IP_IPV6_RECVIF

/***** IP_IPPROTO_ICMPV6 level socket options: *****/
#define IPSTACK_ICMP6_FILTER          IP_ICMP6_FILTER

/***** IP_IPPROTO_TCP level socket options: *****/
#define IPSTACK_TCP_NODELAY           IP_TCP_NODELAY

/* NOTE: This option can NOT be used with getsockopt/setsockopt!
 * Only sent/rcvd with ancillary data!
 */
#define IPSTACK_IPV6_HOPLIMIT         IP_IPV6_HOPLIMIT

#define	IPSTACK_ARPHRD_ETHER          IP_ARPHRD_ETHER


#define IPSTACK_MSG_OOB			IP_MSG_OOB                    /* Specify to send/receive out-of-band data */
#define IPSTACK_MSG_PEEK		IP_MSG_PEEK                   /* Leave the data on the receive buffer (receive only) */
#define IPSTACK_MSG_DONTROUTE	IP_MSG_DONTROUTE              /* Bypass normal routing, the packet will be sent
                                            on the interface that matches the network part
                                            of the destination address */
#define IPSTACK_MSG_WAITALL		IP_MSG_WAITALL               /* Wait for full request or error (receive only) */
#define IPSTACK_MSG_DONTWAIT	IP_MSG_DONTWAIT              /* Enables non-blocking operation */

#define IPSTACK_MSG_NOTIFICATION	IP_MSG_NOTIFICATION    /* SCTP Notification */
#define IPSTACK_MSG_EOR				IP_MSG_EOR

#define IPSTACK_MSG_MORE		IP_MSG_MORE                /* Disable Nagle algorithm (send only) */
#ifdef IPCOM_ZEROCOPY
#define IPSTACK_MSG_ZBUF		IP_MSG_ZBUF               /* zbuf */
#endif
//#endif
/*
 *===========================================================================
 *                         ioctl
 *===========================================================================
 */
/* socket ioctl */
#define IPSTACK_SIOCSPGRP          IP_SIOCSPGRP
#define IPSTACK_SIOCGPGRP          IP_SIOCGPGRP
#define IPSTACK_FIONBIO            IP_FIONBIO

/* AF_INET ioctl. */
#define IPSTACK_SIOCGIFADDR        IP_SIOCGIFADDR
#define IPSTACK_SIOCSIFADDR        IP_SIOCSIFADDR
#define IPSTACK_SIOCGIFBRDADDR     IP_SIOCGIFBRDADDR
#define IPSTACK_SIOCSIFBRDADDR     IP_SIOCSIFBRDADDR
#define IPSTACK_SIOCGIFNETMASK     IP_SIOCGIFNETMASK
#define IPSTACK_SIOCSIFNETMASK     IP_SIOCSIFNETMASK
#define IPSTACK_SIOCGIFDSTADDR     IP_SIOCGIFDSTADDR
#define IPSTACK_SIOCSIFDSTADDR     IP_SIOCSIFDSTADDR
#define IPSTACK_SIOCAIFADDR        IP_SIOCAIFADDR
#define IPSTACK_SIOCDIFADDR        IP_SIOCDIFADDR
#define IPSTACK_SIOCADDRT          IP_SIOCADDRT
#define IPSTACK_SIOCDDDRT          IP_SIOCDDDRT

/* AF_INET6 ioctl. */
#define IPSTACK_SIOCAIFADDR_IN6    IP_SIOCAIFADDR_IN6
#define IPSTACK_SIOCDIFADDR_IN6    IP_SIOCDIFADDR_IN6
#define IPSTACK_SIOCGIFDSTADDR_IN6 IP_SIOCGIFDSTADDR_IN6

/* interface ioctl. */
#define IPSTACK_SIOCGIFFLAGS       IP_SIOCGIFFLAGS
#define IPSTACK_SIOCSIFFLAGS       IP_SIOCSIFFLAGS
#define IPSTACK_SIOCGIFMTU         IP_SIOCGIFMTU
#define IPSTACK_SIOCSIFMTU         IP_SIOCSIFMTU
#define IPSTACK_SIOCGIFMETRIC      IP_SIOCGIFMETRIC
#define IPSTACK_SIOCSIFMETRIC      IP_SIOCSIFMETRIC
#define IPSTACK_SIOCSIFRTAB        IP_SIOCSIFRTAB
#define IPSTACK_SIOCGIFRTAB        IP_SIOCGIFRTAB
#define IPSTACK_SIOCGETTUNNEL      IP_SIOCGETTUNNEL
#define IPSTACK_SIOCADDTUNNEL      IP_SIOCADDTUNNEL
#define IPSTACK_SIOCCHGTUNNEL      IP_SIOCCHGTUNNEL
#define IPSTACK_SIOCDELTUNNEL      IP_SIOCDELTUNNEL

/* arp ioctl. */
#define IPSTACK_SIOCSARP           IP_SIOCSARP
#define IPSTACK_SIOCGARP           IP_SIOCGARP
#define IPSTACK_SIOCDARP           IP_SIOCDARP

#define  IPSTACK_SIOCGIFHWADDR          IP_SIOCGIFHWADDR
#define  IPSTACK_SIOCGIFCONF            IP_SIOCGIFCONF
#define	 IPSTACK_SIOCGIFTXQLEN          IP_SIOCGIFTXQLEN

/*
 *===========================================================================
 *                         IFF_X
 *===========================================================================
 */
#define IPSTACK_IFF_UP          IP_IFF_UP
#define IPSTACK_IFF_BROADCAST   IP_IFF_BROADCAST
#define IPSTACK_IFF_DEBUG       IP_IFF_DEBUG
#define IPSTACK_IFF_LOOPBACK    IP_IFF_LOOPBACK
#define IPSTACK_IFF_POINTOPOINT IP_IFF_POINTOPOINT
#define IPSTACK_IFF_RUNNING     IP_IFF_RUNNING
#define IPSTACK_IFF_NOARP       IP_IFF_NOARP
#define IPSTACK_IFF_PROMISC     IP_IFF_PROMISC
#define IPSTACK_IFF_ALLMULTI    IP_IFF_ALLMULTI
#define IPSTACK_IFF_OACTIVE     IP_IFF_OACTIVE
#define IPSTACK_IFF_SIMPLEX     IP_IFF_SIMPLEX
#define IPSTACK_IFF_LINK0       IP_IFF_LINK0
#define IPSTACK_IFF_LINK1       IP_IFF_LINK1
#define IPSTACK_IFF_LINK2       IP_IFF_LINK2
#define IPSTACK_IFF_MULTICAST   IP_IFF_MULTICAST


/*
 *===========================================================================
 *                    recvxx() argument flags
 *===========================================================================
 */
#define IPSTACK_MSG_PEEK           IP_MSG_PEEK


/*
 *===========================================================================
 *                    shutdown() argument flags
 *===========================================================================
 */
#define IPSTACK_SHUT_RD            IP_SHUT_RD
#define IPSTACK_SHUT_WR            IP_SHUT_WR
#define IPSTACK_SHUT_RDWR          IP_SHUT_RDWR



#define IPSTACK_SOCKADDR_DL_LLADDR(sa)   IP_SOCKADDR_DL_LLADDR(sa)

/* Packet types 'sll_pkttype'.    Packet is addressed to: */
#define IPSTACK_PACKET_HOST          IP_PACKET_HOST
#define IPSTACK_PACKET_BROADCAST     IP_PACKET_BROADCAST
#define IPSTACK_PACKET_MULTICAST     IP_PACKET_MULTICAST
#define IPSTACK_PACKET_OTHERHOST     IP_PACKET_OTHERHOST
#define IPSTACK_PACKET_OUTGOING      IP_PACKET_OUTGOING



/******************************************************************************/
/******************************************************************************/
/*_IPCOM_DEFINE_H_*/

#define IPSTACK_NETLINK_ROUTE   		IP_NETLINK_ROUTE
#define IPSTACK_RT_TABLE_MAIN   		IP_RT_TABLE_MAIN


#define IPSTACK_RTM_VERSION            IPNET_RTM_VERSION
#define IPSTACK_RTM_NEW_RTMSG          IP_RTM_NEWROUTE
#define IPSTACK_RTM_ADD                IPNET_RTM_ADD
#define IPSTACK_RTM_DELETE             IPNET_RTM_DELETE
#define IPSTACK_RTM_MISS               IPNET_RTM_MISS
#define IPSTACK_RTM_LOCK               IPNET_RTM_LOCK
#define IPSTACK_RTM_OLDADD             IPNET_RTM_OLDADD
#define IPSTACK_RTM_OLDDEL             IPNET_RTM_OLDDEL
#define IPSTACK_RTM_RESOLVE            IPNET_RTM_RESOLVE
#define IPSTACK_RTM_CHANGE             IPNET_RTM_CHANGE
#define IPSTACK_RTM_REDIRECT           IPNET_RTM_REDIRECT
#define IPSTACK_RTM_IFINFO             IPNET_RTM_IFINFO


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



#define IPSTACK_RTMGRP_LINK            IP_RTMGRP_LINK
#define IPSTACK_RTMGRP_IPV4_ROUTE      IP_RTMGRP_IPV4_ROUTE
#define IPSTACK_RTMGRP_IPV4_IFADDR     IP_RTMGRP_IPV4_IFADDR
#define IPSTACK_RTMGRP_IPV6_ROUTE      IP_RTMGRP_IPV6_ROUTE
#define IPSTACK_RTMGRP_IPV6_IFADDR     IP_RTMGRP_IPV6_IFADDR

#define IPSTACK_RTM_NEWLINK    		IP_RTM_NEWLINK
#define IPSTACK_RTM_DELLINK    		IP_RTM_DELLINK
#define IPSTACK_RTM_GETLINK    		IP_RTM_GETLINK
#define IPSTACK_RTM_SETLINK    		IP_RTM_SETLINK
#define IPSTACK_RTM_NEWADDR    		IP_RTM_NEWADDR
#define IPSTACK_RTM_DELADDR    		IP_RTM_DELADDR
#define IPSTACK_RTM_GETADDR    		IP_RTM_GETADDR
#define IPSTACK_RTM_NEWROUTE    	IP_RTM_NEWROUTE
#define IPSTACK_RTM_DELROUTE    	IP_RTM_DELROUTE
#define IPSTACK_RTM_GETROUTE    	IP_RTM_GETROUTE
#define IPSTACK_RTM_NEWNEIGH    	IP_RTM_NEWNEIGH
#define IPSTACK_RTM_DELNEIGH    	IP_RTM_DELNEIGH
#define IPSTACK_RTM_GETNEIGH    	IP_RTM_GETNEIGH
#define IPSTACK_RTM_NEWRULE    		IP_RTM_NEWRULE
#define IPSTACK_RTM_DELRULE    		IP_RTM_DELRULE
#define IPSTACK_RTM_GETRULE    		IP_RTM_GETRULE
#define IPSTACK_RTM_NEWQDISC    	IP_RTM_NEWQDISC
#define IPSTACK_RTM_DELQDISC    	IP_RTM_DELQDISC
#define IPSTACK_RTM_GETQDISC    	IP_RTM_GETQDISC
#define IPSTACK_RTM_NEWTCLASS    	IP_RTM_NEWTCLASS
#define IPSTACK_RTM_DELTCLASS    	IP_RTM_DELTCLASS
#define IPSTACK_RTM_GETTCLASS    	IP_RTM_GETTCLASS
#define IPSTACK_RTM_NEWTFILTER    	IP_RTM_NEWTFILTER
#define IPSTACK_RTM_DELTFILTER    	IP_RTM_DELTFILTER
#define IPSTACK_RTM_GETTFILTER    	IP_RTM_GETTFILTER
#define IPSTACK_RTM_NEWACTION    	IP_RTM_NEWACTION
#define IPSTACK_RTM_DELACTION    	IP_RTM_DELACTION
#define IPSTACK_RTM_GETACTION    	IP_RTM_GETACTION
#define IPSTACK_RTM_NEWPREFIX    	IP_RTM_NEWPREFIX
#define IPSTACK_RTM_GETPREFIX    	IP_RTM_GETPREFIX
#define IPSTACK_RTM_GETMULTICAST    IP_RTM_GETMULTICAST
#define IPSTACK_RTM_GETANYCAST    	IP_RTM_GETANYCAST
#define IPSTACK_RTM_NEWNEIGHTBL    	IP_RTM_NEWNEIGHTBL
#define IPSTACK_RTM_GETNEIGHTBL    	IP_RTM_GETNEIGHTBL
#define IPSTACK_RTM_SETNEIGHTBL    	IP_RTM_SETNEIGHTBL
#define IPSTACK_RTM_NEWVR    		IP_RTM_NEWVR
#define IPSTACK_RTM_DELVR    		IP_RTM_DELVR
#define IPSTACK_RTM_GETVR    		IP_RTM_GETVR
#define IPSTACK_RTM_CHANGEVR    	IP_RTM_CHANGEVR
#define IPSTACK_RTM_NEWMIP    		IP_RTM_NEWMIP
#define IPSTACK_RTM_DELMIP    		IP_RTM_DELMIP
#define IPSTACK_RTM_GETMIP    		IP_RTM_GETMIP
#define IPSTACK_RTM_SETMIP    		IP_RTM_SETMIP
#define IPSTACK_RTM_NEWSEND    		IP_RTM_NEWSEND
#define IPSTACK_RTM_GETSEND    		IP_RTM_GETSEND
#define IPSTACK_RTM_SEND_SIGN_REQ   IP_RTM_SEND_SIGN_REQ

#define IPSTACK_RTM_RTA             IP_RTM_RTA


#define IPSTACK_RTM_F_CLONED        IP_RTM_F_CLONED

#define IPSTACK_RTN_UNSPEC    		IP_RTN_UNSPEC
#define IPSTACK_RTN_UNICAST    		IP_RTN_UNICAST
#define IPSTACK_RTN_LOCAL    		IP_RTN_LOCAL
#define IPSTACK_RTN_BROADCAST    	IP_RTN_BROADCAST
#define IPSTACK_RTN_ANYCAST    		IP_RTN_ANYCAST
#define IPSTACK_RTN_MULTICAST    	IP_RTN_MULTICAST
#define IPSTACK_RTN_BLACKHOLE    	IP_RTN_BLACKHOLE
#define IPSTACK_RTN_UNREACHABLE    	IP_RTN_UNREACHABLE
#define IPSTACK_RTN_PROHIBIT    	IP_RTN_PROHIBIT
#define IPSTACK_RTN_THROW    		IP_RTN_THROW
#define IPSTACK_RTN_NAT    			IP_RTN_NAT
#define IPSTACK_RTN_XRESOLVE    	IP_RTN_XRESOLVE
#define IPSTACK_RTN_PROXY    		IP_RTN_PROXY
#define IPSTACK_RTN_CLONE    		IP_RTN_CLONE


#define IPSTACK_RTNH_DATA              IP_RTNH_DATA
#define IPSTACK_RTNH_NEXT              IP_RTNH_NEXT
#define IPSTACK_RTNH_F_ONLINK          IP_RTNH_F_ONLINK


#define IPSTACK_RTPROT_UNSPEC    	IP_RTPROT_UNSPEC
#define IPSTACK_RTPROT_REDIRECT    	IP_RTPROT_REDIRECT
#define IPSTACK_RTPROT_KERNEL    	IP_RTPROT_KERNEL
#define IPSTACK_RTPROT_BOOT    		IP_RTPROT_BOOT
#define IPSTACK_RTPROT_STATIC    	IP_RTPROT_STATIC
#define IPSTACK_RTPROT_GATED    	IP_RTPROT_GATED
#define IPSTACK_RTPROT_RA    		IP_RTPROT_RA
#define IPSTACK_RTPROT_MRT    		IP_RTPROT_MRT
#define IPSTACK_RTPROT_ZEBRA    	IP_RTPROT_ZEBRA
#define IPSTACK_RTPROT_BIRD    		IP_RTPROT_BIRD
#define IPSTACK_RTPROT_DNROUTED    	IP_RTPROT_DNROUTED
#define IPSTACK_RTPROT_XORP    		IP_RTPROT_XORP
#define IPSTACK_RTPROT_NTK    		IP_RTPROT_NTK


#define IPSTACK_RTA_ALIGN           IP_RTA_ALIGN
#define IPSTACK_RTA_LENGTH          IP_RTA_LENGTH
#define IPSTACK_RTA_OK              IP_RTA_OK
#define IPSTACK_RTA_NEXT            IP_RTA_NEXT
#define IPSTACK_RTA_PAYLOAD         IP_RTA_PAYLOAD
#define IPSTACK_RTA_DATA            IP_RTA_DATA
#define IPSTACK_RTA_UNSPEC   		IP_RTA_UNSPEC
#define IPSTACK_RTA_DST   			IP_RTA_DST
#define IPSTACK_RTA_SRC   			IP_RTA_SRC
#define IPSTACK_RTA_IIF   			IP_RTA_IIF
#define IPSTACK_RTA_OIF   			IP_RTA_OIF
#define IPSTACK_RTA_GATEWAY   		IP_RTA_GATEWAY
#define IPSTACK_RTA_PRIORITY   		IP_RTA_PRIORITY
#define IPSTACK_RTA_PREFSRC   		IP_RTA_PREFSRC
#define IPSTACK_RTA_METRICS   		IP_RTA_METRICS
#define IPSTACK_RTA_MULTIPATH   	IP_RTA_MULTIPATH
#define IPSTACK_RTA_PROTOINFO   	IP_RTA_PROTOINFO
#define IPSTACK_RTA_FLOW   			IP_RTA_FLOW
#define IPSTACK_RTA_CACHEINFO   	IP_RTA_CACHEINFO
#define IPSTACK_RTA_SESSION   		IP_RTA_SESSION
#define IPSTACK_RTA_MP_ALGO   		IP_RTA_MP_ALGO
#define IPSTACK_RTA_TABLE   		IP_RTA_TABLE
#define IPSTACK_RTA_NH_PROTO  		IP_RTA_NH_PROTO
#define IPSTACK_RTA_NH_PROTO_DATA   IP_RTA_NH_PROTO_DATA
#define IPSTACK_RTA_PROXY_ARP_LLADDR   IP_RTA_PROXY_ARP_LLADDR
#define IPSTACK_RTA_VR   			IP_RTA_VR
#define IPSTACK_RTA_TABLE_NAME   	IP_RTA_TABLE_NAME
#define IPSTACK_RTA_MAX    			IP_RTA_MAX

#define IPSTACK_RTAX_IFA                        IPNET_RTAX_IFA
#define IPSTACK_RTAX_DST                       	IPNET_RTAX_DST
#define IPSTACK_RTAX_GATEWAY              		IPNET_RTAX_GATEWAY
#define IPSTACK_RTAX_NETMASK               		IPNET_RTAX_NETMASK
#define IPSTACK_RTAX_MAX                       	IPNET_RTAX_MAX
#define IPSTACK_RTAX_MTU 						IP_RTAX_MTU


#define IPSTACK_IFLA_UNSPEC    		IP_IFLA_UNSPEC
#define IPSTACK_IFLA_ADDRESS    	IP_IFLA_ADDRESS
#define IPSTACK_IFLA_BROADCAST    	IP_IFLA_BROADCAST
#define IPSTACK_IFLA_IFNAME        	IP_IFLA_IFNAME
#define IPSTACK_IFLA_MTU       		IP_IFLA_MTU
#define IPSTACK_IFLA_LINK    		IP_IFLA_LINK
#define IPSTACK_IFLA_QDISC    		IP_IFLA_QDISC
#define IPSTACK_IFLA_STATS    		IP_IFLA_STATS
#define IPSTACK_IFLA_COST    		IP_IFLA_COST
#define IPSTACK_IFLA_PRIORITY    	IP_IFLA_PRIORITY
#define IPSTACK_IFLA_MASTER    		IP_IFLA_MASTER
#define IPSTACK_IFLA_WIRELESS    	IP_IFLA_WIRELESS
#define IPSTACK_IFLA_PROTINFO    	IP_IFLA_PROTINFO
#define IPSTACK_IFLA_TXQLEN    		IP_IFLA_TXQLEN
#define IPSTACK_IFLA_MAP    		IP_IFLA_MAP
#define IPSTACK_IFLA_WEIGHT    		IP_IFLA_WEIGHT
#define IPSTACK_IFLA_OPERSTATE    	IP_IFLA_OPERSTATE
#define IPSTACK_IFLA_LINKMODE    	IP_IFLA_LINKMODE
#define IPSTACK_IFLA_LINKINFO    	IP_IFLA_LINKINFO
#define IPSTACK_IFLA_MAX         	IP_IFLA_MAX
#define IPSTACK_IFLA_RTA         	IP_IFLA_RTA


#define IPSTACK_NLMSG_ALIGNTO  IP_NLMSG_ALIGNTO
#define IPSTACK_NLMSG_ALIGN    IP_NLMSG_ALIGN
#define IPSTACK_NLMSG_LENGTH   IP_NLMSG_LENGTH
#define IPSTACK_NLMSG_SPACE    IP_NLMSG_SPACE
#define IPSTACK_NLMSG_DATA     IP_NLMSG_DATA
#define IPSTACK_NLMSG_NEXT     IP_NLMSG_NEXT
#define IPSTACK_NLMSG_OK       IP_NLMSG_OK
#define IPSTACK_NLMSG_PAYLOAD  IP_NLMSG_PAYLOAD
#define IPSTACK_NLMSG_NOOP     IP_NLMSG_NOOP
#define IPSTACK_NLMSG_ERROR    IP_NLMSG_ERROR
#define IPSTACK_NLMSG_DONE     IP_NLMSG_DONE
#define IPSTACK_NLMSG_OVERRUN  IP_NLMSG_OVERRUN
#define IPSTACK_NLMSG_LENGTH   IP_NLMSG_LENGTH


#define IPSTACK_NLM_F_ROOT 			IP_NLM_F_ROOT
#define IPSTACK_NLM_F_MATCH 		IP_NLM_F_MATCH
#define IPSTACK_NLM_F_REQUEST 		IP_NLM_F_REQUEST
#define IPSTACK_NLM_F_CREATE 		IP_NLM_F_CREATE
#define IPSTACK_NLM_F_MULTI  		IP_NLM_F_MULTI
#define IPSTACK_NLM_F_ACK   		IP_NLM_F_ACK
#define IPSTACK_NLM_F_REPLACE   	IP_NLM_F_REPLACE


#define IPSTACK_IFA_MAX     		IP_IFA_MAX
#define IPSTACK_IFA_UNSPEC    		IP_IFA_UNSPEC
#define IPSTACK_IFA_ADDRESS    		IP_IFA_ADDRESS
#define IPSTACK_IFA_LOCAL    		IP_IFA_LOCAL
#define IPSTACK_IFA_LABEL    		IP_IFA_LABEL
#define IPSTACK_IFA_BROADCAST    	IP_IFA_BROADCAST
#define IPSTACK_IFA_ANYCAST    		IP_IFA_ANYCAST
#define IPSTACK_IFA_CACHEINFO    	IP_IFA_CACHEINFO
#define IPSTACK_IFA_MULTICAST    	IP_IFA_MULTICAST
#define IPSTACK_IFA_VR    			IP_IFA_VR
#define IPSTACK_IFA_TABLE    		IP_IFA_TABLE
#define IPSTACK_IFA_TABLE_NAME    	IP_IFA_TABLE_NAME
#define IPSTACK_IFA_X_INFO    		IP_IFA_X_INFO
#define IPSTACK_IFA_F_SECONDARY 	IP_IFA_F_SECONDARY
#define IPSTACK_IFA_RTA         	IP_IFA_RTA


#define IPSTACK_RTF_UP              IPNET_RTF_UP




#define IPSTACK_RT_SCOPE_UNIVERSE 		IP_RT_SCOPE_UNIVERSE
#define IPSTACK_RT_SCOPE_SITE 			IP_RT_SCOPE_SITE
#define IPSTACK_RT_SCOPE_LINK 			IP_RT_SCOPE_LINK
#define IPSTACK_RT_SCOPE_HOST 			IP_RT_SCOPE_HOST
#define IPSTACK_RT_SCOPE_NOWHERE 		IP_RT_SCOPE_NOWHERE

#define IPSTACK_IFT_ETHER                   IP_IFT_ETHER
#define IPSTACK_IFT_BRIDGE                  IP_IFT_BRIDGE
#define IPSTACK_IFT_IEEE8023ADLAG       	IP_IFT_IEEE8023ADLAG
#define IPSTACK_IFT_L2VLAN                  IP_IFT_L2VLAN

#define IPSTACK_RTV_VALUE1                   IPNET_RTV_VALUE1




#define IPSTACK_FD_ZERO IP_FD_ZERO 
#define IPSTACK_FD_CLR IP_FD_CLR
#define IPSTACK_FD_SET IP_FD_SET
#define IPSTACK_FD_ISSET IP_FD_ISSET

#define ipstack_fd_set Ip_fd_set




/*
 *===========================================================================
 *                   host <-> network endian
 *===========================================================================
 */
#ifndef htons
#define htons(x)          ip_htons(x)
#endif
#ifndef htonl
#define htonl(x)          ip_htonl(x)
#endif
#ifndef ntohs
#define ntohs(x)          ip_ntohs(x)
#endif
#ifndef ntohl
#define ntohl(x)          ip_ntohl(x)
#endif


/*
 *===========================================================================
 *                    IP types
 *===========================================================================
 */
/*
#define id		  id
#define len		  tot_len
#define off		  frag_off

#define ipstack_in_addr_t     Ip_in_addr_t
#define ipstack_sa_family_t   Ip_sa_family_t
#define ipstack_in_port_t     Ip_in_port_t
#define ipstack_socklen_t     Ip_socklen_t
*/
#define ipstack_in6_addr      Ip_in6_addr
#define ipstack_in6_addr_t    Ip_in6_addr_t
#define s6_addr       in6.addr8
#define ipstack_ethaddr    Ip_ethaddr

/*
 *===========================================================================
 *                    iovec
 *===========================================================================
 */
#define ipstack_iovec         Ip_iovec


/*
 *===========================================================================
 *                    in_addr
 *===========================================================================
 */
#ifdef s_addr
#undef s_addr
#endif

#define ipstack_in_addr       Ip_in_addr


/*
 *===========================================================================
 *                    sockaddr
 *===========================================================================
 */
#define ipstack_sockaddr_in     Ip_sockaddr_in
#define ipstack_sockaddr_in6    Ip_sockaddr_in6
#define s6_addr32		in6.addr32
#define ipstack_sockaddr_dl     Ip_sockaddr_dl
#define ipstack_sockaddr_ll     Ip_sockaddr_ll
#define ipstack_sockaddr        Ip_sockaddr


/*
 *===========================================================================
 *                         IPv6 Extension Headers
 *===========================================================================
 */
#define ipstack_pkt_ip6_ext_hdr        Ip_pkt_ip6_ext_hdr
#define ipstack_pkt_ip6_hbh            Ip_pkt_ip6_hbh
#define ipstack_pkt_ip6_dest           Ip_pkt_ip6_dest
#define ipstack_pkt_ip6_rthdr          Ip_pkt_ip6_rthdr
#define IPSTACK_IPV6_RTHDR_TYPE_0      IP_IPV6_RTHDR_TYPE_0
#define ipstack_pkt_ip6_rthdr0         Ip_pkt_ip6_rthdr0
#define ipstack_pkt_ip6_frag           Ip_pkt_ip6_frag

#define IPSTACK_IP6F_OFF_MASK          IP_IP6F_OFF_MASK
#define IPSTACK_IP6F_RESERVED_MASK     IP_IP6F_RESERVED_MASK
#define IPSTACK_IP6F_MORE_FRAG         IP_IP6F_MORE_FRAG

#define IPSTACK_IP6F_SIZE              IP_IP6F_SIZE
#define IPSTACK_IP6F_GET_OFFSET(hdr)   IP_IP6F_GET_OFFSET(hdr)


/*
 *===========================================================================
 *                      Extension Header Options
 *===========================================================================
 */
#define ipstack_pkt_ip6_opt              Ip_pkt_ip6_opt
#define IPSTACK_IP6OPT_TYPE(o)           IP_IP6OPT_TYPE(o)
#define IPSTACK_IP6OPT_TYPE_SKIP         IP_IP6OPT_TYPE_SKIP
#define IPSTACK_IP6OPT_TYPE_DISCARD      IP_IP6OPT_TYPE_DISCARD
#define IPSTACK_IP6OPT_TYPE_FORCEICMP    IP_IP6OPT_TYPE_FORCEICMP
#define IPSTACK_IP6OPT_TYPE_ICMP         IP_IP6OPT_TYPE_ICMP
#define IPSTACK_IP6OPT_MUTABLE           IP_IP6OPT_MUTABLE
#define IPSTACK_IP6OPT_PAD1              IP_IP6OPT_PAD1
#define IPSTACK_IP6OPT_PADN              IP_IP6OPT_PADN
#define IPSTACK_IP6OPT_JUMBO             IP_IP6OPT_JUMBO
#define IPSTACK_IP6OPT_NSAP_ADDR         IP_IP6OPT_NSAP_ADDR
#define IPSTACK_IP6OPT_TUNNEL_LIMIT      IP_IP6OPT_TUNNEL_LIMIT
#define IPSTACK_IP6OPT_ROUTER_ALERT      IP_IP6OPT_ROUTER_ALERT

#define ipstack_ip6_opt_router           Ip_ip6_opt_router
#define IPSTACK_IP6_ALERT_MLD            IP_IP6_ALERT_MLD
#define IPSTACK_IP6_ALERT_RSVP           IP_IP6_ALERT_RSVP
#define IPSTACK_IP6_ALERT_AN             IP_IP6_ALERT_AN


/*
 *===========================================================================
 *                    IPv6 icmp filter
 *===========================================================================
 */
#define IPSTACK_ICMP6_FILTER_WILLPASS(type)         IP_ICMP6_FILTER_WILLPASS(type)
#define IPSTACK_ICMP6_FILTER_WILLBLOCK(type)        IP_ICMP6_FILTER_WILLBLOCK(type)
#define IPSTACK_ICMP6_FILTER_SETPASS(type)          IP_ICMP6_FILTER_SETPASS(type)
#define IPSTACK_ICMP6_FILTER_SETBLOCK(type)         IP_ICMP6_FILTER_SETBLOCK(type)
#define IPSTACK_ICMP6_FILTER_SETPASSALL(filterp)    IP_ICMP6_FILTER_SETPASSALL(filterp)
#define IPSTACK_ICMP6_FILTER_SETBLOCKALL(filterp)   IP_ICMP6_FILTER_SETBLOCKALL(filterp)


/*
 *===========================================================================
 *                    msghdr
 *===========================================================================
 */
#define ipstack_msghdr        Ip_msghdr

/* msg_flags values */
#define IPSTACK_MSG_TRUNC     IP_MSG_TRUNC
#define IPSTACK_MSG_CTRUNC    IP_MSG_CTRUNC


/*
 *===========================================================================
 *                    cmsghdr
 *===========================================================================
 */
#define ipstack_cmsghdr        Ip_cmsghdr

#define IPSTACK_CMSG_ALIGN(len)          IP_CMSG_ALIGN(len)
#define IPSTACK_CMSG_FIRSTHDR(mhdrptr)   IP_CMSG_FIRSTHDR(mhdrptr)
#define IPSTACK_CMSG_NXTHDR(mhdr,cmsg)   IP_CMSG_NXTHDR(mhdr,cmsg)
#define IPSTACK_CMSG_DATA(cmsgptr)       IP_CMSG_DATA(cmsgptr)
#define IPSTACK_CMSG_SPACE(len)          IP_CMSG_SPACE(len)
#define IPSTACK_CMSG_LEN(len)            IP_CMSG_LEN(len)


/*
 *===========================================================================
 *                    if_nameindex
 *===========================================================================
 */
#ifdef ipstack_if_nameindex
#undef ipstack_if_nameindex
#endif
struct ipstack_if_nameindex {
    struct Ip_if_nameindex ni;
};

#define if_index ni.if_index
#define if_name  ni.if_name

/*
 *===========================================================================
 *                    ioctl structures
 *===========================================================================
 */
#define ipstack_ip_mreq             Ip_ip_mreq
#define ipstack_ip_mreqn            Ip_ip_mreqn

#define ipstack_arpreq              Ip_arpreq
#define ipstack_ortentry            Ip_ortentry
#define ipstack_aliasreq            Ip_ifaliasreq
#define ifra_broadaddr      ifra_dstaddr

#define ipstack_ip6_mreq            Ip_ipv6_mreq
#define ipstack_ipv6_mreq           Ip_ipv6_mreq
#define ipstack_in6_pktinfo         Ip_in6_pktinfo
#define ipstack_icmp6_filter        Ip_icmp6_filter
#define ipstack_in6_addrlifetime    Ip_in6_addrlifetime
#define ipstack_in6_aliasreq        Ip_in6_aliasreq


/*
 *===========================================================================
 *                        in_pktinfo
 *===========================================================================
 */
#define ipstack_in_pktinfo      Ip_in_pktinfo


/*
 *===========================================================================
 *                        hostent
 *===========================================================================
 */
#define ipstack_hostent         Ip_hostent

#define	IPSTACK_ARPHRD_ETHER             IP_ARPHRD_ETHER

/*
 *===========================================================================
 *                        ifreq
 *===========================================================================
 */
#define ipstack_ifreq           Ip_ifreq

#define ifr_addr        ifr_ifru.ifru_addr
#define ifr_dstaddr     ifr_ifru.ifru_dstaddr
#define ifr_broadaddr   ifr_ifru.ifru_broadaddr
#define ifr_flags       ifr_ifru.ifru_flags
#define ifr_metric      ifr_ifru.ifru_metric
#define ifr_mtu         ifr_ifru.ifru_mtu
#define ifr_data        ifr_ifru.ifru_data
#define ifr_rtab        ifr_ifru.ifru_rtab


/*
 *===========================================================================
 *                        in6_ifreq
 *===========================================================================
 */
#define ipstack_in6_ifreq       Ip_in6_ifreq

#define IPSTACK_IN6_IFF_TENTATIVE   IP_IN6_IFF_TENTATIVE


/*
 ****************************************************************************
 * 6                    FUNCTIONS
 ****************************************************************************
 */


#define	ipstack_sockethowmany(x)          ip_howmany(x)


/* functions used by both sockets & files. */

#define ipstack_fcntl(fd,request,argp)             ipcom_fcntl((int)fd,request,argp)


/*
 *===========================================================================
 *                        socket only functions
 *===========================================================================
 */

#define ipstack_inet_pton(family,strptr,addrptr)      ipcom_inet_pton(family,(const char *)strptr,addrptr)
#define ipstack_inet_ntop(family,addrptr,strptr,len)  ipcom_inet_ntop(family,(const void *)addrptr,strptr,(size_t)len)

#define ipstack_if_nametoindex(ifname)             ipcom_if_nametoindex((const char *)ifname)
#define ipstack_if_indextoname(ifindex,ifname)     ipcom_if_indextoname((int)ifindex, (char *)ifname)
#define ipstack_if_nameindex()                     ipcom_if_nameindex()
#define ipstack_if_freenameindex(pif)              ipcom_if_freenameindex(pif)

//#define ipstack_inet_aton ipcom_inet_aton
#define ipstack_inet_pton ipcom_inet_pton
//#define ipstack_inet_ntoa ipcom_inet_ntoa
//#define ipstack_inet_addr ipcom_inet_addr
#define ipstack_inet_ntoa(in)           ipcom_inet_ntoa(in)
#define ipstack_inet_addr(cp)           ipcom_inet_addr(cp)
#define ipstack_inet_aton(cp,addr)      ipcom_inet_aton(cp,addr)

#define ipstack_inet6_rth_space(t,s)               ipcom_inet6_rth_space(t,s)
#define ipstack_inet6_rth_init(b,l,t,s)            ipcom_inet6_rth_init(b,l,t,s)
#define ipstack_inet6_rth_add(b,a)                 ipcom_inet6_rth_add(b,a)
#define ipstack_inet6_rth_reverse(i,o)             ipcom_inet6_rth_reverse(i,o)
#define ipstack_inet6_rth_segments(b)              ipcom_inet6_rth_segments(b)
#define ipstack_inet6_rth_getaddr(b,i)             ipcom_inet6_rth_getaddr(b,i)

#define ipstack_inet6_opt_init(b,l)                ipcom_inet6_opt_init(b,l)
#define ipstack_inet6_opt_append(eb,el,o,t,l,a,db) ipcom_inet6_opt_append(eb,el,o,t,l,a,db)
#define ipstack_inet6_opt_finish(eb,el,o)          ipcom_inet6_opt_finish(eb,el,o)
#define ipstack_inet6_opt_set_val(db,o,v,l)        ipcom_inet6_opt_set_val(db,o,v,l)
#define ipstack_inet6_opt_next(eb,el,o,t,l,db)     ipcom_inet6_opt_next(eb,el,o,t,l,db)
#define ipstack_inet6_opt_find(eb,el,o,t,l,db)     ipcom_inet6_opt_find(eb,el,o,t,l,db)
#define ipstack_inet6_opt_get_val(db,o,v,l)        ipcom_inet6_opt_get_val(db,o,v,l)


/*
 ****************************************************************************
 * 7                    SOCK2
 ****************************************************************************
 */

#define ipstack_protoent   Ip_protoent
#define ipstack_servent    Ip_servent
#define ipstack_addrinfo   Ip_addrinfo
#define ipstack_group_req   Ip_group_req


/* port/src/ipcom_gethostby.c (use getipnodebyname/getipnodebyaddr for all new code!) */
#define ipstack_gethostbyname(name)     ipcom_gethostbyname(name)
#define ipstack_gethostbyname2(addrptr, family)    ipcom_gethostbyname2(addrptr, family)

#define ipstack_gethostbyaddr(a,l,t)    ipcom_gethostbyaddr(a,l,t)

/* src/ipcom_sock2.c */
#define ipstack_getprotobyname(name)    ipcom_getprotobyname(name)
#define ipstack_getprotobynumber(proto) ipcom_getprotobynumber(proto)

/* port/src/ipcom_getservby.c */
#define ipstack_getservbyname(n,p)      ipcom_getservbyname(n,p)
#define ipstack_getservbyport(p,p2)     ipcom_getservbyport(p,p2)


#define ipstack_getaddrinfo(family,addrptr,strptr,len)   ipcom_getaddrinfo(family,addrptr,strptr,len)
#define ipstack_freeaddrinfo(pif)            ipcom_freeaddrinfo(pif)
#define ipstack_getnameinfo              ipcom_getnameinfo



#define ipstack_nlmsghdr               	Ip_nlmsghdr
#define ipstack_ifaddrmsg 				Ip_ifaddrmsg
#define ipstack_rtmsg     				Ip_rtmsg
#define ipstack_rtgenmsg  				Ip_rtgenmsg
#define ipstack_sockaddr_nl   			Ip_sockaddr_nl
#define ipstack_nlmsgerr      			Ip_nlmsgerr
#define ipstack_rtattr        			Ip_rtattr
#define ipstack_ifinfomsg     			Ip_ifinfomsg
#define ipstack_ifa_cacheinfo 			Ip_ifa_cacheinfo
#define ipstack_rtnexthop      			Ip_rtnexthop

#define  ipstack_ifconf                 Ip_ifconf
#define  ifc_req                ip_ifc_req
#define  ipstack_iphdr                  Ipcom_iphdr
#define  ipstack_ip                     Ipcom_iphdr
#define  IPSTACK_LLADDR                 SOCKADDR_DL_LLADDR


#define ipstack_ip_mreqn           Ip_ip_mreqn


#define ipstack_ifaddrs   Ip_ifaddrs
#define ipstack_getifaddrs  ipcom_getifaddrs
#define ipstack_freeifaddrs ipcom_freeifaddrs
#define ipstack_if_data   Ipnet_if_data

#define   ipstack_utsname                       Ip_utsname
#define    ipstack_uname                          ipcom_uname


/*
 *===========================================================================
 *                    timeval
 *===========================================================================
 */
#define ipstack_timeval       Ip_timeval


#define ipstack_strerror   ipcom_strerror




/*
 *===========================================================================
 *                         ERRNO
 *===========================================================================
 */
#define IPSTACK_EDESTADDRREQ       IP_ERRNO_EDESTADDRREQ
#define IPSTACK_ENETUNREACH        IP_ERRNO_ENETUNREACH
#define IPSTACK_ENETRESET          IP_ERRNO_ENETRESET
#define IPSTACK_ECONNABORTED       IP_ERRNO_ECONNABORTED
#define IPSTACK_ECONNRESET         IP_ERRNO_ECONNRESET
#define IPSTACK_ENOBUFS            IP_ERRNO_ENOBUFS
#define IPSTACK_EISCONN            IP_ERRNO_EISCONN
#define IPSTACK_ENOTCONN           IP_ERRNO_ENOTCONN
#define IPSTACK_ESHUTDOWN          IP_ERRNO_ESHUTDOWN
#define IPSTACK_ETOOMANYREFS       IP_ERRNO_ETOOMANYREFS
#define IPSTACK_ECONNREFUSED       IP_ERRNO_ECONNREFUSED
#define IPSTACK_ENETDOWN           IP_ERRNO_ENETDOWN
#define IPSTACK_EHOSTUNREACH       IP_ERRNO_EHOSTUNREACH
#define IPSTACK_EINPROGRESS        IP_ERRNO_EINPROGRESS
#define IPSTACK_EALREADY           IP_ERRNO_EALREADY
#define IPSTACK_EINVAL             IP_ERRNO_EINVAL
#define IPSTACK_EHOSTDOWN          IP_ERRNO_EHOSTDOWN
#define IPSTACK_ETIMEDOUT          IP_ERRNO_ETIMEDOUT
#define IPSTACK_ETIME              IP_ERRNO_ETIMEDOUT
#define IPSTACK_EADDRINUSE         IP_ERRNO_EADDRINUSE
#define IPSTACK_EOPNOTSUPP         IP_ERRNO_EOPNOTSUPP

#define	IPSTACK_EIO                            IP_ERRNO_EIO
#define IPSTACK_E2BIG                          IP_ERRNO_E2BIG
#define	IPSTACK_ENOENT                         IP_ERRNO_ENOENT

#define IPSTACK_EINTR       IP_ERRNO_EINTR
#define IPSTACK_EWOULDBLOCK IP_ERRNO_EWOULDBLOCK
#define IPSTACK_EAGAIN      IP_ERRNO_EAGAIN
#define IPSTACK_ENODEV      IP_ERRNO_ENODEV
#define IPSTACK_ESRCH       IP_ERRNO_ESRCH
#define IPSTACK_EEXIST      IP_ERRNO_EEXIST

#define  IPSTACK_ENOMEM     	IP_ERRNO_ENOMEM
#define  IPSTACK_EBUSY        	IP_ERRNO_EBUSY
#define  IPSTACK_EFAULT       	IP_ERRNO_EFAULT


#ifdef __cplusplus
}
#endif

#endif /* __IPCOM_STACK_H__ */
