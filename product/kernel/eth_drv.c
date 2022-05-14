// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *      Davicom DM9000 Fast Ethernet driver for Linux.
 * 	Copyright (C) 1997  Sten Wang
 *
 * (C) Copyright 1997-1998 DAVICOM Semiconductor,Inc. All Rights Reserved.
 *
 * Additional updates, Copyright:
 *	Ben Dooks <ben@simtec.co.uk>
 *	Sascha Hauer <s.hauer@pengutronix.de>
 */

#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/crc32.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include "eth_drv.h"
#include "eth_netlink.h"
/*
 * Debug messages level
 */
static int debug;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "dm9000 debug level (0-6)");

/* debug code */

#define ethkernel_dbg(db, lev, msg...)

static inline struct eth_interface *to_ethkernel_board(struct net_device *dev)
{
  return netdev_priv(dev);
}

static void ethkernel_schedule_poll(struct eth_interface *db)
{
  schedule_delayed_work(&db->phy_poll, HZ * 2);
}

static int ethkernel_ioctl(struct net_device *dev, struct ifreq *req, int cmd)
{
  struct eth_interface *dm = to_ethkernel_board(dev);

  if (!netif_running(dev))
    return -EINVAL;

  return 0; // generic_mii_ioctl(&dm->mii, if_mii(req), cmd, NULL);
}

static void
ethkernel_poll_work(struct work_struct *w)
{
  struct delayed_work *dw = to_delayed_work(w);
  struct eth_interface *db = container_of(dw, struct eth_interface, phy_poll);
  struct net_device *ndev = db->ndev;

  if (netif_running(ndev))
    ethkernel_schedule_poll(db);
}

/* Our watchdog timed out. Called by the networking layer */
static void ethkernel_timeout(struct net_device *dev, unsigned int txqueue)
{
  struct eth_interface *db = netdev_priv(dev);

  /* Save previous register address */
  mutex_lock(&db->mutex_lock);

  netif_stop_queue(dev);

  /* We can accept TX packets again */
  netif_trans_update(dev); /* prevent tx timeout */
  netif_wake_queue(dev);

  mutex_unlock(&db->mutex_lock);
}

static void ethkernel_send_packet(struct net_device *dev,
                                  int ip_summed,
                                  u16 pkt_len)
{
}

/*
 *  Hardware start transmission.
 *  Send a packet to media from the upper layer.
 */
static int ethkernel_start_xmit(struct sk_buff *skb, struct net_device *dev)
{

  struct eth_interface *db = netdev_priv(dev);

  ethkernel_dbg(db, 3, "%s:\n", __func__);

  skb_queue_tail (&db->_eth_tx_queue, skb);

  if (db->tx_pkt_cnt > 1)
    return NETDEV_TX_BUSY;

  mutex_lock(&db->mutex_lock);

  dev->stats.tx_bytes += skb->len;

  db->tx_pkt_cnt++;
  /* TX control: First packet immediately send, second packet queue */
  if (db->tx_pkt_cnt == 1)
  {
    ethkernel_send_packet(dev, skb->ip_summed, skb->len);
  }
  else
  {
    /* Second packet */
    db->queue_pkt_len = skb->len;
    db->queue_ip_summed = skb->ip_summed;
    netif_stop_queue(dev);
  }

  mutex_unlock(&db->mutex_lock);

  /* free this SKB */
  dev_consume_skb_any(skb);

  return NETDEV_TX_OK;
}

/*
 * DM9000 interrupt handler
 * receive the packet to upper layer, free the transmitted packet
 */

static void ethkernel_tx_done(struct net_device *dev, struct eth_interface *db)
{
  /* One packet sent complete */
  db->tx_pkt_cnt--;
  dev->stats.tx_packets++;

  /* Queue packet check & send */
  if (db->tx_pkt_cnt > 0)
    ethkernel_send_packet(dev, db->queue_ip_summed,
                          db->queue_pkt_len);
  netif_wake_queue(dev);
}

/*
 *  Received a packet and pass to upper layer
 */
static void ethkernel_rx(struct net_device *dev, char *data, int RxLen)
{
  struct eth_interface *db = netdev_priv(dev);
  struct sk_buff *skb;
  while ((skb = skb_dequeue (&db->_eth_rx_queue)) != NULL)
  {
    // dev = skb->dev;
  }
  /* Packet Status check */
  if (RxLen < 0x40)
  {
    dev->stats.rx_length_errors++;
  }

  //dev->stats.rx_fifo_errors++;

  //dev->stats.rx_crc_errors++;

  skb = netdev_alloc_skb(dev, RxLen + 4);

  /* Move data from DM9000 */
  if (skb != NULL)
  {
    skb_reserve(skb, 2);
    char *rdptr = skb_put(skb, RxLen - 4);

    /* Read received packet from RX SRAM */

    memcpy(rdptr, data, RxLen);
    dev->stats.rx_bytes += RxLen;

    /* Pass to upper layer */
    skb->protocol = eth_type_trans(skb, dev);
    netif_rx(skb);
    dev->stats.rx_packets++;
  }
}

/*
 *  Open the interface.
 *  The interface is opened whenever "ifconfig" actives it.
 */
