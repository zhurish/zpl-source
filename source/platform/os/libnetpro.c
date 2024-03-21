#include "auto_include.h"
#include "zpl_type.h"
#include "os_util.h"
#include "libnetpro.h"

struct net_proto_key
{
    zpl_uint16 proto;
    const char *str;
};
struct net_l2proto_key
{
    zpl_uint8 mac[6];
    zpl_uint16 proto;
    const char *str;
};

static struct net_proto_key ip_proto_str[] =
    {
        {IP_IPPROTO_ANY, "any"},   /* ANY_PROTO */
        {IP_IPPROTO_IP, "ip"},     /* IP_PROTO */
        {IP_IPPROTO_ICMP, "icmp"}, /* ICMP_PROTO */
        {IP_IPPROTO_IGMP, "igmp"},
        {IP_IPPROTO_IPIP, "ipip"},
        {IP_IPPROTO_TCP, "tcp"}, /* UDP_PROTO */
        {IP_IPPROTO_EGP, "egp"}, /* TCP_PROTO */
        {IP_IPPROTO_PUP, "pup"},
        {IP_IPPROTO_UDP, "udp"},
        {IP_IPPROTO_IDP, "idp"},
        {IP_IPPROTO_TP, "tp"},
        {IP_IPPROTO_DCCP, "dccp"},
        {IP_IPPROTO_IPV6, "ipv6"},
        {IP_IPPROTO_RSVP, "rsvp"},
        {IP_IPPROTO_GRE, "gre"},
        {IP_IPPROTO_ESP, "esp"},
        {IP_IPPROTO_AH, "ah"},
        {IP_IPPROTO_MTP, "mtp"},
        {IP_IPPROTO_BEETPH, "beetph"},
        {IP_IPPROTO_ENCAP, "encap"},
        {IP_IPPROTO_PIM, "pim"},
        {IP_IPPROTO_COMP, "comp"},
        {IP_IPPROTO_SCTP, "sctp"},
        {IP_IPPROTO_UDPLITE, "udplite"},
        {IP_IPPROTO_MPLS, "mpls"},
        {IP_IPPROTO_RAW, "raw"},
        {IP_IPPROTO_OSPF, "ospf"},
};

