#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"
#include "nsm_qos_acl.h"

#define QOS_CLASS_MAP_STR "Class Map List\n"
#define QOS_POLICY_MAP_STR "POlicy Map List\n"

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
      "(deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD IP_IPPROTO_CMD ")",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
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
      "no (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (" IP_TCPUDP_CMD IP_IPPROTO_CMD ")",
      NO_STR
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
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
      "(deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD IP_IPPROTO_CMD ")",
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
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
      "no (deny|permit) ipv6 (X:X::X:X/M|any) (X:X::X:X/M|any) (" IP_TCPUDP_CMD IP_IPPROTO_CMD ")",
      NO_STR
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      IPV6_STR
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n" IP_TCPUDP_HELP
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
DEFUN(noseq_noseq_qos_mac_access_list_src_mac_any,
      noseq_noseq_qos_mac_access_list_src_mac_any_cmd,
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

void cmd_qos_acl_init(void)
{
#ifdef ZPL_FILTER_MAC
    noseq_qos_mac_access_list_mac();
#endif

    noseq_qos_access_list_ipv4();
#ifdef HAVE_IPV6
    noseq_qos_access_list_ipv6();
#endif /* HAVE_IPV6 */
}