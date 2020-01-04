/*
 * kernel_firewalld.c
 *
 *  Created on: 2019年8月31日
 *      Author: zhurish
 */

#include <zebra.h>
#include <net/if_arp.h>
#include "linklist.h"
#include "if.h"
#include "prefix.h"
#include "log.h"
#include "rib.h"
#include "interface.h"

#include "nsm_veth.h"
#include "nsm_tunnel.h"
#include "nsm_bridge.h"
#include "nsm_firewalld.h"

//#include "pal_interface.h"
#include "kernel_ioctl.h"


/*
iptables -L -n
iptables -t nat -L
iptables -t filter -L -n -v --line-numbers
*/

/*
 * 端口映射
 */
int pal_firewall_portmap_rule_set(firewall_t *rule, int action)
{
	char cmd[512];
	char proto[16];
	memset(cmd, 0, sizeof(cmd));
	//sudo iptables -t nat -A PREROUTING -d 192.168.10.88 -p tcp --dport 80 -j
	// DNAT --to-destination 192.168.10.88:8080
	//把到80端口的服务请求都转到8080端口或者其他端口上
	//iptables -t nat -A PREROUTING -d 192.168.88.134 -p tcp --dport 80 -j DNAT --to 192.168.88.134:8080
	if (rule && rule->class == FIREWALL_C_PORT)
	{
		sprintf(cmd, "iptables -t nat -%s %s %d ", action ? "I":"D", firewall_type_string(rule->type), rule->ID);

		if(action == 0)
		{
			printf("---%s---:%s\r\n", __func__, cmd);
			return super_system(cmd);
		}
		if (rule->source.family)
		{
			char tmp[64];
			union prefix46constptr pa;
			pa.p = &rule->source;
			strcat(cmd, " –d ");
			if(rule->source.prefixlen == 0 ||
					rule->source.prefixlen == IPV4_MAX_PREFIXLEN ||
					rule->source.prefixlen == IPV6_MAX_PREFIXLEN)
				strcat(cmd, prefix_2_address_str(pa, tmp, sizeof(tmp)));
			else
				strcat(cmd, prefix2str(pa, tmp, sizeof(tmp)));
		}

		if (rule->proto == FIREWALL_P_TCP || rule->proto == FIREWALL_P_UDP
				|| rule->proto == FIREWALL_P_ICMP)
		{
			memset(proto, 0, sizeof(proto));
			strcat(cmd, " -p ");
			sprintf(proto, "%s", firewall_proto_string(rule->proto));
			strcat(cmd, strlwr(proto));
		}

		if (rule->d_port)
		{
			strcat(cmd, " --dport ");
			strcat(cmd, itoa(rule->d_port, 10));
		}

		strcat(cmd, " -j ");
		strcat(cmd, firewall_action_string(rule->action));

		if (rule->destination.family)
		{
			char tmp[64];
			union prefix46constptr pa;
			pa.p = &rule->destination;
			strcat(cmd, " --to ");
			if(rule->destination.prefixlen == 0 ||
					rule->destination.prefixlen == IPV4_MAX_PREFIXLEN ||
					rule->destination.prefixlen == IPV6_MAX_PREFIXLEN)
				strcat(cmd, prefix_2_address_str(pa, tmp, sizeof(tmp)));
			else
				strcat(cmd, prefix2str(pa, tmp, sizeof(tmp)));
			//strcat(cmd, prefix_2_address_str(pa, tmp, sizeof(tmp)));

			if (rule->s_port)
			{
				strcat(cmd, ":");
				strcat(cmd, itoa(rule->s_port, 10));
			}
		}

		printf("---%s---:%s\r\n", __func__, cmd);

		return super_system(cmd);
	}
	return ERROR;
}

/*
 * 端口开放
 */
