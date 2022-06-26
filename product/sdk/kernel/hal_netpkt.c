#include "bsp_types.h"
#include "hal_netlink.h"
#include "bsp_include.h"
#include "hal_netpkt.h"

/* Ingress and egress opcodes */
#define BRCM_OPCODE_SHIFT 5
#define BRCM_OPCODE_MASK 0x7
#define BRCM_EG_RC_RSVD (3 << 6)
#define BRCM_EG_PID_MASK 0x1f

static struct hal_netlink *hal_netpkt = NULL;

static void netpkt_sock_skb_dump(zpl_uint8 *nethdr)
{
  zpl_uint8 *brcm_tag = NULL;  
  brcm_tag = nethdr + 12;  
  //00 00 22 03
  char *reason_str[] = {"mirroring", "SA Learning", "switching", "proto term", "proto snooping","flooding", "res"};
  if((brcm_tag[0] >> BRCM_OPCODE_SHIFT) & BRCM_OPCODE_MASK)
  {
    printk("DEV RECV: type=%c port=%d", (brcm_tag[3] & 0x20)?'T':'R', brcm_tag[3] & BRCM_EG_PID_MASK);
  }
  else//if((brcm_tag[0] >> BRCM_OPCODE_SHIFT) & BRCM_OPCODE_MASK)
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
  if (hal_netpkt)
    if (ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_RECV) && ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_DETAIL)
      && nethdr[16] == 0x81 && nethdr[17] == 0x00)
      printk("DEV RECV: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                 nethdr[0], nethdr[1], nethdr[2], nethdr[3], nethdr[4], nethdr[5],
                 nethdr[6], nethdr[7], nethdr[8], nethdr[9], nethdr[10], nethdr[11],
                 nethdr[12], nethdr[13], nethdr[14], nethdr[15], nethdr[16], nethdr[17], nethdr[18], nethdr[19]);
}

static struct sk_buff *netpkt_sock_skb_copy(const struct sk_buff *skb, int gport)
{
  char *nldata = NULL;
  zpl_uint8 *nethdr = NULL;
  struct sk_buff *nlskb = NULL;
  zpl_netpkt_hdr_t *netkthdr = NULL;
  struct nlmsghdr *nlh = NULL;
  //(db->outblk)(db->io_data, skb->data, skb->len);
  nlskb = XNLMSG_NEW(MTYPE_SDK_DATA, skb->len + sizeof(zpl_netpkt_hdr_t));
  if (nlskb && hal_netpkt)
  {
    // skb_pull_rcsum(skb, 4);
    nethdr = skb->data - 14;
    netpkt_sock_skb_dump(nethdr);
    /*if (ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_RECV) && ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_DETAIL)
      && nethdr[16] == 0x81 && nethdr[17] == 0x00)
      printk("DEV RECV: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                 nethdr[0], nethdr[1], nethdr[2], nethdr[3], nethdr[4], nethdr[5],
                 nethdr[6], nethdr[7], nethdr[8], nethdr[9], nethdr[10], nethdr[11],
                 nethdr[12], nethdr[13], nethdr[14], nethdr[15], nethdr[16], nethdr[17]);
                 */

    nlh = nlmsg_put(nlskb, 0, 0, 0, skb->len + sizeof(zpl_netpkt_hdr_t), 0);
    nldata = nlmsg_data(nlh);
    netkthdr = (zpl_netpkt_hdr_t *)nldata;
    nldata += sizeof(zpl_netpkt_hdr_t);
    if (skb->len)
      memcpy(nldata, skb->data, skb->len);
    return nlskb;
  }
  return nlskb;
}

static int netpkt_sock_skb_recv_callback(const struct sk_buff *skb, const struct net_device *dev)
{
  if (hal_netpkt)
  {
    int source_port = 0;
    u8 *brcm_tag = NULL;
    struct sk_buff *nlskb = NULL;
    if (unlikely(!pskb_may_pull(skb, 4)))
      return NET_RX_SUCCESS;

    brcm_tag = skb->data - 2;

    /* The opcode should never be different than 0b000 */
    if (unlikely((brcm_tag[0] >> BRCM_OPCODE_SHIFT) & BRCM_OPCODE_MASK))
      return NET_RX_SUCCESS;

    /* We should never see a reserved reason code without knowing how to
     * handle it
     */
    if (unlikely(brcm_tag[2] & BRCM_EG_RC_RSVD))
      return NET_RX_SUCCESS;

    /* Locate which port this is coming from */
    source_port = brcm_tag[3] & BRCM_EG_PID_MASK;

    //if (source_port == 3 || source_port == 8) // wan
    //  return NET_RX_SUCCESS;

    nlskb = netpkt_sock_skb_copy(skb, source_port);
    if (nlskb)
    {
      /*if (hal_netlink_unicast(hal_netpkt, hal_netpkt->dstpid, (struct sk_buff *)nlskb) == 0)
      {
        nlmsg_free(nlskb);
        return NET_RX_DROP;
      }*/
      nlmsg_free(nlskb);
    }
  }
  return NET_RX_SUCCESS;
}

