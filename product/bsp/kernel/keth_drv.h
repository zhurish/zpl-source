/* Copyright (C) 2004-2011 IP Infusion, Inc. All Rights Reserved. */

#ifndef __KETH_DRV_H__
#define __KETH_DRV_H__


struct eth_interface {
	struct list_head node;
	struct net_device  *ndev;

	int (*eth_rx)(struct net_device *, char *, int );
	struct mutex	mutex_lock;

	ifindex_t		upifindex;
};

void kloopback_setup(struct net_device *dev);

int ethkernel_init(void);
struct eth_interface * ethkernel_probe(char *name, ifindex_t ifindex, char *mac);
int ethkernel_destroy(struct eth_interface *db);
int kbsp_l3if_module_handle(struct khal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);

#endif /* __KETH_DRV_H__ */