int pal_firewall_port_filter_rule_set(firewall_t *rule, int action)
{
	char cmd[512];
	char proto[16];
	memset(cmd, 0, sizeof(cmd));
	//iptables -A INPUT -p tcp --dport 22 -j ACCEPT
	//iptables -A OUTPUT -p tcp --sport 22 -j ACCEPT
	if (rule && rule->class == FIREWALL_C_FILTER)
	{

		sprintf(cmd, "iptables -t filter -%s %s %d ", action ? "I":"D", firewall_type_string(rule->type), rule->ID);
		if(action == 0)
		{
			printf("---%s---:%s\r\n", __func__, cmd);
			return super_system(cmd);
		}
		if (rule->proto == FIREWALL_P_TCP || rule->proto == FIREWALL_P_UDP
				|| rule->proto == FIREWALL_P_ICMP)
		{
			memset(proto, 0, sizeof(proto));
			strcat(cmd, " -p ");
			sprintf(proto, "%s", firewall_proto_string(rule->proto));
			strcat(cmd, strlwr(proto));
		}

		if(rule->type == FIREWALL_FILTER_INPUT)
		{
			if (rule->source.family)
			{
				char tmp[64];
				union prefix46constptr pa;
				pa.p = &rule->source;
				strcat(cmd, " -s ");
				if(rule->source.prefixlen == 0 ||
						rule->source.prefixlen == IPV4_MAX_PREFIXLEN ||
						rule->source.prefixlen == IPV6_MAX_PREFIXLEN)
					strcat(cmd, prefix_2_address_str(pa, tmp, sizeof(tmp)));
				else
					strcat(cmd, prefix2str(pa, tmp, sizeof(tmp)));
			}
		}

		if(rule->type == FIREWALL_FILTER_OUTPUT)
		{
			if (rule->destination.family)
			{
				char tmp[64];
				union prefix46constptr pa;
				pa.p = &rule->destination;
				strcat(cmd, " -d ");
				if(rule->source.prefixlen == 0 ||
						rule->source.prefixlen == IPV4_MAX_PREFIXLEN ||
						rule->source.prefixlen == IPV6_MAX_PREFIXLEN)
					strcat(cmd, prefix_2_address_str(pa, tmp, sizeof(tmp)));
				else
					strcat(cmd, prefix2str(pa, tmp, sizeof(tmp)));
			}
		}

		if(rule->type == FIREWALL_FILTER_INPUT)
		{
			if (rule->d_port)
			{
				strcat(cmd, " --dport ");
				strcat(cmd, itoa(rule->d_port, 10));
			}

			if (rule->s_ifindex)
			{
				strcat(cmd, " –i ");
				strcat(cmd, ifindex2ifname(rule->s_ifindex));
			}

			if(!str_isempty(rule->s_mac, sizeof(rule->s_mac)))
			{
				strcat(cmd, " -m mac --mac-source ");
				strcat(cmd, inet_ethernet(rule->s_mac));
			}
		}
		else if(rule->type == FIREWALL_FILTER_OUTPUT)
		{
			if (rule->s_port)
			{
				strcat(cmd, " --sport ");
				strcat(cmd, itoa(rule->s_port, 10));
			}

			if (rule->d_ifindex)
			{
				strcat(cmd, " –o ");
				strcat(cmd, ifindex2ifname(rule->d_ifindex));
			}

/*			if(!str_isempty(rule->d_mac, sizeof(rule->d_mac)))
			{
				strcat(cmd, " -m mac --mac-source ");
				strcat(cmd, inet_ethernet(rule->d_mac));
			}*/
		}


		strcat(cmd, " -j ");
		strcat(cmd, firewall_action_string(rule->action));

		printf("---%s---:%s\r\n", __func__, cmd);

		return super_system(cmd);
	}
	return ERROR;
}



int pal_firewall_mangle_rule_set(firewall_t *rule, int action)
{
	if (rule && rule->class == FIREWALL_C_MANGLE)
		return OK;
	return ERROR;
}

