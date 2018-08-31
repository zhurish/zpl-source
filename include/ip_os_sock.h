/*
 * ip_os_sock.h
 *
 *  Created on: Jun 19, 2018
 *      Author: zhurish
 */

#ifndef INCLUDE_IP_OS_SOCK_H_
#define INCLUDE_IP_OS_SOCK_H_

/*
 ****************************************************************************
 * 1                    DESCRIPTION
 ****************************************************************************
 * BSD socket to Interpeak IPCOM sockets wrapper header file.
 */

/*
 ****************************************************************************
 * 2                    CONFIGURATION
 ***********x*****************************************************************
 */


/*
 ****************************************************************************
 * 3                    INCLUDE FILES
 ****************************************************************************
 */


#ifdef __cplusplus
extern "C" {
#endif

/*
 ****************************************************************************
 * 4                    DEFINES
 ****************************************************************************
 */


/*
 *===========================================================================
 *                         misc
 *===========================================================================
 */
//#define IFNAMSIZ      IFNAMSIZ
#define	IPVERSION	4
/*
 *===========================================================================
 *                         domain
 *===========================================================================
 */
/*
#define AF_UNSPEC     IP_AF_UNSPEC
#define AF_INET       IP_AF_INET
#define AF_NETLINK    IP_AF_NETLINK
#define AF_ROUTE      IP_AF_ROUTE
#define AF_LINK       IP_AF_LINK
#define AF_PACKET     IP_AF_PACKET
#define AF_INET6      IP_AF_INET6
#define AF_KEY        IP_AF_KEY
#define AF_MAX        IP_AF_MAX

#define PF_INET       IP_PF_INET
#define PF_NETLINK    IP_PF_NETLINK
#define PF_ROUTE      IP_PF_ROUTE
#define PF_LINK       IP_PF_LINK
#define PF_PACKET     IP_PF_PACKET
#define PF_INET6      IP_PF_INET6
#define PF_KEY        IP_PF_KEY

*/

/*
 *===========================================================================
 *                         Socket types
 *===========================================================================
 */
/*
#define SOCK_STREAM      IP_SOCK_STREAM
#define SOCK_DGRAM       IP_SOCK_DGRAM
#define SOCK_RAW         IP_SOCK_RAW
#define SOCK_PACKET      IP_SOCK_PACKET
*/


/*
 *===========================================================================
 *                         IP protocol values
 *===========================================================================
 */
/*
#define IPPROTO_IP          IP_IPPROTO_IP
#define IPPROTO_ICMP        IP_IPPROTO_ICMP
#define IPPROTO_IGMP        IP_IPPROTO_IGMP
#define IPPROTO_IPIP        IP_IPPROTO_IPIP
#define IPPROTO_IPV4        IP_IPPROTO_IPV4
#define IPPROTO_TCP         IP_IPPROTO_TCP
#define IPPROTO_UDP         IP_IPPROTO_UDP
#define IPPROTO_IPV6        IP_IPPROTO_IPV6
#define IPPROTO_RSVP        IP_IPPROTO_RSVP
#define IPPROTO_GRE         IP_IPPROTO_GRE
#define IPPROTO_ESP         IP_IPPROTO_ESP
#define IPPROTO_AH          IP_IPPROTO_AH
#define IPPROTO_ICMPV6      IP_IPPROTO_ICMPV6
#define IPPROTO_OSPFIGP     IP_IPPROTO_OSPFIGP
#define IPPROTO_PIM         IP_IPPROTO_PIM
#define IPPROTO_RAW         IP_IPPROTO_RAW
#define IPPROTO_MAX         IP_IPPROTO_MAX
*/

/* IPv6 Non-IP Protocol Next Header field values */
/*
#define IPPROTO_HOPOPTS     IP_IPPROTO_HOPOPTS
#define IPPROTO_ROUTING     IP_IPPROTO_ROUTING
#define IPPROTO_FRAGMENT    IP_IPPROTO_FRAGMENT
#define IPPROTO_NONE        IP_IPPROTO_NONE
#define IPPROTO_DSTOPTS     IP_IPPROTO_DSTOPTS
*/


/*
 *===========================================================================
 *                  Standard IP address macros.
 *===========================================================================
 */
/*
#define INADDR_ANY             IP_INADDR_ANY
#define INADDR_DEFAULT         IP_INADDR_DEFAULT
#define INADDR_LOOPBACK        IP_INADDR_LOOPBACK
#define INADDR_BROADCAST       IP_INADDR_BROADCAST
#define	INADDR_NONE            IP_INADDR_NONE

#define INADDR_UNSPEC_GROUP    IP_INADDR_UNSPEC_GROUP
#define INADDR_ALLHOSTS_GROUP  IP_INADDR_ALLHOSTS_GROUP
*/


/*
 *===========================================================================
 *                   host <-> network endian
 *===========================================================================
 */
/*
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
*/


/*
 *===========================================================================
 *                         NETMASK
 *===========================================================================
 */
/*
#define NETMASK(xxip)          IP_NETMASK(xxip)
#define NETBITS(xxip)          IP_NETBITS(xxip)

#define	IN_CLASSA(xxip)	       IP_IN_CLASSA(xxip)
#define	IN_CLASSB(xxip)        IP_IN_CLASSB(xxip)
#define	IN_CLASSC(xxip)        IP_IN_CLASSC(xxip)
#define	IN_CLASSD(xxip)        IP_IN_CLASSD(xxip)
#define	IN_EXPERIMENTAL(xxip)  IP_IN_EXPERIMENTAL(xxip)
#define INET_ADDRSTRLEN        IP_INET_ADDRSTRLEN
#define IN_MULTICAST(xxip)     IN_CLASSD(xxip)
*/
/*
 *===========================================================================
 *                      IPv6 address macros
 *===========================================================================
 */

/*
#define IN6_ARE_ADDR_EQUAL(addr1)            IP_IN6_ARE_ADDR_EQUAL(addr1)
#define IN6_IS_ADDR_UNSPECIFIED(addr)        IP_IN6_IS_ADDR_UNSPECIFIED(addr)
#define IN6_IS_ADDR_LOOPBACK(addr)           IP_IN6_IS_ADDR_LOOPBACK(addr)
#define IN6_IS_ADDR_V4COMPAT(addr)           IP_IN6_IS_ADDR_V4COMPAT(addr)
#define IN6_IS_ADDR_V4MAPPED(addr)           IP_IN6_IS_ADDR_V4MAPPED(addr)
#define IN6_IS_ADDR_AGGR_GLOB_UNICAST(addr)  IP_IN6_IS_ADDR_AGGR_GLOB_UNICAST(addr)
#define IN6_IS_ADDR_SITE_LOCAL(addr)         IP_IN6_IS_ADDR_SITE_LOCAL(addr)
#define IN6_IS_ADDR_LINK_LOCAL(addr)         IP_IN6_IS_ADDR_LINK_LOCAL(addr)
#define IN6_IS_ADDR_MULTICAST(addr)          IP_IN6_IS_ADDR_MULTICAST(addr)
#define IN6_IS_ADDR_LINKLOCAL(addr)          IP_IN6_IS_ADDR_LINK_LOCAL(addr)
*/

/*
 *===========================================================================
 *                         setsockopt/getsockopt
 *===========================================================================
 */
#if 0
#define SOL_SOCKET         IP_SOL_SOCKET

/***** IP_SOL_SOCKET socket options: *****/
#define SO_REUSEADDR       IP_SO_REUSEADDR
#define SO_KEEPALIVE       IP_SO_KEEPALIVE
#define SO_DONTROUTE       IP_SO_DONTROUTE
#define SO_BROADCAST       IP_SO_BROADCAST
#define SO_REUSEPORT       IP_SO_REUSEPORT
#define SO_SNDBUF          IP_SO_SNDBUF
#define SO_RCVBUF          IP_SO_RCVBUF
#define SO_RCVTIMEO        IP_SO_RCVTIMEO
#define SO_ERROR           IP_SO_ERROR
#define SO_TYPE            IP_SO_TYPE
#define SO_BINDTODEVICE    IP_SO_BINDTODEVICE

/***** IP_IPPROTO_IP level socket options: *****/
#define IP_HDRINCL         IP_IP_HDRINCL
#define IP_TOS             IP_IP_TOS
#define IP_TTL             IP_IP_TTL
#define IP_PKTINFO         IP_IP_PKTINFO
#define IP_MULTICAST_IF    IP_IP_MULTICAST_IF
#define IP_MULTICAST_TTL   IP_IP_MULTICAST_TTL
#define IP_MULTICAST_LOOP  IP_IP_MULTICAST_LOOP
#define IP_ADD_MEMBERSHIP  IP_IP_ADD_MEMBERSHIP
#define	IP_JOIN_GROUP      IP_IP_JOIN_GROUP
#define IP_DROP_MEMBERSHIP IP_IP_DROP_MEMBERSHIP
#define	IP_LEAVE_GROUP     IP_IP_LEAVE_GROUP
#define IP_RECVIF          IP_IP_RECVIF
#define IP_ROUTER_ALERT    IP_IP_ROUTER_ALERT
#define IP_DONTFRAG        IP_IP_DONTFRAG

#define	MCAST_JOIN_GROUP	IP_JOIN_GROUP
#define	MCAST_LEAVE_GROUP	IP_LEAVE_GROUP


/***** IP_IPPROTO_IPV6 level socket options: *****/
#define IPV6_UNICAST_HOPS     IP_IPV6_UNICAST_HOPS
#define IPV6_MULTICAST_IF     IP_IPV6_MULTICAST_IF
#define IPV6_MULTICAST_HOPS   IP_IPV6_MULTICAST_HOPS
#define IPV6_MULTICAST_LOOP   IP_IPV6_MULTICAST_LOOP
#define	IPV6_JOIN_GROUP       IP_IPV6_JOIN_GROUP
#define IPV6_ADD_MEMBERSHIP   IP_IPV6_ADD_MEMBERSHIP
#define	IPV6_LEAVE_GROUP      IP_IPV6_LEAVE_GROUP
#define IPV6_DROP_MEMBERSHIP  IP_IPV6_DROP_MEMBERSHIP
#define IPV6_PKTINFO          IP_IPV6_PKTINFO
#define IPV6_TCLASS           IP_IPV6_TCLASS
#define IPV6_NEXTHOP          IP_IPV6_NEXTHOP
#define IPV6_RTHDR            IP_IPV6_RTHDR
#define IPV6_HOPOPTS          IP_IPV6_HOPOPTS
#define IPV6_DSTOPTS          IP_IPV6_DSTOPTS
#define IPV6_RTHDRDSTOPTS     IP_IPV6_RTHDRDSTOPTS
#define IPV6_RECVPKTINFO      IP_IPV6_RECVPKTINFO
#define IPV6_RECVHOPLIMIT     IP_IPV6_RECVHOPLIMIT
#define IPV6_RECVTCLASS       IP_IPV6_RECVTCLASS
#define IPV6_RECVRTHDR        IP_IPV6_RECVRTHDR
#define IPV6_RECVHOPOPTS      IP_IPV6_RECVHOPOPTS
#define IPV6_RECVDSTOPTS      IP_IPV6_RECVDSTOPTS
#define IPV6_CHECKSUM         IP_IPV6_CHECKSUM
#define IPV6_RECVIF           IP_IPV6_RECVIF

/***** IP_IPPROTO_ICMPV6 level socket options: *****/
#define ICMP6_FILTER          IP_ICMP6_FILTER

/***** IP_IPPROTO_TCP level socket options: *****/
#define TCP_NODELAY           IP_TCP_NODELAY

/* NOTE: This option can NOT be used with getsockopt/setsockopt!
 * Only sent/rcvd with ancillary data!
 */
#define IPV6_HOPLIMIT         IP_IPV6_HOPLIMIT

//#define	ARPHRD_ETHER          IP_ARPHRD_ETHER


#define MSG_OOB			IP_MSG_OOB                    /* Specify to send/receive out-of-band data */
#define MSG_PEEK		IP_MSG_PEEK                   /* Leave the data on the receive buffer (receive only) */
#define MSG_DONTROUTE	IP_MSG_DONTROUTE              /* Bypass normal routing, the packet will be sent
                                            on the interface that matches the network part
                                            of the destination address */
#define MSG_WAITALL		IP_MSG_WAITALL               /* Wait for full request or error (receive only) */
#define MSG_DONTWAIT	IP_MSG_DONTWAIT              /* Enables non-blocking operation */

#define MSG_NOTIFICATION	IP_MSG_NOTIFICATION    /* SCTP Notification */
#define MSG_EOR				IP_MSG_EOR

#define MSG_MORE		IP_MSG_MORE                /* Disable Nagle algorithm (send only) */
#ifdef IPCOM_ZEROCOPY
#define MSG_ZBUF		IP_MSG_ZBUF               /* zbuf */
#endif
#endif
/*
 *===========================================================================
 *                         ioctl
 *===========================================================================
 */
#if 0
/* socket ioctl */
#define SIOCSPGRP          IP_SIOCSPGRP
#define SIOCGPGRP          IP_SIOCGPGRP
#define FIONBIO            IP_FIONBIO

/* AF_INET ioctl. */
#define SIOCGIFADDR        IP_SIOCGIFADDR
#define SIOCSIFADDR        IP_SIOCSIFADDR
#define SIOCGIFBRDADDR     IP_SIOCGIFBRDADDR
#define SIOCSIFBRDADDR     IP_SIOCSIFBRDADDR
#define SIOCGIFNETMASK     IP_SIOCGIFNETMASK
#define SIOCSIFNETMASK     IP_SIOCSIFNETMASK
#define SIOCGIFDSTADDR     IP_SIOCGIFDSTADDR
#define SIOCSIFDSTADDR     IP_SIOCSIFDSTADDR
#define SIOCAIFADDR        IP_SIOCAIFADDR
#define SIOCDIFADDR        IP_SIOCDIFADDR
#define SIOCADDRT          IP_SIOCADDRT
#define SIOCDDDRT          IP_SIOCDDDRT

/* AF_INET6 ioctl. */
#define SIOCAIFADDR_IN6    IP_SIOCAIFADDR_IN6
#define SIOCDIFADDR_IN6    IP_SIOCDIFADDR_IN6
#define SIOCGIFDSTADDR_IN6 IP_SIOCGIFDSTADDR_IN6

/* interface ioctl. */
#define SIOCGIFFLAGS       IP_SIOCGIFFLAGS
#define SIOCSIFFLAGS       IP_SIOCSIFFLAGS
#define SIOCGIFMTU         IP_SIOCGIFMTU
#define SIOCSIFMTU         IP_SIOCSIFMTU
#define SIOCGIFMETRIC      IP_SIOCGIFMETRIC
#define SIOCSIFMETRIC      IP_SIOCSIFMETRIC
#define SIOCSIFRTAB        IP_SIOCSIFRTAB
#define SIOCGIFRTAB        IP_SIOCGIFRTAB
#define SIOCGETTUNNEL      IP_SIOCGETTUNNEL
#define SIOCADDTUNNEL      IP_SIOCADDTUNNEL
#define SIOCCHGTUNNEL      IP_SIOCCHGTUNNEL
#define SIOCDELTUNNEL      IP_SIOCDELTUNNEL

/* arp ioctl. */
#define SIOCSARP           IP_SIOCSARP
#define SIOCGARP           IP_SIOCGARP
#define SIOCDARP           IP_SIOCDARP

#define  SIOCGIFHWADDR          IP_SIOCGIFHWADDR
#define  SIOCGIFCONF            IP_SIOCGIFCONF
#define	 SIOCGIFTXQLEN          IP_SIOCGIFTXQLEN


/*
 *===========================================================================
 *                         ERRNO
 *===========================================================================
 */
#define EDESTADDRREQ       IP_ERRNO_EDESTADDRREQ
#define ENETUNREACH        IP_ERRNO_ENETUNREACH
#define ENETRESET          IP_ERRNO_ENETRESET
#define ECONNABORTED       IP_ERRNO_ECONNABORTED
#define ECONNRESET         IP_ERRNO_ECONNRESET
#define ENOBUFS            IP_ERRNO_ENOBUFS
#define EISCONN            IP_ERRNO_EISCONN
#define ENOTCONN           IP_ERRNO_ENOTCONN
#define ESHUTDOWN          IP_ERRNO_ESHUTDOWN
#define ETOOMANYREFS       IP_ERRNO_ETOOMANYREFS
#define ECONNREFUSED       IP_ERRNO_ECONNREFUSED
#define ENETDOWN           IP_ERRNO_ENETDOWN
#define EHOSTUNREACH       IP_ERRNO_EHOSTUNREACH
#define EINPROGRESS        IP_ERRNO_EINPROGRESS
#define EALREADY           IP_ERRNO_EALREADY
#define EINVAL             IP_ERRNO_EINVAL
#define EHOSTDOWN          IP_ERRNO_EHOSTDOWN
#define ETIMEDOUT          IP_ERRNO_ETIMEDOUT
#define ETIME              IP_ERRNO_ETIMEDOUT
#define EADDRINUSE         IP_ERRNO_EADDRINUSE
#define EOPNOTSUPP         IP_ERRNO_EOPNOTSUPP

#define	EIO                            IP_ERRNO_EIO
#define E2BIG                          IP_ERRNO_E2BIG
#define	ENOENT                         IP_ERRNO_ENOENT

#define EINTR       IP_ERRNO_EINTR
#define EWOULDBLOCK IP_ERRNO_EWOULDBLOCK
#define EAGAIN      IP_ERRNO_EAGAIN
#define ENODEV      IP_ERRNO_ENODEV
#define ESRCH       IP_ERRNO_ESRCH
#define EEXIST      IP_ERRNO_EEXIST

#define  ENOMEM     	IP_ERRNO_ENOMEM
#define  EBUSY        	IP_ERRNO_EBUSY
#define  EFAULT       	IP_ERRNO_EFAULT

/*
 *===========================================================================
 *                         IFF_X
 *===========================================================================
 */
#define IFF_UP          IP_IFF_UP
#define IFF_BROADCAST   IP_IFF_BROADCAST
#define IFF_DEBUG       IP_IFF_DEBUG
#define IFF_LOOPBACK    IP_IFF_LOOPBACK
#define IFF_POINTOPOINT IP_IFF_POINTOPOINT
#define IFF_RUNNING     IP_IFF_RUNNING
#define IFF_NOARP       IP_IFF_NOARP
#define IFF_PROMISC     IP_IFF_PROMISC
#define IFF_ALLMULTI    IP_IFF_ALLMULTI
#define IFF_OACTIVE     IP_IFF_OACTIVE
#define IFF_SIMPLEX     IP_IFF_SIMPLEX
#define IFF_LINK0       IP_IFF_LINK0
#define IFF_LINK1       IP_IFF_LINK1
#define IFF_LINK2       IP_IFF_LINK2
#define IFF_MULTICAST   IP_IFF_MULTICAST


/*
 *===========================================================================
 *                    recvxx() argument flags
 *===========================================================================
 */
#define MSG_PEEK           IP_MSG_PEEK


/*
 *===========================================================================
 *                    shutdown() argument flags
 *===========================================================================
 */
#define SHUT_RD            IP_SHUT_RD
#define SHUT_WR            IP_SHUT_WR
#define SHUT_RDWR          IP_SHUT_RDWR


/*
 ****************************************************************************
 * 5                    TYPES
 ****************************************************************************
 */

/*
 *===========================================================================
 *                    IP types
 *===========================================================================
 */
#define ip            Ipcom_iphdr
#define ip_id		  id
#define ip_len		  tot_len
#define ip_off		  frag_off


#define in_addr_t     Ip_in_addr_t
#define sa_family_t   Ip_sa_family_t
#define in_port_t     Ip_in_port_t
#define socklen_t     socklen_t

#define in6_addr      Ip_in6_addr
#define in6_addr_t    Ip_in6_addr_t
#define s6_addr       in6.addr8


/*
 *===========================================================================
 *                    iovec
 *===========================================================================
 */
#define iovec         Ip_iovec


/*
 *===========================================================================
 *                    in_addr
 *===========================================================================
 */
#ifdef s_addr
#undef s_addr
#endif

#define in_addr       Ip_in_addr


/*
 *===========================================================================
 *                    sockaddr
 *===========================================================================
 */
#define sockaddr_in     sockaddr_in
#define sockaddr_in6    sockaddr_in6
#define s6_addr32		in6.addr32
#define sockaddr_dl     sockaddr_dl
#define sockaddr_ll     sockaddr_ll
#define sockaddr        sockaddr

#define SOCKADDR_DL_LLADDR(sa)   IP_SOCKADDR_DL_LLADDR(sa)

/* Packet types 'sll_pkttype'.    Packet is addressed to: */
#define PACKET_HOST          IP_PACKET_HOST
#define PACKET_BROADCAST     IP_PACKET_BROADCAST
#define PACKET_MULTICAST     IP_PACKET_MULTICAST
#define PACKET_OTHERHOST     IP_PACKET_OTHERHOST
#define PACKET_OUTGOING      IP_PACKET_OUTGOING


/*
 *===========================================================================
 *                         IPv6 Extension Headers
 *===========================================================================
 */
#define pkt_ip6_ext_hdr        Ip_pkt_ip6_ext_hdr
#define pkt_ip6_hbh            Ip_pkt_ip6_hbh
#define pkt_ip6_dest           Ip_pkt_ip6_dest
#define pkt_ip6_rthdr          Ip_pkt_ip6_rthdr
#define IPV6_RTHDR_TYPE_0      IP_IPV6_RTHDR_TYPE_0
#define pkt_ip6_rthdr0         Ip_pkt_ip6_rthdr0
#define pkt_ip6_frag           Ip_pkt_ip6_frag

#define IP6F_OFF_MASK          IP_IP6F_OFF_MASK
#define IP6F_RESERVED_MASK     IP_IP6F_RESERVED_MASK
#define IP6F_MORE_FRAG         IP_IP6F_MORE_FRAG

#define IP6F_SIZE              IP_IP6F_SIZE
#define IP6F_GET_OFFSET(hdr)   IP_IP6F_GET_OFFSET(hdr)


/*
 *===========================================================================
 *                      Extension Header Options
 *===========================================================================
 */
#define pkt_ip6_opt              Ip_pkt_ip6_opt
#define IP6OPT_TYPE(o)           IP_IP6OPT_TYPE(o)
#define IP6OPT_TYPE_SKIP         IP_IP6OPT_TYPE_SKIP
#define IP6OPT_TYPE_DISCARD      IP_IP6OPT_TYPE_DISCARD
#define IP6OPT_TYPE_FORCEICMP    IP_IP6OPT_TYPE_FORCEICMP
#define IP6OPT_TYPE_ICMP         IP_IP6OPT_TYPE_ICMP
#define IP6OPT_MUTABLE           IP_IP6OPT_MUTABLE
#define IP6OPT_PAD1              IP_IP6OPT_PAD1
#define IP6OPT_PADN              IP_IP6OPT_PADN
#define IP6OPT_JUMBO             IP_IP6OPT_JUMBO
#define IP6OPT_NSAP_ADDR         IP_IP6OPT_NSAP_ADDR
#define IP6OPT_TUNNEL_LIMIT      IP_IP6OPT_TUNNEL_LIMIT
#define IP6OPT_ROUTER_ALERT      IP_IP6OPT_ROUTER_ALERT

#define ip6_opt_router           Ip_ip6_opt_router
#define IP6_ALERT_MLD            IP_IP6_ALERT_MLD
#define IP6_ALERT_RSVP           IP_IP6_ALERT_RSVP
#define IP6_ALERT_AN             IP_IP6_ALERT_AN


/*
 *===========================================================================
 *                    IPv6 icmp filter
 *===========================================================================
 */
#define ICMP6_FILTER_WILLPASS(type)         IP_ICMP6_FILTER_WILLPASS(type)
#define ICMP6_FILTER_WILLBLOCK(type)        IP_ICMP6_FILTER_WILLBLOCK(type)
#define ICMP6_FILTER_SETPASS(type)          IP_ICMP6_FILTER_SETPASS(type)
#define ICMP6_FILTER_SETBLOCK(type)         IP_ICMP6_FILTER_SETBLOCK(type)
#define ICMP6_FILTER_SETPASSALL(filterp)    IP_ICMP6_FILTER_SETPASSALL(filterp)
#define ICMP6_FILTER_SETBLOCKALL(filterp)   IP_ICMP6_FILTER_SETBLOCKALL(filterp)


/*
 *===========================================================================
 *                    msghdr
 *===========================================================================
 */
#define msghdr        Ip_msghdr

/* msg_flags values */
#define MSG_TRUNC     IP_MSG_TRUNC
#define MSG_CTRUNC    IP_MSG_CTRUNC


/*
 *===========================================================================
 *                    cmsghdr
 *===========================================================================
 */
#define cmsghdr        Ip_cmsghdr

#define CMSG_ALIGN(len)          IP_CMSG_ALIGN(len)
#define CMSG_FIRSTHDR(mhdrptr)   IP_CMSG_FIRSTHDR(mhdrptr)
#define CMSG_NXTHDR(mhdr,cmsg)   IP_CMSG_NXTHDR(mhdr,cmsg)
#define CMSG_DATA(cmsgptr)       IP_CMSG_DATA(cmsgptr)
#define CMSG_SPACE(len)          IP_CMSG_SPACE(len)
#define CMSG_LEN(len)            IP_CMSG_LEN(len)


/*
 *===========================================================================
 *                    if_nameindex
 *===========================================================================
 */
#ifdef if_nameindex
#undef if_nameindex
#endif
struct if_nameindex {
    struct Ip_if_nameindex ni;
};

#define if_index ni.if_index
#define if_name  ni.if_name

/*
 *===========================================================================
 *                    ioctl structures
 *===========================================================================
 */
#define ip_mreq             Ip_ip_mreq
#define ip_mreqn            Ip_ip_mreqn

#define arpreq              Ip_arpreq
#define ortentry            Ip_ortentry
#define aliasreq            Ip_ifaliasreq
#define ifra_broadaddr      ifra_dstaddr

#define ip6_mreq            Ip_ipv6_mreq
#define ipv6_mreq           Ip_ipv6_mreq
#define in6_pktinfo         Ip_in6_pktinfo
#define icmp6_filter        Ip_icmp6_filter
#define in6_addrlifetime    Ip_in6_addrlifetime
#define in6_aliasreq        Ip_in6_aliasreq


/*
 *===========================================================================
 *                        in_pktinfo
 *===========================================================================
 */
#define in_pktinfo      Ip_in_pktinfo


/*
 *===========================================================================
 *                        hostent
 *===========================================================================
 */
#define hostent         Ip_hostent

#define	ARPHRD_ETHER             IP_ARPHRD_ETHER

/*
 *===========================================================================
 *                        ifreq
 *===========================================================================
 */
#define ifreq           Ip_ifreq

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
#define in6_ifreq       Ip_in6_ifreq

#define IN6_IFF_TENTATIVE   IP_IN6_IFF_TENTATIVE

#endif
/*
 ****************************************************************************
 * 6                    FUNCTIONS
 ****************************************************************************
 */

/*
 *===========================================================================
 *             socket & file functions/macros - SEPARATE FLIB
 *===========================================================================
 * Define IPCOM_USE_SEPARATE_FLIB if you use sockets and file IO with
 * system call conflicts in the same file.
 */
#ifdef IPCOM_USE_SEPARATE_FLIB
/* functions used by both sockets & files. */
#define socketioctl(fd,request,argp)       ipcom_socketioctl((int)fd,request,argp)
#define socketclose(fd)                    ipcom_socketclose((int)fd)
#define socketwrite(fd,buf,nbytes)         ipcom_socketwrite((int)fd,(const void *)buf,(size_t)nbytes)
#define socketwritev(fd,iov,iovlen)        ipcom_socketwritev((int)fd,iov,iovlen)
#define socketread(fd,buf,nbytes)          ipcom_socketread((int)fd,buf,(size_t)nbytes)
#define socketselect(nfds,rf,wf,ef,t)      ipcom_socketselect(nfds,rf,wf,ef,t)
/* errno */
#define socketerrno                        ipcom_errno
/* select stuff */
#define SOCKETFD_SETSIZE          IP_FD_SETSIZE
#define socketfd_set              int_set
#define socketFD_ZERO(pfdset)     IP_FD_ZERO(pfdset)
#define socketFD_CLR(fd,pfdset)   IP_FD_CLR(fd,pfdset)
#define socketFD_SET(fd,pfdset)   IP_FD_SET(fd,pfdset)
#define socketFD_ISSET(fd,pfdset) IP_FD_ISSET(fd,pfdset)
#define	sockethowmany(x)          ip_howmany(x)

/*
 *===========================================================================
 *             socket & file functions/macros
 *===========================================================================
 */
#else
/* functions used by both sockets & files. */
#define ip_ioctl(fd,request,argp)             ioctl((int)fd,request,argp)
#define ip_close(fd)                          close((int)fd)
#define ip_write(fd,buf,nbytes)               write((int)fd,(const void *)buf,(size_t)nbytes)
#define ip_writev(fd,iov,iovlen)              writev((int)fd,iov,iovlen)
#define ip_read(fd,buf,nbytes)                read((int)fd,buf,(size_t)nbytes)
#define ip_select(nfds,rf,wf,ef,t)            select(nfds,rf,wf,NULL,t)

/* select stuff */
/*
#undef FD_SETSIZE
#define FD_SETSIZE          FD_SETSIZE
#undef fd_set
#define fd_set              fd_set
#undef FD_ZERO
#define FD_ZERO(pfdset)     FD_ZERO(pfdset)
#undef FD_CLR
#define FD_CLR(fd,pfdset)   FD_CLR(fd,pfdset)
#undef FD_SET
#define FD_SET(fd,pfdset)   FD_SET(fd,pfdset)
#undef FD_ISSET
#define FD_ISSET(fd,pfdset) FD_ISSET(fd,pfdset)
#undef howmany
#define	howmany(x)          howmany(x)
*/
#endif /* IPCOM_USE_SHARED_FLIB */


/*
 *===========================================================================
 *                        socket only functions
 *===========================================================================
 */

#define ip_socket(domain,type,protocol)       socket(domain,type,protocol)
#define ip_shutdown(fd,how)                   shutdown((int)fd,how)
#define ip_bind(fd,addr,addrlen)              bind((int)fd,(const struct sockaddr *)addr,(socklen_t)addrlen)
#define ip_connect(fd,addr,addrlen)           connect((int)fd,(const struct sockaddr *)addr,(socklen_t)addrlen)
#define ip_accept(fd,addr,addrlenp)           accept((int)fd,(struct sockaddr *)addr,(socklen_t *)addrlenp)
#define ip_listen(fd,backlog)                 listen((int)fd,backlog)

#define ip_send(fd,msg,len,flags)             send((int)fd,(const void *)msg,(size_t)len,flags)
#define ip_sendto(fd,msg,len,flags,to,tolen)  sendto((int)fd,(const void *)msg,(size_t)len,flags,(const struct sockaddr *)to,(socklen_t)tolen)
#define ip_sendmsg(fd,msg,flags)              sendmsg((int)fd,(const struct Ip_msghdr *)msg,flags)

#define ip_recv(fd,buf,len,flags)             recv((int)fd,(void *)buf,(size_t)len,flags)
#define ip_recvfrom(fd,buf,len,flags,from,fp) recvfrom((int)fd,(void *)buf,(size_t)len,flags,(struct sockaddr *)from,(socklen_t *)fp)
#define ip_recvmsg(fd,msg,flags)              recvmsg((int)fd,(struct Ip_msghdr *)msg,flags)

#define ip_getsockopt(fd,level,optname,optval,optlenp)  getsockopt((int)fd,level,optname,optval,(socklen_t *)optlenp)
#define ip_setsockopt(fd,level,optname,optval,optlen)   setsockopt((int)fd,level,optname,(const void *)optval,(socklen_t)optlen)

#define ip_getsockname(fd,name,namelenp)         getsockname((int)fd,(struct sockaddr *)name,(socklen_t *)namelenp)
#define ip_getpeername(fd,name,namelenp)         getpeername((int)fd,(struct sockaddr *)name,(socklen_t *)namelenp)

#define inet_pton(family,strptr,addrptr)      inet_pton(family,(const char *)strptr,addrptr)
#define inet_ntop(family,addrptr,strptr,len)  inet_ntop(family,(const void *)addrptr,strptr,(size_t)len)

#define ip_if_nametoindex(ifname)             if_nametoindex((const char *)ifname)
#define ip_if_indextoname(ifindex,ifname)     if_indextoname((int)ifindex, (char *)ifname)
#define ip_if_nameindex()                     if_nameindex()
#define ip_if_freenameindex(pif)              if_freenameindex(pif)

/*
#define inet6_rth_space(t,s)               ipcom_inet6_rth_space(t,s)
#define inet6_rth_init(b,l,t,s)            ipcom_inet6_rth_init(b,l,t,s)
#define inet6_rth_add(b,a)                 ipcom_inet6_rth_add(b,a)
#define inet6_rth_reverse(i,o)             ipcom_inet6_rth_reverse(i,o)
#define inet6_rth_segments(b)              ipcom_inet6_rth_segments(b)
#define inet6_rth_getaddr(b,i)             ipcom_inet6_rth_getaddr(b,i)

#define inet6_opt_init(b,l)                ipcom_inet6_opt_init(b,l)
#define inet6_opt_append(eb,el,o,t,l,a,db) ipcom_inet6_opt_append(eb,el,o,t,l,a,db)
#define inet6_opt_finish(eb,el,o)          ipcom_inet6_opt_finish(eb,el,o)
#define inet6_opt_set_val(db,o,v,l)        ipcom_inet6_opt_set_val(db,o,v,l)
#define inet6_opt_next(eb,el,o,t,l,db)     ipcom_inet6_opt_next(eb,el,o,t,l,db)
#define inet6_opt_find(eb,el,o,t,l,db)     ipcom_inet6_opt_find(eb,el,o,t,l,db)
#define inet6_opt_get_val(db,o,v,l)        ipcom_inet6_opt_get_val(db,o,v,l)
*/

/*
 ****************************************************************************
 * 7                    SOCK2
 ****************************************************************************
 */
#if 0
#define protoent   Ip_protoent
#define servent    Ip_servent
#define addrinfo   Ip_addrinfo
#define group_req   Ip_group_req

#define inet_ntoa(in)           ipcom_inet_ntoa(in)
#define inet_addr(cp)           ipcom_inet_addr(cp)
#define inet_aton(cp,addr)      ipcom_inet_aton(cp,addr)
#endif
/* port/src/ipcom_gethostby.c (use getipnodebyname/getipnodebyaddr for all new code!) */
#define ip_gethostbyname(name)     gethostbyname(name)
#define ip_gethostbyaddr(a,l,t)    gethostbyaddr(a,l,t)

/* src/ipcom_sock2.c */
#define ip_getprotobyname(name)    getprotobyname(name)
#define ip_getprotobynumber(proto) getprotobynumber(proto)

/* port/src/ipcom_getservby.c */
#define ip_getservbyname(n,p)      getservbyname(n,p)
#define ip_getservbyport(p,p2)     getservbyport(p,p2)







#if 0
/******************************************************************************/
/******************************************************************************/
/*_IPCOM_DEFINE_H_*/

#define NETLINK_ROUTE   		IP_NETLINK_ROUTE
#define RT_TABLE_MAIN   		IP_RT_TABLE_MAIN


#define RTM_VERSION            IPNET_RTM_VERSION
#define RTM_NEW_RTMSG          IP_RTM_NEWROUTE
#define RTM_ADD                IPNET_RTM_ADD
#define RTM_DELETE             IPNET_RTM_DELETE
#define RTM_MISS               IPNET_RTM_MISS
#define RTM_LOCK               IPNET_RTM_LOCK
#define RTM_OLDADD             IPNET_RTM_OLDADD
#define RTM_OLDDEL             IPNET_RTM_OLDDEL
#define RTM_RESOLVE            IPNET_RTM_RESOLVE
#define RTM_CHANGE             IPNET_RTM_CHANGE
#define RTM_REDIRECT           IPNET_RTM_REDIRECT
#define RTM_IFINFO             IPNET_RTM_IFINFO


#define RTM_VPN_ADD            0xfff1
#define RTM_VPN_DEL            0xfff2
#define RTM_VPN_RT_CHANGE      0xfff3
#define RTM_TCPSYNC_ADD        0xfff4
#define RTM_TCPSYNC_DEL        0xfff5
#define RTM_SUBSYSTEM_UP       0xfff6
#define RTM_SLOT_UP            0xfff7
#define RTM_CARD_UP            0xfff8
#define RTM_CARD_DOWN          0xfff9
#define RTM_CARD_ROLECHANGE    0xfffa
#define RTM_BFD_SESSION        0xfffb
#define RTM_BFD_IFSTATUS       0xfffc
#define RTM_LOSING             0xfffd



#define RTMGRP_LINK            IP_RTMGRP_LINK
#define RTMGRP_IPV4_ROUTE      IP_RTMGRP_IPV4_ROUTE
#define RTMGRP_IPV4_IFADDR     IP_RTMGRP_IPV4_IFADDR
#define RTMGRP_IPV6_ROUTE      IP_RTMGRP_IPV6_ROUTE
#define RTMGRP_IPV6_IFADDR     IP_RTMGRP_IPV6_IFADDR

#define RTM_NEWLINK    		IP_RTM_NEWLINK
#define RTM_DELLINK    		IP_RTM_DELLINK
#define RTM_GETLINK    		IP_RTM_GETLINK
#define RTM_SETLINK    		IP_RTM_SETLINK
#define RTM_NEWADDR    		IP_RTM_NEWADDR
#define RTM_DELADDR    		IP_RTM_DELADDR
#define RTM_GETADDR    		IP_RTM_GETADDR
#define RTM_NEWROUTE    	IP_RTM_NEWROUTE
#define RTM_DELROUTE    	IP_RTM_DELROUTE
#define RTM_GETROUTE    	IP_RTM_GETROUTE
#define RTM_NEWNEIGH    	IP_RTM_NEWNEIGH
#define RTM_DELNEIGH    	IP_RTM_DELNEIGH
#define RTM_GETNEIGH    	IP_RTM_GETNEIGH
#define RTM_NEWRULE    		IP_RTM_NEWRULE
#define RTM_DELRULE    		IP_RTM_DELRULE
#define RTM_GETRULE    		IP_RTM_GETRULE
#define RTM_NEWQDISC    	IP_RTM_NEWQDISC
#define RTM_DELQDISC    	IP_RTM_DELQDISC
#define RTM_GETQDISC    	IP_RTM_GETQDISC
#define RTM_NEWTCLASS    	IP_RTM_NEWTCLASS
#define RTM_DELTCLASS    	IP_RTM_DELTCLASS
#define RTM_GETTCLASS    	IP_RTM_GETTCLASS
#define RTM_NEWTFILTER    	IP_RTM_NEWTFILTER
#define RTM_DELTFILTER    	IP_RTM_DELTFILTER
#define RTM_GETTFILTER    	IP_RTM_GETTFILTER
#define RTM_NEWACTION    	IP_RTM_NEWACTION
#define RTM_DELACTION    	IP_RTM_DELACTION
#define RTM_GETACTION    	IP_RTM_GETACTION
#define RTM_NEWPREFIX    	IP_RTM_NEWPREFIX
#define RTM_GETPREFIX    	IP_RTM_GETPREFIX
#define RTM_GETMULTICAST    IP_RTM_GETMULTICAST
#define RTM_GETANYCAST    	IP_RTM_GETANYCAST
#define RTM_NEWNEIGHTBL    	IP_RTM_NEWNEIGHTBL
#define RTM_GETNEIGHTBL    	IP_RTM_GETNEIGHTBL
#define RTM_SETNEIGHTBL    	IP_RTM_SETNEIGHTBL
#define RTM_NEWVR    		IP_RTM_NEWVR
#define RTM_DELVR    		IP_RTM_DELVR
#define RTM_GETVR    		IP_RTM_GETVR
#define RTM_CHANGEVR    	IP_RTM_CHANGEVR
#define RTM_NEWMIP    		IP_RTM_NEWMIP
#define RTM_DELMIP    		IP_RTM_DELMIP
#define RTM_GETMIP    		IP_RTM_GETMIP
#define RTM_SETMIP    		IP_RTM_SETMIP
#define RTM_NEWSEND    		IP_RTM_NEWSEND
#define RTM_GETSEND    		IP_RTM_GETSEND
#define RTM_SEND_SIGN_REQ   IP_RTM_SEND_SIGN_REQ

#define RTM_RTA             IP_RTM_RTA


#define RTM_F_CLONED        IP_RTM_F_CLONED

#define RTN_UNSPEC    		IP_RTN_UNSPEC
#define RTN_UNICAST    		IP_RTN_UNICAST
#define RTN_LOCAL    		IP_RTN_LOCAL
#define RTN_BROADCAST    	IP_RTN_BROADCAST
#define RTN_ANYCAST    		IP_RTN_ANYCAST
#define RTN_MULTICAST    	IP_RTN_MULTICAST
#define RTN_BLACKHOLE    	IP_RTN_BLACKHOLE
#define RTN_UNREACHABLE    	IP_RTN_UNREACHABLE
#define RTN_PROHIBIT    	IP_RTN_PROHIBIT
#define RTN_THROW    		IP_RTN_THROW
#define RTN_NAT    			IP_RTN_NAT
#define RTN_XRESOLVE    	IP_RTN_XRESOLVE
#define RTN_PROXY    		IP_RTN_PROXY
#define RTN_CLONE    		IP_RTN_CLONE


#define RTNH_DATA              IP_RTNH_DATA
#define RTNH_NEXT              IP_RTNH_NEXT
#define RTNH_F_ONLINK          IP_RTNH_F_ONLINK


#define RTPROT_UNSPEC    	IP_RTPROT_UNSPEC
#define RTPROT_REDIRECT    	IP_RTPROT_REDIRECT
#define RTPROT_KERNEL    	IP_RTPROT_KERNEL
#define RTPROT_BOOT    		IP_RTPROT_BOOT
#define RTPROT_STATIC    	IP_RTPROT_STATIC
#define RTPROT_GATED    	IP_RTPROT_GATED
#define RTPROT_RA    		IP_RTPROT_RA
#define RTPROT_MRT    		IP_RTPROT_MRT
#define RTPROT_ZEBRA    	IP_RTPROT_ZEBRA
#define RTPROT_BIRD    		IP_RTPROT_BIRD
#define RTPROT_DNROUTED    	IP_RTPROT_DNROUTED
#define RTPROT_XORP    		IP_RTPROT_XORP
#define RTPROT_NTK    		IP_RTPROT_NTK


#define RTA_ALIGN           IP_RTA_ALIGN
#define RTA_LENGTH          IP_RTA_LENGTH
#define RTA_OK              IP_RTA_OK
#define RTA_NEXT            IP_RTA_NEXT
#define RTA_PAYLOAD         IP_RTA_PAYLOAD
#define RTA_DATA            IP_RTA_DATA
#define RTA_UNSPEC   		IP_RTA_UNSPEC
#define RTA_DST   			IP_RTA_DST
#define RTA_SRC   			IP_RTA_SRC
#define RTA_IIF   			IP_RTA_IIF
#define RTA_OIF   			IP_RTA_OIF
#define RTA_GATEWAY   		IP_RTA_GATEWAY
#define RTA_PRIORITY   		IP_RTA_PRIORITY
#define RTA_PREFSRC   		IP_RTA_PREFSRC
#define RTA_METRICS   		IP_RTA_METRICS
#define RTA_MULTIPATH   	IP_RTA_MULTIPATH
#define RTA_PROTOINFO   	IP_RTA_PROTOINFO
#define RTA_FLOW   			IP_RTA_FLOW
#define RTA_CACHEINFO   	IP_RTA_CACHEINFO
#define RTA_SESSION   		IP_RTA_SESSION
#define RTA_MP_ALGO   		IP_RTA_MP_ALGO
#define RTA_TABLE   		IP_RTA_TABLE
#define RTA_NH_PROTO  		IP_RTA_NH_PROTO
#define RTA_NH_PROTO_DATA   IP_RTA_NH_PROTO_DATA
#define RTA_PROXY_ARP_LLADDR   IP_RTA_PROXY_ARP_LLADDR
#define RTA_VR   			IP_RTA_VR
#define RTA_TABLE_NAME   	IP_RTA_TABLE_NAME
#define RTA_MAX    			IP_RTA_MAX

#define RTAX_IFA                        IPNET_RTAX_IFA
#define RTAX_DST                       	IPNET_RTAX_DST
#define RTAX_GATEWAY              		IPNET_RTAX_GATEWAY
#define RTAX_NETMASK               		IPNET_RTAX_NETMASK
#define RTAX_MAX                       	IPNET_RTAX_MAX
#define RTAX_MTU 						IP_RTAX_MTU


#define IFLA_UNSPEC    		IP_IFLA_UNSPEC
#define IFLA_ADDRESS    	IP_IFLA_ADDRESS
#define IFLA_BROADCAST    	IP_IFLA_BROADCAST
#define IFLA_IFNAME        	IP_IFLA_IFNAME
#define IFLA_MTU       		IP_IFLA_MTU
#define IFLA_LINK    		IP_IFLA_LINK
#define IFLA_QDISC    		IP_IFLA_QDISC
#define IFLA_STATS    		IP_IFLA_STATS
#define IFLA_COST    		IP_IFLA_COST
#define IFLA_PRIORITY    	IP_IFLA_PRIORITY
#define IFLA_MASTER    		IP_IFLA_MASTER
#define IFLA_WIRELESS    	IP_IFLA_WIRELESS
#define IFLA_PROTINFO    	IP_IFLA_PROTINFO
#define IFLA_TXQLEN    		IP_IFLA_TXQLEN
#define IFLA_MAP    		IP_IFLA_MAP
#define IFLA_WEIGHT    		IP_IFLA_WEIGHT
#define IFLA_OPERSTATE    	IP_IFLA_OPERSTATE
#define IFLA_LINKMODE    	IP_IFLA_LINKMODE
#define IFLA_LINKINFO    	IP_IFLA_LINKINFO
#define IFLA_MAX         	IP_IFLA_MAX
#define IFLA_RTA         	IP_IFLA_RTA


#define NLMSG_ALIGNTO  IP_NLMSG_ALIGNTO
#define NLMSG_ALIGN    IP_NLMSG_ALIGN
#define NLMSG_LENGTH   IP_NLMSG_LENGTH
#define NLMSG_SPACE    IP_NLMSG_SPACE
#define NLMSG_DATA     IP_NLMSG_DATA
#define NLMSG_NEXT     IP_NLMSG_NEXT
#define NLMSG_OK       IP_NLMSG_OK
#define NLMSG_PAYLOAD  IP_NLMSG_PAYLOAD
#define NLMSG_NOOP     IP_NLMSG_NOOP
#define NLMSG_ERROR    IP_NLMSG_ERROR
#define NLMSG_DONE     IP_NLMSG_DONE
#define NLMSG_OVERRUN  IP_NLMSG_OVERRUN
#define NLMSG_LENGTH   IP_NLMSG_LENGTH


#define NLM_F_ROOT 			IP_NLM_F_ROOT
#define NLM_F_MATCH 		IP_NLM_F_MATCH
#define NLM_F_REQUEST 		IP_NLM_F_REQUEST
#define NLM_F_CREATE 		IP_NLM_F_CREATE
#define NLM_F_MULTI  		IP_NLM_F_MULTI
#define NLM_F_ACK   		IP_NLM_F_ACK
#define NLM_F_REPLACE   	IP_NLM_F_REPLACE


#define IFA_MAX     		IP_IFA_MAX
#define IFA_UNSPEC    		IP_IFA_UNSPEC
#define IFA_ADDRESS    		IP_IFA_ADDRESS
#define IFA_LOCAL    		IP_IFA_LOCAL
#define IFA_LABEL    		IP_IFA_LABEL
#define IFA_BROADCAST    	IP_IFA_BROADCAST
#define IFA_ANYCAST    		IP_IFA_ANYCAST
#define IFA_CACHEINFO    	IP_IFA_CACHEINFO
#define IFA_MULTICAST    	IP_IFA_MULTICAST
#define IFA_VR    			IP_IFA_VR
#define IFA_TABLE    		IP_IFA_TABLE
#define IFA_TABLE_NAME    	IP_IFA_TABLE_NAME
#define IFA_X_INFO    		IP_IFA_X_INFO
#define IFA_F_SECONDARY 	IP_IFA_F_SECONDARY
#define IFA_RTA         	IP_IFA_RTA


#define RTF_UP              IPNET_RTF_UP




#define RT_SCOPE_UNIVERSE 		IP_RT_SCOPE_UNIVERSE
#define RT_SCOPE_SITE 			IP_RT_SCOPE_SITE
#define RT_SCOPE_LINK 			IP_RT_SCOPE_LINK
#define RT_SCOPE_HOST 			IP_RT_SCOPE_HOST
#define RT_SCOPE_NOWHERE 		IP_RT_SCOPE_NOWHERE

#define IFT_ETHER                   IP_IFT_ETHER
#define IFT_BRIDGE                  IP_IFT_BRIDGE
#define IFT_IEEE8023ADLAG       	IP_IFT_IEEE8023ADLAG
#define IFT_L2VLAN                  IP_IFT_L2VLAN

//#define RTV_VALUE1                   IPNET_RTV_VALUE1









#define nlmsghdr               	Ip_nlmsghdr
#define ifaddrmsg 				Ip_ifaddrmsg
#define rtmsg     				Ip_rtmsg
#define rtgenmsg  				Ip_rtgenmsg
#define sockaddr_nl   			sockaddr_nl
#define nlmsgerr      			Ip_nlmsgerr
#define rtattr        			Ip_rtattr
#define ifinfomsg     			Ip_ifinfomsg
#define ifa_cacheinfo 			Ip_ifa_cacheinfo
#define rtnexthop      			Ip_rtnexthop

#define  ifconf                 Ip_ifconf
#define  ifc_req                ip_ifc_req
#define  iphdr                  Ipcom_iphdr
#define  ip                     Ipcom_iphdr
#define  LLADDR                 SOCKADDR_DL_LLADDR


//#define ip_mreqn           Ip_ip_mreqn


#define ifaddrs   Ip_ifaddrs
//#define getifaddrs  ipcom_getifaddrs
//#define freeifaddrs ipcom_freeifaddrs
#define if_data   Ipnet_if_data

//#define    utsname                       Ip_utsname
//#define    uname                          ipcom_uname


/*
 *===========================================================================
 *                    timeval
 *===========================================================================
 */
#define timeval       Ip_timeval


#define strerror   ipcom_strerror

#endif

/*_IPCOM_DEFINE_H_ */


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_IP_OS_SOCK_H_ */
