/* Copyright (C) 2004-2011 IP Infusion, Inc. All Rights Reserved. */

#ifndef _HSL_ETH_DRV_H
#define _HSL_ETH_DRV_H


struct eth_interface {
	struct net_device  *ndev;
	struct delayed_work phy_poll;
  struct sk_buff_head   _eth_tx_queue;
  struct sk_buff_head   _eth_rx_queue;
	struct mutex	mutex_lock;
	unsigned short	tx_pkt_cnt;
	unsigned short	queue_pkt_len;
	unsigned short	queue_start_addr;
	unsigned short	queue_ip_summed;
  int in_suspend;
};

struct eth_interface * ethkernel_probe(char *name);
int ethkernel_destroy_netdevice(struct net_device *ndev);


#endif /* _HSL_ETH_DRV_H */
