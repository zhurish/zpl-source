#! /bin/bash


./replace.sh $1 SOL_SOCKET IPSTACK_SOL_SOCKET
./replace.sh $1 SO_REUSEADDR IPSTACK_SO_REUSEADDR
./replace.sh $1 SO_KEEPALIVE IPSTACK_SO_KEEPALIVE
./replace.sh $1 AF_UNSPEC IPSTACK_AF_UNSPEC
./replace.sh $1 AF_INET IPSTACK_AF_INET
./replace.sh $1 AF_INET6 IPSTACK_AF_INET6
./replace.sh $1 AF_PACKET IPSTACK_AF_PACKET
./replace.sh $1 AF_NETLINK IPSTACK_AF_NETLINK
./replace.sh $1 AF_LINK IPSTACK_AF_LINK
./replace.sh $1 AF_ROUTE IPSTACK_AF_ROUTE

./replace.sh $1 PF_INET IPSTACK_PF_INET
./replace.sh $1 PF_INET6 IPSTACK_PF_INET6
./replace.sh $1 PF_NETLINK IPSTACK_PF_NETLINK
./replace.sh $1 PF_ROUTE IPSTACK_PF_ROUTE
./replace.sh $1 PF_LINK IPSTACK_PF_LINK
./replace.sh $1 PF_PACKET IPSTACK_PF_PACKET
./replace.sh $1 PF_KEY IPSTACK_PF_KEY

./replace.sh $1 SOCK_STREAM   IPSTACK_SOCK_STREAM
./replace.sh $1 SOCK_DGRAM    IPSTACK_SOCK_DGRAM
./replace.sh $1 SOCK_RAW      IPSTACK_SOCK_RAW
./replace.sh $1 SOCK_PACKET   IPSTACK_SOCK_PACKET


./replace.sh $1 IPPROTO_IP          IPSTACK_IPPROTO_IP
./replace.sh $1 IPPROTO_ICMP        IPSTACK_IPPROTO_ICMP
./replace.sh $1 IPPROTO_IGMP        IPSTACK_IPPROTO_IGMP
./replace.sh $1 IPPROTO_IPIP        IPSTACK_IPPROTO_IPIP
./replace.sh $1 IPPROTO_IPV4        IPSTACK_IPPROTO_IPV4
./replace.sh $1 IPPROTO_TCP         IPSTACK_IPPROTO_TCP
./replace.sh $1 IPPROTO_UDP         IPSTACK_IPPROTO_UDP
./replace.sh $1 IPPROTO_IPV6        IPSTACK_IPPROTO_IPV6
./replace.sh $1 IPPROTO_RSVP        IPSTACK_IPPROTO_RSVP
./replace.sh $1 IPPROTO_GRE         IPSTACK_IPPROTO_GRE
./replace.sh $1 IPPROTO_ESP         IPSTACK_IPPROTO_ESP
./replace.sh $1 IPPROTO_AH          IPSTACK_IPPROTO_AH
./replace.sh $1 IPPROTO_ICMPV6      IPSTACK_IPPROTO_ICMPV6
./replace.sh $1 IPPROTO_OSPFIGP     IPSTACK_IPPROTO_OSPFIGP
./replace.sh $1 IPPROTO_PIM         IPSTACK_IPPROTO_PIM
./replace.sh $1 IPPROTO_RAW         IPSTACK_IPPROTO_RAW
./replace.sh $1 IPPROTO_MAX         IPSTACK_IPPROTO_MAX

./replace.sh $1 IPPROTO_HOPOPTS     IPSTACK_IPPROTO_HOPOPTS
./replace.sh $1 IPPROTO_ROUTING     IPSTACK_IPPROTO_ROUTING
./replace.sh $1 IPPROTO_FRAGMENT    IPSTACK_IPPROTO_FRAGMENT
./replace.sh $1 IPPROTO_NONE        IPSTACK_IPPROTO_NONE
./replace.sh $1 IPPROTO_DSTOPTS     IPSTACK_IPPROTO_DSTOPTS


./replace.sh $1 INADDR_ANY             IPSTACK_INADDR_ANY
./replace.sh $1 INADDR_DEFAULT         IPSTACK_INADDR_DEFAULT
./replace.sh $1 INADDR_LOOPBACK        IPSTACK_INADDR_LOOPBACK
./replace.sh $1 INADDR_BROADCAST       IPSTACK_INADDR_BROADCAST
./replace.sh $1 INADDR_NONE            IPSTACK_INADDR_NONE

./replace.sh $1 INADDR_UNSPEC_GROUP    IPSTACK_INADDR_UNSPEC_GROUP
./replace.sh $1 INADDR_ALLHOSTS_GROUP  IPSTACK_INADDR_ALLHOSTS_GROUP



./replace.sh $1 NETMASK          IPSTACK_NETMASK
./replace.sh $1 NETBITS          IPSTACK_NETBITS

./replace.sh $1 IN_CLASSA	       IPSTACK_IN_CLASSA
./replace.sh $1 IN_CLASSB        IPSTACK_IN_CLASSB
./replace.sh $1 IN_CLASSC        IPSTACK_IN_CLASSC
./replace.sh $1 IN_CLASSD        IPSTACK_IN_CLASSD
./replace.sh $1 IN_EXPERIMENTAL  IPSTACK_IN_EXPERIMENTAL
./replace.sh $1 INET_ADDRSTRLEN        IPSTACK_INET_ADDRSTRLEN
./replace.sh $1 IN_MULTICAST     IPSTACK_IN_CLASSD


./replace.sh $1 IN6_ARE_ADDR_EQUAL           IPSTACK_IN6_ARE_ADDR_EQUAL
./replace.sh $1 IN6_IS_ADDR_UNSPECIFIED        IPSTACK_IN6_IS_ADDR_UNSPECIFIED
./replace.sh $1 IN6_IS_ADDR_LOOPBACK           IPSTACK_IN6_IS_ADDR_LOOPBACK
./replace.sh $1 IN6_IS_ADDR_V4COMPAT           IPSTACK_IN6_IS_ADDR_V4COMPAT
./replace.sh $1 IN6_IS_ADDR_V4MAPPED           IPSTACK_IN6_IS_ADDR_V4MAPPED
./replace.sh $1 IN6_IS_ADDR_AGGR_GLOB_UNICAST  IPSTACK_IN6_IS_ADDR_AGGR_GLOB_UNICAST
./replace.sh $1 IN6_IS_ADDR_SITE_LOCAL         IPSTACK_IN6_IS_ADDR_SITE_LOCAL
./replace.sh $1 IN6_IS_ADDR_LINK_LOCAL         IPSTACK_IN6_IS_ADDR_LINK_LOCAL
./replace.sh $1 IN6_IS_ADDR_MULTICAST          IPSTACK_IN6_IS_ADDR_MULTICAST
./replace.sh $1 IN6_IS_ADDR_LINKLOCAL          IPSTACK_IN6_IS_ADDR_LINK_LOCAL