int pal_firewall_raw_rule_set(firewall_t *rule, int action)
{
	if (rule && rule->class == FIREWALL_C_RAW)
		return OK;
	return ERROR;
}
/*
由上图可知，需要将192.168.10.10转换为111.196.211.212，iptables命令如下：
iptables –t nat –A POSTROUTING –s 192.168.10.10 –o eth1 –j SNAT --to-source 111.196.221.212
外网IP地址不稳定的情况即可使用MASQUERADE(动态伪装),能够自动的寻找外网地址并改为当前正确的外网IP地址
iptables -t nat -A POSTROUTING -s 192.168.10.0/24 -j MASQUERADE

目标地址192.168.10.6在路由前就转换成61.240.149.149，需在网关上运行iptables命令如下：
iptables –t nat –A PREROUTING –i eth1 –d 61.240.149.149 –p tcp –dport 80 –j DNAT --to-destination 192.168.10.6:80
eth1网口传入，且想要使用 port 80 的服务时，将该封包重新传导到 192.168.1.210:80 的 IP 及 port 上面,可以同时修改 IP 与 port。此为地址映射与端口转换

还可以使用REDIRECT单独进行端口转换
例：将 80 端口的封包转递到 8080端口
iptables -t nat -A PREROUTING -p tcp --dport 80 -j REDIRECT --to-ports 8080
# 使用 8080 这个 port 来启动 WWW ，但是别人都以80来联机



总结：配置映射方法
   01. 指定哪些网段需要进行映射     -s 172.16.1.0/24
   02. 指定在哪做映射               -o eth0
   03. 用什么方法做映射             -j SNAT/DNAT
   04. 映射成什么地址               --to-source  ip地址/--to-destination ip地址
2.6.2 iptables实现外网IP的端口映射到内网IP的端口
   需求：将网关的IP和9000端口映射到内网服务器的22端口
   端口映射 10.0.0.5:9000 -->172.16.1.7:22
实现命令：
iptables -t nat -A PREROUTING -d 10.0.0.5  -i eth0 -p tcp --dport 9000 -j DNAT --to-destination 172.16.1.7:22
参数说明：
-d 10.0.0.5  目标地址
-j DNAT   目的地址改写
2.6.3 IP一对一映射
需求：将IP地址172.16.1.51 映射到 10.0.0.81
通过辅助IP配置：
ip addr add 10.0.0.81/24 dev eth0 label eth0:0   #<==辅助IP
iptables  -t nat -I PREROUTING -d 10.0.0.81 -j DNAT --to-destination 172.16.1.51
iptables  -t nat -I POSTROUTING -s 172.16.1.51 -o eth0 -j SNAT --to-source 10.0.0.81
适合内网的机器访问NAT外网的IP
iptables  -t nat -I POSTROUTING -s 172.16.1.0/255.255.240.0 -d 10.0.0.81 -j SNAT --to-source 172.16.1.8
2.6.4 映射多个外网IP上网
方法1：
iptables -t nat -A POSTROUTING -s 10.0.1.0/255.255.240.0 -o eth0 -j SNAT --to-source 124.42.60.11-124.42.60.16
三层交换机或路由器，划分VLAN。
方法2：
iptables -t nat -A POSTROUTING -s 10.0.1.0/22 -o eth0 -j SNAT --to-source 124.42.60.11
iptables -t nat -A POSTROUTING -s 10.0.2.0/22 -o eth0 -j SNAT --to-source 124.42.60.12
扩大子网，增加广播风暴。
2.7 系统防火墙与网络内核优化标准参数
*/