static int
ethkernel_open(struct net_device *dev)
{
  struct eth_interface *db = netdev_priv(dev);


  netif_start_queue(dev);

  /* Poll initial link status */
  schedule_delayed_work(&db->phy_poll, 1);

  return 0;
}

/*
 * Stop the interface.
 * The interface is stopped when it is brought.
 */
static int ethkernel_stop(struct net_device *ndev)
{
  struct eth_interface *db = netdev_priv(ndev);

  cancel_delayed_work_sync(&db->phy_poll);

  netif_stop_queue(ndev);
  netif_carrier_off(ndev);

  return 0;
}

static int ethkernel_dev_set_mac_addr(struct net_device *dev, void *p)
{
  struct sockaddr *addr = (struct sockaddr *)p;

  if (netif_running(dev))
    return -EBUSY;

  /* Set address. */
  memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

  return 0;
}

static int ethkernel_change_mtu(struct net_device *dev, int new_mtu)
{
  if (new_mtu < 1500 || new_mtu > 8192)
    return -EINVAL;
  dev->mtu = new_mtu;
  return 0;
}

static struct net_device_stats *ethkernel_get_stats(struct net_device *dev)
{
  return &dev->stats;
}

static const struct net_device_ops ethkernel_netdev_ops = {
    .ndo_open = ethkernel_open,
    .ndo_stop = ethkernel_stop,
    .ndo_start_xmit = ethkernel_start_xmit,
    .ndo_tx_timeout = ethkernel_timeout,
    .ndo_set_rx_mode = NULL,
    .ndo_eth_ioctl = ethkernel_ioctl,
    //.ndo_set_features	= ethkernel_set_features,
    .ndo_set_mac_address = ethkernel_dev_set_mac_addr,
    .ndo_change_mtu = ethkernel_change_mtu,
    .ndo_get_stats = ethkernel_get_stats,
};

const struct header_ops eth_header_ops ____cacheline_aligned = {
    .create = eth_header,
    .parse = eth_header_parse,
    //.rebuild        = eth_rebuild_header,
    .cache = eth_header_cache,
    .cache_update = eth_header_cache_update,
};

static void ethkernel_ether_setup(struct net_device *dev)
{
  dev->header_ops = &eth_header_ops;

  /* Overridden only for change_mtu for devices supporting jumbo frames. */

  dev->netdev_ops = &ethkernel_netdev_ops;

  dev->type = 0; // ARPHRD_ETHER;
  dev->hard_header_len = ETH_HLEN;
  dev->mtu = ETH_DATA_LEN;
  dev->addr_len = ETH_ALEN;
  dev->tx_queue_len = 1000; /* Ethernet wants good queues */
  dev->flags = IFF_BROADCAST | IFF_MULTICAST;

  memset(dev->broadcast, 0xFF, ETH_ALEN);
}

/*
 * Search DM9000 board, allocate space and register it
 */
struct eth_interface *ethkernel_probe(char *name)
{
  struct net_device *ndev;
  struct eth_interface *db;
  int ret = 0;

  /* Init network device */
  ndev = alloc_etherdev(sizeof(struct eth_interface));
  if (!ndev)
  {
    ret = -ENOMEM;
    return ret;
  }

  /* setup board info structure */
  db = netdev_priv(ndev);

  db->ndev = ndev;


  skb_queue_head_init (&db->_eth_tx_queue);
  skb_queue_head_init (&db->_eth_rx_queue);

  mutex_init(&db->mutex_lock);

  INIT_DELAYED_WORK(&db->phy_poll, ethkernel_poll_work);

  ndev->netdev_ops = &ethkernel_netdev_ops;

  // ndev->netdev_ops	= &dm9000_netdev_ops;
  // ndev->watchdog_timeo	= msecs_to_jiffies(watchdog);
  // ndev->ethtool_ops	= &dm9000_ethtool_ops;


  ethkernel_ether_setup(ndev);
  /* Register netdevice. */
  ret = register_netdev(ndev);
  if (ret)
  {
    return NULL;
  }
  netif_carrier_off(ndev);

  return ret;
}
EXPORT_SYMBOL(ethkernel_probe);

  struct sk_buff_head   _eth_tx_queue;
  struct sk_buff_head   _eth_rx_queue;

/* Destroy netdevice. */
int ethkernel_destroy_netdevice(struct net_device *ndev)
{
  struct sk_buff *skb;  
  struct eth_interface *db;
  db = netdev_priv(ndev);
  while ((skb = skb_dequeue (&db->_eth_tx_queue)) != NULL)
    kfree_skb (skb);
  while ((skb = skb_dequeue (&db->_eth_rx_queue)) != NULL)
    kfree_skb (skb);  
  unregister_netdev(ndev);
  return 0;
}

EXPORT_SYMBOL(ethkernel_destroy_netdevice);
/*
  Linux kernel module init and denint registrations.
*/
static int __init _hsl_module_init(void)
{
  eth_netlink_init();
  return 0;
}

static void __exit _hsl_module_deinit(void)
{
  eth_netlink_exit();
}

module_init(_hsl_module_init);
module_exit(_hsl_module_deinit);

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("IP Infusion Inc.");
MODULE_DESCRIPTION("IP Infusion Broadcom Hardware Services Layer");
