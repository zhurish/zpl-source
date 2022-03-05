/* Copyright 2009 IP Infusion, Inc. All Rights Reserved.  */

#ifndef __PKT_NETLINK_H__
#define __PKT_NETLINK_H__



struct nk_pkt_start
{
    int ifindex;
    int value;
    int value1;
    int value2;
};

#define NK_PKT_START    1
#define NK_PKT_STOP     2
#define NK_PKT_SETUP    3
#define NK_PKT_DATA     4
#define NK_PKT_CPU      5
#define NK_PKT_TEST     6


#endif /* __PKT_NETLINK_H__ */

