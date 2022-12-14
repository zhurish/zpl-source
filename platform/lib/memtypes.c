/*
 * Memory type definitions. This file is parsed by memtypes.awk to extract
 * MTYPE_ and memory_list_.. information in order to autogenerate 
 * memtypes.h.
 *
 * The script is sensitive to the format (though not whitespace), see
 * the top of memtypes.awk for more details.
 */
#include "zebra.h"
#include "memory.h"


struct memory_list memory_list_lib[] =
{
  { MTYPE_TMP,			"Temporary memory"		},
  { MTYPE_STRVEC,		"String vector"			},
  { MTYPE_VECTOR,		"Vector"			},
  { MTYPE_VECTOR_INDEX,		"Vector index"			},
  { MTYPE_LINK_LIST,		"Link List"			},
  { MTYPE_LINK_NODE,		"Link Node"			},
  { MTYPE_THREAD,		"Thread"			},
  { MTYPE_THREAD_MASTER,	"Thread master"			},
  { MTYPE_THREAD_STATS,		"Thread stats"			},
  { MTYPE_ELOOP,		"Eloop"			},
  { MTYPE_ELOOP_MASTER,	"Eloop master"			},
  { MTYPE_ELOOP_STATS,		"Eloop stats"			},
  { MTYPE_EPOLL,		"Epoll"			},
  { MTYPE_EPOLL_MASTER,	"Epoll master"			},
  { MTYPE_EPOLL_STATS,		"Epoll stats"			},
  { MTYPE_EVENT,		"Event"			},
  { MTYPE_EVENT_INFO,		"Event info"			},
  { MTYPE_NSM,		"NSM"			},
  { MTYPE_NSM_INFO,		"NSM info"			},
  { MTYPE_NSM_CLIENT,		"NSM Client info"			},
  { MTYPE_VTY,			"VTY"				},
  { MTYPE_VTY_OUT_BUF,		"VTY output buffer"		},
  { MTYPE_VTY_HIST,		"VTY history"			},
  { MTYPE_VTY_HOOK,			"VTY show hook"			},
  { MTYPE_IF,			"Interface"			},
  { MTYPE_IF_DESC,			"Interface destination"	},
  { MTYPE_IF_HOOK,			"Interface Hook"			},
  { MTYPE_SECURITY,			"Security information"			},
  { MTYPE_CONNECTED,		"Connected" 			},
  { MTYPE_CONNECTED_LABEL,	"Connected interface label"	},
  { MTYPE_BUFFER,		"Buffer"			},
  { MTYPE_BUFFER_DATA,		"Buffer data"			},
  { MTYPE_STREAM,		"Stream"			},
  { MTYPE_STREAM_DATA,		"Stream data"			},
  { MTYPE_STREAM_FIFO,		"Stream FIFO"			},
  { MTYPE_DATA,				"Data information"			},
  { MTYPE_LOG,				"Log information"			},
  { MTYPE_LOG_DATA,				"Log data"			},
  { MTYPE_PREFIX,		"Prefix"			},
  { MTYPE_PREFIX_IPV4,		"Prefix IPv4"			},
  { MTYPE_PREFIX_IPV6,		"Prefix IPv6"			},
  { MTYPE_HASH,			"Hash"				},
  { MTYPE_HASH_BACKET,		"Hash Bucket"			},
  { MTYPE_HASH_INDEX,		"Hash Index"			},
  { MTYPE_ROUTE_TABLE,		"Route table"			},
  { MTYPE_ROUTE_NODE,		"Route node"			},
  { MTYPE_DISTRIBUTE,		"Distribute list"		},
  { MTYPE_DISTRIBUTE_IFNAME,	"Dist-list ifname"		},
  { MTYPE_ACCESS_LIST,		"Access List"			},
  { MTYPE_ACCESS_LIST_STR,	"Access List Str"		},
  { MTYPE_ACCESS_FILTER,	"Access Filter"			},
  { MTYPE_PREFIX_LIST,		"Prefix List"			},
  { MTYPE_PREFIX_LIST_ENTRY,	"Prefix List Entry"		},
  { MTYPE_PREFIX_LIST_STR,	"Prefix List Str"		},
  { MTYPE_ROUTE_MAP,		"Route map"			},
  { MTYPE_ROUTE_MAP_NAME,	"Route map name"		},
  { MTYPE_ROUTE_MAP_INDEX,	"Route map index"		},
  { MTYPE_ROUTE_MAP_RULE,	"Route map rule"		},
  { MTYPE_ROUTE_MAP_RULE_STR,	"Route map rule str"		},
  { MTYPE_ROUTE_MAP_COMPILED,	"Route map compiled"		},
  { MTYPE_CMD_TOKENS,		"Command desc"			},
  { MTYPE_KEY,			"Key"				},
  { MTYPE_KEYCHAIN,		"Key chain"			},
  { MTYPE_IF_RMAP,		"Interface route map"		},
  { MTYPE_IF_RMAP_NAME,		"I.f. route map name",		},
  { MTYPE_SOCKUNION,		"Socket union"			},
  { MTYPE_PRIVS,		"Privilege information"		},
  { MTYPE_ZLOG,			"Logging"			},
  { MTYPE_ZCLIENT,		"Zclient"			},
  { MTYPE_WORK_QUEUE,		"Work queue"			},
  { MTYPE_WORK_QUEUE_ITEM,	"Work queue item"		},
  { MTYPE_WORK_QUEUE_NAME,	"Work queue name string"	},
  { MTYPE_PQUEUE,		"Priority queue"		},
  { MTYPE_PQUEUE_DATA,		"Priority queue data"		},
  { MTYPE_HOST,			"Host config"			},
  { MTYPE_VRF,			"VRF"				},
  { MTYPE_VRF_NAME,		"VRF name"			},
  { MTYPE_VRF_BITMAP,		"VRF bit-map"			},
  { MTYPE_IF_LINK_PARAMS,       "Informational Link Parameters" },
  { -1, NULL },
};