static struct net_proto_key eth_proto_str[] =
    {
        {ETH_P_LOOP, "loop"},           //	0x0060		/* Ethernet Loopback packet	*/
        {ETH_P_PUP, "pup"},             //	0x0200		/* Xerox PUP packet		*/
        {ETH_P_PUPAT, "pupat"},         //	0x0201		/* Xerox PUP Addr Trans packet	*/
        #ifdef ETH_P_TSN
        {ETH_P_TSN, "tsn"},             //	0x22F0		/* TSN (IEEE 1722) packet	*/
        #endif
        #ifdef ETH_P_ERSPAN2
        {ETH_P_ERSPAN2, "erspan2"},     //	0x22EB		/* ERSPAN version 2 (type III)	*/
        #endif
        {ETH_P_IP, "ip"},               // 0x0800		/* Internet Protocol packet	*/
        {ETH_P_X25, "x25"},             //	0x0805		/* CCITT X.25			*/
        {ETH_P_ARP, "arp"},             //	0x0806		/* Address Resolution packet	*/
        {ETH_P_BPQ, "bpq"},             //	0x08FF		/* G8BPQ AX.25 Ethernet Packet	[ NOT AN OFFICIALLY REGISTERED ID ] */
        {ETH_P_IEEEPUP, "ieeepup"},     //	0x0a00		/* Xerox IEEE802.3 PUP packet */
        {ETH_P_IEEEPUPAT, "ieeepupat"}, //	0x0a01		/* Xerox IEEE802.3 PUP Addr Trans packet */
        #ifdef ETH_P_BATMAN
        {ETH_P_BATMAN, "batman"},       //	0x4305		/* B.A.T.M.A.N.-Advanced packet [ NOT AN OFFICIALLY REGISTERED ID ] */
        #endif
        {ETH_P_DEC, "dec"},             //       0x6000          /* DEC Assigned proto           */
        {ETH_P_DNA_DL, "dnadl"},        //    0x6001          /* DEC DNA Dump/Load            */
        {ETH_P_DNA_RC, "dnarc"},        //    0x6002          /* DEC DNA Remote Console       */
        {ETH_P_DNA_RT, "dnart"},        //    0x6003          /* DEC DNA Routing              */
        {ETH_P_LAT, "lat"},             //       0x6004          /* DEC LAT                      */
        {ETH_P_DIAG, "diag"},           //      0x6005          /* DEC Diagnostics              */
        {ETH_P_CUST, "cust"},           //     0x6006          /* DEC Customer use             */
        {ETH_P_SCA, "sca"},             //      0x6007          /* DEC Systems Comms Arch       */
        {ETH_P_TEB, "teb"},             //	0x6558		/* Trans Ether Bridging		*/
        {ETH_P_RARP, "rarp"},           //      0x8035		/* Reverse Addr Res packet	*/
        {ETH_P_ATALK, "atalk"},         //	0x809B		/* Appletalk DDP		*/
        {ETH_P_AARP, "aarp"},           //	0x80F3		/* Appletalk AARP		*/
        {ETH_P_8021Q, "8021q"},         //	0x8100          /* 802.1Q VLAN Extended Header  */
        #ifdef ETH_P_ERSPAN
        {ETH_P_ERSPAN, "erspan"},       //	0x88BE		/* ERSPAN type II		*/
        #endif
        {ETH_P_IPX, "ipx"},             //	0x8137		/* IPX over DIX			*/
        {ETH_P_IPV6, "ipv6"},           //	0x86DD		/* IPv6 over bluebook		*/
        {ETH_P_PAUSE, "pause"},         //	0x8808		/* IEEE Pause frames. See 802.3 31B */
        {ETH_P_SLOW, "slow"},           //	0x8809		/* Slow Protocol. See 802.3ad 43B */
        {ETH_P_WCCP, "wccp"},           //	0x883E		/* Web-cache coordination protocol defined in draft-wilson-wrec-wccp-v2-00.txt */
        {ETH_P_MPLS_UC, "mplsuc"},      //	0x8847		/* MPLS Unicast traffic		*/
        {ETH_P_MPLS_MC, "mplsmc"},      //	0x8848		/* MPLS Multicast traffic	*/
        {ETH_P_ATMMPOA, "atmmpoa"},     //	0x884c		/* MultiProtocol Over ATM	*/
        {ETH_P_PPP_DISC, "pppdisc"},    //	0x8863		/* PPPoE discovery messages     */
        {ETH_P_PPP_SES, "pppses"},      //	0x8864		/* PPPoE session messages	*/
        {ETH_P_LINK_CTL, "linkctl"},    //	0x886c		/* HPNA, wlan link local tunnel */
        {ETH_P_ATMFATE, "atmfate"},     //	0x8884		/* Frame-based ATM Transport over Ethernet */
        {ETH_P_PAE, "pae"},             //	0x888E		/* Port Access Entity (IEEE 802.1X) */
        {ETH_P_AOE, "oae"},             //	0x88A2		/* ATA over Ethernet		*/
        {ETH_P_8021AD, "8021ad"},       //	0x88A8          /* 802.1ad Service VLAN		*/
        {ETH_P_802_EX1, "802ex1"},      //	0x88B5		/* 802.1 Local Experimental 1.  */
        #ifdef ETH_P_PREAUTH
        {ETH_P_PREAUTH, "preauth"},     //	0x88C7		/* 802.11 Preauthentication */
        #endif
        {ETH_P_TIPC, "tipc"},           //	0x88CA		/* TIPC 			*/
        #ifdef ETH_P_LLDP
        {ETH_P_LLDP, "lldp"},           //	0x88CC		/* Link Layer Discovery Protocol */
        #endif
        #ifdef ETH_P_MACSEC
        {ETH_P_MACSEC, "macsec"},       //	0x88E5		/* 802.1ae MACsec */
        #endif
        {ETH_P_8021AH, "8021ah"},       //	0x88E7          /* 802.1ah Backbone Service Tag */
        #ifdef ETH_P_MVRP
        {ETH_P_MVRP, "mvrp"},           //	0x88F5          /* 802.1Q MVRP                  */
        #endif
        {ETH_P_1588, "1588"},           //	0x88F7		/* IEEE 1588 Timesync */
        #ifdef ETH_P_NCSI
        {ETH_P_NCSI, "ncsi"},           //	0x88F8		/* NCSI protocol		*/
        #endif
        #ifdef ETH_P_PRP
        {ETH_P_PRP, "prp"},             //	0x88FB		/* IEC 62439-3 PRP/HSRv0	*/
        #endif
        {ETH_P_FCOE, "fcoe"},           //	0x8906		/* Fibre Channel over Ethernet  */
        #ifdef ETH_P_IBOE
        {ETH_P_IBOE, "iboe"},           //	0x8915		/* Infiniband over Ethernet	*/
        #endif
        {ETH_P_TDLS, "tdls"},           //	0x890D          /* TDLS */
        {ETH_P_FIP, "fip"},             //	0x8914		/* FCoE Initialization Protocol */
        #ifdef ETH_P_80221
        {ETH_P_80221, "80221"},         //	0x8917		/* IEEE 802.21 Media Independent Handover Protocol */
        #endif
        #ifdef ETH_P_HSR
        {ETH_P_HSR, "hsr"},             //	0x892F		/* IEC 62439-3 HSRv1	*/
        #endif
        #ifdef ETH_P_NSH
        {ETH_P_NSH, "nsh"},             //	0x894F		/* Network Service Header */
        #endif
        #ifdef ETH_P_LOOPBACK
        {ETH_P_LOOPBACK, "loopback"},   //	0x9000		/* Ethernet loopback packet, per IEEE 802.3 */
        #endif
        {ETH_P_QINQ1, "qinq1"},         //	0x9100		/* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
        {ETH_P_QINQ2, "qinq2"},         //	0x9200		/* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
        {ETH_P_QINQ3, "qinq3"},         //	0x9300		/* deprecated QinQ VLAN [ NOT AN OFFICIALLY REGISTERED ID ] */
        {ETH_P_EDSA, "edsa"},           //	0xDADA		/* Ethertype DSA [ NOT AN OFFICIALLY REGISTERED ID ] */
        #ifdef ETH_P_IFE
        {ETH_P_IFE, "ife"},             //	0xED3E		/* ForCES inter-FE LFB type */
        #endif
        {ETH_P_AF_IUCV, "afiucv"},      //   0xFBFB		/* IBM af_iucv [ NOT AN OFFICIALLY REGISTERED ID ] */
        #ifdef ETH_P_802_3_MIN
        {ETH_P_802_3_MIN, "8023min"}, //	0x0600		/* If the value in the ethernet type is less than this value then the frame is Ethernet II. Else it is 802.3 */
        #endif
        /*
         *	Non DIX types. Won't clash for 1500 types.
         */

        {ETH_P_802_3, "8023"},            //	0x0001		/* Dummy type for 802.3 frames  */
        {ETH_P_AX25, "ax25"},             //	0x0002		/* Dummy protocol id for AX.25  */
        {ETH_P_ALL, "all"},               //	0x0003		/* Every packet (be careful!!!) */
        {ETH_P_802_2, "8022"},            //	0x0004		/* 802.2 frames 		*/
        {ETH_P_SNAP, "snap"},             //	0x0005		/* Internal only		*/
        {ETH_P_DDCMP, "ddcmp"},           //    0x0006          /* DEC DDCMP: Internal only     */
        {ETH_P_WAN_PPP, "wanppp"},        //   0x0007          /* Dummy type for WAN PPP frames*/
        {ETH_P_PPP_MP, "pppmp"},          //   0x0008          /* Dummy type for PPP MP frames */
        {ETH_P_LOCALTALK, "localtalk"},   // 0x0009		/* Localtalk pseudo type 	*/
        {ETH_P_CAN, "can"},               //	0x000C		/* CAN: Controller Area Network */
        #ifdef ETH_P_CANFD
        {ETH_P_CANFD, "canfd"},           //	0x000D		/* CANFD: CAN flexible data rate*/
        #endif
        {ETH_P_PPPTALK, "ppptalk"},       //	0x0010		/* Dummy type for Atalk over PPP*/
        {ETH_P_TR_802_2, "tr8022"},       //	0x0011		/* 802.2 frames 		*/
        {ETH_P_MOBITEX, "mobitex"},       //	0x0015		/* Mobitex (kaz@cafe.net)	*/
        {ETH_P_CONTROL, "control"},       //	0x0016		/* Card specific control frames */
        {ETH_P_IRDA, "irda"},             //	0x0017		/* Linux-IrDA			*/
        {ETH_P_ECONET, "econet"},         //	0x0018		/* Acorn Econet			*/
        {ETH_P_HDLC, "hdlc"},             //	0x0019		/* HDLC frames			*/
        {ETH_P_ARCNET, "arcnet"},         //	0x001A		/* 1A for ArcNet :-)            */
        {ETH_P_DSA, "dsa"},               //	0x001B		/* Distributed Switch Arch.	*/
        {ETH_P_TRAILER, "trailer"},       //	0x001C		/* Trailer switch tagging	*/
        {ETH_P_PHONET, "phonet"},         //	0x00F5		/* Nokia Phonet frames          */
        {ETH_P_IEEE802154, "ieee802154"}, // 0x00F6		/* IEEE802.15.4 frame		*/
        {ETH_P_CAIF, "caif"},             //	0x00F7		/* ST-Ericsson CAIF protocol	*/
        #ifdef ETH_P_XDSA
        {ETH_P_XDSA, "xdsa"},             //	0x00F8		/* Multiplexed DSA protocol	*/
        #endif
        #ifdef ETH_P_MAP
        {ETH_P_MAP, "map"},               //	0x00F9		/* Qualcomm multiplexing and aggregation protocol */
        #endif
};
/*
static const char *port_op_str[] =
    {
        "",
        "eq",
        "ne",
        "gt",
        "lt",
        "ge",
        "le",
        "range"};
*/
zpl_uint16 ip_protocol_type(const char *str)
{
    zpl_uint32 i = 0;
    for (i = 0; i < sizeof(ip_proto_str) / sizeof(ip_proto_str[0]); i++)
    {
        if (!strcmp(ip_proto_str[i].str, str))
            return ip_proto_str[i].proto;
    }
    return atoi(str);
}

