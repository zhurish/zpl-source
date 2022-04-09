/*
 * nsm_firewalld.h
 *
 *  Created on: 2019年8月31日
 *      Author: zhurish
 */

#ifndef __NSM_FIREWALLD_H__
#define __NSM_FIREWALLD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"


/*
 * 对于 INPUT NAT MANGLE 等， WAN侧时源， 本的是目的
 *
 * 四表五链概念

filter表——过滤数据包
Nat表——用于网络地址转换（IP、端口）
Mangle表——修改数据包的服务类型、IPSTACK_TTL、并且可以配置路由实现QOS
Raw表——决定数据包是否被状态跟踪机制处理
INPUT链——进来的数据包应用此规则链中的策略
OUTPUT链——外出的数据包应用此规则链中的策略
FORWARD链——转发数据包时应用此规则链中的策略
PREROUTING链——对数据包作路由选择前应用此链中的规则（所有的数据包进来的时侯都先由这个链处理）
POSTROUTING链——对数据包作路由选择后应用此链中的规则（所有的数据包出来的时侯都先由这个链处理）
 *
 *
 *2.6 iptables NAT表配置
iptables NAT:（配置NAT表就是配置以下两个链）
   01. postrouting（内网---外网）
       路由之后，进行地址映射转换，把源地址进行转换（源私网地址==>源公网地址）
   02. prerouting（外网---内网）
       路由之前，进行地址映射转换，把目标地址进行转换（目标公网地址==>目标变为私网地址）
 iptables -t nat -A POSTROUTING -s 172.16.1.0/24 -o eth0 -j SNAT --to-source 10.0.0.5
-s 172.16.1.0/24      --- 指定将哪些内网网段进行映射转换
-o eth0        --- 指定在共享上网哪个网卡接口上做NAT地址转换
-j SNAT                  --- 将源地址进行转换变更
-j DNAT                  --- 将目标地址进行转换变更
--to-source ip地址         --- 将源地址映射为什么IP地址
--to-destination ip地址    --- 将目标地址映射为什么IP地址
 *
 *
 */

#define FIREWALL_NAME_MAX	32
#define FIREWALL_DEFAULT_PORT_MAX	16

typedef enum firewall_action_s
{
	FIREWALL_A_DROP = 1,
	FIREWALL_A_ACCEPT,
	FIREWALL_A_FORWARD,
	FIREWALL_A_REJECT,

	FIREWALL_A_SNAT,//SNAT即源地址转换，能够让多个内网用户通过一个外网地址上网，解决了IP资源匮乏的问题。一个无线路由器也就使用此技术
	FIREWALL_A_DNAT,//DNAT即目地地址转换，则能够让外网用户访问局域网内不同的服务器
	FIREWALL_A_REDIRECT,//将数据包重新转向到本机或另一台主机的某一个端口，通常功能实现透明代理或对外开放内网的某些服务
	FIREWALL_A_MARK,
	FIREWALL_A_LIMIT,
}firewall_action_t;

typedef enum firewall_proto_s
{
	FIREWALL_P_ALL = 1,
	FIREWALL_P_TCP,
	FIREWALL_P_UDP,
	FIREWALL_P_ICMP,

}firewall_proto_t;


typedef enum firewall_class_s
{
	FIREWALL_C_PORT = 1,
	FIREWALL_C_FILTER,
	FIREWALL_C_SNAT,
	FIREWALL_C_DNAT,
	FIREWALL_C_MANGLE,
	FIREWALL_C_RAW,
}firewall_class_t;


typedef enum firewall_type_s
{
	/* Filter */
	FIREWALL_FILTER_INPUT = 0,
	FIREWALL_FILTER_OUTPUT,
	FIREWALL_FILTER_FORWARD,

	/* Nat */
	FIREWALL_NAT_PREROUTING,
	FIREWALL_NAT_POSTROUTING,
	FIREWALL_NAT_OUTPUT,

	/* Mangle */
	FIREWALL_MANGLE_PREROUTING,
	FIREWALL_MANGLE_OUTPUT,
	FIREWALL_MANGLE_FORWARD,
	FIREWALL_MANGLE_INPUT,
	FIREWALL_MANGLE_POSTROUTING,

	/* Raw */
	FIREWALL_RAW_PREROUTING,
	FIREWALL_RAW_OUTPUT,

	FIREWALL_TYPE_MAX,
}firewall_type_t;

