#include "kbsp_types.h"
#include "khal_netlink.h"
#ifdef ZPL_SDK_KERNEL
#include "bsp_include.h"
#endif
#include "khal_netpkt.h"

/* Ingress and egress opcodes */
#define BRCM_OPCODE_SHIFT 5
#define BRCM_OPCODE_MASK 0x7
#define BRCM_EG_RC_RSVD (3 << 6)
#define BRCM_EG_PID_MASK 0x1f

static struct khal_netlink *hal_netpkt = NULL;
static struct net_device *hal_netdev = NULL;

static int netpkt_loghex(zpl_char *format, zpl_uint32 size, const zpl_uchar *data, zpl_uint32 len)
{
	zpl_uint32 i = 0, offset = 0;
	for(i = 0; i < len; i++)
	{
		if((i+1)%16 == 0)
			offset += snprintf(format + offset, size - offset, "\r\n");
		if((size >= offset) < 5 )
			offset += snprintf(format + offset, size - offset, "0x%02x ", (zpl_uchar)data[i]);
		else
			return ++i;
	}
	return ++i;
}

static int khal_netpkt_skb_hwport(struct khal_netlink *netpkt, zpl_uint8 *nethdr)
{
  zpl_uint8 *brcm_tag = NULL;  
  brcm_tag = nethdr + NETPKT_ETHMAC_HEADER;  
  //00 00 22 03
  //char *reason_str[] = {"mirroring", "SA Learning", "switching", "proto term", "proto snooping","flooding", "res"};
  if(ZPL_TST_BIT(netpkt->debug, KLOG_DEBUG_RECV) && ZPL_TST_BIT(netpkt->debug, KLOG_DEBUG_DETAIL) && 
    (brcm_tag[0] >> BRCM_OPCODE_SHIFT) & BRCM_OPCODE_MASK)
  {
    printk("DEV RECV: type=%c port=%d", (brcm_tag[3] & 0x20)?'T':'R', brcm_tag[3] & BRCM_EG_PID_MASK);
  }
  return (brcm_tag[3] & BRCM_EG_PID_MASK);
}

static void khal_netpkt_sock_skb_dump(struct khal_netlink *netpkt, zpl_uint8 *nethdr, char *hdr)
{
  zpl_uint8 *brcm_tag = NULL;  
  brcm_tag = nethdr + NETPKT_ETHMAC_HEADER;  
  //00 00 22 03
  //char *reason_str[] = {"mirroring", "SA Learning", "switching", "proto term", "proto snooping","flooding", "res"};
  if(ZPL_TST_BIT(netpkt->debug, KLOG_DEBUG_RECV) && ZPL_TST_BIT(netpkt->debug, KLOG_DEBUG_DETAIL) && 
    (brcm_tag[0] >> BRCM_OPCODE_SHIFT) & BRCM_OPCODE_MASK)
  {
    printk("%s: type=%c port=%d", hdr, (brcm_tag[3] & 0x20)?'T':'R', brcm_tag[3] & BRCM_EG_PID_MASK);
  }
  //else//if((brcm_tag[0] >> BRCM_OPCODE_SHIFT) & BRCM_OPCODE_MASK)
  {
    /*
    if(brcm_tag[2] & 0x01)
      printk("DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[0], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x02)
      printk("DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[1], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x04)
      printk("DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[2], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x08)
      printk("DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[3], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x10)
      printk("DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[4], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x20)
      printk("DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[5], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x40)
      printk("DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[6], brcm_tag[3] & BRCM_EG_PID_MASK);
    if(brcm_tag[2] & 0x80)
      printk("DEV RECV: reason=(%x)%s port=%d", brcm_tag[2], reason_str[7], brcm_tag[3] & BRCM_EG_PID_MASK);
      */
  }  
  if (netpkt)
    if (ZPL_TST_BIT(netpkt->debug, KLOG_DEBUG_RECV) && ZPL_TST_BIT(netpkt->debug, KLOG_DEBUG_DETAIL)
      && nethdr[16] == 0x81 && nethdr[17] == 0x00)
      printk("%s: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", hdr,
                 nethdr[0], nethdr[1], nethdr[2], nethdr[3], nethdr[4], nethdr[5],
                 nethdr[6], nethdr[7], nethdr[8], nethdr[9], nethdr[10], nethdr[11],
                 nethdr[12], nethdr[13], nethdr[14], nethdr[15], nethdr[16], nethdr[17], nethdr[18], nethdr[19]);
}