struct memory_list memory_list_zebra[] = 
{
  { MTYPE_RTADV_PREFIX,		"Router Advertisement Prefix"	},
  { MTYPE_ZEBRA_VRF,		"ZEBRA VRF"				},
  { MTYPE_NEXTHOP,		"Nexthop"			},
  { MTYPE_RIB,			"RIB"				},
  { MTYPE_RIB_QUEUE,		"RIB process work queue"	},
  { MTYPE_STATIC_ROUTE,		"Static route"			},
  { MTYPE_RIB_DEST,		"RIB destination"		},
  { MTYPE_RIB_TABLE_INFO,	"RIB table info"		},
  { MTYPE_NETLINK_NAME,	"Netlink name"			},
  { MTYPE_RNH,		        "Nexthop tracking object"	},
  { MTYPE_IP_DNS,			"IP DNS"				},
  { MTYPE_IP_HOST,			"IP HOST"				},
  { -1, NULL },
};

struct memory_list memory_list_bgp[] =
{
  { MTYPE_BGP,			"BGP instance"			},
  { MTYPE_BGP_LISTENER,		"BGP listen socket details"	},
  { MTYPE_BGP_PEER,		"BGP peer"			},
  { MTYPE_BGP_PEER_HOST,	"BGP peer hostname"		},
  { MTYPE_PEER_GROUP,		"Peer group"			},
  { MTYPE_PEER_DESC,		"Peer description"		},
  { MTYPE_PEER_PASSWORD,	"Peer password string"		},
  { MTYPE_ATTR,			"BGP attribute"			},
  { MTYPE_ATTR_EXTRA,		"BGP extra attributes"		},
  { MTYPE_AS_PATH,		"BGP aspath"			},
  { MTYPE_AS_SEG,		"BGP aspath seg"		},
  { MTYPE_AS_SEG_DATA,		"BGP aspath segment data"	},
  { MTYPE_AS_STR,		"BGP aspath str"		},
  { 0, NULL },
  { MTYPE_BGP_TABLE,		"BGP table"			},
  { MTYPE_BGP_NODE,		"BGP node"			},
  { MTYPE_BGP_ROUTE,		"BGP route"			},
  { MTYPE_BGP_ROUTE_EXTRA,	"BGP ancillary route info"	},
  { MTYPE_BGP_CONN,		"BGP connected"			},
  { MTYPE_BGP_STATIC,		"BGP static"			},
  { MTYPE_BGP_ADVERTISE_ATTR,	"BGP adv attr"			},
  { MTYPE_BGP_ADVERTISE,	"BGP adv"			},
  { MTYPE_BGP_SYNCHRONISE,	"BGP synchronise"		},
  { MTYPE_BGP_ADJ_IN,		"BGP adj in"			},
  { MTYPE_BGP_ADJ_OUT,		"BGP adj out"			},
  { MTYPE_BGP_MPATH_INFO,	"BGP multipath info"		},
  { 0, NULL },
  { MTYPE_AS_LIST,		"BGP AS list"			},
  { MTYPE_AS_FILTER,		"BGP AS filter"			},
  { MTYPE_AS_FILTER_STR,	"BGP AS filter str"		},
  { 0, NULL },
  { MTYPE_COMMUNITY,		"community"			},
  { MTYPE_COMMUNITY_VAL,	"community val"			},
  { MTYPE_COMMUNITY_STR,	"community str"			},
  { 0, NULL },
  { MTYPE_ECOMMUNITY,		"extcommunity"			},
  { MTYPE_ECOMMUNITY_VAL,	"extcommunity val"		},
  { MTYPE_ECOMMUNITY_STR,	"extcommunity str"		},
  { 0, NULL },
  { MTYPE_COMMUNITY_LIST,	"community-list"		},
  { MTYPE_COMMUNITY_LIST_NAME,	"community-list name"		},
  { MTYPE_COMMUNITY_LIST_ENTRY,	"community-list entry"		},
  { MTYPE_COMMUNITY_LIST_CONFIG,  "community-list config"	},
  { MTYPE_COMMUNITY_LIST_HANDLER, "community-list handler"	},
  { 0, NULL },
  { MTYPE_CLUSTER,		"Cluster list"			},
  { MTYPE_CLUSTER_VAL,		"Cluster list val"		},
  { 0, NULL },
  { MTYPE_BGP_PROCESS_QUEUE,	"BGP Process queue"		},
  { MTYPE_BGP_CLEAR_NODE_QUEUE, "BGP node clear queue"		},
  { 0, NULL },
  { MTYPE_TRANSIT,		"BGP transit attr"		},
  { MTYPE_TRANSIT_VAL,		"BGP transit val"		},
  { 0, NULL },
  { MTYPE_BGP_DISTANCE,		"BGP distance"			},
  { MTYPE_BGP_NEXTHOP_CACHE,	"BGP nexthop"			},
  { MTYPE_BGP_CONFED_LIST,	"BGP confed list"		},
  { MTYPE_PEER_UPDATE_SOURCE,	"BGP peer update interface"	},
  { MTYPE_BGP_DAMP_INFO,	"Dampening info"		},
  { MTYPE_BGP_DAMP_ARRAY,	"BGP Dampening array"		},
  { MTYPE_BGP_REGEXP,		"BGP regexp"			},
  { MTYPE_BGP_AGGREGATE,	"BGP aggregate"			},
  { MTYPE_BGP_ADDR,		"BGP own address"		},
  { MTYPE_ENCAP_TLV,		"ENCAP TLV",			},
  { MTYPE_LCOMMUNITY,           "Large Community",              },
  { MTYPE_LCOMMUNITY_STR,       "Large Community str",          },
  { MTYPE_LCOMMUNITY_VAL,       "Large Community val",          },
  { -1, NULL }
};

