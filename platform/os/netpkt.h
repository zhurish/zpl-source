
#ifndef __OS_NETPKT_H__
#define __OS_NETPKT_H__

struct ipstack_ethhdr * netpkt_ethhdr_get(zpl_uchar *skb, zpl_uint32 len);
vlan_t netpkt_vlan_get(zpl_uchar *skb, zpl_uint32 len);
struct ipstack_iphdr * netpkt_iphdr_get(zpl_uchar *skb, zpl_uint32 len);
struct ipstack_arphdr * netpkt_arphdr_get(zpl_uchar *skb, zpl_uint32 len);


#endif /* __OS_NETPKT_H__ */
