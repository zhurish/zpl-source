/*
 * hal_netpkt.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_NETPKT_H__
#define __HAL_NETPKT_H__

#ifdef __cplusplus
extern "C" {
#endif



typedef int (*hal_netpkt_rx_cb)(ifindex_t ifindex, unsigned char *buf, int len);

struct hal_netpkt_field
{
    ifindex_t   	ifindex;
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

int hal_netpkt_filter_init(void);
int hal_netpkt_filter_exit(void);
int hal_netpkt_filter_register(struct hal_netpkt_field *hal_netpkt_field, hal_netpkt_rx_cb rx_cb, zpl_socket_t sock);
int hal_netpkt_filter_unregister(struct hal_netpkt_field *hal_netpkt_field);
int hal_netpkt_filter_distribute(char *data, int len);

int hal_netpkt_send(ifindex_t ifindex, zpl_vlan_t vlanid, 
	zpl_uint8 pri, zpl_uchar *data, zpl_uint32 len);
    
#ifdef __cplusplus
}
#endif

#endif /* __HAL_NETPKT_H__ */
