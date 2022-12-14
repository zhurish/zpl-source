/* Auto-generated from route_types.txt by . */
/* Do not edit! */

#ifndef _ZPL_ROUTE_TYPES_H
#define _ZPL_ROUTE_TYPES_H


#define ROUTE_DEFINE_DESC_TABLE

/* ZPL route's types. */
#define ZPL_ROUTE_PROTO_SYSTEM           0
#define ZPL_ROUTE_PROTO_KERNEL           1
#define ZPL_ROUTE_PROTO_CONNECT          2
#define ZPL_ROUTE_PROTO_STATIC           3
#define ZPL_ROUTE_PROTO_RIP              4
#define ZPL_ROUTE_PROTO_RIPNG            5
#define ZPL_ROUTE_PROTO_OSPF             6
#define ZPL_ROUTE_PROTO_OSPF6            7
#define ZPL_ROUTE_PROTO_ISIS             8
#define ZPL_ROUTE_PROTO_BGP              9
#define ZPL_ROUTE_PROTO_PIM              10
#define ZPL_ROUTE_PROTO_HSLS             11
#define ZPL_ROUTE_PROTO_OLSR             12
#define ZPL_ROUTE_PROTO_BABEL            13
#define ZPL_ROUTE_PROTO_NHRP             14
#define ZPL_ROUTE_PROTO_VRRP             15
#define ZPL_ROUTE_PROTO_FRP              16
#define ZPL_ROUTE_PROTO_LLDP             17
#define ZPL_ROUTE_PROTO_BFD              18
#define ZPL_ROUTE_PROTO_LDP              19
#define ZPL_ROUTE_PROTO_DHCP             20
#define ZPL_ROUTE_PROTO_UTILS            21
#define ZPL_ROUTE_PROTO_MANAGE           22
#define ZPL_ROUTE_PROTO_SWITCH           23
#define ZPL_ROUTE_PROTO_MAX              24

#define SHOW_ROUTE_V4_HEADER \
  "Codes: K - kernel route, C - connected, S - static, R - RIP,%s" \
  "       O - OSPF, I - IS-IS, B - BGP, P - PIM, o - OLSR, A - Babel,%s" \
  "       N - NHRP, F - FRP,%s" \
  "       > - selected route, * - FIB route%s%s", \
  VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE
#define SHOW_ROUTE_V6_HEADER \
  "Codes: K - kernel route, C - connected, S - static, R - RIPng,%s" \
  "       O - OSPFv6, I - IS-IS, B - BGP, A - Babel, N - NHRP,%s" \
  "       > - selected route, * - FIB route%s%s", \
  VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE

/* babeld */
#define ROUTE_REDIST_STR_BABELD \
  "(kernel|connected|static|rip|ripng|ospf|ospf6|isis|bgp|pim|olsr|nhrp|frp)"
#define ROUTE_REDIST_HELP_STR_BABELD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"
#define ROUTE_IP_REDIST_STR_BABELD \
  "(kernel|connected|static|rip|ospf|isis|bgp|pim|olsr|nhrp|frp)"
#define ROUTE_IP_REDIST_HELP_STR_BABELD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"
#define ROUTE_IP6_REDIST_STR_BABELD \
  "(kernel|connected|static|ripng|ospf6|isis|bgp|nhrp)"
#define ROUTE_IP6_REDIST_HELP_STR_BABELD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n"

/* bgpd */
#define ROUTE_REDIST_STR_BGPD \
  "(kernel|connected|static|rip|ripng|ospf|ospf6|isis|pim|olsr|babel|nhrp|frp)"
#define ROUTE_REDIST_HELP_STR_BGPD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"
#define ROUTE_IP_REDIST_STR_BGPD \
  "(kernel|connected|static|rip|ospf|isis|pim|olsr|babel|nhrp|frp)"
#define ROUTE_IP_REDIST_HELP_STR_BGPD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"
#define ROUTE_IP6_REDIST_STR_BGPD \
  "(kernel|connected|static|ripng|ospf6|isis|babel|nhrp)"
#define ROUTE_IP6_REDIST_HELP_STR_BGPD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n"

/* frpd */
#define ROUTE_REDIST_STR_FRPD \
  "(kernel|connected|static|rip|ospf|isis|bgp|pim|olsr|babel|nhrp)"
#define ROUTE_REDIST_HELP_STR_FRPD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n"

/* isisd */
#define ROUTE_REDIST_STR_ISISD \
  "(kernel|connected|static|rip|ripng|ospf|ospf6|bgp|pim|olsr|babel|nhrp|frp)"
#define ROUTE_REDIST_HELP_STR_ISISD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"
#define ROUTE_IP_REDIST_STR_ISISD \
  "(kernel|connected|static|rip|ospf|bgp|pim|olsr|babel|nhrp|frp)"
#define ROUTE_IP_REDIST_HELP_STR_ISISD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"
#define ROUTE_IP6_REDIST_STR_ISISD \
  "(kernel|connected|static|ripng|ospf6|bgp|babel|nhrp)"
#define ROUTE_IP6_REDIST_HELP_STR_ISISD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n"