struct memory_list memory_list_rip[] =
{
  { MTYPE_RIP,                "RIP structure"			},
  { MTYPE_RIP_INFO,           "RIP route info"			},
  { MTYPE_RIP_INTERFACE,      "RIP interface"			},
  { MTYPE_RIP_PEER,           "RIP peer"			},
  { MTYPE_RIP_OFFSET_LIST,    "RIP offset list"			},
  { MTYPE_RIP_DISTANCE,       "RIP distance"			},
  { -1, NULL }
};

struct memory_list memory_list_ripng[] =
{
  { MTYPE_RIPNG,              "RIPng structure"			},
  { MTYPE_RIPNG_ROUTE,        "RIPng route info"		},
  { MTYPE_RIPNG_AGGREGATE,    "RIPng aggregate"			},
  { MTYPE_RIPNG_PEER,         "RIPng peer"			},
  { MTYPE_RIPNG_OFFSET_LIST,  "RIPng offset lst"		},
  { MTYPE_RIPNG_RTE_DATA,     "RIPng rte data"			},
  { -1, NULL }
};

struct memory_list memory_list_babel[] =
{
  { MTYPE_BABEL,              "Babel structure"			},
  { MTYPE_BABEL_IF,           "Babel interface"			},
  { -1, NULL }
};

