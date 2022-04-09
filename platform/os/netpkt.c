#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_skbuffer.h"
#include "netpkt.h"

struct ipstack_ethhdr * netpkt_ethhdr_get(zpl_uchar *skb, zpl_uint32 len)
{
    struct ipstack_ethhdr *ethhdr = (struct ipstack_ethhdr *)skb;
    return ethhdr;
}

vlan_t netpkt_vlan_get(zpl_uchar *skb, zpl_uint32 len)
{
    struct ipstack_ethhdr *ethhdr = (struct ipstack_ethhdr *)skb;
    if(ntohs(ethhdr->h_proto) == 0x8100)
    {
        vlan_t *vlan = (vlan_t *)(skb + sizeof(struct ipstack_ethhdr));
        return ntohs(*vlan);
    }
    return 0;
}


struct ipstack_iphdr * netpkt_iphdr_get(zpl_uchar *skb, zpl_uint32 len)
{
    zpl_uint32 offset = 0;
    struct ipstack_iphdr *iphdr = NULL;
    struct ipstack_ethhdr *ethhdr = (struct ipstack_ethhdr *)skb;
    if(ntohs(ethhdr->h_proto) == 0x8100)
    {
        offset = 4;
    }
    iphdr = (struct ipstack_iphdr *)(skb + offset + sizeof(struct ipstack_ethhdr));
    return iphdr;
}

struct ipstack_arphdr * netpkt_arphdr_get(zpl_uchar *skb, zpl_uint32 len)
{
    zpl_uint32 offset = 0;
    struct ipstack_arphdr *arphdr = NULL;
    struct ipstack_ethhdr *ethhdr = (struct ipstack_ethhdr *)skb;
    if(ntohs(ethhdr->h_proto) == 0x8100)
    {
        offset = 4;
    }
    arphdr = (struct ipstack_arphdr *)(skb + offset + sizeof(struct ipstack_ethhdr));
    return arphdr;
}