/* nhrpd */
#define ROUTE_REDIST_STR_NHRPD \
  "(kernel|connected|static|rip|ripng|ospf|ospf6|isis|bgp|pim|olsr|babel|frp)"
#define ROUTE_REDIST_HELP_STR_NHRPD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Babel routing protocol (Babel)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"
#define ROUTE_IP_REDIST_STR_NHRPD \
  "(kernel|connected|static|rip|ospf|isis|bgp|pim|olsr|babel|frp)"
#define ROUTE_IP_REDIST_HELP_STR_NHRPD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Babel routing protocol (Babel)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"
#define ROUTE_IP6_REDIST_STR_NHRPD \
  "(kernel|connected|static|ripng|ospf6|isis|bgp|babel)"
#define ROUTE_IP6_REDIST_HELP_STR_NHRPD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n"

/* nsm */
#define ROUTE_REDIST_STR_NSM \
  "(kernel|connected|static|rip|ripng|ospf|ospf6|isis|bgp|pim|olsr|babel|nhrp|frp)"
#define ROUTE_REDIST_HELP_STR_NSM \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"
#define ROUTE_IP_REDIST_STR_NSM \
  "(kernel|connected|static|rip|ospf|isis|bgp|pim|olsr|babel|nhrp|frp)"
#define ROUTE_IP_REDIST_HELP_STR_NSM \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"
#define ROUTE_IP6_REDIST_STR_NSM \
  "(kernel|connected|static|ripng|ospf6|isis|bgp|babel|nhrp)"
#define ROUTE_IP6_REDIST_HELP_STR_NSM \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n"

/* olsrd */
#define ROUTE_REDIST_STR_OLSRD \
  "(kernel|connected|static|rip|ospf|isis|bgp|pim|babel|nhrp|frp)"
#define ROUTE_REDIST_HELP_STR_OLSRD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"

/* ospf6d */
#define ROUTE_REDIST_STR_OSPF6D \
  "(kernel|connected|static|ripng|isis|bgp|babel|nhrp)"
#define ROUTE_REDIST_HELP_STR_OSPF6D \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n"

/* ospfd */
#define ROUTE_REDIST_STR_OSPFD \
  "(kernel|connected|static|rip|isis|bgp|pim|olsr|babel|nhrp|frp)"
#define ROUTE_REDIST_HELP_STR_OSPFD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"

/* pimd */
#define ROUTE_REDIST_STR_PIMD \
  "(kernel|connected|static|rip|ospf|isis|bgp|olsr|babel|nhrp|frp)"
#define ROUTE_REDIST_HELP_STR_PIMD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"

/* ripd */
#define ROUTE_REDIST_STR_RIPD \
  "(kernel|connected|static|ospf|isis|bgp|pim|olsr|babel|nhrp|frp)"
#define ROUTE_REDIST_HELP_STR_RIPD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Protocol Independent Multicast (PIM)\n" \
  "Optimised Link State Routing (OLSR)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Fast and Reliable Routing Protocol (FRP)\n"

/* ripngd */
#define ROUTE_REDIST_STR_RIPNGD \
  "(kernel|connected|static|ospf6|isis|bgp|babel|nhrp)"
#define ROUTE_REDIST_HELP_STR_RIPNGD \
  "Kernel routes (not installed via the nsm RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n" \
  "Next Hop Resolution Protocol (NHRP)\n"


#ifdef ROUTE_DEFINE_DESC_TABLE

struct rttype_desc_table
{
  unsigned int type;
  const char *string;
  char chr;
};

#define DESC_ENTRY(T,S,C) [(T)] = { (T), (S), (C) }
static const struct rttype_desc_table route_types[] = {
  DESC_ENTRY	(ZPL_ROUTE_PROTO_SYSTEM,	 "system",	'X' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_KERNEL,	 "kernel",	'K' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_CONNECT,	 "connected",	'C' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_STATIC,	 "static",	'S' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_RIP,	 "rip",	'R' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_RIPNG,	 "ripng",	'R' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_OSPF,	 "ospf",	'O' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_OSPF6,	 "ospf6",	'O' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_ISIS,	 "isis",	'I' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_BGP,	 "bgp",	'B' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_PIM,	 "pim",	'P' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_HSLS,	 "hsls",	'H' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_OLSR,	 "olsr",	'o' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_BABEL,	 "babel",	'A' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_NHRP,	 "nhrp",	'N' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_VRRP,	 "vrrp",	'v' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_FRP,	 "frp",	'F' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_LLDP,	 "lldp",	'L' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_BFD,	 "bfd",	'B' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_LDP,	 "ldp",	'l' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_DHCP,	 "dhcp",	'D' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_UTILS,	 "utils",	'u' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_MANAGE,	 "manage",	'm' ),
  DESC_ENTRY	(ZPL_ROUTE_PROTO_SWITCH,	 "switch",	's' ),
};
#undef DESC_ENTRY

#endif /* ROUTE_DEFINE_DESC_TABLE */

#endif /* _ZPL_ROUTE_TYPES_H */