struct memory_list memory_list_ospf[] =
{
  { MTYPE_OSPF_TOP,           "OSPF top"			},
  { MTYPE_OSPF_AREA,          "OSPF area"			},
  { MTYPE_OSPF_AREA_RANGE,    "OSPF area range"			},
  { MTYPE_OSPF_NETWORK,       "OSPF network"			},
  { MTYPE_OSPF_NEIGHBOR_STATIC,"OSPF static nbr"		},
  { MTYPE_OSPF_IF,            "OSPF interface"			},
  { MTYPE_OSPF_NEIGHBOR,      "OSPF neighbor"			},
  { MTYPE_OSPF_ROUTE,         "OSPF route"			},
  { MTYPE_OSPF_TMP,           "OSPF tmp mem"			},
  { MTYPE_OSPF_LSA,           "OSPF LSA"			},
  { MTYPE_OSPF_LSA_DATA,      "OSPF LSA data"			},
  { MTYPE_OSPF_LSDB,          "OSPF LSDB"			},
  { MTYPE_OSPF_PACKET,        "OSPF packet"			},
  { MTYPE_OSPF_FIFO,          "OSPF FIFO queue"			},
  { MTYPE_OSPF_VERTEX,        "OSPF vertex"			},
  { MTYPE_OSPF_VERTEX_PARENT, "OSPF vertex parent",		},
  { MTYPE_OSPF_NEXTHOP,       "OSPF nexthop"			},
  { MTYPE_OSPF_PATH,	      "OSPF path"			},
  { MTYPE_OSPF_VL_DATA,       "OSPF VL data"			},
  { MTYPE_OSPF_CRYPT_KEY,     "OSPF crypt key"			},
  { MTYPE_OSPF_EXTERNAL_INFO, "OSPF ext. info"			},
  { MTYPE_OSPF_DISTANCE,      "OSPF distance"			},
  { MTYPE_OSPF_IF_INFO,       "OSPF if info"			},
  { MTYPE_OSPF_IF_PARAMS,     "OSPF if params"			},
  { MTYPE_OSPF_MESSAGE,		"OSPF message"			},
  { MTYPE_OSPF_MPLS_TE,       "OSPF MPLS parameters"            },
  { MTYPE_OSPF_PCE_PARAMS,    "OSPF PCE parameters"             },
  { -1, NULL },
};

struct memory_list memory_list_ospf6[] =
{
  { MTYPE_OSPF6_TOP,          "OSPF6 top"			},
  { MTYPE_OSPF6_AREA,         "OSPF6 area"			},
  { MTYPE_OSPF6_IF,           "OSPF6 interface"			},
  { MTYPE_OSPF6_NEIGHBOR,     "OSPF6 neighbor"			},
  { MTYPE_OSPF6_ROUTE,        "OSPF6 route"			},
  { MTYPE_OSPF6_PREFIX,       "OSPF6 prefix"			},
  { MTYPE_OSPF6_MESSAGE,      "OSPF6 message"			},
  { MTYPE_OSPF6_LSA,          "OSPF6 LSA"			},
  { MTYPE_OSPF6_LSA_SUMMARY,  "OSPF6 LSA summary"		},
  { MTYPE_OSPF6_LSDB,         "OSPF6 LSA database"		},
  { MTYPE_OSPF6_VERTEX,       "OSPF6 vertex"			},
  { MTYPE_OSPF6_SPFTREE,      "OSPF6 SPF tree"			},
  { MTYPE_OSPF6_NEXTHOP,      "OSPF6 nexthop"			},
  { MTYPE_OSPF6_EXTERNAL_INFO,"OSPF6 ext. info"			},
  { MTYPE_OSPF6_OTHER,        "OSPF6 other"			},
  { MTYPE_OSPF6_DISTANCE,     "OSPF6 distance"			},
  { -1, NULL },
};