int pal_firewall_snat_rule_set(firewall_t *rule, int action)
{
	/*
	需要将192.168.10.10转换为111.196.211.212，iptables命令如下：
	iptables –t nat –A POSTROUTING –s 192.168.10.10 –o eth1 –j SNAT --to-source 111.196.221.212
	外网IP地址不稳定的情况即可使用MASQUERADE(动态伪装),能够自动的寻找外网地址并改为当前正确的外网IP地址
	iptables -t nat -A POSTROUTING -s 192.168.10.0/24 -j MASQUERADE
	*/
	char cmd[512];
	//char proto[16];
	memset(cmd, 0, sizeof(cmd));
	if (rule && rule->class == FIREWALL_C_SNAT)
	{
		sprintf(cmd, "iptables -t nat -%s %s %d ", action ? "I":"D", firewall_type_string(rule->type), rule->ID);
		if(action == 0)
		{
			printf("---%s---:%s\r\n", __func__, cmd);
			return super_system(cmd);
		}
/*
		if (rule->proto == FIREWALL_P_TCP || rule->proto == FIREWALL_P_UDP
				|| rule->proto == FIREWALL_P_ICMP)
		{
			memset(proto, 0, sizeof(proto));
			strcat(cmd, " -p ");
			sprintf(proto, "%s", firewall_proto_string(rule->proto));
			strcat(cmd, strlwr(proto));
		}
*/
		if (rule->source.family)
		{
			char tmp[64];
			union prefix46constptr pa;
			pa.p = &rule->source;
			if(rule->action == FIREWALL_A_SNAT)
				strcat(cmd, " -s ");
			else if(rule->action == FIREWALL_A_DNAT)
				strcat(cmd, " -d ");
			else
				strcat(cmd, " -d ");
			if(rule->source.prefixlen == 0 ||
					rule->source.prefixlen == IPV4_MAX_PREFIXLEN ||
					rule->source.prefixlen == IPV6_MAX_PREFIXLEN)
				strcat(cmd, prefix_2_address_str(pa, tmp, sizeof(tmp)));
			else
				strcat(cmd, prefix2str(pa, tmp, sizeof(tmp)));
		}

/*		if (rule->s_port)
		{
			if(rule->action == FIREWALL_A_SNAT)
				strcat(cmd, " –sport ");
			else if(rule->action == FIREWALL_A_DNAT)
				strcat(cmd, " –dport ");
			else
				strcat(cmd, " –dport ");
			strcat(cmd, itoa(rule->s_port, 10));
		}*/

		if (rule->d_ifindex)
		{
			if(rule->action == FIREWALL_A_SNAT)
				strcat(cmd, " –o ");
			else if(rule->action == FIREWALL_A_DNAT)
				strcat(cmd, " –i ");
			else
				strcat(cmd, " –i ");
			strcat(cmd, ifindex2ifname(rule->d_ifindex));
		}
		if (rule->destination.family)
		{
			strcat(cmd, " -j ");
			strcat(cmd, firewall_action_string(rule->action));

			char tmp[64];
			union prefix46constptr pa;
			pa.p = &rule->destination;
			if(rule->action == FIREWALL_A_SNAT)
				strcat(cmd, " --to-source ");
			else if(rule->action == FIREWALL_A_DNAT)
				strcat(cmd, " --to-destination ");
			else
				strcat(cmd, " --to-destination ");
			strcat(cmd, prefix_2_address_str(pa, tmp, sizeof(tmp)));
		}
		else
		{
			strcat(cmd, " -j MASQUERADE");
		}
/*
		if (rule->d_port)
		{
			if(rule->action == FIREWALL_A_SNAT)
				strcat(cmd, " --to-ports ");
			else if(rule->action == FIREWALL_A_DNAT)
				strcat(cmd, " --to-ports ");
			else
				strcat(cmd, " --to-ports ");
			strcat(cmd, itoa(rule->d_port, 10));
		}

		if (rule->d_ifindex)
		{
			strcat(cmd, " –o ");
			strcat(cmd, ifindex2ifname(rule->d_ifindex));
		}
*/
		printf("---%s---:%s\r\n", __func__, cmd);
		return  super_system(cmd);
	}
	return ERROR;
}