./replace.sh $1 SO_REUSEADDR       IPSTACK_SO_REUSEADDR
./replace.sh $1 SO_KEEPALIVE       IPSTACK_SO_KEEPALIVE
./replace.sh $1 SO_DONTROUTE       IPSTACK_SO_DONTROUTE
./replace.sh $1 SO_BROADCAST       IPSTACK_SO_BROADCAST
./replace.sh $1 SO_REUSEPORT       IPSTACK_SO_REUSEPORT
./replace.sh $1 SO_SNDBUF          IPSTACK_SO_SNDBUF
./replace.sh $1 SO_RCVBUF          IPSTACK_SO_RCVBUF
./replace.sh $1 SO_RCVTIMEO        IPSTACK_SO_RCVTIMEO
./replace.sh $1 SO_ERROR           IPSTACK_SO_ERROR
./replace.sh $1 SO_TYPE            IPSTACK_SO_TYPE
./replace.sh $1 SO_BINDTODEVICE    IPSTACK_SO_BINDTODEVICE

./replace.sh $1 HDRINCL         IPSTACK_HDRINCL
./replace.sh $1 TOS             IPSTACK_TOS
./replace.sh $1 TTL             IPSTACK_TTL
./replace.sh $1 PKTINFO         IPSTACK_PKTINFO
./replace.sh $1 MULTICAST_IF    IPSTACK_MULTICAST_IF
./replace.sh $1 MULTICAST_TTL   IPSTACK_MULTICAST_TTL
./replace.sh $1 MULTICAST_LOOP  IPSTACK_MULTICAST_LOOP
./replace.sh $1 ADD_MEMBERSHIP  IPSTACK_ADD_MEMBERSHIP
./replace.sh $1 JOIN_GROUP      IPSTACK_JOIN_GROUP
./replace.sh $1 DROP_MEMBERSHIP IPSTACK_DROP_MEMBERSHIP
./replace.sh $1 LEAVE_GROUP     IPSTACK_LEAVE_GROUP
./replace.sh $1 RECVIF          IPSTACK_RECVIF
./replace.sh $1 ROUTER_ALERT    IPSTACK_ROUTER_ALERT
./replace.sh $1 DONTFRAG        IPSTACK_DONTFRAG

./replace.sh $1 MCAST_JOIN_GROUP	IPSTACK_JOIN_GROUP
./replace.sh $1 MCAST_LEAVE_GROUP	IPSTACK_LEAVE_GROUP


./replace.sh $1 IPV6_UNICAST_HOPS     IPSTACK_IPV6_UNICAST_HOPS
./replace.sh $1 IPV6_MULTICAST_IF     IPSTACK_IPV6_MULTICAST_IF
./replace.sh $1 IPV6_MULTICAST_HOPS   IPSTACK_IPV6_MULTICAST_HOPS
./replace.sh $1 IPV6_MULTICAST_LOOP   IPSTACK_IPV6_MULTICAST_LOOP
./replace.sh $1 IPV6_JOIN_GROUP       IPSTACK_IPV6_JOIN_GROUP
./replace.sh $1 IPV6_ADD_MEMBERSHIP   IPSTACK_IPV6_ADD_MEMBERSHIP
./replace.sh $1 IPV6_LEAVE_GROUP      IPSTACK_IPV6_LEAVE_GROUP
./replace.sh $1 IPV6_DROP_MEMBERSHIP  IPSTACK_IPV6_DROP_MEMBERSHIP
./replace.sh $1 IPV6_PKTINFO          IPSTACK_IPV6_PKTINFO
./replace.sh $1 IPV6_TCLASS           IPSTACK_IPV6_TCLASS
./replace.sh $1 IPV6_NEXTHOP          IPSTACK_IPV6_NEXTHOP
./replace.sh $1 IPV6_RTHDR            IPSTACK_IPV6_RTHDR
./replace.sh $1 IPV6_HOPOPTS          IPSTACK_IPV6_HOPOPTS
./replace.sh $1 IPV6_DSTOPTS          IPSTACK_IPV6_DSTOPTS
./replace.sh $1 IPV6_RTHDRDSTOPTS     IPSTACK_IPV6_RTHDRDSTOPTS
./replace.sh $1 IPV6_RECVPKTINFO      IPSTACK_IPV6_RECVPKTINFO
./replace.sh $1 IPV6_RECVHOPLIMIT     IPSTACK_IPV6_RECVHOPLIMIT
./replace.sh $1 IPV6_RECVTCLASS       IPSTACK_IPV6_RECVTCLASS
./replace.sh $1 IPV6_RECVRTHDR        IPSTACK_IPV6_RECVRTHDR
./replace.sh $1 IPV6_RECVHOPOPTS      IPSTACK_IPV6_RECVHOPOPTS
./replace.sh $1 IPV6_RECVDSTOPTS      IPSTACK_IPV6_RECVDSTOPTS
./replace.sh $1 IPV6_CHECKSUM         IPSTACK_IPV6_CHECKSUM
./replace.sh $1 IPV6_RECVIF           IPSTACK_IPV6_RECVIF

./replace.sh $1 ICMP6_FILTER          IPSTACK_ICMP6_FILTER

./replace.sh $1 TCP_NODELAY           IPSTACK_TCP_NODELAY

./replace.sh $1 IPV6_HOPLIMIT         IPSTACK_IPV6_HOPLIMIT

./replace.sh $1 ARPHRD_ETHER          IPSTACK_ARPHRD_ETHER


./replace.sh $1 MSG_OOB			IPSTACK_MSG_OOB                   
./replace.sh $1 MSG_PEEK		IPSTACK_MSG_PEEK                  
./replace.sh $1 MSG_DONTROUTE	IPSTACK_MSG_DONTROUTE             
./replace.sh $1 MSG_WAITALL		IPSTACK_MSG_WAITALL              
./replace.sh $1 MSG_DONTWAIT	IPSTACK_MSG_DONTWAIT              

