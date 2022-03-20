#ifndef __CLI_HELPER_H__
#define __CLI_HELPER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_SHELL_MODULE


/*
 * Sometimes #defines create maximum values that
 * need to have strings created from them that
 * allow the parser to match against them.
 * These macros allow that.
 */
#define CMD_CREATE_STR(s)  CMD_CREATE_STR_HELPER(s)
#define CMD_CREATE_STR_HELPER(s) #s
#define CMD_RANGE_STR(a,s) "<" CMD_CREATE_STR(a) "-" CMD_CREATE_STR(s) ">"


/* Common descriptions. */
#define SHOW_STR "Show running system information\n"
#define IP_STR "IP information\n"
#define IPV6_STR "IPv6 information\n"
#define NO_STR "Negate a command or set its defaults\n"
#define REDIST_STR "Redistribute information from another routing protocol\n"
#define CLEAR_STR "Reset functions\n"
#define RIP_STR "RIP information\n"
#define BGP_STR "BGP information\n"
#define BGP_SOFT_STR "Soft reconfig inbound and outbound updates\n"
#define BGP_SOFT_IN_STR "Send route-refresh unless using 'soft-reconfiguration inbound'\n"
#define BGP_SOFT_OUT_STR "Resend all outbound updates\n"
#define BGP_SOFT_RSCLIENT_RIB_STR "Soft reconfig for rsclient RIB\n"
#define OSPF_STR "OSPF information\n"
#define NEIGHBOR_STR "Specify neighbor router\n"
#define DEBUG_STR "Debugging functions (see also 'undebug')\n"
#define UNDEBUG_STR "Disable debugging functions (see also 'debug')\n"
#define ROUTER_STR "Enable a routing process\n"
#define AS_STR "AS number\n"
#define MBGP_STR "MBGP information\n"
#define MATCH_STR "Match values from routing table\n"
#define SET_STR "Set values in destination routing protocol\n"
#define OUT_STR "Filter outgoing routing updates\n"
#define IN_STR  "Filter incoming routing updates\n"
#define V4NOTATION_STR "specify by IPv4 address notation(e.g. 0.0.0.0)\n"
#define OSPF6_NUMBER_STR "Specify by number\n"
#define INTERFACE_STR "Interface information\n"
#define IFNAME_STR "Interface name(e.g. eth0/1/1)\n"
#define IP6_STR "IPv6 Information\n"
#define OSPF6_STR "Open Shortest Path First (OSPF) for IPv6\n"
#define OSPF6_ROUTER_STR "Enable a routing process\n"
#define OSPF6_INSTANCE_STR "<1-65535> Instance ID\n"
#define SECONDS_STR "<1-65535> Seconds\n"
#define ROUTE_STR "Routing Table\n"
#define PREFIX_LIST_STR "Build a prefix list\n"
#define OSPF6_DUMP_TYPE_LIST \
"(neighbor|interface|area|lsa|zebra|config|dbex|spf|route|lsdb|redistribute|hook|asbr|prefix|abr)"
#define ISIS_STR "IS-IS information\n"
#define AREA_TAG_STR "[area tag]\n"
#define MPLS_TE_STR "MPLS-TE specific commands\n"
#define LINK_PARAMS_STR "Configure interface link parameters\n"
#define OSPF_RI_STR "OSPF Router Information specific commands\n"
#define PCE_STR "PCE Router Information specific commands\n"


#define CMD_KEY_IPV4		"A.B.C.D"
#define CMD_KEY_IPV4_PREFIX	"A.B.C.D/M"
#define CMD_KEY_IPV6		"X:X::X:X"
#define CMD_KEY_IPV6_PREFIX	"X:X::X:X/M"
#define CMD_KEY_IPV4_HELP	"Specify IPv4 address notation(e.g. 0.0.0.0)\n"

#define CMD_INTERFACE_STR	"interface"
#define CMD_INTERFACE_STR_HELP	INTERFACE_STR

#define CMD_IF_USPV_STR		"(serial|ethernet|gigabitethernet|wireless|tunnel|brigde)"
#define CMD_IF_USPV_STR_HELP	"Serial interface\n Ethernet interface\n" \
								"GigabitEthernet interface\n" \
								"Wireless interface\n"\
								"Tunnel interface\n Brigde interface\n"

#define CMD_IF_SUB_USPV_STR		"(ethernet|gigabitethernet)"
#define CMD_IF_SUB_USPV_STR_HELP	"Ethernet interface\n" \
								"GigabitEthernet interface\n"

#define CMD_USP_STR			"<unit/slot/port>"
#define CMD_USP_STR_HELP	"specify interface name:<unit/slot/port> (e.g. 0/1/3)\n"

#define CMD_USP_SUB_STR			"<unit/slot/port.id>"
#define CMD_USP_SUB_STR_HELP	"specify interface name:<unit/slot/port.id> (e.g. 0/1/3.1)\n"

#define CMD_USP_RANGE_STR			"<unit/slot/port-end>"
#define CMD_USP_RANGE_STR_HELP	"specify interface name:<unit/slot/port> (e.g. 0/1/3-6)\n"

#define CMD_USP_SUB_RANGE_STR			"<unit/slot/port.id-id>"
#define CMD_USP_SUB_RANGE_STR_HELP	"specify interface name:<unit/slot/port.id-id> (e.g. 0/1/3.1-4)\n"


