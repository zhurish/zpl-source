
#include <linux/init.h>
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
 
#include <net/netlink.h>
#include <net/sock.h>
 
#include "eth_netlink.h"
 
eth_netlink_t _eth_netlink;
 
static void eth_netlink_cleanup(void)
{
    netlink_kernel_release(_eth_netlink.nl_sk);
    device_destroy(_eth_netlink.cls, _eth_netlink.devId);
    class_destroy(_eth_netlink.cls);
    unregister_chrdev_region(_eth_netlink.devId, 1);
}
 
static void eth_netlink_send(int pid, int msgtype, uint8_t *message, int len)
{
        struct sk_buff *skb_1;
        struct nlmsghdr *nlh;
 
        if(!message || !_eth_netlink.nl_sk) {
                return;
        }
 
        skb_1 = alloc_skb(NLMSG_SPACE(len), GFP_KERNEL);
        if( !skb_1 ) {
                printk(KERN_ERR "alloc_skb error!\n");
        }
 
        nlh = nlmsg_put(skb_1, 0, 0, 0, len, 0);
        NETLINK_CB(skb_1).portid = 0;
        NETLINK_CB(skb_1).dst_group = 0;
        nlh->nlmsg_type = msgtype;
        memcpy(NLMSG_DATA(nlh), message, len);
        netlink_unicast(_eth_netlink.nl_sk, skb_1, pid, MSG_DONTWAIT);
}
 
static void eth_netlink_input(struct sk_buff *__skb)
{
        struct sk_buff *skb;
        char str[100];
        struct nlmsghdr *nlh;
 
        if( !__skb ) {
                return;
        }
 
        skb = skb_get(__skb);
        if( skb->len < NLMSG_SPACE(0)) {
                return;
        }
 
        nlh = nlmsg_hdr(skb);

        eth_from_userspace(nlh->nlmsg_type, NLMSG_DATA(nlh), nlh->nlmsg_len - NLMSG_SPACE(0));

        memset(str, 0, sizeof(str));
        memcpy(str, NLMSG_DATA(nlh), sizeof(str));
        printk(KERN_INFO "receive message (pid:%d):%s\n", nlh->nlmsg_pid, str);
        printk(KERN_INFO "space:%d\n", NLMSG_SPACE(0));
        printk(KERN_INFO "size:%d\n", nlh->nlmsg_len);
        eth_netlink_send(nlh->nlmsg_pid, 0, NLMSG_DATA(nlh), nlh->nlmsg_len - NLMSG_SPACE(0));
 
        return;
}

int eth_from_userspace(int msgtype, uint8_t *message, int len)
{
    return 0;
} 

EXPORT_SYMBOL(eth_from_userspace);

int eth_send_userspace(int msgtype, uint8_t *message, int len)
{
    eth_netlink_send(0, msgtype, message,  len);
    return 0;
}

EXPORT_SYMBOL(eth_send_userspace);

int eth_netlink_init(void)
{
        int result;
        struct netlink_kernel_cfg nkc;
 
        printk(KERN_WARNING "netlink init start!\n");
 
        //动态注册设备号
        if(( result = alloc_chrdev_region(&_eth_netlink.devId, 0, 1, "stone-alloc-dev") ) != 0) {
                printk(KERN_WARNING "register dev id error:%d\n", result);
                goto err;
        } else {
                printk(KERN_WARNING "register dev id success!\n");
        }
        //动态创建设备节点
        _eth_netlink.cls = class_create(THIS_MODULE, "stone-class");
        if(IS_ERR(_eth_netlink.cls)) {
                printk(KERN_WARNING "create class error!\n");
                goto err;
        }
        if(device_create(_eth_netlink.cls, NULL, _eth_netlink.devId, "", "hello%d", 0) == NULL) {
                printk(KERN_WARNING "create device error!\n");
                goto err;
        }
 
        //初始化netlink
        nkc.groups = 0;
        nkc.flags = 0;
        nkc.input = eth_netlink_input;
        nkc.cb_mutex = NULL;
        nkc.bind = NULL;
        nkc.unbind = NULL;
        nkc.compare = NULL;
        _eth_netlink.nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, &nkc);
        if( !_eth_netlink.nl_sk ) {
                printk(KERN_ERR "[netlink] create netlink socket error!\n");
                goto err;
        }
 
        printk(KERN_ALERT "netlink init success!\n");
        return 0;
err:
        eth_netlink_cleanup();
        return -1;
}
EXPORT_SYMBOL(eth_netlink_init);

void eth_netlink_exit(void)
{
        eth_netlink_cleanup();
        printk(KERN_WARNING "netlink exit!\n");
}

EXPORT_SYMBOL(eth_netlink_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("IP Infusion Inc.");
MODULE_DESCRIPTION("IP Infusion Broadcom Hardware Services Layer");