int pal_firewall_dnat_rule_set(firewall_t *rule, int action)
{
	/*
目标地址192.168.10.6在路由前就转换成61.240.149.149，需在网关上运行iptables命令如下：
iptables –t nat –A PREROUTING –i eth1 –d 61.240.149.149 –p tcp –dport 80 –j DNAT --to-destination 192.168.10.6:8
eth1网口传入，且想要使用 port 80 的服务时，将该封包重新传导到 192.168.1.210:80 的 IP 及 port 上面,可以同时修改 IP 与 port。此为地址映射与端口转换
	*/
	char cmd[512];
	char proto[16];
	memset(cmd, 0, sizeof(cmd));
	if (rule && rule->class == FIREWALL_C_DNAT)
	{
		sprintf(cmd, "iptables -t nat -%s %s %d ", action ? "I":"D", firewall_type_string(rule->type), rule->ID);
		if(action == 0)
		{
			printf("---%s---:%s\r\n", __func__, cmd);
			return super_system(cmd);
		}
		if (rule->s_ifindex)
		{
			strcat(cmd, " –i ");
			strcat(cmd, ifindex2ifname(rule->s_ifindex));
		}

		if (rule->source.family)
		{
			char tmp[64];
			union prefix46constptr pa;
			pa.p = &rule->source;
			if(rule->action == FIREWALL_A_SNAT)
				strcat(cmd, " -s ");
			else if(rule->action == FIREWALL_A_DNAT)
				strcat(cmd, " -d ");
			else
				strcat(cmd, " -d ");
			if(rule->source.prefixlen == 0 ||
					rule->source.prefixlen == IPV4_MAX_PREFIXLEN ||
					rule->source.prefixlen == IPV6_MAX_PREFIXLEN)
				strcat(cmd, prefix_2_address_str(pa, tmp, sizeof(tmp)));
			else
				strcat(cmd, prefix2str(pa, tmp, sizeof(tmp)));
		}

		if (rule->proto == FIREWALL_P_TCP || rule->proto == FIREWALL_P_UDP
				|| rule->proto == FIREWALL_P_ICMP)
		{
			memset(proto, 0, sizeof(proto));
			strcat(cmd, " -p ");
			sprintf(proto, "%s", firewall_proto_string(rule->proto));
			strcat(cmd, strlwr(proto));
		}

		if (rule->s_port)
		{
			if(rule->action == FIREWALL_A_SNAT)
				strcat(cmd, " –sport ");
			else if(rule->action == FIREWALL_A_DNAT)
				strcat(cmd, " –dport ");
			else
				strcat(cmd, " –dport ");
			strcat(cmd, itoa(rule->s_port, 10));
		}

		strcat(cmd, " -j ");
		strcat(cmd, firewall_action_string(rule->action));

		if (rule->d_ifindex)
		{
			if(rule->action == FIREWALL_A_SNAT)
				strcat(cmd, " –i ");
			else if(rule->action == FIREWALL_A_DNAT)
				strcat(cmd, " –o ");
			else
				strcat(cmd, " –o ");
			strcat(cmd, ifindex2ifname(rule->d_ifindex));
		}
		if (rule->destination.family)
		{
			char tmp[64];
			union prefix46constptr pa;
			pa.p = &rule->destination;
			if(rule->action == FIREWALL_A_SNAT)
				strcat(cmd, " --to-source ");
			else if(rule->action == FIREWALL_A_DNAT)
				strcat(cmd, " --to-destination ");
			else
				strcat(cmd, " --to-destination ");
			strcat(cmd, prefix_2_address_str(pa, tmp, sizeof(tmp)));
		}

		if (rule->d_port)
		{
			if(rule->action == FIREWALL_A_DNAT)
			{
				strcat(cmd, ":");
				strcat(cmd, itoa(rule->d_port, 10));
			}
		}
		printf("---%s---:%s\r\n", __func__, cmd);
		return super_system(cmd);
	}
	return ERROR;
}

#if 0

2,添加规则
添加基本的NAT地址转换,(关于如何配置NAT可以看我的另一篇文章),
添加规则,我们只添加DROP链.因为默认链全是ACCEPT.
防止外网用内网IP欺骗
[root@tp sysconfig]# iptables -t nat -A PREROUTING -i eth0 -s 10.0.0.0/8 -j DROP
[root@tp sysconfig]# iptables -t nat -A PREROUTING -i eth0 -s 172.16.0.0/12 -j DROP
[root@tp sysconfig]# iptables -t nat -A PREROUTING -i eth0 -s 192.168.0.0/16 -j DROP
 如果我们想,比如阻止MSN,QQ,BT等的话,需要找到它们所用的端口或者IP,(个人认为没有太大必要)