./replace.sh $1 MSG_NOTIFICATION	IPSTACK_MSG_NOTIFICATION   
./replace.sh $1 MSG_EOR				IPSTACK_MSG_EOR

./replace.sh $1 MSG_MORE		IPSTACK_MSG_MORE                
./replace.sh $1 MSG_ZBUF		IPSTACK_MSG_ZBUF             

./replace.sh $1 SIOCSPGRP          IPSTACK_SIOCSPGRP
./replace.sh $1 SIOCGPGRP          IPSTACK_SIOCGPGRP
./replace.sh $1 FIONBIO            IPSTACK_FIONBIO

./replace.sh $1 SIOCGIFADDR        IPSTACK_SIOCGIFADDR
./replace.sh $1 SIOCSIFADDR        IPSTACK_SIOCSIFADDR
./replace.sh $1 SIOCGIFBRDADDR     IPSTACK_SIOCGIFBRDADDR
./replace.sh $1 SIOCSIFBRDADDR     IPSTACK_SIOCSIFBRDADDR
./replace.sh $1 SIOCGIFNETMASK     IPSTACK_SIOCGIFNETMASK
./replace.sh $1 SIOCSIFNETMASK     IPSTACK_SIOCSIFNETMASK
./replace.sh $1 SIOCGIFDSTADDR     IPSTACK_SIOCGIFDSTADDR
./replace.sh $1 SIOCSIFDSTADDR     IPSTACK_SIOCSIFDSTADDR
./replace.sh $1 SIOCAIFADDR        IPSTACK_SIOCAIFADDR
./replace.sh $1 SIOCDIFADDR        IPSTACK_SIOCDIFADDR
./replace.sh $1 SIOCADDRT          IPSTACK_SIOCADDRT
./replace.sh $1 SIOCDDDRT          IPSTACK_SIOCDDDRT

./replace.sh $1 SIOCAIFADDR_IN6    IPSTACK_SIOCAIFADDR_IN6
./replace.sh $1 SIOCDIFADDR_IN6    IPSTACK_SIOCDIFADDR_IN6
./replace.sh $1 SIOCGIFDSTADDR_IN6 IPSTACK_SIOCGIFDSTADDR_IN6

./replace.sh $1 SIOCGIFFLAGS       IPSTACK_SIOCGIFFLAGS
./replace.sh $1 SIOCSIFFLAGS       IPSTACK_SIOCSIFFLAGS
./replace.sh $1 SIOCGIFMTU         IPSTACK_SIOCGIFMTU
./replace.sh $1 SIOCSIFMTU         IPSTACK_SIOCSIFMTU
./replace.sh $1 SIOCGIFMETRIC      IPSTACK_SIOCGIFMETRIC
./replace.sh $1 SIOCSIFMETRIC      IPSTACK_SIOCSIFMETRIC
./replace.sh $1 SIOCSIFRTAB        IPSTACK_SIOCSIFRTAB
./replace.sh $1 SIOCGIFRTAB        IPSTACK_SIOCGIFRTAB
./replace.sh $1 SIOCGETTUNNEL      IPSTACK_SIOCGETTUNNEL
./replace.sh $1 SIOCADDTUNNEL      IPSTACK_SIOCADDTUNNEL
./replace.sh $1 SIOCCHGTUNNEL      IPSTACK_SIOCCHGTUNNEL
./replace.sh $1 SIOCDELTUNNEL      IPSTACK_SIOCDELTUNNEL

./replace.sh $1 SIOCSARP           IPSTACK_SIOCSARP
./replace.sh $1 SIOCGARP           IPSTACK_SIOCGARP
./replace.sh $1 SIOCDARP           IPSTACK_SIOCDARP

./replace.sh $1 SIOCGIFHWADDR          IPSTACK_SIOCGIFHWADDR
./replace.sh $1 SIOCGIFCONF            IPSTACK_SIOCGIFCONF
./replace.sh $1 SIOCGIFTXQLEN          IPSTACK_SIOCGIFTXQLEN

./replace.sh $1 IFF_UP          IPSTACK_IFF_UP
./replace.sh $1 IFF_BROADCAST   IPSTACK_IFF_BROADCAST
./replace.sh $1 IFF_DEBUG       IPSTACK_IFF_DEBUG
./replace.sh $1 IFF_LOOPBACK    IPSTACK_IFF_LOOPBACK
./replace.sh $1 IFF_POINTOPOINT IPSTACK_IFF_POINTOPOINT
./replace.sh $1 IFF_RUNNING     IPSTACK_IFF_RUNNING
./replace.sh $1 IFF_NOARP       IPSTACK_IFF_NOARP
./replace.sh $1 IFF_PROMISC     IPSTACK_IFF_PROMISC
./replace.sh $1 IFF_ALLMULTI    IPSTACK_IFF_ALLMULTI
./replace.sh $1 IFF_OACTIVE     IPSTACK_IFF_OACTIVE
./replace.sh $1 IFF_SIMPLEX     IPSTACK_IFF_SIMPLEX
./replace.sh $1 IFF_LINK0       IPSTACK_IFF_LINK0
./replace.sh $1 IFF_LINK1       IPSTACK_IFF_LINK1
./replace.sh $1 IFF_LINK2       IPSTACK_IFF_LINK2
./replace.sh $1 IFF_MULTICAST   IPSTACK_IFF_MULTICAST


./replace.sh $1 MSG_PEEK           IPSTACK_MSG_PEEK

./replace.sh $1 SHUT_RD            IPSTACK_SHUT_RD
./replace.sh $1 SHUT_WR            IPSTACK_SHUT_WR
./replace.sh $1 SHUT_RDWR          IPSTACK_SHUT_RDWR



./replace.sh $1 SOCKADDR_DL_LLADDR   IPSTACK_SOCKADDR_DL_LLADDR

./replace.sh $1 PACKET_HOST          IPSTACK_PACKET_HOST
./replace.sh $1 PACKET_BROADCAST     IPSTACK_PACKET_BROADCAST
./replace.sh $1 PACKET_MULTICAST     IPSTACK_PACKET_MULTICAST
./replace.sh $1 PACKET_OTHERHOST     IPSTACK_PACKET_OTHERHOST
./replace.sh $1 PACKET_OUTGOING      IPSTACK_PACKET_OUTGOING