struct memory_list memory_list_isis[] =
{
  { MTYPE_ISIS,               "ISIS"				},
  { MTYPE_ISIS_TMP,           "ISIS TMP"			},
  { MTYPE_ISIS_CIRCUIT,       "ISIS circuit"			},
  { MTYPE_ISIS_LSP,           "ISIS LSP"			},
  { MTYPE_ISIS_ADJACENCY,     "ISIS adjacency"			},
  { MTYPE_ISIS_AREA,          "ISIS area"			},
  { MTYPE_ISIS_AREA_ADDR,     "ISIS area address"		},
  { MTYPE_ISIS_TLV,           "ISIS TLV"			},
  { MTYPE_ISIS_DYNHN,         "ISIS dyn hostname"		},
  { MTYPE_ISIS_SPFTREE,       "ISIS SPFtree"			},
  { MTYPE_ISIS_VERTEX,        "ISIS vertex"			},
  { MTYPE_ISIS_ROUTE_INFO,    "ISIS route info"			},
  { MTYPE_ISIS_NEXTHOP,       "ISIS nexthop"			},
  { MTYPE_ISIS_NEXTHOP6,      "ISIS nexthop6"			},
  { MTYPE_ISIS_DICT,          "ISIS dictionary"			},
  { MTYPE_ISIS_DICT_NODE,     "ISIS dictionary node"		},
  { MTYPE_ISIS_MPLS_TE,       "ISIS MPLS_TE parameters"         },
  { -1, NULL },
};

struct memory_list memory_list_pim[] =
{
  { MTYPE_PIM_CHANNEL_OIL,       "PIM SSM (S,G) channel OIL"      },
  { MTYPE_PIM_INTERFACE,         "PIM interface"	          },
  { MTYPE_PIM_IGMP_JOIN,         "PIM interface IGMP static join" },
  { MTYPE_PIM_IGMP_SOCKET,       "PIM interface IGMP socket"      },
  { MTYPE_PIM_IGMP_GROUP,        "PIM interface IGMP group"       },
  { MTYPE_PIM_IGMP_GROUP_SOURCE, "PIM interface IGMP source"      },
  { MTYPE_PIM_NEIGHBOR,          "PIM interface neighbor"         },
  { MTYPE_PIM_IFCHANNEL,         "PIM interface (S,G) state"      },
  { MTYPE_PIM_UPSTREAM,          "PIM upstream (S,G) state"       },
  { MTYPE_PIM_SSMPINGD,          "PIM sspimgd socket"             },
  { MTYPE_PIM_STATIC_ROUTE,      "PIM Static Route"               },
  { -1, NULL },
};

struct memory_list memory_list_nhrp[] =
{
  { MTYPE_NHRP_IF,		"NHRP interface"		},
  { MTYPE_NHRP_VC,		"NHRP virtual connection"	},
  { MTYPE_NHRP_PEER,		"NHRP peer entry"		},
  { MTYPE_NHRP_CACHE,		"NHRP cache entry"		},
  { MTYPE_NHRP_NHS,		"NHRP next hop server"		},
  { MTYPE_NHRP_REGISTRATION,	"NHRP registration entries"	},
  { MTYPE_NHRP_SHORTCUT,	"NHRP shortcut"			},
  { MTYPE_NHRP_ROUTE,		"NHRP routing entry"		},
  { -1, NULL }
};

struct memory_list memory_list_vtysh[] =
{
  { MTYPE_VTYSH_CONFIG,		"Vtysh configuration",		},
  { MTYPE_VTYSH_CONFIG_LINE,	"Vtysh configuration line"	},
  { -1, NULL },
};

