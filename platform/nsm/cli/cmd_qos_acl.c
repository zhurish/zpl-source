#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"
#include "nsm_qos_acl.h"

#define QOS_CLASS_MAP_STR "Class Map List\n"
#define QOS_POLICY_MAP_STR "POlicy Map List\n"

#ifdef ZPL_FILTER_ZEBRA_EXT
static int qos_access_filter_zebos_extended_adddel(qos_access_filter_list_t *acllist, struct vty *vty, const char *name_str,
                                                   const char *type_str, afi_t afi,
                                                   const char *prot_str, const char *sprefix,
                                                   const char *sport_op, const char *sport, const char *seport,
                                                   const char *dprefix, const char *dport_op,
                                                   const char *dport, const char *deport, u_char set)
{
    int ret;
    qos_access_filter_t *node = qos_access_filter_alloc();
    if (node)
    {
        /* Access-list name check. */
        if (name_str)
        {
            node->seqnum = atoi(name_str);
            if (node->seqnum == UINT32_MAX)
            {
                vty_out(vty, "%% filter name rang invalid%s", VTY_NEWLINE);
                qos_access_filter_free(node);
                return CMD_WARNING;
            }
        }
        else
        {
            if (acllist->seqnum == 0)
                node->seqnum = 1;
            else
                node->seqnum = (acllist->seqnum + acllist->seqnum_step);
        }

        /* Check of filter type. */
        if (strncmp(type_str, "p", 1) == 0)
            node->type = FILTER_PERMIT;
        else if (strncmp(type_str, "d", 1) == 0)
            node->type = FILTER_DENY;
        else
        {
            vty_out(vty, "%% filter type must be permit or deny%s", VTY_NEWLINE);
            qos_access_filter_free(node);
            return CMD_WARNING;
        }
        if (filter_zebos_ext_format(vty, afi, prot_str, sprefix,
                                    sport_op, sport, seport,
                                    dprefix, dport_op,
                                    dport, deport,
                                    &node->u.zextfilter) != CMD_SUCCESS)
        {
            vty_out(vty, "%% filter type must be permit or deny%s", VTY_NEWLINE);
            qos_access_filter_free(node);
            return CMD_WARNING;
        }
        node->nodetype = FILTER_ZEBOS_EXT;
        if (set)
        {
            if (qos_access_filter_list_lookup(acllist, node->seqnum, NULL))
            {
                vty_out(vty, "%% filter %d is already exist.%s", node->seqnum, VTY_NEWLINE);
                qos_access_filter_free(node);
                return CMD_WARNING;
            }
            else if (qos_access_filter_list_lookup(acllist, -1, node))
            {
                vty_out(vty, "%% filter is already exist.%s", VTY_NEWLINE);
                qos_access_filter_free(node);
                return CMD_WARNING;
            }
            else
            {
                acllist->seqnum = node->seqnum;
                return qos_access_filter_list_add(acllist, node);
            }
        }
        else
        {
            qos_access_filter_t *delete_filter;

            delete_filter = qos_access_filter_list_lookup(acllist, node->seqnum, NULL);
            if (delete_filter)
            {
                qos_access_filter_free(node);
                return qos_access_filter_list_del(acllist, delete_filter);
            }
            vty_out(vty, "%% can not find this filter.%s", VTY_NEWLINE);
            qos_access_filter_free(node);
            return CMD_WARNING;
        }
    }
    return CMD_WARNING;
}

/*
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536>
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> <1-65536>
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536>

<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-port any
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-port <1-65536>
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-gport <1-65536> <1-65536>
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-port (ne|gt|lt|ge|le) <1-65536>

<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> dest-port any
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> dest-port <1-65536>
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> dest-gport <1-65536> <1-65536>
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>

<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> <1-65536> dest-port any
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> <1-65536> dest-port <1-65536>
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> <1-65536> dest-gport <1-65536> <1-65536>
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>

<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port any
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port <1-65536>
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-gport <1-65536> <1-65536>
<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>

<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (ip|ipip|egp|rsvp|gre|rsvp|esp|pim|sctp|mpls|raw|ospf|icmp|igmp)

<1-2147483646> (deny|permit) any any any
<1-2147483646> (deny|permit) any A.B.C.D/M any

<1-2147483646> (deny|permit) A.B.C.D/M any any
<1-2147483646> (deny|permit) A.B.C.D/M any any

<1-2147483646> (deny|permit) A.B.C.D/M A.B.C.D/M any
*/
DEFUN(qos_access_list_ip_srcany,
      qos_access_list_ip_srcany_cmd,
      "<1-2147483646> (deny|permit) any (any|A.B.C.D/M) (any|)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Any\n"
      "Specify Destination Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Any IP Protocol\n")
{
    char *prot_str = "any";
    char *sprefix = "any";
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(qos_access_list_ip_dstany,
      qos_access_list_ip_dstany_cmd,
      "<1-2147483646> (deny|permit) A.B.C.D/M (any|A.B.C.D/M) (any|)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Any IP Protocol\n")
{
    char *prot_str = "any";
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = "any";
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(qos_access_list_ip_proany,
      qos_access_list_ip_proany_cmd,
      "<1-2147483646> (deny|permit) A.B.C.D/M A.B.C.D/M (any|)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Any IP Protocol\n")
{
    char *prot_str = "any";
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(qos_access_list_ip_protocol,
      qos_access_list_ip_protocol_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_IPPROTO_CMD ")",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" 
          IP_IPPROTO_HELP)
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(no_qos_access_list_ip_protocol,
      no_qos_access_list_ip_protocol_cmd,
      "no <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_IPPROTO_CMD ")",
      NO_STR
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" 
          IP_IPPROTO_HELP)
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 0);
}

DEFUN(no_qos_access_list_ip_tcpudp,
      no_qos_access_list_ip_tcpudp_cmd,
      "no <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ")",
      NO_STR
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP)
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 6)
    {
        sport_op = "eq";
        sport = argv[5];
    }
    else if (argc == 7)
    {
        if (port_operation_type(argv[5]) == OPT_NONE)
        {
            sport_op = argv[5];
            sport = argv[6];
        }
        else
        {
            sport_op = "eq";
            sport = argv[5];
            seport = argv[6];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 0);
}

ALIAS(no_qos_access_list_ip_tcpudp,
      no_qos_access_list_ip_tcpudp_name_cmd,
      "no <1-2147483646>",
      NO_STR
      "Access list Seqnum\n")