static void netpkt_sock_send_tocpu(struct net_device *dev, struct sk_buff *skb)
{
  if (skb)
  {
    skb->dev = dev;
    netif_rx(skb);
  }
}

static void netpkt_sock_send_todev(struct net_device *dev, struct sk_buff *skb)
{
  if (skb && dev)
  {
    zlog_err(MODULE_SDK, "nk_pkt_sock_send_todev %s", dev->name);
    skb->dev = dev;
    dev->netdev_ops->ndo_start_xmit(skb, dev);
  }
}

static void netpkt_netlink_input(struct sk_buff *__skb)
{
  zpl_netpkt_hdr_t *netkthdr = NULL;
  struct nlmsghdr *nlh = nlmsg_hdr(__skb);
  const char *nlmsg = NULL;
  struct net_device *dev = NULL;
  zpl_uint8 *nethdr = NULL;
  nethdr = (zpl_uint8 *)nlmsg_data(nlh);
  netkthdr = (zpl_netpkt_hdr_t *)nethdr;
  nethdr += sizeof(zpl_netpkt_hdr_t);
  if (hal_netpkt && hal_netpkt)
  {
    switch (ntohl(netkthdr->reason))
    {
    case NETPKT_REASON_NONE:
      break;
    case NETPKT_REASON_COPYTOCPU:
      dev = dev_get_by_index(&init_net, ntohl(netkthdr->ifindex));
      if (dev)
      {
        skb_pull(__skb, sizeof(zpl_netpkt_hdr_t)); //从缓冲区的开始删除数据
        if (ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_RECV) && ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_DETAIL))
          zlog_debug(MODULE_SDK, "TOCPU: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                     nethdr[0], nethdr[1], nethdr[2], nethdr[3], nethdr[4], nethdr[5],
                     nethdr[6], nethdr[7], nethdr[8], nethdr[9], nethdr[10], nethdr[11],
                     nethdr[12], nethdr[13], nethdr[14], nethdr[15], nethdr[16], nethdr[17]);
        netpkt_sock_send_tocpu(dev, __skb);
      }
      break;
    case NETPKT_REASON_TONETDEV:
      dev = dev_get_by_index(&init_net, ntohl(netkthdr->ifindex));
      if (dev)
      {
        skb_pull(__skb, sizeof(zpl_netpkt_hdr_t));
        if (ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_SEND) && ZPL_TST_BIT(hal_netpkt->debug, KLOG_DEBUG_DETAIL))
          zlog_debug(MODULE_SDK, "DEV SEND: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                     nethdr[0], nethdr[1], nethdr[2], nethdr[3], nethdr[4], nethdr[5],
                     nethdr[6], nethdr[7], nethdr[8], nethdr[9], nethdr[10], nethdr[11],
                     nethdr[12], nethdr[13], nethdr[14], nethdr[15], nethdr[16], nethdr[17]);
        netpkt_sock_send_todev(dev, __skb);
      }
      break;
    }
  }
}

int netpkt_netlink_debug_set(int set, int val)
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

int netpkt_netlink_bind(int ifindex, int bind)
{
  struct net_device *netdev = NULL;
  netdev = dev_get_by_index(&init_net, ifindex);
  if (netdev)
  {
    zlog_debug(MODULE_SDK, " nk_pkt_sock_cmd NK_PKT_SETUP %d", ifindex);
    if (bind)
      netdev->ndev_dsa_pktrx = netpkt_sock_skb_recv_callback;
    else
      netdev->ndev_dsa_pktrx = NULL;
    return OK;
  }
  return ERROR;
}

int netpkt_netlink_dstpid(int pid)
{
  if(hal_netpkt)
    hal_netlink_group_dstpid(hal_netpkt, 0,  pid);
  return OK;  
}

static struct netlink_kernel_cfg _netpkt_sock_nkc = {
    .groups = 0,
    .flags = 0,
    .input = netpkt_netlink_input,
    .cb_mutex = NULL,
    .bind = NULL,
    .unbind = NULL,
    .compare = NULL,
};

int netpkt_netlink_init(void)
{
  struct net_device *netdev = NULL;
  hal_netpkt = hal_netlink_create("netpkt", HAL_DATA_NETLINK_PROTO, 0, &_netpkt_sock_nkc);
  if (!hal_netpkt)
  {
    zlog_err(MODULE_SDK, "[netlink] create netlink socket error!");
    return ERROR;
  }
  hal_netpkt->debug = 0x00ffffff;
  netdev = dev_get_by_name(&init_net, "eth0");
  if (netdev)
  {
    zlog_debug(MODULE_SDK, " nk_pkt_sock_cmd NK_PKT_SETUP %s", "eth0");
    netdev->ndev_dsa_pktrx = netpkt_sock_skb_recv_callback;
    return OK;
  }  
  return 0;
}

void netpkt_netlink_exit(void)
{
  if (hal_netpkt)
  {
    hal_netlink_destroy(hal_netpkt);
    hal_netpkt = NULL;
  }
}