#ifdef CUSTOM_INTERFACE
#define CMD_IF_MIP_STR		"(modem|wifi)"
#define CMD_IF_MIP_STR_HELP	"Modem interface\n" "Wifi interface\n"
#endif


#define CMD_IF_VLAN_STR			"vlan <1-4094>"
#define CMD_IF_VLAN_STR_HELP	"Super Vlan interface\n" \
								"VLAN ID\n"

#define CMD_IF_LOOP_STR			"loopback <1-1024>"
#define CMD_IF_LOOP_STR_HELP	"Loopback interface\n" \
								"Interface ID\n"

#define CMD_IF_TRUNK_STR		"port-channel <1-64>"
#define CMD_IF_TRUNK_STR_HELP	"Ethernet Channel of interface\n" \
								"Channel ID\n"



#define CMD_MAC_ADDRESS_STR	"mac-address-table"
#define CMD_MAC_ADDRESS_STR_HELP	"mac-address-table\n"

#define CMD_MAC_ADDRESS_LEARN_STR	"mac-address learning"
#define CMD_MAC_ADDRESS_LEARN_STR_HELP	"Mac Address\nMac Address Learning\n"

#define CMD_MAC_STR			"HHHH-HHHH-HHHH"
#define CMD_MAC_STR_HELP	"specify MAC address:HHHH-HHHH-HHHH (e.g. 0012-2234-5631)\n"

#define CMD_FORWARD_STR			"forward"
#define CMD_FORWARD_STR_HELP	"forward\n"

#define CMD_DISCARD_STR			"discard"
#define CMD_DISCARD_STR_HELP	"discard\n"

#define CMD_VLAN_DATABASE_STR		"vlan-database"
#define CMD_VLAN_STR_DATABASE_HELP	"Vlan database\n"
#define CMD_VLAN_STR		"vlan <2-4094>"
#define CMD_VLAN_STR_HELP	"Vlan information\nVlan ID\n"

#define CMD_AGEING_TIME_STR			"ageing-time"
#define CMD_AGEING_TIME_STR_HELP	"ageing-time\n"

#define CMD_ARP_STR			"ip arp"
#define CMD_ARP_STR_HELP	IP_STR"arp\n"


/* IPv4 only machine should not accept IPv6 address for peer's IP
   address.  So we replace VTY command string like below. */
#ifdef HAVE_IPV6
#define NEIGHBOR_CMD       "neighbor (A.B.C.D|X:X::X:X) "
#define NO_NEIGHBOR_CMD    "no neighbor (A.B.C.D|X:X::X:X) "
#define NEIGHBOR_ADDR_STR  "Neighbor address\nIPv6 address\n"
#define NEIGHBOR_CMD2      "neighbor (A.B.C.D|X:X::X:X|WORD) "
#define NO_NEIGHBOR_CMD2   "no neighbor (A.B.C.D|X:X::X:X|WORD) "
#define NEIGHBOR_ADDR_STR2 "Neighbor address\nNeighbor IPv6 address\nNeighbor tag\n"
#else
#define NEIGHBOR_CMD       "neighbor A.B.C.D "
#define NO_NEIGHBOR_CMD    "no neighbor A.B.C.D "
#define NEIGHBOR_ADDR_STR  "Neighbor address\n"
#define NEIGHBOR_CMD2      "neighbor (A.B.C.D|WORD) "
#define NO_NEIGHBOR_CMD2   "no neighbor (A.B.C.D|WORD) "
#define NEIGHBOR_ADDR_STR2 "Neighbor address\nNeighbor tag\n"
#endif /* HAVE_IPV6 */


#define IP_IPPROTO_CMD			"ip|icmp|igmp|ipip|egp|pup|idp|tp|dccp|ipv6|rsvp|gre|esp|ah|mtp|beetph|encap|pim|comp|sctp|udplite|mpls|raw|ospf|<0-5>|<7-16>|<18-255>"
#define IP_IPPROTO_HELP			"Dummy protocol for TCP\nInternet Control Message Protocol\nInternet Group Management Protocol\n" \
									"IPIP tunnels (older KA9Q tunnels use 94)\n" \
									"Exterior Gateway Protocol\nPUP protocol\nXNS IDP protocol\n" \
									"SO Transport Protocol Class 4\nDatagram Congestion Control Protocol\nIPv6 Protocol\n" \
									"Reservation Protocol\nGeneral Routing Encapsulation\nEncapsulating security payload\n" \
									"Authentication header\nMulticast Transport Protocol\nIP option pseudo header for BEET\n" \
									"Encapsulation Header\nProtocol Independent Multicast\nCompression Header Protocol\n" \
									"Stream Control Transmission Protocol\nUDP-Lite protocol\nMPLS in IP\nRaw IP packets\n" \
									"Ospf Protocol\nProtocol HEX Value\nProtocol HEX Value\nProtocol HEX Value\n"


#define IP_TCPUDP_CMD       "tcp|udp"
#define IP_TCPUDP_HELP       "Transmission Control Protocol\n" \
							 "User Datagram Protocol\n"

#endif /* ZPL_SHELL_MODULE */ 
#ifdef __cplusplus
}
#endif

#endif /* __CLI_HELPER_H__ */
