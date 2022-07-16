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
#include <linux/list.h>
#include <linux/if_arp.h>

#include "bsp_types.h"
#include "khal_client.h"
#include "keth_drv.h"
#include "khal_netpkt.h"
#include "khal_netlink.h"

struct list_head keth_head;

int ethkernel_init(void)
{
  INIT_LIST_HEAD(&keth_head);
  return OK;
}

static int ethkernel_list_add(struct eth_interface *dev)
{
  list_add(&dev->node, &keth_head);
  return OK;
}

static int ethkernel_list_del(struct eth_interface *dev)
{
  list_del(&dev->node);
  return OK;
}


static struct eth_interface * ethkernel_lookup(ifindex_t ifindex)
{
	struct eth_interface *dev = NULL;
  if (list_empty(&keth_head))
    return NULL;
	list_for_each_entry(dev, &keth_head, node)
  {
		if (dev->upifindex == ifindex) 
    {
			return dev;
		}
  }
  return NULL;
}


static inline struct eth_interface *to_ethkernel_board(struct net_device *dev)
{
  return netdev_priv(dev);
}




static void ethkernel_send_packet(struct net_device *dev,struct sk_buff *skb)
{
  netpkt_fromdev(dev, skb);
}

/*
 *  Hardware start transmission.
 *  Send a packet to media from the upper layer.
 */
static int ethkernel_start_xmit(struct sk_buff *skb, struct net_device *dev)
{

  struct eth_interface *db = netdev_priv(dev);
  /* Stop queue. */
  netif_stop_queue (dev);
  skb->dev = dev;
/*  skb_queue_tail (&db->_eth_tx_queue, skb);

  if (db->tx_pkt_cnt > 1)
    return NETDEV_TX_BUSY;
*/
  mutex_lock(&db->mutex_lock);
//	(db->outblk)(db->io_data, skb->data, skb->len);

  dev->stats.tx_bytes += skb->len;
  dev->stats.tx_packets++;

  ethkernel_send_packet(dev, skb);
    /* Free skb. */
  //kfree_skb (skb);
  mutex_unlock(&db->mutex_lock);

  /* Wakeup queue. */
  netif_wake_queue (dev);
  return NETDEV_TX_OK;
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
static int ethkernel_open(struct net_device *dev)
{
  netif_start_queue(dev);
  return 0;
}

/*
 * Stop the interface.
 * The interface is stopped when it is brought.
 */
static int ethkernel_stop(struct net_device *ndev)
{
  netif_stop_queue(ndev);
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
    .ndo_set_mac_address = ethkernel_dev_set_mac_addr,
    .ndo_change_mtu = ethkernel_change_mtu,
    .ndo_get_stats = ethkernel_get_stats,
};

const struct header_ops eth_header_ops ____cacheline_aligned = {
    .create = eth_header,
    .parse = eth_header_parse,
    .cache = eth_header_cache,
    .cache_update = eth_header_cache_update,
};

static void ethkernel_ether_setup(struct net_device *dev)
{
  dev->header_ops = &eth_header_ops;

  /* Overridden only for change_mtu for devices supporting jumbo frames. */

  dev->netdev_ops = &ethkernel_netdev_ops;

  dev->type =  ARPHRD_ETHER;
  dev->hard_header_len = ETH_HLEN;
  dev->mtu = ETH_DATA_LEN;
  dev->addr_len = ETH_ALEN;
  dev->tx_queue_len = 1000; /* Ethernet wants good queues */
  dev->flags = IFF_BROADCAST | IFF_MULTICAST;

  memset(dev->broadcast, 0xFF, ETH_ALEN);
}
/*
static void ethkernel_slip_setup(struct net_device *ndev)
{
  struct eth_interface *db = netdev_priv(ndev);
	slip_proto_init(&db->slip);

	ndev->hard_header_len = 0;
	ndev->header_ops = NULL;
	ndev->addr_len = 0;
	ndev->type = ARPHRD_SLIP;
	ndev->tx_queue_len = 256;
	ndev->flags = IFF_NOARP;
}
*/
/*
 * Search DM9000 board, allocate space and register it
 */
struct eth_interface *ethkernel_create(char *name, ifindex_t ifindex, char *mac)
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
  if(mac)
    memcpy(ndev->dev_addr, mac, ndev->addr_len);
  db->upifindex = ifindex;
  ethkernel_list_add(db); 
  return ret;
}


