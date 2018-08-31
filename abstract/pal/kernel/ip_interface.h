/*
 * ip_interface.h
 *
 *  Created on: Jul 14, 2018
 *      Author: zhurish
 */

#ifndef ABSTRACT_PAL_KERNEL_IP_INTERFACE_H_
#define ABSTRACT_PAL_KERNEL_IP_INTERFACE_H_

#define HAVE_UTILS_BRCTL
#define HAVE_UTILS_TUNNEL
#define HAVE_UTILS_VLAN

/*type*/
#define UTILS_IF_BRIDGE (1)
#define UTILS_IF_TUNNEL (2)
#define UTILS_IF_VLAN (3)


struct utils_interface
{
	struct interface *ifp;
	int type;//接口类型，tunnel，vlan，bridge
	char name[INTERFACE_NAMSIZ + 1];
#ifdef HAVE_UTILS_BRCTL
	int br_mode;//接口状态，网桥，桥接接口
	char br_name[INTERFACE_NAMSIZ + 1];//桥接接口的网桥名称
	int br_stp;//网桥生成树
	int br_stp_state;//生成树状态
	int max_age;
	int hello_time;
	int forward_delay;
	//
#define BR_PORT_MAX	32
	int br_ifindex[BR_PORT_MAX];
#endif
	//int message_age_timer_value;
	//int forward_delay_timer_value;
	//int hold_timer_value;
#ifdef HAVE_UTILS_TUNNEL
	int tun_index;//tunnel接口的隧道ID编号
	int tun_mode;//隧道模式
	int tun_ttl;//change: ip tunnel change tunnel0 ttl
	int tun_mtu;//change: ip link set dev tunnel0 mtu 1400
    struct in_addr source;//ip tunnel change tunnel1 local 192.168.122.1
    struct in_addr remote;//change: ip tunnel change tunnel1 remote 19.1.1.1
    int active;//隧道接口是否激活
#define TUNNEL_ACTIVE 1
#endif
#ifdef HAVE_UTILS_VLAN
#define SKB_QOS_MAX	8
	int vlanid;//vlan id
	int egress_vlan_qos[SKB_QOS_MAX];//出口skb的优先级到qos的映射
	int ingress_vlan_qos[SKB_QOS_MAX];//入口skb的优先级到qos的映射
	//unsigned int flag; /* Matches vlan_dev_priv flags */
#endif
};



#endif /* ABSTRACT_PAL_KERNEL_IP_INTERFACE_H_ */