DEFUN(qos_access_list_ip_tcpudp,
      qos_access_list_ip_tcpudp_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ")",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP)
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 6)
    {
        sport_op = "eq";
        sport = argv[5];
    }
    else if (argc == 7)
    {
        if (port_operation_type(argv[5]) == OPT_NONE)
        {
            sport_op = argv[5];
            sport = argv[6];
        }
        else
        {
            sport_op = "rn";
            sport = argv[5];
            seport = argv[6];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(qos_access_list_ip_tcpudp,
      qos_access_list_ip_tcpudp_srcport_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n")

ALIAS(qos_access_list_ip_tcpudp,
      qos_access_list_ip_tcpudp_srcport_range_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n")

ALIAS(qos_access_list_ip_tcpudp,
      qos_access_list_ip_tcpudp_srcport_ng_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n")

DEFUN(qos_access_list_ip_tcpudp_dstport,
      qos_access_list_ip_tcpudp_dstport_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port any dest-port eq <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = "any";
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 6)
    {
        dport_op = "eq";
        dport = argv[5];
    }
    else if (argc == 7)
    {
        if (port_operation_type(argv[5]) == OPT_NONE)
        {
            dport_op = argv[5];
            dport = argv[6];
        }
        else
        {
            dport_op = "rn";
            dport = argv[5];
            deport = argv[6];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(qos_access_list_ip_tcpudp_dstport,
      qos_access_list_ip_tcpudp_dstport_range_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port any dest-port range <1-65536> <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(qos_access_list_ip_tcpudp_dstport,
      qos_access_list_ip_tcpudp_dstport_ng_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port any dest-port (ne|gt|lt|ge|le) <1-65536>",
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

DEFUN(qos_access_list_ip_tcpudp_srcport_dstport,
      qos_access_list_ip_tcpudp_srcport_dstport_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536> dest-port eq <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = argv[5];
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 7)
    {
        dport_op = "eq";
        dport = argv[6];
    }
    else if (argc == 8)
    {
        if (port_operation_type(argv[6]) == OPT_NONE)
        {
            dport_op = argv[6];
            dport = argv[7];
        }
        else
        {
            dport_op = "rn";
            dport = argv[6];
            deport = argv[7];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(qos_access_list_ip_tcpudp_srcport_dstport,
      qos_access_list_ip_tcpudp_srcport_dstport_range_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536> dest-port range <1-65536> <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(qos_access_list_ip_tcpudp_srcport_dstport,
      qos_access_list_ip_tcpudp_srcport_dstport_ng_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

DEFUN(qos_access_list_ip_tcpudp_srcport_range_dstport,
      qos_access_list_ip_tcpudp_srcport_range_dstport_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536> dest-port eq <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = "rn";
    char *sport = argv[5];
    char *seport = argv[6];
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 8)
    {
        dport_op = "eq";
        dport = argv[7];
    }
    else if (argc == 9)
    {
        if (port_operation_type(argv[7]) == OPT_NONE)
        {
            dport_op = argv[7];
            dport = argv[8];
        }
        else
        {
            dport_op = "rn";
            dport = argv[7];
            deport = argv[8];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(qos_access_list_ip_tcpudp_srcport_range_dstport,
      qos_access_list_ip_tcpudp_srcport_range_dstport_range_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536> dest-port range <1-65536> <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(qos_access_list_ip_tcpudp_srcport_range_dstport,
      qos_access_list_ip_tcpudp_srcport_range_dstport_ng_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

DEFUN(qos_access_list_ip_tcpudp_srcport_ng_dstport,
      qos_access_list_ip_tcpudp_srcport_ng_dstport_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536> dest-port eq <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = argv[5];
    char *sport = argv[6];
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 8)
    {
        dport_op = "eq";
        dport = argv[7];
    }
    else if (argc == 9)
    {
        if (port_operation_type(argv[7]) == OPT_NONE)
        {
            dport_op = argv[7];
            dport = argv[8];
        }
        else
        {
            dport_op = "rn";
            dport = argv[7];
            deport = argv[8];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(qos_access_list_ip_tcpudp_srcport_ng_dstport,
      qos_access_list_ip_tcpudp_srcport_ng_dstport_range_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536> dest-port range <1-65536> <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(qos_access_list_ip_tcpudp_srcport_ng_dstport,
      qos_access_list_ip_tcpudp_srcport_ng_dstport_ng_cmd,
      "<1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
"Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

#ifdef HAVE_IPV6
/////////////
DEFUN(qos_access_list_ipv6_srcany,
      qos_access_list_ipv6_srcany_cmd,
      "<1-2147483646> (deny|permit) ipv6 any (any|X:X::X:X/M) (any|)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Any\n"
      "Specify Destination Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Any IP Protocol\n")
{
    char *prot_str = "any";
    char *sprefix = "any";
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(qos_access_list_ipv6_dstany,
      qos_access_list_ipv6_dstany_cmd,
      "<1-2147483646> (deny|permit) ipv6 X:X::X:X/M (any|X:X::X:X/M) (any|)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Any IP Protocol\n")
{
    char *prot_str = "any";
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = "any";
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(qos_access_list_ipv6_proany,
      qos_access_list_ipv6_proany_cmd,
      "<1-2147483646> (deny|permit) ipv6 X:X::X:X/M X:X::X:X/M (any|)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Any IP Protocol\n")
{
    char *prot_str = "any";
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(qos_access_list_ipv6_protocol,
      qos_access_list_ipv6_protocol_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_IPPROTO_CMD ")",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" 
          IP_IPPROTO_HELP)
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(no_qos_access_list_ipv6_protocol,
      no_qos_access_list_ipv6_protocol_cmd,
      "no <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_IPPROTO_CMD ")",
      NO_STR
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" 
          IP_IPPROTO_HELP)
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 0);
}

DEFUN(no_qos_access_list_ipv6_tcpudp,
      no_qos_access_list_ipv6_tcpudp_cmd,
      "no <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ")",
      NO_STR
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP)
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 6)
    {
        sport_op = "eq";
        sport = argv[5];
    }
    else if (argc == 7)
    {
        if (port_operation_type(argv[5]) == OPT_NONE)
        {
            sport_op = argv[5];
            sport = argv[6];
        }
        else
        {
            sport_op = "eq";
            sport = argv[5];
            seport = argv[6];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 0);
}

ALIAS(no_qos_access_list_ipv6_tcpudp,
      no_qos_access_list_ipv6_tcpudp_name_cmd,
      "no <1-2147483646> ipv6",
      NO_STR
      "Access list Seqnum\n"
      IPV6_STR)

DEFUN(qos_access_list_ipv6_tcpudp,
      qos_access_list_ipv6_tcpudp_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ")",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP)
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 6)
    {
        sport_op = "eq";
        sport = argv[5];
    }
    else if (argc == 7)
    {
        if (port_operation_type(argv[5]) == OPT_NONE)
        {
            sport_op = argv[5];
            sport = argv[6];
        }
        else
        {
            sport_op = "rn";
            sport = argv[5];
            seport = argv[6];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(qos_access_list_ipv6_tcpudp,
      qos_access_list_ipv6_tcpudp_srcport_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n")

ALIAS(qos_access_list_ipv6_tcpudp,
      qos_access_list_ipv6_tcpudp_srcport_range_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n")

ALIAS(qos_access_list_ipv6_tcpudp,
      qos_access_list_ipv6_tcpudp_srcport_ng_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n")

DEFUN(qos_access_list_ipv6_tcpudp_dstport,
      qos_access_list_ipv6_tcpudp_dstport_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port any dest-port eq <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = "any";
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 6)
    {
        dport_op = "eq";
        dport = argv[5];
    }
    else if (argc == 7)
    {
        if (port_operation_type(argv[5]) == OPT_NONE)
        {
            dport_op = argv[5];
            dport = argv[6];
        }
        else
        {
            dport_op = "rn";
            dport = argv[5];
            deport = argv[6];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(qos_access_list_ipv6_tcpudp_dstport,
      qos_access_list_ipv6_tcpudp_dstport_range_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port any dest-port range <1-65536> <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(qos_access_list_ipv6_tcpudp_dstport,
      qos_access_list_ipv6_tcpudp_dstport_ng_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port any dest-port (ne|gt|lt|ge|le) <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

DEFUN(qos_access_list_ipv6_tcpudp_srcport_dstport,
      qos_access_list_ipv6_tcpudp_srcport_dstport_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536> dest-port eq <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = NULL;
    char *sport = argv[5];
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 7)
    {
        dport_op = "eq";
        dport = argv[6];
    }
    else if (argc == 8)
    {
        if (port_operation_type(argv[6]) == OPT_NONE)
        {
            dport_op = argv[6];
            dport = argv[7];
        }
        else
        {
            dport_op = "rn";
            dport = argv[6];
            deport = argv[7];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(qos_access_list_ipv6_tcpudp_srcport_dstport,
      qos_access_list_ipv6_tcpudp_srcport_dstport_range_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536> dest-port range <1-65536> <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(qos_access_list_ipv6_tcpudp_srcport_dstport,
      qos_access_list_ipv6_tcpudp_srcport_dstport_ng_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

DEFUN(qos_access_list_ipv6_tcpudp_srcport_range_dstport,
      qos_access_list_ipv6_tcpudp_srcport_range_dstport_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536> dest-port eq <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = "rn";
    char *sport = argv[5];
    char *seport = argv[6];
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 8)
    {
        dport_op = "eq";
        dport = argv[7];
    }
    else if (argc == 9)
    {
        if (port_operation_type(argv[7]) == OPT_NONE)
        {
            dport_op = argv[7];
            dport = argv[8];
        }
        else
        {
            dport_op = "rn";
            dport = argv[7];
            deport = argv[8];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(qos_access_list_ipv6_tcpudp_srcport_range_dstport,
      qos_access_list_ipv6_tcpudp_srcport_range_dstport_range_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536> dest-port range <1-65536> <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(qos_access_list_ipv6_tcpudp_srcport_range_dstport,
      qos_access_list_ipv6_tcpudp_srcport_range_dstport_ng_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

DEFUN(qos_access_list_ipv6_tcpudp_srcport_ng_dstport,
      qos_access_list_ipv6_tcpudp_srcport_ng_dstport_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536> dest-port eq <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[4];
    char *sprefix = argv[2];
    char *sport_op = argv[5];
    char *sport = argv[6];
    char *seport = NULL;
    char *dprefix = argv[3];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 8)
    {
        dport_op = "eq";
        dport = argv[7];
    }
    else if (argc == 9)
    {
        if (port_operation_type(argv[7]) == OPT_NONE)
        {
            dport_op = argv[7];
            dport = argv[8];
        }
        else
        {
            dport_op = "rn";
            dport = argv[7];
            deport = argv[8];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, argv[0], argv[1], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(qos_access_list_ipv6_tcpudp_srcport_ng_dstport,
      qos_access_list_ipv6_tcpudp_srcport_ng_dstport_range_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536> dest-port range <1-65536> <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(qos_access_list_ipv6_tcpudp_srcport_ng_dstport,
      qos_access_list_ipv6_tcpudp_srcport_ng_dstport_ng_cmd,
      "<1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")
#endif
#endif

#ifdef ZPL_FILTER_MAC
static int qos_access_filter_l2mac_adddel(qos_access_filter_list_t *acllist, struct vty *vty, const char *name_str,
                                          const char *type_str, u_char mpls, struct filter_l2 *l2new,
                                          u_char set)
{
    int ret;
    qos_access_filter_t *node = qos_access_filter_alloc();
    if (node)
    {
        /* Access-list name check. */
        if (name_str)
        {
            node->seqnum = atoi(name_str);
            if (node->seqnum == UINT32_MAX)
            {
                vty_out(vty, "%% filter name rang invalid%s", VTY_NEWLINE);
                qos_access_filter_free(node);
                return CMD_WARNING;
            }
        }
        else
        {
            if (acllist->seqnum == 0)
                node->seqnum = 1;
            else
                node->seqnum = (acllist->seqnum + acllist->seqnum_step);
        }

        /* Check of filter type. */
        if (strncmp(type_str, "p", 1) == 0)
            node->type = FILTER_PERMIT;
        else if (strncmp(type_str, "d", 1) == 0)
            node->type = FILTER_DENY;
        else
        {
            vty_out(vty, "%% filter type must be permit or deny%s", VTY_NEWLINE);
            qos_access_filter_free(node);
            return CMD_WARNING;
        }
        memcpy(&node->u.mac_filter, l2new, sizeof(struct filter_l2));
        node->nodetype = FILTER_MAC;
        if (set)
        {
            if (qos_access_filter_list_lookup(acllist, node->seqnum, NULL))
            {
                vty_out(vty, "%% filter is already exist.%s", VTY_NEWLINE);
                qos_access_filter_free(node);
                return CMD_WARNING;
            }
            else if (qos_access_filter_list_lookup(acllist, -1, node))
            {
                vty_out(vty, "%% filter is already exist.%s", VTY_NEWLINE);
                qos_access_filter_free(node);
                return CMD_WARNING;
            }
            else
            {
                acllist->seqnum = node->seqnum;
                return qos_access_filter_list_add(acllist, node);
            }
        }
        else
        {
            qos_access_filter_t *delete_filter;

            delete_filter = qos_access_filter_list_lookup(acllist, node->seqnum, NULL);
            if (delete_filter)
            {
                qos_access_filter_free(node);
                return qos_access_filter_list_del(acllist, delete_filter);
            }
            vty_out(vty, "%% can not find this filter.%s", VTY_NEWLINE);
            qos_access_filter_free(node);
            return CMD_WARNING;
        }
    }
    return CMD_WARNING;
}

/*

<1-2147483646> (deny|permit) src-mac any
<1-2147483646> (deny|permit) src-mac MAC MASK
<1-2147483646> (deny|permit) src-mac host MAC

<1-2147483646> (deny|permit) src-mac any dest-mac any
<1-2147483646> (deny|permit) src-mac any dest-mac MAC MASK
<1-2147483646> (deny|permit) src-mac any dest-mac host MAC

<1-2147483646> (deny|permit) src-mac any dest-mac any vlan xxx

<1-2147483646> (deny|permit) src-mac any dest-mac any vlan xxx cos xx

<1-2147483646> (deny|permit) src-mac any dest-mac any vlan xxx cos xx inner-vlan xxx
<1-2147483646> (deny|permit) src-mac any dest-mac any vlan xxx cos xx inner-vlan xxx inner-cos xx
<1-2147483646> (deny|permit) src-mac any dest-mac any vlan xxx cos xx inner-vlan xxx inner-cos xx protocol xxxx
*/
DEFUN(qos_mac_access_list_src_mac_any,
      qos_mac_access_list_src_mac_any_cmd,
      "<1-2147483646> (deny|permit) src-mac any",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = "any";
    char *srcmask = NULL;
    char *dst = NULL;
    char *dstmask = NULL;
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;

    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}
/*
DEFUN(qos_mac_access_list_src_mac_mask,
      qos_mac_access_list_src_mac_mask_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}

DEFUN(qos_mac_access_list_src_mac_host,
      qos_mac_access_list_src_mac_host_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR "",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}
*/
////
DEFUN(qos_mac_access_list_src_mac_any_dstany,
      qos_mac_access_list_src_mac_any_dstany_cmd,
      "<1-2147483646> (deny|permit) src-mac any dest-mac any",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = "any";
    char *srcmask = NULL;
    char *dst = "any";
    char *dstmask = NULL;
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 3)
    {
        if (strstr(argv[2], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[2]);
    }
    else if (argc == 4)
    {
        if (strstr(argv[2], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[2]);

        if (strstr(argv[3], "any"))
            cos = 0;
        else
            cos = atoi(argv[3]);
    }
    else if (argc == 5)
    {
        if (strstr(argv[2], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[2]);

        if (strstr(argv[3], "any"))
            cos = 0;
        else
            cos = atoi(argv[3]);

        proto = eth_protocol_type(argv[4]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(qos_mac_access_list_src_mac_any_dstany,
      qos_mac_access_list_src_mac_any_dstany_vlan_cmd,
      "<1-2147483646> (deny|permit) src-mac any dest-mac any vlan <1-4094>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(qos_mac_access_list_src_mac_any_dstany,
      qos_mac_access_list_src_mac_any_dstany_vlan_cos_cmd,
      "<1-2147483646> (deny|permit) src-mac any dest-mac any vlan (<1-4094>|any) cos <0-7>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(qos_mac_access_list_src_mac_any_dstany,
      qos_mac_access_list_src_mac_any_dstany_vlan_cos_type_cmd,
      "<1-2147483646> (deny|permit) src-mac any dest-mac any vlan (<1-4094>|any) "
      " cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(qos_mac_access_list_src_mac_any_dstmask,
      qos_mac_access_list_src_mac_any_dstmask_cmd,
      "<1-2147483646> (deny|permit) src-mac any dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = "any";
    char *srcmask = NULL;
    char *dst = argv[2];
    char *dstmask = argv[3];
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 5)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);
    }
    else if (argc == 6)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);

        if (strstr(argv[5], "any"))
            cos = 0;
        else
            cos = atoi(argv[5]);
    }
    else if (argc == 7)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);

        if (strstr(argv[5], "any"))
            cos = 0;
        else
            cos = atoi(argv[5]);
        proto = eth_protocol_type(argv[6]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(qos_mac_access_list_src_mac_any_dstmask,
      qos_mac_access_list_src_mac_any_dstmask_vlan_cmd,
      "<1-2147483646> (deny|permit) src-mac any dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR " vlan <1-4094>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(qos_mac_access_list_src_mac_any_dstmask,
      qos_mac_access_list_src_mac_any_dstmask_vlan_cos_cmd,
      "<1-2147483646> (deny|permit) src-mac any dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR " vlan (<1-4094>|any) cos <0-7>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(qos_mac_access_list_src_mac_any_dstmask,
      qos_mac_access_list_src_mac_any_dstmask_vlan_cos_type_cmd,
      "<1-2147483646> (deny|permit) src-mac any dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(qos_mac_access_list_src_mac_any_dsthost,
      qos_mac_access_list_src_mac_any_dsthost_cmd,
      "<1-2147483646> (deny|permit) src-mac any dest-mac host " CMD_MAC_STR "",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = "any";
    char *srcmask = NULL;
    char *dst = argv[2];
    char *dstmask = "host";
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 4)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);
    }
    else if (argc == 5)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);

        if (strstr(argv[4], "any"))
            cos = 0;
        else
            cos = atoi(argv[4]);
    }
    else if (argc == 6)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);

        if (strstr(argv[4], "any"))
            cos = 0;
        else
            cos = atoi(argv[4]);
        proto = eth_protocol_type(argv[5]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(qos_mac_access_list_src_mac_any_dsthost,
      qos_mac_access_list_src_mac_any_dsthost_vlan_cmd,
      "<1-2147483646> (deny|permit) src-mac any dest-mac host " CMD_MAC_STR " vlan <1-4094>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(qos_mac_access_list_src_mac_any_dsthost,
      qos_mac_access_list_src_mac_any_dsthost_vlan_cos_cmd,
      "<1-2147483646> (deny|permit) src-mac any dest-mac host " CMD_MAC_STR " vlan (<1-4094>|any) cos <0-7>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(qos_mac_access_list_src_mac_any_dsthost,
      qos_mac_access_list_src_mac_any_dsthost_vlan_cos_type_cmd,
      "<1-2147483646> (deny|permit) src-mac any dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(qos_mac_access_list_src_mac_mask,
      qos_mac_access_list_src_mac_mask_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[2];
    char *srcmask = argv[3];
    char *dst = NULL;
    char *dstmask = NULL;
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 4)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);
    }
    else if (argc == 5)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);

        if (strstr(argv[4], "any"))
            cos = 0;
        else
            cos = atoi(argv[4]);
    }
    else if (argc == 6)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);

        if (strstr(argv[4], "any"))
            cos = 0;
        else
            cos = atoi(argv[4]);
        proto = eth_protocol_type(argv[5]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}

////
DEFUN(qos_mac_access_list_src_mac_mask_dstany,
      qos_mac_access_list_src_mac_mask_dstany_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac any",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[2];
    char *srcmask = argv[3];
    char *dst = "any";
    char *dstmask = NULL;
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 5)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);
    }
    else if (argc == 6)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);

        if (strstr(argv[5], "any"))
            cos = 0;
        else
            cos = atoi(argv[5]);
    }
    else if (argc == 7)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);

        if (strstr(argv[5], "any"))
            cos = 0;
        else
            cos = atoi(argv[5]);
        proto = eth_protocol_type(argv[6]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(qos_mac_access_list_src_mac_mask_dstany,
      qos_mac_access_list_src_mac_mask_dstany_vlan_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac any vlan <1-4094>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(qos_mac_access_list_src_mac_mask_dstany,
      qos_mac_access_list_src_mac_mask_dstany_vlan_cos_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac any vlan (<1-4094>|any) cos <0-7>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(qos_mac_access_list_src_mac_mask_dstany,
      qos_mac_access_list_src_mac_mask_dstany_vlan_cos_type_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac any vlan (<1-4094>|any) cos (<0-7>|any)"
      " protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(qos_mac_access_list_src_mac_mask_dstmask,
      qos_mac_access_list_src_mac_mask_dstmask_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[2];
    char *srcmask = argv[3];
    char *dst = argv[4];
    char *dstmask = argv[5];
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 7)
    {
        if (strstr(argv[6], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[6]);
    }
    else if (argc == 8)
    {
        if (strstr(argv[6], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[6]);

        if (strstr(argv[7], "any"))
            cos = 0;
        else
            cos = atoi(argv[7]);
    }
    else if (argc == 9)
    {
        if (strstr(argv[6], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[6]);

        if (strstr(argv[7], "any"))
            cos = 0;
        else
            cos = atoi(argv[7]);
        proto = eth_protocol_type(argv[8]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(qos_mac_access_list_src_mac_mask_dstmask,
      qos_mac_access_list_src_mac_mask_dstmask_vlan_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR " vlan <1-4094>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(qos_mac_access_list_src_mac_mask_dstmask,
      qos_mac_access_list_src_mac_mask_dstmask_vlan_cos_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR " vlan (<1-4094>|any) cos <0-7>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(qos_mac_access_list_src_mac_mask_dstmask,
      qos_mac_access_list_src_mac_mask_dstmask_vlan_cos_type_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(qos_mac_access_list_src_mac_mask_dsthost,
      qos_mac_access_list_src_mac_mask_dsthost_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac host " CMD_MAC_STR "",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[2];
    char *srcmask = argv[3];
    char *dst = argv[4];
    char *dstmask = "host";
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 6)
    {
        if (strstr(argv[5], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[5]);
    }
    else if (argc == 7)
    {
        if (strstr(argv[5], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[5]);

        if (strstr(argv[6], "any"))
            cos = 0;
        else
            cos = atoi(argv[6]);
    }
    else if (argc == 8)
    {
        if (strstr(argv[5], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[5]);

        if (strstr(argv[6], "any"))
            cos = 0;
        else
            cos = atoi(argv[6]);
        proto = eth_protocol_type(argv[7]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(qos_mac_access_list_src_mac_mask_dsthost,
      qos_mac_access_list_src_mac_mask_dsthost_vlan_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac host " CMD_MAC_STR " vlan <1-4094>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(qos_mac_access_list_src_mac_mask_dsthost,
      qos_mac_access_list_src_mac_mask_dsthost_vlan_cos_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos <0-7>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(qos_mac_access_list_src_mac_mask_dsthost,
      qos_mac_access_list_src_mac_mask_dsthost_vlan_cos_type_cmd,
      "<1-2147483646> (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(qos_mac_access_list_src_mac_host,
      qos_mac_access_list_src_mac_host_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR "",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[2];
    char *srcmask = "host";
    char *dst = NULL;
    char *dstmask = NULL;
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    /*if(argc == 6)
    {
      if(strstr(argv[5], "any"))
        vlan = 0;
      else
        vlan = atoi(argv[5]);
    }
    else if(argc == 7)
    {
      if(strstr(argv[5], "any"))
        vlan = 0;
      else
        vlan = atoi(argv[5]);

      if(strstr(argv[6], "any"))
        cos = 0;
      else
        cos = atoi(argv[6]);
    }
    else if(argc == 8)
    {
      if(strstr(argv[5], "any"))
        vlan = 0;
      else
        vlan = atoi(argv[5]);

      if(strstr(argv[6], "any"))
        cos = 0;
      else
        cos = atoi(argv[6]);
      proto = eth_protocol_type(argv[7]);
    }*/
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}

DEFUN(qos_mac_access_list_src_mac_host_dstany,
      qos_mac_access_list_src_mac_host_dstany_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR " dest-mac any",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[2];
    char *srcmask = "host";
    char *dst = "any";
    char *dstmask = NULL;
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 4)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);
    }
    else if (argc == 5)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);

        if (strstr(argv[4], "any"))
            cos = 0;
        else
            cos = atoi(argv[4]);
    }
    else if (argc == 6)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);

        if (strstr(argv[4], "any"))
            cos = 0;
        else
            cos = atoi(argv[4]);
        proto = eth_protocol_type(argv[5]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(qos_mac_access_list_src_mac_host_dstany,
      qos_mac_access_list_src_mac_host_dstany_vlan_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR " dest-mac any vlan <1-4094>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(qos_mac_access_list_src_mac_host_dstany,
      qos_mac_access_list_src_mac_host_dstany_vlan_cos_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR " dest-mac any"
      " vlan (<1-4094>|any) cos <0-7>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(qos_mac_access_list_src_mac_host_dstany,
      qos_mac_access_list_src_mac_host_dstany_vlan_cos_type_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR " dest-mac any"
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(qos_mac_access_list_src_mac_host_dstmask,
      qos_mac_access_list_src_mac_host_dstmask_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[2];
    char *srcmask = "host";
    char *dst = argv[3];
    char *dstmask = argv[4];
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 6)
    {
        if (strstr(argv[5], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[5]);
    }
    else if (argc == 7)
    {
        if (strstr(argv[5], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[5]);

        if (strstr(argv[6], "any"))
            cos = 0;
        else
            cos = atoi(argv[6]);
    }
    else if (argc == 8)
    {
        if (strstr(argv[5], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[5]);

        if (strstr(argv[6], "any"))
            cos = 0;
        else
            cos = atoi(argv[6]);
        proto = eth_protocol_type(argv[7]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(qos_mac_access_list_src_mac_host_dstmask,
      qos_mac_access_list_src_mac_host_dstmask_vlan_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan <1-4094>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(qos_mac_access_list_src_mac_host_dstmask,
      qos_mac_access_list_src_mac_host_dstmask_vlan_cos_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan (<1-4094>|any) cos <0-7>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(qos_mac_access_list_src_mac_host_dstmask,
      qos_mac_access_list_src_mac_host_dstmask_vlan_cos_type_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(qos_mac_access_list_src_mac_host_dsthost,
      qos_mac_access_list_src_mac_host_dsthost_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR " dest-mac host " CMD_MAC_STR "",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[2];
    char *srcmask = "host";
    char *dst = argv[3];
    char *dstmask = "host";
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 5)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);
    }
    else if (argc == 6)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);

        if (strstr(argv[5], "any"))
            cos = 0;
        else
            cos = atoi(argv[5]);
    }
    else if (argc == 7)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);

        if (strstr(argv[5], "any"))
            cos = 0;
        else
            cos = atoi(argv[5]);
        proto = eth_protocol_type(argv[6]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(qos_mac_access_list_src_mac_host_dsthost,
      qos_mac_access_list_src_mac_host_dsthost_vlan_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR " dest-mac host " CMD_MAC_STR " vlan <1-4094>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(qos_mac_access_list_src_mac_host_dsthost,
      qos_mac_access_list_src_mac_host_dsthost_vlan_cos_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR " dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos <0-7>",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(qos_mac_access_list_src_mac_host_dsthost,
      qos_mac_access_list_src_mac_host_dsthost_vlan_cos_type_cmd,
      "<1-2147483646> (deny|permit) src-mac host " CMD_MAC_STR " dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Access list Seqnum\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

#endif /* ZPL_FILTER_MAC */

/***************************************************************************/
#ifdef ZPL_FILTER_NOSEQ
#ifdef ZPL_FILTER_ZEBRA_EXT

DEFUN(noseq_qos_access_list_ip_srcany,
      noseq_qos_access_list_ip_srcany_cmd,
      "(deny|permit) any (any|A.B.C.D/M) (any|)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Any\n"
      "Specify Destination Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Any IP Protocol\n")
{
    char *prot_str = "any";
    char *sprefix = "any";
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[1];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(noseq_qos_access_list_ip_dstany,
      noseq_qos_access_list_ip_dstany_cmd,
      "(deny|permit) A.B.C.D/M (any|A.B.C.D/M) (any|)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Any IP Protocol\n")
{
    char *prot_str = "any";
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = "any";
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(noseq_qos_access_list_ip_proany,
      noseq_qos_access_list_ip_proany_cmd,
      "(deny|permit) A.B.C.D/M A.B.C.D/M (any|)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Any IP Protocol\n")
{
    char *prot_str = "any";
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(noseq_qos_access_list_ip_protocol,
      noseq_qos_access_list_ip_protocol_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_IPPROTO_CMD ")",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" 
          IP_IPPROTO_HELP)
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(no_noseq_qos_access_list_ip_protocol,
      no_noseq_qos_access_list_ip_protocol_cmd,
      "no (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_IPPROTO_CMD ")",
      NO_STR
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" 
          IP_IPPROTO_HELP)
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 0);
}

DEFUN(no_noseq_qos_access_list_ip_tcpudp,
      no_noseq_qos_access_list_ip_tcpudp_cmd,
      "no (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ")",
      NO_STR
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP)
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 5)
    {
        sport_op = "eq";
        sport = argv[4];
    }
    else if (argc == 6)
    {
        if (port_operation_type(argv[4]) == OPT_NONE)
        {
            sport_op = argv[4];
            sport = argv[5];
        }
        else
        {
            sport_op = "eq";
            sport = argv[4];
            seport = argv[5];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 0);
}

ALIAS(no_noseq_qos_access_list_ip_tcpudp,
      no_noseq_qos_access_list_ip_tcpudp_name_cmd,
      "no <1-2147483646>",
      NO_STR
      )

DEFUN(noseq_qos_access_list_ip_tcpudp,
      noseq_qos_access_list_ip_tcpudp_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ")",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP)
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 5)
    {
        sport_op = "eq";
        sport = argv[4];
    }
    else if (argc == 6)
    {
        if (port_operation_type(argv[4]) == OPT_NONE)
        {
            sport_op = argv[4];
            sport = argv[5];
        }
        else
        {
            sport_op = "rn";
            sport = argv[4];
            seport = argv[5];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(noseq_qos_access_list_ip_tcpudp,
      noseq_qos_access_list_ip_tcpudp_srcport_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n")

ALIAS(noseq_qos_access_list_ip_tcpudp,
      noseq_qos_access_list_ip_tcpudp_srcport_range_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n")

ALIAS(noseq_qos_access_list_ip_tcpudp,
      noseq_qos_access_list_ip_tcpudp_srcport_ng_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n")

DEFUN(noseq_qos_access_list_ip_tcpudp_dstport,
      noseq_qos_access_list_ip_tcpudp_dstport_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port any dest-port eq <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = "any";
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 5)
    {
        dport_op = "eq";
        dport = argv[4];
    }
    else if (argc == 6)
    {
        if (port_operation_type(argv[4]) == OPT_NONE)
        {
            dport_op = argv[4];
            dport = argv[5];
        }
        else
        {
            dport_op = "rn";
            dport = argv[4];
            deport = argv[5];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(noseq_qos_access_list_ip_tcpudp_dstport,
      noseq_qos_access_list_ip_tcpudp_dstport_range_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port any dest-port range <1-65536> <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(noseq_qos_access_list_ip_tcpudp_dstport,
      noseq_qos_access_list_ip_tcpudp_dstport_ng_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port any dest-port (ne|gt|lt|ge|le) <1-65536>",
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

DEFUN(noseq_qos_access_list_ip_tcpudp_srcport_dstport,
      noseq_qos_access_list_ip_tcpudp_srcport_dstport_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536> dest-port eq <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = argv[4];
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 6)
    {
        dport_op = "eq";
        dport = argv[5];
    }
    else if (argc == 7)
    {
        if (port_operation_type(argv[5]) == OPT_NONE)
        {
            dport_op = argv[5];
            dport = argv[6];
        }
        else
        {
            dport_op = "rn";
            dport = argv[5];
            deport = argv[6];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(noseq_qos_access_list_ip_tcpudp_srcport_dstport,
      noseq_qos_access_list_ip_tcpudp_srcport_dstport_range_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536> dest-port range <1-65536> <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(noseq_qos_access_list_ip_tcpudp_srcport_dstport,
      noseq_qos_access_list_ip_tcpudp_srcport_dstport_ng_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

DEFUN(noseq_qos_access_list_ip_tcpudp_srcport_range_dstport,
      noseq_qos_access_list_ip_tcpudp_srcport_range_dstport_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536> dest-port eq <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = "rn";
    char *sport = argv[4];
    char *seport = argv[5];
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 7)
    {
        dport_op = "eq";
        dport = argv[6];
    }
    else if (argc == 8)
    {
        if (port_operation_type(argv[6]) == OPT_NONE)
        {
            dport_op = argv[6];
            dport = argv[7];
        }
        else
        {
            dport_op = "rn";
            dport = argv[6];
            deport = argv[7];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(noseq_qos_access_list_ip_tcpudp_srcport_range_dstport,
      noseq_qos_access_list_ip_tcpudp_srcport_range_dstport_range_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536> dest-port range <1-65536> <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(noseq_qos_access_list_ip_tcpudp_srcport_range_dstport,
      noseq_qos_access_list_ip_tcpudp_srcport_range_dstport_ng_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

DEFUN(noseq_qos_access_list_ip_tcpudp_srcport_ng_dstport,
      noseq_qos_access_list_ip_tcpudp_srcport_ng_dstport_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536> dest-port eq <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = argv[4];
    char *sport = argv[5];
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 7)
    {
        dport_op = "eq";
        dport = argv[6];
    }
    else if (argc == 8)
    {
        if (port_operation_type(argv[6]) == OPT_NONE)
        {
            dport_op = argv[6];
            dport = argv[7];
        }
        else
        {
            dport_op = "rn";
            dport = argv[6];
            deport = argv[7];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(noseq_qos_access_list_ip_tcpudp_srcport_ng_dstport,
      noseq_qos_access_list_ip_tcpudp_srcport_ng_dstport_range_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536> dest-port range <1-65536> <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(noseq_qos_access_list_ip_tcpudp_srcport_ng_dstport,
      noseq_qos_access_list_ip_tcpudp_srcport_ng_dstport_ng_cmd,
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

#ifdef HAVE_IPV6
/////////////
DEFUN(noseq_qos_access_list_ipv6_srcany,
      noseq_qos_access_list_ipv6_srcany_cmd,
      "(deny|permit) ipv6 any (any|X:X::X:X/M) (any|)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Any\n"
      "Specify Destination Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Any IP Protocol\n")
{
    char *prot_str = "any";
    char *sprefix = "any";
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[1];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(noseq_qos_access_list_ipv6_dstany,
      noseq_qos_access_list_ipv6_dstany_cmd,
      "(deny|permit) ipv6 X:X::X:X/M (any|X:X::X:X/M) (any|)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Any IP Protocol\n")
{
    char *prot_str = "any";
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = "any";
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(noseq_qos_access_list_ipv6_proany,
      noseq_qos_access_list_ipv6_proany_cmd,
      "(deny|permit) ipv6 X:X::X:X/M X:X::X:X/M (any|)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Any IP Protocol\n")
{
    char *prot_str = "any";
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(noseq_qos_access_list_ipv6_protocol,
      noseq_qos_access_list_ipv6_protocol_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_IPPROTO_CMD ")",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" 
          IP_IPPROTO_HELP)
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(no_noseq_qos_access_list_ipv6_protocol,
      no_noseq_qos_access_list_ipv6_protocol_cmd,
      "no (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_IPPROTO_CMD ")",
      NO_STR
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" 
          IP_IPPROTO_HELP)
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 0);
}

DEFUN(no_noseq_qos_access_list_ipv6_tcpudp,
      no_noseq_qos_access_list_ipv6_tcpudp_cmd,
      "no (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ")",
      NO_STR
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP)
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 5)
    {
        sport_op = "eq";
        sport = argv[4];
    }
    else if (argc == 6)
    {
        if (port_operation_type(argv[4]) == OPT_NONE)
        {
            sport_op = argv[4];
            sport = argv[5];
        }
        else
        {
            sport_op = "eq";
            sport = argv[4];
            seport = argv[5];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 0);
}

ALIAS(no_noseq_qos_access_list_ipv6_tcpudp,
      no_noseq_qos_access_list_ipv6_tcpudp_name_cmd,
      "no ipv6",
      NO_STR
      IPV6_STR)

DEFUN(noseq_qos_access_list_ipv6_tcpudp,
      noseq_qos_access_list_ipv6_tcpudp_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ")",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP)
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = NULL;
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 5)
    {
        sport_op = "eq";
        sport = argv[4];
    }
    else if (argc == 6)
    {
        if (port_operation_type(argv[4]) == OPT_NONE)
        {
            sport_op = argv[4];
            sport = argv[5];
        }
        else
        {
            sport_op = "rn";
            sport = argv[4];
            seport = argv[5];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(noseq_qos_access_list_ipv6_tcpudp,
      noseq_qos_access_list_ipv6_tcpudp_srcport_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n")

ALIAS(noseq_qos_access_list_ipv6_tcpudp,
      noseq_qos_access_list_ipv6_tcpudp_srcport_range_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n")

ALIAS(noseq_qos_access_list_ipv6_tcpudp,
      noseq_qos_access_list_ipv6_tcpudp_srcport_ng_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n")

DEFUN(noseq_qos_access_list_ipv6_tcpudp_dstport,
      noseq_qos_access_list_ipv6_tcpudp_dstport_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port any dest-port eq <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = "any";
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 5)
    {
        dport_op = "eq";
        dport = argv[4];
    }
    else if (argc == 6)
    {
        if (port_operation_type(argv[4]) == OPT_NONE)
        {
            dport_op = argv[4];
            dport = argv[5];
        }
        else
        {
            dport_op = "rn";
            dport = argv[4];
            deport = argv[5];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(noseq_qos_access_list_ipv6_tcpudp_dstport,
      noseq_qos_access_list_ipv6_tcpudp_dstport_range_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port any dest-port range <1-65536> <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(noseq_qos_access_list_ipv6_tcpudp_dstport,
      noseq_qos_access_list_ipv6_tcpudp_dstport_ng_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port any dest-port (ne|gt|lt|ge|le) <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

DEFUN(noseq_qos_access_list_ipv6_tcpudp_srcport_dstport,
      noseq_qos_access_list_ipv6_tcpudp_srcport_dstport_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536> dest-port eq <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = NULL;
    char *sport = argv[4];
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 6)
    {
        dport_op = "eq";
        dport = argv[5];
    }
    else if (argc == 7)
    {
        if (port_operation_type(argv[5]) == OPT_NONE)
        {
            dport_op = argv[5];
            dport = argv[6];
        }
        else
        {
            dport_op = "rn";
            dport = argv[5];
            deport = argv[6];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(noseq_qos_access_list_ipv6_tcpudp_srcport_dstport,
      noseq_qos_access_list_ipv6_tcpudp_srcport_dstport_range_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536> dest-port range <1-65536> <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(noseq_qos_access_list_ipv6_tcpudp_srcport_dstport,
      noseq_qos_access_list_ipv6_tcpudp_srcport_dstport_ng_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port eq <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

DEFUN(noseq_qos_access_list_ipv6_tcpudp_srcport_range_dstport,
      noseq_qos_access_list_ipv6_tcpudp_srcport_range_dstport_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536> dest-port eq <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = "rn";
    char *sport = argv[4];
    char *seport = argv[5];
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 7)
    {
        dport_op = "eq";
        dport = argv[6];
    }
    else if (argc == 8)
    {
        if (port_operation_type(argv[6]) == OPT_NONE)
        {
            dport_op = argv[6];
            dport = argv[7];
        }
        else
        {
            dport_op = "rn";
            dport = argv[6];
            deport = argv[7];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(noseq_qos_access_list_ipv6_tcpudp_srcport_range_dstport,
      noseq_qos_access_list_ipv6_tcpudp_srcport_range_dstport_range_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536> dest-port range <1-65536> <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(noseq_qos_access_list_ipv6_tcpudp_srcport_range_dstport,
      noseq_qos_access_list_ipv6_tcpudp_srcport_range_dstport_ng_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port range <1-65536> <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")

DEFUN(noseq_qos_access_list_ipv6_tcpudp_srcport_ng_dstport,
      noseq_qos_access_list_ipv6_tcpudp_srcport_ng_dstport_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536> dest-port eq <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
    char *prot_str = argv[3];
    char *sprefix = argv[1];
    char *sport_op = argv[4];
    char *sport = argv[5];
    char *seport = NULL;
    char *dprefix = argv[2];
    char *dport_op = NULL;
    char *dport = NULL;
    char *deport = NULL;
    if (argc == 7)
    {
        dport_op = "eq";
        dport = argv[6];
    }
    else if (argc == 8)
    {
        if (port_operation_type(argv[6]) == OPT_NONE)
        {
            dport_op = argv[6];
            dport = argv[7];
        }
        else
        {
            dport_op = "rn";
            dport = argv[6];
            deport = argv[7];
        }
    }
    return qos_access_filter_zebos_extended_adddel(vty->index, vty, NULL, argv[0], AFI_IP6, prot_str,
                                                   sprefix, sport_op, sport, seport,
                                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(noseq_qos_access_list_ipv6_tcpudp_srcport_ng_dstport,
      noseq_qos_access_list_ipv6_tcpudp_srcport_ng_dstport_range_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536> dest-port range <1-65536> <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n")

ALIAS(noseq_qos_access_list_ipv6_tcpudp_srcport_ng_dstport,
      noseq_qos_access_list_ipv6_tcpudp_srcport_ng_dstport_ng_cmd,
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD ") src-port (ne|gt|lt|ge|le) <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n")
#endif
#endif

#ifdef ZPL_FILTER_MAC

/*
(deny|permit) src-mac any
(deny|permit) src-mac MAC MASK
(deny|permit) src-mac host MAC

(deny|permit) src-mac any dest-mac any
(deny|permit) src-mac any dest-mac MAC MASK
(deny|permit) src-mac any dest-mac host MAC

(deny|permit) src-mac any dest-mac any vlan xxx

(deny|permit) src-mac any dest-mac any vlan xxx cos xx

(deny|permit) src-mac any dest-mac any vlan xxx cos xx inner-vlan xxx
(deny|permit) src-mac any dest-mac any vlan xxx cos xx inner-vlan xxx inner-cos xx
(deny|permit) src-mac any dest-mac any vlan xxx cos xx inner-vlan xxx inner-cos xx protocol xxxx
*/
DEFUN(noseq_qos_mac_access_list_src_mac_any,
      noseq_qos_mac_access_list_src_mac_any_cmd,
      "(deny|permit) src-mac any",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = "any";
    char *srcmask = NULL;
    char *dst = NULL;
    char *dstmask = NULL;
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;

    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}
/*
DEFUN(noseq_qos_mac_access_list_src_mac_mask,
      noseq_qos_mac_access_list_src_mac_mask_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}

DEFUN(noseq_qos_mac_access_list_src_mac_host,
      noseq_qos_mac_access_list_src_mac_host_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR "",
      
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}
*/
////
DEFUN(noseq_qos_mac_access_list_src_mac_any_dstany,
      noseq_qos_mac_access_list_src_mac_any_dstany_cmd,
      "(deny|permit) src-mac any dest-mac any",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = "any";
    char *srcmask = NULL;
    char *dst = "any";
    char *dstmask = NULL;
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 2)
    {
        if (strstr(argv[1], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[1]);
    }
    else if (argc == 3)
    {
        if (strstr(argv[1], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[1]);

        if (strstr(argv[2], "any"))
            cos = 0;
        else
            cos = atoi(argv[2]);
    }
    else if (argc == 4)
    {
        if (strstr(argv[1], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[1]);

        if (strstr(argv[2], "any"))
            cos = 0;
        else
            cos = atoi(argv[2]);

        proto = eth_protocol_type(argv[3]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}

ALIAS(noseq_qos_mac_access_list_src_mac_any_dstany,
      noseq_qos_mac_access_list_src_mac_any_dstany_vlan_cmd,
      "(deny|permit) src-mac any dest-mac any vlan <1-4094>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(noseq_qos_mac_access_list_src_mac_any_dstany,
      noseq_qos_mac_access_list_src_mac_any_dstany_vlan_cos_cmd,
      "(deny|permit) src-mac any dest-mac any vlan (<1-4094>|any) cos <0-7>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(noseq_qos_mac_access_list_src_mac_any_dstany,
      noseq_qos_mac_access_list_src_mac_any_dstany_vlan_cos_type_cmd,
      "(deny|permit) src-mac any dest-mac any vlan (<1-4094>|any) "
      " cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(noseq_qos_mac_access_list_src_mac_any_dstmask,
      noseq_qos_mac_access_list_src_mac_any_dstmask_cmd,
      "(deny|permit) src-mac any dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = "any";
    char *srcmask = NULL;
    char *dst = argv[1];
    char *dstmask = argv[2];
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 4)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);
    }
    else if (argc == 5)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);

        if (strstr(argv[4], "any"))
            cos = 0;
        else
            cos = atoi(argv[4]);
    }
    else if (argc == 6)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);

        if (strstr(argv[4], "any"))
            cos = 0;
        else
            cos = atoi(argv[4]);
        proto = eth_protocol_type(argv[5]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}

ALIAS(noseq_qos_mac_access_list_src_mac_any_dstmask,
      noseq_qos_mac_access_list_src_mac_any_dstmask_vlan_cmd,
      "(deny|permit) src-mac any dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR " vlan <1-4094>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(noseq_qos_mac_access_list_src_mac_any_dstmask,
      noseq_qos_mac_access_list_src_mac_any_dstmask_vlan_cos_cmd,
      "(deny|permit) src-mac any dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR " vlan (<1-4094>|any) cos <0-7>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(noseq_qos_mac_access_list_src_mac_any_dstmask,
      noseq_qos_mac_access_list_src_mac_any_dstmask_vlan_cos_type_cmd,
      "(deny|permit) src-mac any dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(noseq_qos_mac_access_list_src_mac_any_dsthost,
      noseq_qos_mac_access_list_src_mac_any_dsthost_cmd,
      "(deny|permit) src-mac any dest-mac host " CMD_MAC_STR "",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = "any";
    char *srcmask = NULL;
    char *dst = argv[1];
    char *dstmask = "host";
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 3)
    {
        if (strstr(argv[2], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[2]);
    }
    else if (argc == 4)
    {
        if (strstr(argv[2], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[2]);

        if (strstr(argv[3], "any"))
            cos = 0;
        else
            cos = atoi(argv[3]);
    }
    else if (argc == 5)
    {
        if (strstr(argv[2], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[2]);

        if (strstr(argv[3], "any"))
            cos = 0;
        else
            cos = atoi(argv[3]);
        proto = eth_protocol_type(argv[4]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}

ALIAS(noseq_qos_mac_access_list_src_mac_any_dsthost,
      noseq_qos_mac_access_list_src_mac_any_dsthost_vlan_cmd,
      "(deny|permit) src-mac any dest-mac host " CMD_MAC_STR " vlan <1-4094>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(noseq_qos_mac_access_list_src_mac_any_dsthost,
      noseq_qos_mac_access_list_src_mac_any_dsthost_vlan_cos_cmd,
      "(deny|permit) src-mac any dest-mac host " CMD_MAC_STR " vlan (<1-4094>|any) cos <0-7>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(noseq_qos_mac_access_list_src_mac_any_dsthost,
      noseq_qos_mac_access_list_src_mac_any_dsthost_vlan_cos_type_cmd,
      "(deny|permit) src-mac any dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(noseq_qos_mac_access_list_src_mac_mask,
      noseq_qos_mac_access_list_src_mac_mask_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[1];
    char *srcmask = argv[2];
    char *dst = NULL;
    char *dstmask = NULL;
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 3)
    {
        if (strstr(argv[2], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[2]);
    }
    else if (argc == 4)
    {
        if (strstr(argv[2], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[2]);

        if (strstr(argv[3], "any"))
            cos = 0;
        else
            cos = atoi(argv[3]);
    }
    else if (argc == 5)
    {
        if (strstr(argv[2], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[2]);

        if (strstr(argv[3], "any"))
            cos = 0;
        else
            cos = atoi(argv[3]);
        proto = eth_protocol_type(argv[4]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}

////
DEFUN(noseq_qos_mac_access_list_src_mac_mask_dstany,
      noseq_qos_mac_access_list_src_mac_mask_dstany_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac any",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[1];
    char *srcmask = argv[2];
    char *dst = "any";
    char *dstmask = NULL;
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 4)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);
    }
    else if (argc == 5)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);

        if (strstr(argv[4], "any"))
            cos = 0;
        else
            cos = atoi(argv[4]);
    }
    else if (argc == 6)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);

        if (strstr(argv[4], "any"))
            cos = 0;
        else
            cos = atoi(argv[4]);
        proto = eth_protocol_type(argv[5]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}

ALIAS(noseq_qos_mac_access_list_src_mac_mask_dstany,
      noseq_qos_mac_access_list_src_mac_mask_dstany_vlan_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac any vlan <1-4094>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(noseq_qos_mac_access_list_src_mac_mask_dstany,
      noseq_qos_mac_access_list_src_mac_mask_dstany_vlan_cos_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac any vlan (<1-4094>|any) cos <0-7>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(noseq_qos_mac_access_list_src_mac_mask_dstany,
      noseq_qos_mac_access_list_src_mac_mask_dstany_vlan_cos_type_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac any vlan (<1-4094>|any) cos (<0-7>|any)"
      " protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(noseq_qos_mac_access_list_src_mac_mask_dstmask,
      noseq_qos_mac_access_list_src_mac_mask_dstmask_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[1];
    char *srcmask = argv[2];
    char *dst = argv[3];
    char *dstmask = argv[4];
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 6)
    {
        if (strstr(argv[5], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[5]);
    }
    else if (argc == 7)
    {
        if (strstr(argv[5], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[5]);

        if (strstr(argv[6], "any"))
            cos = 0;
        else
            cos = atoi(argv[6]);
    }
    else if (argc == 8)
    {
        if (strstr(argv[5], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[5]);

        if (strstr(argv[6], "any"))
            cos = 0;
        else
            cos = atoi(argv[6]);
        proto = eth_protocol_type(argv[7]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}

ALIAS(noseq_qos_mac_access_list_src_mac_mask_dstmask,
      noseq_qos_mac_access_list_src_mac_mask_dstmask_vlan_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR " vlan <1-4094>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(noseq_qos_mac_access_list_src_mac_mask_dstmask,
      noseq_qos_mac_access_list_src_mac_mask_dstmask_vlan_cos_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR " vlan (<1-4094>|any) cos <0-7>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(noseq_qos_mac_access_list_src_mac_mask_dstmask,
      noseq_qos_mac_access_list_src_mac_mask_dstmask_vlan_cos_type_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(noseq_qos_mac_access_list_src_mac_mask_dsthost,
      noseq_qos_mac_access_list_src_mac_mask_dsthost_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac host " CMD_MAC_STR "",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[1];
    char *srcmask = argv[2];
    char *dst = argv[3];
    char *dstmask = "host";
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 5)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);
    }
    else if (argc == 6)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);

        if (strstr(argv[5], "any"))
            cos = 0;
        else
            cos = atoi(argv[5]);
    }
    else if (argc == 7)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);

        if (strstr(argv[5], "any"))
            cos = 0;
        else
            cos = atoi(argv[5]);
        proto = eth_protocol_type(argv[6]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}

ALIAS(noseq_qos_mac_access_list_src_mac_mask_dsthost,
      noseq_qos_mac_access_list_src_mac_mask_dsthost_vlan_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac host " CMD_MAC_STR " vlan <1-4094>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(noseq_qos_mac_access_list_src_mac_mask_dsthost,
      noseq_qos_mac_access_list_src_mac_mask_dsthost_vlan_cos_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos <0-7>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(noseq_qos_mac_access_list_src_mac_mask_dsthost,
      noseq_qos_mac_access_list_src_mac_mask_dsthost_vlan_cos_type_cmd,
      "(deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(noseq_qos_mac_access_list_src_mac_host,
      noseq_qos_mac_access_list_src_mac_host_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR "",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[1];
    char *srcmask = "host";
    char *dst = NULL;
    char *dstmask = NULL;
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    /*if(argc == 5)
    {
      if(strstr(argv[4], "any"))
        vlan = 0;
      else
        vlan = atoi(argv[4]);
    }
    else if(argc == 6)
    {
      if(strstr(argv[4], "any"))
        vlan = 0;
      else
        vlan = atoi(argv[4]);

      if(strstr(argv[5], "any"))
        cos = 0;
      else
        cos = atoi(argv[5]);
    }
    else if(argc == 7)
    {
      if(strstr(argv[4], "any"))
        vlan = 0;
      else
        vlan = atoi(argv[4]);

      if(strstr(argv[5], "any"))
        cos = 0;
      else
        cos = atoi(argv[5]);
      proto = eth_protocol_type(argv[6]);
    }*/
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}

DEFUN(noseq_qos_mac_access_list_src_mac_host_dstany,
      noseq_qos_mac_access_list_src_mac_host_dstany_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR " dest-mac any",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[1];
    char *srcmask = "host";
    char *dst = "any";
    char *dstmask = NULL;
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 3)
    {
        if (strstr(argv[2], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[2]);
    }
    else if (argc == 4)
    {
        if (strstr(argv[2], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[2]);

        if (strstr(argv[3], "any"))
            cos = 0;
        else
            cos = atoi(argv[3]);
    }
    else if (argc == 5)
    {
        if (strstr(argv[2], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[2]);

        if (strstr(argv[3], "any"))
            cos = 0;
        else
            cos = atoi(argv[3]);
        proto = eth_protocol_type(argv[4]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}

ALIAS(noseq_qos_mac_access_list_src_mac_host_dstany,
      noseq_qos_mac_access_list_src_mac_host_dstany_vlan_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR " dest-mac any vlan <1-4094>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(noseq_qos_mac_access_list_src_mac_host_dstany,
      noseq_qos_mac_access_list_src_mac_host_dstany_vlan_cos_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR " dest-mac any"
      " vlan (<1-4094>|any) cos <0-7>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(noseq_qos_mac_access_list_src_mac_host_dstany,
      noseq_qos_mac_access_list_src_mac_host_dstany_vlan_cos_type_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR " dest-mac any"
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(noseq_qos_mac_access_list_src_mac_host_dstmask,
      noseq_qos_mac_access_list_src_mac_host_dstmask_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[1];
    char *srcmask = "host";
    char *dst = argv[2];
    char *dstmask = argv[3];
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 5)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);
    }
    else if (argc == 6)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);

        if (strstr(argv[5], "any"))
            cos = 0;
        else
            cos = atoi(argv[5]);
    }
    else if (argc == 7)
    {
        if (strstr(argv[4], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[4]);

        if (strstr(argv[5], "any"))
            cos = 0;
        else
            cos = atoi(argv[5]);
        proto = eth_protocol_type(argv[6]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}

ALIAS(noseq_qos_mac_access_list_src_mac_host_dstmask,
      noseq_qos_mac_access_list_src_mac_host_dstmask_vlan_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan <1-4094>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(noseq_qos_mac_access_list_src_mac_host_dstmask,
      noseq_qos_mac_access_list_src_mac_host_dstmask_vlan_cos_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan (<1-4094>|any) cos <0-7>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(noseq_qos_mac_access_list_src_mac_host_dstmask,
      noseq_qos_mac_access_list_src_mac_host_dstmask_vlan_cos_type_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(noseq_qos_mac_access_list_src_mac_host_dsthost,
      noseq_qos_mac_access_list_src_mac_host_dsthost_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR " dest-mac host " CMD_MAC_STR "",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
    struct filter_l2 l2new;
    char *src = argv[1];
    char *srcmask = "host";
    char *dst = argv[2];
    char *dstmask = "host";
    int proto = -1;
    int vlan = -1;
    int cos = -1;
    int innervlan = -1;
    int innercos = -1;
    int label = -1;
    int exp = -1;
    int innerlabel = -1;
    int innerexp = -1;
    if (argc == 4)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);
    }
    else if (argc == 5)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);

        if (strstr(argv[4], "any"))
            cos = 0;
        else
            cos = atoi(argv[4]);
    }
    else if (argc == 6)
    {
        if (strstr(argv[3], "any"))
            vlan = 0;
        else
            vlan = atoi(argv[3]);

        if (strstr(argv[4], "any"))
            cos = 0;
        else
            cos = atoi(argv[4]);
        proto = eth_protocol_type(argv[5]);
    }
    filter_l2mac_format(vty, src, srcmask,
                        dst, dstmask,
                        proto,
                        vlan, cos, innervlan, innercos,
                        label, exp, innerlabel, innerexp,
                        &l2new);
    return qos_access_filter_l2mac_adddel(vty->index, vty, NULL, argv[0], 0, &l2new, 1);
}

ALIAS(noseq_qos_mac_access_list_src_mac_host_dsthost,
      noseq_qos_mac_access_list_src_mac_host_dsthost_vlan_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR " dest-mac host " CMD_MAC_STR " vlan <1-4094>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(noseq_qos_mac_access_list_src_mac_host_dsthost,
      noseq_qos_mac_access_list_src_mac_host_dsthost_vlan_cos_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR " dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos <0-7>",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(noseq_qos_mac_access_list_src_mac_host_dsthost,
      noseq_qos_mac_access_list_src_mac_host_dsthost_vlan_cos_type_cmd,
      "(deny|permit) src-mac host " CMD_MAC_STR " dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

#endif /* ZPL_FILTER_MAC */


/* Install vty related command. */
static void noseq_qos_access_list_ipv4(void)
{
#ifdef ZPL_FILTER_ZEBRA_EXT
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_srcany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_dstany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_proany_cmd);

    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &no_noseq_qos_access_list_ip_tcpudp_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &no_noseq_qos_access_list_ip_tcpudp_name_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_protocol_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &no_noseq_qos_access_list_ip_protocol_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_srcport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_srcport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_srcport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_dstport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_srcport_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_srcport_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_srcport_dstport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_srcport_range_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_srcport_range_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_srcport_range_dstport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_srcport_ng_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_srcport_ng_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ip_tcpudp_srcport_ng_dstport_ng_cmd);
#endif
}
#ifdef HAVE_IPV6
static void noseq_qos_access_list_ipv6(void)
{
#ifdef ZPL_FILTER_ZEBRA_EXT
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_srcany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_dstany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_proany_cmd);

    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &no_noseq_qos_access_list_ipv6_tcpudp_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &no_noseq_qos_access_list_ipv6_tcpudp_name_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_protocol_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &no_noseq_qos_access_list_ipv6_protocol_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_srcport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_srcport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_srcport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_dstport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_srcport_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_srcport_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_srcport_dstport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_srcport_range_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_srcport_range_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_srcport_range_dstport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_srcport_ng_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_srcport_ng_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_access_list_ipv6_tcpudp_srcport_ng_dstport_ng_cmd);
#endif
}
#endif /* HAVE_IPV6 */

#ifdef ZPL_FILTER_MAC
/* Install vty related command. */
static void noseq_qos_mac_access_list_mac(void)
{
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_any_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_any_dstany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_any_dstany_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_any_dstany_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_any_dstany_vlan_cos_type_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_any_dstmask_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_any_dstmask_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_any_dstmask_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_any_dstmask_vlan_cos_type_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_any_dsthost_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_any_dsthost_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_any_dsthost_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_any_dsthost_vlan_cos_type_cmd);

    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_mask_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_mask_dstany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_mask_dstany_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_mask_dstany_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_mask_dstany_vlan_cos_type_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_mask_dstmask_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_mask_dstmask_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_mask_dstmask_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_mask_dstmask_vlan_cos_type_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_mask_dsthost_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_mask_dsthost_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_mask_dsthost_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_mask_dsthost_vlan_cos_type_cmd);

    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_host_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_host_dstany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_host_dstany_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_host_dstany_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_host_dstany_vlan_cos_type_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_host_dstmask_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_host_dstmask_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_host_dstmask_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_host_dstmask_vlan_cos_type_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_host_dsthost_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_host_dsthost_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_host_dsthost_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &noseq_qos_mac_access_list_src_mac_host_dsthost_vlan_cos_type_cmd);
}
#endif
#endif /*ZPL_FILTER_NOSEQ*/
/***************************************************************************/
////////////////////////////////////////////////////////

DEFUN(qos_access_list_name,
      qos_access_list_name_cmd,
      "access-list WORD",
      "Add an access list entry\n"
      "Access-list Name\n")
{
    qos_access_filter_list_t *policybind = NULL;
    policybind = qos_access_list_lookup(argv[0]);
    if (policybind == NULL)
        policybind = qos_access_list_create(argv[0]);
    if (policybind)
    {
        vty->index = policybind;
        vty->node = QOS_ACCESS_NODE;
    }
    else
    {
		vty_out(vty, "access-list can not create%s",VTY_NEWLINE);
		return CMD_WARNING;
    }
    return (policybind != NULL) ? CMD_SUCCESS : CMD_WARNING;
}
DEFUN(no_qos_access_list_name,
      no_qos_access_list_name_cmd,
      "no access-list WORD",
      NO_STR
      "An access list entry\n"
      "Access list Name\n")
{
    int ret = ERROR;
    if(qos_access_list_lookup(argv[0]))
        ret = qos_access_list_destroy(argv[0]);
    else
    {
		vty_out(vty, "this access-list '%s' is not exist%s",argv[0],VTY_NEWLINE);
		return CMD_WARNING;
    }
    return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}


DEFUN(qos_class_map_match,
      qos_class_map_match_cmd,
      "class-map WORD",
      QOS_CLASS_MAP_STR
      "Class Map Name\n")
{
    qos_class_map_t *classmap = NULL;
    char *name = argv[0];
    if(argc == 2)
    {
        name = argv[1];
    }
    classmap = qos_class_map_lookup(name);
    if (classmap == NULL)
    {
        classmap = qos_class_map_create(name);
        if(classmap && argc == 1)
        {
            classmap->match_type = NSM_QOS_MATCH_ANY;
        }
        else if(classmap && argc == 2)
        {
            if(strstr(argv[0],"any"))
                classmap->match_type = NSM_QOS_MATCH_ANY;
            else
                classmap->match_type = NSM_QOS_MATCH_ALL;    
        }
        else
        {
            vty_out(vty, "can not create class-map '%s'%s",argv[0],VTY_NEWLINE);
            return CMD_WARNING;
        }
    }
    if (classmap)
    {
        vty->index = classmap;
        vty->node = QOS_CLASS_MAP_NODE;
    }
    return (classmap != NULL) ? CMD_SUCCESS : CMD_WARNING;
}

ALIAS(qos_class_map_match,
      qos_class_map_match_any_cmd,
      "class-map (match-all|match-any) WORD",
      QOS_CLASS_MAP_STR
      "Match All Of This List\n"
      "Match Any Of This List\n"
      "Class Map Name\n")


DEFUN(no_qos_class_map_match,
      no_qos_class_map_match_cmd,
      "no class-map WORD",
      NO_STR
      QOS_CLASS_MAP_STR
      "Class Map Name\n")
{
    int ret = ERROR;
    qos_class_map_t *classmap = qos_class_map_lookup(argv[0]);
    if (classmap)
        ret = qos_class_map_del(classmap);
    else
    {
		vty_out(vty, "this class-map '%s' is not exist%s",argv[0],VTY_NEWLINE);
		return CMD_WARNING;
    }
    return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}


DEFUN(qos_class_map_match_access,
      qos_class_map_match_access_cmd,
      "match access-group WORD",
      "Match Type\n"
      "Match Access Group List\n"
      "Access List Name\n")
{
    int ret = ERROR;
    if(qos_access_list_lookup(argv[0]) == NULL)
    {
		vty_out(vty, "the access-list '%s' is not exist%s",argv[0], VTY_NEWLINE);
		return CMD_WARNING;
    }
    if(qos_class_map_bind_access_list_get((qos_class_map_t *)vty->index))
    {
		vty_out(vty, "match access-group is already set%s",VTY_NEWLINE);
		return CMD_WARNING;
    }
    ret = qos_class_map_bind_access_list_set((qos_class_map_t *)vty->index, argv[0], zpl_true);

    return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_qos_class_map_match_access,
      no_qos_class_map_match_access_cmd,
      "no match access-group",
      NO_STR
      "Match Type\n"
      "Match Access Group List\n")
{
    int ret = ERROR;
    if(!qos_class_map_bind_access_list_get((qos_class_map_t *)vty->index))
    {
		vty_out(vty, "match access-group is not set%s",VTY_NEWLINE);
		return CMD_WARNING;
    }
    ret = qos_class_map_bind_access_list_set((qos_class_map_t *)vty->index, NULL, zpl_false);

    return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}


DEFUN (qos_class_map_cir,
		qos_class_map_cir_cmd,
		"policer cir <1-10000000>" ,
		"Policer Rate Limit\n"
		"Committed Information Rate\n"
		"Cir Value\n")
{
	int ret = ERROR;
	nsm_qos_limit_t rate;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	memset(&rate, 0, sizeof(rate));

	ret = qos_class_map_limit_get((qos_class_map_t *)vty->index, &rate);	

	if(argc == 1)
	{
		rate.qos_cir = atoi(argv[0]);
	}
	if(argc == 2)
	{
		rate.qos_cir = atoi(argv[0]);
		rate.qos_pir = atoi(argv[1]);
	}
	if(argc == 3)
	{
		rate.qos_cir = atoi(argv[0]);
		rate.qos_pir = atoi(argv[1]);
		rate.qos_cbs = atoi(argv[2]);
	}
	ret = qos_class_map_limit_set((qos_class_map_t *)vty->index, &rate);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

ALIAS (qos_class_map_cir,
		qos_class_map_cir_pir_cmd,
		"policer cir <1-10000000> pir <0-4000000>" ,
		"Policer Rate Limit\n"
		"Committed Information Rate\n"
		"Cir Value\n"
		"Peak Information Rate\n"
		"Pir Value\n")

ALIAS (qos_class_map_cir,
		qos_class_map_cir_pir_cbs_cmd,
		"policer cir <1-10000000> pir <0-4000000> cbs <0-4000000>",
		"Policer Rate Limit\n"
		"Committed Information Rate\n"
		"Cir Value\n"
		"Peak Information Rate\n"
		"Pir Value\n"
		"Committed Burst Size\n"
		"Cbs Value\n")

DEFUN (no_qos_class_map_cir,
		no_qos_class_map_cir_cmd,
		"no policer",
		NO_STR
		"Policer Rate Limit\n")
{
	int ret = ERROR;
	nsm_qos_limit_t rate;
	if(!nsm_qos_global_get())
	{
		vty_out(vty, "Please Enable Qos frist.%s",VTY_NEWLINE);
		return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
	}
	memset(&rate, 0, sizeof(rate));
	ret = qos_class_map_limit_set((qos_class_map_t *)vty->index, &rate);	
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

///////////////////////////////////////////
DEFUN(qos_service_policy_match,
      qos_service_policy_match_cmd,
      "policy-map WORD",
      QOS_POLICY_MAP_STR
      "Policy Map Name\n")
{
    qos_service_policy_t *policy = NULL;
    policy = qos_service_policy_lookup(argv[0]);
    if (policy == NULL)
    {
        policy = qos_service_policy_create(argv[0]);
        if(policy == NULL)
        {
            vty_out(vty, "can not create policy-map '%s'%s",argv[0],VTY_NEWLINE);
            return CMD_WARNING;
        }
    }
    if (policy)
    {
        vty->index = policy;
        vty->node = QOS_POLICY_MAP_NODE;
    }
    return (policy != NULL) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(no_qos_service_policy_match,
      no_qos_service_policy_match_cmd,
      "no policy-map WORD",
      NO_STR
          QOS_POLICY_MAP_STR
      "Policy Map Name\n")
{
    int ret = ERROR;
    qos_service_policy_t *policy = qos_service_policy_lookup(argv[0]);
    if (policy)
        ret = qos_service_policy_del(policy);
    else
    {
		vty_out(vty, "this policy-map '%s' is not exist%s",argv[0],VTY_NEWLINE);
		return CMD_WARNING;
    }
    return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}

DEFUN(qos_service_policy_class_match,
      qos_service_policy_class_match_cmd,
      "class-match WORD",
      QOS_CLASS_MAP_STR
      "Class Map Name\n")
{
    int ret = ERROR;
    if(qos_class_map_lookup(argv[0]) == NULL)
    {
		vty_out(vty, "the class-map '%s' is not exist%s",argv[0], VTY_NEWLINE);
		return CMD_WARNING;
    }
    if(qos_service_policy_bind_class_map_get((qos_service_policy_t *)vty->index))
    {
		vty_out(vty, "match access-group is already set%s",VTY_NEWLINE);
		return CMD_WARNING;
    }
    ret = qos_service_policy_bind_class_map_set((qos_service_policy_t *)vty->index, argv[0], zpl_true);

    return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}


DEFUN(no_qos_service_policy_class_match,
      no_qos_service_policy_class_match_cmd,
      "no class-match",
      NO_STR
      QOS_CLASS_MAP_STR)
{
    int ret = ERROR;
    if(!qos_service_policy_bind_class_map_get((qos_service_policy_t *)vty->index))
    {
		vty_out(vty, "match access-group is not set%s",VTY_NEWLINE);
		return CMD_WARNING;
    }
    ret = qos_service_policy_bind_class_map_set((qos_service_policy_t *)vty->index, NULL, zpl_false);

    return (ret == OK) ? CMD_SUCCESS : CMD_WARNING;
}




static int nsm_access_list_write_config(struct vty *vty)
{
    //vty_out(vty, "nsm_access_list_write_config%s", VTY_NEWLINE);
    qos_access_list_write_config(vty, NULL);
    vty_out(vty, "!%s", VTY_NEWLINE);
    return 1;
}

static int nsm_class_map_write_config(struct vty *vty)
{
    qos_class_map_write_config(vty, NULL);
    vty_out(vty, "!%s", VTY_NEWLINE);
    return 1;
}

static int nsm_service_policy_write_config(struct vty *vty)
{
    qos_service_policy_write_config(vty, NULL);
    vty_out(vty, "!%s", VTY_NEWLINE);
    return 1;
}

static struct cmd_node access_list_node =
    {
        QOS_ACCESS_NODE,
        "%s(config-access-list)# ",
};

static struct cmd_node class_map_node =
    {
        QOS_CLASS_MAP_NODE,
        "%s(config-class-map)# ",
};
static struct cmd_node policy_map_node =
    {
        QOS_POLICY_MAP_NODE,
        "%s(config-policy-map)# ",
};

static struct cmd_node policy_class_map_node =
    {
        QOS_POLICY_CLASS_MAP_NODE,
        "%s(config-policy-map-c)# ",
};

/* Install vty related command. */
static void qos_access_list_ipv4(void)
{
#ifdef ZPL_FILTER_ZEBRA_EXT

    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_srcany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_dstany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_proany_cmd);

    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &no_qos_access_list_ip_tcpudp_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &no_qos_access_list_ip_tcpudp_name_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_protocol_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &no_qos_access_list_ip_protocol_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_srcport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_srcport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_srcport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_dstport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_srcport_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_srcport_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_srcport_dstport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_srcport_range_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_srcport_range_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_srcport_range_dstport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_srcport_ng_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_srcport_ng_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ip_tcpudp_srcport_ng_dstport_ng_cmd);
#endif
}
#ifdef HAVE_IPV6
static void qos_access_list_ipv6(void)
{
#ifdef ZPL_FILTER_ZEBRA_EXT
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_srcany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_dstany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_proany_cmd);

    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &no_qos_access_list_ipv6_tcpudp_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &no_qos_access_list_ipv6_tcpudp_name_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_protocol_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &no_qos_access_list_ipv6_protocol_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_srcport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_srcport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_srcport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_dstport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_srcport_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_srcport_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_srcport_dstport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_srcport_range_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_srcport_range_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_srcport_range_dstport_ng_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_srcport_ng_dstport_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_srcport_ng_dstport_range_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_access_list_ipv6_tcpudp_srcport_ng_dstport_ng_cmd);
#endif
}
#endif /* HAVE_IPV6 */

#ifdef ZPL_FILTER_MAC
/* Install vty related command. */
static void qos_mac_access_list_mac(void)
{
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_any_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_any_dstany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_any_dstany_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_any_dstany_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_any_dstany_vlan_cos_type_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_any_dstmask_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_any_dstmask_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_any_dstmask_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_any_dstmask_vlan_cos_type_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_any_dsthost_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_any_dsthost_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_any_dsthost_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_any_dsthost_vlan_cos_type_cmd);

    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_mask_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_mask_dstany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_mask_dstany_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_mask_dstany_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_mask_dstany_vlan_cos_type_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_mask_dstmask_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_mask_dstmask_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_mask_dstmask_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_mask_dstmask_vlan_cos_type_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_mask_dsthost_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_mask_dsthost_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_mask_dsthost_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_mask_dsthost_vlan_cos_type_cmd);

    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_host_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_host_dstany_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_host_dstany_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_host_dstany_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_host_dstany_vlan_cos_type_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_host_dstmask_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_host_dstmask_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_host_dstmask_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_host_dstmask_vlan_cos_type_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_host_dsthost_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_host_dsthost_vlan_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_host_dsthost_vlan_cos_cmd);
    install_element(QOS_ACCESS_NODE, CMD_CONFIG_LEVEL, &qos_mac_access_list_src_mac_host_dsthost_vlan_cos_type_cmd);
}
#endif

void cmd_qos_acl_init(void)
{
    install_node(&access_list_node, nsm_access_list_write_config);
    install_node(&class_map_node, nsm_class_map_write_config);
    install_node(&policy_map_node, nsm_service_policy_write_config);
    install_node(&policy_class_map_node, NULL);

    install_default(QOS_ACCESS_NODE);
    install_default_basic(QOS_ACCESS_NODE);

    install_default(QOS_CLASS_MAP_NODE);
    install_default_basic(QOS_CLASS_MAP_NODE);

    install_default(QOS_POLICY_MAP_NODE);
    install_default_basic(QOS_POLICY_MAP_NODE);

    install_default(QOS_POLICY_CLASS_MAP_NODE);
    install_default_basic(QOS_POLICY_CLASS_MAP_NODE);

    install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &qos_access_list_name_cmd);
    install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_qos_access_list_name_cmd);

    install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &qos_class_map_match_cmd);
    install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &qos_class_map_match_any_cmd);
    install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_qos_class_map_match_cmd);

    install_element(QOS_CLASS_MAP_NODE, CMD_CONFIG_LEVEL, &qos_class_map_match_access_cmd);
    install_element(QOS_CLASS_MAP_NODE, CMD_CONFIG_LEVEL, &no_qos_class_map_match_access_cmd);

    install_element(QOS_CLASS_MAP_NODE, CMD_CONFIG_LEVEL, &qos_class_map_cir_cmd);
    install_element(QOS_CLASS_MAP_NODE, CMD_CONFIG_LEVEL, &qos_class_map_cir_pir_cmd);
    install_element(QOS_CLASS_MAP_NODE, CMD_CONFIG_LEVEL, &qos_class_map_cir_pir_cbs_cmd);
    install_element(QOS_CLASS_MAP_NODE, CMD_CONFIG_LEVEL, &no_qos_class_map_cir_cmd);

    install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &qos_service_policy_match_cmd);
    install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_qos_service_policy_match_cmd);

    install_element(QOS_POLICY_MAP_NODE, CMD_CONFIG_LEVEL, &qos_service_policy_class_match_cmd);
    install_element(QOS_POLICY_MAP_NODE, CMD_CONFIG_LEVEL, &no_qos_service_policy_class_match_cmd);

#ifdef ZPL_FILTER_MAC
    qos_mac_access_list_mac();
#endif

    qos_access_list_ipv4();
#ifdef HAVE_IPV6
    qos_access_list_ipv6();
#endif /* HAVE_IPV6 */

#ifdef ZPL_FILTER_NOSEQ
#ifdef ZPL_FILTER_MAC
    noseq_qos_mac_access_list_mac();
#endif

    noseq_qos_access_list_ipv4();
#ifdef HAVE_IPV6
    noseq_qos_access_list_ipv6();
#endif /* HAVE_IPV6 */
#endif /*ZPL_FILTER_NOSEQ*/
}

/*
routing-plaform(config-access-list)# 
routing-plaform(config-access-list)# 
routing-plaform(config-access-list)# list
  exit
  quit
  help
  list
  end
  write terminal
  write file
  write memory
  write
  show running-config
  <1-2147483646> (deny|permit) src-mac any
  <1-2147483646> (deny|permit) src-mac any dest-mac any
  <1-2147483646> (deny|permit) src-mac any dest-mac any vlan <1-4094>
  <1-2147483646> (deny|permit) src-mac any dest-mac any vlan (<1-4094>|any) cos <0-7>
  <1-2147483646> (deny|permit) src-mac any dest-mac any vlan (<1-4094>|any)  cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  <1-2147483646> (deny|permit) src-mac any dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH
  <1-2147483646> (deny|permit) src-mac any dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan <1-4094>
  <1-2147483646> (deny|permit) src-mac any dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan (<1-4094>|any) cos <0-7>
  <1-2147483646> (deny|permit) src-mac any dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  <1-2147483646> (deny|permit) src-mac any dest-mac host HHHH-HHHH-HHHH
  <1-2147483646> (deny|permit) src-mac any dest-mac host HHHH-HHHH-HHHH vlan <1-4094>
  <1-2147483646> (deny|permit) src-mac any dest-mac host HHHH-HHHH-HHHH vlan (<1-4094>|any) cos <0-7>
  <1-2147483646> (deny|permit) src-mac any dest-mac host HHHH-HHHH-HHHH vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  <1-2147483646> (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH
  <1-2147483646> (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac any
  <1-2147483646> (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac any vlan <1-4094>
  <1-2147483646> (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac any vlan (<1-4094>|any) cos <0-7>
  <1-2147483646> (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac any vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  <1-2147483646> (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH
  <1-2147483646> (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan <1-4094>
  <1-2147483646> (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan (<1-4094>|any) cos <0-7>
  <1-2147483646> (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  <1-2147483646> (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH
  <1-2147483646> (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH vlan <1-4094>
  <1-2147483646> (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH vlan (<1-4094>|any) cos <0-7>
  <1-2147483646> (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  <1-2147483646> (deny|permit) src-mac host HHHH-HHHH-HHHH
  <1-2147483646> (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac any
  <1-2147483646> (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac any vlan <1-4094>
  <1-2147483646> (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac any vlan (<1-4094>|any) cos <0-7>
  <1-2147483646> (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac any vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  <1-2147483646> (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH
  <1-2147483646> (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan <1-4094>
  <1-2147483646> (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan (<1-4094>|any) cos <0-7>
  <1-2147483646> (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  <1-2147483646> (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH
  <1-2147483646> (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH vlan <1-4094>
  <1-2147483646> (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH vlan (<1-4094>|any) cos <0-7>
  <1-2147483646> (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  <1-2147483646> (deny|permit) any (any|A.B.C.D/M) (any|)
  <1-2147483646> (deny|permit) A.B.C.D/M (any|A.B.C.D/M) (any|)
  <1-2147483646> (deny|permit) A.B.C.D/M A.B.C.D/M (any|)
  no <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp)
  no <1-2147483646>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udpip|icmp|igmp|ipip|egp|pup|idp|tp|dccp|ipv6|rsvp|gre|esp|ah|mtp|beetph|encap|pim|comp|sctp|udplite|mpls|raw|ospf|<0-5>|<7-16>|<18-255>)
  no <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udpip|icmp|igmp|ipip|egp|pup|idp|tp|dccp|ipv6|rsvp|gre|esp|ah|mtp|beetph|encap|pim|comp|sctp|udplite|mpls|raw|ospf|<0-5>|<7-16>|<18-255>)
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp)
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port eq <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port range <1-65536> <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-port eq <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-port range <1-65536> <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-port (ne|gt|lt|ge|le) <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port eq <1-65536> dest-port eq <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port eq <1-65536> dest-port range <1-65536> <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port eq <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port range <1-65536> <1-65536> dest-port eq <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port range <1-65536> <1-65536> dest-port range <1-65536> <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port range <1-65536> <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port eq <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port range <1-65536> <1-65536>
  <1-2147483646> (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>
  <1-2147483646> (deny|permit) ipv6 any (any|X:X::X:X/M) (any|)
  <1-2147483646> (deny|permit) ipv6 X:X::X:X/M (any|X:X::X:X/M) (any|)
  <1-2147483646> (deny|permit) ipv6 X:X::X:X/M X:X::X:X/M (any|)
  no <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp)
  no <1-2147483646> ipv6
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udpip|icmp|igmp|ipip|egp|pup|idp|tp|dccp|ipv6|rsvp|gre|esp|ah|mtp|beetph|encap|pim|comp|sctp|udplite|mpls|raw|ospf|<0-5>|<7-16>|<18-255>)
  no <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udpip|icmp|igmp|ipip|egp|pup|idp|tp|dccp|ipv6|rsvp|gre|esp|ah|mtp|beetph|encap|pim|comp|sctp|udplite|mpls|raw|ospf|<0-5>|<7-16>|<18-255>)
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp)
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port eq <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port range <1-65536> <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port any dest-port eq <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port any dest-port range <1-65536> <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port any dest-port (ne|gt|lt|ge|le) <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port eq <1-65536> dest-port eq <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port eq <1-65536> dest-port range <1-65536> <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port eq <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port range <1-65536> <1-65536> dest-port eq <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port range <1-65536> <1-65536> dest-port range <1-65536> <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port range <1-65536> <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port eq <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port range <1-65536> <1-65536>
  <1-2147483646> (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>
  (deny|permit) src-mac any
  (deny|permit) src-mac any dest-mac any
  (deny|permit) src-mac any dest-mac any vlan <1-4094>
  (deny|permit) src-mac any dest-mac any vlan (<1-4094>|any) cos <0-7>
  (deny|permit) src-mac any dest-mac any vlan (<1-4094>|any)  cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  (deny|permit) src-mac any dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH
  (deny|permit) src-mac any dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan <1-4094>
  (deny|permit) src-mac any dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan (<1-4094>|any) cos <0-7>
  (deny|permit) src-mac any dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  (deny|permit) src-mac any dest-mac host HHHH-HHHH-HHHH
  (deny|permit) src-mac any dest-mac host HHHH-HHHH-HHHH vlan <1-4094>
  (deny|permit) src-mac any dest-mac host HHHH-HHHH-HHHH vlan (<1-4094>|any) cos <0-7>
  (deny|permit) src-mac any dest-mac host HHHH-HHHH-HHHH vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH
  (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac any
  (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac any vlan <1-4094>
  (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac any vlan (<1-4094>|any) cos <0-7>
  (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac any vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH
  (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan <1-4094>
  (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan (<1-4094>|any) cos <0-7>
  (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH
  (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH vlan <1-4094>
  (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH vlan (<1-4094>|any) cos <0-7>
  (deny|permit) src-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  (deny|permit) src-mac host HHHH-HHHH-HHHH
  (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac any
  (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac any vlan <1-4094>
  (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac any vlan (<1-4094>|any) cos <0-7>
  (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac any vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH
  (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan <1-4094>
  (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan (<1-4094>|any) cos <0-7>
  (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac HHHH-HHHH-HHHH mask HHHH-HHHH-HHHH vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH
  (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH vlan <1-4094>
  (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH vlan (<1-4094>|any) cos <0-7>
  (deny|permit) src-mac host HHHH-HHHH-HHHH dest-mac host HHHH-HHHH-HHHH vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)
  (deny|permit) any (any|A.B.C.D/M) (any|)
  (deny|permit) A.B.C.D/M (any|A.B.C.D/M) (any|)
  (deny|permit) A.B.C.D/M A.B.C.D/M (any|)
  no (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp)
  no <1-2147483646>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udpip|icmp|igmp|ipip|egp|pup|idp|tp|dccp|ipv6|rsvp|gre|esp|ah|mtp|beetph|encap|pim|comp|sctp|udplite|mpls|raw|ospf|<0-5>|<7-16>|<18-255>)
  no (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udpip|icmp|igmp|ipip|egp|pup|idp|tp|dccp|ipv6|rsvp|gre|esp|ah|mtp|beetph|encap|pim|comp|sctp|udplite|mpls|raw|ospf|<0-5>|<7-16>|<18-255>)
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp)
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port eq <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port range <1-65536> <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-port eq <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-port range <1-65536> <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-port (ne|gt|lt|ge|le) <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port eq <1-65536> dest-port eq <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port eq <1-65536> dest-port range <1-65536> <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port eq <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port range <1-65536> <1-65536> dest-port eq <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port range <1-65536> <1-65536> dest-port range <1-65536> <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port range <1-65536> <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port eq <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port range <1-65536> <1-65536>
  (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>
  (deny|permit) ipv6 any (any|X:X::X:X/M) (any|)
  (deny|permit) ipv6 X:X::X:X/M (any|X:X::X:X/M) (any|)
  (deny|permit) ipv6 X:X::X:X/M X:X::X:X/M (any|)
  no (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp)
  no ipv6
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udpip|icmp|igmp|ipip|egp|pup|idp|tp|dccp|ipv6|rsvp|gre|esp|ah|mtp|beetph|encap|pim|comp|sctp|udplite|mpls|raw|ospf|<0-5>|<7-16>|<18-255>)
  no (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udpip|icmp|igmp|ipip|egp|pup|idp|tp|dccp|ipv6|rsvp|gre|esp|ah|mtp|beetph|encap|pim|comp|sctp|udplite|mpls|raw|ospf|<0-5>|<7-16>|<18-255>)
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp)
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port eq <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port range <1-65536> <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port any dest-port eq <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port any dest-port range <1-65536> <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port any dest-port (ne|gt|lt|ge|le) <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port eq <1-65536> dest-port eq <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port eq <1-65536> dest-port range <1-65536> <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port eq <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port range <1-65536> <1-65536> dest-port eq <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port range <1-65536> <1-65536> dest-port range <1-65536> <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port range <1-65536> <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port eq <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port range <1-65536> <1-65536>
  (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>
routing-plaform(config-access-list)# 
*/