
#ifndef __BSP_NETPKT_H__
#define __BSP_NETPKT_H__


typedef int (*netpkt_rx_cb)(ifindex_t ifindex, unsigned char *buf, int len);

struct netpkt_field
{
    ifindex_t   ifindex;
    struct ipstack_ethhdr   ethhdr;
    vlan_t   vlanid;
    union 
    {
        struct ipstack_iphdr   iphdr;
        struct ipstack_arphdr   arphdr;
    }ippkt;
    union 
    {
        struct ipstack_icmphdr   icmphdr;
        struct ipstack_igmphdr   igmp;
        struct ipstack_igmpv3_query   igmpv3_query;
        struct ipstack_igmpv3_report   igmpv3_report;
        struct ipstack_igmpv3_grec   igmpv3_queigmpv3_grecry;
        struct ipstack_tcphdr   tcphdr;
        struct ipstack_udphdr   udphdr;
    }ipplay;
};

extern int netpkt_filter_init(void);
extern int netpkt_filter_exit(void);
extern int netpkt_filter_register(struct netpkt_field *netpkt_field, netpkt_rx_cb rx_cb);
extern int netpkt_filter_unregister(struct netpkt_field *netpkt_field);


#endif /* __BSP_NETPKT_H__ */