/* 2016??????6??????27?????? 21:07:44 zhurish: ?????????????????????????????????????????????????????? */
#ifdef ZEBRA_ROUTE_OLSR 
struct memory_list memory_list_olsr[] =
{
  { MTYPE_OLSR_TOP,		"OLSR top",		},
  { MTYPE_OLSR_IF,		"OLSR interface",	},
  { MTYPE_OLSR_IF_INFO,		"OLSR interface info ",	},
  { MTYPE_OLSR_HEADER,		"OLSR packet header",	},
  { MTYPE_OLSR_HELLO_HEADER,	"OLSR hello header",	},
  { MTYPE_OLSR_LINK_HEADER,	"OLSR link header",	},
  { MTYPE_OLSR_LINK,		"OLSR link",		},
  { MTYPE_OLSR_NEIGH,		"OLSR neighbor",	},
  { MTYPE_OLSR_OI_LINK,		"OLSR interface link",	},
  { MTYPE_OLSR_MSG,		"OLSR message",		},
  { MTYPE_OLSR_FIFO,		"OLSR message fifo",	},
  { MTYPE_OLSR_DUP,		"OLSR Duplicate Tuple",	},
  { MTYPE_OLSR_MID,		"OLSR admin address",	},
  { MTYPE_OLSR_HOP2,		"OLSR 2-hop tuples",	},
  { MTYPE_OLSR_1N,		"OLSR mpr 1-hop",	},
  { MTYPE_OLSR_2N,		"OLSR mpr 2-hop ",	},
  { MTYPE_OLSR_ROUTE,  		"OLSR route",		},
  { -1, NULL },
}; 
#endif
#ifdef ZEBRA_ROUTE_LDP
struct memory_list memory_list_ldp[] =
{
  /* LDP Related Memory Types */
  { MTYPE_LDP,			"LDP"				},
  { MTYPE_LDP_TOP,		"LDP top"			},
  { MTYPE_LDP_IF_INFO,		"LDP interface information"	},
  { MTYPE_LDP_ROUTE_INFO,	"LDP route information"		},
  { MTYPE_LDP_HELLOADJ,		"LDP hello adj information"	},
  { MTYPE_LDP_HELLOMGR,		"LDP hello mgr information"	},
  { MTYPE_LDP_PEER,		"LDP perr information"		},
  { MTYPE_LDP_PEER_HASH_IFINDEX_ENTRY,	"LDP peer hash ifindex entry"},
  { MTYPE_LDP_FECUNION,			"LDP fecunion"	},
  { MTYPE_LDP_LIB,			"LDP lib"	},
  { MTYPE_LDP_LABEL,			"LDP label"	},
  { MTYPE_LDP_LABEL_MAP_ENTRY,		"LDP label map entry"	},
  { MTYPE_LDP_PACKET,			"LDP packet"		},
  { MTYPE_LDP_PACKET_FIFO,		"LDP packet info"	},
  { MTYPE_LDP_ADDR,			"LDP addr"		},
  { MTYPE_LDP_FEC_RT,			"LDP fec rt"		},
  { MTYPE_LDP_PEER_LABEL_DB,		"LDP peer label db"	},
  { -1, NULL },
};
#endif
#ifdef ZEBRA_ROUTE_LLDP
struct memory_list memory_list_lldp[] =
{
  /* LDP Related Memory Types */
  { MTYPE_LLDP,			"LLDP"				},
  { MTYPE_LLDP_IF_INFO,		"LLDP interface information"	},
  { MTYPE_LLDP_PORT_DESC,	"LLDP port desc information"		},
  { MTYPE_LLDP_SYSTEM,		"LLDP system information"	},
  { MTYPE_LLDP_NEIGHBOR,	"LLDP neighbor information"},
  { MTYPE_LLDP_ORG,			"LLDP IEEE ORG tlv information"	},
  { MTYPE_LLDP_PACKET,			"LLDP packet"		},
  { MTYPE_LLDP_TLV,		"LLDP TLV information"	},
  { -1, NULL },
};
#endif

#ifdef PL_MODEM_MODULE
struct memory_list memory_list_modem[] =
{
  { MTYPE_MODEM,             "MODEM structure"		},
  { MTYPE_MODEM_CLIENT,      "MODEM Client info"	},
  { MTYPE_MODEM_SERIAL,      "MODEM serial"			},
  { MTYPE_MODEM_DRIVER,      "MODEM driver"			},
  { MTYPE_MODEM_DATA,    	 "MODEM data"			},
  { -1, NULL }
};
#endif
#ifdef PL_DHCP_MODULE
struct memory_list memory_list_dhcp[] =
{
  { MTYPE_DHCP,                "DHCP structure"				},
  { MTYPE_DHCPC,                "DHCP Client structure"		},
  { MTYPE_DHCPC_INFO,           "DHCP Client info"			},
  { MTYPE_DHCPC_ADDR,           "DHCP Client address"		},
  { MTYPE_DHCPS,                "DHCP Server structure"		},
  { MTYPE_DHCPS_INFO,           "DHCP Server info"			},
  { MTYPE_DHCPS_ADDR,           "DHCP Server address"		},
  { MTYPE_DHCPS_POOL,           "DHCP Server address pool"	},
  { -1, NULL }
};
#endif