例：
禁止与211.101.46.253的所有连接
 [root@tp ~]# iptables -t nat -A PREROUTING  -d 211.101.46.253 -j DROP
禁用FTP(21)端口
[root@tp ~]# iptables -t nat -A PREROUTING -p tcp --dport 21 -j DROP
这样写范围太大了,我们可以更精确的定义.
[root@tp ~]# iptables -t nat -A PREROUTING  -p tcp --dport 21 -d 211.101.46.253 -j DROP
这样只禁用211.101.46.253地址的FTP连接,其他连接还可以.如web(80端口)连接.
按照我写的,你只要找到QQ,MSN等其他软件的IP地址,和端口,以及基于什么协议,只要照着写就行了.

一、查看规则集
iptables –list -n // 加一个-n以数字形式显示IP和端口，看起来更舒服

二、配置默认规则
iptables -P INPUT DROP  // 不允许进
iptables -P FORWARD DROP  // 不允许转发
iptables -P OUTPUT ACCEPT  // 允许出

三、增加规则
iptables -A INPUT -s 192.168.0.0/24 -j ACCEPT
//允许源IP地址为192.168.0.0/24网段的包流进（包括所有的协议，这里也可以指定单个IP）
iptables -A INPUT -d 192.168.0.22 -j ACCEPT
//允许所有的IP到192.168.0.22的访问
iptables -A INPUT -p tcp –dport 80 -j ACCEPT
//开放本机80端口
iptables -A INPUT -p icmp –icmp-type echo-request -j ACCEPT
//开放本机的ICMP协议

四、删除规则
iptables -D INPUT -s 192.168.0.21 -j ACCEPT


开放指定的端口

iptables -A INPUT -s 127.0.0.1 -d 127.0.0.1 -j ACCEPT               #允许本地回环接口(即运行本机访问本机)
iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT    #允许已建立的或相关连的通行
iptables -A OUTPUT -j ACCEPT         #允许所有本机向外的访问
iptables -A INPUT -p tcp --dport 22 -j ACCEPT    #允许访问22端口
iptables -A INPUT -p tcp --dport 80 -j ACCEPT    #允许访问80端口
iptables -A INPUT -p tcp --dport 21 -j ACCEPT    #允许ftp服务的21端口
iptables -A INPUT -p tcp --dport 20 -j ACCEPT    #允许FTP服务的20端口
iptables -A INPUT -j reject       #禁止其他未允许的规则访问
iptables -A FORWARD -j REJECT     #禁止其他未允许的规则访问
屏蔽IP

iptables -I INPUT -s 123.45.6.7 -j DROP       #屏蔽单个IP的命令
iptables -I INPUT -s 123.0.0.0/8 -j DROP      #封整个段即从123.0.0.1到123.255.255.254的命令
iptables -I INPUT -s 124.45.0.0/16 -j DROP    #封IP段即从123.45.0.1到123.45.255.254的命令
iptables -I INPUT -s 123.45.6.0/24 -j DROP    #封IP段即从123.45.6.1到123.45.6.254的命令是


2.HTTP

HTTP的配置与SSH类似：

# 1.允许接收远程主机的HTTP请求
iptables -A INPUT -i eth0 -p tcp –dport 80 -m state –state NEW,ESTABLISHED -j ACCEPT

# 1.允许发送本地主机的HTTP响应
iptables -A OUTPUT -o eth0 -p tcp –sport 80 -m state –state ESTABLISHED -j ACCEPT

# 1.送出的数据包目的端口为22
iptables -A OUTPUT -o eth0 -p tcp –dport 22 -m state –state NEW,ESTABLISHED -j ACCEPT