./replace.sh $1 NETLINK_ROUTE   		IPSTACK_NETLINK_ROUTE
./replace.sh $1 RT_TABLE_MAIN   		IPSTACK_RT_TABLE_MAIN


./replace.sh $1 RTM_VERSION            IPSTACK_RTM_VERSION
./replace.sh $1 RTM_NEW_RTMSG          IPSTACK_RTM_NEWROUTE
./replace.sh $1 RTM_ADD                IPSTACK_RTM_ADD
./replace.sh $1 RTM_DELETE             IPSTACK_RTM_DELETE
./replace.sh $1 RTM_MISS               IPSTACK_RTM_MISS
./replace.sh $1 RTM_LOCK               IPSTACK_RTM_LOCK
./replace.sh $1 RTM_OLDADD             IPSTACK_RTM_OLDADD
./replace.sh $1 RTM_OLDDEL             IPSTACK_RTM_OLDDEL
./replace.sh $1 RTM_RESOLVE            IPSTACK_RTM_RESOLVE
./replace.sh $1 RTM_CHANGE             IPSTACK_RTM_CHANGE
./replace.sh $1 RTM_REDIRECT           IPSTACK_RTM_REDIRECT
./replace.sh $1 RTM_IFINFO             IPSTACK_RTM_IFINFO


./replace.sh $1 RTMGRP_LINK            IPSTACK_RTMGRP_LINK
./replace.sh $1 RTMGRP_IPV4_ROUTE      IPSTACK_RTMGRP_IPV4_ROUTE
./replace.sh $1 RTMGRP_IPV4_IFADDR     IPSTACK_RTMGRP_IPV4_IFADDR
./replace.sh $1 RTMGRP_IPV6_ROUTE      IPSTACK_RTMGRP_IPV6_ROUTE
./replace.sh $1 RTMGRP_IPV6_IFADDR     IPSTACK_RTMGRP_IPV6_IFADDR

./replace.sh $1 RTM_NEWLINK    		IPSTACK_RTM_NEWLINK
./replace.sh $1 RTM_DELLINK    		IPSTACK_RTM_DELLINK
./replace.sh $1 RTM_GETLINK    		IPSTACK_RTM_GETLINK
./replace.sh $1 RTM_SETLINK    		IPSTACK_RTM_SETLINK
./replace.sh $1 RTM_NEWADDR    		IPSTACK_RTM_NEWADDR
./replace.sh $1 RTM_DELADDR    		IPSTACK_RTM_DELADDR
./replace.sh $1 RTM_GETADDR    		IPSTACK_RTM_GETADDR
./replace.sh $1 RTM_NEWROUTE    	IPSTACK_RTM_NEWROUTE
./replace.sh $1 RTM_DELROUTE    	IPSTACK_RTM_DELROUTE
./replace.sh $1 RTM_GETROUTE    	IPSTACK_RTM_GETROUTE
./replace.sh $1 RTM_NEWNEIGH    	IPSTACK_RTM_NEWNEIGH
./replace.sh $1 RTM_DELNEIGH    	IPSTACK_RTM_DELNEIGH
./replace.sh $1 RTM_GETNEIGH    	IPSTACK_RTM_GETNEIGH
./replace.sh $1 RTM_NEWRULE    		IPSTACK_RTM_NEWRULE
./replace.sh $1 RTM_DELRULE    		IPSTACK_RTM_DELRULE
./replace.sh $1 RTM_GETRULE    		IPSTACK_RTM_GETRULE
./replace.sh $1 RTM_NEWQDISC    	IPSTACK_RTM_NEWQDISC
./replace.sh $1 RTM_DELQDISC    	IPSTACK_RTM_DELQDISC
./replace.sh $1 RTM_GETQDISC    	IPSTACK_RTM_GETQDISC
./replace.sh $1 RTM_NEWTCLASS    	IPSTACK_RTM_NEWTCLASS
./replace.sh $1 RTM_DELTCLASS    	IPSTACK_RTM_DELTCLASS
./replace.sh $1 RTM_GETTCLASS    	IPSTACK_RTM_GETTCLASS
./replace.sh $1 RTM_NEWTFILTER    	IPSTACK_RTM_NEWTFILTER
./replace.sh $1 RTM_DELTFILTER    	IPSTACK_RTM_DELTFILTER
./replace.sh $1 RTM_GETTFILTER    	IPSTACK_RTM_GETTFILTER
./replace.sh $1 RTM_NEWACTION    	IPSTACK_RTM_NEWACTION
./replace.sh $1 RTM_DELACTION    	IPSTACK_RTM_DELACTION
./replace.sh $1 RTM_GETACTION    	IPSTACK_RTM_GETACTION
./replace.sh $1 RTM_NEWPREFIX    	IPSTACK_RTM_NEWPREFIX
./replace.sh $1 RTM_GETPREFIX    	IPSTACK_RTM_GETPREFIX
./replace.sh $1 RTM_GETMULTICAST    IPSTACK_RTM_GETMULTICAST
./replace.sh $1 RTM_GETANYCAST    	IPSTACK_RTM_GETANYCAST
./replace.sh $1 RTM_NEWNEIGHTBL    	IPSTACK_RTM_NEWNEIGHTBL
./replace.sh $1 RTM_GETNEIGHTBL    	IPSTACK_RTM_GETNEIGHTBL
./replace.sh $1 RTM_SETNEIGHTBL    	IPSTACK_RTM_SETNEIGHTBL
./replace.sh $1 RTM_NEWVR    		IPSTACK_RTM_NEWVR
./replace.sh $1 RTM_DELVR    		IPSTACK_RTM_DELVR
./replace.sh $1 RTM_GETVR    		IPSTACK_RTM_GETVR
./replace.sh $1 RTM_CHANGEVR    	IPSTACK_RTM_CHANGEVR
./replace.sh $1 RTM_NEWMIP    		IPSTACK_RTM_NEWMIP
./replace.sh $1 RTM_DELMIP    		IPSTACK_RTM_DELMIP
./replace.sh $1 RTM_GETMIP    		IPSTACK_RTM_GETMIP
./replace.sh $1 RTM_SETMIP    		IPSTACK_RTM_SETMIP
./replace.sh $1 RTM_NEWSEND    		IPSTACK_RTM_NEWSEND
./replace.sh $1 RTM_GETSEND    		IPSTACK_RTM_GETSEND
./replace.sh $1 RTM_SEND_SIGN_REQ   IPSTACK_RTM_SEND_SIGN_REQ