static struct sk_buff *khal_netpkt_sock_skb_copy(struct khal_netlink *netpkt, struct net_device *dev, const struct sk_buff *skb, int cmd)
{
  int ret = 0;
  char *nldata = NULL;
  zpl_uint8 *nethdr = skb->data;
  struct sk_buff *nlskb = NULL;
  struct nlmsghdr *nlh = NULL;//16-20
  khal_nettpkt_cmd_t  *hdr = (khal_nettpkt_cmd_t*)nldata;
  /* 芯片上来的数据 */
  if(cmd == NETPKT_FROMSWITCH)
  {
    ret = khal_netpkt_skb_hwport(netpkt, skb->data - NETPKT_ETHERNET_HEADER);  
    if(ret == 3)
      return NULL;
  }
          
  nlskb = XNLMSG_NEW(MTYPE_SDK_DATA, skb->len + sizeof(khal_nettpkt_cmd_t));
  if (nlskb && hal_netpkt)
  {
    if(cmd == NETPKT_FROMSWITCH)
    {
      nethdr = skb->data - NETPKT_ETHERNET_HEADER;
      khal_netpkt_sock_skb_dump(netpkt, nethdr, (cmd==NETPKT_FROMSWITCH)?"DEV RECV":"FROM DEV");
    }
    nlh = nlmsg_put(nlskb, 0, 0, HAL_DATA_REQUEST_CMD, skb->len + sizeof(khal_nettpkt_cmd_t), 0);
    nldata = nlmsg_data(nlh);
    hdr = (khal_nettpkt_cmd_t*)nldata;
    hdr->cmd = cmd;
    if(dev)
      hdr->dstval = dev->ifindex;
    if (skb->len)
      memcpy(nldata + sizeof(khal_nettpkt_cmd_t), skb->data-NETPKT_ETHERNET_HEADER, skb->len+NETPKT_ETHERNET_HEADER);
    return nlskb;
  }
  return nlskb;
}

static int khal_netpkt_sock_skb_recv_callback(const struct sk_buff *skb, const struct net_device *dev)
{
  char tmpbuf[2048];
  if (hal_netpkt)
  {
    struct sk_buff *nlskb = NULL;
    if (unlikely(!pskb_may_pull(skb, 4)))
    {
      zlog_err(MODULE_SDK, "switch pkt to netlink pskb_may_pull pid=%d", hal_netpkt->dstpid);
      return NET_RX_SUCCESS;
    }
    if(ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_RECV) && ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_DETAIL))
    {
      netpkt_loghex(tmpbuf, sizeof(tmpbuf), skb->data-NETPKT_ETHERNET_HEADER, skb->len+NETPKT_ETHERNET_HEADER);
      zlog_err(MODULE_SDK, " Recv from switch skb len=%d data={%s}", skb->len, tmpbuf);
    }
    nlskb = khal_netpkt_sock_skb_copy(hal_netpkt, dev, skb, NETPKT_FROMSWITCH);
    if (nlskb && hal_netpkt && hal_netpkt->dstpid)
    {
      int ret = khal_netlink_unicast(hal_netpkt, hal_netpkt->dstpid, nlskb);     
      if (ret > 0)
      {
        //zlog_err(MODULE_SDK, "switch pkt to netlink pid=%d len=%d nllen=%d, ret=%d", hal_netpkt->dstpid, skb->len, nlskb->len, ret);
        //nlmsg_free(nlskb);
        //nlskb = NULL;
        return NET_RX_DROP;
      }
      nlmsg_free(nlskb);
      nlskb = NULL;
      return NET_RX_DROP;
    }
    if(nlskb != NULL)
    {
      nlmsg_free(nlskb);
      nlskb = NULL;
    }
  }
  return NET_RX_SUCCESS;
}

static void khal_netpkt_sock_send_tocpu(struct net_device *dev, struct sk_buff *skb)
{
  if (skb)
  {
    skb->dev = dev;
    dev->stats.rx_bytes += skb->len;
    /* Pass to upper layer */
    skb->protocol = eth_type_trans(skb, dev);
    dev->stats.rx_packets++;
    netif_rx(skb);
  }
}

static int khal_netpkt_sock_send_switch(struct sk_buff *skb)
{
  if (skb && hal_netdev)
  {
    if(hal_netpkt && ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_SEND) && ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_DETAIL))
    {
      char tmpbuf[2048];
      netpkt_loghex(tmpbuf, sizeof(tmpbuf), skb->data, skb->len);
      zlog_err(MODULE_SDK, " Send to switch skb len=%d data={%s}", skb->len, tmpbuf);
    }
    skb->dev = hal_netdev;
    hal_netdev->netdev_ops->ndo_start_xmit(skb, hal_netdev);
  }
  return 0;
}

static int khal_netpkt_sock_touser(struct khal_netlink *netpkt, struct net_device *dev, struct sk_buff *skb)
{
  if (netpkt)
  {
    struct sk_buff *nlskb = NULL;
    nlskb = khal_netpkt_sock_skb_copy(netpkt, dev, skb, NETPKT_FROMDEV);
    if (nlskb && netpkt && netpkt->dstpid)
    {
      if (khal_netlink_unicast(netpkt, netpkt->dstpid, (struct sk_buff *)nlskb) > 0)
      {
        nlmsg_free(nlskb);
        return NET_RX_DROP;
      }
      nlmsg_free(nlskb);
    }
  }
  return 0;
}