const char *ip_protocol_type_string(zpl_uint16 type)
{
    zpl_uint32 i = 0;
    for (i = 0; i < sizeof(ip_proto_str) / sizeof(ip_proto_str[0]); i++)
    {
        if (ip_proto_str[i].proto == type)
            return ip_proto_str[i].str;
    }
    return itoa(type, 10);
}

zpl_uint16 eth_protocol_type(const char *str)
{
    zpl_uint32 i = 0;
    for (i = 0; i < sizeof(eth_proto_str) / sizeof(eth_proto_str[0]); i++)
    {
        if (!strcmp(eth_proto_str[i].str, str))
            return eth_proto_str[i].proto;
    }
    return strtol(str, NULL, 16);
}

const char *eth_protocol_type_string(zpl_uint16 type)
{
    zpl_uint32 i = 0;
    for (i = 0; i < sizeof(eth_proto_str) / sizeof(eth_proto_str[0]); i++)
    {
        if (eth_proto_str[i].proto == type)
            return eth_proto_str[i].str;
    }
    return itoa(type, 16);
}

zpl_uint16 port_operation_type(const char *str)
{
    if (!strncmp(str, "eq", 2))
        return OPT_EQ;
    else if (!strncmp(str, "ne", 2))
        return OPT_NE;
    else if (!strncmp(str, "le", 2))
        return OPT_LE;
    else if (!strncmp(str, "ge", 2))
        return OPT_GE;
    else if (!strncmp(str, "gt", 2))
        return OPT_GT;
    else if (!strncmp(str, "lt", 2))
        return OPT_LT;
    else if (!strncmp(str, "range", 2))
        return OPT_RN;
    return OPT_NONE;
}