./replace.sh $1 RTM_RTA             IPSTACK_RTM_RTA


./replace.sh $1 RTM_F_CLONED        IPSTACK_RTM_F_CLONED

./replace.sh $1 RTN_UNSPEC    		IPSTACK_RTN_UNSPEC
./replace.sh $1 RTN_UNICAST    		IPSTACK_RTN_UNICAST
./replace.sh $1 RTN_LOCAL    		IPSTACK_RTN_LOCAL
./replace.sh $1 RTN_BROADCAST    	IPSTACK_RTN_BROADCAST
./replace.sh $1 RTN_ANYCAST    		IPSTACK_RTN_ANYCAST
./replace.sh $1 RTN_MULTICAST    	IPSTACK_RTN_MULTICAST
./replace.sh $1 RTN_BLACKHOLE    	IPSTACK_RTN_BLACKHOLE
./replace.sh $1 RTN_UNREACHABLE    	IPSTACK_RTN_UNREACHABLE
./replace.sh $1 RTN_PROHIBIT    	IPSTACK_RTN_PROHIBIT
./replace.sh $1 RTN_THROW    		IPSTACK_RTN_THROW
./replace.sh $1 RTN_NAT    			IPSTACK_RTN_NAT
./replace.sh $1 RTN_XRESOLVE    	IPSTACK_RTN_XRESOLVE
./replace.sh $1 RTN_PROXY    		IPSTACK_RTN_PROXY
./replace.sh $1 RTN_CLONE    		IPSTACK_RTN_CLONE


./replace.sh $1 RTNH_DATA              IPSTACK_RTNH_DATA
./replace.sh $1 RTNH_NEXT              IPSTACK_RTNH_NEXT
./replace.sh $1 RTNH_F_ONLINK          IPSTACK_RTNH_F_ONLINK


./replace.sh $1 RTPROT_UNSPEC    	IPSTACK_RTPROT_UNSPEC
./replace.sh $1 RTPROT_REDIRECT    	IPSTACK_RTPROT_REDIRECT
./replace.sh $1 RTPROT_KERNEL    	IPSTACK_RTPROT_KERNEL
./replace.sh $1 RTPROT_BOOT    		IPSTACK_RTPROT_BOOT
./replace.sh $1 RTPROT_STATIC    	IPSTACK_RTPROT_STATIC
./replace.sh $1 RTPROT_GATED    	IPSTACK_RTPROT_GATED
./replace.sh $1 RTPROT_RA    		IPSTACK_RTPROT_RA
./replace.sh $1 RTPROT_MRT    		IPSTACK_RTPROT_MRT
./replace.sh $1 RTPROT_ZEBRA    	IPSTACK_RTPROT_ZEBRA
./replace.sh $1 RTPROT_BIRD    		IPSTACK_RTPROT_BIRD
./replace.sh $1 RTPROT_DNROUTED    	IPSTACK_RTPROT_DNROUTED
./replace.sh $1 RTPROT_XORP    		IPSTACK_RTPROT_XORP
./replace.sh $1 RTPROT_NTK    		IPSTACK_RTPROT_NTK


./replace.sh $1 RTA_ALIGN           IPSTACK_RTA_ALIGN
./replace.sh $1 RTA_LENGTH          IPSTACK_RTA_LENGTH
./replace.sh $1 RTA_OK              IPSTACK_RTA_OK
./replace.sh $1 RTA_NEXT            IPSTACK_RTA_NEXT
./replace.sh $1 RTA_PAYLOAD         IPSTACK_RTA_PAYLOAD
./replace.sh $1 RTA_DATA            IPSTACK_RTA_DATA
./replace.sh $1 RTA_UNSPEC   		IPSTACK_RTA_UNSPEC
./replace.sh $1 RTA_DST   			IPSTACK_RTA_DST
./replace.sh $1 RTA_SRC   			IPSTACK_RTA_SRC
./replace.sh $1 RTA_IIF   			IPSTACK_RTA_IIF
./replace.sh $1 RTA_OIF   			IPSTACK_RTA_OIF
./replace.sh $1 RTA_GATEWAY   		IPSTACK_RTA_GATEWAY
./replace.sh $1 RTA_PRIORITY   		IPSTACK_RTA_PRIORITY
./replace.sh $1 RTA_PREFSRC   		IPSTACK_RTA_PREFSRC
./replace.sh $1 RTA_METRICS   		IPSTACK_RTA_METRICS
./replace.sh $1 RTA_MULTIPATH   	IPSTACK_RTA_MULTIPATH
./replace.sh $1 RTA_PROTOINFO   	IPSTACK_RTA_PROTOINFO
./replace.sh $1 RTA_FLOW   			IPSTACK_RTA_FLOW
./replace.sh $1 RTA_CACHEINFO   	IPSTACK_RTA_CACHEINFO
./replace.sh $1 RTA_SESSION   		IPSTACK_RTA_SESSION
./replace.sh $1 RTA_MP_ALGO   		IPSTACK_RTA_MP_ALGO
./replace.sh $1 RTA_TABLE   		IPSTACK_RTA_TABLE
./replace.sh $1 RTA_NH_PROTO  		IPSTACK_RTA_NH_PROTO
./replace.sh $1 RTA_NH_PROTO_DATA   IPSTACK_RTA_NH_PROTO_DATA
./replace.sh $1 RTA_PROXY_ARP_LLADDR   IPSTACK_RTA_PROXY_ARP_LLADDR
./replace.sh $1 RTA_VR   			IPSTACK_RTA_VR
./replace.sh $1 RTA_TABLE_NAME   	IPSTACK_RTA_TABLE_NAME
./replace.sh $1 RTA_MAX    			IPSTACK_RTA_MAX

./replace.sh $1 RTAX_IFA                        IPSTACK_RTAX_IFA
./replace.sh $1 RTAX_DST                       	IPSTACK_RTAX_DST
./replace.sh $1 RTAX_GATEWAY              		IPSTACK_RTAX_GATEWAY
./replace.sh $1 RTAX_NETMASK               		IPSTACK_RTAX_NETMASK
./replace.sh $1 RTAX_MAX                       	IPSTACK_RTAX_MAX
./replace.sh $1 RTAX_MTU 						IPSTACK_RTAX_MTU


