/*
 * ip_brigde.c
 *
 *  Created on: Jul 14, 2018
 *      Author: zhurish
 */

/*
 * ip_brctl.c
 *
 *  Created on: Oct 16, 2016
 *      Author: zhurish
 */


#include <zebra.h>

#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "log.h"
#include "zclient.h"
#include "thread.h"

#include "nsm_veth.h"
#include "nsm_tunnel.h"
#include "nsm_bridge.h"

#include "kernel_ioctl.h"

#define _LINUX_IP_H

#include "linux/if_tunnel.h"
#include "linux/sockios.h"
#include "linux/if_bridge.h"

#ifndef HAVE_UTILS_BRCTL


int _ipkernel_bridge_create(nsm_bridge_t *br)
{
	int ret;
#ifdef SIOCBRADDBR
	ret = if_ioctl(SIOCBRADDBR, br->ifp->k_name);
	if (ret < 0)
#endif
	{
		char _br[IFNAMSIZ];
		unsigned long arg[3] = { BRCTL_ADD_BRIDGE, (unsigned long) _br };
		strncpy(_br, br->ifp->k_name, IFNAMSIZ);
		ret = if_ioctl(SIOCSIFBR, (caddr_t)&arg);
	}
	return ret < 0 ? errno : 0;
}

int _ipkernel_bridge_delete(nsm_bridge_t *br)
{
	int ret = -1;
#ifdef SIOCBRDELBR
	ret = if_ioctl(SIOCBRDELBR, br->ifp->k_name);
	if (ret < 0)
#endif
	{
		char _br[IFNAMSIZ];
		unsigned long arg[3] = { BRCTL_DEL_BRIDGE, (unsigned long) _br };
		strncpy(_br, br->ifp->k_name, IFNAMSIZ);
		ret = if_ioctl(SIOCSIFBR, arg);
	}
	return  ret < 0 ? errno : 0;
}

int _ipkernel_bridge_add_interface(nsm_bridge_t *br, int ifindex)
{
	struct ifreq ifr;
	int err = -1;
	strncpy(ifr.ifr_name, br->ifp->k_name, IFNAMSIZ);
#ifdef SIOCBRADDIF
	ifr.ifr_ifindex = ifindex;
	err = if_ioctl(SIOCBRADDIF, &ifr);
	if (err < 0)
#endif
	{
		unsigned long args[4] = { BRCTL_ADD_IF, ifindex, 0, 0 };
		ifr.ifr_data = (char *) args;
		err = if_ioctl(SIOCDEVPRIVATE, &ifr);
	}
	return err < 0 ? errno : 0;
}

int _ipkernel_bridge_del_interface(nsm_bridge_t *br, int ifindex)
{
	struct ifreq ifr;
	int err = -1;
	strncpy(ifr.ifr_name, br->ifp->k_name, IFNAMSIZ);
#ifdef SIOCBRDELIF
	ifr.ifr_ifindex = ifindex;
	err = if_ioctl(SIOCBRDELIF, &ifr);
	if (err < 0)
#endif
	{
		unsigned long args[4] = { BRCTL_DEL_IF, ifindex, 0, 0 };
		ifr.ifr_data = (char *) args;
		err = if_ioctl(SIOCDEVPRIVATE, &ifr);
	}
	return err < 0 ? errno : 0;
}

#else

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
	int type;//???????????????tunnel???vlan???bridge
	char name[INTERFACE_NAMSIZ + 1];
#ifdef HAVE_UTILS_BRCTL
	int br_mode;//????????????????????????????????????
	char br_name[INTERFACE_NAMSIZ + 1];//???????????????????????????
	int br_stp;//???????????????
	int br_stp_state;//???????????????
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
	int tun_index;//tunnel???????????????ID??????
	int tun_mode;//????????????
	int tun_ttl;//change: ip tunnel change tunnel0 ttl
	int tun_mtu;//change: ip link set dev tunnel0 mtu 1400
    struct in_addr source;//ip tunnel change tunnel1 local 192.168.122.1
    struct in_addr remote;//change: ip tunnel change tunnel1 remote 19.1.1.1
    int active;//????????????????????????
#define TUNNEL_ACTIVE 1
#endif
#ifdef HAVE_UTILS_VLAN
#define SKB_QOS_MAX	8
	int vlanid;//vlan id
	int egress_vlan_qos[SKB_QOS_MAX];//??????skb???????????????qos?????????
	int ingress_vlan_qos[SKB_QOS_MAX];//??????skb???????????????qos?????????
	//unsigned int flag; /* Matches vlan_dev_priv flags */
#endif
};
//#define BRCTL_CMD_DEBUG

#ifndef BRCTL_CMD_DEBUG
#include "linux/if_bridge.h"

static int br_sock_fd = -1;