/* Destroy netdevice. */
int ethkernel_destroy(struct eth_interface *db)
{
  struct sk_buff *skb = NULL;  
  while ((skb = skb_dequeue (&db->_eth_tx_queue)) != NULL)
    kfree_skb (skb);
  while ((skb = skb_dequeue (&db->_eth_rx_queue)) != NULL)
    kfree_skb (skb);  
  ethkernel_list_del(db);  
  unregister_netdev(db->ndev);
  return 0;
}

static int bsp_l3if_create(void *driver, hal_l3if_param_t *l3ifparam, hal_l3if_addr_param_t *l3addrparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
  hal_port_header_t  *port = &l3ifparam->port;
  ifindex_t upifindex = IF_INFINDEX_GET(port);
  struct eth_interface *ethdev = NULL;
	BSP_ENTER_FUNC();
  ethdev = ethkernel_lookup(upifindex);
  if(ethdev == NULL)
  {
    ethdev = ethkernel_create(l3ifparam->ifname, upifindex, l3ifparam->mac);
    if(ethdev)
      ret = OK;
  }
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3if_destroy(void *driver, hal_l3if_param_t *l3ifparam, hal_l3if_addr_param_t *l3addrparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
  hal_port_header_t  *port = &l3ifparam->port;
  ifindex_t upifindex = IF_INFINDEX_GET(port);
  struct eth_interface *ethdev = NULL;
	BSP_ENTER_FUNC();
  ethdev = ethkernel_lookup(upifindex);
  if(ethdev != NULL)
  {
    ret = ethkernel_destroy(ethdev);
  }
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3addr_add(void *driver, hal_l3if_param_t *l3ifparam, hal_l3if_addr_param_t *l3addrparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
  hal_port_header_t  *port = &l3ifparam->port;
  ifindex_t upifindex = IF_INFINDEX_GET(port);
  struct eth_interface *ethdev = NULL;
	BSP_ENTER_FUNC();
  ethdev = ethkernel_lookup(upifindex);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3addr_del(void *driver, hal_l3if_param_t *l3ifparam, hal_l3if_addr_param_t *l3addrparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
  hal_port_header_t  *port = &l3ifparam->port;
  ifindex_t upifindex = IF_INFINDEX_GET(port);
  struct eth_interface *ethdev = NULL;
	BSP_ENTER_FUNC();
  ethdev = ethkernel_lookup(upifindex);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3dstaddr_add(void *driver, hal_l3if_param_t *l3ifparam, hal_l3if_addr_param_t *l3addrparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
  hal_port_header_t  *port = &l3ifparam->port;
  ifindex_t upifindex = IF_INFINDEX_GET(port);
  struct eth_interface *ethdev = NULL;
	BSP_ENTER_FUNC();
  ethdev = ethkernel_lookup(upifindex);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3dstaddr_del(void *driver, hal_l3if_param_t *l3ifparam, hal_l3if_addr_param_t *l3addrparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
  hal_port_header_t  *port = &l3ifparam->port;
  ifindex_t upifindex = IF_INFINDEX_GET(port);
  struct eth_interface *ethdev = NULL;
	BSP_ENTER_FUNC();
  ethdev = ethkernel_lookup(upifindex);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3if_vrf(void *driver, hal_l3if_param_t *l3ifparam, hal_l3if_addr_param_t *l3addrparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
  hal_port_header_t  *port = &l3ifparam->port;
  ifindex_t upifindex = IF_INFINDEX_GET(port);
  struct eth_interface *ethdev = NULL;
	BSP_ENTER_FUNC();
  ethdev = ethkernel_lookup(upifindex);
	BSP_LEAVE_FUNC();
	return ret;
}

static int bsp_l3if_mac(void *driver, hal_l3if_param_t *l3ifparam, hal_l3if_addr_param_t *l3addrparam)
{
	int ret = NO_SDK;
	BSP_DRIVER(bspdev, driver);
  hal_port_header_t  *port = &l3ifparam->port;
  ifindex_t upifindex = IF_INFINDEX_GET(port);
  struct eth_interface *ethdev = NULL;
	BSP_ENTER_FUNC();
  ethdev = ethkernel_lookup(upifindex);
  if(ethdev != NULL)
  {
    memcpy(ethdev->ndev->dev_addr, l3ifparam->mac, ethdev->ndev->addr_len);
    ret = OK;//ethkernel_dev_set_mac_addr(ethdev->dev, l3ifparam.mac);
  }
	BSP_LEAVE_FUNC();
	return ret;
}

static hal_ipcsubcmd_callback_t subcmd_table[] = {
	HAL_CALLBACK_ENTRY(HAL_L3IF_NONE, NULL),
	HAL_CALLBACK_ENTRY(HAL_L3IF_CREATE, bsp_l3if_create),
	HAL_CALLBACK_ENTRY(HAL_L3IF_DELETE, bsp_l3if_destroy),
	HAL_CALLBACK_ENTRY(HAL_L3IF_ADDR_ADD, bsp_l3addr_add),
	HAL_CALLBACK_ENTRY(HAL_L3IF_ADDR_DEL, bsp_l3addr_del),
  HAL_CALLBACK_ENTRY(HAL_L3IF_DSTADDR_ADD, bsp_l3dstaddr_add),
  HAL_CALLBACK_ENTRY(HAL_L3IF_DSTADDR_DEL, bsp_l3dstaddr_del),
  HAL_CALLBACK_ENTRY(HAL_L3IF_VRF, bsp_l3if_vrf),
  HAL_CALLBACK_ENTRY(HAL_L3IF_MAC, bsp_l3if_mac),
};


int bsp_l3if_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver)
{
	int ret = OK;
	hal_l3if_param_t	l3ifparam;
	hal_l3if_addr_param_t	l3addrparam;
	int i = 0;
	hal_ipcsubcmd_callback_t * callback = NULL;
	BSP_ENTER_FUNC();	
	for(i = 0; i < ZPL_ARRAY_SIZE(subcmd_table); i++)
	{
		if(subcmd_table[i].subcmd == subcmd && subcmd_table[i].cmd_handle)
		{
            callback = &subcmd_table[i];
			break;
		}
	}
	if(callback == NULL)
	{
		zlog_warn(MODULE_HAL, "Can not Find this subcmd:%d ", subcmd);
		BSP_LEAVE_FUNC();
		return OS_NO_CALLBACK;
	}
  hal_ipcmsg_port_get(&client->ipcmsg, &l3ifparam.port);

	switch(subcmd)
	{
	case HAL_L3IF_CREATE:
	  hal_ipcmsg_get(&client->ipcmsg, &l3ifparam.ifname, 64);
    hal_ipcmsg_get(&client->ipcmsg, &l3ifparam.mac, 6);
	  break;
	case HAL_L3IF_DELETE:
	  break;
	case HAL_L3IF_VRF:
	  hal_ipcmsg_getw(&client->ipcmsg, &l3ifparam.vrfid);
	  break;
	case HAL_L3IF_MAC:
	  hal_ipcmsg_get(&client->ipcmsg, &l3ifparam.mac, 6);
	  break;
	case HAL_L3IF_ADDR_ADD:
	case HAL_L3IF_ADDR_DEL:
	case HAL_L3IF_DSTADDR_ADD:
	case HAL_L3IF_DSTADDR_DEL:
    memcpy(&l3addrparam.port, &l3ifparam.port, sizeof(hal_port_header_t));
    hal_ipcmsg_getc(&client->ipcmsg, &l3addrparam.family);
    hal_ipcmsg_getc(&client->ipcmsg, &l3addrparam.prefixlen);
    if (l3addrparam.family == AF_INET)
      hal_ipcmsg_getl(&client->ipcmsg, &l3addrparam.address.ipv4.s_addr);
    else
      hal_ipcmsg_get(&client->ipcmsg, &l3addrparam.address.ipv6.s6_addr, 16); 
    if(subcmd == HAL_L3IF_ADDR_DEL || subcmd == HAL_L3IF_ADDR_ADD)   
    {
      hal_ipcmsg_getc(&client->ipcmsg, &l3addrparam.sec);
    }
    break;
	}

	switch (cmd)
	{
	case HAL_MODULE_CMD_REQ:         //设置
	ret = (callback->cmd_handle)(driver, &l3ifparam, &l3addrparam);
	break;
	default:
		break;
	}
	
	BSP_LEAVE_FUNC();
	return ret;
}