./replace.sh $1 IFLA_UNSPEC    		IPSTACK_IFLA_UNSPEC
./replace.sh $1 IFLA_ADDRESS    	IPSTACK_IFLA_ADDRESS
./replace.sh $1 IFLA_BROADCAST    	IPSTACK_IFLA_BROADCAST
./replace.sh $1 IFLA_IFNAME        	IPSTACK_IFLA_IFNAME
./replace.sh $1 IFLA_MTU       		IPSTACK_IFLA_MTU
./replace.sh $1 IFLA_LINK    		IPSTACK_IFLA_LINK
./replace.sh $1 IFLA_QDISC    		IPSTACK_IFLA_QDISC
./replace.sh $1 IFLA_STATS    		IPSTACK_IFLA_STATS
./replace.sh $1 IFLA_COST    		IPSTACK_IFLA_COST
./replace.sh $1 IFLA_PRIORITY    	IPSTACK_IFLA_PRIORITY
./replace.sh $1 IFLA_MASTER    		IPSTACK_IFLA_MASTER
./replace.sh $1 IFLA_WIRELESS    	IPSTACK_IFLA_WIRELESS
./replace.sh $1 IFLA_PROTINFO    	IPSTACK_IFLA_PROTINFO
./replace.sh $1 IFLA_TXQLEN    		IPSTACK_IFLA_TXQLEN
./replace.sh $1 IFLA_MAP    		IPSTACK_IFLA_MAP
./replace.sh $1 IFLA_WEIGHT    		IPSTACK_IFLA_WEIGHT
./replace.sh $1 IFLA_OPERSTATE    	IPSTACK_IFLA_OPERSTATE
./replace.sh $1 IFLA_LINKMODE    	IPSTACK_IFLA_LINKMODE
./replace.sh $1 IFLA_LINKINFO    	IPSTACK_IFLA_LINKINFO
./replace.sh $1 IFLA_MAX         	IPSTACK_IFLA_MAX
./replace.sh $1 IFLA_RTA         	IPSTACK_IFLA_RTA


./replace.sh $1 NLMSG_ALIGNTO  IPSTACK_NLMSG_ALIGNTO
./replace.sh $1 NLMSG_ALIGN    IPSTACK_NLMSG_ALIGN
./replace.sh $1 NLMSG_LENGTH   IPSTACK_NLMSG_LENGTH
./replace.sh $1 NLMSG_SPACE    IPSTACK_NLMSG_SPACE
./replace.sh $1 NLMSG_DATA     IPSTACK_NLMSG_DATA
./replace.sh $1 NLMSG_NEXT     IPSTACK_NLMSG_NEXT
./replace.sh $1 NLMSG_OK       IPSTACK_NLMSG_OK
./replace.sh $1 NLMSG_PAYLOAD  IPSTACK_NLMSG_PAYLOAD
./replace.sh $1 NLMSG_NOOP     IPSTACK_NLMSG_NOOP
./replace.sh $1 NLMSG_ERROR    IPSTACK_NLMSG_ERROR
./replace.sh $1 NLMSG_DONE     IPSTACK_NLMSG_DONE
./replace.sh $1 NLMSG_OVERRUN  IPSTACK_NLMSG_OVERRUN
./replace.sh $1 NLMSG_LENGTH   IPSTACK_NLMSG_LENGTH


./replace.sh $1 NLM_F_ROOT 			IPSTACK_NLM_F_ROOT
./replace.sh $1 NLM_F_MATCH 		IPSTACK_NLM_F_MATCH
./replace.sh $1 NLM_F_REQUEST 		IPSTACK_NLM_F_REQUEST
./replace.sh $1 NLM_F_CREATE 		IPSTACK_NLM_F_CREATE
./replace.sh $1 NLM_F_MULTI  		IPSTACK_NLM_F_MULTI
./replace.sh $1 NLM_F_ACK   		IPSTACK_NLM_F_ACK
./replace.sh $1 NLM_F_REPLACE   	IPSTACK_NLM_F_REPLACE


./replace.sh $1 IFA_MAX     		IPSTACK_IFA_MAX
./replace.sh $1 IFA_UNSPEC    		IPSTACK_IFA_UNSPEC
./replace.sh $1 IFA_ADDRESS    		IPSTACK_IFA_ADDRESS
./replace.sh $1 IFA_LOCAL    		IPSTACK_IFA_LOCAL
./replace.sh $1 IFA_LABEL    		IPSTACK_IFA_LABEL
./replace.sh $1 IFA_BROADCAST    	IPSTACK_IFA_BROADCAST
./replace.sh $1 IFA_ANYCAST    		IPSTACK_IFA_ANYCAST
./replace.sh $1 IFA_CACHEINFO    	IPSTACK_IFA_CACHEINFO
./replace.sh $1 IFA_MULTICAST    	IPSTACK_IFA_MULTICAST
./replace.sh $1 IFA_VR    			IPSTACK_IFA_VR
./replace.sh $1 IFA_TABLE    		IPSTACK_IFA_TABLE
./replace.sh $1 IFA_TABLE_NAME    	IPSTACK_IFA_TABLE_NAME
./replace.sh $1 IFA_X_INFO    		IPSTACK_IFA_X_INFO
./replace.sh $1 IFA_F_SECONDARY 	IPSTACK_IFA_F_SECONDARY
./replace.sh $1 IFA_RTA         	IPSTACK_IFA_RTA


./replace.sh $1 RTF_UP              IPSTACK_RTF_UP


./replace.sh $1 RT_SCOPE_UNIVERSE 		IPSTACK_RT_SCOPE_UNIVERSE
./replace.sh $1 RT_SCOPE_SITE 			IPSTACK_RT_SCOPE_SITE
./replace.sh $1 RT_SCOPE_LINK 			IPSTACK_RT_SCOPE_LINK
./replace.sh $1 RT_SCOPE_HOST 			IPSTACK_RT_SCOPE_HOST
./replace.sh $1 RT_SCOPE_NOWHERE 		IPSTACK_RT_SCOPE_NOWHERE