static int bridge_sock_init(void)
{
	if ((br_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return errno;
	return br_sock_fd;
}

static int bridge_create(const char *brname)
{
	int ret;
#ifdef SIOCBRADDBR
	ret = ioctl(br_sock_fd, SIOCBRADDBR, brname);
	if (ret < 0)
#endif
	{
		char _br[IFNAMSIZ];
		unsigned long arg[3] = { BRCTL_ADD_BRIDGE, (unsigned long) _br };
		strncpy(_br, brname, IFNAMSIZ);
		ret = ioctl(br_sock_fd, SIOCSIFBR, arg);
	}
	return ret < 0 ? errno : 0;
}
static int bridge_delete(const char *brname)
{
	int ret;

#ifdef SIOCBRDELBR
	ret = ioctl(br_sock_fd, SIOCBRDELBR, brname);
	if (ret < 0)
#endif
	{
		char _br[IFNAMSIZ];
		unsigned long arg[3] = { BRCTL_DEL_BRIDGE, (unsigned long) _br };
		strncpy(_br, brname, IFNAMSIZ);
		ret = ioctl(br_sock_fd, SIOCSIFBR, arg);
	}
	return  ret < 0 ? errno : 0;
}
static int bridge_updown(struct utils_interface *ifp,  int value)
{
	return 0;//utils_interface_status (ifp,  value);
}

static int bridge_add_interface(const char *bridge, const char *dev)
{
	struct ifreq ifr;
	int err;
	int ifindex = if_nametoindex(dev);
	if (ifindex == 0)
		return ENODEV;
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
#ifdef SIOCBRADDIF
	ifr.ifr_ifindex = ifindex;
	err = ioctl(br_sock_fd, SIOCBRADDIF, &ifr);
	if (err < 0)
#endif
	{
		unsigned long args[4] = { BRCTL_ADD_IF, ifindex, 0, 0 };
		ifr.ifr_data = (char *) args;
		err = ioctl(br_sock_fd, SIOCDEVPRIVATE, &ifr);
	}
	return err < 0 ? errno : 0;
}

static int bridge_del_interface(const char *bridge, const char *dev)
{
	struct ifreq ifr;
	int err;
	int ifindex = if_nametoindex(dev);
	if (ifindex == 0)
		return ENODEV;
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
#ifdef SIOCBRDELIF
	ifr.ifr_ifindex = ifindex;
	err = ioctl(br_sock_fd, SIOCBRDELIF, &ifr);
	if (err < 0)
#endif
	{
		unsigned long args[4] = { BRCTL_DEL_IF, ifindex, 0, 0 };
		ifr.ifr_data = (char *) args;
		err = ioctl(br_sock_fd, SIOCDEVPRIVATE, &ifr);
	}
	return err < 0 ? errno : 0;
}
static int bridge_option_set(const char *bridge, const char *name,
		  unsigned long value, unsigned long oldcode)
{
	int ret = -1;
	struct ifreq ifr;
	unsigned long args[4] = { oldcode, value, 0, 0 };
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
	ifr.ifr_data = (char *) &args;
	ret = ioctl(br_sock_fd, SIOCDEVPRIVATE, &ifr);
	return ret < 0 ? errno : 0;
}
static int bridge_cmd_error(struct vty *vty, int type, const char *bridge, const char *dev,int ret)
{
	switch(ret)
	{
	case EEXIST:
		if(type == BRCTL_ADD_BRIDGE)
		{
			if(vty)
				vty_out (vty, "%% bridge device %s already exists; can't create %s",bridge, VTY_NEWLINE);
			else
				zlog_debug(ZLOG_PAL, "%% bridge device %s already exists; can't create \n",bridge);
		}
		break;
	case ENXIO:
		if(type == BRCTL_DEL_BRIDGE)
		{
			if(vty)
				vty_out (vty, "%% bridge device %s doesn't exist; can't delete it.%s",bridge, VTY_NEWLINE);
			else
				zlog_debug(ZLOG_PAL, "%% bridge device %s doesn't exist; can't delete it.\n",bridge);
		}
		break;

	case EBUSY:
		if(type == BRCTL_DEL_BRIDGE)
		{
			if(vty)
				vty_out (vty, "%% bridge device %s is still up; can't delete it\%s",bridge, VTY_NEWLINE);
			else
				zlog_debug(ZLOG_PAL, "%% bridge device %s is still up; can't delete it\n",bridge);
		}
		if(type == BRCTL_ADD_IF)
		{
			if(vty)
				vty_out (vty, "%% interface %s is already to bridge %s %s",dev,bridge, VTY_NEWLINE);
			else
				zlog_debug(ZLOG_PAL, "%% interface %s is already to bridge %s \n",dev,bridge);
		}
		break;

	case ENODEV:
		if(type == BRCTL_ADD_IF)
		{
			if(vty)
				vty_out (vty, "%% interface %s does not exist. %s",dev, VTY_NEWLINE);
			else
				zlog_debug(ZLOG_PAL, "%% interface %s does not exist. \n",dev);
		}
		if(type == BRCTL_DEL_IF)
		{
			if(vty)
				vty_out (vty, "%% interface %s does not exist.%s",dev, VTY_NEWLINE);
			else
				zlog_debug(ZLOG_PAL, "%% interface %s does not exist. \n",dev);
		}
		break;

	case ELOOP:
		if(type == BRCTL_ADD_IF)
		{
			if(vty)
				vty_out (vty, "%% interface %s is a bridge device itself can't enslave a bridge device to a bridge device. %s",dev, VTY_NEWLINE);
			else
				zlog_debug(ZLOG_PAL, "%% interface %s is a bridge device itself can't enslave a bridge device to a bridge device.\n",dev);
		}
		break;

	case EINVAL:
		if(type == BRCTL_DEL_IF)
		{
			if(vty)
				vty_out (vty, "%% interface %s is not bridge on bridge device %s. %s",dev, bridge, VTY_NEWLINE);
			else
				zlog_debug(ZLOG_PAL, "%% interface %s is not bridge on bridge device %s.\n",dev, bridge);
		}
		break;
	}
	return CMD_WARNING;
}
static int ip_bridge_dev_cmd(struct vty *vty, int type, const char *bridge, const char *dev)
{
	int ret = -1;
	if(br_sock_fd == -1)
		bridge_sock_init();
	switch(type)
	{
	case BRCTL_ADD_BRIDGE:
		ret = bridge_create(bridge);
		break;
	case BRCTL_DEL_BRIDGE:
		ret = bridge_delete(bridge);
		break;
	case BRCTL_ADD_IF:
		ret = bridge_add_interface(bridge, dev);
		break;
	case BRCTL_DEL_IF:
		ret = bridge_del_interface(bridge, dev);
		break;
	}
	if(br_sock_fd)
		close(br_sock_fd);
	br_sock_fd = -1;
	if(ret < 0)
		return bridge_cmd_error(vty, type, bridge, dev, ret);
	else if(ret == CMD_WARNING)
		return CMD_WARNING;
	return CMD_SUCCESS;
}
static int ip_bridge_option_cmd(struct vty *vty, int type, const char *bridge, const char *dev, int value)
{
	int ret = 0;
	if(br_sock_fd == -1)
		bridge_sock_init();
	switch(type)
	{
	case BRCTL_SET_BRIDGE_STP_STATE:
		bridge_option_set(bridge, "stp_state", value, BRCTL_SET_BRIDGE_STP_STATE);
		break;
		/*
	case BRCTL_DEL_BRIDGE:
		ret = bridge_delete(bridge);
		break;
	case BRCTL_ADD_IF:
		ret = bridge_add_interface(bridge, dev);
		break;
	case BRCTL_DEL_IF:
		ret = bridge_del_interface(bridge, dev);
		break;
		*/
	}
	if(br_sock_fd)
		close(br_sock_fd);
	br_sock_fd = -1;
	if(ret)
		return bridge_cmd_error(vty, type, bridge, dev, ret);
	return CMD_SUCCESS;
}
static void ip_bridge_jiffies(struct vty *vty, const char *str, unsigned long long jiffies)
{
	unsigned long long tvusec;
	struct timeval tv;
	//tvusec = (1000000ULL*jiffies)/HZ;
	tvusec = (1000000ULL*jiffies)/100;
	tv.tv_sec = tvusec/1000000;
	tv.tv_usec = tvusec - 1000000 * tv.tv_sec;
	vty_out(vty,"  %s: %4i.%.2i", str, (int)tv.tv_sec, (int)tv.tv_usec/10000);
}
static int ip_bridge_port_get(struct utils_interface *uifp)
{
	struct ifreq ifr;
	int port_index[BR_PORT_MAX];

	unsigned long args[4] = { BRCTL_GET_PORT_LIST,(unsigned long) &port_index, 0, 0 };
	strncpy(ifr.ifr_name, uifp->name, IFNAMSIZ);
	ifr.ifr_data = (char *) &args;

	if (ioctl(br_sock_fd, SIOCDEVPRIVATE, &ifr) < 0) {
		zlog_err(ZLOG_PAL, "%s: can't get info %s\n",uifp->name, strerror(errno));
		return CMD_WARNING;
	}
	memcpy(uifp->br_ifindex, port_index, sizeof(port_index));
	return CMD_SUCCESS;
}
static int ip_bridge_port_info_get(struct utils_interface *uifp)
{
	struct ifreq ifr;
	struct __port_info port;

	unsigned long args[4] = { BRCTL_GET_PORT_INFO,(unsigned long) &port, 0, 0 };
	strncpy(ifr.ifr_name, uifp->name, IFNAMSIZ);
	ifr.ifr_data = (char *) &args;

	if (ioctl(br_sock_fd, SIOCDEVPRIVATE, &ifr) < 0) {
		zlog_err(ZLOG_PAL, "%s: can't get info %s\n",uifp->name, strerror(errno));
		return CMD_WARNING;
	}
	uifp->br_stp_state = port.state;
	//uifp->message_age_timer_value = port.message_age_timer_value;
	//uifp->forward_delay_timer_value = port.forward_delay_timer_value;
	//uifp->hold_timer_value = port.hold_timer_value;
	return CMD_SUCCESS;
}
int ip_bridge_startup_config(struct utils_interface *uifp)
{
	struct ifreq ifr;
	struct __bridge_info i;

	unsigned long args[4] = { BRCTL_GET_BRIDGE_INFO,(unsigned long) &i, 0, 0 };

	if(br_sock_fd == -1)
		bridge_sock_init();
	strncpy(ifr.ifr_name, uifp->name, IFNAMSIZ);
	ifr.ifr_data = (char *) &args;

	if (ioctl(br_sock_fd, SIOCDEVPRIVATE, &ifr) < 0) {
		zlog_err(ZLOG_PAL, "%s: can't get info %s\n",uifp->name, strerror(errno));
		if(br_sock_fd)
			close(br_sock_fd);
		br_sock_fd = -1;
		return CMD_WARNING;
	}
	//BRCTL_SET_BRIDGE_STP_STATE
	if(br_sock_fd)
		close(br_sock_fd);
	br_sock_fd = -1;
	uifp->type = UTILS_IF_BRIDGE;
	uifp->br_stp = i.stp_enabled;
	uifp->max_age = i.max_age;
	uifp->hello_time = i.hello_time;
	uifp->forward_delay = i.forward_delay;
	ip_bridge_port_get(uifp);
	/*
	memcpy(val, &i.bridge_id, 8);
	i.root_path_cost;
	i.root_port;
	i.topology_change;
	i.topology_change_detected;
	i.bridge_hello_time;
	i.bridge_forward_delay;
	i.bridge_max_age;
	i.hello_time;
	i.forward_delay;
	i.max_age;
	i.ageing_time;
	i.hello_timer_value;
	i.tcn_timer_value;
	i.topology_change_timer_value;
	i.gc_timer_value;
	*/
	return CMD_SUCCESS;
}
/*int ip_bridge_port_startup_config(struct utils_interface *uifp)
{
	int i = 0;
	struct interface *ifp = NULL;
	struct listnode *node =  NULL;
	struct utils_interface *bifp = NULL;
	if(uifp->type == UTILS_IF_BRIDGE || uifp->type == UTILS_IF_TUNNEL )
		return CMD_SUCCESS;
	for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
	{
		bifp = ifp->info;
		if(bifp && bifp->type == UTILS_IF_BRIDGE)
		{
			for(i = 0; i < BR_PORT_MAX; i++)
			{
				if(bifp->br_ifindex[i] && bifp->br_ifindex[i] == uifp->ifindex)
				{
					//uifp->type;
					uifp->br_mode = IF_MODE_BRIGDE;
					strcpy(uifp->br_name, ifp->name);
					ip_bridge_port_info_get(uifp);
					return CMD_SUCCESS;
				}
			}
		}
	}
	return CMD_SUCCESS;
}*/
static int ip_bridge_info(struct vty *vty, const char *bridge)
{
	int j = 0;
	struct ifreq ifr;
	struct __bridge_info i;
	unsigned char *val = NULL;
	//char val_str[64];
	unsigned long args[4] = { BRCTL_GET_BRIDGE_INFO,(unsigned long) &i, 0, 0 };
	if(br_sock_fd == -1)
		bridge_sock_init();
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
	ifr.ifr_data = (char *) &args;

	if (ioctl(br_sock_fd, SIOCDEVPRIVATE, &ifr) < 0) {
		zlog_err(ZLOG_PAL, "%s: can't get info %s\n",bridge, strerror(errno));
		if(br_sock_fd)
			close(br_sock_fd);
		br_sock_fd = -1;
		return errno;
	}
	if(br_sock_fd)
		close(br_sock_fd);
	br_sock_fd = -1;

	//vty_out(vty,"  bridge interface name: %s %s",bridge, VTY_NEWLINE);
	//vty_out(vty,"  bridge protocol stp: %s %s",i.stp_enabled? "on":"off", VTY_NEWLINE);

	val = (unsigned char *)&i.bridge_id;
	vty_out(vty,"  bridge id: ");
	for(j = 0; j < 8; j++)
	{
		vty_out(vty,"%02x",val[j]);
		if(j==1)
			vty_out(vty,".");
	}
	vty_out(vty,"%s", VTY_NEWLINE);
	vty_out(vty,"  root cost: %d root port:%d %s",
			i.root_path_cost, i.root_port,VTY_NEWLINE);
	vty_out(vty,"  topology change:%d topology change detected:%d %s",
			i.topology_change,i.topology_change_detected,VTY_NEWLINE);

	ip_bridge_jiffies(vty, "bridge hello time", i.bridge_hello_time);
	ip_bridge_jiffies(vty, "bridge forward delay", i.bridge_forward_delay);
	ip_bridge_jiffies(vty, "bridge max age", i.bridge_max_age);
	vty_out(vty,"%s", VTY_NEWLINE);

	ip_bridge_jiffies(vty, "hello time", i.hello_time);
	ip_bridge_jiffies(vty, "forward delay", i.forward_delay);
	ip_bridge_jiffies(vty, "max age", i.max_age);
	vty_out(vty,"%s", VTY_NEWLINE);

	ip_bridge_jiffies(vty, "ageing time", i.ageing_time);
	ip_bridge_jiffies(vty, "hello time value", i.hello_timer_value);
	ip_bridge_jiffies(vty, "tcn timer", i.tcn_timer_value);
	ip_bridge_jiffies(vty, "topology change timer", i.topology_change_timer_value);
	ip_bridge_jiffies(vty, "gc time", i.gc_timer_value);
	vty_out(vty,"%s", VTY_NEWLINE);
	return 0;
}
#endif
/***********************************************************************************/
static int ip_bridge_lookup(struct interface *bifp)
{
	  return 0;
}

int no_bridge_interface(struct vty *vty, const char *ifname)
{
#ifdef BRCTL_CMD_DEBUG
	char cmd[512];
#endif
	struct interface *ifp = NULL;
	struct utils_interface *bifp = NULL;
	if(ifname == NULL)
	{
		if(vty)
	      vty_out (vty, "%% invalid input bridge interface name is null %s",VTY_NEWLINE);
		else
			zlog_debug(ZLOG_PAL, "%% invalid input bridge interface name is null\n");
	    return CMD_WARNING;
	}
	//????????????
	ifp = if_lookup_by_name (ifname);
	if(!ifp)
	{
		if(vty)
			vty_out (vty, "%% Can't lookup bridge interface %s %s",ifname,VTY_NEWLINE);
		else
			zlog_debug(ZLOG_PAL, "%% Can't lookup bridge interface %s",ifname);
		return CMD_WARNING;
	}
	//????????????????????????????????????
	bifp = (struct utils_interface *)ifp->info;
	if(bifp->type != UTILS_IF_BRIDGE)
	{
		return CMD_SUCCESS;
		//vty_out (vty, "%% this is not bridge interface %s",VTY_NEWLINE);
		//return CMD_WARNING;
	}
	//????????????????????????????????????????????????
	if(ip_bridge_lookup(ifp))
	{
		if(vty)
			vty_out (vty, "%% Can't delete bridge interface %s there have sub interface %s",ifname,VTY_NEWLINE);
		else
			zlog_debug(ZLOG_PAL, "%% Can't delete bridge interface %s there have sub interface",ifname);
		return CMD_WARNING;
	}
#ifdef BRCTL_CMD_DEBUG
	sprintf(cmd, "ip link set dev %s down",ifname);
	super_system(cmd);
	sprintf(cmd, "brctl delbr %s",ifname);
	if(super_system(cmd)!=CMD_SUCCESS)
	{
		if(vty)
			vty_out (vty, "%% Can't delete bridge interface %s%s",ifname,VTY_NEWLINE);
		else
			zlog_debug(ZLOG_PAL, "%% Can't delete bridge interface %s",ifname);
		return CMD_WARNING;
	}
#else
	bridge_updown(bifp, 0);
	if(ip_bridge_dev_cmd(vty, BRCTL_DEL_BRIDGE, ifname, NULL)!=CMD_SUCCESS)
	{
		//if_delete(ifp);
		if(vty)
			vty_out (vty, "%% Can't delete bridge interface %s%s",ifname,VTY_NEWLINE);
		else
			zlog_debug(ZLOG_PAL, "%% Can't delete bridge interface %s",ifname);
		return CMD_WARNING;
	}
#endif
	//???????????????
	return CMD_SUCCESS;
}

#if 0
/* brigde interface ctl */
DEFUN (ip_bridge,
		ip_bridge_cmd,
		"interface bridge <0-32>",//"ip bridge NAME",
		INTERFACE_STR
	    "bridge Interface\n"
		"Interface name num\n")
{
#ifdef BRCTL_CMD_DEBUG
	char cmd[512];
#endif
	char ifname[INTERFACE_NAMSIZ];
	struct utils_interface *bifp = NULL;
	if(argv[0] == NULL)
	{
	      vty_out (vty, "%% invalid input bridge interface name is null %s",VTY_NEWLINE);
	      return CMD_WARNING;
	}
	memset(ifname, 0, sizeof(ifname));
	sprintf(ifname, "bridge%d", atoi(argv[0]));
	bifp = utils_interface_lookup_by_name (ifname);
	if(!bifp)
	{
		bifp = utils_interface_create(ifname);
		if(!bifp)
		{
			vty_out (vty, "%% Can't create bridge interface %s %s",ifname,VTY_NEWLINE);
			return CMD_WARNING;
		}
#ifdef BRCTL_CMD_DEBUG
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "brctl addbr %s",ifname);
		if(super_system(cmd)!=CMD_SUCCESS)
		{
			utils_interface_delete(bifp);
			vty_out (vty, "%% Can't create bridge interface %s",ifname,VTY_NEWLINE);
			return CMD_WARNING;
		}
#else
		if(ip_bridge_dev_cmd(vty, BRCTL_ADD_BRIDGE, ifname, NULL)!=CMD_SUCCESS)
		{
			utils_interface_delete(bifp);
			vty_out (vty, "%% Can't create bridge interface %s%s",ifname,VTY_NEWLINE);
			return CMD_WARNING;
		}
		bridge_updown(bifp, 1);
#endif
		if_get_ifindex(bifp);
	}
#ifdef BRCTL_CMD_DEBUG
	sprintf(cmd, "ip link set dev %s up",ifname);
	super_system(cmd);
#endif
	//bifp->index = atoi(argv[0]);
	bifp->type = UTILS_IF_BRIDGE;
	//bifp->br_mode = ZEBRA_INTERFACE_BRIDGE;
	bifp->vty = vty;
	vty->index = bifp->ifp;
	vty->node = INTERFACE_NODE;
	return CMD_SUCCESS;
}
DEFUN (no_ip_bridge,
		no_ip_bridge_cmd,
		"no interface bridge <0-32>",//"ip bridge NAME",
		NO_STR
		INTERFACE_STR
	    "bridge Interface\n"
		"Interface name num\n")
{
	char ifname[INTERFACE_NAMSIZ];
	if(argv[0] == NULL)
	{
	      vty_out (vty, "%% invalid input bridge interface name is null %s",VTY_NEWLINE);
	      return CMD_WARNING;
	}
	memset(ifname, 0, sizeof(ifname));
	sprintf(ifname, "bridge%d", atoi(argv[0]));
	return no_bridge_interface(vty, ifname);
}
/* ????????????????????? */
DEFUN (ip_bridge_add_sub,
		ip_bridge_add_sub_cmd,
	    "ip bridge sub NAME",
		IP_STR
	    "bridge Interface\n"
		"sub bridge Interface\n"
		"Interface Name\n")
{
#ifdef BRCTL_CMD_DEBUG
	char cmd[512];
#endif
	struct interface *ifp = NULL;
	struct utils_interface *bifp = NULL;
	bifp = ((struct interface *)vty->index)->info;
	//???????????????????????????
	if(bifp->type != UTILS_IF_BRIDGE)
	{
		vty_out (vty, "%% this cmd is't only use on bridge interface %s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argv[0] == NULL)
	{
	      vty_out (vty, "%% invalid input bridge interface name is null %s",VTY_NEWLINE);
	      return CMD_WARNING;
	}
	ifp = if_lookup_by_name (argv[0]);
	if(ifp == NULL)
	{
	      vty_out (vty, "%% Can't find interface %s %s",argv[0],VTY_NEWLINE);
	      return CMD_WARNING;
	}

	bifp = ifp->info;
	//??????????????????????????????
	if(bifp->type == UTILS_IF_BRIDGE )
	{
	      vty_out (vty, "%% this interface  bridge interface %s %s",bifp->name,VTY_NEWLINE);
	      return CMD_WARNING;
	}
	//??????????????????
	if(bifp->br_mode == ZEBRA_INTERFACE_SUB)
	{
	      vty_out (vty, "%% this interface %s is already bridge on %s",ifp->name,VTY_NEWLINE);
	      return CMD_WARNING;
	}
	bifp = ((struct interface *)vty->index)->info;
#ifdef BRCTL_CMD_DEBUG
	sprintf(cmd, "brctl addif %s %s",bifp->name, argv[0]);
	if(super_system(cmd)!=CMD_SUCCESS)
	{
			vty_out (vty, "%% interface %s Can't bridge on %s%s",bifp->name, argv[0], VTY_NEWLINE);
			return CMD_WARNING;
	}
#else
	if(ip_bridge_dev_cmd(vty, BRCTL_ADD_IF, bifp->name, argv[0])!=CMD_SUCCESS)
	{
		vty_out (vty, "%% interface %s Can't bridge on %s%s", argv[0],bifp->name, VTY_NEWLINE);
		return CMD_WARNING;
	}
#endif
	//????????????????????????
	bifp = ifp->info;
	bifp->br_mode = ZEBRA_INTERFACE_SUB;
	strcpy(bifp->br_name, ((struct interface *)vty->index)->name);
	return CMD_SUCCESS;
}
DEFUN (no_ip_bridge_add_sub,
		no_ip_bridge_add_sub_cmd,
	    "no ip bridge sub NAME",
		NO_STR
		IP_STR
	    "bridge Interface\n"
		"sub bridge Interface\n"
		"Interface Name\n")
{
#ifdef BRCTL_CMD_DEBUG
	char cmd[512];
#endif
	struct interface *ifp = NULL;
	struct utils_interface *bifp = NULL;
	bifp = ((struct interface *)vty->index)->info;
	//???????????????????????????
	if(bifp->type != UTILS_IF_BRIDGE)
	{
		vty_out (vty, "%% this interface is not bridge interface %s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argv[0] == NULL)
	{
	      vty_out (vty, "%% invalid input bridge interface name is null  %s",VTY_NEWLINE);
	      return CMD_WARNING;
	}
	ifp = if_lookup_by_name (argv[0]);
	if(ifp == NULL)
	{
	      vty_out (vty, "%% Can't find this interface %s %s",argv[0],VTY_NEWLINE);
	      return CMD_WARNING;
	}

	bifp = ifp->info;
	//?????????????????????????????????????????????
	if(bifp->type == UTILS_IF_BRIDGE )
	{
	      vty_out (vty, "%% this interface  bridge interface %s %s",bifp->name,VTY_NEWLINE);
	      return CMD_WARNING;
	}
	//??????????????????
	if((bifp->br_mode != ZEBRA_INTERFACE_SUB))
	{
	      vty_out (vty, "%% this interface is not bridge on %s",VTY_NEWLINE);
	      return CMD_WARNING;
	}
	bifp = ((struct interface *)vty->index)->info;
#ifdef BRCTL_CMD_DEBUG
	sprintf(cmd, "brctl delif %s %s",bifp->name, argv[0]);
	if(super_system(cmd)!=CMD_SUCCESS)
	{
		vty_out (vty, "%% Can't delete interface %s on bridge interface %s %s", argv[0],bifp->name, VTY_NEWLINE);
		return CMD_WARNING;
	}
#else
	if(ip_bridge_dev_cmd(vty, BRCTL_DEL_IF, bifp->name, argv[0])!=CMD_SUCCESS)
	{
		vty_out (vty, "%% Can't delete interface %s on bridge interface %s %s", argv[0], bifp->name, VTY_NEWLINE);
		return CMD_WARNING;
	}
#endif
	bifp = ifp->info;
	//????????????????????????
	bifp->br_mode = 0;
	return CMD_SUCCESS;
}
//??????????????????????????????
DEFUN (ip_bridge_on,
		ip_bridge_on_cmd,
	    "ip bridge on NAME",
		IP_STR
	    "bridge Interface\n"
		"bridge on Interface\n"
		"bridge Interface Name\n")
{
#ifdef BRCTL_CMD_DEBUG
	char cmd[512];
#endif
	struct interface *ifp = NULL;
	struct utils_interface *bifp = NULL;
	ifp = vty->index;
	bifp = (struct utils_interface *)ifp->info;
	//??????????????????????????????
	if((bifp->type == UTILS_IF_BRIDGE))
	{
	      vty_out (vty, "%% this interface is not bridge interface %s",VTY_NEWLINE);
	      return CMD_WARNING;
	}
	//???????????????????????????
	if(bifp->br_mode == ZEBRA_INTERFACE_SUB)
	{
		vty_out (vty, "%% this interface is already bridge on %s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argv[0] == NULL)
	{
	      vty_out (vty, "%% invalid input bridge interface name is null %s",VTY_NEWLINE);
	      return CMD_WARNING;
	}
	//????????????
	bifp = utils_interface_lookup_by_name (argv[0]);
	if(bifp == NULL)
	{
	      vty_out (vty, "%% Can't find bridge interface %s",VTY_NEWLINE);
	      return CMD_WARNING;
	}
	//??????????????????
	if((bifp->type == UTILS_IF_BRIDGE))
	{
	      vty_out (vty, "%% this interface is not bridge interface %s",VTY_NEWLINE);
	      return CMD_WARNING;
	}
#ifdef BRCTL_CMD_DEBUG
	sprintf(cmd, "brctl addif %s %s", argv[0], ifp->name);
	if(super_system(cmd)!=CMD_SUCCESS)
	{
		vty_out (vty, "%% interface %s Cna't bridge on interface %s%s",argv[0], ifp->name,VTY_NEWLINE);
		return CMD_WARNING;
	}
#else
	if(ip_bridge_dev_cmd(vty, BRCTL_ADD_IF, argv[0], ifp->name)!=CMD_SUCCESS)
	{
		vty_out (vty, "%% interface %s Cna't bridge on interface %s%s",argv[0], ifp->name,VTY_NEWLINE);
		return CMD_WARNING;
	}
#endif
	//????????????????????????
	bifp = ifp->info;
	bifp->br_mode = ZEBRA_INTERFACE_SUB;
	strcpy(((struct utils_interface *)ifp->info)->br_name, argv[0]);
	return CMD_SUCCESS;
}
DEFUN (no_ip_bridge_on,
		no_ip_bridge_on_cmd,
	    "no ip bridge on NAME",
		NO_STR
		IP_STR
	    "bridge Interface\n"
		"bridge on Interface\n"
		"Interface Name\n")
{
#ifdef BRCTL_CMD_DEBUG
	char cmd[512];
#endif
	struct interface *ifp = NULL;
	struct utils_interface *bifp = NULL;
	ifp = vty->index;
	bifp = (struct utils_interface *)ifp->info;
	//??????????????????????????????
	if((bifp->type == UTILS_IF_BRIDGE))
	{
	      vty_out (vty, "%% this interface is not bridge interface %s",VTY_NEWLINE);
	      return CMD_WARNING;
	}
	//???????????????????????????
	if((bifp->br_mode != ZEBRA_INTERFACE_SUB))
	{
		vty_out (vty, "%% this interface is not bridge on %s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(argv[0] == NULL)
	{
	      vty_out (vty, "%% invalid input bridge interface name is null %s",VTY_NEWLINE);
	      return CMD_WARNING;
	}
	bifp = utils_interface_lookup_by_name (argv[0]);
	if(bifp == NULL)
	{
	      vty_out (vty, "%% Can't find bridge interface %s %s",argv[0],VTY_NEWLINE);
	      return CMD_WARNING;
	}
	//??????????????????
	if((bifp->type != UTILS_IF_BRIDGE))
	{
	      vty_out (vty, "%% this interface is not a bridge interface %s",VTY_NEWLINE);
	      return CMD_WARNING;
	}

#ifdef BRCTL_CMD_DEBUG
	sprintf(cmd, "brctl delif %s %s", argv[0], ifp->name);
	if(super_system(cmd)!=CMD_SUCCESS)
	{
		vty_out (vty, "%% Can't delete interface %s on bridge interface %s %s",ifp->name, argv[0], VTY_NEWLINE);
		return CMD_WARNING;
	}
#else
	if(ip_bridge_dev_cmd(vty, BRCTL_DEL_IF, argv[0], ifp->name)!=CMD_SUCCESS)
	{
		vty_out (vty, "%% Can't delete interface %s on bridge interface %s %s",ifp->name, argv[0], VTY_NEWLINE);
		return CMD_WARNING;
	}
#endif
	//????????????????????????
	bifp = ifp->info;
	bifp->br_mode = 0;
	return CMD_SUCCESS;
}
DEFUN (ip_bridge_stp_on,
		ip_bridge_stp_on_cmd,
	    "ip bridge stp on",
		IP_STR
	    "bridge Interface\n"
		"stp on bridge Interface\n"
		"open stp\n")
{
#ifdef BRCTL_CMD_DEBUG
	char cmd[512];
#endif
	int ret = 0;
	struct utils_interface *bifp = NULL;
	bifp = ((struct interface *)vty->index)->info;
	//??????????????????
	if(bifp->type != UTILS_IF_BRIDGE)
	{
		vty_out (vty, "%% this interface is not bridge interface %s",VTY_NEWLINE);
		return CMD_WARNING;
	}
#ifdef BRCTL_CMD_DEBUG
	sprintf(cmd, "brctl stp %s on", bifp->name);
	if(super_system(cmd)!=CMD_SUCCESS)
	{
		vty_out (vty, "%% Can't enable stp on bridge interface %s %s",bifp->name,VTY_NEWLINE);
		return CMD_WARNING;
	}
#else
	ret = ip_bridge_option_cmd(vty, BRCTL_SET_BRIDGE_STP_STATE, bifp->name, NULL, 1);
	if(ret!=CMD_SUCCESS)
	{
		vty_out (vty, "%% Can't enable stp on bridge interface %s %s",bifp->name,VTY_NEWLINE);
		return CMD_WARNING;
	}
#endif
	bifp->br_stp = 1;
	return CMD_SUCCESS;
}
DEFUN (no_ip_bridge_stp_on,
		no_ip_bridge_stp_on_cmd,
	    "ip bridge stp off",
		IP_STR
	    "bridge Interface\n"
		"stp on bridge Interface\n"
		"close stp\n")
{
#ifdef BRCTL_CMD_DEBUG
	char cmd[512];
#endif
	int ret = 0;
	struct utils_interface *bifp = NULL;
	bifp = ((struct interface *)vty->index)->info;
	//??????????????????
	if(bifp->type != UTILS_IF_BRIDGE)
	{
		vty_out (vty, "%% this interface is not bridge interface  %s",VTY_NEWLINE);
		return CMD_WARNING;
	}
#ifdef BRCTL_CMD_DEBUG
	sprintf(cmd, "brctl stp %s off", bifp->name);
	if(super_system(cmd)!=CMD_SUCCESS)
	{
		vty_out (vty, "%% Can't disable stp on bridge interface %s %s",bifp->name,VTY_NEWLINE);
		return CMD_WARNING;
	}
#else
	ret = ip_bridge_option_cmd(vty, BRCTL_SET_BRIDGE_STP_STATE, bifp->name, NULL, 0);
	if(ret!=CMD_SUCCESS)
	{
		vty_out (vty, "%% Can't disable stp on bridge interface %s %s",bifp->name,VTY_NEWLINE);
		return CMD_WARNING;
	}
#endif
	bifp->br_stp = 0;
	return CMD_SUCCESS;
}

static int show_bridge_interface_detail(struct vty *vty, struct interface *ifp)
{
	int i = 0;
	struct utils_interface *uifp = ifp->info;
	//show_utils_interface(vty, ifp);
	if(uifp->type != UTILS_IF_BRIDGE && uifp->type == ZEBRA_INTERFACE_SUB)
	{
		const char *stp_str[] = {"Disable","Listening","Learning","Forwarding","Blocking"};
		show_utils_interface(vty, ifp);
		vty_out (vty, "  bridge on %s",uifp->br_name);
		if(uifp->br_stp_state >=BR_STATE_DISABLED && uifp->br_stp_state <= BR_STATE_BLOCKING)
			vty_out (vty, " stp state %s%s",stp_str[uifp->br_stp_state],VTY_NEWLINE);
		else
			vty_out (vty, " stp state Unknow%s",VTY_NEWLINE);
	}
	if(uifp->type == UTILS_IF_BRIDGE)
	{
		show_utils_interface(vty, ifp);
		vty_out (vty, "  stp protocol %s",uifp->br_stp? "on":"off");
		vty_out (vty, " max age %d",uifp->max_age);
		vty_out (vty, " hello time %d",uifp->hello_time);
		vty_out (vty, " forward delay %d%s",uifp->forward_delay,VTY_NEWLINE);
		for(i = 0; i < BR_PORT_MAX; i++)
		{
			vty_out (vty, "  sub interface:");
			if(uifp->br_ifindex[i] && ifindex2ifname(uifp->br_ifindex[i]))
			 vty_out (vty, "  %s",ifindex2ifname(uifp->br_ifindex[i]));
			vty_out (vty, "%s",VTY_NEWLINE);
		}
#ifndef BRCTL_CMD_DEBUG
		ip_bridge_info(vty, uifp->name);
#endif
	}
	return CMD_SUCCESS;
}
DEFUN (show_bridge_interface,
		show_bridge_interface_cmd,
	    "show bridge interface [NAME]",
		SHOW_STR
	    "bridge Interface\n"
		INTERFACE_STR
		"Interface Name\n")
{
#ifdef BRCTL_CMD_DEBUG
	char cmd[512];
	if(argv[0] == NULL)
	{
		super_system("brctl show");
	}
	else
	{
		sprintf(cmd, "brctl show %s", argv[0]);
		super_system(cmd);
	}
#else
	struct interface *ifp = NULL;
	if(argv[0] != NULL && argc == 1)
	{
		ifp = if_lookup_by_name (argv[0]);
		if(!ifp)
		{
			vty_out (vty, "%% Can't lookup interface %s %s",argv[0],VTY_NEWLINE);
			return CMD_WARNING;
		}
		show_bridge_interface_detail(vty, ifp);
	}
	else
	{
		struct listnode *node;
		for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
		{
			show_bridge_interface_detail(vty, ifp);
		}
	}
#endif
	return CMD_SUCCESS;
}
DEFUN (show_bridge_table,
		show_bridge_table_cmd,
	    "show bridge-table",
		SHOW_STR
	    "bridge Interface\n"
		INTERFACE_STR
		"Interface Name\n")
{
		  struct listnode *node;
		  struct interface *ifp;
		  struct utils_interface *uifp;
		  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
		    {
			  uifp = (struct utils_interface *)ifp->info;
			  //if(uifp->status == ZEBRA_INTERFACE_BRIDGE)
			  {
				  vty_out (vty, "Interface %s%s", ifp->name, VTY_NEWLINE);
				  vty_out (vty, " sub Interface %s%s", uifp->name, VTY_NEWLINE);
				  vty_out (vty, " Interface bridge on %s%s", uifp->br_name, VTY_NEWLINE);
				  vty_out (vty, " Interface type 0x%x%s", uifp->type, VTY_NEWLINE);
				  vty_out (vty, " Interface mode 0x%x%s", uifp->br_mode, VTY_NEWLINE);
				  vty_out (vty, " Interface stp %d%s", uifp->br_stp, VTY_NEWLINE);
				  //ip_bridge_info(vty, ifp->name);
			  }
		    }
	return CMD_SUCCESS;
}

int bridge_interface_config_write (struct vty *vty, struct interface *ifp)
{
	struct utils_interface *uifp = ifp->info;
	if( (uifp && uifp->br_mode == ZEBRA_INTERFACE_SUB) && uifp->name)
	{
		vty_out (vty, " ip bridge on %s%s", uifp->br_name,VTY_NEWLINE);
		vty_out (vty, " ip bridge stp %s%s", uifp->br_stp? "on":"off",VTY_NEWLINE);
	}
	return CMD_SUCCESS;
}

void utils_bridge_cmd_init (void)
{
	install_element (ENABLE_NODE, &show_bridge_interface_cmd);
	install_element (ENABLE_NODE, &show_bridge_table_cmd);

	install_element (CONFIG_NODE, &ip_bridge_cmd);
	install_element (CONFIG_NODE, &no_ip_bridge_cmd);

	install_element (INTERFACE_NODE, &ip_bridge_add_sub_cmd);
	install_element (INTERFACE_NODE, &no_ip_bridge_add_sub_cmd);
	install_element (INTERFACE_NODE, &ip_bridge_on_cmd);
	install_element (INTERFACE_NODE, &no_ip_bridge_on_cmd);

	install_element (INTERFACE_NODE, &ip_bridge_stp_on_cmd);
	install_element (INTERFACE_NODE, &no_ip_bridge_stp_on_cmd);
}
#endif
#endif