const char *port_operation_type_string(zpl_uint16 type)
{
    switch (type)
    {
    case OPT_NONE:
        return "none";
    case OPT_EQ:
        return "eq";
    case OPT_NE:
        return "ne";
    case OPT_GT:
        return "gt";
    case OPT_LT:
        return "lt";
    case OPT_GE:
        return "ge";
    case OPT_LE:
        return "le";
    case OPT_RN:
        return "range";
    case OPT_MAX:
        return "none";
    }
    return "none";
}

static struct net_l2proto_key l2mac_proto_str[] = {
{ {0x01, 0x80, 0xC2, 0x00, 0x00, 0x00}, 0, "aa"},
{ {0x01, 0x80, 0xC2, 0x00, 0x00, 0x01}, 0, "aa"}, 
{ {0x01, 0x80, 0xC2, 0x00, 0x00, 0x02}, 0, "aa"},
{ {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03}, 0, "aa"},
{ {0x01, 0x80, 0xC2, 0x00, 0x00, 0x04}, 0, "aa"},
{ {0x01, 0x80, 0xC2, 0x00, 0x00, 0x05}, 0, "aa"},
{ {0x01, 0x80, 0xC2, 0x00, 0x00, 0x06}, 0, "aa"},
{ {0x01, 0x80, 0xC2, 0x00, 0x00, 0x07}, 0, "aa"},
{ {0x01, 0x80, 0xC2, 0x00, 0x00, 0x08}, 0, "aa"},
{ {0x01, 0x80, 0xC2, 0x00, 0x00, 0x09}, 0, "aa"},
{ {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0A}, 0, "aa"},
{ {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0B}, 0, "aa"},
};


zpl_uint16 eth_l2protocol_type(const char *smac)
{
    zpl_uint32 i = 0;
    for (i = 0; i < sizeof(l2mac_proto_str) / sizeof(l2mac_proto_str[0]); i++)
    {
        if (memcmp(l2mac_proto_str[i].mac, smac, 6))
            return l2mac_proto_str[i].proto;
    }
    return 0;
}

const char *eth_l2protocol_type_string(zpl_uint16 type)
{
    zpl_uint32 i = 0;
    for (i = 0; i < sizeof(l2mac_proto_str) / sizeof(l2mac_proto_str[0]); i++)
    {
        if (l2mac_proto_str[i].proto == type)
            return l2mac_proto_str[i].str;
    }
    return itoa(type, 16);
}