./replace.sh $1 IFT_ETHER                   IPSTACK_IFT_ETHER
./replace.sh $1 IFT_BRIDGE                  IPSTACK_IFT_BRIDGE
./replace.sh $1 IFT_IEEE8023ADLAG       	IPSTACK_IFT_IEEE8023ADLAG
./replace.sh $1 IFT_L2VLAN                  IPSTACK_IFT_L2VLAN

./replace.sh $1 RTV_VALUE1                   IPSTACK_RTV_VALUE1



./replace.sh $1 in6_addr     ipstack_in6_addr
./replace.sh $1 in6_addr_t   ipstack_in6_addr_t
./replace.sh $1 ethaddr    ipstack_ethaddr
./replace.sh $1 iovec         ipstack_iovec


./replace.sh $1 in_addr       ipstack_in_addr


./replace.sh $1 sockaddr_in     ipstack_sockaddr_in
./replace.sh $1 sockaddr_in6    ipstack_sockaddr_in6
./replace.sh $1 sockaddr_dl     ipstack_sockaddr_dl
./replace.sh $1 sockaddr_ll     ipstack_sockaddr_ll
./replace.sh $1 sockaddr        ipstack_sockaddr
./replace.sh $1 sockaddr_un     ipstack_sockaddr_un



./replace.sh $1 IPV6_RTHDR_TYPE_0      IPSTACK_IPV6_RTHDR_TYPE_0

./replace.sh $1 IP6F_OFF_MASK          IPSTACK_IP6F_OFF_MASK
./replace.sh $1 IP6F_RESERVED_MASK     IPSTACK_IP6F_RESERVED_MASK
./replace.sh $1 IP6F_MORE_FRAG         IPSTACK_IP6F_MORE_FRAG


./replace.sh $1 IP6OPT_TYPE          IPSTACK_IP6OPT_TYPE
./replace.sh $1 IP6OPT_TYPE_SKIP         IPSTACK_IP6OPT_TYPE_SKIP
./replace.sh $1 IP6OPT_TYPE_DISCARD      IPSTACK_IP6OPT_TYPE_DISCARD
./replace.sh $1 IP6OPT_TYPE_FORCEICMP    IPSTACK_IP6OPT_TYPE_FORCEICMP
./replace.sh $1 IP6OPT_TYPE_ICMP         IPSTACK_IP6OPT_TYPE_ICMP
./replace.sh $1 IP6OPT_MUTABLE           IPSTACK_IP6OPT_MUTABLE
./replace.sh $1 IP6OPT_PAD1              IPSTACK_IP6OPT_PAD1
./replace.sh $1 IP6OPT_PADN              IPSTACK_IP6OPT_PADN
./replace.sh $1 IP6OPT_JUMBO             IPSTACK_IP6OPT_JUMBO
./replace.sh $1 IP6OPT_NSAP_ADDR         IPSTACK_IP6OPT_NSAP_ADDR
./replace.sh $1 IP6OPT_TUNNEL_LIMIT      IPSTACK_IP6OPT_TUNNEL_LIMIT
./replace.sh $1 IP6OPT_ROUTER_ALERT      IPSTACK_IP6OPT_ROUTER_ALERT

./replace.sh $1 IP6_ALERT_MLD            IPSTACK_IP6_ALERT_MLD
./replace.sh $1 IP6_ALERT_RSVP           IPSTACK_IP6_ALERT_RSVP
./replace.sh $1 IP6_ALERT_AN             IPSTACK_IP6_ALERT_AN


./replace.sh $1 ICMP6_FILTER_WILLPASS         IPSTACK_ICMP6_FILTER_WILLPASS
./replace.sh $1 ICMP6_FILTER_WILLBLOCK        IPSTACK_ICMP6_FILTER_WILLBLOCK
./replace.sh $1 ICMP6_FILTER_SETPASS          IPSTACK_ICMP6_FILTER_SETPASS
./replace.sh $1 ICMP6_FILTER_SETBLOCK         IPSTACK_ICMP6_FILTER_SETBLOCK
./replace.sh $1 ICMP6_FILTER_SETPASSALL    IPSTACK_ICMP6_FILTER_SETPASSALL
./replace.sh $1 ICMP6_FILTER_SETBLOCKALL   IPSTACK_ICMP6_FILTER_SETBLOCKALL


./replace.sh $1 msghdr        ipstack_msghdr

./replace.sh $1 MSG_TRUNC     IPSTACK_MSG_TRUNC
./replace.sh $1 MSG_CTRUNC    IPSTACK_MSG_CTRUNC


./replace.sh $1 cmsghdr        ipstack_cmsghdr

./replace.sh $1 CMSG_ALIGN          IPSTACK_CMSG_ALIGN
./replace.sh $1 CMSG_FIRSTHDR   IPSTACK_CMSG_FIRSTHDR
./replace.sh $1 CMSG_NXTHDR  IPSTACK_CMSG_NXTHDR
./replace.sh $1 CMSG_DATA       IPSTACK_CMSG_DATA
./replace.sh $1 CMSG_SPACE          IPSTACK_CMSG_SPACE
./replace.sh $1 CMSG_LEN            IPSTACK_CMSG_LEN


./replace.sh $1 ip_mreq             ipstack_ip_mreq
./replace.sh $1 ip_mreqn            ipstack_ip_mreqn

./replace.sh $1 arpreq              ipstack_arpreq
./replace.sh $1 ortentry            ipstack_ortentry
./replace.sh $1 aliasreq            ipstack_ifaliasreq
./replace.sh $1 ip6_mreq            ipstack_ipv6_mreq
./replace.sh $1 ipv6_mreq           ipstack_ipv6_mreq
./replace.sh $1 in6_pktinfo         ipstack_in6_pktinfo
./replace.sh $1 icmp6_filter        ipstack_icmp6_filter
./replace.sh $1 in6_addrlifetime    ipstack_in6_addrlifetime
./replace.sh $1 in6_aliasreq        ipstack_in6_aliasreq
./replace.sh $1 ether_header        ipstack_ether_header
./replace.sh $1 ether_arp           ipstack_ether_arp
./replace.sh $1 sockaddr_storage    ipstack_sockaddr_storage
./replace.sh $1 in_pktinfo      ipstack_in_pktinfo

./replace.sh $1 hostent         ipstack_hostent

./replace.sh $1 ARPHRD_ETHER             IPSTACK_ARPHRD_ETHER

./replace.sh $1 ifreq           ipstack_ifreq