typedef struct firewall_port_s
{
	zpl_uint16				port[FIREWALL_DEFAULT_PORT_MAX];
	ifindex_t			ifindex;
}firewall_port_t;

typedef struct firewall_s
{
	NODE					node;
	zpl_uint32				ID;
	zpl_uint8				family;
	zpl_int8 				name[FIREWALL_NAME_MAX];
	firewall_class_t  class;
	firewall_type_t	type;
	firewall_action_t action;

	firewall_proto_t	proto;

	struct prefix   	source;
	struct prefix   	destination;
	zpl_uint16				s_port;
	zpl_uint16				d_port;

	ifindex_t			s_ifindex;
	ifindex_t			d_ifindex;

	zpl_uint8 				s_mac[NSM_MAC_MAX];
	zpl_uint8 				d_mac[NSM_MAC_MAX];

}firewall_t;

typedef struct firewall_zone_s
{
	NODE					node;
	zpl_int8 	zonename[FIREWALL_NAME_MAX];
	LIST	*zone_list;
	void	*mutex;

}firewall_zone_t;

typedef struct Gfirewall_s
{
	LIST	*firewall_list;
	void	*mutex;
	zpl_bool	init;

		//默认基础参数
	firewall_port_t all_port;	//开放的TCP/UDP目的端口
	firewall_port_t tcp_port;	//开放的TCP目的端口
	firewall_port_t udp_port;	//开放的UDP目的端口

	zpl_uint32		rule_id[FIREWALL_TYPE_MAX];
}Gfirewall_t;


extern const char * nsm_firewall_type_string(firewall_type_t type);
extern const char * nsm_firewall_action_string(firewall_action_t action);
extern const char * nsm_firewall_proto_string(firewall_proto_t type);

extern firewall_t * nsm_firewall_rule_lookup_api(firewall_zone_t *zone, firewall_t *value);
extern int nsm_firewall_rule_del_api(firewall_zone_t *zone, firewall_t *value);
extern int nsm_firewall_rule_add_api(firewall_zone_t *zone, firewall_t *value);

extern int nsm_firewall_rule_foreach_api(firewall_zone_t *zone, int(*cb)(firewall_t *, void *), void *);


extern int nsm_firewall_zone_del(zpl_int8 	*zonename);
extern firewall_zone_t * nsm_firewall_zone_lookup(zpl_int8 	*zonename);
extern firewall_zone_t * nsm_firewall_zone_add(zpl_int8 	*zonename);
extern int nsm_firewall_zone_foreach_api(int(*cb)(firewall_zone_t *, void *), void *p);

extern int nsm_firewall_init(void);
extern int nsm_firewall_exit(void);


#ifdef ZPL_SHELL_MODULE
extern int nsm_firewall_rule_show_api(struct vty *vty, firewall_zone_t *zone, zpl_char * intype);
extern void cmd_firewall_init (void);
#endif

extern int nsm_halpal_firewall_portmap_rule_set(firewall_t *rule, zpl_action action);
extern int nsm_halpal_firewall_port_filter_rule_set(firewall_t *rule, zpl_action action);

extern int nsm_halpal_firewall_mangle_rule_set(firewall_t *rule, zpl_action action);
extern int nsm_halpal_firewall_raw_rule_set(firewall_t *rule, zpl_action action);


extern int nsm_halpal_firewall_snat_rule_set(firewall_t *rule, zpl_action action);
extern int nsm_halpal_firewall_dnat_rule_set(firewall_t *rule, zpl_action action);

 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_FIREWALLD_H__ */
