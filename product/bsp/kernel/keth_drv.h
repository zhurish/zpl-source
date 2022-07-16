/* Copyright (C) 2004-2011 IP Infusion, Inc. All Rights Reserved. */

#ifndef __KETH_DRV_H__
#define __KETH_DRV_H__


struct eth_interface {
	struct list_head node;
	struct net_device  *ndev;

	struct sk_buff_head   _eth_tx_queue;
	struct sk_buff_head   _eth_rx_queue;
	struct mutex	mutex_lock;

	ifindex_t	upifindex;
};


int ethkernel_init(void);
struct eth_interface * ethkernel_probe(char *name, ifindex_t ifindex, char *mac);
int ethkernel_destroy(struct eth_interface *db);
int bsp_l3if_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);

#endif /* __KETH_DRV_H__ */