./replace.sh $1 in6_ifreq       ipstack_in6_ifreq

./replace.sh $1 IN6_IFF_TENTATIVE   IPSTACK_IN6_IFF_TENTATIVE


./replace.sh $1 protoent   ipstack_protoent
./replace.sh $1 servent    ipstack_servent
./replace.sh $1 addrinfo   ipstack_addrinfo
./replace.sh $1 group_req   ipstack_group_req


./replace.sh $1 gethostbyname    ipstack_gethostbyname
./replace.sh $1 gethostbyname2   ipstack_gethostbyname2
./replace.sh $1 gethostbyaddr    ipstack_gethostbyaddr

./replace.sh $1 getprotobyname   ipstack_getprotobyname
./replace.sh $1 getprotobynumber ipstack_getprotobynumber

./replace.sh $1 getservbyname      ipstack_getservbyname
./replace.sh $1 getservbyport     ipstack_getservbyport


./replace.sh $1 getaddrinfo   ipstack_getaddrinfo
./replace.sh $1 freeaddrinfo            ipstack_freeaddrinfo
./replace.sh $1 getnameinfo              ipstack_getnameinfo

./replace.sh $1 socket ipstack_socket
./replace.sh $1 socketpair  ipstack_socketpair
./replace.sh $1 bind    ipstack_bind
./replace.sh $1 getsockname ipstack_getsockname
./replace.sh $1 connect ipstack_connect
./replace.sh $1 getpeername ipstack_getpeername
./replace.sh $1 send    ipstack_send
./replace.sh $1 recv    ipstack_recv
./replace.sh $1 sendto  ipstack_sendto
./replace.sh $1 recvfrom    ipstack_recvfrom
./replace.sh $1 sendmsg ipstack_sendmsg
./replace.sh $1 recvmsg ipstack_recvmsg
./replace.sh $1 getsockopt  ipstack_getsockopt
./replace.sh $1 setsockopt  ipstack_setsockopt
./replace.sh $1 listen  ipstack_listen
./replace.sh $1 accept  ipstack_accept
./replace.sh $1 shutdown    ipstack_shutdown
./replace.sh $1 errno    ipstack_errno

./replace.sh $1 nlmsghdr               	ipstack_nlmsghdr
./replace.sh $1 ifaddrmsg 				ipstack_ifaddrmsg
./replace.sh $1 rtmsg     				ipstack_rtmsg
./replace.sh $1 rtgenmsg  				ipstack_rtgenmsg
./replace.sh $1 sockaddr_nl   			ipstack_sockaddr_nl
./replace.sh $1 nlmsgerr      			ipstack_nlmsgerr
./replace.sh $1 rtattr        			ipstack_rtattr
./replace.sh $1 ifinfomsg     			ipstack_ifinfomsg
./replace.sh $1 ifa_cacheinfo 			ipstack_ifa_cacheinfo
./replace.sh $1 rtnexthop      			ipstack_rtnexthop

./replace.sh $1 ifconf                 ipstack_ifconf
./replace.sh $1 iphdr                  ipstack_iphdr

./replace.sh $1 ifaddrs   ipstack_ifaddrs
./replace.sh $1 getifaddrs  ipstack_getifaddrs
./replace.sh $1 freeifaddrs ipstack_freeifaddrs

./replace.sh $1 EDESTADDRREQ       IPSTACK_ERRNO_EDESTADDRREQ
./replace.sh $1 ENETUNREACH        IPSTACK_ERRNO_ENETUNREACH
./replace.sh $1 ENETRESET          IPSTACK_ERRNO_ENETRESET
./replace.sh $1 ECONNABORTED       IPSTACK_ERRNO_ECONNABORTED
./replace.sh $1 ECONNRESET         IPSTACK_ERRNO_ECONNRESET
./replace.sh $1 ENOBUFS            IPSTACK_ERRNO_ENOBUFS
./replace.sh $1 EISCONN            IPSTACK_ERRNO_EISCONN
./replace.sh $1 ENOTCONN           IPSTACK_ERRNO_ENOTCONN
./replace.sh $1 ESHUTDOWN          IPSTACK_ERRNO_ESHUTDOWN
./replace.sh $1 ETOOMANYREFS       IPSTACK_ERRNO_ETOOMANYREFS
./replace.sh $1 ECONNREFUSED       IPSTACK_ERRNO_ECONNREFUSED
./replace.sh $1 ENETDOWN           IPSTACK_ERRNO_ENETDOWN
./replace.sh $1 EHOSTUNREACH       IPSTACK_ERRNO_EHOSTUNREACH
./replace.sh $1 EINPROGRESS        IPSTACK_ERRNO_EINPROGRESS
./replace.sh $1 EALREADY           IPSTACK_ERRNO_EALREADY
./replace.sh $1 EINVAL             IPSTACK_ERRNO_EINVAL
./replace.sh $1 EHOSTDOWN          IPSTACK_ERRNO_EHOSTDOWN
./replace.sh $1 ETIMEDOUT          IPSTACK_ERRNO_ETIMEDOUT
./replace.sh $1 ETIME              IPSTACK_ERRNO_ETIMEDOUT
./replace.sh $1 EADDRINUSE         IPSTACK_ERRNO_EADDRINUSE
./replace.sh $1 EOPNOTSUPP         IPSTACK_ERRNO_EOPNOTSUPP

./replace.sh $1 EIO                            IPSTACK_ERRNO_EIO
./replace.sh $1 E2BIG                          IPSTACK_ERRNO_E2BIG
./replace.sh $1 ENOENT                         IPSTACK_ERRNO_ENOENT

./replace.sh $1 EINTR       IPSTACK_ERRNO_EINTR
./replace.sh $1 EWOULDBLOCK IPSTACK_ERRNO_EWOULDBLOCK
./replace.sh $1 EAGAIN      IPSTACK_ERRNO_EAGAIN
./replace.sh $1 ENODEV      IPSTACK_ERRNO_ENODEV
./replace.sh $1 ESRCH       IPSTACK_ERRNO_ESRCH
./replace.sh $1 EEXIST      IPSTACK_ERRNO_EEXIST

./replace.sh $1 ENOMEM     	IPSTACK_ERRNO_ENOMEM
./replace.sh $1 EBUSY        	IPSTACK_ERRNO_EBUSY
./replace.sh $1 EFAULT       	IPSTACK_ERRNO_EFAULT
