// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Pseudo-driver for the loopback interface.
 *
 * Version:	@(#)loopback.c	1.0.4b	08/16/93
 *
 * Authors:	Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Donald Becker, <becker@scyld.com>
 *
 *		Alan Cox	:	Fixed oddments for NET3.014
 *		Alan Cox	:	Rejig for NET3.029 snap #3
 *		Alan Cox	:	Fixed NET3.029 bugs and sped up
 *		Larry McVoy	:	Tiny tweak to double performance
 *		Alan Cox	:	Backed out LMV's tweak - the linux mm
 *					can't take it...
 *              Michael Griffith:       Don't bother computing the checksums
 *                                      on packets received on the loopback
 *                                      interface.
 *		Alexey Kuznetsov:	Potential hang under some extreme
 *					cases removed.
 */
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/in.h>

#include <linux/uaccess.h>
#include <linux/io.h>

#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <net/sock.h>
#include <net/checksum.h>
#include <linux/if_ether.h>	/* For the statistics structure. */
#include <linux/if_arp.h>	/* For ARPHRD_ETHER */
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/percpu.h>
#include <linux/net_tstamp.h>
#include <net/net_namespace.h>
#include <linux/u64_stats_sync.h>

#include "kbsp_types.h"
#include "keth_drv.h"

//extern const struct header_ops eth_header_ops;

static const struct header_ops keth_header_ops ____cacheline_aligned = {
	.create		= eth_header,
	.parse		= eth_header_parse,
	.cache		= eth_header_cache,
	.cache_update	= eth_header_cache_update,
	.parse_protocol	= eth_header_parse_protocol,
};

static inline void kdev_lstats_add(struct net_device *dev, unsigned int len)
{
	struct pcpu_lstats *lstats = this_cpu_ptr(dev->lstats);

	u64_stats_update_begin(&lstats->syncp);
	u64_stats_add(&lstats->bytes, len);
	u64_stats_inc(&lstats->packets);
	u64_stats_update_end(&lstats->syncp);
}

/* The higher levels take care of making this non-reentrant (it's
 * called with bh's disabled).
 */
static netdev_tx_t kloopback_xmit(struct sk_buff *skb,
				 struct net_device *dev)
{
	int len;

	skb_tx_timestamp(skb);

	/* do not fool net_timestamp_check() with various clock bases */
	skb->tstamp = 0;

	skb_orphan(skb);

	/* Before queueing this packet to netif_rx(),
	 * make sure dst is refcounted.
	 */
	skb_dst_force(skb);

	skb->protocol = eth_type_trans(skb, dev);

	len = skb->len;
	if (likely(netif_rx(skb) == NET_RX_SUCCESS))
		kdev_lstats_add(dev, len);

	return NETDEV_TX_OK;
}

static void kdev_lstats_read(struct net_device *dev, u64 *packets, u64 *bytes)
{
	int i;

	*packets = 0;
	*bytes = 0;

	for_each_possible_cpu(i) {
		const struct pcpu_lstats *lb_stats;
		u64 tbytes, tpackets;
		unsigned int start;

		lb_stats = per_cpu_ptr(dev->lstats, i);
		do {
			start = u64_stats_fetch_begin_irq(&lb_stats->syncp);
			tpackets = u64_stats_read(&lb_stats->packets);
			tbytes = u64_stats_read(&lb_stats->bytes);
		} while (u64_stats_fetch_retry_irq(&lb_stats->syncp, start));
		*bytes   += tbytes;
		*packets += tpackets;
	}
}

static void kloopback_get_stats64(struct net_device *dev,
				 struct rtnl_link_stats64 *stats)
{
	u64 packets, bytes;

	kdev_lstats_read(dev, &packets, &bytes);

	stats->rx_packets = packets;
	stats->tx_packets = packets;
	stats->rx_bytes   = bytes;
	stats->tx_bytes   = bytes;
}

static u32 kalways_on(struct net_device *dev)
{
	return 1;
}

static const struct ethtool_ops kloopback_ethtool_ops = {
	.get_link		= kalways_on,
	.get_ts_info		= ethtool_op_get_ts_info,
};

static int kloopback_dev_init(struct net_device *dev)
{
	dev->lstats = netdev_alloc_pcpu_stats(struct pcpu_lstats);
	if (!dev->lstats)
		return -ENOMEM;
	return 0;
}

static void kloopback_dev_free(struct net_device *dev)
{
	free_percpu(dev->lstats);
}

static const struct net_device_ops kloopback_ops = {
	.ndo_init        = kloopback_dev_init,
	.ndo_start_xmit  = kloopback_xmit,
	.ndo_get_stats64 = kloopback_get_stats64,
	.ndo_set_mac_address = eth_mac_addr,
};

static void kgen_lo_setup(struct net_device *dev,
			 unsigned int mtu,
			 const struct ethtool_ops *eth_ops,
			 const struct header_ops *hdr_ops,
			 const struct net_device_ops *dev_ops,
			 void (*dev_destructor)(struct net_device *dev))
{
	dev->mtu		= mtu;
	dev->hard_header_len	= ETH_HLEN;	/* 14	*/
	dev->min_header_len	= ETH_HLEN;	/* 14	*/
	dev->addr_len		= ETH_ALEN;	/* 6	*/
	dev->type		= ARPHRD_LOOPBACK;	/* 0x0001*/
	dev->flags		= IFF_LOOPBACK;
	dev->priv_flags		|= IFF_LIVE_ADDR_CHANGE | IFF_NO_QUEUE;
	netif_keep_dst(dev);
	dev->hw_features	= NETIF_F_GSO_SOFTWARE;
	dev->features		= NETIF_F_SG | NETIF_F_FRAGLIST
		| NETIF_F_GSO_SOFTWARE
		| NETIF_F_HW_CSUM
		| NETIF_F_RXCSUM
		| NETIF_F_SCTP_CRC
		| NETIF_F_HIGHDMA
		| NETIF_F_LLTX
		| NETIF_F_NETNS_LOCAL
		| NETIF_F_VLAN_CHALLENGED
		| NETIF_F_LOOPBACK;
	dev->ethtool_ops	= eth_ops;
	dev->header_ops		= hdr_ops;
	dev->netdev_ops		= dev_ops;
	dev->needs_free_netdev	= true;
	dev->priv_destructor	= dev_destructor;
}

/* The loopback device is special. There is only one instance
 * per network namespace.
 */
void kloopback_setup(struct net_device *dev)
{
	kgen_lo_setup(dev, (64 * 1024), &kloopback_ethtool_ops, &keth_header_ops,
		     &kloopback_ops, kloopback_dev_free);
}

#if 0
/* Setup and register the loopback device. */
static int kloopback_net_init(char *name)
{
	struct net_device *dev;
	int err;

	err = -ENOMEM;
	dev = alloc_netdev(0, name, NET_NAME_UNKNOWN, kloopback_setup);
	if (!dev)
		return -ENOMEM;

	//dev_net_set(dev, net);
	err = register_netdev(dev);
	if (err)
		goto out_free_netdev;

	BUG_ON(dev->ifindex != LOOPBACK_IFINDEX);
	return 0;

out_free_netdev:
	free_netdev(dev);

	return err;
}


struct eth_interface *ethkernel_create(char *name, ifindex_t ifindex)
{
  struct net_device *ndev;
  struct eth_interface *db;
  int ret = 0;

  /* Init network device */
  ndev = alloc_netdev(sizeof(struct eth_interface), name, NET_NAME_UNKNOWN, ether_setup);
  //ndev = alloc_etherdev(sizeof(struct eth_interface));
  if (!ndev)
  {
    ret = -ENOMEM;
    return NULL;
  }
  //SET_NETDEV_DEV(ndev, &pdev->dev);
  /* setup board info structure */
  db = netdev_priv(ndev);

  db->ndev = ndev;

  //skb_queue_head_init (&db->_eth_tx_queue);
  //skb_queue_head_init (&db->_eth_rx_queue);

  mutex_init(&db->mutex_lock);

  ndev->netdev_ops = &ethkernel_netdev_ops;

  /* Register netdevice. */
  ret = register_netdev(ndev);
  if (ret)
  {
    free_netdev(ndev);
    return NULL;
  }
  if(mac)
    memcpy(ndev->dev_addr, mac, ndev->addr_len);
  db->upifindex = ifindex;
  ethkernel_list_add(db); 
  return db;
}


/* Destroy netdevice. */
int ethkernel_destroy(struct eth_interface *db)
{
  /*struct sk_buff *skb = NULL;  
  while ((skb = skb_dequeue (&db->_eth_tx_queue)) != NULL)
    kfree_skb (skb);
  while ((skb = skb_dequeue (&db->_eth_rx_queue)) != NULL)
    kfree_skb (skb);  */
  ethkernel_list_del(db);  
  unregister_netdev(db->ndev);
  free_netdev(db->ndev);
  return 0;
}

#endif