# 2.接收的数据包源端口为22
iptables -A INPUT -i eth0 -p tcp –sport 22 -m state –state ESTABLISHED -j ACCEPT

1.SSH

# 1.允许接收远程主机的SSH请求
iptables -A INPUT -i eth0 -p tcp –dport 22 -m state –state NEW,ESTABLISHED -j ACCEPT

# 2.允许发送本地主机的SSH响应
iptables -A OUTPUT -o eth0 -p tcp –sport 22 -m state –state ESTABLISHED -j ACCEPT


# 2.接收目标端口为22的数据包
iptables -A INPUT -i eth0 -p tcp –dport 22 -j ACCEPT
# 3.拒绝所有其他数据包
iptables -A INPUT -j DROP

iptables –flush
或者
iptables -F
iptables -t NAT -F

a) 1. 删除现有规则

iptables -F

b) 2. 配置默认链策略

iptables -P INPUT DROP

iptables -P FORWARD DROP

iptables -P OUTPUT DROP

c) 3. 允许远程主机进行SSH连接

iptables -A INPUT -i eth0 -p tcp –dport 22 -m state –state NEW,ESTABLISHED -j ACCEPT

iptables -A OUTPUT -o eth0 -p tcp –sport 22 -m state –state ESTABLISHED -j ACCEPT

d) 4. 允许本地主机进行SSH连接

iptables -A OUTPUT -o eth0 -p tcp –dport 22 -m state –state NEW,ESTABLISHED -j ACCEPT

iptables -A INPUT -i eth0 -p tcp –sport 22 -m state –state ESTABLISHED -j ACCEPT

e) 5. 允许HTTP请求

iptables -A INPUT -i eth0 -p tcp –dport 80 -m state –state NEW,ESTABLISHED -j ACCEPT

iptables -A OUTPUT -o eth0 -p tcp –sport 80 -m state –state ESTABLISHED -j ACCEPT


4.限制ping 192.168.0.1主机的数据包数，平均2/s个，最多不能超过3个
iptables -A INPUT -i ens33 -d 192.168.0.1 -p icmp --icmp-type 8 -m limit --limit 2/second --limit-burst 3 -j ACCEPT


放行本机端的流入流出
 iptables -A  INPUT  -s 127.0.0.1 -d 127.0.0.1 -i lo -j ACCEPT

  iptables -A  OUTPUT  -s 127.0.0.1 -d 127.0.0.1 -o lo -j ACCEPT


2.放行httpd/nginx服务
 iptables -I OUTPUT -s 192.168.0.1 -p tcp --sport 80 -j ACCEPT

 iptables -I INPUT -d 192.168.0.1 -p tcp --dport 80 -j ACCEPT

放行sshd服务
 iptables -t filter -A INPUT -s 192.168.0.0/24 -d 192.168.0.1 -p tcp --dport 22 -j ACCEPT

 iptables -t filter -A OUTPUT -s 192.168.0.1  -p tcp --sport 22 -j ACCEPT

multiport(多端口)
        iptables -I INPUT -d 172.16.100.7 -p tcp -m multiport --dports 22,80 -j ACCEPT

        iptables -I OUTPUT -s 172.16.100.7 -p tcp -m multiport --sports 22,80 -j ACCEPT
iprange（ip范围）
		iptables -A INPUT -d 172.16.100.7 -p tcp --dport 23 -m iprange --src-range 172.16.100.1-172.16.100.100 -j ACCEPT

		    iptables -A OUTPUT -s 172.16.100.7 -p tcp --sport 23 -m iprange --dst-range 172.16.100.1-172.16.100.100 -j ACCEPT


time（时间范围）
			iptables -A INPUT -d 172.16.100.7 -p tcp --dport 901 -m time --weekdays Mon,Tus,Wed,Thu,Fri --timestart 08:00:00 --time-stop 18:00:00 -j ACCEPT

			        iptables -A OUTPUT -s 172.16.100.7 -p tcp --sport 901 -j ACCEPT
#endif
