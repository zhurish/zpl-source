/* Copyright 2009 IP Infusion, Inc. All Rights Reserved.  */

#include <linux/delay.h>
#include <linux/export.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_data/b53.h>
#include <linux/phy.h>
#include <linux/phylink.h>
#include <linux/etherdevice.h>
#include <linux/if_bridge.h>
#include <net/dsa.h>

#include <linux/init.h>
#include <linux/stat.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include <net/netlink.h>
#include <net/sock.h>

#include "../b53_regs.h"
#include "../b53_priv.h"

#include "pkt_netlink.h"

#define NETLINK_TEST (MAX_LINKS-1)



struct nk_pkt_sock
{
    dev_t devId;
    struct class *cls;
    struct sock *nl_sk;
    struct net_device *netdev;
    int start;
	int pid;
};

static struct nk_pkt_sock _pkt_netlink;

static void nk_pkt_sock_cleanup(void)
{
	netlink_kernel_release(_pkt_netlink.nl_sk);
	device_destroy(_pkt_netlink.cls, _pkt_netlink.devId);
	class_destroy(_pkt_netlink.cls);
	unregister_chrdev_region(_pkt_netlink.devId, 1);
}



static struct sk_buff *_nk_pkt_sock_post_skb (const char *buf, int size)
{
  struct sk_buff *skb = NULL;
  skb = alloc_skb (size, GFP_ATOMIC);
  if (! skb)
    return NULL;
  memcpy (skb->data, buf, size);
  skb->len = size;
  return skb;
}

static void nk_pkt_sock_send_tocpu(struct net_device *dev, const char *buf, int size)
{
	struct sk_buff *skb = _nk_pkt_sock_post_skb (buf,  size);
	if(skb)
	{
		skb->dev = dev;
		netif_rx(skb);
	}
}

static void nk_pkt_sock_send_todev(struct net_device *dev, const char *buf, int size)
{
	struct sk_buff *skb = _nk_pkt_sock_post_skb (buf,  size);
	if(skb && dev)
	{
		skb->dev = dev;
		dev->netdev_ops->ndo_start_xmit(skb, dev);
		kfree_skb (skb);
	}
}

static int nk_pkt_sock_skb_recv_from_dev(const struct sk_buff *skb, const struct net_device *dev)
{
	if(_pkt_netlink.nl_sk && _pkt_netlink.start && _pkt_netlink.pid)
	{
		if(netlink_unicast(_pkt_netlink.nl_sk, (struct sk_buff *)skb, _pkt_netlink.pid, MSG_DONTWAIT) == 0)
			return NET_RX_DROP;
	}
	return NET_RX_SUCCESS;
}

static void nk_pkt_sock_cmd(int cmd, struct nk_pkt_start *nkpkt, const char *skb, int len)
{
	struct net_device *dev = NULL;
	switch(cmd)
	{
	case NK_PKT_START:
	_pkt_netlink.start = 1;
	break;
	case NK_PKT_STOP:
	_pkt_netlink.start = 0;
	break;
	case NK_PKT_SETUP:
	{
		_pkt_netlink.netdev = dev_get_by_index(&init_net, nkpkt->ifindex);
		if(_pkt_netlink.netdev)
		{
			_pkt_netlink.pid = nkpkt->value;
			_pkt_netlink.netdev->ndev_dsa_pktrx = nk_pkt_sock_skb_recv_from_dev;
		}
	}
	break;
	case NK_PKT_DATA:
		nk_pkt_sock_send_todev(_pkt_netlink.netdev, skb, len);
	break;
	case NK_PKT_CPU:
	{
		dev = dev_get_by_index(&init_net, nkpkt->ifindex);
		if(dev)
			nk_pkt_sock_send_tocpu(dev, skb, len);
	}
	break;

	case NK_PKT_TEST:
	{

	}
	default:
	break;
	}
}

static void nk_pkt_sock_input(struct sk_buff *__skb)
{
	struct nk_pkt_start *nkpkt = NULL;
	struct nlmsghdr *nlh = nlmsg_hdr(__skb);
	const char *nlmsg = NULL;
	if (__skb->len < sizeof(*nlh) ||
	    nlh->nlmsg_len < sizeof(*nlh) ||
	    __skb->len < nlh->nlmsg_len)
		return;

	nkpkt = (struct nk_pkt_start *)nlmsg_data(nlh);
	nlmsg = (const char *)nlmsg_data(nlh);
	nk_pkt_sock_cmd(nlh->nlmsg_type, nkpkt, nlmsg + sizeof(struct nk_pkt_start), 
		nlmsg_len(nlh) - sizeof(struct nk_pkt_start));
}

struct netlink_kernel_cfg _nk_pkt_sock_nkc = {
	.groups = 0,
	.flags = 0,
	.input = nk_pkt_sock_input,
	.cb_mutex = NULL,
	.bind = NULL,
	.unbind = NULL,
	.compare = NULL,
};

static __init int nk_pkt_sock_init(void)
{
	int result;
	
	memset(&_pkt_netlink, 0, sizeof(_pkt_netlink));
	printk(KERN_WARNING "netlink init start!\n");

	//动态注册设备号
	if ((result = alloc_chrdev_region(&_pkt_netlink.devId, 0, 1, "pktnl")) !=
	    0) {
		printk(KERN_WARNING "register dev id error:%d\n", result);
		goto err;
	} else {
		printk(KERN_WARNING "register dev id success!\n");
	}
	//动态创建设备节点
	_pkt_netlink.cls = class_create(THIS_MODULE, "pktnl");
	if (IS_ERR(_pkt_netlink.cls)) {
		printk(KERN_WARNING "create class error!\n");
		goto err;
	}
	if (device_create(_pkt_netlink.cls, NULL, _pkt_netlink.devId, "", "pktnl%d", 0) == NULL) {
		printk(KERN_WARNING "create device error!\n");
		goto err;
	}

	//初始化netlink
	_pkt_netlink.nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, &_nk_pkt_sock_nkc);
	if (!_pkt_netlink.nl_sk) {
		printk(KERN_ERR "[netlink] create netlink socket error!\n");
		goto err;
	}

	printk(KERN_ALERT "netlink init success!\n");
	return 0;
err:
	nk_pkt_sock_cleanup();
	return -1;
}

static __exit void nk_pkt_sock_exit(void)
{
	nk_pkt_sock_cleanup();
	printk(KERN_WARNING "netlink exit!\n");
}

module_init(nk_pkt_sock_init);
module_exit(nk_pkt_sock_exit);

MODULE_AUTHOR("zhurish <zhurish@163.com>");
MODULE_DESCRIPTION("B53 switch pkt library");
MODULE_LICENSE("Dual BSD/GPL");