#ifdef PL_WIFI_MODULE
struct memory_list memory_list_wifi[] =
{
  { MTYPE_WIFI,              "WIFI structure"				},
  { MTYPE_WIFI_DB,           "WIFI DB structure"			},
  { MTYPE_WIFI_AP,           "WIFI AP info"			},
  { MTYPE_WIFI_CLIENT,       "WIFI AP Client info"			},
  { -1, NULL }
};
#endif

struct memory_list memory_list_port[] =
{
	{ MTYPE_PORT,			"PHY PORT information"			},
	{ MTYPE_MAC,			"MAC information"			},
	{ MTYPE_ARP,			"ARP information"			},
	{ MTYPE_ACL,			"ACL information"			},
	{ MTYPE_PPP,			"PPP information"		},

	{ MTYPE_PPPOE,			"PHY PORT information"			},
	{ MTYPE_KERNEL,			"MAC information"			},
	{ MTYPE_SERIAL,			"Serial information"			},

	{ MTYPE_VLAN,			"VLAN information"			},
	{ MTYPE_TRUNK,			"Trunk information"			},
	{ MTYPE_QOS,			"QOS information"			},
	{ MTYPE_DOT1X,			"DOT1X information"			},
	{ MTYPE_MIRROR,			"MIRROR information"		},
	{ -1, NULL },
};

struct memory_list memory_list_ssh[] =
{
  { MTYPE_SSH,			"SSH information"	},
  { MTYPE_SSHD,			"SSHD information"	},

  { MTYPE_SSH_CLIENT,	"SSH Client information"	},
  { MTYPE_SSH_SCP,		"SSH SCP information"	},
  { MTYPE_SSH_BUF,		"SSH Buffer information"	},
  { MTYPE_SSH_DATA,		"SSH Data information"	},
  { MTYPE_SSH_KEY,		"SSH Key information"	},
  { MTYPE_SSH_MSG,		"SSH Msg information"	},
  { -1, NULL },
};

#ifdef PL_VOIP_MODULE
struct memory_list memory_list_voip[] =
{
  { MTYPE_VOIP,			"VOIP information"	},
  { MTYPE_VOIP_SESSION,			"VOIP Session information"	},
  { MTYPE_VOIP_CHANNEL,		"VOIP Channel information"	},
  { MTYPE_VOIP_RNG,			"VOIP Rng information"	},
  { MTYPE_VOIP_DATA,			"VOIP Data information"	},
  { MTYPE_VOIP_RAW_DATA,		"VOIP Raw Data information"	},
  { -1, NULL },
};
#endif

struct mlist mlists[] __attribute__ ((unused)) = {
  { memory_list_lib,	"LIB"	},
  { memory_list_zebra,	"ZEBRA"	},
  { memory_list_rip,	"RIP"	},
  { memory_list_ripng,	"RIPNG"	},
  { memory_list_ospf,	"OSPF"	},
  { memory_list_ospf6,	"OSPF6"	},
  { memory_list_isis,	"ISIS"	},
  { memory_list_bgp,	"BGP"	},
  { memory_list_pim,	"PIM"	},
  { memory_list_nhrp,	"NHRP"	},
/* 2016??????6??????27?????? 21:07:44 zhurish: ?????????????????????????????????????????????????????? */  
#ifdef ZEBRA_ROUTE_OLSR 
  { memory_list_olsr,	"OLSR"	},
#endif  

#ifdef ZEBRA_ROUTE_LDP 
  { memory_list_ldp,	"LDP"	},
#endif 

#ifdef ZEBRA_ROUTE_LLDP
  { memory_list_lldp,	"LLDP"	},
#endif
#ifdef PL_MODEM_MODULE
  { memory_list_modem,	"MODEM"	},
#endif
#ifdef PL_DHCP_MODULE
  { memory_list_dhcp,	"DHCP"	},
#endif
#ifdef PL_WIFI_MODULE
  { memory_list_wifi,	"WIFI"	},
#endif
  { memory_list_port,	"PORT"	},

  { memory_list_ssh,	"SSH"	},
#ifdef PL_VOIP_MODULE
  { memory_list_voip,	"VOIP"	},
#endif
/* 2016??????6??????27?????? 21:07:44 zhurish: ?????????????????????????????????????????????????????? */
  { NULL, NULL},
};