void khal_netpkt_fromdev(struct net_device *dev, struct sk_buff *skb)
{
  if(hal_netpkt)
    khal_netpkt_sock_touser(hal_netpkt, dev, skb);
}

static void khal_netpkt_input(struct sk_buff *__skb)
{
  khal_nettpkt_cmd_t *netkthdr = NULL;
  struct nlmsghdr *nlh = nlmsg_hdr(__skb);
  struct net_device *dev = NULL;
  zpl_uint8 *nethdr = NULL;
  nethdr = (zpl_uint8 *)nlmsg_data(nlh);
  netkthdr = (khal_nettpkt_cmd_t *)nethdr;
  nethdr += sizeof(khal_nettpkt_cmd_t);
  if (hal_netpkt && hal_netpkt)
  {  
    switch (nlh->nlmsg_type)
    {
    case NETPKT_TOCPU:
      dev = dev_get_by_index(&init_net, ntohl(netkthdr->dstval));
      if (dev)
      {
        skb_pull(__skb, sizeof(khal_nettpkt_cmd_t)); //从缓冲区的开始删除数据
        if (ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_RECV) && ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_DETAIL))
          zlog_debug(MODULE_SDK, "TOCPU: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                     nethdr[0], nethdr[1], nethdr[2], nethdr[3], nethdr[4], nethdr[5],
                     nethdr[6], nethdr[7], nethdr[8], nethdr[9], nethdr[10], nethdr[11],
                     nethdr[12], nethdr[13], nethdr[14], nethdr[15], nethdr[16], nethdr[17]);
        khal_netpkt_sock_send_tocpu(dev, __skb);
      }
      break;
    case NETPKT_TOSWITCH:
      skb_pull(__skb, sizeof(khal_nettpkt_cmd_t));
      khal_netpkt_sock_send_switch(__skb);
      break;
    case NETPKT_FROMSWITCH:
      break;
    case NETPKT_FROMDEV:
      break;
    }
  }
}

int khal_netpkt_debug_set(int set, int val)
{
  if (hal_netpkt)
  {
    if (set)
      ZPL_SET_BIT(hal_netpkt->debug, val);
    else
      ZPL_CLR_BIT(hal_netpkt->debug, val);
  }
  return OK;
}

int khal_netpkt_bind(int ifindex, int bind)
{
  //hal_netdev = dev_get_by_index(&init_net, ifindex);
  hal_netdev = dev_get_by_name(&init_net, "eth0");
  if (hal_netdev)
  {
    zlog_debug(MODULE_SDK, " khal_netpkt_bind  ifndex=%d, bind=%d", ifindex, bind);
#ifdef ZPL_BUILD_ARCH_X86_64
#else    
    //if (bind)
    //  hal_netdev->ndev_dsa_pktrx = khal_netpkt_sock_skb_recv_callback;
    ///else
    //  hal_netdev->ndev_dsa_pktrx = NULL;
#endif      
    return OK;
  }
  return ERROR;
}

int khal_netpkt_dstpid(int pid)
{
  if(hal_netpkt)
    khal_netlink_group_dstpid(hal_netpkt, 0,  pid);
  return OK;  
}

static struct netlink_kernel_cfg _netpkt_sock_nkc = {
    .groups = 0,
    .flags = 0,
    .input = khal_netpkt_input,
    .cb_mutex = NULL,
    .bind = NULL,
    .unbind = NULL,
    .compare = NULL,
};


int khal_netpkt_init(void)
{
  hal_netpkt = khal_netlink_create("netpkt", HAL_DATA_NETLINK_PROTO, 0, &_netpkt_sock_nkc);
  if (!hal_netpkt)
  {
    zlog_err(MODULE_SDK, "[netlink] create netlink socket error!");
    return ERROR;
  }
  hal_netpkt->debug = 0;
  hal_netdev = dev_get_by_name(&init_net, "eth0");
  if (hal_netdev)
  {
    zlog_debug(MODULE_SDK, " nk_pkt_sock_cmd bind %s", "eth0");
#ifdef ZPL_BUILD_ARCH_X86_64
#else       
    hal_netdev->ndev_dsa_pktrx = khal_netpkt_sock_skb_recv_callback;
#endif    
    return OK;
  }  
  return 0;
}

void khal_netpkt_exit(void)
{
  if (hal_netpkt)
  {
    khal_netlink_destroy(hal_netpkt);
    hal_netpkt = NULL;
    hal_netdev = NULL;
  }
}